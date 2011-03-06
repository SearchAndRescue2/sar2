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
		Control Panel Instrument - Altimeter
 */

#ifndef CPINSALTIMETER_H
#define CPINSALTIMETER_H

#include <sys/types.h>
#include <GL/gl.h>
#include "v3dtex.h"
#include "cpvalues.h"
#include "cpins.h"

/*
 *	This instrument type code:
 */
#define CPINS_TYPE_ALTIMETER	13

/*
 *      Altimeter Instrument structure:
 */
typedef struct _CPInsAltimeter CPInsAltimeter;
struct _CPInsAltimeter {

	CPIns ins;

};
#define CPINS_ALTIMETER(p)	((CPInsAltimeter *)(p))
#define CPINS_IS_ALTIMETER(p)	(CPINS_TYPE(p) == CPINS_TYPE_ALTIMETER)


extern CPIns *CPInsAltimeterNew(void *cp);


#endif	/* CPINSALTIMETER_H */
