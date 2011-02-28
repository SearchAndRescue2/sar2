/*
                           SAR Reality Constants

	All units are in meters, radians, and cycles unless noted
	otherwise.
 */

#ifndef SARREALITY_H
#define SARREALITY_H


/*
 *	PI constant:
 */
#ifndef PI
# define PI	3.14159265
#endif

/*
 *	Cycle interval in microseconds and milliseconds:
 */
#define CYCLE_LAPSE_US	1000000
#define CYCLE_LAPSE_MS	(CYCLE_LAPSE_US / 1000)

/*
 *	Seconds to cycles and vice versa coefficients:
 */
#define SAR_SEC_TO_CYCLE_COEFF	(double)1.0
#define SAR_CYCLE_TO_SEC_COEFF	(double)1.0

/*
 *	Shadow visibility height (in meters):
 *
 *	Objects higher than this distance from ground do not have their
 *	shadows drawn.
 */
#define SAR_SHADOW_VISIBILITY_HEIGHT	100

/*
 *	Gravity (in meters per cycle^2):
 */
#define SAR_GRAVITY	(9.8 * SAR_SEC_TO_CYCLE_COEFF * SAR_SEC_TO_CYCLE_COEFF)

/*
 *	Maximum visibility (in meters), this value is used for generation
 *	of large quads. This value needs to be bigger than the drawing
 *	clip distance (visibility_max), which is at most 21 miles.
 */
#define SAR_MAX_VISIBILITY_DISTANCE	35000.0


/*
 *	Map view maximum altitude (in feet):
 */
#define SAR_MAX_MAP_VIEW_HEIGHT		10000000.0


/*
 *	Radius of primary light position (the sun) to camera
 *	position in meters.
 */
#define SAR_PRILIGHT_TO_CAMERA_RAD	100000.0


#endif	/* SARREALITY_H */
