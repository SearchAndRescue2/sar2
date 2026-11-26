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

#include <SDL2/SDL.h>
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
 *	Joystick Axis Inversion Bits:
 */
typedef enum {
	GCTL_JS_AXIS_INV_HEADING		= (1 << 0),
	GCTL_JS_AXIS_INV_PITCH			= (1 << 1),
	GCTL_JS_AXIS_INV_BANK			= (1 << 2),
	GCTL_JS_AXIS_INV_THROTTLE		= (1 << 3),
	GCTL_JS_AXIS_INV_HAT_X			= (1 << 4),
	GCTL_JS_AXIS_INV_HAT_Y			= (1 << 5),
	GCTL_JS_AXIS_INV_POV_HAT		= (1 << 6),
	GCTL_JS_AXIS_INV_BRAKE_LEFT		= (1 << 7),
	GCTL_JS_AXIS_INV_BRAKE_RIGHT		= (1 << 8)
} gctl_js_axes_inversion_bits;


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
	/* Pointer to the toplevel window handle (for Win32) */
	void		*window;

	/* SDL_GUID, in ASCII string representation.
	 * If this string is filled, it means that joystick is mapped
	 * to at least one role.
	 */
	char		sdl_guid_s[32+1];
	/* Joystick name.
	 * Name is only given to help player for joystick configuration,
	 * no treatment is made on it.
	 */
	char		*sdl_js_name;

	/* Axes inversion bits. Bit 0 for axis role 0, and so on...
	 * Each corresponding bit is set if axis movement must be inverted.
	 */
	gctl_js_axes_inversion_bits	js_axes_inversion_bits;

	/* Joystick axes mappings, indicates which joystick axis
	 * number corresponds with which role
	 */
	int		axis_heading,
			axis_pitch,
			axis_bank,
			axis_throttle,
			axis_hat_x,
			axis_hat_y,
			pov_hat,
			axis_brake_left,
			axis_brake_right;

	/* Joystick button mappings, indicates which joystick button
	 * number corresponds with which role
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
	/* SDL_GUID, in ASCII string representation.
	 * If this string is filled, it means that joystick is mapped
	 * to at least one role.
	 */
	char		sdl_guid_s[32+1];
	/* Joystick name.
	 * Name is only given to help player for joystick configuration,
	 * no treatment is made on it.
	 */
	char		*sdl_js_name;

	/* Joystick axis role mappings, reffers an axis by number to
	 * a specific role (-1 for none)
	 */
	int		axis_heading,
			axis_bank,
			axis_pitch,
			axis_throttle,
			axis_hat_x,
			axis_hat_y,
			pov_hat,
			axis_brake_left,
			axis_brake_right;

	/* Axes inversion bits. Bit 0 for axis role 0, and so on...
	 * Each corresponding bit is set if axis movement must be inverted.
	 */
	int		axes_inversion_bits;

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
 *	Used in gctl_struct.
 *	Only defined when mapping menu is activated and for each axis
 *	of each joystick (8 axes * 3 joysticks).
 */
typedef struct {
	/* Joysticks axes min and max reached values. */
	Sint16		min,
			max;
} gctl_js_mapping_menu_data_struct;
#define GCTL_JS_MAPPING_MENU_DATA(p)	((gctl_js_mapping_menu_data_struct *)(p))


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
	 * 	* air_brakes_js_btn_released is used for joystick air brakes
	 * 	button rising edge detection.
	 *	*_coeff members determines the magnitude [0.0 to 1.0] of
	 *	the state regardless if it is on or off. This is for when
	 *	the game controller has gone from off to on to off in one
	 *	call to GCtlUpdate()
	 */
	Boolean		air_brakes_state,
			air_brakes_js_btn_released;
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

	/* Maximum number of joysticks and maximum number of analog axes,
	 * buttons and hats per joystick.
	 * These numbers cannot be modified without modifying a lot of code:
	 * opt->jsX, mapping menu spins, option spins call back, and so on...
	 *
	 * Three joysticks can hold:
	 *	Flight stick
	 *	Throttle controller
	 *	Rudder controller
	 */
#define MAX_JOYSTICKS 3
#define MAX_JOYSTICK_AXES 8
#define MAX_JOYSTICK_BTNS 16
#define MAX_JOYSTICK_HATS 2

	/* Each structure reffers to a joystick */
	gctl_js_struct	*joystick;
	int		total_joysticks;
	/* we add a an array for SDL joystick here. */
	SDL_Joystick	**sdljoystick;
	int		total_sdl_joysticks;

	/* Used only in mapping menu for axes movement detection. */
	gctl_js_mapping_menu_data_struct    	*js_mapping_axes_values;
} gctl_struct;
#define GCTL(p)		((gctl_struct *)(p))


extern char *GCtlGetError(void);
extern void GctlJsOpenAndMapp(gctl_struct *gc, gctl_values_struct *v);
extern int GetSdlJsIndexFromGuidString(char *guid);
extern int GetSdlJsIndexFromGcSdlJoystick(gctl_struct *gc, int i);
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
