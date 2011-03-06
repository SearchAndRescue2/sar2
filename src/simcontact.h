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
		 SAR Simulation Object to Object Contact

	Checks and handles object to object contacts, but does not
	handle surface (ground) contacts. See simsurface.c for
	surface contact checking.
 */

#ifndef SIMCONTACT_H
#define SIMCONTACT_H

#include "obj.h"
#include "sar.h"

extern int SARSimCrashContactCheck(
	sar_core_struct *core_ptr,
	sar_object_struct *obj_ptr
);

extern int SARSimHoistDeploymentContactCheck(
	sar_core_struct *core_ptr,
	sar_object_struct *obj_ptr
);

#endif	/* SIMCONTACT_H */
