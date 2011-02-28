#ifndef SCENEIO_H
#define SCENEIO_H

#include "obj.h"
#include "sar.h"


extern void SARSceneDestroy(
	sar_core_struct *core_ptr,
	sar_scene_struct *scene,
	sar_object_struct ***ptr, int *total
);
extern void SARSceneLoadLocationsToList(
	sar_core_struct *core_ptr, sar_menu_struct *m,
	sar_menu_list_struct *list, int list_num,
	const char *filename
);
extern int SARSceneAddPlayerObject(
	sar_core_struct *core_ptr, sar_scene_struct *scene,
	const char *model_file,
	sar_position_struct *pos, sar_direction_struct *dir
);
extern int SARSceneLoadFromFile(
	sar_core_struct *core_ptr,
	sar_scene_struct *scene,
	const char *filename,
	const char *weather_preset_name,
	void *client_data,
	int (*progress_func)(void *, long, long)
);


#endif	/* SCENEIO_H */
