#include <gtk/gtk.h>

/* Object type names (as string) */
#define SAR_OBJ_TYPE_GARBAGE_S "garbage"
#define SAR_OBJ_TYPE_STATIC_S "static"
#define SAR_OBJ_TYPE_AUTOMOBILE_S "automobile"
#define SAR_OBJ_TYPE_WATERCRAFT_S "watercraft"
#define SAR_OBJ_TYPE_AIRCRAFT_S "aircraft"
#define SAR_OBJ_TYPE_GROUND_S "ground"
#define SAR_OBJ_TYPE_RUNWAY_S "runway"
#define SAR_OBJ_TYPE_HELIPAD_S "helipad"
#define SAR_OBJ_TYPE_HUMAN_S "human"
#define SAR_OBJ_TYPE_SMOKE_S "smoke"
#define SAR_OBJ_TYPE_FIRE_S "fire"
#define SAR_OBJ_TYPE_EXPLOSION_S "explosion"
#define SAR_OBJ_TYPE_CHEMICAL_SPRAY_S "chemical_spray"
#define SAR_OBJ_TYPE_FUELTANK_S "fueltank"
#define SAR_OBJ_TYPE_PREMODELED_S "premodeled"

/* Helipad styles */
#define SAR_HELIPAD_STYLE_GROUND_PAVED_S "ground_paved"
#define SAR_HELIPAD_STYLE_GROUND_BARE_S "ground_bare"
#define SAR_HELIPAD_STYLE_BUILDING_S "building"
#define SAR_HELIPAD_STYLE_VEHICLE_S "vehicle"
#define SAR_HELIPAD_STYLE_DEFAULT_S "default"
#define SAR_HELIPAD_STYLE_STANDARD_S "standard"

/* Runway surfaces */
#define SAR_RUNWAY_SURFACE_PAVED_S "paved"
#define SAR_RUNWAY_SURFACE_GRAVEL_S "gravel"
#define SAR_RUNWAY_SURFACE_CONCRETE_S "concrete"
#define SAR_RUNWAY_SURFACE_GROVED_S "groved"

/* Premodeled types */
#define SAR_PREMODELED_BUILDING_S "building"
#define SAR_PREMODELED_CONTROL_TOWER_S "control_tower"
#define SAR_PREMODELED_POWER_TRANSMISSION_TOWER_S "power_transmission_tower"
#define SAR_PREMODELED_TOWER_S "tower"
#define SAR_PREMODELED_RADIO_TOWER_S "radio_tower"
#define SAR_PREMODELED_HANGAR_S "hangar"
#define SAR_PREMODELED_UNKNOWN_S "(unknown)"

/* Smoke colors */
#define SAR_SMOKE_COLOR_0_S "light grey / white"
#define SAR_SMOKE_COLOR_1_S "medium grey"
#define SAR_SMOKE_COLOR_2_S "dark / black"
#define SAR_SMOKE_COLOR_3_S "orange"

/*
 * Editor object data structure.
 *
 * This structure groups all necessary data to generate an object, regardless
 * of its type.
 *
 * Some data are stored in char or string type in order to be "ready to write"
 * in the scenery file. All strings (except 'type_s') can be NULL.
 *
 * Height values are stored in feet (not in meters) and direction values are
 * stored in degrees (not in radians).
 *
 * Any parameter added in this structure must be (at least) added in the
 * EditorObjectDataStructNew(), EditorObjectDataStructFree() and
 * EditorObjectDataStructAreEqual() functions.
 *
 * Members names are same as those used in sar2 object-dedicated structures.
 */

typedef struct {

	/*
	 * "create_*" common data (used by more than one object type):
	 */

	sar_obj_type		type;		/* one of SAR_OBJ_TYPE_* */
	char			*type_s,	/* one of SAR_OBJ_TYPE_*_S */
				*name,		/* the 'name' parameter */
				*obj_name,	/* the 'object_name' parameter */
				*object_map_description;

	sar_position_struct	pos;		/* x(m), y(m), z(IN FEET) */
	sar_direction_struct	dir;		/* heading, pitch, bank (IN DEGREES) */

	float			range,		/* visual range (meters) */
				length,		/* meters */
				width,		/* meters */
				height;		/* feet */

	int			ref_obj_num;
	char 			*ref_obj_name;	/* reference object name */
	sar_position_struct	ref_obj_pos;	/* Reference object position */
	sar_direction_struct	ref_obj_dir;	/* Reference object direction */
	sar_position_struct	offset_pos;	/* Relative to ref_object */


	/*
	 * "create_fire" specific data:
	 */

	float			radius;


	/*
	 * "create_helipad" specific data:
	 */

	sar_helipad_style	style;			/* one of SAR_HELIPAD_STYLE_* */
	char			*style_s;		/* one of SAR_HELIPAD_STYLE_*_S */
	float			recession;		/* feet */
	char			*label;			/* String without space */
	char			edge_lighting_c,	/* 'y' or 'n' */
				has_fuel_c,		/* 'y' or 'n' */
				has_repair_c,		/* 'y' or 'n' */
				has_drop_off_c,		/* 'y' or 'n' */
				restarting_point_c;	/* 'y' or 'n' */
	sar_direction_struct	offset_dir;		/* Relative to ref_object */


	/*
	 * "create_human" specific data:
	 */

	char			*type_name,		/* Human preset name */
				*need_rescue_s,		/* "need_rescue" or NULL */
				*sit_up_s,		/* "sit_up" or NULL */
				*sit_down_s,		/* "sit_down" or NULL */
				*sitting_s,		/* "sitting" or NULL */
				*lying_s,		/* "lying" or NULL */
				*alert_s,		/* "alert" or NULL */
				*aware_s,		/* "aware" or NULL */
				*in_water_s,		/* "in_water" or NULL */
				*on_stretcher_s,	/* "on_stretcher" or NULL */
				*assisted_s;		/* "assisted" or NULL */
	int			assistants;	/* 0 up to SAR_ASSISTING_HUMANS_MAX */
	char 			*assist_type_name[SAR_ASSISTING_HUMANS_MAX];
	char			*human_displacement_dir_s; /* run_towards or run_away */


	/*
	 * "create_object" (*.3d file defined object) specific data:
	 */
	char			*file_name;	/* model file name, including
						 * sar2/data/ relative path.
						 */


	/*
	 * "create_premodeled" specific data:
	 */

	sar_premodeled_type	pm_type;	/* one of SAR_OBJ_PREMODELED_* */
	char			*pm_type_s;	/* one of SAR_PREMODELED_*_s */
	int			hazard_lights;
	char			*walls_texture_s,
				*walls_texture_night_s,
				*roof_texture_s;


	/*
	 * "create_runway" specific data:
	 */

	sar_runway_surface_type	surface_type;	/* one of SAR_RUNWAY_SURFACE_* */
	char			*surface_type_s;/* one of SAR_RUNWAY_SURFACE_*_S */
	int			dashes;
	float			edge_light_spacing;		/* meters */
	char			*north_label,		/* String without space */
				*south_label;		/* String without space */
	float			north_displaced_threshold,	/* meters */
				south_displaced_threshold;	/* meters */

	char			*has_thresholds_s,	/* "thresholds" or NULL */
				*has_borders_s,		/* "borders" or NULL */
				*has_td_markers_s,	/* "td_markers" or NULL */
				*has_midway_markers_s,	/* "midway_markers" or NULL */
				*has_north_gs_s,	/* "north_gs" or NULL */
				*has_south_gs_s;	/* "south_gs" or NULL */


	/*
	 * "create_smoke" specific data:
	 */

	float			radius_start,		/* meters */
				radius_max,		/* meters */
				radius_rate,		/* meters */
				hide_at_max;		/* meters (not feet) */
	time_t			respawn_int;
	int			total_units;
	int			color_code;

} editor_object_data_struct;


typedef struct {

#define EDITOR_OBJECT_FLAG_ORIGINAL	(1 << 0)
#define EDITOR_OBJECT_FLAG_DELETED	(1 << 1)
#define EDITOR_OBJECT_FLAG_MODIFIED	(1 << 2)
#define EDITOR_OBJECT_FLAG_MOVED	(1 << 3)
	sar_obj_flags_t		flags;

	editor_object_data_struct	*obj_data_original,	/* at scenery opening */
					*obj_data_new;		/* new data */

} editor_modified_object_struct;


/*
typedef struct {
	char	*val_original,		// at scenery opening /
		*val_new;		// new data /

} editor_modified_value_struct;
*/


/*
 * In-game scenery editor:
 */
typedef struct {

	/* Sceney work file name and pointer.
	 * Work file is a read/write copy of game current scenery file.
	 */
	char			*scn_file_name;
	FILE			*scn_file_fp;

	/* Currently edited object */
	sar_obj_type		cur_obj_type;		/* Type */
	int			cur_obj_num;		/* Number */
	char			*cur_obj_arg;		/* Command arguments */
	sar_position_struct 	cur_obj_pos;		/* Position */
	sar_direction_struct 	cur_obj_dir;		/* Direction */

	Boolean			in_move_state,		/* True if the current object
							* is an existing object being
							* moved.
							*/
				in_modif_state;		/* True if the current object
							* is an existing object being
							* modified.
							*/


	Boolean			gtk_mode_on;		/* True if GTK UI is ON */
	GMainContext		*gtk_context;		/* For running GTK application
							* without blocking the sar2
							* main loop.
							*/
	GtkApplication		*gtk_application;
	gboolean		gtk_app_running;	/* True when GTK app is running */


	int			pick_skip_list[5];
	int			pick_skip_list_index;

	/* Previously edited object */
	sar_obj_type		prev_obj_type;		/* Type */
	int			prev_obj_num;		/* Number */
	char			*prev_obj_arg;		/* Command arguments */

	int			mod_obj_num;		/* Currently modified object number */

	int				total_original_objects,
					total_objects;
	editor_modified_object_struct	**modification_list;

} sar_scenery_editor_struct;
