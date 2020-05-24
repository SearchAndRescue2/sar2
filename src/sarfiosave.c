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

static int SARParmSaveToFileAnyIterate(
	const char *filename, FILE *fp, const void *p
);
static int SARParmSaveToFileAny(
        const char *filename, FILE *fp,
        void **parm, int total_parms,
        void *client_data,
        int (*progress_func)(void *, long, long)
);
int SARParmSaveToFile(
        const char *filename, int file_format,
        void **parm, int total_parms,
        void *client_data,
        int (*progress_func)(void *, long, long)
);


#ifndef SAR_COMMENT_CHAR
# define SAR_COMMENT_CHAR       '#'
#endif

#define ATOI(s)         (((s) != NULL) ? atoi(s) : 0)
#define ATOL(s)         (((s) != NULL) ? atol(s) : 0)
#define ATOF(s)         (((s) != NULL) ? atof(s) : 0.0f)
#define STRDUP(s)       (((s) != NULL) ? strdup(s) : NULL)

#define MAX(a,b)        (((a) > (b)) ? (a) : (b))
#define MIN(a,b)        (((a) < (b)) ? (a) : (b))
#define CLIP(a,l,h)     (MIN(MAX((a),(l)),(h)))

#define ISCOMMENT(c)    ((c) == SAR_COMMENT_CHAR)
#define ISCR(c)         (((c) == '\n') || ((c) == '\r'))

#define DEGTORAD(d)     ((d) * PI / 180)
#define RADTODEG(r)     ((r) * 180 / PI)


/*
 *	Saves the given parameter to file.
 *
 *	Returns the number of parameters saved.
 */
static int SARParmSaveToFileAnyIterate(
        const char *filename, FILE *fp, const void *p
)
{
	int parms_saved = 0;
	int type = *(int *)p;

	/* Version. */
	if(type == SAR_PARM_VERSION)
	{
#define PARM_TYPE	const sar_parm_version_struct
	    PARM_TYPE *pv = (PARM_TYPE *)p;
	    fprintf(
		fp,
		"version %i %i %i%s%s\n",
		pv->major, pv->minor, pv->release,
		(pv->copyright != NULL) ? " " : "",
		(pv->copyright != NULL) ? pv->copyright : ""
	    );
	    parms_saved++;
#undef PARM_TYPE
	}
	/* Name. */
	else if(type == SAR_PARM_NAME)
	{
#define PARM_TYPE	const sar_parm_name_struct
	    PARM_TYPE *pv = (PARM_TYPE *)p;
	    if(pv->name != NULL)
	    {
		fprintf(
		    fp,
		    "name %s\n",
		    pv->name
		);
		parms_saved++;
	    }
#undef PARM_TYPE
	}
        /* Description. */
        else if(type == SAR_PARM_DESCRIPTION)
        {
#define PARM_TYPE	const sar_parm_description_struct
            PARM_TYPE *pv = (PARM_TYPE *)p;
            if(pv->description != NULL)
            {
                fprintf(
                    fp,
                    "description %s\n",
                    pv->description
                );
                parms_saved++;
            }
#undef PARM_TYPE
        }
        /* Player model file. */
        else if(type == SAR_PARM_PLAYER_MODEL_FILE)
        {
#define PARM_TYPE       const sar_parm_player_model_file_struct
            PARM_TYPE *pv = (PARM_TYPE *)p;
            if(pv->file != NULL)
            {
                fprintf(
                    fp,
                    "player_model_file %s\n",
                    pv->file
                );
                parms_saved++;
            }
#undef PARM_TYPE
        }
        /* Weather. */
        else if(type == SAR_PARM_WEATHER)
        {
#define PARM_TYPE       const sar_parm_weather_struct
            PARM_TYPE *pv = (PARM_TYPE *)p;
            if(pv->weather_preset_name != NULL)
            {
                fprintf(
                    fp,
                    "weather %s\n",
                    pv->weather_preset_name
                );
                parms_saved++;
            }
#undef PARM_TYPE
        }
        /* Time of day. */
        else if(type == SAR_PARM_TIME_OF_DAY)
        {
#define PARM_TYPE	const sar_parm_time_of_day_struct
            PARM_TYPE *pv = (PARM_TYPE *)p;
            fprintf(
                fp,
                "time_of_day %f\n",
                pv->tod
            );
            parms_saved++;
#undef PARM_TYPE
	}
	/* Wind */
	else if(type == SAR_PARM_WIND)
	{
#define PARM_TYPE	const sar_parm_wind_struct
            PARM_TYPE *pv = (PARM_TYPE *)p;
	    fprintf(
		fp,
		"wind %f %f"
		);
	    if(pv->flags & SAR_WIND_FLAG_GUSTS)
		fprintf(fp, " gusts");

	    fputc('\n', fp);
	    parms_saved++;
#undef PARM_TYPE
        }
        /* Registered location. */
        else if(type == SAR_PARM_REGISTER_LOCATION)
        {
#define PARM_TYPE	const sar_parm_register_location_struct
            PARM_TYPE *pv = (PARM_TYPE *)p;
            fprintf(
                fp,
                "register_location %f %f %f %f %f %f%s%s\n",
                pv->pos.x, pv->pos.y, SFMMetersToFeet(pv->pos.z),
		RADTODEG(pv->dir.heading),
                RADTODEG(pv->dir.pitch),
                RADTODEG(pv->dir.bank),
		(pv->name != NULL) ? " " : "",
		(pv->name != NULL) ? pv->name : ""
	    );
            parms_saved++;
#undef PARM_TYPE
        }
        /* Scene GPS. */
        else if(type == SAR_PARM_SCENE_GPS)
        {
#define PARM_TYPE       const sar_parm_scene_gps_struct
            PARM_TYPE *pv = (PARM_TYPE *)p;
            fprintf(
                fp,
                "scene_gps %f %f %f\n",
		pv->dms_x_offset, pv->dms_y_offset,
		pv->planet_radius
            );
            parms_saved++;
#undef PARM_TYPE
        }
        /* Scene map. */
        else if(type == SAR_PARM_SCENE_MAP)
        {
#define PARM_TYPE       const sar_parm_scene_map_struct
            PARM_TYPE *pv = (PARM_TYPE *)p;
            fprintf(
                fp,
                "scene_map %f %f%s%s\n",
                pv->width, pv->height,
                (pv->file != NULL) ? " " : "",
                (pv->file != NULL) ? pv->file : ""
            );
            parms_saved++;
#undef PARM_TYPE
        }
        /* Scene elevation. */
        else if(type == SAR_PARM_SCENE_ELEVATION)
        {
#define PARM_TYPE       const sar_parm_scene_elevation_struct
            PARM_TYPE *pv = (PARM_TYPE *)p;
            fprintf(
                fp,
                "scene_elevation %f\n",
                SFMMetersToFeet(pv->elevation)
            );
            parms_saved++;
#undef PARM_TYPE
        }
        /* Scene CANT. */
        else if(type == SAR_PARM_SCENE_CANT)
        {
#define PARM_TYPE       const sar_parm_scene_cant_struct
            PARM_TYPE *pv = (PARM_TYPE *)p;
            fprintf(
                fp,
                "scene_cant %f\n",
                RADTODEG(pv->cant)
            );
            parms_saved++;
#undef PARM_TYPE
        }
        /* Scene ground flags. */
        else if(type == SAR_PARM_SCENE_GROUND_FLAGS)
        {
#define PARM_TYPE       const sar_parm_scene_ground_flags_struct
            PARM_TYPE *pv = (PARM_TYPE *)p;
            fprintf(fp, "scene_ground_flags");
	    if(pv->flags & SAR_SCENE_BASE_FLAG_IS_WATER)
		fprintf(fp, " is_water");
	    fputc('\n', fp);
            parms_saved++;
#undef PARM_TYPE
        }
        /* Scene ground tile. */
        else if(type == SAR_PARM_SCENE_GROUND_TILE)
        {
#define PARM_TYPE       const sar_parm_scene_ground_tile_struct
            PARM_TYPE *pv = (PARM_TYPE *)p;
            fprintf(
                fp,
                "scene_ground_tile %i %i %f %.2f %.2f %.2f %.2f%s%s\n",
                pv->tile_width, pv->tile_height, pv->close_range,
		pv->color.r, pv->color.g, pv->color.b, pv->color.a,
		(pv->texture_name != NULL) ? " " : "",
		(pv->texture_name != NULL) ? pv->texture_name : ""
	    );
            parms_saved++;
#undef PARM_TYPE
        }
        /* Texture base directory. */
        else if(type == SAR_PARM_TEXTURE_BASE_DIRECTORY)
        {
#define PARM_TYPE       const sar_parm_texture_base_directory_struct
            PARM_TYPE *pv = (PARM_TYPE *)p;
            fprintf(
                fp,
                "texture_base_directory%s%s\n",
                (pv->directory != NULL) ? " " : "",
                (pv->directory != NULL) ? pv->directory : ""
            );
            parms_saved++;
#undef PARM_TYPE
        }
        /* Texture load. */
        else if(type == SAR_PARM_TEXTURE_LOAD)
        {
#define PARM_TYPE       const sar_parm_texture_load_struct
            PARM_TYPE *pv = (PARM_TYPE *)p;
            fprintf(
                fp,
                "texture_load %s %s %f\n",
                pv->name, pv->file, pv->priority
            );
            parms_saved++;
#undef PARM_TYPE
        }
        /* Mission scene file. */
        else if(type == SAR_PARM_MISSION_SCENE_FILE)
        {
#define PARM_TYPE       const sar_parm_mission_scene_file_struct
            PARM_TYPE *pv = (PARM_TYPE *)p;
            fprintf(
                fp,
                "mission_scene_file%s%s\n",
                (pv->file != NULL) ? " " : "",
                (pv->file != NULL) ? pv->file : ""
            );
            parms_saved++;
#undef PARM_TYPE
        }
        /* Mission new objective. */
        else if(type == SAR_PARM_MISSION_NEW_OBJECTIVE)
        {
#define PARM_TYPE       const sar_parm_mission_new_objective_struct
            PARM_TYPE *pv = (PARM_TYPE *)p;
	    fprintf(fp, "mission_objective_new");
	    switch(pv->objective_type)
	    {
	      case SAR_MISSION_OBJECTIVE_ARRIVE_AT:
		fprintf(fp, " arrive");
		break;
              case SAR_MISSION_OBJECTIVE_PICK_UP_ARRIVE_AT:
                fprintf(fp, " pick_up_arrive");
                break;
              case SAR_MISSION_OBJECTIVE_PICK_UP:
                fprintf(fp, " pick_up");
                break;
	    }
	    fputc('\n', fp);
            parms_saved++;
#undef PARM_TYPE
        }
        /* Mission time left. */
        else if(type == SAR_PARM_MISSION_TIME_LEFT)
        {
#define PARM_TYPE	const sar_parm_mission_time_left_struct
            PARM_TYPE *pv = (PARM_TYPE *)p;
            fprintf(
                fp,
                "mission_objective_time_left %f\n",
		pv->time_left
            );
            parms_saved++;
#undef PARM_TYPE
        }
        /* Mission begin at. */
        else if(type == SAR_PARM_MISSION_BEGIN_AT)
        {
#define PARM_TYPE       const sar_parm_mission_begin_at_struct
            PARM_TYPE *pv = (PARM_TYPE *)p;
            fprintf(
                fp,
                "mission_begin_at%s%s\n",
                (pv->name != NULL) ? " " : "",
                (pv->name != NULL) ? pv->name : ""
            );
            parms_saved++;
#undef PARM_TYPE
        }
        /* Mission begin at position. */
        else if(type == SAR_PARM_MISSION_BEGIN_AT_POS)
        {
#define PARM_TYPE       const sar_parm_mission_begin_at_pos_struct
            PARM_TYPE *pv = (PARM_TYPE *)p;
            fprintf(
                fp,
                "mission_begin_at_pos %f %f %f %f %f %f\n",
		pv->pos.x, pv->pos.y, SFMMetersToFeet(pv->pos.z),
		RADTODEG(pv->dir.heading),
                RADTODEG(pv->dir.pitch),
                RADTODEG(pv->dir.bank)
            );
            parms_saved++;
#undef PARM_TYPE
        }
        /* Mission arrive at. */
        else if(type == SAR_PARM_MISSION_ARRIVE_AT)
        {
#define PARM_TYPE	const sar_parm_mission_arrive_at_struct
            PARM_TYPE *pv = (PARM_TYPE *)p;
            fprintf(
                fp,
                "mission_objective_arrive_at%s%s\n",
                (pv->name != NULL) ? " " : "",
                (pv->name != NULL) ? pv->name : ""
            );
            parms_saved++;
#undef PARM_TYPE
        }
        /* Mission message success. */
        else if(type == SAR_PARM_MISSION_MESSAGE_SUCCESS)
        {
#define PARM_TYPE	const sar_parm_mission_message_success_struct
            PARM_TYPE *pv = (PARM_TYPE *)p;
            fprintf(
                fp,
                "mission_objective_message_success%s%s\n",
                (pv->message != NULL) ? " " : "",
                (pv->message != NULL) ? pv->message : ""
            );
            parms_saved++;
#undef PARM_TYPE
        }
        /* Mission message fail. */
        else if(type == SAR_PARM_MISSION_MESSAGE_FAIL)
        {
#define PARM_TYPE	const sar_parm_mission_message_fail_struct
            PARM_TYPE *pv = (PARM_TYPE *)p;
            fprintf(
                fp,
                "mission_objective_message_fail%s%s\n",
                (pv->message != NULL) ? " " : "",
                (pv->message != NULL) ? pv->message : ""
            );
            parms_saved++;
#undef PARM_TYPE
        }
        /* Mission humans tally. */
        else if(type == SAR_PARM_MISSION_HUMANS_TALLY)
        {
#define PARM_TYPE	const sar_parm_mission_humans_tally_struct
            PARM_TYPE *pv = (PARM_TYPE *)p;
            fprintf(
                fp,
                "mission_objective_humans_tally %i %i\n",
                pv->humans_need_rescue, pv->total_humans
            );
            parms_saved++;
#undef PARM_TYPE
        }
        /* Mission add intercept. */
        else if(type == SAR_PARM_MISSION_ADD_INTERCEPT)
        {
#define PARM_TYPE	const sar_parm_mission_add_intercept_struct
            PARM_TYPE *pv = (PARM_TYPE *)p;
	    switch(pv->ref_code)
	    {
	      case 3: case 2:	/* Arrive or begin at location. */
                fprintf(
                    fp,
                    "mission_add_intercept %i %f %f\n",
		    pv->ref_code,
		    pv->radius, pv->urgency
                );
                parms_saved++;
		break;
	      default:		/* Standard intercept way point. */
                fprintf(
                    fp,
                    "mission_add_intercept %i %f %f %f %f %f\n",
                    pv->ref_code,
		    pv->pos.x,
		    pv->pos.y,
		    pv->pos.z,		/* In meters. */
                    pv->radius, pv->urgency
                );
                parms_saved++;
                break;
	    }
#undef PARM_TYPE
        }
        /* Mission log header. */
        else if(type == SAR_PARM_MISSION_LOG_HEADER)
        {
#define PARM_TYPE	const sar_parm_mission_log_header_struct
            PARM_TYPE *pv = (PARM_TYPE *)p;
            fprintf(fp, "BeginHeader\n");
	    fprintf(fp, "    Version %i %i %i\n",
		pv->version_major, pv->version_minor, pv->version_release
	    );
            if(pv->title != NULL)
                fprintf(fp, "    Title %s\n", pv->title);
	    if(pv->scene_file != NULL)
		fprintf(fp, "    SceneFile %s\n", pv->scene_file);
	    if(pv->player_stats_file != NULL)
		fprintf(fp, "    PlayerStatsFile %s\n", pv->player_stats_file);
            fprintf(fp, "EndHeader\n");
            parms_saved++;
#undef PARM_TYPE
        }
        /* Mission log event. */
        else if(type == SAR_PARM_MISSION_LOG_EVENT)
        {
#define PARM_TYPE	const sar_parm_mission_log_event_struct
            PARM_TYPE *pv = (PARM_TYPE *)p;
            fprintf(
                fp,
                "%i %f %f %f %f%c%s\n",
		pv->event_type,
		pv->tod,
		pv->pos.x,
		pv->pos.y,
		pv->pos.z,		/* In meters. */
		(pv->message != NULL) ? ':' : '\0',
		(pv->message != NULL) ? pv->message : ""
            );
            parms_saved++;
#undef PARM_TYPE
        }
        /* New object. */
        else if(type == SAR_PARM_NEW_OBJECT)
        {
#define PARM_TYPE	const sar_parm_new_object_struct
            PARM_TYPE *pv = (PARM_TYPE *)p;
            fprintf(
                fp,
                "new_object %i\n",
		pv->object_type
	    );
            parms_saved++;
#undef PARM_TYPE
        }
        /* New helipad. */
        else if(type == SAR_PARM_NEW_HELIPAD)
        {
#define PARM_TYPE	const sar_parm_new_helipad_struct
            PARM_TYPE *pv = (PARM_TYPE *)p;
	    if(pv->ref_obj_name != NULL)
               fprintf(
                    fp,
"new_helipad %s %f %f %f %s %c %c %c %c %c %s %f %f %f %f %f %f\n",
                    pv->style,
                    pv->length, pv->width,
                    SFMMetersToFeet(pv->recession),
                    (pv->label != NULL) ? pv->label : "_",
		(pv->flags & SAR_HELIPAD_FLAG_EDGE_LIGHTING) ? 'y' : 'n',
		(pv->flags & SAR_HELIPAD_FLAG_FUEL) ? 'y' : 'n',
		(pv->flags & SAR_HELIPAD_FLAG_REPAIR) ? 'y' : 'n',
		(pv->flags & SAR_HELIPAD_FLAG_DROPOFF) ? 'y' : 'n',
		(pv->flags & SAR_HELIPAD_FLAG_RESTART_POINT) ? 'y' : 'n',
		    pv->ref_obj_name,
		    pv->ref_offset.x, pv->ref_offset.y,
		    SFMMetersToFeet(pv->ref_offset.z),
		    RADTODEG(pv->ref_dir.heading),
                    RADTODEG(pv->ref_dir.pitch),
                    RADTODEG(pv->ref_dir.bank)
                );
	    else
		fprintf(
		    fp,
"new_helipad %s %f %f %f %s %c %c %c %c %c\n",
		    pv->style,
		    pv->length, pv->width,
		    SFMMetersToFeet(pv->recession),
		    (pv->label != NULL) ? pv->label : "_",
		(pv->flags & SAR_HELIPAD_FLAG_EDGE_LIGHTING) ? 'y' : 'n',
		(pv->flags & SAR_HELIPAD_FLAG_FUEL) ? 'y' : 'n',
		(pv->flags & SAR_HELIPAD_FLAG_REPAIR) ? 'y' : 'n',
		(pv->flags & SAR_HELIPAD_FLAG_DROPOFF) ? 'y' : 'n',
                (pv->flags & SAR_HELIPAD_FLAG_RESTART_POINT) ? 'y' : 'n'
		);
            parms_saved++;
#undef PARM_TYPE
        }
        /* New runway. */
        else if(type == SAR_PARM_NEW_RUNWAY)
        {
#define PARM_TYPE	const sar_parm_new_runway_struct
            PARM_TYPE *pv = (PARM_TYPE *)p;
	    fprintf(
		fp,
		"new_runway %f %f %f %i %i %f %s %s %f %f",
		pv->range, pv->length, pv->width, pv->surface_type,
		pv->dashes, pv->edge_light_spacing,
		(pv->north_label != NULL) ? pv->north_label : "_",
		(pv->south_label != NULL) ? pv->south_label : "_",
		pv->north_displaced_threshold,
		pv->south_displaced_threshold
	    );
            if(pv->flags & SAR_RUNWAY_FLAG_THRESHOLDS)
                fprintf(fp, " thresholds");
            if(pv->flags & SAR_RUNWAY_FLAG_BORDERS)
                fprintf(fp, " borders");
            if(pv->flags & SAR_RUNWAY_FLAG_TD_MARKERS)
                fprintf(fp, " td_markers");
            if(pv->flags & SAR_RUNWAY_FLAG_MIDWAY_MARKERS)
                fprintf(fp, " midway_markers");
            if(pv->flags & SAR_RUNWAY_FLAG_NORTH_GS)
                fprintf(fp, " north_gs");
            if(pv->flags & SAR_RUNWAY_FLAG_SOUTH_GS)
                fprintf(fp, " south_gs");
            fputc('\n', fp);
            parms_saved++;
#undef PARM_TYPE
        }
        /* New human. */
        else if(type == SAR_PARM_NEW_HUMAN)
        {
#define PARM_TYPE       const sar_parm_new_human_struct
            PARM_TYPE *pv = (PARM_TYPE *)p;
            fprintf(
                fp,
                "new_human %s",
		pv->type_name
	    );
	    if(pv->flags & SAR_HUMAN_FLAG_NEED_RESCUE)
		fprintf(fp, " need_rescue");
            if(pv->flags & SAR_HUMAN_FLAG_SIT)
                fprintf(fp, " sit");
            if(pv->flags & SAR_HUMAN_FLAG_SIT_UP)
                fprintf(fp, " sit_up");
            if(pv->flags & SAR_HUMAN_FLAG_SIT_DOWN)
                fprintf(fp, " sit_down");
            if(pv->flags & SAR_HUMAN_FLAG_LYING)
                fprintf(fp, " lying");
            if(pv->flags & SAR_HUMAN_FLAG_ALERT)
                fprintf(fp, " alert");
            if(pv->flags & SAR_HUMAN_FLAG_AWARE)
                fprintf(fp, " aware");
            if(pv->flags & SAR_HUMAN_FLAG_IN_WATER)
                fprintf(fp, " in_water");
	    fputc('\n', fp);
            parms_saved++;
#undef PARM_TYPE
        }
        /* New fire. */
        else if(type == SAR_PARM_NEW_FIRE)
        {
#define PARM_TYPE	const sar_parm_new_fire_struct
            PARM_TYPE *pv = (PARM_TYPE *)p;
            fprintf(
                fp,
                "new_fire %f %f\n",
		pv->radius, SFMMetersToFeet(pv->height)
            );
            parms_saved++;
#undef PARM_TYPE
        }
        /* New smoke. */
        else if(type == SAR_PARM_NEW_SMOKE)
        {
#define PARM_TYPE	const sar_parm_new_smoke_struct
            PARM_TYPE *pv = (PARM_TYPE *)p;
            fprintf(
                fp,
                "new_smoke %f %f %f %f %f %f %i %ld %i %i\n",
		pv->offset.x,
		pv->offset.y,
		pv->offset.z,		/* In meters. */
		pv->radius_start,
		pv->radius_max,
		pv->radius_rate,
		pv->hide_at_max,
		pv->respawn_int,
		pv->total_units,
		pv->color_code
	    );
            parms_saved++;
#undef PARM_TYPE
        }
        /* New premodeled object. */
        else if(type == SAR_PARM_NEW_PREMODELED)
        {
#define PARM_TYPE	const sar_parm_new_premodeled_struct
	    int i;
            PARM_TYPE *pv = (PARM_TYPE *)p;
            fprintf(
                fp,
                "new_premodeled %s",
		pv->model_type
            );
	    for(i = 0; i < pv->argc; i++)
		fprintf(fp, " %s", pv->argv[i]);
	    fputc('\n', fp);
            parms_saved++;
#undef PARM_TYPE
        }
        /* Select object by name. */
        else if(type == SAR_PARM_SELECT_OBJECT_BY_NAME)
        {
#define PARM_TYPE	const sar_parm_select_object_by_name_struct
            PARM_TYPE *pv = (PARM_TYPE *)p;
	    if(pv->name != NULL)
	    {
		fprintf(
		    fp,
		    "select_object_by_name %s\n",
		    pv->name
		);
		parms_saved++;
	    }
#undef PARM_TYPE
        }
        /* Model file. */
        else if(type == SAR_PARM_MODEL_FILE)
        {
#define PARM_TYPE	const sar_parm_model_file_struct
            PARM_TYPE *pv = (PARM_TYPE *)p;
            if(pv->file != NULL)
            {
                fprintf(
                    fp,
                    "model_file %s\n",
                    pv->file
                );
                parms_saved++;
            }
#undef PARM_TYPE
        }
	/* Range. */
        else if(type == SAR_PARM_RANGE)
        {
#define PARM_TYPE	const sar_parm_range_struct
            PARM_TYPE *pv = (PARM_TYPE *)p;
	    fprintf(
		fp,
		"range %f\n",
		pv->range
	    );
	    parms_saved++;
#undef PARM_TYPE
        }
        /* Range far. */
        else if(type == SAR_PARM_RANGE_FAR)
        {
#define PARM_TYPE	const sar_parm_range_far_struct
            PARM_TYPE *pv = (PARM_TYPE *)p;
            fprintf(
                fp,
                "range_far %f\n",
                pv->range_far
            );
            parms_saved++;
#undef PARM_TYPE
        }
        /* Translate. */
        else if(type == SAR_PARM_TRANSLATE)
        {
#define PARM_TYPE	const sar_parm_translate_struct
            PARM_TYPE *pv = (PARM_TYPE *)p;
            fprintf(
                fp,
                "translate %f %f %f\n",
                pv->translate.x,
		pv->translate.y,
		SFMMetersToFeet(pv->translate.z)
            );
            parms_saved++;
#undef PARM_TYPE
        }
        /* Translate random. */
        else if(type == SAR_PARM_TRANSLATE_RANDOM)
        {
#define PARM_TYPE	const sar_parm_translate_random_struct
            PARM_TYPE *pv = (PARM_TYPE *)p;
            fprintf(
                fp,
                "translate_random %f %f\n",
                pv->radius_bound,
                pv->z_bound
            );
            parms_saved++;
#undef PARM_TYPE
        }
        /* Rotate. */
        else if(type == SAR_PARM_ROTATE)
        {
#define PARM_TYPE	const sar_parm_rotate_struct
            PARM_TYPE *pv = (PARM_TYPE *)p;
            fprintf(
                fp,
                "rotate %f %f %f\n",
                RADTODEG(pv->rotate.heading),
                RADTODEG(pv->rotate.pitch),
                RADTODEG(pv->rotate.bank)
            );
            parms_saved++;
#undef PARM_TYPE
        }
        /* No depth test. */
        else if(type == SAR_PARM_NO_DEPTH_TEST)
        {
#define PARM_TYPE	const sar_parm_no_depth_test_struct
/*	    PARM_TYPE *pv = (PARM_TYPE *)p; */
            fprintf(
                fp,
                "no_depth_test\n"
            );
            parms_saved++;
#undef PARM_TYPE
        }
        /* Polygon offset. */
        else if(type == SAR_PARM_POLYGON_OFFSET)
        {
#define PARM_TYPE	const sar_parm_polygon_offset_struct
            PARM_TYPE *pv = (PARM_TYPE *)p;
            fprintf(fp, "polygon_offset");
	    if(pv->flags & SAR_OBJ_FLAG_POLYGON_OFFSET_REVERSE)
		fprintf(fp, " reverse");
            if(pv->flags & SAR_OBJ_FLAG_POLYGON_OFFSET_WRITE_DEPTH)
                fprintf(fp, " write_depth");
	    fputc('\n', fp);
            parms_saved++;
#undef PARM_TYPE
        }
	/* Countact bounds spherical. */
        else if(type == SAR_PARM_CONTACT_BOUNDS_SPHERICAL)
        {
#define PARM_TYPE	const sar_parm_contact_bounds_spherical_struct
            PARM_TYPE *pv = (PARM_TYPE *)p;
	    fprintf(
		fp,
		"contact_spherical %f\n",
		pv->radius
	    );
            parms_saved++;
#undef PARM_TYPE
        }
        /* Countact bounds cylendrical. */
        else if(type == SAR_PARM_CONTACT_BOUNDS_CYLENDRICAL)
        {
#define PARM_TYPE	const sar_parm_contact_bounds_cylendrical_struct
            PARM_TYPE *pv = (PARM_TYPE *)p;
            fprintf(
                fp,
                "contact_cylendrical %f %f %f\n",
                pv->radius,
		pv->height_min, pv->height_max	/* In meters. */
            );
            parms_saved++;
#undef PARM_TYPE
        }
        /* Countact bounds rectangular. */
        else if(type == SAR_PARM_CONTACT_BOUNDS_RECTANGULAR)
        {
#define PARM_TYPE	const sar_parm_contact_bounds_rectangular_struct
            PARM_TYPE *pv = (PARM_TYPE *)p;
            fprintf(
                fp,
                "contact_rectangular %f %f %f %f %f %f\n",
                pv->x_min, pv->x_max,
                pv->y_min, pv->y_max,
                pv->z_min, pv->z_max	/* In meters. */
            );
            parms_saved++;
#undef PARM_TYPE
        }
        /* Ground elevation. */
        else if(type == SAR_PARM_GROUND_ELEVATION)
        {
#define PARM_TYPE	const sar_parm_ground_elevation_struct
            PARM_TYPE *pv = (PARM_TYPE *)p;
            fprintf(
                fp,
                "ground_elevation %f\n",
                SFMMetersToFeet(pv->elevation)
            );
            parms_saved++;
#undef PARM_TYPE
        }
        /* Object name. */
        else if(type == SAR_PARM_OBJECT_NAME)
        {
#define PARM_TYPE	const sar_parm_object_name_struct
            PARM_TYPE *pv = (PARM_TYPE *)p;
            fprintf(
                fp,
                "object_name%s%s\n",
		(pv->name != NULL) ? " " : "",
		(pv->name != NULL) ? pv->name : ""
            );
            parms_saved++;
#undef PARM_TYPE
        }
        /* Object map description. */
        else if(type == SAR_PARM_OBJECT_MAP_DESCRIPTION)
        {
#define PARM_TYPE	const sar_parm_object_map_description_struct
            PARM_TYPE *pv = (PARM_TYPE *)p;
            fprintf(
                fp,
                "object_map_description%s%s\n",
                (pv->description != NULL) ? " " : "",
                (pv->description != NULL) ? pv->description : ""
            );
            parms_saved++;
#undef PARM_TYPE
        }
        /* Object fuel. */
        else if(type == SAR_PARM_FUEL)
        {
#define PARM_TYPE       const sar_parm_fuel_struct
            PARM_TYPE *pv = (PARM_TYPE *)p;
            fprintf(
                fp,
                "fuel %f %f\n",
		pv->fuel, pv->fuel_max
            );
            parms_saved++;
#undef PARM_TYPE
        }
        /* Object hit points. */
        else if(type == SAR_PARM_HITPOINTS)
        {
#define PARM_TYPE	const sar_parm_hitpoints_struct
            PARM_TYPE *pv = (PARM_TYPE *)p;
            fprintf(
                fp,
                "hitpoints %f %f\n",
                pv->hitpoints, pv->hitpoints_max
            );
            parms_saved++;
#undef PARM_TYPE
        }
        /* Object engine state. */
        else if(type == SAR_PARM_ENGINE_STATE)
        {
#define PARM_TYPE	const sar_parm_engine_state_struct
            PARM_TYPE *pv = (PARM_TYPE *)p;
            fprintf(
                fp,
                "engine_state %i\n",
                pv->state
            );
            parms_saved++;
#undef PARM_TYPE
        }
        /* Object passengers. */
        else if(type == SAR_PARM_PASSENGERS)
        {
#define PARM_TYPE	const sar_parm_passengers_struct
            PARM_TYPE *pv = (PARM_TYPE *)p;
            fprintf(
                fp,
                "passengers %i %i\n",
                pv->passengers, pv->passengers_max
            );
            parms_saved++;
#undef PARM_TYPE
        }
        /* Runway approach lighting north. */
        else if(type == SAR_PARM_RUNWAY_APPROACH_LIGHTING_NORTH)
        {
#define PARM_TYPE       const sar_parm_runway_approach_lighting_north_struct
            PARM_TYPE *pv = (PARM_TYPE *)p;
            fprintf(
                fp,
                "runway_approach_lighting_north"
            );
            if(pv->flags & SAR_RUNWAY_APPROACH_LIGHTING_END)
                fprintf(fp, " end");
            if(pv->flags & SAR_RUNWAY_APPROACH_LIGHTING_TRACER)
                fprintf(fp, " tracer");
            if(pv->flags & SAR_RUNWAY_APPROACH_LIGHTING_ALIGN)
                fprintf(fp, " align");
            if(pv->flags & SAR_RUNWAY_APPROACH_LIGHTING_ILS_GLIDE)
                fprintf(fp, " ils_glide");
            fputc('\n', fp);
            parms_saved++;
#undef PARM_TYPE
        }
        /* Runway approach lighting south. */
        else if(type == SAR_PARM_RUNWAY_APPROACH_LIGHTING_SOUTH)
        {
#define PARM_TYPE       const sar_parm_runway_approach_lighting_south_struct
            PARM_TYPE *pv = (PARM_TYPE *)p;
            fprintf(
                fp,
                "runway_approach_lighting_south"
            );
            if(pv->flags & SAR_RUNWAY_APPROACH_LIGHTING_END)
                fprintf(fp, " end");
            if(pv->flags & SAR_RUNWAY_APPROACH_LIGHTING_TRACER)
                fprintf(fp, " tracer");
            if(pv->flags & SAR_RUNWAY_APPROACH_LIGHTING_ALIGN)
                fprintf(fp, " align");
            if(pv->flags & SAR_RUNWAY_APPROACH_LIGHTING_ILS_GLIDE)
                fprintf(fp, " ils_glide");
            fputc('\n', fp);
            parms_saved++;
#undef PARM_TYPE
        }
        /* Human message enter. */
        else if(type == SAR_PARM_HUMAN_MESSAGE_ENTER)
        {
#define PARM_TYPE	const sar_parm_human_message_enter_struct
            PARM_TYPE *pv = (PARM_TYPE *)p;
            fprintf(
                fp,
                "set_human_message_enter%s%s\n",
                (pv->message != NULL) ? " " : "",
                (pv->message != NULL) ? pv->message : ""
            );
            parms_saved++;
#undef PARM_TYPE
        }
        /* Human reference. */
        else if(type == SAR_PARM_HUMAN_REFERENCE)
        {
#define PARM_TYPE	const sar_parm_human_reference_struct
            PARM_TYPE *pv = (PARM_TYPE *)p;
	    if(pv->reference_name != NULL)
	    {
		fprintf(fp, "human_reference %s", pv->reference_name);
		if(pv->flags & SAR_HUMAN_FLAG_RUN_TOWARDS)
		    fprintf(fp, " run_towards");
		if(pv->flags & SAR_HUMAN_FLAG_RUN_AWAY)
		    fprintf(fp, " run_away");
		fputc('\n', fp);
	    }
            parms_saved++;
#undef PARM_TYPE
        }
        /* Welcome message. */
        else if (type == SAR_PARM_WELCOME_MESSAGE)
        {
#define PARM_TYPE      const sar_parm_welcome_message_struct
            PARM_TYPE *pv = (PARM_TYPE *)p;
            if (pv->message != NULL)
            {
                fprintf(fp, "set_welcome_message %s", pv->message);
                fputc('\n',fp);
            }
            parms_saved++;
#undef PARM_TYPE
        }
            

	return(parms_saved);
}

/*
 *	Saves parameters to file regardless of file format.
 */
static int SARParmSaveToFileAny(
	const char *filename, FILE *fp,
	void **parm, int total_parms,
        void *client_data,
        int (*progress_func)(void *, long, long)
)
{
	int i;
	const void *p;

	/* Iterate through all parameters. */
	for(i = 0; i < total_parms; i++)
	{
	    p = parm[i];
	    if(p == NULL)
		continue;

            /* Call progress callback. */
            if(progress_func != NULL)
            {
                if(progress_func(client_data, i + 1, total_parms))
                    break;
            }

            /* Save this parameter. */
            SARParmSaveToFileAnyIterate(filename, fp, p);
	}

	return(0);
}

/*
 *	Saves the list of parameters to file.
 *
 *      The given file_format is a hint to the file's format.
 *
 *      Returns non-zero on error.
 */
int SARParmSaveToFile(
        const char *filename, int file_format,
        void **parm, int total_parms,
        void *client_data,
        int (*progress_func)(void *, long, long)
)
{
        int status = -1;
        FILE *fp;


        if(filename == NULL)
            return(status);

        /* Open file for writing. */
        fp = FOpen(filename, "wb");
        if(fp == NULL)
            return(status);

        /* Save by file format type. */
	if(1)
        {
            status = SARParmSaveToFileAny(
                filename, fp,
                parm, total_parms,
                client_data, progress_func
            );
        }

        /* Close file. */
        FClose(fp);

        return(status);
}
