#ifndef SARMEMORY_H
#define SARMEMORY_H

#include "obj.h"
#include "sar.h"


/*
 *	Memory stats structure:
 */
typedef struct _sar_memory_stat_struct sar_memory_stat_struct;
struct _sar_memory_stat_struct {

	/* All memory units are in bytes unless noted otherwise */
	unsigned long	total,
			texture,
			vmodel,
			scene,
			object;

	int		ntextures,
			nvmodels,
			nobjects;

};

extern void SARMemoryStat(
	sar_core_struct *core_ptr,
	sar_scene_struct *scene,
	sar_object_struct **object, int total_objects,
	sar_memory_stat_struct *stat_buf
);


#endif	/* SARMEMORY_H */
