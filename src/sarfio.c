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

#include "../include/string.h"

#include "obj.h"
#include "sarfio.h"
#include "config.h"


void *SARParmNew(int type);
void *SARParmNewAppend(int type, void ***ptr, int *total);
void SARParmDelete(void *p);
void SARParmDeleteAll(void ***ptr, int *total);


#ifndef SAR_COMMENT_CHAR
# define SAR_COMMENT_CHAR	'#'
#endif

#define ATOI(s)		(((s) != NULL) ? atoi(s) : 0)
#define ATOL(s)		(((s) != NULL) ? atol(s) : 0)
#define ATOF(s)		(((s) != NULL) ? atof(s) : 0.0f)
#define STRDUP(s)	(((s) != NULL) ? strdup(s) : NULL)

#define MAX(a,b)        (((a) > (b)) ? (a) : (b))
#define MIN(a,b)        (((a) < (b)) ? (a) : (b))
#define CLIP(a,l,h)     (MIN(MAX((a),(l)),(h)))
   
#define ISCOMMENT(c)    ((c) == SAR_COMMENT_CHAR)
#define ISCR(c)         (((c) == '\n') || ((c) == '\r'))

#define DEGTORAD(d)     ((d) * PI / 180)
#define RADTODEG(r)     ((r) * 180 / PI)


/*
 *	Allocates a new parameter structure of the given type or NULL
 *	on error.
 */
void *SARParmNew(int type)
{
	void *p = NULL;
	int len = 0;


	switch(type)
	{
	  case SAR_PARM_VERSION:
	    len = sizeof(sar_parm_version_struct);
	    break;
	  case SAR_PARM_NAME:
	    len = sizeof(sar_parm_name_struct);
	    break;
          case SAR_PARM_DESCRIPTION:
            len = sizeof(sar_parm_description_struct);
            break;

          case SAR_PARM_PLAYER_MODEL_FILE:
            len = sizeof(sar_parm_player_model_file_struct);
            break;
	  case SAR_PARM_WEATHER:
	    len = sizeof(sar_parm_weather_struct);
	    break;
	  case SAR_PARM_WIND:
	    len = sizeof(sar_parm_wind_struct);
	    break;
	  case SAR_PARM_TIME_OF_DAY:
	    len = sizeof(sar_parm_time_of_day_struct);
            break;
          case SAR_PARM_REGISTER_LOCATION:
            len = sizeof(sar_parm_register_location_struct);
            break;

          case SAR_PARM_SCENE_GPS:
            len = sizeof(sar_parm_scene_gps_struct);
            break;
          case SAR_PARM_SCENE_MAP:
            len = sizeof(sar_parm_scene_map_struct);
            break;
          case SAR_PARM_SCENE_ELEVATION:
            len = sizeof(sar_parm_scene_elevation_struct);
            break;
          case SAR_PARM_SCENE_CANT:
            len = sizeof(sar_parm_scene_cant_struct);
            break;
          case SAR_PARM_SCENE_GROUND_FLAGS:
            len = sizeof(sar_parm_scene_ground_flags_struct);
            break;
          case SAR_PARM_SCENE_GROUND_TILE:
            len = sizeof(sar_parm_scene_ground_tile_struct);
            break;

          case SAR_PARM_TEXTURE_BASE_DIRECTORY:
            len = sizeof(sar_parm_texture_base_directory_struct);
            break;
          case SAR_PARM_TEXTURE_LOAD:
            len = sizeof(sar_parm_texture_load_struct);
            break;

          case SAR_PARM_MISSION_SCENE_FILE:
            len = sizeof(sar_parm_mission_scene_file_struct);
            break;
          case SAR_PARM_MISSION_NEW_OBJECTIVE:
            len = sizeof(sar_parm_mission_new_objective_struct);
            break;
          case SAR_PARM_MISSION_TIME_LEFT:
            len = sizeof(sar_parm_mission_time_left_struct);
            break;
          case SAR_PARM_MISSION_BEGIN_AT:
            len = sizeof(sar_parm_mission_begin_at_struct);
            break;
	  case SAR_PARM_MISSION_BEGIN_AT_POS:
            len = sizeof(sar_parm_mission_begin_at_pos_struct);
            break;
          case SAR_PARM_MISSION_ARRIVE_AT:
            len = sizeof(sar_parm_mission_arrive_at_struct);
            break;
          case SAR_PARM_MISSION_MESSAGE_SUCCESS:
            len = sizeof(sar_parm_mission_message_success_struct);
            break;
          case SAR_PARM_MISSION_MESSAGE_FAIL:
            len = sizeof(sar_parm_mission_message_fail_struct);
            break;
          case SAR_PARM_MISSION_HUMANS_TALLY:
            len = sizeof(sar_parm_mission_humans_tally_struct);
            break;
          case SAR_PARM_MISSION_ADD_INTERCEPT:
            len = sizeof(sar_parm_mission_add_intercept_struct);
            break;

	  case SAR_PARM_MISSION_LOG_HEADER:
            len = sizeof(sar_parm_mission_log_header_struct);
            break;
	  case SAR_PARM_MISSION_LOG_EVENT:
            len = sizeof(sar_parm_mission_log_event_struct);
            break;

          case SAR_PARM_NEW_OBJECT:
            len = sizeof(sar_parm_new_object_struct);
            break;
          case SAR_PARM_NEW_HELIPAD:
            len = sizeof(sar_parm_new_helipad_struct);
            break;
          case SAR_PARM_NEW_RUNWAY:
            len = sizeof(sar_parm_new_runway_struct);
            break;
          case SAR_PARM_NEW_HUMAN:
            len = sizeof(sar_parm_new_human_struct);
            break;
	  case SAR_PARM_NEW_FIRE:
	    len = sizeof(sar_parm_new_fire_struct);
	    break;
	  case SAR_PARM_NEW_SMOKE:
	    len = sizeof(sar_parm_new_smoke_struct);
            break;
          case SAR_PARM_NEW_PREMODELED:
            len = sizeof(sar_parm_new_premodeled_struct);
            break;

	  case SAR_PARM_SELECT_OBJECT_BY_NAME:
	    len = sizeof(sar_parm_select_object_by_name_struct);
	    break;

          case SAR_PARM_MODEL_FILE:
            len = sizeof(sar_parm_model_file_struct);
            break;
          case SAR_PARM_RANGE:
            len = sizeof(sar_parm_range_struct);
            break;
	  case SAR_PARM_RANGE_FAR:
	    len = sizeof(sar_parm_range_far_struct);
            break;
          case SAR_PARM_TRANSLATE:
            len = sizeof(sar_parm_translate_struct);
            break;
          case SAR_PARM_TRANSLATE_RANDOM:
            len = sizeof(sar_parm_translate_random_struct);
            break;
          case SAR_PARM_ROTATE:
            len = sizeof(sar_parm_rotate_struct);
            break;

          case SAR_PARM_NO_DEPTH_TEST:
            len = sizeof(sar_parm_no_depth_test_struct);
            break;
          case SAR_PARM_POLYGON_OFFSET:
            len = sizeof(sar_parm_polygon_offset_struct);
            break;

          case SAR_PARM_CONTACT_BOUNDS_SPHERICAL:
            len = sizeof(sar_parm_contact_bounds_spherical_struct);
            break;
          case SAR_PARM_CONTACT_BOUNDS_CYLENDRICAL:
            len = sizeof(sar_parm_contact_bounds_cylendrical_struct);
            break;
          case SAR_PARM_CONTACT_BOUNDS_RECTANGULAR:
            len = sizeof(sar_parm_contact_bounds_rectangular_struct);
            break;

          case SAR_PARM_GROUND_ELEVATION:
            len = sizeof(sar_parm_ground_elevation_struct);
            break;

          case SAR_PARM_OBJECT_NAME:
            len = sizeof(sar_parm_object_name_struct);
            break;
          case SAR_PARM_OBJECT_MAP_DESCRIPTION:
            len = sizeof(sar_parm_object_map_description_struct);
            break;

          case SAR_PARM_FUEL:
            len = sizeof(sar_parm_fuel_struct);
            break;
          case SAR_PARM_HITPOINTS:
            len = sizeof(sar_parm_hitpoints_struct);
            break;
          case SAR_PARM_ENGINE_STATE:
            len = sizeof(sar_parm_engine_state_struct);
            break;
          case SAR_PARM_PASSENGERS:
            len = sizeof(sar_parm_passengers_struct);
            break;

          case SAR_PARM_RUNWAY_APPROACH_LIGHTING_NORTH:
            len = sizeof(sar_parm_runway_approach_lighting_north_struct);
            break;
          case SAR_PARM_RUNWAY_APPROACH_LIGHTING_SOUTH:
            len = sizeof(sar_parm_runway_approach_lighting_south_struct);
            break;

          case SAR_PARM_HUMAN_MESSAGE_ENTER:
            len = sizeof(sar_parm_human_message_enter_struct);
            break;
            case SAR_PARM_HUMAN_REFERENCE:
                len = sizeof(sar_parm_human_reference_struct);
                break;
            case SAR_PARM_WELCOME_MESSAGE:
                len = sizeof(sar_parm_welcome_message_struct);
                break;

/* Add support for other parm structure types here */

	  default:
	    fprintf(
		stderr,
"SARParmNew(): Unsupported object type `%i'.\n",
		type
	    );
	    break;
	}

	/* If length is positive it means a valid structure was
	 * matched above
	 */
	if(len > 0)
	{
	    p = calloc(1, len);
	    if(p != NULL)
		*(int *)p = type;
	}

	return(p);
}

/*
 *	Creates a new prarameter of the given type and appends it to the
 *	given list of parameters.
 */
void *SARParmNewAppend(int type, void ***ptr, int *total)
{
	int i;
	void *p;

	if((ptr == NULL) || (total == NULL))
	    return(NULL);

	i = MAX(*total, 0);
	*total = i + 1;
	*ptr = (void **)realloc(*ptr, (*total) * sizeof(void *));
	if(*ptr == NULL)
	{
	    *total = 0;
	    return(NULL);
	}

	(*ptr)[i] = p = SARParmNew(type);

	return(p);
}

/*
 *	Deletes the parameter.
 */
void SARParmDelete(void *p)
{
	int type;
	sar_parm_version_struct *p_version;
	sar_parm_name_struct *p_name;
	sar_parm_description_struct *p_description;
	sar_parm_player_model_file_struct *p_player_model_file;
	sar_parm_weather_struct *p_weather;
	sar_parm_wind_struct *p_wind;
	sar_parm_register_location_struct *p_register_location;
	sar_parm_scene_map_struct *p_scene_map;
	sar_parm_scene_ground_tile_struct *p_scene_ground_tile;
	sar_parm_texture_base_directory_struct *p_texture_base_directory;
	sar_parm_texture_load_struct *p_texture_load;
	sar_parm_mission_scene_file_struct *p_mission_scene_file;
	sar_parm_mission_new_objective_struct *p_mission_new_objective;
	sar_parm_mission_begin_at_struct *p_mission_begin_at;
	sar_parm_mission_begin_at_pos_struct *p_mission_begin_at_pos;
	sar_parm_mission_arrive_at_struct *p_mission_arrive_at;
	sar_parm_mission_message_success_struct *p_mission_message_success;
	sar_parm_mission_message_fail_struct *p_mission_message_fail;
	sar_parm_mission_add_intercept_struct *p_mission_add_intercept;
	sar_parm_mission_log_header_struct *p_mission_log_header;
	sar_parm_mission_log_event_struct *p_mission_log_event;
	sar_parm_new_helipad_struct *p_new_helipad;
	sar_parm_new_runway_struct *p_new_runway;
	sar_parm_new_human_struct *p_new_human;
	sar_parm_new_fire_struct *p_new_fire;
	sar_parm_new_smoke_struct *p_new_smoke;
	sar_parm_new_premodeled_struct *p_new_premodeled;
	sar_parm_select_object_by_name_struct *p_select_object_by_name;
	sar_parm_model_file_struct *p_model_file;
	sar_parm_object_name_struct *p_object_name;
	sar_parm_object_map_description_struct *p_object_map_description;
	sar_parm_runway_approach_lighting_north_struct *p_runway_approach_lighting_north;
	sar_parm_runway_approach_lighting_south_struct *p_runway_approach_lighting_south;
	sar_parm_human_message_enter_struct *p_human_message_enter;
	sar_parm_human_reference_struct *p_human_reference;


	if(p == NULL)
	    return;

	type = *(int *)p;
	switch(type)
	{
	  case SAR_PARM_VERSION:
	    p_version = (sar_parm_version_struct *)p;
	    free(p_version->copyright);
	    break;
	  case SAR_PARM_NAME:
	    p_name = (sar_parm_name_struct *)p;
	    free(p_name->name);
	    break;
	  case SAR_PARM_DESCRIPTION:
	    p_description = (sar_parm_description_struct *)p;
	    free(p_description->description);
	    break;

	  case SAR_PARM_PLAYER_MODEL_FILE:
	    p_player_model_file = (sar_parm_player_model_file_struct *)p;
	    free(p_player_model_file->file);
	    break;
	  case SAR_PARM_WEATHER:
	    p_weather = (sar_parm_weather_struct *)p;
	    free(p_weather->weather_preset_name);
	    break;
	  case SAR_PARM_WIND:
	    p_wind = (sar_parm_wind_struct *)p;
	    break;
	  case SAR_PARM_REGISTER_LOCATION:
	    p_register_location = (sar_parm_register_location_struct *)p;
	    free(p_register_location->name);
	    break;

	  case SAR_PARM_SCENE_MAP:
	    p_scene_map = (sar_parm_scene_map_struct *)p;
	    free(p_scene_map->file);
	    break;
	  case SAR_PARM_SCENE_GROUND_TILE:
	    p_scene_ground_tile = (sar_parm_scene_ground_tile_struct *)p;
	    free(p_scene_ground_tile->texture_name);
	    break;

	  case SAR_PARM_TEXTURE_BASE_DIRECTORY:
	    p_texture_base_directory = (sar_parm_texture_base_directory_struct *)p;
	    free(p_texture_base_directory->directory);
	    break;
	  case SAR_PARM_TEXTURE_LOAD:
	    p_texture_load = (sar_parm_texture_load_struct *)p;
	    free(p_texture_load->name);
	    free(p_texture_load->file);
	    break;

	  case SAR_PARM_MISSION_SCENE_FILE:
	    p_mission_scene_file = (sar_parm_mission_scene_file_struct *)p;
	    free(p_mission_scene_file->file);
	    break;
	  case SAR_PARM_MISSION_NEW_OBJECTIVE:
	    p_mission_new_objective = (sar_parm_mission_new_objective_struct *)p;
	    break;
	  case SAR_PARM_MISSION_BEGIN_AT:
	    p_mission_begin_at = (sar_parm_mission_begin_at_struct *)p;
	    free(p_mission_begin_at->name);
	    break;
          case SAR_PARM_MISSION_BEGIN_AT_POS:
            p_mission_begin_at_pos = (sar_parm_mission_begin_at_pos_struct *)p;
            break;
          case SAR_PARM_MISSION_ARRIVE_AT:
            p_mission_arrive_at = (sar_parm_mission_arrive_at_struct *)p;
            free(p_mission_arrive_at->name);
            break;
	  case SAR_PARM_MISSION_MESSAGE_SUCCESS:
	    p_mission_message_success = (sar_parm_mission_message_success_struct *)p;
	    free(p_mission_message_success->message);
	    break;
	  case SAR_PARM_MISSION_MESSAGE_FAIL:
	    p_mission_message_fail = (sar_parm_mission_message_fail_struct *)p;
	    free(p_mission_message_fail->message);
	    break;
	  case SAR_PARM_MISSION_ADD_INTERCEPT:
	    p_mission_add_intercept = (sar_parm_mission_add_intercept_struct *)p;
	    free(p_mission_add_intercept->name);
	    break;
	  case SAR_PARM_MISSION_LOG_HEADER:
	    p_mission_log_header = (sar_parm_mission_log_header_struct *)p;
	    free(p_mission_log_header->title);
	    free(p_mission_log_header->scene_file);
	    free(p_mission_log_header->player_stats_file);
	    break;
	  case SAR_PARM_MISSION_LOG_EVENT:
	    p_mission_log_event = (sar_parm_mission_log_event_struct *)p;
	    free(p_mission_log_event->message);
	    break;

	  case SAR_PARM_NEW_HELIPAD:
	    p_new_helipad = (sar_parm_new_helipad_struct *)p;
	    free(p_new_helipad->style);
	    free(p_new_helipad->label);
	    free(p_new_helipad->ref_obj_name);
	    break;
	  case SAR_PARM_NEW_RUNWAY:
	    p_new_runway = (sar_parm_new_runway_struct *)p;
	    free(p_new_runway->north_label);
	    free(p_new_runway->south_label);
	    break;
	  case SAR_PARM_NEW_HUMAN:
	    p_new_human = (sar_parm_new_human_struct *)p;
	    free(p_new_human->type_name);
	    break;
	  case SAR_PARM_NEW_FIRE:
	    p_new_fire = (sar_parm_new_fire_struct *)p;
	    break;
	  case SAR_PARM_NEW_SMOKE:
	    p_new_smoke = (sar_parm_new_smoke_struct *)p;
	    break;
	  case SAR_PARM_NEW_PREMODELED:
	    p_new_premodeled = (sar_parm_new_premodeled_struct *)p;
	    free(p_new_premodeled->model_type);
	    strlistfree(p_new_premodeled->argv, p_new_premodeled->argc);
	    break;

	  case SAR_PARM_SELECT_OBJECT_BY_NAME:
	    p_select_object_by_name = (sar_parm_select_object_by_name_struct *)p;
	    free(p_select_object_by_name->name);
	    break;

	  case SAR_PARM_MODEL_FILE:
	    p_model_file = (sar_parm_model_file_struct *)p;
	    free(p_model_file->file);
	    break;
	  case SAR_PARM_OBJECT_NAME:
	    p_object_name = (sar_parm_object_name_struct *)p;
	    free(p_object_name->name);
	    break;
	  case SAR_PARM_OBJECT_MAP_DESCRIPTION:
	    p_object_map_description = (sar_parm_object_map_description_struct *)p;
	    free(p_object_map_description->description);
	    break;

	  case SAR_PARM_RUNWAY_APPROACH_LIGHTING_NORTH:
	    p_runway_approach_lighting_north =
		(sar_parm_runway_approach_lighting_north_struct *)p;
	    break;
          case SAR_PARM_RUNWAY_APPROACH_LIGHTING_SOUTH:
            p_runway_approach_lighting_south =
                (sar_parm_runway_approach_lighting_south_struct *)p;
            break;

	  case SAR_PARM_HUMAN_MESSAGE_ENTER:
	    p_human_message_enter = (sar_parm_human_message_enter_struct *)p;
	    free(p_human_message_enter->message);
	    break;
	  case SAR_PARM_HUMAN_REFERENCE:
	    p_human_reference = (sar_parm_human_reference_struct *)p;
	    free(p_human_reference->reference_name);
	    break;

/* Add support for parm structures that have allocated members here.
 * All parm structures without allocated members should be ignored.
 */
	}

	free(p);
}

/*
 *	Deletes the list of parameters.
 */
void SARParmDeleteAll(void ***ptr, int *total)
{
	int i;

	if((ptr == NULL) || (total == NULL))
	    return;

	for(i = 0; i < *total; i++)
	    SARParmDelete((*ptr)[i]);

	if(*ptr != NULL)
	{
	    free(*ptr);
	    *ptr = NULL;
	}
	*total = 0;
}
