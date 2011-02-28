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
