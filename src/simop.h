/*
		SAR Simulation Operations and Procedures

	Most of these functions are procedures called by the front end
	functions in simmanage.c
 */

#ifndef SIMOP_H
#define SIMOP_H

#include "obj.h"
#include "sar.h"

extern float SARSimFindGround(
	sar_scene_struct *scene,
	sar_object_struct **ptr, int total,
	const sar_position_struct *pos  
);

extern int SARSimDoMortality(
	sar_core_struct *core_ptr, sar_object_struct *obj_ptr
);

extern int SARSimRestart(
	sar_core_struct *core_ptr, sar_scene_struct *scene,
	sar_object_struct ***ptr, int *total,
	int obj_num, sar_object_struct *obj_ptr
);

/* Stop smoke trail objects from respawning but do not delete them. */
#define SARSIM_DELETE_EFFECTS_SMOKE_STOP_RESPAWN	(1 << 2)
extern int SARSimDeleteEffects(
	sar_core_struct *core_ptr, sar_scene_struct *scene,
	sar_object_struct ***ptr, int *total,
	int obj_num,
	unsigned int filter_flags	/* Any of SARSIM_DELETE_EFFECTS_* */
);

extern void SARSimSetAircraftCrashed(
	sar_scene_struct *scene,
	sar_object_struct ***ptr, int *total,
	sar_object_struct *obj_ptr,
	sar_object_aircraft_struct *obj_aircraft_ptr
);

extern void SARSimSetSFMValues(
	sar_core_struct *core_ptr, sar_scene_struct *scene,
	sar_object_struct *obj_ptr
);
extern void SARSimGetSFMValues(
	sar_scene_struct *scene, sar_object_struct *obj_ptr
);

extern int SARSimApplyNaturalForce(
	sar_core_struct *core_ptr,
	sar_object_struct *obj_ptr
);
extern int SARSimApplyArtificialForce(
	sar_core_struct *core_ptr,
	sar_object_struct *obj_ptr
);

extern void SARSimApplyGCTL(
	sar_core_struct *core_ptr,
	sar_object_struct *obj_ptr
);

#endif	/* SIMOP_H */
