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

#include <stdlib.h>
#include <string.h>

#include "v3dtex.h"
#include "obj.h"
#include "objutils.h"
#include "explosion.h"
#include "sar.h"
#include "config.h"


int ExplosionCreate(
	sar_core_struct *core_ptr, sar_scene_struct *scene,
	sar_object_struct ***ptr, int *total,
	const sar_position_struct *pos,
	float radius,
	int ref_object,
	const char *tex_name, const char *ir_tex_name
);

int SplashCreate(
	sar_core_struct *core_ptr, sar_scene_struct *scene,
	sar_object_struct ***ptr, int *total,
	const sar_position_struct *pos,
	float radius,
	int ref_object,
	const char *tex_name, const char *ir_tex_name
);


#define ATOI(s)		(((s) != NULL) ? atoi(s) : 0)
#define ATOL(s)		(((s) != NULL) ? atol(s) : 0)
#define ATOF(s)		(((s) != NULL) ? atof(s) : 0.0f)
#define STRDUP(s)	(((s) != NULL) ? strdup(s) : NULL)

#define MAX(a,b)	(((a) > (b)) ? (a) : (b))
#define MIN(a,b)	(((a) < (b)) ? (a) : (b))
#define CLIP(a,l,h)	(MIN(MAX((a),(l)),(h)))
#define STRLEN(s)	(((s) != NULL) ? strlen(s) : 0)
#define STRISEMPTY(s)	(((s) != NULL) ? (*(s) == '\0') : 1)

#define RADTODEG(r)	((r) * 180.0 / PI)
#define DEGTORAD(d)	((d) * PI / 180.0)


/*
 *	Creates a new Explosion.
 */
int ExplosionCreate(
	sar_core_struct *core_ptr, sar_scene_struct *scene,
	sar_object_struct ***ptr, int *total,
	const sar_position_struct *pos,
	float radius,
	int ref_object,
	const char *tex_name, const char *ir_tex_name
)
{
	int obj_num;
	sar_object_struct *obj_ptr;
	sar_object_explosion_struct *explosion;
	const sar_option_struct *opt = &core_ptr->option;
	if((scene == NULL) || (pos == NULL))
	    return(-1);

	/* Create Explosion */
	obj_num = SARObjNew(
	    scene, ptr, total,
	    SAR_OBJ_TYPE_EXPLOSION
	);
	if(obj_num < 0)
	    return(-1);

	obj_ptr = (*ptr)[obj_num];
	if(obj_ptr == NULL)
	    return(-1);

	explosion = SAR_OBJ_GET_EXPLOSION(obj_ptr);
	if(explosion == NULL)
	    return(-1);


	/* Begin setting new values */

	/* Position */
	memcpy(
	    &obj_ptr->pos, pos, sizeof(sar_position_struct)
	);

	/* Set visible range based on radius */
	if(radius >= 20.0f)
	    obj_ptr->range = (float)SFMMilesToMeters(14.0);
	else if(radius >= 12.0f)
	    obj_ptr->range = (float)SFMMilesToMeters(10.0);
	else if(radius >= 4.0f)
	    obj_ptr->range = (float)SFMMilesToMeters(6.0);
	else
	    obj_ptr->range = (float)SFMMilesToMeters(3.0);

	/* Temperature */
	obj_ptr->temperature = 1.0f;

	/* Set size of explosion */
	explosion->radius = radius;

	/* Color emission */
	explosion->color_emission = SAR_EXPLOSION_COLOR_EMISSION_IS_LIGHT;

	/* Regular explosions always have center of gravity at
	 * object's center
	 */
	explosion->center_offset = SAR_EXPLOSION_CENTER_OFFSET_NONE;

	/* Set frame animation values */
	explosion->cur_frame = 0;
	explosion->frame_inc_int = opt->explosion_frame_int;
	explosion->next_frame_inc = cur_millitime +
	    explosion->frame_inc_int;

	explosion->cur_frame = 0;
	explosion->frame_repeats = 0;
	explosion->total_frame_repeats = 1;

	/* Get texture references on the scene */
	explosion->tex_num = SARGetTextureRefNumberByName(
	    scene, tex_name
	);
	explosion->ir_tex_num = SARGetTextureRefNumberByName(
	    scene, ir_tex_name
	);
 
	/* Set reference object */
	explosion->ref_object = ref_object;

	return(obj_num);
}

/*
 *	Creates a new Splash.
 */
int SplashCreate(
	sar_core_struct *core_ptr, sar_scene_struct *scene,
	sar_object_struct ***ptr, int *total,
	const sar_position_struct *pos,
	float radius,
	int ref_object,
	const char *tex_name, const char *ir_tex_name
)
{
	int obj_num;
	sar_object_struct *obj_ptr;
	sar_object_explosion_struct *explosion;
	const sar_option_struct *opt = &core_ptr->option;
	if((scene == NULL) || (pos == NULL))
	    return(-1);

	/* Create Splash */
	obj_num = SARObjNew(
	    scene, ptr, total,
	    SAR_OBJ_TYPE_EXPLOSION
	);
	if(obj_num < 0)
	    return(-1);

	obj_ptr = (*ptr)[obj_num];
	if(obj_ptr == NULL)
	    return(-1);

	explosion = SAR_OBJ_GET_EXPLOSION(obj_ptr);
	if(explosion == NULL)
	    return(-1);


	/* Begin setting new values */

	/* Position */
	memcpy(
	    &obj_ptr->pos, pos, sizeof(sar_position_struct)
	);

	/* Set visible range based on radius */
	if(radius >= 8.0f)
	    obj_ptr->range = (float)SFMMilesToMeters(10.0);
	else if(radius >= 4.0f)
	    obj_ptr->range = (float)SFMMilesToMeters(6.0);
	else
	    obj_ptr->range = (float)SFMMilesToMeters(3.0);

	/* Temperature */
	obj_ptr->temperature = 0.0f;

	/* Set size of explosion */
	explosion->radius = radius;

	/* Color emission */
	explosion->color_emission = SAR_EXPLOSION_COLOR_EMISSION_NONE;

	/* `Splash' explosions always have center of gravity at object's
	 * base, so that it's kept above water
	 */
	explosion->center_offset = SAR_EXPLOSION_CENTER_OFFSET_BASE;

	/* Set frame animation values */
	explosion->frame_inc_int = opt->splash_frame_int;
	explosion->next_frame_inc = cur_millitime +
	    explosion->frame_inc_int;

	explosion->cur_frame = 0;
	explosion->frame_repeats = 0;
	explosion->total_frame_repeats = 1;

	/* Get texture references on the scene */
	explosion->tex_num = SARGetTextureRefNumberByName(
	    scene, tex_name
	);
	explosion->ir_tex_num = SARGetTextureRefNumberByName(
	    scene, ir_tex_name
	);

	/* Set reference object */
	explosion->ref_object = ref_object;

	return(obj_num);
}
