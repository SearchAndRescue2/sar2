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
#include "sfm.h"
#include "obj.h"
#include "objutils.h"
#include "messages.h"
#include "simutils.h"
#include "cmd.h"
#include "sar.h"
#include "config.h"


void SARCmdSet(SAR_CMD_PROTOTYPE);


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

#define NOTIFY(s)                       \
{ if(SAR_CMD_IS_VERBOSE(flags) &&       \
     (scene != NULL) && ((s) != NULL)   \
  ) { SARMessageAdd(scene, (s)); }      \
}


/*
 *	Object setting.
 */
void SARCmdSet(SAR_CMD_PROTOTYPE)
{
	char **argv, **strv, *strptr;
	int argc, strc;
	char *parm, *val;

	int obj_num = -1;
	sar_object_struct *obj_ptr = NULL;

	Boolean got_match = False;
	sar_core_struct *core_ptr = SAR_CORE(data);
	sar_scene_struct *scene = core_ptr->scene;

	/* No argument? */
	if(*arg == '\0')
	{
	    NOTIFY(
		"Usage: set <object>=<parameter>:<value>"
	    );
	    return;
	}

	/* Parse arguments, format:
	 *
	 * <object_name>=<parameter>:<value>
	 */
	argv = strchrexp(arg, '=', &argc);
	if(argv == NULL)
	    return;

	/* First argument is always object name or number, so
	 * match it.
	 */
	strptr = ((argc > 0) ? argv[0] : NULL);
	if(strptr != NULL)
	{
	    while(ISBLANK(*strptr))
		strptr++;

	    /* First character the number sign? */
	    if(*strptr == '#')
	    {
		/* Match obj_ptr explicitly. */
		strptr++;
		obj_num = atoi(strptr);
		if(SARObjIsAllocated(
		    core_ptr->object, core_ptr->total_objects,
		    obj_num
		))
		{
		    obj_ptr = core_ptr->object[obj_num];
		}
		else
		{
		    obj_num = -1;
		    obj_ptr = NULL;
		}
	    }
	    else
	    {
		/* Match obj_ptr by name. */
		strstrip(strptr);
		obj_ptr = SARObjMatchPointerByName(
		    core_ptr->scene,
		    core_ptr->object, core_ptr->total_objects,
		    strptr, &obj_num
		);
	    }
	}

	/* No such object? */
	if((obj_ptr == NULL) && (argc > 0))
	{
	    char *buf = (char *)malloc(
		(80 + STRLEN(argv[0]) + STRLEN(SAR_MESG_NO_SUCH_OBJECT)) *
		sizeof(char)
	    );
	    sprintf(
		buf,
		"%s: %s.",
		argv[0],
		SAR_MESG_NO_SUCH_OBJECT
	    );
	    NOTIFY(buf);
	    free(buf);
	    strlistfree(argv, argc);
	    return;
	}


	/* Second argument is the "<parameter>:<value>". */
	if(argc > 1)
	    strv = strchrexp(argv[1], ':', &strc);
	else
	    strv = NULL;
	if((strv == NULL) || (strc <= 1))
	{
	    NOTIFY(
		"Usage: set <object>=<parameter>:<value>"
	    );
	    strlistfree(argv, argc);
	    strlistfree(strv, strc);
	    return;
	}

	/* Get links to parm and val in the strv array. */
	if(strc > 0)
	    parm = strv[0];
	else
	    parm = NULL;

	if(strc > 1)
	    val = strv[1];
	else
	    val = NULL;

	if((parm == NULL) || (val == NULL))
	{   
	    NOTIFY(
		"Usage: set <object>=<parameter>:<value>"
	    );
	    strlistfree(argv, argc);
	    strlistfree(strv, strc);
	    return;
	} 

	strstrip(parm);
	strstrip(val);


	/* Begin handling parameter. */

	/* Name */
	if(!strcasecmp(parm, "name"))
	{
	    free(obj_ptr->name);
	    if(*val != '\0')
		obj_ptr->name = STRDUP(val);
	    else
		obj_ptr->name = NULL;
	    got_match = True;
	}

	/* X (from meters) */
	else if(!strcasecmp(parm, "x"))
	{
	    if(strpfx(val, "++"))
	    {
		/* Relative move */
		float dx = ATOF(val + 2);
		obj_ptr->pos.x += dx;
	    }
	    else if(strpfx(val, "--"))
	    {
		/* Relative move */
		float dx = ATOF(val + 1);
		obj_ptr->pos.x += dx;
	    }
	    else
	    {
		/* Explicit move */
		float x = ATOF(val);
		obj_ptr->pos.x = x;
	    }
	    /* Realize new position */
	    SARSimWarpObject(
		scene, obj_ptr,
		&obj_ptr->pos, NULL
	    );
	    got_match = True;
	}
	/* Y (from meters) */
	else if(!strcasecmp(parm, "y"))
	{
	    if(strpfx(val, "++"))
	    {
		/* Relative move */
		float dy = ATOF(val + 2);
		obj_ptr->pos.y += dy;
	    }
	    else if(strpfx(val, "--"))
	    {
		/* Relative move */
		float dy = ATOF(val + 1);
		obj_ptr->pos.y += dy;
	    }
	    else
	    {
		/* Explicit move */
		float y = ATOF(val);
		obj_ptr->pos.y = y;
	    }
	    /* Realize new position */
	    SARSimWarpObject(
		scene, obj_ptr,
		&obj_ptr->pos, NULL
	    );
	    got_match = True;
	}
	/* Z (from meters) */
	else if(!strcasecmp(parm, "z"))
	{
	    if(strpfx(val, "++"))
	    {
		/* Relative move */
		float dz = ATOF(val + 2);
		obj_ptr->pos.z += dz;
	    }
	    else if(strpfx(val, "--"))
	    {
		/* Relative move */
		float dz = ATOF(val + 1);
		obj_ptr->pos.z += dz;
	    }
	    else
	    {
		/* Explicit move */
		float z = ATOF(val);
		obj_ptr->pos.z = z;
	    }
	    /* Realize new position */
	    SARSimWarpObject(
		scene, obj_ptr,
		&obj_ptr->pos, NULL
	    );
	    got_match = True;
	}
	/* Heading (from degrees) */
	else if(!strcasecmp(parm, "heading"))
	{
	    if(strpfx(val, "++"))
	    {
		obj_ptr->dir.heading = (float)SFMSanitizeRadians(
		    obj_ptr->dir.heading +
		    DEGTORAD(ATOF(val + 2))
		);
	    }
	    else if(strpfx(val, "--"))
	    {
		obj_ptr->dir.heading = (float)SFMSanitizeRadians(
		    obj_ptr->dir.heading +
		    DEGTORAD(ATOF(val + 1))
		);
	    }
	    else
	    {
		obj_ptr->dir.heading = (float)SFMSanitizeRadians(
		    DEGTORAD(ATOF(val))
		);
	    }
	    /* Realize new direction */
	    SARSimWarpObject(
		scene, obj_ptr,
		NULL, &obj_ptr->dir
	    );
	    got_match = True;
	}
	/* Pitch (from degrees) */
	else if(!strcasecmp(parm, "pitch"))
	{
	    if(strpfx(val, "++"))
	    {
		obj_ptr->dir.pitch = (float)SFMSanitizeRadians(
		    obj_ptr->dir.pitch +
		    DEGTORAD(ATOF(val + 2))
		);
	    }
	    else if(strpfx(val, "--"))
	    {
		obj_ptr->dir.pitch = (float)SFMSanitizeRadians(
		    obj_ptr->dir.pitch +
		    DEGTORAD(ATOF(val + 1))
		);
	    }
	    else
	    {
		obj_ptr->dir.pitch = (float)SFMSanitizeRadians(
		    DEGTORAD(ATOF(val))
		);
	    }
	    /* Realize new direction */
	    SARSimWarpObject(
		scene, obj_ptr,
		NULL, &obj_ptr->dir
	    );
	    got_match = True;
	}
	/* Bank (from degrees)? */
	else if(!strcasecmp(parm, "bank"))
	{
	    if(strpfx(val, "++"))
	    {
		obj_ptr->dir.bank = (float)SFMSanitizeRadians(
		    obj_ptr->dir.bank +
		    DEGTORAD(ATOF(val + 2))
		);
	    }
	    else if(strpfx(val, "--"))
	    {
		obj_ptr->dir.bank = (float)SFMSanitizeRadians(
		    obj_ptr->dir.bank +
		    DEGTORAD(ATOF(val + 1))
		);
	    }
	    else
	    {
		obj_ptr->dir.bank = (float)SFMSanitizeRadians(
		    DEGTORAD(ATOF(val))
		);
	    }
	    /* Realize new direction */
	    SARSimWarpObject(
		scene, obj_ptr,
		NULL, &obj_ptr->dir
	    );
	    got_match = True;
	}

	/* Range (from meters) */
	else if(!strcasecmp(parm, "range"))
	{
	    obj_ptr->range = (float)MAX(ATOF(val), 0.0);
	}
	/* Range far (from meters) */
	else if(!strcasecmp(parm, "range_far"))
	{
	    obj_ptr->range_far = (float)MAX(ATOF(val), 0.0);
	}

	/* Hit points (from meters) */
	else if(!strcasecmp(parm, "hit_points"))
	{
	    obj_ptr->hit_points = (float)CLIP(
		ATOF(val), 0.0, obj_ptr->hit_points_max
	    );
	}
	/* Hit points max (from meters) */
	else if(!strcasecmp(parm, "hit_points_max"))
	{
	    obj_ptr->hit_points_max = (float)MAX(ATOF(val), 0.0);
	}

	/* Air worthy state */
	else if(!strcasecmp(parm, "air_worthy_state"))
	{
	    sar_object_aircraft_struct *obj_aircraft_ptr =
		SAR_OBJ_GET_AIRCRAFT(obj_ptr);
	    if(obj_aircraft_ptr != NULL)
	    {
		sar_air_worthy_state air_worthy_state = -1;

		if(strcasestr(val, "NOT_FLYABLE") != NULL)
		    air_worthy_state = SAR_AIR_WORTHY_NOT_FLYABLE;
		else if(strcasestr(val, "OUT_OF_CONTROL") != NULL)
		    air_worthy_state = SAR_AIR_WORTHY_OUT_OF_CONTROL;
		else if(strcasestr(val, "FLYABLE") != NULL)
		    air_worthy_state = SAR_AIR_WORTHY_FLYABLE;

		if(air_worthy_state > -1)
		{
		    obj_aircraft_ptr->air_worthy_state = air_worthy_state;
		}
		else
		{
		    NOTIFY(
"<value> must be NOT_FLYABLE | OUT_OF_CONTROL | FLYABLE"
		    );
		}
		got_match = True;
	    }
	}

	/* Horizontal speed (Y axis) (meters per cycle) */
	else if(!strcasecmp(parm, "speed"))
	{
	    sar_object_aircraft_struct *obj_aircraft_ptr =
		SAR_OBJ_GET_AIRCRAFT(obj_ptr);
	    if(obj_aircraft_ptr != NULL)
	    {
		obj_aircraft_ptr->vel.y = (float)MAX(ATOF(val), 0.0);
		got_match = True;
	    }
	}
	/* Speed stall (from meters per cycle) */
	else if(!strcasecmp(parm, "speed_stall"))
	{
	    sar_object_aircraft_struct *obj_aircraft_ptr =
		SAR_OBJ_GET_AIRCRAFT(obj_ptr);
	    if(obj_aircraft_ptr != NULL)
	    {
		obj_aircraft_ptr->speed_stall = (float)MAX(ATOF(val), 0.0);
		got_match = True;
	    }
	}
	/* Stall coeff */
	else if(!strcasecmp(parm, "stall_coeff"))
	{
	    sar_object_aircraft_struct *obj_aircraft_ptr =
		SAR_OBJ_GET_AIRCRAFT(obj_ptr);
	    if(obj_aircraft_ptr != NULL)
	    {
		obj_aircraft_ptr->stall_coeff = ATOF(val);
		got_match = True;
	    }
	}
	/* Speed max (from meters per cycle) */
	else if(!strcasecmp(parm, "speed_max"))
	{
	    sar_object_aircraft_struct *obj_aircraft_ptr =
		SAR_OBJ_GET_AIRCRAFT(obj_ptr);
	    if(obj_aircraft_ptr != NULL)
	    {
		obj_aircraft_ptr->speed_max = (float)MAX(ATOF(val), 0.0);
		got_match = True;
	    }
	}
	/* Overspeed expected (from meters per cycle) */
	else if(!strcasecmp(parm, "overspeed_expected"))
	{
	    sar_object_aircraft_struct *obj_aircraft_ptr =
		SAR_OBJ_GET_AIRCRAFT(obj_ptr);
	    if(obj_aircraft_ptr != NULL)
	    {
		obj_aircraft_ptr->overspeed_expected = (float)MAX(ATOF(val), 0.0);
		got_match = True;
	    }
	}
	/* Overspeed (from meters per cycle) */
	else if(!strcasecmp(parm, "overspeed"))
	{
	    sar_object_aircraft_struct *obj_aircraft_ptr =
		SAR_OBJ_GET_AIRCRAFT(obj_ptr);
	    if(obj_aircraft_ptr != NULL)
	    {
		obj_aircraft_ptr->overspeed = (float)MAX(ATOF(val), 0.0);
		got_match = True;
	    }
	}
	/* Min drag (from meters per cycle) */
	else if(!strcasecmp(parm, "min_drag"))
	{
	    sar_object_aircraft_struct *obj_aircraft_ptr =
		SAR_OBJ_GET_AIRCRAFT(obj_ptr);
	    if(obj_aircraft_ptr != NULL)
	    {
		obj_aircraft_ptr->min_drag = (float)MAX(ATOF(val), 0.0);
		got_match = True;
	    }
	}
	/* Pitch leveling */
	else if(!strcasecmp(parm, "pitch_leveling"))
	{
	    sar_object_aircraft_struct *obj_aircraft_ptr =
		SAR_OBJ_GET_AIRCRAFT(obj_ptr);
	    if(obj_aircraft_ptr != NULL)
	    {
		obj_aircraft_ptr->pitch_leveling = (float)DEGTORAD(
		    ATOF(val)
		);
		got_match = True;
	    }
	}
	/* Bank leveling */
	else if(!strcasecmp(parm, "bank_leveling"))
	{
	    sar_object_aircraft_struct *obj_aircraft_ptr =
		SAR_OBJ_GET_AIRCRAFT(obj_ptr);
	    if(obj_aircraft_ptr != NULL)
	    {
		obj_aircraft_ptr->bank_leveling = (float)DEGTORAD(
		    ATOF(val)
		);
		got_match = True;
	    }
	}
	/* Ground pitch offset */
	else if(!strcasecmp(parm, "ground_pitch_offset"))
	{
	    sar_object_aircraft_struct *obj_aircraft_ptr =
		SAR_OBJ_GET_AIRCRAFT(obj_ptr);
	    if(obj_aircraft_ptr != NULL)
	    {
		obj_aircraft_ptr->ground_pitch_offset = (float)DEGTORAD(
		    ATOF(val)
		);
		got_match = True;
	    }
	}
	/* Belly height */
	else if(!strcasecmp(parm, "belly_height"))
	{
	    sar_object_aircraft_struct *obj_aircraft_ptr =
		SAR_OBJ_GET_AIRCRAFT(obj_ptr);
	    if(obj_aircraft_ptr != NULL)
	    {
		obj_aircraft_ptr->belly_height = (float)ATOF(val);
		got_match = True;
	    }
	}
	/* Length */
	else if(!strcasecmp(parm, "length"))
	{
	    sar_object_aircraft_struct *obj_aircraft_ptr =
		SAR_OBJ_GET_AIRCRAFT(obj_ptr);
	    if(obj_aircraft_ptr != NULL)
	    {
		obj_aircraft_ptr->length = (float)ATOF(val);
		got_match = True;
	    }
	}
	/* Wingspan */
	else if(!strcasecmp(parm, "wingspan"))
	{
	    sar_object_aircraft_struct *obj_aircraft_ptr =
		SAR_OBJ_GET_AIRCRAFT(obj_ptr);
	    if(obj_aircraft_ptr != NULL)
	    {
		obj_aircraft_ptr->wingspan = (float)ATOF(val);
		got_match = True;
	    }
	}
	/* Gear height */
	else if(!strcasecmp(parm, "gear_height"))
	{
	    sar_object_aircraft_struct *obj_aircraft_ptr =
		SAR_OBJ_GET_AIRCRAFT(obj_ptr);
	    if(obj_aircraft_ptr != NULL)
	    {
		obj_aircraft_ptr->gear_height = (float)ATOF(val);
		got_match = True;
	    }
	}

	/* Dry mass (from kg) */
	else if(!strcasecmp(parm, "dry_mass"))
	{
	    sar_object_aircraft_struct *obj_aircraft_ptr =
		SAR_OBJ_GET_AIRCRAFT(obj_ptr);
	    if(obj_aircraft_ptr != NULL)
	    {
		obj_aircraft_ptr->dry_mass = (float)MAX(
		    ATOF(val), 0.0
		);
		got_match = True;
	    }
	}
	/* Fuel rate (from kg per cycle) */
	else if(!strcasecmp(parm, "fuel_rate"))
	{
	    sar_object_aircraft_struct *obj_aircraft_ptr =
		SAR_OBJ_GET_AIRCRAFT(obj_ptr);
	    if(obj_aircraft_ptr != NULL)
	    {
		obj_aircraft_ptr->fuel_rate = (float)MAX(
		    ATOF(val), 0.0
		);
		got_match = True;
	    }
	}
	/* Fuel (from kg) */
	else if(!strcasecmp(parm, "fuel"))
	{
	    sar_object_aircraft_struct *obj_aircraft_ptr =
		SAR_OBJ_GET_AIRCRAFT(obj_ptr);
	    if(obj_aircraft_ptr != NULL)
	    {
		obj_aircraft_ptr->fuel = (float)CLIP(
		    ATOF(val), 0.0, obj_aircraft_ptr->fuel_max
		);
		got_match = True;
	    }
	}
	/* Fuel max (from kg) */
	else if(!strcasecmp(parm, "fuel_max"))
	{
	    sar_object_aircraft_struct *obj_aircraft_ptr =
		SAR_OBJ_GET_AIRCRAFT(obj_ptr);
	    if(obj_aircraft_ptr != NULL)
	    {
		obj_aircraft_ptr->fuel_max = (float)MAX(
		    ATOF(val), 0.0
		);
		got_match = True;
	    }
	}

	/* Service ceiling (in meters)? */
	else if(!strcasecmp(parm, "service_ceiling"))
	{
	    sar_object_aircraft_struct *obj_aircraft_ptr =
		SAR_OBJ_GET_AIRCRAFT(obj_ptr);
	    if(obj_aircraft_ptr != NULL)
	    {
		obj_aircraft_ptr->service_ceiling = 
		    (float)MAX(ATOF(val), 0.0f);
		got_match = True;
	    }
	}


	if(got_match)
	{
	    char *s;
	    if(obj_ptr->name != NULL)
	    {
		s = (char *)malloc(
 (80 + STRLEN(obj_ptr->name) + STRLEN(parm) + STRLEN(val)) * sizeof(char)
		);
		sprintf(
		    s,
"Set object \"%s\" parameter \"%s\" to \"%s\".",
		    obj_ptr->name, parm, val
		);
	    }
	    else
	    {
		s = (char *)malloc(
 (80 + 80 + STRLEN(parm) + STRLEN(val)) * sizeof(char)
		);
		sprintf(
		    s,
"Set object #%i parameter \"%s\" to \"%s\".",
		    obj_num, parm, val
		);
	    }
	    NOTIFY(s);
	    free(s);
	}
	else
	{
	    char *s;
	    if(obj_ptr->name != NULL)
	    {
		s = (char *)malloc(
 (80 + STRLEN(parm) + STRLEN(obj_ptr->name)) * sizeof(char)
		);
		sprintf(
		    s,
"%s: No such parameter on object \"%s\".",
		    parm, obj_ptr->name
		);
	    }
	    else
	    {
		s = (char *)malloc(
 (80 + STRLEN(parm) + 80) * sizeof(char)
		);
		sprintf(
		    s,
"%s: No such parameter on object #%i.",
		    parm, obj_num
		);
	    }
	    NOTIFY(s);
	    free(s);
	}


	/* Deallocate exploded argument strings. */
	strlistfree(argv, argc);

	/* Exploded parameter and value strings. */
	strlistfree(strv, strc);

	/* Links to parm and val strings are now invalid. */
	parm = NULL;
	val = NULL;
}
