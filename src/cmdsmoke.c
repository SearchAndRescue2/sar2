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
#include "smoke.h"
#include "sar.h"
#include "scenesound.h"
#include "config.h"


void SARCmdSmoke(SAR_CMD_PROTOTYPE);


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
 *	Smoke creation
 */
void SARCmdSmoke(SAR_CMD_PROTOTYPE)
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
		"Usage: smoke <x> <y> <z> <type> <size>"
	    );
	    return;
	}

	/* Parse argument */
	strv = strexp(arg, &strc);
	if(strc == 5)
	{
	    const char *tex_name;
	    float size = (float)MAX(ATOF(strv[4]), 1.0);
	    sar_position_struct pos;

	    pos.x = ATOF(strv[0]);
	    pos.y = ATOF(strv[1]);
	    pos.z = ATOF(strv[2]);

	    /* Type */
	    switch(ATOI(strv[3]))
	    {
	      case 0:	/* Light */
		tex_name = SAR_STD_TEXNAME_SMOKE_LIGHT;
		break;
	      case 1:	/* Medium */
		tex_name = SAR_STD_TEXNAME_SMOKE_MEDIUM;
		break;
	      case 2:	/* Dark */
		tex_name = SAR_STD_TEXNAME_SMOKE_DARK;
		break;
	      case 3:	/* Orange */
		tex_name = SAR_STD_TEXNAME_SMOKE_ORANGE;
		break;
	      default:	/* Light */
		tex_name = SAR_STD_TEXNAME_SMOKE_LIGHT;
		break;
	    }

	    obj_num = SmokeCreate(
		scene,
		&core_ptr->object, &core_ptr->total_objects,
		SAR_SMOKE_TYPE_SMOKE,
		&pos,		/* Position */
		NULL,		/* Respawn offset */
		size / 4.0f,	/* Size start */
		size,		/* Size max */
		-1.0f,		/* Size increase rate */
		1,		/* Hide at max? */
		5,		/* Units */
		1000,		/* Respawn interval */
		tex_name,	/* Texture name */
		-1,		/* Reference object */
		0		/* Life span */
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
"Created smoke object #%i.",
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
