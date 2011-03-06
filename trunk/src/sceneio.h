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
