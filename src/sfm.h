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
			  SAR Flight Model

	Flight dynamics model employing a new `simple' dimensional
	recursive approach to flight dynamics.

 */

#ifndef SFM_H
#define SFM_H

#include "sfmtypes.h"
#include "sfmmodel.h"


/*
 *	Unit cycle constant, in milliseconds. This value determines
 *	what SFM considers a `unit cycle'.
 */
#define SFMCycleUnitsMS		1000
#define SFMCycleUnitsUS		(SFMCycleUnitsMS * 1000)


/*
 *	Default gravity in (meters per cycle^2).
 */
#define SFMDefaultGravity	9.80665

/*
 *	Core structure:
 */
typedef struct {


	/* Timings for current management */
	SFMTime		lapsed_time;		/* In milliseconds */
	double		time_compensation;	/* Coefficient */
	double		time_compression;	/* Coefficient */

	/* Simulation constants */
	double		gravity;		/* Gravity (in meters per cycle^2) */

	/* Wind params */
	SFMPositionStruct       wind_vector;	/* In meters per Cycle */
	unsigned long		wind_flags;
	/* Callbacks, typical inputs are; realm pointer,
	 * model pointer, client data
	 */

	/* Model just added to the realm */
	void		*init_model_cb_client_data;
	void		(*init_model_cb)(void *, SFMModelStruct *, void *);

	/* Model just destroyed, WARNING arg2 is always invalid! */
	void		*destroy_model_cb_client_data;
	void		(*destroy_model_cb)(void *, SFMModelStruct *, void *);

	/* Model has become airborne */
	void		*airborne_cb_client_data;
	void		(*airborne_cb)(void *, SFMModelStruct *, void *);

	/* Model has touched down, arg4 is impact tolorance coeff */
	void		*touch_down_cb_client_data;
	void		(*touch_down_cb)(void *, SFMModelStruct *, void *, double);

	/* Model has parked (it is landed and stopped) */
	void		*parked_cb_client_data;
	void		(*parked_cb)(SFMModelStruct *, void *);

	/* Model has exceeded its maximum expected speed. arg4 is the
	 * current speed and arg5 is maximum expected speed (all units in
	 * meters per cycle)
	 *
	 * This function is called whenever the model is above its max
	 * expected speed (so this can be called multiple times)
	 */
	void		*overspeed_cb_client_data;
	void		(*overspeed_cb)(
	    void *, SFMModelStruct *, void *,
	    double,		/* Current speed, in meters per cycle */
	    double,		/* Expected overspeed, in meters per cycle */
	    double		/* Actual overspeed, in meters per cycle */
	);

	/* Model has collided with another object, arg3 is the other
	 * that has been collided into, arg5 is the impact tolorance
	 * coeff.
	 */
	void		*collision_cb_client_data;
	void		(*collision_cb)(
	    void *, SFMModelStruct *, SFMModelStruct *,
	    void *, double
	);

	/* Flight dynamics model list */
	SFMModelStruct	**model;
	int		total_models;

} SFMRealmStruct;

#define SFM_REALM(p)	((SFMRealmStruct *)(p))


/* In sfm.c */
extern SFMRealmStruct *SFMInit(int argc, char **argv);
extern void SFMShutdown(SFMRealmStruct *realm);

extern void SFMSetTiming(SFMRealmStruct *realm, SFMTime lapsed_ms);
extern void SFMSetTimeCompression(SFMRealmStruct *realm, double compression);
extern void SFMUpdateRealm(SFMRealmStruct *realm, SFMTime lapsed_ms);


/* In sfmmath.c */
extern double SFMHypot2(double dx, double dy);
extern double SFMHypot3(double dx, double dy, double dz);

extern double SFMSanitizeRadians(double r);
extern double SFMSanitizeDegrees(double d);
extern double SFMRadiansToDegrees(double r);
extern double SFMDegreesToRadians(double d);

extern double SFMDeltaRadians(double a1, double a2);
extern void SFMOrthoRotate2D(double theta, double *i, double *j);
/*
extern int SFMRandom(void);
 */

extern double SFMMetersToFeet(double m);
extern double SFMFeetToMeters(double feet);
extern double SFMMetersToMiles(double m);
extern double SFMMilesToMeters(double miles);
extern double SFMMPHToMPC(double mph);
extern double SFMKTSToMPC(double kts);
extern double SFMMPHToKTS(double mph);
extern double SFMKTSToMPH(double kts);
extern double SFMMPCToMPH(double mpc);
extern double SFMMPCToFPS(double mpc);
extern double SFMMPCToKPH(double mpc);

extern double SFMLBSToKG(double lbs);
extern double SFMKGToLBS(double kg);
extern void SFMMToDMS(
	double m_x, double m_y, double m_r,
	double dms_x_offset, double dms_y_offset,
	float *dms_x, float *dms_y
);
extern char *SFMLongitudeToString(double dms_x);
extern char *SFMLatitudeToString(double dms_y);

extern double SFMStallCoeff(
    double current_speed, double stall_speed, double speed_max
);
extern double SFMCurrentSpeedForStall(
    double vel_y, double vel_z, double pitch);



/* In sfmmodel.c */
extern int SFMModelInRealm(SFMRealmStruct *realm, SFMModelStruct *model);
extern SFMModelStruct *SFMModelAllocate(void);
extern int SFMModelAdd(SFMRealmStruct *realm, SFMModelStruct *model);
extern void SFMModelDelete(SFMRealmStruct *realm, SFMModelStruct *model);
extern SFMBoolean SFMModelChangeValues(
	SFMRealmStruct *realm, SFMModelStruct *model,
	SFMModelStruct *value
);
extern void SFMModelUndefineValue(
	SFMRealmStruct *realm, SFMModelStruct *model, SFMFlags flags
);


/* In sfmsimforce.c */
extern int SFMForceApplyNatural(
	SFMRealmStruct *realm, SFMModelStruct *model
);
extern int SFMForceApplyArtificial(
	SFMRealmStruct *realm, SFMModelStruct *model
);
extern int SFMForceApplyControl(
	SFMRealmStruct *realm, SFMModelStruct *model
);
extern void SFMSetAirspeed(
	SFMRealmStruct *realm, SFMModelStruct *model
);

#endif	/* SFM_H */
