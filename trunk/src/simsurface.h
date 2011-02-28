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
