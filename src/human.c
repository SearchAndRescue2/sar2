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

#include <string.h>
#include <stdlib.h>
#include <sys/types.h>

#include "../include/string.h"

#include "sfm.h"
#include "obj.h"
#include "objutils.h"
#include "sarreality.h"
#include "human.h"
#include "config.h"


sar_human_data_entry_struct *SARHumanMatchEntryByName(
	sar_human_data_struct *hd, const char *name
);

sar_human_data_struct *SARHumanPresetsInit(void *core_ptr);
void SARHumanPresetsShutdown(sar_human_data_struct *hd);

void SARHumanEntryDelete(
	sar_human_data_struct *hd, sar_human_data_entry_struct *entry
);

int SARHumanCreate(
	sar_human_data_struct *hd,
	sar_scene_struct *scene,
	sar_object_struct ***object, int *total_objects,
	sar_obj_flags_t human_flags,
	int assisting_humans,
	const char *assisting_human_preset_name[SAR_ASSISTING_HUMANS_MAX],
	const char *name
);
void SARHumanSetObjectPreset(
	sar_human_data_struct *hd,
	sar_object_struct *obj_ptr,
	const char *name
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
 *	Returns a pointer to the entry structure on the given hd that
 *	matches name.
 *
 *	Can return NULL on failed match or error.
 */
sar_human_data_entry_struct *SARHumanMatchEntryByName(
	sar_human_data_struct *hd, const char *name
)
{
	int i;
	sar_human_data_entry_struct *entry;

	if((hd == NULL) || STRISEMPTY(name))
	    return(NULL);

	for(i = 0; i < hd->total_presets; i++)
	{
	    entry = hd->preset[i];
	    if(entry == NULL)
		continue;

	    if(entry->name != NULL)
	    {
		if(!strcasecmp(entry->name, name))
		    return(entry);
	    }
	}

	return(NULL);
}

/*
 *	Initializes a new sar_human_data_struct structure and returns
 *	the pointer to it.
 *
 *	This pointer must be freed by the calling function by a call to
 *	SARHumanPresetsShutdown().
 */
sar_human_data_struct *SARHumanPresetsInit(void *core_ptr)
{
	sar_human_data_struct *hd = (sar_human_data_struct *)calloc(
	    1, sizeof(sar_human_data_struct)
	);
	if(hd == NULL)
	    return(hd);

	hd->core_ptr = core_ptr;

	return(hd);
}

/*
 *      Deallocates all human preset resources on the given
 *	sar_human_data_struct structure and the structure itself.
 *
 *	The given pointer must not be referenced again after calling
 *	this function.
 */
void SARHumanPresetsShutdown(sar_human_data_struct *hd)
{
	int i;

	if(hd == NULL)
	    return;

	/* Deallocate each preset human entry structure */
	for(i = 0; i < hd->total_presets; i++)
	    SARHumanEntryDelete(
		hd, hd->preset[i]
	    );

	free(hd->preset);
	hd->preset = NULL;
	hd->total_presets = 0;

	free(hd);
}

/*
 *	Deallocates the given human entry structure and all its
 *	resources.
 */
void SARHumanEntryDelete(
	sar_human_data_struct *hd, sar_human_data_entry_struct *entry
)
{
	if(entry == NULL)
	    return;

	free(entry->name);
	free(entry);
}


/*
 *	Creates a new object of type SAR_OBJ_TYPE_HUMAN in the given
 *	scene.
 *
 *	Returns the index to the newly added object or -1 on failure.
 *
 *	If name is not NULL, then the human object will be modeled after
 *	the one matching name on the given human presets data structure.
 */
int SARHumanCreate(
	sar_human_data_struct *hd,
	sar_scene_struct *scene,
	sar_object_struct ***object, int *total_objects,
	sar_obj_flags_t human_flags,
	int assisting_humans,
	const char *assisting_human_preset_name[SAR_ASSISTING_HUMANS_MAX],
	const char *name
)
{
	int obj_num;
	sar_object_struct *obj_ptr;
	sar_object_human_struct *human;


	if((scene == NULL) || (object == NULL) || (total_objects == NULL))
	    return(-1);

	/* Create new human object */
	obj_num = SARObjNew(
	    scene, object, total_objects,
	    SAR_OBJ_TYPE_HUMAN
	);
	obj_ptr = ((obj_num < 0) ? NULL : (*object)[obj_num]);
	if(obj_ptr == NULL)
	    return(-1);

	/* Get pointer to human data substructure */
	human = SAR_OBJ_GET_HUMAN(obj_ptr);
	if(human == NULL)
	    return(obj_num);


	/* Visual range */
	obj_ptr->range = SAR_HUMAN_DEF_RANGE;

	/* Set contact bounds, this is needed so we can pick up human
	 * and do other size detection with it.
	 */
	SARObjAddContactBoundsCylendrical(
	    obj_ptr,
	    0, 0,           /* Crash flags, crash type */
	    (float)SAR_HUMAN_CONTACT_RADIUS,
	    (float)SAR_HUMAN_CONTACT_ZMIN, (float)SAR_HUMAN_CONTACT_ZMAX
	);

	/* Temperature */
	obj_ptr->temperature = 1.0f;

	/* Set human flags */
	human->flags = human_flags;

	/* Set default animation rate */
	human->anim_rate = SAR_HUMAN_ANIM_RATE;

	/* Water ripples texture */
	human->water_ripple_tex_num = SARGetTextureRefNumberByName(
	    scene, SAR_STD_TEXNAME_WATER_RIPPLE
	);

	/* Reset other human object values here */
	human->intercepting_object = -1;

	human->intercepting_object_distance2d =
	    human->intercepting_object_distance3d = 0.0;

	/* Number of assisting humans */
	human->assisting_humans = assisting_humans;

	/* Assisting humans preset(s) */
	if (human->assisting_humans)
	{
	    for(int i = 0; i < human->assisting_humans; i++)
		human->assisting_human_preset_name[i] = assisting_human_preset_name[i];
	}

	/* Model human object values to preset values from the human
	 * data presets.
	 */
	SARHumanSetObjectPreset(
	    hd, obj_ptr, name
	);

	return(obj_num);
}

/*
 *	Sets the specified object (which must be of type
 *	SAR_OBJ_TYPE_HUMAN) to the human presets data entry given by
 *	name on the given hd.
 *
 *	If obj_ptr is invalid, not of type SAR_OBJ_TYPE_HUMAN, or the
 *	given name of a human data entry could not be found on the hd
 *	then no operation will be performed.
 */
void SARHumanSetObjectPreset(
	sar_human_data_struct *hd,
	sar_object_struct *obj_ptr,
	const char *name
)
{
	sar_object_human_struct *human;
	sar_human_data_entry_struct *entry;

	if((hd == NULL) || (obj_ptr == NULL) || (name == NULL))
	    return;

	/* Match human presets data entry by the given name */
	entry = SARHumanMatchEntryByName(hd, name);
	if(entry == NULL)
	    return;

	human = SAR_OBJ_GET_HUMAN(obj_ptr);
	if(human == NULL)
	    return;


	/* Begin setting values from the entry to the human object */

	/* Color palette */
	memcpy(
	    &human->color[0],
	    &entry->color[0],
	    SAR_HUMAN_COLORS_MAX * sizeof(sar_color_struct)
	);

	/* Height in meters */
	human->height = entry->height;

	/* Weight in kg */
	human->mass = entry->mass;

	/* Gender */
	if(entry->preset_entry_flags & SAR_HUMAN_FLAG_GENDER_FEMALE)
	    human->flags |= SAR_HUMAN_FLAG_GENDER_FEMALE;

	/* Number of assisting humans. Do not overwrite if assisting_humans
	 * number has been defined earlier by a create_human parameter.
	 */
	if(human->assisting_humans == 0)
	    human->assisting_humans = entry->assisting_humans;

	/* Color palette for assisting humans */
	memcpy(
	    &human->assisting_human_color[0],
	    &entry->assisting_human_color[0],
	    SAR_HUMAN_COLORS_MAX * sizeof(sar_color_struct)
	);



	/* Add support for other things that need to be coppied here */



}
