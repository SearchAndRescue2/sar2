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
#include <sys/types.h>
#include <math.h>

#include "sfm.h"


int SFMModelInRealm(SFMRealmStruct *realm, SFMModelStruct *model);

SFMModelStruct *SFMModelAllocate(void);
int SFMModelAdd(SFMRealmStruct *realm, SFMModelStruct *model);
void SFMModelDelete(SFMRealmStruct *realm, SFMModelStruct *model);

SFMBoolean SFMModelChangeValues(
	SFMRealmStruct *realm, SFMModelStruct *model,
	SFMModelStruct *value
);
void SFMModelUndefineValue(
	SFMRealmStruct *realm, SFMModelStruct *model, SFMFlags flags
);


#define MAX(a,b)        (((a) > (b)) ? (a) : (b))
#define MIN(a,b)        (((a) < (b)) ? (a) : (b))
#define CLIP(a,l,h)     (MIN(MAX((a),(l)),(h)))

#define RADTODEG(r)     ((r) * 180 / PI)
#define DEGTORAD(d)     ((d) * PI / 180)


/*
 *	Checks if the specified FDM is in the FDM Realm.
 *
 *	Returns the FDM's index number or -1 on error.
 */
int SFMModelInRealm(SFMRealmStruct *realm, SFMModelStruct *model)
{
	int i;

	if((realm == NULL) || (model == NULL))
	    return(-1);

	for(i = 0; i < realm->total_models; i++)
	{
	    if(model == realm->model[i])
		return(i);
	}

	return(-1);
}

/*
 *	Creates a new FDM.
 */
SFMModelStruct *SFMModelAllocate(void)
{
	SFMModelStruct *m = (SFMModelStruct *)calloc(
	    1, sizeof(SFMModelStruct)
	);
	if(m == NULL)
	    return(NULL);

	return(m);
}

/*
 *	Adds the FDM to the FDM Realm.
 *
 *	The FDM must have all required values set up prior to this
 *	call.
 *
 *	Returns the FDM's index number at which it was added at or
 *	-1 on error.
 */
int SFMModelAdd(SFMRealmStruct *realm, SFMModelStruct *model)
{
	int i, n;

	if((realm == NULL) || (model == NULL))
	    return(-1);

	if(realm->total_models < 0)
	    realm->total_models = 0;

	for(i = 0; i < realm->total_models; i++)
	{
	    if(realm->model[i] == NULL)
		break;
	}
	if(i < realm->total_models)
	{
	    n = i;
	}
	else
	{
	    n = realm->total_models;
	    realm->total_models++;

	    realm->model = (SFMModelStruct **)realloc(
		realm->model,
		realm->total_models * sizeof(SFMModelStruct *)
	    );
	    if(realm->model == NULL)
	    {
		free(model);
		realm->total_models = 0;
		return(-1);
	    }
	}

	realm->model[n] = model;

	/* Report the FDM being added to the FDM Realm */
	if(realm->init_model_cb != NULL)
	    realm->init_model_cb(
		realm,
		model,		/* Invalid */
		realm->init_model_cb_client_data
	    );

	/* Callback deleted model? */
	if(SFMModelInRealm(realm, model) < 0)
	{
	    /* Do nothing */
	}

	return(n);
}

/*
 *	Deletes the FDM from the FDM Realm.
 */
void SFMModelDelete(SFMRealmStruct *realm, SFMModelStruct *model)
{
	int i, n;

	if((realm == NULL) || (model == NULL))
	    return;

	/* Check if the specified FDM exists and get its index number */
	for(i = 0; i < realm->total_models; i++)
	{
	    if(realm->model[i] == model)
		break;
	}

	/* Specified FDM exists? */
	if(i < realm->total_models)
	{
	    /* Delete the FDM */

	    free(model);


	    /* Reallocate pointers */
	    realm->total_models--;
	    for(n = i; n < realm->total_models; n++)
		realm->model[n] = realm->model[n + 1];

	    if(realm->total_models > 0)
	    {
		realm->model = (SFMModelStruct **)realloc(
		    realm->model,
		    realm->total_models * sizeof(SFMModelStruct *)
		);
		if(realm->model == NULL)
		{
		    realm->total_models = 0;
		    return;
		}
	    }
	    else
	    {
		free(realm->model);
		realm->model = NULL;

		realm->total_models = 0;
	    }
	}

	/* Report the FDM being destroyed */
	if(realm->destroy_model_cb != NULL)
	    realm->destroy_model_cb(
		realm, 
		model,		/* model is invalid, use only as reference */
		realm->destroy_model_cb_client_data
	    );
}


/*
 *	Changes the FDM's values.
 *
 *	Returns False on failure or True on success.
 */
SFMBoolean SFMModelChangeValues(
	SFMRealmStruct *realm, SFMModelStruct *model,
	SFMModelStruct *value
)
{
	SFMFlags flags;

	if((realm == NULL) || (model == NULL) || (value == NULL))
	    return(False);

	flags = value->flags;

	/* Begin updating value depending on which flags on the value
	 * are set
	 */

	if(flags & SFMFlagFlightModelType)
	{
	    model->type = value->type;
	}
	if(flags & SFMFlagPosition)
	{
	    memcpy(&model->position, &value->position, sizeof(SFMPositionStruct));
	}
	if(flags & SFMFlagDirection)
	{
	    model->direction.heading = value->direction.heading;
	    model->direction.pitch = value->direction.pitch;
	    model->direction.bank = value->direction.bank;

	    model->direction.i = sin(value->direction.heading);
	    model->direction.j = cos(value->direction.heading);
	    model->direction.k = -sin(value->direction.pitch);
	}
	if(flags & SFMFlagVelocityVector)
	{
	    memcpy(
		&model->velocity_vector, &value->velocity_vector,
		sizeof(SFMPositionStruct)
	    );
	}
	if(flags & SFMFlagAirspeedVector)
	{
	    memcpy(
		&model->airspeed_vector, &value->airspeed_vector,
		sizeof(SFMPositionStruct));
	}
	if(flags & SFMFlagSpeedStall)
	{
	    model->speed_stall = value->speed_stall;
	}
	if(flags & SFMFlagDragMin)
	{
	    model->drag_min = value->drag_min;
	}
	if(flags & SFMFlagSpeedMax)
	{
	    model->speed_max = value->speed_max;
	}
	if(flags & SFMFlagAccelResponsiveness)
	{
	    memcpy(
		&model->accel_responsiveness, &value->accel_responsiveness,
		sizeof(SFMPositionStruct)
	    );
	}
	if(flags & SFMFlagGroundElevation)
	{
	    model->ground_elevation_msl = value->ground_elevation_msl;
	}
	if(flags & SFMFlagServiceCeiling)
	{
	    model->service_ceiling = value->service_ceiling;
	}
	if(flags & SFMFlagBellyHeight)
	{
	    model->belly_height = value->belly_height;
	}
	if(flags & SFMFlagLength)
	{
	    model->length = value->length;
	}
	if(flags & SFMFlagWingspan)
	{
	    model->wingspan = value->wingspan;
	}
	if(flags & SFMFlagGearState)
	{
	    model->gear_state = value->gear_state;
	}
	if(flags & SFMFlagGearType)
	{
	    model->gear_type = value->gear_type;
	}
	if(flags & SFMFlagGearHeight)
	{
	    model->gear_height = value->gear_height;
	}
	if(flags & SFMFlagGearBrakesState)
	{
	    model->gear_brakes_state = value->gear_brakes_state;
	    model->gear_brakes_coeff = value->gear_brakes_coeff;
	}
	if(flags & SFMFlagGearTurnVelocityOptimul)
	{
	    model->gear_turn_velocity_optimul = value->gear_turn_velocity_optimul;
	}
	if(flags & SFMFlagGearTurnVelocityMax)
	{
	    model->gear_turn_velocity_max = value->gear_turn_velocity_max;
	}
	if(flags & SFMFlagGearTurnRate)
	{
	    model->gear_turn_rate = value->gear_turn_rate;
	}
	if(flags & SFMFlagLandedState)
	{
	    model->landed_state = value->landed_state;
	}
	if (flags & SFMFlagStopped)
	{
	    model->stopped = value->stopped;
	}
	if(flags & SFMFlagGroundContactType)
	{
	    model->ground_contact_type = value->ground_contact_type;
	}
	if(flags & SFMFlagHeadingControlCoeff)
	{
	    model->heading_control_coeff = CLIP(
		value->heading_control_coeff, -1.0, 1.0
	    );
	}
	if(flags & SFMFlagBankControlCoeff)
	{
	    model->bank_control_coeff = CLIP(
		value->bank_control_coeff, -1.0, 1.0
	    );
	}
	if(flags & SFMFlagPitchControlCoeff)
	{
	    model->pitch_control_coeff = CLIP(
		value->pitch_control_coeff, -1.0, 1.0
	    );
	}
	if(flags & SFMFlagThrottleCoeff)
	{
	    model->throttle_coeff = CLIP(
		value->throttle_coeff, 0.0, 1.0
	    );
	}
	if(flags & SFMFlagAfterBurnerState)
	{
	    model->after_burner_state = value->after_burner_state;
	}
	if(flags & SFMFlagAfterBurnerPowerCoeff)
	{
	    model->after_burner_power_coeff = MAX(
		value->after_burner_power_coeff, 0.0
	    );
	}
	if(flags & SFMFlagEnginePower)
	{
	    model->engine_power = MAX(
		value->engine_power, 0.0
	    );
	}
	if(flags & SFMFlagTotalMass)
	{
	    model->total_mass = MAX(
		value->total_mass, 0.0
	    );
	}
	if(flags & SFMFlagAttitudeChangeRate)
	{
	    memcpy(
		&model->attitude_change_rate, &value->attitude_change_rate,
		sizeof(SFMDirectionStruct)
	    );
	}
	if(flags & SFMFlagAttitudeLevelingRate)
	{
	    memcpy(
		&model->attitude_leveling_rate, &value->attitude_leveling_rate,
		sizeof(SFMDirectionStruct)
	    );
	}
	if(flags & SFMFlagAirBrakesState)
	{
	    model->air_brakes_state = value->air_brakes_state;
	}
	if(flags & SFMFlagAirBrakesArea)
	{
	    model->air_brakes_area = value->air_brakes_area;
	}
	if(flags & SFMFlagCanCrashIntoOther)
	{
	    model->can_crash_into_other = value->can_crash_into_other;
	}
	if(flags & SFMFlagCanCauseCrash)
	{
	    model->can_cause_crash = value->can_cause_crash;
	}
	if(flags & SFMFlagCrashContactShape)
	{
	    model->crash_contact_shape = value->crash_contact_shape;
	}
	if(flags & SFMFlagCrashableSizeRadius)
	{
	    model->crashable_size_radius = value->crashable_size_radius;
	}
	if(flags & SFMFlagCrashableSizeZMin)
	{
	    model->crashable_size_z_min = value->crashable_size_z_min;
	}
	if(flags & SFMFlagCrashableSizeZMax)
	{
	    model->crashable_size_z_max = value->crashable_size_z_max;
	}

	/* Set given flags */
	model->flags |= flags;

	return(True);
}


/*
 *	Unsets the FDM's flags.
 */
void SFMModelUndefineValue(
	SFMRealmStruct *realm, SFMModelStruct *model, SFMFlags flags
)
{
	if((realm == NULL) || (model == NULL))
	    return;

	/* Leave value on actual member as garbage */

	/* Remove given flags */
	model->flags &= ~flags;
}
