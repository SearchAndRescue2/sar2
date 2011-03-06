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
	 SAR Simulation HeightField and Support Surface Checking
 */

#ifndef SIMSURFACE_H
#define SIMSURFACE_H

#include "obj.h"

extern float SARSimSupportSurfaceHeight(
	sar_object_struct *obj_ptr,
	const sar_contact_bounds_struct *cb,
	const sar_position_struct *pos,
	float z_tolor
);
extern float SARSimHFGetGroundHeight(
	sar_object_struct *obj_ptr,
	const sar_position_struct *pos
);

#endif	/* SIMSURFACE_H */
