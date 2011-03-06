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
#include <ctype.h>
#include <stdlib.h>

#include "../include/string.h"

#include "sfm.h"
#include "v3dtex.h"

#include "sarreality.h"
#include "objutils.h"
#include "obj.h"
#include "objio.h"
#include "sar.h"


static void SARObjPremodeledPowerTransmissionTowerNew(
	sar_core_struct *core_ptr, sar_scene_struct *scene,
	int obj_num, sar_object_struct *obj_ptr,
	sar_object_premodeled_struct *premodeled,
	int argc, char **argv
);
static void SARObjPremodeledRadioTowerNew(
	sar_core_struct *core_ptr, sar_scene_struct *scene,
	int obj_num, sar_object_struct *obj_ptr,
	sar_object_premodeled_struct *premodeled,
	int argc, char **argv
);
static void SARObjPremodeledTowerNew(
	sar_core_struct *core_ptr, sar_scene_struct *scene,
	int obj_num, sar_object_struct *obj_ptr,
	sar_object_premodeled_struct *premodeled,
	int argc, char **argv
);
static void SARObjPremodeledControlTowerNew(
	sar_core_struct *core_ptr, sar_scene_struct *scene,
	int obj_num, sar_object_struct *obj_ptr,
	sar_object_premodeled_struct *premodeled,
	int argc, char **argv
);
static void SARObjPremodeledBuildingNew(
	sar_core_struct *core_ptr, sar_scene_struct *scene,
	int obj_num, sar_object_struct *obj_ptr,
	sar_object_premodeled_struct *premodeled,
	int argc, char **argv
);
int SARObjPremodeledNew(
	sar_core_struct *core_ptr, sar_scene_struct *scene,
	const char *type_name, int argc, char **argv
);


#define ATOI(s)         (((s) != NULL) ? atoi(s) : 0)
#define ATOL(s)         (((s) != NULL) ? atol(s) : 0)
#define ATOF(s)         (((s) != NULL) ? (float)atof(s) : 0.0f)
#define STRDUP(s)       (((s) != NULL) ? strdup(s) : NULL)

#define MAX(a,b)        (((a) > (b)) ? (a) : (b))
#define MIN(a,b)        (((a) < (b)) ? (a) : (b))
#define CLIP(a,l,h)     (MIN(MAX((a),(l)),(h)))


/*
 *	Creates a new Power Transmission Tower.
 */
static void SARObjPremodeledPowerTransmissionTowerNew(
	sar_core_struct *core_ptr, sar_scene_struct *scene,
	int obj_num, sar_object_struct *obj_ptr,
	sar_object_premodeled_struct *premodeled,
	int argc, char **argv
)
{
	int flashing;
	float z;
	int i, hazard_lights;
	sar_light_struct *light;


	/* Format:
	 *
	 * <range> <height> <hazard_lights>
	 */

	premodeled->type = SAR_OBJ_PREMODELED_POWER_TRANSMISSION_TOWER;

	/* Range*/
	i = 0;
	obj_ptr->range = (float)((i < argc) ? ATOF(argv[i]) : 1000.0);

	/* Height (in feet)*/
	i = 1;
	premodeled->height = (float)SFMFeetToMeters(
	    (i < argc) ? ATOF(argv[i]) : 80.0
	);

	/* Number of hazard lights? */
	i = 2;
	hazard_lights = (i < argc) ? ATOI(argv[i]) : 0;
	flashing = 1;
	z = premodeled->height;
	for(i = 0; i < hazard_lights; i++)
	{
	    light = SARObjLightNew(
		scene,
		&obj_ptr->light, &obj_ptr->total_lights
	    );
	    if(light != NULL)
	    {
		if(flashing)
		{
		    light->pos.x = 0.0f;
		    light->pos.y = 0.0f;
		    light->pos.z = z;

		    light->color.r = 1.0f;
		    light->color.g = 0.0f;
		    light->color.b = 0.0f;
		    light->color.a = 1.0f;

		    light->radius = 2;

		    light->flags |= SAR_LIGHT_FLAG_ON;
		    light->flags |= SAR_LIGHT_FLAG_STROBE;

		    light->int_on = 800;
		    light->int_off = 1700;
		}
		else
		{
		    light->pos.x = 0.0f;
		    light->pos.y = 0.0f;
		    light->pos.z = z;

		    light->color.r = 1.0f;
		    light->color.g = 0.0f;
		    light->color.b = 0.0f;
		    light->color.a = 1.0f;

		    light->radius = 2;

		    light->flags |= SAR_LIGHT_FLAG_ON;

		    light->int_on = 0;
		    light->int_off = 0;
		}
	    }

	    /* Toggle flashing, this is to create alternating steady
	     * state and flashing hazard lights.
	     */
	    flashing = !flashing;

	    /* Decrease height*/
	    z -= (premodeled->height / hazard_lights);
	}

	/* Set cylendrical contact bounds, the radius of the contact
	 * bounds is 0.3 times the height, since the height controls
	 * a uniformed scaling the the unit width is 0.3.
	 */
	SARObjAddContactBoundsCylendrical(
	    obj_ptr,
	    SAR_CRASH_FLAG_CRASH_CAUSE,
	    SAR_CRASH_TYPE_OBSTRUCTION,
	    (float)(0.3 * premodeled->height),	/* Radius*/
	    0.0f, premodeled->height		/* Height min and max*/
	);
}

/*
 *	Creates a new Radio Tower.
 */
static void SARObjPremodeledRadioTowerNew(
	sar_core_struct *core_ptr, sar_scene_struct *scene,
	int obj_num, sar_object_struct *obj_ptr,
	sar_object_premodeled_struct *premodeled,
	int argc, char **argv
)
{
	int flashing;
	float z;
	int i, hazard_lights;
	sar_light_struct *light;


	/* Format:
	 *
	 * <range> <height> <hazard_lights>
	 */

	premodeled->type = SAR_OBJ_PREMODELED_RADIO_TOWER;

	/* Range */
	i = 0;
	obj_ptr->range = (float)((i < argc) ? ATOF(argv[i]) : 1000.0);

	/* Height (in feet) */
	i = 1;
	premodeled->height = (float)SFMFeetToMeters(
	    (i < argc) ? ATOF(argv[i]) : 80.0
	);

	/* Number of hazard lights? */
	i = 2;
	hazard_lights = (i < argc) ? ATOI(argv[i]) : 0;
	flashing = 1;
	z = premodeled->height;
	for(i = 0; i < hazard_lights; i++)
	{
	    light = SARObjLightNew(
		scene,
		&obj_ptr->light, &obj_ptr->total_lights
	    );
	    if(light != NULL)
	    {
		if(flashing)
		{
		    light->pos.x = 0.0f;
		    light->pos.y = 0.0f;
		    light->pos.z = z;

		    light->color.r = 1.0f;
		    light->color.g = 0.0f;
		    light->color.b = 0.0f;
		    light->color.a = 1.0f;

		    light->radius = 2;

		    light->flags |= SAR_LIGHT_FLAG_ON;
		    light->flags |= SAR_LIGHT_FLAG_STROBE;

		    light->int_on = 800;
		    light->int_off = 1700;
		}
		else
		{
		    light->pos.x = 0.0f;
		    light->pos.y = 0.0f;
		    light->pos.z = z;

		    light->color.r = 1.0f;
		    light->color.g = 0.0f;
		    light->color.b = 0.0f;
		    light->color.a = 1.0f;

		    light->radius = 2;

		    light->flags |= SAR_LIGHT_FLAG_ON;

		    light->int_on = 0;
		    light->int_off = 0;
		}
	    }

	    /* Toggle flashing, this is to create alternating steady
	     * state and flashing hazard lights.
	     */
	    flashing = !flashing;

	    /* Decrease height*/
	    z -= (premodeled->height / hazard_lights);
	}

	/* Set cylendrical contact bounds, the radius of the contact
	 * bounds is 0.3 times the height, since the height controls
	 * a uniformed scaling the the unit width is 0.3.
	 */
	SARObjAddContactBoundsCylendrical(
	    obj_ptr,
	    SAR_CRASH_FLAG_CRASH_CAUSE,
	    SAR_CRASH_TYPE_OBSTRUCTION,
	    5.0,                                /* Fixed radius*/
	    0.0, premodeled->height       /* Height min and max*/
	);
}

/*
 *	Creates a new Tower.
 */
static void SARObjPremodeledTowerNew(
	sar_core_struct *core_ptr, sar_scene_struct *scene,
	int obj_num, sar_object_struct *obj_ptr,
	sar_object_premodeled_struct *premodeled,
	int argc, char **argv
)
{
	int flashing;
	float z;
	int i, hazard_lights;
	sar_light_struct *light;


	/* Format:
	 *
	 * <range> <height> <hazard_lights>
	 */

	premodeled->type = SAR_OBJ_PREMODELED_TOWER;

	/* Range*/
	i = 0;
	obj_ptr->range = (float)((i < argc) ? ATOF(argv[i]) : 1000.0);

	/* Height (in feet)*/
	i = 1;
	premodeled->height = (float)SFMFeetToMeters(
	    (i < argc) ? ATOF(argv[i]) : 80.0
	);

	/* Number of hazard lights? */
	i = 2;
	hazard_lights = (i < argc) ? ATOI(argv[i]) : 0;
	flashing = 1;
	z = premodeled->height;
	for(i = 0; i < hazard_lights; i++)
	{
	    light = SARObjLightNew(
		scene,
		&obj_ptr->light, &obj_ptr->total_lights
	    );
	    if(light != NULL)
	    {
		if(flashing)
		{
		    light->pos.x = 0.0f;
		    light->pos.y = 0.0f;
		    light->pos.z = z;

		    light->color.r = 1.0f;
		    light->color.g = 0.0f;
		    light->color.b = 0.0f;
		    light->color.a = 1.0f;

		    light->radius = 2;

		    light->flags |= SAR_LIGHT_FLAG_ON;
		    light->flags |= SAR_LIGHT_FLAG_STROBE;

		    light->int_on = 800;
		    light->int_off = 1700;
		}
		else
		{
		    light->pos.x = 0.0f;
		    light->pos.y = 0.0f;
		    light->pos.z = z;

		    light->color.r = 1.0f;
		    light->color.g = 0.0f;
		    light->color.b = 0.0f;
		    light->color.a = 1.0f;

		    light->radius = 2;

		    light->flags |= SAR_LIGHT_FLAG_ON;

		    light->int_on = 0;
		    light->int_off = 0;
		}
	    }

	    /* Toggle flashing, this is to create alternating steady
	     * state and flashing hazard lights.
	     */
	    flashing = !flashing;

	    /* Decrease height*/
	    z -= (premodeled->height / hazard_lights);
	}

	/* Set cylendrical contact bounds, the radius of the contact
	 * bounds is 0.3 times the height, since the height controls
	 * a uniformed scaling the the unit width is 0.3.
	 */
	SARObjAddContactBoundsCylendrical(
	    obj_ptr,
	    SAR_CRASH_FLAG_CRASH_CAUSE,
	    SAR_CRASH_TYPE_OBSTRUCTION,
	    5.0,				/* Fixed radius*/
	    0.0, premodeled->height       /* Height min and max*/
	);
}

/*
 *	Creates a new Control Tower.
 */
static void SARObjPremodeledControlTowerNew(
	sar_core_struct *core_ptr, sar_scene_struct *scene,
	int obj_num, sar_object_struct *obj_ptr,
	sar_object_premodeled_struct *premodeled,
	int argc, char **argv
)
{
	int i, n;

	/* Format:
	 *
	 * <range> <length> <width> <height> <walls_texture> <roof_texture>
	 */

	premodeled->type = SAR_OBJ_PREMODELED_CONTROL_TOWER;

	/* Range*/
	i = 0;
	obj_ptr->range = (float)((i < argc) ? ATOF(argv[i]) : 1000.0);

	/* Length*/
	i = 1;
	premodeled->length = (float)((i < argc) ? ATOF(argv[i]) : 15.0);

	/* Width*/
	i = 2;
	premodeled->width = (float)((i < argc) ? ATOF(argv[i]) : 15.0);

	/* Height (in feet)*/
	i = 3;
	premodeled->height = (float)SFMFeetToMeters(
	    (i < argc) ? ATOF(argv[i]) : 80.0
	);
  
	/* Walls texture*/
	i = 4;
	if(i < argc)
	{
	    n = 0;
	    if(n < SAR_OBJ_PREMODEL_MAX_TEXTURES)
		premodeled->tex_num[n] = SARGetTextureRefNumberByName(
		    scene, argv[i]
		);
	}

	/* Roof texture*/
	i = 5;
	if(i < argc)
	{
	    n = 1;
	    if(n < SAR_OBJ_PREMODEL_MAX_TEXTURES)
		premodeled->tex_num[n] = SARGetTextureRefNumberByName(
		    scene, argv[i]
		);
	}

	/* Set contact bounds*/
	SARObjAddContactBoundsRectangular(
	    obj_ptr,
	    SAR_CRASH_FLAG_CRASH_CAUSE | SAR_CRASH_FLAG_SUPPORT_SURFACE,
	    SAR_CRASH_TYPE_BUILDING,
	    -(premodeled->length / 2), (premodeled->length / 2),
	    -(premodeled->width / 2), (premodeled->width / 2),
	    0.0, premodeled->height
	);
}


/*
 *	Creates a new Building.
 */
static void SARObjPremodeledBuildingNew(
	sar_core_struct *core_ptr, sar_scene_struct *scene,
	int obj_num, sar_object_struct *obj_ptr,
	sar_object_premodeled_struct *premodeled,
	int argc, char **argv
)
{
	int i, n;

	/* Format:
	 *
	 * <range> <length> <width> <height> <walls_texture>
	 * <walls_texture_night> <roof_texture>
	 */

	premodeled->type = SAR_OBJ_PREMODELED_BUILDING;

	/* Range*/
	i = 0;
	obj_ptr->range = (float)((i < argc) ? ATOF(argv[i]) : 1000.0);

	/* Length*/
	i = 1;
	premodeled->length = (float)((i < argc) ? ATOF(argv[i]) : 50.0);

	/* Width*/
	i = 2;
	premodeled->width = (float)((i < argc) ? ATOF(argv[i]) : 20.0);

	/* Height (in feet)*/
	i = 3;
	premodeled->height = (float)SFMFeetToMeters(
	    (i < argc) ? ATOF(argv[i]) : 50.0
	);

	/* Walls texture*/
	i = 4; n = 0;
	if(i < argc)
	{
	    if(n < SAR_OBJ_PREMODEL_MAX_TEXTURES)
	        premodeled->tex_num[n] = SARGetTextureRefNumberByName(
		    scene, argv[i]
	        );
	}

	/* Walls night*/
	i = 5; n = 1;
	if(i < argc)
	{
	    if(n < SAR_OBJ_PREMODEL_MAX_TEXTURES)
		premodeled->tex_num[n] = SARGetTextureRefNumberByName(
		    scene, argv[i]
		);
	}

	/* Roof texture*/
	i = 6; n = 2;
	if(i < argc)
	{
	    if(n < SAR_OBJ_PREMODEL_MAX_TEXTURES)
		premodeled->tex_num[n] = SARGetTextureRefNumberByName(
		    scene, argv[i]
		);
	}

	/* Set contact bounds*/
	SARObjAddContactBoundsRectangular(
	    obj_ptr,
	    SAR_CRASH_FLAG_CRASH_CAUSE | SAR_CRASH_FLAG_SUPPORT_SURFACE,
	    SAR_CRASH_TYPE_BUILDING,
	    -(premodeled->length / 2), (premodeled->length / 2),
	    -(premodeled->width / 2), (premodeled->width / 2),
	    0.0, premodeled->height
	);
}

/*
 *	Creates a new Permodeled Object.
 */
int SARObjPremodeledNew(
	sar_core_struct *core_ptr, sar_scene_struct *scene,
	const char *type_name, int argc, char **argv
)
{
	int obj_num;
	sar_object_struct *obj_ptr;
	sar_object_premodeled_struct *premodeled;


	if((scene == NULL) || (type_name == NULL))
	    return(-1);

	/* Create new premodeled object */
	obj_num = SARObjNew(
	    scene,
	    &core_ptr->object, &core_ptr->total_objects,
	    SAR_OBJ_TYPE_PREMODELED
	);
	obj_ptr = (obj_num > -1) ? core_ptr->object[obj_num] : NULL;
	if(obj_ptr == NULL)
	    return(-1);

	/* Get pointer to premodeled object data substructure*/
	premodeled = SAR_OBJ_GET_PREMODELED(obj_ptr);


	/* Handle by preset model type name */

	/* Standard building */
	if(!strcasecmp(type_name, "building"))
	{
	    SARObjPremodeledBuildingNew(
		core_ptr, scene,
		obj_num, obj_ptr, premodeled,
		argc, argv
	    );
	}
	/* Control tower */
	else if(!strcasecmp(type_name, "control_tower"))
	{
	    SARObjPremodeledControlTowerNew(
		core_ptr, scene,
		obj_num, obj_ptr, premodeled,
		argc, argv
	    );
	}
	/* Tower */
	else if(!strcasecmp(type_name, "tower"))
	{
	    SARObjPremodeledTowerNew(
		core_ptr, scene,
		obj_num, obj_ptr, premodeled,
		argc, argv
	    );
	}
	/* Radio tower */
	else if(!strcasecmp(type_name, "radio_tower"))
	{
	    SARObjPremodeledRadioTowerNew(
		core_ptr, scene,
		obj_num, obj_ptr, premodeled,
		argc, argv
	    );
	}
	/* Power transmission tower */
	else if(!strcasecmp(type_name, "power_transmission_tower"))
	{
	    SARObjPremodeledPowerTransmissionTowerNew(
		core_ptr, scene,
		obj_num, obj_ptr, premodeled,
		argc, argv
	    );
	}

	else
	{
	    fprintf(
		stderr,
"SARObjPremodeledNew(): Unsupported preset model type name `%s'.\n",
		type_name
	    );
	}

	return(obj_num);
}
