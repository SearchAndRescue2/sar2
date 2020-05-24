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
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "matrixmath.h"
#include "sfm.h"

#include "sarreality.h"
#include "obj.h"
#include "objutils.h"
#include "objsound.h"
#include "smoke.h"
#include "explosion.h"
#include "messages.h" 
#include "simmanage.h"
#include "simutils.h"
#include "sartime.h"
#include "sar.h"
#include "config.h"


float SARSimGetFlatContactRadius(sar_object_struct *obj_ptr);
sar_object_struct *SARSimMatchObjectFromFDM(
	sar_object_struct **list, int total,
	SFMModelStruct *fdm, int *index
);
float SARSimThrottleOutputCoeff(
	int flight_model_type,
	float throttle,
	float collective, float collective_range
);
float SARSimStallSpeed(
	const sar_object_aircraft_struct *obj_aircraft_ptr
);

void SARSimWarpObjectRelative(
	sar_scene_struct *scene, sar_object_struct *obj_ptr,
	sar_object_struct **ptr, int total, int ref_obj_num,
	sar_position_struct *offset_pos,
	sar_direction_struct *offset_dir
);	
void SARSimWarpObject(
	sar_scene_struct *scene, sar_object_struct *obj_ptr,
	sar_position_struct *new_pos,
	sar_direction_struct *new_dir
);

void SARSimUpdatePart(
	sar_scene_struct *scene,
	sar_object_struct *obj_ptr, sar_obj_part_struct *part_ptr,
	snd_recorder_struct *recorder, int play_sound, int ear_in_cockpit
);
void SARSimOpAirBrakes(
	sar_scene_struct *scene,
	sar_object_struct ***ptr, int *total,
	sar_object_struct *obj_ptr, int air_brakes_state,
	snd_recorder_struct *recorder, int play_sound, int ear_in_cockpit

);
void SARSimOpLandingGear(  
	sar_scene_struct *scene,
	sar_object_struct ***ptr, int *total,
	sar_object_struct *obj_ptr, int gear_state,
	snd_recorder_struct *recorder, int play_sound, int ear_in_cockpit
);
void SARSimOpDoorRescue(
	sar_scene_struct *scene,
	sar_object_struct ***ptr, int *total,
	sar_object_struct *obj_ptr,
	int state	/* 0 = close, 1 = open */
);

void SARSimRotorUpdateControls(
	sar_obj_rotor_struct *rotor_ptr,
	float pitch, float bank
);

sar_engine_state SARSimGetEngineState(sar_object_struct *obj_ptr);
void SARSimOpEngine(
	sar_scene_struct *scene,
	sar_object_struct ***ptr, int *total,
	sar_object_struct *obj_ptr, sar_engine_state engine_state,
	snd_recorder_struct *recorder, char play_sound, char ear_in_cockpit
);
void SARSimPitchEngine(
	sar_scene_struct *scene,
	sar_object_struct ***ptr, int *total,
	sar_object_struct *obj_ptr, int pitch_state
);

void SARSimUpdateLights(sar_object_struct *obj_ptr);
int SARSimGetLightsState(sar_object_struct *obj_ptr);
void SARSimOpLights(
	sar_object_struct *obj_ptr, int state
);
int SARSimGetStrobesState(sar_object_struct *obj_ptr);
void SARSimOpStrobes(
	sar_object_struct *obj_ptr, int state
);
int SARSimGetAttenuateState(sar_object_struct *obj_ptr);
void SARSimOpAttenuate(
	sar_object_struct *obj_ptr, int state
);

int SARSimOpRepair(
	sar_scene_struct *scene, sar_object_struct *obj_ptr
);
int SARSimOpRefuel(
	sar_scene_struct *scene, sar_object_struct *obj_ptr
);

int SARSimOpPassengersSetLeave(
	sar_scene_struct *scene, sar_object_struct *obj_ptr,
	int passengers_leave_pending, int passengers_drop_pending
);
int SARSimOpPassengersUnloadAll(
	sar_scene_struct *scene, sar_object_struct *obj_ptr
);

float SARSimOpTransferFuelFromTanks(
	sar_scene_struct *scene, sar_object_struct *obj_ptr
);

int SARSimOpDropFuelTankNext(
	sar_scene_struct *scene,
	sar_object_struct ***ptr, int *total,
	int obj_num, sar_object_struct *obj_ptr
);

void SARSimSmokeSpawn(
	sar_core_struct *core_ptr, sar_object_struct *obj_ptr,
	sar_object_smoke_struct *obj_smoke_ptr
);

int SARSimIsSlew(sar_object_struct *obj_ptr);
void SARSimSetSlew(
	sar_object_struct *obj_ptr, int enter_slew
);
int SARSimDoPickUpHuman(
	sar_scene_struct *scene,
	sar_object_struct *obj_ptr,
	sar_object_human_struct *obj_human_ptr,
	int human_obj_num
);
int SARSimDoHoistIn(
	sar_core_struct *core_ptr, sar_object_struct *obj_ptr
);
int SARSimBoardObject(
	sar_core_struct *core_ptr,
	sar_object_struct *tar_obj_ptr, int src_obj_num
);
void SARSimSetFlyByPosition(
	sar_scene_struct *scene,
	sar_object_struct ***ptr,
	int *total,
	sar_object_struct *obj_ptr,
	sar_position_struct *pos_result
);


#define SQRT(x)		(((x) > 0.0f) ? sqrt(x) : 0.0f)

#define ATOI(s)         (((s) != NULL) ? atoi(s) : 0)
#define ATOL(s)         (((s) != NULL) ? atol(s) : 0)
#define ATOF(s)         (((s) != NULL) ? atof(s) : 0.0f)
#define STRDUP(s)       (((s) != NULL) ? strdup(s) : NULL)

#define MAX(a,b)        (((a) > (b)) ? (a) : (b))
#define MIN(a,b)        (((a) < (b)) ? (a) : (b))
#define CLIP(a,l,h)     (MIN(MAX((a),(l)),(h)))

#define RADTODEG(r)     ((r) * 180.0 / PI)
#define DEGTORAD(d)     ((d) * PI / 180.0)


/*
 *      Returns the flat (XY plane) contact bounds radius for
 *      the given object.
 *
 *      Can return 0.0 if contact bounds are not availbale for
 *      the object.
 */
float SARSimGetFlatContactRadius(sar_object_struct *obj_ptr)
{
	float contact_radius = 0.0f;
	const sar_contact_bounds_struct *cb = (obj_ptr != NULL) ?
	    obj_ptr->contact_bounds : NULL;
	if(cb == NULL)
	    return(contact_radius);

	switch(cb->contact_shape)
	{
	  case SAR_CONTACT_SHAPE_SPHERICAL:
	  case SAR_CONTACT_SHAPE_CYLENDRICAL:
	    contact_radius = cb->contact_radius;
	    break;

	  case SAR_CONTACT_SHAPE_RECTANGULAR:
	    contact_radius = MAX(
		cb->contact_x_max - cb->contact_x_min,
		cb->contact_y_max - cb->contact_y_min
	    );
	    break;
	}

	return(MAX(contact_radius, 0.0f));
}

/*
 *	Returns the pointer to the object in the give nobject list that
 *	contains an FDM that matches the given fdm or NULL on error.
 *
 *      If index is not NULL then the index to the object number on
 *      the objects list will be set or -1 on failed match.
 */
sar_object_struct *SARSimMatchObjectFromFDM(
	sar_object_struct **list, int total,
	SFMModelStruct *fdm, int *index
)
{
	int i;
	sar_object_struct *obj_ptr;
	sar_object_aircraft_struct *obj_aircraft_ptr;

	if(index != NULL)
	    *index = -1;

	if((list == NULL) || (fdm == NULL))
	    return(NULL);

	/* Iterate through all objects */  
	for(i = 0; i < total; i++)
	{
	    obj_ptr = list[i]; 
	    if(obj_ptr == NULL)
		continue;

	    switch(obj_ptr->type)
	    {
	      case SAR_OBJ_TYPE_GARBAGE:
	      case SAR_OBJ_TYPE_STATIC:
	      case SAR_OBJ_TYPE_AUTOMOBILE:
	      case SAR_OBJ_TYPE_WATERCRAFT:
		break;
	      case SAR_OBJ_TYPE_AIRCRAFT:
		obj_aircraft_ptr = SAR_OBJ_GET_AIRCRAFT(obj_ptr);
		if(obj_aircraft_ptr != NULL)
		{
		    if(obj_aircraft_ptr->fdm == fdm)
		    {
			if(index != NULL)
			    *index = i;
			return(obj_ptr);
		    }
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
	}

	return(NULL);
}


/*
 *	Returns the actual throttle output based on the given
 *	values.
 */
float SARSimThrottleOutputCoeff(
	int flight_model_type,
	float throttle,
	float collective, float collective_range
)
{
	switch(flight_model_type)
	{
	  case SAR_FLIGHT_MODEL_AIRPLANE:
	    return(throttle);
	    break;

	  case SAR_FLIGHT_MODEL_HELICOPTER:
	    if(collective_range >= 0.0f)
		return(CLIP(
		    (throttle + (throttle * collective * collective_range))
			/ (1.0f + collective_range),
		    0.0f, 1.0f
		));
	    else
		return(throttle);
	    break;

	  default:	/* Slew, return throttle as given */
	    return(throttle);
	    break;
	}
}

/*
 *	Returns the speed_stall in meters per cycle on the given
 *	aircraft, applying any modifications by flaps.
 */
float SARSimStallSpeed(
	const sar_object_aircraft_struct *obj_aircraft_ptr
)
{
	if(obj_aircraft_ptr == NULL)
	    return(0.0f);

	/* Flaps fully retracted? */
	if(obj_aircraft_ptr->flaps_position <= 0.0f)
	{
	    return(obj_aircraft_ptr->speed_stall);
	}
	else
	{
	    /* Flaps are deployed, so reduce the stall speed by the
	     * amount the flaps are deployed times the effective
	     * stall speed modification of the flaps.
	     */
	    return(MAX(
		obj_aircraft_ptr->speed_stall - (
		    obj_aircraft_ptr->flaps_position *
		    obj_aircraft_ptr->flaps_stall_offset
		), 0.0f
	    ));
	}
}


/*
 *	Moves the given object obj_ptr relative to the object ref_obj_num
 *	if it is valid with respect to the given offset_pos and offset_dir
 *	(both relative to ref_obj_num).
 *
 *	If ref_obj_num is invalid then no operation will be performed.
 *
 *	This function automatically calls SARSimWarpObject() afterwards.
 */
void SARSimWarpObjectRelative(
	sar_scene_struct *scene, sar_object_struct *obj_ptr,
	sar_object_struct **ptr, int total, int ref_obj_num,
	sar_position_struct *offset_pos,
	sar_direction_struct *offset_dir
)
{
	double a[3 * 1], r[3 * 1];
	sar_position_struct *pos, *ref_pos;
	sar_direction_struct *dir, *ref_dir;
	sar_object_struct *ref_obj_ptr;

	if((scene == NULL) || (obj_ptr == NULL))
	    return;

	/* Check if reference object is valid */
	if((ref_obj_num >= 0) && (ref_obj_num < total))
	    ref_obj_ptr = ptr[ref_obj_num];
	else
	    ref_obj_ptr = NULL;

	/* Got a reference object? */
	if(ref_obj_ptr == NULL)
	    return;

	/* Get pointers to position of reference object */
	ref_pos = &ref_obj_ptr->pos;
	ref_dir = &ref_obj_ptr->dir;

	/* Set up and transform offset matrix a */
	if(offset_pos != NULL)
	{
	    a[0] = offset_pos->x;
	    a[1] = offset_pos->y;
	    a[2] = offset_pos->z;
	}
	else
	{
	    a[0] = 0.0;
	    a[1] = 0.0;
	    a[2] = 0.0;
	}

	/* Rotate the offset matrix a to get rotated offset matrix r */
	MatrixRotateBank3(a, -ref_dir->bank, r);	/* Our bank is negative, so
							 * pass as flipped sign */
	MatrixRotatePitch3(r, ref_dir->pitch, a);
	MatrixRotateHeading3(a, ref_dir->heading, r);

	/* Move the specified object to the new offset position relative
	 * to the reference object
	 */
	pos = &obj_ptr->pos;
	pos->x = (float)(ref_pos->x + r[0]);
	pos->y = (float)(ref_pos->y + r[1]);
	pos->z = (float)(ref_pos->z + r[2]);

	/* Rotate the given object by the relative object's direction
	 * added with the offset direction
	 */
	dir = &obj_ptr->dir;
	if(offset_dir != NULL)
	{
	    dir->heading = (float)SFMSanitizeRadians(
		ref_dir->heading + offset_dir->heading
	    );
	    dir->pitch = (float)SFMSanitizeRadians( 
		ref_dir->pitch + offset_dir->pitch
	    );
	    dir->bank = (float)SFMSanitizeRadians(
		ref_dir->bank + offset_dir->bank
	    );
	}
	else
	{
	    memcpy(dir, ref_dir, sizeof(sar_direction_struct));
	}

	/* Realize new position of the given object */
	SARSimWarpObject(scene, obj_ptr, pos, dir);
}



/*
 *	Updates (realizes) all position and attitude values for the
 *	given object to the specified new position and direction.
 *
 *	If any input is NULL then the values will be left unchanged.
 *
 *	If the given input pointers match the pointers of the structures
 *	on the object then the structure in question will be left
 *	unchanged.
 *
 *	Rotatation values for the contact bounds and ground object
 *	structure will be updated as needed.
 */
void SARSimWarpObject(
	sar_scene_struct *scene, sar_object_struct *obj_ptr,  
	sar_position_struct *new_pos,
	sar_direction_struct *new_dir
)
{
	sar_object_aircraft_struct *obj_aircraft_ptr = NULL;
	sar_object_ground_struct *obj_ground_ptr = NULL;
	sar_contact_bounds_struct *cb;
	SFMModelStruct *fdm = NULL;


	if((scene == NULL) || (obj_ptr == NULL))
	    return;

	if((new_pos != NULL) && (new_pos != &obj_ptr->pos))
	{
	    obj_ptr->pos.x = new_pos->x;
	    obj_ptr->pos.y = new_pos->y;
	    obj_ptr->pos.z = new_pos->z;
	}
	if((new_dir != NULL) && (new_dir != &obj_ptr->dir))
	{
	    obj_ptr->dir.heading = new_dir->heading;
	    obj_ptr->dir.pitch = new_dir->pitch;
	    obj_ptr->dir.bank = new_dir->bank;
	}

	switch(obj_ptr->type)
	{
	  case SAR_OBJ_TYPE_GARBAGE:
	  case SAR_OBJ_TYPE_STATIC:
	  case SAR_OBJ_TYPE_AUTOMOBILE:
	  case SAR_OBJ_TYPE_WATERCRAFT:
	    break;
	  case SAR_OBJ_TYPE_AIRCRAFT:
	    obj_aircraft_ptr = SAR_OBJ_GET_AIRCRAFT(obj_ptr);
	    if(obj_aircraft_ptr != NULL)
	    {
		/* Get pointer to SFM flight dynamics structure */
		fdm = obj_aircraft_ptr->fdm;
	    }
	    break;
	  case SAR_OBJ_TYPE_GROUND:
	    obj_ground_ptr = SAR_OBJ_GET_GROUND(obj_ptr);
	    if(obj_ground_ptr != NULL)
	    {
		/* Update trig functions for ground's heightfield
		 * rotation
		 */
		if(new_dir != NULL)
		{
		    obj_ground_ptr->cos_heading = (float)cos(-new_dir->heading);
		    obj_ground_ptr->sin_heading = (float)sin(-new_dir->heading);
		}
	    }
	    break;
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

	/* Set values on FDM if exists */
	if(fdm != NULL)
	{
	    if(new_pos != NULL)
	    {
		fdm->position.x = new_pos->x;
		fdm->position.y = new_pos->y;
		fdm->position.z = new_pos->z;
	    }
	    if(new_dir != NULL)
	    {
		fdm->direction.heading = new_dir->heading;
		fdm->direction.pitch = new_dir->pitch;
		fdm->direction.bank = new_dir->bank;
	    }
	}

	/* Update values on contact bounds */
	cb = obj_ptr->contact_bounds;
	if(cb != NULL)
	{
	    /* Update values based on contact shape */
	    if(cb->contact_shape == SAR_CONTACT_SHAPE_RECTANGULAR)
	    {
		/* For rectangular contact bounds, the cos and sin
		 * values for the heading need to be updated based
		 * on the new rotation.
		 */
		if(new_dir != NULL)
		{
		    cb->cos_heading = (float)cos(-new_dir->heading);
		    cb->sin_heading = (float)sin(-new_dir->heading);
		}
	    }

	}
}

/*
 *	Updates the animated position of the given object part
 *	structure.  This does not update parts that respond to control
 *	position such as ailerons, rudders, and elevators.
 */
void SARSimUpdatePart(
	sar_scene_struct *scene,
	sar_object_struct *obj_ptr, sar_obj_part_struct *part_ptr,
	snd_recorder_struct *recorder, int play_sound, int ear_in_cockpit
)
{
	int type;
	sar_obj_flags_t flags;


	if((scene == NULL) || (obj_ptr == NULL) || (part_ptr == NULL))
	    return;

	type = part_ptr->type;
	flags = part_ptr->flags;

	/* Handle by object part type */
	switch(type)
	{
	  case SAR_OBJ_PART_TYPE_FLAP:
/* TODO */
	    break;

	  case SAR_OBJ_PART_TYPE_AIR_BRAKE:
	    /* Air brakes deploying/deployed? */
	    if(flags & SAR_OBJ_PART_FLAG_STATE)
	    {
		/* Going to max */
		sar_grad_anim_t prev_pos;

		/* Already deployed? */
		if(part_ptr->anim_pos == (sar_grad_anim_t)-1)
		    break;

		prev_pos = part_ptr->anim_pos;

		/* Increase animation position value */
		part_ptr->anim_pos = (sar_grad_anim_t)MIN(
		    (int)part_ptr->anim_pos +
		    ((int)part_ptr->anim_rate * time_compensation *
			time_compression),
		    (sar_grad_anim_t)-1
		);
	    }
	    /* Air brakes closing/closed? */
	    else
	    {
		/* Going to min */
		sar_grad_anim_t prev_pos;

		/* Already closed? */
		if(part_ptr->anim_pos == 0)
		    break;

		prev_pos = part_ptr->anim_pos;

		/* Decrease animation position value */
		part_ptr->anim_pos = (sar_grad_anim_t)MAX(
		    (int)part_ptr->anim_pos -
			((int)part_ptr->anim_rate * time_compensation *
			time_compression),
		    0
		);
	    }
	    break;

	  case SAR_OBJ_PART_TYPE_DOOR:
	  case SAR_OBJ_PART_TYPE_DOOR_RESCUE:
	  case SAR_OBJ_PART_TYPE_CANOPY:
	    /* Door opening? */
	    if(flags & SAR_OBJ_PART_FLAG_STATE)
	    {
		/* Opening */
		sar_grad_anim_t prev_pos;

		/* Already opened? */
		if(part_ptr->anim_pos == (sar_grad_anim_t)-1)
		    break;

		/* Door fixed and cannot be opened? */
		if(flags & SAR_OBJ_PART_FLAG_DOOR_FIXED)
		    break;

		prev_pos = part_ptr->anim_pos;

		/* Increase animation position value */
		part_ptr->anim_pos = (sar_grad_anim_t)MIN(
		    (int)part_ptr->anim_pos +
		    ((int)part_ptr->anim_rate * time_compensation *
			time_compression),
		    (int)((sar_grad_anim_t)-1)
		);

		/* Play sound if just started to move? */
		if(play_sound && (prev_pos == 0))
		    SARSoundSourcePlayFromList(
			recorder,
			obj_ptr->sndsrc, obj_ptr->total_sndsrcs,
			ear_in_cockpit ?
			    "door_open_inside" : "door_open",
			&obj_ptr->pos, &obj_ptr->dir,
			&scene->ear_pos
		    );
	    }
	    else
	    {
		/* Closing */
		sar_grad_anim_t prev_pos;

		/* Door already closed? */
		if(part_ptr->anim_pos == 0)
		    break;

		/* Door fixed and cannot be closed? */
		if(flags & SAR_OBJ_PART_FLAG_DOOR_FIXED)
		    break;

		prev_pos = part_ptr->anim_pos;

		/* Decrease animation position value */
		part_ptr->anim_pos = (sar_grad_anim_t)MAX(
		    (int)part_ptr->anim_pos -
		    ((int)part_ptr->anim_rate * time_compensation *
			time_compression),
		    0
		);

		/* Play sound if just closed? */
		if(play_sound && (part_ptr->anim_pos == 0))
		    SARSoundSourcePlayFromList(
			recorder,
			obj_ptr->sndsrc, obj_ptr->total_sndsrcs,
			ear_in_cockpit ?
			    "door_close_inside" : "door_close",
			&obj_ptr->pos, &obj_ptr->dir,
			&scene->ear_pos
		    );
	    }
	    break;

	  case SAR_OBJ_PART_TYPE_LANDING_GEAR:
	    /* When the state flag is set, it means the landing gear is
	     * deployed. However the deployed position is considered the
	     * minimum position (unlike other object part types).
	     */
	    if(flags & SAR_OBJ_PART_FLAG_STATE)
	    {
		/* Lowering landing gear */

		if(part_ptr->anim_pos == 0)
		    break;

		/* Do not change animation position if landing gear is
		 * fixed or damaged.
		 */
		if((flags & SAR_OBJ_PART_FLAG_LGEAR_FIXED) ||
		   (flags & SAR_OBJ_PART_FLAG_LGEAR_DAMAGED)
		)
		    break;

		/* Decrease animation position value */
		part_ptr->anim_pos = (sar_grad_anim_t)MAX(
		    (int)part_ptr->anim_pos -
		    ((int)part_ptr->anim_rate * time_compensation *
			time_compression),
		    0
		);
	    }
	    else
	    {
		/* Retracting landing gears */

		if(part_ptr->anim_pos == (sar_grad_anim_t)-1)
		    break;

		/* Do not change animation position if landing gear is
		 * fixed or damaged.
		 */
		if((flags & SAR_OBJ_PART_FLAG_LGEAR_FIXED) ||
		   (flags & SAR_OBJ_PART_FLAG_LGEAR_DAMAGED)
		)
		    break;

		/* Increase animation position value */
		part_ptr->anim_pos = (sar_grad_anim_t)MIN(
		    (int)part_ptr->anim_pos +
		    ((int)part_ptr->anim_rate * time_compensation *
			time_compression),
		    (int)((sar_grad_anim_t)-1)
		);
	    }
	    break;

	}
}

/*
 *	Deploys or retracts air brakes on the object specified by
 *	obj_ptr.
 *
 *	This marks all air brake parts on the object to start deploying
 *	or retracting.
 */
void SARSimOpAirBrakes(
	sar_scene_struct *scene,
	sar_object_struct ***ptr, int *total,
	sar_object_struct *obj_ptr, int air_brakes_state,
	snd_recorder_struct *recorder, int play_sound, int ear_in_cockpit
)
{
	int i, cur_state = -1, oped = 0;
	sar_obj_part_struct *air_brake;
	sar_object_aircraft_struct *obj_aircraft_ptr;


	if((obj_ptr == NULL) || (air_brakes_state < 0))
	    return;

	/* Check object type specific values to see if operation to the
	 * air brakes can be made
	 */
	switch(obj_ptr->type)
	{
	  case SAR_OBJ_TYPE_GARBAGE:
	  case SAR_OBJ_TYPE_STATIC:
	  case SAR_OBJ_TYPE_AUTOMOBILE:
	  case SAR_OBJ_TYPE_WATERCRAFT:
	    break;
	  case SAR_OBJ_TYPE_AIRCRAFT:
	    obj_aircraft_ptr = SAR_OBJ_GET_AIRCRAFT(obj_ptr);
	    if(obj_aircraft_ptr != NULL)
	    {
		/* Cannot deploy or retract air brakes if the
		 * aircraft is not flyable
		 */
		if(obj_aircraft_ptr->air_worthy_state == SAR_AIR_WORTHY_NOT_FLYABLE)
		    return;

		/* Already deployed or retracted? */
		if(obj_aircraft_ptr->air_brakes_state == air_brakes_state)
		    return;
		else
		    obj_aircraft_ptr->air_brakes_state = air_brakes_state;
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

	/* Iterate through all air brakes */
	for(i = 0; 1; i++)
	{
	    air_brake = SARObjGetPartPtr(
		obj_ptr, SAR_OBJ_PART_TYPE_AIR_BRAKE, i
	    );
	    if(air_brake == NULL)
		break;

	    /* Need to get initial state? */
	    if(cur_state == -1)
		cur_state = (air_brake->flags & SAR_OBJ_PART_FLAG_STATE) ?
		    1 : 0;

	    /* Is retracted and we want to deploy it? */
	    if(!cur_state && air_brakes_state)
	    {
		/* Currently retracted so deploy it */
		air_brake->flags |= SAR_OBJ_PART_FLAG_STATE;
		oped = 1;
	    }
	    /* Is deployed and we want to retract it? */
	    else if(cur_state && !air_brakes_state)
	    {
		/* Currently deployed so retract it */
		air_brake->flags &= ~SAR_OBJ_PART_FLAG_STATE;
		oped = 1;
	    }
	}

	/* Play air brakes sound? Note that we play the air brakes
	 * sound here since SARSimUpdatePart() that does the actual
	 * air brakes parts updating will play sounds for multiple
	 * air brakes and we don't want that.
	 */
	if(play_sound && oped)
	{
	    if(air_brakes_state)
		SARSoundSourcePlayFromList(
		    recorder,
		    obj_ptr->sndsrc, obj_ptr->total_sndsrcs,
		    ear_in_cockpit ?
			"air_brake_out_inside" : "air_brake_out",
		    &obj_ptr->pos, &obj_ptr->dir,
		    &scene->ear_pos
		);
	    else
		SARSoundSourcePlayFromList(
		    recorder,
		    obj_ptr->sndsrc, obj_ptr->total_sndsrcs,
		    ear_in_cockpit ?
			"air_brake_in_inside" : "air_brake_in",
		    &obj_ptr->pos, &obj_ptr->dir,
		    &scene->ear_pos
		);
	}
}

/*
 *	Raise or lower landing gear on the object specified by obj_ptr.
 *
 *	This marks all landing gear parts on the object to start
 *	lowering or raising.
 */
void SARSimOpLandingGear(
	sar_scene_struct *scene,
	sar_object_struct ***ptr, int *total,
	sar_object_struct *obj_ptr, int gear_state,
	snd_recorder_struct *recorder, int play_sound, int ear_in_cockpit
)
{
	int i, cur_state = -1, oped = 0;
	sar_obj_part_struct *gear;
	sar_object_aircraft_struct *obj_aircraft_ptr;


	if((obj_ptr == NULL) || (gear_state < 0))
	    return;

	/* Input state can be 0 or 1, for landing gear state values
	 * have the following meanings:
	 * -1	non-existant
	 * 0	up/retracted/in
	 * 1	down/deployed/out
	 * 2	down/deployed/out and fixed
	 */

	/* Check object type specific values to see if operation to the
	 * landing gears can be made.
	 */
	obj_aircraft_ptr = SAR_OBJ_GET_AIRCRAFT(obj_ptr);
	if(obj_aircraft_ptr != NULL)
	{
	    /* Cannot raise or lower landing gears while landed
	     * or if landing gears are fixed.
	     */
	    if(obj_aircraft_ptr->landed ||
	       (obj_aircraft_ptr->landing_gear_state < 0) ||
	       (obj_aircraft_ptr->landing_gear_state == 2)
	    )
		return;

	    /* No change in landing gear state? */
	    if(gear_state == obj_aircraft_ptr->landing_gear_state)
		return;

	    /* Set new gear state (must be 0 or 1) */
	    switch(gear_state)
	    {
	      case 1:
		obj_aircraft_ptr->landing_gear_state = 1;
		break;
	      default:
		obj_aircraft_ptr->landing_gear_state = 0;
		break;
	    }
	}


	/* Iterate through all landing gears */
	for(i = 0; 1; i++)
	{
	    gear = SARObjGetPartPtr(
		obj_ptr, SAR_OBJ_PART_TYPE_LANDING_GEAR, i
	    );
	    if(gear == NULL)
		break;

	    /* Need to get initial gear state? */
	    if(cur_state == -1)
		cur_state = (gear->flags & SAR_OBJ_PART_FLAG_STATE) ?
		    1 : 0;

	    /* Is gear up and we want to lower it? */
	    if(!cur_state && gear_state)
	    {
		/* Landing gear is currently up and we want to lower it */
		gear->flags |= SAR_OBJ_PART_FLAG_STATE;
		oped = 1;
	    }
	    /* Is gear down and we want to raise it? */
	    else if(cur_state && !gear_state)
	    {
		/* Landing gear is currently down and we want to raise it */
		if(!(gear->flags & SAR_OBJ_PART_FLAG_LGEAR_FIXED))
		{
		    gear->flags &= ~SAR_OBJ_PART_FLAG_STATE;
		    oped = 1;
		}
	    }
	}

	/* Play landing gear sound? Note that we play the landing
	 * gear sound here since SARSimUpdatePart() that does the
	 * actual landing gear parts updating will play sounds for
	 * multiple landing gears and we don't want that.
	 */
	if(play_sound && oped)
	{
	    if(gear_state)
		SARSoundSourcePlayFromList(
		    recorder,
		    obj_ptr->sndsrc, obj_ptr->total_sndsrcs,
		    ear_in_cockpit ?
			"gear_down_inside" : "gear_down",
		    &obj_ptr->pos, &obj_ptr->dir,
		    &scene->ear_pos
		);
	    else
		SARSoundSourcePlayFromList(
		    recorder,
		    obj_ptr->sndsrc, obj_ptr->total_sndsrcs,
		    ear_in_cockpit ?
			"gear_up_inside" : "gear_up",
		    &obj_ptr->pos, &obj_ptr->dir,
		    &scene->ear_pos
		);
	}
}

/*
 *	Open or close the first rescue door on the given object.
 *
 *	This marks the door to either start opening or closing.
 *
 *	If the door is to be closed and has the SAR_DOOR_FLAG_STAY_OPEN
 *	flag set, then this flag will be unset.
 */
void SARSimOpDoorRescue(
	sar_scene_struct *scene,
	sar_object_struct ***ptr, int *total,
	sar_object_struct *obj_ptr,
	int state		/* 0 = close, 1 = open */
)
{
	int i;
	sar_obj_part_struct *door_ptr;


	if(obj_ptr == NULL)
	    return;

	/* Iterate through all rescue doors */
	for(i = 0; 1; i++)
	{
	    door_ptr = SARObjGetPartPtr(
		obj_ptr, SAR_OBJ_PART_TYPE_DOOR_RESCUE, i
	    );
	    if(door_ptr == NULL)
		break;

	    /* Door is opened, we want to close it? */
	    if((door_ptr->flags & SAR_OBJ_PART_FLAG_STATE) &&
	       !state
	    )
	    {
		if(!(door_ptr->flags & SAR_OBJ_PART_FLAG_DOOR_FIXED))
		{
		    door_ptr->flags &= ~SAR_OBJ_PART_FLAG_STATE;
		    door_ptr->flags &= ~SAR_OBJ_PART_FLAG_DOOR_STAY_OPEN;
		}
	    }
	    /* Door is closed, we want to open it? */
	    else if(!(door_ptr->flags & SAR_OBJ_PART_FLAG_STATE) &&
		    state
	    )
	    {
		if(!(door_ptr->flags & SAR_OBJ_PART_FLAG_DOOR_FIXED) &&
		   !(door_ptr->flags & SAR_OBJ_PART_FLAG_DOOR_LOCKED)
		)
		    door_ptr->flags |= SAR_OBJ_PART_FLAG_STATE;
	    }
	}

	/* Let SARSimUpdatePart() play door sound */

}

/*
 *	Updates control positions on the rotor in the given list as
 *	needed in accordance with the given control positions pitch
 *	and bank which must be in the range of -1.0 to 1.0.
 *
 *	All inputs assumed valid.
 */
void SARSimRotorUpdateControls(
	sar_obj_rotor_struct *rotor_ptr,
	float pitch, float bank
)
{
	if(rotor_ptr->flags & SAR_ROTOR_FLAG_FOLLOW_CONTROLS)
	{
	    rotor_ptr->control_coeff_pitch = pitch;
	    rotor_ptr->control_coeff_bank = bank;
	}
}

/*
 *	Returns the engine state of the object.
 *
 *	If object is not a type that has engines, then
 *	SAR_ENGINE_OFF is returned.
 */
sar_engine_state SARSimGetEngineState(sar_object_struct *obj_ptr)
{
	sar_object_aircraft_struct *obj_aircraft_ptr;

	if(obj_ptr == NULL)
	    return(SAR_ENGINE_OFF);

	switch(obj_ptr->type)
	{
	  case SAR_OBJ_TYPE_GARBAGE:
	  case SAR_OBJ_TYPE_STATIC:
	  case SAR_OBJ_TYPE_AUTOMOBILE:
	  case SAR_OBJ_TYPE_WATERCRAFT:
	    break;
	  case SAR_OBJ_TYPE_AIRCRAFT:
	    obj_aircraft_ptr = SAR_OBJ_GET_AIRCRAFT(obj_ptr);
	    if(obj_aircraft_ptr != NULL)
		return(obj_aircraft_ptr->engine_state);
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

	return(SAR_ENGINE_OFF);
}

/*
 *	Turns engine off or starts engine, depending on 
 *	engine_state which can be either SAR_ENGINE_ON or
 *	SAR_ENGINE_OFF.
 *
 *	When passing engine_state as SAR_ENGINE_ON, it does not
 *	actually turn the engine on but rather sets engine_state on
 *	the object to initialize (start) and sets the initialization
 *	time depending on the current throttle position.
 */
void SARSimOpEngine(
	sar_scene_struct *scene,
	sar_object_struct ***ptr, int *total,
	sar_object_struct *obj_ptr, sar_engine_state engine_state,
	snd_recorder_struct *recorder, char play_sound, char ear_in_cockpit
)
{
	char turned_engine_on = 0, turned_engine_off = 0;
	time_t engine_start_delay = 5000l;	/* In ms */
	sar_object_aircraft_struct *obj_aircraft_ptr;
  
 
	if(obj_ptr == NULL)
	    return;

	/* Request to start engine as init is the same as on */
	if(engine_state == SAR_ENGINE_INIT)
	    engine_state = SAR_ENGINE_ON;

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

	    /* Cannot turn engine on if not flyable */
	    if((obj_aircraft_ptr->air_worthy_state != SAR_AIR_WORTHY_FLYABLE) &&
	       (engine_state == SAR_ENGINE_ON)
	    )
		break;

	    /* Turn engine on and engine is currently off? */
	    if((engine_state == SAR_ENGINE_ON) &&
	       (obj_aircraft_ptr->engine_state == SAR_ENGINE_OFF)
	    )
	    {
		obj_aircraft_ptr->engine_state = SAR_ENGINE_INIT;

		/* Check if landed, if landed use normal engine start
	         * delay, otherwise start engine immediatly
		 */
		if(obj_aircraft_ptr->landed)
		    obj_aircraft_ptr->next_engine_on = cur_millitime +
			(time_t)(engine_start_delay *
			    CLIP(1.0 - obj_aircraft_ptr->throttle, 0.0, 1.0)
			);
		else
		    obj_aircraft_ptr->next_engine_on = cur_millitime;

		turned_engine_on = 1;
	    }
	    /* Turn engine off? */
	    else if(engine_state == SAR_ENGINE_OFF)
	    {
		obj_aircraft_ptr->engine_state = SAR_ENGINE_OFF;
		obj_aircraft_ptr->next_engine_on = 0l;
		turned_engine_off = 1;
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


	/* Play landing gear sound? */
	if(play_sound)
	{
	    if(turned_engine_on)
		SARSoundSourcePlayFromList(
		    recorder,
		    obj_ptr->sndsrc, obj_ptr->total_sndsrcs,
		    ear_in_cockpit ?
			"engine_start_inside" : "engine_start",
		    &obj_ptr->pos, &obj_ptr->dir,
		    &scene->ear_pos
		);
	    else if(turned_engine_off)
		SARSoundSourcePlayFromList(
		    recorder,
		    obj_ptr->sndsrc, obj_ptr->total_sndsrcs,
		    ear_in_cockpit ?
			"engine_shutdown_inside" : "engine_shutdown",
		    &obj_ptr->pos, &obj_ptr->dir,
		    &scene->ear_pos
		);
	}
}

/*
 *	Pitches engines up (if pitch_state is 0) or forwards (if 
 *	pitch_state is 1).
 *
 *	All rotors will be marked to begin pitching towards the newly
 *	specified pitch_state.
 */
void SARSimPitchEngine(
	sar_scene_struct *scene,
	sar_object_struct ***ptr, int *total,
	sar_object_struct *obj_ptr, int pitch_state
)
{
	sar_obj_rotor_struct **rotor = NULL, *rotor_ptr;
	char can_pitch = 0;
	int total_rotors = 0;
	sar_object_aircraft_struct *obj_aircraft_ptr;


	if(obj_ptr == NULL)
	    return;

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

	    /* Engines cannot pitch? */
	    if(obj_aircraft_ptr->engine_can_pitch != 1)
		break;

	    /* Cannot pitch engine on if not flyable */
	    if(obj_aircraft_ptr->air_worthy_state == SAR_AIR_WORTHY_NOT_FLYABLE)
		break;

	    /* Get list of rotors */
	    rotor = obj_aircraft_ptr->rotor;
	    total_rotors = obj_aircraft_ptr->total_rotors;
	    can_pitch = 1;

	    /* Iterate through rotors, checking if one does not allow
	     * pitch when landed
	     */
	    if(obj_aircraft_ptr->landed)
	    {
		int i;

		for(i = 0; i < total_rotors; i++)
		{
		    rotor_ptr = rotor[i];
		    if(rotor_ptr == NULL)
			continue;

		    if(rotor_ptr->flags & SAR_ROTOR_FLAG_NO_PITCH_LANDED)
		    {
			/* Landed and this rotor does not allow pitch when
			 * landed
			 */
			total_rotors = 0;
			rotor = NULL;
			can_pitch = 0;
			break;
		    }
		}
	    }
	    if(!can_pitch)
		break;

	    /* Set new flight model */
	    if(pitch_state == 1)
	    {
		obj_aircraft_ptr->flight_model_type =
		    SAR_FLIGHT_MODEL_AIRPLANE;
	    }
	    else
	    {
		obj_aircraft_ptr->flight_model_type =
		    SAR_FLIGHT_MODEL_HELICOPTER;
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

	/* Iterate through each valid rotor found on the object and
	 * set the new pitch state
	 */
	if(can_pitch)
	{
	    int i;
	    for(i = 0; i < total_rotors; i++)
	    {
	        rotor_ptr = rotor[i];
	        if(rotor_ptr == NULL)
		    continue;
	        if(rotor_ptr->flags & SAR_ROTOR_FLAG_CAN_PITCH)
	        {
		    if(pitch_state)
		        rotor_ptr->flags |= SAR_ROTOR_FLAG_PITCH_STATE;
		    else
		        rotor_ptr->flags &= ~SAR_ROTOR_FLAG_PITCH_STATE;
	        }
	    }
	}
}


/*
 *	Updates all lights on the object.
 */
void SARSimUpdateLights(sar_object_struct *obj_ptr)
{
	int i;
	sar_light_struct *light;

	if(obj_ptr == NULL)
	    return;

	for(i = 0; i < obj_ptr->total_lights; i++)
	{
	    light = obj_ptr->light[i];
	    if(light == NULL)
		continue;

	    /* Strobe light? */
	    if(light->flags & SAR_LIGHT_FLAG_STROBE)
	    {
		if(light->next_on > 0)
		{
		    /* Waiting to be turned on, implies it's currently
		     * off.  Now check if it's time to turn it on.
		     */
		    if(light->next_on <= cur_millitime)
		    {
			light->next_on = 0;
			light->next_off = cur_millitime +
			    light->int_on;
		    }
		    else
		    {
			continue;
		    }
		}
		else
		{
		    /* Waiting to be turned off, implies it's currently
		     * on.  Now check if it's time to turn it off.
		     */
		    if(light->next_off <= cur_millitime)
		    {
			light->next_on = cur_millitime +
			    light->int_off;
			light->next_off = 0;
			continue;
		    }
		}
	    }
	}
}

/*
 *	Returns the state of the first light structure on the object.
 *
 *	Can return false if no lights exist or error.
 */
int SARSimGetLightsState(sar_object_struct *obj_ptr)
{
	int i;
	const sar_light_struct *light;


	if(obj_ptr == NULL)
	    return(0);

	for(i = 0; i < obj_ptr->total_lights; i++)
	{
	    light = obj_ptr->light[i];
	    if(light == NULL)
		continue;

	    /* Is it a strobe light? */
	    if(!(light->flags & SAR_LIGHT_FLAG_STROBE) &&
	       !(light->flags & SAR_LIGHT_FLAG_ATTENUATE)
	    )
		return(light->flags & SAR_LIGHT_FLAG_ON);
	}

	return(0);
}

/*
 *	Turns all regular lights on object on or off.
 */
void SARSimOpLights(
	sar_object_struct *obj_ptr, int state
)
{
	int i;
	sar_light_struct *light;

	if(obj_ptr == NULL)
	    return;

	for(i = 0; i < obj_ptr->total_lights; i++)
	{
	    light = obj_ptr->light[i];
	    if(light == NULL)
		continue;

	    /* Skip non-standard lights */
	    if((light->flags & SAR_LIGHT_FLAG_STROBE) ||
	       (light->flags & SAR_LIGHT_FLAG_ATTENUATE)
	    )
		continue;

	    if(state)
	    {
		/* Turn on */
		light->flags |= SAR_LIGHT_FLAG_ON;
	    }
	    else
	    {
		/* Turn off */
		light->flags &= ~SAR_LIGHT_FLAG_ON;
	    }
	}
}

/*
 *      Returns the state of the first strobe light structure on the
 *	object.
 *
 *      Can return false if no lights exist or error.
 */
int SARSimGetStrobesState(sar_object_struct *obj_ptr)
{
	int i;
	const sar_light_struct *light;


	if(obj_ptr == NULL)
	    return(0);

	for(i = 0; i < obj_ptr->total_lights; i++)
	{
	    light = obj_ptr->light[i];
	    if(light == NULL)
		continue;

	    /* Is it a strobe light? */
	    if(light->flags & SAR_LIGHT_FLAG_STROBE)
		return(light->flags & SAR_LIGHT_FLAG_ON);
	}

	return(0);
}

/*
 *	Turns all strobe lights on object on or off.
 */
void SARSimOpStrobes(
	sar_object_struct *obj_ptr, int state
)
{
	int i;
	sar_light_struct *light;

	if(obj_ptr == NULL)
	    return;

	for(i = 0; i < obj_ptr->total_lights; i++)
	{
	    light = obj_ptr->light[i];
	    if(light == NULL)
		continue;

	    /* Skip non-strobe lights */
	    if(!(light->flags & SAR_LIGHT_FLAG_STROBE))
		continue;

	    if(state)
	    {
		/* Turn on */
		light->flags |= SAR_LIGHT_FLAG_ON;
	    }
	    else
	    {   
		/* Turn off */
		light->flags &= ~SAR_LIGHT_FLAG_ON;
	    }
	}
}

/*
 *      Returns the state of the first attenuation light structure on the
 *      object.
 *
 *      Can return false if no lights exist or error.
 */
int SARSimGetAttenuateState(sar_object_struct *obj_ptr)
{
	int i;
	const sar_light_struct *light;


	if(obj_ptr == NULL)
	    return(0);

	for(i = 0; i < obj_ptr->total_lights; i++)
	{
	    light = obj_ptr->light[i];
	    if(light == NULL)  
		continue;

	    /* Is it a attenuation light? */
	    if(light->flags & SAR_LIGHT_FLAG_ATTENUATE)
		return(light->flags & SAR_LIGHT_FLAG_ON);
	}

	return(0);
}

/*
 *      Turns all attenuation lights on object on or off.
 */
void SARSimOpAttenuate(
	sar_object_struct *obj_ptr, int state
)
{
	int i;
	sar_light_struct *light;

	if(obj_ptr == NULL)
	    return;

	for(i = 0; i < obj_ptr->total_lights; i++)
	{
	    light = obj_ptr->light[i];
	    if(light == NULL)
		continue;

	    /* Skip non-attenuation lights */
	    if(!(light->flags & SAR_LIGHT_FLAG_ATTENUATE))
		continue;

	    if(state)
	    {
		/* Turn on */
		light->flags |= SAR_LIGHT_FLAG_ON;
	    }
	    else
	    {
		/* Turn off */
		light->flags &= ~SAR_LIGHT_FLAG_ON;
	    }
	}
}

/*
 *	Repairs the object if possible.
 *
 *	Does not check if the object is near any repair faclity, the
 *	calling function must do that.
 *
 *      Returns -1 on error, -2 if the object cannot be repaired or
 *      0 on success.
 */
int SARSimOpRepair(
	sar_scene_struct *scene, sar_object_struct *obj_ptr
)
{
	sar_contact_bounds_struct *cb;
	sar_object_aircraft_struct *obj_aircraft_ptr;

	if((scene == NULL) || (obj_ptr == NULL))
	    return(-1);

	switch(obj_ptr->type)
	{
	  case SAR_OBJ_TYPE_AIRCRAFT:
	    obj_aircraft_ptr = SAR_OBJ_GET_AIRCRAFT(obj_ptr);
	    if(obj_aircraft_ptr != NULL)
	    {
		obj_aircraft_ptr->air_worthy_state = SAR_AIR_WORTHY_FLYABLE;
		obj_ptr->hit_points = obj_ptr->hit_points_max;

		/* Reset crash flags */
		cb = obj_ptr->contact_bounds;
		if(cb != NULL)
		{
		    cb->crash_flags = SAR_CRASH_FLAG_CRASH_OTHER;
		}
		/* Return success */
		return(0);
	    }
	    break;

/* Add support for other objects that can be repaired here */
	  default:
	    break;
	}

	return(-2);
}

/*
 *	Refuels the object if possible.
 *
 *      Does not check if the object is near any refueling faclity, the
 *      calling function must do that.
 *
 *	Returns -1 on error, -2 if the object cannot be refueled or
 *	0 on success.
 */
int SARSimOpRefuel(
	sar_scene_struct *scene, sar_object_struct *obj_ptr
)
{
	int eft_num;
	sar_object_aircraft_struct *obj_aircraft_ptr;
	sar_object_fueltank_struct *obj_fueltank_ptr;
	sar_external_fueltank_struct *eft_ptr;

	if((scene == NULL) || (obj_ptr == NULL))
	    return(-1);

	switch(obj_ptr->type)
	{
	  case SAR_OBJ_TYPE_AIRCRAFT:
	    obj_aircraft_ptr = SAR_OBJ_GET_AIRCRAFT(obj_ptr);
	    if(obj_aircraft_ptr != NULL)
	    {
		/* Set current fuel to amount of maximum fuel */
		obj_aircraft_ptr->fuel = obj_aircraft_ptr->fuel_max;
		/* Replace and refuel all external reserved tanks */
		for(eft_num = 0; eft_num < obj_aircraft_ptr->total_external_fueltanks; eft_num++)
		{
		    eft_ptr = obj_aircraft_ptr->external_fueltank[eft_num];
		    if(eft_ptr == NULL)
			continue;

		    eft_ptr->flags |= SAR_EXTERNAL_FUELTANK_FLAG_ONBOARD;
		    eft_ptr->fuel = eft_ptr->fuel_max;
		}
		return(0);
	    }
	    break;

	  case SAR_OBJ_TYPE_FUELTANK:
	    obj_fueltank_ptr = SAR_OBJ_GET_FUELTANK(obj_ptr);
	    if(obj_fueltank_ptr != NULL)
	    {
		/* Set current fuel to amount of maximum fuel */
		obj_fueltank_ptr->fuel = obj_fueltank_ptr->fuel_max;
		return(0);
	    }
	    break;

/* Add support for other objects that can be refueled here */
	  default:
	    break;
	}

	return(-2);
}


/*
 *	Marks the given number of passengers to leave or drop from the
 *	given object as pending. If either value is -1 then the value
 *	in question will be interprited as `all'.
 *
 *	Returns non-zero on error.
 */
int SARSimOpPassengersSetLeave(  
	sar_scene_struct *scene, sar_object_struct *obj_ptr,
	int passengers_leave_pending, int passengers_drop_pending
)
{
	int *crew, *passengers, *passengers_max;
	float *passengers_mass;
	int *cur_passengers_leave_pending, *cur_passengers_drop_pending;


	if((scene == NULL) || (obj_ptr == NULL))
	    return(-1);

	if(SARObjGetOnBoardPtr(
	    obj_ptr,
	    &crew, &passengers, &passengers_max,
	    &passengers_mass,
	    &cur_passengers_leave_pending,
	    &cur_passengers_drop_pending
	))
	    return(-2);

	if(passengers_max == NULL)
	    return(-2);

	/* Set passengers leave pending */
	if(cur_passengers_leave_pending != NULL)
	{
	  if(passengers_leave_pending < 0)
	  {
	    if((cur_passengers_leave_pending != NULL) &&
	       (passengers_max != NULL)
	    )
		*cur_passengers_leave_pending = *passengers_max;
	  }
	  else
	  {
	    if(passengers_max != NULL)
	    {
		if(passengers_leave_pending > *passengers_max)
		    passengers_leave_pending = *passengers_max;

		if(cur_passengers_leave_pending != NULL)
		    *cur_passengers_leave_pending = passengers_leave_pending;
	    }
	  }
	}

	/* Set passengers drop pending */
	if(cur_passengers_drop_pending != NULL)
	{
	    if(passengers_drop_pending < 0)
	    {
		if((cur_passengers_drop_pending != NULL) &&
		   (passengers_max != NULL)
		)
		    *cur_passengers_drop_pending = *passengers_max;
	    }
	    else
	    {
		if(passengers_max != NULL)
		{
		    if(passengers_drop_pending > *passengers_max)
			passengers_drop_pending = *passengers_max;

		    if(cur_passengers_drop_pending != NULL)
			*cur_passengers_drop_pending = passengers_drop_pending;
		}
	    }
	}

	return(0);
}

/*
 *	Unloads all passengers from the given object by checking if the
 *	object can have passengers and setting the object's passengers
 *	count to 0 and passengers mass to 0.
 *
 *	Does not check if unloading passengers is permitted, the calling
 *	function must do that.
 *
 *	Returns the number of passengers unloaded.
 */
int SARSimOpPassengersUnloadAll(
	sar_scene_struct *scene, sar_object_struct *obj_ptr
)
{
	int passengers_unloaded = 0;
	int *crew, *passengers, *passengers_max;
	float *passengers_mass;
	int *passengers_leave_pending, *passengers_drop_pending;


	if((scene == NULL) || (obj_ptr == NULL))
	    return(passengers_unloaded);

	if(SARObjGetOnBoardPtr(
	    obj_ptr,
	    &crew, &passengers, &passengers_max,
	    &passengers_mass,
	    &passengers_leave_pending, &passengers_drop_pending
	))
	    return(passengers_unloaded);

	/* Got passengers? */
	if(passengers != NULL)
	{
	    /* Unload all passengers */
	    passengers_unloaded += *passengers;
	    *passengers = 0;

	    /* Reset passengers mass (in kg) if any */
	    if(passengers_mass != NULL)
		*passengers_mass = 0.0f;

	    /* Since all passengers gone, no pending to leave */
	    if(passengers_leave_pending != NULL)
		*passengers_leave_pending = 0;
	    if(passengers_drop_pending != NULL)
		*passengers_drop_pending = 0;
	}

	return(passengers_unloaded);
}


/*
 *	Transfers fuel from external reserve tanks on the object
 *	(if it has any) to the main fuel tank.
 *
 *	Returns the amount transfered or 0.0 on failure.
 */
float SARSimOpTransferFuelFromTanks(
	sar_scene_struct *scene, sar_object_struct *obj_ptr
)
{
	int i;
	sar_object_aircraft_struct *obj_aircraft_ptr;
	float *fuel = NULL, *fuel_max = NULL;
	sar_external_fueltank_struct **eft = NULL, *eft_ptr;
	int eft_total = 0;
	float fuel_transfered = 0.0f;

	if((scene == NULL) || (obj_ptr == NULL))
	    return(fuel_transfered);

	switch(obj_ptr->type)
	{
	  case SAR_OBJ_TYPE_GARBAGE:
	  case SAR_OBJ_TYPE_STATIC:
	  case SAR_OBJ_TYPE_AUTOMOBILE:
	  case SAR_OBJ_TYPE_WATERCRAFT:
	    break;
	  case SAR_OBJ_TYPE_AIRCRAFT:
	    obj_aircraft_ptr = SAR_OBJ_GET_AIRCRAFT(obj_ptr);
	    if(obj_aircraft_ptr != NULL)
	    {
		fuel = &obj_aircraft_ptr->fuel;
		fuel_max = &obj_aircraft_ptr->fuel_max;
		eft = obj_aircraft_ptr->external_fueltank;
		eft_total = obj_aircraft_ptr->total_external_fueltanks;
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

	/* No fuel tanks to transfer to? */
	if((fuel == NULL) || (fuel_max == NULL))
	    return(fuel_transfered);

	/* No room for fuel on main tanks? */
	if(*fuel_max <= 0.0f)
	    return(fuel_transfered);

	/* Current fuel too much? */
	if(*fuel > *fuel_max)
	{
	    *fuel = *fuel_max;
	    return(fuel_transfered);
	}
	/* Sanitize current fuel */
	if(*fuel < 0.0f)
	    *fuel = 0.0f;

	/* No fuel tanks to transfer from? */
	if((eft == NULL) || (eft_total < 1))
	    return(fuel_transfered);

	/* Iterate through each external reserved fuel tank */
	for(i = 0; i < eft_total; i++)
	{
	    eft_ptr = eft[i];
	    if(eft_ptr == NULL)
		continue;

	    /* This external fuel tank not dropped? */
	    if(eft_ptr->flags & SAR_EXTERNAL_FUELTANK_FLAG_ONBOARD)
	    {
		float fuel_needed = MAX(*fuel_max - *fuel, 0.0f);
		float fuel_to_transfer;

		/* Check if external reserved tank has enough fuel,
		 * that we need to transfer. Then get the amount of
		 * fuel we can actually transfer.
		 */
		if(eft_ptr->fuel < fuel_needed)
		    fuel_to_transfer = eft_ptr->fuel;
		else
		    fuel_to_transfer = fuel_needed;

		/* Transfer fuel */
		*fuel += fuel_to_transfer;
		eft_ptr->fuel -= fuel_to_transfer;

		/* Update total amount of fuel transfered */
		fuel_transfered += fuel_to_transfer;
	    }
	}


	return(fuel_transfered);
}


/*
 *	Drops the `next' fuel tank that is on board the given object
 *	(if any).
 *
 *	Returns -1 on error, -2 if no tanks left to drop, or the
 *	index number of the newly created fuel tank object.
 *
 *	Inputs assumed valid except for obj_ptr and obj_num.
 */
int SARSimOpDropFuelTankNext(
	sar_scene_struct *scene,
	sar_object_struct ***ptr, int *total,
	int obj_num, sar_object_struct *obj_ptr
)
{
	int i, ft_obj_num;
	sar_object_aircraft_struct *obj_aircraft_ptr = NULL;
	sar_external_fueltank_struct **eft = NULL, *eft_ptr = NULL;
	int total_efts = 0;

	/* obj_ptr is the object dropping a fuel tank
	 *
	 * eft_ptr is the external fuel tank on obj_ptr
	 *
	 * ft_obj_ptr is the newly dropped fuel tank object
	 */

	if(!SARObjIsAllocated(*ptr, *total, obj_num))
	    return(-1);
	if(obj_ptr == NULL)
	    return(-1);

	/* Get pointer array to external fuel tanks on the given
	 * object
	 */
	switch(obj_ptr->type)
	{
	  case SAR_OBJ_TYPE_GARBAGE:
	  case SAR_OBJ_TYPE_STATIC:
	  case SAR_OBJ_TYPE_AUTOMOBILE:
	  case SAR_OBJ_TYPE_WATERCRAFT:
	    break;
	  case SAR_OBJ_TYPE_AIRCRAFT:
	    obj_aircraft_ptr = SAR_OBJ_GET_AIRCRAFT(obj_ptr);
	    if(obj_aircraft_ptr != NULL)
	    {
		eft = obj_aircraft_ptr->external_fueltank;
		total_efts = obj_aircraft_ptr->total_external_fueltanks;
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

	/* Iterate through existing fuel tanks */
	for(i = 0; i < total_efts; i++)
	{
	    eft_ptr = eft[i];
	    if(eft_ptr == NULL)
		continue;

	    /* Not droppable? */
	    if(eft_ptr->flags & SAR_EXTERNAL_FUELTANK_FLAG_FIXED)
		continue;

	    /* Already dropped? */
	    if(!(eft_ptr->flags & SAR_EXTERNAL_FUELTANK_FLAG_ONBOARD))
		continue;

	    /* Got valid external fuel tank that is on board and
	     * can be dropped.
	     */
	    break;
	}
	/* No more fuel tanks to drop? */
	if((i >= total_efts) || (eft_ptr == NULL))
	    return(-2);


	/* Got valid fuel tank i to drop, begin dropping it */

	/* Drop it by marking it as not on board */
	eft_ptr->flags &= ~SAR_EXTERNAL_FUELTANK_FLAG_ONBOARD;

	/* Create a new fuel tank object */
	ft_obj_num = SARObjNew(
	    scene, ptr, total, SAR_OBJ_TYPE_FUELTANK
	);
	if(ft_obj_num > -1)
	{
	    double a[3 * 1], r[3 * 1];
	    sar_direction_struct *dir;
	    sar_contact_bounds_struct *cb;
	    sar_object_struct *ft_obj_ptr = (*ptr)[ft_obj_num];
	    sar_object_fueltank_struct *obj_fueltank_ptr;

	    /* Newly created fuel tank object valid? */
	    if(ft_obj_ptr != NULL)
	    {
		/* Begin setting values on nwe fuel tank object */

		ft_obj_ptr->flags = 0;
		if(obj_ptr->flags & SAR_OBJ_FLAG_SHADE_MODEL_SMOOTH)
		    ft_obj_ptr->flags |= SAR_OBJ_FLAG_SHADE_MODEL_SMOOTH;

		free(ft_obj_ptr->name);
		ft_obj_ptr->name = STRDUP("Fueltank");

		ft_obj_ptr->range = 500.0f;	/* 500 meters */

		ft_obj_ptr->ground_elevation_msl = obj_ptr->ground_elevation_msl;

		/* Create contact bounds for new fuel tank object */
		cb = ft_obj_ptr->contact_bounds;
		if(cb == NULL)
		    ft_obj_ptr->contact_bounds = cb = SAR_CONTACT_BOUNDS(
			calloc(1, sizeof(sar_contact_bounds_struct))
		    );
		if(cb != NULL)
		{
		    cb->crash_flags = 0;
		    cb->crash_type = SAR_CRASH_TYPE_AIRCRAFT;

		    /* Set spherical contact bounds */
		    SARObjAddContactBoundsSpherical(
			ft_obj_ptr,
			cb->crash_flags,
			cb->crash_type,
			eft_ptr->radius
		    );
		} 

		ft_obj_ptr->life_span = 0l;
		ft_obj_ptr->hit_points = 1.0f;
		ft_obj_ptr->hit_points_max = 1.0f;
		ft_obj_ptr->temperature = eft_ptr->temperature;

		/* Remove existing visual model on new fueltank object
		 * (if it has one or more)
		 */
		SARVisualModelUnref(scene, ft_obj_ptr->visual_model);

		/* Transfer visual model from eft_ptr's visual model
		 * to the new fuel tank object and increment ref count
		 */
		ft_obj_ptr->visual_model = eft_ptr->visual_model;
		SARVisualModelRef(ft_obj_ptr->visual_model);
		ft_obj_ptr->visual_model_ir = eft_ptr->visual_model_ir;
		SARVisualModelRef(ft_obj_ptr->visual_model_ir);


		/* Get fueltank substructure from the object that is the
		 * fueltank
		 */
		obj_fueltank_ptr = SAR_OBJ_GET_FUELTANK(ft_obj_ptr);
		if(obj_fueltank_ptr != NULL)
		{
		    sar_position_struct *vel;

		    obj_fueltank_ptr->flags = 0;

		    obj_fueltank_ptr->belly_to_center_height =
			eft_ptr->belly_to_center_height;

		    obj_fueltank_ptr->dry_mass = eft_ptr->dry_mass;
		    obj_fueltank_ptr->fuel = eft_ptr->fuel;
		    obj_fueltank_ptr->fuel_max = eft_ptr->fuel_max;

		    obj_fueltank_ptr->ref_object = obj_num;


		    /* Maximum vertical velocity in meters per cycle */
		    obj_fueltank_ptr->vel_z_max = (float)(SFMFeetToMeters(-100.0) *
			SAR_SEC_TO_CYCLE_COEFF);

		    /* Get velocity from object */
		    vel = &obj_fueltank_ptr->vel;
		    if(obj_aircraft_ptr != NULL)
			memcpy(vel, &obj_aircraft_ptr->vel, sizeof(sar_position_struct));
		    else
			memset(vel, 0x00, sizeof(sar_position_struct));

		    obj_fueltank_ptr->speed = (float)SFMHypot3(
			vel->x, vel->y, vel->z
		    );
		}


		/* Move new fuel tank object to proper position */
		dir = &obj_ptr->dir;

		a[0] = eft_ptr->offset_pos.x;
		a[1] = eft_ptr->offset_pos.y;
		a[2] = eft_ptr->offset_pos.z;

		/* Rotate matrix a into r */
		MatrixRotateBank3(a, -dir->bank, r);	/* Our bank is negative so,
							 * so pass as flipped sign
							 */
		MatrixRotatePitch3(r, dir->pitch, a);
		MatrixRotateHeading3(a, dir->heading, r);

		ft_obj_ptr->pos.x = (float)(obj_ptr->pos.x + r[0]);
		ft_obj_ptr->pos.y = (float)(obj_ptr->pos.y + r[1]);
		ft_obj_ptr->pos.z = (float)(obj_ptr->pos.z + r[2]);

		memcpy(&ft_obj_ptr->dir, dir, sizeof(sar_direction_struct));

		SARSimWarpObject(
		    scene, ft_obj_ptr,
		    &ft_obj_ptr->pos, &ft_obj_ptr->dir
		);
	    }
	}

	return(i);
}


/*
 *	Generates a new smoke unit on the specified smoke object.
 */
void SARSimSmokeSpawn(
	sar_core_struct *core_ptr, sar_object_struct *obj_ptr,
	sar_object_smoke_struct *obj_smoke_ptr
)
{
	int i;
	sar_object_smoke_unit_struct *u;

	if((obj_ptr == NULL) || (obj_smoke_ptr == NULL))
	    return;

	/* No units */
	if(obj_smoke_ptr->unit == NULL)
	    return;

	/* Spawn unit by smoke type */
	switch(obj_smoke_ptr->type)
	{
	  case SAR_SMOKE_TYPE_SMOKE:
	    /* Find available unit */
	    u = NULL;
	    for(i = 0; i < obj_smoke_ptr->total_units; i++)
	    {
		u = &obj_smoke_ptr->unit[i];

		/* Unit available? (if it is no longer visible) */
		if(u->visibility <= 0.0f)
		    break;
	    }
	    /* Not able to find any available units? */
	    if(i >= obj_smoke_ptr->total_units)
	    {
		/* Overwrite oldest (first) unit and shift the rest of
		 * the units, leaving the last one available
		 */
		for(i = 0; i < (obj_smoke_ptr->total_units - 1); i++)
		    memcpy(
			&obj_smoke_ptr->unit[i],
			&obj_smoke_ptr->unit[i + 1],
			sizeof(sar_object_smoke_unit_struct)
		    );
		/* Get last unit */
		u = (obj_smoke_ptr->total_units > 0) ?
		    &obj_smoke_ptr->unit[
			obj_smoke_ptr->total_units - 1
		    ] : NULL;
	    }
	    /* Got available unit? */
	    if(u != NULL)
	    {
		sar_position_struct *pos;

		/* Set initial position */
		memcpy(&u->pos, &obj_ptr->pos, sizeof(sar_position_struct));

		/* Set initial velocity to drift upwards */
/* TODO need to apply wind and other variables to drift velocity */
		pos = &u->vel;
                /*
		pos->x = 0.0f;
		pos->y = 0.0f;
		pos->z = 4.0f;
                */

                /* Smoke moves up at random speed and with a small but random
                   horizontal speeds */
                pos->x = 2 * (rand()/(float)(RAND_MAX) - 0.5);
                pos->y = 2 * (rand()/(float)(RAND_MAX) - 0.5);
                pos->z = 2 * rand()/(float)(RAND_MAX) + 3;

		/* Apply offset to initial position */
		pos = &u->pos;
		pos->x += obj_smoke_ptr->respawn_offset.x;
		pos->y += obj_smoke_ptr->respawn_offset.y;
		pos->z += obj_smoke_ptr->respawn_offset.z;

		u->radius = obj_smoke_ptr->radius_start;
		u->visibility = 1.0f;
	    }
	    break;

	  case SAR_SMOKE_TYPE_SPARKS:
	    /* When sparks are respawned, all of them are respawned
	     * at once
	     */
	    for(i = 0; i < obj_smoke_ptr->total_units; i++)
	    {
		u = &obj_smoke_ptr->unit[i];

		/* Begin updating this unit's values */

		/* Set initial position */
		memcpy(&u->pos, &obj_ptr->pos, sizeof(sar_position_struct));

		/* Set random velocity */
		if(1)
		{
		    const time_t respawn_int = (obj_smoke_ptr->respawn_int > 0l) ?
			obj_smoke_ptr->respawn_int : 1000l;
		    float rand_theta = (float)(SARRandomCoeff(i + 0) * 2.0 * PI);
		    float rand_z = SARRandomCoeff(i + 1);
#if 0
		    float respawn_int_coeff = (float)obj_smoke_ptr->respawn_int /
			(float)CYCLE_LAPSE_MS;
		    float rand_speed = obj_smoke_ptr->radius_max /
			((respawn_int_coeff > 0.0f) ? respawn_int_coeff : 1.0f) *
			 ((SARRandomCoeff(i + 2) * 0.6f) + 0.3f);
#endif
		    float s = obj_smoke_ptr->radius_max *
			(float)CYCLE_LAPSE_MS / (float)respawn_int;
		    sar_position_struct *pos = &u->vel;
		    pos->x = (float)(cos(rand_theta) * s);
		    pos->y = (float)(sin(rand_theta) * s);
		    pos->z = (float)(s * rand_z);
		}

		/* Apply offset to initial position */
		if(1)
		{
		    sar_position_struct *pos = &u->pos;
		    pos->x += obj_smoke_ptr->respawn_offset.x;
		    pos->y += obj_smoke_ptr->respawn_offset.y;
		    pos->z += obj_smoke_ptr->respawn_offset.z;

		    u->radius = obj_smoke_ptr->radius_start;
		    u->visibility = 1.0f;
		}

		/* Set random spark color */
		if(1)
		{
		    float rand_green = SARRandomCoeff(i + 10);
		    float rand_blue = SARRandomCoeff(i + 11);
		    sar_color_struct *c = &u->color;
		    c->r = 1.0f;
		    c->g = (rand_green * 0.1f) + 0.9f;
		    c->b = (rand_green * rand_blue * 0.4f) + 0.6f;
		    c->a = 1.0f;
		}
	    }
	    break;

	  case SAR_SMOKE_TYPE_DEBRIS:
	    /* TODO */
	    break;
	}
}

/*
 *	Returns 1 if the object is in slew mode.
 *
 *	The object must be of type SAR_OBJ_TYPE_AIRCRAFT, otherwise
 *	it will always return False.
 */
int SARSimIsSlew(sar_object_struct *obj_ptr)
{
	sar_object_aircraft_struct *obj_aircraft_ptr;


	if(obj_ptr == NULL)
	    return(0);

	switch(obj_ptr->type)
	{
	  case SAR_OBJ_TYPE_AIRCRAFT:
	    obj_aircraft_ptr = SAR_OBJ_GET_AIRCRAFT(obj_ptr);
	    if(obj_aircraft_ptr != NULL)
		return(
(obj_aircraft_ptr->flight_model_type == SAR_FLIGHT_MODEL_SLEW) ? 1 : 0
		);
	    break;

	  default:
	    break;
	}

	return(0);
}

/*
 *	Sets the object to enter slew mode if enter_slew is True.
 *	Otherwise it returns the object to its previous flight mode.
 *
 *	The object must be of type SAR_OBJ_TYPE_AIRCRAFT.
 */
void SARSimSetSlew(
	sar_object_struct *obj_ptr, int enter_slew
)
{
	sar_object_aircraft_struct *obj_aircraft_ptr;

	if(obj_ptr == NULL)
	    return;

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
	    /* Enter slew mode? */
	    if(enter_slew &&
	       (obj_aircraft_ptr->flight_model_type != SAR_FLIGHT_MODEL_SLEW)
	    )
	    {
		/* Record current flight model type */
		obj_aircraft_ptr->last_flight_model_type =
		    obj_aircraft_ptr->flight_model_type;

		/* Set flight model to slew mode */
		obj_aircraft_ptr->flight_model_type =
		    SAR_FLIGHT_MODEL_SLEW;
	    }
	    /* Leave slew mode? */
	    else if(!enter_slew &&
		    (obj_aircraft_ptr->flight_model_type == SAR_FLIGHT_MODEL_SLEW)
	    )
	    {
		/* Restore previous flight model type */
		obj_aircraft_ptr->flight_model_type =
		    obj_aircraft_ptr->last_flight_model_type;
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
}

/*
 *	Moves the human object specified by obj_human_ptr (the
 *	substructure) into the hoist deployment of the object specified
 *	by obj_ptr.
 *
 *	Note that the given obj_human_ptr may not be the substructure
 *	of the given obj_ptr.
 *
 *	Inputs assumed valid.
 *
 *	Returns the number of objects put into the hoist deployment.
 */
int SARSimDoPickUpHuman(
	sar_scene_struct *scene,
	sar_object_struct *obj_ptr,
	sar_object_human_struct *obj_human_ptr,	/* Not data structure of obj_ptr */
	int human_obj_num
)
{
	int n;
	int passengers = 0, passengers_max = 0;
	sar_object_aircraft_struct *obj_aircraft_ptr;
	sar_obj_hoist_struct *hoist = NULL;


	if(scene == NULL)
	    return(0);

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
	    if(obj_aircraft_ptr != NULL)
	    {
		hoist = SARObjGetHoistPtr(obj_ptr, 0, NULL);
		passengers = obj_aircraft_ptr->passengers;
		passengers_max = obj_aircraft_ptr->passengers_max;
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

	/* Did we get a hoist structure? */
	if(hoist == NULL)
	    return(0);

	/* No more room left onboard for additional passengers? */
	if(passengers >= passengers_max)
	{
	    /* Warn if this is the player object and the scene is
	     * not currently displaying any messages
	     */
	    if((obj_ptr == scene->player_obj_ptr) &&
	       (scene->message_display_until <= 0)
	    )
	    {
		SARMessageAdd(
		    scene,
		    SAR_MESG_NO_ROOM_LEFT_FOR_PASSENGERS
		);
	    }
	    return(0);
	}

	/* Can hoist accomidate more occupants? (just limit to 1 for
	 * now)
	 */
	if(hoist->total_occupants >= 1)
	    return(0);

	/* Checks passed, begin adding human to hoist's list of
	 * occupants
	 */
	n = MAX(hoist->total_occupants, 0);
	hoist->total_occupants = n + 1;
	hoist->occupant = (int *)realloc(
	    hoist->occupant,
	    hoist->total_occupants * sizeof(int)
	);
	if(hoist->occupant == NULL)
	{
	    hoist->total_occupants = 0;
	    return(0);
	}
	else
	{
	    hoist->occupant[n] = human_obj_num;
	}

	/* Update hoist's occupants mass (in kg) */
	hoist->occupants_mass += obj_human_ptr->mass;

	/* Update nessesary flags on human object by what is on
	 * the end of the hoist rope
	 */
	switch(hoist->cur_deployment)
	{
	  case SAR_HOIST_DEPLOYMENT_BASKET:
	    obj_human_ptr->flags = ~SAR_HUMAN_FLAG_NEED_RESCUE;
	    obj_human_ptr->flags &= ~SAR_HUMAN_FLAG_SIT;
	    obj_human_ptr->flags |= SAR_HUMAN_FLAG_SIT_DOWN;
	    obj_human_ptr->flags &= ~SAR_HUMAN_FLAG_SIT_UP;
	    obj_human_ptr->flags &= ~SAR_HUMAN_FLAG_LYING;
	    /* Leave SAR_HUMAN_FLAG_ALERT and SAR_HUMAN_FLAG_AWARE as is */
	    obj_human_ptr->flags &= ~SAR_HUMAN_FLAG_IN_WATER;
	    obj_human_ptr->flags &= ~SAR_HUMAN_FLAG_ON_STREATCHER;
	    obj_human_ptr->flags &= ~SAR_HUMAN_FLAG_RUN;
	    obj_human_ptr->flags &= ~SAR_HUMAN_FLAG_RUN_TOWARDS;
	    obj_human_ptr->flags &= ~SAR_HUMAN_FLAG_RUN_AWAY;
	    obj_human_ptr->flags &= ~SAR_HUMAN_FLAG_PUSHING;
	    obj_human_ptr->flags &= ~SAR_HUMAN_FLAG_DIVER_CATCHER;
	    obj_human_ptr->flags |= SAR_HUMAN_FLAG_GRIPPED;
	    break;

	  case SAR_HOIST_DEPLOYMENT_DIVER:
	    obj_human_ptr->flags = ~SAR_HUMAN_FLAG_NEED_RESCUE;
	    obj_human_ptr->flags &= ~SAR_HUMAN_FLAG_SIT;
	    obj_human_ptr->flags &= ~SAR_HUMAN_FLAG_SIT_DOWN;
	    obj_human_ptr->flags &= ~SAR_HUMAN_FLAG_SIT_UP;
	    obj_human_ptr->flags &= ~SAR_HUMAN_FLAG_LYING;
	    /* Leave SAR_HUMAN_FLAG_ALERT and SAR_HUMAN_FLAG_AWARE as is */
	    obj_human_ptr->flags &= ~SAR_HUMAN_FLAG_IN_WATER;
	    obj_human_ptr->flags &= ~SAR_HUMAN_FLAG_ON_STREATCHER;
	    obj_human_ptr->flags &= ~SAR_HUMAN_FLAG_RUN;
	    obj_human_ptr->flags &= ~SAR_HUMAN_FLAG_RUN_TOWARDS;
	    obj_human_ptr->flags &= ~SAR_HUMAN_FLAG_RUN_AWAY;
	    obj_human_ptr->flags &= ~SAR_HUMAN_FLAG_PUSHING;
	    obj_human_ptr->flags &= ~SAR_HUMAN_FLAG_DIVER_CATCHER;
	    obj_human_ptr->flags |= SAR_HUMAN_FLAG_GRIPPED;
	    break;

	  case SAR_HOIST_DEPLOYMENT_HOOK:
	    obj_human_ptr->flags = ~SAR_HUMAN_FLAG_NEED_RESCUE;
	    obj_human_ptr->flags &= ~SAR_HUMAN_FLAG_SIT;
	    obj_human_ptr->flags &= ~SAR_HUMAN_FLAG_SIT_DOWN;
	    obj_human_ptr->flags &= ~SAR_HUMAN_FLAG_SIT_UP;
	    obj_human_ptr->flags &= ~SAR_HUMAN_FLAG_LYING;
	    /* Leave SAR_HUMAN_FLAG_ALERT and SAR_HUMAN_FLAG_AWARE as is */
	    obj_human_ptr->flags &= ~SAR_HUMAN_FLAG_IN_WATER;
	    obj_human_ptr->flags &= ~SAR_HUMAN_FLAG_ON_STREATCHER;
	    obj_human_ptr->flags &= ~SAR_HUMAN_FLAG_RUN;
	    obj_human_ptr->flags &= ~SAR_HUMAN_FLAG_RUN_TOWARDS;
	    obj_human_ptr->flags &= ~SAR_HUMAN_FLAG_RUN_AWAY;
	    obj_human_ptr->flags &= ~SAR_HUMAN_FLAG_PUSHING;
	    obj_human_ptr->flags &= ~SAR_HUMAN_FLAG_DIVER_CATCHER;
	    obj_human_ptr->flags |= SAR_HUMAN_FLAG_GRIPPED;
	    break;
	}

 	return(1);
}


/*
 *	Called each time the hoist is pulled in, it will bring all
 *	objects in the hoist's deployment in. This will delete those
 *	object(s) and update the passenger count.
 *
 *	If a mission is active then the mission hoist in callback will
 *	be called.
 *
 *	Inputs assumed valid.
 *
 *	Returns the number of objects hoisted in.
 */
int SARSimDoHoistIn(
	sar_core_struct *core_ptr, sar_object_struct *obj_ptr
)
{
	int i, passenger_obj_num;
	int prev_passengers = 0;
	int *passengers = NULL, *passengers_max = NULL;
	int objects_hoisted_in = 0;
	sar_object_struct *passenger_obj_ptr;
	sar_object_human_struct *obj_human_ptr;
	sar_scene_struct *scene = core_ptr->scene;
	int is_player = (scene->player_obj_ptr == obj_ptr) ? 1 : 0;
	sar_obj_hoist_struct *hoist = SARObjGetHoistPtr(obj_ptr, 0, NULL);
	if((scene == NULL) || (hoist == NULL))
	    return(objects_hoisted_in);

	/* Get passenger counts */
	SARObjGetOnBoardPtr(
	    obj_ptr,
	    NULL, &passengers, &passengers_max,
	    NULL,
	    NULL, NULL
	);
	if(passengers != NULL)
	    prev_passengers = *passengers;

	/* Delete all objects in hoist's deployment */
	for(i = 0; i < hoist->total_occupants; i++)
	{
	    passenger_obj_num = hoist->occupant[i];
	    if(SARObjIsAllocated(
		core_ptr->object,
		core_ptr->total_objects,
		passenger_obj_num
	    ))
		passenger_obj_ptr = core_ptr->object[passenger_obj_num];
	    else
		continue;

	    /* Avoid deleting the given object */
	    if(passenger_obj_ptr == obj_ptr)
		continue;

	    /* Handle by occupant object type */
	    switch(passenger_obj_ptr->type)
	    {
	      case SAR_OBJ_TYPE_HUMAN:
		obj_human_ptr = SAR_OBJ_GET_HUMAN(passenger_obj_ptr);
		if(obj_human_ptr != NULL)
		{
		    /* If this is the player object, then print the
		     * hoist in message specified on the human object
		     */
		    if(is_player)
			SARMessageAdd(scene, obj_human_ptr->mesg_enter);
		}
		break;

	      default:
		/* Print hoist in message */
		if(is_player)
		{
/* TODO */
		}
		break;
	    }

	    /* Move the passenger object into the given object, the
	     * passenger object will be deleted if this call returns
	     * true
	     *
	     * Passenger count and related values will be updated on
	     * the given object
	     */
	    if(SARSimBoardObject(core_ptr, obj_ptr, passenger_obj_num))
	    {
		passenger_obj_num = -1;
		passenger_obj_ptr = NULL;
		objects_hoisted_in++;
	    }
	}

	/* Delete occupants list on hoist structure */
	free(hoist->occupant);
	hoist->occupant = NULL;
	hoist->total_occupants = 0;


	/* Print new number of passengers on the given object */
	if((passengers != NULL) && (passengers_max != NULL))
	{
	    /* Got more passengers? */
	    if(prev_passengers < *passengers)
	    {
		/* Is this the player object? */
	        if(is_player)
	        {
		    char text[256];

		    sprintf(
			text,
			"Passengers: %i(%i)",
			*passengers,
			*passengers_max
		    );
		    SARMessageAdd(scene, text);
		}
		else
		{
		    char name[SAR_OBJ_NAME_MAX];
		    char text[SAR_OBJ_NAME_MAX + 256];

		    if(obj_ptr->name == NULL)
		    {
			int obj_num = SARGetObjectNumberFromPointer(
			    scene, core_ptr->object, core_ptr->total_objects,
			    obj_ptr
			);
			if(SAR_OBJ_NAME_MAX > 80)
			    sprintf(name, "Object #%i\n", obj_num);
			else
			    *name = '\0';
		    }
		    else
		    {
			strncpy(name, obj_ptr->name, SAR_OBJ_NAME_MAX);
		    }
		    name[SAR_OBJ_NAME_MAX - 1] = '\0';

		    sprintf(text, "%s picked up %i passengers",
			name, (*passengers) - prev_passengers
		    );
		    SARMessageAdd(scene, text);
		}
	    }
	}

	/* If a mission is active then call the mission hoist in
	 * notify callback
	 */
	if(core_ptr->mission != NULL)
	    SARMissionHoistInNotify(core_ptr, obj_ptr, objects_hoisted_in);

	return(objects_hoisted_in);
}

/*
 *	Updates the source object to have taken the target object
 *	boarding it. The source object may be deleted by this function.
 *
 *	The target object must be able to hold the source object (must
 *	(be able to accept passengers and have room for more passengers).
 *
 *	For example if the target object is an aircraft and the source
 *	object is a human, then the human object will be deleted and
 *	the source object will have its passenger count increment (if
 *	it has room for more passengers).
 *
 *	Does not check if the source object is within range to board the
 *	target object.
 *
 *	Returns 0 on failure to board or 1 if the source object has
 *	boarded the target and the source object no longer exists.
 */
int SARSimBoardObject(
	sar_core_struct *core_ptr,
	sar_object_struct *tar_obj_ptr, int src_obj_num
)
{
	int *passengers, *passengers_max;
	float *passengers_mass;
	float src_mass = 0.0f;
	sar_object_struct *src_obj_ptr;
	sar_object_human_struct *obj_human_ptr;


	if(tar_obj_ptr == NULL)
	    return(0);

	/* Get source object pointer */
	src_obj_ptr = SARObjGetPtr(
	    core_ptr->object, core_ptr->total_objects, src_obj_num
	);
	/* Got source object pointer and it is not the same as the
	 * target object?
	 */
	if((src_obj_ptr == NULL) || (src_obj_ptr == tar_obj_ptr))
	    return(0);

	/* Check if the target object cannot hold passengers */
	if(SARObjGetOnBoardPtr(
	    tar_obj_ptr,
	    NULL, &passengers, &passengers_max,
	    &passengers_mass,
	    NULL, NULL
	))
	    return(0);

	if((passengers == NULL) || (passengers_max == NULL))
	    return(0);

	/* Target object has no more room for passengers? */
	if(*passengers >= *passengers_max)
	    return(0);

	/* Get some information about the source object by its type */
	switch(src_obj_ptr->type)
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
	    obj_human_ptr = SAR_OBJ_GET_HUMAN(src_obj_ptr);
	    if(obj_human_ptr != NULL)
	    {
		src_mass = obj_human_ptr->mass;
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


	/* Begin moving source object into target object */

	/* Increment passengers count */
	*passengers = MAX(*passengers, 0) + 1;

	/* Increment passengers mass if possible */
	if(passengers_mass != NULL)
	    *passengers_mass = MAX(*passengers_mass, 0.0f) + src_mass;
	
	/* If a mission is active then call the mission passengers enter
	 * notify callback
	 */
	if(core_ptr->mission != NULL)
	    SARMissionPassengersEnterNotify(core_ptr, tar_obj_ptr, 1);
	
	/* Delete the source object */
	SARObjDelete(
	    core_ptr,
	    &core_ptr->object, &core_ptr->total_objects,
	    src_obj_num
	);

	return(1);
}

/*
 *	Calculates the `fly by view' position with respect to
 *	the given object. The result of the calculation is stored
 *	in the pos_result structure.
 */
void SARSimSetFlyByPosition(
	sar_scene_struct *scene,
	sar_object_struct ***ptr, int *total,
	sar_object_struct *obj_ptr,
	sar_position_struct *pos_result
)
{
	float contact_radius;
	sar_object_aircraft_struct *obj_aircraft_ptr;

	if((obj_ptr == NULL) || (pos_result == NULL))
	    return;

	contact_radius = SARSimGetFlatContactRadius(obj_ptr);

	switch(obj_ptr->type)
	{
	  case SAR_OBJ_TYPE_GARBAGE:
	  case SAR_OBJ_TYPE_STATIC:
	  case SAR_OBJ_TYPE_AUTOMOBILE:
	  case SAR_OBJ_TYPE_WATERCRAFT:
	    break;
	  case SAR_OBJ_TYPE_AIRCRAFT:
	    obj_aircraft_ptr = SAR_OBJ_GET_AIRCRAFT(obj_ptr);
	    if(obj_aircraft_ptr != NULL)
	    {
		sar_direction_struct vel_dir;
		sar_position_struct	*pos = &obj_ptr->pos,
					*vel = &obj_aircraft_ptr->vel;
		float	d = (float)MAX(
			    (5 * SAR_CYCLE_TO_SEC_COEFF) * obj_aircraft_ptr->airspeed.y,
			    contact_radius * 2
			),
			h = (float)(5 * SAR_CYCLE_TO_SEC_COEFF) * vel->z,
			r = (float)MAX(
			    SQRT((d * d) - (h * h)),
			    contact_radius * 2
			);

		vel_dir.heading = (float)SFMSanitizeRadians(
		    ((PI / 2) - atan2(vel->y, vel->x)) +
			obj_ptr->dir.heading
		);
		pos_result->x = (float)(r * sin(vel_dir.heading)) + pos->x -
		    (contact_radius * 2);
		pos_result->y = (float)(r * cos(vel_dir.heading)) + pos->y -
		    (contact_radius * 2);
		pos_result->z = h + pos->z;

		/* Don't position lower than object's center to ground */
		if(pos_result->z < (pos->z + obj_aircraft_ptr->center_to_ground_height))
		    pos_result->z = pos->z + obj_aircraft_ptr->center_to_ground_height;
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
}
