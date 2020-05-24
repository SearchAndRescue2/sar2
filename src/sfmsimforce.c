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
#include <sys/types.h>
#include <math.h>
#include "matrixmath.h"
#include "sfm.h"
#include "sarreality.h"


static SFMBoolean SFMForceTouchDownCheck(
	SFMRealmStruct *realm, SFMModelStruct *model
);
#if 0
static SFMBoolean SFMForceCollisionCheck(
	SFMRealmStruct *realm, SFMModelStruct *model
);
#endif

static int SFMForceApplySlew(
	 SFMRealmStruct *realm, SFMModelStruct *model
);
int SFMForceApplyNatural(
	SFMRealmStruct *realm, SFMModelStruct *model
);

static int SFMForceApplyAirDrag(
	SFMRealmStruct *realm, SFMModelStruct *model
);

void SFMSetAirspeed(
    SFMRealmStruct *realm, SFMModelStruct *model);

int SFMForceApplyArtificial(
	SFMRealmStruct *realm, SFMModelStruct *model
);
static int SFMForceApplyControlSlew(
	SFMRealmStruct *realm, SFMModelStruct *model
);
int SFMForceApplyControl(
	SFMRealmStruct *realm, SFMModelStruct *model
);


#define POW(x,y)        (((x) > 0.0f) ? pow(x,y) : 0.0f)

#define MAX(a,b)        (((a) > (b)) ? (a) : (b))
#define MIN(a,b)        (((a) < (b)) ? (a) : (b))
#define CLIP(a,l,h)     (MIN(MAX((a),(l)),(h)))
#define ABS(x)		(((x) < 0.0) ? ((x) * -1.0) : (x))
#define SQRT(x)              (((x) > 0.0f) ? sqrt(x) : 0.0f)
#define RADTODEG(r)     ((r) * 180 / PI)
#define DEGTORAD(d)     ((d) * PI / 180)

#define GM_DIV_RL (SAR_GRAVITY * SAR_DRY_AIR_MOLAR_MASS) /\
    (SAR_IDEAL_GAS_CONSTANT * SAR_TEMP_LAPSE_RATE)
#define P0M_DIV_RT0 (SAR_SEA_LEVEL_PRESSURE * SAR_DRY_AIR_MOLAR_MASS) /\
    (SAR_IDEAL_GAS_CONSTANT * SAR_SEA_LEVEL_TEMP)
#define L_DIV_T0 SAR_TEMP_LAPSE_RATE / SAR_SEA_LEVEL_TEMP

/*
 *	Checks if the object has come down and impacted the ground
 *	beyond its tolorance. Returns True if a crash occures and
 *	impact_force_coeff will be set to the coefficient value
 *	that has exceeded its touch_down_crash_resistance.
 *
 *	Inputs assumed valid.
 *
 *      Collision notify function specified on the realm will be called.
 */
static SFMBoolean SFMForceTouchDownCheck(
	SFMRealmStruct *realm, SFMModelStruct *model
)
{
	SFMBoolean status = False;
	SFMFlags		flags = model->flags;
	SFMPositionStruct	*vel = &model->velocity_vector;
	double impact_force_coeff = 0.0;

	if(flags & (SFMFlagPosition | SFMFlagVelocityVector |
	            SFMFlagTouchDownCrashResistance)
	)
	{
	    /* Vertical velocity at moment of touch down exceeding
	     * touch down crash resistance?
	     */
	    if(vel->z < -model->touch_down_crash_resistance)
		status = True;

	    /* Calculate impact of crash as a coeff (regardless if
	     * a crash occured or not.
	     */
	    if(model->touch_down_crash_resistance > 0.0)
	    {
		impact_force_coeff = (vel->z /
		    -model->touch_down_crash_resistance);
		if(impact_force_coeff < 0.0)
		    impact_force_coeff = 0.0;
	    }
	}

	/* Report FDM touch down */
	if(realm->touch_down_cb != NULL)
	    realm->touch_down_cb(
		realm, model,
		realm->touch_down_cb_client_data,
		impact_force_coeff
	    );

	/* Inputs may be invalid now, since callback might have
	 * deleted one of them
	 */

	return(status);
}

#if 0
/*
 *	*** Note this function is currently not used! ***
 *
 *	Checks for collision with another object in the realm's
 *	models list. Returns True if a crash occured.
 *
 *      Inputs assumed valid.
 *
 *	Collision notify function specified on the realm will be called.
 */
static SFMBoolean SFMForceCollisionCheck(
	SFMRealmStruct *realm, SFMModelStruct *model
)
{
	int i;
	SFMBoolean status = False;
	SFMFlags	flags = model->flags;
	int		contact_shape = model->crash_contact_shape;
	double		contact_radius = model->crashable_size_radius;
	double r, z, d;
	SFMPositionStruct	*pos = &model->position;
	SFMModelStruct *model2;


	/* Cannot crash into other? */
	if(!(flags & (SFMFlagPosition | SFMFlagCanCrashIntoOther |
		      SFMFlagCrashContactShape)
	))
	    return(status);

	if(!model->can_crash_into_other)
	    return(status);

	/* If crash contact type is cylendrical, make sure z bounds
	 * are set.
	 */
	if(contact_shape == SFMCrashContactShapeCylendrical)
	{
	    if(!(flags & (SFMFlagCrashableSizeZMin | SFMFlagCrashableSizeZMax)
	    ))
		return(status);
	}


	/* Go through all flight models on realm */
	for(i = 0; i < realm->total_models; i++)
	{
	    model2 = realm->model[i];
	    if(model2 == NULL)
		continue;
		 
	    if(!(model2->flags & (SFMFlagPosition | SFMFlagCanCauseCrash |
				  SFMFlagCrashContactShape |
				  SFMFlagCrashableSizeRadius)
	    ))
		continue;
	    if(!model2->can_cause_crash)
		continue;

	    if(model2 == model)	/* Skip itself */
		continue;

	    /* Calculate distance in meters */  
	    r = SFMHypot2(
		model2->position.x - pos->x,
		model2->position.y - pos->y
	    );
	    /* Definatly not in contact? */
	    if((r - contact_radius - model2->crashable_size_radius) > 0)
		continue;

/* Note, we need to calculate the collision impact coeff */

	    /* Our object contact shape spherical? */
	    if(contact_shape == SFMCrashContactShapeSpherical)
	    {
		/* Check target object contact shape */
		if(model2->crash_contact_shape == SFMCrashContactShapeSpherical)
		{
		    /* Spherical to spherical */

		    z = model2->position.z - pos->z;
		    d = SFMHypot2(r, z);
		    if((d - contact_radius - model2->crashable_size_radius) <= 0)
		    {
			/* Call collision notify */
			status = True;

			if(realm->collision_cb != NULL)
			    realm->collision_cb(
				realm, model, model2,
				realm->collision_cb_client_data,
				1.1
			    );

			/* Callback deleted model? */
			if(SFMModelInRealm(realm, model) < 0)
			    return(status);  
		    }
		}
		else if((model2->crash_contact_shape == SFMCrashContactShapeCylendrical) &&
			(model2->flags & (SFMFlagCrashableSizeZMin | SFMFlagCrashableSizeZMax))
		)
		{
		    /* Spherical to cylendrical */

		    z = model2->position.z;
 /* Not too accurate check on z */
		    if((pos->z >= (z + model2->crashable_size_z_min)) &&
		       (pos->z <= (z + model2->crashable_size_z_max))
		    )
		    {
			/* Call collision notify */
			status = True;

			if(realm->collision_cb != NULL)
			    realm->collision_cb(
				realm, model, model2,
				realm->collision_cb_client_data,
				1.1
			    );

			/* Callback deleted model? */
			if(SFMModelInRealm(realm, model) < 0)
			    return(status);
		    }
		}
	    }
	    /* Our object contact shape cylendrical? */
	    else if(contact_shape == SFMCrashContactShapeCylendrical)
	    {
		/* Check target object contact shape */
		if(model2->crash_contact_shape == SFMCrashContactShapeSpherical)
		{
		    /* Cylendrical to spherical */
		    z = model2->position.z;
 /* Not too accurate check on z */
		    if((z >= (pos->z + model->crashable_size_z_min)) &&
		       (z <= (pos->z + model->crashable_size_z_max))
		    )
		    {
			/* Call collision notify */
			status = True;

			if(realm->collision_cb != NULL)
			    realm->collision_cb(
				realm, model, model2,
				realm->collision_cb_client_data,
				1.1
			    );

			/* Callback deleted model? */
			if(SFMModelInRealm(realm, model) < 0)
			    return(status);
		    }
		}
		else if((model2->crash_contact_shape == SFMCrashContactShapeCylendrical) &&
			(model2->flags & (SFMFlagCrashableSizeZMin | SFMFlagCrashableSizeZMax))
		)
		{
		    /* Cylendrical to cylendrical */
		    double z1_min = pos->z + model->crashable_size_z_min,
			   z1_max = pos->z + model->crashable_size_z_max;
		    double z2_min = model2->position.z + model2->crashable_size_z_min,
			   z2_max = model2->position.z + model2->crashable_size_z_max;

		    if((z1_min <= z2_max) &&
		       (z2_min <= z1_max)
		    )
		    {
			/* Call collision notify */
			status = True;

			if(realm->collision_cb != NULL)
			    realm->collision_cb(
				realm, model, model2,
				realm->collision_cb_client_data,
				1.1
			    );

			/* Callback deleted model? */
			if(SFMModelInRealm(realm, model) < 0)
			    return(status);
		    }
		}
	    }
	}
		
	return(status);
}
#endif

/*
 *	Applies slew forces to the FDM from its current control
 *	positions.
 *
 *	This function should be called from SFMForceApplyArtificial().
 */
static int SFMForceApplySlew(
	 SFMRealmStruct *realm, SFMModelStruct *model
)
{
	double time_compensation = realm->time_compensation;
	double time_compression = realm->time_compression;
	SFMFlags		flags = model->flags;
	double          h_con_coeff = CLIP(
				model->heading_control_coeff, -1.0, 1.0
	);
	double          p_con_coeff = CLIP(
				model->pitch_control_coeff, -1.0, 1.0
	);
	double          b_con_coeff = CLIP(
				model->bank_control_coeff, -1.0, 1.0
	);
	SFMDirectionStruct      *dir = &model->direction;
	SFMPositionStruct       *pos = &model->position;
	double			ground_elevation_msl = model->ground_elevation_msl;


	if((flags & SFMFlagPosition) &&
	   (flags & SFMFlagDirection) &&
	   (flags & SFMFlagGroundElevation) &&
	   (flags & SFMFlagHeadingControlCoeff) &&
	   (flags & SFMFlagBankControlCoeff) &&
	   (flags & SFMFlagPitchControlCoeff) &&
	   (flags & SFMFlagThrottleCoeff)
	)
	{
	    double a[3], r[3];
	    double b_con_coeff_p3 = b_con_coeff * b_con_coeff * b_con_coeff;
	    double p_con_coeff_p3 = p_con_coeff * p_con_coeff * p_con_coeff;


	    /* Calculate change in position relative to FDM's current
	     * control positions (not the FDM's current velocity)
	     *
	     * Because for slew, the controller positions dictate the
	     * velocity (instead of velocity dictating velocity)
	     */
	    a[0] = (b_con_coeff_p3 * 10000.0) *
		time_compensation * time_compression;
	    a[1] = (p_con_coeff_p3 * 10000.0) *
		time_compensation * time_compression;
	    a[2] = 0.0;

	    /* Rotate position change so that it is relative to world 
	     * cooridnates.
	     */
	    MatrixRotateHeading3(a, dir->heading, r);

	    /* Move position */
	    pos->x += r[0];
	    pos->y += r[1];
	    pos->z += r[2];

	    /* Keep above ground level */
	    if(pos->z < ground_elevation_msl)
		pos->z = ground_elevation_msl;

	    /* Turn */
	    dir->heading = SFMSanitizeRadians(
		dir->heading + (h_con_coeff * (0.5 * PI) *
		time_compensation * time_compression)
	    );

	    /* Update internally maintained ground to center of object
	     * height.
	     */
	    model->center_to_ground_height = ground_elevation_msl - pos->z;
	}

	return(0);
}


/*
 *	Applies natural forces to the FDM:
 *
 *	* Air/ground drag & attitude leveling
 *
 *	* Wind force
 *
 *	* Recalculate ground to center height
 *
 *	* Callbacks will be called when needed.
 *
 *	Returns non-zero if the FDM was deleted.
 *
 *	Inputs assumed valid.
 */
int SFMForceApplyNatural(
	SFMRealmStruct *realm, SFMModelStruct *model
)
{
	double			 tc_min		      = MIN(realm->time_compensation, 1.0);
	double			 time_compensation    = realm->time_compensation;
	double			 time_compression     = realm->time_compression;
	SFMFlags		 flags		      = model->flags;
	SFMDirectionStruct	*dir		      = &model->direction;
	SFMPositionStruct	*pos		      = &model->position;
	SFMPositionStruct	*vel		      = &model->velocity_vector;
	SFMPositionStruct	*airspeed	      = &model->airspeed_vector;
	SFMBoolean		 gear_state	      = model->gear_state;
	int			 gear_type	      = model->gear_type;
	double			 ground_elevation_msl = model->ground_elevation_msl;


	/* Flight model type must be defined */
	if(!(flags & SFMFlagFlightModelType))
	    return(0);

	/* If in slew mode, then skip applying natural forces */
	if(model->type == SFMFlightModelSlew)
	    return(0);


	/* Update internally maintained ground to center of object height */
	if(flags & (SFMFlagPosition | SFMFlagGroundElevation))
	{
	    model->center_to_ground_height = ground_elevation_msl - pos->z;
	}
	else
	{
	    model->center_to_ground_height = 0.0;
	}

	/* Apply air drag and wind. */
	if(SFMForceApplyAirDrag(realm,model))
	    return(1);

	/* Handle air friction caused drag by flight model type */
	switch(model->type)
	{
	  case SFMFlightModelAirplane:

	    if(flags & (SFMFlagPosition | SFMFlagDirection |
			SFMFlagVelocityVector | SFMFlagAirspeedVector |
			SFMFlagSpeedMax | SFMFlagDragMin |
			SFMFlagLandedState | SFMFlagEnginePower)
	    )
	    {
		/* Airplane flight model */

		double sin_pitch = sin(dir->pitch),
		    cos_pitch = cos(dir->pitch),
		    sin_bank = sin(dir->bank),
		    cos_bank = cos(dir->bank);

		/* Effective speed used to calculate if we are stalling */
		double stall_speed = SFMCurrentSpeedForStall(airspeed->y, airspeed->z, dir->pitch);
		/* Ground speed */
		double prev_abs_speed = SFMHypot2(vel->y, vel->z);

		/* Pitch change rates in radians per cycle */
		double pitch_drop_rate = (0.25 * PI);
		double pitch_raise_rate = (0.075 * PI);
		double pitch_drop_coeff, pitch_raise_coeff;

		/* Speed with a gravity modifier */
		double gravity_speed_zy;
		/* Change coeff of speed with gravity vs current speed */
		double speed_change_coeff;


		/* Calculate pitch drop coeff, independant of stall coeff
		 * even though pitch_drop_coeff is based on the stall
		 * coeff. pitch_drop_coeff is calculated differently.
		 * it reaches 0.0 at overspeed_expected instead of the
		 * speed_stall.
		 */
		if(stall_speed >  model->speed_stall)
		{
		    double sdc = 0.10;	/* Drop coeff over stall thres */
		    double sm = model->overspeed_expected - model->speed_stall;
		    double sc = stall_speed - model->speed_stall;

		    /* Calculate pitch drop */
		    if(sm > 0.0)
			pitch_drop_coeff = (1.0 - MIN(sc / sm, 1.0)) *
			    sdc;
		    else
			pitch_drop_coeff = sdc;

		    /* Calculate pitch raise, 0.0 at stall, 1.0 at
		     * overspeed_expected.
		     */
		    if(sm > 0.0)
			pitch_raise_coeff = MIN(sc / sm, 1.0) * 
			    cos_bank;
		    else
			pitch_raise_coeff = 0.0;
		}
		else
		{
		    double sdc = 0.10;	/* Drop coeff under stall thres */
		    double sm = model->speed_stall;
		    double sc = stall_speed;

		    /* Calculate pitch drop */
		    if(sm > 0.0)
			pitch_drop_coeff = ((1.0 - MIN(sc / sm, 1.0)) *
			    (1.0 - sdc)) + sdc;
		    else
			pitch_drop_coeff = 1.0;

		    /* Never pitch raise while below stall speeds */
		    pitch_raise_coeff = 0.0;
		}

		/* Increase speed if pitched down due to gravity or decrease
		 * speed if pitched up due to gravity. In either gravity has
		 * its greatest affect with pitched directly up or down but
		 * gravity has minimal affect when pitched level.
		 *
		 * This compensates not having a real airdrag model that
		 * accounts for lift produced by the winds.
		 *
		 * Also, if we are banking, that part of the gravity speed is
		 * transferred to X axis.
		 */
		gravity_speed_zy = prev_abs_speed +
		    realm->gravity * tc_min * (sin_pitch - ABS(sin_bank * cos_pitch));

		/* Keep speed zero or positive */
		if(gravity_speed_zy < 0.0)
		    gravity_speed_zy = 0.0;

		/* Calculate the amount of speed changed from the above
		 * gravity calculation, and apply to the to the velocity
		 * vector.
		 */
		if(prev_abs_speed > 0.0)
		    speed_change_coeff = (gravity_speed_zy / prev_abs_speed);
		else
		    speed_change_coeff = 1.0;

		/* Add speed change to velocity vector */
		vel->y *= speed_change_coeff;
		vel->z *= speed_change_coeff;

		/* An airplane flying sideways should get some X-speed.
		 */
		vel->x += realm->gravity * tc_min * sin_bank * cos_pitch;

		/* Apply pitch drop? */
		if((pitch_drop_coeff > 0.0) &&
		   (flags & SFMFlagAttitudeLevelingRate)
		)
		{
		    double prev_angle = dir->pitch;

		    /* Check if landed */
		    if(model->landed_state)
		    {
			/* On the ground pitch leveling */
			if(dir->pitch > (1.0 * PI))
			{
			    /* Pitch is currently up, so pitch down */
			    dir->pitch = SFMSanitizeRadians(
				dir->pitch + (pitch_drop_rate * time_compensation *
				time_compression * pitch_drop_coeff)
			    );
			    if((dir->pitch < prev_angle) &&
			       (dir->pitch > (0.0 * PI))
			    )
				dir->pitch = (0.0 * PI);
			}
			else
			{
			    /* Landed on ground and pitch is negative,
			     * so pitch back to ground level.
			     */
			    dir->pitch = (0.0 * PI);
			}
		    }
		    else
		    {
			/* In flight pitch drop */
			if(dir->pitch > (1.0 * PI))
			{
			    /* Pitch is currently up, so pitch down */
			    dir->pitch = SFMSanitizeRadians(
				dir->pitch + (pitch_drop_rate * time_compensation *
				time_compression * pitch_drop_coeff)
			    );
			    if((dir->pitch < prev_angle) &&
			       (dir->pitch > (0.5 * PI))
			    )
				dir->pitch = (0.5 * PI);
			}
			else
			{
			    /* Pitch is currently down, keep pitching down */
			    dir->pitch = SFMSanitizeRadians(
				dir->pitch + (pitch_drop_rate * time_compensation *
				time_compression * pitch_drop_coeff)
			    );
			    if(dir->pitch > (0.5 * PI))
				dir->pitch = (0.5 * PI);
			}
		    }
		}
		/* Apply pitch raise? Note that this will negate some
		 * of the pitch drop applied above. Also pitch raise
		 * can be negative.
		 */
		if((pitch_raise_coeff != 0.0) &&
		   (flags & SFMFlagAttitudeLevelingRate)
		)
		{
		    dir->pitch = SFMSanitizeRadians(
			dir->pitch - (pitch_raise_rate *
			time_compensation * time_compression *
			pitch_raise_coeff)
		    );
		    /* Pitch flipping check */
		    if((dir->pitch > (0.5 * PI)) &&
		       (dir->pitch < (1.5 * PI))
		    )
		    {
			dir->heading = SFMSanitizeRadians(dir->heading + PI);
			dir->bank = SFMSanitizeRadians(dir->bank + PI);
			if(dir->pitch > (1.0 * PI))
			    dir->pitch = SFMSanitizeRadians(
				(2.0 * PI) - dir->pitch + PI
			    );
			else
			    dir->pitch = SFMSanitizeRadians(
				PI - dir->pitch
			    );
		    }
		}
	    }
	    break;

	  /* ***************************************************** */
	  case SFMFlightModelHelicopter:
	    if(flags & (SFMFlagPosition | SFMFlagDirection |
			SFMFlagVelocityVector | SFMFlagAirspeedVector |
			SFMFlagSpeedMax | SFMFlagDragMin |
			SFMFlagLandedState | SFMFlagEnginePower)
	    )
	    {
		/* Helicopter flight model */
		double prev_angle;

		/* Helicopter flight model landed? */
		if((model->landed_state) &&
		   (flags & SFMFlagAttitudeLevelingRate)
		)
		{
		    /* Landed so apply ground leveling */
		    double	pitch_change_rate = DEGTORAD(3.0),
				pitch_leveling_rate;
		    double	pitch_up_max = (2.0 * PI) - pitch_change_rate,
				pitch_down_max = (0.0 * PI) + pitch_change_rate;

		    prev_angle = dir->pitch;	/* Record pitch */

		    /* Begin applying pitch ground leveling */

		    /* Check if pitch is outside the controller affected
		     * pitch range
		     */
		    if((dir->pitch < pitch_up_max) &&
		       (dir->pitch > pitch_down_max)
		    )
		    {
			/* Apply rough pitch leveling */
			pitch_leveling_rate = DEGTORAD(90.0);

			/* Pitched up? */
			if(dir->pitch > (1.0 * PI))
			{
			    /* Pitched up so need to pitch down */
			    dir->pitch += pitch_leveling_rate *
				time_compensation * time_compression;
			    if(dir->pitch > pitch_up_max)
				dir->pitch = pitch_up_max;
			}
			else
			{
			    /* Pitched down so need to pitch up */
			    dir->pitch -= pitch_leveling_rate * 
				time_compensation * time_compression;
			    if(dir->pitch < pitch_down_max)
				dir->pitch = pitch_down_max;
		        }
		    }
		    /* Apply mild pitch leveling */
		    pitch_leveling_rate = DEGTORAD(2.0);
		    /* Pitched up? */
		    if(dir->pitch > (1.0 * PI))
		    {
			/* Pitched up so need to pitch down */
			dir->pitch += pitch_leveling_rate *
			    time_compensation * time_compression;
			if(dir->pitch  > (2.0 * PI))
			    dir->pitch = (0.0 * PI);
		    }
		    else
		    {
			/* Pitched down so need to pitch up */
			dir->pitch -= pitch_leveling_rate *
			    time_compensation * time_compression;
			if(dir->pitch < (0.0 * PI))  
			    dir->pitch = (0.0 * PI);
		    }
		}
		/* Helicopter flight model in flight pitch attitude
	         * leveling (bank leveling will be applied later).
		 */
		else if(flags & SFMFlagAttitudeLevelingRate)
		{
		    double leveling_coeff;

		    if(dir->pitch > (1.5 * PI))
		    {
			leveling_coeff = MIN(
			    (dir->pitch - (1.5 * PI)) / (0.5 * PI),
			    1.0
			);
			dir->pitch = MIN(
			    dir->pitch + (leveling_coeff *
			    model->attitude_leveling_rate.pitch *
				time_compensation * time_compression),
			    2.0 * PI
			);
		    }
		    else if(dir->pitch < (0.5 * PI))
		    {
			leveling_coeff = MAX(
			    ((0.5 * PI) - dir->pitch) / (0.5 * PI),
			    0.0
			);
			dir->pitch = MAX(
			    dir->pitch - (leveling_coeff *
			    model->attitude_leveling_rate.pitch *
				time_compensation * time_compression),
			    0.0 * PI
			);
		    }
		}
	    }
	    break;
	}

	/* ******************************************************** */
	/* Apply ground drag and levelings regardless of flight
	 * model type.
	 */
	if(flags & (SFMFlagPosition | SFMFlagDirection |
		    SFMFlagVelocityVector | SFMFlagAirspeedVector |
		    SFMFlagSpeedMax | SFMFlagDragMin |
		    SFMFlagLandedState | SFMFlagEnginePower)
	)
	{
	    /* Check if landed */
	    if(model->landed_state)
	    {
		/* Is landed so handle ground friction caused drag plus
		 * pitch and bank ground leveling
		 */
		double prev_angle;
		/* Drag so that wheels do not skid to the sides.
		 * Value rather arbitrary.
		 */
		double x_vel_ground_drag = (10.0 * tc_min);	/* Meters per cycle */

		/* Small drag for wheels when rolling forward or backwards */
		double y_vel_ground_drag = (0.1 * tc_min);	/* Meters per cycle */

		/* Applying strong ground drag if landing gear is not
		 * down or gear type is not wheels.
		 */
		if((gear_type != SFMGearTypeWheels) ||
		   !gear_state
		)
		{
		    /* Landing gear up or landing gear is not of
		     * type SFMGearTypeWheels.
		     */
		    if(vel->x < 0.0)
		    {
			vel->x += x_vel_ground_drag;
			if(vel->x > 0.0)
			    vel->x = 0.0;
		    }
		    else
		    {
			vel->x -= x_vel_ground_drag;
			if(vel->x < 0.0) 
			    vel->x = 0.0;
		    }

		    /* Use x velocity ground drag for y axis */
		    if(vel->y < 0.0)
		    {
			vel->y += x_vel_ground_drag;
			if(vel->y > 0.0)
			    vel->y = 0.0;
		    }
		    else
		    {   
			vel->y -= x_vel_ground_drag;
			if(vel->y < 0.0)
			    vel->y = 0.0;
		    }
		}
		else
		{
		    /* Landed and on wheels, apply drags to the X and Y
		     * components.
		     */

		    if(vel->x < 0.0)
		    {
			vel->x += x_vel_ground_drag;
			if(vel->x > 0.0)
			    vel->x = 0.0;
		    }
		    else
		    {
			vel->x -= x_vel_ground_drag;
			if(vel->x < 0.0)
			    vel->x = 0.0;
		    }

		    /* While little, wheels also drag when going
		     * forward/backwards.
		     */
		    if(vel->y < 0.0)
		    {
			vel->y += y_vel_ground_drag;
			if(vel->y > 0.0)
			    vel->y = 0.0;
		    }
		    else
		    {
			vel->y -= y_vel_ground_drag;
			if(vel->y < 0.0)
			    vel->y = 0.0;
		    }
		}

		/* Apply ground bank leveling? */
		if(flags & SFMFlagAttitudeLevelingRate)
		{
		    prev_angle = dir->bank;
		    if(dir->bank > (1.0 * PI))
		    {
		        dir->bank = SFMSanitizeRadians(
			    dir->bank + (1.5 * time_compensation *
			    time_compression)
		        );
		        if(dir->bank < prev_angle)
			    dir->bank = (0.0 * PI);
		        else if(dir->bank < (1.0 * PI))
			    dir->bank = (0.0 * PI);
		    }
		    else
		    {
		        dir->bank = SFMSanitizeRadians(
			    dir->bank - (1.5 * time_compensation *
			    time_compression)
		        );
		        if(dir->bank > prev_angle)
			    dir->bank = (0.0 * PI);
		        else if(dir->bank > (1.0 * PI))
			    dir->bank = (0.0 * PI);
		    }
		}
	    }
	    else	/* Check if landed */
	    {
		/* Not landed so in flight */
		double leveling_coeff;

		/* Apply in flight bank leveling? */
		if(flags & SFMFlagAttitudeLevelingRate)
		{
		    if(dir->bank < (1.0 * PI))
		    {
		        leveling_coeff = MAX(
			    1.0 - (dir->bank / (0.5 * PI)), 0.0
		        );
		        dir->bank = MAX(
			    dir->bank - (leveling_coeff *
			    model->attitude_leveling_rate.bank *
			        time_compensation * time_compression),
			    0.0
		        );
		    }
		    else
		    {
		        leveling_coeff = MAX(
			    1.0 - (((2.0 * PI) - dir->bank) / (0.5 * PI)),
			    0.0
			);
		        dir->bank = dir->bank + (leveling_coeff * 
			    model->attitude_leveling_rate.bank *
			    time_compensation * time_compression
		        );
		        if(dir->bank >= (2.0 * PI))
			    dir->bank = 0.0 * PI;
		    }
		}
	    }	/* In flight */
	}

	return(0);
}


/*
 * Apply air drag to the aircrafts. The drag will be different depending on
 * the reaml's wind, so calculations are made depending on the relative speed between
 * the aircraft and the wind.
 *
 * This follows the aerodynamic drag formulas by calculating first the surface
 * area exposed to each velocity component, given the aircraft orientation,
 * followed by the air density at the current height and putting all that
 * together to produce acceleration components that depend on the aircraft's
 * total mass. The effect of the wind can be tuned by adjusting the aircraft's
 * drag parameter.
 *
 * This is called from SFMForceApplyNatural.
 */
static int SFMForceApplyAirDrag(
	SFMRealmStruct *realm, SFMModelStruct *model
)
{
    double tc_min = MIN(realm->time_compensation, 1.0);

    SFMFlags		 flags	    = model->flags;
    SFMDirectionStruct	*dir	    = &model->direction;
    SFMPositionStruct	*pos	    = &model->position;
    SFMPositionStruct	*vel	    = &model->velocity_vector;
    SFMPositionStruct	*wind	    = &realm->wind_vector;
    double		 cur_height = pos->z;
    double		 mass	    = model->total_mass;
    double		 air_density, pc, lift_compensation;

    SFMPositionStruct rel_wind, area, surfaces, drag, accel;

    //SFMBoolean		gear_state	     = model->gear_state;
    //int			gear_type	     = model->gear_type;
    //double			ground_elevation_msl = model->ground_elevation_msl;
    rel_wind.x = wind->x;
    rel_wind.y = wind->y;
    rel_wind.z = wind->z;

    if(flags & (SFMFlagTotalMass | SFMFlagLandedState |
		SFMFlagPosition | SFMFlagDirection |
		SFMFlagVelocityVector | SFMFlagAirspeedVector |
		SFMFlagBellyHeight)
	)
    {
	/* Rotate wind coordinates so that they match the velocity ones (which
	 * are relative to the heading of the aircraft.
	 */
	SFMOrthoRotate2D(-dir->heading, &rel_wind.x, &rel_wind.y);

	/* Calculate relative wind speeds to the aircraft */
	rel_wind.x -= vel->x;
	rel_wind.y -= vel->y;
	rel_wind.z -= -vel->z;

	/* Calculate the surface area of the aircraft on every component:
	 *   X is seen from the sides
	 *   Y is seen from the back/front
	 *   Z is seen from the top
	 *
	 * These are rough approximations based on known dimensions.
	 */
	switch(model->type)
	{
	    case SFMFlightModelAirplane:
		// An 1.5x rectangle length/diameter
		surfaces.x = 1.5 * 2 * model->belly_height * model->length;
		// 2x Circle of belly_height radius
		surfaces.y = 2 * PI * model->belly_height * model->belly_height;
		// 1/3rd of the rectangle
		surfaces.z = model->wingspan * model->length / 3;
		break;
	    case SFMFlightModelHelicopter:
		// Asume a rectangle
		surfaces.x = 2 * model->belly_height * model->length;
		// A circle of belly_height radius
		surfaces.y = PI * model->belly_height * model->belly_height;
		// Half a rectangle
		surfaces.z = 0.5 * 2 * model->belly_height * model->length;
		break;
	}

	/* Air brakes deployed? That increases the Y surface. */
	if(flags & (SFMFlagAirBrakesState | SFMFlagAirBrakesArea))
	{
	    if(model->air_brakes_state)
		surfaces.y += model->air_brakes_area;
	}

	//printf("sx: %f.2; sy: %f.2; sz: %f.2;\n", surfaces.x, surfaces.y, surfaces.z);

	/* Calculate the actual areas exposed to each wind component. They depend on the
	 * current values of bank/pitch.
	 */

	// Area exposed to X wind
	area.x = ABS(cos(dir->bank)) * surfaces.x + ABS(sin(dir->bank)) * surfaces.z;
	// Area exposed to Y wind
	area.y = ABS(cos(dir->pitch)) * surfaces.y + ABS(sin(dir->pitch)) * surfaces.z;
	// Area exposed to Z wind
	area.z = ABS(sin(dir->pitch)) * surfaces.y +
	    ABS(cos(dir->pitch) * cos(dir->bank)) * surfaces.z +
	    ABS(cos(dir->pitch) * sin(dir->bank)) * surfaces.x;
	//printf("ax: %f.2; ay: %f.2; az: %f.2;\n", area.x, area.y, area.z);

	/* Obtain air density at current height.
	 * https://en.wikipedia.org/wiki/Density_of_air#Variation_with_altitude
	 */
	air_density = P0M_DIV_RT0 * POW(1 - (L_DIV_T0 * cur_height), GM_DIV_RL - 1);

	// pc = 1/2 * density * drag coeff. We will add speed and area for
	// every component separately.
	pc = 0.5 * air_density * model->drag_min;

	/* Obtain aerodynamic drag force for each component.
	 * https://en.wikipedia.org/wiki/Drag_%28physics%29#Types_of_drag
	 * https://physics.info/drag/
	 *
	 * Since v^2 would lose the magnitude sign we just keep it with ABS(v) * v.
	 */
	drag.x = pc * area.x * ABS(rel_wind.x) * rel_wind.x;
	drag.y = pc * area.y * ABS(rel_wind.y) * rel_wind.y;
	drag.z = pc * area.z * ABS(rel_wind.z) * rel_wind.z;

	/* On airplanes there is in practice excessive Y drag when pitching up
	 * or down. Since we do not have a lift force model, this does not
	 * translate on increased/decreased Z speed. When going up there is
	 * additionally more gravity. This results in a penalty when climbing.
	 *
	 * We adjust by adding "lift compensation".  (sin_pitch*cos_pitch)
	 * maximum is 0.5 at 45 degrees and -0.5 a -45 degrees. It is 0 at 0 deg.
	 * This reduces y-drag by 0.5 when pitched at 45 degrees. By then,
	 * the gravity and thrust calculations transfer enough y-velocity to
	 * the Z axis so that we do not notice the Y-drag so much anymore.
	 */

	if(model->type == SFMFlightModelAirplane && model->stall_coeff == 0)
	{
	    // Note it cannot be > 1.
	    lift_compensation = ABS(sin(dir->pitch)*cos(dir->pitch));
	    drag.y*= (1 - lift_compensation);
	    // reduce z drag only when going up
	    if(vel->z > 0)
		drag.z*= (1 - 2*lift_compensation);
	}

	// Acceleration (F=ma). The units are m/cycles^2
	accel.x = drag.x / mass;
	accel.y = drag.y / mass;
	accel.z = -drag.z / mass;

	// printf("t: %d, wind: (%.2f,%.2f); wind_rel: (%.2f, %.2f); drag: (%.2f,%.2f,%.2f); accel: (%.2f,%.2f, %.2f)\n", model->type, wind->x, wind->y, rel_wind.x, rel_wind.y,drag.x, drag.y, drag.z, accel.x, accel.y, accel.z);

	// Adjust velocity components. When aircraft is landed the wheels drag
	// should compensate small winds (particularly side winds).
	vel->x += accel.x * tc_min;
	vel->y += accel.y * tc_min;
	vel->z += accel.z * tc_min;
    }
    return (0);
}

/*
 * Set the model's airspeed vector from the velocity vector using the current
 * wind.
 *
 * This is called once from simmanage.c.
 */
void SFMSetAirspeed(
    SFMRealmStruct *realm, SFMModelStruct *model)
{

    SFMFlags		 flags		= model->flags;
    SFMDirectionStruct	*dir		= &model->direction;
    SFMPositionStruct	*vel		= &model->velocity_vector;
    SFMPositionStruct	*airspeed	= &model->airspeed_vector;
    SFMPositionStruct	*wind		= &realm->wind_vector;
    double		 rotated_wind_x = wind->x, rotated_wind_y = wind->y;

    if(flags & (SFMFlagVelocityVector | SFMFlagAirspeedVector))
    {
	// Rotate wind to match velocity vector (relative to aircraft).
	SFMOrthoRotate2D(-dir->heading, &rotated_wind_x, &rotated_wind_y);

	airspeed->x = vel->x - rotated_wind_x;
	airspeed->y = vel->y - rotated_wind_y;
	airspeed->z = vel->z;
	//printf("vel_x: %.2f,  vel_y: %.2f, vel_z: %.2f, rel_x: %.2f, rel_y: %.2f\n", vel->x, vel->y, vel->z, airspeed->x, airspeed->y);
    }
}

/*
 *	Apply artificial forces; thrust and brakes.
 *
 *	If model is in SFMFlightModelSlew mode then this function will
 *	call the appropriate slew function.
 *
 *	Notify callbacks may be called, return will be non-zero if the
 *	model was deleted and/or no longer valid.
 *
 *	Inputs assumed valid.
 */
int SFMForceApplyArtificial(
	SFMRealmStruct *realm, SFMModelStruct *model
)
{
	double	di		      = 0.0, dj = 0.0, dk = 0.0;
	double	dic		      = 0.0, djc = 0.0, dkc = 0.0;
	double	thrust_output	      = 0;
	double	net_weight	      = 0.0;	/* net_mass * SAR_GRAVITY */
	double	ground_elevation_msl  = model->ground_elevation_msl,
		center_to_gear_height = 0.0;
	double  airspeed_3d;

	double	tc_min		  = MIN(realm->time_compensation, 1.0);
	double	time_compensation = realm->time_compensation;
	double	time_compression  = realm->time_compression;

	SFMFlags		 flags		   = model->flags;
	SFMDirectionStruct      *dir		   = &model->direction;
	SFMPositionStruct       *pos		   = &model->position;
	SFMPositionStruct	*vel		   = &model->velocity_vector;
	SFMPositionStruct	*airspeed	   = &model->airspeed_vector;
	SFMPositionStruct	*ar		   = &model->accel_responsiveness;
	SFMBoolean		 gear_state	   = model->gear_state;
	int			 gear_type	   = model->gear_type;


	/* Flight model type must be defined */
	if(!(flags & SFMFlagFlightModelType))
	    return(0);

	/* Check if in slew mode, if so then apply slew forces */
	if(model->type == SFMFlightModelSlew)
	    return(SFMForceApplySlew(realm, model));

	/* Calculate center to gear height */
	if(flags & (SFMFlagBellyHeight | SFMFlagGearState |
		    SFMFlagGearType | SFMFlagGearHeight)
	)
	{
	    center_to_gear_height = model->belly_height + (
		(gear_state) ? model->gear_height : 0.0
	    );
	}

	/* Calculate net weight of object (gravity in cycles) */
	if(flags & SFMFlagTotalMass)
	    net_weight = model->total_mass * realm->gravity;

	if(net_weight < 0.0)
	    net_weight = 0.0;


	/* Calculate thrust output based on altitude and throttle */
	if(flags & (SFMFlagPosition | SFMFlagServiceCeiling |
		    SFMFlagThrottleCoeff | SFMFlagEnginePower)
	)
	{
	    double height_coeff;

	    if(model->service_ceiling > 0.0)
		/* If we go above the service ceiling the coeff should stay at 1. */
		height_coeff = MIN(1, (pos->z / model->service_ceiling));
	    else
		height_coeff = 0.0;

	    thrust_output = model->throttle_coeff *
		model->engine_power * MAX(
		    1.0 - height_coeff, 0.0
		);
	    if(thrust_output < 0.0)
		thrust_output = 0.0;

	    /* Add afterburner to thrust? */
	    if(flags & (SFMFlagAfterBurnerState | SFMFlagAfterBurnerPowerCoeff))
	    {
		if(model->after_burner_state)
		{
		    thrust_output += ((model->after_burner_power_coeff *
			model->engine_power) * MAX(
			    1.0 - height_coeff, 0.0
			));
		}
	    }
	}

	/* Check if current speed is exceeding its maximum expected
	 * speed (overspeed).
	 */
	if(flags & (SFMFlagSpeedMax | SFMFlagAirspeedVector))
	{
	    /* Use all airspeeds. */
	    airspeed_3d = SFMHypot3(airspeed->x, airspeed->y, airspeed->z);
	    /* Current speed greater than expected overspeed? */
	    if(airspeed_3d > model->overspeed_expected)
	    {
		if(realm->overspeed_cb != NULL)
		    realm->overspeed_cb(
			realm,
			model,
			realm->overspeed_cb_client_data,
			airspeed_3d,
			model->overspeed_expected,
			model->overspeed
		    );

		/* Check if model is still valid */
		if(SFMModelInRealm(realm, model) < 0)
		    return(1);
	    }
	}


	switch(model->type)
	{
	  case SFMFlightModelAirplane:
	    /* Airplane flight model */
	    if(flags & (SFMFlagPosition | SFMFlagDirection |
			SFMFlagVelocityVector | SFMFlagAirspeedVector |
			SFMFlagSpeedStall | SFMFlagSpeedMax |
			SFMFlagAccelResponsiveness | SFMFlagLandedState)
	    )
	    {
	        double	speed_coeff;	/* Inverse speed coeff, where 1.0 is
					 * at standing still and 0.0 is at
					 * maximum speed.
					 */
		double	stall_coeff;	/* The stall coeff, 1.0 is max
					 * stall and 0.0 is no stall.
					 */
		double	safe_stall_coeff; /* Similar to above, but at higher
					  * speeds
					  */

		double	sin_pitch = sin(dir->pitch);
		double	cos_pitch = cos(dir->pitch);
		double	sin_bank  = sin(dir->bank);

		//printf("sin_bank: %.3f; cos_bank: %.3f; sin_pitch: %.3f; cos_pitch: %.3f\n", sin_bank, cos_bank, sin_pitch, cos_pitch);

	        double	prev_ground_speed, prev_airspeed, vel_adjustment;

		/* New velocity applied by aircraft attitude and thrust
		 * to be added to current velocity.
		 */
		double vel_thrust_mag;
		SFMPositionStruct vel_thrust;

		double current_speed_for_stall = SFMCurrentSpeedForStall(airspeed->y, airspeed->z, dir->pitch);

		// Speeds in YZ axis. Used for stalling calculations, thrust etc.
		if(model->landed_state)
		{
		    prev_airspeed = ABS(airspeed->y);
		    prev_ground_speed = ABS(vel->y);
		}
		else
		{
		    prev_airspeed= SFMHypot2(airspeed->y, airspeed->z);
		    prev_ground_speed = SFMHypot2(vel->y, vel->z);
		}

		/* Calculate speed coeff, where 1.0 is standing still
		 * and 0.0 is at maximum.
		 */
		if(model->speed_max > 0.0)
		    speed_coeff = MAX(
			1.0 - (prev_airspeed / model->speed_max),
			0.0
		    );
		else
		    speed_coeff = 0.0;

		/* Calculate stall coefficient. 1.0 is highest
		 * stall and 0.0 is no stall.
		 */
		stall_coeff = SFMStallCoeff(
		    current_speed_for_stall,
		    model->speed_stall,
		    model->speed_max
		);

		/* Calculate safe stall. Unlike the previous, this one
		 * triggers at a higher speed and represents velocity in which
		 * the airplane can lose height without being in full stall.
		 */
		safe_stall_coeff = SFMStallCoeff(
		    current_speed_for_stall,
		    model->speed_stall * 1.75,
		    model->speed_max
		    );

		//printf("cur speed stall: %.3f, speed stall: %.3f; stall coeff: %.3f\n", current_speed_for_stall, model->speed_stall, stall_coeff);

		/* Calculate heading change caused by bank, the
		 * heading change becomes minimal when the speed reaches
		 * the maximum and heading change is 0.04 * PI radians
		 * per cycle when the speed is at minimum.
		 */
		if(!model->landed_state)
		    dir->heading = SFMSanitizeRadians(
			dir->heading + (sin_bank * speed_coeff *
			(0.03 * PI) * time_compensation *
		        time_compression)
		    );

		/* Calculate new thrust vector in meters per cycle, include
		 * tc_min. Use acceleration responsiveness' y compoent.
		 *
		 * The key here is that Y velocity needs to be converted to Z
		 * velocity when pitching up/down. X and Y are only relative to the
		 * aircraft heading but remain in the 2D XY plane.
		 *
		 * Therefore, thrust is assigned to Y or Z based on pitch.
		 */

		vel_thrust_mag = thrust_output * tc_min / MAX(ar->y, 1.0);
		vel_thrust.x = 0.0 * vel_thrust_mag; // thrust does not affect X directly
		vel_thrust.y = cos_pitch * vel_thrust_mag; // flying forward, Y increases
		vel_thrust.z = -sin_pitch * vel_thrust_mag; // flying up, Z increases
		//printf("thrust y: %.3f, pitch: %.3f\n", vel_thrust.y, dir->pitch);

		/* This applies when we are recovering from stall (current stall smaller than
		 * the aircraft's). We do not let the stall factor recover too quickly.
		 * The problem was that the stall factor would sink and this would add speed,
		 * which would make the stall factor sink again, resulting in a quadratic
		 * speed gain. This makes it more realistic to get out of a stall by having
		 * to wait regardless of the speed gained.
		 */
		if(!model->landed_state && stall_coeff < model->stall_coeff) {
		    stall_coeff = MAX(0, model->stall_coeff - 0.1 * tc_min);
		}

		vel_adjustment = stall_coeff;
		if(model->landed_state)
		    vel_adjustment = 0.0;

		/* Adjust Y/Z velocities by adding thrust, but also by taking into account the current
		 * stall factor. If we are stalling, velocity is greatly impacted.
		 */
		vel->y = (cos_pitch * prev_ground_speed * (1- vel_adjustment)) + vel_thrust.y;
		vel->z = (-sin_pitch * prev_ground_speed * (1- vel_adjustment)) + vel_thrust.z;
		model->stall_coeff = stall_coeff;

		/* Now, additionally increase Z speed when stalling.  We use
		 * two stall values:
		 *
		 * The "normal" one given by stall_coeff represents the loss
		 *     of aerodynamic lift and triggers alarms.
		 *
		 * The "safe" one given by safe_stall_coeff represents gliding
		 * at low speeds, which allows an airplane pitched up to lose some
		 * height (usually right before landing).
		 */

		vel->z -= (model->speed_stall / MAX(ar->z, 1.0) *
			   (MIN(stall_coeff, 1) + (1.75 * MIN(safe_stall_coeff, 1))));

		/* Calculate new x y position: rotate speed to match scene
		 * heading.
		 */
		dic = vel->x * time_compensation * time_compression;
		djc = vel->y * time_compensation * time_compression;
		SFMOrthoRotate2D(dir->heading, &dic, &djc);
		pos->x += dic;
		pos->y += djc;

		/* Calculate new z position (but it will actually be
		 * set farther below).
		 */
		dkc = vel->z * time_compensation * time_compression;
	    }
	    break;

	  /* **************************************************** */
	  case SFMFlightModelHelicopter:
	    /* Helicopter flight model */
	    if(flags & (SFMFlagPosition | SFMFlagDirection |
			SFMFlagVelocityVector | SFMFlagAirspeedVector |
			SFMFlagSpeedStall | SFMFlagSpeedMax |
			SFMFlagAccelResponsiveness | SFMFlagLandedState)
	    )
	    {
		double	sin_pitch = sin(dir->pitch),
			cos_pitch = cos(dir->pitch),
			sin_bank = sin(dir->bank),
			cos_bank = cos(dir->bank);

		/* To calculate movement offset, use the equation:
		 *
		 * (last_vel / compilation) + ((thrust_in_direction) / response)
		 *
		 * Compilation is a value from 1 to infinity, the closer
		 * it is to 1, the stronger the velocity compilation.
		 *
		 * Response is how sensitive the thrust (force) is
		 * applied, lower values increase the responsiveness.
		 */

		/* Heading by pitch and bank caused by thrust direction.
		 * When the helicopter pitches forward and banks, the
		 * heading will then change.
		 */
		if(!model->landed_state)
		    dir->heading = SFMSanitizeRadians(
			dir->heading + (sin_pitch * sin_bank *
			    (0.2 * PI) *
			    time_compensation * time_compression)
		    );


	        /* XY Plane: Pitch and bank applied force */

	        /* Calculate i force compoent (obj relative) */
	        di = (vel->x / 1.00) +
		    (thrust_output * sin_bank * cos_pitch *
		    tc_min / MAX(ar->x, 1.0));
		dic = di * time_compensation * time_compression;
		vel->x = di;

		/* Calculate j force compoent (obj relative) */
		dj = (vel->y / 1.00) +
		    (thrust_output * sin_pitch * cos_bank *
		    tc_min / MAX(ar->y, 1.0));
		djc = dj * time_compensation * time_compression;
		vel->y = dj;

		/* Set new positions after rotating the speeds to match
		 * the scene's heading.
		 */
		SFMOrthoRotate2D(dir->heading, &dic, &djc);
		pos->x += dic;
		pos->y += djc;


		/* Vertical velocity */
/*
		dk = (vel->z / 1.60) +
		    (((thrust_output * cos_pitch * cos_bank * tc_min) -
		    (net_weight * tc_min))
		    / MAX(ar->z, 1.0));
 */
/*
		dk = (vel->z / (1.0 + (3.0 * tc_min))) +
		    (((thrust_output * cos_pitch * cos_bank * tc_min) -
		    (net_weight * tc_min))
		    / MAX(ar->z, 1.0));
 */
		if(1)
		{
		    /* Better solution for vertical velocity,
		     * but with one small problem, with good frame
		     * rates, tc_min will be really small which makes
		     * the convergence to dk_new take longer than
		     * it would if tc_min was big (poor frame rates).
		     */
		    double dk_prev, dk_new, dk_dv;

		    dk_prev = vel->z;
		    dk_new = ((thrust_output * cos_pitch * cos_bank) -
			(net_weight)) / MAX(ar->z, 1.0);
		    dk_dv = dk_new - dk_prev;

		    /* Add the previous velocit to the new delta change
		     * in velocity multiplied by the time compensation 
		     * min. Make sure tc_min stays about 0.01 to ensure
		     * that as this function cycles that it will converge
		     * to dk.
		     */
		    dk = dk_prev + (dk_dv * MAX(tc_min / 1.0, 0.01));
		}
		/* No negative velocity if landed */
		if((dk < 0.0) && model->landed_state)
		    dk = 0.0;

		dkc = dk * time_compensation * time_compression;
		vel->z = dk;
	    }
	    break;
	}

	/* ************************************************************ */
	/* Set z position for all flight model types (note that
	 * dkc was calculated above).
	 */
	if(flags & (SFMFlagPosition | SFMFlagDirection |
		    SFMFlagVelocityVector | SFMFlagAirspeedVector |
		    SFMFlagGroundElevation | SFMFlagServiceCeiling |
		    SFMFlagBellyHeight | SFMFlagLandedState)
	)
	{
	    /* Adjust vertical position (take land into account).
	     * Variable dkc should be properly calculated from above.
	     */
	    double ground_to_center_height;
	    double prev_z = pos->z;	/* Record previous z position */

	    /* Now set new Z position */
	    pos->z += dkc;


	    /* Contact with ground? */
	    ground_to_center_height = ground_elevation_msl + center_to_gear_height;
	    if(pos->z <= ground_to_center_height)
	    {
		/* Reset z position (leave z velocity alone) */
/*              vel->z = 0.0; */
		pos->z = ground_to_center_height;

		if(!model->landed_state)
		{
		    /* Was previously not landed */
		    model->landed_state = True;

		    /* Just touched down? */
		    if(prev_z > ground_to_center_height)
		    {
			/* Calculate touch down impact force and
			 * and call touch down notify function.
			 */
			SFMForceTouchDownCheck(
			    realm, model
			);
		    }
		    else
		    {
			/* Was previously `under the ground' but not
			 * landed. So we do not call SFMForceTouchDownCheck()
			 * because we know that the impact force is 0.0.
			 * So just call touch down callback with a impact
			 * force of 0.0.
			 */
			if(realm->touch_down_cb != NULL)
			    realm->touch_down_cb(
				realm, model,
				realm->touch_down_cb_client_data,
				0.0		/* Impact force coeff */
			    );
		    }

		    /* Check if model is still valid */
		    if(SFMModelInRealm(realm, model) < 0)
			return(1);
		}
	    }
	    else
	    {
		/* Is currently above ground */

		/* Check if above ground plus tolerance (0.5 meters) */
		/* A previous version also set landed_state = False when */
		/* the vel->z was > 0.0. This caused landed state flippin */
		if((pos->z - 0.5) > ground_to_center_height) // || vel->z > 0.0)
		{
		    /* Very much above ground, so mark as not landed */
		    if(model->landed_state)
		    {
			/* Mark as not landed */
			model->landed_state = False;

			/* Call airborne callback */
			if(realm->airborne_cb != NULL)
			    realm->airborne_cb(
				realm,
				model,
				realm->airborne_cb_client_data
			    );
			/* Check if model is still valid after callback */
			if(SFMModelInRealm(realm, model) < 0)
			    return(1);
		    }
		}
	    }


	    /* Air brakes are applied in SFMForceApplyNatural() */

	    /* Apply wheel brakes if landed */
	    if(flags & (SFMFlagGearState | SFMFlagGearType |
			SFMFlagGearBrakesState)
	    )
	    {
		if(model->landed_state &&
		   gear_state && (gear_type == SFMGearTypeWheels) &&
		   model->gear_brakes_state
		)
		{
		    double wheel_brakes_power = (10.0 *
						 model->gear_brakes_coeff *
						 tc_min);    /* Meters per second */

		    if(vel->y < 0)
		    {
			vel->y += wheel_brakes_power;
			if(vel->y > 0) 
			    vel->y = 0;
		    }
		    else
		    {
			vel->y -= wheel_brakes_power;
			if(vel->y < 0)
			    vel->y = 0;
		    }
		}
	    }

	    /* Check if stopped on land */
	    if (flags & (SFMFlagStopped | SFMFlagLandedState))
	    {
		if(model->landed_state)
		{
		    SFMBoolean prev_stopped = model->stopped;
		    double ground_speed = SFMHypot2(vel->x, vel->y);
		    /* We stop if below 0.01. But only consider to be moving again
		     * if above 0.05. This avoids flipping. */
		    if(ground_speed < 0.01)
			model->stopped = True;
		    else if(ground_speed > 0.05)
			model->stopped = False;

		    if(model->stopped && !prev_stopped) {
			/* Some aircrafts may need to taxi to destination and we need
			   to check if they have arrived. We cannot do this on every
			   cycle when we are landed as it is way too expensive.
			   So we just do it when we stop.
			*/

			if(realm->parked_cb != NULL)
			    realm->parked_cb(model, realm->parked_cb_client_data);
		    }
		} else {
		    model->stopped = False;
		}
	    }
	}

	return(0);
}


/*
 *	Applies control positions for slew mode.
 *
 *	This function should be called from SFMForceApplyControl().
 */
static int SFMForceApplyControlSlew(
	SFMRealmStruct *realm, SFMModelStruct *model
)
{
	/* Changing of control positions do not affect slew */
	return(0);
}

/*
 *	Apply flight model control positions to change model's
 *	attitude (direction).
 *
 *	Notify callbacks may be called, return will be non-zero if the
 *	model was deleted and/or no longer valid.
 *
 *	Inputs assumed valid.
 */
int SFMForceApplyControl(
	SFMRealmStruct *realm, SFMModelStruct *model
)
{
	double	time_compensation = realm->time_compensation,
		time_compression = realm->time_compression;

	SFMFlags	flags = model->flags;
	SFMDirectionStruct	*dir = &model->direction;
	SFMPositionStruct	*vel = &model->velocity_vector;
	SFMDirectionStruct	*acr = &model->attitude_change_rate;
	double		h_con_coeff = CLIP(
				model->heading_control_coeff, -1.0, 1.0
	);
	double          p_con_coeff = CLIP(
				model->pitch_control_coeff +
				model->elevator_trim_coeff, -1.0, 1.0
	);
	double          b_con_coeff = CLIP(
				model->bank_control_coeff, -1.0, 1.0
	);

	SFMDirectionStruct	prev_dir;
	double			theta;


	/* Direction and all control positions must be defined */
	if(!(flags & (SFMFlagFlightModelType | SFMFlagDirection |
		      SFMFlagVelocityVector | SFMFlagAirspeedVector |
		      SFMFlagHeadingControlCoeff | SFMFlagBankControlCoeff |
		      SFMFlagPitchControlCoeff)
	))
	    return(0);


	/* If in slew mode, skip control handling */
	if(model->type == SFMFlightModelSlew)
	    return(SFMForceApplyControlSlew(realm, model));


	/* Record previous attitude */
	memcpy(&prev_dir, dir, sizeof(SFMDirectionStruct));


	/* Handle by flight model type */
	switch(model->type)
	{
	  case SFMFlightModelAirplane:
	    /* Airplane flight model */

	    break;


	}


	/* Landed state defined? */
	if(flags & SFMFlagLandedState)
	{
	    /* Is landed? */
	    if(model->landed_state)
	    {
		/* Landed */

		/* Bank by bank (helicopter only) */
		if(model->type == SFMFlightModelHelicopter)
		    dir->bank = SFMSanitizeRadians(
			dir->bank + (b_con_coeff * acr->bank *
		        time_compensation * time_compression)
		    );

		/* Pitch by pitch */
		dir->pitch = SFMSanitizeRadians(
		    dir->pitch + (cos(dir->bank) * p_con_coeff * acr->pitch *
	            time_compensation * time_compression)
		);
		/* Pitch flipping check */
		if((dir->pitch > (0.5 * PI)) &&
		   (dir->pitch < (1.5 * PI))
		)
		{
		    dir->heading = SFMSanitizeRadians(dir->heading + PI);
		    dir->bank = SFMSanitizeRadians(dir->bank + PI);
		    if(dir->pitch > (1.0 * PI))
			dir->pitch = SFMSanitizeRadians(
			    (2.0 * PI) - dir->pitch + PI 
			);
		    else
			dir->pitch = SFMSanitizeRadians(
			    PI - dir->pitch
			);
		}
		/* Heading by pitch */
		dir->heading = SFMSanitizeRadians(
		    dir->heading + (sin(dir->bank) * p_con_coeff *
		    acr->pitch * -1 * time_compensation * time_compression)
		);

		/* Ground turning */
		if(flags & (SFMFlagGearTurnVelocityOptimul |
			    SFMFlagGearTurnVelocityMax |
			    SFMFlagGearTurnRate)
		)
		{
		    /* Heading by heading (turning) */
		    if((model->gear_turn_velocity_optimul > 0.0) &&
		       (model->gear_turn_velocity_max > 0.0)
		    )
		    {
/* TODO: Leaving model->gturn_radius out of the calculation, we are
 * just using a constant turn rate (in radians per second) for now
 */
			const double turn_rate = (0.2 * PI);
			double turn_coeff;

			/* Calculate turning coefficient, the amount of
			 * turning possible based on speed. The
			 * turning coefficient is optimul (best turning)
			 * when the current velocity is at
			 * gear_turn_velocity_optimul.
			 */
			if(vel->y < 0.0)
			{
			    /* Moving backwards */
			    turn_coeff = (vel->y /
				model->gear_turn_velocity_optimul);
			    if(turn_coeff < -1.0)
				turn_coeff = -MAX(
				    1.0 + (vel->y /
				    model->gear_turn_velocity_max),
				    0.0
				);
			}
			else
			{
			    /* Moving forwards */
			    turn_coeff = (vel->y /
				model->gear_turn_velocity_optimul);
			    if(turn_coeff > 1.0)
				turn_coeff = MAX(
				    1.0 - (vel->y /
				    model->gear_turn_velocity_max),
				    0.0
				);
			}
			/* Apply heading control to turn coefficient to
			 * calculate the actual amount of turning rate
			 * coefficient.
			 */
			turn_coeff = turn_coeff * h_con_coeff * turn_rate;

			/* Heading by ground turning */
			dir->heading = SFMSanitizeRadians(
			    dir->heading + (cos(dir->bank) * turn_coeff *
			    time_compensation * time_compression)
			);

			/* Pitch by ground turning */
			dir->pitch = SFMSanitizeRadians(
			    dir->pitch + (sin(dir->bank) * turn_coeff *
			    time_compensation * time_compression)
			);
			/* Bank by ground turning */
			dir->bank = SFMSanitizeRadians(
			    dir->bank + (sin(dir->pitch) * turn_coeff *
			    -1 * time_compensation * time_compression)
			);
		    }	/* Heading by heading (turning) */
		}	/* Ground landing gear turning defined? */
	    }
	    else
	    {
		/* Not landed */

		/* Bank by bank */
		dir->bank = SFMSanitizeRadians(
		    dir->bank + (b_con_coeff * acr->bank *
		    time_compensation * time_compression)
		);

		/* Pitch by pitch */
		dir->pitch = SFMSanitizeRadians(
		    dir->pitch + (cos(dir->bank) * p_con_coeff * acr->pitch *
		    time_compensation * time_compression)
		);
		/* Pitch flipping check */
		if((dir->pitch > (0.5 * PI)) &&
		   (dir->pitch < (1.5 * PI))
		)
		{
		    dir->heading = SFMSanitizeRadians(dir->heading + PI);
		    dir->bank = SFMSanitizeRadians(dir->bank + PI);
		    if(dir->pitch > (1.0 * PI))
			dir->pitch = SFMSanitizeRadians(
			    (2.0 * PI) - dir->pitch + PI 
			);
		    else
			dir->pitch = SFMSanitizeRadians(
			    PI - dir->pitch
			);
		}

		/* Heading by pitch */
		dir->heading = SFMSanitizeRadians(
		    dir->heading + (sin(dir->bank) * p_con_coeff *
		    acr->pitch * -1 * time_compensation * time_compression)
		);


		/* Check if engines are on */
		if(1)
		{
		    /* Heading by heading (rudders) */
		    dir->heading = SFMSanitizeRadians(
			dir->heading + (cos(dir->bank) * h_con_coeff *
			acr->heading * time_compensation * time_compression)
		    );
		    /* Pitch by heading */
		    dir->pitch = SFMSanitizeRadians(
			dir->pitch + (sin(dir->bank) * h_con_coeff *
			acr->heading * time_compensation * time_compression)
		    );
		    /* Bank by heading */
		    dir->bank = SFMSanitizeRadians(
			dir->bank + (sin(dir->pitch) * h_con_coeff *
			acr->heading * -1 * time_compensation * time_compression)
		    );
		}

		/* We avoid adding additional drags here. Consider them handled by the
		 * aerodynamic drag model.
		 */

	        /* /\* Drag caused by change of heading *\/ */
		/* if(0) */
		/* { */
		/*     /\* Heading delta must always be positive since only the */
		/*      * net change is important to heading drag regardless */
		/*      * of direction. */
		/*      *\/ */
		/*     double heading_delta = ABS( */
		/* 	SFMDeltaRadians(prev_dir.heading, dir->heading)); */


		/*     printf("heading ch drag: %f\n", MAX(1.0 - (heading_delta / (0.9 * PI)), 0.0)); */
		/*     vel->y *= MAX(1.0 - (heading_delta / (0.5 * PI)), 0.0); */
		/* } */

		/* /\* Drag caused by change of pitch *\/ */
		/* if(0) */
		/* { */
		/*     /\* Heading delta must always be positive since only the */
		/*      * net change is important to pitch drag regardless */
		/*      * of direction. */
		/*      *\/ */
		/*     double pitch_delta = ABS(SFMDeltaRadians( */
		/* 	prev_dir.pitch, dir->pitch)); */

		/*     vel->y *= MAX(1.0 - (pitch_delta / (0.9 * PI)), 0.0); */
		/* } */

		/* /\* Drag caused on X axis by change of bank *\/ */
		/* if(0) */
		/* { */
		/*     /\* Heading delta must always be positive since only the */
		/*      * net change is important to pitch drag regardless */
		/*      * of direction. */
		/*      *\/ */
		/*     double pitch_delta = ABS(SFMDeltaRadians( */
		/* 	prev_dir.pitch, dir->pitch)); */

		/*     vel->x *= MAX(1.0 - (pitch_delta / (0.9 * PI)), 0.0); */
		/* } */

	    }	/* Not landed */


	    /* Adjust velocity direction due to heading change.
	     * This needs to happen any time we turn.
	     */
	    theta = SFMDeltaRadians(prev_dir.heading, dir->heading);
	    if(theta != 0.0)
	    {
		double a[3], r[3];

		a[0] = vel->x;
		a[1] = vel->y;
		a[2] = vel->z;
		MatrixRotateHeading3(a, -theta, r);
		vel->x = r[0];
		vel->y = r[1];
		vel->z = r[2];
	    }
	}

	return(0);
}
