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

#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include "../include/string.h"

#include "gw.h"
#include "obj.h"
#include "objutils.h"
#include "messages.h"
#include "simop.h"
#include "cmd.h"
#include "sar.h"
#include "config.h"


void SARCmdClean(SAR_CMD_PROTOTYPE);


#define ATOI(s)         (((s) != NULL) ? atoi(s) : 0)
#define ATOL(s)         (((s) != NULL) ? atol(s) : 0)
#define ATOF(s)         (((s) != NULL) ? (float)atof(s) : 0.0f)
#define STRDUP(s)       (((s) != NULL) ? strdup(s) : NULL)

#define MAX(a,b)        (((a) > (b)) ? (a) : (b))
#define MIN(a,b)        (((a) < (b)) ? (a) : (b))
#define CLIP(a,l,h)     (MIN(MAX((a),(l)),(h)))
#define STRLEN(s)       (((s) != NULL) ? ((int)strlen(s)) : 0)

#define RADTODEG(r)     ((r) * 180.0 / PI)
#define DEGTORAD(d)     ((d) * PI / 180.0)


/*
 *	Scene "cleaning" (deletion of effects objects).
 */
void SARCmdClean(SAR_CMD_PROTOTYPE)
{
	int objects_deleted = 0;
	sar_core_struct *core_ptr = SAR_CORE(data);
	sar_scene_struct *scene = core_ptr->scene;
	if(scene == NULL)
	    return;

	/* Delete all effects objects? */
	if(!strcasecmp(arg, "all"))
	{
	    objects_deleted = SARSimDeleteEffects(
		core_ptr, scene,
		&core_ptr->object, &core_ptr->total_objects,
		-1,
		0
	    );
	}
	else
	{
	    /* Delete effects objects relative to the player object */
	    objects_deleted = SARSimDeleteEffects(
		core_ptr, scene,
		&core_ptr->object, &core_ptr->total_objects,
		scene->player_obj_num,
		0
	    );
	}

	if(SAR_CMD_IS_VERBOSE(flags))
	{
	    char *s = (char *)malloc(
		(80 + 80) * sizeof(char)
	    );
	    sprintf(
		s,
		"%i object%s deleted.",
		objects_deleted,
		(objects_deleted != 1) ? "s" : ""
	    );
	    SARMessageAdd(scene, s);
	    free(s);
	}
}
