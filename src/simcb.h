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

#ifndef SIMCB_H
#define SIMCB_H

#include <sys/types.h>
#include "sfm.h"
#include "obj.h"
#include "sar.h"


extern void SARSimInitModelCB(
	void *realm_ptr, SFMModelStruct *model,
	void *client_data
);
extern void SARSimDestroyModelCB(
	void *realm_ptr, SFMModelStruct *model,
	void *client_data                     
);

extern void SARSimAirborneCB(
	void *realm_ptr, SFMModelStruct *model,
	void *client_data
);
extern void SARSimTouchDownCB(
	void *realm_ptr, SFMModelStruct *model,
	void *client_data, double impact_coeff
);
extern void SARSimOverspeedCB(
	void *realm_ptr, SFMModelStruct *model,
	void *client_data, double cur_speed,
	double overspeed_expected,
	double overspeed
);
extern void SARSimCollisionCB(
	void *realm_ptr, SFMModelStruct *model, SFMModelStruct *obstruction,
	void *client_data, double impact_coeff 
);
extern void SARSimObjectCollisionCB(
	void *client_data,              /* Core structure. */
	sar_object_struct *obj_ptr, sar_object_struct *obstruction_obj_ptr,
	double impact_coeff
);


#endif	/* SIMCB_H */
