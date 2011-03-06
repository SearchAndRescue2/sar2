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

#ifndef OBJIO_H
#define OBJIO_H

#include "obj.h"
#include "sar.h"
#include "sarfio.h"

/* objio.c */
extern sar_visual_model_struct *SARObjLoadX3DDataVisualModel(
	sar_core_struct *core_ptr, sar_scene_struct *scene,
	char **x3d_data,
	const sar_scale_struct *scale
);
extern sar_visual_model_struct *SARObjLoadTextVisualModel(
	sar_core_struct *core_ptr, sar_scene_struct *scene,
	float font_width, float font_height,    /* In meters. */
	float character_spacing,                /* In meters. */
	const char *s,
	float *string_width                     /* In meters. */
);
extern int SARObjLoadTranslate(
	sar_core_struct *core_ptr, sar_scene_struct *scene,
	sar_object_struct *obj_ptr,
	sar_parm_translate_struct *p_translate
);
extern int SARObjLoadTranslateRandom(
	sar_core_struct *core_ptr, sar_scene_struct *scene,
	sar_object_struct *obj_ptr,
	sar_parm_translate_random_struct *p_translate_random
);
extern int SARObjLoadTexture(
	sar_core_struct *core_ptr, sar_scene_struct *scene,
	sar_parm_texture_load_struct *p_texture_load
);
extern int SARObjLoadHelipad(
	sar_core_struct *core_ptr, sar_scene_struct *scene,
	sar_parm_new_helipad_struct *p_new_helipad
);
extern int SARObjLoadRunway(
	sar_core_struct *core_ptr, sar_scene_struct *scene,
	sar_parm_new_runway_struct *p_new_runway
);
extern int SARObjLoadHuman(
	sar_core_struct *core_ptr, sar_scene_struct *scene,
	sar_parm_new_human_struct *p_new_human
);
extern int SARObjLoadFire(
	sar_core_struct *core_ptr, sar_scene_struct *scene,
	sar_parm_new_fire_struct *p_new_fire
);
extern int SARObjLoadSmoke(
	sar_core_struct *core_ptr, sar_scene_struct *scene,
	sar_parm_new_smoke_struct *p_new_smoke
);
extern int SARObjLoadLineHeightField(
	sar_core_struct *core_ptr, sar_scene_struct *scene,
	const char *line,
	sar_object_struct *obj_ptr
);
extern int SARObjLoadFromFile(
	sar_core_struct *core_struct, int obj_num, const char *filename
);

/* objiopremodeled.c */
extern int SARObjPremodeledNew(
	sar_core_struct *core_ptr, sar_scene_struct *scene,
	const char *type_name, int argc, char **argv
);

#endif	/* OBJIO_H */
