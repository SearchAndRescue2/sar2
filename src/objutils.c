#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#ifdef __MSW__
# include <windows.h>
#endif

#include <GL/gl.h>

#include "../include/string.h"

#include "matrixmath.h"
#include "sfm.h"

#include "sound.h"
#include "sarreality.h"
#include "obj.h"
#include "objsound.h"
#include "objutils.h"
#include "sar.h"
#include "config.h"


#ifdef __MSW__
static double rint(double x);
#endif	/* __MSW__ */

int SARObjIsAllocated(
	sar_object_struct **ptr, int total,
	int n
);
sar_object_struct *SARObjGetPtr(
	sar_object_struct **ptr, int total,
	int n
);
int SARGetObjectNumberFromPointer(
	sar_scene_struct *scene,
	sar_object_struct **ptr, int total,
	sar_object_struct *obj_ptr
);
int SARIsTextureAllocated(sar_scene_struct *scene, int n);
sar_object_struct *SARObjMatchPointerByName(
	sar_scene_struct *scene,
	sar_object_struct **ptr, int total,
	const char *name, int *obj_num
);

v3d_texture_ref_struct *SARGetTextureRefByName(
	sar_scene_struct *scene, const char *name
);
int SARGetTextureRefNumberByName(
	sar_scene_struct *scene, const char *name
);

int SARObjLandingGearState(sar_object_struct *obj_ptr);

sar_obj_part_struct *SARObjGetPartPtr(
	sar_object_struct *obj_ptr, sar_obj_part_type type, int skip
);
sar_obj_rotor_struct *SARObjGetRotorPtr(
	sar_object_struct *obj_ptr, int n, int *total
);
sar_obj_hoist_struct *SARObjGetHoistPtr(
	sar_object_struct *obj_ptr, int n, int *total
);
int SARObjGetOnBoardPtr(
	sar_object_struct *obj_ptr,
	int **crew, int **passengers, int **passengers_max,
	float **passengers_mass,
	int **passengers_leave_pending, int **passengers_drop_pending
);
sar_external_fueltank_struct *SARObjGetFuelTankPtr(
	sar_object_struct *obj_ptr, int n, int *total
);

sar_visual_model_struct *SARVisualModelNew(
	sar_scene_struct *scene,
	const char *filename, const char *name
);
void *SARVisualModelNewList(sar_visual_model_struct *vmodel);
int SARVisualModelGetRefCount(sar_visual_model_struct *vmodel);  
void SARVisualModelRef(sar_visual_model_struct *vmodel);
void SARVisualModelUnref(
	sar_scene_struct *scene, sar_visual_model_struct *vmodel
);
void SARVisualModelCallList(sar_visual_model_struct *vmodel);
static void SARVisualModelDelete(sar_visual_model_struct *vmodel);
void SARVisualModelDeleteAll(sar_scene_struct *scene);

sar_cloud_layer_struct *SARCloudLayerNew(
	sar_scene_struct *scene,
	int tile_width, int tile_height,
	float range, float altitude,
	const char *tex_name
);
void SARCloudLayerDelete(
	sar_scene_struct *scene,
	sar_cloud_layer_struct *cloud_layer_ptr
);

sar_cloud_bb_struct *SARCloudBBNew(
	sar_scene_struct *scene,
	int tile_width, int tile_height,
	const sar_position_struct *pos,
	float width, float height,	/* In meters */
	const char *tex_name,
	time_t lightening_min_int,      /* In ms */
	time_t lightening_max_int       /* In ms */
);
void SARCloudBBDelete(sar_cloud_bb_struct *cloud_bb_ptr);

int SARObjAddToGroundList(
	sar_scene_struct *scene, sar_object_struct *obj_ptr
);
void SARObjRemoveFromGroundList(
	sar_scene_struct *scene, sar_object_struct *obj_ptr
);

int SARObjAddToHumanRescueList(
	sar_scene_struct *scene, sar_object_struct *obj_ptr
);
void SARObjRemoveFromHumanRescueList(
	sar_scene_struct *scene, sar_object_struct *obj_ptr
);

int SARObjAddContactBoundsSpherical(
	sar_object_struct *obj_ptr,
	sar_obj_flags_t crash_flags, int crash_type,
	float contact_radius
);
int SARObjAddContactBoundsCylendrical(
	sar_object_struct *obj_ptr,
	sar_obj_flags_t crash_flags, int crash_type,
	float contact_radius,
	float contact_h_min, float contact_h_max
);
int SARObjAddContactBoundsRectangular(
	sar_object_struct *obj_ptr,
	sar_obj_flags_t crash_flags, int crash_type,
	float contact_x_min, float contact_x_max,
	float contact_y_min, float contact_y_max,
	float contact_z_min, float contact_z_max
);

int SARObjInterceptNew(
	sar_scene_struct *scene,
	sar_intercept_struct ***ptr, int *total,
	sar_obj_flags_t flags,
	float x, float y, float z,
	float radius,
	float urgency,
	const char *name
);
static sar_obj_part_struct *SARObjPartNewNexus(
	sar_scene_struct *scene,
	sar_obj_part_struct ***ptr, int *total,
	sar_obj_part_type type
);
sar_obj_part_struct *SARObjPartNew(
	sar_scene_struct *scene,
	sar_obj_part_struct ***ptr, int *total,
	sar_obj_part_type type
);
sar_obj_part_struct *SARObjAirBrakeNew(
	sar_scene_struct *scene,
	sar_obj_part_struct ***ptr, int *total
);
sar_obj_part_struct *SARObjDoorRescueNew(
	sar_scene_struct *scene,
	sar_obj_part_struct ***ptr, int *total
);
sar_obj_part_struct *SARObjLandingGearNew(
	sar_scene_struct *scene,
	sar_obj_part_struct ***ptr, int *total
);
int SARObjExternalFuelTankNew(
	sar_scene_struct *scene,
	sar_external_fueltank_struct ***ptr, int *total
);
int SARObjRotorNew(
	sar_scene_struct *scene,
	sar_obj_rotor_struct ***ptr, int *total
);
sar_light_struct *SARObjLightNew(
	sar_scene_struct *scene,
	sar_light_struct ***ptr, int *total
);
int SARObjNew(
	sar_scene_struct *scene,
	sar_object_struct ***ptr, int *total,
	int type
);

void SARObjDeleteIntercepts(
	sar_scene_struct *scene,
	sar_intercept_struct ***ptr, int *total
);
void SARObjDeleteLights(
	sar_scene_struct *scene,
	sar_light_struct ***ptr, int *total
);
void SARObjDeleteParts(
	sar_scene_struct *scene,
	sar_obj_part_struct ***ptr, int *total
);
void SARObjDeleteExternalFuelTanks(
	sar_scene_struct *scene,
	sar_external_fueltank_struct ***ptr, int *total
);
void SARObjDeleteRotors(
	sar_scene_struct *scene,
	sar_obj_rotor_struct ***ptr, int *total
);
void SARObjDelete(
	void *core_ptr,
	sar_object_struct ***ptr, int *total, int n
);

void SARObjGenerateTilePlane(
	float min, float max,
	float tile_width, float tile_height
);


#define ATOI(s)         (((s) != NULL) ? atoi(s) : 0)
#define ATOL(s)         (((s) != NULL) ? atol(s) : 0)
#define ATOF(s)         (((s) != NULL) ? atof(s) : 0.0f)
#define STRDUP(s)       (((s) != NULL) ? strdup(s) : NULL)

#define MAX(a,b)        (((a) > (b)) ? (a) : (b))
#define MIN(a,b)        (((a) < (b)) ? (a) : (b))
#define CLIP(a,l,h)     (MIN(MAX((a),(l)),(h)))

#define ISSTREMPTY(s)	(((s) != NULL) ? (*(s) == '\0') : True)
#define STRLEN(s)	(((s) != NULL) ? ((int)strlen(s)) : 0)


#ifdef __MSW__
static double rint(double x)
{
	if((double)((double)x - (int)x) > (double)0.5)
	    return((double)((int)x + (int)1));
	else
	    return((double)((int)x));
}
#endif	/* __MSW__ */


/*
 *	Returns true if object n is allocated in the given array.
 */
int SARObjIsAllocated(
	sar_object_struct **ptr, int total,
	int n
)
{
	if((ptr == NULL) || (n < 0) || (n >= total))
	    return(0);
	else if(ptr[n] == NULL)
	    return(0);
	else
	    return(1);
}

/*
 *	Returns pointer to object n if it is valid and not NULL.
 *
 *	Returns NULL on error.
 */
sar_object_struct *SARObjGetPtr(
	sar_object_struct **ptr, int total,
	int n
)
{
	if((ptr == NULL) || (n < 0) || (n >= total))
	    return(NULL);
	else
	    return(ptr[n]);
}


/*
 *	Returns the number of the object pointer in the given pointer
 *	array or -1 if no match.
 */
int SARGetObjectNumberFromPointer(
	sar_scene_struct *scene,
	sar_object_struct **ptr, int total,
	sar_object_struct *obj_ptr
)
{
	int i;

	if(obj_ptr == NULL)
	    return(-1);

	for(i = 0; i < total; i++)
	{
	    if(ptr[i] == obj_ptr)
		return(i);
	}

	return(-1);
}

/*
 *	Checks if the texture is allocated on the scene.
 */
int SARIsTextureAllocated(sar_scene_struct *scene, int n)
{
	if((scene == NULL) || (n < 0) || (n >= scene->total_texture_refs))
	    return(0);
	else if(scene->texture_ref[n] == NULL)
	    return(0);
	else
	    return(1);
}

/*
 *	Returns the pointer to the object matching the given name or
 *	NULL if no match was made.
 */
sar_object_struct *SARObjMatchPointerByName(
	sar_scene_struct *scene,
	sar_object_struct **ptr, int total,
	const char *name, int *obj_num
) 
{
	int i;
	sar_object_struct *obj_ptr;


	if(obj_num != NULL)
	    *obj_num = -1;

	/* Go through each object */
	for(i = 0; i < total; i++)
	{
	    obj_ptr = ptr[i];
	    if((obj_ptr != NULL) ? (obj_ptr->name == NULL) : 1)
		continue;

	    /* Names match? */
	    if(!strcasecmp(obj_ptr->name, name))
	    {
		if(obj_num != NULL)
		    *obj_num = i;

		return(obj_ptr);
	    }
	}

	return(NULL);
}

/*
 *	Returns a pointer to the texture reference structure matching
 *	the given name. Can return NULL for failed match.
 */
v3d_texture_ref_struct *SARGetTextureRefByName(
	sar_scene_struct *scene, const char *name
)
{
	int i, total;
	v3d_texture_ref_struct *t, **ptr;


	if((scene == NULL) || (name == NULL))
	    return(NULL);

	ptr = scene->texture_ref;
	total = scene->total_texture_refs;

	for(i = 0; i < total; i++)
	{
	    t = ptr[i];
	    if((t != NULL) ? (t->name == NULL) : 1)
		continue;

	    if((t->name == name) ? 1 : !strcasecmp(t->name, name))
		return(t);
	}

	return(NULL);
}

/*
 *	Same as SARGetTextureRefByName() except that it returns the
 *	texture index number of -1 on error.
 */
int SARGetTextureRefNumberByName(
	sar_scene_struct *scene, const char *name
)
{
	int i, total;
	v3d_texture_ref_struct *t, **ptr;

	if((scene == NULL) || ISSTREMPTY(name))
	    return(-1);

	ptr = scene->texture_ref;
	total = scene->total_texture_refs;

	for(i = 0; i < total; i++)
	{
	    t = ptr[i];
	    if((t != NULL) ? (t->name == NULL) : 1)
		continue;

	    if((t->name == name) ? 1 : !strcasecmp(t->name, name))
		return(i);
	}

	return(-1);
}


/*
 *	Returns -1 if the object does not have landing gears, 0 if the
 *	landing gears exist but are up, or 1 if the landing gears are
 *	down.
 */
int SARObjLandingGearState(sar_object_struct *obj_ptr)
{
	sar_object_aircraft_struct *aircraft;

	if(obj_ptr == NULL)
	    return(-1);

	aircraft = SAR_OBJ_GET_AIRCRAFT(obj_ptr);
	return((aircraft != NULL) ?
	    aircraft->landing_gear_state : -1
	);
}

/*
 *	Returns the pointer to the part structure on the object that
 *	matches the given part type. The skip indicates the number
 *	of part structures of the specified type to be skipped.
 */
sar_obj_part_struct *SARObjGetPartPtr(
	sar_object_struct *obj_ptr, sar_obj_part_type type, int skip
)
{
	int i, j, m = 0;
	sar_object_aircraft_struct *aircraft;
	sar_obj_part_struct *part, **list = NULL;


	if(obj_ptr == NULL)
	    return(NULL);


	/* Begin checking object type and get the object parts list */

	/* Aircraft */
	aircraft = SAR_OBJ_GET_AIRCRAFT(obj_ptr);
	if(aircraft != NULL)
	{
	    list = aircraft->part;
	    m = aircraft->total_parts;
	}


	/* Got list of object parts? */
	if(list != NULL)
	{
	    for(i = 0, j = 0; i < m; i++)
	    {
		part = list[i];
		if(part == NULL)
		    continue;
		if(part->type != type)
		    continue;

		/* Increment j, the skip count. If the skip count is still
		 * at or below the requested number of times to skip n,
		 * then skip this part.
		 */
		j++;
		if(j <= skip)
		    continue;

		return(part);
	    }
	}

	return(NULL);
}

/*
 *	Returns pointer to the n th index rotor and optional total
 *	on the given object. Can return NULL if none exist.
 *
 *	If object only has one, then pass n as 0.
 */
sar_obj_rotor_struct *SARObjGetRotorPtr(
	sar_object_struct *obj_ptr, int n, int *total
)
{
	sar_object_aircraft_struct *aircraft;


	if(total != NULL)
	    *total = 0;

	if(obj_ptr == NULL)
	    return(NULL);

	aircraft = SAR_OBJ_GET_AIRCRAFT(obj_ptr);
	if(aircraft != NULL)
	{
	    if(total != NULL)
		*total = aircraft->total_rotors;
	    if((aircraft->rotor != NULL) &&
	       (n >= 0) && (n < aircraft->total_rotors)
	    )
		return(aircraft->rotor[n]);
	}

	return(NULL);
}

/*
 *      Returns pointer to the n th index door and optional total
 *      on the given object. Can return NULL if none exist.
 *      
 *      If object only has one, then pass n as 0.
 */
sar_obj_hoist_struct *SARObjGetHoistPtr(
	sar_object_struct *obj_ptr, int n, int *total
)
{
	sar_object_aircraft_struct *aircraft;


	if(total != NULL)
	    *total = 0;

	if(obj_ptr == NULL)
	    return(NULL);

	aircraft = SAR_OBJ_GET_AIRCRAFT(obj_ptr);
	if(aircraft != NULL)
	{
	    if(total != NULL)
		*total = 1;
	    if(n == 0)
		return(aircraft->hoist);
	}

	return(NULL);
}

/*
 *	Returns pointers to the object's value for crew and passengers
 *	(and related information).
 *
 *	Returns -1 on error, -2 if the object's type does not have 
 *	passengers, and 0 on success.
 */
int SARObjGetOnBoardPtr(
	sar_object_struct *obj_ptr,                   
	int **crew, int **passengers, int **passengers_max,
	float **passengers_mass,
	int **passengers_leave_pending, int **passengers_drop_pending
)
{
	sar_object_aircraft_struct *aircraft;


	if(crew != NULL)
	    *crew = NULL;
	if(passengers != NULL)
	    *passengers = NULL;
	if(passengers_max != NULL)
	    *passengers_max = NULL;
	if(passengers_mass != NULL)
	    *passengers_mass = NULL;
	if(passengers_leave_pending != NULL)
	    *passengers_leave_pending = NULL;
	if(passengers_drop_pending != NULL)
	    *passengers_drop_pending = NULL;

	if(obj_ptr == NULL)
	    return(-1);

	switch(obj_ptr->type)
	{
	  case SAR_OBJ_TYPE_GARBAGE:
	  case SAR_OBJ_TYPE_STATIC:
	  case SAR_OBJ_TYPE_AUTOMOBILE:
	  case SAR_OBJ_TYPE_WATERCRAFT:
	    return(-2);
	    break;
	  case SAR_OBJ_TYPE_AIRCRAFT:
	    aircraft = SAR_OBJ_GET_AIRCRAFT(obj_ptr);
	    if(aircraft != NULL)
	    {
		if(crew != NULL)
		    *crew = &aircraft->crew;
		if(passengers != NULL)  
		    *passengers = &aircraft->passengers;
		if(passengers_max != NULL)  
		    *passengers_max = &aircraft->passengers_max;
		if(passengers_mass != NULL)
		    *passengers_mass = &aircraft->passengers_mass;
		if(passengers_leave_pending != NULL)
		    *passengers_leave_pending = &aircraft->passengers_leave_pending;
		if(passengers_drop_pending != NULL)
		    *passengers_drop_pending = &aircraft->passengers_drop_pending;
		return(0);
	    }
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
	    return(-2);
  	    break;
	}

	return(-1);
}

/*
 *      Returns pointer to the n th external reserved fuel tank and 
 *	optional total on the given object. Can return NULL if none exist.
 *
 *      If object only has one, then pass n as 0.
 */
sar_external_fueltank_struct *SARObjGetFuelTankPtr(
	sar_object_struct *obj_ptr, int n, int *total
)
{
	sar_object_aircraft_struct *aircraft;

	if(total != NULL)
	    *total = 0;

	if(obj_ptr == NULL)
	    return(NULL);

	switch(obj_ptr->type)
	{
	  case SAR_OBJ_TYPE_GARBAGE:
	  case SAR_OBJ_TYPE_STATIC:
	  case SAR_OBJ_TYPE_AUTOMOBILE:
	  case SAR_OBJ_TYPE_WATERCRAFT:
	    break;
	  case SAR_OBJ_TYPE_AIRCRAFT:
	    aircraft = SAR_OBJ_GET_AIRCRAFT(obj_ptr);
	    if(aircraft != NULL)
	    {
		if(total != NULL)
		    *total = aircraft->total_external_fueltanks;

		if((aircraft->external_fueltank != NULL) &&
		   (n >= 0) && (n < aircraft->total_external_fueltanks)
		)
		    return(aircraft->external_fueltank[n]);
	    }
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
	return(NULL);
}


/*
 *	Creates a new or returns an existing Visual Model on the
 *	specified Scene.
 *
 *	If the specified Visual Model filename and name both match an
 *	existing Visual Model on the Scene then that Visual Model is
 *	returned.
 *
 *	If filename or name is NULL then a new visual model will be
 *	created and will be treated as `unique' meaning that it will not
 *	be returned for match with any other call to SARVisualModelNew()
 *	(since the filename and name are unmatchable).
 *
 *	A new visual model will be given one ref count, while an existing
 *	visual model will have its ref count incremented by one. There
 *	will never be a case where a returned visual model will have only
 *	0 or negative ref counts. Ref counts on returned visual models
 *	will always be 1 or greater.
 *
 *	Can return NULL on error.
 */
sar_visual_model_struct *SARVisualModelNew(
	sar_scene_struct *scene,
	const char *filename, const char *name
)
{
	int i;
	sar_visual_model_struct *vmodel;

	if(scene == NULL)
	    return(NULL);

	/* Both filename and name given? */
	if((filename != NULL) && (name != NULL))
	{
	    /* Check for an existing visual model with the same
	     * filename and name
	     */
	    for(i = 0; i < scene->total_visual_models; i++)
	    {
		vmodel = scene->visual_model[i];
		if(vmodel == NULL)
		    continue;

		/* Both filename and name match (case sensitive)? */
		if((vmodel->filename != NULL) && (vmodel->name != NULL))
		{
		    if(!strcmp(vmodel->filename, filename) &&
		       !strcmp(vmodel->name, name)
		    )
		    {
			/* Increment the ref count for matched visual
			 * model and return its pointer
			 */
			vmodel->ref_count++;
			return(vmodel);
		    }
		}
	    }
	}


	/* Create new visual model */

	/* Sanitize total */
	if(scene->total_visual_models < 0) 
	    scene->total_visual_models = 0;

	/* Look for available pointer */
	for(i = 0; i < scene->total_visual_models; i++)
	{
	    if(scene->visual_model[i] == NULL)
		break;
	}
	if(i < scene->total_visual_models)
	{
	    /* Got available pointer at index i */
	}
	else
	{
	    /* Need to allocate a new pointer */
	    i = scene->total_visual_models;
	    scene->total_visual_models = i + 1;
	    scene->visual_model = (sar_visual_model_struct **)realloc(
		scene->visual_model,
		scene->total_visual_models * sizeof(sar_visual_model_struct *)
	    );
	    if(scene->visual_model == NULL)
	    {
		scene->total_visual_models = 0;
		return(NULL);
	    }
	    else
	    {
		scene->visual_model[i] = NULL;
	    }
	}

	/* Allocate new structure */
	scene->visual_model[i] = vmodel = SAR_VISUAL_MODEL(calloc(
	    1, sizeof(sar_visual_model_struct)
	));
	if(vmodel != NULL)
	{
	    vmodel->load_state = SAR_VISUAL_MODEL_NOT_LOADED;

	    /* Set initial ref count to 1 for all new visual models
	     * this is how calling functions can tell that this is a
	     * new visual model
	     */
	    vmodel->ref_count = 1;

	    vmodel->filename = STRDUP(filename);
	    vmodel->name = STRDUP(name);

	    vmodel->data = NULL;

	    vmodel->mem_size = 0;
	    vmodel->statements = 0;
	    vmodel->primitives = 0;
	}

	return(vmodel);
}

/*
 *	(Re)generates a GL display list on the given visual model.
 *
 *	The return value is really a GLuint (not a void *), can return
 *	NULL on failure.
 */
void *SARVisualModelNewList(sar_visual_model_struct *vmodel)
{
	GLuint list;

	if(vmodel == NULL)
	    return(NULL);

	/* Get existing GL display list if any */
	list = (GLuint)vmodel->data;
	if(list > 0)
	{
	    /* Already has an allocated GL display list so delete it
	     * and create a new one
	     */
	    glDeleteLists(list, 1);
	    list = 0;
	    vmodel->data = NULL;
	}

	/* Create new GL display list */
	list = glGenLists(1);

	/* Set new GL display list as the data pointer on the visual
	 * model structure
	 */
	vmodel->data = (void *)list;

	return((void *)list);
}

/*
 *	Returns number of ref counts on visual model.
 *
 *	Can return -1 if visual model is NULL.
 */
int SARVisualModelGetRefCount(sar_visual_model_struct *vmodel)
{
	if(vmodel != NULL)
	    return(vmodel->ref_count);
	else
	    return(-1);
} 

/*
 *	Increases the ref count on the given visual model by one.
 */
void SARVisualModelRef(sar_visual_model_struct *vmodel)
{
	if(vmodel != NULL)
	    vmodel->ref_count++;
}

/*
 *	Decrease the ref count on the given visual model by one.
 *
 *	If (after decrement) the ref count drops to 0 then the given
 *	visual model will be destroyed and its pointer should not
 *	be referenced again.
 *
 *	It's a general sense to not reference any pointer value again from
 *	any one variable after it's passed to this function. The scene
 *	structure's visual_model list is ultimatly to hold the pointer
 *	the visual model all the way to when it is down to its last
 *	ref count of 0.
 *
 *	If scene is not NULL then any visual model pointers on the
 *	scene's visual models list matching the given vmodel pointer
 *	will be set to NULL.
 */
void SARVisualModelUnref(
	sar_scene_struct *scene, sar_visual_model_struct *vmodel
)
{
	if(vmodel == NULL)
	    return;

	/* Reference counts have already reached 0? */
	if(vmodel->ref_count < 1)
	    fprintf(
		stderr,
"SARVisualModelUnref(): Warning: \
Visual model \"%s\" has less than 1 reference count.\n",
		vmodel->name
	    );

	/* Reduce reference count */
	vmodel->ref_count--;
	if(vmodel->ref_count < 0)
	    vmodel->ref_count = 0;

	/* No reference counts left? */
	if(vmodel->ref_count == 0)
	{
	    /* Reference counts have reached 0 on this visual model,
	     * so once that happens the visual model needs
	     * to be deleted and removed from the Scene
	     */

	    /* Delete this visual model, deleting its GL
	     * display list and the visual model structure itself
	     */
	    SARVisualModelDelete(vmodel);

	    /* Pointer vmodel is now invalid, but do not set it to
	     * NULL just yet as it needs to be used to check for 
	     * references to it on the scene structure
	     */
	    if(scene != NULL)
	    {
		int i;

		for(i = 0; i < scene->total_visual_models; i++)
		{
		    if(scene->visual_model[i] == vmodel)
			scene->visual_model[i] = NULL;
		}
	    }
	}
}

/*
 *      Calls the GL list on the visual model if it is not 0.  
 */
void SARVisualModelCallList(sar_visual_model_struct *vmodel)
{
	GLuint list = (vmodel != NULL) ? (GLuint)vmodel->data : 0;
	if(list > 0)
	    glCallList(list);
}

/*
 *	Deletes the Visual Model.
 *
 *	This function is intended to be called from SARVisualModelUnref().
 */
static void SARVisualModelDelete(sar_visual_model_struct *vmodel)
{
	GLuint list;

	if(vmodel == NULL)
	    return;

	if(vmodel->ref_count > 0)
	    fprintf(
		stderr,
"SARVisualModelDelete(): Warning: \
Visual model `%s' still has %i reference counts.\n",
		vmodel->name, vmodel->ref_count
	    );

	free(vmodel->filename);
	free(vmodel->name);

	list = (GLuint)vmodel->data;
	if(list > 0)
	    glDeleteLists(list, 1);

	free(vmodel);
}

/*
 *	Deletes all the Visual Models on the specified Scene.
 */
void SARVisualModelDeleteAll(sar_scene_struct *scene)
{
	int i;

	if(scene == NULL)
	    return;

	for(i = 0; i < scene->total_visual_models; i++)
	{
	    /* Delete the visual model, note that its ref counts
	     * should be 0 at this point
	     */
	    SARVisualModelDelete(scene->visual_model[i]);
	}
	free(scene->visual_model);
	scene->visual_model = NULL;
	scene->total_visual_models = 0;
}


/*
 *	Creates a new Cloud Layer.
 */
sar_cloud_layer_struct *SARCloudLayerNew(
	sar_scene_struct *scene,
	int tile_width, int tile_height,
	float range,			/* Tiling range in meters */
	float altitude,		/* Altitude in meters */
	const char *tex_name
)
{
	GLuint list;
	float min, max;
	v3d_texture_ref_struct *t;
	sar_visual_model_struct **vmodel;
	sar_cloud_layer_struct *cloud_layer_ptr = (sar_cloud_layer_struct *)
	    calloc(1, sizeof(sar_cloud_layer_struct));


	if(cloud_layer_ptr == NULL)
	    return(NULL);

	cloud_layer_ptr->tile_width = tile_width;
	cloud_layer_ptr->tile_height = tile_height;
	cloud_layer_ptr->range = MAX(range, MAX(tile_width, tile_height));
	cloud_layer_ptr->z = altitude;

	/* Match texture by name */
	t = SARGetTextureRefByName(scene, tex_name);

	/* Calculate entire size limits */
	min = -cloud_layer_ptr->range;
	max = cloud_layer_ptr->range;

	/* Get pointer to visual model */
	vmodel = &cloud_layer_ptr->visual_model;

	/* Unref visual model if its pointer to reference is not NULL */
	if((*vmodel) != NULL)
	{
	    SARVisualModelUnref(scene, *vmodel);
	    (*vmodel) = NULL;
	}

	/* Create new visual model */
	(*vmodel) = SARVisualModelNew(
	    scene, NULL, NULL
	);

	/* (Re)generate GL list on visual model structure */
	list = (GLuint)SARVisualModelNewList(*vmodel);
	if(list != 0)
	{
	    /* Mark as loading */
	    (*vmodel)->load_state = SAR_VISUAL_MODEL_LOADING;

	    /* Begin recording GL list */
	    glNewList(list, GL_COMPILE);
	    {
		/* Select texture if defined */
		V3DTextureSelect(t);

		/* Do not set color */

		/* Close range textured tiles */
		SARObjGenerateTilePlane(
		    min, max,			/* Min and max */
		    (float)tile_width, (float)tile_height	/* Tiling size */
		);
	    }
	    glEndList();

	    /* Mark as done loading */
	    (*vmodel)->load_state = SAR_VISUAL_MODEL_LOADED;
	}

	return(cloud_layer_ptr);
}

/*
 *	Deletes the Cloud Layer.
 *
 *	If scene is NULL then the visual model will not be unref'ed.
 */
void SARCloudLayerDelete(
	sar_scene_struct *scene,
	sar_cloud_layer_struct *cloud_layer_ptr
)
{
	if(cloud_layer_ptr == NULL)
	    return;

	SARVisualModelUnref(scene, cloud_layer_ptr->visual_model);
	free(cloud_layer_ptr->tex_name);
	free(cloud_layer_ptr);
}

/*
 *	Creates a new Cloud BillBoard.
 */
sar_cloud_bb_struct *SARCloudBBNew(
	sar_scene_struct *scene,
	int tile_width, int tile_height,
	const sar_position_struct *pos,
	float width, float height,	/* In meters */
	const char *tex_name,
	time_t lightening_min_int,      /* In ms */
	time_t lightening_max_int       /* In ms */
)
{
	sar_cloud_bb_struct *cloud = SAR_CLOUD_BB(
	    calloc(1, sizeof(sar_cloud_bb_struct))
	);
	if(cloud == NULL)
	    return(NULL);

	cloud->tile_width = tile_width;
	cloud->tile_height = tile_height;

	cloud->tex_name = STRDUP(tex_name);

	/* Match texture number on scene */
	cloud->tex_num = SARGetTextureRefNumberByName(
	    scene, tex_name
	);

	if(pos != NULL)
	    memcpy(&cloud->pos, pos, sizeof(sar_position_struct));

	cloud->width = width;
	cloud->height = height;

	cloud->lightening_min_int = lightening_min_int;
	cloud->lightening_max_int = lightening_max_int;
	cloud->lightening_next_on = 0;
	cloud->lightening_next_off = 0;
	cloud->lightening_started_on = 0;

	memset(
	    &cloud->lightening_point[0], 0x00,
	    SAR_LIGHTENING_POINTS_MAX * sizeof(sar_position_struct)
	);

	return(cloud);
}

/*      
 *	Deletes the Cloud BillBoard.
 */
void SARCloudBBDelete(sar_cloud_bb_struct *cloud_bb_ptr)
{
	if(cloud_bb_ptr == NULL)
	    return;

	free(cloud_bb_ptr->tex_name);
	free(cloud_bb_ptr);
}


/*
 *	Checks if the object is in the scene's ground list.
 *	If not, then it will add it to the list. Returns 0 on success
 *	or negative on error/already in list.
 */
int SARObjAddToGroundList(
	sar_scene_struct *scene,  
	sar_object_struct *obj_ptr
)
{
	int i;


	if((scene == NULL) ||
	   (obj_ptr == NULL)
	)
	    return(-1);

	if(scene->total_ground_objects < 0)
	    scene->total_ground_objects = 0;

	/* Check if already in list */
	for(i = 0; i < scene->total_ground_objects; i++)
	{
	    if(obj_ptr == scene->ground_object[i])
		return(-2);
	}

	/* Not in list, so add to list */
	for(i = 0; i < scene->total_ground_objects; i++)
	{
	    if(scene->ground_object[i] == NULL)
	    {
		/* Found an empty entry */
		scene->ground_object[i] = obj_ptr;
		return(0);
	    }
	}

	/* Need to allocate more entries */
	i = scene->total_ground_objects;
	scene->total_ground_objects++;

	scene->ground_object = (sar_object_struct **)realloc(
	    scene->ground_object,
	    scene->total_ground_objects * sizeof(sar_object_struct *)
	);
	if(scene->ground_object == NULL)
	{
	    scene->total_ground_objects = 0;
	    return(-1);
	}

	scene->ground_object[i] = obj_ptr;

	return(0);
}

/*
 *	Removes the object from the ground list if it is in there.
 */
void SARObjRemoveFromGroundList(
	sar_scene_struct *scene,
	sar_object_struct *obj_ptr
)
{
	int i;


	if((scene == NULL) || (obj_ptr == NULL))
	    return;

	for(i = 0; i < scene->total_ground_objects; i++)
	{
	    if(obj_ptr == scene->ground_object[i])
		scene->ground_object[i] = NULL;
	}
}

/*
 *	Adds object pointer to the humans need rescue list on the scene:
 */
int SARObjAddToHumanRescueList(
	sar_scene_struct *scene,
	sar_object_struct *obj_ptr
)
{
	int i;
	    
	    
	if((scene == NULL) ||
	   (obj_ptr == NULL)
	)
	    return(-1);
	    
	if(scene->total_human_need_rescue_objects < 0)
	    scene->total_human_need_rescue_objects = 0;

	/* Check if already in list */
	for(i = 0; i < scene->total_human_need_rescue_objects; i++)
	{
	    if(obj_ptr == scene->human_need_rescue_object[i])
		return(-2);
	}

	/* Not in list, so add to list */
	for(i = 0; i < scene->total_human_need_rescue_objects; i++)
	{
	    if(scene->human_need_rescue_object[i] == NULL)
	    {
		/* Found an empty entry */
		scene->human_need_rescue_object[i] = obj_ptr;
		return(0);
	    }
	}

	/* Need to allocate more entries */
	i = scene->total_human_need_rescue_objects;
	scene->total_human_need_rescue_objects++;

	scene->human_need_rescue_object = (sar_object_struct **)realloc(
	    scene->human_need_rescue_object,
	    scene->total_human_need_rescue_objects * sizeof(sar_object_struct *)
	);
	if(scene->human_need_rescue_object == NULL)
	{
	    scene->total_human_need_rescue_objects = 0;
	    return(-1);
	}

	scene->human_need_rescue_object[i] = obj_ptr;

	return(0);
}

/*
 *	Removes object from humans that need rescue list if it is in
 *	there.
 */
void SARObjRemoveFromHumanRescueList(
	sar_scene_struct *scene,
	sar_object_struct *obj_ptr
)
{
	int i;

	if((scene == NULL) || (obj_ptr == NULL))
	    return;

	for(i = 0; i < scene->total_human_need_rescue_objects; i++)
	{
	    if(obj_ptr == scene->human_need_rescue_object[i])
		scene->human_need_rescue_object[i] = NULL;
	}
}


/*
 *	Allocate (as needed) contact bounds structure to object
 *	for shape SAR_CONTACT_SHAPE_SPHERICAL.
 *
 *      Returns non-zero on error.
 */
int SARObjAddContactBoundsSpherical(
	sar_object_struct *obj_ptr,
	sar_obj_flags_t crash_flags, int crash_type,
	float contact_radius
)
{
	sar_contact_bounds_struct *cb;


	if(obj_ptr == NULL)
	    return(-1);

	/* Allocate as needed */
	if(obj_ptr->contact_bounds == NULL)
	    obj_ptr->contact_bounds = SAR_CONTACT_BOUNDS(
		calloc(1, sizeof(sar_contact_bounds_struct))
	    );

	cb = obj_ptr->contact_bounds;
	if(cb == NULL)
	    return(-1);

	cb->crash_flags = crash_flags;
	cb->crash_type = crash_type;
	cb->contact_shape = SAR_CONTACT_SHAPE_SPHERICAL;

	cb->contact_radius = (float)MAX(contact_radius, 0.0);

	return(0);
}

/*
 *      Allocate (as needed) contact bounds structure to object
 *      for shape SAR_CONTACT_SHAPE_CYLENDRICAL.
 *
 *      Returns non-zero on error.
 */
int SARObjAddContactBoundsCylendrical(
	sar_object_struct *obj_ptr,
	sar_obj_flags_t crash_flags, int crash_type,
	float contact_radius,
	float contact_h_min, float contact_h_max 
)
{
	sar_contact_bounds_struct *cb;


	if(obj_ptr == NULL)
	    return(-1);

	/* Allocate as needed */
	if(obj_ptr->contact_bounds == NULL)
	    obj_ptr->contact_bounds = SAR_CONTACT_BOUNDS(
		calloc(1, sizeof(sar_contact_bounds_struct))
	    );

	cb = obj_ptr->contact_bounds;
	if(cb == NULL)
	    return(-1);

	cb->crash_flags = crash_flags;
	cb->crash_type = crash_type;
	cb->contact_shape = SAR_CONTACT_SHAPE_CYLENDRICAL;

	cb->contact_radius = (float)MAX(contact_radius, 0.0);
	cb->contact_h_min = contact_h_min;
	cb->contact_h_max = contact_h_max;

	/* Flip height as needed */
	if(cb->contact_h_min > cb->contact_h_max)
	{
	    float h = cb->contact_h_min;

	    cb->contact_h_min = cb->contact_h_max;
	    cb->contact_h_max = h;
	}

	return(0);
}

/*
 *      Allocate (as needed) contact bounds structure to object
 *      for shape SAR_CONTACT_SHAPE_RECTANGULAR.
 *
 *	Returns non-zero on error.
 */
int SARObjAddContactBoundsRectangular(
	sar_object_struct *obj_ptr,
	sar_obj_flags_t crash_flags, int crash_type,
	float contact_x_min, float contact_x_max,
	float contact_y_min, float contact_y_max,
	float contact_z_min, float contact_z_max 
)
{
	sar_contact_bounds_struct *cb;


	if(obj_ptr == NULL)
	    return(-1);

	/* Allocate as needed */
	if(obj_ptr->contact_bounds == NULL)
	    obj_ptr->contact_bounds = SAR_CONTACT_BOUNDS(
		calloc(1, sizeof(sar_contact_bounds_struct))
	    );

	cb = obj_ptr->contact_bounds;
	if(cb == NULL)
	    return(-1);

	cb->crash_flags = crash_flags;
	cb->crash_type = crash_type;
	cb->contact_shape = SAR_CONTACT_SHAPE_RECTANGULAR;

	cb->contact_x_min = contact_x_min;
	cb->contact_x_max = contact_x_max;
	if(cb->contact_x_min > cb->contact_x_max)
	{
	    float d = cb->contact_x_min;

	    cb->contact_x_min = cb->contact_x_max;
	    cb->contact_x_max = d;
	}

	cb->contact_y_min = contact_y_min;
	cb->contact_y_max = contact_y_max;
	if(cb->contact_y_min > cb->contact_y_max)
	{
	    float d = cb->contact_y_min;

	    cb->contact_y_min = cb->contact_y_max;
	    cb->contact_y_max = d;
	}

	cb->contact_z_min = contact_z_min;
	cb->contact_z_max = contact_z_max;
	if(cb->contact_z_min > cb->contact_z_max)
	{
	    float d = cb->contact_z_min;

	    cb->contact_z_min = cb->contact_z_max;
	    cb->contact_z_max = d;
	}

	/* Calculate trig values based on object's current heading */
	cb->cos_heading = (float)cos(-obj_ptr->dir.heading);
	cb->sin_heading = (float)sin(-obj_ptr->dir.heading);

	return(0);
}


/*
 *	Adds an intercept to the object. Returns the allocated intercept
 *	index number or -1 on error.
 */
int SARObjInterceptNew(
	sar_scene_struct *scene,
	sar_intercept_struct ***ptr, int *total,
	sar_obj_flags_t flags,
	float x, float y, float z,
	float radius,
	float urgency,
	const char *name
)
{
	int n;
	sar_intercept_struct *intercept_ptr;


	if((ptr == NULL) || (total == NULL))
	    return(-1);

	/* Append intercept */
	n = MAX(*total, 0);
	*total = n + 1;
	*ptr = (sar_intercept_struct **)realloc(
	    *ptr,
	    (*total) * sizeof(sar_intercept_struct *)
	);
	if(*ptr == NULL)
	{
	    *total = 0;
	    return(-1);
	}

	/* Allocate structure */
	(*ptr)[n] = intercept_ptr = (sar_intercept_struct *)calloc(
	    1, sizeof(sar_intercept_struct)
	);
	if(intercept_ptr == NULL)
	    return(-1);

	/* Set values */
	intercept_ptr->flags = flags;
	intercept_ptr->x = x;
	intercept_ptr->y = y;
	intercept_ptr->z = z;
	intercept_ptr->radius = (float)MAX(radius, 0.0);
	intercept_ptr->urgency = (float)CLIP(urgency, 0.0, 1.0);
	intercept_ptr->name = STRDUP(name);

	return(n);
}


/*
 *	Nexus for creating a new Part in the Object Parts List.
 */
static sar_obj_part_struct *SARObjPartNewNexus(
	sar_scene_struct *scene,
	sar_obj_part_struct ***ptr, int *total,
	sar_obj_part_type type
)
{
	int i, n;
	sar_obj_part_struct *part;

	if((ptr == NULL) || (total == NULL))
	    return(NULL);

	for(i = 0; i < *total; i++)
	{
	    if((*ptr)[i] == NULL)
		break;
	}
	if(i < *total)
	{
	    n = i;
	}
	else
	{
	    n = MAX(*total, 0);
	    *total = n + 1;

	    *ptr = (sar_obj_part_struct **)realloc(
		*ptr,
		(*total) * sizeof(sar_obj_part_struct *)
	    );
	    if(*ptr == NULL)
	    {
		*total = 0;
		return(NULL);
	    }
	}

	(*ptr)[n] = part = (sar_obj_part_struct *)calloc(
	    1, sizeof(sar_obj_part_struct)
	);
	if(part == NULL)
	    return(NULL);

	part->type = type;
	part->flags = 0;
	part->temperature = SAR_DEF_TEMPERATURE;

	return(part);
}

/*
 *	Creates a new Part on the Object Parts List.
 */
sar_obj_part_struct *SARObjPartNew(
	sar_scene_struct *scene,
	sar_obj_part_struct ***ptr, int *total,
	sar_obj_part_type type
)
{
	return(SARObjPartNewNexus(
	    scene, ptr, total, type
	));
}

/*
 *	Creates a new Air Brake on the Object Parts List.
 */
sar_obj_part_struct *SARObjAirBrakeNew(
	sar_scene_struct *scene,
	sar_obj_part_struct ***ptr, int *total
)
{
	return(SARObjPartNewNexus(
	    scene, ptr, total, SAR_OBJ_PART_TYPE_AIR_BRAKE
	));
}

/*
 *	Creates a new Rescue Door on the Object Parts List.
 */
sar_obj_part_struct *SARObjDoorRescueNew(
	sar_scene_struct *scene,
	sar_obj_part_struct ***ptr, int *total
)
{
	return(SARObjPartNewNexus(
	    scene, ptr, total, SAR_OBJ_PART_TYPE_DOOR_RESCUE
	));
}

/*
 *	Creates a new Landing Gear on the Object Parts List.
 */
sar_obj_part_struct *SARObjLandingGearNew(
	sar_scene_struct *scene,
	sar_obj_part_struct ***ptr, int *total
)
{
	return(SARObjPartNewNexus(
	    scene, ptr, total, SAR_OBJ_PART_TYPE_LANDING_GEAR
	));
}

/*
 *	Creates a new External Fuel Tank Object.
 */
int SARObjExternalFuelTankNew(
	sar_scene_struct *scene,
	sar_external_fueltank_struct ***ptr, int *total
)
{        
	int i, n;
	sar_external_fueltank_struct *eft;

	if((ptr == NULL) || (total == NULL))
	    return(-1);

	for(i = 0; i < (*total); i++)
	{   
	    if((*ptr)[i] == NULL)
		break;
	}
	if(i < *total)
	{
	    n = i;
	}
	else
	{
	    n = MAX(*total, 0);
	    *total = (*total) + 1;

	    *ptr = (sar_external_fueltank_struct **)realloc(
		*ptr,
		(*total) * sizeof(sar_external_fueltank_struct *)
	    );
	    if(*ptr == NULL)
	    {
		*total = 0;
		return(-1);
	    }
	}

	(*ptr)[n] = (sar_external_fueltank_struct *)calloc(
	    1, sizeof(sar_external_fueltank_struct)
	);
	eft = (*ptr)[n];
	if(eft == NULL)
	    return(-1);

	/* Reset values */
	eft->flags = 0;

	return(n);
}

/*
 *	Creates a new Rotor on the specified Object Rotors List.
 */
int SARObjRotorNew(
	sar_scene_struct *scene,
	sar_obj_rotor_struct ***ptr, int *total
) 
{
	int i, n;
	sar_obj_rotor_struct *rotor;

	if((ptr == NULL) || (total == NULL))
	    return(-1);

	for(i = 0; i < *total; i++)
	{
	    if((*ptr)[i] == NULL)
		break;
	}
	if(i < *total)
	{
	    n = i;
	}
	else
	{
	    n = MAX(*total, 0);
	    *total = n + 1;

	    *ptr = (sar_obj_rotor_struct **)realloc(
		*ptr,
		(*total) * sizeof(sar_obj_rotor_struct *)
	    );
	    if(*ptr == NULL) 
	    {
		*total = 0;
		return(-1);
	    }
	}

	(*ptr)[n] = rotor = (sar_obj_rotor_struct *)calloc( 
	    1, sizeof(sar_obj_rotor_struct)
	);
	if(rotor == NULL)
	    return(-1);

	rotor->flags = 0;
	rotor->dir.heading = (float)(0.0 * PI);
	rotor->dir.pitch = (float)(0.5 * PI);
	rotor->dir.bank = (float)(0.0 * PI);
	rotor->blade_blur_tex_num = -1;
	rotor->wash_tex_num = -1;

	return(n);
}

/*
 *	Creates a new Light on the specified Object Light List.
 */
sar_light_struct *SARObjLightNew(
	sar_scene_struct *scene,
	sar_light_struct ***ptr, int *total
)
{
	int i, n;
	sar_light_struct *light;

	if((ptr == NULL) || (total == NULL))
	    return(NULL);

	for(i = 0; i < *total; i++)
	{
	    if((*ptr)[i] == NULL)
		break;
	}
	if(i < *total)
	{
	    n = i;
	}
	else
	{
	    n = MAX(*total, 0);
	    *total = n + 1;
	    *ptr = (sar_light_struct **)realloc(
		*ptr,
		(*total) * sizeof(sar_light_struct *)
	    );
	    if(*ptr == NULL)
	    {
		*total = 0;
		return(NULL);
	    }
	}

	(*ptr)[n] = light = (sar_light_struct *)calloc(
	    1, sizeof(sar_light_struct)
	);
	if(light == NULL)
	    return(NULL);

	light->flags = 0;

	return(light);
}


/*
 *	Creates a new Object on the specified Scene.
 */
int SARObjNew(
	sar_scene_struct *scene,
	sar_object_struct ***ptr, int *total,
	int type
)
{
	int i, n, len;
	sar_object_struct *obj_ptr;

	if((scene == NULL) || (ptr == NULL) || (total == NULL) ||
	   (type <= SAR_OBJ_TYPE_GARBAGE)
	)
	    return(-1);

	for(i = 0; i < *total; i++)
	{
	    if((*ptr)[i] == NULL)
		break;

	    if((*ptr)[i]->type <= SAR_OBJ_TYPE_GARBAGE)
		break;
	}
	if(i < *total)
	{
	    n = i;
	}
	else
	{
	    n = MAX(*total, 0);
	    (*total) = n + 1;

	    *ptr = (sar_object_struct **)realloc(
		*ptr,
		(*total) * sizeof(sar_object_struct *)
	    );
	    if(*ptr == NULL)
	    {
		*total = 0;
		return(-1);
	    }

	    (*ptr)[n] = NULL;
	}

	/* Allocate object structure as needed */
	if((*ptr)[n] == NULL)
	{
	    (*ptr)[n] = (sar_object_struct *)calloc(
		1, sizeof(sar_object_struct)
	    );
	    if((*ptr)[n] == NULL)
		return(-1);
	}

	obj_ptr = (*ptr)[n];


	/* Set object type */
	obj_ptr->type = type;

	obj_ptr->temperature = SAR_DEF_TEMPERATURE;

	/* Mark birth time */
	obj_ptr->birth_time_ms = cur_millitime;
	obj_ptr->birth_time_sec = cur_systime;

	/* Allocate substructure by the Object's type */
	switch(type)
	{
	  case SAR_OBJ_TYPE_GARBAGE:
	    len = 0;
	    break;
	  case SAR_OBJ_TYPE_STATIC:
	    len = 0;
	    break;
	  case SAR_OBJ_TYPE_AUTOMOBILE:
	    len = 0;
	    break;
	  case SAR_OBJ_TYPE_WATERCRAFT:
	    len = 0;
	    break;
	  case SAR_OBJ_TYPE_AIRCRAFT:
	    len = sizeof(sar_object_aircraft_struct);
	    break;
	  case SAR_OBJ_TYPE_GROUND:
	    len = sizeof(sar_object_ground_struct);
	    break;
	  case SAR_OBJ_TYPE_RUNWAY:
	    len = sizeof(sar_object_runway_struct);
	    break;
	  case SAR_OBJ_TYPE_HELIPAD:
	    len = sizeof(sar_object_helipad_struct);
	    break;
	  case SAR_OBJ_TYPE_HUMAN:
	    len = sizeof(sar_object_human_struct);
	    break;
	  case SAR_OBJ_TYPE_SMOKE:
	    len = sizeof(sar_object_smoke_struct);
	    break;
	  case SAR_OBJ_TYPE_FIRE:
	    len = sizeof(sar_object_fire_struct);;
	    break;
	  case SAR_OBJ_TYPE_EXPLOSION:
	    len = sizeof(sar_object_explosion_struct);
	    break;
	  case SAR_OBJ_TYPE_FUELTANK:
	    len = sizeof(sar_object_fueltank_struct);
	    break;
	  case SAR_OBJ_TYPE_PREMODELED:
	    len = sizeof(sar_object_premodeled_struct);
	    break;
	  default:
	    len = 0;
	    break;
	}

	/* If size of substructure is positive then allocate it */
	if(len > 0)
	    obj_ptr->data = calloc(1, len);
	else
	    obj_ptr->data = NULL;


	/* If object type is SAR_OBJ_TYPE_GROUND, then add it to the
	 * scene's list of ground objects
	 */
	if((scene != NULL) &&
	   (obj_ptr->type == SAR_OBJ_TYPE_GROUND)
	)
	    SARObjAddToGroundList(scene, obj_ptr);


	return(n);
}


/*
 *	Deletes all intercept structures in the given array.
 */
void SARObjDeleteIntercepts(
	sar_scene_struct *scene,
	sar_intercept_struct ***ptr, int *total
)
{
	int i;
	sar_intercept_struct *intercept_ptr;


	if((ptr == NULL) || (total == NULL))
	    return;

	for(i = 0; i < (*total); i++)
	{
	    intercept_ptr = (*ptr)[i];
	    if(intercept_ptr == NULL)
		continue;

	    free(intercept_ptr->name);
	    free(intercept_ptr);
	}

	free(*ptr);
	(*ptr) = NULL;
	(*total) = 0;
}

/*
 *	Deletes all structures and resources in the array of light
 *	structures.
 */
void SARObjDeleteLights(
	sar_scene_struct *scene,
	sar_light_struct ***ptr, int *total
)
{
	int i;
	sar_light_struct *light;


	if((ptr == NULL) || (total == NULL))
	    return;

	for(i = 0; i < *total; i++)
	{
	    light = (*ptr)[i];
	    if(light == NULL)
		continue;

	    free(light);
	}

	free(*ptr);
	*ptr = NULL;
	*total = 0;
}


/*
 *	Delete the list of object parts.
 */
void SARObjDeleteParts(
	sar_scene_struct *scene,
	sar_obj_part_struct ***ptr, int *total
)
{
	int i;
	sar_obj_part_struct *part;

	if((ptr == NULL) || (total == NULL))
	    return;

	for(i = 0; i < *total; i++)
	{
	    part = (*ptr)[i];
	    if(part == NULL)
		continue;

	    SARVisualModelUnref(scene, part->visual_model);
	    SARVisualModelUnref(scene, part->visual_model_ir);
	    free(part);
	}

	free(*ptr);
	(*ptr) = NULL;
	(*total) = 0;
}


/*
 *	Deletes the list of external fuel tanks.
 */
void SARObjDeleteExternalFuelTanks(
	sar_scene_struct *scene,
	sar_external_fueltank_struct ***ptr, int *total
)
{
	int i;
	sar_external_fueltank_struct *eft;

	for(i = 0; i < (*total); i++)
	{
	    eft = (*ptr)[i];
	    if(eft == NULL)
		continue;  

	    SARVisualModelUnref(scene, eft->visual_model);
	    SARVisualModelUnref(scene, eft->visual_model_ir);
	    free(eft);
	}

	free(*ptr);
	(*ptr) = NULL;

	(*total) = 0;
}

/*                  
 *      Deletes all structures and resources in the array of landing
 *      gear structures.
 */                     
void SARObjDeleteRotors(
	sar_scene_struct *scene,
	sar_obj_rotor_struct ***ptr, int *total
)                   
{ 
	int i;
	sar_obj_rotor_struct *rotor;

	if((ptr == NULL) || (total == NULL))
	    return;

	for(i = 0; i < *total; i++)
	{
	    rotor = (*ptr)[i];
	    if(rotor == NULL)
		continue;

	    SARVisualModelUnref(scene, rotor->visual_model);
	    SARVisualModelUnref(scene, rotor->visual_model_ir);
	    free(rotor);
	}

	free(*ptr);
	*ptr = NULL;
	*total = 0;     
}                     

/*
 *	Deletes object n from the array, setting it to NULL.
 */
void SARObjDelete(
	void *core_ptr,
	sar_object_struct ***ptr, int *total, int n
)
{
	sar_core_struct *cp = core_ptr;
	sar_scene_struct *scene;
	snd_recorder_struct *recorder;

	if(cp == NULL)
	    return;

	scene = cp->scene;
	if(scene == NULL)
	{
	    fprintf(
		stderr,
		"SARObjDelete(): Error: Scene is NULL.\n"
	    );
	    return;
	}

	recorder = cp->recorder;

	if(SARObjIsAllocated(*ptr, *total, n))
	{
	    int i;
	    sar_object_struct *obj_ptr, *obj_ptr2;
	    sar_object_aircraft_struct *aircraft;
	    sar_object_runway_struct *runway;
	    sar_object_helipad_struct *helipad;
	    sar_object_ground_struct *ground;
	    sar_object_smoke_struct *smoke;
	    sar_object_explosion_struct *explosion;
	    sar_object_fire_struct *fire;
	    sar_object_chemical_spray_struct *chemical_spray;
	    sar_object_fueltank_struct *fueltank;
	    sar_object_human_struct *human;
	    sar_object_premodeled_struct *premodeled;


	    obj_ptr = (*ptr)[n];

#define VISUAL_MODEL_UNREF(_vm_)	\
 SARVisualModelUnref(scene, (_vm_))

#define SOUND_STOP_PLAY(_sp_)		\
 SoundStopPlay(recorder, (_sp_))

	    /* Delete substructure */
	    if(obj_ptr->data != NULL)
	    {
		switch(obj_ptr->type)
		{
		  case SAR_OBJ_TYPE_GARBAGE:
		  case SAR_OBJ_TYPE_STATIC:
		  case SAR_OBJ_TYPE_AUTOMOBILE:
		  case SAR_OBJ_TYPE_WATERCRAFT:
		    break;
		  case SAR_OBJ_TYPE_AIRCRAFT:
		    aircraft = SAR_OBJ_GET_AIRCRAFT(obj_ptr);
		    if(aircraft != NULL)
		    {
			sar_obj_hoist_struct *hoist;

		        /* Engine sound play object and source index */
		        SOUND_STOP_PLAY(aircraft->engine_inside_sndplay);
		        aircraft->engine_inside_sndsrc = -1;

		        SOUND_STOP_PLAY(aircraft->engine_outside_sndplay);
		        aircraft->engine_outside_sndsrc = -1;

			/* Repeating warning sound play object and source index */
			SOUND_STOP_PLAY(aircraft->stall_sndplay);
			aircraft->stall_sndsrc = -1;

			SOUND_STOP_PLAY(aircraft->overspeed_sndplay);
			aircraft->overspeed_sndsrc = -1;

			/* Cockpit visual model */
			VISUAL_MODEL_UNREF(aircraft->visual_model_cockpit);

			/* Delete all parts and their visual models */
			SARObjDeleteParts(
			    scene, &aircraft->part, &aircraft->total_parts
			);

			/* Delete all rotors and their visual models */
			SARObjDeleteRotors(
			    scene, &aircraft->rotor, &aircraft->total_rotors
			);

			/* Delete all external fuel tanks and their visual models */
			SARObjDeleteExternalFuelTanks(
			    scene,
			    &aircraft->external_fueltank,
			    &aircraft->total_external_fueltanks
			);

			/* Delete hoist */
			hoist = aircraft->hoist;
		        if(hoist != NULL)
		        {
			    free(hoist->occupant);
			    free(hoist);
			    aircraft->hoist = NULL;
		        }

		        /* Delete all way pointer intercepts */
		        SARObjDeleteIntercepts(
			    scene,
			    &aircraft->intercept,
			    &aircraft->total_intercepts
		        );

		        /* Flight dynamics model */
		        if((scene->realm != NULL) &&
		           (aircraft->fdm != NULL)
		        )
		        {
			    SFMModelDelete(scene->realm, aircraft->fdm);
#if 0
			    /* SFM fdm destroy callback will call our
			     * callback function to set this pointer to
			     * NULL
			     */
			    aircraft->fdm = NULL;
#endif
			}
		    }
		    break;
		  case SAR_OBJ_TYPE_GROUND:
		    ground = SAR_OBJ_GET_GROUND(obj_ptr);
		    if(ground != NULL)
		    {
		        /* Heightfield z points map */
		        free(ground->z_point_value);
		        ground->z_point_value = NULL;

		        ground->grid_points_x = 0;
		        ground->grid_points_y = 0;
		        ground->grid_points_total = 0;
		    }
		    break;
	          case SAR_OBJ_TYPE_RUNWAY:
		    runway = SAR_OBJ_GET_RUNWAY(obj_ptr);
		    if(runway != NULL)
		    {
		        free(runway->north_label);
		        free(runway->south_label);
		        VISUAL_MODEL_UNREF(runway->north_label_vmodel);
		        VISUAL_MODEL_UNREF(runway->south_label_vmodel);
		        VISUAL_MODEL_UNREF(runway->threshold_vmodel);
		        VISUAL_MODEL_UNREF(runway->td_marker_vmodel);
		        VISUAL_MODEL_UNREF(runway->midway_marker_vmodel);
		        VISUAL_MODEL_UNREF(
			    runway->north_displaced_threshold_vmodel
		        );
		        VISUAL_MODEL_UNREF(
			    runway->south_displaced_threshold_vmodel
		        );
		    }
		    break;
		  case SAR_OBJ_TYPE_HELIPAD:
		    helipad = SAR_OBJ_GET_HELIPAD(obj_ptr);
		    if(helipad != NULL)
		    {
		        free(helipad->label);
		        VISUAL_MODEL_UNREF(helipad->label_vmodel);
		    }
		    break;
		  case SAR_OBJ_TYPE_HUMAN:
		    human = SAR_OBJ_GET_HUMAN(obj_ptr);
		    if(human != NULL)
		    {
			free(human->mesg_enter);
		    }
		    break;
		  case SAR_OBJ_TYPE_SMOKE:
		    smoke = SAR_OBJ_GET_SMOKE(obj_ptr);
		    if(smoke != NULL)
		    {
		        free(smoke->unit);
		        smoke->unit = NULL;
		        smoke->total_units = 0;
		    }
		    break;
		  case SAR_OBJ_TYPE_FIRE:
		    fire = SAR_OBJ_GET_FIRE(obj_ptr);
		    if(fire != NULL)
		    {

		    }
		    break;
		  case SAR_OBJ_TYPE_EXPLOSION:
		    explosion = SAR_OBJ_GET_EXPLOSION(obj_ptr);
		    if(explosion != NULL)
		    {

		    }
		    break;
		  case SAR_OBJ_TYPE_CHEMICAL_SPRAY:
		    chemical_spray = SAR_OBJ_GET_CHEMICAL_SPRAY(obj_ptr);
		    if(chemical_spray != NULL)
		    {
		        chemical_spray->owner = -1;
		        chemical_spray->tex_num = -1;
		    }
		    break;
		  case SAR_OBJ_TYPE_FUELTANK:
		    fueltank = SAR_OBJ_GET_FUELTANK(obj_ptr);
		    if(fueltank != NULL)
		    {

		    }
		    break;
		  case SAR_OBJ_TYPE_PREMODELED:
		    premodeled = SAR_OBJ_GET_PREMODELED(obj_ptr);
		    if(premodeled != NULL)
		    {

		    }
		    break;
		}

		free(obj_ptr->data);
		obj_ptr->data = NULL;
	    }

	    /* Delete standard visual models */
	    VISUAL_MODEL_UNREF(obj_ptr->visual_model_shadow);
	    VISUAL_MODEL_UNREF(obj_ptr->visual_model_night);
	    VISUAL_MODEL_UNREF(obj_ptr->visual_model_dusk);
	    VISUAL_MODEL_UNREF(obj_ptr->visual_model_dawn);
	    VISUAL_MODEL_UNREF(obj_ptr->visual_model_far);
	    VISUAL_MODEL_UNREF(obj_ptr->visual_model_ir);
	    VISUAL_MODEL_UNREF(obj_ptr->visual_model);

	    /* Delete contact bounds */
	    if(obj_ptr->contact_bounds != NULL)
	    {
		free(obj_ptr->contact_bounds);
		obj_ptr->contact_bounds = NULL;
	    }

	    /* Delete lights */
	    SARObjDeleteLights(
		scene,
		&obj_ptr->light, &obj_ptr->total_lights
	    );

	    /* Delete sound sources */
	    for(i = 0; i < obj_ptr->total_sndsrcs; i++)
		SARSoundSourceDelete(obj_ptr->sndsrc[i]);
	    free(obj_ptr->sndsrc);
	    obj_ptr->sndsrc = NULL;
	    obj_ptr->total_sndsrcs = 0;


	    /* Delete name */
	    free(obj_ptr->name);
	    obj_ptr->name = NULL;

	    /* Delete the Object itself */
	    free(obj_ptr);
	    (*ptr)[n] = NULL;


	    /* Object has now been deleted, now we need to check if
	     * any other objects are referencing this object
	     *
	     * If so, then we need to reset those references
	     */
	    for(i = 0; i < *total; i++)
	    {
		obj_ptr2 = (*ptr)[i];
		if(obj_ptr2 == NULL)
		    continue;

		/* Skip this object */
		if(i == n)
		    continue;

		switch(obj_ptr2->type)
		{
		  case SAR_OBJ_TYPE_GARBAGE:
		  case SAR_OBJ_TYPE_STATIC:
		  case SAR_OBJ_TYPE_AUTOMOBILE:
		  case SAR_OBJ_TYPE_WATERCRAFT:
		  case SAR_OBJ_TYPE_AIRCRAFT:
		  case SAR_OBJ_TYPE_GROUND:
		  case SAR_OBJ_TYPE_RUNWAY:
		    break;
		  case SAR_OBJ_TYPE_HELIPAD:
		    helipad = SAR_OBJ_GET_HELIPAD(obj_ptr2);
		    if(helipad != NULL)
		    {
			if(helipad->ref_object == n)
			    helipad->ref_object = -1;
		    }
		    break;
		  case SAR_OBJ_TYPE_HUMAN:
		    human = SAR_OBJ_GET_HUMAN(obj_ptr2);
		    if(human != NULL)
		    {
			if(human->intercepting_object == n)
			    human->intercepting_object = -1;
		    }
		    break;
		  case SAR_OBJ_TYPE_SMOKE:
		    smoke = SAR_OBJ_GET_SMOKE(obj_ptr2);
		    if(smoke != NULL)
		    {
			if(smoke->ref_object == n)
			    smoke->ref_object = -1;
		    }
		    break;
		  case SAR_OBJ_TYPE_FIRE:
		    fire = SAR_OBJ_GET_FIRE(obj_ptr2);
		    if(fire != NULL)
		    {
			if(fire->ref_object == n)
			    fire->ref_object = -1;
		    }
		    break;
		  case SAR_OBJ_TYPE_EXPLOSION:
		    explosion = SAR_OBJ_GET_EXPLOSION(obj_ptr2);
		    if(explosion != NULL)
		    {
			if(explosion->ref_object == n)
			    explosion->ref_object = -1;
		    }
		    break;
		  case SAR_OBJ_TYPE_CHEMICAL_SPRAY:
		    chemical_spray = SAR_OBJ_GET_CHEMICAL_SPRAY(obj_ptr2);
		    if(chemical_spray != NULL)
		    {
			if(chemical_spray->owner == n)
			    chemical_spray->owner = -1;
		    }
		    break;
		  case SAR_OBJ_TYPE_FUELTANK:
		    fueltank = SAR_OBJ_GET_FUELTANK(obj_ptr2);
		    if(fueltank != NULL)
		    {
			if(fueltank->ref_object == n)
			    fueltank->ref_object = -1;
		    }
		    break;
		  case SAR_OBJ_TYPE_PREMODELED:
		    break;
		}
	    }

	    /* Unreference this object from the scene structure so that
	     * no other resources check the scene structure and mistake
	     * the object as still being allocated
	     */
	    if(scene != NULL)
	    {
		/* Was this referenced as the player object? */
		if(obj_ptr == scene->player_obj_ptr)
		    scene->player_obj_ptr = NULL;

		if(n == scene->player_obj_num)
		    scene->player_obj_num = -1;

		/* Was this referenced as the camera target? */
		if(n == scene->camera_target)
		    scene->camera_target = -1;

		/* Remove this object from ground list as needed */
		SARObjRemoveFromGroundList(scene, obj_ptr);

		/* Remove this object from human need rescue list as needed */
		SARObjRemoveFromHumanRescueList(scene, obj_ptr);
	    }

#undef VISUAL_MODEL_UNREF
#undef SOUND_STOP_PLAY
	}
}

/*
 *	Generates an array of tiles. The call to begin a list and
 *	set up color and texture should already have been made before
 *	calling this function.
 */
void SARObjGenerateTilePlane(
	float min, float max,	/* In meters */
	float tile_width, float tile_height
)
{
	float x, y, x2, y2;


	/* Begin gl instructions */
	glBegin(GL_QUADS);

	/* Quadrant 1 is `upper right' and quadrants are numbed
	 * clockwise.
	 */

	/* Quads in quadrant 1 */
	for(y = 0; y < max; y += tile_height)
	{
	    for(x = 0; x < max; x += tile_width)
	    {
		x2 = x + tile_width;
		y2 = y + tile_height;
		{
		    glNormal3f(0.0f, 1.0f, 0.0f);
		    glTexCoord2f(0.0f, 1.0f - 0.0f);
		    glVertex3f((GLfloat)x, (GLfloat)0.0f, (GLfloat)-y);
		    glTexCoord2f(1.0f, 1.0f - 0.0f);
		    glVertex3f((GLfloat)x2, (GLfloat)0.0f, (GLfloat)-y);
		    glTexCoord2f(1.0f, 1.0f - 1.0f);
		    glVertex3f((GLfloat)x2, (GLfloat)0.0f, (GLfloat)-y2);
		    glTexCoord2f(0.0f, 1.0f - 1.0f);
		    glVertex3f((GLfloat)x, (GLfloat)0.0f, (GLfloat)-y2);
		}
	    }
	}
	/* Quads in quadrant 2 */
	for(y = 0; y > min; y -= tile_height)
	{
	    for(x = 0; x < max; x += tile_width)
	    {
		x2 = x + tile_width;
		y2 = y - tile_height;
		{
		    glNormal3f(0.0f, 1.0f, 0.0f);
		    glTexCoord2f(0.0f, 1.0f - 0.0f);
		    glVertex3f((GLfloat)x, 0.0f, (GLfloat)-y2);
		    glTexCoord2f(1.0f, 1.0f - 0.0f);
		    glVertex3f((GLfloat)x2, 0.0f, (GLfloat)-y2);
		    glTexCoord2f(1.0f, 1.0f - 1.0f);
		    glVertex3f((GLfloat)x2, 0.0f, (GLfloat)-y);
		    glTexCoord2f(0.0f, 1.0f - 1.0f);
		    glVertex3f((GLfloat)x, 0.0f, (GLfloat)-y);
	       }
	   }
       }
       /* Quads in quadrant 3 */
       for(y = 0; y > min; y -= tile_height)
       {
	   for(x = 0; x > min; x -= tile_width)
	   {
	       x2 = x - tile_width;
	       y2 = y - tile_height;
	       {
		   glNormal3f(0.0f, 1.0f, 0.0f);
		   glTexCoord2f(1.0f, 1.0f - 1.0f);
		   glVertex3f((GLfloat)x, 0.0f, (GLfloat)-y);
		   glTexCoord2f(0.0f, 1.0f - 1.0f);
		   glVertex3f((GLfloat)x2, 0.0f, (GLfloat)-y);
		   glTexCoord2f(0.0f, 1.0f - 0.0f);
		   glVertex3f((GLfloat)x2, 0.0f, (GLfloat)-y2);
		   glTexCoord2f(1.0f, 1.0f - 0.0f);
		   glVertex3f((GLfloat)x, 0.0f, (GLfloat)-y2);
	       }
	    }
	}
	/* Quads in quadrant 4 */
	for(y = 0; y < max; y += tile_height)
	{
	    for(x = 0; x > min; x -= tile_width)
	    {
		x2 = x - tile_width;
		y2 = y + tile_height;
		{
		   glNormal3f(0.0f, 1.0f, 0.0f);
		   glTexCoord2f(1.0f, 1.0f - 1.0f);
		   glVertex3f((GLfloat)x, 0.0f, (GLfloat)-y2);
		   glTexCoord2f(0.0f, 1.0f - 1.0f);
		   glVertex3f((GLfloat)x2, 0.0f, (GLfloat)-y2);
		   glTexCoord2f(0.0f, 1.0f - 0.0f);
		   glVertex3f((GLfloat)x2, 0.0f, (GLfloat)-y);
		   glTexCoord2f(1.0f, 1.0f - 0.0f);
		   glVertex3f((GLfloat)x, 0.0f, (GLfloat)-y);
		}
	    }
	}
	glEnd();
}
