#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <math.h>
#include "matrixmath.h"
#include "sfm.h"


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

#define RADTODEG(r)     ((r) * 180 / PI)
#define DEGTORAD(d)     ((d) * PI / 180)


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
	double drag_net = 0.0;
	double tc_min = MIN(realm->time_compensation, 1.0);
	double time_compensation = realm->time_compensation;
	double time_compression = realm->time_compression;
	SFMFlags		flags = model->flags;
	SFMDirectionStruct	*dir = &model->direction;
	SFMPositionStruct	*pos = &model->position,
				*vel = &model->velocity_vector;
	SFMBoolean		gear_state = model->gear_state;
	int			gear_type = model->gear_type;
	double			ground_elevation_msl = model->ground_elevation_msl;


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


	/* Handle air friction caused drag by flight model type */
	switch(model->type)
	{
	  case SFMFlightModelAirplane:
	    if(flags & (SFMFlagPosition | SFMFlagDirection |
			SFMFlagVelocityVector | SFMFlagSpeed |
			SFMFlagSpeedMax | SFMFlagDragMin |
			SFMFlagLandedState | SFMFlagEnginePower)
	    )
	    {
		/* Airplane flight model */
		double prev_speed = model->speed;
		double speed_change_coeff;

		double pitch_drop_coeff, pitch_raise_coeff;
		/* Pitch change rates in radians per cycle */
		double pitch_drop_rate = (0.25 * PI);
		double pitch_raise_rate = (0.075 * PI);


		/* Calculate pitch drop coeff, independant of stall coeff
		 * even though pitch_drop_coeff is based on the stall
		 * coeff. pitch_drop_coeff is calculated differently.
		 * it reaches 0.0 at overspeed_expected instead of the
		 * speed_stall.
		 */
		if(model->speed > model->speed_stall)
		{
		    double sdc = 0.10;	/* Drop coeff at stall thres */
		    double sm = model->overspeed_expected - model->speed_stall;
		    double sc = model->speed - model->speed_stall;

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
			    cos(dir->bank);
		    else
			pitch_raise_coeff = 0.0;
		}
		else
		{
		    double sdc = 0.10;	/* Drop coeff at stall thres */
		    double sm = model->speed_stall;
		    double sc = model->speed;

		    /* Calculate pitch drop */
		    if(sm > 0.0)
			pitch_drop_coeff = ((1.0 - MIN(sc / sm, 1.0)) *
			    (1.0 - sdc)) + sdc;
		    else
			pitch_drop_coeff = 1.0;

		    /* Never pitch raise while below stall speeds */
		    pitch_raise_coeff = 0.0;
		}


		/* Calculate net drag (air frame drag) based on the engine
		 * power and the current speed relative to its maximum
		 * speed.  As current speed approaches the maximum speed,
		 * the net drag (airframe drag) approaches the engine power
		 * (which should counteract all engine appied speed).
		 * Note that the net drag must always remain at or above
		 * the minimum drag.
		 */
		if(model->speed_max > 0.0f)
		    drag_net = MAX(
			model->engine_power *
			POW(model->speed / model->speed_max, 1.7f),
			model->drag_min
		    ) * tc_min;

		/* Add air brakes to net drag if deployed */
		if(flags & (SFMFlagAirBrakesState | SFMFlagAirBrakesRate))
		{
		    if(model->air_brakes_state)
			drag_net += model->air_brakes_rate * tc_min;
		}


		/* Reduce speed due to net drag (airframe drag) */
		model->speed -= drag_net;

		/* Increase speed if pitched down due to gravity or
		 * decrease speed if pitched up due to gravity. In either
		 * gravity has its greatest affect with pitched directly
		 * up or down but gravity has minimal affect when pitched
		 * level.
		 */
		model->speed += sin(dir->pitch) * realm->gravity * tc_min;

		/* Keep speed zero or positive */
		if(model->speed < 0.0)
		    model->speed = 0.0;


		/* Calculate the amount of speed changed from the above
		 * drag and gravity calculations, this will be applied
		 * to the velocity vector.
		 */
		if(prev_speed > 0.0)
		    speed_change_coeff = (model->speed / prev_speed);
		else
		    speed_change_coeff = 1.0;

		/* Add speed change to velocity vector */
		vel->x *= speed_change_coeff;	/* Always 0.0 */
		vel->y *= speed_change_coeff;
		vel->z *= speed_change_coeff;


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
			SFMFlagVelocityVector | SFMFlagSpeed |
			SFMFlagSpeedMax | SFMFlagDragMin |
			SFMFlagLandedState | SFMFlagEnginePower)
	    )
	    {
		/* Helicopter flight model */
		double prev_angle;


		/* Calculate net drag */
		if(model->speed_max > 0.0f)
		    drag_net = MAX(
			model->engine_power *
			POW(model->speed / model->speed_max, 1.7f),
			model->drag_min
		    ) * tc_min;

		/* Add air brakes to net drag if deployed */
		if(flags & (SFMFlagAirBrakesState | SFMFlagAirBrakesRate))
		{
		    if(model->air_brakes_state)
			drag_net += model->air_brakes_rate * tc_min;
		}


		/* Apply sideways drag */
		if(vel->x < 0.0)
		{
		    vel->x += drag_net;
		    if(vel->x > 0.0)
		        vel->x = 0.0;
		}
		else
		{
		    vel->x -= drag_net;
		    if(vel->x < 0.0)
			vel->x = 0.0;
		}

		/* Apply forwards/backwards drag */
		if(vel->y < 0.0)
		{
		    vel->y += drag_net;
		    if(vel->y > 0.0)
			vel->y = 0.0;
		}   
		else
		{
		    vel->y -= drag_net;
		    if(vel->y < 0.0)
			vel->y = 0.0;
		}

		/* Set new current speed based on drag applied IJ plane
		 * velocities
		 */
		model->speed = SFMHypot2(vel->x, vel->y);

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
		    SFMFlagVelocityVector | SFMFlagSpeed |
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
		double x_vel_ground_drag = (5.0 * tc_min);	/* Meters per cycle */


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

		    /* Do not modify z velocity */

		    /* Update speed */
		    model->speed = SFMHypot2(vel->x, vel->y);
		}
		else
		{
		    /* Landed and on wheels, apply drag to just the
		     * I compoent direction.
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

		    /* Do not modify z velocity */

		    /* Update speed */
		    model->speed = SFMHypot2(vel->x, vel->y);
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
	double theta, r;
	double di = 0.0, dj = 0.0, dk = 0.0;
	double dic = 0.0, djc = 0.0, dkc = 0.0;
	double thrust_output = 0;
	double net_mass = 0.0;		/* In kg */
	double net_weight;		/* net_mass * SAR_GRAVITY */
	double	ground_elevation_msl = model->ground_elevation_msl,
		center_to_gear_height = 0.0;
	double tc_min = MIN(realm->time_compensation, 1.0);
	double time_compensation = realm->time_compensation;
	double time_compression = realm->time_compression;
	SFMFlags                flags = model->flags;
	SFMDirectionStruct      *dir = &model->direction;
	SFMPositionStruct       *pos = &model->position,
				*vel = &model->velocity_vector,
				*ar = &model->accel_responsiveness;
	SFMBoolean              gear_state = model->gear_state;
	int                     gear_type = model->gear_type;


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

	/* Calculate net mass of object in kg */
	if(flags & SFMFlagTotalMass)
	    net_mass = model->total_mass;
	if(net_mass < 0.0) 
	    net_mass = 0.0;

	/* Calculate net weight of object (gravity in cycles) */
	net_weight = net_mass * realm->gravity;


	/* Calculate thrust output based on altitude and throttle */
	if(flags & (SFMFlagPosition | SFMFlagServiceCeiling |
		    SFMFlagThrottleCoeff | SFMFlagEnginePower)
	)
	{
	    double height_coeff;

	    if(model->service_ceiling > 0.0)
		height_coeff = (pos->z / model->service_ceiling);
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
	if(flags & (SFMFlagSpeedMax | SFMFlagSpeed))
	{
	    /* Current speed greater than expected overspeed? */
	    if(model->speed > model->overspeed_expected)
	    {
		if(realm->overspeed_cb != NULL)
		    realm->overspeed_cb(
			realm,
			model,
			realm->overspeed_cb_client_data,
			model->speed,
			model->overspeed_expected,
			model->overspeed
		    );

		/* Check if model is still valid */
		if(SFMModelInRealm(realm, model) < 0)
		    return(1);
	    }
	}


	/* Handle air friction caused drag by flight model type */
	switch(model->type)
	{
	  case SFMFlightModelAirplane:
	    /* Airplane flight model */
	    if(flags & (SFMFlagPosition | SFMFlagDirection |
			SFMFlagVelocityVector | SFMFlagSpeed |
			SFMFlagSpeedStall | SFMFlagSpeedMax |
			SFMFlagAccelResponsiveness | SFMFlagLandedState)
	    )
	    {
	        double	speed_coeff,	/* Inverse speed coeff, where 1.0
					 * is at standing still and 0.0
					 * is at maximum speed.
					 */
			stall_coeff;	/* The stall coeff, 1.0 is max
					 * stall and 0.0 is no stall.
					 */
	        double	prev_speed;
		/* Velocity sin and cos of pitch and bank, they come
		 * from attitude but are later adjusted for velocity
		 * direction.
		 */
		double sin_pitch, cos_pitch, sin_bank, cos_bank;
		/* New velocity applied by aircraft attitude and thrust
		 * to be added to current velocity.
		 */
		double vel_thrust_mag;
		SFMPositionStruct vel_thrust;


		/* Record previous speed */
		prev_speed = model->speed;

		/* Calculate speed coeff, where 1.0 is standing still
		 * and 0.0 is at maximum.
		 */
		if(model->speed_max > 0.0)
		    speed_coeff = MAX(
			1.0 - (model->speed / model->speed_max),
			0.0
		    );
		else
		    speed_coeff = 0.0;

		/* Update current stall coefficent. 1.0 is highest
		 * stall and 0.0 is no stall.
		 */
		model->stall_coeff = SFMStallCoeff(
		    model->speed,
		    model->speed_stall,
		    model->speed_max
		);
		stall_coeff = model->stall_coeff;


		/* Calculate new pitch and bank trig values */
		sin_pitch = sin(dir->pitch);
		cos_pitch = cos(dir->pitch);
		sin_bank = sin(dir->bank);
		cos_bank = cos(dir->bank);

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

		/* Calculate new thrust vector in meters per cycle,
		 * include tc_min. Use acceleration responsiveness'
		 * j compoent.
		 */
		vel_thrust_mag = thrust_output * tc_min / MAX(ar->y, 1.0);
		vel_thrust.x = 0.0 * vel_thrust_mag;
		vel_thrust.y = cos_pitch * vel_thrust_mag;
		vel_thrust.z = -sin_pitch * vel_thrust_mag;

		/* Add thrust velocity to current velocity */
		vel->x = (0.0 * prev_speed) + vel_thrust.x;
		vel->y = (cos_pitch * prev_speed) + vel_thrust.y;
		vel->z = (-sin_pitch * prev_speed) + vel_thrust.z;


		/* Calculate stalling velocity */
		vel->z -= model->speed_stall *
		    MIN(stall_coeff, 0.25) / MAX(ar->z, 1.0);


		/* Re-update speed based on velocity vector compoents
		 * (ignore x velocity vector compoent for airplane models).
		 */
		if(model->landed_state)
		    model->speed = vel->y;
		else
		    model->speed = SFMHypot2(vel->y, vel->z);

/*
printf(" Y:%f Z:%f  new Y:%f Z%f  S:%f  %f\r",
 vel->y, vel->z, vel_thrust.y, vel_thrust.z, obj_helicopter_ptr->speed,
 ar->z
);
fflush(stdout);
 */

		/* Calculate new x y position */
		r = vel->y * time_compensation * time_compression;
		theta = dir->heading;
		pos->x += r * sin(theta);
		pos->y += r * cos(theta);

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
			SFMFlagVelocityVector | SFMFlagSpeed |
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

		/* Rotate direction with object's scene heading */
		theta = SFMSanitizeRadians((PI / 2) -
		    atan2(dj, di) + dir->heading
		);
		/* Calculate speed on the x y velocity vector plane */
		model->speed = SFMHypot2(di, dj);

		/* Calculate new position */
		r = model->speed * time_compensation * time_compression;
		pos->x += r * sin(theta);
		pos->y += r * cos(theta);


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
		    SFMFlagVelocityVector | SFMFlagSpeed |
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

		/* Check if currently marked `landed'? */
		if(model->landed_state)
		{
		    /* Already landed, check if we landed on mission target.
                       This check is required for flights involving planes
                       that need to taxi to their destination.
                       -- Jesse
                       */
                    realm->touch_down_cb(realm, model, 
                           realm->touch_down_cb_client_data,
                           0);
		}
		else
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

		/* Check if above ground plus tolorance (0.05 meters) */
		if((pos->z - 0.05) > ground_to_center_height)
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

		/* Currently marked landed but velocity going up? */
		if(model->landed_state && (vel->z > 0.0))
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
		    double wheel_brakes_power = (5.0 * tc_min);	/* Meters per second */

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

		    model->speed = SFMHypot2(vel->x, vel->y);
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

	SFMDirectionStruct prev_dir;


	/* Direction and all control positions must be defined */
	if(!(flags & (SFMFlagFlightModelType | SFMFlagDirection |
		      SFMFlagVelocityVector | SFMFlagSpeed |
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
		double theta;


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

		/* Adjust velocity direction due to heading change */
		theta = SFMDeltaRadians(
		    prev_dir.heading, dir->heading
		);
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

	        /* Drag caused by change of heading */
		if(1)
		{
		    double heading_delta = SFMDeltaRadians(
			prev_dir.heading, dir->heading
		    );

		    /* Heading delta must always be positive since only the
		     * net change is important to heading drag regardless
		     * of direction.
		     */
		    if(heading_delta < 0.0)
			heading_delta *= -1.0;

		    vel->y *= MAX(1.0 - (heading_delta / (0.5 * PI)), 0.0);

		    /* Update speed depending on flight model */
		    switch(model->type)
		    {
		      case SFMFlightModelAirplane:
			model->speed = SFMHypot2(vel->z, vel->y);
			break;

		      case SFMFlightModelHelicopter:
			model->speed = SFMHypot2(vel->x, vel->y);
			break;
		    }
		}
#if 0
printf("\r Speed %.2f  Z Vel: %.2f ",
 SFMMPCToMPH(obj_helicopter_ptr->speed),
 SFMMPCToMPH(vel->z)
); fflush(stdout);
#endif
	    }	/* Not landed */
	}

	return(0);
}
