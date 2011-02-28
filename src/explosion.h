/*
 *                      Explosion & Splash Creation
 */

#ifndef EXPLOSION_H
#define EXPLOSION_H

#include "obj.h"
#include "sar.h"


extern int ExplosionCreate(
	sar_core_struct *core_ptr, sar_scene_struct *scene,
	sar_object_struct ***ptr, int *total,
	const sar_position_struct *pos,
	float radius,
	int ref_object,
	const char *tex_name, const char *ir_tex_name
);

extern int SplashCreate(
	sar_core_struct *core_ptr, sar_scene_struct *scene,
	sar_object_struct ***ptr, int *total,
	const sar_position_struct *pos,
	float radius,
	int ref_object,
	const char *tex_name, const char *ir_tex_name
);


#endif	/* EXPLOSION_H */
