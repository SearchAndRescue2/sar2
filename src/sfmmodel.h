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

/*
		 SAR Flight Model - Flight Dynamics Model

	Contains the flight dyanmics parameters for simulation of
	each model.

	When adding a parameter, make sure the functions;
	SFMModelChangeValues() and SFMModelUndefineValue() get updated
	to support the new parameter.
 */

#ifndef SFMMODEL_H
#define SFMMODEL_H

#include "sfmtypes.h"


/*
 *	Flight model defined parameter flags:
 */
#define SFMFlagFlightModelType		((SFMFlags)1 << 0)
#define SFMFlagPosition			((SFMFlags)1 << 1)
#define SFMFlagDirection		((SFMFlags)1 << 2)
#define SFMFlagVelocityVector		((SFMFlags)1 << 3)
#define SFMFlagAirspeedVector		((SFMFlags)1 << 4)
#define SFMFlagSpeedStall		((SFMFlags)1 << 5)
#define SFMFlagDragMin			((SFMFlags)1 << 6)
#define SFMFlagSpeedMax			((SFMFlags)1 << 7)	/* And overspeed_expected and overspeed. */
#define SFMFlagAccelResponsiveness	((SFMFlags)1 << 8)
#define SFMFlagGroundElevation		((SFMFlags)1 << 9)
#define SFMFlagServiceCeiling		((SFMFlags)1 << 10)
#define SFMFlagBellyHeight		((SFMFlags)1 << 11)
#define SFMFlagGearState		((SFMFlags)1 << 12)	/* Landing gear state. */
#define SFMFlagGearType			((SFMFlags)1 << 13)
#define SFMFlagGearHeight               ((SFMFlags)1 << 14)
#define SFMFlagGearBrakesState		((SFMFlags)1 << 15)
#define SFMFlagGearTurnVelocityOptimul	((SFMFlags)1 << 16)
#define SFMFlagGearTurnVelocityMax	((SFMFlags)1 << 17)
#define SFMFlagGearTurnRate		((SFMFlags)1 << 18)
#define SFMFlagLandedState		((SFMFlags)1 << 19)
#define SFMFlagGroundContactType	((SFMFlags)1 << 20)
#define SFMFlagHeadingControlCoeff	((SFMFlags)1 << 21)
#define SFMFlagBankControlCoeff		((SFMFlags)1 << 22)
#define SFMFlagPitchControlCoeff	((SFMFlags)1 << 23)
#define SFMFlagThrottleCoeff		((SFMFlags)1 << 24)
#define SFMFlagAfterBurnerState		((SFMFlags)1 << 25)
#define SFMFlagAfterBurnerPowerCoeff	((SFMFlags)1 << 26)
#define SFMFlagEnginePower		((SFMFlags)1 << 27)
#define SFMFlagTotalMass		((SFMFlags)1 << 28)
#define SFMFlagAttitudeChangeRate	((SFMFlags)1 << 29)
#define SFMFlagAttitudeLevelingRate	((SFMFlags)1 << 30)
#define SFMFlagAirBrakesState		((SFMFlags)1 << 31)
#define SFMFlagAirBrakesArea		((SFMFlags)1 << 32)
#define SFMFlagCanCrashIntoOther	((SFMFlags)1 << 33)
#define SFMFlagCanCauseCrash		((SFMFlags)1 << 34)
#define SFMFlagCrashContactShape	((SFMFlags)1 << 35)
#define SFMFlagCrashableSizeRadius	((SFMFlags)1 << 36)
#define SFMFlagCrashableSizeZMin	((SFMFlags)1 << 37)
#define SFMFlagCrashableSizeZMax	((SFMFlags)1 << 38)
#define SFMFlagTouchDownCrashResistance ((SFMFlags)1 << 39)
#define SFMFlagCollisionCrashResistance ((SFMFlags)1 << 40)
#define SFMFlagStopped			((SFMFlags)1 << 41)
#define SFMFlagLength			((SFMFlags)1 << 42)
#define SFMFlagWingspan			((SFMFlags)1 << 43)

/*
 *	Flight model types:
 */
#define SFMFlightModelAirplane		0
#define SFMFlightModelHelicopter	1
#define SFMFlightModelSlew		2


/*
 *	Landing gear types:
 */
#define SFMGearTypeWheels		0
#define SFMGearTypeSkis			1
#define SFMGearTypeFloats		2


/*
 *	Ground types:
 */
#define SFMGroundTypeLandUnpaved	0
#define SFMGroundTypeLandPaved		1
#define SFMGroundTypeWaterCalm		2
#define SFMGroundTypeWaterRough		3

/*
 *	Crash contact shapes:
 */
#define SFMCrashContactShapeSpherical	0
#define SFMCrashContactShapeCylendrical	1



/*
 *	Flight Dynamics Model structure:
 *
 *	Generally, units for position are in meters and
 *	velocity/speed/rate are in meters per cycle.
 */
typedef struct {

	/* Flags indicating which parameters (members in the
	 * structure) are defined (non-garbage). Since the
	 * SFMFlags type has up to 64 bits, we can have up to
	 * 64 defineable members in this structure, use them
	 * wisely! Members marked as `internal' do not have
	 * an associated flag.
	 */
	SFMFlags	flags;

	int			type;		/* One of SFMFlightModel*. */
	SFMPositionStruct	position;	/* Meters. */
	SFMDirectionStruct	direction;	/* Radians. */
	SFMPositionStruct	velocity_vector;	/* Meters/cycle. */
	SFMPositionStruct	slew_velocity_vector;	/* Internal, used only
							 * when type is set to
							 * SFMFlightModelSlew.
							 */
	SFMPositionStruct       airspeed_vector;	/* Meters/cycle. */
	double			speed_stall,	/* Meters/cycle. */
				stall_coeff;	/* Internal. */
	double			drag_min;	/* Meters/cycle. */
	double			speed_max;	/* Meters/cycle. */
	double			overspeed_expected;	/* Meters/cycle. */
	double			overspeed;	/* Meters/cycle. */
	SFMPositionStruct	accel_responsiveness;
	double			ground_elevation_msl;	/* Meters. */
	double			service_ceiling;	/* Meters. */
	double			length;		/* Meters */
	double			wingspan;	/* Meters */
	double			belly_height;	/* Undercarrage to center, meters. */
	SFMBoolean		gear_state;	/* True when down. */
	int			gear_type;	/* One of SFMGearType*. */
	double			gear_height;	/* Meters. */
	SFMBoolean		gear_brakes_state;
	double			gear_brakes_coeff;
	double			gear_turn_velocity_optimul;	/* Meters/cycle. */
	double			gear_turn_velocity_max;	/* Meters/cycle. */
	double			gear_turn_rate;
	SFMBoolean		landed_state;	/* True if landed. */
	SFMBoolean		stopped;	/* True if not moving on land. */
	int			ground_contact_type;	/* One of SFMGroundType*. */
	double			center_to_ground_height;	/* Internal, center of
								 * object to touchable
								 * ground height,
								 * in meters.
								 */
	double			heading_control_coeff;	/* -1.0 to 1.0. */
	double			pitch_control_coeff;	/* -1.0 to 1.0. */
	double			bank_control_coeff;	/* -1.0 to 1.0. */
	double			elevator_trim_coeff;	/* -1.0 to 1.0. */
	double			throttle_coeff;		/* 0.0 to 1.0. */
	SFMBoolean		after_burner_state;
	double			after_burner_power_coeff;	/* Times engine power. */
	double			engine_power;	/* In kg * m / cycle^2. */
	double			total_mass;	/* In kg. */
	SFMDirectionStruct	attitude_change_rate;	/* Radians/cycle, */
	SFMDirectionStruct	attitude_leveling_rate;	/* Radians/cycle. */
	SFMBoolean		air_brakes_state;
	double			air_brakes_area;	/* In square meters */
	SFMBoolean		can_crash_into_other;
	SFMBoolean		can_cause_crash;
	int			crash_contact_shape;	/* One of SFMCrashContactShape*. */
	double			crashable_size_radius;	/* Meters. */
	double			crashable_size_z_min;	/* Meters. */
	double			crashable_size_z_max;	/* Meters. */
	double			touch_down_crash_resistance;	/* Meters/cycle. */
	double			collision_crash_resistance;	/* Meters/cycle. */

} SFMModelStruct;




#endif	/* SFMMODEL_H */
