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
