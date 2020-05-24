/**********************************************************************
*   This file is part of Search and Rescue II (SaR2).                 *
*                                                                     *
*   SaR2 is free software: you can redistribute it and/or modify      *
*   it under the terms of the GNU General Public License v.2 as       *
*   published by the Free Software Foundation.                        *
*                                                                     *
*   SaR2 is distributed in the hope that it will be useful, but       *
*   WITHOUT ANY WARRANTY; without even the implied warranty of        *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See          *
*   the GNU General Public License for more details.                  *
*                                                                     *
*   You should have received a copy of the GNU General Public License *
*   along with SaR2.  If not, see <http://www.gnu.org/licenses/>.     *
***********************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#include "../include/disk.h"

#include "sfm.h"
#include "horizon.h"
#include "sarreality.h"
#include "sound.h"
#include "obj.h"
#include "sar.h"
#include "objutils.h"
#include "sartime.h"
#include "simcb.h"
#include "simutils.h"
#include "simsurface.h"
#include "simcontact.h"
#include "simop.h"
#include "simmanage.h"
#include "config.h"


static void SARSimGenerateLighteningPoints(
	sar_position_struct *point, int points,
	const sar_position_struct *pos,
	float width, float height
);

int SARSimUpdateScene(
	sar_core_struct *core_ptr, sar_scene_struct *scene
);
int SARSimUpdateSceneObjects(
	sar_core_struct *core_ptr, sar_scene_struct *scene
);


#define POW(x,y)        (((x) > 0.0f) ? pow(x,y) : 0.0f)

#define STRDUP(s)	(((s) != NULL) ? strdup(s) : NULL)

#define MAX(a,b)        (((a) > (b)) ? (a) : (b))
#define MIN(a,b)        (((a) < (b)) ? (a) : (b))
#define CLIP(a,l,h)     (MIN(MAX((a),(l)),(h)))

#define DEGTORAD(d)     ((d) * PI / 180.0)
#define RADTODEG(r)     ((r) * 180.0 / PI)

#define PLAY_SOUND(p)                                           \
{ if((opt->event_sounds) && (recorder != NULL)) {               \
 char *full_path = STRDUP((ISPATHABSOLUTE(p)) ?                 \
  (p) : PrefixPaths(dname.global_data, (p))                     \
 );                                                             \
 SoundStartPlayVoid(                                            \
  recorder, full_path, 1.0, 1.0, 1.0f, 0                        \
 );                                                             \
 free(full_path);                                               \
} }


/*
 *	Generates random lightening points
 */
static void SARSimGenerateLighteningPoints(
	sar_position_struct *point, int points,
	const sar_position_struct *pos,
	float width, float height
)
{
	/* Generate points starting at the cloud's base and go all the
	 * way down to the ground, we need to start at the cloud base
	 * as Z = 0 and reach the ground as Z = -pos->z
	 *
	 * The width of the cloud will help determine the amount of
	 * "zigzagging" the lightening will make, the zigzagging will
	 * be no more than 50% of the width
	 *
	 * The starting Z position z_cur will be from the base of the
	 * cloud to 25% of the height
	 */
	int i;
	unsigned int seed = cur_millitime;
	sar_position_struct *pt, *last_pt = NULL;
	float	z_cur = height * (SARRandomCoeff(seed) * 0.25f),
		z_inc = 0.0f;

	/* Set starting point, the origin of the lightening */
	if(points >= 2)
	{
	    last_pt = pt = &point[0];	/* Get first point */

	    /* Set origin of lightening */
	    pt->x = width * (SARRandomCoeff(seed) - 0.5f);
	    pt->y = width * (SARRandomCoeff(seed) - 0.5f);
	    pt->z = z_cur;

	    /* Calculate absolute Z increment between points */
	    z_inc = (z_cur + pos->z) / (points - 1);

	    z_cur -= z_inc;	/* Increment for next point */
	}

	/* Set subsequent points */
	for(i = 1; i < points; i++)
	{
	    pt = &point[i];

	    if(last_pt != NULL)
	    {
		/* Set point using last point as a reference */
		pt->x = last_pt->x +
		    (width * (SARRandomCoeff(seed) - 0.5f) * 0.2f);
		pt->y = last_pt->y +
		    (width * (SARRandomCoeff(seed) - 0.5f) * 0.2f);
		pt->z = z_cur;
	    }

	    /* Increment current Z position and record last point */
	    z_cur -= z_inc;
	    last_pt = pt;
	}
}


/*
 *	Updates the given scene structure's members (but does not update
 *	any objects).
 *
 *	Time of day will be updated on the scene, then along with the
 *	primary light position light_pos and color light_color.
 *
 *	Horizon texture will be (re)generated as need per a given interval
 *	(every 5 minutes).
 *
 *	Returns 0 on success and non-zero on error or memory pointer
 *	change.
 */
int SARSimUpdateScene(sar_core_struct *core_ptr, sar_scene_struct *scene)
{
	float scene_lumination_coeff, horizon_sat_max_coeff;
	int prev_tod_code, new_tod_code;
	sar_position_struct *pos;
	sar_color_struct *c;
	sar_scene_horizon_struct *horizon;
	snd_recorder_struct *recorder = core_ptr->recorder;
	const sar_option_struct *opt = &core_ptr->option;
	if(scene == NULL)
	    return(-1);

/* Converts hours into seconds */
#define HTOS(h)	((h) * 3600.0)


	/* Time Of Day */

	/* Update time of day since midnight, in seconds */
	scene->tod += (float)lapsed_millitime * time_compression / 1000.0f;

	/* Sanitize time of day */
	while(scene->tod >= HTOS(24.0))
	    scene->tod -= HTOS(24.0);
	while(scene->tod < HTOS(0.0))
	    scene->tod += HTOS(24.0);

	/* Record previous time of day code */
	prev_tod_code = scene->tod_code;

	/* Set time of day code depending on new time of day (which was
	 * updated above)
	 */
	if(scene->tod < HTOS(6.0))		/* Before 6:00 am */
	    new_tod_code = SAR_TOD_CODE_NIGHT;
	else if(scene->tod < HTOS(7.0))		/* Before 7:00 am */
	    new_tod_code = SAR_TOD_CODE_DAWN;
	else if(scene->tod < HTOS(17.0))	/* Before 5:00 pm */
	    new_tod_code = SAR_TOD_CODE_DAY;
	else if(scene->tod < HTOS(18.0))	/* Before 6:00 pm */
	    new_tod_code = SAR_TOD_CODE_DUSK;
	else					/* After 6:00 pm */
	    new_tod_code = SAR_TOD_CODE_NIGHT;

	scene->tod_code = new_tod_code;		/* Set new tod_code */


	/* Global Lumination */

	/* Calculate global scene lumination coeff for time 4:00 to
	 * 20:00, where 0.0 is darkest and 1.0 is brightest. Darkest
	 * occures at 4:00 and 20:00, brightest is at 12:00
	 *
	 * Also calculate horizon saturation extreme coefficient, the
	 * coefficient to determine the amount of dawn/dusk saturation
	 * on the horizon
	 */

	/* First calculate from -1.0 to 1.0 linearly where -1.0 is
	 * at 4:00 and 1.0 is at 20:00
	 */
	scene_lumination_coeff = (float)((scene->tod - HTOS(12.0)) / HTOS(8.0));
	if(scene_lumination_coeff < 0.0f)
	{
	    /* Before 12:00 */
	    float pow_coeff = (float)POW(-scene_lumination_coeff, 2);

	    /* Horizon saturation coefficient, calculate from
	     * scene lumination so far (with power of 2)
	     */
	    horizon_sat_max_coeff = (float)CLIP(
		1.0 - pow_coeff,
		0.0, 1.0
	    );

	    /* Scene lumination; flip, change sign, and apply non-linear
	     * curvature (power of 5)
	     */
	    pow_coeff = (float)POW(-scene_lumination_coeff, 5);
	    scene_lumination_coeff = (float)CLIP(
		1.0 - pow_coeff,
		0.0, 1.0
	    );
	}
	else
	{
	    /* 12:00 or later */
	    float pow_coeff = (float)POW(scene_lumination_coeff, 2);

	    /* Horizon saturation coefficient, calculate from
	     * scene lumination so far (with power of 2)
	     */
	    horizon_sat_max_coeff = (float)CLIP(
		1.0 - pow_coeff, 0.0, 1.0
	    );

	    /* Scene lumination; flip and apply non-linear
	     * curvature (power of 5)
	     */
	    pow_coeff = (float)POW(scene_lumination_coeff, 5);
	    scene_lumination_coeff = (float)CLIP(
		1.0 - pow_coeff, 0.0, 1.0
	    );
	}


	/* Sun (Scene's Global Light) */

	/* Move scene's global light position relative to the origin based
	 * on the time of day. So this position is really an offset that
	 * needs to be added to the actual camera's position
	 *
	 * The scene's global light is in a orbital pattern depending on
	 * the time of day, to simulate it as the sun
	 */
	pos = &scene->light_pos;	/* Scene global light position */
	if(scene->tod > HTOS(12.0))
	{
	    float orbital_coeff;

	    /* Afternoon or later, calculate orbital position coefficient
	     * linearly from 12:00 (coeff 1.0) to 19:00 (coeff 0.0)
	     */
	    orbital_coeff = (float)(1.0 - MIN(
		(scene->tod - HTOS(12.0)) / HTOS(7.0),
		1.0
	    ));

	    /* Calculate longitude aligned orbital position of light
	     * relative to camera's XY plane
	     */
	    pos->x = (float)(-cos(orbital_coeff * (0.5 * PI)));
	    pos->y = 0.0f;	/* Calculated later */
	    pos->z = (float)(sin(orbital_coeff * (0.5 * PI)));
	}
	else
	{
	    float orbital_coeff;

	    /* Morning, calculate orbital position coefficient
	     * linearly from 2:00 (coeff 0.0) to 12:00 (coeff 1.0)
	     */
	    orbital_coeff = (float)MAX(
		(scene->tod - HTOS(5.0)) / HTOS(7.0),
		0.0
	    );

	    /* Calculate longitude aligned orbital position of light    
	     * relative to camera's XY plane
	     */
	    pos->x = (float)(cos(orbital_coeff * (0.5 * PI)));
	    pos->y = 0.0f;	/* Calculated later */
	    pos->z = (float)(sin(orbital_coeff * (0.5 * PI)));
	}
	/* Apply latitude offset to scene's global light position */
	if(1)
	{
	    float	lat_offset_deg = scene->dms_y_offset,
			lat_offset_rad,
			len;

	    /* Calculate latitude offset in radians */
	    lat_offset_rad = (float)CLIP(
		DEGTORAD(lat_offset_deg),
		-0.5 * PI, 0.5 * PI
	    );
	    pos->y = (float)(pos->y + sin(-lat_offset_rad));
/*	    pos->z = MAX(pos->z - sin(-lat_offset_rad), 0.0); */

	    /* Make into unit length */
	    len = (float)SFMHypot3(pos->x, pos->y, pos->z);
	    if(len > 0.0f)
	    {
		pos->x = pos->x / len;
		pos->y = pos->y / len;
		pos->z = pos->z / len;
	    }

	    /* Update light position hint indicating if it is above or
	     * below the horizon (foundation of z = 0.0)
	     */
	    scene->light_visibility_hint = (pos->z > 0.0f) ? 1 : 0;
	}

	/* Set scene global lighting to match sun lumination */
	c = &scene->light_color;	/* Scene global light color */
	c->a = 1.0f;
	c->r = scene_lumination_coeff;
	c->g = scene_lumination_coeff;
	c->b = scene_lumination_coeff;


	/* Moon */

	/* Move moon position based on time of day */
	pos = &scene->moon_pos;		/* Scene global moon position */
	if(scene->tod > HTOS(12.0))
	{
	    /* Before midnight, calculate orbital position coefficient
	     * linearly from 17:00 (coeff 0.0) to 24:00 (coeff 1.0).
	     */
	    float orbital_coeff = (float)MAX(
		(scene->tod - HTOS(17.0)) / HTOS(7.0),
		0.0
	    );

	    /* Calculate longitude aligned orbital position of light
	     * relative to camera's xy plane.
	     */
	    pos->x = (float)(cos(orbital_coeff * (0.5 * PI)));
	    pos->y = 0.0f;	/* Calculated later */
	    pos->z = (float)(sin(orbital_coeff * (0.5 * PI)));
	}
	else
	{
	    float orbital_coeff;

	    /* After midnight, calculate orbital position coefficient
	     * linearly from 0:00 (coeff 1.0) to 7:00 (coeff 0.0).
	     */
	    orbital_coeff = (float)(1.0 - MIN(
		(scene->tod - HTOS(0.0)) / HTOS(7.0),
		1.0
	    ));

	    /* Calculate longitude aligned orbital position of light
	     * relative to camera's xy plane.
	     */
	    pos->x = (float)(-cos(orbital_coeff * (0.5 * PI)));
	    pos->y = 0.0f;       /* Calculated later */
	    pos->z = (float)(sin(orbital_coeff * (0.5 * PI)));
	}
	if(1)
	{
	    float	lat_offset_deg = scene->dms_y_offset,
			lat_offset_rad,
			len;

	    /* Calculate latitude offset in radians */
	    lat_offset_rad = (float)CLIP(
		DEGTORAD(lat_offset_deg),
		-0.5 * PI, 0.5 * PI
	    );
	    pos->y = (float)(pos->y + sin(-lat_offset_rad));
/*	    pos->z = MAX(pos->z - sin(-lat_offset_rad), 0.0); */

	    /* Make into unit length */
	    len = (float)SFMHypot3(pos->x, pos->y, pos->z);
	    if(len > 0.0f)
	    {
		pos->x = pos->x / len;
		pos->y = pos->y / len;
		pos->z = pos->z / len;
	    }

	    /* Update moon position hint indicating if it is above or
	     * below the horizon (foundation of z = 0.0)
	     */
	    scene->moon_visibility_hint = (pos->z > 0.0f) ? 1 : 0;
	}


	/* Horizon */

	/* Regenerate the Horizon every 5 minutes */
	horizon = &scene->horizon;
	if(horizon->last_tod != (int)(scene->tod / (5 * 60)))
	{
	    int i, total, tex_num;
	    float rc, gc, bc;		/* Coefficient color change */
	    float darken_coeff, midpoint = 0.88f;
	    sar_color_struct start_color, end_color;


	    if(opt->runtime_debug)
		printf(
 "SARSimUpdateScene(): Updating horizon textures.\n"
		);

	    /* Update last tod on the Horizon to the current time in
	     * 5 minute units so we know when to update the Horizon
	     * again
	     */
	    horizon->last_tod = (int)(scene->tod / (5 * 60));

/* Creates a new horizon texture on the horizon */
#define DO_CREATE_TEXTURE	{			\
 tex_num = MAX(horizon->total_textures, 0);		\
 horizon->total_textures = tex_num + 1;			\
 horizon->texture = (v3d_texture_ref_struct **)realloc(	\
  horizon->texture,					\
  horizon->total_textures * sizeof(v3d_texture_ref_struct *) \
 );							\
 if(horizon->texture == NULL)				\
 {							\
  horizon->total_textures = 0;				\
 }							\
 else							\
 {							\
  horizon->texture[tex_num] = SARCreateHorizonTexture(	\
   NULL,	/* No name */				\
   &start_color, &end_color,				\
   32, midpoint						\
  );							\
 }							\
}

	    /* Adjust gradient horizon textures by recreating them.
	     * Do not add lumination/gamma to them, the light color
	     * will be multiplied during drawing
	     */

	    for(i = 0; i < horizon->total_textures; i++)
		V3DTextureDestroy(horizon->texture[i]);
	    free(horizon->texture);
	    horizon->texture = NULL;
	    horizon->total_textures = 0;


	    /* Create textures, starting with most brightest */
	    memcpy(
		&start_color, &scene->sky_nominal_color,
		sizeof(sar_color_struct)
	    );

	    /* Create 7 horizon textures from order of brightest to
	     * to darkest
	     */
	    total = 7;
	    for(i = 0; i < total; i++)
	    {
		/* Calculate darken coefficient based on index of
		 * the current horizon texture
		 */
		darken_coeff = (float)i / (float)(total - 1);

		/* Calculate rgb blend values from 1.0 to 2.0 */
		rc = (float)MIN(
		    (scene->sky_brighten_color.r * (1.0 - darken_coeff)) +
		    (scene->sky_darken_color.r * darken_coeff),
		    1.0
		);
		gc = (float)MIN(
		    (scene->sky_brighten_color.g * (1.0 - darken_coeff)) +
		    (scene->sky_darken_color.g * darken_coeff),
		    1.0
		);
		bc = (float)MIN(
		    (scene->sky_brighten_color.b * (1.0 - darken_coeff)) +
		    (scene->sky_darken_color.b * darken_coeff),
		    1.0
		);

		/* Calculate end color */
		end_color.a = 1.0f;
		end_color.r = (float)MIN(
		    (1.0 * horizon_sat_max_coeff) +
			(rc * (1.0 - horizon_sat_max_coeff)),
		    1.0
		);
		end_color.g = (float)MIN(
		    (1.0 * horizon_sat_max_coeff) +
			(gc * (1.0 - horizon_sat_max_coeff)), 
		    1.0
		);
		end_color.b = (float)MIN(
		    (1.0 * horizon_sat_max_coeff) +
			(bc * (1.0 - horizon_sat_max_coeff)),
		    1.0
		);

		DO_CREATE_TEXTURE
	    }
#undef DO_CREATE_TEXTURE
	}


	/* Cloud Billboards */
	if(1)
	{
	    int i;
	    sar_cloud_bb_struct *cloud;

	    /* Reset Primary Lightening Coeff */
	    scene->pri_lightening_coeff = 0.0f;

	    /* Iterate through each Cloud Billboard */
	    for(i = 0; i < scene->total_cloud_bbs; i++)
	    {
		cloud = scene->cloud_bb[i];
		if(cloud == NULL)
		    continue;

		/* Has lightening? */
		if(cloud->lightening_min_int > 0)
		{
		    /* Lightening currently off and waiting to go on? */
		    if(cloud->lightening_next_on > 0)
		    {
			/* Time to go on? */
			if(cloud->lightening_next_on <= cur_millitime)
			{
			    const time_t ligthening_on_int = 1800l;

			    /* Generate random lightening points */
			    SARSimGenerateLighteningPoints(
				cloud->lightening_point,
				SAR_LIGHTENING_POINTS_MAX,
				&cloud->pos,
				cloud->width, cloud->height
			    );

			    /* Schedual next off time */
			    cloud->lightening_next_off = cur_millitime +
				ligthening_on_int;
			    cloud->lightening_started_on = cur_millitime;
			    cloud->lightening_next_on = 0;

			    /* Update the Primary Lightening Coeff */
			    scene->pri_lightening_coeff = 1.0f;
			}
		    }
		    else
		    {
			/* Time to go off? */
			if(cloud->lightening_next_off <= cur_millitime)
			{
			    /* Turn lightening off, play thunder sound,
			     * and schedual next lightening on
			     */
			    const time_t dt = cloud->lightening_max_int -
				cloud->lightening_min_int;

			    /* If was on and now going off, then play
			     * thunder sound
			     */
			    if(cloud->lightening_next_off > 0)
			    {
				switch(SARRandom(cur_millitime) % 3)
				{
				  case 2:
				    PLAY_SOUND(SAR_DEF_SOUND_THUNDER_LOUD);
				    break;
				  case 1:
				    PLAY_SOUND(SAR_DEF_SOUND_THUNDER_MODERATE);
				    break;
				  case 0:
				    PLAY_SOUND(SAR_DEF_SOUND_THUNDER_FAINT);
				    break;
				}
			    }

			    /* Schedual next on time */
			    cloud->lightening_next_off = 0;
			    cloud->lightening_started_on = 0;
			    cloud->lightening_next_on = cur_millitime +
				cloud->lightening_min_int +
				(time_t)(dt * SARRandomCoeff(cur_millitime));
			}
			else
			{
			    /* Ligthening is on, update the Primary
			     * Lightening Coeff
			     */
			    const float	t = (float)(
				cur_millitime - cloud->lightening_started_on
			    ),
					dt = (float)(
				cloud->lightening_next_off -
				cloud->lightening_started_on
			    );
			    const float c = (dt > 0.0f) ?
				(1.0f - (t / dt)) : 0.0f;
			    if(c > scene->pri_lightening_coeff)
				scene->pri_lightening_coeff = c;
			}
		    }
		}	/* Has lightening? */



	    }
	}






#undef HTOS

	return(0);
}

/*
 *	First updates the timings on the scene structure's SFMRealmStruct
 *	and then updates all objects with respect to the given scene and
 *	core structure.
 *
 *	Returns 0 on success and non-zero on error or memory pointer
 *	change.
 */
int SARSimUpdateSceneObjects(
	sar_core_struct *core_ptr, sar_scene_struct *scene
)
{
	int i, status;
	int *obj_total;
	sar_object_struct *obj_ptr, ***obj_pa;
	sar_object_aircraft_struct *obj_aircraft_ptr;
	sar_contact_bounds_struct *cb;
	SFMModelStruct *fdm;
	if(scene == NULL)
	    return(-1);

	/* Update timing on realm */
	if(scene->realm != NULL)
	{
	    SFMSetTiming(scene->realm, lapsed_millitime);
	    SFMSetTimeCompression(scene->realm, time_compression);
	}


	/* Begin handling each object on core structure */
	obj_pa = &core_ptr->object;
	obj_total = &core_ptr->total_objects;
	for(i = 0; i < (*obj_total); i++)
	{
	    obj_ptr = (*obj_pa)[i];
	    if(obj_ptr == NULL)
		continue;

	    status = 0;		/* Reset `SFM handled' value */

	    /* SFM realm structure allocated? */
	    if(scene->realm != NULL)
	    {
		/* Handle by object type */
		switch(obj_ptr->type)
		{
		  case SAR_OBJ_TYPE_GARBAGE:
		  case SAR_OBJ_TYPE_STATIC:
		  case SAR_OBJ_TYPE_AUTOMOBILE:
		  case SAR_OBJ_TYPE_WATERCRAFT:
		    break;

		  case SAR_OBJ_TYPE_AIRCRAFT:
		    obj_aircraft_ptr = SAR_OBJ_GET_AIRCRAFT(obj_ptr);
		    if(obj_aircraft_ptr == NULL)
			break;

		    fdm = obj_aircraft_ptr->fdm;
		    if(fdm == NULL)
			break;

		    /* Set object values to SFM structure */
		    SARSimSetSFMValues(core_ptr, scene, obj_ptr);

		    /* If this is the player object then apply control
		     * positions
		     */
		    if(obj_ptr == scene->player_obj_ptr)
		    {
			/* Apply player control */
			SARSimApplyGCTL(core_ptr, obj_ptr);
		    }
		    else
		    {
			/* Apply AI */
/* TODO */
		    }

		    /* Set status to 1, noting that the SFM structure
		     * was handled (which we will actually do so just
		     * below)
		     */
		    status = 1;

		    /* Begin SFM updating */
		    if(SFMForceApplyArtificial(scene->realm, fdm))
			break;
/* Swapped, calculating natural forces after artifical gives us
 * more accurate center to ground height.
 */
		    if(SFMForceApplyNatural(scene->realm, fdm))
			break;
		    if(SFMForceApplyControl(scene->realm, fdm))
			break;

		    SFMSetAirspeed(scene->realm, fdm);
		    /* Get new object values from the updated SFM */
		    SARSimGetSFMValues(scene, obj_ptr);
		    break;

		  case SAR_OBJ_TYPE_GROUND:
		  case SAR_OBJ_TYPE_RUNWAY:
		  case SAR_OBJ_TYPE_HELIPAD:
		  case SAR_OBJ_TYPE_HUMAN:
		  case SAR_OBJ_TYPE_SMOKE:
		  case SAR_OBJ_TYPE_FIRE:
		  case SAR_OBJ_TYPE_EXPLOSION:
		  case SAR_OBJ_TYPE_CHEMICAL_SPRAY:
		  case SAR_OBJ_TYPE_FUELTANK:
		  case SAR_OBJ_TYPE_PREMODELED:
		    break;
		}
	    }
	    /* Check if object survived SFM handling, in other words is
	     * it still allocated?
	     */
	    if(status)
	    {
		/* SFM was handled on the object, so check if object
		 * is still allocated.
		 */
		if(SARObjIsAllocated(*obj_pa, *obj_total, i))
		    obj_ptr = (*obj_pa)[i];
		else
		    continue;
	    }

	    /* Apply natural forces to object */
	    if(SARSimApplyNaturalForce(core_ptr, obj_ptr))
		continue;

	    /* Apply artificial forces to object */
	    if(SARSimApplyArtificialForce(core_ptr, obj_ptr))
		continue;


	    /* Get pointer to object's contact bounds structure (which
	     * may be NULL)
	     */
	    cb = obj_ptr->contact_bounds;

	    /* Can this object crash into other objects? */
	    if(((cb != NULL) ? cb->crash_flags : 0) &
		SAR_CRASH_FLAG_CRASH_OTHER
	    )
	    {
		/* Perform object to object crash contact check */
		if(SARSimCrashContactCheck(core_ptr, obj_ptr) > -1)
		{
		    /* Crash contact into another object has occured,
		     * so we need to check if this object is still
		     * allocated.
		     */
		    if(SARObjIsAllocated(*obj_pa, *obj_total, i))
			obj_ptr = (*obj_pa)[i];
		    else
			continue;
		}
	    }

	    /* Hoist deployment contact check
	     *
	     * If object has a hoist, its hoist deployment is out, and
	     * an object is picked up then a valid index is returned
	     * and the proper procedure will have been performed
	     */
	    if(SARSimHoistDeploymentContactCheck(core_ptr, obj_ptr) > -1)
	    {
		/* Rescue basket contact has occured, need to check if
		 * this object is still allocated
		 */
		if(SARObjIsAllocated(*obj_pa, *obj_total, i))
		    obj_ptr = (*obj_pa)[i];
		else
		    continue;
	    }

	    /* Check if the object is "too old", if it is then it
	     * will be deleted
	     */
	    if(SARSimDoMortality(core_ptr, obj_ptr))
	    {
		/* Object has been deleted, check if it is still
		 * allocated (which it probably won't be).
		 */
		if(SARObjIsAllocated(*obj_pa, *obj_total, i))
		    obj_ptr = (*obj_pa)[i];
		else
		    continue;
	    }

	    /* Add more object updates here */

	}

	return(0);
}
