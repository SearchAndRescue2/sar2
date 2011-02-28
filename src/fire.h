/*
 *                            Fire Creation
 */

#ifndef FIRE_H
#define FIRE_H

#include "obj.h"
#include "sar.h"


extern int FireCreate(
	sar_core_struct *core_ptr, sar_scene_struct *scene,
	sar_object_struct ***ptr, int *total,
	const sar_position_struct *pos,
	float radius, float height,
	int ref_object,
	const char *tex_name, const char *ir_tex_name
);


#endif	/* FIRE_H */
