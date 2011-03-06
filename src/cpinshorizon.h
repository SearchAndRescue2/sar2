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
		Control Panel Instrument - Artificial Horizon
 */

#ifndef CPINSHORIZON_H
#define CPINSHORIZON_H

#include <sys/types.h>
#include <GL/gl.h>
#include "v3dtex.h"
#include "cpvalues.h"
#include "cpins.h"

/*
 *	This instrument type code:
 */
#define CPINS_TYPE_HORIZON	12

/*
 *      Artificial Horizon Instrument structure:
 */
typedef struct _CPInsHorizon CPInsHorizon;
struct _CPInsHorizon {

	CPIns ins;

	int	deg_visible;		/* Pitch degrees visible. */

	float	bank_notch_radius;	/* Coefficient from center (0.0)
					 * to edge (1.0).
					 */

};
#define CPINS_HORIZON(p)	((CPInsHorizon *)(p))
#define CPINS_IS_HORIZON(p)	(CPINS_TYPE(p) == CPINS_TYPE_HORIZON)


extern CPIns *CPInsHorizonNew(void *cp);


#endif	/* CPINSHORIZON_H */
