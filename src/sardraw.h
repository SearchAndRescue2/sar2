/*
	                   Scene & Object Drawing
 */

#ifndef SARDRAW_H
#define SARDRAW_H

#include "gw.h"
#include "obj.h"
#include "sar.h"

/*
 *	Drawing context:
 *
 *	This structure is passed around in the SARDraw*() functions
 *	starting from SARDraw() or SARDrawMap(), it contains values
 *	that other SARDraw*() functions can conviently obtain and
 *	modify while sharing with other SARDraw*() functions.
 */
typedef struct {

	sar_core_struct		*core_ptr;	/* Core */
	sar_option_struct	*option;	/* Options (read-only) */
	sar_scene_struct	*scene;		/* Scene */
	sar_object_struct	**object;	/* Objects list */
	int			total_objects;
	gw_display_struct	*display;	/* Graphics Wrapper */
	gctl_struct		*gctl;		/* Game Controller */

	int		width,		/* Size of the GL Context in pixels */
			height;

	float		visibility_max;	/* Far Clip, in meters */
	float		view_aspect;	/* Width / height (with the aspect
					 * offset applied) */
	float		fovz_um;	/* Field Of View in unit meters
					 * about the Z axis, this is a
					 * coefficient that produces the width
					 * of the viewport at any given
					 * distance away from it, where:
				         * view_width_m = distance_m *
					 * fovz_um */

	Boolean		camera_in_cockpit,	/* Camera in cockpit hint */
			ear_in_cockpit,		/* Ear in cockpit hint */
			flir;			/* FLIR (night vision) hint */

	sar_camera_ref	camera_ref;
	sar_position_struct	camera_pos;
	sar_direction_struct	camera_dir;

	float		map_dxm,	/* Map view visible size, in meters */
			map_dym;

	sar_position_struct	primary_light_pos;

	GLfloat			sky_color[3],
				atmosphere_color[3],
				light_color[3];	/* Scene light color (not sun) */

	sar_cloud_layer_struct	*lowest_cloud_layer_ptr,
				*highest_cloud_layer_ptr;

	sar_object_struct	*player_obj_cockpit_ptr;

	sar_flight_model_type	player_flight_model_type;
	int		player_wheel_brakes;	/* 0 = off
						 * 1 = on
						 * 2 = parking */
	Boolean		player_air_brakes,
			player_autopilot,
			player_stall,
			player_overspeed;

} sar_dc_struct;

#define SAR_DC(p)		((sar_dc_struct *)(p))


/* sardrawhelipad. */
extern void SARDrawHelipad(
	sar_dc_struct *dc, sar_object_struct *obj_ptr,
	sar_object_helipad_struct *helipad,
	float distance
);
extern void SARDrawHelipadMap(
	sar_dc_struct *dc, sar_object_struct *obj_ptr,
	sar_object_helipad_struct *helipad,
	float icon_len
);

/* sardrawhuman.c */
extern void SARDrawHumanIterate(
	sar_dc_struct *dc,
	float height, float mass,
	sar_human_flags flags,
	const sar_color_struct *palette,	/* Human colors */
	int water_ripple_tex_num,
	sar_grad_anim_t anim_pos
);
extern void SARDrawHuman(
	sar_dc_struct *dc, sar_object_struct *obj_ptr,
	sar_object_human_struct *human
);

/* sardrawrunway.c */
extern void SARDrawRunway(
	sar_dc_struct *dc, sar_object_struct *obj_ptr,
	sar_object_runway_struct *runway,
	float distance
);
extern void SARDrawRunwayMap(
	sar_dc_struct *dc, sar_object_struct *obj_ptr,
	sar_object_runway_struct *runway,
	float icon_len
);

/* sardrawmessages.c */
extern void SARDrawHelp(sar_dc_struct *dc);
extern void SARDrawMessages(sar_dc_struct *dc);
extern void SARDrawBanner(sar_dc_struct *dc);
extern void SARDrawCameraRefTitle(sar_dc_struct *dc);
extern void SARDrawControlMessages(sar_dc_struct *dc);

/* sardrawutils.c */
extern void SARDrawGetDirFromPos(
	const sar_position_struct *pos1, const sar_position_struct *pos2,
	sar_direction_struct *dir_rtn
);
extern void SARDrawSetColor(const sar_color_struct *c);
extern void SARDrawSetColorFLIRTemperature(float t);
extern void SARDrawBoxNS(float x_len, float y_len, float z_len);
extern void SARDrawBoxBaseNS(
	float x_len, float y_len, float z_len,
	Boolean draw_base
);

/* sardraw.c */
extern void SARDraw(sar_core_struct *core_ptr);
extern void SARDrawMap(
	sar_core_struct *core_ptr,
	Boolean draw_for_gcc, Boolean draw_for_ghc, int gcc_obj_num
);

#endif	/* SARDRAW_H */
