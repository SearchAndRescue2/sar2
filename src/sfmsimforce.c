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
#include "obj.h"
#include "sar.h"
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
	     * coordinates.
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
	if(SFMForceApplyAirDrag(realm, model))
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
		/* Helicopter flight model in flight */
		else
		{
		    /* pitch attitude leveling (bank leveling will be applied later). */
		    if(flags & SFMFlagAttitudeLevelingRate)
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

		    /* The source of the next chunk of code is a sar2 fork by
		     * @metiscus that was manually ported and adapted:
		     * https://github.com/metiscus/sar2/commit/7e94bf4649791d0baa8b9cb5eb5593169be2fbee
		     *
		     * It improves upon the original code doing this, which
		     * has been removed:

			if(!model->landed_state)
			    dir->heading = SFMSanitizeRadians(
			    dir->heading + (sin_pitch * sin_bank *
				(0.2 * PI) *
				time_compensation * time_compression)

		     *
		     * When the helicopter pitches forward and banks, the
		     * heading will then change.
		     *
		     * atan2() becomes smaller if significant Z speed (up or
		     * down) compared to the total airspeed (essentially
		     * hovering, since Z speeds are never too high). In such
		     * case, the helicopter yaws a little less when
		     * rolling. Other than that atan is usually around 0.8.
		     *
		     * The aircraft changes heading depending on the pitch +
		     * compensation from the atan2 above and the bank angle value.
		     *
		     * Original comment by @metiscus in this calculation was that:
		     *
		     * "relative_pitch is the actual helicopter pitch plus the
		     * inflow component of the airspeed. This ensures the
		     * helicopter will turn at a standard rate (15 degree bank 100
		     * knots 3 degrees a second)"
		     *
		     * We have however reduced the PI multiplier to 0.15
		     * instead of 0.2.
		     */

		    // Originally this was part of ArtificialForces, but
		    // now it is here (i don't remember the reason). It is a
		    // thrust-independent effect after all.
		    double airspeed_3d = SFMHypot3(airspeed->x, airspeed->y, airspeed->z);
		    double relative_pitch = sin(dir->pitch) + 0.5f * atan2(ABS(vel->x)+ABS(vel->y), airspeed_3d);
		    dir->heading = SFMSanitizeRadians(
			dir->heading + (relative_pitch * sin(dir->bank) *
					(0.15 * PI) * time_compensation * time_compression)
			);
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

    const SFMFlags		 flags	     = model->flags;
    const SFMDirectionStruct	*dir	     = &model->direction;
    const SFMPositionStruct	*pos	     = &model->position;
    const SFMPositionStruct	*base_wind   = &realm->base_wind_vector;
    const double		 cur_height  = pos->z;
    const double		 mass	     = model->total_mass;
    SFMPositionStruct		*vel	     = &model->velocity_vector;
    SFMPositionStruct		*actual_wind = &realm->actual_wind_vector;
    double			 air_density, total_base_wind, gusts_value, gust_time, gusts_coeff, pc, lift_compensation;

    SFMPositionStruct rel_wind, area, surfaces, drag, accel;

    //SFMBoolean		gear_state	     = model->gear_state;
    //int			gear_type	     = model->gear_type;
    //double			ground_elevation_msl = model->ground_elevation_msl;
    if(!realm->wind_enabled)
    {
	actual_wind->x = 0.0;
	actual_wind->y = 0.0;
	actual_wind->z = 0.0;
    }
    else if(realm->wind_flags & (SAR_WIND_FLAG_GUSTS))
    {
	/* Wind gusts are calculated by using the following function which
	 * returns a different value given the current time. This function has
	 * a period of about 60, so the gusts behaviour will repeat every 60
	 * seconds or so.
	 *
	 * The function adds about 15 knots to the base wind at most, and removes about
	 * 5 knots.
	 */
	total_base_wind = SFMHypot2(base_wind->x, base_wind->y);
	gust_time = (double)cur_millitime / 1000.0;
	gusts_value = total_base_wind +
	    5 + 6 * (cos(PI/6*gust_time)+sin(PI/10*gust_time)-cos(PI/10*gust_time));
	gusts_coeff = gusts_value / total_base_wind;
	actual_wind->x = base_wind->x * gusts_coeff;
	actual_wind->y = base_wind->y * gusts_coeff;
	//printf("gusts_coeff: %f; total_wind: %f; gusts_value: %f\n", gusts_coeff, total_base_wind, gusts_value);
    }

    rel_wind.x = actual_wind->x;
    rel_wind.y = actual_wind->y;
    rel_wind.z = actual_wind->z;
    
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
		// An 2x rectangle length/diameter
		surfaces.x = 2 * 2 * model->belly_height * model->length;
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

	//printf("t: %d, wind: (%.2f,%.2f); wind_rel: (%.2f, %.2f); drag: (%.2f,%.2f,%.2f); accel: (%.2f,%.2f, %.2f)\n", model->type, actual_wind->x, actual_wind->y, rel_wind.x, rel_wind.y,drag.x, drag.y, drag.z, accel.x, accel.y, accel.z);

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
    SFMPositionStruct	*wind		= &realm->actual_wind_vector;
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
	double  airspeed_3d, airspeed_2d;

	double	tc_min		  = MIN(realm->time_compensation, 1.0);
	double  time_compx        = realm->time_compensation * realm->time_compression;

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
	    {
		/* Height coeff range between 0 and 1. We multiply the service
		 * ceiling by x1.35 so that it can realistically reach it given
		 * air drag etc.. It grows very slowly at the beginning and
		 * faster as it reaches 1, at which point the aircraft will
		 * not climb anymore.
		 */
		height_coeff = pos->z / (model->service_ceiling * 1.35);
		height_coeff = MIN(1, height_coeff);
		height_coeff = POW(height_coeff, 4);
	    }
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

	if(flags & SFMFlagAirspeedVector)
	{
	    airspeed_3d = SFMHypot3(airspeed->x, airspeed->y, airspeed->z);
	    airspeed_2d = SFMHypot2(airspeed->x, airspeed->y);
	}

	/* Check if current speed is exceeding its maximum expected
	 * speed (overspeed).
	 */
	if(flags & (SFMFlagSpeedMax | SFMFlagAirspeedVector))
	{
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
			(0.03 * PI) * time_compx)
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
		dic = vel->x * time_compx;
		djc = vel->y * time_compx;
		SFMOrthoRotate2D(dir->heading, &dic, &djc);
		pos->x += dic;
		pos->y += djc;

		/* Calculate new z position (but it will actually be
		 * set farther below).
		 */
		dkc = vel->z * time_compx;
	    }
	    break;

	  /* **************************************************** */
	  case SFMFlightModelHelicopter:
	    /* Helicopter flight model */
	    if(flags & (SFMFlagPosition | SFMFlagDirection |
			SFMFlagVelocityVector | SFMFlagAirspeedVector |
			SFMFlagSpeedStall | SFMFlagSpeedMax |
			SFMFlagAccelResponsiveness | SFMFlagLandedState |
			SFMFlagBellyHeight
		   )
	    )
	    {
		double	sin_pitch = sin(dir->pitch),
			cos_pitch = cos(dir->pitch),
			sin_bank = sin(dir->bank),
			cos_bank = cos(dir->bank);

		/* Rotor effects */

		/* IGE effect (In-Ground-Effect) adds more lift force when close to the ground.
		 * We effectively give thrust_output a maximum of 28% bonus in that region.
		 *
		 * Effect starts at a rotor height of 1.25 rotor diameter.
		 * http://www.copters.com/aero/ground_effect.html
		 */
		if(flags & SFMFlagRotorDiameter &&
		    realm->flight_physics_level >= SFM_FLIGHT_PHYSICS_MODERATE)
		{
		    // Twin-rotor aircrafts note:
		    //   - Coaxial are assumed to experience normal IGE since its a single
		    //     engine after all.
		    //   - Transverse (V22 Ospray): the model engine is already sized at 2x
		    //     I think, so we should be good.

		    // horizontallity_coeff dampens IGE based on how horizontal to the
		    // ground the aircraft is. It can be played with.
		    double horizontallity_coeff = POW(ABS(cos_pitch) * ABS(cos_bank),2);
		    double ige_height = 1.25 * model->rotor_diameter;
		    // The rotor is as high as the center of the aircraft plus
		    // the belly_height (assume distance from center to belly
		    // is the same as from center to rotor. This saves having
		    // to carry exact rotor elevation to the FSM model,
		    // although that could be done.
		    double rotor_height = ABS(pos->z - model->ground_elevation_msl + model->belly_height);
		    // Increases towards 1 when ground_elevation is lower. Non
		    // linear, approximate by the square.
		    double ige_coeff = (1 - POW(CLIP(rotor_height, 0, ige_height) / ige_height, 2));
		    // Increase thrust output 28% at most.
		    // It is always assumed that the ground is horizontal and effect happens
		    // when we are parallel to ground.
		    thrust_output = (1 + 0.28 * ige_coeff * horizontallity_coeff) * thrust_output;
		    // fprintf(stderr, "IGE. hor_coeff: %.2f, coeff: %.2f, thrust_bonus: %.2f\n", horizontallity_coeff, ige_coeff, 1 + 0.28 * ige_coeff);
		}

		/* This is the speed vector relative to the rotor blades.
		   Used to calculate pitch and bank changes. It should match
		   airspeed vector when aircraft horizontal */
		SFMPositionStruct	airspeed_rotor;
		// Set rotor speed vector by rotating airspeeds according to
		// pitch/bank/heading.  Airspeed is already relative to the
		// heading of the aircraft so we don't need to rotate
		// heading. A rotor moving forward will just see y_speed and
		// z_speed (because pitched).
		// FIXME: rotor follows control so it may not be perpendicular to
		// aircraft as assumed here, but close enough.
		double a[3 *1], r[3 * 1];
		a[0] = airspeed->x;
		a[1] = airspeed->y;
		a[2] = airspeed->z;
		MatrixRotateBank3(a, -dir->bank, r);
		MatrixRotatePitch3(r, dir->pitch, a); // re-use variable a
		airspeed_rotor.x = a[0];
		airspeed_rotor.y = a[1];
		airspeed_rotor.z = a[2];

		double airspeed_rotor_2d = SFMHypot2(airspeed_rotor.x, airspeed_rotor.y);
		// fprintf(stderr, "airspeed_rotor. b: %.2f, p: %.2f, h: %.2f x: %.2f, y: %.2f, z: %.2f, 2d: %.2f\n", dir->bank, dir->pitch, dir->heading, a[0], a[1], a[2], airspeed_rotor_2d);



		/* Transverse Flow Effect (TF) happens from around 5 knots,
		 * reaches max magnitude at 15 knots and dissapears by 25
		 * knots. When moving forward, the blades at the front work on
		 * clean air while the back work on downwards-accelerated air.
		 * It causes a roll to the right when moving forward due to
		 * ~90 degree phase shift.
		 * https://en.wikipedia.org/wiki/Transverse_flow_effect
		 */

		if (flags & SFMFlagSingleMainRotor && // does not affect twin as they compensate.
		    realm->flight_physics_level >= SFM_FLIGHT_PHYSICS_REALISTIC &&
		    !model->landed_state &&
		    airspeed_rotor_2d > 0
		    ) {
		    // The effect starts at SFMTFStart and follows a sin wave
		    // incidence until it is 0 again at SFMTFEnd. Otherwise 0.
		    // sin((speed-effect_start)*PI / effect_speed_range)
		    double tf_coeff = sin(
			(CLIP(airspeed_rotor_2d, SFMTFStart, SFMTFEnd) - SFMTFStart) * PI / (SFMTFEnd - SFMTFStart));

		    double tf_coeff_bank = (airspeed_rotor.y / airspeed_rotor_2d) * tf_coeff;
		    double tf_coeff_pitch = -(airspeed_rotor.x / airspeed_rotor_2d) * tf_coeff;
		    dir->pitch = SFMSanitizeRadians(dir->pitch + tf_coeff_pitch * 0.08 * PI * time_compx);
		    dir->bank = SFMSanitizeRadians(dir->bank + tf_coeff_bank * 0.08 * PI * time_compx);
		    // fprintf(stderr, "TF. coeff: %.2f, coeff_bank: %.2f, coeff_pitch: %.2f\n", tf_coeff, tf_coeff_bank, tf_coeff_pitch);
		}

		/* Effective Transactional Lift (ETL): as airspeed increases, air
		 * vortexes at the tip of the rotor blades dissappear,
		 * providing a bonus lift effect. Fully effective at
		 * SFMETLSpeed (24 knots). Without ETL, we suffer a thrust
		 * penalty of up to 25%.
		 *
		 * ETL effect on the advancing side vs retreating side
		 * of the rotor, plus phase shift results in an additional
		 * pitch increase which needs to be compensated by the pilot,
		 * when flying forward. This would need calculating pitch/bank
		 * adjustments based on speed direction.
		 *
		 * https://en.wikipedia.org/wiki/Translational_lift
		 */

		// ETL Thrust penalty
		// Goes from 1 (full penalty) to 0 when it reaches SFMETLSpeed
		// Square progression so that it goes a bit slower close to 0.
		if (realm->flight_physics_level >= SFM_FLIGHT_PHYSICS_MODERATE) {
		    double etl_thrust_coeff = 1 - POW(CLIP(airspeed_rotor_2d / SFMETLSpeed, 0, 1),2);
		    thrust_output = (1 - 0.25 * etl_thrust_coeff) * thrust_output;
		}
		// ETL pitch
		if (realm->flight_physics_level >= SFM_FLIGHT_PHYSICS_REALISTIC &&
		    !model->landed_state &&
		    airspeed_rotor_2d > 0
		    ) {
		    // Similar to TF, we add some pitch/bank changes while
		    // entering ETL. The difference here is that nose rises
		    // when going forward, rather than causing a roll.  By end
		    // of ETL we assume tail has compensated effect and
		    // dissappears.
		    double etl_pitch_bank_coeff = sin(MIN(airspeed_rotor_2d, SFMETLSpeed) * PI / SFMETLSpeed);
		    double etl_bank_coeff = -(airspeed_rotor.x / airspeed_rotor_2d) * etl_pitch_bank_coeff;
		    double etl_pitch_coeff = -(airspeed_rotor.y / airspeed_rotor_2d) * etl_pitch_bank_coeff;
		    dir->pitch = SFMSanitizeRadians(
			dir->pitch + etl_pitch_coeff * 0.03 * PI * time_compx);
		    dir->bank = SFMSanitizeRadians(
			dir->bank + etl_bank_coeff * 0.03 * PI * time_compx);
		    // fprintf(stderr, "ETL: speed: %.2f, thrust_coeff: %.2f, pb_coeff: %.2f, coeff_bank: %.2f, coeff_pitch: %.2f\n", airspeed_rotor_2d, etl_thrust_coeff, etl_pitch_bank_coeff, etl_bank_coeff, etl_pitch_coeff);
		}

		/* Torque: as the motor pushes the rotor Counter-Clock-Wise,
		 * the aircraft is pushed Clock-Wise. At higher speeds the
		 * tail counteracts this but otherwise the pilot should use
		 * the tail rotor. We introduce a heading change to the
		 * right. The effect depends onn thottle and is maximum at 0
		 * speed, reduces up to SFMETLEnd and is counteracted from
		 * there.
		 */

		if(flags & SFMFlagSingleMainRotor && // does not affect twin rotors
		   realm->flight_physics_level >= SFM_FLIGHT_PHYSICS_REALISTIC &&
		   !model->landed_state
		    ) {
		    // torque_coeff: 1 at 0-speed, 0 at SFMETLEnd and negative
		    // above that so that torque acceleration dissappears.
		    double torque_coeff = 1 - airspeed_2d / SFMETLSpeed;
		    double torque_accel;
		    // Remove effect faster than it appears.
		    if (torque_coeff > 0) {
			torque_accel = torque_coeff * 0.2;
		    } else {
			torque_accel = torque_coeff * 0.5;
		    }

		    model->torque_velocity += torque_accel * time_compx;
		    if (model->torque_velocity<0) {
			model->torque_velocity = 0.0;
		    }
		    double dir_change = atan(model->torque_velocity) * 0.1 * model->throttle_coeff * time_compx;
		    dir->heading += dir_change;
		    // fprintf(stderr, "torque accel: %f, torque_vel: %f, atan: %f, throttle_coeff: %f, dir_change: %f\n", torque_accel, model->torque_velocity, atan(model->torque_velocity), model->throttle_coeff, dir_change);
		} else {
		    model->torque_velocity = 0.0;
		}

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

	        /* XY Plane: Pitch and bank applied force */

	        /* Calculate i force compoent (obj relative) */
	        di = (vel->x / 1.00) +
		    (thrust_output * sin_bank * cos_pitch *
		    tc_min / MAX(ar->x, 1.0));
		dic = di * time_compx;
		vel->x = di;

		/* Calculate j force compoent (obj relative) */
		dj = (vel->y / 1.00) +
		    (thrust_output * sin_pitch * cos_bank *
		    tc_min / MAX(ar->y, 1.0));
		djc = dj * time_compx;
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

		dkc = dk * time_compx;
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

		/* Check if above ground plus tolerance (0.3 meters) */
		/* A previous version also set landed_state = False when */
		/* the vel->z was > 0.0. This caused landed state flippin */
		if((pos->z - 0.3) > ground_to_center_height) // || vel->z > 0.0)
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
		    if(ground_speed < 0.02)
			model->stopped = True;
		    else if(ground_speed > 0.1)
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
		//printf("h_con_coeff: %f\n", h_con_coeff);

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
