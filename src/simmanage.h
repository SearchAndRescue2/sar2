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
