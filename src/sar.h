/*
			  Search and Rescue Core
 */

#ifndef SAR_H
#define SAR_H

#include <limits.h>
#include <sys/types.h>
#ifdef __MSW__
# include "../include/os.h"
#else
# include <sys/time.h>
#endif

#include "gw.h"
#include "stategl.h"
#include "textinput.h"
#include "obj.h"
#include "human.h"
#include "weather.h"
#include "gctl.h"
#include "menu.h"
#include "sound.h"
#include "musiclistio.h"
#include "mission.h"
#include "texturelistio.h"
#include "sarfps.h"
#include "sarfio.h"


/*
 *	Units:
 */
typedef enum {
	SAR_UNITS_ENGLISH,
	SAR_UNITS_METRIC,
	SAR_UNITS_METRIC_ALT_FEET
} sar_units;

/*
 *	Rotor Blur Styles:
 */
typedef enum {
	SAR_ROTOR_BLUR_SOLID,
	SAR_ROTOR_BLUR_CLOCK_RADIAL
} sar_rotor_blur_style;

/*
 *	Flight Physics Levels:
 */
typedef enum {
	FLIGHT_PHYSICS_EASY,
	FLIGHT_PHYSICS_MODERATE,
	FLIGHT_PHYSICS_REALISTIC
} sar_flight_physics_level;

/*
 *	Options:
 */
typedef struct {

	Boolean		menu_backgrounds,	/* Draw menu backgrounds */
			menu_change_affects,	/* Special affects when changing menus */
			menu_always_full_redraw;	/* Always do complete
							 * menu redraws even if
							 * only a part of the
							 * menu has changed
							 * (needed for some
							 * accelerated video
							 * cards */

        Boolean		console_quiet,
			internal_debug,
			runtime_debug,
			prioritize_memory;

	/* Units */
	sar_units	units;

	/* Graphics options */
	Boolean		textured_ground,
			textured_objects,
			textured_clouds,
			atmosphere,
			dual_pass_depth,
			prop_wash,
			smoke_trails,
			celestial_objects;

	float		gl_polygon_offset_factor;	/* For glPolygonOffset() */

	/* GL shade model for drawing premodeled objects only
	 * (GL_FLAT or GL_SMOOTH
	 */
	int		gl_shade_model;

	/* Maximum drawing visibility (0 to 6)
	 *
	 * Computed by; miles = 3 + (option.visibility_max * 3)
	 */
	int		visibility_max;

	/* Rotor blade blur style */
	sar_rotor_blur_style	rotor_blur_style;

	/* Graphics acceleration coefficient, 1.0 is for maximum and
	 * 0.0 is for none/minimum
	 */
	float		graphics_acceleration;

	/* Sound options */
	Boolean		engine_sounds,
			event_sounds,
			voice_sounds,
			music;

	/* Sound priority (one of SND_PRIORITY_*) */
	int		sound_priority;

	/* Use system time when starting free flights */
	Boolean		system_time_free_flight;

	/* HUD color and font */
	sar_color_struct	hud_color;
	GWFont		*hud_font;

	/* Standard messages color and font */
	sar_color_struct	message_color;
	GWFont		*message_font;
	GWFont		*menu_font;	/* Menu std font (except menu values) */

	Boolean		show_hud_text,		/* Show HUD text */
			show_outside_text;	/* Show outside display text */

	/* Sticky banner text (first line) big font */
	GWFont		*banner_font;


	/* Intervals in milliseconds */
	time_t		explosion_frame_int,	/* Explosion frame inc interval */
			splash_frame_int;	/* Splash frame inc interval */

	time_t		crash_explosion_life_span,	/* Explosions by crashes */
			fuel_tank_life_span;	/* Dropped tanks landed on ground */

	float		rotor_wash_vis_coeff;	/* 0.0 to 1.0 */

	/* Game controllers mask, any of GCTL_CONTROLLER_* */
	gctl_controllers	gctl_controllers;
	/* Game controller options mask */
	gctl_options		gctl_options;

	/* Joystick priority, one of GCTL_JS_PRIORITY_* */
	gctl_js_priority	js_priority;

	/* Joystick connection type, one of GCTL_JS_CONNECTION_* */
	int		js0_connection,
			js1_connection;

	/* Joystick button mappings */
	int		js0_btn_rotate,		/* Treat bank to heading axis */
			js0_btn_air_brakes,
			js0_btn_wheel_brakes,
			js0_btn_zoom_in,
			js0_btn_zoom_out,
			js0_btn_hoist_up,
			js0_btn_hoist_down;

	int		js1_btn_rotate,		/* Treat bank as heading axis */
			js1_btn_air_brakes,
			js1_btn_wheel_brakes,
			js1_btn_zoom_in,
			js1_btn_zoom_out,
			js1_btn_hoist_up,
			js1_btn_hoist_down;

	/* Game controller joystick axis roles, any of
	 * GCTL_JS_AXIS_ROLE_*
	 *
	 * If any of these are all 0 then that implies the joystick is
	 * not enabled
	 */
	gctl_js_axis_roles	gctl_js0_axis_roles,
				gctl_js1_axis_roles;

	/* Simulation difficulty */
	float		hoist_contact_expansion_coeff;	/* Hoist contact radius coeff */
	float		damage_resistance_coeff;
	sar_flight_physics_level	flight_physics_level;


	/* Last run selections */
	int		last_selected_player,
			last_selected_mission,	/* Matches menu list obj sel pos */
			last_selected_ffscene,
			last_selected_ffaircraft,
			last_selected_ffweather;

	/* Last geometry of main window */
	int		last_x,
			last_y,
			last_width,
			last_height;
	Boolean		last_fullscreen;

} sar_option_struct;
#define SAR_OPTION(p)	((sar_option_struct *)(p))


/*
 *	Directory names:
 */
typedef struct {

	char local_data[PATH_MAX];
	char global_data[PATH_MAX];

} sar_dname_struct;
extern sar_dname_struct dname;

/*
 *	File names:
 */
typedef struct {

	/* Main configuration file */
	char options[PATH_MAX + NAME_MAX];

	/* Predefined humans file */
	char human[PATH_MAX + NAME_MAX];

	/* Music references file */
	char music[PATH_MAX + NAME_MAX];

	/* Players file */
	char players[PATH_MAX + NAME_MAX];

	/* Global textures file */
	char textures[PATH_MAX + NAME_MAX];

	/* Predefined weather conditions file */
	char weather[PATH_MAX + NAME_MAX];

	/* Mission log file */
	char mission_log[PATH_MAX + NAME_MAX];

} sar_fname_struct;
extern sar_fname_struct fname;


/*
 *	Timmers:
 *
 *	All members are in milliseconds.
 */
typedef struct {

	time_t	reclaim;

} sar_next_struct;
extern sar_next_struct next;


/*
 *	Player statistics:
 */
typedef struct {

	char	*name;

	int	score,
		crashes,
		rescues,
		missions_succeeded,
		missions_failed;

	time_t	time_aloft;		/* Seconds */

	time_t	last_mission,		/* Systime seconds */
		last_free_flight;

} sar_player_stat_struct;
#define SAR_PLAYER_STAT(p)	((sar_player_stat_struct *)(p))


/*
 *	Map name matching structure for SARDrawMap():
 *
 *	Map draw OpenGL names to string names record structure, used in
 *	sardraw.c's map drawing function. This is used for selection
 *	matching.
 *
 *	Note that the structure's index is not the matched object's index.
 *	The matched object's index will be dmon_ptr->gl_name if and only
 *	if it is 0 <= dmon_ptr->gl_name < total_objects.
 */
typedef struct {

	/* The object's index number or some special code. A special
	 * code is when: gl_name >= total_objects AND when total_objects
	 * is subtracted from gl_name: special_code = gl_name - 
	 * total_objects (and not negative)
	 */
	GLuint	gl_name;

	/* Coppied pointer to the object's name */
	char	*obj_name;

} sar_drawmap_objname_struct;
#define SAR_DRAWMAP_OBJNAME(p)	((sar_drawmap_objname_struct *)(p))


/*
 *	Core structure:
 */
typedef struct {

	/* Program file (name only) and full path */
	char		*prog_file;
	char		*prog_file_full_path;

	/* Stop count, used during the loading of scene/mission and
	 * other places where the user may want to abort
	 */
	int		stop_count;

	/* Video, sound, and game controller */
	gw_display_struct	*display;	/* Graphics Wrapper */
	char		*recorder_address;	/* Sound Server Address */
	snd_recorder_struct	*recorder;	/* Sound Server Connection */
	char		*audio_mode_name;	/* Sound Server's current Audio Mode */
	gctl_struct	*gctl;			/* Game Controller */

	/* Our code to indicate what background music is currently being
	 * played (one of SAR_MUSIC_ID_*).
	 */
	int		cur_music_id;

	/* Music file references list */
	sar_music_ref_struct	**music_ref;
	int		total_music_refs;


	/* Human presets data */
	sar_human_data_struct	*human_data;

	/* Weather presets data */
	sar_weather_data_struct	*weather_data;

	/* SAR Scene, Objects, and Mission */
	sar_scene_struct	*scene;		/* SAR scene */
	sar_mission_struct	*mission;	/* SAR mission */
	sar_object_struct	**object;	/* SAR objects */
	int		total_objects;


	/* Menu and shared data for menus */
	sar_menu_struct	**menu;
	int		total_menus,
			cur_menu;
	/* Menu images (shared amoung menu object/widgets */
	sar_image_struct	**menu_list_bg_img,	/* 9 images */
				*menu_button_armed_img,
				*menu_button_unarmed_img,
				*menu_button_highlighted_img,
				*menu_button_label_img,
				*menu_label_bg_img,
				*menu_switch_bg_img,
				*menu_switch_off_img,
				*menu_switch_on_img,
				*menu_spin_label_img,
				*menu_spin_value_img,
				*menu_spin_dec_armed_img,
				*menu_spin_dec_unarmed_img,
				*menu_spin_inc_armed_img,
				*menu_spin_inc_unarmed_img,
				*menu_slider_label_img,
				*menu_slider_trough_img,
				*menu_slider_handle_img,
				*menu_progress_bg_img,
				*menu_progress_fg_img,

				*menu_button_pan_up_armed_img,
				*menu_button_pan_up_unarmed_img,
				*menu_button_pan_down_armed_img,
				*menu_button_pan_down_unarmed_img,
				*menu_button_pan_left_armed_img,
				*menu_button_pan_left_unarmed_img,
				*menu_button_pan_right_armed_img,
				*menu_button_pan_right_unarmed_img,

				*menu_button_zoom_in_armed_img,
				*menu_button_zoom_in_unarmed_img,
				*menu_button_zoom_out_armed_img,
				*menu_button_zoom_out_unarmed_img,

				*menumap_helipad_img,	/* Helipad */
				*menumap_intercept_img,	/* Intercept point */
				*menumap_helicopter_img,/* Helicopter */
				*menumap_victim_img,	/* Victim */
				*menumap_vessel_img,	/* Vessel */
				*menumap_crash_img;	/* Crash/explosion */

	/* Global list of standard texture reference names and file names
	 * that are to be loaded *when* a scene is loaded so that it
	 * has a standard set of textures to use
	 */
	sar_texture_name_struct	**texture_list;
	int		total_texture_list;

	/* Player stats (also list of players) */
	sar_player_stat_struct	**player_stat;
	int		total_player_stats;

	/* Selected files */
	char		*cur_mission_file;
	char		*cur_player_model_file;
	char		*cur_scene_file;

	/* Display help during simulation, value of 0 means display
	 * nothing while any positive value corresponds to the `page'
	 * being displayed starting from page 1 being a value of 1
	 */
	int		display_help;

	/* FLIR (night visiion) state */
	Boolean		flir;

	/* Draw map object name reference structures, used by
	 * SARDrawMap()
	 */
	sar_drawmap_objname_struct	**drawmap_objname;
	int		total_drawmap_objnames;
	/* Draw map ground hit contact result, a value from 0.0 to
	 * 1.0 which corresponds to the alpha value from reading
	 * back the pixels
	 */
	float		drawmap_ghc_result;

	/* Text input prompt */
	text_input_struct	*text_input;
	void		(*text_input_cb)(
		void *,			/* Data */
		const char *		/* Value */
	);

	/* Options */
	sar_option_struct	option;

	/* Frames Per Second counter
	 *
	 * Note that that we have one FPS per Graphics Wrapper
	 * (which we only have one) so we only have one FPS
	 */
	sar_fps_struct	fps;

} sar_core_struct;

#define SAR_CORE(p)	((sar_core_struct *)(p))


/*
 *	Client data for menu list widget/object items.
 */
typedef struct {

	void		*core_ptr;
	char		*name;
	char		*filename;	/* For items that have files */
	sar_position_struct	pos;	/* For items that have locations */
	sar_direction_struct	dir;

} sar_menu_list_item_data_struct;

#define SAR_MENU_LIST_ITEM_DATA(p)	((sar_menu_list_item_data_struct *)(p))


/*
 *	Callback data for load progress menu updates:
 */
typedef struct {

	void		*core_ptr;
	float		coeff_offset;	/* From 0.0 to 1.0 */
	float		coeff_range;	/* From 0.0 to 1.0 */
	Boolean		can_abort;	/* True if operation can be aborted */

} sar_progress_cb_struct;

#define SAR_PROGRESS_CB(p)	((sar_progress_cb_struct *)(p))



extern float	debug_value;		/* Arbitary global debug value */

extern int	runlevel;		/* Program run level from 0 to 4 */

extern time_t	cur_millitime,		/* Current time in ms */
		cur_systime,		/* Current time in systime seconds */
		lapsed_millitime;	/* Delta ms between main loop */

extern float	time_compensation,	/* Coeff of delta ms of loop vs 1 second */
		time_compression;	/* Slow motion or acceleration, 0.25 to 8.0 */



/* In main.c */
extern void SARHandleSignal(int s);

extern int SARInitGCTL(sar_core_struct *core_ptr);

extern void SARFullScreen(sar_core_struct *core_ptr);
extern void SARResolution(gw_display_struct *display, int width, int height);
extern void SARResolutionIncrease(gw_display_struct *display);
extern void SARResolutionDecrease(gw_display_struct *display);

extern int SARLoadProgressCB(void *ptr, long pos, long size);

extern void SARTextInputCBSendMessage(const char *value, void *data);
extern void SARTextInputCBQuitSimulation(const char *value, void *data);

extern void SARDrawCB(int ctx_num, void *ptr);
extern void SARReshapeCB(int ctx_num, void *ptr, int x, int y, int width, int height);
extern void SARKeyBoardCB(
	void *ptr, int c, Boolean state, unsigned long t);
extern void SARPointerCB(
	int ctx_num, void *ptr,
	int x, int y, gw_event_type type, int btn_num, unsigned long t
);
extern void SARVisibilityCB(int ctx_num, void *ptr, gw_visibility v);
extern void SARSaveYourselfCB(int ctx_num, void *ptr);
extern void SARCloseCB(int ctx_num, void *ptr, void *data);
extern void SARResetTimmersCB(sar_core_struct *core_ptr, time_t t_new);

extern sar_core_struct *SARInit(int argc, char **argv);
extern void SARManage(void *ptr);
extern void SARShutdown(sar_core_struct *core_ptr);

/* In mission.c */
extern sar_mission_struct *SARMissionNew(void);
extern void SARMissionDelete(sar_mission_struct *mission);

extern void SARMissionPrintStats(
	sar_core_struct *core_ptr,
	sar_scene_struct *scene,
	sar_mission_struct *mission,
	sar_object_struct *obj_ptr
);

extern void SARMissionDestroyNotify(
	sar_core_struct *core_ptr, sar_object_struct *obj_ptr
);
extern void SARMissionHoistInNotify(
	sar_core_struct *core_ptr, sar_object_struct *obj_ptr,
	int hoisted_in		/* Number of humans hoisted in */
);
extern void SARMissionPassengersEnterNotify(
	sar_core_struct *core_ptr, sar_object_struct *obj_ptr,
	int passengers_entered	/* Number of passengers entered */
);
extern void SARMissionPassengersLeaveNotify(
	sar_core_struct *core_ptr, sar_object_struct *obj_ptr,
	int passengers_left	/* Number of passengers that left */
);
extern void SARMissionLandNotify(
	sar_core_struct *core_ptr, sar_object_struct *obj_ptr,
	const int *ground_contact, int total_ground_contacts
);
extern int SARMissionManage(sar_core_struct *core_ptr);

/* In sarutils.c (sar specific utility functions) */
extern Boolean SARGetGLVersion(int *major, int *minor, int *release);
extern char *SARGetGLVendorName(void);
extern char *SARGetGLRendererName(void);
extern char **SARGelGLExtensionNames(int *strc);

extern int SARIsMenuAllocated(sar_core_struct *core_ptr, int n);
extern int SARMatchMenuByName(
	sar_core_struct *core_ptr, const char *name
);
extern sar_menu_struct *SARMatchMenuByNamePtr(
	sar_core_struct *core_ptr, const char *name
);
extern sar_menu_struct *SARGetCurrentMenuPtr(sar_core_struct *core_ptr);

extern sar_player_stat_struct *SARGetPlayerStatPtr(
	sar_core_struct *core_ptr, int i
);
extern sar_player_stat_struct *SARPlayerStatNew(
	const char *name,
	int score
);
extern void SARPlayerStatDelete(sar_player_stat_struct *pstat);
extern void SARPlayerStatResort(sar_player_stat_struct **list, int total);
extern sar_player_stat_struct *SARPlayerStatCurrent(
        sar_core_struct *core_ptr, int *n
);

extern void SARDeleteListItemData(
	sar_menu_list_item_struct *item_ptr
);

extern char *SARTimeOfDayString(sar_core_struct *core_ptr, float t);
extern char *SARDeltaTimeString(sar_core_struct *core_ptr, time_t t);

extern void SARSetGlobalTextColorBrightness(
	sar_core_struct *core_ptr, float g
);
extern void SARReportGLError(
	sar_core_struct *core_ptr, GLenum error_code
);

/* In sceneio.c (scenery file io) */
extern void SARSceneDestroy(    
	sar_core_struct *core_ptr,
	sar_scene_struct *scene,  
	sar_object_struct ***ptr,
	int *total
);
extern void SARSceneLoadLocationsToList(
	sar_core_struct *core_ptr,
	sar_menu_struct *menu_ptr,
	sar_menu_list_struct *list_ptr,
	int list_num,
	const char *filename
);
extern int SARSceneAddPlayerObject(
	sar_core_struct *core_ptr,
	sar_scene_struct *scene,
	const char *model_file,
	sar_position_struct *pos,
	sar_direction_struct *dir
);
extern int SARSceneLoadFromFile(
	sar_core_struct *core_struct,   
	sar_scene_struct *scene,
	const char *filename,
	const char *weather_preset_name,
	void *client_data,
	int (*progress_func)(void *, long, long)
);


#endif	/* SAR_H */
