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
