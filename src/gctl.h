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
			       Game Controller
 */

#ifndef GCTL_H
#define GCTL_H

#include <SDL/SDL.h>
#include <sys/types.h>
#include "gw.h"


/*
 *	Game Controller:
 *
 *	Determines which controller(s) are in use or to be used.
 */
typedef enum {
	GCTL_CONTROLLER_KEYBOARD		= (1 << 0),
	GCTL_CONTROLLER_POINTER			= (1 << 1),
	GCTL_CONTROLLER_JOYSTICK		= (1 << 2)
} gctl_controllers;


/*
 *	Options:
 */
typedef enum {
	GCTL_OPTION_NONE			= (1 << 0)
} gctl_options;


/*
 *	Joystick Priorities:
 */
typedef enum {
	GCTL_JS_PRIORITY_BACKGROUND,
	GCTL_JS_PRIORITY_FOREGROUND,
	GCTL_JS_PRIORITY_PREEMPT
} gctl_js_priority;

/*
 *	Joystick Connections:
 */
typedef enum {
	GCTL_JS_CONNECTION_STANDARD		= 0,
	GCTL_JS_CONNECTION_USB			= 1
} gctl_js_connection;

/*
 *	Joystick Axis Roles:
 */
typedef enum {
	GCTL_JS_AXIS_ROLE_PITCH			= (1 << 0),
	GCTL_JS_AXIS_ROLE_BANK			= (1 << 1),
	GCTL_JS_AXIS_ROLE_HEADING		= (1 << 2),
	GCTL_JS_AXIS_ROLE_THROTTLE		= (1 << 3),
	GCTL_JS_AXIS_ROLE_HAT			= (1 << 4),
	/* Throttle and rudder unit joystick */
	GCTL_JS_AXIS_ROLE_AS_THROTTLE_AND_RUDDER	= (1 << 6),
	/* Joystick unit with no axises (only buttons) */
	GCTL_JS_AXIS_ROLE_NONE			= (1 << 7)
} gctl_js_axis_roles;


/*
 *	Pointer Buttons:
 */
typedef enum {
	GCTL_POINTER_BUTTON1			= (1 << 0),
	GCTL_POINTER_BUTTON2			= (1 << 1),
	GCTL_POINTER_BUTTON3			= (1 << 2)
} gctl_pointer_buttons;


/*
 *	Game Controller Joystick Values:
 */
typedef struct {

	/* Abstract string idenifying the device.
	 *
	 * When using libjsw this would be "/dev/js#" (where # is a
	 * number) or "/dev/input/js#" for libjsw USB devices
	 */
	char		*device;

	/* Priority, one of GCTL_JS_PRIORITY_* */
	gctl_js_priority	priority;

	/* Standard or USB connection, one of GCTL_JS_CONNECTION_* */
	gctl_js_connection	connection;

	/* Pointer to the toplevel window handle (for Win32) */
	void		*window;

	/* Axis role flags (any of GCTL_JS_AXIS_ROLE_*), this basically
	 * specifies how many axises there are and each of their roles
	 */
	gctl_js_axis_roles	axis_role;

	/* Joystick button mappings, indicates which joystick button
	 * number corresponds with which action
	 */
	int		button_rotate,		/* Button that treats
						 * bank as heading axis
						 */
			button_air_brakes,
			button_wheel_brakes,
			button_zoom_in,
			button_zoom_out,
			button_hoist_up,
			button_hoist_down;

} gctl_values_js_struct;
#define GCTL_VALUES_JS(p)	((gctl_values_js_struct *)(p))

/*
 *	Game Controller Values:
 */
typedef struct {

	/* Specifies which game controller type to check for (any of
	 * GCTL_CONTROLLER_*)
	 */
	gctl_controllers	controllers;

	/* General options (any of GCTL_OPT_*) */
	gctl_options		options;

	/* Joystick values, each structure reffers to a joystick */
	gctl_values_js_struct	*joystick;
	int			total_joysticks;

} gctl_values_struct;
#define GCTL_VALUES(p)	((gctl_values_struct *)(p))


/*
 *	Game Controller Joystick:
 *
 *	Used in gctl_struct
 */
typedef struct {

	/* Joystick device specific data handle
	 *
	 * For example, when using libjsw the void *joystick would be
	 * pointing to a js_data_struct structure
	 */
	void		*data;

	/* Axis role mappings, reffers an axis by number to a specific
	 * role
	 *
	 * The axis number can be -1 for non-existant
	 */
	int		axis_heading,
			axis_bank,
			axis_pitch,
			axis_throttle,
			axis_hat_x,
			axis_hat_y;

	/* Joystick button role mappings, reffers a button by number to
	 * a specific role (-1 for none)
	 */
	int		button_rotate,		/* Button that treats
						 * bank as heading axis
						 */
			button_air_brakes,
			button_wheel_brakes,
			button_zoom_in,
			button_zoom_out,
			button_hoist_up,
			button_hoist_down;

} gctl_js_struct;
#define GCTL_JS(p)	((gctl_js_struct *)(p))

/*
 *	Game Controller:
 *
 *	Contains controller positions, values, and resources.
 *
 *	This is passed to all GCTL*() functions.
 */
typedef struct {

	/* Specifies which controllers are checked for, any of
	 * GCTL_CONTROLLER_*
	 */
	gctl_controllers	controllers;

	/* General options (any of GCTL_OPT_*) */
	gctl_options		options;

	/* Last time GCtlUpdate() was called passing this structure,
	 * in milliseconds
	 */
	time_t		last_updated;

	/* Control positions */
	float		heading,	/* -1.0 to 1.0 */
			pitch,		/* -1.0 to 1.0 */
			bank,		/* -1.0 to 1.0 */
			throttle,	/*  0.0 to 1.0 */
			hat_x,		/* -1.0 to 1.0 */
			hat_y;		/* -1.0 to 1.0 */

	/* Zoom in and out:
	 *      *_state members specifies a boolean on/off value
	 *      *_kb_last members record the last keyboard event for this
	 *      behavour
	 *      *_coeff members determines the magnitude [0.0 to 1.0] of
	 *      the state regardless if it is on or off. This is for when
	 *      the game controller has gone from off to on to off in one
	 *      call to GCtlUpdate()
	 */
	Boolean		zoom_in_state,
			zoom_out_state;
	time_t		zoom_in_kb_last,	/* In milliseconds */
			zoom_out_kb_last;
	float		zoom_in_coeff,		/* 0.0 to 1.0 */
			zoom_out_coeff;

	/* Hoist up and down:
	 *      *_state members specifies a boolean on/off value.
	 *      *_kb_last members record the last keyboard event for this
	 *      behavour.
	 *      *_coeff members determines the magnitude [0.0 to 1.0] of
	 *      the state regardless if it is on or off. This is for when
	 *      the game controller has gone from off to on to off in one
	 *      call to GCtlUpdate().
	 */
	Boolean		hoist_up_state,
			hoist_down_state;
	time_t		hoist_up_kb_last,	/* In milliseconds */
			hoist_down_kb_last;
	float		hoist_up_coeff,		/* 0.0 to 1.0 */
			hoist_down_coeff;

	/* Alt, ctrl, and shift (not nessasarly keyboard) states */
	Boolean		alt_state,
			ctrl_state,
			shift_state;

	/* Wheel and air brakes:
	 *	*_state members specifies a boolean on/off value
	 *	*_kb_last members record the last keyboard event for this
	 *	behavior
	 *	*_coeff members determines the magnitude [0.0 to 1.0] of
	 *	the state regardless if it is on or off. This is for when
	 *	the game controller has gone from off to on to off in one
	 *	call to GCtlUpdate()
	 */
	Boolean		air_brakes_state;
	int		wheel_brakes_state;	/* 0 = off
						 * 1 = on
						 * 2 = parking brakes
						 */
	time_t		air_brakes_kb_last,	/* In milliseconds */
			wheel_brakes_kb_last;
	float		air_brakes_coeff,	/* 0.0 to 1.0 */
			wheel_brakes_coeff;


	/* Keyboard Specific Values */

	/* Keyboard key states */
	Boolean		heading_kb_state,
			pitch_kb_state,
			bank_kb_state,
			throttle_kb_state,
			hat_x_kb_state,
			hat_y_kb_state;

	/* Keyboard last press time stamps for control positions */
	time_t		heading_kb_last,	/* In milliseconds */
			pitch_kb_last,
			bank_kb_last,
			throttle_kb_last,
			hat_x_kb_last,
			hat_y_kb_last;

	/* Set to True if a key is currently down and that key
	 * would/should override a joystick or pointer controlled
	 * axis.
	 */
	Boolean		axis_kb_state;

	/* Set to True if a key is currently down and that key
	 * would/should override a joystick or pointer controlled
	 * button
	 */
	Boolean		button_kb_state;


	/* Pointer Specific Values */

	/* Current pressed pointer button(s), any flag set to true
	 * means that button is pressed
	 */
	gctl_pointer_buttons	pointer_buttons;

	/* Pointer box position and size in window coordinates, upper-left
	 * corner oriented
	 */
	int		pointer_box_x,
			pointer_box_y,
			pointer_box_width,
			pointer_box_height;

	/* Last pointer event coordinates, in window coordinates relative
	 * to pointer_box_x and pointer_box_y (upper-left corner oriented)
	 */
	int		pointer_x,
			pointer_y;


	/* Joystick specific stuff */

	/* Each structure reffers to a joystick */
	gctl_js_struct	*joystick;
    int		total_joysticks;
    /* we add a an array for SDL joystick here. */
    SDL_Joystick **sdljoystick;

} gctl_struct;
#define GCTL(p)		((gctl_struct *)(p))


extern char *GCtlGetError(void);
extern gctl_struct *GCtlNew(gctl_values_struct *v);
extern void GCtlUpdate(
	gctl_struct *gc,
	Boolean heading_has_nz,		/* Include heading pitch nullzone */
	Boolean pitch_has_nz,		/* Include pitch nullzone */
	Boolean bank_has_nz,		/* Include bank nullzone */
	time_t cur_ms, time_t lapsed_ms,
	float time_compensation
);
extern void GCtlHandlePointer(
	gw_display_struct *display, gctl_struct *gc,
	gw_event_type type,
	int button,		/* Button number */
	int x, int y,
	time_t t,		/* Time stamp (in ms) */
	time_t lapsed_ms
);
extern void GCtlHandleKey(
	gw_display_struct *display, gctl_struct *gc,
	int k, Boolean state,
	Boolean alt_state, Boolean ctrl_state, Boolean shift_state,
	time_t t,			/* Time stamp (in ms) */
	time_t lapsed_ms
);
extern void GCtlResetValues(gctl_struct *gc);
extern void GCtlResetTimmers(gctl_struct *gc);
extern void GCtlDelete(gctl_struct *gc);


#endif	/* GCTL_H */
