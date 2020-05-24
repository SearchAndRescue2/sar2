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
#include "sfm.h"

double SFMHypot2(double dx, double dy);
double SFMHypot3(double dx, double dy, double dz);

double SFMSanitizeRadians(double r);
double SFMSanitizeDegrees(double d);
double SFMRadiansToDegrees(double r);
double SFMDegreesToRadians(double d);

double SFMDeltaRadians(double a1, double a2);
void SFMOrthoRotate2D(double theta, double *i, double *j);

double SFMMetersToFeet(double m);
double SFMFeetToMeters(double feet);
double SFMMetersToMiles(double m);
double SFMMilesToMeters(double miles);
double SFMMPHToMPC(double mph);
double SFMKTSToMPC(double kts);
double SFMMPHToKTS(double mph);
double SFMKTSToMPH(double kts);
double SFMMPCToMPH(double mpc);
double SFMMPCToFPS(double mpc);
double SFMMPCToKPH(double mpc);

double SFMLBSToKG(double lbs);
double SFMKGToLBS(double kg);

void SFMMToDMS(
	double m_x, double m_y, double m_r,
	double dms_x_offset, double dms_y_offset,
	float *dms_x, float *dms_y
);
char *SFMLongitudeToString(double dms_x);
char *SFMLatitudeToString(double dms_y);

double SFMStallCoeff(
    double current_speed, double stall_speed, double speed_max
);
double SFMCurrentSpeedForStall(
    double vel_y, double vel_z, double pitch);

#define POW(x,y)        (((x) > 0.0f) ? pow(x,y) : 0.0f)
#define SQRT(x)		(((x) > 0.0f) ? sqrt(x) : 0.0f)

#define MAX(a,b)        (((a) > (b)) ? (a) : (b))
#define MIN(a,b)        (((a) < (b)) ? (a) : (b))
#define CLIP(a,l,h)     (MIN(MAX((a),(l)),(h)))
#define ABS(x)		(((x) < 0.0) ? ((x) * -1.0) : (x))

#define RADTODEG(r)     ((r) * 180 / PI)
#define DEGTORAD(d)     ((d) * PI / 180)


/*
 *	Returns the 2d hypot of the given delta distances.
 */
double SFMHypot2(double dx, double dy)
{
	return(SQRT((dx * dx) + (dy * dy)));
}

/*
 *      Returns the 3d hypot of the given delta distances.
 */
double SFMHypot3(double dx, double dy, double dz)
{
	return(SQRT((dx * dx) + (dy * dy) + (dz * dz)));
}


/*
 *      Sanitize radian value:
 */
double SFMSanitizeRadians(double r)
{
	while(r < 0)
	    r += (2 * PI);
	while(r >= (2 * PI))
	    r -= (2 * PI);  
	return(r);
}

/*
 *      Sanitize degree value:
 */
double SFMSanitizeDegrees(double d)
{
	while(d < 0.0)
	    d += 360.0;
	while(d >= 360.0)
	    d -= 360.0;
	return(d);
}

/* 
 *      Convert radians to degrees and sanitize:
 */
double SFMRadiansToDegrees(double r)
{
       return(SFMSanitizeDegrees(r * (180 / PI)));
}

/*
 *      Convert degrees to radians and sanitize:
 */
double SFMDegreesToRadians(double d)
{
	return(SFMSanitizeRadians(d * (PI / 180)));
}


/*
 *      Returns the delta angle in radians of a2 - a1.
 */
double SFMDeltaRadians(double a1, double a2)
{
	double theta = SFMSanitizeRadians(a2) - SFMSanitizeRadians(a1);

	if(theta < -PI)
	    return((2 * PI) - a1 + a2);
	else if(theta > PI)
	    return(-((2 * PI) - a2 + a1));
	else
	    return(theta);
}

/*
 *      Rotates i and j about the crossed perpendicular axis (orthogonal)
 *      to the plane with i and j in theta. Where theta is in radians and
 *      in bearing.
 */
void SFMOrthoRotate2D(double theta, double *i, double *j)
{
	double sin_theta = sin(theta);
	double cos_theta = cos(theta);
	double i0, j0;

	if((i == NULL) || (j == NULL))
	    return;

	i0 = *i;
	j0 = *j;

	*i = (i0 * cos_theta) + (j0 * sin_theta);
	*j = (j0 * cos_theta) - (i0 * sin_theta);
} 

/*
 *      Convert meters to feet:
 */
double SFMMetersToFeet(double m)
{
	return(m * 3.280833);
}
/*
 *      Convert feet to meters:
 */
double SFMFeetToMeters(double feet)
{
	return(feet / 3.280833);
}

/*
 *      Convert meters to miles.
 */
double SFMMetersToMiles(double m)
{
	return(m * 0.00062137);
}
/*
 *      Convert miles to meters.
 */
double SFMMilesToMeters(double miles)
{
	return(miles / 0.00062137);
}

/*
 *      Convert miles (US Statute) per hour to meters per cycle:
 */
double SFMMPHToMPC(double mph)
{  
	return(
	    mph * (1609.347 / 3600.0 * (double)SFMCycleUnitsUS / 1000000.0)
	);
}

/*
 *      Convert nautical miles per hour to meters per cycle:
 */
double SFMKTSToMPC(double kts)
{
    return SFMMPHToMPC(SFMKTSToMPH(kts));
}
/*
 *	Convert miles (US Statute) per hour to nautical miles per hour:
 */
double SFMMPHToKTS(double mph)
{
	return(mph / 1.15151515);
}

/*
 *	Convert nautical miles per hour to miles (US Statute) per hour:
 */
double SFMKTSToMPH(double kts)
{
	return(kts * 1.15151515);
}

/*
 *      Convert meters per cycle to miles (US Statute) per hour:
 */
double SFMMPCToMPH(double mpc)
{
	return(
	    mpc / (1609.34738 / 3600.0 * (double)SFMCycleUnitsUS / 1000000.0)
	);
}

/*      
 *      Convert meters per cycle to feet per second:
 */
double SFMMPCToFPS(double mpc)
{
	return(
	    mpc * (3.280833 * (double)SFMCycleUnitsUS / 1000000.0)
	);
}

/*
 *	Convert meters per cycle to kilometers per hour:
 */
double SFMMPCToKPH(double mpc)
{
	return(
	    mpc * (3.6 * (double)SFMCycleUnitsUS / 1000000.0)
	);
}

/*
 *      Convert pounds (avoirdupois) to kg:
 */
double SFMLBSToKG(double lbs)
{
	return(lbs * 0.453592);
}        
/*
 *      Convert kg to pounds:
 */
double SFMKGToLBS(double kg)
{
	return(kg / 0.453592);
}


/*
 *      Converts meters to DMS in units of degrees.
 *
 *      Range for dms_x is unsanitized;
 *      Range for dms_y is -90.0 to 90.0.
 *
 *      m_r must be positive and non-zero.
 */
void SFMMToDMS(
	double m_x, double m_y, double m_r,
	double dms_x_offset, double dms_y_offset,
	float *dms_x, float *dms_y
)
{
	float dms_y_radians, mz_r;


	if((dms_x == NULL) || (dms_y == NULL) || (m_r <= 0))
	    return;


	/* Calculate y_dms first */
	*dms_y = (float)((m_y / m_r * (180 / PI)) + dms_y_offset);

	/* Sanitize dms_y */
	if(*dms_y > 90.0f)
	    *dms_y = 180.0f - (*dms_y);
	if(*dms_y < -90.0f)
	    *dms_y = 180.0f + (*dms_y);

	dms_y_radians = (float)((*dms_y) * (PI / 180));


	/* Calculate x_dms */
	mz_r = (float)(m_r * cos(dms_y_radians));
	if(mz_r > 0.0f)
	    *dms_x = (float)((m_x / mz_r * (180 / PI)) + dms_x_offset);
	else
	    *dms_x = 0.0f;
}

/*
 *      Returns a statically allocated string containing
 *      a formatted longitude string.
 *
 *      This function never returns NULL.
 */
char *SFMLongitudeToString(double dms_x)
{
	double m, s;
	static char str[256];


	/* Sanitize */
	while(dms_x < -140.0)
	    dms_x += (2 * 140.0);
	while(dms_x > 140.0)
	    dms_x -= (2 * 140.0);

	/* West or east? */
	if(dms_x < 0.0)
	{
	    /* West */
	    dms_x = fabs(dms_x);
	    m = (dms_x - floor(dms_x)) * 60.0;
	    s = floor((m - floor(m)) * 60.0);
	    m = floor(m);
	    sprintf(
		str,
		"%.0f'%s%.0f:%s%.0fW",
		floor(dms_x),
		(m < 10.0f) ? "0" : "", m,
		(s < 10.0f) ? "0" : "", s
	    );
	}
	else
	{
	    /* East */
	    m = (dms_x - floor(dms_x)) * 60.0;
	    s = floor((m - floor(m)) * 60.0);
	    m = floor(m);
	    sprintf(
		str,
		"%.0f'%s%.0f:%s%.0fE",
		floor(dms_x),
		(m < 10.0f) ? "0" : "", m,
		(s < 10.0f) ? "0" : "", s
	    );
	}

	return(str);
}

/*
 *      Returns a statically allocated string containing
 *      a formatted latitude string.
 *
 *      This function never returns NULL.
 */
char *SFMLatitudeToString(double dms_y)
{
	double m, s;
	static char str[256];

#if 0
	/* Sanitize */
/* Should already be sanitized */
	while(dms_y < -90.0)
	    dms_y += 90.0;
	while(dms_y > 90.0)
	    dms_y -= 90.0;
#endif
	/* Above or below the equator? */
	if(dms_y < 0.0)
	{
	    /* South */
	    dms_y = fabs(dms_y);
	    m = (dms_y - floor(dms_y)) * 60.0;
	    s = floor((m - floor(m)) * 60.0);
	    m = floor(m);
	    sprintf(
		str,
		"%.0f'%s%.0f:%s%.0fS",
		floor(dms_y),
		(m < 10.0f) ? "0" : "", m,
		(s < 10.0f) ? "0" : "", s
	    );
	}
	else
	{
	    /* North */
	    m = (dms_y - floor(dms_y)) * 60.0;
	    s = floor((m - floor(m)) * 60.0);
	    m = floor(m);
	    sprintf(
		str,
		"%.0f'%s%.0f:%s%.0fN",
		floor(dms_y),
		(m < 10.0f) ? "0" : "", m,
		(s < 10.0f) ? "0" : "", s
	    );
	}

	return(str);
}       

/*
 *      Calculates a stall coefficient based on the given stall
 *      speed (all units in meters per cycle), current speed, and
 *      maximum speed. Return is in the range of 0 to 1, where 1
 *      is the highest stall coefficient (when current speed is 0).
 */
double SFMStallCoeff(
    double current_speed, double stall_speed, double speed_max
)
{
    //printf("SFMstall: cur: %.3f, stall: %.3f\n", current_speed, stall_speed);
    if(current_speed > stall_speed)
	return(0.0);
    else if(stall_speed <= 0.0)
	return(0.0);
    else if(current_speed <= 0.0)
	return(1.0);
    else
	return(1.0 - (current_speed / stall_speed));
}

/*
 * SFMCurrentSpeedForStall returns the actual speed that should be used to
 * calculate the stall coefficient given the current Y and Z speeds and pitch.
 * This ensures that falling fast while level on the horizon does not count as
 * when calculating the speed that gives the stall coefficient.
 */

double SFMCurrentSpeedForStall(
    double vel_y, double vel_z, double pitch)
{
    double sin_pitch = sin(pitch);
    double meaningful_z = 0.0;

    // Does z matter to knowing if we are stalling?
    // Yes when pitched up heavily and positive
    if(sin_pitch < -0.2 && vel_z > 0)
	meaningful_z += vel_z;

    // Or when pitched down heavily and negative
    else if(sin_pitch > -0.2 && vel_z < 0)
	meaningful_z += vel_z;

    // Otherwise falling down with z speed should not get us out of stall.
    // This does not mean much in practice anyways.

    return SFMHypot2(vel_y, meaningful_z);
}
