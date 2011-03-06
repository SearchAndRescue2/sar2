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

#include <string.h>
#include <stdlib.h>
#include <sys/types.h>

#include "../include/string.h"
#include "../include/strexp.h"

#include "gw.h"
#include "messages.h"
#include "cmd.h"
#include "fire.h"
#include "sar.h"
#include "scenesound.h"
#include "config.h"


void SARCmdFire(SAR_CMD_PROTOTYPE);


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

#define STR_IS_YES(s)	(StringIsYes(s))

#define NOTIFY(s)			\
{ if(SAR_CMD_IS_VERBOSE(flags) &&	\
     (scene != NULL) && ((s) != NULL)	\
  ) { SARMessageAdd(scene, (s)); }	\
}


/*
 *	Fire creation
 */
void SARCmdFire(SAR_CMD_PROTOTYPE)
{
	char **strv;
	int strc, obj_num = -1;
	Boolean got_match = False;
	sar_core_struct *core_ptr = SAR_CORE(data);
	sar_scene_struct *scene = core_ptr->scene;
/*	sar_option_struct *opt = &core_ptr->option; */

	/* No argument? */
	if(*arg == '\0')
	{
	    NOTIFY(
		"Usage: fire <x> <y> <z> <radius> <height>"
	    );
	    return;
	}

	/* Parse argument */
	strv = strexp(arg, &strc);
	if(strc == 5)
	{
	    float radius = (float)MAX(ATOF(strv[3]), 1.0);
	    float height = (float)MAX(ATOF(strv[4]), 1.0);
	    sar_position_struct pos;

	    pos.x = ATOF(strv[0]);
	    pos.y = ATOF(strv[1]);
	    pos.z = ATOF(strv[2]);

	    obj_num = FireCreate(
		core_ptr, scene,
		&core_ptr->object, &core_ptr->total_objects,
		&pos,			/* Position */
		radius, height,		/* Size */
		-1,			/* Reference object */
		SAR_STD_TEXNAME_FIRE, SAR_STD_TEXNAME_FIRE_IR
	    );

	    got_match = True;
	}

	if(got_match)
	{
	    char *s = (char *)malloc(
		(80 + STRLEN(arg)) * sizeof(char)
	    );
	    sprintf(
		s,
"Created fire object #%i.",
		obj_num
	    );
	    NOTIFY(s);
	    free(s);
	}
	else
	{
	    char *s = (char *)malloc(
		(80 + STRLEN(arg)) * sizeof(char)
	    );
	    sprintf(
		s,
"%s: Invalid value.",
		arg
	    );
	    NOTIFY(s);
	    free(s);
	}

	strlistfree(strv, strc);
}
