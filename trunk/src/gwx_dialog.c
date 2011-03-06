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
#include <string.h>
#include <stdlib.h>   

#ifndef __MSW__
#include <unistd.h>

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <X11/Xproto.h>
#include <X11/Xatom.h>
#include <X11/cursorfont.h>
#include <X11/extensions/shape.h>

/* XPM Library */
#ifdef HAVE_LIBXPM
# include <X11/xpm.h>
# ifndef XPM_H
#  define XPM_H
# endif
# ifndef XpmDefaultColorCloseness
#  define XpmDefaultColorCloseness	40000
# endif
# include "gwdata/icon_info.xpm"
# include "gwdata/icon_warning.xpm"
# include "gwdata/icon_error.xpm"
# include "gwdata/icon_question.xpm"
#endif

/* Motif WM Hints Definations */
#ifdef HAVE_MWMUTIL_H
# include "gwdata/MwmUtil.h"
# ifndef MWMUTIL_H
#  define MWMUTIL_H
# endif
#endif

#include "gw.h"


/* Utility functions */
static Pixel GWXPixelNew(
	gw_display_struct *display,
	u_int8_t r, u_int8_t g, u_int8_t b
);
static void GWXPixelDelete(gw_display_struct *display, Pixel pixel);

static XFontStruct *GWXFontNew(
	gw_display_struct *display, const char *name
);
static void GWXFontDelete(gw_display_struct *display, XFontStruct *font);
static int GWXGetFontStringLength(XFontStruct *font, const char *s);

/* Widgets */
void GWXWidgetSetFlags(
	gw_display_struct *display, gwx_widget_struct *widget,
	gwx_widget_flags flags
);
void GWXWidgetUnsetFlags(
	gw_display_struct *display, gwx_widget_struct *widget,
	gwx_widget_flags flags
);
void GWXWidgetMapRaise(
	gw_display_struct *display, gwx_widget_struct *widget
);
void GWXWidgetMap(
	gw_display_struct *display, gwx_widget_struct *widget
);
void GWXWidgetUnmap(
	gw_display_struct *display, gwx_widget_struct *widget
);
void GWXWidgetFocus(
	gw_display_struct *display, gwx_widget_struct *widget
);
void GWXWidgetUnfocus(
	gw_display_struct *display, gwx_widget_struct *widget
);
void GWXWidgetGrabDefault(
	gw_display_struct *display, gwx_widget_struct *widget
);
void GWXWidgetUngrabDefault(
	gw_display_struct *display, gwx_widget_struct *widget
);
void GWXWidgetSetSensitive(
	gw_display_struct *display, gwx_widget_struct *widget,
	Boolean sensitive
);

/* Push button */
int GWXButtonCreate(
	gw_display_struct *display, gwx_button_struct *btn,
	void *dialog,           /* Parent gwx_dialog_struct */
	Window parent,
	int x, int y,
	unsigned int width, unsigned int height,
	const char *label, 
	void *client_data,
	void (*func_cb)(void *, void *)
);
void GWXButtonUpdateSize(
	gw_display_struct *display, gwx_button_struct *btn
);
void GWXButtonResize(
	gw_display_struct *display, gwx_button_struct *btn
);
void GWXButtonDraw(
	gw_display_struct *display, gwx_button_struct *btn
);
int GWXButtonManage(
	gw_display_struct *display, gwx_button_struct *btn,
	XEvent *event
);
void GWXButtonDestroy(
	gw_display_struct *display, gwx_button_struct *btn
);

/* Message dialog */
int GWXDialogCreate(
	gw_display_struct *display, gwx_dialog_struct *md,
	gwx_dialog_type type
);
void GWXDialogLoadIcon(
	gw_display_struct *display, gwx_dialog_struct *md,
	gwx_icon icon_code	/* One of GWX_ICON_* */
);
void GWXDialogUpdateSize(
	gw_display_struct *display, gwx_dialog_struct *md
);
void GWXDialogResize(
	gw_display_struct *display, gwx_dialog_struct *md
);
void GWXDialogDraw(
	gw_display_struct *display, gwx_dialog_struct *md
);
int GWXDialogManage(
	gw_display_struct *display, gwx_dialog_struct *md,
	XEvent *event
);
void GWXDialogDestroy(
	gw_display_struct *display, gwx_dialog_struct *md
);

/* Message dialog callbacks */
void GWXDialogNoBtnCB(void *object, void *client_data);
void GWXDialogYesBtnCB(void *object, void *client_data);
void GWXDialogCancelBtnCB(void *object, void *client_data);
void GWXDialogOKBtnCB(void *object, void *client_data);

/* Message dialog operations */
void GWXDialogSetMesg(
	gw_display_struct *display, gwx_dialog_struct *md,
	const char *title,
	const char *mesg,
	const char *details
);
void GWXDialogMap(
	gw_display_struct *display, gwx_dialog_struct *md
);
int GWXDoBlockUntilConf(gw_display_struct *display);

int GWX_Widget_Has_Default(gwx_button_struct *my_button);

#define ATOI(s)         (((s) != NULL) ? atoi(s) : 0)
#define ATOL(s)         (((s) != NULL) ? atol(s) : 0)
#define ATOF(s)         (((s) != NULL) ? atof(s) : 0.0f)
#define STRDUP(s)       (((s) != NULL) ? strdup(s) : NULL)

#define MAX(a,b)        (((a) > (b)) ? (a) : (b))
#define MIN(a,b)        (((a) < (b)) ? (a) : (b))
#define CLIP(a,l,h)     (MIN(MAX((a),(l)),(h)))
#define STRLEN(s)	(((s) != NULL) ? ((int)strlen(s)) : 0)

#define ISSTREMPTY(s)	(((s) != NULL) ? ((s) == '\0') : True)

#define RADTODEG(r)     ((r) * 180 / PI)
#define DEGTORAD(d)     ((d) * PI / 180)

#define FREE_PIXMAP(p)		\
{ if(*(p) != None) { XFreePixmap(dpy, *(p)); *(p) = None; } }
#define FREE_BITMAP(p)		FREE_PIXMAP(p)
#define DESTROY_WINDOW(p)	\
{ if(*(p) != None) { XDestroyWindow(dpy, *(p)); *(p) = None; } }
#define DELETE_FONT(p)		\
{ GWXFontDelete(display, *(p)); *(p) = NULL; }

#define DESTROY_TOPLEVEL_WINDOW(p)			\
{ if(*(p) != None) {					\
 /* Get WM hints and destroy icon Window, Pixmap, and	\
  * mask.						\
  */							\
 XWMHints *wm_hints = XGetWMHints(dpy, *(p));		\
 if(wm_hints != NULL) {					\
  if(wm_hints->flags & IconWindowHint)                  \
   DESTROY_WINDOW(&wm_hints->icon_window);		\
  if(wm_hints->flags & IconPixmapHint)			\
   FREE_PIXMAP(&wm_hints->icon_pixmap);			\
  if(wm_hints->flags & IconMaskHint)			\
   FREE_BITMAP(&wm_hints->icon_mask);			\
  XFree(wm_hints);					\
 }							\
 /* Destroy toplevel Window */				\
 DESTROY_WINDOW(p);					\
} }

#define XCOLOR_SET_RGB_COEFF(p,r,g,b)                   \
{ if((p) != NULL) {                                     \
 (p)->flags     = DoRed | DoGreen | DoBlue;             \
 (p)->red       = (float)(r) * (unsigned short)-1;      \
 (p)->green     = (float)(g) * (unsigned short)-1;      \
 (p)->blue      = (float)(b) * (unsigned short)-1;      \
 (p)->pad       = 0;                                    \
} }
#define XCOLOR_SET_GREY_COEFF(p,g)                      \
{ if((p) != NULL) {                                     \
 (p)->flags     = DoRed | DoGreen | DoBlue;             \
 (p)->red       = (float)(g) * (unsigned short)-1;      \
 (p)->green     = (p)->red;                             \
 (p)->blue      = (p)->red;                             \
 (p)->pad       = 0;                                    \
} }


/* Default button sizes and margins */
#define GWX_BTN_DEF_WIDTH	80
#define GWX_BTN_DEF_HEIGHT	25
#define GWX_BTN_XMARGIN		5
#define GWX_BTN_YMARGIN		5

/* Widget style colors {a, r, g, b} in values from 0x00 to 0xff */
static u_int8_t style_fg_color[]	= {0xff, 0x00, 0x00, 0x00};
static u_int8_t style_bg_color[]	= {0xff, 0xd7, 0xd7, 0xd7};
static u_int8_t style_fg_insensitive[]	= {0xff, 0x80, 0x80, 0x80};
static u_int8_t style_bg_highlight_color[]	= {0xff, 0xe8, 0xe8, 0xe8};
static u_int8_t style_highlight_color[]	= {0xff, 0xf0, 0xf0, 0xf0};
static u_int8_t style_shade_color[]	= {0xff, 0x80, 0x80, 0x80};



/* Records the last confirmation dialog return code, one of
 * GWConfirmation*.
 */
static int last_conf_dialog_code;



/* A replacement for the macro of the name name that
   is giving us optimization warnings.
*/
int GWX_Widget_Has_Default(gwx_button_struct *my_button)
{
   if (! my_button)
      return False;

   return (my_button->flags & GWX_HAS_DEFAULT);
}



/*
 *	Allocates a new pixel color with the given rgb values.
 *
 *	The returned Pixel value needs to be deallocated by calling
 *	GWXPixelDelete().
 */
static Pixel GWXPixelNew(
	gw_display_struct *display,
	u_int8_t r, u_int8_t g, u_int8_t b
)
{
	Display *dpy = (display != NULL) ? display->display : NULL;
	Colormap cmap;
	XColor c;
	if(dpy == NULL)
	    return(0);

	cmap = display->colormap;
	if(cmap == None)
	    return(0);

	XCOLOR_SET_RGB_COEFF(
	    &c,
	    (float)r / (float)0xff,
	    (float)g / (float)0xff,
	    (float)b / (float)0xff
	);

	if(XAllocColor(dpy, cmap, &c))
	    return(c.pixel);
	else
	    return(0);
}
/*
 *	Deletess a pixel returned by GWXPixelNew().
 */
static void GWXPixelDelete(gw_display_struct *display, Pixel pixel)
{
	Display *dpy = (display != NULL) ? display->display : NULL;
	Colormap cmap;
	if(dpy == NULL)
	    return;

	cmap = display->colormap;
	if(cmap == None)
	    return;

	XFreeColors(dpy, cmap, &pixel, 1, 0);
}

/*
 *	Creates (loads) a font from the given font name.
 *
 *	Can return NULL on error.
 */
static XFontStruct *GWXFontNew(
	gw_display_struct *display, const char *name
)
{
	Display *dpy = (display != NULL) ? display->display : NULL;
	if((dpy == NULL) || (name == NULL))
	    return(NULL);

	return(XLoadQueryFont(dpy, name));
}
/*
 *	Deletes a font returned by GWXFontNew().
 */
static void GWXFontDelete(gw_display_struct *display, XFontStruct *font)
{
	Display *dpy = (display != NULL) ? display->display : NULL;
	if((dpy == NULL) || (font == NULL))
	    return;

	XFreeFont(dpy, font);
}

/*
 *	Returns the length of the string is in pixels with respect to
 *	the size of the font.
 *
 *	The string is terminated by either a '\0' or '\n' character
 *	(exclusive), so it will never be counted beyond its end or
 *	end of line.
 *
 *	Can return 0 on error.
 */
static int GWXGetFontStringLength(XFontStruct *font, const char *s)
{
	int len = 0;		/* In pixels */
	XCharStruct *csp;


	if((font == NULL) || (s == NULL))
	    return(len);

	/* Do we have per character geometry information? */
	if(font->per_char != NULL)
	{
	    /* 8 bits per character type? */
	    if(!font->min_byte1 && !font->max_byte1)
	    {
		int c;
		int i = 0;
		int m = (int)font->max_char_or_byte2 -
			(int)font->min_char_or_byte2;

		/* Iterate through given string */
		while((*s != '\0') && (*s != '\n') && (*s != '\r'))
		{
		    c = *s++;	/* Get current string char and increment */
		    c -= (int)font->min_char_or_byte2;	/* Offset for per_char index */

		    /* Clip, check if string char in bounds? */
		    if((c >= i) && (c <= m))
		    {
			csp = &font->per_char[c];	/* Ptr to geometry */
			len += csp->width;		/* Inc str len */
		    }
		}
	    }
	    else
	    {
		/* 16 bits per character, ouch, code this in later */


	    }
	}
	else
	{
	    /* No per character info, use max bounds */
	    csp = &font->max_bounds;

	    /* Iterate through given string */
	    while((*s != '\0') && (*s != '\n') && (*s != '\r'))
		len += csp->width;
	}

	return(len);
}


/*
 *	Sets the specified flags on the widget.
 */
void GWXWidgetSetFlags(
	gw_display_struct *display, gwx_widget_struct *widget,
	gwx_widget_flags flags
)
{
	if(widget == NULL)
	    return;

	widget->flags |= flags;
}

/*
 *	Unsets the specified flags on the widget.
 */
void GWXWidgetUnsetFlags(
	gw_display_struct *display, gwx_widget_struct *widget,
	gwx_widget_flags flags
)
{
	if(widget == NULL)
	    return;

	widget->flags &= ~flags;
}

/*
 *	Maps and raises the widget's toplevel Window.
 */
void GWXWidgetMapRaise(
	gw_display_struct *display, gwx_widget_struct *widget
)
{
	Display *dpy = (display != NULL) ? display->display : NULL;
	if((dpy == NULL) || (widget == NULL))
	    return;

	if(widget->toplevel != None)
	    XMapRaised(dpy, widget->toplevel);
	GWXWidgetSetFlags(display, widget, GWX_MAPPED);
}

/*
 *	Maps the widget's toplevel Window.
 */
void GWXWidgetMap(
	gw_display_struct *display, gwx_widget_struct *widget
)
{
	Display *dpy = (display != NULL) ? display->display : NULL;
	if((dpy == NULL) || (widget == NULL))
	    return;

	if(widget->toplevel != None)
	    XMapWindow(dpy, widget->toplevel);
	GWXWidgetSetFlags(display, widget, GWX_MAPPED);
}

/*
 *	Unmaps the widget's toplevel Window.
 */
void GWXWidgetUnmap(
	gw_display_struct *display, gwx_widget_struct *widget
)
{
	Display *dpy = (display != NULL) ? display->display : NULL;
	if((dpy == NULL) || (widget == NULL))
	    return;

	if(widget->toplevel != None)
	    XUnmapWindow(dpy, widget->toplevel);
	GWXWidgetUnsetFlags(display, widget, GWX_MAPPED);
}

/*
 *	Sets the GWX_HAS_FOCUS flag on the widget (if the widget has
 *	the GWX_CAN_FOCUS flag).
 */
void GWXWidgetFocus(
	gw_display_struct *display, gwx_widget_struct *widget
)
{
	if(GWX_WIDGET_CAN_FOCUS(widget))
	    GWXWidgetSetFlags(display, widget, GWX_HAS_FOCUS);
}

/*
 *	Unsets the GWX_HAS_FOCUS flag on the widget.
 */
void GWXWidgetUnfocus(
	gw_display_struct *display, gwx_widget_struct *widget
)
{
	GWXWidgetUnsetFlags(display, widget, GWX_HAS_FOCUS);
}

/*
 *	Sets the GWX_HAS_DEFAULT flag on the widget (if the widget has
 *	the GWX_CAN_DEFAULT flag).
 */
void GWXWidgetGrabDefault(
	gw_display_struct *display, gwx_widget_struct *widget
)
{
	if(GWX_WIDGET_CAN_DEFAULT(widget))
	    GWXWidgetSetFlags(display, widget, GWX_HAS_DEFAULT);
}

/*
 *	Unsets the GWX_HAS_DEFAULT flag on the widget.
 */
void GWXWidgetUngrabDefault(
	gw_display_struct *display, gwx_widget_struct *widget
)
{
	GWXWidgetUnsetFlags(display, widget, GWX_HAS_DEFAULT);
}

/*
 *	Sets the widget as sensitive or insensitive.
 */
void GWXWidgetSetSensitive(
	gw_display_struct *display, gwx_widget_struct *widget,
	Boolean sensitive
)
{
	if(sensitive)
	    GWXWidgetSetFlags(display, widget, GWX_SENSITIVE);
	else
	    GWXWidgetUnsetFlags(display, widget, GWX_SENSITIVE);
}


/*
 *	Creates a button by setting up the given button structure's
 *	values. Returns non-zero on error.
 */
int GWXButtonCreate(
	gw_display_struct *display, gwx_button_struct *btn,
	void *dialog,           /* Parent gwx_dialog_struct */
	Window parent,
	int x, int y,
	unsigned int width, unsigned int height,
	const char *label,
	void *client_data,
	void (*func_cb)(void *, void *)
)
{
	Display *dpy = (display != NULL) ? display->display : NULL;
	Window w;
	XFontStruct *font;
	u_int8_t *sc;		/* Style color */
	if((dpy == NULL) || (btn == NULL) || (parent == None))
	    return(-1);

	memset(btn, 0x00, sizeof(gwx_button_struct));

	btn->flags =	GWX_CAN_FOCUS | GWX_CAN_DEFAULT |
			GWX_SENSITIVE;
	btn->parent = parent;
	btn->x = x;
	btn->y = y;
	btn->width = width;
	btn->height = height;
	btn->dialog = dialog;

	/* Create button's toplevel window */
	w = GWCreateWindow(
	    display, parent,
	    x, y,
	    width, height,
	    label
	);
	if(w == None)
	    return(-1);
	else
	    btn->toplevel = w;
	XSelectInput(
	    dpy, w, 
	    ButtonPressMask | ButtonReleaseMask |
	    ExposureMask |
	    EnterWindowMask | LeaveWindowMask
	);

	btn->accelerator = NULL;
	btn->total_accelerators = 0;

	sc = style_fg_color;
	btn->color_fg = GWXPixelNew(display, sc[1], sc[2], sc[3]);
	sc = style_bg_color;
	btn->color_bg = GWXPixelNew(display, sc[1], sc[2], sc[3]);
	sc = style_fg_insensitive;
	btn->color_fg_insensitive = GWXPixelNew(display, sc[1], sc[2], sc[3]);
	sc = style_bg_highlight_color;
	btn->color_bg_highlighted = GWXPixelNew(display, sc[1], sc[2], sc[3]);
	sc = style_highlight_color;
	btn->color_highlight = GWXPixelNew(display, sc[1], sc[2], sc[3]);
	sc = style_shade_color;
	btn->color_shade = GWXPixelNew(display, sc[1], sc[2], sc[3]);

	btn->colors_initialized = True;

	btn->font = font = GWXFontNew(
	    display, display->def_xfont_name
	);

	btn->label = STRDUP(label);
	btn->label_len_pixels = GWXGetFontStringLength(
	    font, label
	);
	if(font == NULL)
	    btn->label_height_pixels = 0;
	else
	    btn->label_height_pixels = font->max_bounds.ascent +
		font->max_bounds.descent;

	btn->client_data = client_data;
	btn->func_cb = func_cb;

	GWXButtonResize(display, btn);

	return(0);
}

/*
 *	Recalculates the size of the button's toplevel window and
 *      sets the new size, then calls GWXButtonResize() to realize the
 *      changes.
 */
void GWXButtonUpdateSize(
	gw_display_struct *display, gwx_button_struct *btn
)
{
	Display *dpy = (display != NULL) ? display->display : NULL;
	Window w;
	if((dpy == NULL) || (btn == NULL))
	    return;

	w = btn->toplevel;
	if(w == None)
	    return;




	GWXButtonResize(display, btn);
}

/*
 *	Resizes all button resources to its newly resized toplevel
 *	window size.
 */
void GWXButtonResize(gw_display_struct *display, gwx_button_struct *btn)
{
	Display *dpy = (display != NULL) ? display->display : NULL;
	GC gc;
	Window w;
	XWindowAttributes wattr;
	Boolean size_changed = False;
	if((dpy == NULL) || (btn == NULL))
	    return;

	gc = display->gc;
	w = btn->toplevel;
	if((gc == None) || (w == None))
	    return;

/*	XSync(dpy, False); */
	XGetWindowAttributes(dpy, w, &wattr);
	if((wattr.width != btn->width) ||
	   (wattr.height != btn->height)
	)
	    size_changed = True;

	btn->x = wattr.x;
	btn->y = wattr.y;
	btn->width = wattr.width;
	btn->height = wattr.height;

	if(size_changed || (btn->toplevel_buf == None))
	{
	    FREE_PIXMAP(&btn->toplevel_buf);
	    btn->toplevel_buf = XCreatePixmap(
	        dpy, btn->toplevel, btn->width, btn->height, display->depth
	    );
	}
}

/*
 *	Redraws the button.
 */
void GWXButtonDraw(gw_display_struct *display, gwx_button_struct *btn)
{
	Display *dpy = (display != NULL) ? display->display : NULL;
	GC gc;
	Window w;
	Pixmap pm;
	Pixel c_fg = 0l, c_bg = 0l, c_highlight = 0l, c_shade = 0l;
	unsigned int width, height;
	XPoint p[3];
	char *label;
	XFontStruct *font;
	int label_len_pixels, label_height_pixels;
	XGCValues gcv;
	if((dpy == NULL) || (btn == NULL))
	    return;

	gc = display->gc;
	w = btn->toplevel;
	pm = btn->toplevel_buf;
	if((gc == None) || (w == None) || (pm == None))
	    return;

	width = btn->width;
	height = btn->height;

	GWXWidgetMapRaise(display, GWX_WIDGET(btn));

	switch(btn->state)
	{
	  case GWX_BUTTON_STATE_UNARMED:
	    c_fg = btn->color_fg;   
	    c_bg = btn->color_bg;
	    c_highlight = btn->color_highlight;
	    c_shade = btn->color_shade;
	    break;
	  case GWX_BUTTON_STATE_ARMED:
	    c_fg = btn->color_fg;
	    c_bg = btn->color_bg_highlighted;
	    c_highlight = btn->color_shade;
	    c_shade = btn->color_highlight;
	    break;
	  case GWX_BUTTON_STATE_HIGHLIGHTED:
	    c_fg = btn->color_fg;
	    c_bg = btn->color_bg_highlighted;
	    c_highlight = btn->color_highlight;
	    c_shade = btn->color_shade;
	    break;
	}
	font = btn->font;


	/* Draw (clear) background */
	XSetForeground(dpy, gc, c_bg);
	XFillRectangle(dpy, pm, gc, 0, 0, width, height);

	/* Draw highlight */
	XSetForeground(dpy, gc, c_highlight);
	XSetLineAttributes(
	    dpy, gc,
	    2, LineSolid, CapRound, JoinMiter
	);
	p[0].x = 1;
	p[0].y = (int)((int)height - 1);
	p[1].x = 1;
	p[1].y = 1;
	p[2].x = (int)((int)width - 1);
	p[2].y = 1;
	XDrawLines(
	    dpy, pm, gc,
	    &(p[0]), 3,		/* Point array and number of points */
	    CoordModeOrigin
	);

	/* Draw shadow */
	XSetForeground(dpy, gc, c_shade);
	p[0].x = (int)((int)width - 1);
	p[0].y = 0;
	p[1].x = (int)((int)width - 1);
	p[1].y = (int)((int)height - 1);
	p[2].x = 0;
	p[2].y = (int)((int)height - 1);
	XDrawLines(
	    dpy, pm, gc,
	    &(p[0]), 3,         /* Point array and number of points */
	    CoordModeOrigin
	);


	/* Get pointer to button label */
	label = btn->label;
	label_len_pixels = btn->label_len_pixels;
	label_height_pixels = btn->label_height_pixels;
	/* Got enough info to draw button's label? */
	if((font != NULL) && (label != NULL) &&
	   (label_len_pixels > 0) && (label_height_pixels > 0)
	)
	{
	    /* Calculate length and position to draw label */
	    int len = STRLEN(label);
	    int x = (int)(((int)width / 2) - (label_len_pixels / 2));
	    int y = (int)(((int)height / 2) - (label_height_pixels / 2));

	    /* Set up GC for button label drawing */
	    if(font->fid != None)
		XSetFont(dpy, gc, font->fid);
	    XSetForeground(
		dpy, gc,
		GWX_WIDGET_SENSITIVE(btn) ? c_fg : btn->color_fg
	    );

	    /* Draw the button's label */
	    XDrawString(
		dpy, pm, gc,
		x, y + font->max_bounds.ascent,
		label, len
	    );

	    /* Button has default? */
	    if(GWX_WIDGET_CAN_DEFAULT(btn) &&
	       GWX_WIDGET_HAS_DEFAULT(btn)
	    )
	    {
		int focus_margin = 3;
		unsigned long gcv_mask =	GCFunction | GCCapStyle |
						GCJoinStyle | GCLineWidth |
						GCLineStyle | GCDashOffset |
						GCDashList;

		/* Set up GC to draw `focus rectangle' */
		gcv.function = GXinvert;
		gcv.cap_style = CapNotLast;
		gcv.join_style = JoinBevel;
		gcv.line_width = 1;
		gcv.line_style = LineOnOffDash;
		gcv.dash_offset = 0;
		gcv.dashes = 1;
		XChangeGC(dpy, gc, gcv_mask, &gcv);

		/* Draw `focus rectangle' */
		XDrawRectangle(
		    dpy, pm, gc,
		    0 + focus_margin, 0 + focus_margin,
		    width - (focus_margin * 2) - 1,
		    height - (focus_margin * 2) - 1
		);

		/* Restore GC */
		gcv.function = GXcopy;
		gcv.cap_style = CapNotLast;
		gcv.join_style = JoinMiter;
		gcv.line_width = 1;
		gcv.line_style = LineSolid;
		gcv.dash_offset = 0;
		gcv.dashes = 2;
		XChangeGC(dpy, gc, gcv_mask, &gcv);
	    }
	}

	/* Put Pixmap buffer to Window */
	XCopyArea(dpy, pm, w, gc, 0, 0, width, height, 0, 0);
}

/*
 *	Manages the given event if it is for the button.
 */
int GWXButtonManage(
	gw_display_struct *display, gwx_button_struct *btn,
	XEvent *event
)
{
	Display *dpy = (display != NULL) ? display->display : NULL;
	Window w, ew;
	int events_handled = 0;
	if((dpy == NULL) || (btn == NULL) || (event == NULL))
	    return(events_handled);

	w = btn->toplevel;
	if(w == None)
	    return(events_handled);

	ew = event->xany.window;

	switch(event->type)
	{
	  case ButtonPress:
	    if(w == ew)
	    {
		btn->state = GWX_BUTTON_STATE_ARMED;
		GWXButtonDraw(display, btn);
		events_handled++;
	    }
	    break;

	  case ButtonRelease:
	    if(w == ew)
	    {
		if(btn->state == GWX_BUTTON_STATE_ARMED)
		{
		    btn->state = GWX_BUTTON_STATE_HIGHLIGHTED;
		    GWXButtonDraw(display, btn);
		    events_handled++;

		    if(btn->func_cb != NULL)
		    {
			btn->func_cb(
			    (void *)btn,	/* This object */
			    btn->client_data	/* Client data */
			);
		    }
		}
	    }
	    break;

	  case Expose:
	    if(w == ew)
	    {
		GWXButtonDraw(display, btn);
		events_handled++;
	    }
	    break;

	  case EnterNotify:
	    if(w == ew)
	    {
		btn->state = GWX_BUTTON_STATE_HIGHLIGHTED;
		GWXButtonDraw(display, btn);
		events_handled++;
	    }
	    break;

	  case LeaveNotify:
	    if(w == ew)
	    {
		btn->state = GWX_BUTTON_STATE_UNARMED;
		GWXButtonDraw(display, btn);
		events_handled++;
	    }
	    break;
	}

	return(events_handled);
}

/*
 *	Deallocates all resources on the button structure but does not
 *	deallocate the structure itself.
 */
void GWXButtonDestroy(gw_display_struct *display, gwx_button_struct *btn)
{
	Display *dpy = (display != NULL) ? display->display : NULL;
	if((dpy == NULL) || (btn == NULL))
	    return;

	DESTROY_WINDOW(&btn->toplevel);
	FREE_PIXMAP(&btn->toplevel_buf);

	if(btn->colors_initialized)
	{
	    GWXPixelDelete(display, btn->color_fg);
	    GWXPixelDelete(display, btn->color_bg);
	    GWXPixelDelete(display, btn->color_fg_insensitive);
	    GWXPixelDelete(display, btn->color_bg_highlighted);
	    GWXPixelDelete(display, btn->color_highlight);
	    GWXPixelDelete(display, btn->color_shade);
	    btn->colors_initialized = False;
	}

	DELETE_FONT(&btn->font);

	free(btn->label);
	btn->label = NULL;

	GWAcceleratorListDelete(
	    &btn->accelerator, &btn->total_accelerators
	);

	memset(btn, 0x00, sizeof(gwx_button_struct));
}


/*
 *	Create new message dialog by setting up the values in the given
 *	message dialog structure.
 */
int GWXDialogCreate(
	gw_display_struct *display, gwx_dialog_struct *md,
	gwx_dialog_type type
)
{
	Display *dpy = (display != NULL) ? display->display : NULL;
	Window w, parent;
	const char *cstrptr;
	int len;
	u_int8_t *sc;
#ifdef MWMUTIL_H
	PropMwmHints mwm_prop;
#endif	/* MWMUTIL_H */
	gwx_button_struct *btn;
	if((dpy == NULL) || (md == NULL))
	    return(-1);

	parent = display->root;
	if(parent == None)
	    return(-1);

	memset(md, 0x00, sizeof(gwx_dialog_struct));

	md->flags =	GWX_CAN_FOCUS | GWX_CAN_DEFAULT |
			GWX_SENSITIVE;
	md->parent = parent;
	md->x = 0;
	md->y = 0;
	md->width = 200;
	md->height = 100;

	md->type = type;
	md->margin = 10;

	w = GWCreateWindow(
	    display, parent,
	    md->x, md->y,
	    md->width, md->height,
	    "Message Dialog"
	);
	if(w == None)
	    return(-1);
	else
	    md->toplevel = w;

	XSelectInput(
	    dpy, w,
	    KeyPressMask | KeyReleaseMask |
	    ExposureMask | FocusChangeMask |
	    VisibilityChangeMask | StructureNotifyMask
	);

#ifdef MWMUTIL_H
	/* Set up decorations */
	mwm_prop.flags =	MWM_HINTS_FUNCTIONS | MWM_HINTS_DECORATIONS
/*
				| MWM_HINTS_INPUT_MODE | MWM_HINTS_STATUS
 */
	;
	mwm_prop.functions =	/* MWM_FUNC_RESIZE | */
				MWM_FUNC_MOVE     |
				/* MWM_FUNC_MINIMIZE | */
				/* MWM_FUNC_MAXIMIZE | */
				MWM_FUNC_CLOSE
	;
	mwm_prop.decorations =	MWM_DECOR_BORDER   |
				/* MWM_DECOR_RESIZEH  | */
				MWM_DECOR_TITLE |
				MWM_DECOR_MENU
				/* MWM_DECOR_MINIMIZE | */
				/* MWM_DECOR_MAXIMIZE */
	;
	mwm_prop.inputMode = 0;
	mwm_prop.status = 0;
	XChangeProperty(
	    dpy, w,
	    display->atom_wm_motif_hints,	/* Property atom */
	    display->atom_wm_motif_hints,	/* Type atom */
	    32,					/* Bits */
	    PropModeReplace,			/* Format */
	    (unsigned char *)&mwm_prop,		/* Data */
	    PROP_MWM_HINTS_ELEMENTS		/* Number of elements */
	);
#endif	/* MWMUTIL_H */

	/* Set transient for the first toplevel Window */
	if((display->total_gl_contexts > 0) ?
	    (display->toplevel[0] != None) : False
	)
	    XSetTransientForHint(dpy, w, display->toplevel[0]);

	sc = style_fg_color;
	md->color_fg = GWXPixelNew(display, sc[1], sc[2], sc[3]);
	sc = style_bg_color;
	md->color_bg = GWXPixelNew(display, sc[1], sc[2], sc[3]);
	sc = style_highlight_color;
	md->color_highlight = GWXPixelNew(display, sc[1], sc[2], sc[3]);
	sc = style_shade_color;
	md->color_shade = GWXPixelNew(display, sc[1], sc[2], sc[3]);

	md->colors_initialized = True;

	md->font = GWXFontNew(
	    display, display->def_xfont_name
	);


	switch(type)
	{
	  case GWX_DIALOG_TYPE_CONF_CANCEL:
	    cstrptr = "Cancel";
	    len = STRLEN(cstrptr);
	    btn = &md->cancel_btn;
	    GWXButtonCreate(
		display, btn, md, w,
		0, 0,
		GWX_BTN_DEF_WIDTH,
		GWX_BTN_DEF_HEIGHT,
		cstrptr,
		display, GWXDialogCancelBtnCB
	    );
	    GWAcceleratorListAdd(
		&btn->accelerator, &btn->total_accelerators,
		'c', 0
	    );
	    GWAcceleratorListAdd(
		&btn->accelerator, &btn->total_accelerators,
		0x1b, 0
	    );
	    /* Fall through */

	  case GWX_DIALOG_TYPE_CONF:
	    cstrptr = "No";
	    len = STRLEN(cstrptr);
	    btn = &md->no_btn;
	    GWXButtonCreate(
		display, btn, md, w,
		0, 0,
		GWX_BTN_DEF_WIDTH,
		GWX_BTN_DEF_HEIGHT,
		cstrptr,
		display, GWXDialogNoBtnCB
	    );
	    GWAcceleratorListAdd(
		&btn->accelerator, &btn->total_accelerators,
		'n', 0
	    );
	    if(type == GWX_DIALOG_TYPE_CONF)
		GWAcceleratorListAdd(
		    &btn->accelerator, &btn->total_accelerators,
		    0x1b, 0
		);

	    cstrptr = "Yes";
	    len = STRLEN(cstrptr);
	    btn = &md->yes_btn;
	    GWXButtonCreate(
		display, btn, md, w,
		0, 0,
		GWX_BTN_DEF_WIDTH,
		GWX_BTN_DEF_HEIGHT,
		cstrptr,
		display, GWXDialogYesBtnCB
	    );
	    GWAcceleratorListAdd(
		&btn->accelerator, &btn->total_accelerators,
		'y', 0
	    );
	    break;

	  case GWX_DIALOG_TYPE_MESG:
	    cstrptr = "OK";
	    len = STRLEN(cstrptr);
	    btn = &md->ok_btn;
	    GWXButtonCreate(
	        display, btn, md, w,
	        0, 0,
		GWX_BTN_DEF_WIDTH,
		GWX_BTN_DEF_HEIGHT,
	        cstrptr,
	        display, GWXDialogOKBtnCB
	    );
	    GWAcceleratorListAdd(
		&btn->accelerator, &btn->total_accelerators,
		'o', 0
	    );
	    GWAcceleratorListAdd(
		&btn->accelerator, &btn->total_accelerators,
		0x1b, 0
	    );
	    break;
	}


	GWXDialogResize(display, md);

	return(0);
}

/*
 *	Loads the message dialog icon (the icon displayed next to
 *	dialog's text, not the WM icon).
 */
void GWXDialogLoadIcon(
	gw_display_struct *display, gwx_dialog_struct *md,
	gwx_icon icon_code		/* One of GWX_ICON_* */
)
{
	Display *dpy = (display != NULL) ? display->display : NULL;
	GC gc;
	Window w, *icon_w;
	Pixmap *icon, *icon_mask;
	unsigned int btn_height = 0;
#ifdef XPM_H
	int xpm_status;
	XpmAttributes xpm_attr;
	char **xpm_data;
#endif
	if((dpy == NULL) || (md == NULL))
	    return;

	gc = display->gc;
	w = md->toplevel;
	if((gc == None) || (w == None))
	    return;

	/* Calculate height based on type of dialog */
	switch(md->type)
	{
	  case GWX_DIALOG_TYPE_CONF_CANCEL:
	  case GWX_DIALOG_TYPE_CONF:
	    btn_height = md->yes_btn.height;
	    break;
	  case GWX_DIALOG_TYPE_MESG:
	    btn_height = md->ok_btn.height;
	    break;
	}

	/* Get pointers to icon window, pixmap, and pixmap mask on dialog
	 * structure.
	 */
	icon_w = &md->icon_w;
	icon = &md->icon;
	icon_mask = &md->icon_mask;

	/* Destroy old icons and window if any */
	DESTROY_WINDOW(icon_w);
	FREE_PIXMAP(icon);
	FREE_BITMAP(icon_mask);

#ifdef XPM_H
	/* Get icon data based on the icon code */  
	switch(icon_code)
	{
	  case GWX_ICON_WARNING:
	    xpm_data = icon_warning_xpm;
	    break;

	  case GWX_ICON_ERROR:
	    xpm_data = icon_error_xpm;
	    break;

	  case GWX_ICON_QUESTION:
	    xpm_data = icon_question_xpm;
	    break;

	  default:
	    xpm_data = icon_info_xpm;
	    break;
	}

	/* Set up XPM attributes for the loading of the XPM data */
	xpm_attr.valuemask =	XpmCloseness | XpmVisual | XpmColormap |
				XpmSize | XpmDepth;
	xpm_attr.closeness = XpmDefaultColorCloseness;
	xpm_attr.depth = display->depth;
	xpm_attr.visual = display->visual;
	xpm_attr.colormap = display->colormap;
 
	/* Load XPM data */
	xpm_status = XpmCreatePixmapFromData(
	    dpy, w, xpm_data,
	    icon, icon_mask,
	    &xpm_attr
	);
	if(xpm_status == XpmSuccess)
	{
	    /* Get loaded XPM size */
	    md->icon_width = xpm_attr.width,
	    md->icon_height = xpm_attr.height;

	    /* Create icon Window */
	    *icon_w = GWCreateWindow(
		display, w,
		0, 0,
		md->icon_width, md->icon_height,
		NULL
	    );
	    if(*icon_w != None)
	    {
		/* Begin setting up icon Window */
		XSelectInput(dpy, *icon_w, ExposureMask);
		XShapeCombineMask(
		    dpy, *icon_w,
		    ShapeBounding,
		    0, 0,
		    *icon_mask,
		    ShapeSet
		);
		XMapRaised(dpy, *icon_w);
	    }
	}
#endif	/* XPM_H */

	/* Update size of dialog due to change in icon size */
	GWXDialogUpdateSize(display, md);
}

/*
 *	Recalculates the size of the dialog's toplevel window and
 *	sets the new size, then calls GWXDialogResize() to realize the
 *	changes.
 */
void GWXDialogUpdateSize(gw_display_struct *display, gwx_dialog_struct *md)
{
	Display *dpy = (display != NULL) ? display->display : NULL;
	Window w;
	unsigned int width, height;
	unsigned int btn_width_total = 0, btn_height = 0;
	gwx_button_struct *btn;
	if((dpy == NULL) || (md == NULL))
	    return;

	w = md->toplevel;
	if(w == None)
	    return;

	/* Get button height depending on dialog type */
	switch(md->type)
	{
	  case GWX_DIALOG_TYPE_CONF_CANCEL:
	    btn = &md->cancel_btn;
	    btn_width_total += btn->width + md->margin;
	    btn_height = MAX(btn->height, btn_height);
	  case GWX_DIALOG_TYPE_CONF:
	    btn = &md->yes_btn;
	    btn_width_total += btn->width + md->margin;
	    btn_height = MAX(btn->height, btn_height);
	    btn = &md->no_btn;
	    btn_width_total += btn->width;
	    btn_height = MAX(btn->height, btn_height);
	    break;
	  case GWX_DIALOG_TYPE_MESG:
	    btn = &md->ok_btn;
	    btn_width_total += btn->width;
	    btn_height = MAX(btn->height, btn_height);
	    break;
	}

	/* Calculate new size of dialog */
	width = (unsigned int)(
	    ((md->icon_w == None) ? (md->margin * 2) : (md->margin * 3)) +
	    md->icon_width +
	    MAX(md->longest_line_pixels, btn_width_total)
	);
	height = (unsigned int)MAX(
	    (md->margin * 4) + btn_height + md->icon_height,
	    (md->margin * 4) + btn_height + md->lines_height_pixels
	);

	/* Set new toplevel Window size */
	XResizeWindow(dpy, w, width, height);

	GWXDialogResize(display, md);
}

/*
 *	Resizes message dialog resources with respect to the dialog's
 *	toplevel window.
 */
void GWXDialogResize(gw_display_struct *display, gwx_dialog_struct *md)
{
	Display *dpy = (display != NULL) ? display->display : NULL;
	GC gc; 
	Window w;
	Boolean size_changed = False;
	XWindowAttributes wattr;
	gwx_button_struct *btn;
	int btn_x, btn_width_total;
	if((dpy == NULL) || (md == NULL))
	    return;

	gc = display->gc;
	w = md->toplevel;
	if((gc == None) || (w == None))
	    return;

/*	XSync(dpy, False); */
	XGetWindowAttributes(dpy, w, &wattr);
	if((wattr.width != md->width) ||
	   (wattr.height != md->height)
	)
	    size_changed = True;

	md->x = wattr.x;
	md->y = wattr.y;
	md->width = wattr.width;
	md->height = wattr.height;

	if(size_changed || (md->toplevel_buf == None))
	{
	    FREE_PIXMAP(&md->toplevel_buf);
	    md->toplevel_buf = XCreatePixmap(
		dpy, md->toplevel, md->width, md->height, display->depth
	    );
	}

	/* Move icon window? */
	if(md->icon_w != None)
	    XMoveWindow(
		dpy, md->icon_w,
		(int)md->margin,
		(int)md->margin + (int)MAX(
		    (md->lines_height_pixels / 2) - ((int)md->icon_height / 2),
		    0
		)
	    );

	/* Reposition buttons */
	switch(md->type)
	{
	  case GWX_DIALOG_TYPE_CONF_CANCEL:
	    btn_width_total = 0;
	    btn = &md->yes_btn;
	    btn_width_total += btn->width + md->margin;
	    btn = &md->no_btn;
	    btn_width_total += btn->width + md->margin;
	    btn = &md->cancel_btn;
	    btn_width_total += btn->width;

	    btn_x = ((int)md->width / 2) - (btn_width_total / 2);

	    btn = &md->yes_btn;
	    XMoveWindow(
		dpy, btn->toplevel,
		btn_x,
		(int)((int)md->height - (int)md->margin - (int)btn->height)
	    );
	    GWXButtonResize(display, btn);
	    btn_x += btn->width + md->margin;

	    btn = &md->no_btn;
	    XMoveWindow(
		dpy, btn->toplevel,
		btn_x,
		(int)((int)md->height - (int)md->margin - (int)btn->height)
	    );
	    GWXButtonResize(display, btn);
	    btn_x += btn->width + md->margin;

	    btn = &md->cancel_btn;
	    XMoveWindow(
		dpy, btn->toplevel,
		btn_x,
		(int)((int)md->height - (int)md->margin - (int)btn->height)
	    );
	    GWXButtonResize(display, btn);

	    break;

	  case GWX_DIALOG_TYPE_CONF:
	    btn_width_total = 0;
	    btn = &md->yes_btn;
	    btn_width_total += btn->width + md->margin;
	    btn = &md->no_btn;
	    btn_width_total += btn->width;

	    btn_x = ((int)md->width / 2) - (btn_width_total / 2);

	    btn = &md->yes_btn;
	    XMoveWindow(
		dpy, btn->toplevel,
		btn_x,
		(int)((int)md->height - (int)md->margin - (int)btn->height)
	    );
	    GWXButtonResize(display, btn);
	    btn_x += btn->width + md->margin;

	    btn = &md->no_btn;
	    XMoveWindow(
		dpy, btn->toplevel,
		btn_x,
		(int)((int)md->height - (int)md->margin - (int)btn->height)
	    );
	    GWXButtonResize(display, btn);

	    break;

	  case GWX_DIALOG_TYPE_MESG:
	    btn_width_total = 0;
	    btn = &md->ok_btn;
	    btn_width_total += btn->width;

	    btn_x = ((int)md->width / 2) - (btn_width_total / 2);

	    btn = &md->ok_btn;
	    XMoveWindow(
		dpy, btn->toplevel,
		btn_x,
		(int)((int)md->height - (int)md->margin - (int)btn->height)
	    ); 
	    GWXButtonResize(display, btn);
	    break;
	}
}

/*
 *	Redraws message dialog.
 */
void GWXDialogDraw(gw_display_struct *display, gwx_dialog_struct *md)
{
	Display *dpy = (display != NULL) ? display->display : NULL;
	GC gc;
	Window w;
	Pixmap pm;
	Pixel c_fg, c_bg, c_highlight, c_shade;
	unsigned int width, height, margin;
	unsigned int btn_height = 0;
	XFontStruct *font;
	char **strv;
	int strc;
	int line_height_pixels;
	XPoint p[2];
	if((dpy == NULL) || (md == NULL))
	    return;

	gc = display->gc;
	w = md->toplevel;
	pm = md->toplevel_buf;
	if((gc == None) || (w == None) || (pm == None))
	    return;

	width = md->width;
	height = md->height;
	margin = md->margin;

	GWXWidgetMapRaise(display, GWX_WIDGET(md));

	switch(md->type)
	{
	  case GWX_DIALOG_TYPE_CONF_CANCEL:
	  case GWX_DIALOG_TYPE_CONF:
	    btn_height = md->yes_btn.height;
	    break;
	    
	  case GWX_DIALOG_TYPE_MESG:
	    btn_height = md->ok_btn.height;
	    break;
	}
	    
	c_fg = md->color_fg;
	c_bg = md->color_bg;
	c_highlight = md->color_highlight;
	c_shade = md->color_shade;

	font = md->font;


	/* Draw background */
	XSetForeground(dpy, gc, c_bg);
	XFillRectangle(dpy, pm, gc, 0, 0, width, height);


	/* Draw HR */
	XSetForeground(dpy, gc, c_shade);
	XSetLineAttributes(
	    dpy, gc,
	    2, LineSolid, CapRound, JoinMiter
	);
	p[0].x = 0;
	p[0].y = (int)((int)height - ((int)md->margin * 2) - (int)btn_height);
	p[1].x = (int)width;
	p[1].y = p[0].y;
	XDrawLines(
	    dpy, pm, gc,
	    &(p[0]), 2,         /* Point array and number of points */
	    CoordModeOrigin
	);

	XSetForeground(dpy, gc, c_highlight);
	p[0].y += 1;
	p[1].y = p[0].y;
	XDrawLines(
	    dpy, pm, gc,
	    &(p[0]), 2,         /* Point array and number of points */
	    CoordModeOrigin
	);


	/* Get values for drawing message lines */
	strv = md->strv;
	strc = md->strc;
	line_height_pixels = md->line_height_pixels;

	/* Got enough information to draw message lines? */
	if((font != NULL) && (strv != NULL) &&
	   (line_height_pixels > 0)
	)
	{
	    int	x = (int)((md->margin * 2) + md->icon_width),
		y = (int)md->margin,
		i, len;
	    const char *s;

	    /* Set up GC for message lines drawing */
	    if(font->fid != None)
		XSetFont(dpy, gc, font->fid);
	    XSetForeground(dpy, gc, c_fg);

	    /* Draw each message line */
	    for(i = 0; i < strc; i++)
	    {
		s = (const char *)strv[i];
		if(s == NULL)
		    continue;

		len = STRLEN(s);
		if(len > 0)
		    XDrawString(
			dpy, pm, gc,
			x, y + font->max_bounds.ascent,
			s, len
		    );

		y += line_height_pixels;
	    }
	}

	/* Copy Pixmap buffer to Window */
	XCopyArea(dpy, pm, w, gc, 0, 0, width, height, 0, 0);


	/* Draw icon */
	w = md->icon_w;
	pm = md->icon;
	width = md->icon_width;
	height = md->icon_height;
	if((w != None) && (pm != None) &&
	   (width > 0) && (height > 0)
	)
	    XCopyArea(dpy, pm, w, gc, 0, 0, width, height, 0, 0);
}

/*
 *	Manages the event if it is for the given message dialog.
 */
int GWXDialogManage(
	gw_display_struct *display, gwx_dialog_struct *md,
	XEvent *event
)
{
	Display *dpy = (display != NULL) ? display->display : NULL;
	Window w, ew;
	int events_handled = 0;
	if((dpy == NULL) || (md == NULL) || (event == NULL))
	    return(events_handled);

	w = md->toplevel;
	if(w == None)
	    return(events_handled);

	ew = event->xany.window; 

	switch(event->type)
	{
	  case Expose:
	    if(w == ew)
	    {
		GWXDialogDraw(display, md);
		events_handled++;
	    }
	    break;

	  case ConfigureNotify:
	    if(w == ew)
	    {
		GWXDialogResize(display, md);
		events_handled++;
	    }
	    break;

	  case VisibilityNotify:
	    if(w == ew)
	    {


		events_handled++;
	    }
	    break;

	  case KeyPress:
	    if(GWX_WIDGET_MAPPED(md) && GWX_WIDGET_HAS_FOCUS(md))
	    {
		gwx_button_struct *btn, *btn_next;
		Boolean shift = display->shift_key_state;
		int modifier = GWGetCharacterFromKeyCode(
		    display, event->xkey.state
		);
		int k = GWGetCharacterFromKeyCode(
		    display, event->xkey.keycode
		);
#define DO_FOCUS_NEXT_BUTTON				\
{ if(GWX_WIDGET_CAN_DEFAULT(btn_next)) {		\
 GWXWidgetUngrabDefault(display, GWX_WIDGET(btn));	\
 GWXWidgetUnfocus(display, GWX_WIDGET(btn));		\
 GWXWidgetGrabDefault(display, GWX_WIDGET(btn_next));	\
 GWXWidgetFocus(display, GWX_WIDGET(btn_next));		\
 GWXButtonDraw(display, btn);				\
 GWXButtonDraw(display, btn_next);			\
} }
#define DO_BTN_HANDLE_ACCEL_KEY(b,k)			\
{ if( GWAcceleratorListCheck(				\
 (b)->accelerator, (b)->total_accelerators,		\
 (k), modifier)) {					\
  (b)->state = GWX_BUTTON_STATE_ARMED;			\
  GWXButtonDraw(display, (b));				\
} }
#define DO_BTN_SET_ARMED(b)				\
{ if((b) != NULL) {					\
  (b)->state = GWX_BUTTON_STATE_ARMED;			\
  GWXButtonDraw(display, (b));				\
} }
		switch(k)
		{
		  case '\t':
		    switch(md->type)
		    {
		      case GWX_DIALOG_TYPE_CONF_CANCEL:
			if(GWX_Widget_Has_Default(&md->yes_btn))
			{
			    btn = &md->yes_btn;
			    btn_next = shift ?
				&md->cancel_btn : &md->no_btn;
			    DO_FOCUS_NEXT_BUTTON
			}
			else if(GWX_Widget_Has_Default(&md->no_btn))
			{
			    btn = &md->no_btn;
			    btn_next = shift ?
				&md->yes_btn : &md->cancel_btn;
			    DO_FOCUS_NEXT_BUTTON
			}
			else if(GWX_Widget_Has_Default(&md->cancel_btn))
			{
			    btn = &md->cancel_btn;
			    btn_next = shift ?
				&md->no_btn : &md->yes_btn;
			    DO_FOCUS_NEXT_BUTTON
			}
			break;
		      case GWX_DIALOG_TYPE_CONF:
			if(GWX_Widget_Has_Default(&md->yes_btn))
			{
			    btn = &md->yes_btn;
			    btn_next = shift ?
				&md->no_btn : &md->no_btn;
			    DO_FOCUS_NEXT_BUTTON
			}
			else if(GWX_Widget_Has_Default(&md->no_btn))
			{
			    btn = &md->no_btn;
			    btn_next = shift ?
				&md->yes_btn : &md->yes_btn;
			    DO_FOCUS_NEXT_BUTTON
			}
			break;
		      case GWX_DIALOG_TYPE_MESG:
			break;
		    }
		    break;

		  case '\n': case ' ':
		    switch(md->type)
		    {
		      case GWX_DIALOG_TYPE_CONF_CANCEL:
			if(GWX_Widget_Has_Default(&md->yes_btn))
			{
			     DO_BTN_SET_ARMED(&md->yes_btn);
			}
			else if(GWX_Widget_Has_Default(&md->no_btn))
			{
			     DO_BTN_SET_ARMED(&md->no_btn);
			}
			else if(GWX_Widget_Has_Default(&md->cancel_btn))
			{
			     DO_BTN_SET_ARMED(&md->cancel_btn);
			}
			break;
		      case GWX_DIALOG_TYPE_CONF:
			if(GWX_Widget_Has_Default(&md->yes_btn))
			{
			    DO_BTN_SET_ARMED(&md->yes_btn);
			}
			else if(GWX_Widget_Has_Default(&md->no_btn))
			{
			    DO_BTN_SET_ARMED(&md->no_btn);
			}
			break;
		      case GWX_DIALOG_TYPE_MESG:
			if(GWX_Widget_Has_Default(&md->ok_btn))
			{
			    DO_BTN_SET_ARMED(&md->ok_btn);
			}
			break;
		    }
		    break;

		  default:
		    switch(md->type)
		    {
		      case GWX_DIALOG_TYPE_CONF_CANCEL:
			DO_BTN_HANDLE_ACCEL_KEY(
			    &md->cancel_btn, k
			);
		      case GWX_DIALOG_TYPE_CONF:
			DO_BTN_HANDLE_ACCEL_KEY(
			    &md->yes_btn, k
			);
			DO_BTN_HANDLE_ACCEL_KEY(
			    &md->no_btn, k
			);
			break;
		      case GWX_DIALOG_TYPE_MESG:
			DO_BTN_HANDLE_ACCEL_KEY(
			    &md->ok_btn, k
			);
			break;
		    }
		    break;
		}
#undef DO_BTN_SET_ARMED
#undef DO_BTN_HANDLE_ACCEL_KEY
#undef DO_FOCUS_NEXT_BUTTON
		events_handled++;
	    }
	    break;

	  case KeyRelease:
	    if(GWX_WIDGET_MAPPED(md) && GWX_WIDGET_HAS_FOCUS(md))
	    {
		int modifier = GWGetCharacterFromKeyCode(
		    display, event->xkey.state
		);
		int k = GWGetCharacterFromKeyCode(
		    display, event->xkey.keycode
		);
#define DO_BTN_HANDLE_ACCEL_KEY(b,k)                    \
{ if(GWAcceleratorListCheck(                            \
 (b)->accelerator, (b)->total_accelerators,             \
 (k), modifier)) {					\
  (b)->state = GWX_BUTTON_STATE_UNARMED;                \
  GWXButtonDraw(display, (b));				\
  if((b)->func_cb != NULL)				\
   (b)->func_cb((b), (b)->client_data);			\
} }
#define DO_BTN_SET_UNARMED_CALL(b)			\
{ if((b) != NULL) {                                     \
  (b)->state = GWX_BUTTON_STATE_UNARMED;		\
  GWXButtonDraw(display, (b));                          \
  if((b)->func_cb != NULL)                              \
   (b)->func_cb((b), (b)->client_data);                 \
} }
		switch(k)
		{
		  case '\n': case ' ':
		    switch(md->type)
		    {
		      case GWX_DIALOG_TYPE_CONF_CANCEL:
			if(GWX_Widget_Has_Default(&md->yes_btn))
			{
			     DO_BTN_SET_UNARMED_CALL(&md->yes_btn);
			}
			else if(GWX_Widget_Has_Default(&md->no_btn))
			{
			     DO_BTN_SET_UNARMED_CALL(&md->no_btn);
			}
			else if(GWX_Widget_Has_Default(&md->cancel_btn))
			{
			     DO_BTN_SET_UNARMED_CALL(&md->cancel_btn);
			}
			break;
		      case GWX_DIALOG_TYPE_CONF:
			if(GWX_Widget_Has_Default(&md->yes_btn))
			{
			    DO_BTN_SET_UNARMED_CALL(&md->yes_btn);
			}
			else if(GWX_Widget_Has_Default(&md->no_btn))
			{
			    DO_BTN_SET_UNARMED_CALL(&md->no_btn);
			}
			break;
		      case GWX_DIALOG_TYPE_MESG:
			if(GWX_Widget_Has_Default(&md->ok_btn))
			{
			    DO_BTN_SET_UNARMED_CALL(&md->ok_btn);
			}
			break;
		    }
		    break;

		  default:
		    switch(md->type)
		    {
		      case GWX_DIALOG_TYPE_CONF_CANCEL:
			DO_BTN_HANDLE_ACCEL_KEY(
			    &md->cancel_btn, k
			);
		      case GWX_DIALOG_TYPE_CONF:
			DO_BTN_HANDLE_ACCEL_KEY(
			    &md->yes_btn, k
			);
			DO_BTN_HANDLE_ACCEL_KEY(
			    &md->no_btn, k
			);
			break;
		      case GWX_DIALOG_TYPE_MESG:
			DO_BTN_HANDLE_ACCEL_KEY(
			    &md->ok_btn, k
			);
			break;
		    }
		    break;
		}
#undef DO_BTN_SET_UNARMED_CALL
#undef DO_BTN_HANDLE_ACCEL_KEY
		events_handled++;
	    }
	    break;

	  case FocusOut:
	    if(w == ew)
	    {
		GWXWidgetUnfocus(display, GWX_WIDGET(md));
		events_handled++;
	    }
	    break;

	  case FocusIn:
	    if(w == ew)
	    {
		GWXWidgetFocus(display, GWX_WIDGET(md));
		events_handled++;
	    }
	    break;

	  case ClientMessage:
	    if((event->xclient.format == 32) &&
	       (event->xclient.data.l[0] == display->atom_wm_delete_window) &&
	       (w == ew)
	    )
	    {
		switch(md->type)
		{
		  case GWX_DIALOG_TYPE_CONF_CANCEL:
		    GWXDialogCancelBtnCB(
			&md->cancel_btn,
			md->cancel_btn.client_data
		    );
		    break;

		  case GWX_DIALOG_TYPE_CONF:
		    GWXDialogNoBtnCB(
			&md->no_btn,
			md->no_btn.client_data
		    );
		    break;

		  default:
		    GWXDialogOKBtnCB(
		        &md->ok_btn,
		        md->ok_btn.client_data
		    );
		    break;
		}
		events_handled++;
	    }
	    break;
	}

	switch(md->type)
	{
	  case GWX_DIALOG_TYPE_CONF_CANCEL:
	    if(!events_handled)
		events_handled += GWXButtonManage(display, &md->cancel_btn, event);
	  case GWX_DIALOG_TYPE_CONF:
	    if(!events_handled)
		events_handled += GWXButtonManage(display, &md->no_btn, event);
	    if(!events_handled)
		events_handled += GWXButtonManage(display, &md->yes_btn, event);
	    break;

	  case GWX_DIALOG_TYPE_MESG:
	    if(!events_handled)
		events_handled += GWXButtonManage(display, &md->ok_btn, event);
	    break;
	}

	return(events_handled);
}

/*
 *	Destroys and deallocates all resources on the given dialog.
 */
void GWXDialogDestroy(
	gw_display_struct *display, gwx_dialog_struct *md
)
{
	int i;
	Display *dpy = (display != NULL) ? display->display : NULL;
	if((dpy == NULL) || (md == NULL))
	    return;

	switch(md->type)
	{
	  case GWX_DIALOG_TYPE_CONF_CANCEL:
	    GWXButtonDestroy(display, &md->cancel_btn);
	  case GWX_DIALOG_TYPE_CONF:
	    GWXButtonDestroy(display, &md->yes_btn);
	    GWXButtonDestroy(display, &md->no_btn);
	    break;
	  case GWX_DIALOG_TYPE_MESG:
	    GWXButtonDestroy(display, &md->ok_btn);
	    break;
	}

	DESTROY_WINDOW(&md->icon_w);
	FREE_PIXMAP(&md->icon);
	FREE_BITMAP(&md->icon_mask);

	DESTROY_TOPLEVEL_WINDOW(&md->toplevel);
	FREE_PIXMAP(&md->toplevel_buf);

	if(md->colors_initialized)
	{
	    GWXPixelDelete(display, md->color_fg);
	    GWXPixelDelete(display, md->color_bg);
	    GWXPixelDelete(display, md->color_highlight);
	    GWXPixelDelete(display, md->color_shade);
	    md->colors_initialized = False;
	}

	DELETE_FONT(&md->font);

	for(i = 0; i < md->strc; i++)
	    free(md->strv[i]);
	free(md->strv);
	md->strv = NULL;
	md->strc = 0;

	memset(md, 0x00, sizeof(gwx_dialog_struct));
}


/*
 *      Confirmation dialog No button callback.
 */
void GWXDialogNoBtnCB(void *object, void *client_data)
{
	gw_display_struct *display = GW_DISPLAY(client_data);
	gwx_button_struct *btn = GWX_BUTTON(object);
	gwx_dialog_struct *md;
	if((display == NULL) || (btn == NULL))
	    return;

	md = &display->conf_dialog;
	if(&md->no_btn == btn)
	    last_conf_dialog_code = GWConfirmationNo;
}

/*
 *	Confirmation dialog Yes button callback.
 */
void GWXDialogYesBtnCB(void *object, void *client_data)
{
	gw_display_struct *display = GW_DISPLAY(client_data);
	gwx_button_struct *btn = GWX_BUTTON(object);
	gwx_dialog_struct *md;
	if((display == NULL) || (btn == NULL))
	    return;

	md = &display->conf_dialog;
	if(&md->yes_btn == btn)
	    last_conf_dialog_code = GWConfirmationYes;
}

/*
 *      Confirmation dialog Cancel button callback.
 */
void GWXDialogCancelBtnCB(void *object, void *client_data)
{
	gw_display_struct *display = GW_DISPLAY(client_data);
	gwx_button_struct *btn = GWX_BUTTON(object);
	gwx_dialog_struct *md;
	if((display == NULL) || (btn == NULL))
	    return;

	md = &display->conf_dialog;
	if(&md->cancel_btn == btn)
	    last_conf_dialog_code = GWConfirmationCancel;
}

/*
 *	Dialog OK button callback.
 */
void GWXDialogOKBtnCB(void *object, void *client_data)
{
	gw_display_struct *display = GW_DISPLAY(client_data);
	gwx_button_struct *btn = GWX_BUTTON(object);
	gwx_dialog_struct *md;
	if((display == NULL) || (btn == NULL))
	    return;

	/* Is this our message dialog's ok button? */
	md = &display->mesg_dialog;
	if(&md->ok_btn == btn)
	    GWXWidgetUnmap(display, GWX_WIDGET(md));
}

/*
 *	Sets the dialog messages, parsinging it into seperate lines
 *	and updates the longest line value and resizes the dialog.
 */
void GWXDialogSetMesg(
	gw_display_struct *display, gwx_dialog_struct *md,
	const char *title,
	const char *mesg,
	const char *details
)
{
	int i;
	Display *dpy = (display != NULL) ? display->display : NULL;
	Window w;
	XFontStruct *font;
	if((dpy == NULL) || (md == NULL))
	    return;

	/* Delete old message and reset message values */
	for(i = 0; i < md->strc; i++)
	    free(md->strv[i]);
	free(md->strv);
	md->strv = NULL;
	md->strc = 0;
	md->longest_line = 0;
	md->longest_line_pixels = 0;
	md->line_height_pixels = 0;
	md->lines_height_pixels = 0;

	w = md->toplevel;
	if(w == None)
	    return;

	/* Set new title? */
	if(title != NULL)
	    XStoreName(dpy, w, title);

	/* Get pointer to message dialog's font structure */
	font = md->font;

	/* Parse message and store each line, also updating the longest
	 * line.
	 */
	if(mesg != NULL)
	{
	    int i, len;
	    int len_pixels;
	    const char *s = mesg, *sdelim;

	    while(s != NULL)
	    {
		/* Seek to next newlinedeliminator (if any) */
		sdelim = strchr(s, '\n');
		if(sdelim == NULL)
		    sdelim = strchr(s, '\r');

		/* Allocate a new line */
		i = MAX(md->strc, 0);
		md->strc = i + 1;
		md->strv = (char **)realloc(
		    md->strv, md->strc * sizeof(char *)
		);
		if(md->strv == NULL)
		{
		    md->strc = 0;
		    break;
		}

		/* No deliminter in this line? */
		if(sdelim == NULL)
		{
		    /* This is the last line */
		    len = STRLEN(s);
		    md->strv[i] = STRDUP(s);

		    len_pixels = GWXGetFontStringLength(font, s);
		}
		else
		{
		    /* Got new line deliminator, so copy this line segment */
		    char *s2;

		    len = sdelim - s;
		    md->strv[i] = s2 = (char *)malloc(
			(len + 1) * sizeof(char)
		    );
		    if(len > 0)
			strncpy(s2, s, len);
		    s2[len] = '\0';

		    len_pixels = GWXGetFontStringLength(font, s2);
		}
		/* Now len and len_pixels should be set properly */

		/* If this line is longer than the longest */
		if(len > md->longest_line)
		    md->longest_line = len;
		if(len_pixels > md->longest_line_pixels)
		    md->longest_line_pixels = len_pixels;

		/* Seek to next line if sdelim is not NULL */
		s = (sdelim != NULL) ? (sdelim + 1) : NULL;
	    }
	}

	/* Update message line heights */
	if(font != NULL)
	{
	    md->line_height_pixels = font->max_bounds.ascent +
		font->max_bounds.descent;
	    md->lines_height_pixels = md->line_height_pixels * md->strc;
	}

	/* Update dialog's size due to change in message */
	GWXDialogUpdateSize(display, md);
}

/*
 *	Maps the dialog, the dialog will be resized and drawn.
 */
void GWXDialogMap(gw_display_struct *display, gwx_dialog_struct *md)
{
	Display *dpy = (display != NULL) ? display->display : NULL;
	Window w;
	int x, y, width, height;
	if((dpy == NULL) || (md == NULL))
	    return;

	w = md->toplevel;

	GWContextGet(
	    display, GWContextCurrent(display),
	    NULL, NULL,
	    &x, &y,
	    &width, &height
	);

	/* Move dialog to center over toplevel window */
	if(w != None)
	{
	    int cx = (int)(
		x + (width / 2) - ((int)md->width / 2)
	    );
	    int cy = (int)(
		y + (height / 2) - ((int)md->height / 2)
	    );

	    if((cx + (int)md->width) > (int)display->root_width)
		cx = (int)display->root_width - (int)md->width;
	    if((cy + (int)md->height) > (int)display->root_height)
		cy = (int)display->root_height - (int)md->height;

	    if(cx < 0)
		cx = 0;
	    if(cy < 0)
		cy = 0;

	    XMoveWindow(dpy, w, cx, cy);
	}

	/* Map dialog by calling the drawing function, it will map
	 * it and redraw it.
	 */
	GWXDialogDraw(display, md);

	/* Map buttons and subwindows */
	switch(md->type)
	{
	  case GWX_DIALOG_TYPE_CONF_CANCEL:
	    GWXButtonDraw(display, &md->cancel_btn);
	  case GWX_DIALOG_TYPE_CONF:
	    GWXButtonDraw(display, &md->no_btn);
	    GWXButtonDraw(display, &md->yes_btn);
	    break;

	  case GWX_DIALOG_TYPE_MESG:
	    GWXButtonDraw(display, &md->ok_btn);
	    break;
	}
}

/*
 *	Blocks client program execution until confirmation is
 *	recieved.
 *
 *	Calling function is responsible for checking reentry to this
 *	function.
 *
 *	Confirmation dialog is already assumed to have been set
 *	up and mapped prior to this call.
 *
 *	The confermation dialog will not be unmapped by this call,
 *	it is up the calling function to do that.
 */
int GWXDoBlockUntilConf(gw_display_struct *display)
{
	if(display == NULL)
	    return(GWConfirmationNo);


	/* Reset local global confirmation code */
	last_conf_dialog_code = GWConfirmationNotAvailable;

	/* Keep managing here until confirmation dialog code
	 * has been set or display has been closed.
	 */
	while((last_conf_dialog_code == GWConfirmationNotAvailable) &&
	      (display->display != NULL)
	)
	{
	    GWManage(display);
	    usleep(1000);
	}

	return(last_conf_dialog_code);
}







#endif	/* Not __MSW__ */
