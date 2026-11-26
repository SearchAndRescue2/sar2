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
                            SAR Menus and Menu Objects
 */

#ifndef MENU_H
#define MENU_H

#include <sys/types.h>
#include "v3dtex.h"
#include "gw.h"
#include "image.h"


/*
 *	Flags:
 */
#define sar_menu_flags_t	unsigned long

/*
 *	Menu Types:
 */
typedef enum {
	SAR_MENU_TYPE_STANDARD
} sar_menu_type;

/*
 *	Object Types:
 */
typedef enum {
	SAR_MENU_OBJECT_TYPE_LABEL,
	SAR_MENU_OBJECT_TYPE_BUTTON,
	SAR_MENU_OBJECT_TYPE_PROGRESS,
        SAR_MENU_OBJECT_TYPE_MESSAGE_BOX,
        SAR_MENU_OBJECT_TYPE_LIST,
        SAR_MENU_OBJECT_TYPE_MDISPLAY,
        SAR_MENU_OBJECT_TYPE_SWITCH,
        SAR_MENU_OBJECT_TYPE_SPIN,
        SAR_MENU_OBJECT_TYPE_SLIDER,
        SAR_MENU_OBJECT_TYPE_MAP,
        SAR_MENU_OBJECT_TYPE_OBJVIEW
} sar_menu_object_type;
#define SAR_MENU_OBJECT_TYPE(p)	(((p) != NULL) ? \
 (*(sar_menu_object_type *)p) : -1)


/*
 *	Color:
 */
typedef struct {
	float	a, r, g, b;	/* 0.0 to 1.0 */
} sar_menu_color_struct;
#define SAR_MENU_COLOR(p)	((sar_menu_color_struct *)(p))


/*
 *	Label:
 *
 *	A plain label with an optional background image.
 */
typedef struct {

	sar_menu_object_type	type;	/* SAR_MENU_OBJECT_TYPE_LABEL */
	int	id;		/* Value used to identify this object */
	void	*client_data;	/* User data */

	float	x, y;		/* Position coefficient */
	int	offset_x,	/* Offset from center in pixels */
		offset_y;
	int	width,		/* Size in pixels */
		height;
	Boolean	sensitive;
	const sar_image_struct *image;	/* Shared bg image */

	GWFont	*font;
	sar_menu_color_struct	color;
	char	*label;
#define SAR_MENU_LABEL_ALIGN_CENTER	0
#define SAR_MENU_LABEL_ALIGN_LEFT	1
#define SAR_MENU_LABEL_ALIGN_RIGHT	2
	int	align;		/* One of SAR_MENU_LABEL_ALIGN_* */

} sar_menu_label_struct;
#define SAR_MENU_LABEL(p)	((sar_menu_label_struct *)(p))
#define SAR_MENU_IS_LABEL(p)	(SAR_MENU_OBJECT_TYPE(p) == SAR_MENU_OBJECT_TYPE_LABEL)

/*
 *	Button:
 *
 *	Push button with label, the label is to the right of the button.
 *	Both the button and label are of fixed width.
 */
typedef struct {

	sar_menu_object_type	type;	/* SAR_MENU_OBJECT_TYPE_BUTTON */
	int	id;		/* Value used to identify this object */
	void	*client_data;	/* User data */

	float	x, y;		/* Position coefficient */
	int	offset_x,	/* Offset from center in pixels */
		offset_y;
	int	width,		/* Size in pixels */
		height;
	Boolean	sensitive;

#define SAR_MENU_BUTTON_STATE_UNARMED		0
#define SAR_MENU_BUTTON_STATE_ARMED		1
#define SAR_MENU_BUTTON_STATE_HIGHLIGHTED	2
	int	state;		/* One of SAR_MENU_BUTTON_STATE_* */
	const sar_image_struct	*unarmed_image,		/* Shared images */
				*armed_image,
				*highlighted_image;

	GWFont	*font;
	sar_menu_color_struct	color;
	char	*label;		/* Button label */

	void	(*func_cb)(
		void *,		/* This Object */
		int,		/* ID Code */
		void *		/* Data */
	);

} sar_menu_button_struct;
#define SAR_MENU_BUTTON(p)	((sar_menu_button_struct *)(p))
#define SAR_MENU_IS_BUTTON(p)	(SAR_MENU_OBJECT_TYPE(p) == SAR_MENU_OBJECT_TYPE_BUTTON)

/*
 *	Progress Bar:
 *
 *	A left to right progress bar, the width and height can be -1 to
 *	indicate the entire width of the window and a default height
 *	(respectively).
 */
typedef struct {

	sar_menu_object_type	type;	/* SAR_MENU_OBJECT_TYPE_PROGRESS */
	int	id;		/* Value used to identify this object */
	void	*client_data;	/* User data */

	float	x, y;		/* Position coefficient */
	int	width,		/* Size in pixels */
		height;
	Boolean	sensitive;

	const sar_image_struct	*bg_image,	/* Shared images */
				*fg_image;

	GWFont	*font;
	sar_menu_color_struct	color;
	char	*label;

	float	progress;	/* 0.0 to 1.0 */

} sar_menu_progress_struct;
#define SAR_MENU_PROGRESS(p)	((sar_menu_progress_struct *)(p))
#define SAR_MENU_IS_PROGRESS(p)	(SAR_MENU_OBJECT_TYPE(p) == SAR_MENU_OBJECT_TYPE_PROGRESS)

/*
 *	Message Box:
 *
 *	A message box displaying several lines of text and capable of
 *	scrolling.  Uses XML-like tags for font color and style.
 */
typedef struct {

	sar_menu_object_type	type;	/* SAR_MENU_OBJECT_TYPE_MESSAGE_BOX */
	int	id;		/* Value used to identify this object */
	void	*client_data;	/* User data */

	float	x, y;		/* Position coefficient */
	float	width,		/* Size coefficient */
		height;
	Boolean	sensitive;

	GWFont	*font;
	sar_menu_color_struct	color,		/* Foreground */
				bg_color,	/* Background */
				bold_color,
				underline_color;

#define MENU_MESSAGE_BOX_BG_TOTAL_IMAGES	9
	const sar_image_struct	**bg_image;	/* Pointer to array of 9 shared images */

	int	scrolled_line;
	char	*message;

} sar_menu_message_box_struct;
#define SAR_MENU_MESSAGE_BOX(p)	((sar_menu_message_box_struct *)(p))
#define SAR_MENU_IS_MESSAGE_BOX(p)	(SAR_MENU_OBJECT_TYPE(p) == SAR_MENU_OBJECT_TYPE_MESSAGE_BOX)

/*
 *	List Item:
 */
typedef struct {

	sar_menu_flags_t	flags;
	char	*name;
	void	*client_data;	/* User data */

} sar_menu_list_item_struct;
#define SAR_MENU_LIST_ITEM(p)	((sar_menu_list_item_struct *)(p))

/*
 *	List:
 *
 *	A list displaying items and capable of scrolling.
 */
typedef struct {

	sar_menu_object_type	type;	/* SAR_MENU_OBJECT_TYPE_LIST */
	int	id;		/* Value used to identify this object */
	void	*client_data;	/* User data */

	float	x, y;		/* Position coefficient */
	float	width,		/* Size coefficient */
		height;
	Boolean	sensitive;

#define MENU_LIST_BG_TOTAL_IMAGES	9
	const sar_image_struct	**bg_image;	/* Pointer to array of 9 shared images */

	GWFont	*font;
	sar_menu_color_struct	color;
	char	*label;

	sar_menu_list_item_struct	**item;
	int	total_items,
		items_visable,	/* Number of items visible */
		scrolled_item,	/* First visible item */
		selected_item;	/* Selected item */

	void	(*select_cb)(
		void *,		/* This Object */
		int,		/* ID Code */
		void *,		/* Data */
		int,		/* Item Number */
		void *,		/* Item Pointer */
		void *		/* Item Data */
	);

	void	(*activate_cb)(
		void *,		/* This Object */
		int,		/* ID Code */
		void *,		/* Data */
		int,		/* Item Number */
		void *,		/* Item Pointer */
		void *		/* Item Data */
	);

} sar_menu_list_struct;
#define SAR_MENU_LIST(p)	((sar_menu_list_struct *)(p))
#define SAR_MENU_IS_LIST(p)	(SAR_MENU_OBJECT_TYPE(p) == SAR_MENU_OBJECT_TYPE_LIST)

/*
 *	Multi-Purpose DIsplay:
 *
 *	This is basically a drawing area that allows you to draw
 *	anything you want on to it when the draw_cb() is called.
 */
typedef struct {

	sar_menu_object_type	type;	/* SAR_MENU_OBJECT_TYPE_MDISPLAY */
	int	id;		/* Value used to identify this object */
	void	*client_data;	/* User data */

	float	x, y;		/* Position coefficient */
	float	width,		/* Size coefficient */
		height;
	Boolean	sensitive;

#define MENU_MDISPLAY_BG_TOTAL_IMAGES	9
	const sar_image_struct	**bg_image;	/* Pointer to array of 9 shared images */

	void	(*draw_cb)(
		void *,		/* Display */
		void *,		/* Menu */
		void *,		/* This Object */
		int,		/* ID Code */
		void *,		/* Data */
		int, int, int, int	/* x_min, y_min, x_max, y_max */
	);

} sar_menu_mdisplay_struct;
#define SAR_MENU_MDISPLAY(p)	((sar_menu_mdisplay_struct *)(p))
#define SAR_MENU_IS_MDISPLAY(p)	(SAR_MENU_OBJECT_TYPE(p) == SAR_MENU_OBJECT_TYPE_MDISPLAY)

/*
 *	Switch:
 *
 *	A toggle switch that holds a boolean value.
 */
typedef struct {

	sar_menu_object_type	type;	/* SAR_MENU_OBJECT_TYPE_SWITCH */
	int	id;		/* Value used to identify this object */
	void	*client_data;	/* User data */

	float	x, y;		/* Position coefficient */
	int	width,		/* Size coefficient */
		height;
	Boolean	sensitive;

	const sar_image_struct	*bg_image,		/* Shared images */
				*switch_off_image,
				*switch_on_image;

	GWFont	*font;
	sar_menu_color_struct	color;
	char	*label;

	Boolean state;

	void	(*switch_cb)(
		void *,		/* This Object */
		int,		/* ID Code */
		void *,		/* Data */
		Boolean		/* Value */
	);

} sar_menu_switch_struct;
#define SAR_MENU_SWITCH(p)	((sar_menu_switch_struct *)(p))
#define SAR_MENU_IS_SWITCH(p)	(SAR_MENU_OBJECT_TYPE(p) == SAR_MENU_OBJECT_TYPE_SWITCH) 

/*
 *	Spin:
 *
 *	A box with an array of values that can be selected by
 *	"spinning".
 */
typedef struct {

	sar_menu_object_type	type;	/* SAR_MENU_OBJECT_TYPE_SPIN */
	int	id;		/* Value used to identify this object */
	void	*client_data;	/* User data */

	float	x, y;		/* Position coefficient */
	float	width,		/* Size coefficient */
		height;
	Boolean	sensitive;

	const sar_image_struct	*label_image,	/* Shared images */
				*value_image,
				*dec_armed_image,
				*dec_unarmed_image,
				*inc_armed_image,
				*inc_unarmed_image;

	GWFont	*font,		/* Label font */
		*value_font;	/* Value font */
	sar_menu_color_struct	label_color,
				value_color;

	int	dec_state,	/* One of SAR_MENU_BUTTON_STATE_* */
		inc_state;

	char	*label;

#define SAR_MENU_SPIN_TYPE_STRING	0
#define SAR_MENU_SPIN_TYPE_NUMERIC	1
	int	value_type;	/* One of SAR_MENU_SPIN_TYPE_* */

	Boolean	allow_warp;	/* Allow end to beginning cycle warping */

	float	step;	/* Increment step (for SAR_MENU_SPIN_TYPE_NUMERIC) */


	/* List of values, if the value_type is set to
	 * SAR_MENU_SPIN_TYPE_NUMERIC then there will only be one value.
	 * However if value_type is set to SAR_MENU_SPIN_TYPE_STRING then
	 * there may be one or more values in this list.
	 */
	char	**value;
	int	total_values,
		cur_value;

	void	(*change_cb)(
		void *,		/* This Object */
		int,		/* ID Code */
		void *,		/* Data */
		char *		/* Value */
	);

} sar_menu_spin_struct;
#define SAR_MENU_SPIN(p)	((sar_menu_spin_struct *)(p))
#define SAR_MENU_IS_SPIN(p)	(SAR_MENU_OBJECT_TYPE(p) == SAR_MENU_OBJECT_TYPE_SPIN)

/*
 *	Slider:
 *
 *	A "slideable" scale that can hold a gradient value.
 */
typedef struct {

	sar_menu_object_type	type;	/* SAR_MENU_OBJECT_TYPE_SLIDER */
	int	id;		/* Value used to identify this object */
	void	*client_data;	/* User data */

	float	x, y;		/* Position coefficient */
	float	width,		/* Size coefficient */
		height;
	Boolean	sensitive;

	const sar_image_struct	*label_image,	/* Shared images */
				*trough_image,
				*handle_image;

	GWFont	*font;
	sar_menu_color_struct	label_color;

	char	*label;

	float	value,		/* Current value */
		lower,		/* Lowest value */
		upper;		/* Highest value */

	int	drag_button;	/* Pointer button that is currently
				 * dragging or 0 for none
				 */

	void	(*change_cb)(
		void *,		/* This Object */
		int,		/* ID Code */
		void *,		/* Data */
		float		/* Value */
	);

} sar_menu_slider_struct;
#define SAR_MENU_SLIDER(p)	((sar_menu_slider_struct *)(p))
#define SAR_MENU_IS_SLIDER(p)	(SAR_MENU_OBJECT_TYPE(p) == SAR_MENU_OBJECT_TYPE_SLIDER)


/*
 *	Map Marking Types:
 */
typedef enum {
	SAR_MENU_MAP_MARKING_TYPE_ICON,
	SAR_MENU_MAP_MARKING_TYPE_INTERCEPT_LINE
} sar_menu_map_marking_type;

/*
 *	Map Marking:
 */
typedef struct {

	sar_menu_map_marking_type	type;
	sar_menu_color_struct	fg_color;
	float	x, y;		/* In meters */
	float	x_end, y_end;	/* In meters (for intercept line) */

	const sar_image_struct	*icon;	/* Shared icon image */
	char	*desc;		/* Description (preferably short one-liner) */

} sar_menu_map_marking_struct;
#define SAR_MENU_MAP_MARKING(p)	((sar_menu_map_marking_struct *)(p))

/*
 *	Map:
 */
typedef struct {

	sar_menu_object_type	type;	/* SAR_MENU_OBJECT_TYPE_MAP */
	int	id;		/* Value used to identify this object */
	void	*client_data;	/* User data */

	float	x, y;		/* Position coefficient */
	float	width,		/* Size coefficient */
		height;
	Boolean	sensitive;

	GWFont	*font;
	sar_menu_color_struct	color;

	const sar_image_struct	**bg_image;	/* Pointer to array of 9 shared images */

	char	*title;

	int	scroll_x,	/* Scroll position, upper left corner, in pixels */
		scroll_y;

	float	m_to_pixels_coeff;	/* Meters to pixels coefficient */ 

	v3d_texture_ref_struct	*bg_tex;	/* Background texture */
	float	bg_tex_width,	/* Intended size of texture in meters */
		bg_tex_height;


#define SAR_MENU_MAP_SHOW_MARKING_ALL		0	/* Show all markings */
#define SAR_MENU_MAP_SHOW_MARKING_SELECTED	1	/* Show only selected marking */
	int	show_markings_policy;	/* One of SAR_MENU_MAP_SHOW_MARKING_* */

	sar_menu_map_marking_struct	**marking;
	int	total_markings;

	/* Marking that pointer is over, show desc of this marking.
	 * Can be -1 for none
	 */
	int	selected_marking;

} sar_menu_map_struct;
#define SAR_MENU_MAP(p)		((sar_menu_map_struct *)(p))
#define SAR_MENU_IS_MAP(p)	(SAR_MENU_OBJECT_TYPE(p) == SAR_MENU_OBJECT_TYPE_MAP)


/*
 *	Object Viewer:
 *
 *	Displays an OpenGL object in 3D and allows interactive rotate
 *	and zoom.
 */
typedef struct {

	sar_menu_object_type	type;	/* SAR_MENU_OBJECT_TYPE_OBJVIEW */
	int	id;		/* Value used to identify this object */
	void	*client_data;	/* User data */

	float	x, y;		/* Position coefficient */
	float	width,		/* Size coefficient */
		height;
	Boolean	sensitive;

	GWFont	*font;
	sar_menu_color_struct	color,		/* Foreground */
				bg_color,	/* Background */
				bold_color,
				underline_color;

#define MENU_OBJVIEW_BG_TOTAL_IMAGES	9
	const sar_image_struct	**bg_image;	/* Pointer to array of 9 shared images */

	/* Pointer drag info */
	Boolean	rotate_drag_state,
		bank_drag_state;
	int	prev_motion_x, prev_motion_y;
	time_t	last_motion_time;

	/* Render options */
	Boolean	render_lighting,
		render_antialias;

	char	*title;
	char	*message;
	Boolean	show_message;

	void	*obj_list;	/* GL list of the 3d object */
	void	*obj_glres;	/* libv3d GL resources for the 3d
				 * object (v3d_glresource_struct *).
				 */

	float	camera_distance;	/* Camera distance from object in meters */
	float	obj_size_radius;	/* Size of object's radius in meters */

	/* Rotation of object in radians */
	float	obj_heading,
		obj_pitch,
		obj_bank;

	/* Position of light in radians */
	float	light_heading,
		light_pitch,
		light_bank;

	/* Last drawn gl image pixels of the 3d object, we buffer it
	 * instead of drawing the list each time. So we draw this image
	 * (which has transparent pixels) to speed things up. The format
	 * is GL_RGBA and the data type is GL_UNSIGNED_BYTE (so 4 bytes
	 * per pixel)
	 */
	void	*objimg;
	int	objimg_width,
		objimg_height;

} sar_menu_objview_struct;
#define SAR_MENU_OBJVIEW(p)	((sar_menu_objview_struct *)(p))
#define SAR_MENU_IS_OBJVIEW(p)	(SAR_MENU_OBJECT_TYPE(p) == SAR_MENU_OBJECT_TYPE_OBJVIEW) 

/*
 *	Menu:
 */
typedef struct {

	sar_menu_type	type;
	char	*name;		/* Name of this menu (for matching) */

	Boolean	always_full_redraw;	/* Always perform complete menu
					 * redraws even if only a part of
					 * the menu has changed (for some
					 * fixing problems on some of the
					 * accelerated video cards
					 */                       

	char	*bg_image_path;	/* Path to background image file */
	sar_image_struct	*bg_image;	/* Background image (not shared) */

	/* List of menu objects */
	void	**object;
	int	total_objects,
		selected_object;

} sar_menu_struct;
#define SAR_MENU(p)		((sar_menu_struct *)(p))


/* In menu.c */
/* Utility functions */
extern void SARMenuGLStateReset(gw_display_struct *display);
extern void *SARMenuGetObject(sar_menu_struct *m, int n);
extern void *SARMenuGetObjectByID(sar_menu_struct *m, int id, int *n);
extern Boolean SARMenuObjectIsSensitive(sar_menu_struct *m, int n);

/* Menu functions */
extern sar_menu_struct *SARMenuNew(
	sar_menu_type type, const char *name, const char *bg_image
);
extern void SARMenuLoadBackgroundImage(sar_menu_struct *m);
extern void SARMenuUnloadBackgroundImage(sar_menu_struct *m);
extern void SARMenuDelete(sar_menu_struct *m);

/* Label */
extern int SARMenuLabelNew(
	sar_menu_struct *m,
	float x, float y,
	int width, int height,
	const char *label,
	sar_menu_color_struct *fg_color, GWFont *font,
	const sar_image_struct *image
);
extern int SARMenuLabelSetLabel(
	gw_display_struct *display, sar_menu_struct *m, int n,
	const char *label,
	sar_menu_color_struct *fg_color,
	Boolean redraw
);

/* Button */
extern int SARMenuButtonNew(
	sar_menu_struct *m,
	float x, float y,
	int width, int height,
	const char *label,
	sar_menu_color_struct *fg_color,
	GWFont *font,
	const sar_image_struct *unarmed_image,
	const sar_image_struct *armed_image,
	const sar_image_struct *highlighted_image,
	void *client_data, int id,
	void (*func_cb)(void *, int, void *)
);

/* Progress Bar */
extern int SARMenuProgressNew(
	sar_menu_struct *m,
	float x, float y,
	int width, int height,
	const char *label,
	sar_menu_color_struct *fg_color,
	GWFont *font,
	const sar_image_struct *bg_image,
	const sar_image_struct *fg_image,
	float progress
);
extern void SARMenuProgressSet(
	gw_display_struct *display, sar_menu_struct *m, int n,
	float progress, Boolean redraw
);

/* Message Box */
extern int SARMenuMessageBoxNew(
	sar_menu_struct *m,
	float x, float y,
	float width, float height,
	sar_menu_color_struct *fg_color, GWFont *font,
	const sar_image_struct **bg_image,	/* Pointer to 9 images */
	void *client_data, int id,
	const char *message
);
extern void SARMenuMessageBoxSet(
	gw_display_struct *display, sar_menu_struct *m, int n,
	const char *message, Boolean redraw
);

/* List */
extern int SARMenuListNew(
	sar_menu_struct *m,
	float x, float y,
	float width, float height,
	sar_menu_color_struct *fg_color, GWFont *font,
	const char *label,
	const sar_image_struct **bg_image,
	void *client_data, int id,
	void (*select_cb)(void *, int, void *, int, void *, void *),
	void (*activate_cb)(void *, int, void *, int, void *, void *)
);
extern int SARMenuListAppendItem(
	sar_menu_struct *m, int n,
	const char *name,
	void *client_data,
	sar_menu_flags_t flags
);
extern sar_menu_list_item_struct *SARMenuListGetItemByNumber(
	sar_menu_list_struct *list, int i
);
extern void SARMenuListSelect(
	gw_display_struct *display, sar_menu_struct *m, int n,
	int i,
	Boolean scroll, Boolean redraw
);
extern void SARMenuListDeleteAllItems(
	sar_menu_struct *m,
	int n			/* List object number on m */
);

/* Multi-purpose Display */
extern int SARMenuMDisplayNew(
	sar_menu_struct *m,
	float x, float y,
	float width, float height,
	const sar_image_struct **bg_image,	/* Pointer to 9 images */
	void *client_data, int id,
	void (*draw_cb)(
		void *,         /* Display */
		void *,         /* Menu */
		void *,         /* This Object */
		int,            /* ID Code */
		void *,         /* Data */
		int, int, int, int      /* x_min, y_min, x_max, y_max */
	)
);
extern void SARMenuMDisplayDraw(
	gw_display_struct *display, sar_menu_struct *m, int n
);

/* Switch */
extern int SARMenuSwitchNew(
	sar_menu_struct *m,
	float x, float y,
	int width, int height,   
	sar_menu_color_struct *fg_color, GWFont *font,
	const char *label,
	const sar_image_struct *bg_image,
	const sar_image_struct *switch_off_image,
	const sar_image_struct *switch_on_image,
	Boolean state,
	void *client_data, int id,
	void (*switch_cb)(void *, int, void *, Boolean)
);
extern Boolean SARMenuSwitchGetValue(sar_menu_struct *m, int n);
extern void SARMenuSwitchSetValue(
	gw_display_struct *display, sar_menu_struct *m, int n,
	Boolean state, Boolean redraw
);

/* Spin */
extern int SARMenuSpinNew(
	sar_menu_struct *m,
	float x, float y,
	float width, float height,
	sar_menu_color_struct *label_color,
	sar_menu_color_struct *value_color,
	GWFont *font, GWFont *value_font,
	const char *label,
	const sar_image_struct *label_image,
	const sar_image_struct *value_image,
	const sar_image_struct *dec_armed_image,
	const sar_image_struct *dec_unarmed_image,
	const sar_image_struct *inc_armed_image,
	const sar_image_struct *inc_unarmed_image,
	void *client_data, int id,
	void (*change_cb)(void *, int, void *, char *)
);
extern void SARMenuSpinSetValueType(
	sar_menu_struct *m, int n, int type
);
extern int SARMenuSpinAddValue(
	sar_menu_struct *m, int n, const char *value
);
extern char *SARMenuSpinGetCurrentValue(
	sar_menu_struct *m, int n, int *sel_num
);
extern void SARMenuSpinSelectValueIndex(
	gw_display_struct *display, sar_menu_struct *m, int n,
	int value_num,
	Boolean redraw
);
extern void SARMenuSpinSetValueIndex(
	gw_display_struct *display, sar_menu_struct *m, int n,
	int value_num, const char *value,
	Boolean redraw
);
extern void SARMenuSpinDeleteAllValues(sar_menu_struct *m, int n);
extern void SARMenuSpinDoInc(
	gw_display_struct *display, sar_menu_struct *m, int n,
	Boolean redraw
);
extern void SARMenuSpinDoInc(
	gw_display_struct *display, sar_menu_struct *m, int n,
	Boolean redraw
);

/* Slider */
extern int SARMenuSliderNew(
	sar_menu_struct *m,
	float x, float y,
	float width, float height,
	sar_menu_color_struct *label_color,
	GWFont *font,
	const char *label,
	const sar_image_struct *label_image,
	const sar_image_struct *trough_image,
	const sar_image_struct *handle_image,
	void *client_data, int id,
	void (*change_cb)(void *, int, void *, float)
);
extern void SARMenuSliderSetValueBounds(
	gw_display_struct *display, sar_menu_struct *m, int n,
	float lower, float upper,
	Boolean redraw
);
extern void SARMenuSliderSetValue(
	gw_display_struct *display, sar_menu_struct *m, int n,
	float value,
	Boolean redraw
);
extern float SARMenuSliderGetValue(sar_menu_struct *m, int n);

/* Common object functions */
extern void SARMenuObjectSetSensitive(
	gw_display_struct *display, sar_menu_struct *m, int n,
	Boolean sensitive, Boolean redraw
);
extern void SARMenuObjectDelete(sar_menu_struct *m, int n);

/* Drawing */
extern void SARMenuDrawWindowBG(
	gw_display_struct *display,
	const sar_image_struct **bg_image,		/* Total 9 images */
	const sar_menu_color_struct *base_color,	/* Set NULL to use image as base */
	int x, int y,
	int width, int height,
	Boolean draw_base,
	Boolean draw_shadow,
	Boolean is_selected
);
extern void SARMenuDrawObject(
	gw_display_struct *display,
	sar_menu_struct *m,
	int n
);
extern void SARMenuDrawAll(
	gw_display_struct *display,
	sar_menu_struct *m
);
extern int SARMenuManagePointer(
	gw_display_struct *display, sar_menu_struct *m,
	int x, int y, gw_event_type type, int btn_num
);
extern int SARMenuManageKey(
	gw_display_struct *display,
	sar_menu_struct *m,   
	int key, Boolean state
);


/* In menumap.c */
extern sar_menu_map_marking_struct *SARMenuMapGetMarkingPtr(
	sar_menu_map_struct *map, int i
);
extern int SARMenuMapNew(
	sar_menu_struct *m,
	float x, float y,
	float width, float height,
	GWFont *font,
	sar_menu_color_struct *color,
	const sar_image_struct **bg_image,	/* Total 9 images */
	float scroll_x, float scroll_y,	/* In meters */
	float m_to_pixels_coeff,	/* Meters to pixels coeff */
	int show_markings_policy
);
extern int SARMenuMapAppendMarking(
	sar_menu_map_struct *map,
	int type,			/* One of SAR_MENU_MAP_MARKING_TYPE_* */
	const sar_menu_color_struct *fg_color,
	float x, float y,		/* In meters */
	float x_end, float y_end,	/* In meters (for intercept line) */
	const sar_image_struct *icon,	/* Shared icon image */
	const char *desc
);
extern void SARMenuMapDeleteAllMarkings(sar_menu_map_struct *map);
extern void SARMenuMapMarkingSelectNext(
	sar_menu_map_struct *map, Boolean allow_warp
);
extern void SARMenuMapMarkingSelectPrev(
	sar_menu_map_struct *map, Boolean allow_warp
);
extern void SARMenuMapDraw(
	gw_display_struct *display,
	sar_menu_struct *m,
	sar_menu_map_struct *map,
	int map_num
);
extern int SARMenuMapManagePointer(
	gw_display_struct *display, sar_menu_struct *m,
	sar_menu_map_struct *map, int menu_obj_num,
	int x, int y, gw_event_type type, int btn_num
);

/* In menuobjview.c */
extern int SARMenuObjViewNew(
	sar_menu_struct *m,
	float x, float y,
	float width, float height,
	GWFont *font,
	sar_menu_color_struct *color,
	const sar_image_struct **bg_image,	/* Pointer to 9 images */
	const char *title
);
extern int SARMenuObjViewManagePointer(
	gw_display_struct *display, sar_menu_struct *m,
	sar_menu_objview_struct *ov_ptr, int ov_num,
	int x, int y, gw_event_type type, int btn_num
);
extern int SARMenuObjViewManageKey(
	gw_display_struct *display, sar_menu_struct *m,
	sar_menu_objview_struct *ov_ptr, int ov_num,
	int key, Boolean state
);
extern int SARMenuObjViewLoad(
	gw_display_struct *display, sar_menu_struct *m,
	sar_menu_objview_struct *ov_ptr, int ov_num,
	const char *filename
);
extern void SARMenuObjViewRebufferImage(
	gw_display_struct *display, sar_menu_struct *m,
	sar_menu_objview_struct *ov_ptr, int ov_num
);


#endif	/* MENU_H */
