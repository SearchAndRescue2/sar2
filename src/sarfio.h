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

/*
 *                          SAR File Format IO
 *
 *	For mission and scenery files, loads data from those files into
 *	an array of structures for easy manipulation.
 */

#ifndef SARFIO_H
#define SARFIO_H

#include <sys/types.h>
#include "obj.h"


/*
 *	File format codes:
 */
#define SAR_FILE_FORMAT_SCENE		1
#define SAR_FILE_FORMAT_MISSION		2
#define SAR_FILE_FORMAT_MISSION_LOG	4

/*
 *	File format parameter and value type codes:
 */
#define SAR_PARM_VERSION		10	/* Of respective file */
#define SAR_PARM_NAME			11	/* Of respective file */
#define SAR_PARM_DESCRIPTION		12	/* Of respective file */
#define SAR_PARM_PLAYER_MODEL_FILE	13
#define SAR_PARM_WEATHER		14
#define SAR_PARM_TIME_OF_DAY		15

#define SAR_PARM_REGISTER_LOCATION	20
#define SAR_PARM_SCENE_GPS		21
#define SAR_PARM_SCENE_MAP              22
#define SAR_PARM_SCENE_ELEVATION	23
#define SAR_PARM_SCENE_CANT		24
#define SAR_PARM_SCENE_GROUND_FLAGS	25
#define SAR_PARM_SCENE_GROUND_TILE	26

#define SAR_PARM_TEXTURE_BASE_DIRECTORY 30
#define SAR_PARM_TEXTURE_LOAD		31

#define SAR_PARM_MISSION_SCENE_FILE	40
#define SAR_PARM_MISSION_NEW_OBJECTIVE	41
#define SAR_PARM_MISSION_TIME_LEFT	42
#define SAR_PARM_MISSION_BEGIN_AT	43
#define SAR_PARM_MISSION_BEGIN_AT_POS	44
#define SAR_PARM_MISSION_ARRIVE_AT	45
#define SAR_PARM_MISSION_MESSAGE_SUCCESS	46
#define SAR_PARM_MISSION_MESSAGE_FAIL		47
#define SAR_PARM_MISSION_HUMANS_TALLY	48
#define SAR_PARM_MISSION_ADD_INTERCEPT	49

#define SAR_PARM_MISSION_LOG_HEADER	80
#define SAR_PARM_MISSION_LOG_EVENT	81

#define SAR_PARM_NEW_OBJECT		100
#define SAR_PARM_NEW_HELIPAD		101
#define SAR_PARM_NEW_RUNWAY		102
#define SAR_PARM_NEW_HUMAN		103
#define SAR_PARM_NEW_FIRE		104
#define SAR_PARM_NEW_SMOKE		105
#define SAR_PARM_NEW_PREMODELED		106
#define SAR_PARM_SELECT_OBJECT_BY_NAME	107
#define SAR_PARM_MODEL_FILE		108
#define SAR_PARM_RANGE			109
#define SAR_PARM_RANGE_FAR		110
#define SAR_PARM_TRANSLATE		111
#define SAR_PARM_TRANSLATE_RANDOM	112
#define SAR_PARM_ROTATE			113
#define SAR_PARM_NO_DEPTH_TEST		114
#define SAR_PARM_POLYGON_OFFSET		115
#define SAR_PARM_CONTACT_BOUNDS_SPHERICAL	116
#define SAR_PARM_CONTACT_BOUNDS_CYLENDRICAL	117
#define SAR_PARM_CONTACT_BOUNDS_RECTANGULAR	118
#define SAR_PARM_GROUND_ELEVATION	119

#define SAR_PARM_OBJECT_NAME		130
#define SAR_PARM_OBJECT_MAP_DESCRIPTION	131
#define SAR_PARM_FUEL			132
#define SAR_PARM_HITPOINTS		133
#define SAR_PARM_ENGINE_STATE		134
#define SAR_PARM_PASSENGERS		135	/* And crew */
#define SAR_PARM_RUNWAY_APPROACH_LIGHTING_NORTH	150
#define SAR_PARM_RUNWAY_APPROACH_LIGHTING_SOUTH	151
#define SAR_PARM_HUMAN_MESSAGE_ENTER	180
#define SAR_PARM_HUMAN_REFERENCE	181
#define SAR_PARM_WELCOME_MESSAGE        182
#define SAR_PARM_WIND                   183

/*
 *	File format parameter and value structures:
 *
 *	All structures have the first member to be an int named type
 *	to indicate what kind of structure it is.
 */
/* SAR_PARM_VERSION */
typedef struct {
	int type;
	int major, minor, release;
	char *copyright;
} sar_parm_version_struct;

/* SAR_PARM_NAME */
typedef struct {

	int type;
	char *name;

} sar_parm_name_struct;

/* SAR_PARM_DESCRIPTION */
typedef struct {

        int type;
        char *description;

} sar_parm_description_struct;

/* SAR_PARM_PLAYER_MODEL_FILE */
typedef struct {

        int type;
        char *file;

} sar_parm_player_model_file_struct;

/* SAR_PARM_WEATHER */
typedef struct {

	int type;
	char *weather_preset_name;

} sar_parm_weather_struct;

/* SAR_PARM_WIND */
typedef struct {

    int type;
    float heading; /* Radians */
    float speed;   /* In meters per cycle */
    sar_obj_flags_t flags;	/* Any of SAR_WIND_FLAG_* */

} sar_parm_wind_struct;


/* SAR_PARM_TIME_OF_DAY */
typedef struct {
   
        int type;
	float tod;	/* Time of day (in seconds) since midnight */

} sar_parm_time_of_day_struct;


/* SAR_PARM_REGISTER_LOCATION */
typedef struct {

        int type;
	char *name;
	sar_position_struct pos;
	sar_direction_struct dir;

} sar_parm_register_location_struct;

/* SAR_PARM_SCENE_GPS */
typedef struct {

        int type;
	float	dms_x_offset,	/* In degrees */
		dms_y_offset;
	float	planet_radius;	/* In meters */

} sar_parm_scene_gps_struct;

/* SAR_PARM_SCENE_MAP */
typedef struct {

        int type;
        char	*file;		/* Scene map texture file */
        float	width, height;	/* In meters */

} sar_parm_scene_map_struct;

/* SAR_PARM_SCENE_ELEVATION */
typedef struct {

        int type;
	float	elevation;	/* In meters */

} sar_parm_scene_elevation_struct;

/* SAR_PARM_SCENE_CANT */
typedef struct {

        int type;
	float	cant;	/* Radians to be applied (added) to readout angles */

} sar_parm_scene_cant_struct;

/* SAR_PARM_SCENE_GROUND_FLAGS */
typedef struct {

        int type;
	sar_obj_flags_t flags;	/* Any of SAR_SCENE_BASE_FLAG_* */

} sar_parm_scene_ground_flags_struct;

/* SAR_PARM_SCENE_GROUND_TILE */
typedef struct {

        int type;
	int tile_width, tile_height;		/* In meters */
	float close_range;			/* In meters */
	sar_color_struct color;			/* Far solid color */
	char *texture_name;

} sar_parm_scene_ground_tile_struct;


/* SAR_PARM_TEXTURE_BASE_DIRECTORY */
typedef struct {

        int type;
	char *directory;

} sar_parm_texture_base_directory_struct;

/* SAR_PARM_TEXTURE_LOAD */
typedef struct {

        int type;
	char *name;
	char *file;
	float priority;

} sar_parm_texture_load_struct;


/* SAR_PARM_MISSION_SCENE_FILE */
typedef struct {

        int type;
        char *file;

} sar_parm_mission_scene_file_struct;

/* SAR_PARM_MISSION_NEW_OBJECTIVE */
typedef struct {

	int type;
	int objective_type;	/* One of SAR_MISSION_OBJECTIVE_* */

} sar_parm_mission_new_objective_struct;

/* SAR_PARM_MISSION_TIME_LEFT */
typedef struct {

        int type;
	float	time_left;	/* In seconds */

} sar_parm_mission_time_left_struct;

/* SAR_PARM_MISSION_BEGIN_AT */
typedef struct {

        int type;
	char *name;

} sar_parm_mission_begin_at_struct;

/* SAR_PARM_MISSION_BEGIN_AT_POS */
typedef struct {

	int type;
	sar_position_struct pos;
	sar_direction_struct dir;

} sar_parm_mission_begin_at_pos_struct;

/* SAR_PARM_MISSION_ARRIVE_AT */
typedef struct {

        int type;
        char *name;		/* Name of object to arrive at */

} sar_parm_mission_arrive_at_struct;

/* SAR_PARM_MISSION_MESSAGE_SUCCESS */
typedef struct {

        int type;
        char *message;

} sar_parm_mission_message_success_struct;

/* SAR_PARM_MISSION_MESSAGE_FAIL */
typedef struct {

        int type;
        char *message;

} sar_parm_mission_message_fail_struct;

/* SAR_PARM_MISSION_HUMANS_TALLY */
typedef struct {

        int type;
	int humans_need_rescue;	/* Initial starting total, must be
				 * less or equal to total_humans.
				 */
	int total_humans;	/* Total humans that need rescue */

} sar_parm_mission_humans_tally_struct;

/* SAR_PARM_MISSION_ADD_INTERCEPT */
typedef struct {

        int type;

	/* Reference code:
	 * 0 = Standard
	 * 1 = (Reserved)
	 * 2 = Begin at location
	 * 3 = Arrive at location
	 */
	int	ref_code;

	sar_position_struct pos;
	float	radius;
	float	urgency;
	char	*name;

} sar_parm_mission_add_intercept_struct;


/* SAR_PARM_MISSION_LOG_HEADER */
typedef struct {

	int type;
	int	version_major,
		version_minor,
		version_release;
	char	*title;
	char	*scene_file;
	char	*player_stats_file;

} sar_parm_mission_log_header_struct;

/* SAR_PARM_MISSION_LOG_EVENT */
typedef struct {

	int type;
	int	event_type;	/* One of SAR_MISSION_LOG_EVENT_* */
	float	tod;		/* Time of day (in seconds) since midnight */
	sar_position_struct pos;
	char	*message;

} sar_parm_mission_log_event_struct;


/* SAR_PARM_NEW_OBJECT */
typedef struct {

        int type;
	int object_type;	/* One of SAR_OBJ_TYPE_* */

} sar_parm_new_object_struct;

/* SAR_PARM_NEW_HELIPAD */
typedef struct {

        int type;

	sar_obj_flags_t flags;  /* Any of SAR_HELIPAD_FLAG_* */

	char	*style;
	float	length, width;	/* In meters */
	float	recession;	/* In meters */

	char	*label;

	char	*ref_obj_name;
	sar_position_struct	ref_offset;	/* Rel to ref object */
        sar_direction_struct	ref_dir;	/* Rel to ref object */

} sar_parm_new_helipad_struct;

/* SAR_PARM_NEW_RUNWAY */
typedef struct {

        int		type;

	sar_obj_flags_t	flags;  /* Any of SAR_RUNWAY_FLAG_* */

	float		range;		/* Visual range in meters */
	float		length,		/* Size (in meters) */
			width;
	int		surface_type;	/* One of SAR_RUNWAY_SURFACE_* */

	char		*north_label,
			*south_label;

	int		dashes;		/* Number of dashes (0 for none) */

	float		north_displaced_threshold,	/* In meters (0.0 for none) */
			south_displaced_threshold;

	float		edge_light_spacing;		/* In meters (0.0 for none) */

} sar_parm_new_runway_struct;

/* SAR_PARM_NEW_HUMAN */
typedef struct {

	int type;
	char	*type_name;
	sar_obj_flags_t	flags;	/* Any of SAR_HUMAN_FLAG_* */

} sar_parm_new_human_struct;

/* SAR_PARM_NEW_FIRE */
typedef struct {

	int type;
	float	radius,		/* Size in meters */
		height;		/* Height above base in meters */

} sar_parm_new_fire_struct;

/* SAR_PARM_NEW_SMOKE */
typedef struct {

	int type;
	sar_position_struct offset;
	float	radius_start,	/* In meters */
		radius_max,	/* In meters */
		radius_rate;	/* Can be -1.0 for autocalculate */
	int hide_at_max;
	time_t respawn_int;	/* Respawn interval in ms */
	int total_units;
	int color_code;		/* 0 = light grey/white
				 * 1 = medium grey
				 * 2 = dark/black
				 */
} sar_parm_new_smoke_struct;

/* SAR_PARM_NEW_PREMODELED */
typedef struct {

        int type;
	char *model_type;

	/* Arguments (not including the model type) */
	char **argv;
	int argc;

} sar_parm_new_premodeled_struct;

/* SAR_PARM_SELECT_OBJECT_BY_NAME */
typedef struct {

	int type;
	char *name;

} sar_parm_select_object_by_name_struct;

/* SAR_PARM_MODEL_FILE */
typedef struct {

        int type;
	char *file;

} sar_parm_model_file_struct;

/* SAR_PARM_RANGE */
typedef struct {

        int type;
	float	range;		/* In meters */

} sar_parm_range_struct;

/* SAR_PARM_RANGE_FAR */
typedef struct {

        int type;
        float	range_far;	/* In meters */

} sar_parm_range_far_struct;

/* SAR_PARM_TRANSLATE */
typedef struct {

        int type;
	sar_position_struct translate;

} sar_parm_translate_struct;

/* SAR_PARM_TRANSLATE_RANDOM */
typedef struct {

	int type;
	float	radius_bound;	/* In meters */
	float	z_bound;	/* In meters */

} sar_parm_translate_random_struct;

/* SAR_PARM_ROTATE */
typedef struct {

        int type;
	sar_direction_struct rotate;

} sar_parm_rotate_struct;

/* SAR_PARM_NO_DEPTH_TEST */
typedef struct {

        int type;

} sar_parm_no_depth_test_struct;

/* SAR_PARM_POLYGON_OFFSET */
typedef struct {

        int type;

	/* Offset flags, can contain any of the following:
	 * SAR_OBJ_FLAG_POLYGON_OFFSET, 
	 * SAR_OBJ_FLAG_POLYGON_OFFSET_REVERSE, and/or
	 * SAR_OBJ_FLAG_POLYGON_OFFSET_WRITE_DEPTH
	 */
	sar_obj_flags_t flags;

} sar_parm_polygon_offset_struct;

/* SAR_PARM_CONTACT_BOUNDS_SPHERICAL */
typedef struct {

        int type;
	float	radius;		/* In meters */

} sar_parm_contact_bounds_spherical_struct;

/* SAR_PARM_CONTACT_BOUNDS_CYLENDRICAL */
typedef struct {

        int type;
	float	radius;		/* In meters */
	float	height_min,	/* In meters */
		height_max;

} sar_parm_contact_bounds_cylendrical_struct;

/* SAR_PARM_CONTACT_BOUNDS_RECTANGULAR */
typedef struct {

        int type;
	float	x_min, x_max,		/* In meters */
		y_min, y_max,
		z_min, z_max;

} sar_parm_contact_bounds_rectangular_struct;

/* SAR_PARM_GROUND_ELEVATION */
typedef struct {

        int type;
	float	elevation;		/* In meters */

} sar_parm_ground_elevation_struct;

/* SAR_PARM_OBJECT_NAME */
typedef struct {

        int type;
        char *name;

} sar_parm_object_name_struct;

/* SAR_PARM_OBJECT_MAP_DESCRIPTION */
typedef struct {

        int type;
        char *description;

} sar_parm_object_map_description_struct;

/* SAR_PARM_FUEL */
typedef struct {

        int type;
	float	fuel,		/* In kg */
		fuel_max;

} sar_parm_fuel_struct;

/* SAR_PARM_HITPOINTS */
typedef struct {

        int type;
	float	hitpoints,
		hitpoints_max;

} sar_parm_hitpoints_struct;

/* SAR_PARM_ENGINE_STATE */
typedef struct {

        int type;
	int state;		/* One of SAR_ENGINE_STATE_* */

} sar_parm_engine_state_struct;

/* SAR_PARM_PASSENGERS */
typedef struct {

        int type;
	int	passengers,
		passengers_max;	/* Can be -1 for leave as is */

} sar_parm_passengers_struct;

/* SAR_PARM_RUNWAY_APPROACH_LIGHTING_NORTH */
typedef struct {

        int type;

        /* Any of SAR_RUNWAY_APPROACH_LIGHTING_* */
        sar_obj_flags_t flags;

} sar_parm_runway_approach_lighting_north_struct;

/* SAR_PARM_RUNWAY_APPROACH_LIGHTING_SOUTH */
typedef struct {

        int type;

        /* Any of SAR_RUNWAY_APPROACH_LIGHTING_* */
        sar_obj_flags_t flags;

} sar_parm_runway_approach_lighting_south_struct;

/* SAR_PARM_HUMAN_MESSAGE_ENTER */
typedef struct {

        int type;
	char *message;

} sar_parm_human_message_enter_struct;

/* SAR_PARM_HUMAN_REFERENCE */
typedef struct {

        int type;

	/* Can contain one of the following flags;
	 * SAR_HUMAN_FLAG_RUN_TOWARDS or SAR_HUMAN_FLAG_RUN_AWAY
	 */
	sar_obj_flags_t flags;

	/* Name of reference object to run to or run away from */
        char *reference_name;

} sar_parm_human_reference_struct;

typedef struct {
    int type;
    char *message;


} sar_parm_welcome_message_struct;

/* In sarfio.c */
extern void *SARParmNew(int type);
extern void *SARParmNewAppend(int type, void ***ptr, int *total);
extern void SARParmDelete(void *p);
extern void SARParmDeleteAll(void ***ptr, int *total);

/* In sarfioopen.c */
extern int SARParmLoadFromFile(
        const char *filename, int file_format,
        void ***parm, int *total_parms,
	int filter_parm_type,		/* One of SAR_PARM_* or -1 */
        void *client_data,
        int (*progress_func)(void *, long, long)
);

/* In sarfiosave.c */
extern int SARParmSaveToFile(
	const char *filename, int file_format,
	void **parm, int total_parms,
	void *client_data,
	int (*progress_func)(void *, long, long)
);


#endif	/* SARFIO_H */
