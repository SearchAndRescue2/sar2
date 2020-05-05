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
	Graphics IO Wrapper (easier to use and more powerful than GLUT)
 */

#ifndef GW_H
#define GW_H

#ifdef __cplusplus
# define NOMANGLE
extern "C"
{
#endif	/* __cplusplus */

#include <sys/types.h>

/* Include OS specific header files */
#ifdef __MSW__

#else
# include <GL/glx.h>
#endif

#include "stategl.h"


/*
 *	Type make-ups:
 */
#ifndef Pixel
# define Pixel		unsigned long
#endif
#ifndef Bool
# define Bool		char
#endif
#ifndef Boolean
# define Boolean	Bool
#endif

#ifdef __MSW__
# ifndef False
#  define False	0
# endif
# ifndef True
#  define True	1
# endif
#endif	/* __MSW__ */

#ifdef __MSW__
# ifndef u_int8_t
#  define u_int8_t	unsigned char
# endif
# ifndef u_int16_t
#  define u_int16_t	unsigned short
# endif
# ifndef u_int32_t   
#  define u_int32_t	unsigned int
# endif
#endif	/* __MSW__ */

#ifndef BYTES_PER_PIXEL8
# define BYTES_PER_PIXEL8	1
#endif
#ifndef BYTES_PER_PIXEL15
# define BYTES_PER_PIXEL15	2
#endif
#ifndef BYTES_PER_PIXEL16
# define BYTES_PER_PIXEL16	2
#endif
#ifndef BYTES_PER_PIXEL24
# define BYTES_PER_PIXEL24	4
#endif
#ifndef BYTES_PER_PIXEL32
# define BYTES_PER_PIXEL32	4
#endif


/*
 *	Toplevel window size bounds (in pixels):
 */
#define GW_MIN_TOPLEVEL_WIDTH		8
#define GW_MIN_TOPLEVEL_HEIGHT		8

#define GW_MIN_TOPLEVEL_WIDTH_INC	1
#define GW_MIN_TOPLEVEL_HEIGHT_INC	1

/*
 *	Geometry string maximum length:
 */
#define GW_GEOMETRY_STRING_MAX		256

/*
 *	Default double and trible click intervals (in milliseconds):
 */
#define GW_2BUTTON_PRESS_INTERVAL	600
#define GW_3BUTTON_PRESS_INTERVAL	600


/*
 *	Non-ascii key values for GW sent to calling functions:
 */
#define GWKeyAlt		0x0100
#define GWKeyCtrl		0x0101
#define GWKeyShift		0x0102

#define GWKeyF1			0x0200
#define GWKeyF2			0x0201
#define GWKeyF3			0x0202
#define GWKeyF4			0x0203
#define GWKeyF5			0x0204
#define GWKeyF6			0x0205
#define GWKeyF7			0x0206
#define GWKeyF8			0x0207
#define GWKeyF9			0x0208
#define GWKeyF10		0x0209
#define GWKeyF11		0x020a
#define GWKeyF12		0x020b
#define GWKeyF13		0x020c
#define GWKeyF14		0x020d
#define GWKeyF15		0x020e
#define GWKeyF16		0x020f
#define GWKeyF17		0x0210
#define GWKeyF18		0x0211
#define GWKeyF19		0x0212
#define GWKeyF20		0x0213

#define GWKeyUp			0x0300
#define GWKeyDown		0x0301
#define GWKeyLeft		0x0302
#define GWKeyRight		0x0303
#define GWKeyCenter		0x0304
#define GWKeyHome		0x0305
#define GWKeyEnd		0x0306
#define GWKeyPageUp		0x0307
#define GWKeyPageDown		0x0308

#define GWKeyBackSpace		0x0400
#define GWKeyDelete		0x0401
#define GWKeyInsert		0x0402

#define GWKeyPause		0x0500
#define GWKeyScrollLock		0x0501
#define GWKeySysReq		0x0502


/*
 *	Visibility Types:
 */
typedef enum {
	GWVisibilityUnobscured,
	GWVisibilityPartiallyObscured,
	GWVisibilityFullyObscured
} gw_visibility;

/*
 *	Event Types:
 */
typedef enum {
	GWEventTypeButtonPress		= 1,
	GWEventTypeButtonRelease	= 2,
	GWEventTypePointerMotion	= 3,
	GWEventType2ButtonPress		= 4,	/* Double click */
	GWEventType3ButtonPress		= 5	/* Trible click */
} gw_event_type;

/*
 *	GW Pointer Cursor Types:
 */
typedef enum {
	GWPointerCursorStandard,	/* Basic arrow */
	GWPointerCursorBusy,		/* Usually hourglass or watch */
	GWPointerCursorText,		/* Text cursor */
	GWPointerCursorTranslate,	/* 4 arrows in all directions */
	GWPointerCursorZoom,		/* Square with smaller square (ie resizine) */
	GWPointerCursorInvisible	/* Totally invisible cursor */
} gw_pointer_cursor;


/*
 *	GW Font Type:
 */
#define GWFont	u_int8_t


#ifdef __MSW__
/*
 *	Window geometry (for Win32 use):
 */
typedef struct {
	int		x,
			y;
	int		width,
			height;
} WRectangle;
#endif	/* __MSW__ */

/*
 *	Vidmode information structure:
 */
typedef struct {
	int		width, 		/* Display size in pixels */
			height,
			vwidth,		/* Virtual screen size in pixels */
			vheight;
} gw_vidmode_struct;

/*
 *      Accelerator Key:
 */
typedef struct {
	int		key;		/* Any character value or GWKey* */
	int		modifier;	/* GWKeyAlt, GWKeyCtrl, GWKeyShift, or 0 */
} gw_accelerator_struct;


#ifdef X_H
/*
 *	Icon Codes:
 */
typedef enum {
	GWX_ICON_NONE,
	GWX_ICON_INFO,
	GWX_ICON_WARNING,
	GWX_ICON_ERROR,
	GWX_ICON_QUESTION
} gwx_icon;

/*
 *	Widget Flags:
 */
typedef enum {
	GWX_MAPPED		= (1 << 0),
	GWX_CAN_FOCUS		= (1 << 1),
	GWX_HAS_FOCUS		= (1 << 2),
	GWX_CAN_DEFAULT		= (1 << 3),
	GWX_HAS_DEFAULT		= (1 << 4),
	GWX_SENSITIVE		= (1 << 5)
} gwx_widget_flags;

/*
 *	Widget:
 *
 *	All GWX widgets have the first set of members as prototyped
 *	in GWX_WIDGET_MEMBERS_PROTOTYPE.
 */
#define GWX_WIDGET_MEMBERS_PROTOTYPE				\
	gwx_widget_flags	flags;				\
	Window		toplevel,				\
			parent;					\
	Pixmap		toplevel_buf;				\
	int		x,		/* Position */		\
			y;					\
	unsigned int	width,		/* Size */		\
			height;
typedef struct {

	GWX_WIDGET_MEMBERS_PROTOTYPE

} gwx_widget_struct;
#define GWX_WIDGET(p)	((gwx_widget_struct *)(p))
#define GWX_WIDGET_MAPPED(p)		(((p) != NULL) ? \
 (GWX_WIDGET(p)->flags & GWX_MAPPED) : False)
#define GWX_WIDGET_CAN_FOCUS(p)		(((p) != NULL) ? \
 (GWX_WIDGET(p)->flags & GWX_CAN_FOCUS) : False)
#define GWX_WIDGET_HAS_FOCUS(p)		(((p) != NULL) ? \
 (GWX_WIDGET(p)->flags & GWX_HAS_FOCUS) : False)
#define GWX_WIDGET_CAN_DEFAULT(p)	(((p) != NULL) ? \
 (GWX_WIDGET(p)->flags & GWX_CAN_DEFAULT) : False)
#define GWX_WIDGET_HAS_DEFAULT(p)	(((p) != NULL) ? \
 (GWX_WIDGET(p)->flags & GWX_HAS_DEFAULT) : False)
#define GWX_WIDGET_SENSITIVE(p)		(((p) != NULL) ? \
 (GWX_WIDGET(p)->flags & GWX_SENSITIVE) : False)


/*
 *	Button widget for X dialogs:
 */
typedef enum {
	GWX_BUTTON_STATE_UNARMED,
	GWX_BUTTON_STATE_ARMED,
	GWX_BUTTON_STATE_HIGHLIGHTED
} gwx_button_state;
typedef struct {

	GWX_WIDGET_MEMBERS_PROTOTYPE

	void		*dialog;	/* Parent gwx_dialog_struct */

	gwx_button_state	state;

	/* Accelerator keys */
	gw_accelerator_struct	**accelerator;
	int		total_accelerators;

	/* Colors */
	Pixel		color_fg,
			color_bg,
			color_fg_insensitive,
			color_bg_highlighted,
			color_highlight,
			color_shade;
	Boolean		colors_initialized;

	XFontStruct	*font;
	char		*label;
	int		label_len_pixels,
			label_height_pixels;

	/* Button press callback, btn_ptr, client_data */
	void		(*func_cb)(
		void *,		/* Button */
		void *		/* Data */
	);
	void		*client_data;

} gwx_button_struct;
#define GWX_BUTTON(p)	((gwx_button_struct *)(p))


/*
 *	Message dialog for X:
 */
typedef enum {
	GWX_DIALOG_TYPE_MESG,
	GWX_DIALOG_TYPE_CONF,
	GWX_DIALOG_TYPE_CONF_CANCEL	/* With cancel button */
} gwx_dialog_type;
typedef struct {

	GWX_WIDGET_MEMBERS_PROTOTYPE

	gwx_dialog_type	type;

	unsigned int	margin;

	Pixel		color_fg, color_bg,
			color_highlight, color_shade;
	Boolean		colors_initialized;
	Window		icon_w;
	Pixmap		icon,
			icon_mask;
	unsigned int	icon_width,
			icon_height;

	XFontStruct	*font;

	/* Lines of messages */
	char		**strv;
	int		strc,
			longest_line;
	int		longest_line_pixels,
			line_height_pixels,
			lines_height_pixels;

	gwx_button_struct	ok_btn,
				yes_btn,
				no_btn,
				cancel_btn;

} gwx_dialog_struct;
#define GWX_DIALOG(p)	((gwx_dialog_struct *)(p))

#undef GWX_WIDGET_MEMBERS_PROTOTYPE

#endif	/* X_H */


/*
 *	GW Display:
 */
typedef struct {

	int		gl_version_major,
			gl_version_minor;

	/* Correction to calculated aspects coefficients, this value
	 * is added to aspect coefficients to correct for displays
	 * that have non 4/3 aspect ratios
	 */
	float		aspect_offset;

	/* Maximum texture sizes, in texels. A value of 0 may suggest
	 * that texture is not supported
	 */
	GLint		texture_1d_max,
			texture_2d_max,
			texture_3d_max;

#ifdef X_H
	int		glx_version_major,
			glx_version_minor;
	Boolean		direct_rendering;
	Display		*display;
	XVisualInfo	*visual_info;
	Boolean		has_double_buffer;
	int		alpha_channel_bits;
	Boolean		has_vidmode_ext;
	int		vidmode_ext_event_offset;
	Visual		*visual;
	GC		gc;
	int		depth;
	Pixel		black_pix,
			white_pix;
	Colormap	colormap;
	Atom		atom_wm_motif_all_clients,
			atom_wm_motif_hints,
			atom_wm_motif_info,
			atom_wm_motif_menu,
			atom_wm_motif_messages,
			atom_wm_motif_offset,
			atom_wm_motif_query,
			atom_wm_close_window,   /* _NET_CLOSE_WINDOW */
			atom_wm_delete_window,	/* WM_DELETE_WINDOW */
			atom_wm_ping,
			atom_wm_save_yourself,
			atom_wm_state,
			atom_wm_take_focus,
			atom_wm_workarea;	/* _NET_WORKAREA */

	char		*def_xfont_name;

	Boolean		cursor_shown;
	Cursor		cursor_standard,
			cursor_busy,
			cursor_text,
			cursor_translate,
			cursor_zoom,
			cursor_invisible;

	/* Root window and its geometry */
	Window		root;
	int		root_width,
			root_height;

	/* Work area viewport (not GL frame buffer viewport) */
	int		viewport_x,
			viewport_y,
			viewport_width,
			viewport_height;

	/* GLX rendering contexts and toplevel Windows */
	int		gl_context_num;		/* Current GLX context */
	int		total_gl_contexts;	/* Total; GLX contexts,
						 * toplevel Windows, and
						 * toplevel rectangles */
	GLXContext	*glx_context;
	Window		*toplevel;
	XRectangle	*toplevel_geometry;

	int		*draw_count;		/* Posted draws */

	/* Last ButtonPress timestamp */
	Time		last_button_press;

	/* Button press count, used for double and triple clicks */
	int		button_press_count;

	/* Indicates that pointer is currently grabbed if not None */
	Window		grab_window;

	/* Indicates if currently full screen */
	Boolean		fullscreen;
	/* Current GL context number in full screen (-1 for none) */
	int		fullscreen_context_num;
	/* Previous vidmode geometry for GUI */
	XRectangle	vidmode_last_gui_geometry;
	/* Previous window coordinates for toplevel window in GUI mode
	 * (prior to using vidmode to go full screen).
	 */
	int		vidmode_last_gui_wx,
			vidmode_last_gui_wy;

	gwx_dialog_struct	mesg_dialog;
	gwx_dialog_struct	conf_dialog;
#endif	/* X_H */
#ifdef __MSW__
	int		glw_version_major,
			glw_version_minor;
	int		depth;		/* Depth in bits */
	Boolean		has_double_buffer;
	int		alpha_channel_bits;
	void		*pid;		/* (HINSTANCE) */

	/* Root window (monitor) geometry */
	void		*root;		/* (HWND) */
	int		root_width,
			root_height;

	/* GLW rendering contexts and toplevel Windows */
	int		gl_context_num;		/* Current GLX context */
	int		total_gl_contexts;	/* Total; GLX contexts,
						 * toplevel Windows, and
						 * toplevel rectangles */
	void		**toplevel;		/* Toplevel Windows (HWND) */
	void		**dc;			/* Device contexts (HDC) */
	void		**rc;			/* Rendering contexts (HGLRC) */
	WRectangle	*toplevel_geometry;

	int		*draw_count;		/* Posted draws */

	Boolean		keyboard_autorepeat;

	/* Records the state of each key, the key's ASCII value
	 * corresponds to the index of the key_state array
	 */
#define GW_KEY_STATE_TABLE_MAX_KEYS	256
	Boolean		key_state[GW_KEY_STATE_TABLE_MAX_KEYS];

	/* Last WM_*BUTTONDOWN message in milliseconds */
	unsigned long	last_button_press;

	/* Button press count, used for double and triple clicks */
	int		button_press_count;

	Boolean		fullscreen;
	int		last_gui_wx,
			last_gui_wy,
			last_gui_wwidth,
			last_gui_wheight;

	Boolean		cursor_shown;
	void		*cursor_current;	/* Shared, do not reference */
	void		*cursor_standard,
			*cursor_busy,
			*cursor_text,
			*cursor_translate,
			*cursor_zoom;

#endif  /* __MSW__ */


	/* Draw callback */
	void		(*func_draw)(
		int,		/* Context Number */
		void *		/* Data */
	);
	void		*func_draw_data;

	/* Resize callback */
	void		(*func_resize)(
		int,		/* Context Number */
		void *,		/* Data */
		int, int,	/* X, Y */
		int, int	/* Width, Height */
	);
	void		*func_resize_data;

	/* Keypress callback */
	void		(*func_keyboard)(
		void *,		/* Data */ 
		int,		/* Key */
		Boolean,	/* State (True = press) */
		unsigned long	/* Timestamp (in ms) */
	);
	void		*func_keyboard_data;

	/* Pointer callback */
	void		(*func_pointer)(
		int,		/* Context Number */
		void *,		/* Data */
		int, int,	/* X, Y */
		gw_event_type,	/* Event Type */
		int,		/* Button Number */
		unsigned long	/* Timestamp (in ms) */
	);
	void		*func_pointer_data;

	/* Visibility Change callback */
	void		(*func_visibility)(
		int,		/* Context Number */
		void *,		/* Data */
		gw_visibility	/* Visiblity Type */
	);
	void		*func_visibility_data;

	/* Save Yourself callback */
	void		(*func_save_yourself)(
		int,		/* Context Number */
		void *		/* Data */
	);
	void		*func_save_yourself_data;

	/* Close callback */
	void		(*func_close)(
		int,		/* Context Number */
		void *,		/* Data */
		void *		/* Window ID */
	);
	void		*func_close_data;

	/* Timeout callback */
	void		(*func_timeout)(
		void *		/* Data */
	);
	void		*func_timeout_data;

	/* Modifier Key states */
	Boolean		alt_key_state,
			ctrl_key_state,
			shift_key_state;

	GWFont		*current_font;

	/* OpenGL state record structure for our GL context */
	state_gl_struct	state_gl;
        Boolean         allow_autorepeat;
} gw_display_struct;
#define GW_DISPLAY(p)		((gw_display_struct *)(p))


#ifdef X_H
extern int GWGetCharacterFromKeyCode(gw_display_struct *dpy, KeyCode keycode);
extern Window GWGetParent(gw_display_struct *dpy, Window w);
#endif	/* X_H */
extern void GWSetWindowIconFile(
	gw_display_struct *display, int ctx_num,
	const char *icon_path, const char *icon_name
);
#ifdef X_H
extern Window GWCreateWindow(
	gw_display_struct *dpy,
	Window parent,
	int x, int y,
	int width, int height,
	const char *title
);
#endif	/* X_H */
#ifdef __MSW__
extern int GWCreateWindow(
	gw_display_struct *dpy,
	void *parent,
	void **hwnd_rtn,
	void **dc_rtn,
	void **rc_rtn,
	int x, int y,
	int width, int height,
	int depth,		/* In bits */
	const char *title
);
#endif	/* __MSW__ */

/* GW init/management/shutdown */
extern gw_display_struct *GWInit(int argc, char **argv);
extern void GWManage(gw_display_struct *display);
extern void GWShutdown(gw_display_struct *display);

/* GW sync */
extern void GWFlush(gw_display_struct *display);
extern void GWFlush(gw_display_struct *display);
extern int GWEventsPending(gw_display_struct *display);

/* GW dialog message and response */
#define GWOutputMessageTypeGeneral	0
#define GWOutputMessageTypeWarning	1
#define GWOutputMessageTypeError	2
#define GWOutputMessageTypeQuestion	3
extern void GWOutputMessage( 
	gw_display_struct *display,
	int type,	/* One of GWOutputMessageType* */
	const char *subject,
	const char *message,
	const char *help_details
);
#define GWConfirmationNotAvailable	-1
#define GWConfirmationNo		0
#define GWConfirmationYes		1
#define GWConfirmationCancel		2
extern int GWConfirmation(
	gw_display_struct *display,
	int type,		/* One of GWOutputMessageType* */
	const char *subject,
	const char *message,
	const char *help_details,
	int default_response	/* One of GWConfirmation* */
);
extern int GWConfirmationSimple(
	gw_display_struct *display,
	const char *message
);

/* GW callback setting */
extern void GWSetDrawCB(
	gw_display_struct *display,
	void (*func)(int, void *),
	void *data
);
extern void GWSetResizeCB(
	gw_display_struct *display, 
	void (*func)(int, void *, int, int, int, int),
	void *data
);
extern void GWSetKeyboardCB(
	gw_display_struct *display,
	void (*func)(void *, int, Boolean, unsigned long),
	void *data
);
extern void GWSetPointerCB(
	gw_display_struct *display,
	void (*func)(int, void *, int, int, gw_event_type, int, unsigned long),
	void *data
);
extern void GWSetVisibilityCB(
	gw_display_struct *display,
	void (*func)(int, void *, gw_visibility),
	void *data
);
extern void GWSetSaveYourselfCB(
	gw_display_struct *display,
	void (*func)(int, void *),
	void *data
);
extern void GWSetCloseCB(
	gw_display_struct *display,
	void (*func)(int, void *, void *),
	void *data
);
extern void GWSetTimeoutCB(
	gw_display_struct *display,
	void (*func)(void *),
	void *data
);

/* GW context */
extern int GWContextNew(
	gw_display_struct *display,
	int x, int y, int width, int height,
	const char *title,
	const char *icon_path, const char *icon_name,
	Bool no_windows
);
extern void GWContextDelete(
	gw_display_struct *display, int ctx_num
);
extern int GWContextCurrent(gw_display_struct *display);
extern int GWContextGet(
	gw_display_struct *display, int ctx_num,
	void **window_id_rtn, void **gl_context_rtn,
	int *x_rtn, int *y_rtn, int *width_rtn, int *height_rtn
);
extern int GWContextSet(gw_display_struct *display, int ctx_num);
extern void GWContextPosition(
	gw_display_struct *display, int x, int y
);
extern void GWContextSize(
	gw_display_struct *display, int width, int height
);
extern Boolean GWContextIsFullScreen(gw_display_struct *display);
extern int GWContextFullScreen(gw_display_struct *display, Boolean state);

/* Buffer IO */
extern void GWPostRedraw(gw_display_struct *display);
extern void GWSwapBuffer(gw_display_struct *display);

/* Set up gl projection matrix for 2d drawing */
extern void GWOrtho2D(gw_display_struct *display);
extern void GWOrtho2DCoord(
	gw_display_struct *display,
	float left, float right, float top, float bottom
);

/* Keyboard */
extern void GWKeyboardAutoRepeat(gw_display_struct *display, Boolean b);

/* Video modes */
extern gw_vidmode_struct *GWVidModesGet(
	gw_display_struct *display, int *n
);
extern void GWVidModesFree(gw_vidmode_struct *vidmode, int n);
extern void GWVidModesSort(gw_vidmode_struct *vidmode, int n);

/* Accelerator keys */
extern gw_accelerator_struct *GWAcceleratorNew(
	int key, int modifier
);
extern void GWAcceleratorDelete(gw_accelerator_struct *a);
extern int GWAcceleratorListAdd(
	gw_accelerator_struct ***a, int *total,
	int key, int modifier
);
extern void GWAcceleratorListDelete(
	gw_accelerator_struct ***a, int *total
);
extern Boolean GWAcceleratorListCheck(
	gw_accelerator_struct **a, int total,
	int key, int modifier
);

/* GW pointer cursor */
#ifdef X_H
extern void *GWCursorNew(
	gw_display_struct *display,
	int width, int height,  /* In pixels */
	int hot_x, int hot_y,   /* In pixels */
	int bpl,                /* Bytes per line, 0 for autocalc */
	const void *data        /* RGBA data (4 bytes per pixel) */
);
extern void GWCursorSet(gw_display_struct *display, void *cursor_ptr);
extern void GWCursorDelete(gw_display_struct *display, void *cursor_ptr);
#endif	/* X_H */
extern void GWSetPointerCursor(
	gw_display_struct *display, gw_pointer_cursor cursor
);
extern void GWShowCursor(gw_display_struct *display);
extern void GWHideCursor(gw_display_struct *display);
extern Boolean GWIsCursorShown(gw_display_struct *display);
extern void GWSetInputBusy(gw_display_struct *display);
extern void GWSetInputReady(gw_display_struct *display);

/* GW font IO */
extern void GWSetFont(gw_display_struct *display, GWFont *font);
extern int GWGetFontSize(
	GWFont *font,
	int *width, int *height,
	int *character_spacing, int *line_spacing
);

/* GW string drawing */
extern void GWDrawString(
	gw_display_struct *display,
	int x, int y,
	const char *string
);
extern void GWDrawCharacter(
	gw_display_struct *display,
	int x, int y,
	char c
);

/* GW image IO */
extern int GWImageLoadHeaderFromData(
	gw_display_struct *display,
	int *width, int *height, int *bpl, int *bpp,
	const void *data
);
extern u_int8_t *GWImageLoadFromDataRGB(
	gw_display_struct *display,
	int *width, int *height, int *bpl,
	const void *data
);
extern const u_int8_t *GWImageLoadFromDataSharedRGB(
	gw_display_struct *display,
	int *width, int *height, int *bpl,
	const void *data
);
extern u_int8_t *GWImageLoadFromDataRGBA(
	gw_display_struct *display,
	int *width, int *height, int *bpl,
	const void *data
);
extern const u_int8_t *GWImageLoadFromDataSharedRGBA(
	gw_display_struct *display,
	int *width, int *height, int *bpl,
	const void *data
);
extern void GWImageDrawFromDataRGB(
	gw_display_struct *display, const void *data,
	int x, int y, int width, int height
);
extern void GWImageDrawFromDataRGBA(
	gw_display_struct *display, const void *data,
	int x, int y, int width, int height
);


#ifdef X_H
/* In gwx_dialog.c */
/* Widgets */
extern void GWXWidgetSetFlags(
	gw_display_struct *display, gwx_widget_struct *widget,
	gwx_widget_flags flags
);
extern void GWXWidgetUnsetFlags(
	gw_display_struct *display, gwx_widget_struct *widget,
	gwx_widget_flags flags
);
extern void GWXWidgetMapRaise(
	gw_display_struct *display, gwx_widget_struct *widget
);
extern void GWXWidgetMap(
	gw_display_struct *display, gwx_widget_struct *widget
);
extern void GWXWidgetUnmap(
	gw_display_struct *display, gwx_widget_struct *widget
);
extern void GWXWidgetFocus(
	gw_display_struct *display, gwx_widget_struct *widget
);
extern void GWXWidgetUnfocus(
	gw_display_struct *display, gwx_widget_struct *widget
);
extern void GWXWidgetGrabDefault(
	gw_display_struct *display, gwx_widget_struct *widget
);
extern void GWXWidgetUngrabDefault(
	gw_display_struct *display, gwx_widget_struct *widget
);
extern void GWXWidgetSetSensitive(
	gw_display_struct *display, gwx_widget_struct *widget,
	Boolean sensitive
);
/* Push Buttons */
extern int GWXButtonCreate(
	gw_display_struct *display, gwx_button_struct *btn,
	void *dialog,           /* Parent gwx_dialog_struct */
	Window parent,
	int x, int y,
	unsigned int width, unsigned int height,
	const char *label,
	void *client_data,
	void (*func_cb)(void *, void *)
);
extern void GWXButtonUpdateSize(
	gw_display_struct *display, gwx_button_struct *btn
);
extern void GWXButtonResize(
	gw_display_struct *display, gwx_button_struct *btn
);
extern void GWXButtonDraw(
	gw_display_struct *display, gwx_button_struct *btn
);
extern int GWXButtonManage(
	gw_display_struct *display,
	gwx_button_struct *btn,
	XEvent *event
);
extern void GWXButtonDestroy(
	gw_display_struct *display, gwx_button_struct *btn
);
/* Dialogs */
extern void GWXDialogLoadIcon(
	gw_display_struct *display, gwx_dialog_struct *md,
	gwx_icon icon_code
);
extern int GWXDialogCreate(
	gw_display_struct *display, gwx_dialog_struct *md,
	gwx_dialog_type type
);
extern void GWXDialogUpdateSize(
	gw_display_struct *display, gwx_dialog_struct *md
);
extern void GWXDialogResize(
	gw_display_struct *display, gwx_dialog_struct *md
);
extern void GWXDialogDraw(
	gw_display_struct *display, gwx_dialog_struct *md
);
extern int GWXDialogManage(
	gw_display_struct *display, gwx_dialog_struct *md,
	XEvent *event
);
extern void GWXDialogDestroy(
	gw_display_struct *display, gwx_dialog_struct *md
);
/* Dialog Callbacks */
extern void GWXDialogNoBtnCB(void *object, void *client_data);
extern void GWXDialogYesBtnCB(void *object, void *client_data);
extern void GWXDialogCancelBtnCB(void *object, void *client_data);
extern void GWXDialogOKBtnCB(void *object, void *client_data);
extern void GWXDialogSetMesg(
	gw_display_struct *display, gwx_dialog_struct *md,
	const char *title,
	const char *mesg,
	const char *details
);
extern void GWXDialogMap(
	gw_display_struct *display, gwx_dialog_struct *md
);
extern int GWXDoBlockUntilConf(gw_display_struct *display);
#endif	/* X_H */





#ifdef NOMANGLE
# undef NOMANGLE
}
#endif	/* NOMANGLE */

#endif	/* GW_H */
