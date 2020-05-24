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
		    SAR Flight Model - Data Types
 */

#ifndef SFMTYPES_H
#define SFMTYPES_H

#include <sys/types.h>
#ifdef __MSW__
# include "../include/os.h"
#endif

/*
 *	Basic data types:
 */
#define SFMBoolean	u_int8_t
#ifdef __MSW__
# define SFMFlags	unsigned long
#else
# define SFMFlags	u_int64_t
#endif
#define SFMTime		time_t


/*
 *	Boolean states:
 */
#ifndef True
# define True	1
#endif
#ifndef False
# define False	0
#endif

/*
 *	PI constant:
 */
#ifndef PI
# define PI     3.14159265
#endif

/*
 *	Position structure (also the vector structure):
 */
typedef struct {

	double x, y, z;

} SFMPositionStruct;


/*
 *	Direction structure:
 */
typedef struct {

	double heading, pitch, bank;

	double i, j, k;		/* Unit vector of heading and pitch
				 * (no bank).
				 */

} SFMDirectionStruct;





#endif	/* SFMTYPES_H */
