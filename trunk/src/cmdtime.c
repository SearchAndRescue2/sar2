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
#include "sar.h"
#include "scenesound.h"
#include "sartime.h"


void SARCmdTime(SAR_CMD_PROTOTYPE);


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
 *	Time setting
 */
void SARCmdTime(SAR_CMD_PROTOTYPE)
{
	Boolean got_match = False;
	sar_core_struct *core_ptr = SAR_CORE(data);
	sar_scene_struct *scene = core_ptr->scene;
/*	sar_option_struct *opt = &core_ptr->option; */

	/* No argument? */
	if(*arg == '\0')
	{
	    NOTIFY(
		"Usage: time <h:m:s|day|dawn|dusk|night>"
	    );
	    return;
	}

	/* Parse argument */
	if(isdigit(*arg))
	{
	    /* Use h:m:s format */
	    if(scene != NULL)
	    {
		int h, m, s;
		SARParseTimeOfDay(arg, &h, &m, &s);
		scene->tod = (float)(
		    (h * 3600.0) + (m * 60.0) + s
		);
	    }
	    got_match = True;
	}
	else
	{
	    /* Use day|dawn|dusk|night format */
	    if(scene != NULL)
	    {
/* Convert hours into seconds */
#define HTOS(h) ((h) * 3600.0)

		if(!strcasecmp(arg, "day") ||
		   !strcasecmp(arg, "noon")
		)
		{
		    scene->tod = HTOS(12.0);	/* 12:00 */
		    got_match = True;
		}
		else if(!strcasecmp(arg, "dawn") ||
			!strcasecmp(arg, "morning")
		)
		{
		    scene->tod = HTOS(6.5);	/* 6:30 */
		    got_match = True;
		}
		else if(!strcasecmp(arg, "dusk") ||
			!strcasecmp(arg, "evening")
		)
		{
		    scene->tod = HTOS(17.5);     /* 17:30 */
		    got_match = True;
		}
		else if(!strcasecmp(arg, "night") ||
			!strcasecmp(arg, "midnight")
		)
		{
		    scene->tod = HTOS(0.0);     /* 0:00 */
		    got_match = True;
		}
#undef HTOS
	    }
	}

	if(got_match)
	{
	    char *s = (char *)malloc(
		(80 + STRLEN(arg)) * sizeof(char)
	    );
	    sprintf(
		s,
"Time set to \"%s\".",
		arg
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
}
