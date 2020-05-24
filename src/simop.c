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

#include "matrixmath.h"
#include "sfm.h"
#include "v3dtex.h"

#include "gctl.h"
#include "sarreality.h"

#include "obj.h"
#include "objsound.h"
#include "objutils.h"
#include "sar.h"
#include "messages.h"
#include "sardrawselect.h"
#include "simcb.h"
#include "simutils.h"
#include "simsurface.h"
#include "simop.h"
#include "explosion.h"
#include "smoke.h"
#include "config.h"


static float SARSimLandingGearDragCoeff(
	sar_scene_struct *scene, sar_object_struct *obj_ptr,
	sar_obj_part_struct *lgear_ptr		/* First landing gear */
);
static float SARSimFlapDragCoeff(
	sar_scene_struct *scene, sar_object_struct *obj_ptr,
	sar_object_aircraft_struct *aircraft
);
static float SARSimHoistDragCoeff(
	sar_scene_struct *scene, sar_object_struct *obj_ptr,
	const sar_obj_hoist_struct *hoist
);

float SARSimFindGround(
	sar_scene_struct *scene,
	sar_object_struct **ptr, int total,
	const sar_position_struct *pos
);

int SARSimDoMortality(
	sar_core_struct *core_ptr, sar_object_struct *obj_ptr
);

int SARSimRestart(
	sar_core_struct *core_ptr, sar_scene_struct *scene,
	sar_object_struct ***ptr, int *total,
	int obj_num, sar_object_struct *obj_ptr
);
int SARSimDeleteEffects(
	sar_core_struct *core_ptr, sar_scene_struct *scene,
	sar_object_struct ***ptr, int *total,
	int obj_num,
	unsigned int filter_flags       /* Any of SARSIM_DELETE_EFFECTS_* */
);
void SARSimSetAircraftCrashed(
	sar_scene_struct *scene,
	sar_object_struct ***ptr, int *total,
	sar_object_struct *obj_ptr,
	sar_object_aircraft_struct *aircraft
);

void SARSimSetSFMValues(
	sar_core_struct *core_ptr, sar_scene_struct *scene,
	sar_object_struct *obj_ptr
);
void SARSimGetSFMValues(
	sar_scene_struct *scene, sar_object_struct *obj_ptr
);

int SARSimApplyNaturalForce(
	sar_core_struct *core_ptr, sar_object_struct *obj_ptr
);
int SARSimApplyArtificialForce(
	sar_core_struct *core_ptr, sar_object_struct *obj_ptr
);

static void SARSimApplyGCTLAutoPilot(
	sar_core_struct *core_ptr, sar_object_struct *obj_ptr,
	gctl_struct *gc,
	float *h_con_coeff,
	float *p_con_coeff,
	float *b_con_coeff,
	float *elevator_trim,
	float *throttle_coeff,
	float *collective
);
void SARSimApplyGCTL(
	sar_core_struct *core_ptr, sar_object_struct *obj_ptr
);


#define MAX(a,b)        (((a) > (b)) ? (a) : (b))
#define MIN(a,b)        (((a) < (b)) ? (a) : (b))
#define CLIP(a,l,h)     (MIN(MAX((a),(l)),(h)))

#define RADTODEG(r)     ((r) * 180.0 / PI)
#define DEGTORAD(d)     ((d) * PI / 180.0)


#define SAR_ABSOLUTE(x)	(((x) < 0.0) ? ((x) * -1.0) : (x))


/*
 *	Returns the drag coeff caused by the state of the first
 *	landing gear on the given object.
 *
 *	Return value is from 0.0 to 0.5 where 0.0 is no additional
 *	drag and 0.5 is full drag.
 */
static float SARSimLandingGearDragCoeff(
	sar_scene_struct *scene, sar_object_struct *obj_ptr,
	sar_obj_part_struct *lgear_ptr		/* First landing gear */
)
{
	if(lgear_ptr == NULL)
	    return(0.0f);

	/* Is landing gear retracted or not retractable? If so then
	 * no additional drag should be added.
	 */
	if(!(lgear_ptr->flags & SAR_OBJ_PART_FLAG_STATE) ||
	   (lgear_ptr->flags & SAR_OBJ_PART_FLAG_LGEAR_FIXED)
	)
	    return(0.0f);

	/* Return the position of the gear by its animated position,
	 * where 0 is up and (sar_grad_anim_t)-1 is down.
	 * This will produce a coefficient in the range of 0.0 to 0.5.
	 */
	return((float)(
	    (1.0 - ((float)lgear_ptr->anim_pos /
	    (float)((sar_grad_anim_t)-1))) * 0.5
	));
}

/*
 *      Returns the drag coeff caused by the state of the flaps.
 *
 *      Return value is from 0.0 to 0.3 where 0.0 is no additional
 *      drag and 0.3 is full drag.
 */
static float SARSimFlapDragCoeff(
	sar_scene_struct *scene, sar_object_struct *obj_ptr,
	sar_object_aircraft_struct *aircraft
)
{
	if(aircraft == NULL)
	    return(0.0f);

	return((float)(aircraft->flaps_position * 0.3));
}

/*
 *      Returns the drag coeff caused by the position of the hoist
 *	(how far the rescue basket is from the hoist).
 *
 *      Return value is from 0.0 to 1.0 where 0.0 is no additional
 *      drag and 1.0 is full drag.
 */            
static float SARSimHoistDragCoeff(
	sar_scene_struct *scene, sar_object_struct *obj_ptr,
	const sar_obj_hoist_struct *hoist
)
{
	float rope_cur, rope_max;

	if(hoist == NULL)
	    return(0.0f);

	/* Get rope lengths which imply position of rescue basket.
	 * Remember to subtract the upper z bounds off the lengths.
	 */
	rope_cur = (float)MAX(
	    hoist->rope_cur - hoist->contact_z_min, 0.0
	);
	rope_max = (float)MAX(
	    hoist->rope_max - hoist->contact_z_min, 0.0
	);

	if(rope_max > 0.0f)
	    return((float)(MIN(rope_cur / rope_max, 1.0) * 0.9));
	else
	    return(0.0f);
}


/*
 *      Returns the elevation of (not to) the solid ground with respect
 *      to the given position pos.
 *
 *      Only objects of type ground will be checked, becareful not
 *      to check the given pos if it came from a ground object or
 *      else it would return its own elevation.
 *
 *      So this function should only be used to check landable/walkable
 *      ground for objects that need it (ie aircraft and humans).
 *      Also note that pos passed to this function should not come from
 *      an object who's contact bounds specify the crash flag
 *      SAR_CRASH_FLAG_SUPPORT_SURFACE or else they will keep being
 *      supported on themselves.
 */
float SARSimFindGround(
	sar_scene_struct *scene,
	sar_object_struct **ptr, int total,
	const sar_position_struct *pos 
)
{ 
	int i;
	sar_object_struct *tar_obj_ptr;
	float new_height, cur_height = 0.0f;
	const sar_contact_bounds_struct *cb_tar;
	const sar_position_struct *pos_src = pos;

	if((scene == NULL) || (pos_src == NULL))
	    return(cur_height);

	/* Iterate from last object to first */
	for(i = total - 1; i >= 0; i--)
	{
	    tar_obj_ptr = ptr[i];
	    if(tar_obj_ptr == NULL)
		continue;
 
	    /* Check if target object specifies a landable surface */
	    cb_tar = tar_obj_ptr->contact_bounds;
	    if(cb_tar != NULL)
	    {
		if(cb_tar->crash_flags & SAR_CRASH_FLAG_SUPPORT_SURFACE)
		{
		    new_height = SARSimSupportSurfaceHeight(
			tar_obj_ptr, cb_tar, pos_src,
			(float)SAR_DEF_SURFACE_CONTACT_Z_TOLORANCE
		    );
		    if(new_height > cur_height)
			cur_height = new_height;
		}
	    }
 
	    /* Get ground object height (function will check
	     * if object really is a ground object), on error
	     * it will return 0.0 so accepting its value unconditionally
	     * is safe.
	     *
	     * Both ground object cylendrical contact and heightfield
	     * contact will be checked.
	     */
	    new_height = (float)SARSimHFGetGroundHeight(
		tar_obj_ptr,    /* Ground object */
		pos_src
	    );

	    /* New height higher? */
	    if(new_height > cur_height)
		cur_height = new_height;
	}

	return(cur_height);
}


/*
 *      Checks if the life span of the given object has been exceeded,
 *      if so then that object will be deleted and returns non-zero.
 *      Calling function may need to consider the pointer array base for
 *	the objects changed if non-zero is returned.
 *
 *	Any object who's life_span is 0 or less will be ignored.
 */
int SARSimDoMortality(
	sar_core_struct *core_ptr, sar_object_struct *obj_ptr
)
{
	if(obj_ptr == NULL)
	    return(0);

	/* Ignore objects who's life_span is 0 or less */
	if(obj_ptr->life_span <= 0)
	    return(0);

	/* Check if object's life_span has exceeded the current
	 * time. Note that the life_span value was calculated from
	 * when it was created plus its life span (cur_millitime +
	 * life_expectancy) so when the current time exceeds the
	 * object's life_span it indicates the object must be deleted.
	 * All units are in milliseconds.
	 */
	if(obj_ptr->life_span <= cur_millitime)
	{
	    int i;

	    for(i = 0; i < core_ptr->total_objects; i++)
	    {
		if(core_ptr->object[i] == obj_ptr)
		    SARObjDelete(
			core_ptr,
			&core_ptr->object,
			&core_ptr->total_objects,
			i
		    );
	    }
	    return(1);
	}
	else 
	{
	    return(0);
	}
}

/*
 *	Moves the given object to the nearest helipad mark restartable
 *	and refuels/repairs the object.
 *
 *	This function is intended to be called when the player has
 *	crashed and needs to be restarted.
 *
 *	Does not check if restarting is valid, all permission checks must
 *	be done by the calling function before calling this function.
 *
 *	Returns the index to the helipad object the object is restarting
 *	at or -1 on failure.
 */
int SARSimRestart(
	sar_core_struct *core_ptr, sar_scene_struct *scene,
	sar_object_struct ***ptr, int *total,
	int obj_num, sar_object_struct *obj_ptr
)
{
	int i, closest_obj_num = -1;
	float closest_distance = 0.0;
	sar_position_struct *pos;
	sar_object_struct *start_obj_ptr, *closest_obj_ptr = NULL;
	sar_object_helipad_struct *obj_helipad_ptr = NULL;
	sar_object_aircraft_struct *aircraft;
	sar_obj_hoist_struct *hoist_ptr;
	SFMModelStruct *fdm = NULL;


	if((scene == NULL) || (ptr == NULL) || (total == NULL) ||
	   (obj_ptr == NULL)
	)
	    return(-1);

	pos = &obj_ptr->pos;

	/* Check if this is the player object on the scene structure */
	if((obj_num == scene->player_obj_num) ||
	   (obj_ptr == scene->player_obj_ptr)
	)
	{
	    /* Reset player has crashed mark on scene structure */
	    scene->player_has_crashed = False;

	    /* Also reset time compression */
	    time_compression = 1.0f;

	    /* Move camera reference back to cockpit */
	    scene->camera_ref = SAR_CAMERA_REF_COCKPIT;

	}

	/* Repair and refuel the object */
	SARSimOpRepair(scene, obj_ptr);
	SARSimOpRefuel(scene, obj_ptr);

	/* Unload any passengers since we're re-starting */
	SARSimOpPassengersUnloadAll(scene, obj_ptr);

	/* Search for the closest helipad to restart at */
	for(i = 0; i < *total; i++)
	{
	    start_obj_ptr = (*ptr)[i];
	    if(start_obj_ptr == NULL)
		continue;

	    if(start_obj_ptr == obj_ptr)
		continue;

	    obj_helipad_ptr = SAR_OBJ_GET_HELIPAD(start_obj_ptr);
	    if(obj_helipad_ptr == NULL)
		continue;

	    /* Skip if helipad is referencing this object */
	    if(obj_helipad_ptr->flags & SAR_HELIPAD_FLAG_REF_OBJECT)
	    {
		if((obj_helipad_ptr->ref_object == obj_num) &&
		   (obj_num > -1)
		)
		    continue;
	    }

	    /* Does this helipad allow restarts? */
	    if(obj_helipad_ptr->flags & SAR_HELIPAD_FLAG_RESTART_POINT)
	    {
		/* Calculate 2d distance from object to potential starting
		 * point object
		 */
		float dlen = (float)SFMHypot2(
		    start_obj_ptr->pos.x - pos->x, start_obj_ptr->pos.y - pos->y
		);

		/* No previous closest object recorded? */
		if(closest_obj_ptr == NULL)
		{
		    closest_distance = dlen;
		    closest_obj_ptr = start_obj_ptr;
		    closest_obj_num = i;
		}
		else
		{
		    /* Closer? */
		    if(dlen < closest_distance)
		    {
			closest_distance = dlen;
			closest_obj_ptr = start_obj_ptr;
			closest_obj_num = i;
		    }
		}
	    }
	}
	/* Got closest helipad? */
	if(closest_obj_ptr != NULL)
	{
	    /* Move object to closest helipad */
	    memcpy(pos, &closest_obj_ptr->pos, sizeof(sar_position_struct));
	    obj_ptr->ground_elevation_msl = pos->z;

	    /* Set direction to match helipad */
	    memcpy(&obj_ptr->dir, &closest_obj_ptr->dir, sizeof(sar_direction_struct));

	    /* Reset values by object type */
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
		    fdm = aircraft->fdm;
		    if(fdm != NULL)
		    {
#define TAR_PTR fdm
			TAR_PTR->heading_control_coeff = 0.0f;
			TAR_PTR->pitch_control_coeff = 0.0f;
			TAR_PTR->bank_control_coeff = 0.0f;
			TAR_PTR->elevator_trim_coeff = 0.0f;
			TAR_PTR->throttle_coeff = 0.0f;

			TAR_PTR->velocity_vector.x = 0.0f;
			TAR_PTR->velocity_vector.y = 0.0f;
			TAR_PTR->velocity_vector.z = 0.0f;

			TAR_PTR->airspeed_vector.x = 0.0f;
			TAR_PTR->airspeed_vector.y = 0.0f;
			TAR_PTR->airspeed_vector.z = 0.0f;
			TAR_PTR->landed_state = True;
			TAR_PTR->stopped = True;
#undef TAR_PTR
		    }

#define TAR_PTR	aircraft
		    /* Reset current airspeed and velocity */
		    memset(&TAR_PTR->vel, 0x00, sizeof(sar_position_struct));
		    memset(&TAR_PTR->airspeed, 0x00, sizeof(sar_position_struct));
		    TAR_PTR->z_accel = 0.0f;

		    /* Control positions */
		    TAR_PTR->control_heading = 0.0f;
		    TAR_PTR->control_pitch = 0.0f;
		    TAR_PTR->control_bank = 0.0f;
		    TAR_PTR->control_effective_heading = 0.0f;
		    TAR_PTR->control_effective_pitch = 0.0f;
		    TAR_PTR->control_effective_bank = 0.0f;
		    TAR_PTR->elevator_trim = 0.0f;
		    TAR_PTR->flaps_position = 0.0f;

		    TAR_PTR->throttle = 0.0f;
		    TAR_PTR->collective = 0.0f;

		    /* Reset air brakes state, note that visual models
		     * for air brakes will be reset during regular 
		     * simulation when they detect the new state we
		     * set here
		     */
		    if(TAR_PTR->air_brakes_state > -1)
		    {
			TAR_PTR->air_brakes_state = 1;
			SARSimOpAirBrakes(
			    scene, ptr, total, obj_ptr, 0,
			    NULL, 0, 0
			);
		    }

		    /* Lower landing gears, first mark as not landed in
		     * order to trick it to lower landing gears
		     */
		    TAR_PTR->landed = 0;
		    SARSimOpLandingGear(
			scene, ptr, total, obj_ptr, 1,
			NULL, 0, 0
		    );
		    /* Pitch rotors up if the rotors can pitch (landed
		     * state also needs to be 0 for this)
		     */
		    if(TAR_PTR->engine_can_pitch)
		    {
			SARSimPitchEngine(
			    scene, ptr, total, obj_ptr, 0
			);
		    }

		    /* Spot light direction */
		    if(True)
		    {
			sar_direction_struct *dir = &TAR_PTR->spotlight_dir;
			dir->heading = (float)SFMDegreesToRadians(
			    SAR_DEF_SPOTLIGHT_HEADING
			);
			dir->pitch = (float)SFMDegreesToRadians(
			    SAR_DEF_SPOTLIGHT_PITCH
			);
			dir->bank = (float)SFMDegreesToRadians(
			    SAR_DEF_SPOTLIGHT_BANK
			);
		    }

		    /* Switch off autopilot */
		    TAR_PTR->autopilot_state = SAR_AUTOPILOT_OFF;

		    /* Reset hoist */
		    hoist_ptr = SARObjGetHoistPtr(obj_ptr, 0, NULL);
		    if(hoist_ptr != NULL)
		    {
			hoist_ptr->rope_cur = 0.0f;
		    }

		    /* Close the door */
		    SARSimOpDoorRescue(
			scene, ptr, total, obj_ptr, 0
		    );

		    /* Turn engine off (restart) */
		    SARSimOpEngine(
			scene, ptr, total,
			obj_ptr, SAR_ENGINE_OFF,
			NULL, 0, 0
		    );

		    /* Move aircraft up just a bit to offset for its
		     * belly and gear heights.
		     */
		    pos->z += (TAR_PTR->belly_height +
			TAR_PTR->gear_height);

		    /* Implicitly mark as landed and not on water */
		    TAR_PTR->landed = 1;
		    TAR_PTR->on_water = 0;
#undef TAR_PTR
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

	    /* Realize new position */
	    SARSimWarpObject(
		scene, obj_ptr, pos, &obj_ptr->dir
	    );
	}

	/* Destroy all effects objects that reference this object */
	SARSimDeleteEffects(
	    core_ptr, scene, ptr, total,
	    obj_num,
	    0
	);

	return(closest_obj_num);
}


/*
 *	Deletes all explosion, smoke, and other `effects objects'
 *	with respect to the given reference object number.
 *
 *	If the given object is -1 then all of the above will be deleted
 *	if they are non-errant (have a reference object).
 *
 *	filter_flags can be any of the following:
 *
 *	SARSIM_DELETE_EFFECTS_SMOKE_STOP_RESPAWN
 *		Stop smoke trail objects from respawning but do not delete
 *
 *	Returns the number of objects deleted.
 */
int SARSimDeleteEffects(
	sar_core_struct *core_ptr, sar_scene_struct *scene,
	sar_object_struct ***ptr, int *total,
	int obj_num,
	unsigned int filter_flags       /* Any of SARSIM_DELETE_EFFECTS_* */
)
{
	int i, objects_deleted = 0;
	Boolean delete_this_object;
	sar_object_struct *effects_obj_ptr;
	sar_object_smoke_struct *smoke;
	sar_object_fire_struct *fire;
	sar_object_explosion_struct *explosion;
	sar_object_chemical_spray_struct *spray;

	if((scene == NULL) || (ptr == NULL) || (total == NULL))
	    return(objects_deleted);

	for(i = 0; i < *total; i++)
	{
	    effects_obj_ptr = (*ptr)[i];
	    if(effects_obj_ptr == NULL)
		continue;

	    /* Skip given reference object */
	    if(obj_num == i)
		continue;

	    /* Reset delete_this_object marker to False so if the below
	     * check determines that this object should be deleted,
	     * delete_this_object will be set to True.
	     */
	    delete_this_object = False;

	    /* Handle by object type */
	    switch(effects_obj_ptr->type)
	    {
	      case SAR_OBJ_TYPE_GARBAGE:
	      case SAR_OBJ_TYPE_STATIC:
	      case SAR_OBJ_TYPE_AUTOMOBILE:
	      case SAR_OBJ_TYPE_WATERCRAFT:
	      case SAR_OBJ_TYPE_AIRCRAFT:
	      case SAR_OBJ_TYPE_GROUND:
	      case SAR_OBJ_TYPE_RUNWAY:
	      case SAR_OBJ_TYPE_HELIPAD:
	      case SAR_OBJ_TYPE_HUMAN:
		break;

	      case SAR_OBJ_TYPE_SMOKE:
		smoke = SAR_OBJ_GET_SMOKE(effects_obj_ptr);
		if(smoke != NULL)
		{
		    if(obj_num > -1)
		    {
			/* Does the effect object reference the given
			 * object?
			 */
			if(smoke->ref_object == obj_num)
			    delete_this_object = True;
		    }
		    else
		    {
			/* No object given as input, so treat criteria
			 * as delete all non-errant effects objects
			 */
			if(smoke->ref_object > -1)
			    delete_this_object = True;
		    }
		}
		break;

	      case SAR_OBJ_TYPE_FIRE:
		fire = SAR_OBJ_GET_FIRE(effects_obj_ptr);
		if(fire != NULL)
		{
		    if(obj_num > -1)
		    {
			/* Does the effect object reference the given
			 * object?
			 */
			if(fire->ref_object == obj_num)
			    delete_this_object = True;
		    }
		    else
		    {
			/* No object given as input, so treat criteria
			 * as delete all non-errant effects objects
			 */
			if(fire->ref_object > -1)
			    delete_this_object = True;
		    }
		}
		break;

	      case SAR_OBJ_TYPE_EXPLOSION:
		explosion = SAR_OBJ_GET_EXPLOSION(effects_obj_ptr);
		if(explosion != NULL)
		{
		    if(obj_num > -1)
		    {
			/* Does the effect object reference the given 
			 * object?
			 */
			if(explosion->ref_object == obj_num)
			    delete_this_object = True;
		    }
		    else
		    {
			/* No object given as input, so treat criteria
			 * as delete all non-errant effects objects
			 */
			if(explosion->ref_object > -1)
			    delete_this_object = True;
		    }
		}
		break;

	      case SAR_OBJ_TYPE_CHEMICAL_SPRAY:
		spray = SAR_OBJ_GET_CHEMICAL_SPRAY(effects_obj_ptr);
		if(spray != NULL)
		{
#if 0
/* TODO */
		    if(obj_num > -1)
		    {
			/* Does the effect object reference the given
			 * object? 
			 */
			if(spray->owner == obj_num)
			    delete_this_object = True;
		    }
		    else
		    {   
			/* No object given as input, so treat criteria
			 * as delete all non-errant effects objects
			 */     
			if(spray->owner > -1)
			    delete_this_object = True;
		    }
#endif
		}    
		break;

	      case SAR_OBJ_TYPE_FUELTANK:
	      case SAR_OBJ_TYPE_PREMODELED:
		break;
	    }

	    /* Delete this effect object? */
	    if(delete_this_object)
	    {
		/* Delete effect object by type, this is so we can
		 * handle type-specific filters
		 */
		switch(effects_obj_ptr->type)
		{
		  case SAR_OBJ_TYPE_SMOKE:
		    /* Stop smoke trail from respawning instead of
		     * deleting it?
		     */
		    if(filter_flags & SARSIM_DELETE_EFFECTS_SMOKE_STOP_RESPAWN)
		    {
			smoke = SAR_OBJ_GET_SMOKE(effects_obj_ptr);
			if(smoke != NULL)
			{
			    smoke->respawn_int = 0l;

			    /* Mark that smoke trail should be deleted
			     * when all of its units are no longer
			     * visible
			     */
			    smoke->delete_when_no_units = 1;
			}
		    }
		    else
		    {
			SARObjDelete(
			    core_ptr, ptr, total, i
			);
			objects_deleted++;
		    }
		    break;

		  default:
		    /* We know this is an effects object, but of a type
		     * that is not filtered. So we can safely delete it
		     * without checking filter_flags
		     */
		    SARObjDelete(
			core_ptr, ptr, total, i
		    );
		    objects_deleted++;
		    break;
		}

		/* Do not go further since this effects object is now
		 * now deleted
		 */
		continue;

	    }	/* Delete this effect object? */

	}

	return(objects_deleted);
}

/*
 *	Sets up values on the given aircraft object to be crashed.
 *
 *	Does not create any explosions, play sounds, or updates related
 *	resources, it is up to the calling function to do that about this
 *	crash.
 */
void SARSimSetAircraftCrashed(
	sar_scene_struct *scene,
	sar_object_struct ***ptr, int *total,
	sar_object_struct *obj_ptr,
	sar_object_aircraft_struct *aircraft
)
{
	int i, occupant_obj_num;
	sar_direction_struct *dir, new_dir;
	sar_obj_hoist_struct *hoist_ptr;
	sar_object_struct *occupant_obj_ptr;
	sar_object_human_struct *human;


	if((scene == NULL) || (obj_ptr == NULL) ||
	   (aircraft == NULL) || (ptr == NULL) || (total == NULL)
	)
	    return;

	/* Get direction of object */
	dir = &obj_ptr->dir;

	/* Get pointer to aircraft's hoist */
	hoist_ptr = SARObjGetHoistPtr(obj_ptr, 0, NULL);
	if(hoist_ptr != NULL)
	{
	  /* Iterate through occupants in hoist */
	  for(i = 0; i < hoist_ptr->total_occupants; i++)
	  {
	    occupant_obj_num = hoist_ptr->occupant[i];
	    if(SARObjIsAllocated(*ptr, *total, occupant_obj_num))
		occupant_obj_ptr = (*ptr)[occupant_obj_num];
	    else
		continue;

	    /* Skip our object itself just in case */
	    if(occupant_obj_ptr == obj_ptr)
		continue;

	    /* Handle by occupant object type */
	    switch(occupant_obj_ptr->type)
	    {
	      case SAR_OBJ_TYPE_GARBAGE:
	      case SAR_OBJ_TYPE_STATIC:
	      case SAR_OBJ_TYPE_AUTOMOBILE:
	      case SAR_OBJ_TYPE_WATERCRAFT:
	      case SAR_OBJ_TYPE_AIRCRAFT:
	      case SAR_OBJ_TYPE_GROUND:
	      case SAR_OBJ_TYPE_RUNWAY:
	      case SAR_OBJ_TYPE_HELIPAD:
		break;

	      case SAR_OBJ_TYPE_HUMAN:
		human = SAR_OBJ_GET_HUMAN(occupant_obj_ptr);
		if(human != NULL)
		{
		    /* Mark human as no longer gripped in the rescue
		     * basket.
		     */
		    human->flags &= ~SAR_HUMAN_FLAG_GRIPPED;

		    /* Mark as lying down */
		    human->flags &= ~SAR_HUMAN_FLAG_SIT;
		    human->flags &= ~SAR_HUMAN_FLAG_SIT_DOWN;
		    human->flags &= ~SAR_HUMAN_FLAG_SIT_UP;
		    human->flags |= SAR_HUMAN_FLAG_LYING;

		    /* Mark human as not alert and not aware */
		    human->flags &= ~SAR_HUMAN_FLAG_ALERT;
		    human->flags &= ~SAR_HUMAN_FLAG_AWARE;

		    human->flags &= ~SAR_HUMAN_FLAG_RUN_TOWARDS;
		    human->flags &= ~SAR_HUMAN_FLAG_RUN_AWAY;
		    human->flags &= ~SAR_HUMAN_FLAG_PUSHING;
		    human->flags &= ~SAR_HUMAN_FLAG_DIVER_CATCHER;

		    /* Not in water (need to work on this) */
		    human->flags &= ~SAR_HUMAN_FLAG_IN_WATER;

		    human->flags &= ~SAR_HUMAN_FLAG_ON_STREATCHER;
		}
		break;

	      case SAR_OBJ_TYPE_SMOKE:
	      case SAR_OBJ_TYPE_FIRE:
	      case SAR_OBJ_TYPE_EXPLOSION:
	      case SAR_OBJ_TYPE_CHEMICAL_SPRAY:
	      case SAR_OBJ_TYPE_FUELTANK:
	      case SAR_OBJ_TYPE_PREMODELED:
		break;
	    }

	    /* Set attitude to make it `upright' or lie flat */
	    occupant_obj_ptr->dir.pitch = (float)(0.0 * PI);
	    occupant_obj_ptr->dir.bank = (float)(0.0 * PI);

	    /* Mark this occupant as no longer in the list */
	    hoist_ptr->occupant[i] = -1;
	  }

	  /* Clear hoist list */
	  free(hoist_ptr->occupant);
	  hoist_ptr->occupant = NULL;
	  hoist_ptr->total_occupants = 0;

	  /* Retract rope */
	  hoist_ptr->rope_cur = 0.0f;
	  hoist_ptr->rope_cur_vis = 0.0f;
	  hoist_ptr->on_ground = 0;
	  hoist_ptr->occupants_mass = 0;
	}

	/* Begin setting values on aircraft */

	/* Mark as not flyable */
	aircraft->air_worthy_state = SAR_AIR_WORTHY_NOT_FLYABLE;

	/* Turn engine off */
	aircraft->engine_state = SAR_ENGINE_OFF;

	/* Set new direction */
	new_dir.heading = dir->heading;
	new_dir.pitch = (float)(0.0 * PI);
	new_dir.bank = (float)(1.7 * PI);   /* Tilt it */

	SARSimWarpObject(scene, obj_ptr, NULL, &new_dir);

	/* Turn off lights */
	SARSimOpLights(obj_ptr, False);
	/* Turn off spot lights */
	SARSimOpAttenuate(obj_ptr, False);
	/* Turn off strobes */
	SARSimOpStrobes(obj_ptr, False);
}


/*
 *      Sets the flyable object's values to the SFM's values.
 */
void SARSimSetSFMValues(
	sar_core_struct *core_ptr, sar_scene_struct *scene,
	sar_object_struct *obj_ptr
)
{
	int i;
	SFMModelStruct *fdm = NULL;
	sar_object_aircraft_struct *aircraft;
	sar_air_worthy_state air_worthy_state;
	sar_obj_part_struct *lgear_ptr = NULL;
	sar_obj_hoist_struct *hoist_ptr;
	sar_contact_bounds_struct *cb;
	const sar_option_struct *opt = &core_ptr->option;
	if((scene == NULL) || (obj_ptr == NULL))
	    return;

	cb = obj_ptr->contact_bounds;

	switch(obj_ptr->type)
	{
	  case SAR_OBJ_TYPE_GARBAGE:
	  case SAR_OBJ_TYPE_STATIC:
	  case SAR_OBJ_TYPE_AUTOMOBILE:
	  case SAR_OBJ_TYPE_WATERCRAFT:
	    break;

	  case SAR_OBJ_TYPE_AIRCRAFT:
	    aircraft = SAR_OBJ_GET_AIRCRAFT(obj_ptr);
	    if(aircraft == NULL)
		break;

	    fdm = aircraft->fdm;
	    if(fdm == NULL)
		break;
	 
	    air_worthy_state = aircraft->air_worthy_state;
	    hoist_ptr = SARObjGetHoistPtr(obj_ptr, 0, NULL);

	    /* Get pointer to first landing gear */
	    lgear_ptr = SARObjGetPartPtr(
		obj_ptr, SAR_OBJ_PART_TYPE_LANDING_GEAR, 0
	    );

	    /* No landing gears if not flyable */
	    if(air_worthy_state == SAR_AIR_WORTHY_NOT_FLYABLE)
		lgear_ptr = NULL;  


	    /* Begin setting SFM values from object */
#define TAR_PTR fdm
#define SRC_PTR aircraft

	    TAR_PTR->flags = (SFMFlagFlightModelType |
			      SFMFlagPosition | SFMFlagDirection |
			      SFMFlagVelocityVector | SFMFlagAirspeedVector |
			      SFMFlagSpeedStall | SFMFlagDragMin |
			      SFMFlagSpeedMax | SFMFlagAccelResponsiveness |
			      SFMFlagGroundElevation | SFMFlagServiceCeiling |
			      SFMFlagBellyHeight | SFMFlagGearState |
			      SFMFlagGearType | SFMFlagGearHeight |
			      SFMFlagGearBrakesState | SFMFlagGearTurnVelocityOptimul |
			      SFMFlagGearTurnVelocityMax | SFMFlagGearTurnRate |
			      SFMFlagLandedState | SFMFlagGroundContactType |
			      SFMFlagHeadingControlCoeff | SFMFlagBankControlCoeff |
			      SFMFlagPitchControlCoeff | SFMFlagThrottleCoeff |
			      SFMFlagAfterBurnerState | SFMFlagAfterBurnerPowerCoeff |
			      SFMFlagEnginePower | SFMFlagTotalMass |
			      SFMFlagAttitudeChangeRate | SFMFlagAttitudeLevelingRate |
			      SFMFlagAirBrakesState | SFMFlagAirBrakesArea |
			      SFMFlagCanCrashIntoOther | SFMFlagCanCauseCrash |
			      SFMFlagCrashContactShape | SFMFlagCrashableSizeRadius |
			      SFMFlagCrashableSizeZMin | SFMFlagCrashableSizeZMax |
			      SFMFlagTouchDownCrashResistance | SFMFlagCollisionCrashResistance |
			      SFMFlagStopped | SFMFlagLength | SFMFlagWingspan
		);

	    /* Update flight model type only if SFM not in slew mode */
	    switch(SRC_PTR->flight_model_type)
	    {
	      case SAR_FLIGHT_MODEL_SLEW:
		TAR_PTR->type = SFMFlightModelSlew;
		break;
	      case SAR_FLIGHT_MODEL_AIRPLANE:
		TAR_PTR->type = SFMFlightModelAirplane;
		break;
	      default:
		TAR_PTR->type = SFMFlightModelHelicopter;
		break;
	    }
	    TAR_PTR->speed_stall = SARSimStallSpeed(SRC_PTR);
	    TAR_PTR->drag_min = SRC_PTR->min_drag;
	    /* Calculate maximum speed, take drag into account */
	    TAR_PTR->speed_max =
		SRC_PTR->speed_max *
		(1.0 - SARSimHoistDragCoeff(scene, obj_ptr, hoist_ptr)) *
		(1.0 - SARSimLandingGearDragCoeff(scene, obj_ptr, lgear_ptr)) *
		(1.0 - SARSimFlapDragCoeff(scene, obj_ptr, SRC_PTR));
	    TAR_PTR->overspeed_expected = SRC_PTR->overspeed_expected;
	    TAR_PTR->overspeed = SRC_PTR->overspeed;
	    /* If not flyable, then reduce speed bounds */
	    if(air_worthy_state == SAR_AIR_WORTHY_NOT_FLYABLE)
	    {
		TAR_PTR->drag_min = TAR_PTR->drag_min * 5.0;
		TAR_PTR->speed_max = TAR_PTR->speed_max / 15.0;
	    }
	    /* Set acceleration responsiveness by flight model type */
	    switch(TAR_PTR->type)
	    {
	      case SFMFlightModelAirplane:
		TAR_PTR->accel_responsiveness.x =   
		    SRC_PTR->airplane_accel_responsiveness.x; 
		TAR_PTR->accel_responsiveness.y =
		    SRC_PTR->airplane_accel_responsiveness.y;
		TAR_PTR->accel_responsiveness.z =
		    SRC_PTR->airplane_accel_responsiveness.z;
		break;
	      default:  /* Assume SFMFlightModelHelicopter */ 
		TAR_PTR->accel_responsiveness.x =
		    SRC_PTR->accel_responsiveness.x;
		TAR_PTR->accel_responsiveness.y =
		    SRC_PTR->accel_responsiveness.y;   
		TAR_PTR->accel_responsiveness.z =
		    SRC_PTR->accel_responsiveness.z;
		break;
	    }
	    TAR_PTR->service_ceiling = SRC_PTR->service_ceiling;
	    TAR_PTR->belly_height = SRC_PTR->belly_height;
	    TAR_PTR->length = SRC_PTR->length;
	    TAR_PTR->wingspan = SRC_PTR->wingspan;
	    TAR_PTR->ground_elevation_msl = obj_ptr->ground_elevation_msl;
	    TAR_PTR->gear_state = (SFMBoolean)((lgear_ptr != NULL) ?
		(lgear_ptr->flags & SAR_OBJ_PART_FLAG_STATE) : False
	    );
	    if(lgear_ptr == NULL)
	    {
		/* Assume wheels */
		TAR_PTR->gear_type = SFMGearTypeWheels;
	    }
	    else
	    {
		if(lgear_ptr->flags & SAR_OBJ_PART_FLAG_LGEAR_SKI)
		    TAR_PTR->gear_type = SFMGearTypeSkis;
		else
		    TAR_PTR->gear_type = SFMGearTypeWheels;
	    }
	    TAR_PTR->gear_height = SRC_PTR->gear_height;
	    TAR_PTR->gear_brakes_state = ((SRC_PTR->wheel_brakes_state > 0) ?
		True : False
	    );
	    TAR_PTR->gear_brakes_coeff = SRC_PTR->wheel_brakes_coeff;
	    TAR_PTR->gear_turn_velocity_optimul = SRC_PTR->gturn_vel_opt;
	    TAR_PTR->gear_turn_velocity_max = SRC_PTR->gturn_vel_max;
/*            TAR_PTR->gear_turn_rate = SRC_PTR->; */
		
/*            TAR_PTR->ground_contact_type = SRC_PTR->; */
	    TAR_PTR->heading_control_coeff = 0.0;
	    TAR_PTR->bank_control_coeff = 0.0;
	    TAR_PTR->pitch_control_coeff = 0.0;
	    TAR_PTR->elevator_trim_coeff = 0.0;
	    TAR_PTR->throttle_coeff = 0.0;
	    TAR_PTR->after_burner_state = False;
	    TAR_PTR->after_burner_power_coeff = 0.0;
	    TAR_PTR->engine_power = SRC_PTR->engine_power;
	    TAR_PTR->total_mass = SRC_PTR->dry_mass + SRC_PTR->fuel;
	    /* Add up mass in existing external fuel tanks */
	    for(i = 0; i < SRC_PTR->total_external_fueltanks; i++)
	    {
		const sar_external_fueltank_struct *eft_ptr = SRC_PTR->external_fueltank[i];
		if((eft_ptr != NULL) ?
		    (eft_ptr->flags & SAR_EXTERNAL_FUELTANK_FLAG_ONBOARD) : 0
		)
		    TAR_PTR->total_mass += eft_ptr->dry_mass + eft_ptr->fuel;
	    }
	    /* Add up mass in the contents of the hoist */
	    if((hoist_ptr != NULL) ?
		(!hoist_ptr->on_ground && (hoist_ptr->rope_cur > 0.0f)) : 0
	    )
	    {
		TAR_PTR->total_mass += hoist_ptr->occupants_mass;
	    }
	    /* Add up mass of passengers (if any) */
	    TAR_PTR->total_mass += SRC_PTR->passengers_mass;

	    TAR_PTR->attitude_change_rate.heading =
		SRC_PTR->attitude_change_rate.heading;
	    TAR_PTR->attitude_change_rate.pitch =
		SRC_PTR->attitude_change_rate.pitch;
	    TAR_PTR->attitude_change_rate.bank =
		SRC_PTR->attitude_change_rate.bank;
	    TAR_PTR->attitude_leveling_rate.heading = 0.0;
	    TAR_PTR->attitude_leveling_rate.pitch = SRC_PTR->pitch_leveling;
	    TAR_PTR->attitude_leveling_rate.bank = SRC_PTR->bank_leveling;
	    TAR_PTR->air_brakes_state = (SRC_PTR->air_brakes_state > 0) ?
		True : False;
	    TAR_PTR->air_brakes_area = SRC_PTR->air_brakes_area;
	    /* We'll do all collision crash checks, so set 
	     * can_crash_into_other and can_cause_crash to False so
	     * FDM does not check for collisions.
	     */
	    TAR_PTR->can_crash_into_other = False;
	    TAR_PTR->can_cause_crash = False;
	    if(cb == NULL)
	    {
		/* Assume values for unavailable contact bounds values */
		TAR_PTR->crash_contact_shape =
		    SFMCrashContactShapeSpherical;
		TAR_PTR->crashable_size_radius = 1.0;
		TAR_PTR->crashable_size_z_min = -0.5;
		TAR_PTR->crashable_size_z_max = 0.5;
	    }
	    else
	    {
		switch(cb->contact_shape)
		{
		  case SAR_CONTACT_SHAPE_CYLENDRICAL:
		    TAR_PTR->crash_contact_shape = 
			SFMCrashContactShapeCylendrical;
		    break;

		  /* All else assume spherical */
		  case SAR_CONTACT_SHAPE_SPHERICAL:
		  case SAR_CONTACT_SHAPE_RECTANGULAR:
		    TAR_PTR->crash_contact_shape = 
			SFMCrashContactShapeSpherical;
		    break;
		}

		TAR_PTR->crashable_size_radius = cb->contact_radius;
		TAR_PTR->crashable_size_z_min = cb->contact_h_min;
		TAR_PTR->crashable_size_z_max = cb->contact_h_max;
	    }
	    TAR_PTR->touch_down_crash_resistance = 7.0 *
		MAX(opt->damage_resistance_coeff, 1.0f);
/* Need to work on this */
	    TAR_PTR->collision_crash_resistance = 2.0;

	    /* Do not allow attitude leveling if not flyable */
	    if(SRC_PTR->air_worthy_state != SAR_AIR_WORTHY_FLYABLE)
		TAR_PTR->flags &= ~(SFMFlagAttitudeLevelingRate);


#undef TAR_PTR
#undef SRC_PTR
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

	/* Was the SFM obtained from the above checks? */
	if(fdm != NULL)
	{
	    /* Set other common SFM values from the object's general
	     * structure here
	     */
#define SRC_PTR fdm
#define TAR_PTR obj_ptr



#undef TAR_PTR
#undef SRC_PTR
	}
}

/*
 *      Gets the flyable object's SFM values to the object
 */
void SARSimGetSFMValues(sar_scene_struct *scene, sar_object_struct *obj_ptr)
{
	SFMModelStruct *fdm = NULL;
	sar_object_aircraft_struct *aircraft = NULL;

	if((scene == NULL) || (obj_ptr == NULL))
	    return;

	/* Get FDM based on object type */
	switch(obj_ptr->type)
	{
	  case SAR_OBJ_TYPE_GARBAGE:
	  case SAR_OBJ_TYPE_STATIC:
	  case SAR_OBJ_TYPE_AUTOMOBILE:
	  case SAR_OBJ_TYPE_WATERCRAFT:
	    break;

	  case SAR_OBJ_TYPE_AIRCRAFT:
#define TAR_PTR	aircraft
#define SRC_PTR	fdm
	    TAR_PTR = SAR_OBJ_GET_AIRCRAFT(obj_ptr);
	    if(TAR_PTR == NULL)
		break;

	    SRC_PTR = TAR_PTR->fdm;
	    if(SRC_PTR == NULL)
		break;

	    TAR_PTR->center_to_ground_height =
		(float)SRC_PTR->center_to_ground_height;

	    /* Get z acceleration before updating velocity */
	    TAR_PTR->z_accel = (float)SRC_PTR->velocity_vector.z -
		TAR_PTR->vel.z;

	    TAR_PTR->vel.x = (float)SRC_PTR->velocity_vector.x;
	    TAR_PTR->vel.y = (float)SRC_PTR->velocity_vector.y;
	    TAR_PTR->vel.z = (float)SRC_PTR->velocity_vector.z;

	    TAR_PTR->airspeed.x = (float)SRC_PTR->airspeed_vector.x;
	    TAR_PTR->airspeed.y = (float)SRC_PTR->airspeed_vector.y;
	    TAR_PTR->airspeed.z = (float)SRC_PTR->airspeed_vector.z;
	    TAR_PTR->landed = SRC_PTR->landed_state;
#undef TAR_PTR
#undef SRC_PTR
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

	/* Got FDM? */
	if(fdm != NULL)
	{   
#define SRC_PTR fdm
#define TAR_PTR obj_ptr
	    /* Get common FDM values */
	    sar_contact_bounds_struct *cb = TAR_PTR->contact_bounds;

	    TAR_PTR->pos.x = (float)SRC_PTR->position.x;
	    TAR_PTR->pos.y = (float)SRC_PTR->position.y;
	    TAR_PTR->pos.z = (float)SRC_PTR->position.z;

	    TAR_PTR->dir.heading = (float)SRC_PTR->direction.heading;
	    TAR_PTR->dir.pitch = (float)SRC_PTR->direction.pitch;
	    TAR_PTR->dir.bank = (float)SRC_PTR->direction.bank;

	    /* If object is not flyable and on water (implying it has
	     * crashed into the water) we need to lower the position
	     * so that the object appears submersed in the water
	     */
	    if((aircraft != NULL) ?
		aircraft->on_water : False
	    )
	    {
		if(aircraft->air_worthy_state == SAR_AIR_WORTHY_NOT_FLYABLE)
		{
		    TAR_PTR->pos.z = TAR_PTR->ground_elevation_msl;
		}
	    }

	    /* Update contact bounds rotation if rectangular */
	    if((cb != NULL) ?
		(cb->contact_shape == SAR_CONTACT_SHAPE_RECTANGULAR) : False
	    )
	    {
		cb->cos_heading = (float)cos(-TAR_PTR->dir.heading);
		cb->sin_heading = (float)sin(-TAR_PTR->dir.heading);
	    }
#undef TAR_PTR
#undef SRC_PTR
	}
}


/*
 *      Applies natural forces to object.
 *
 *	Note that natural forces applied to flyable objects will be
 *	applied by the SFM and not here.
 *
 *	Returns 1 to indicate object has been deleted or memory change.
 *
 *	Other non-zero return indicates error.
 */
int SARSimApplyNaturalForce(
	sar_core_struct *core_ptr, sar_object_struct *obj_ptr
)
{
	int tar_obj_num;
	sar_direction_struct *dir;
	sar_position_struct *pos, *vel;
	sar_object_helipad_struct *helipad;
	sar_object_smoke_struct *smoke;
	sar_object_fire_struct *fire;
	sar_object_explosion_struct *explosion;
	sar_object_human_struct *human;
	sar_object_fueltank_struct *fueltank;
	sar_scene_struct *scene = core_ptr->scene;
	const sar_option_struct *opt = &core_ptr->option;
	if((scene == NULL) || (obj_ptr == NULL))
	    return(-1);

	/* Handle by object type */
	switch(obj_ptr->type)
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
	    helipad = SAR_OBJ_GET_HELIPAD(obj_ptr);
	    if(helipad == NULL)
		break;

	    /* Helipad follows a reference object? */
	    if(helipad->flags & SAR_HELIPAD_FLAG_FOLLOW_REF_OBJECT)
	    {
		sar_object_struct *ref_obj_ptr = SARObjGetPtr(
		    core_ptr->object, core_ptr->total_objects,
		    helipad->ref_object
		);
		if(ref_obj_ptr != NULL)
		{
		    /* Check the type of the reference object, only
		     * move to the reference object if it is a type of
		     * object that moves around (since the helipad is
		     * already moved to the correct reference position
		     * when it is loaded)
		     */
		    if((ref_obj_ptr->type == SAR_OBJ_TYPE_AUTOMOBILE) ||
		       (ref_obj_ptr->type == SAR_OBJ_TYPE_WATERCRAFT) ||
		       (ref_obj_ptr->type == SAR_OBJ_TYPE_AIRCRAFT) ||
		       (ref_obj_ptr->type == SAR_OBJ_TYPE_HUMAN)
		    )
		    {
			SARSimWarpObjectRelative(
			    scene, obj_ptr,
			    core_ptr->object, core_ptr->total_objects,
			    helipad->ref_object,
			    &helipad->ref_offset,
			    &helipad->ref_dir
			);
		    }
		}
	    }
	    break;

	  case SAR_OBJ_TYPE_SMOKE:
	    smoke = SAR_OBJ_GET_SMOKE(obj_ptr);
	    if(smoke == NULL)
		break;

	    pos = &obj_ptr->pos;

	    /* Any units on this smoke trails object? */
	    if(smoke->unit != NULL)
	    {
		int n, units_visible = 0, smoke_puff_hidden = 0;
		sar_object_smoke_unit_struct *u;

		/* Iterate through all smoke trail units, updating
		 * each one to reflect on the smoke trail type's
		 * behavour
		 */
		for(n = 0; n < smoke->total_units; n++)
		{
		    u = &smoke->unit[n];

		    /* Skip this unit if it is not visible */
		    if(u->visibility <= 0.0f)
			continue;

		    units_visible++;	/* Count this unit visible */

		    /* Change position and size of this unit */
		    u->pos.x += (u->vel.x *
			time_compensation * time_compression
		    );
		    u->pos.y += (u->vel.y *
			time_compensation * time_compression
		    );
		    u->pos.z += (u->vel.z *
			time_compensation * time_compression
		    );
  		    u->radius += smoke->radius_rate * time_compensation * time_compression;

		    /* Has the smoke unit's radius (size) grown too big? */
		    if(u->radius > smoke->radius_max)
		    {
			u->radius = smoke->radius_max;

			/* Should smoke puff disappear when it gets
			 * too big?
			 */
			if(smoke->hide_at_max)
			{
			    u->visibility = 0.0f;
			    smoke_puff_hidden++;
			    continue;
			}
		    }
		}
#if 0
		/* If one or more smoke units were hidden then the units
		 * need to be shifted so that the first contiguous units
		 * in the list are not hidden
		 */
		if(smoke_puff_hidden > 0)
		{
/* TODO This won't really affect anything if we don't have it, it just
 * makes smoke puffs draw in possible random order (which is still
 * unlikly since we time things well)
 */
		}
#endif

		/* If no units are visible and this smoke trail should
		 * be deleted when that happens then mark the life span
		 * to be 1 ms
		 */
		if((units_visible <= 0) &&
		   smoke->delete_when_no_units
		)
		{
		    obj_ptr->life_span = 1l;
		}
	    }

	    /* Get smoke trail's reference object, that's the object
	     * it is following
	     */
	    tar_obj_num = smoke->ref_object;
	    if(SARObjIsAllocated(
		core_ptr->object, core_ptr->total_objects, tar_obj_num
	    ))
	    {
		sar_object_struct *tar_obj_ptr = core_ptr->object[tar_obj_num];

		/* Move smoke trail object to position of reference object */
		memcpy(pos, &tar_obj_ptr->pos, sizeof(sar_position_struct));

		/* Realize changes in position */
		SARSimWarpObject(
		    scene, obj_ptr,
		    pos, NULL
		);
	    }

	    /* Time to spawn a new smoke unit and should we respawn? */
	    if((smoke->respawn_next <= cur_millitime) &&
	       (smoke->respawn_int > 0l)
	    )
	    {
		SARSimSmokeSpawn(core_ptr, obj_ptr, smoke);

		/* Schedual next respawn */
		smoke->respawn_next = cur_millitime +
		    smoke->respawn_int;
	    }
	    break;

	  case SAR_OBJ_TYPE_FIRE:
	    fire = SAR_OBJ_GET_FIRE(obj_ptr);
	    if(fire == NULL)
		break;

	    pos = &obj_ptr->pos;
            /* Keep fire above ground level */
            if(pos->z < obj_ptr->ground_elevation_msl)
                 pos->z = obj_ptr->ground_elevation_msl;

             /* Realize changes in position */
             SARSimWarpObject(
                    scene, obj_ptr,
                    pos, NULL);


	    /* Time to increment explosion animation frame? */
	    if(fire->next_frame_inc <= cur_millitime)
	    {
		int tex_num = fire->tex_num;

		/* Set next time to increment animation frame */
		fire->next_frame_inc = cur_millitime +
		    fire->frame_inc_int;

		/* Increment current animation frame */
		fire->cur_frame++;

		/* Is fire object's texture structure available? */   
		if(SARIsTextureAllocated(scene, tex_num))
		{
		    v3d_texture_ref_struct *t = scene->texture_ref[tex_num];

		    /* Animation frames cycled? */
		    if(fire->cur_frame >= t->total_frames)
		    {
			fire->cur_frame = 0;
			fire->frame_repeats++;
	    
			/* Total animation frame cycle repeats exceeded? */
			if((fire->frame_repeats >=
			    fire->total_frame_repeats) &&
			   (fire->total_frame_repeats > 0)
			)
			{   
			    /* Mark this object to be deleted by setting
			     * its life span to 1 ms which will cause it to
			     * be deleted
			     */
			    obj_ptr->life_span = 1l;

			    /* Set current frame to be the last frame */
			    fire->cur_frame = t->total_frames - 1;
			}
		    }
		    /* Sanitize current animation frame */
		    if(fire->cur_frame < 0)
			fire->cur_frame = 0;
		}
	    }
	    break;

	  case SAR_OBJ_TYPE_EXPLOSION:
	    explosion = SAR_OBJ_GET_EXPLOSION(obj_ptr);
	    if(explosion == NULL)
		break;

	    pos = &obj_ptr->pos;

	    /* Time to increment explosion animation frame? */
	    if(explosion->next_frame_inc <= cur_millitime)
	    {
		int tex_num = explosion->tex_num;

		/* Set next time to increment animation frame */
		explosion->next_frame_inc = cur_millitime +
		    explosion->frame_inc_int;

		/* Increment current animation frame */
		explosion->cur_frame++;

		/* Is explosion object's texture structure available? */
		if(SARIsTextureAllocated(scene, tex_num))
		{
		    v3d_texture_ref_struct *t = scene->texture_ref[tex_num];

		    /* Animation frames cycled? */
		    if(explosion->cur_frame >= t->total_frames)
		    {
			explosion->cur_frame = 0;
			explosion->frame_repeats++;

			/* Total animation frame cycle repeats exceeded? */
			if((explosion->frame_repeats >=
			    explosion->total_frame_repeats) &&
			   (explosion->total_frame_repeats > 0)
			)
			{
			    /* Mark this object to be deleted by setting
			     * its life span to 1 ms which will cause it to
			     * be deleted
			     */
			    obj_ptr->life_span = 1l;

			    /* Set current frame to be the last frame */
			    explosion->cur_frame = t->total_frames - 1;
			}
		    }
		    /* Sanitize current animation frame */
		    if(explosion->cur_frame < 0)
			explosion->cur_frame = 0;
		}
	    }

	    /* Get explosion's reference object, that's the object it
	     * is following
	     */
	    tar_obj_num = explosion->ref_object;
	    if(SARObjIsAllocated(
		core_ptr->object, core_ptr->total_objects,
		tar_obj_num
	    ))
	    {
		float ground_elevation_msl;
		sar_object_struct *tar_obj_ptr = core_ptr->object[tar_obj_num];

		/* Move explosion object's position to the position of
		 * the referenced object.
		 */
		memcpy(pos, &tar_obj_ptr->pos, sizeof(sar_position_struct));

		/* Get ref object's ground elevation or its z position
		 * (whichever is greater).
		 */
		ground_elevation_msl = MAX(
		    tar_obj_ptr->ground_elevation_msl,
		    tar_obj_ptr->pos.z
		);

		/* Keep above ground level by gravity offset */
		switch(explosion->center_offset)
		{
		  case SAR_EXPLOSION_CENTER_OFFSET_BASE:
		    /* Need to move the position up */
		    pos->z += explosion->radius;
		    /* Sanitize z position to base */
		    if(pos->z < (ground_elevation_msl +
			explosion->radius)
		    )
			pos->z = ground_elevation_msl +
			    explosion->radius;
		    break;
		  case SAR_EXPLOSION_CENTER_OFFSET_NONE:
		    /* Sanitize z position to center */
		    if(pos->z < ground_elevation_msl)
			pos->z = ground_elevation_msl;
		    break;
		}
		/* Realize changes in position */
		SARSimWarpObject(
		    scene, obj_ptr,
		    pos, NULL
		);
	    }
	    break;

	  case SAR_OBJ_TYPE_CHEMICAL_SPRAY:
	    break;

	  case SAR_OBJ_TYPE_HUMAN:
	    human = SAR_OBJ_GET_HUMAN(obj_ptr);
	    if(human == NULL)
		break;

	    pos = &obj_ptr->pos;

	    /* Move human only if it is not being moved by something else
	     * we check the human's SAR_HUMAN_FLAG_GRIPPED flag. If it
	     * is set then we do not apply any forces to the human.
	     */
	    if(human->flags & SAR_HUMAN_FLAG_GRIPPED)
	    {
		/* Human is being moved by another object, possibly the
		 * it is in a rescue basket. In any case we do not apply
		 * any forces to the human object.
		 */
	    }
	    else
	    {
		/* Pull human down with gravity (use constant 3 meters per
		 * second velocity for now).
		 *
		 * We must keep the rate less than 3.0 since contact
		 * checks to keep human above objects like buildings
		 * (because buildings have hollow contact bounds, but
		 * heightfields are solid contact bounds).
		 */
		pos->z += (float)MAX(
		    (SAR_DEF_HUMAN_FALL_RATE * time_compensation * time_compression),
		    SAR_DEF_HUMAN_FALL_RATE
		);

		/* Keep human above ground level */
		if(pos->z < obj_ptr->ground_elevation_msl)
		    pos->z = obj_ptr->ground_elevation_msl;

		/* Realize changes in position */
		SARSimWarpObject(
		    scene, obj_ptr,
		    pos, NULL
		);
	    }
	    break;

	  case SAR_OBJ_TYPE_FUELTANK:
	    fueltank = SAR_OBJ_GET_FUELTANK(obj_ptr);
	    if(fueltank == NULL)
		break;

	    dir = &obj_ptr->dir;
	    pos = &obj_ptr->pos;

	    /* Landed on ground yet? */
	    if(fueltank->flags & SAR_FUELTANK_FLAG_ON_GROUND)
	    {

	    }
	    else
	    {
		float	cos_heading = (float)cos(dir->heading),
			sin_heading = (float)sin(dir->heading);

		vel = &fueltank->vel;

		vel->z += (float)-(
		    SAR_GRAVITY * time_compensation * time_compression
		);
		if(vel->z < fueltank->vel_z_max)
		    vel->z = fueltank->vel_z_max;

		pos->x += (((sin_heading * vel->y) +
		    (cos_heading * vel->x)) *
		    time_compensation * time_compression
		);
		pos->y += (((cos_heading * vel->y) -
		    (sin_heading * vel->x)) *
		    time_compensation * time_compression
		);
		pos->z += vel->z * time_compensation * time_compression;

		/* Landed? */
		if((pos->z - fueltank->belly_to_center_height) <
		    obj_ptr->ground_elevation_msl
		)
		{
		    Boolean over_water;
		    int obj_num;
		    float contact_radius = MAX(
			SARSimGetFlatContactRadius(obj_ptr), 1.0f
		    );

		    /* Get object number */
		    for(obj_num = 0; obj_num < core_ptr->total_objects; obj_num++)
		    {
			if(obj_ptr == core_ptr->object[obj_num])
			    break;
		    }
		    if(obj_num >= core_ptr->total_objects)
			obj_num = -1;

		    /* Keep position above ground level */
		    pos->z = obj_ptr->ground_elevation_msl +
			fueltank->belly_to_center_height;

		    /* Mark as landed */
		    fueltank->flags |= SAR_FUELTANK_FLAG_ON_GROUND;

		    /* Set lifespan so it gets removed after a while */
		    obj_ptr->life_span = cur_millitime +
			opt->fuel_tank_life_span;

		    /* Landed on water? */
		    SARGetGHCOverWater(
			core_ptr, scene,
			&core_ptr->object, &core_ptr->total_objects,
			obj_num,
			NULL, &over_water
		    );
		    if(over_water)
		    {
			/* Sink down a little so that center is at the
			 * water level
			 */
			pos->z = obj_ptr->ground_elevation_msl;

			/* Landed on water, create splash */
			SplashCreate(
			    core_ptr, scene,
			    &core_ptr->object, &core_ptr->total_objects,
			    pos,
			    1.5f * contact_radius,	/* Radius size in meters */
			    obj_num,			/* Reference object */
			    SAR_STD_TEXNAME_SPLASH, SAR_STD_TEXNAME_SPLASH
			);
			if(opt->event_sounds)
			    SARSoundSourcePlayFromList(
				core_ptr->recorder,
				scene->sndsrc, scene->total_sndsrcs,
				"splash_human",
				pos, dir, &scene->ear_pos
			    );
		    }
		    else
		    {
			/* Landed on ground, create a smoke puff */
			SmokeCreate(
			    scene, &core_ptr->object, &core_ptr->total_objects,
			    SAR_SMOKE_TYPE_SMOKE,
			    pos, NULL,
			    0.5f * contact_radius,	/* Radius min */
			    2.0f * contact_radius,	/* Radius max */
			    -1.0f,		/* Autocalc growth */
			    1,			/* Hide at max */
			    1,			/* Total units */
			    3000,		/* Respawn interval in ms */
			    SAR_STD_TEXNAME_SMOKE_LIGHT,
			    -1,			/* No reference object */
			    cur_millitime + 3000
			);
			if(opt->event_sounds)
			    SARSoundSourcePlayFromList(
				core_ptr->recorder,
				scene->sndsrc, scene->total_sndsrcs,
				"thud_medium",
				pos, dir, &scene->ear_pos
			    );
		    }
		}
		/* Realize changes in position */
		SARSimWarpObject(
		    scene, obj_ptr,
		    pos, NULL
		);
	    }
	    break;

	  case SAR_OBJ_TYPE_PREMODELED:
	    break;
	}
	    
	return(0);
}



/*
 *      Called once per cycle for each object to apply artificial
 *      forces to the object.
 *
 *      Can return 1 to indicate object has been deleted or memory
 *	change.
 *
 *	Other non-zero return indicates error.
 */
int SARSimApplyArtificialForce(
	sar_core_struct *core_ptr, sar_object_struct *obj_ptr
)
{
	int i, air_worthy_state;
	float thrust_output = 0.0f, throttle_output = 0.0f;
	float ground_elevation, cen_to_gear_height;
	char ear_in_cockpit, is_player;
	sar_direction_struct *dir;
	sar_position_struct *pos, *vel;
	sar_object_aircraft_struct *aircraft = NULL;
	sar_object_human_struct *human;
	sar_obj_rotor_struct *rotor_ptr;
	sar_obj_part_struct *gear;
	sar_obj_hoist_struct *hoist;
	sar_contact_bounds_struct *cb;
	sar_scene_struct *scene = core_ptr->scene;
	const sar_option_struct *opt = &core_ptr->option;
	if((scene == NULL) || (obj_ptr == NULL))
	    return(-1);

	/* Check if the given object is the player object */
	if(scene->player_obj_ptr == obj_ptr)
	{
	    is_player = 1;
	    ear_in_cockpit = SAR_IS_EAR_IN_COCKPIT(scene);
	}
	else
	{
	    is_player = 0;
	    ear_in_cockpit = 0;
	}


	pos = &obj_ptr->pos;
	dir = &obj_ptr->dir;

	/* Get contact bounds (may be NULL) */
	cb = obj_ptr->contact_bounds;


	/* Begin calculating vertical distance from 0.0 to object for
	 * object types that need to land or walk on surfaces (humans,
	 * aircrafts, and fueltanks)
	 *
	 * Set starting elevation to 0.0 (not scene's msl elevation,
	 * that is only added when displaying current MSL altitudes)
	 */
	ground_elevation = 0.0f;

	/* Check objects of types that need to know about ground
	 * elevation.
	 */
	if((obj_ptr->type == SAR_OBJ_TYPE_AIRCRAFT) ||
	   (obj_ptr->type == SAR_OBJ_TYPE_HUMAN) ||
	   (obj_ptr->type == SAR_OBJ_TYPE_FUELTANK)
	)
	{
	    /* We need to check if this object has contact bounds that
	     * specify SAR_CRASH_FLAG_SUPPORT_SURFACE, because if it
	     * does then checking for surface at the position of this
	     * object may hit itself as a surface and cause recursion
	     */
	    if(!(((cb != NULL) ? cb->crash_flags : 0) &
		SAR_CRASH_FLAG_SUPPORT_SURFACE)
	    )
	    {
		/* This object does not specify countact bounds for
		 * SAR_CRASH_FLAG_SUPPORT_SURFACE
		 */

		/* Check for other objects that are "under" this object
		 * for increased ground elevation, this checks for all
		 * all objects that have either a contact bounds set for
		 * SAR_CRASH_FLAG_SUPPORT_SURFACE and objects of type
		 * SAR_OBJ_TYPE_GROUND
		 *
		 * Only SAR_OBJ_TYPE_GROUND objects will be treated as
		 * "solid" ground while objects with contact bounds will
		 * be "hollow".
		 */
		ground_elevation += SARSimFindGround(
		    scene,
		    core_ptr->object, core_ptr->total_objects,
		    pos			/* Position of our object */
		);
	    }
	}
	/* Set new ground elevation value on the object */
	obj_ptr->ground_elevation_msl = ground_elevation;


	/* Handle by object type */
	switch(obj_ptr->type)
	{
	  case SAR_OBJ_TYPE_GARBAGE:
	  case SAR_OBJ_TYPE_STATIC:
	  case SAR_OBJ_TYPE_AUTOMOBILE:
	  case SAR_OBJ_TYPE_WATERCRAFT:
	    break;

	  case SAR_OBJ_TYPE_AIRCRAFT:
	    aircraft = SAR_OBJ_GET_AIRCRAFT(obj_ptr);
	    if(aircraft == NULL)
		break;

	    vel = &aircraft->vel;
	    air_worthy_state = aircraft->air_worthy_state;

	    /* Get pointer to first landing gear */
	    gear = SARObjGetPartPtr(
		obj_ptr, SAR_OBJ_PART_TYPE_LANDING_GEAR, 0
	    );

	    /* Calculate center of object to lower of landing gear height */
	    cen_to_gear_height = aircraft->belly_height;
	    if(gear != NULL)
		cen_to_gear_height += (float)((gear->flags & SAR_OBJ_PART_FLAG_STATE) ?
		    aircraft->gear_height : 0.0
		);

	    /* Calculate throttle output */
	    throttle_output = SARSimThrottleOutputCoeff(
		(aircraft->flight_model_type != SAR_FLIGHT_MODEL_SLEW) ?
		    aircraft->flight_model_type :
		    aircraft->last_flight_model_type
		,
		aircraft->throttle,
		aircraft->collective,
		aircraft->collective_range
	    );

	    /* Any fuel left? */
	    if(aircraft->fuel > 0.0f)
	    {
		/* Handle engine initialization */
		if(aircraft->engine_state == SAR_ENGINE_INIT)
		{
		    /* Time to initialize? */
		    if(aircraft->next_engine_on <= cur_millitime)
		    {
			aircraft->engine_state = SAR_ENGINE_ON;
			aircraft->next_engine_on = 0l;
		    }
		}
	  
		/* Calculate thrust output and consume fuel if engine
		 * is on
		 */
		if((aircraft->service_ceiling > 0.0f) &&
		   (aircraft->engine_state == SAR_ENGINE_ON)
		)   
		{
		    /* Has fuel left, consume based on the fuel rate
		     * and the actual throttle output coeff
		     */
		    aircraft->fuel -= aircraft->fuel_rate *
			throttle_output	* time_compensation * time_compression;
		    if(aircraft->fuel <= 0.0f)   
		    {
			aircraft->fuel = 0.0f;
			aircraft->engine_state = SAR_ENGINE_OFF;
		    }
		    else
		    {
			/* Calculate thrust output, the closer the
			 * object is to its service ceiling the less
			 * thrust output will be
			 */
			thrust_output = (float)(throttle_output *
			    aircraft->engine_power *
			    MAX(1.0 - (pos->z /
				aircraft->service_ceiling),
				0.0
			    )
			);
			if(thrust_output < 0.0f)
			    thrust_output = 0.0f;
		    }
		}
	    }
	    else if(aircraft->engine_state == SAR_ENGINE_ON)
	    {
		/* No fuel and engine state still on, in which case turn
		 * engine off
		 */
		aircraft->engine_state = SAR_ENGINE_OFF;
	    }


	    /* Spin rotor(s) based on actual throttle and engine state,
	     * and update rotor pitch animation values
	     */
	    for(i = 0; i < aircraft->total_rotors; i++)
	    {
		rotor_ptr = aircraft->rotor[i];
		if(rotor_ptr == NULL)
		    continue;

		/* Spin rotor only if engine or throttle is on */
		if((aircraft->engine_state == SAR_ENGINE_ON) ||
		   (aircraft->throttle > 0.0f)
		)
		{
		    rotor_ptr->anim_pos += (int)MAX(
			(float)(aircraft->throttle + 0.2) *
			(float)((int)((sar_grad_anim_t)-1) * 1.40) *
			time_compensation * time_compression,
			1
		    );
		    rotor_ptr->rotor_wash_anim_pos += (int)MAX(
			(float)(aircraft->throttle + 0.2) *
			(float)((int)((sar_grad_anim_t)-1) * 0.50) *
			time_compensation * time_compression,
			1
		    );
		}

		if(rotor_ptr->flags & SAR_ROTOR_FLAG_CAN_PITCH)
		{
		    /* Pitched down? */
		    if((rotor_ptr->flags & SAR_ROTOR_FLAG_PITCH_STATE) &&
		       (rotor_ptr->pitch_anim_pos < (sar_grad_anim_t)-1)
		    )
		    {
			rotor_ptr->pitch_anim_pos = (sar_grad_anim_t)MIN(
			    rotor_ptr->pitch_anim_pos +
				(10000 * time_compensation * time_compression),
			    (sar_grad_anim_t)-1
			);
		    }
		    /* Pitched up? */
		    else if(!(rotor_ptr->flags & SAR_ROTOR_FLAG_PITCH_STATE) &&
			     (rotor_ptr->pitch_anim_pos > 0)
		    )
		    {
			rotor_ptr->pitch_anim_pos = (sar_grad_anim_t)MAX(
			    rotor_ptr->pitch_anim_pos -
				(10000 * time_compensation * time_compression),
			    0
			);
		    }
		}
	    }

	    /* Update animation position for all object parts */
	    for(i = 0; i < aircraft->total_parts; i++)
		SARSimUpdatePart(
		    scene,
		    obj_ptr, aircraft->part[i],
		    core_ptr->recorder, opt->event_sounds, ear_in_cockpit
		);


	    /* Update hoist and its occupants (if rope is out) */
	    hoist = SARObjGetHoistPtr(obj_ptr, 0, NULL);
	    if((hoist != NULL) ? (hoist->rope_cur > 0.0f) : 0)
	    {
		double a[3 * 1], r[3 * 1];
		sar_position_struct *basket_pos = &hoist->pos;
		int tar_obj_num;
		sar_object_struct *tar_obj_ptr;


		/* Get matrix a as hoist offset translation */
		a[0] = hoist->offset.x;
		a[1] = hoist->offset.y;
		a[2] = hoist->offset.z;

		/* Rotate matrix a into r */
		MatrixRotateBank3(a, -dir->bank, r);	/* Our bank is negative,
							 * so pass as flipped sign
							 */
		MatrixRotatePitch3(r, dir->pitch, a);
		MatrixRotateHeading3(a, dir->heading, r);

		/* Calculate position of hoist's rescue basket in world
		 * coordinates by adding rorated offset relative to
		 * object's center
		 */
		basket_pos->x = (float)(pos->x + r[0]);
		basket_pos->y = (float)(pos->y + r[1]);
		basket_pos->z = (float)(pos->z + r[2]);

		/* Set basket position to be at end of rope length */
		basket_pos->z -= hoist->rope_cur;

		/* Update visual length of rope */
		hoist->rope_cur_vis = hoist->rope_cur;

		/* Check if basket is `close enough' to be contacting
		 * the ground
		 */
		if(basket_pos->z <= (ground_elevation + 0.01))
		    hoist->on_ground = 1;
		else
		    hoist->on_ground = 0;

		/* Is rope out so much that basket is embedded into
		 * the ground?
		 */
		if(basket_pos->z < ground_elevation)
		{
		    /* Reduce current visual extension of rope */
		    hoist->rope_cur_vis = (float)MAX(
			hoist->rope_cur -
			ground_elevation + basket_pos->z,
			0.0
		    );

		    /* Sanitize basket z position */
		    basket_pos->z = ground_elevation;
		}

		/* Adjust heading of hoist basket to match that of
		 * the object it is connected to
		 */
		memcpy(
		    &hoist->dir,
		    dir,
		    sizeof(sar_direction_struct)
		);


		/* Move occupants in rescue basket to the new position
		 * of the rescue basket
		 */
		for(i = 0; i < hoist->total_occupants; i++)
		{
		    tar_obj_num = hoist->occupant[i];
		    if(SARObjIsAllocated(
			core_ptr->object, core_ptr->total_objects,
			tar_obj_num
		    ))   
			tar_obj_ptr = core_ptr->object[tar_obj_num];
		    else
			continue;

		    /* Move passenger along with rescue basket */
		    memcpy(
			&tar_obj_ptr->pos,
			basket_pos,
			sizeof(sar_position_struct)
		    );
		    /* Offset passenger position by current deployment */
		    switch(hoist->cur_deployment)
		    {
		      case SAR_HOIST_DEPLOYMENT_BASKET:
			/* Move passenger up since basket is above the
			 * water
			 */
			tar_obj_ptr->pos.z += 0.2f;
			break;
		      case SAR_HOIST_DEPLOYMENT_DIVER:
			/* Move passenger down a bit since diver is
			 * waist deep offset into the water
			 */
			tar_obj_ptr->pos.z -= 1.0;
			break;
		      case SAR_HOIST_DEPLOYMENT_HOOK:
			tar_obj_ptr->pos.z -= 1.0;
			break;
		    }

		    /* Direction of occupant is to match direction of
		     * hoist rescue basket
		     */
		    memcpy(
			&tar_obj_ptr->dir,
			&hoist->dir,
			sizeof(sar_direction_struct)
		    );

		    /* Formally realize changes to occupant object */
		    SARSimWarpObject(
			scene, tar_obj_ptr,
			&tar_obj_ptr->pos, &tar_obj_ptr->dir
		    );
		}

		/* Update animation position */
		hoist->anim_pos += (sar_grad_anim_t)(
		    hoist->anim_rate * time_compensation * time_compression
		);

	    }	/* Update hoist */

	    /* Check if any passengers are pending to leave */
	    if(aircraft->passengers_leave_pending > 0)
	    {
		sar_obj_part_struct *door_ptr = SARObjGetPartPtr(
		    obj_ptr, SAR_OBJ_PART_TYPE_DOOR_RESCUE, 0
		);

		/* Check if door is opened all the way and the
		 * aircraft is safely landed?
		 */
		if(aircraft->landed &&
		   (door_ptr == NULL) ?
			1 : ((door_ptr->anim_pos == (sar_grad_anim_t)-1) &&
			     (door_ptr->flags & SAR_OBJ_PART_FLAG_STATE))
		)
		{
		    /* Landed and door fully opened */

/* TODO
 *
 * The door needs to be checked if its fully opened and then
 * remove the passengers by creating human objects at the
 * door position
 *
 * The mission callback for humans leaving aircraft needs to
 * be called as well
 */



		}
	    }
	    break;

	  case SAR_OBJ_TYPE_GROUND:
	  case SAR_OBJ_TYPE_RUNWAY:
	  case SAR_OBJ_TYPE_HELIPAD:
	    break;

	  case SAR_OBJ_TYPE_HUMAN:
	    human = SAR_OBJ_GET_HUMAN(obj_ptr);
	    if(human == NULL)
		break;

	    /* Update animation */
	    human->anim_pos += (sar_grad_anim_t)(
		(float)human->anim_rate *
		time_compensation * time_compression   
	    );

	    /* Move human only if it is not being moved by something else
	     * we check the human's SAR_HUMAN_FLAG_GRIPPED flag
	     *
	     * If it is set then we do not apply any forces to the
	     * human
	     */
	    if(human->flags & SAR_HUMAN_FLAG_GRIPPED)
	    {
		/* Human is being moved by another object, possibly
		 * the it is in a rescue basket
		 *
		 * In any case we do not apply any forces to the human
		 * object
		 */
	    }
	    else
	    {
		/* Check if human is running towards or away from an
		 * object
		 */
		int ref_obj_num = human->intercepting_object;
		sar_object_struct *ref_obj_ptr = NULL;

		/* Check cases where intercepting object number would be
		 * a special value
		 */
		switch(ref_obj_num)
		{
		  case -2:	/* Player object */
		    ref_obj_num = scene->player_obj_num;
		    break;
		}

		/* Match intercept object */
		ref_obj_ptr = SARObjGetPtr(
		    core_ptr->object, core_ptr->total_objects,
		    ref_obj_num
		);

		/* Got an intercepting object and it is not the same as
		 * the given object?
		 */
		if((ref_obj_ptr != NULL) &&
		   (ref_obj_ptr != obj_ptr)
		)
		{
		    sar_position_struct *ref_pos = &ref_obj_ptr->pos, delta;
		    sar_direction_struct *ref_dir = &ref_obj_ptr->dir;
		    float ref_distance2d, ref_distance3d;

		    /* Calculate general deltas from human to reference
		     * object, a more specific deltas calculation will
		     * be performed later if possible otherwise it falls
		     * back to this calculation
		     */
		    delta.x = ref_pos->x - pos->x;
		    delta.y = ref_pos->y - pos->y;
		    delta.z = ref_pos->z - pos->z;

		    /* Run towards? */
		    if(human->flags & SAR_HUMAN_FLAG_RUN_TOWARDS)
		    {
			sar_obj_part_struct *door_ptr = NULL;
			int passengers = 0, passengers_max = 0;

			/* Reset human run flag (it will be updated below) */
			human->flags &= ~SAR_HUMAN_FLAG_RUN;

			/* Handle movement by the reference object type */
			switch(ref_obj_ptr->type)
			{
			  case SAR_OBJ_TYPE_GARBAGE:
			  case SAR_OBJ_TYPE_STATIC:
			  case SAR_OBJ_TYPE_AUTOMOBILE:
			  case SAR_OBJ_TYPE_WATERCRAFT:
			    break;

			  case SAR_OBJ_TYPE_AIRCRAFT:
			    aircraft = SAR_OBJ_GET_AIRCRAFT(ref_obj_ptr);
			    /* Aircraft landed and is airworthy? */
			    if((aircraft != NULL) ?
				(aircraft->landed &&
				 (aircraft->air_worthy_state == SAR_AIR_WORTHY_FLYABLE)) : 0
			    )
			    {
				door_ptr = SARObjGetPartPtr(
				    ref_obj_ptr, SAR_OBJ_PART_TYPE_DOOR_RESCUE, 0
				);
				passengers = aircraft->passengers;
				passengers_max = aircraft->passengers_max;
			    }
			    break;

			  case SAR_OBJ_TYPE_GROUND:
			  case SAR_OBJ_TYPE_RUNWAY:
			  case SAR_OBJ_TYPE_HELIPAD:
			    break;

			  case SAR_OBJ_TYPE_HUMAN:
			    /* TODO: Another human */
			    break;

			  case SAR_OBJ_TYPE_SMOKE:
			  case SAR_OBJ_TYPE_FIRE:
			  case SAR_OBJ_TYPE_EXPLOSION:
			  case SAR_OBJ_TYPE_CHEMICAL_SPRAY:
			  case SAR_OBJ_TYPE_FUELTANK:
			  case SAR_OBJ_TYPE_PREMODELED:
			    break;
			}

			/* Door exists on the reference object to
			 * calculate accurate delta position?
			 */
			if(door_ptr != NULL)
			{
			    /* Calculate door's world position */
			    sar_position_struct *offset = &door_ptr->pos_cen;
			    sar_position_struct thres_pos;
			    double a[3 * 1], r[3 * 1];

			    /* Taking the reference object's rotation
			     * and offseting from the reference object's
			     * world position
			     */
			    a[0] = offset->x;
			    a[1] = offset->y;
			    a[2] = offset->z;
			    /* Rotate matrix a into r */
			    MatrixRotateBank3(a, -ref_dir->bank, r);	/* Our bank is negative,
									 * so pass as flipped sign.
									 */
			    MatrixRotatePitch3(r, ref_dir->pitch, a);
			    MatrixRotateHeading3(a, ref_dir->heading, r);
			    /* Calculate door's threshold world position */
			    thres_pos.x = (float)(ref_pos->x + r[0]);
			    thres_pos.y = (float)(ref_pos->y + r[1]);
			    thres_pos.z = (float)(ref_pos->z + r[2]);
			    /* Update delta from human to reference object */
			    delta.x = thres_pos.x - pos->x;
			    delta.y = thres_pos.y - pos->y;
			    delta.z = thres_pos.z - pos->z;
			}

			/* Calculate 2d and 3d distances to reference object */
			human->intercepting_object_distance2d =
			    ref_distance2d = (float)SFMHypot2(delta.x, delta.y);
			human->intercepting_object_distance3d =
			    ref_distance3d = (float)SFMHypot2(ref_distance2d, delta.z);

			/* Close enough for human to enter reference
			 * object and is there enough room for one more
			 * passenger
			 */
			if((ref_distance2d < 0.5f) && (passengers < passengers_max))
			{
			    /* Door exists and close enough in 3d distance? */
			    if((door_ptr != NULL) && (ref_distance3d < 4.0f))
			    {
				/* Door not fully opened? */
				if(door_ptr->anim_pos != (sar_grad_anim_t)(-1))
				{
				    /* Door marked opened? */
				    if(door_ptr->flags & SAR_OBJ_PART_FLAG_STATE)
				    {
					/* Wait patiently for door to open */
				    }
				    else
				    {
					/* Open the door then */
					SARSimOpDoorRescue(
					    scene, &core_ptr->object, &core_ptr->total_objects,
					    ref_obj_ptr, 1
					);
				    }
				}
				else
				{
				    /* Door fully opened and in range, enter! */
				    int this_obj_num = SARGetObjectNumberFromPointer(
					scene, core_ptr->object, core_ptr->total_objects,
					obj_ptr
				    );

				    /* Begin closing door on reference object
				     * if the door is not to remain opened.
				     */
				    if(!(door_ptr->flags & SAR_OBJ_PART_FLAG_DOOR_STAY_OPEN))
				        SARSimOpDoorRescue(
					    scene, &core_ptr->object, &core_ptr->total_objects,
					    ref_obj_ptr, 0
				        );

				    /* Print enter message */
				    if((scene->player_obj_ptr == ref_obj_ptr) &&
				       (human->mesg_enter != NULL)
				    )
					SARMessageAdd(scene, human->mesg_enter);

				    /* Move our object into the reference object if
				     * possible and update the reference object's
				     * passengers count.
				     */
				    SARSimBoardObject(
					core_ptr, ref_obj_ptr, this_obj_num
				    );

				    /* Given human object no longer exists, so
				     * return marking that the object is gone.
				     */
				    return(1);
				}
			    }
			}
			/* Close enough to run towards (100m), there is a
			 * door (implying reference object is landed),
			 * and room for one more passenger, and 
                         * the aircraft's engine is nearly stopped
			 */
			else if((ref_distance2d < 100.0f) &&
				(door_ptr != NULL) &&
				(passengers < passengers_max) &&
                                (aircraft->throttle < 0.3f)
			)
			{
			    /* Set run flag on human */
			    human->flags |= SAR_HUMAN_FLAG_RUN;

			    pos->x += (float)(
				sin(dir->heading) * 2.5 *
				time_compensation * time_compression
			    );
			    pos->y += (float)(
				cos(dir->heading) * 2.5 *
				time_compensation * time_compression
			    );
			}

			/* Set new direction of human object */
			dir->heading = (float)SFMSanitizeRadians(
			    (0.5 * PI) - atan2(delta.y, delta.x)
			);
			dir->pitch = (float)(0.0 * PI);
			dir->bank = (float)(0.0 * PI);

			/* Realize new position and direction of human */
			SARSimWarpObject(scene, obj_ptr, pos, dir);
		    }
		    /* Run away? */
		    else if(human->flags & SAR_HUMAN_FLAG_RUN_AWAY)
		    {
			/* Reset human run flag (it will be updated below) */
			human->flags &= ~SAR_HUMAN_FLAG_RUN;

/* Need to work on this */
			if(0)
			{
			    human->flags |= SAR_HUMAN_FLAG_RUN;

			    pos->x += (float)(
				sin(dir->heading) * 1.0 *
				time_compensation * time_compression
			    );
			    pos->y += (float)(
				cos(dir->heading) * 1.0 *
				time_compensation * time_compression
			    );
			}  

			/* Set new direction of human object */
			dir->heading = (float)SFMSanitizeRadians(
			    (0.5 * PI) - atan2(delta.y, delta.x)
			);
			dir->pitch = (float)(0.0 * PI);
			dir->bank = (float)(0.0 * PI);

			/* Realize new position and direction of human */
			SARSimWarpObject(scene, obj_ptr, pos, dir);
		    }
		}
	    }
	    break;

	  case SAR_OBJ_TYPE_SMOKE:
	  case SAR_OBJ_TYPE_FIRE:
	  case SAR_OBJ_TYPE_EXPLOSION:
	  case SAR_OBJ_TYPE_CHEMICAL_SPRAY:
	  case SAR_OBJ_TYPE_FUELTANK:
	  case SAR_OBJ_TYPE_PREMODELED:
	    break;
	}


	/* Update values common to all object types here */

	/* Lights */
	SARSimUpdateLights(obj_ptr);

	return(0);
}


/*
 *	Called by SARSimApplyGCTL() to modify the given control position
 *	inputs by autopilot if autopilot is activated on the
 *	given object.
 *
 *	If autopilot is not on then no modifications will be made.
 *
 *	If autopilot is on then the game controller will be checked
 *	against the given control position values to see if the
 *	autopilot should be automatically turned off.  If not then
 *	the given control positions (not those in the gctl structure)
 *	will be modified to simulate corrections made by the autopilot.
 */
static void SARSimApplyGCTLAutoPilot(
	sar_core_struct *core_ptr, sar_object_struct *obj_ptr,
	gctl_struct *gc,
	float *h_con_coeff,
	float *p_con_coeff,
	float *b_con_coeff,
	float *elevator_trim,	/* Not the effective elevator trim */
	float *throttle_coeff,
	float *collective
)
{
	sar_object_aircraft_struct *aircraft;


	if((obj_ptr == NULL) || (gc == NULL))
	    return;

	if((h_con_coeff == NULL) || (p_con_coeff == NULL) ||
	   (b_con_coeff == NULL) || (elevator_trim == NULL) ||
	   (throttle_coeff == NULL) || (collective == NULL)
	)
	    return;


	/* Begin handling by object type */

	aircraft = SAR_OBJ_GET_AIRCRAFT(obj_ptr);
	if(aircraft != NULL)
	{
	    char need_autopilot_off = 0;
	    float tmp_delta;
	    float new_collective;
	    float tar_z, cur_z;

	    /* Check if autopilot is off */
	    if(aircraft->autopilot_state == SAR_AUTOPILOT_OFF)
		return;

	    /* Begin calculating new controller positions based on
	     * flight model type
	     */
	    switch(aircraft->flight_model_type)
	    {
	      case SAR_FLIGHT_MODEL_HELICOPTER:
		/* Begin adjusting the collective, calculate the new
		 * collective value.
		 */
		new_collective = *collective;
		cur_z = obj_ptr->pos.z;
		tar_z = aircraft->autopilot_altitude;

		tmp_delta = tar_z - cur_z;

		/* Need to go up? */
		if(tmp_delta > 0.1)
		{
		    /* Increase or decrease collective to achieve the
		     * target velocity.
		     */
		    float tar_vel_z = (float)MIN(tmp_delta / 2.0, 1.0);
		    if(aircraft->vel.z < tar_vel_z)
			new_collective += (float)(0.01 * time_compensation);
		    else
			new_collective -= (float)(0.01 * time_compensation);
		}
		/* Need to go down? */
		else if(tmp_delta < -0.1)
		{
		    /* Increase or decrease collective to achieve the
		     * target velocity.
		     */
		    float tar_vel_z = (float)MAX(tmp_delta / 2.0, -1.0);
		    if(aircraft->vel.z < tar_vel_z)
			new_collective += (float)(0.01 * time_compensation);
		    else
			new_collective -= (float)(0.01 * time_compensation);
		}

		/* Check if new controller positions differ too great
		 * from current controller positions that autopilot needs
		 * to be turned off.
		 */
		tmp_delta = new_collective - gc->throttle;
		if((tmp_delta > 0.1) || (tmp_delta < -0.1))
		    need_autopilot_off = 1;

		/* Does autopilot need to be turned off? */
		if(need_autopilot_off)
		{
		    aircraft->autopilot_state = SAR_AUTOPILOT_OFF;
		    SARMessageAdd(core_ptr->scene, "Autopilot: Off");
		}
		else
		{
		    /* Autopilot should remain on, so set new calculated
		     * control positions
		     */
		    *collective = new_collective;
		}
		break;

	      case SAR_FLIGHT_MODEL_AIRPLANE:

		/* Autopilot for airplane flight models is poor right
		 * now, instead of modifying the controls the actual
		 * pitch and bank is modified. This should be improved
		 * later however in order to adjust only the controls
		 * the pitch and bank *rates* need to be known and they
		 * are not known so far.
		 */
		if(1)
		{
		    SFMModelStruct *fdm = aircraft->fdm;

		    /* Adjust pitch */
		    if(obj_ptr->dir.pitch < (1.0 * PI))
		    {
			obj_ptr->dir.pitch = (float)MAX(
			    obj_ptr->dir.pitch - 0.005, 0.0 * PI
			);
		    }
		    else
		    {
			obj_ptr->dir.pitch = (float)MIN(
			    obj_ptr->dir.pitch + 0.005, 2.0 * PI
			);
			if(obj_ptr->dir.pitch >= (2.0 * PI))
			    obj_ptr->dir.pitch = (float)(0.0 * PI);
		    }

		    /* Adjust bank */
		    if(obj_ptr->dir.bank < (1.0 * PI))
		    {
			obj_ptr->dir.bank = (float)MAX(
			    obj_ptr->dir.bank - 0.005, 0.0 * PI
			);
		    }
		    else
		    {
			obj_ptr->dir.bank = (float)MIN(
			    obj_ptr->dir.bank + 0.005, 2.0 * PI
			);
			if(obj_ptr->dir.bank >= (2.0 * PI))
			    obj_ptr->dir.bank = (float)(0.0 * PI);
		    }

		    if(fdm != NULL)
		    {
			fdm->direction.pitch = obj_ptr->dir.pitch;
			fdm->direction.bank = obj_ptr->dir.bank;
		    }
		}


		/* Check if new controller positions differ too great
		 * from current controller positions that autopilot needs
		 * to be turned off.
		 */
		tmp_delta = gc->pitch - 0.0f;
		if((tmp_delta > 0.2) || (tmp_delta < -0.2f))
		    need_autopilot_off = 1;
		tmp_delta = gc->bank - 0.0f;
		if((tmp_delta > 0.2) || (tmp_delta < -0.2f))
		    need_autopilot_off = 1;

		/* Does autopilot need to be turned off? */
		if(need_autopilot_off)
		{
		    aircraft->autopilot_state = SAR_AUTOPILOT_OFF;
		    SARMessageAdd(core_ptr->scene, "Autopilot: Off");
		}
		else
		{
		    /* Autopilot should remain on, so set new calculated
		     * control positions
		     */

		}
		break;

	      /* Other flight model, possibly slew, ignore */
	      case SAR_FLIGHT_MODEL_SLEW:
	        break;
	    }


	    /* Check if game controller positions would cause the
	     * autopilot to turn off automatically (that is, if they
	     * are moved far beyond the autopilot calculated control
	     * range
	     */





	}
}

/*
 *      Apply game controller (gctl) values to the given player object
 *	and camera on the scene structure found on the given core
 *	structure.
 *
 *	The gctl values will be applied to both the object and the
 *	object's FDM (if it has one).
 */
void SARSimApplyGCTL(sar_core_struct *core_ptr, sar_object_struct *obj_ptr)
{
	const float camera_turn_rate = (float)(0.30 * PI * SAR_SEC_TO_CYCLE_COEFF);

	int rotor_num;
	sar_air_worthy_state air_worthy_state;
	sar_engine_state engine_state;
	sar_flight_model_type flight_model_type;
	sar_autopilot_state autopilot_state = SAR_AUTOPILOT_OFF;
	sar_direction_struct *dir;
	sar_position_struct *pos;
	sar_object_aircraft_struct *aircraft = NULL;
	const sar_contact_bounds_struct *cb;
	float contact_radius = 0.0f;
	sar_obj_hoist_struct *hoist = NULL;
	sar_obj_part_struct *door_ptr = NULL;
	sar_obj_rotor_struct *rotor_ptr;
	SFMModelStruct *fdm = NULL;
	float	h_con_coeff = 0.0f,
		p_con_coeff = 0.0f,
		b_con_coeff = 0.0f,
		elevator_trim = 0.0f,
		elevator_trim_effective = 0.0f,
		throttle_coeff = 0.0f,
		throttle_output = 0.0f,
		collective = 0.0f;
	sar_scene_struct *scene = core_ptr->scene;
	gctl_struct *gc = core_ptr->gctl;
	const sar_option_struct *opt = &core_ptr->option;
	if((scene == NULL) || (gc == NULL) || (obj_ptr == NULL))
	    return;

	cb = obj_ptr->contact_bounds;	/* Get contact bounds */

	/* Get flat contact radius */
	contact_radius = SARSimGetFlatContactRadius(obj_ptr);


	/* Apply game controller values based on object type */
	switch(obj_ptr->type)
	{
	  case SAR_OBJ_TYPE_GARBAGE:
	  case SAR_OBJ_TYPE_STATIC:
	  case SAR_OBJ_TYPE_AUTOMOBILE:
	  case SAR_OBJ_TYPE_WATERCRAFT:
	    break;

	  case SAR_OBJ_TYPE_AIRCRAFT:
	    aircraft = SAR_OBJ_GET_AIRCRAFT(obj_ptr);
	    if(aircraft == NULL)
		break;

	    dir = &obj_ptr->dir;
	    pos = &obj_ptr->pos;
	    air_worthy_state = aircraft->air_worthy_state;
	    engine_state = aircraft->engine_state;
	    flight_model_type = aircraft->flight_model_type;
	    autopilot_state = aircraft->autopilot_state;

	    elevator_trim = aircraft->elevator_trim;

	    door_ptr = SARObjGetPartPtr(obj_ptr, SAR_OBJ_PART_TYPE_DOOR_RESCUE, 0);
	    hoist = SARObjGetHoistPtr(obj_ptr, 0, NULL);
	    fdm = aircraft->fdm;

	    /* If flight model is slew then use previous flight model type */
	    if(flight_model_type == SAR_FLIGHT_MODEL_SLEW)
		flight_model_type = aircraft->last_flight_model_type;

	    /* Begin fetching control position values based on landed
	     * state and airworthyness
	     */

	    /* Check if landed */
	    if(aircraft->landed)
	    {
		/* Is landed */

		/* Handle flight control by airworthy states */
		if(air_worthy_state == SAR_AIR_WORTHY_FLYABLE)
		{
		    if(engine_state == SAR_ENGINE_ON)
		    {
			h_con_coeff = (float)CLIP(gc->heading, -1.0, 1.0);
			p_con_coeff = (float)CLIP(gc->pitch, -1.0, 1.0);
			b_con_coeff = (float)CLIP(gc->bank, -1.0, 1.0);
			elevator_trim_effective = (float)CLIP(
			    aircraft->elevator_trim, -1.0, 1.0
			);

			switch(flight_model_type)
			{
			  case SAR_FLIGHT_MODEL_AIRPLANE:
			    throttle_output = throttle_coeff = (float)CLIP(gc->throttle, 0.0, 1.0);
			    collective = 1.0f;
			    break;

			  case SAR_FLIGHT_MODEL_HELICOPTER:
			    throttle_coeff = 1.0f;
			    if(autopilot_state == SAR_AUTOPILOT_ON)
				collective = (float)CLIP(aircraft->collective, 0.0, 1.0);
			    else
				collective = (float)CLIP(gc->throttle, 0.0, 1.0);
 			    throttle_output = (float)CLIP(throttle_coeff -
				aircraft->collective_range + (
				    collective * aircraft->collective_range),
				0.0, 1.0
			    );
			    break;

			  case SAR_FLIGHT_MODEL_SLEW:
			    break;
			}
		    }
		    else
		    {
			/* Engine off while on ground, only allow ground turning */
			h_con_coeff = (float)CLIP(gc->heading, -1.0, 1.0);
		    }
		}
		else if(air_worthy_state == SAR_AIR_WORTHY_OUT_OF_CONTROL)
		{
		    /* Out of control and landed */
 
		    /* Check if engines are on, if so give it just  
		     * half power at most
		     */
		    if(engine_state == SAR_ENGINE_ON)
		    {
			switch(flight_model_type)
			{
			  case SAR_FLIGHT_MODEL_AIRPLANE:
			    throttle_output = throttle_coeff = (float)CLIP(gc->throttle * 0.5, 0.0, 1.0);
			    collective = 1.0f;
			    break;

			  case SAR_FLIGHT_MODEL_HELICOPTER:
			    throttle_coeff = 0.5f;
			    if(autopilot_state == SAR_AUTOPILOT_ON)
				collective = (float)CLIP(aircraft->collective, 0.0, 1.0);
			    else
				collective = (float)CLIP(gc->throttle, 0.0, 1.0);
			    throttle_output = (float)CLIP(throttle_coeff -
				aircraft->collective_range + (
				    collective * aircraft->collective_range),
				0.0, 1.0
			    );
			    break;

			  case SAR_FLIGHT_MODEL_SLEW:
			    break;
			}
		    }
		}
		else
		{
		    /* Not flyable and landed */
		}
	    }
	    else
	    {   
		/* In flight */

		/* Handle flight control by airworthy states */
		if(air_worthy_state == SAR_AIR_WORTHY_FLYABLE)
		{
		    h_con_coeff = (float)CLIP(gc->heading, -1.0, 1.0);
		    p_con_coeff = (float)CLIP(gc->pitch, -1.0, 1.0);
		    b_con_coeff = (float)CLIP(gc->bank, -1.0, 1.0);
		    elevator_trim_effective = (float)CLIP(
			aircraft->elevator_trim, -1.0, 1.0
		    );

		    /* Check if engines are on */
		    if(engine_state == SAR_ENGINE_ON)
		    {
			switch(flight_model_type)
			{
			  case SAR_FLIGHT_MODEL_AIRPLANE:
			    throttle_output = throttle_coeff = (float)CLIP(gc->throttle, 0.0, 1.0);
			    collective = 1.0f;
			    break;

			  case SAR_FLIGHT_MODEL_HELICOPTER:
			    throttle_coeff = 1.0f;
			    if(autopilot_state == SAR_AUTOPILOT_ON)
				collective = (float)CLIP(aircraft->collective, 0.0, 1.0);
			    else
				collective = (float)CLIP(gc->throttle, 0.0, 1.0);
			    throttle_output = (float)CLIP(throttle_coeff -
				aircraft->collective_range + (
				    collective * aircraft->collective_range),
				0.0, 1.0
			    );
			    break;

			  case SAR_FLIGHT_MODEL_SLEW:
			    break;
			}
		    }
		}
		else if(air_worthy_state == SAR_AIR_WORTHY_OUT_OF_CONTROL)
		{
		    /* In flight, but out of control */

		    elevator_trim_effective = (float)CLIP(
			aircraft->elevator_trim, -1.0, 1.0
		    );

		    switch(flight_model_type)
		    {
		      case SAR_FLIGHT_MODEL_AIRPLANE:
			/* Aircraft, roll when out of control */
			h_con_coeff = (float)CLIP(gc->heading, -0.1, 0.1); 
			p_con_coeff = (float)CLIP(gc->pitch, -0.1, 0.1);
			b_con_coeff = (float)CLIP(gc->bank + 1.1, -1.0, 1.0);
			break;

		      case SAR_FLIGHT_MODEL_HELICOPTER:
			/* Helicopter, spin when out of control */
			h_con_coeff = (float)CLIP(gc->heading + 1.1, -1.0, 1.0);
			p_con_coeff = (float)CLIP(gc->pitch, -0.1, 0.1);
			b_con_coeff = (float)CLIP(gc->bank, -0.1, 0.1);
			break;

		      case SAR_FLIGHT_MODEL_SLEW:
			break;
		    }
	    
		    /* Check if engines are on, if so give it less power
		     * (75% power)
		     */
		    if(engine_state == SAR_ENGINE_ON)
		    {
			switch(flight_model_type)
			{
			  case SAR_FLIGHT_MODEL_AIRPLANE:
			    throttle_output = throttle_coeff = (float)CLIP(gc->throttle * 0.75, 0.0, 1.0);
			    collective = 1.0f;
			    break;

			  case SAR_FLIGHT_MODEL_HELICOPTER:
			    throttle_coeff = 0.75f;	/* 75% power */
			    if(autopilot_state == SAR_AUTOPILOT_ON)
				collective = (float)CLIP(aircraft->collective, 0.0, 1.0);
			    else
				collective = (float)CLIP(gc->throttle, 0.0, 1.0);
			    throttle_output = (float)CLIP(throttle_coeff -
				aircraft->collective_range + (
				    collective * aircraft->collective_range),
				0.0, 1.0
			    );
			    break;

			  case SAR_FLIGHT_MODEL_SLEW:
			    break;
			}
		    }
		}
		else
		{
		    /* In flight and not flyable */

		}
	    }

	    /* At this point the new control position values have
	     * been fetched
	     */

	    /* Apply autopilot (if on) to the newly fetched control
	     * position values, modifying them as needed
	     */
	    SARSimApplyGCTLAutoPilot(
		core_ptr, obj_ptr, gc,
		&h_con_coeff, &p_con_coeff, &b_con_coeff,
		&elevator_trim,		/* Pass not effective elevator trim */
		&throttle_coeff, &collective
	    );


	    /* Begin updating the actual throttle value from the newly
	     * calculated one, check if there is a change in the
	     * throttle value from the one calculated above
	     */
	    if(aircraft->throttle != throttle_coeff)
	    {
		/* Calculate maximum allowed change in throttle on this
		 * cycle, this is to simulate slow throttle
		 * responsiveness
		 */
/* Should get throttle change coeff from object. But for now we use
 * constants dependant on the global opt->flight_physics_level
 */
		float throttle_dc = 0.0f;

		switch(opt->flight_physics_level)
		{
		  case FLIGHT_PHYSICS_REALISTIC:
		    throttle_dc = (float)(
			0.035 * time_compensation *
			time_compression
		    );
		    break;
		  case FLIGHT_PHYSICS_MODERATE:
		    throttle_dc = (float)(
			0.06 * time_compensation *
			time_compression
		    );
		    break;
		  case FLIGHT_PHYSICS_EASY:
		    throttle_dc = (float)(
			0.1 * time_compensation *
			time_compression
		    );
		    break;
		}

		/* Decrease in throttle? */
		if(aircraft->throttle > throttle_coeff)
		{
		    aircraft->throttle = (float)MAX(MAX(
			throttle_coeff,
			aircraft->throttle - throttle_dc
		    ), 0.0);
		}
		/* Increase in throttle? */
		else if(aircraft->throttle < throttle_coeff)
		{
		    aircraft->throttle = (float)MIN(MIN(
			throttle_coeff,
			aircraft->throttle + throttle_dc
		    ), 1.0);
		}
	    }

	    /* Begin updating new control positions on aircraft */

	    /* Displayed attitude control positions */
	    aircraft->control_heading = gc->heading;
	    aircraft->control_pitch = gc->pitch;
	    aircraft->control_bank = gc->bank;

	    /* Effective attitude control positions */
	    aircraft->control_effective_heading = (float)CLIP(
		h_con_coeff, -1.0, 1.0
	    );
	    aircraft->control_effective_pitch = (float)CLIP(
		p_con_coeff, -1.0, 1.0
	    );
	    aircraft->control_effective_bank = (float)CLIP(
		b_con_coeff, -1.0, 1.0
	    );

	    /* Collective */
	    aircraft->collective = collective;

	    /* If air brakes exist then turn air brakes on/off */
	    if(aircraft->air_brakes_state > -1)
		SARSimOpAirBrakes(
		    scene, &core_ptr->object, &core_ptr->total_objects,
		    obj_ptr,
		    (gc->air_brakes_state) ? 1 : 0,
		    core_ptr->recorder, opt->event_sounds,
		    SAR_IS_EAR_IN_COCKPIT(scene)
		);

	    /* Wheel brakes on/off */
	    if(aircraft->wheel_brakes_state > -1)
	    {
		aircraft->wheel_brakes_state = gc->wheel_brakes_state;
		aircraft->wheel_brakes_coeff = gc->wheel_brakes_coeff;
	    }

	    /* Update SAR FDM control positions */
	    if(fdm != NULL)
	    {
		fdm->heading_control_coeff = h_con_coeff;  
		fdm->pitch_control_coeff = p_con_coeff;
		fdm->bank_control_coeff = b_con_coeff;
		fdm->elevator_trim_coeff = elevator_trim_effective;
		fdm->throttle_coeff = SARSimThrottleOutputCoeff(
		    flight_model_type,		/* Flight model never slew */
		    aircraft->throttle,
		    aircraft->collective,
		    aircraft->collective_range
		);
		fdm->gear_brakes_state = (aircraft->wheel_brakes_state > 0) ?
		    True : False;
		fdm->gear_brakes_coeff = aircraft->wheel_brakes_coeff;
		fdm->air_brakes_state = (aircraft->air_brakes_state > 0) ?
		    True : False;
	    }

	    /* Update rotor control positions, this is to update the
	     * pitch and bank angles of the rotor according to the
	     * game controller positions
	     */
	    for(rotor_num = 0; rotor_num < aircraft->total_rotors; rotor_num++)
	    {
		rotor_ptr = aircraft->rotor[rotor_num];
		if(rotor_ptr == NULL)
		    continue;

		/* Report values from gc since *_con_coeff reflects those
		 * dependant on engine state and other physics values
		 */
		SARSimRotorUpdateControls(
		    rotor_ptr,
		    (float)CLIP(gc->pitch, -1.0, 1.0),
		    (float)CLIP(gc->bank, -1.0, 1.0)
		);
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


	/* Operate hoist */
	if(hoist != NULL)
	{
	    float	rope_rate = hoist->rope_rate,
			hoist_up_coeff = (float)((gc->ctrl_state) ?
			    gc->zoom_in_coeff : gc->hoist_up_coeff
			),
			hoist_down_coeff = (float)((gc->ctrl_state) ?
			    gc->zoom_out_coeff : gc->hoist_down_coeff
			);
	    /* Rope movable? */
	    if(rope_rate > 0.0f)
	    {
		/* Hoist up? */
		if(hoist_up_coeff > 0.0f)
		{
		    float prev_rope = hoist->rope_cur;

		    /* Pull in/up */
		    hoist->rope_cur -= (rope_rate * time_compensation *
			time_compression * hoist_up_coeff);

		    /* Do not update rope_cur_vis, it will be updated
		     * in the other simulation calls
		     */

		    /* Rope hoisted in? */
		    if(hoist->rope_cur < hoist->contact_z_max)
		    {
			hoist->rope_cur = 0.0f;
			hoist->rope_cur_vis = 0.0f;
			hoist->on_ground = 0;

			/* Was rope previously out? */
			if(prev_rope > hoist->contact_z_max)
			{
			    /* Rope has just been hoisted in, reset some
			     * hoist values
			     */
			    hoist->occupants_mass = 0.0f;

			    /* Call hoist in procedure to remove the
			     * occupants from the hoist and process them
			     */
			    SARSimDoHoistIn(core_ptr, obj_ptr);

			    /* Door exists and is currently opened? */
			    if((door_ptr != NULL) ?
			       (door_ptr->flags & SAR_OBJ_PART_FLAG_STATE) : 0
			    )
			    {
				/* Door is opened, try to close it? */
				if(!(door_ptr->flags & SAR_OBJ_PART_FLAG_DOOR_STAY_OPEN))
				    SARSimOpDoorRescue(
				        scene,
				        &core_ptr->object, &core_ptr->total_objects,
				        obj_ptr,
				        0	/* Close */
				    );
			    }
			}	/* Was rope previously out? */
		    }
		}
		/* Hoist down? */
		else if(hoist_down_coeff > 0.0f)
		{
		    /* Open rescue door as needed */
/* TODO Check if rescue door needs to be opened */
		    if((door_ptr != NULL) ?
			!(door_ptr->flags & SAR_OBJ_PART_FLAG_STATE) : 0
		    )
		    {
			/* Rescue door exists but is closed, so we need
			 * to open it
			 */
			SARSimOpDoorRescue(
			    scene,
			    &core_ptr->object, &core_ptr->total_objects,
			    obj_ptr,
			    1		/* Open */
			);

			/* Here is also a good point to check if we have
			 * enough room for additional passengers
			 */
			if((aircraft != NULL) ?
			    (aircraft->passengers >=
				aircraft->passengers_max) : 0
			)
			{
			    /* Is this the player object? */
			    if(scene->player_obj_ptr == obj_ptr)
				SARMessageAdd(
				    scene,
				    SAR_MESG_NO_ROOM_LEFT_FOR_PASSENGERS
				);
			}
		    }

		    /* Was rope previously in? */
		    if(hoist->rope_cur < hoist->contact_z_max)
		    {
			/* Hoist deployment was in so now it needs to
			 * be put out and its values initialized
			 */
			sar_position_struct	*hoist_pos = &hoist->pos,
						*hoist_offset = &hoist->offset;
			double	tx = hoist_offset->x,
				ty = hoist_offset->y,
				tz = hoist_offset->z;

			/* Move to initial position at hoist center */
			SFMOrthoRotate2D(
			    obj_ptr->dir.heading, &tx, &ty
			);
			hoist_pos->x = (float)(
			    obj_ptr->pos.x + tx
			);
			hoist_pos->y = (float)(
			    obj_ptr->pos.y + ty
			);
			hoist_pos->z = (float)(
			    obj_ptr->pos.z + tz -
			    hoist->rope_cur
			);

			/* Reset hoist values */
			hoist->rope_cur = hoist->contact_z_max;
			hoist->rope_cur_vis = hoist->rope_cur;
			hoist->on_ground = 0;
		    }
		    else
		    {
			/* Lower rope normally */
			hoist->rope_cur += (rope_rate *
			    time_compensation * time_compression *
			    hoist_down_coeff);
			if(hoist->rope_cur > hoist->rope_max)
			    hoist->rope_cur = hoist->rope_max;

			/* Do not update rope_cur_vis, it will be updated
			 * in the other simulation calls
			 */
		    }
		}
	    }
	}


	/* Begin updating camera distance/direction/position */

/* Updates the spot light on the aircraft if the ctrl key is
 * down and the hat is moved
 */
#define ROTATE_SPOTLIGHT_BY_HAT(_hat_x_,_hat_y_) {		\
								\
 /* Rotate spotlight heading */					\
 if(((_hat_x_) != 0.0f) && (aircraft != NULL)) {	\
  sar_direction_struct *dir = &aircraft->spotlight_dir;	\
  /* Calculate rate (rad/sec) based on the pitch */		\
  const float rate = (float)(0.6 * PI *				\
   ((dir->pitch + (0.05 * PI)) / (0.55 * PI))			\
  );								\
  /* Rotate heading */						\
  dir->heading = (float)SFMSanitizeRadians(			\
   dir->heading +						\
   ((_hat_x_) * rate * time_compensation)			\
  );								\
 }								\
								\
 /* Rotate spotlight pitch */					\
 if(((_hat_y_) != 0.0f) && (aircraft != NULL)) {	\
  sar_direction_struct *dir = &aircraft->spotlight_dir;	\
  /* Calculate rate (rad/sec) based on the pitch */		\
  const float rate = (float)(0.3 * PI *				\
   ((dir->pitch + (0.05 * PI)) / (0.55 * PI))			\
  );								\
  /* Rotate pitch */						\
  dir->pitch = (float)(						\
   dir->pitch +							\
   (-(_hat_y_) * rate * time_compensation)			\
  );								\
  if(dir->pitch <= (0.02 * PI))					\
   dir->pitch = (float)(0.02 * PI);				\
  else if(dir->pitch > (0.5 * PI))				\
   dir->pitch = (float)(0.5 * PI);				\
 }								\
}

	/* Update camera based on current camera reference */
	switch(scene->camera_ref)
	{
	  case SAR_CAMERA_REF_HOIST:
	    dir = &scene->camera_hoist_dir;

	    /* Use hat position to rotate spot light? */
	    if(gc->ctrl_state)
	    {
		ROTATE_SPOTLIGHT_BY_HAT(gc->hat_x, gc->hat_y);
	    }
	    else
	    {
		/* Hat moves camera around target (basket or aircraft)
		 * as if camera was set to SAR_CAMERA_REF_SPOT
		 */
		if(gc->hat_x != 0.0f)
		{
		    dir->heading = (float)SFMSanitizeRadians(
			dir->heading - (gc->hat_x * camera_turn_rate *
			time_compensation)	/* No time acceleration */
		    );
		}
		if(gc->hat_y != 0.0f)
		{
		    float prev_pitch = dir->pitch;
		    dir->pitch = (float)SFMSanitizeRadians(
			dir->pitch - (gc->hat_y * camera_turn_rate *
			time_compensation)	/* No time acceleration */
		    );
		    /* Clip pitch */
		    if((dir->pitch < (float)(1.51 * PI)) &&
		       (dir->pitch > (float)(0.49 * PI))
		    )
			dir->pitch = (float)((prev_pitch < (float)(1.0 * PI)) ?
			    (0.49 * PI) : (1.51 * PI)
			);
		}
	    }
	    /* Zoom in and out */
	    if(!gc->ctrl_state)
	    {
		if(gc->zoom_in_coeff > 0.0f)
		{               
		    scene->camera_hoist_dist -= (float)(
			((gc->shift_state) ? 50.0 : 10.0) *
			time_compensation * SAR_SEC_TO_CYCLE_COEFF *
			gc->zoom_in_coeff
		    );
		    /* Do not zoom closer than contact radius of hoist */
		    if(scene->camera_hoist_dist < contact_radius)
			scene->camera_hoist_dist = contact_radius;
		}
		if(gc->zoom_out_coeff > 0.0f)
		{
		    scene->camera_hoist_dist += (float)(
			((gc->shift_state) ? 50.0 : 10.0) *
			time_compensation * SAR_SEC_TO_CYCLE_COEFF *
			gc->zoom_out_coeff
		    );
		}
	    }
	    break;

	  case SAR_CAMERA_REF_MAP:
	    pos = &scene->camera_map_pos;

	    /* Use hat position to rotate spot light? */
	    if(gc->ctrl_state)
	    {
		ROTATE_SPOTLIGHT_BY_HAT(gc->hat_x, gc->hat_y);
	    }
	    else
	    {
		/* Hat moves map location */
		if(gc->hat_x != 0.0f)
		    pos->x += (float)(gc->hat_x *
			(gc->shift_state ? 32500.0 : 5000.0)
			* time_compensation
		    );
		if(gc->hat_y != 0.0f)
		    pos->y += (float)(gc->hat_y *
			(gc->shift_state ? 32500.0 : 5000.0)
			* time_compensation
		    );
	    }
	    /* Zoom map in and out */
	    if(!gc->ctrl_state)
	    {
		if(gc->zoom_in_coeff > 0.0f)
		{
		    pos->z -= (float)(
			(gc->shift_state ? 30000.0 : 1200.0) *
			time_compensation * SAR_SEC_TO_CYCLE_COEFF *
			gc->zoom_in_coeff
		    );
		    /* Stay atleast 10 meters above ground */
		    if(pos->z < 10.0f)
			pos->z = 10.0f;
		}       
		if(gc->zoom_out_coeff > 0.0f)
		{
		    pos->z += (float)(
			(gc->shift_state ? 30000.0 : 1200.0) *
			time_compensation * SAR_SEC_TO_CYCLE_COEFF *
			gc->zoom_out_coeff
		    );
		    /* Keep below `map ceiling' */
		    if(pos->z > (float)SFMFeetToMeters(SAR_MAX_MAP_VIEW_HEIGHT))
			pos->z = (float)SFMFeetToMeters(SAR_MAX_MAP_VIEW_HEIGHT);
		}
	    }
	    break;

	  case SAR_CAMERA_REF_TOWER:
	    pos = &scene->camera_tower_pos;

	    /* Use hat position to rotate spot light? */
	    if(gc->ctrl_state)
	    {
		ROTATE_SPOTLIGHT_BY_HAT(gc->hat_x, gc->hat_y);
	    }
	    else
	    {
		/* Hat x orbits camera position around camera_target
		 * object.
		 */
		if(gc->hat_x != 0.0)
		{
		    sar_object_struct *tar_obj_ptr = SARObjGetPtr(
			core_ptr->object, core_ptr->total_objects,
			scene->camera_target
		    );
		    if(tar_obj_ptr != NULL)
		    {
			sar_position_struct *tar_pos = &tar_obj_ptr->pos;
			double a[3], r[3];

			/* Calculate deltas from target to camera */
			a[0] = pos->x - tar_pos->x;
			a[1] = pos->y - tar_pos->y;
			a[2] = 0.0;
			/* Rotate delta, thus orbiting the tower offset */
			MatrixRotateHeading3(
			    a,
			    -gc->hat_x * camera_turn_rate * time_compensation,
			    r
			);
			/* Set new tower position based on offset from target */
			pos->x = (float)(tar_pos->x + r[0]);
			pos->y = (float)(tar_pos->y + r[1]);
		    }
		}

		/* Hat y moves camera up and down */
		if(gc->hat_y != 0.0)
		{
		    sar_object_struct *tar_obj_ptr = SARObjGetPtr(
			core_ptr->object, core_ptr->total_objects,
			scene->camera_target
		    );
		    if(tar_obj_ptr != NULL)
		    {
			sar_position_struct *tar_pos = &tar_obj_ptr->pos;
		        float distance, ddist;


		        /* Calculate distance from target to camera */
		        distance = (float)SAR_ABSOLUTE(pos->z - tar_pos->z);
		        /* Interprite distance as always 5 meters or more */
		        if(distance < 5.0)
			    distance = 5.0;

		        /* Calculate distance change */
		        ddist = (float)(
			    distance *
			    ((gc->shift_state) ? 2.5 : 0.9) *
			    gc->hat_y *
			    time_compensation * SAR_SEC_TO_CYCLE_COEFF
			);

		        /* Set new camera position */
		        pos->z += ddist;
		        if(pos->z < 0.0f)
			    pos->z = 0.0f;
		    }
	        }
	    }

	    /* Move closer or away when ctrl key is not held */
	    if(!gc->ctrl_state)
	    {
		if((gc->zoom_in_coeff > 0.0f) || (gc->zoom_out_coeff > 0.0f))
		{
		    sar_object_struct *tar_obj_ptr = SARObjGetPtr(
			core_ptr->object, core_ptr->total_objects,
			scene->camera_target
		    );
		    if(tar_obj_ptr != NULL)
		    {
			sar_position_struct *tar_pos = &tar_obj_ptr->pos;
			float dx, dy, distance_xy, ddist, dcoeff;
			float zoom_coeff = gc->zoom_out_coeff - 
			    gc->zoom_in_coeff;

			/* Calculate deltas from camera to target */
			dx = tar_pos->x - pos->x;
			dy = tar_pos->y - pos->y;
			/* Calculate xy distance between camera and target */
			distance_xy = (float)SFMHypot2(dx, dy);

			/* Do this only if the current distance away is
			 * greater than a meter.
			 */
			if(distance_xy > 1.0f)
			{
			/* Calculate delta distance change based on *1.5
			 * (or *3.0 if shift is held) of current xy 
			 * distance relative to 1 second.
			 */
			ddist = (float)(
			    distance_xy *
			    ((gc->shift_state) ? 2.5 : 0.9) *
			    zoom_coeff *
			    time_compensation * SAR_SEC_TO_CYCLE_COEFF
			);
			/* Calculate coefficient change from old distance to
			 * new distance (note we know distance_xy is > 1.0).
			 */
			dcoeff = (ddist + distance_xy) / distance_xy;

			/* Use similar triangles to calculate new tower
			 * camera position.
			 */
			pos->x = tar_pos->x - (dx * dcoeff);
			pos->y = tar_pos->y - (dy * dcoeff);
			}
		    }
		}
	    }
	    break;
	     
	  case SAR_CAMERA_REF_SPOT:
	    dir = &scene->camera_spot_dir;

	    /* Use hat position to rotate spot light? */
	    if(gc->ctrl_state)
	    {
		ROTATE_SPOTLIGHT_BY_HAT(gc->hat_x, gc->hat_y);
	    }
	    else
	    {
		/* Hat moves camera around target in spot reference */
		if(gc->hat_x != 0.0f)
		    dir->heading = (float)SFMSanitizeRadians(
			dir->heading - (gc->hat_x * camera_turn_rate *
			time_compensation)	/* No time acceleration */
		    );

		if(gc->hat_y != 0.0f)
		{
		    float prev_pitch = dir->pitch;
		    dir->pitch = (float)SFMSanitizeRadians(
			dir->pitch - (gc->hat_y * camera_turn_rate *
			time_compensation)	/* No time acceleration */
		    );

		    /* Clip pitch */
		    if((dir->pitch < (float)(1.51 * PI)) &&
		       (dir->pitch > (float)(0.49 * PI))
		    )
			dir->pitch = (float)((prev_pitch < (float)(1.0 * PI)) ?
			    (0.49 * PI) : (1.51 * PI)
			);
		}
/*
		dir->bank = 0.0f;
 */
	    }
	    /* Zoom in and out when ctrl key is not held */
	    if(!gc->ctrl_state)
	    {
		if(gc->zoom_in_coeff > 0.0f)
		{
		    float ddist = (float)(scene->camera_spot_dist *
			((gc->shift_state) ? 2.0 : 0.8) * 
			-gc->zoom_in_coeff * time_compensation *
			SAR_SEC_TO_CYCLE_COEFF
		    );

		    scene->camera_spot_dist += ddist;                        
		    if(scene->camera_spot_dist < contact_radius)
			scene->camera_spot_dist = contact_radius;
		}
		if(gc->zoom_out_coeff > 0.0)
		{
		    float ddist = (float)(scene->camera_spot_dist *
			((gc->shift_state) ? 2.0 : 0.8) *
			gc->zoom_out_coeff * time_compensation *
			SAR_SEC_TO_CYCLE_COEFF
		    );

		    scene->camera_spot_dist += ddist;
		}
	    }
	    break;

	  default:      /* Default to cockpit */
	    dir = &scene->camera_cockpit_dir;   

	    /* Use hat position to rotate spot light? */
	    if(gc->ctrl_state)
	    {
		ROTATE_SPOTLIGHT_BY_HAT(gc->hat_x, gc->hat_y);
	    }
	    else
	    {
		/* If shift key is held, then rotate camera instantly
		 * to right angle directions. Otherwise rotate normally.
		 */
		if(gc->shift_state)
		{
		    if(gc->hat_x > 0.5)
			dir->heading = (float)(0.5 * PI);
		    else if(gc->hat_x < -0.5)
			dir->heading = (float)(1.5 * PI);

		    if(gc->hat_y > 0.5)
			dir->heading = (float)(0.0 * PI);
		    else if(gc->hat_y < -0.5)
			dir->heading = (float)(1.0 * PI);

		    dir->pitch = (float)(0.0 * PI);
		}
		else
		{                    
		    if(gc->hat_x != 0.0f)
		    {
			dir->heading = (float)SFMSanitizeRadians(
			    dir->heading + (gc->hat_x * camera_turn_rate *
			    time_compensation)
			);
		    }

		    if(gc->hat_y != 0.0f)
		    {
			float prev_pitch = dir->pitch;
			dir->pitch = (float)SFMSanitizeRadians(
			    dir->pitch - (gc->hat_y * camera_turn_rate *
			    time_compensation)      
			);

			/* Clip pitch */
			if((dir->pitch < (float)(1.51 * PI)) &&
			   (dir->pitch > (float)(0.49 * PI))
			)
			    dir->pitch = (float)((prev_pitch < (float)(1.0 * PI)) ?
				(0.49 * PI) : (1.51 * PI)
			    );
		    }

/*		    dir->bank = 0.0f;	Camera bank is unaffected */
		}
	    }
	    break;
	}
#undef ROTATE_SPOTLIGHT_BY_HAT
}
