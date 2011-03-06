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

#ifndef SARDRAWSELECT_H
#define SARDRAWSELECT_H

#include <GL/gl.h>
#include "sar.h"

/* Select processing used by SARDrawMap() */
extern void SARDrawMapProcessHits(
        sar_core_struct *core_ptr, GLint hits, GLuint buffer[]
);
extern int SARDrawMapObjNameListAppend(
        sar_drawmap_objname_struct ***ptr, int *total,
        GLuint gl_name, const char *obj_name
);
extern void SARDrawMapObjNameListDelete(
        sar_drawmap_objname_struct ***ptr, int *total
);

/* Ground contact check hit list */
extern int *SARGetGCCHitList(
        sar_core_struct *core_ptr, sar_scene_struct *scene,
        sar_object_struct ***ptr, int *total,
        int obj_num,
        int *hits
);
/* Ground contact check if over water */
extern int SARGetGHCOverWater(
        sar_core_struct *core_ptr, sar_scene_struct *scene,
        sar_object_struct ***ptr, int *total,
        int obj_num, Boolean *got_hit, Boolean *over_water
);

#endif	/* SARDRAWSELECT_H */
