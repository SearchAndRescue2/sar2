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

#ifndef OBJUTILS_H
#define OBJUTILS_H

#include "obj.h"

extern int SARObjIsAllocated(
	sar_object_struct **ptr, int total,
	int n
);
extern sar_object_struct *SARObjGetPtr(
	sar_object_struct **ptr, int total,
	int n
);
extern int SARGetObjectNumberFromPointer(
	sar_scene_struct *scene,
	sar_object_struct **ptr, int total,
	sar_object_struct *obj_ptr
);
extern sar_object_struct *SARObjMatchPointerByName(
	sar_scene_struct *scene,
	sar_object_struct **ptr, int total,
	const char *name, int *obj_num
);

extern int SARIsTextureAllocated(
	sar_scene_struct *scene,
	int n
);
extern v3d_texture_ref_struct *SARGetTextureRefByName(
	sar_scene_struct *scene, const char *name
);
extern int SARGetTextureRefNumberByName(
	sar_scene_struct *scene, const char *name
);

extern int SARObjLandingGearState(sar_object_struct *obj_ptr);

extern sar_obj_part_struct *SARObjGetPartPtr(
	sar_object_struct *obj_ptr, sar_obj_part_type type, int skip
);
extern sar_obj_rotor_struct *SARObjGetRotorPtr(
	sar_object_struct *obj_ptr, int n, int *total
);
extern sar_obj_hoist_struct *SARObjGetHoistPtr(
	sar_object_struct *obj_ptr, int n, int *total
);
extern int SARObjGetOnBoardPtr(
	sar_object_struct *obj_ptr,
	int **crew, int **passengers, int **passengers_max,
	float **passengers_mass,
	int **passengers_leave_pending, int **passengers_drop_pending
);
extern sar_external_fueltank_struct *SARObjGetFuelTankPtr(
	sar_object_struct *obj_ptr, int n, int *total
);

extern sar_visual_model_struct *SARVisualModelNew(
	sar_scene_struct *scene,
	const char *filename, const char *name
);
extern void *SARVisualModelNewList(sar_visual_model_struct *vmodel);
extern int SARVisualModelGetRefCount(sar_visual_model_struct *vmodel);
extern void SARVisualModelRef(sar_visual_model_struct *vmodel);
extern void SARVisualModelUnref(
	sar_scene_struct *scene, sar_visual_model_struct *vmodel
);
extern void SARVisualModelCallList(sar_visual_model_struct *vmodel);
extern void SARVisualModelDeleteAll(sar_scene_struct *scene);

extern sar_cloud_layer_struct *SARCloudLayerNew(
	sar_scene_struct *scene,
	int tile_width, int tile_height,
	float range,		/* Tiling range in meters */
	float altitude,		/* Altitude in meters */
	const char *tex_name
);
extern void SARCloudLayerDelete(
	sar_scene_struct *scene,
	sar_cloud_layer_struct *cloud_layer_ptr
);

extern sar_cloud_bb_struct *SARCloudBBNew(
	sar_scene_struct *scene,
	int tile_width, int tile_height,
	const sar_position_struct *pos,
	float width, float height,	/* In meters */
	const char *tex_name,
	time_t lightening_min_int,	/* In ms */
	time_t lightening_max_int	/* In ms */
);
extern void SARCloudBBDelete(sar_cloud_bb_struct *cloud_bb_ptr);

extern int SARObjAddToGroundList(
	sar_scene_struct *scene,
	sar_object_struct *obj_ptr
);
extern void SARObjRemoveFromGroundList(
	sar_scene_struct *scene,
	sar_object_struct *obj_ptr
);
extern int SARObjAddToHumanRescueList(
	sar_scene_struct *scene,
	sar_object_struct *obj_ptr
);
extern void SARObjRemoveFromHumanRescueList(
	sar_scene_struct *scene,
	sar_object_struct *obj_ptr
);

extern int SARObjAddContactBoundsSpherical(
	sar_object_struct *obj_ptr,
	sar_obj_flags_t crash_flags, int crash_type,
	float contact_radius
);
extern int SARObjAddContactBoundsCylendrical(
	sar_object_struct *obj_ptr,
	sar_obj_flags_t crash_flags, int crash_type,
	float contact_radius,
	float contact_h_min, float contact_h_max
);
extern int SARObjAddContactBoundsRectangular(
	sar_object_struct *obj_ptr,
	sar_obj_flags_t crash_flags, int crash_type,
	float contact_x_min, float contact_x_max,
	float contact_y_min, float contact_y_max,
	float contact_z_min, float contact_z_max
);

extern int SARObjInterceptNew(
	sar_scene_struct *scene,
	sar_intercept_struct ***ptr, int *total,
	sar_obj_flags_t flags,
	float x, float y, float z,
	float radius,
	float urgency,
	const char *name
);
extern sar_obj_part_struct *SARObjPartNew(
	sar_scene_struct *scene,
	sar_obj_part_struct ***ptr, int *total,
	sar_obj_part_type type
);
extern sar_obj_part_struct *SARObjAirBrakeNew(
	sar_scene_struct *scene,
	sar_obj_part_struct ***ptr, int *total
);
extern sar_obj_part_struct *SARObjDoorRescueNew(
	sar_scene_struct *scene,
	sar_obj_part_struct ***ptr, int *total
);
extern sar_obj_part_struct *SARObjLandingGearNew(
	sar_scene_struct *scene,
	sar_obj_part_struct ***ptr, int *total
);
extern int SARObjExternalFuelTankNew(
	sar_scene_struct *scene,
	sar_external_fueltank_struct ***ptr, int *total
);
extern int SARObjRotorNew(
	sar_scene_struct *scene,
	sar_obj_rotor_struct ***ptr, int *total
);
extern sar_light_struct *SARObjLightNew(
	sar_scene_struct *scene,
	sar_light_struct ***ptr, int *total
);
extern int SARObjNew(
	sar_scene_struct *scene,
	sar_object_struct ***ptr, int *total,
	int type
);

extern void SARObjDeleteIntercepts(
	sar_scene_struct *scene,
	sar_intercept_struct ***ptr, int *total
);
extern void SARObjDeleteLights(
	sar_scene_struct *scene,
	sar_light_struct ***ptr, int *total
);
extern void SARObjDeleteParts(
	sar_scene_struct *scene,
	sar_obj_part_struct ***ptr, int *total
);
extern void SARObjDeleteExternalFuelTanks(
	sar_scene_struct *scene,
	sar_external_fueltank_struct ***ptr, int *total
);
extern void SARObjDeleteRotors(
	sar_scene_struct *scene,
	sar_obj_rotor_struct ***ptr, int *total
);
extern void SARObjDelete(
	void *core_ptr,
	sar_object_struct ***ptr, int *total,
	int n
);

extern void SARObjGenerateTilePlane(
	float min, float max, /* In meters */
	float tile_width, float tile_height
);


#endif	/* OBJUTILS_H */
