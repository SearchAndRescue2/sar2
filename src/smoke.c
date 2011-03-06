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
#include "smoke.h"


int SmokeCreate(
	sar_scene_struct *scene,
	sar_object_struct ***ptr, int *total,
	sar_smoke_type smoke_type,
	const sar_position_struct *pos,
	const sar_position_struct *respawn_offset,
	float radius_start,		/* In meters */
	float radius_max,		/* In meters */
	float radius_rate,		/* In meters per second */
	int hide_at_max,
	int total_units,		/* Max smoke units in trail */
	time_t respawn_int,             /* In ms */
	const char *tex_name,
	int ref_object,			/* Can be -1 for none */
	time_t life_span
);

int SmokeCreateSparks(
	sar_scene_struct *scene,
	sar_object_struct ***ptr, int *total,
	const sar_position_struct *pos,
	const sar_position_struct *respawn_offset,
	float sparks_distance,		/* How far should sparks fly */
	int ref_object,
	time_t life_span
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
 *	Creates a new Smoke Trail.
 */
int SmokeCreate(
	sar_scene_struct *scene,
	sar_object_struct ***ptr, int *total,
	sar_smoke_type smoke_type,
	const sar_position_struct *pos,
	const sar_position_struct *respawn_offset,
	float radius_start,		/* In meters */
	float radius_max,		/* In meters */
	float radius_rate,		/* In meters per cycle */
	int hide_at_max,
	int total_units,                /* Max smoke units in trail */
	time_t respawn_int,             /* In ms */
	const char *tex_name,
	int ref_object,			/* Can be -1 for none */
	time_t life_span		/* In ms */
)
{
	int i, obj_num;
	sar_object_struct *obj_ptr;
	sar_object_smoke_struct *smoke;

	if((scene == NULL) || (pos == NULL))
	    return(-1);

	/* Create a new Smoke Trail */
	obj_num = SARObjNew(
	    scene, ptr, total,
	    SAR_OBJ_TYPE_SMOKE
	);
	if(obj_num < 0)
	    return(-1);

	obj_ptr = (*ptr)[obj_num];
	if(obj_ptr == NULL)
	    return(-1);

	smoke = SAR_OBJ_GET_SMOKE(obj_ptr);
	if(smoke == NULL)
	    return(-1);


	/* Need to automatically calculate radius_rate? */
	if(radius_rate < 0.0f)
	{
	    float dt = (float)(total_units * respawn_int / 1000.0f);

	    if(dt > 0.0f)
		radius_rate = (float)((radius_max - radius_start) / dt);
	    else
		radius_rate = 0.0f;
	}


	/* Begin setting values */

	/* Position */
	memcpy(
	    &obj_ptr->pos,
	    pos,
	    sizeof(sar_position_struct)
	);
	/* Set visible range based on maximum size */
	if(radius_max >= 400.0f)
	    obj_ptr->range = (float)SFMMilesToMeters(18.0);
	else if(radius_max >= 200.0f)
	    obj_ptr->range = (float)SFMMilesToMeters(16.0);
	else if(radius_max >= 100.0f)
	    obj_ptr->range = (float)SFMMilesToMeters(13.0);
	else
	    obj_ptr->range = (float)SFMMilesToMeters(10.0);

	SARObjAddContactBoundsSpherical(
	    obj_ptr,
	    0, 0,		/* Crash flags and crash type */
	    radius_start	/* Use starting radius size as contact bounds */
	);

	obj_ptr->life_span = life_span;

	obj_ptr->temperature = 0.25;

	/* Set smoke trail type to smoke */
	smoke->type = smoke_type;

	/* Set smoke trail specific values */
	if(respawn_offset != NULL)
	    memcpy(
		&smoke->respawn_offset,
		respawn_offset,
		sizeof(sar_position_struct)
	    );

	smoke->radius_start = radius_start;
	smoke->radius_max = radius_max;
	smoke->radius_rate = radius_rate;

	smoke->hide_at_max = hide_at_max;
	smoke->delete_when_no_units = 0;

	smoke->respawn_int = respawn_int;
	smoke->respawn_next = 0l;

	smoke->tex_num = SARGetTextureRefNumberByName(
	    scene, tex_name
	);

	smoke->ref_object = ref_object;

	smoke->total_units = total_units;
	if(smoke->total_units > 0)
	{
	    smoke->unit = (sar_object_smoke_unit_struct *)realloc(
		smoke->unit,
		smoke->total_units * sizeof(sar_object_smoke_unit_struct)
	    );
	    if(smoke->unit == NULL)
	    {
		smoke->total_units = total_units = 0;
	    }
	}
	for(i = 0; i < smoke->total_units; i++)
	{
	    sar_object_smoke_unit_struct *smoke_unit_ptr = &smoke->unit[i];

	    memset(&smoke_unit_ptr->pos, 0x00, sizeof(sar_position_struct));
	    smoke_unit_ptr->radius = 0.0f;
	    smoke_unit_ptr->visibility = 0.0f;
	}


	return(obj_num);
}


/*
 *	Creates a new Sparks.
 */
int SmokeCreateSparks(
	sar_scene_struct *scene,
	sar_object_struct ***ptr, int *total,
	const sar_position_struct *pos,
	const sar_position_struct *respawn_offset,
	float sparks_distance,		/* How far should sparks fly */
	int ref_object,
	time_t life_span
)
{
	const time_t respawn_int = 1000l;
	int obj_num;
	sar_object_struct *obj_ptr;
	sar_object_smoke_struct *smoke;

	if((scene == NULL) || (pos == NULL))
	    return(-1);

	/* Note about respawn interval: The respawning of sparks per
	 * respawn interval is a group of sparks (instead of just one
	 * unit like the smoke puffs)
	 */

	/* Create a new Sparks */
	obj_num = SmokeCreate(
	    scene, ptr, total,
	    SAR_SMOKE_TYPE_SPARKS,
	    pos, respawn_offset,
	    1.0f,		/* Radius start (ignored for sparks) */
	    sparks_distance,	/* Radius max (how far sparks should fly) */
	    1.0f,		/* Radius growth rate in meters per cycle
				 * (ignored for sparks) */
	    0,			/* Do not hide at max */
	    20,			/* Total units */
	    respawn_int,	/* Respawn interval in ms (*) */
	    NULL,		/* No texture */
	    ref_object,
	    life_span
	);
	if(obj_num < 0)
	    return(-1);

	obj_ptr = (*ptr)[obj_num];
	if(obj_ptr == NULL)
	    return(-1);

	smoke = SAR_OBJ_GET_SMOKE(obj_ptr); 
	if(smoke == NULL)
	    return(-1);



	return(obj_num);
}
