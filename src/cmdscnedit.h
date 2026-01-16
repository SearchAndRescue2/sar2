/* COMPILE_EDITOR_WITH_GTK_UI is defined (or not) in SConscript */
#ifdef COMPILE_EDITOR_WITH_GTK_UI
#include <gtk/gtk.h>
#include <gdk/gdk.h>
#if GTK_MAJOR_VERSION == 3
#include <gdk/gdkx.h>
#endif
#if GTK_MAJOR_VERSION == 4
#include <gdk/x11/gdkx.h>
#endif
#include "editorgtkui.h"
#endif

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
 * Object placer (triedron) *.3d file data.
 * If it doesn't exist yet, a $TMPDIR/object_placer.3d file will be created
 * then the hereunder string will be written to it. Then, this file will be
 * loaded as needed.
 */
#define EDITOR_OBJECT_PLACER_DATA "\
begin_header\n\
creator v3dconverter\n\
end_header\n\
\n\
type 4\n\
range 20000\n\
crash_flags 1  0  0\n\
contact_cylendrical 1.0 -1.0 1.0\n\
dry_mass 1000.0\n\
fuel 0.0001 1000.0 1000.0\n\
engine n 0 100 1.0\n\
\n\
begin_model standard\n\
#OBJFILE material: White\n\
color 0.800000 0.800000 0.800000 1.000000 1.000000 1.000000 0.500000 0.250000 0.000000\n\
begin_triangles\n\
 normal 0.000000 -1.000000 0.000000\n\
 0.050000 -0.050000 -0.050000\n\
 -0.050000 -0.050000 -0.050000\n\
 0.050000 -0.050000 0.050000\n\
 normal -0.000000 -0.000000 -1.000000\n\
 -0.050000 0.050000 -0.050000\n\
 -0.050000 -0.050000 -0.050000\n\
 0.050000 0.050000 -0.050000\n\
 normal -1.000000 -0.000000 0.000000\n\
 -0.050000 -0.050000 -0.050000\n\
 -0.050000 0.050000 -0.050000\n\
 -0.050000 -0.050000 0.050000\n\
 normal 0.000000 -1.000000 0.000000\n\
 -0.050000 -0.050000 -0.050000\n\
 -0.050000 -0.050000 0.050000\n\
 0.050000 -0.050000 0.050000\n\
 normal -0.000000 -0.000000 -1.000000\n\
 -0.050000 -0.050000 -0.050000\n\
 0.050000 -0.050000 -0.050000\n\
 0.050000 0.050000 -0.050000\n\
 normal -1.000000 -0.000000 0.000000\n\
 -0.050000 0.050000 -0.050000\n\
 -0.050000 0.050000 0.050000\n\
 -0.050000 -0.050000 0.050000\n\
end_triangles\n\
#OBJFILE material: Green\n\
color 0.000000 1.000000 0.000000 1.000000 1.000000 1.000000 0.500000 0.250000 0.000000\n\
begin_triangles\n\
 normal 0.000000 0.052600 0.998600\n\
 -0.050000 0.050000 0.050000\n\
 -0.000000 1.000000 0.000000\n\
 0.050000 0.050000 0.050000\n\
 normal -0.998600 0.052600 0.000000\n\
 -0.000000 1.000000 0.000000\n\
 -0.050000 0.050000 0.050000\n\
 -0.050000 0.050000 -0.050000\n\
 normal 0.998600 0.052600 -0.000000\n\
 0.050000 0.050000 0.050000\n\
 -0.000000 1.000000 0.000000\n\
 0.050000 0.050000 -0.050000\n\
 normal -0.000000 0.052600 -0.998600\n\
 0.050000 0.050000 -0.050000\n\
 -0.000000 1.000000 0.000000\n\
 -0.050000 0.050000 -0.050000\n\
end_triangles\n\
#OBJFILE material: Red\n\
color 1.000000 0.000000 0.000000 1.000000 1.000000 1.000000 0.500000 0.250000 0.000000\n\
begin_triangles\n\
 normal 0.052600 -0.998600 -0.000000\n\
 0.050000 -0.050000 0.050000\n\
 1.000000 0.000000 -0.000000\n\
 0.050000 -0.050000 -0.050000\n\
 normal 0.052600 0.000000 0.998600\n\
 0.050000 0.050000 0.050000\n\
 1.000000 0.000000 -0.000000\n\
 0.050000 -0.050000 0.050000\n\
 normal 0.052600 0.998600 -0.000000\n\
 0.050000 0.050000 -0.050000\n\
 1.000000 0.000000 -0.000000\n\
 0.050000 0.050000 0.050000\n\
 normal 0.052600 0.000000 -0.998600\n\
 0.050000 -0.050000 -0.050000\n\
 1.000000 0.000000 -0.000000\n\
 0.050000 0.050000 -0.050000\n\
end_triangles\n\
#OBJFILE material: Blue\n\
color 0.000000 0.000000 1.000000 1.000000 1.000000 1.000000 0.500000 0.250000 0.000000\n\
begin_triangles\n\
 normal 0.998600 0.000000 0.052600\n\
 0.000000 0.000000 1.000000\n\
 0.050000 0.050000 0.050000\n\
 0.050000 -0.050000 0.050000\n\
 normal 0.000000 -0.998600 0.052600\n\
 -0.050000 -0.050000 0.050000\n\
 0.000000 0.000000 1.000000\n\
 0.050000 -0.050000 0.050000\n\
 normal -0.998600 -0.000000 0.052600\n\
 -0.050000 0.050000 0.050000\n\
 0.000000 0.000000 1.000000\n\
 -0.050000 -0.050000 0.050000\n\
 normal -0.000000 0.998600 0.052600\n\
 0.050000 0.050000 0.050000\n\
 0.000000 0.000000 1.000000\n\
 -0.050000 0.050000 0.050000\n\
end_triangles\n\
end_model standard\n\
\n\
# Pilot head (eyes) position\n\
#                x     y    z\n\
cockpit_offset 0.00  0.00  2.00\n\
\n\
begin_model cockpit\n\
# Cockpit model declaration is mandatory for an aircraft, but\n\
# 3d primitives are not needed for the scenery editor \"aircraft\".\n\
end_model cockpit\n\
\n\
begin_model shadow\n\
begin_triangles\n\
 normal 0.000000 0.000000 1.000000\n\
 -0.050000 -0.050000 0.000000\n\
 -0.050000 0.050000 0.000000\n\
 0.050000 0.050000 -0.000000\n\
 -0.000000 1.000000 0.000000\n\
 0.050000 0.050000 -0.000000\n\
 -0.050000 0.050000 0.000000\n\
 1.000000 0.000000 -0.000000\n\
 0.050000 -0.050000 -0.000000\n\
 0.050000 0.050000 -0.000000\n\
 0.050000 -0.050000 -0.000000\n\
 -0.050000 -0.050000 0.000000\n\
 0.050000 0.050000 -0.000000\n\
end_triangles\n\
end_model shadow\n\
"

/* Editor action types */
typedef enum {
	EDITOR_ACTION_NONE,
	EDITOR_ACTION_QUIT,		// quit from SARCmdSceneEditor()
	EDITOR_ACTION_QUIT_FROM_GTK,	// quit from GTK UI
	EDITOR_ACTION_QUIT_WITHOUT_PRINT,
	EDITOR_ACTION_ASK_TO_PRINT_BEFORE_QUIT,
	EDITOR_ACTION_PRINT,
	EDITOR_ACTION_NEW,
	EDITOR_ACTION_SET,
	EDITOR_ACTION_UNLOAD,
	EDITOR_ACTION_COPY,
	EDITOR_ACTION_MODIFY,
	EDITOR_ACTION_INFO,
	EDITOR_ACTION_INFO_NEXT,
	EDITOR_ACTION_MOVE,
	EDITOR_ACTION_REMOVE,
	EDITOR_ACTION_NAME
} editor_action_type;

/*
 * Editor object data structure.
 *
 * This structure groups all necessary data to generate an object, regardless
 * of its type.
 *
 * Some data are stored in char or string type in order to be "ready to write"
 * in the scenery file. All strings, except the 'type_s' one, can be NULL.
 *
 * Height values are stored in feet (not in meters) and direction values are
 * stored in degrees (not in radians).
 *
 * Any parameter added to this structure must be (at least) added in the
 * EditorObjectDataStructNew(), EditorObjectDataStructFree() functions.
 *
 * Members names are same as those used in sar2 object-dedicated structures.
 */

typedef struct {
	/*
	 * "create_*" common data (used by more than one object type):
	 */
	sar_obj_type		type;		/* one of SAR_OBJ_TYPE_* */
	char			*type_s,	/* one of SAR_OBJ_TYPE_*_S */
				*name;		/* the object name */

	sar_position_struct	pos;		/* x(m), y(m), z(IN FEET) */
	sar_direction_struct	dir;		/* heading, pitch, bank (IN DEGREES) */

	float			range,		/* visual range (meters) */
				length,		/* meters */
				width,		/* meters */
				height;		/* feet */

	int			ref_obj_num;
	char 			*ref_obj_name;	/* Reference object name */
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
				radius_rate,		/* meters per second */
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
#define EDITOR_OBJECT_FLAG_NAMED	(1 << 4)
	sar_obj_flags_t		flags;

	editor_object_data_struct	*obj_data_original,	/* at scenery opening */
					*obj_data_new;		/* new data */

	Boolean				is_landable;
} editor_modified_object_struct;


/*
 * In-game scenery editor:
 */
typedef struct {

#define YES_NO_QUERY_NONE		0
#define QUERY_YES_NO_PRINT_BEFORE_QUIT	1

/* Maximum number of objects in the pick list for the "info next" command */
#define PICKSKIPMAX 5

	/* Object placer (aka the triedron) file name */
	char			*object_placer_file_name;

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

	/* "User text input" commands states.
	 * User text input commands are commands which need some text to be
	 * validated by user after the command call. For example, once user
	 * has entered '/mod' to modify an object, he must validate the data
	 * by pressing the <Enter> key.
	 * If he presses the <Esc> key instead of the <Enter> key, then the
	 * text_input_escaped variable will be set to True by SARKeyEscape().
	 */
	Boolean			in_modif_state,		/* True while user
							 * enters modifying data.
							 */
				in_move_at_state,	/* True while user
							 * enters positioning data.
							 */
				in_name_state,		/* True while user
							 * enters an object name.
							 */
				text_input_escaped;	/* True if text input has been
							 * cancelled by user by
							 * pressing the 'esc' key.
							 */

	Boolean			in_move_state;		/* True if the current object
							 * is an existing object being
							 * moved.
							 */

	Boolean			must_print;		/* False when all
							 * modification have been
							 * printed.
							 */

	int			current_action;		/* One of EDITOR_ACTION_* */

	float			altitude_increment;

	Boolean			gui_mode_on;		/* True if any Graphical UI
							 * (GTK, QT, ...) is ON.
							 */
#ifdef COMPILE_EDITOR_WITH_GTK_UI
	Boolean			gtk_mode_on;		/* True if a GTK UI is ON */
	editor_gtk_ui_struct	*editor_gtk_ui;
#endif

	/* For the "info next" command */
	int			pick_skip_list[PICKSKIPMAX];
	int			pick_skip_list_index;

        /* Currently modified object data.
	 * Used in functions in which ones user can modify
	 * some existing parameters.
	 */
	char			*mod_cur_parm;

	/* Previously edited object */
	int			prev_obj_num;		/* Number */

	/* Previous player model file name (before entering scenery editor) */
	char			*player_old_model_file;

	int			mod_obj_num;		/* Currently modified object number */
	int			total_original_objects,
				total_objects;
	editor_modified_object_struct	**modification_list;
	int			yes_no_query;

} sar_scenery_editor_struct;

#ifndef SAR_CMD_PROTOTYPE
/*
 *	Prototype for all SARCmd*() function input parameters.
 */
#define SAR_CMD_PROTOTYPE	void *data, const char *arg, unsigned long flags
#endif /* SAR_CMD_PROTOTYPE */
void SARCmdSceneEditor(SAR_CMD_PROTOTYPE);

/* In editorgtkui.c */
extern int gtkAppStart(sar_core_struct *core_ptr, unsigned long flags);
extern void gtkAppStop(sar_core_struct *core_ptr);
extern void EditorGtkAskToQuitWithoutPrint();

#ifndef SAR_KEY_FUNC_PROTOTYPE
/* Prototype for all SARKey*() functions */
#define SAR_KEY_FUNC_PROTOTYPE				\
sar_core_struct *core_ptr, gw_display_struct *display,	\
sar_scene_struct *scene, Boolean state
#endif	/* SAR_KEY_FUNC_PROTOTYPE */
extern void SARKeyCommand(SAR_KEY_FUNC_PROTOTYPE);

char* DoParametersLineFromEditorObjectData(const editor_object_data_struct *editor_obj_data);
