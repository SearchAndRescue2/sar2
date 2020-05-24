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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "../include/cfgfmt.h"
#include "../include/string.h"
#include "../include/strexp.h"
#include "../include/fio.h"
#include "../include/disk.h"

#include "obj.h"
#include "mission.h"
#include "sartime.h"
#include "sarfio.h"
#include "config.h"
#include "sfm.h"

static const char *NEXT_ARG(const char *s);
static void CHOP_STR_BLANK(char *s);

static char *SARParmLoadFromFileGetParmString(FILE *fp);
static int SARParmLoadFromFileIterate(
        const char *filename,
        void ***parm, int *total_parms,
        const char *parm_str, const char *val_str,
        int *lines_read, int filter_parm_type
);
static int SARParmLoadFromFileScene(
        const char *filename, FILE *fp,
        void ***parm, int *total_parms,
        int filter_parm_type,
        void *client_data,
        int (*progress_func)(void *, long, long),
        long file_size, int *lines_read
);
static int SARParmLoadFromFileMissionLog(
        const char *filename, FILE *fp,
        void ***parm, int *total_parms,
        int filter_parm_type,
        void *client_data,
        int (*progress_func)(void *, long, long),
        long file_size, int *lines_read
);
int SARParmLoadFromFile(
        const char *filename, int file_format,
        void ***parm, int *total_parms,
	int filter_parm_type,
        void *client_data,
        int (*progress_func)(void *, long, long)
);


#ifndef SAR_COMMENT_CHAR
# define SAR_COMMENT_CHAR	'#'
#endif

#define ATOI(s)		(((s) != NULL) ? atoi(s) : 0)
#define ATOL(s)		(((s) != NULL) ? atol(s) : 0)
#define ATOF(s)		(((s) != NULL) ? (float)atof(s) : 0.0f)
#define STRDUP(s)	(((s) != NULL) ? strdup(s) : NULL)

#define MAX(a,b)        (((a) > (b)) ? (a) : (b))
#define MIN(a,b)        (((a) < (b)) ? (a) : (b))
#define CLIP(a,l,h)     (MIN(MAX((a),(l)),(h)))
   
#define ISCOMMENT(c)    ((c) == SAR_COMMENT_CHAR)
#define ISCR(c)         (((c) == '\n') || ((c) == '\r'))

#define DEGTORAD(d)     ((d) * PI / 180)
#define RADTODEG(r)     ((r) * 180 / PI)


/*
 *	Seeks s to next argument.
 *
 *	If s is "red green blue" then return will be "green blue".
 */
static const char *NEXT_ARG(const char *s)
{
	if(s == NULL)
	    return(s);

	/* Seek past current arg till next blank or end of string */
	while(!ISBLANK(*s) && (*s != '\0'))
	    s++;

	/* Seek past spaces to next arg */
	while(ISBLANK(*s))
	    s++;

	return(s);
}

/*
 *	Terminates the string on the first blank character encountered.
 */
static void CHOP_STR_BLANK(char *s)
{
	if(s == NULL)
	    return;

	while(*s != '\0')
	{
	    if(ISBLANK(*s))
	    {
		*s = '\0';
		break;
	    }
	    else
	    {
		s++;
	    }
	}
}


/*
 *      Return a statically allocated string indicating the
 *      operation fetched from the pointed to fp.  Return can be
 *      an empty string if there was no op string to be found or
 *      error.
 *
 *      fp is positioned at the end of the op string which the next
 *      fetch can be used to get its argument. If a new line character
 *      is detected at the end of string on file, then the fp will be
 *      repositioned at that new line character. The next reading of fp
 *      will read that new line character.
 */
static char *SARParmLoadFromFileGetParmString(FILE *fp)
{
        int c, i;
#define len	80
        static char rtn_str[len];

        *rtn_str = '\0';

        if(fp == NULL)
            return(rtn_str);

        /* Seek past spaces */
        FSeekPastSpaces(fp);

        /* Iterate through file in order to get next parameter */
        for(i = 0; i < len; i++)
        {
            c = fgetc(fp);

	    /* End of parameter string or end of file? */
            if(ISBLANK(c) || (c == EOF))
            {
                rtn_str[i] = '\0';
                break;
            } 
            /* Escape sequence? */
            else if(c == '\\')
            {
                c = fgetc(fp);
                if(c == EOF)
                {
                    rtn_str[i] = '\0';
                    break;
                }
        
                if(c != '\\')
                    c = fgetc(fp);

                if(c == EOF)
                {
                    rtn_str[i] = '\0';
                    break;
                }
            }   
            /* Newline? */
            else if(ISCR(c))
            {
                /* Newline right at the end of the op string, seek
                 * back one character so subsequent read gets this
		 * newline character.
                 */
                fseek(fp, -1, SEEK_CUR);
 
                rtn_str[i] = '\0';
                break;          
            }

            rtn_str[i] = (char)c;
        }

#undef len
        return(rtn_str);
}

/*
 *	Loads the value in parm_str and val_str into the given parm
 *	array.
 *
 *	The number of lines_read will be incremented.
 *
 *	If filter_parm_type is not -1 then only parms matching the
 * 	specified type will be added to the given parm list.
 *
 *	Returns the number of parameters loaded during this call.
 */
static int SARParmLoadFromFileIterate(
	const char *filename,
	void ***parm, int *total_parms,
	const char *parm_str, const char *val_str,
	int *lines_read, int filter_parm_type
)
{
	int parms_loaded = 0;
	int i, parm_num;
	const char *cstrptr;
	void *p;


	if((parm_str == NULL) || (val_str == NULL))
	    return(parms_loaded);

	/* Increment *lines_read if a newline is found by iterating
	 * through val_str.  Note that we count this call as at least
	 * adding one new line.
	 */
	i = *lines_read + 1;
	for(cstrptr = val_str; *cstrptr != '\0'; cstrptr++)
	{
	    if(ISCR(*cstrptr))
		i++;
	}
	*lines_read = i;

	/* Seek val_str past initial spaces */
	while(ISBLANK(*val_str))
	    val_str++;

/* Adds parm p to the given parm list */
#define DO_ADD_PARM					\
{							\
 parm_num = *total_parms;				\
 *total_parms = parm_num + 1;				\
 *parm = (void **)realloc(				\
  *parm, (*total_parms) * sizeof(void *)		\
 );							\
 if(*parm == NULL)					\
 {							\
  *total_parms = 0;					\
  return(parms_loaded);					\
 }							\
 else							\
 {							\
  (*parm)[parm_num] = p;				\
  parms_loaded++;					\
 }							\
}

/* Checks if filter parm type matches with the given parm type.
 * If the parm matches or filter_parm_type is -1 then true is returned.
 */
#define FILTER_CHECK(t)		(			\
 (filter_parm_type == (t)) || (filter_parm_type < 0)	\
)


	/* Begin matching the given parameter name string */
	/* Version (of file format) */
	if(!strcasecmp(parm_str, "version") &&
           FILTER_CHECK(SAR_PARM_VERSION)
        )
	{
	    p = SARParmNew( SAR_PARM_VERSION );
            sar_parm_version_struct *pv = (sar_parm_version_struct *)p;

            cstrptr = val_str;
            pv->major = ATOI(cstrptr);
            cstrptr = NEXT_ARG(cstrptr);
            pv->minor = ATOI(cstrptr);
            cstrptr = NEXT_ARG(cstrptr);
            pv->release = ATOI(cstrptr);

            cstrptr = NEXT_ARG(cstrptr);
            free(pv->copyright);
            pv->copyright = STRDUP(cstrptr);
            /* Do not chop name */
/*          CHOP_STR_BLANK(pv->name); */

            DO_ADD_PARM
	}
	/* Name (of file) */
	else if(!strcasecmp(parm_str, "name") &&
                FILTER_CHECK(SAR_PARM_NAME)
	)
	{
	    p = SARParmNew(SAR_PARM_NAME);
	    sar_parm_name_struct *pv =
		(sar_parm_name_struct *)p;

	    free(pv->name);
	    pv->name = STRDUP(val_str);

	    DO_ADD_PARM
	}
        /* Description (of file) */
        else if((!strcasecmp(parm_str, "description") ||
                 !strcasecmp(parm_str, "desc")
                ) && FILTER_CHECK(SAR_PARM_DESCRIPTION)
	)
        {
            p = SARParmNew(SAR_PARM_DESCRIPTION);
            sar_parm_description_struct *pv =
                (sar_parm_description_struct *)p;

            free(pv->description);
            pv->description = STRDUP(val_str);

            DO_ADD_PARM
        }
        /* Player model file */
        else if(!strcasecmp(parm_str, "player_model_file") &&
                FILTER_CHECK(SAR_PARM_PLAYER_MODEL_FILE)
	)
        {
            p = SARParmNew(SAR_PARM_PLAYER_MODEL_FILE);
            sar_parm_player_model_file_struct *pv =
                (sar_parm_player_model_file_struct *)p;

	    cstrptr = val_str;
            free(pv->file);
            pv->file = STRDUP(cstrptr);
	    CHOP_STR_BLANK(pv->file);

            DO_ADD_PARM
        }
	/* Weather */
        else if(!strcasecmp(parm_str, "weather") &&
                FILTER_CHECK(SAR_PARM_WEATHER)
	)
        {
            p = SARParmNew(SAR_PARM_WEATHER); 
            sar_parm_weather_struct *pv =
                (sar_parm_weather_struct *)p;

            free(pv->weather_preset_name);
            pv->weather_preset_name = STRDUP(val_str);
	    /* do not chop off name */
/*	    CHOP_STR_BLANK(pv->weather_preset_name); */
            
            DO_ADD_PARM
        }
	else if(!strcasecmp(parm_str, "wind") &&
		FILTER_CHECK(SAR_PARM_WIND)
	    )
	{
	    p = SARParmNew(SAR_PARM_WIND);
	    sar_parm_wind_struct *pv =
		(sar_parm_wind_struct *)p;

	    /* Heading */
	    cstrptr = val_str;
	    pv->heading = (float)DEGTORAD(ATOF(cstrptr));
	    cstrptr = NEXT_ARG(cstrptr);
	    pv->speed = (float)SFMKTSToMPC((double)ATOF(cstrptr));
	    cstrptr = NEXT_ARG(cstrptr);
	    pv->flags = 0;
	    while((cstrptr != NULL) ? (*cstrptr != '\0') : 0)
	    {
		if(strcasepfx(cstrptr, "gusts"))
		    pv->flags |= SAR_WIND_FLAG_GUSTS;

		cstrptr = NEXT_ARG(cstrptr);
	    }

	    DO_ADD_PARM
	}
        /* Time of day */
        else if(!strcasecmp(parm_str, "time_of_day") &&
                FILTER_CHECK(SAR_PARM_TIME_OF_DAY)
        )
        {
	    int h, m, s;
            p = SARParmNew(SAR_PARM_TIME_OF_DAY);
            sar_parm_time_of_day_struct *pv =
                (sar_parm_time_of_day_struct *)p;

            cstrptr = val_str;
	    SARParseTimeOfDay(cstrptr, &h, &m, &s);
	    pv->tod = (float)(
		(h * 3600.0) + (m * 60.0) + s
	    );

            DO_ADD_PARM
        }

        /* Registered location */
        else if((!strcasecmp(parm_str, "register_location") ||
                 !strcasecmp(parm_str, "reg_location") ||
                 !strcasecmp(parm_str, "reg_loc")
                ) && FILTER_CHECK(SAR_PARM_REGISTER_LOCATION)
	)
        {
            p = SARParmNew(SAR_PARM_REGISTER_LOCATION);
            sar_parm_register_location_struct *pv =
                (sar_parm_register_location_struct *)p;

	    /* Position (xyz) */
	    cstrptr = val_str;
	    pv->pos.x = ATOF(cstrptr);
	    cstrptr = NEXT_ARG(cstrptr);
	    pv->pos.y = ATOF(cstrptr);
            cstrptr = NEXT_ARG(cstrptr);
	    pv->pos.z = (float)SFMFeetToMeters(ATOF(cstrptr));

	    /* Direction */
            cstrptr = NEXT_ARG(cstrptr);
            pv->dir.heading = (float)DEGTORAD(ATOF(cstrptr));
            cstrptr = NEXT_ARG(cstrptr);
            pv->dir.pitch = (float)DEGTORAD(ATOF(cstrptr));
            cstrptr = NEXT_ARG(cstrptr);
            pv->dir.bank = (float)DEGTORAD(ATOF(cstrptr));

	    /* Name */
            cstrptr = NEXT_ARG(cstrptr);
	    free(pv->name);
	    pv->name = STRDUP(cstrptr);
	    /* Do not chop name */
/*	    CHOP_STR_BLANK(pv->name); */

            DO_ADD_PARM
        }
        /* Scene global positioning */
        else if((!strcasecmp(parm_str, "scene_gps") ||
                 !strcasecmp(parm_str, "scene_global_position")
                ) && FILTER_CHECK(SAR_PARM_SCENE_GPS)
        )
        {
            p = SARParmNew(SAR_PARM_SCENE_GPS);  
            sar_parm_scene_gps_struct *pv = 
                (sar_parm_scene_gps_struct *)p;

            /* Center offset in degrees */
            cstrptr = val_str;
	    SARParseLongitudeDMS(cstrptr, &pv->dms_x_offset);
            cstrptr = NEXT_ARG(cstrptr);
	    SARParseLatitudeDMS(cstrptr, &pv->dms_y_offset);

	    /* Planet radius in meters */
            cstrptr = NEXT_ARG(cstrptr);
            pv->planet_radius = ATOF(cstrptr);

            DO_ADD_PARM
	}
        /* Scene map */
        else if(!strcasecmp(parm_str, "scene_map") &&
                FILTER_CHECK(SAR_PARM_SCENE_MAP)
	)
        {
            p = SARParmNew(SAR_PARM_SCENE_MAP);
            sar_parm_scene_map_struct *pv =
                (sar_parm_scene_map_struct *)p;

            /* Size in meters */
            cstrptr = val_str;
            pv->width = ATOF(cstrptr);
            cstrptr = NEXT_ARG(cstrptr);
            pv->height = ATOF(cstrptr);

	    /* Texture file */
            cstrptr = NEXT_ARG(cstrptr);
            free(pv->file);
            pv->file = STRDUP(cstrptr);
	    CHOP_STR_BLANK(pv->file);

            DO_ADD_PARM
        }
	/* Scene globally applied elevation */
        else if(!strcasecmp(parm_str, "scene_elevation") &&
                FILTER_CHECK(SAR_PARM_SCENE_ELEVATION)
	)
        {
	    p = SARParmNew(SAR_PARM_SCENE_ELEVATION);
	    sar_parm_scene_elevation_struct *pv =
                (sar_parm_scene_elevation_struct *)p;

            /* Elevation in meters */
            cstrptr = val_str;
            pv->elevation = (float)SFMFeetToMeters(ATOF(cstrptr));

	    DO_ADD_PARM
	}
        /* Scene globally applied cant angle */
        else if(!strcasecmp(parm_str, "scene_cant") &&
                FILTER_CHECK(SAR_PARM_SCENE_CANT)
	)
        {
            p = SARParmNew(SAR_PARM_SCENE_CANT);
            sar_parm_scene_cant_struct *pv =
                (sar_parm_scene_cant_struct *)p;

            /* Angle in radians */
            cstrptr = val_str;
            pv->cant = (float)DEGTORAD(ATOF(cstrptr));
         
            DO_ADD_PARM
        }
        /* Scene ground base flags */
        else if(!strcasecmp(parm_str, "scene_ground_flags") &&
                FILTER_CHECK(SAR_PARM_SCENE_GROUND_FLAGS)
	)
        {
            p = SARParmNew(SAR_PARM_SCENE_GROUND_FLAGS);
            sar_parm_scene_ground_flags_struct *pv =
                (sar_parm_scene_ground_flags_struct *)p;

	    pv->flags = 0;

	    /* Get flags */
            cstrptr = val_str;
	    while((cstrptr != NULL) ? (*cstrptr != '\0') : 0)
	    {
		if(strcasepfx(cstrptr, "is_water") ||
                   strcasepfx(cstrptr, "iswater")
		)
		    pv->flags |= SAR_SCENE_BASE_FLAG_IS_WATER;

		cstrptr = NEXT_ARG(cstrptr);
	    }

            DO_ADD_PARM
        }
	/* Scene ground tile */
	else if(!strcasecmp(parm_str, "scene_ground_tile") &&
                FILTER_CHECK(SAR_PARM_SCENE_GROUND_TILE)
	)
        {
            p = SARParmNew(SAR_PARM_SCENE_GROUND_TILE);
            sar_parm_scene_ground_tile_struct *pv =
                (sar_parm_scene_ground_tile_struct *)p;

            /* Tile size in meters */
            cstrptr = val_str;
            pv->tile_width = ATOI(cstrptr);
            cstrptr = NEXT_ARG(cstrptr);
            pv->tile_height = ATOI(cstrptr);

	    /* Close range in meters */
            cstrptr = NEXT_ARG(cstrptr);
            pv->close_range = ATOF(cstrptr);

	    /* Far solid color */
            cstrptr = NEXT_ARG(cstrptr);
            pv->color.r = ATOF(cstrptr);
            cstrptr = NEXT_ARG(cstrptr);
            pv->color.g = ATOF(cstrptr);
            cstrptr = NEXT_ARG(cstrptr);
            pv->color.b = ATOF(cstrptr);
            cstrptr = NEXT_ARG(cstrptr);
            pv->color.a = ATOF(cstrptr);

            /* Texture name */
            cstrptr = NEXT_ARG(cstrptr);
            free(pv->texture_name);
            pv->texture_name = STRDUP(cstrptr);
	    CHOP_STR_BLANK(pv->texture_name);

            DO_ADD_PARM
	}
        /* Texture base directory */
        else if(!strcasecmp(parm_str, "texture_base_directory") &&
                FILTER_CHECK(SAR_PARM_TEXTURE_BASE_DIRECTORY)
	)
        {
            p = SARParmNew(SAR_PARM_TEXTURE_BASE_DIRECTORY);
            sar_parm_texture_base_directory_struct *pv =
                (sar_parm_texture_base_directory_struct *)p;

            free(pv->directory);
            pv->directory = STRDUP(val_str);
	    CHOP_STR_BLANK(pv->directory);

            DO_ADD_PARM
        }
        /* Texture load */
        else if(!strcasecmp(parm_str, "texture_load") &&
                FILTER_CHECK(SAR_PARM_TEXTURE_LOAD)
	)
        {
            p = SARParmNew(SAR_PARM_TEXTURE_LOAD);
            sar_parm_texture_load_struct *pv =
                (sar_parm_texture_load_struct *)p;

	    /* Name */
            cstrptr = val_str;
            free(pv->name);  
            pv->name = STRDUP(cstrptr);
	    CHOP_STR_BLANK(pv->name);

            /* File */
            cstrptr = NEXT_ARG(cstrptr);
            free(pv->file);
            pv->file = STRDUP(cstrptr);
	    CHOP_STR_BLANK(pv->file);

            /* Priority */
            cstrptr = NEXT_ARG(cstrptr);
            pv->priority = ATOF(cstrptr);

            DO_ADD_PARM
        }

        /* Mission scene file */
        else if(!strcasecmp(parm_str, "mission_scene_file") &&
                FILTER_CHECK(SAR_PARM_MISSION_SCENE_FILE)
	)
        {
            p = SARParmNew(SAR_PARM_MISSION_SCENE_FILE);
            sar_parm_mission_scene_file_struct *pv =
                (sar_parm_mission_scene_file_struct *)p;

            free(pv->file);
            pv->file = STRDUP(val_str);

            DO_ADD_PARM
        }
	/* Mission create new objective */
        else if(!strcasecmp(parm_str, "mission_objective_new") &&
                FILTER_CHECK(SAR_PARM_MISSION_NEW_OBJECTIVE)
        )
        {
            p = SARParmNew(SAR_PARM_MISSION_NEW_OBJECTIVE);
            sar_parm_mission_new_objective_struct *pv =
                (sar_parm_mission_new_objective_struct *)p;

            /* Objective type */
            cstrptr = val_str;
	    /* Arrive at objective? */
	    if(strcasepfx(cstrptr, "arrive"))
	    {
		pv->objective_type = SAR_MISSION_OBJECTIVE_ARRIVE_AT;
	    }
	    /* Pick up and arrive at? */
	    else if(strcasepfx(cstrptr, "pick_up_arrive"))
            {
                pv->objective_type = SAR_MISSION_OBJECTIVE_PICK_UP_ARRIVE_AT;
	    }
            /* Pick up? */
            else if(strcasepfx(cstrptr, "pick_up"))
            {
                pv->objective_type = SAR_MISSION_OBJECTIVE_PICK_UP;
            }
	    /* Unsupported mission objective type */
	    else
	    {
		pv->objective_type = -1;
		fprintf(
		    stderr,
"%s: %i: %s: Warning: Unsupported objective type `%s'.\n",
		    filename, *lines_read, parm_str, cstrptr
		);
	    }

            DO_ADD_PARM
        }
        /* Mission time left */
        else if(!strcasecmp(parm_str, "mission_objective_time_left") &&
                FILTER_CHECK(SAR_PARM_MISSION_TIME_LEFT)
	)
        {
            p = SARParmNew(SAR_PARM_MISSION_TIME_LEFT);
            sar_parm_mission_time_left_struct *pv =
                (sar_parm_mission_time_left_struct *)p;

            /* Time left in seconds (note that the type is float) */
            cstrptr = val_str;
            pv->time_left = ATOF(cstrptr);

            DO_ADD_PARM
        }
        /* Mission begin at */
        else if(!strcasecmp(parm_str, "mission_begin_at") &&
                FILTER_CHECK(SAR_PARM_MISSION_BEGIN_AT)
	)
        {
            p = SARParmNew(SAR_PARM_MISSION_BEGIN_AT);
            sar_parm_mission_begin_at_struct *pv =
                (sar_parm_mission_begin_at_struct *)p;

            /* Begin at object name */
            cstrptr = val_str;
	    free(pv->name);
	    pv->name = STRDUP(cstrptr);
	    /* Do not chop off string */
/*	    CHOP_STR_BLANK(pv->name); */

            DO_ADD_PARM
        }
        /* Mission begin at position */
        else if(!strcasecmp(parm_str, "mission_begin_at_pos") &&
                FILTER_CHECK(SAR_PARM_MISSION_BEGIN_AT_POS)
        )
        {
            p = SARParmNew(SAR_PARM_MISSION_BEGIN_AT_POS);
            sar_parm_mission_begin_at_pos_struct *pv =
                (sar_parm_mission_begin_at_pos_struct *)p;

	    /* Position */
            cstrptr = val_str;
            pv->pos.x = ATOF(cstrptr);
            cstrptr = NEXT_ARG(cstrptr);
            pv->pos.y = ATOF(cstrptr);
            cstrptr = NEXT_ARG(cstrptr);
            pv->pos.z = (float)SFMFeetToMeters(ATOF(cstrptr));

	    /* Direction */
	    cstrptr = NEXT_ARG(cstrptr);
            pv->dir.heading = (float)DEGTORAD(ATOF(cstrptr));
            cstrptr = NEXT_ARG(cstrptr);
            pv->dir.pitch = (float)DEGTORAD(ATOF(cstrptr));
            cstrptr = NEXT_ARG(cstrptr);
            pv->dir.bank = (float)DEGTORAD(ATOF(cstrptr));

            DO_ADD_PARM
        }
        /* Mission arrive at */
        else if(!strcasecmp(parm_str, "mission_objective_arrive_at") &&
                FILTER_CHECK(SAR_PARM_MISSION_ARRIVE_AT)
	)
        {
            p = SARParmNew(SAR_PARM_MISSION_ARRIVE_AT);
            sar_parm_mission_arrive_at_struct *pv =
                (sar_parm_mission_arrive_at_struct *)p;

            /* Arrive at object name */
            cstrptr = val_str;
            free(pv->name);
            pv->name = STRDUP(cstrptr);
	    /* Do not chop off string */
/*	    CHOP_STR_BLANK(pv->name); */

            DO_ADD_PARM
        }
        /* Mission message success */
        else if(!strcasecmp(parm_str, "mission_objective_message_success") &&
                FILTER_CHECK(SAR_PARM_MISSION_MESSAGE_SUCCESS)
	)
        {
            p = SARParmNew(SAR_PARM_MISSION_MESSAGE_SUCCESS);
            sar_parm_mission_message_success_struct *pv =
                (sar_parm_mission_message_success_struct *)p;

            /* Mission success message */
            cstrptr = val_str;
            free(pv->message);
            pv->message = STRDUP(cstrptr);
	    /* Do not chop off string */
/*	    CHOP_STR_BLANK(pv->message); */

            DO_ADD_PARM
        }
        /* Mission message fail */
        else if(!strcasecmp(parm_str, "mission_objective_message_fail") &&
                FILTER_CHECK(SAR_PARM_MISSION_MESSAGE_FAIL)
	)
        {
            p = SARParmNew(SAR_PARM_MISSION_MESSAGE_FAIL);
            sar_parm_mission_message_fail_struct *pv =
                (sar_parm_mission_message_fail_struct *)p;

            /* Mission fail message */
            cstrptr = val_str;
            free(pv->message);
            pv->message = STRDUP(cstrptr);
	    /* Do not chop off string */
/*	    CHOP_STR_BLANK(pv->message); */

            DO_ADD_PARM
        }
        /* Mission humans tally */
        else if(!strcasecmp(parm_str, "mission_objective_humans_tally") &&
                FILTER_CHECK(SAR_PARM_MISSION_HUMANS_TALLY)
	)
        {
            p = SARParmNew(SAR_PARM_MISSION_HUMANS_TALLY);
            sar_parm_mission_humans_tally_struct *pv =
                (sar_parm_mission_humans_tally_struct *)p;

            /* Initial humans that need to be rescued (can be less or
	     * equal to total_humans.
	     */
            cstrptr = val_str;
	    pv->humans_need_rescue = ATOI(cstrptr);

	    /* Total humans that needed to be rescued */
	    cstrptr = NEXT_ARG(cstrptr);
	    pv->total_humans = ATOI(cstrptr);

	    if(pv->humans_need_rescue < pv->total_humans)
		fprintf(
		    stderr,
"%s: %i: %s: Warning: humans_need_rescue `%i' is less than total_humans `%i'.\n",
		    filename, *lines_read, parm_str,
		    pv->humans_need_rescue, pv->total_humans
		);

            DO_ADD_PARM
        }
        /* Mission add intercept */
        else if(!strcasecmp(parm_str, "mission_add_intercept") &&
                FILTER_CHECK(SAR_PARM_MISSION_ADD_INTERCEPT)
	)
        {
            p = SARParmNew(SAR_PARM_MISSION_ADD_INTERCEPT);
            sar_parm_mission_add_intercept_struct *pv =
                (sar_parm_mission_add_intercept_struct *)p;

            /* Reference code */
            cstrptr = val_str;
            pv->ref_code = ATOI(cstrptr);

	    /* Handle rest by reference code to determine how many
	     * arguments.
	     */
	    switch(pv->ref_code)
	    {
	      /* Arrive or begin at location:
	       *
	       *	<radius> <urgency>
	       */
	      case 3: case 2:
		/* Radius */
		cstrptr = NEXT_ARG(cstrptr);
		pv->radius = ATOF(cstrptr);
                /* Urgency */
                cstrptr = NEXT_ARG(cstrptr);
                pv->urgency = ATOF(cstrptr);
		break;

	      /* Standard intercept way point:
	       *
	       *	<x> <y> <z> <radius> <urgency>
	       */
	      case 0:
		/* Position */
                cstrptr = NEXT_ARG(cstrptr);
                pv->pos.x = ATOF(cstrptr);
                cstrptr = NEXT_ARG(cstrptr);
                pv->pos.y = ATOF(cstrptr);
                cstrptr = NEXT_ARG(cstrptr);
                pv->pos.z = ATOF(cstrptr);	/* In meters */
                /* Radius */
                cstrptr = NEXT_ARG(cstrptr);
                pv->radius = ATOF(cstrptr);            
                /* Urgency */
                cstrptr = NEXT_ARG(cstrptr);
                pv->urgency = ATOF(cstrptr);
		break;

	      default:
                fprintf(
                    stderr,
"%s: %i: %s: Warning: Unsupported intercept code `%s'.\n",
                    filename, *lines_read, parm_str, cstrptr
                );
		break;
	    }

            /* Last argument is the intercept point's name */
            cstrptr = NEXT_ARG(cstrptr);
            free(pv->name);
            pv->name = STRDUP(cstrptr);
	    /* Do not chop off string */
/*	    CHOP_STR_BLANK(pv->name); */

            DO_ADD_PARM
        }

        /* New object */
        else if((!strcasecmp(parm_str, "create_object") ||
                 !strcasecmp(parm_str, "new_object") ||
                 !strcasecmp(parm_str, "add_object")
	        ) && FILTER_CHECK(SAR_PARM_NEW_OBJECT)
	)
        {
            p = SARParmNew(SAR_PARM_NEW_OBJECT);
            sar_parm_new_object_struct *pv =
                (sar_parm_new_object_struct *)p;

            /* First int is object type */
            cstrptr = val_str;
	    pv->object_type = ATOI(cstrptr);

            DO_ADD_PARM
        }
        /* New helipad */
        else if((!strcasecmp(parm_str, "create_helipad") ||
                 !strcasecmp(parm_str, "new_helipad") ||
                 !strcasecmp(parm_str, "add_helipad")
                ) && FILTER_CHECK(SAR_PARM_NEW_HELIPAD)
        )
        {
            p = SARParmNew(SAR_PARM_NEW_HELIPAD);
            sar_parm_new_helipad_struct *pv =
                (sar_parm_new_helipad_struct *)p;

            /* Begin parsing line, format:
             *
             *      <style> <length> <width> <recession> <label>
             *      <edge_lighting> <has_fuel> <has_repair> <has_drop_off>
             *      <restarting_point> <ref_obj_name>
             *      <offset_x> <offset_y> <offset_z>
             *      <offset_h> <offset_p> <offset_b>
             *
             * Note, underscore characters ('_') in label will be 
             * substituted for spaces after parsing.
             */

            /* Style name */
            cstrptr = val_str;
	    free(pv->style);
            pv->style = STRDUP(cstrptr);
	    CHOP_STR_BLANK(pv->style);

	    /* Length and width in meters */
	    cstrptr = NEXT_ARG(cstrptr);
	    pv->length = ATOF(cstrptr);
            cstrptr = NEXT_ARG(cstrptr);
            pv->width = ATOF(cstrptr);

	    /* Recession in meters */
            cstrptr = NEXT_ARG(cstrptr);
            pv->recession = (float)SFMFeetToMeters(ATOF(cstrptr));

            /* Label */
            cstrptr = NEXT_ARG(cstrptr);
            free(pv->label);
            pv->label = STRDUP(cstrptr);
	    CHOP_STR_BLANK(pv->label);
	    /* Set has label flag? */
	    if(pv->label != NULL)
		pv->flags |= SAR_HELIPAD_FLAG_LABEL;

	    /* Edge lighting */
	    cstrptr = NEXT_ARG(cstrptr);
	    if(StringIsYes(cstrptr))
		pv->flags |= SAR_HELIPAD_FLAG_EDGE_LIGHTING;

            /* Has fuel */
            cstrptr = NEXT_ARG(cstrptr);
            if(StringIsYes(cstrptr))
                pv->flags |= SAR_HELIPAD_FLAG_FUEL;

            /* Has repair */
            cstrptr = NEXT_ARG(cstrptr);
            if(StringIsYes(cstrptr))
                pv->flags |= SAR_HELIPAD_FLAG_REPAIR;

            /* Has drop off */
            cstrptr = NEXT_ARG(cstrptr);
            if(StringIsYes(cstrptr))
                pv->flags |= SAR_HELIPAD_FLAG_DROPOFF;

            /* Is a restarting point? */
            cstrptr = NEXT_ARG(cstrptr);
            if(StringIsYes(cstrptr))
                pv->flags |= SAR_HELIPAD_FLAG_RESTART_POINT;

	    /* Ref object name */
	    cstrptr = NEXT_ARG(cstrptr);
            free(pv->ref_obj_name);
            pv->ref_obj_name = STRDUP(cstrptr);
	    CHOP_STR_BLANK(pv->ref_obj_name);
            /* Set has ref object flag? */
            if(pv->ref_obj_name != NULL)
	    {
                pv->flags |= SAR_HELIPAD_FLAG_REF_OBJECT;
		pv->flags |= SAR_HELIPAD_FLAG_FOLLOW_REF_OBJECT;
	    }

	    /* Reference offset */
            cstrptr = NEXT_ARG(cstrptr);
            pv->ref_offset.x = ATOF(cstrptr);
            cstrptr = NEXT_ARG(cstrptr);
            pv->ref_offset.y = ATOF(cstrptr);
            cstrptr = NEXT_ARG(cstrptr);
            pv->ref_offset.z = (float)SFMFeetToMeters(ATOF(cstrptr));

	    /* Reference direction */
            cstrptr = NEXT_ARG(cstrptr);
            pv->ref_dir.heading = (float)DEGTORAD(ATOF(cstrptr));
            cstrptr = NEXT_ARG(cstrptr);
            pv->ref_dir.pitch = (float)DEGTORAD(ATOF(cstrptr));
            cstrptr = NEXT_ARG(cstrptr);
            pv->ref_dir.bank = (float)DEGTORAD(ATOF(cstrptr));

            DO_ADD_PARM
	}
        /* New runway */
        else if((!strcasecmp(parm_str, "create_runway") ||
                 !strcasecmp(parm_str, "new_runway") ||
                 !strcasecmp(parm_str, "add_runway")
                ) && FILTER_CHECK(SAR_PARM_NEW_RUNWAY)
        )
        {
            p = SARParmNew(SAR_PARM_NEW_RUNWAY);
            sar_parm_new_runway_struct *pv =
                (sar_parm_new_runway_struct *)p;

	    /* Format:
	     *
	     * <range> <length> <width> <surface>
	     * <dashes> <edge_light_spacing>
	     * <north_label> <south_label>
	     * <north_displaced_threshold> <south_displaced_threshold>
	     * [flags...]
	     */

	    /* Range (in meters) */
	    cstrptr = val_str;
	    pv->range = ATOF(cstrptr);

	    /* Size (in meters) */
	    cstrptr = NEXT_ARG(cstrptr);
            pv->length = ATOF(cstrptr);
            cstrptr = NEXT_ARG(cstrptr);
            pv->width = ATOF(cstrptr);

	    /* Surface Type */
	    cstrptr = NEXT_ARG(cstrptr);
            pv->surface_type = (sar_runway_surface_type)ATOI(cstrptr);

	    /* Dashes */
	    cstrptr = NEXT_ARG(cstrptr);
	    pv->dashes = ATOI(cstrptr);

	    /* Edge light spacing in meters */
            cstrptr = NEXT_ARG(cstrptr);
            pv->edge_light_spacing = ATOF(cstrptr);

            /* North label */
            cstrptr = NEXT_ARG(cstrptr);
            free(pv->north_label);
            pv->north_label = STRDUP(cstrptr);
            CHOP_STR_BLANK(pv->north_label);
	    /* South label */
            cstrptr = NEXT_ARG(cstrptr);
            free(pv->south_label);
            pv->south_label = STRDUP(cstrptr);
            CHOP_STR_BLANK(pv->south_label);

	    /* North displaced threshold */
            cstrptr = NEXT_ARG(cstrptr);
            pv->north_displaced_threshold = ATOF(cstrptr);
            /* South displaced threshold */
            cstrptr = NEXT_ARG(cstrptr);
            pv->south_displaced_threshold = ATOF(cstrptr);

	    /* Flags */
            cstrptr = NEXT_ARG(cstrptr);
            while((cstrptr != NULL) ? (*cstrptr != '\0') : 0)
            {
                /* Thresholds? */
                if(strcasepfx(cstrptr, "thresholds"))
                    pv->flags |= SAR_RUNWAY_FLAG_THRESHOLDS;
                /* Borders? */
                else if(strcasepfx(cstrptr, "borders"))
                    pv->flags |= SAR_RUNWAY_FLAG_BORDERS;
                /* Touch Down Markers? */
                else if(strcasepfx(cstrptr, "td_markers") ||
		        strcasepfx(cstrptr, "tdmarkers")
		)
                    pv->flags |= SAR_RUNWAY_FLAG_TD_MARKERS;
                /* Midway Markers? */
                else if(strcasepfx(cstrptr, "midway_markers") ||
                        strcasepfx(cstrptr, "midwaymarkers")
                )
                    pv->flags |= SAR_RUNWAY_FLAG_MIDWAY_MARKERS;
                /* North Glide Slope? */
                else if(strcasepfx(cstrptr, "north_gs") ||
                        strcasepfx(cstrptr, "northgs")
                )
                    pv->flags |= SAR_RUNWAY_FLAG_NORTH_GS;
                /* South Glide Slope? */
                else if(strcasepfx(cstrptr, "south_gs") ||
                        strcasepfx(cstrptr, "southgs")
                )
                    pv->flags |= SAR_RUNWAY_FLAG_SOUTH_GS;
                else
                    fprintf(
                        stderr,
"%s: %i: %s: Warning: Unsupported flag(s) `%s'\n",
                        filename, *lines_read, parm_str, cstrptr
                    );

                cstrptr = NEXT_ARG(cstrptr);
	    }

            DO_ADD_PARM
        }
        /* New human */
        else if((!strcasecmp(parm_str, "create_human") ||
                 !strcasecmp(parm_str, "new_human") ||
                 !strcasecmp(parm_str, "add_human")
                ) && FILTER_CHECK(SAR_PARM_NEW_HUMAN)
        )
        {
            p = SARParmNew(SAR_PARM_NEW_HUMAN);
            sar_parm_new_human_struct *pv =
                (sar_parm_new_human_struct *)p;

	    /* Begin parsing line, format:
	     *
	     *	<type_name>
	     *	<need_rescue?> <sitting?> <lying?> <alert?>
	     *	<aware?> <in_water?> <on_streatcher?>
             */

	    /* First argument is type name */
	    cstrptr = val_str;
	    free(pv->type_name);
	    pv->type_name = STRDUP(cstrptr);
	    CHOP_STR_BLANK(pv->type_name);

            /* Get flags */
            cstrptr = NEXT_ARG(cstrptr);
            while((cstrptr != NULL) ? (*cstrptr != '\0') : 0)
            {
                /* Need rescue? */
                if(strcasepfx(cstrptr, "need_rescue") ||
                   strcasepfx(cstrptr, "needrescue")
                )
                    pv->flags |= SAR_HUMAN_FLAG_NEED_RESCUE;
                /* Sit up? */
                else if(strcasepfx(cstrptr, "sit_up"))
                    pv->flags |= SAR_HUMAN_FLAG_SIT_UP;
                /* Sit down? */
                else if(strcasepfx(cstrptr, "sit_down"))
                    pv->flags |= SAR_HUMAN_FLAG_SIT_DOWN;
                /* Sit? */
                else if(strcasepfx(cstrptr, "sitting") ||
                        strcasepfx(cstrptr, "sit")
                )
                    pv->flags |= SAR_HUMAN_FLAG_SIT;
                /* Lying? */
                else if(strcasepfx(cstrptr, "lying") ||
                        strcasepfx(cstrptr, "lie")
                )
                    pv->flags |= SAR_HUMAN_FLAG_LYING;
                /* Alert? */
                else if(strcasepfx(cstrptr, "alert"))
                    pv->flags |= SAR_HUMAN_FLAG_ALERT;
                /* Aware? */
                else if(strcasepfx(cstrptr, "aware"))
                    pv->flags |= SAR_HUMAN_FLAG_AWARE;
                /* In water? */
                else if(strcasepfx(cstrptr, "in_water") ||
                        strcasepfx(cstrptr, "inwater")
                )
                    pv->flags |= SAR_HUMAN_FLAG_IN_WATER;
                /* On streatcher? */
                else if(strcasepfx(cstrptr, "on_streatcher") ||
                        strcasepfx(cstrptr, "onstreatcher")
                )
                    pv->flags |= SAR_HUMAN_FLAG_ON_STREATCHER;
		else
		    fprintf(
			stderr,
"%s: %i: %s: Warning: Unsupported flag(s) `%s'\n",
			filename, *lines_read, parm_str, cstrptr
		    );

		cstrptr = NEXT_ARG(cstrptr);
	    }

	    DO_ADD_PARM
	}
        /* New fire object */
        else if((!strcasecmp(parm_str, "create_fire") ||
                 !strcasecmp(parm_str, "new_fire") ||
                 !strcasecmp(parm_str, "add_fire")
                ) && FILTER_CHECK(SAR_PARM_NEW_FIRE)
        )
        {
            p = SARParmNew(SAR_PARM_NEW_FIRE);
            sar_parm_new_fire_struct *pv =
                (sar_parm_new_fire_struct *)p;

            /* Begin parsing line, format:
             *
             *      <radius> <height>
             */

            /* Radius in meters */
            cstrptr = val_str;
            pv->radius = (float)MAX(ATOF(cstrptr), 0.0);
            cstrptr = NEXT_ARG(cstrptr);
            pv->height = (float)MAX(SFMFeetToMeters(ATOF(cstrptr)), 0.0);

	    DO_ADD_PARM
	}
        /* New smoke object */
        else if((!strcasecmp(parm_str, "create_smoke") ||
                 !strcasecmp(parm_str, "new_smoke") ||
                 !strcasecmp(parm_str, "add_smoke")
                ) && FILTER_CHECK(SAR_PARM_NEW_SMOKE)
        )
        {
            p = SARParmNew(SAR_PARM_NEW_SMOKE);
            sar_parm_new_smoke_struct *pv =
                (sar_parm_new_smoke_struct *)p;

            /* Begin parsing line, format:
             *
             *      <offset_x> <offset_y> <offset_z> <radius_start>
	     *      <radius_max> <radius_rate> <hide_at_max> <respawn_int>
	     *      <total_units> <color_code>
             */

            /* Radius in meters */
            cstrptr = val_str;
            pv->offset.x = ATOF(cstrptr);
            cstrptr = NEXT_ARG(cstrptr);
            pv->offset.y = ATOF(cstrptr);
            cstrptr = NEXT_ARG(cstrptr);
            pv->offset.z = ATOF(cstrptr);	/* In meters */
            cstrptr = NEXT_ARG(cstrptr);
            pv->radius_start = ATOF(cstrptr);
            cstrptr = NEXT_ARG(cstrptr);
            pv->radius_max = ATOF(cstrptr);
            cstrptr = NEXT_ARG(cstrptr);
            pv->radius_rate = ATOF(cstrptr);
            cstrptr = NEXT_ARG(cstrptr);
            pv->hide_at_max = ATOI(cstrptr);
            cstrptr = NEXT_ARG(cstrptr);
            pv->respawn_int = ATOL(cstrptr);
            cstrptr = NEXT_ARG(cstrptr);
            pv->total_units = ATOI(cstrptr);
            cstrptr = NEXT_ARG(cstrptr);
	    pv->color_code = ATOI(cstrptr);

            DO_ADD_PARM
        }
        /* New premodeled object */
        else if((!strcasecmp(parm_str, "create_premodeled") ||
                 !strcasecmp(parm_str, "new_premodeled") ||
                 !strcasecmp(parm_str, "add_premodeled")
                ) && FILTER_CHECK(SAR_PARM_NEW_PREMODELED)
        )
        {
            p = SARParmNew(SAR_PARM_NEW_PREMODELED);
            sar_parm_new_premodeled_struct *pv =
                (sar_parm_new_premodeled_struct *)p;

	    /* First argument is model type */
	    cstrptr = val_str;
	    free(pv->model_type);
	    pv->model_type = STRDUP(cstrptr);
	    CHOP_STR_BLANK(pv->model_type);

	    /* Get remaining arguments as an exploded string array */
	    cstrptr = NEXT_ARG(cstrptr);
	    strlistfree(pv->argv, pv->argc);
	    pv->argv = strexp(cstrptr, &pv->argc);

            DO_ADD_PARM
        }

        /* Select object by name */
        else if(!strcasecmp(parm_str, "select_object_by_name") &&
                FILTER_CHECK(SAR_PARM_SELECT_OBJECT_BY_NAME)
        )
        {
            p = SARParmNew(SAR_PARM_SELECT_OBJECT_BY_NAME);
            sar_parm_select_object_by_name_struct *pv =
                (sar_parm_select_object_by_name_struct *)p;

            /* Name */
            cstrptr = val_str;
            free(pv->name);
            pv->name = STRDUP(cstrptr);
	    /* Do not chop off string */
/*	    CHOP_STR_BLANK(pv->name); */

            DO_ADD_PARM
	}

	/* Model file */
        else if(!strcasecmp(parm_str, "model_file") &&
                FILTER_CHECK(SAR_PARM_MODEL_FILE)
	)
        {
            p = SARParmNew(SAR_PARM_MODEL_FILE);
            sar_parm_model_file_struct *pv =
                (sar_parm_model_file_struct *)p;

	    /* File name */
	    cstrptr = val_str;
	    free(pv->file);
            pv->file = STRDUP(cstrptr);
	    CHOP_STR_BLANK(pv->file);

	    DO_ADD_PARM
	}
	/* Visual range */
        else if(!strcasecmp(parm_str, "range") &&
                FILTER_CHECK(SAR_PARM_RANGE)
        )
        {
            p = SARParmNew(SAR_PARM_RANGE);
            sar_parm_range_struct *pv =
                (sar_parm_range_struct *)p;

	    /* Range in meters */
	    cstrptr = val_str;
	    pv->range = ATOF(cstrptr);
	    if(pv->range < 0.0)
		fprintf(
		    stderr,
"%s: %i: %s: Warning: radius `%s' is negative.\n",
		    filename, *lines_read, parm_str, cstrptr
		);

            DO_ADD_PARM
        }
	/* Visual range far model display */
	else if(!strcasecmp(parm_str, "range_far") &&
                FILTER_CHECK(SAR_PARM_RANGE_FAR)
        )
        {
            p = SARParmNew(SAR_PARM_RANGE_FAR);
            sar_parm_range_far_struct *pv =
                (sar_parm_range_far_struct *)p;

            /* Far range in meters */
            cstrptr = val_str;
            pv->range_far = ATOF(cstrptr);
            if(pv->range_far < 0.0)
                fprintf(
                    stderr,
"%s: %i: %s: Warning: radius `%s' is negative.\n",
                    filename, *lines_read, parm_str, cstrptr
                );

            DO_ADD_PARM
        }
	/* Translate */
        else if((!strcasecmp(parm_str, "translate") ||
                 !strcasecmp(parm_str, "translation")
                ) && FILTER_CHECK(SAR_PARM_TRANSLATE)
	)
        {
            p = SARParmNew(SAR_PARM_TRANSLATE);
            sar_parm_translate_struct *pv =
                (sar_parm_translate_struct *)p;

            /* Translate in meters */
            cstrptr = val_str;
            pv->translate.x = ATOF(cstrptr);
	    cstrptr = NEXT_ARG(cstrptr);
            pv->translate.y = ATOF(cstrptr);
            cstrptr = NEXT_ARG(cstrptr);
            pv->translate.z = (float)SFMFeetToMeters(ATOF(cstrptr));

            DO_ADD_PARM
        }
        /* Translate random */
        else if((!strcasecmp(parm_str, "translate_random") ||
                 !strcasecmp(parm_str, "translation_random")
                ) && FILTER_CHECK(SAR_PARM_TRANSLATE_RANDOM)
        )
        {
            p = SARParmNew(SAR_PARM_TRANSLATE_RANDOM);
            sar_parm_translate_random_struct *pv =
                (sar_parm_translate_random_struct *)p;

	    /* Bounds in meters */
	    cstrptr = val_str;
	    pv->radius_bound = ATOF(cstrptr);

	    cstrptr = NEXT_ARG(cstrptr);
	    pv->z_bound = ATOF(cstrptr);

            DO_ADD_PARM
        }
        /* Rotate */
        else if(!strcasecmp(parm_str, "rotate") &&
                FILTER_CHECK(SAR_PARM_ROTATE)
	)
        {
            p = SARParmNew(SAR_PARM_ROTATE);
            sar_parm_rotate_struct *pv =
                (sar_parm_rotate_struct *)p;

            /* Rotate in radians */
            cstrptr = val_str;
            pv->rotate.heading = (float)DEGTORAD(ATOF(cstrptr));
            cstrptr = NEXT_ARG(cstrptr);
            pv->rotate.pitch = (float)DEGTORAD(ATOF(cstrptr));
            cstrptr = NEXT_ARG(cstrptr);
            pv->rotate.bank = (float)DEGTORAD(ATOF(cstrptr));

            DO_ADD_PARM
        }   
        /* No depth test */
        else if(!strcasecmp(parm_str, "no_depth_test") &&
                FILTER_CHECK(SAR_PARM_NO_DEPTH_TEST)
	)
        {
            p = SARParmNew(SAR_PARM_NO_DEPTH_TEST);
            sar_parm_no_depth_test_struct *pv =
                (sar_parm_no_depth_test_struct *)p;

            /* No arguments */
	    if(pv != NULL)
		DO_ADD_PARM
        }
        /* Polygon offset */
        else if(!strcasecmp(parm_str, "polygon_offset") &&
                FILTER_CHECK(SAR_PARM_POLYGON_OFFSET)
        )
        {
           p = SARParmNew(SAR_PARM_POLYGON_OFFSET);
            sar_parm_polygon_offset_struct *pv =
                (sar_parm_polygon_offset_struct *)p;

	    /* Reset flags */
	    pv->flags = SAR_OBJ_FLAG_POLYGON_OFFSET;

            /* Get flags */
            cstrptr = val_str;
            while((cstrptr != NULL) ? (*cstrptr != '\0') : 0)
            {
                /* Reverse offset? */
                if(strcasepfx(cstrptr, "reverse") ||
                   strcasepfx(cstrptr, "rev")
                )
		{
		    pv->flags &= ~SAR_OBJ_FLAG_POLYGON_OFFSET;
		    pv->flags |= SAR_OBJ_FLAG_POLYGON_OFFSET_REVERSE;
		}
                /* Write depth after polygon offsetting? */
                else if(strcasepfx(cstrptr, "write_depth") ||
                        strcasepfx(cstrptr, "writedepth")
                )
		{
                    pv->flags |= SAR_OBJ_FLAG_POLYGON_OFFSET_WRITE_DEPTH;
		}
		else
		{
		    fprintf(
			stderr,
"%s: %i: %s: Warning: Unsupported flag(s) `%s'.\n",
			filename, *lines_read, parm_str, cstrptr
		    );
		}

		cstrptr = NEXT_ARG(cstrptr);
	    }

            DO_ADD_PARM
        }
        /* Countact bounds spherical */
        else if(!strcasecmp(parm_str, "contact_spherical") &&
                FILTER_CHECK(SAR_PARM_CONTACT_BOUNDS_SPHERICAL)
	)
        {
            p = SARParmNew(SAR_PARM_CONTACT_BOUNDS_SPHERICAL);
            sar_parm_contact_bounds_spherical_struct *pv =
                (sar_parm_contact_bounds_spherical_struct *)p;

            /* Radius in meters */
	    cstrptr = val_str;
            pv->radius = ATOF(cstrptr);
            if(pv->radius < 0.0)
                fprintf(
                    stderr,
"%s: %i: %s: Warning: radius `%f' is negative.\n",
                    filename, *lines_read, parm_str, pv->radius
                );

            DO_ADD_PARM
        }
        /* Countact bounds cylendrical */
        else if(!strcasecmp(parm_str, "contact_cylendrical") &&
                FILTER_CHECK(SAR_PARM_CONTACT_BOUNDS_CYLENDRICAL)
	)
        {
            p = SARParmNew(SAR_PARM_CONTACT_BOUNDS_CYLENDRICAL);
            sar_parm_contact_bounds_cylendrical_struct *pv =
                (sar_parm_contact_bounds_cylendrical_struct *)p;

            /* Radius in meters */
            cstrptr = val_str;
            pv->radius = ATOF(cstrptr);
	    if(pv->radius < 0.0)
                fprintf(
                    stderr,
"%s: %i: %s: Warning: radius `%f' is negative.\n",
                    filename, *lines_read, parm_str, pv->radius
                );

	    /* Height min and max in meters */
	    cstrptr = NEXT_ARG(cstrptr);
	    pv->height_min = ATOF(cstrptr);
            cstrptr = NEXT_ARG(cstrptr);
            pv->height_max = ATOF(cstrptr);
	    if(pv->height_min > pv->height_max)
		fprintf(
		    stderr,
"%s: %i: %s: Warning: height_min `%f' is greater than height_max `%f'.\n",
		    filename, *lines_read, parm_str,
		    pv->height_min, pv->height_max
		);

            DO_ADD_PARM
        }
        /* Countact bounds rectangular */
        else if(!strcasecmp(parm_str, "contact_rectangular") &&
                FILTER_CHECK(SAR_PARM_CONTACT_BOUNDS_RECTANGULAR)
        )
        {
            p = SARParmNew(SAR_PARM_CONTACT_BOUNDS_RECTANGULAR);
            sar_parm_contact_bounds_rectangular_struct *pv =
                (sar_parm_contact_bounds_rectangular_struct *)p;

            /* X min and max */
            cstrptr = val_str;
            pv->x_min = ATOF(cstrptr);
	    cstrptr = NEXT_ARG(cstrptr);
            pv->x_max = ATOF(cstrptr);
            if(pv->x_min > pv->x_max)
                fprintf(
                    stderr,
"%s: %i: %s: Warning: x_min `%f' is greater than x_max `%f'.\n",
                    filename, *lines_read, parm_str,
                    pv->x_min, pv->x_max
                );

            /* Y min and max */
            cstrptr = NEXT_ARG(cstrptr);
            pv->y_min = ATOF(cstrptr);
            cstrptr = NEXT_ARG(cstrptr);
            pv->y_max = ATOF(cstrptr);
            if(pv->y_min > pv->y_max)
                fprintf(
                    stderr,
"%s: %i: %s: Warning: y_min `%f' is greater than y_max `%f'.\n",
                    filename, *lines_read, parm_str,
                    pv->y_min, pv->y_max
                );

            /* Z min and max */
            cstrptr = NEXT_ARG(cstrptr);
            pv->z_min = ATOF(cstrptr);
            cstrptr = NEXT_ARG(cstrptr);
            pv->z_max = ATOF(cstrptr);
            if(pv->z_min > pv->z_max)
                fprintf(
                    stderr,
"%s: %i: %s: Warning: z_min `%f' is greater than z_max `%f'.\n",
                    filename, *lines_read, parm_str,
                    pv->z_min, pv->z_max
                );

            DO_ADD_PARM
        }
        /* Ground elevation */
        else if(!strcasecmp(parm_str, "ground_elevation") &&
                FILTER_CHECK(SAR_PARM_GROUND_ELEVATION)
	)
        {
            p = SARParmNew(SAR_PARM_GROUND_ELEVATION);
            sar_parm_ground_elevation_struct *pv =
                (sar_parm_ground_elevation_struct *)p;

	    /* Ground elevation in meters */
	    cstrptr = val_str;
	    pv->elevation = (float)SFMFeetToMeters(ATOF(cstrptr));

            DO_ADD_PARM
        }
        /* Object name */
        else if((!strcasecmp(parm_str, "object_name") ||
                 !strcasecmp(parm_str, "obj_name")
                ) && FILTER_CHECK(SAR_PARM_OBJECT_NAME)
	)
        {
            p = SARParmNew(SAR_PARM_OBJECT_NAME);
            sar_parm_object_name_struct *pv =
                (sar_parm_object_name_struct *)p;

            cstrptr = val_str;
	    free(pv->name);
  	    pv->name = STRDUP(cstrptr);
	    /* Do not chop off string */
/*	    CHOP_STR_BLANK(pv->name); */

            DO_ADD_PARM
        }
        /* Object map description */
        else if((!strcasecmp(parm_str, "object_map_description") ||
                 !strcasecmp(parm_str, "object_map_desc") ||
                 !strcasecmp(parm_str, "obj_map_description") ||
                 !strcasecmp(parm_str, "obj_map_desc")
                ) && FILTER_CHECK(SAR_PARM_OBJECT_MAP_DESCRIPTION)
        )
        {
            p = SARParmNew(SAR_PARM_OBJECT_MAP_DESCRIPTION);
            sar_parm_object_map_description_struct *pv =
                (sar_parm_object_map_description_struct *)p;

            cstrptr = val_str;
            free(pv->description);
            pv->description = STRDUP(cstrptr);
            /* Do not chop off string */
/*	    CHOP_STR_BLANK(pv->description); */

            DO_ADD_PARM
        }
        /* Object fuel */
        else if(!strcasecmp(parm_str, "fuel") &&
                FILTER_CHECK(SAR_PARM_FUEL)
	)
        {
            p = SARParmNew(SAR_PARM_FUEL);
            sar_parm_fuel_struct *pv =
                (sar_parm_fuel_struct *)p;

	    /* Get fuel values in kg */
            cstrptr = val_str;
	    pv->fuel = ATOF(cstrptr);
            cstrptr = NEXT_ARG(cstrptr);
            pv->fuel_max = ATOF(cstrptr);
            if(pv->fuel > pv->fuel_max)
                fprintf(
                    stderr,
"%s: %i: %s: Warning: fuel `%f' is greater than fuel_max `%f'.\n",
                    filename, *lines_read, parm_str,
                    pv->fuel, pv->fuel_max
                );

            DO_ADD_PARM
        }
        /* Object hit points */
        else if(!strcasecmp(parm_str, "hitpoints") &&
                FILTER_CHECK(SAR_PARM_HITPOINTS)
	)
        {
            p = SARParmNew(SAR_PARM_HITPOINTS);
            sar_parm_hitpoints_struct *pv =
                (sar_parm_hitpoints_struct *)p;

	    /* Get hit point values */
            cstrptr = val_str;
            pv->hitpoints = ATOF(cstrptr);
            cstrptr = NEXT_ARG(cstrptr);
            pv->hitpoints_max = ATOF(cstrptr);
            if(pv->hitpoints > pv->hitpoints_max)
                fprintf(
                    stderr,
"%s: %i: %s: Warning: hitpoints `%f' is greater than hitpoints_max `%f'.\n",
                    filename, *lines_read, parm_str,
                    pv->hitpoints, pv->hitpoints_max
                );

            DO_ADD_PARM
        }
        /* Engine state */
        else if(!strcasecmp(parm_str, "engine_state") &&
                FILTER_CHECK(SAR_PARM_ENGINE_STATE)
	)
        {
            p = SARParmNew(SAR_PARM_ENGINE_STATE);
            sar_parm_engine_state_struct *pv =
                (sar_parm_engine_state_struct *)p;

	    /* Get engine state */
            cstrptr = val_str;
            pv->state = ATOI(cstrptr);

            DO_ADD_PARM
	}
        /* Passengers */
        else if(!strcasecmp(parm_str, "passengers") &&
                FILTER_CHECK(SAR_PARM_PASSENGERS)
	)
        {
            p = SARParmNew(SAR_PARM_PASSENGERS);
            sar_parm_passengers_struct *pv =
                (sar_parm_passengers_struct *)p;

	    /* Get passenger values */
            cstrptr = val_str;
            pv->passengers = ATOI(cstrptr);
            cstrptr = NEXT_ARG(cstrptr);
            pv->passengers_max = ATOI(cstrptr);
            if((pv->passengers > pv->passengers_max) &&
	       (pv->passengers_max >= 0)
	    )
                fprintf(
                    stderr,
"%s: %i: %s: Warning: passengers `%i' is greater than passengers_max `%i'.\n",
                    filename, *lines_read, parm_str,
                    pv->passengers, pv->passengers_max
                );

            DO_ADD_PARM
        }
        /* Runway approach lighting north */
        else if(!strcasecmp(parm_str, "runway_approach_lighting_north")
		&& FILTER_CHECK(SAR_PARM_RUNWAY_APPROACH_LIGHTING_NORTH)
	)
	{
	    p = SARParmNew(SAR_PARM_RUNWAY_APPROACH_LIGHTING_NORTH);
	    sar_parm_runway_approach_lighting_north_struct *pv =
		(sar_parm_runway_approach_lighting_north_struct *)p;

	    /* Flags */
	    cstrptr = val_str;
            while((cstrptr != NULL) ? (*cstrptr != '\0') : 0)
            {
                /* End lighting? */
                if(strcasepfx(cstrptr, "end"))
                    pv->flags |= SAR_RUNWAY_APPROACH_LIGHTING_END;
                /* "Tracer shells"? */
                else if(strcasepfx(cstrptr, "tracer"))
                    pv->flags |= SAR_RUNWAY_APPROACH_LIGHTING_TRACER;
                /* Align lighting? */
                else if(strcasepfx(cstrptr, "align"))
                    pv->flags |= SAR_RUNWAY_APPROACH_LIGHTING_ALIGN;
                /* ILS glide mark lighting? */
                else if(strcasepfx(cstrptr, "ils_glide"))
                    pv->flags |= SAR_RUNWAY_APPROACH_LIGHTING_ILS_GLIDE;
                else
                    fprintf(
                        stderr,
"%s: %i: %s: Warning: Unsupported flag(s) `%s'\n",
                        filename, *lines_read, parm_str, cstrptr
                    );

                cstrptr = NEXT_ARG(cstrptr);
            }

	    DO_ADD_PARM
	}
        /* Runway approach lighting south */
        else if(!strcasecmp(parm_str, "runway_approach_lighting_south")
                && FILTER_CHECK(SAR_PARM_RUNWAY_APPROACH_LIGHTING_SOUTH)
        )
        {
            p = SARParmNew(SAR_PARM_RUNWAY_APPROACH_LIGHTING_SOUTH);
            sar_parm_runway_approach_lighting_south_struct *pv =
                (sar_parm_runway_approach_lighting_south_struct *)p;

            /* Flags */
            cstrptr = val_str;
            while((cstrptr != NULL) ? (*cstrptr != '\0') : 0)
            {
                /* End lighting? */
                if(strcasepfx(cstrptr, "end"))
                    pv->flags |= SAR_RUNWAY_APPROACH_LIGHTING_END;
                /* "Tracer shells"? */
                else if(strcasepfx(cstrptr, "tracer"))
                    pv->flags |= SAR_RUNWAY_APPROACH_LIGHTING_TRACER;
                /* Align lighting? */
                else if(strcasepfx(cstrptr, "align"))
                    pv->flags |= SAR_RUNWAY_APPROACH_LIGHTING_ALIGN;
                /* ILS glide mark lighting? */
                else if(strcasepfx(cstrptr, "ils_glide"))
                    pv->flags |= SAR_RUNWAY_APPROACH_LIGHTING_ILS_GLIDE;
                else
                    fprintf(
                        stderr,
"%s: %i: %s: Warning: Unsupported flag(s) `%s'\n",
                        filename, *lines_read, parm_str, cstrptr
                    );

                cstrptr = NEXT_ARG(cstrptr);
            }

            DO_ADD_PARM
        }
        /* Human message enter */
        else if((!strcasecmp(parm_str, "set_human_message_enter") ||
                 !strcasecmp(parm_str, "set_human_mesg_enter")
                ) && FILTER_CHECK(SAR_PARM_HUMAN_MESSAGE_ENTER)
	)
        {
            p = SARParmNew(SAR_PARM_HUMAN_MESSAGE_ENTER);
            sar_parm_human_message_enter_struct *pv =
                (sar_parm_human_message_enter_struct *)p;

            cstrptr = val_str;
	    free(pv->message);
	    pv->message = STRDUP(cstrptr);
            /* Do not chop off string */
/*	    CHOP_STR_BLANK(pv->message); */

            DO_ADD_PARM
        }
        /* Human reference */
        else if((!strcasecmp(parm_str, "human_reference") ||
                 !strcasecmp(parm_str, "human_ref")
                ) && FILTER_CHECK(SAR_PARM_HUMAN_REFERENCE)
        )
        {
            p = SARParmNew(SAR_PARM_HUMAN_REFERENCE);
            sar_parm_human_reference_struct *pv =
                (sar_parm_human_reference_struct *)p;

	    /* Begin parsing line, format:
	     *
	     *	<ref_name>
	     *  <run_to?> <run_away?>
	     */

            /* First argument is reference object name */
            cstrptr = val_str;
            free(pv->reference_name);
            pv->reference_name = STRDUP(cstrptr);
            CHOP_STR_BLANK(pv->reference_name);

            /* Get flags */
            cstrptr = NEXT_ARG(cstrptr);
            while((cstrptr != NULL) ? (*cstrptr != '\0') : 0)
            {
                /* Run towards? */
                if(strcasepfx(cstrptr, "run_towards") ||
                   strcasepfx(cstrptr, "run_to")
                )
                    pv->flags |= SAR_HUMAN_FLAG_RUN_TOWARDS;
                /* Run away? */
                else if(strcasepfx(cstrptr, "run_away"))
                    pv->flags |= SAR_HUMAN_FLAG_RUN_AWAY;
		else
                    fprintf(
                        stderr,
"%s: %i: %s: Warning: Unsupported flag(s) `%s'.\n",
                        filename, *lines_read, parm_str, cstrptr
                    );

                cstrptr = NEXT_ARG(cstrptr);
            }

            DO_ADD_PARM
	}

        else if (!strcasecmp(parm_str, "set_welcome_message") &&
                 FILTER_CHECK(SAR_PARM_WELCOME_MESSAGE)){


            p = SARParmNew(SAR_PARM_WELCOME_MESSAGE);
            sar_parm_welcome_message_struct *pv =
                (sar_parm_welcome_message_struct *)p;
            
            cstrptr = val_str;
	    free(pv->message);
	    pv->message = STRDUP(cstrptr);
            
            DO_ADD_PARM
        }

	/* Unsupported parameter */
	else
	{
	    /* Print warning only if we are not filtering any
	     * parm types.
	     */
	    if(filter_parm_type < 0)
		fprintf(
		    stderr,
"%s: %i: Warning: Unsupported parameter `%s'.\n",
		    filename, *lines_read, parm_str
		);
	}

#undef FILTER_CHECK
#undef DO_ADD_PARM

	return(parms_loaded);
}


/*
 *	Loads a scene or mission file, called from SARParmLoadFromFile().
 */
static int SARParmLoadFromFileScene(
        const char *filename, FILE *fp,
        void ***parm, int *total_parms,
        int filter_parm_type,
        void *client_data,
        int (*progress_func)(void *, long, long),
	long file_size, int *lines_read
)
{
        int c;
        const char *parm_str;
        char *val_str;


        /* Begin reading file */
        while(1)
        {
            c = fgetc(fp);
            if(c == EOF)
                break;

            /* Call progress callback */
            if(progress_func != NULL)
            {
                if(progress_func(client_data, ftell(fp), file_size))
                    break;
            }

            /* Read past leading spaces */
            if(ISBLANK(c))
            {
                FSeekPastSpaces(fp);

                /* Get first non space character */
                c = fgetc(fp);
                if(c == EOF)
                    break;
            }

            /* Newline character? */
            if(ISCR(c))
            {
                *lines_read = (*lines_read) + 1;
                continue;
            }

            /* Comment? */
            if(ISCOMMENT(c))
            {
                FSeekNextLine(fp);      /* Seek to next line */
                *lines_read = (*lines_read) + 1;
                continue;
            }

            /* Seek back one character for fetching this parm */
            fseek(fp, -1, SEEK_CUR);

            /* Get pointer to parameter string */
            parm_str = SARParmLoadFromFileGetParmString(fp);

            /* Get value string, escape sequences parsed for escaped
             * newline characters (escaped newlines not saved).
             */
            val_str = FGetString(fp);

            /* Handle this parameter */
            SARParmLoadFromFileIterate(
                filename, parm, total_parms,
                parm_str, val_str,
                lines_read, filter_parm_type
            );

	    free(val_str);
        }

	return(0);
}

/*
 *	Loads a mission log file, called from SARParmLoadFromFile().
 */
static int SARParmLoadFromFileMissionLog(
        const char *filename, FILE *fp,
        void ***parm, int *total_parms,
        int filter_parm_type,
        void *client_data,
        int (*progress_func)(void *, long, long),
        long file_size, int *lines_read
)
{
	int in_header = 0;
	int parm_num;
	const char *line_parm, *val;
	char *line_buf;
	void *p;
	sar_parm_mission_log_header_struct *p_mission_log_header = NULL;
	sar_parm_mission_log_event_struct *p_mission_log_event;


/* Adds parm p to the given parm list */
#define DO_ADD_PARM				\
{						\
 parm_num = MAX(*total_parms, 0);		\
 *total_parms = parm_num + 1;			\
 *parm = (void **)realloc(			\
  *parm, (*total_parms) * sizeof(void *)	\
 );						\
 if((*parm) == NULL)				\
 {						\
  *total_parms = 0;				\
  return(0);					\
 }						\
 else						\
 {						\
  (*parm)[parm_num] = p;			\
 }						\
}

/* Checks if filter parm type matches with the given parm type.
 * If matches or filter_parm_type is -1 then returns true.
 */
#define FILTER_CHECK(t)		( \
 (filter_parm_type == (t)) || (filter_parm_type < 0) \
)

	/* Allocate header structure just before loading of actual header
	 * from file.
	 */
	if(FILTER_CHECK(SAR_PARM_MISSION_LOG_HEADER))
	{
	    p = SARParmNew(SAR_PARM_MISSION_LOG_HEADER);
	    p_mission_log_header = (sar_parm_mission_log_header_struct *)p;

	    DO_ADD_PARM
	}

        /* Begin reading header (first few lines) */
        line_buf = NULL;
        while(1)
        {
            /* Delete previous line and load new line
	     * if new line is NULL then that implies end of file is
	     * reached
             */
            free(line_buf);
            line_buf = FGetStringLiteral(fp);
            if(line_buf == NULL)
                break;

            /* Check if this is a comment or empty line and get the
	     * pointer to the parameter.
	     */
	    line_parm = line_buf;
            while(ISBLANK(*line_parm))
                line_parm++;
            if(ISCOMMENT(*line_parm))
                continue;

            /* Set val pointer to start of argument, can be NULL if
             * there is no argument.
             */
	    val = line_parm;
	    while(!ISBLANK(*val) && (*val != '\0'))
		val++;
	    while(ISBLANK(*val))
		val++;

            /* Begin handling by log header parameter name */

	    /* Version */
	    if(strcasepfx(line_parm, "Version"))
            {
                if((p_mission_log_header != NULL) && in_header)
                {
		    sscanf(
			val, "%i %i %i",
			&p_mission_log_header->version_major,
			&p_mission_log_header->version_minor,
			&p_mission_log_header->version_release
		    );
		}
	    }
            /* Title */
            else if(strcasepfx(line_parm, "Title"))
            {
                if((p_mission_log_header != NULL) && in_header)
                {
                    free(p_mission_log_header->title);
                    p_mission_log_header->title = STRDUP(val);
                }
            }
            /* Scene file */
            else if(strcasepfx(line_parm, "SceneFile"))
            {
		if((p_mission_log_header != NULL) && in_header)
		{
		    free(p_mission_log_header->scene_file);
		    p_mission_log_header->scene_file = STRDUP(val);
		}
            }
            /* Player stats file */
            else if(strcasepfx(line_parm, "PlayerStatsFile"))
            {
                if((p_mission_log_header != NULL) && in_header)
                {
		    free(p_mission_log_header->player_stats_file);
		    p_mission_log_header->player_stats_file = STRDUP(val);
		}
            }
	    /* Begin of header? */
	    else if(strcasepfx(line_parm, "BeginHeader"))
            {
		in_header = 1;
            }
            /* End of header? */
            else if(strcasepfx(line_parm, "EndHeader"))
            {
		in_header = 0;
		break;
            }
            /* Unsupported header parameter */
            else
            {

            }
        }

        /* Done reading header (if any), now load events which are
	 * subsequent to the header
	 */
        while(1)
        {
            /* Delete previous line and load new line
	     * If the new line is NULL then that implies end of file
	     * is reached
             */
            free(line_buf);
            line_buf = FGetStringLiteral(fp);
            if(line_buf == NULL)
                break;

	    /* Check if this is a comment or empty line and set the
	     * pointer to the start of the value.
	     */
	    val = line_buf;
            while(ISBLANK(*val))
                val++;
            if(ISCOMMENT(*val))
                continue;
	    if(*val == '\0')
		continue;


            /* Begin handing this line as one logged event */

            /* First set of arguments are numbers leading up to a ':'
             * character.
             */
            if((val != NULL) && FILTER_CHECK(SAR_PARM_MISSION_LOG_EVENT))
            {
#define num_str_len     1024
		char *s;
		const char *mesg_ptr;
                char num_str[num_str_len];


		/* Allocate a new log event structure and add it to the
		 * parm list.
		 */
		p = SARParmNew(SAR_PARM_MISSION_LOG_EVENT);
		p_mission_log_event = (sar_parm_mission_log_event_struct *)p;
		DO_ADD_PARM


                /* Copy value string to num_str for tempory parsing */
                strncpy(num_str, val, num_str_len);
                num_str[num_str_len - 1] = '\0';

                /* Look for ending ':' character and null terminate as
                 * needed, this marks the end of the arguments portion of
		 * the line.
                 */
		s = strchr(num_str, ':');
                if(s != NULL)
                    *s = '\0';

                /* Parse arguments before the ':' character, format:
		 *
		 *	<type> <tod> <x> <y> <z> [...]
		 */
                sscanf(
                    num_str,
                    "%i %f %f %f %f",
		    &p_mission_log_event->event_type,
		    &p_mission_log_event->tod,
                    &p_mission_log_event->pos.x,
                    &p_mission_log_event->pos.y,
                    &p_mission_log_event->pos.z		/* In meters */
                );

                /* Get pointer to message portion of logged event,
		 * this is the portion of the line after the ':'
		 * character.
		 */
                mesg_ptr = strchr(val, ':');
                if(mesg_ptr != NULL)
		{
                    mesg_ptr++;
		    while(ISBLANK(*mesg_ptr))
			mesg_ptr++;

		    /* Is string past the ':' character an empty string?
		     * If it is then mesg_ptr needs to be set back to
		     * NULL indicating that there was no message.
		     */
		    if(*mesg_ptr == '\0')
			mesg_ptr = NULL;
		}

                /* Get pointer to message portion of logged event */
		free(p_mission_log_event->message);
		p_mission_log_event->message = STRDUP(mesg_ptr);

#undef num_str_len
	    }
	}

#undef DO_ADD_PARM
#undef FILTER_CHECK

	return(0);
}


/*
 *	Loads a set of parameters from file.
 *
 *	The given file_format is a hint to the file's format.
 *
 *	Returns non-zero on error.
 *
 *	The given parm and total_parms are assumed valid but undefined.
 */
int SARParmLoadFromFile(
        const char *filename, int file_format,
	void ***parm, int *total_parms,
	int filter_parm_type,
        void *client_data,
        int (*progress_func)(void *, long, long)
)
{
        int lines_read = 0, status = -1;
        FILE *fp;
        long file_size;
	struct stat stat_buf;


	/* Reset loaded parms array */
	*parm = NULL;
	*total_parms = 0;

	if(filename == NULL)
	    return(status);

	/* Check if file does not exist */
        if(stat(filename, &stat_buf))
            return(status);

	/* Get file size */
        file_size = stat_buf.st_size;

        /* Open file for reading */
        fp = FOpen(filename, "rb");
        if(fp == NULL)
            return(status);

	/* Load by file format type */
	/* Scene or mission format? */
	if((file_format == SAR_FILE_FORMAT_SCENE) ||
           (file_format == SAR_FILE_FORMAT_MISSION)
	)
	{
	    status = SARParmLoadFromFileScene(
		filename, fp,
		parm, total_parms,
		filter_parm_type,
		client_data, progress_func,
		file_size, &lines_read
	    );
	}
	/* Mission log? */
	else if(file_format == SAR_FILE_FORMAT_MISSION_LOG)
	{
            status = SARParmLoadFromFileMissionLog(
                filename, fp,
                parm, total_parms,
                filter_parm_type,
                client_data, progress_func,
                file_size, &lines_read
            );
	}

	/* Close file */
	FClose(fp);

	return(status);
}
