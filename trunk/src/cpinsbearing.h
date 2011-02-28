/*
		Control Panel Instrument - Bearing
 */

#ifndef CPINSBEARING_H
#define CPINSBEARING_H

#include <sys/types.h>
#include <GL/gl.h>
#include "v3dtex.h"
#include "cpvalues.h"
#include "cpins.h"

/*
 *	This instrument type code:
 */
#define CPINS_TYPE_BEARING	15

/*
 *      Bearing Instrument structure:
 */
typedef struct _CPInsBearing CPInsBearing;
struct _CPInsBearing {

	CPIns ins;

	/* Last megnetic bearing in radians. */
	float	bearing_mag_last;

	/* Current megnetic bearing change velocity in radians per
	 * second.
	 */
	float	bearing_mag_vel;

	/* Megnetic bearing change velocity decrease in radians per
	 * second (must be positive).
	 */
	float	bearing_mag_vel_dec;


	/* Current mechanical bearing offset in delta radians. */
	float	bearing_mechanical_offset;

};
#define CPINS_BEARING(p)	((CPInsBearing *)(p))
#define CPINS_IS_BEARING(p)	(CPINS_TYPE(p) == CPINS_TYPE_BEARING)


extern CPIns *CPInsBearingNew(void *cp);


#endif	/* CPINSBEARING_H */
