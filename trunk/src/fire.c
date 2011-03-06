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
#include "fire.h"
#include "sar.h"
#include "simop.h"
#include "config.h"


int FireCreate(
	sar_core_struct *core_ptr, sar_scene_struct *scene,
	sar_object_struct ***ptr, int *total,
	const sar_position_struct *pos,
	float radius, float height,
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
 *	Creates a new Fire.
 */
int FireCreate(
	sar_core_struct *core_ptr, sar_scene_struct *scene,
	sar_object_struct ***ptr, int *total,
	const sar_position_struct *pos,
	float radius, float height,
	int ref_object,				/* Not used */
	const char *tex_name, const char *ir_tex_name
)
{
	int obj_num;
	sar_object_struct *obj_ptr;
	sar_object_fire_struct *fire;
	const sar_option_struct *opt = &core_ptr->option;
	if((scene == NULL) || (pos == NULL))
	    return(-1);

	/* Create fire object */
	obj_num = SARObjNew(
	    scene, ptr, total,
	    SAR_OBJ_TYPE_FIRE
	);
	if(obj_num < 0)
	    return(-1);

	obj_ptr = (*ptr)[obj_num];
	if(obj_ptr == NULL)
	    return(-1);

	fire = SAR_OBJ_GET_FIRE(obj_ptr);
	if(fire == NULL)
	    return(-1);


	/* Begin setting new values */
	/* Position */
	memcpy( &obj_ptr->pos, pos, sizeof(sar_position_struct)	);

	/* Set visible range based on radius */
	if(radius >= 100.0f)
	    obj_ptr->range = (float)SFMMilesToMeters(16.0);
	else if(radius >= 70.0f)
	    obj_ptr->range = (float)SFMMilesToMeters(14.0);
	else if(radius >= 30.0f)
	    obj_ptr->range = (float)SFMMilesToMeters(12.0);
	else if(radius >= 10.0f)
	    obj_ptr->range = (float)SFMMilesToMeters(10.0);
	else
	    obj_ptr->range = (float)SFMMilesToMeters(7.0);

        /* We can crash if we touch fire */
        SARObjAddContactBoundsSpherical(
            obj_ptr,
            SAR_CRASH_FLAG_CRASH_CAUSE,
            SAR_CRASH_TYPE_FIRE,
            radius);
	/* Temperature */
	obj_ptr->temperature = 1.0f;

	/* Set size of fire */
	fire->radius = radius;
	fire->height = height;

	/* Set frame animation values */
	fire->cur_frame = 0;
/* Use explosion frame increment interval */
	fire->frame_inc_int = opt->explosion_frame_int;
	fire->next_frame_inc = cur_millitime +
	    fire->frame_inc_int;

	fire->cur_frame = 0;
	fire->frame_repeats = 0;
	fire->total_frame_repeats = 1;

	/* Get texture references on the scene*/
	fire->tex_num = SARGetTextureRefNumberByName(
	    scene, tex_name
	);
	fire->ir_tex_num = SARGetTextureRefNumberByName(
	    scene, ir_tex_name
	);

#if 0
	/* Set reference object */
/* Not used */
	fire->ref_object = ref_object;
#endif
	fire->ref_object = -1;

	return(obj_num);
}
