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
		       SAR Simulation Management

	Primary front end procedures for SAR simulation updates on
	its SAR objects.
 */

#ifndef SIMMANAGE_H
#define SIMMANAGE_H

#include <sys/types.h>
#include "sfm.h"
#include "obj.h"
#include "sar.h"

extern int SARSimUpdateScene(
	sar_core_struct *core_ptr,
	sar_scene_struct *scene 
);
extern int SARSimUpdateSceneObjects(
	sar_core_struct *core_ptr,
	sar_scene_struct *scene
);


#endif	/* SIMMANAGE_H */
