#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#if defined(__MSW__)
# include <windows.h>
#else
# include <X11/X.h>
# include <X11/Xlib.h>
# include <X11/Xutil.h>
# include <X11/keysym.h>
# include <X11/Xproto.h>
# include <X11/Xatom.h>
# include <X11/cursorfont.h>
# include <X11/extensions/shape.h>
# ifdef HAVE_XF86_VIDMODE
#  include <X11/extensions/xf86vmode.h>
#  ifndef XF86VIDMODE_H
#   define XF86VIDMODE_H
#  endif
#  ifndef XF86VIDMODE_EVENTS
#   define XF86VIDMODE_EVENTS
#  endif
# endif	/* HAVE_XF86_VIDMODE */
# ifdef HAVE_LIBXPM
#  include <X11/xpm.h>
#  ifndef XPM_H
#   define XPM_H
#  endif
#  ifndef XpmDefaultColorCloseness
#   define XpmDefaultColorCloseness      40000
#  endif
# endif	/* HAVE_LIBXPM */
#endif

#include <GL/gl.h>
#include <GL/glu.h>

#include "gw.h"
#include "stategl.h"

#include "gwdata/default.fnt"


#ifdef X_H
static int GWXErrorHandler(Display *xdpy, XErrorEvent *xerrorevent);
int GWGetCharacterFromKeyCode(gw_display_struct *dpy, KeyCode keycode);
Window GWGetParent(gw_display_struct *dpy, Window w);

/*
static void GWGrabPointerCursor(
	gw_display_struct *display, Window w, Boolean confine,
	Cursor cursor
);
*/

static void GWGrabPointer(
	gw_display_struct *display, Window w, Boolean confine
);
static void GWUngrabPointer(gw_display_struct *display);

void GWSetWindowIconFile(
	gw_display_struct *display, int ctx_num,
	const char *icon_path, const char *icon_name
);
Window GWCreateWindow(
	gw_display_struct *display,
	Window parent,
	int x, int y,
	int width, int height,
	const char *title
);
#endif	/* X_H */

#ifdef X_H
/* GW init/management/shutdown */
gw_display_struct *GWInit(int argc, char **argv);
void GWManage(gw_display_struct *display);
void GWShutdown(gw_display_struct *display);
#endif	/* X_H */

#ifdef X_H
/* GW sync */
void GWFlush(gw_display_struct *display);
void GWSync(gw_display_struct *display);
int GWEventsPending(gw_display_struct *display);
#endif

/* GW dialog message and response */
void GWOutputMessage(
	gw_display_struct *display,
	int type,
	const char *subject,
	const char *message,
	const char *help_details
);
int GWConfirmation(
	gw_display_struct *display,
	int type,
	const char *subject,
	const char *message,
	const char *help_details,
	int default_response
);
int GWConfirmationSimple(
	gw_display_struct *display, const char *message
);

/* GW callback setting */
void GWSetDrawCB(
	gw_display_struct *display,
	void (*func)(int, void *),
	void *data
);
void GWSetResizeCB(
	gw_display_struct *display,
	void (*func)(int, void *, int, int, int, int),
	void *data
);
void GWSetKeyboardCB(
	gw_display_struct *display,
	void (*func)(void *, int, Boolean, unsigned long),
	void *data
);
void GWSetPointerCB(
	gw_display_struct *display,
	void (*func)(int, void *, int, int, gw_event_type, int, unsigned long),
	void *data
);
void GWSetVisibilityCB(
	gw_display_struct *display,
	void (*func)(int, void *, gw_visibility),
	void *data
);
void GWSetSaveYourselfCB(
	gw_display_struct *display,
	void (*func)(int, void *),
	void *data
);
void GWSetCloseCB(
	gw_display_struct *display,
	void (*func)(int, void *, void *),
	void *data
);
void GWSetTimeoutCB(
	gw_display_struct *display,
	void (*func)(void *),
	void *data
);

/* GW context */
#ifdef X_H
int GWContextNew(
	gw_display_struct *display,
	int x, int y, int width, int height,
	const char *title,
	const char *icon_path, const char *icon_name,
	Bool no_windows
);
void GWContextDelete(
	gw_display_struct *display, int ctx_num
);
int GWContextCurrent(gw_display_struct *display);
int GWContextGet(
	gw_display_struct *display, int ctx_num,
	void **window_id_rtn, void **gl_context_rtn,
	int *x_rtn, int *y_rtn, int *width_rtn, int *height_rtn
);
int GWContextSet(gw_display_struct *display, int ctx_num);
void GWContextPosition(
	gw_display_struct *display, int x, int y
);
void GWContextSize(
	gw_display_struct *display, int width, int height
);
Boolean GWContextIsFullScreen(gw_display_struct *display);
int GWContextFullScreen(gw_display_struct *display, Boolean state);
#endif	/* X_H */

/* Frame buffer */
#ifdef X_H
void GWPostRedraw(gw_display_struct *display);
void GWSwapBuffer(gw_display_struct *display);
#endif  /* X_H */

/* Set up gl projection matrix for 2d drawing */
void GWOrtho2D(gw_display_struct *display);
void GWOrtho2DCoord(
	gw_display_struct *display,
	float left, float right, float top, float bottom
);

/* Keyboard */
#ifdef X_H
void GWKeyboardAutoRepeat(gw_display_struct *display, Boolean b);
#endif	/* X_H */

/* Video modes */
#ifdef X_H
gw_vidmode_struct *GWVidModesGet(
	gw_display_struct *display, int *n
);
#endif	/* X_H */
void GWVidModesFree(gw_vidmode_struct *vidmode, int n);

/* Accelerator keys */
gw_accelerator_struct *GWAcceleratorNew(
	int key, int modifier
);
void GWAcceleratorDelete(gw_accelerator_struct *a);
int GWAcceleratorListAdd(
	gw_accelerator_struct ***a, int *total,
	int key, int modifier
);
void GWAcceleratorListDelete(
	gw_accelerator_struct ***a, int *total
);
Boolean GWAcceleratorListCheck(
	gw_accelerator_struct **a, int total,
	int key, int modifier
);

/* GW pointer cursor */
#ifdef X_H
void *GWCursorNew(
	gw_display_struct *display,
	int width, int height,	/* In pixels */
	int hot_x, int hot_y,	/* In pixels */
	int bpl,		/* Bytes per line, 0 for autocalc */
	const void *data	/* RGBA data (4 bytes per pixel) */
);
void GWCursorSet(gw_display_struct *display, void *cursor_ptr);
void GWCursorDelete(gw_display_struct *display, void *cursor_ptr);
void GWSetPointerCursor(
	gw_display_struct *display, gw_pointer_cursor cursor
);
void GWShowCursor(gw_display_struct *display);
void GWHideCursor(gw_display_struct *display);
Boolean GWIsCursorShown(gw_display_struct *display);
void GWSetInputBusy(gw_display_struct *display);
void GWSetInputReady(gw_display_struct *display);
#endif  /* X_H */

/* GW font IO */
void GWSetFont(gw_display_struct *display, GWFont *font);
int GWGetFontSize(
	GWFont *font,
	int *width, int *height,
	int *character_spacing, int *line_spacing
);

/* GW string drawing */
void GWDrawString(
	gw_display_struct *display,
	int x, int y,
	const char *string
);
void GWDrawCharacter(
	gw_display_struct *display,
	int x, int y,
	char c
);

/* GW image IO */
int GWImageLoadHeaderFromData(
	gw_display_struct *display,
	int *width, int *height, int *bpl, int *bpp,
	const void *data
);
u_int8_t *GWImageLoadFromDataRGB(
	gw_display_struct *display,
	int *width, int *height, int *bpl,
	const void *data
);
const u_int8_t *GWImageLoadFromDataSharedRGB(
	gw_display_struct *display,
	int *width, int *height, int *bpl,
	const void *data
);
u_int8_t *GWImageLoadFromDataRGBA(
	gw_display_struct *display,
	int *width, int *height, int *bpl,
	const void *data
);
const u_int8_t *GWImageLoadFromDataSharedRGBA(
	gw_display_struct *display,
	int *width, int *height, int *bpl,
	const void *data
);

static void GWImageDrawFromDataNexus(
	gw_display_struct *display, const u_int8_t *img_data,
	int x, int y, int width, int height,
	int img_width, int img_height,
	GLenum img_format
);
void GWImageDrawFromDataRGB(
	gw_display_struct *display, const void *data,
	int x, int y, int width, int height
);
void GWImageDrawFromDataRGBA(
	gw_display_struct *display, const void *data,
	int x, int y, int width, int height
);




#define ATOI(s)         (((s) != NULL) ? atoi(s) : 0)
#define ATOL(s)         (((s) != NULL) ? atol(s) : 0)
#define ATOF(s)         (((s) != NULL) ? (float)atof(s) : 0.0f)
#define STRDUP(s)       (((s) != NULL) ? strdup(s) : NULL)

#define MAX(a,b)        (((a) > (b)) ? (a) : (b))
#define MIN(a,b)        (((a) < (b)) ? (a) : (b))
#define CLIP(a,l,h)     (MIN(MAX((a),(l)),(h)))
#define STRLEN(s)	(((s) != NULL) ? ((int)strlen(s)) : 0)
#define STRISEMPTY(s)	(((s) != NULL) ? ((s) == '\0') : True)

#define RADTODEG(r)     ((r) * 180 / PI)
#define DEGTORAD(d)     ((d) * PI / 180)

#define FREE_PIXMAP(p)          \
{ if(*(p) != None) { XFreePixmap(dpy, *(p)); *(p) = None; } }
#define FREE_BITMAP(p)          FREE_PIXMAP(p)
#define DESTROY_WINDOW(p)       \
{ if(*(p) != None) { XDestroyWindow(dpy, *(p)); *(p) = None; } }
#define DELETE_FONT(p)          \
{ GWXFontDelete(display, *(p)); *(p) = NULL; }

#define DESTROY_TOPLEVEL_WINDOW(p)                      \
{ if(*(p) != None) {                                    \
 /* Get WM hints and destroy icon Window, Pixmap, and   \
  * mask.                                               \
  */                                                    \
 XWMHints *wm_hints = XGetWMHints(dpy, *(p));           \
 if(wm_hints != NULL) {                                 \
  if(wm_hints->flags & IconWindowHint)                  \
   DESTROY_WINDOW(&wm_hints->icon_window);              \
  if(wm_hints->flags & IconPixmapHint)                  \
   FREE_PIXMAP(&wm_hints->icon_pixmap);                 \
  if(wm_hints->flags & IconMaskHint)                    \
   FREE_BITMAP(&wm_hints->icon_mask);                   \
  XFree(wm_hints);                                      \
 }                                                      \
 /* Destroy toplevel Window */                          \
 DESTROY_WINDOW(p);                                     \
} }

#define XCOLOR_SET_RGB_COEFF(p,r,g,b)			\
{ if((p) != NULL) {					\
 (p)->flags	= DoRed | DoGreen | DoBlue;		\
 (p)->red	= (float)(r) * (unsigned short)-1;	\
 (p)->green	= (float)(g) * (unsigned short)-1;	\
 (p)->blue	= (float)(b) * (unsigned short)-1;	\
 (p)->pad	= 0;					\
} }
#define XCOLOR_SET_GREY_COEFF(p,g)			\
{ if((p) != NULL) {					\
 (p)->flags	= DoRed | DoGreen | DoBlue;		\
 (p)->red	= (float)(g) * (unsigned short)-1;	\
 (p)->green	= (p)->red;				\
 (p)->blue	= (p)->red;				\
 (p)->pad	= 0;					\
} }



/* Graphics wrapped debugging flag */
static Boolean gw_debug = False;
/* String prefixed to all GW debugging messages */
#define GW_DEBUG_PREFIX	"GW Debug: "


#ifdef X_H
/*
 *	Modified by GWXErrorHandler() to indicate error or none.
 *
 *	This will be set to a non-zero value when GWXErrorHandler()
 *	is called, GWXErrorHandler() is called whenever the X server
 *	generates an error.
 *
 *	If xerrval is not 0 then xerrmsg will contain a string
 *	describing the X error.
 */
static int xerrval;
static char xerrmsg[256];
#endif	/* X_H */


#ifdef X_H
/*
 *	Visual attributes list (for picking Visuals), last member must
 *	be None to mark end of array.
 */
static int attributeListSglRGB[] = {
	GLX_RGBA,		/* Use only TrueColor or DirectColor */
	GLX_RED_SIZE,	1,	/* Get deepest buffer with >= 1 red bit */
	GLX_GREEN_SIZE,	1,
	GLX_BLUE_SIZE,	1,
	GLX_DEPTH_SIZE,	1,	/* Z buffer depth, not bit depth */
	GLX_STENCIL_SIZE, 1,
	None
};

static int attributeListSglRGBA[] = {
	GLX_RGBA,		/* Use only TrueColor or DirectColor */
	GLX_RED_SIZE,	1,	/* Get deepest buffer with >= 1 red bit */
	GLX_GREEN_SIZE,	1,
	GLX_BLUE_SIZE,	1,
	GLX_ALPHA_SIZE,	1,
	GLX_DEPTH_SIZE,	1,
	GLX_STENCIL_SIZE, 1,
	None
};

static int attributeListDblRGB[] = {
	GLX_RGBA,		/* Use only TrueColor or DirectColor */
	GLX_DOUBLEBUFFER,	/* Double buffer */
	GLX_RED_SIZE,	1,
	GLX_GREEN_SIZE,	1,
	GLX_BLUE_SIZE,	1,
	GLX_DEPTH_SIZE,	1,
	GLX_STENCIL_SIZE, 1,
	None
};

static int attributeListDblRGBA[] = {
	GLX_RGBA,		/* Use only TrueColor or DirectColor */
	GLX_DOUBLEBUFFER,	/* Double buffer */
	GLX_RED_SIZE,	1,
	GLX_GREEN_SIZE,	1,
	GLX_BLUE_SIZE,	1,
	GLX_ALPHA_SIZE,	1,
	GLX_DEPTH_SIZE,	1,
	GLX_STENCIL_SIZE, 1,
	None
};
#endif	/* X_H */


/*
 *	Keysym name to character value table, even numbers are pointers
 *	to strings while odd numbers are pointers who's value is a
 *	character value.
 */
static char *keytable[] = {
	"Alt_L",	(char *)GWKeyAlt,
	"Alt_R",        (char *)GWKeyAlt,
	"Control_L",    (char *)GWKeyCtrl,
	"Control_R",    (char *)GWKeyCtrl,
	"Shift_L",      (char *)GWKeyShift,
	"Shift_R",      (char *)GWKeyShift,

	"BackSpace",    (char *)GWKeyBackSpace,
	"Tab",          (char *)'\t',
	"ISO_Left_Tab",	(char *)'\t',
	"Linefeed",     (char *)'\f',
	"Clear",        (char *)'\0',
	"Return",       (char *)'\n',
	"Pause",        (char *)GWKeyPause,
	"Scroll_Lock",  (char *)GWKeyScrollLock,
	"Sys_Req",      (char *)GWKeySysReq,
	"Escape",       (char *)0x1b,
	"Delete",       (char *)GWKeyDelete,

	"Home",		(char *)GWKeyHome,
	"Left",         (char *)GWKeyLeft,
	"Up",           (char *)GWKeyUp,
	"Right",        (char *)GWKeyRight,
	"Down",         (char *)GWKeyDown,
	"Prior",        (char *)GWKeyPageUp,	/* ??? */
	"Page_Up",      (char *)GWKeyPageUp,
	"Next",         (char *)GWKeyPageDown,	/* ??? */
	"Page_Down",    (char *)GWKeyPageDown,
	"End",          (char *)GWKeyEnd,
	"Begin",        (char *)GWKeyHome,	/* ??? */

	"KP_0",         (char *)'0',
	"KP_1",         (char *)'1',
	"KP_2",         (char *)'2',
	"KP_3",         (char *)'3',
	"KP_4",         (char *)'4',
	"KP_5",         (char *)'5',
	"KP_6",         (char *)'6',
	"KP_7",         (char *)'7',
	"KP_8",         (char *)'8',
	"KP_9",         (char *)'9',

	"KP_Space",     (char *)' ',
	"KP_Tab",       (char *)'\t',
	"KP_Enter",     (char *)'\n',
	"KP_F1",        (char *)GWKeyF1,
	"KP_F2",        (char *)GWKeyF2,
	"KP_F3",        (char *)GWKeyF3,
	"KP_F4",        (char *)GWKeyF4,
	"KP_Home",      (char *)GWKeyHome,
	"KP_Left",      (char *)GWKeyLeft,
	"KP_Up",        (char *)GWKeyUp,
	"KP_Right",     (char *)GWKeyRight,
	"KP_Down",      (char *)GWKeyDown,
	"KP_Prior",     (char *)GWKeyPageUp,
	"KP_Page_Up",   (char *)GWKeyPageUp,
	"KP_Next",      (char *)GWKeyPageDown,
	"KP_Page_Down", (char *)GWKeyPageDown,
	"KP_End",       (char *)GWKeyEnd,
	"KP_Begin",     (char *)GWKeyHome,
	"KP_Insert",    (char *)GWKeyInsert,
	"KP_Delete",    (char *)GWKeyDelete,
	"KP_Equal",     (char *)'=',
	"KP_Multiply",  (char *)'*',
	"KP_Add",       (char *)'+',
	"KP_Separator", (char *)'\0',
	"KP_Subtract",  (char *)'-',
	"KP_Decimal",   (char *)'.',
	"KP_Divide",    (char *)'/',

	"F1",		(char *)GWKeyF1,
	"F2",           (char *)GWKeyF2,
	"F3",           (char *)GWKeyF3,
	"F4",           (char *)GWKeyF4,
	"F5",           (char *)GWKeyF5,
	"F6",           (char *)GWKeyF6,
	"F7",           (char *)GWKeyF7,
	"F8",           (char *)GWKeyF8,
	"F9",           (char *)GWKeyF9,
	"F10",          (char *)GWKeyF10,
	"F11",          (char *)GWKeyF11,
	"F12",          (char *)GWKeyF12,
	"F13",          (char *)GWKeyF13,
	"F14",          (char *)GWKeyF14,
	"F15",          (char *)GWKeyF15,
	"F16",          (char *)GWKeyF16,
	"F17",          (char *)GWKeyF17,
	"F18",          (char *)GWKeyF18,
	"F19",          (char *)GWKeyF19,
	"F20",          (char *)GWKeyF20,

	"space",        (char *)' ',
	"exclam",       (char *)'!',
	"quotedbl",     (char *)'"',
	"numbersign",   (char *)'#',
	"dollar",       (char *)'$',
	"percent",      (char *)'%',
	"ampersand",    (char *)'&',
	"apostrophe",   (char *)'\'',
	"quoteright",   (char *)'"',
	"parenleft",    (char *)'(',
	"parenright",   (char *)')',
	"asterisk",     (char *)'*',
	"plus",         (char *)'+',
	"comma",        (char *)',',
	"minus",        (char *)'-',
	"period",       (char *)'.',
	"slash",        (char *)'/',

	"0",		(char *)'0',
	"1",            (char *)'1',
	"2",            (char *)'2',
	"3",            (char *)'3',
	"4",            (char *)'4',
	"5",            (char *)'5',
	"6",            (char *)'6',
	"7",            (char *)'7',
	"8",            (char *)'8',
	"9",            (char *)'9',

	"colon",        (char *)':',
	"semicolon",    (char *)';',
	"less",         (char *)'\0',
	"equal",        (char *)'=',
	"greater",      (char *)'>',
	"question",     (char *)'?',
	"at",           (char *)'@',

	"A",            (char *)'A',
	"B",            (char *)'B',
	"C",            (char *)'C',
	"D",            (char *)'D',
	"E",            (char *)'E',
	"F",            (char *)'F',
	"G",            (char *)'G',
	"H",            (char *)'H',
	"I",            (char *)'I',
	"J",            (char *)'J',
	"K",            (char *)'K',
	"L",            (char *)'L',
	"M",            (char *)'M',
	"N",            (char *)'N',
	"O",            (char *)'O',
	"P",            (char *)'P',
	"Q",            (char *)'Q',
	"R",            (char *)'R',
	"S",            (char *)'S',
	"T",            (char *)'T',
	"U",            (char *)'U',
	"V",            (char *)'V',
	"W",            (char *)'W',
	"X",            (char *)'X',
	"Y",            (char *)'Y',
	"Z",            (char *)'Z',

	"bracketleft",  (char *)'[',
	"backslash",    (char *)'\\',
	"bracketright", (char *)']',
	"asciicircum",  (char *)'^',
	"underscore",   (char *)'_',
	"grave",        (char *)'\0',
	"quoteleft",    (char *)'"',

	"a",            (char *)'a',
	"b",            (char *)'b',
	"c",            (char *)'c',
	"d",            (char *)'d',
	"e",            (char *)'e',
	"f",            (char *)'f',
	"g",            (char *)'g',
	"h",            (char *)'h',
	"i",            (char *)'i',
	"j",            (char *)'j',
	"k",            (char *)'k',
	"l",            (char *)'l',
	"m",            (char *)'m',
	"n",            (char *)'n',
	"o",            (char *)'o',
	"p",            (char *)'p',
	"q",            (char *)'q',
	"r",            (char *)'r',
	"s",            (char *)'s',
	"t",            (char *)'t',
	"u",            (char *)'u',
	"v",            (char *)'v',
	"w",            (char *)'w',
	"x",            (char *)'x',
	"y",            (char *)'y',
	"z",            (char *)'z',

	"braceleft",    (char *)'{',
	"bar",          (char *)'|',
	"braceright",   (char *)'}',
	"asciitilde",   (char *)'~'

};


#ifdef X_H
/*
 *	X error handler:
 */
static int GWXErrorHandler(Display *dpy, XErrorEvent *ev)
{
	const char *error_code_name;


	if((dpy == NULL) || (ev == NULL))
	    return(0);

	/* Reset global xerror value */
	xerrval = 0;
	*xerrmsg = '\0';

	/* ******************************************************* */
	/* Ignore these error codes */

	/* Success */
	if(ev->error_code == Success)
	    return(0);

	/* Calls to XGetImage() often return error when the error is
	 * non-critical, we need to ignore that here.
	 */
	if(ev->request_code == X_GetImage)
	    return(0);

	/* ******************************************************** */
	/* If this point is reached, it means we got a definate
	 * (serious) error from the X server.
	 */

	/* Set global xerrval to -1, indicating a serious error */
	xerrval = -1;

	switch(ev->error_code)
	{
	  case Success:
	    error_code_name = "Success";
	    break;
	  case BadRequest:
	    error_code_name = "BadRequest";
	    break;
	  case BadValue:
	    error_code_name = "BadValue";
	    break;
	  case BadWindow:
	    error_code_name = "BadWindow";
	    break;
	  case BadPixmap:
	    error_code_name = "BadPixmap";
	    break;
	  case BadAtom:
	    error_code_name = "BadAtom";
	    break;
	  case BadCursor:
	    error_code_name = "BadCursor";
	    break;
	  case BadFont:
	    error_code_name = "BadFont";
	    break;
	  case BadMatch:
	    error_code_name = "BadMatch";
	    break;
	  case BadDrawable:
	    error_code_name = "BadDrawable";
	    break;
	  case BadAccess:
	    error_code_name = "BadAccess";
	    break;
	  case BadAlloc:
	    error_code_name = "BadAlloc";
	    break;
	  case BadColor:
	    error_code_name = "BadColor";
	    break;
	  case BadGC:
	    error_code_name = "BadGC";
	    break;
	  case BadIDChoice:
	    error_code_name = "BadIDChoice";
	    break;
	  case BadName:
	    error_code_name = "BadName";
	    break;
	  case BadLength:
	    error_code_name = "BadLength"; 
	    break;
	  case BadImplementation:
	    error_code_name = "BadImplementation";
	    break;
	  default:
	    error_code_name = "Unknown";
	    break;
	}

	/* Set X error message */
	sprintf(
	    xerrmsg,
"X Server error #%ld:  type: %s  major_op_code: %i  minor_op_code: %i",
	    ev->serial,
	    error_code_name,
	    ev->request_code,
	    ev->minor_code
	);

	/* Print this error to stderr */
	fprintf(stderr, "%s\n", xerrmsg);

	return(0);
}
#endif	/* X_H */

#ifdef X_H
/*
 *	Returns the character or key value for the given keycode
 *	looked up on the keytable.
 */
int GWGetCharacterFromKeyCode(gw_display_struct *dpy, KeyCode keycode)
{
	int i;
	const int total = sizeof(keytable) / sizeof(char *);
	KeySym keysym;
	const char *cstrptr;

	if(dpy == NULL)
	    return((int)'\0');

	if(dpy->display == NULL)
	    return((int)'\0');

	xerrval = 0;
	keysym = XKeycodeToKeysym(
	    dpy->display,
	    keycode,
/*	    (int)((dpy->shift_key_state) ? 1 : 0) */
	    0
	);
	if(xerrval)
	    return((int)'\0');

	xerrval = 0;
	if((keysym != XK_Shift_R) && (keysym != XK_Shift_L) &&
	   dpy->shift_key_state
	)
	{
	    keysym = XKeycodeToKeysym(
		dpy->display,
		keycode,
		1
	    );
	    if(xerrval)
		return((int)'\0');
	}

	xerrval = 0;
	cstrptr = XKeysymToString(keysym);
	if(xerrval || (cstrptr == NULL))
	    return((int)'\0');

	for(i = 0; i < total; i += 2)
	{
	    if(!strcmp(keytable[i], cstrptr))
		return((int)(keytable[i + 1]));
	}

	return('\0');
}
#endif	/* X_H */

#ifdef X_H
/*
 *	Returns the Window's parent;
 *
 *	Can return None if w is the root window or an error occured.
 */
Window GWGetParent(gw_display_struct *dpy, Window w)
{
	Window root, parent, *children = NULL;
	unsigned int total_children = 0;

	if((w == None) || (w == dpy->root))
	    return(None);

	xerrval = 0;
	if(XQueryTree(
	    dpy->display, w, &root, &parent, &children, &total_children
	))
	    XFree(children);
	else
	    parent = None;
	if(xerrval && gw_debug)
	    printf(GW_DEBUG_PREFIX
"XQueryTree(): Unable to get parent or children for Window 0x%.8x.\n",
		(u_int32_t)w
	    );

	return(parent);
}
#endif  /* X_H */

#ifdef X_H
/*
 *      Grabs pointer.
 *      This function is not used. Commenting it out.
 *      - Jesse
static void GWGrabPointerCursor(
	gw_display_struct *display, Window w, Boolean confine,
	Cursor cursor
)
{
	Display *dpy;

	if(w == None)
	    GWUngrabPointer(display);

	if(display == NULL)
	    return;

	dpy = display->display;
	if(dpy == NULL)
	    return;

	GWUngrabPointer(display);

	xerrval = 0;
	if(XGrabPointer(
	    dpy, w, True,
	    ButtonPressMask | ButtonReleaseMask |
	    PointerMotionMask,
	    GrabModeAsync, GrabModeAsync,
	    confine ? w : None, cursor, CurrentTime
	) == GrabSuccess)
	    display->grab_window = w;
	else
	    display->grab_window = None;
	if(xerrval && gw_debug)
	    printf(GW_DEBUG_PREFIX
"XGrabPointer(): Error grabbing pointer for Window 0x%.8x.\n",
		(u_int32_t)w
	    );
}
*/

#endif	/* X_H */

#ifdef X_H
/*
 *	Grabs pointer.
 */
static void GWGrabPointer(
	gw_display_struct *display, Window w, Boolean confine
)
{
	Display *dpy;

	if(w == None)
	    GWUngrabPointer(display);

	if(display == NULL)
	    return;

	dpy = display->display;
	if(dpy == NULL)
	    return;

	// Ungrab pointer first as needed
	GWUngrabPointer(display);

	// Grab pointer
	xerrval = 0;
	if(XGrabPointer(
	    dpy, w, True,
	    ButtonPressMask | ButtonReleaseMask |
	    PointerMotionMask,
	    GrabModeAsync, GrabModeAsync,
	    confine ? w : None, None, CurrentTime
	) == GrabSuccess)
	    display->grab_window = w;
	else
	    display->grab_window = None;
	if(xerrval && gw_debug)
	    printf(GW_DEBUG_PREFIX
"XGrabPointer(): Error grabbing pointer for Window 0x%.8x.\n",
		(u_int32_t)w
	    );
}
#endif	/* X_H */

#ifdef X_H
/*
 *	Ungrabs pointer.
 */
static void GWUngrabPointer(gw_display_struct *display)
{
	if(display == NULL)
	    return;

	if(display->grab_window != None)
	{
	    xerrval = 0;
	    XUngrabPointer(display->display, CurrentTime);
	    if(xerrval && gw_debug)
		printf(GW_DEBUG_PREFIX
"XUngrabPointer(): Error ungrabbing pointer for Window 0x%.8x.\n",
		    (u_int32_t)display->grab_window
		);
	    display->grab_window = None;
	}
}
#endif  /* X_H */


#ifdef X_H
/*
 *	Sets the WM icon for the given toplevel Window in display.
 *
 *	The path to the icon data icon_path needs to be an xpm file.
 *
 *	The icon_name will be set as the icon's icon name which appears
 *	next to the icon (or however the WM has it displayed).
 */
void GWSetWindowIconFile(
	gw_display_struct *display, int ctx_num,
	const char *icon_path, const char *icon_name
)
{
#ifdef XPM_H
	int depth;
	Display *dpy = (display != NULL) ? display->display : NULL;
	char *tmp_filename;
	Window w, root, icon_w = None;
	Pixmap icon_pm, icon_mask;
	int xpm_status;
	XpmAttributes xpm_attr;
	XWMHints *wm_hints;
	if((dpy == NULL) || STRISEMPTY(icon_path))
	    return;

	if((ctx_num < 0) || (ctx_num >= display->total_gl_contexts))
	    return;

	w = display->toplevel[ctx_num];
	root = display->root;
	depth = display->depth;
	if((w == None) || (root == None) || (depth < 1))
	    return;

	/* Set up XPM attributes for XPM loading */
	xpm_attr.valuemask = XpmSize | XpmCloseness | XpmDepth;
	xpm_attr.closeness = XpmDefaultColorCloseness;
	xpm_attr.depth = depth;
	xpm_attr.width = 0;             /* Reset size returns */
	xpm_attr.height = 0;
	tmp_filename = STRDUP(icon_path);
	xpm_status = XpmReadFileToPixmap(
	    dpy, root, tmp_filename,
	    &icon_pm, &icon_mask,
	    &xpm_attr
	);
	free(tmp_filename);
	if(xpm_status == XpmSuccess)
	{
	    int count;
	    int width = xpm_attr.width;
	    int height = xpm_attr.height;
	    Pixel black_pix = display->black_pix;
	    Pixel white_pix = display->white_pix;
	    XIconSize *icon_sizes;

	    /* Check if the loaded icon has is a valid size by
	     * confirming the size with the X server
	     */
	    xerrval = 0;
	    if(XGetIconSizes(dpy, w, &icon_sizes, &count))
	    {
		int i;

		/* Iterate through X server's prefered icon sizees */
		for(i = 0; i < count; i++)
		{
		    /* Icon size falls in bounds with this X server
		     * prefered size?
		     */
		    if((width >= icon_sizes[i].min_width) &&
		       (width <= icon_sizes[i].max_width) &&
		       (height >= icon_sizes[i].min_height) &&
		       (height <= icon_sizes[i].max_height)
		    )
			break; 
		}
		XFree(icon_sizes);
	
		/* Did not get valid icon size bounds? */
		if(i >= count)
		{
		    fprintf(
			stderr,
			"GWSetWindowIconFile(): Invalid icon size %ix%i.\n",
			width, height
		    );
		    /* Delete icon pixmap and mask before returning */
 		    FREE_PIXMAP(&icon_pm);
		    FREE_BITMAP(&icon_mask);
		    return;
		}
	    }
	    if(xerrval && gw_debug)
		printf(GW_DEBUG_PREFIX
"XGetIconSizes(): Error obtaining preffered icon sizes for Window 0x%.8x.\n",
		    (u_int32_t)w
		);


	    /* Create icon window */
	    xerrval = 0;
	    icon_w = XCreateSimpleWindow(
		dpy, root,
		0, 0,           /* Coordinates */
		width, height,  /* Size */
		0,              /* Border width */
		white_pix,      /* Border color */
		black_pix       /* Background color */
	    );
	    if(xerrval || (icon_w == None))
	    {
		/* Error creating icon window */

		if(gw_debug)
		    printf(GW_DEBUG_PREFIX
"XCreateSimpleWindow(): Error creating a Window for the icon.\n"
		    );

		/* Delete icon pixmap and mask before returning */
		FREE_PIXMAP(&icon_pm);
		FREE_BITMAP(&icon_mask);
		return;
	    }
	    /* If the icon mask is available then mask the newly
	     * created Window to the shape of the icon mask
	     */
	    if(icon_mask != None)
	    {
		xerrval = 0;
		XShapeCombineMask(
		    dpy, icon_w,
		    ShapeBounding,
		    0, 0,
		    icon_mask,
		    ShapeSet
		);
		if(xerrval && gw_debug)
		    printf(GW_DEBUG_PREFIX
"XShapeCombineMask(): Error setting the shape of the icon window 0x%.8x.\n",
			(u_int32_t)icon_w
		    );
	    }
	}
	else
	{
	    /* Unable to load the XPM file */
	    fprintf(stderr, "%s: Unable to load.\n", icon_path);
	    return;
	}

	/* Set the Window Manager Hints */
	wm_hints = XAllocWMHints();
	if(wm_hints != NULL)
	{
	    wm_hints->flags =	InputHint | StateHint |
				IconPixmapHint | IconWindowHint |
				IconMaskHint;
	    wm_hints->input = True;
	    wm_hints->initial_state = NormalState;
	    wm_hints->icon_pixmap = icon_pm;
	    wm_hints->icon_mask = icon_mask;
	    wm_hints->icon_window = icon_w;

	    xerrval = 0;
	    XSetWMHints(dpy, w, wm_hints);
	    if(xerrval && gw_debug)
		printf(GW_DEBUG_PREFIX
"XSetWMHints(): Error setting icon w=0x%.8x p=0x%.8x m=0x%.8x for window 0x%.8x.\n",
		    (u_int32_t)icon_w, (u_int32_t)icon_pm,
		    (u_int32_t)icon_mask, (u_int32_t)w
		);

	    XFree(wm_hints);
	}

	/* Set the Window Manager Icon Name */
	if(icon_name != NULL)
	{
	    XTextProperty *t = (XTextProperty *)malloc(
		sizeof(XTextProperty)
	    );
	    t->value = (unsigned char *)icon_name;
	    t->encoding = XA_STRING;
	    t->format = 8;
	    t->nitems = STRLEN(icon_name);

	    xerrval = 0;
	    XSetWMIconName(dpy, w, t);
	    if(xerrval && gw_debug)
		printf(GW_DEBUG_PREFIX
"XSetWMIconName(): Error setting name of icon window 0x%.8x for window 0x%.8x.\n",
		    (u_int32_t)icon_w, (u_int32_t)w
		);

	    free(t);
	}

#endif  /* XPM_H */
}
#endif	/* X_H */


#ifdef X_H
/*
 *	Creates a new Window.
 */
Window GWCreateWindow(
	gw_display_struct *display,
	Window parent,
	int x, int y,
	int width, int height,
	const char *title
)
{
	Bool is_toplevel;
	unsigned long wa_flags;
	XVisualInfo *vi;
	Window w = None;
	XSetWindowAttributes wa;
	Display *dpy;

	if((display == NULL) || (width <= 0) || (height <= 0))
	    return(w);

	dpy = display->display;
	vi = display->visual_info;

	if((dpy == NULL) || (vi == NULL))
	    return(w);

	/* If the specified parent is None, then use the use root
	 * Window (the desktop) as the parent
	 */
	if(parent == None)
	{
	    parent = display->root;

	    /* Root window not available? */
	    if(parent == None)
	    {
		if(gw_debug)
		    printf(GW_DEBUG_PREFIX
 "GWCreateWindow(): Root window not available as parent to create the window.\n"
		    );
		return(w);
	    }
	}

	/* Are we going to create a toplevel Window? */
	is_toplevel = (parent == display->root) ? True : False;

	/* Set values for the new Window */
	wa_flags =	CWBackPixmap | CWBorderPixel |
			CWBitGravity | CWWinGravity |
			CWBackingStore | CWOverrideRedirect |
			CWSaveUnder | CWEventMask |
			CWColormap | CWCursor;
	wa.background_pixmap = None;
	wa.border_pixel = display->black_pix;
	wa.bit_gravity = ForgetGravity;
	wa.win_gravity = CenterGravity;
	wa.backing_store = NotUseful;
	wa.save_under = False;
	wa.event_mask = StructureNotifyMask | ExposureMask |
			 KeyPressMask | KeyReleaseMask |
			 ButtonPressMask | ButtonReleaseMask |
			 PointerMotionMask | VisibilityChangeMask;
	wa.override_redirect = False;
	wa.colormap = display->colormap;
	wa.cursor = None;

	/* Create the new Window */
	if(gw_debug)
	    printf(GW_DEBUG_PREFIX
 "Creating X window parented to 0x%.8x of geometry %ix%i%s%i%s%i at Depth %i bits\n",
		(unsigned int)parent,
		width, height,
		(x < 0) ? "" : "+", x,
		(y < 0) ? "" : "+", y,
		display->depth
	    );
	xerrval = 0;
	w = XCreateWindow(
	    dpy,
	    parent,
	    x, y,
	    width, height,
	    0,
	    display->depth,
	    InputOutput,
	    vi->visual,
	    wa_flags,
	    &wa
	);
	if(xerrval || (w == None))
	{
	    if(gw_debug)
		printf(GW_DEBUG_PREFIX
 "XCreateWindow(): Error creating Window.\n"
		);
	    return(w);
	}
	else
	{
	    if(gw_debug)
		printf(GW_DEBUG_PREFIX
		    "Created Window 0x%.8x.\n",
		    (unsigned int)w
		);
	}

	/* Set additional values if the Window is a toplevel Window */
	if(is_toplevel)
	{
	    const int natoms = 6;
	    Atom *a = (Atom *)malloc(natoms * sizeof(Atom));
	    XSizeHints *sz_hints = XAllocSizeHints();;

	    /* Set Size Hints */
	    if(sz_hints != NULL)
	    {
		sz_hints->flags =	USPosition | PSize |
					PMinSize | PBaseSize;
		sz_hints->x = x;
		sz_hints->y = y;
		sz_hints->width = width;
		sz_hints->height = height;
		sz_hints->min_width = GW_MIN_TOPLEVEL_WIDTH;
		sz_hints->min_width = GW_MIN_TOPLEVEL_HEIGHT;
		sz_hints->base_width = 0; 
		sz_hints->base_height = 0;

		xerrval = 0;
		XSetWMNormalHints(dpy, w, sz_hints);
		if(xerrval && gw_debug)
		    printf(GW_DEBUG_PREFIX
"XSetWMNormalHints(): Error setting WM size hints for window 0x%.8x.\n",
			(u_int32_t)w
		    );
	    }

	    /* Set Title */
	    if(!STRISEMPTY(title))
	    {
		xerrval = 0;
		XStoreName(dpy, w, title);
		if(xerrval && gw_debug)
		    printf(GW_DEBUG_PREFIX
"XStoreName(): Error setting the title for window 0x%.8x.\n",
			(u_int32_t)w
		    );
	    }

	    /* Set Atoms */
	    a[0] = display->atom_wm_close_window;
	    a[1] = display->atom_wm_delete_window;
	    a[2] = display->atom_wm_ping;
	    a[3] = display->atom_wm_save_yourself;
	    a[4] = display->atom_wm_take_focus;
	    a[5] = display->atom_wm_workarea;
	    xerrval = 0;
	    XSetWMProtocols(
		dpy,		/* Display */
		w,		/* Window */
		a,		/* Atoms */
		natoms		/* Number of Atoms */
	    );
	    if(xerrval && gw_debug)
		printf(GW_DEBUG_PREFIX
"XSetWMProtocols(): Cannot set atoms for window 0x%.8x.\n",
		    (u_int32_t)w
		);


	    XFree(sz_hints);
	    free(a);
	}

	return(w);
}
#endif	/* X_H */


#ifdef X_H
/*
 *	Initializes the graphics wrapper.
 */
gw_display_struct *GWInit(int argc, char **argv)
{
	Boolean direct_rendering = True;
	Boolean fullscreen = False;
	float aspect_offset = 0.0f;
	Boolean no_windows = False;
	int i, status;
	int major_version, minor_version;
#ifdef XF86VIDMODE_H
	int vm_event_base, vm_error_base;
#endif	/* XF86VIDMODE_H */
	const char *arg;
	XVisualInfo *vi;
	int x = 0, y = 0, off_screen_margin = 10;
	int width = 320, height = 240;
	gw_display_struct *display = NULL;
	const char *display_address = NULL;
	const char *background_color = NULL;
	const char *foreground_color = NULL;
	const char *font_name =
	    "-adobe-helvetica-medium-r-normal-*-12-*-*-*-p-*-iso8859-1";
	const char *title = "Untitled";
	const char *icon_path = NULL;
	const char *icon_name = NULL;
	const char *pointer_color = NULL;
        Bool allow_key_repeat = True;

	/* Parse arguments */
	for(i = 0; i < argc; i++)
	{
	    arg = argv[i];
	    if(arg == NULL)
		continue;

	    /* Print GW debug messages to stdout? */
	    if(!strcasecmp(arg, "--gw_debug") ||
	       !strcasecmp(arg, "-gw_debug")
	    )
	    {
		gw_debug = True;
	    }
	    /* Software rendering? */
	    else if(!strcasecmp(arg, "--software_rendering") ||
		    !strcasecmp(arg, "-software_rendering") ||
		    !strcasecmp(arg, "--software-rendering") ||
		    !strcasecmp(arg, "-software-rendering") ||
		    !strcasecmp(arg, "--software") ||
		    !strcasecmp(arg, "-software")
	    )
	    {
		direct_rendering = False;
	    }
	    /* Hardware rendering? */ 
	    else if(!strcasecmp(arg, "--hardware_rendering") ||
		    !strcasecmp(arg, "-hardware_rendering") ||
		    !strcasecmp(arg, "--hardware-rendering") ||
		    !strcasecmp(arg, "-hardware-rendering") ||
		    !strcasecmp(arg, "--hardware") ||
		    !strcasecmp(arg, "-hardware") ||
		    !strcasecmp(arg, "--direct_rendering") ||
		    !strcasecmp(arg, "-direct_rendering") ||
		    !strcasecmp(arg, "--direct-rendering") ||
		    !strcasecmp(arg, "-direct-rendering")
	    )
	    {
		direct_rendering = True;
	    }
	    /* Display address (address to X server)? */
	    else if(!strcasecmp(arg, "--display") ||
		    !strcasecmp(arg, "-display") ||
		    !strcasecmp(arg, "--dpy") ||
		    !strcasecmp(arg, "-dpy")
	    )
	    {
		i++;
		arg = (i < argc) ? argv[i] : NULL;
		if(arg != NULL)
		{
		    display_address = arg;
		}
		else
		{
		    fprintf(
			stderr,
			"%s: Requires argument.\n",
			argv[i - 1]
		    );
		}
	    }
	    /* Background color? */
	    else if(!strcasecmp(arg, "--background") ||
		    !strcasecmp(arg, "-background") ||
		    !strcasecmp(arg, "--bg") ||
		    !strcasecmp(arg, "-bg")
	    )
	    {
		i++;
		arg = (i < argc) ? argv[i] : NULL;
		if(arg != NULL)
		{
		    background_color = arg;
		}
		else
		{
		    fprintf(
			stderr,
			"%s: Requires argument.\n",
			argv[i - 1]
		    );
		}
	    }
	    /* Foreground color? */
	    else if(!strcasecmp(arg, "--foreground") ||
		    !strcasecmp(arg, "-foreground") ||
		    !strcasecmp(arg, "--fg") ||
		    !strcasecmp(arg, "-fg")
	    )
	    {
		i++;
		arg = (i < argc) ? argv[i] : NULL;
		if(arg != NULL)
		{
		    foreground_color = arg;
		}
		else
		{
		    fprintf(
			stderr,
			"%s: Requires argument.\n",
			argv[i - 1]
		    );
		}
	    }
	    /* Font name? */
	    else if(!strcasecmp(arg, "--font") ||
		    !strcasecmp(arg, "-font") ||
		    !strcasecmp(arg, "--fn") ||
		    !strcasecmp(arg, "-fn")
	    )
	    {
		i++;
		arg = (i < argc) ? argv[i] : NULL;
		if(arg != NULL)
		{
		    font_name = arg;
		}
		else
		{
		    fprintf(
			stderr,
			"%s: Requires argument.\n",
			argv[i - 1]
		    );
		}
	    }
	    /* Title of toplevel window? */
	    else if(!strcasecmp(arg, "--title") ||
		    !strcasecmp(arg, "-title")
	    )
	    {
		i++;
		arg = (i < argc) ? argv[i] : NULL;
		if(arg != NULL)
		{   
		    title = arg;
		}
		else
		{
		    fprintf(
			stderr,
			"%s: Requires argument.\n",
			argv[i - 1]
		    );
		}
	    }
	    /* Toplevel window's WM icon? */
	    else if(!strcasecmp(arg, "--icon_path") ||
		    !strcasecmp(arg, "-icon_path") ||
		    !strcasecmp(arg, "--icon_file") ||
		    !strcasecmp(arg, "-icon_file") ||
		    !strcasecmp(arg, "--icon") ||
		    !strcasecmp(arg, "-icon")
	    )
	    {
		i++;
		arg = (i < argc) ? argv[i] : NULL;
		if(arg != NULL)
		{
		    icon_path = arg;
		}
		else
		{
		    fprintf(
			stderr,
			"%s: Requires argument.\n",
			argv[i - 1]
		    );
		}
	    }
	    /* Toplevel window's WM icon name? */
	    else if(!strcasecmp(arg, "--icon_name") ||
		    !strcasecmp(arg, "-icon_name")
	    )
	    {
		i++;
		arg = (i < argc) ? argv[i] : NULL;
		if(arg != NULL)
		{
		    icon_name = arg;
		}
		else
		{
		    fprintf(
			stderr,
			"%s: Requires argument.\n",
			argv[i - 1]
		    );
		}
	    }
	    /* Full screen mode? */
	    else if(!strcasecmp(arg, "--fullscreen") ||
		    !strcasecmp(arg, "-fullscreen") ||
		    !strcasecmp(arg, "--full_screen") ||
		    !strcasecmp(arg, "-full_screen")
	    )
	    {
		fullscreen = True;
	    }
            else if ( (!strcasecmp(arg, "--window")) ||
                      (!strcasecmp(arg, "-window") )
                    )
            {
                fullscreen = False;
            }
            else if ( (!strcasecmp(arg, "--no-autorepeat")) ||
                      (!strcasecmp(arg, "--no-keyrepeat") )
                    )
            {
                allow_key_repeat = False;
            }
	    /* Position and size of toplevel window? */
	    else if(!strcasecmp(arg, "--geometry") ||
		    !strcasecmp(arg, "-geometry")
	    )
	    {
		i++;
		arg = (i < argc) ? argv[i] : NULL;
		if(arg != NULL)
		{
		    int start_x = 0, start_y = 0;
		    unsigned int start_width = 320, start_height = 240;

		    XParseGeometry(
			arg,
			&start_x, &start_y,
			&start_width, &start_height
		    );

		    x = start_x;
		    y = start_y;
		    width = MAX((int)start_width, off_screen_margin);
		    height = MAX((int)start_height, off_screen_margin);
		}
		else
		{
		    fprintf(
			stderr,
			"%s: Requires argument.\n",
			argv[i - 1]
		    );
		}
	    }
	    /* Pointer color? */
	    else if(!strcasecmp(arg, "--pointer_color") ||
		    !strcasecmp(arg, "-pointer_color") ||
		    !strcasecmp(arg, "--pointer-color") ||
		    !strcasecmp(arg, "-pointer-color") ||
		    !strcasecmp(arg, "--ms") ||
		    !strcasecmp(arg, "-ms")
	    )
	    {
		i++;
		arg = (i < argc) ? argv[i] : NULL;
		if(arg != NULL)
		{
		    pointer_color = arg;
		}
		else
		{
		    fprintf(
			stderr,
			"%s: Requires argument.\n",
			argv[i - 1]
		    );
		}
	    }
	    /* Aspect offset? */
	    else if(!strcasecmp(arg, "--aspect_offset") ||
		    !strcasecmp(arg, "-aspect_offset") ||
		    !strcasecmp(arg, "--aspect-offset") ||
		    !strcasecmp(arg, "-aspect-offset") ||
		    !strcasecmp(arg, "--aspectoffset") ||
		    !strcasecmp(arg, "-aspectoffset")
	    )
	    {
		i++;
		arg = (i < argc) ? argv[i] : NULL;
		if(arg != NULL)
		{
		    aspect_offset = ATOF(arg);
		}
		else
		{
		    fprintf(
			stderr,
			"%s: Requires argument.\n",
			argv[i - 1]
		    );
		}
	    }
	    /* No window, this is a special argument to specify that
	     * no windows (or dialogs) are to be created.
	     */
	    else if(!strcasecmp(arg, "--no_windows") ||
		    !strcasecmp(arg, "-no_windows") ||
		    !strcasecmp(arg, "--nowindows") ||
		    !strcasecmp(arg, "-nowindows")
	    )
	    {
		no_windows = True;
	    }
	}

	/* Is GW debugging enabled? */
	if(gw_debug)
	    printf(GW_DEBUG_PREFIX
"Debugging enabled, messages will be sent to stdout.\n"
	    );


	/* Allocate a new graphics wrapper display structure */
	display = (gw_display_struct *)calloc(
	    1, sizeof(gw_display_struct)
	);
	if(display == NULL)
	    return(display);

	/* Reset values */
	display->grab_window = None;
	display->cursor_shown = True;
	display->last_button_press = 0;
	display->button_press_count = 0;
	display->fullscreen = False;
	display->fullscreen_context_num = -1;
        display->allow_autorepeat = allow_key_repeat;
	memset(&display->vidmode_last_gui_geometry, 0x00, sizeof(XRectangle));


	/* Open connection to display */
	status = -1;
	while(True)
	{
	    /* First try connecting using the given display address
	     * (which may be NULL).
	     */
	    display->display = XOpenDisplay(display_address);
	    if(display->display != NULL)
	    {
		status = 0;     /* Success */
		if(gw_debug)
		    printf(GW_DEBUG_PREFIX
 "Connected to X server display: %s\n",
			display_address
		    );
		break;
	    }

	    /* Check if host address was unspecified */
	    if(display_address == NULL)
	    {
		/* Try using the value from the DISPLAY environment
		 * variable (if any).
		 */
		display_address = getenv("DISPLAY");
		if(display_address != NULL)
		    display->display = XOpenDisplay(display_address);
		if(display->display != NULL)
		{
		    status = 0; /* Success */
		    if(gw_debug)
			printf(GW_DEBUG_PREFIX
 "Connected to X server display (environment variable \"DISPLAY\"): %s\n",
			    display_address
			);
		    break;
		}

#if 0
		/* All else fails try explicitly using localhost */
		display_address = "127.0.0.1:0.0";
		if(display_address != NULL)
		    display->display = XOpenDisplay(display_address);
		if(display->display != NULL)
		{
		    status = 0; /* Success */
		    if(gw_debug)
			printf(GW_DEBUG_PREFIX
 "Connected to X server display (default local): %s\n",
			    display_address
			);
		    break;
		}
#endif
	    }

	    /* Failed */
	    break;
	}
	/* Connect failed? */
	if(status)
	{
	    if(gw_debug)
		printf(GW_DEBUG_PREFIX
 "Connection to X server failed, see stderr output for details.\n"
		);

	    /* Print reason for failed connect */
	    fprintf(stderr, "Cannot connect to X server: ");
	    if(getenv("DISPLAY") == NULL)
		fprintf(stderr, "`DISPLAY' environment variable not set.");
	    else
		fprintf(stderr, "%s", XDisplayName(NULL));
	    fprintf(stderr, "\n");

	    /* Print verbose help */
	    fprintf(stderr,
"\nCheck to make sure that your X server is running and that your\n\
enviroment variable `DISPLAY' is set properly.\n"
	    );

	    free(display);
	    return(NULL);
	}

	/* Set X server error handler */
	XSetErrorHandler(GWXErrorHandler);


	/* Check version numbers of the glX extension */
	if(glXQueryVersion(display->display, &major_version, &minor_version))
	{
	    if(gw_debug)
		printf(GW_DEBUG_PREFIX
 "glX version: major=%i minor=%i\n",
		    major_version, minor_version
		);
	    /* Store glX version */
	    display->glx_version_major = major_version;
	    display->glx_version_minor = minor_version;
	}
	else
	{
	    /* Could not get glX version, implying glX not available */

	    if(gw_debug)
		printf(GW_DEBUG_PREFIX
 "glXQueryVersion(): Failed, implying glX is not available.\n"
		);

	    fprintf(stderr,
 "Warning: Unable to find glX, this is used for X and OpenGL communications.\n"
	    );
	}

	/* Get Visual */
	vi = NULL;
	display->alpha_channel_bits = 0;
	display->has_double_buffer = False;
	for(i = 0; i < 4; i++)
	{
	    switch(i)
	    {
	      case 0:	/* First try double buffer RGBA */
		vi = glXChooseVisual(
		    display->display,
		    DefaultScreen(display->display),
		    attributeListDblRGBA
		);
		if(vi == NULL)
		{
		    if(gw_debug)
			printf(GW_DEBUG_PREFIX
 "glXChooseVisual(): RGBA double buffer X Visual not available.\n"
			);
		}
		break;

	      case 1:	/* Second try single buffer RGBA */
		vi = glXChooseVisual(
		    display->display,
		    DefaultScreen(display->display),
		    attributeListSglRGBA
		);
		if(vi == NULL)
		{ 
		    if(gw_debug)
			printf(GW_DEBUG_PREFIX
 "glXChooseVisual(): RGBA single buffer X Visual not available.\n"
			);
		}
		break;

	      case 2:	/* Third try double buffer RGB */
		vi = glXChooseVisual(
		    display->display,
		    DefaultScreen(display->display),
		    attributeListDblRGB
		);
		if(vi == NULL)
		{ 
		    if(gw_debug)
			printf(GW_DEBUG_PREFIX
 "glXChooseVisual(): RGB double buffer X Visual not available.\n"
			);
		}
		break;

	      case 3:	/* Fourth try single buffer RGB */
		vi = glXChooseVisual(
		    display->display,
		    DefaultScreen(display->display),
		    attributeListSglRGB
		);
		if(vi == NULL)
		{
		    if(gw_debug)
			printf(GW_DEBUG_PREFIX
 "glXChooseVisual(): RGB single buffer X Visual not available.\n"
			);
		}
		break;
	    }
	    if(vi != NULL)
		break;
	}
	/* Record the matched XVisualInfo from above (even if NULL) */
	display->visual_info = vi;

	/* Get values from matched Visual */
	if(vi != NULL)
	{
	    int vi_val;

	    if(!glXGetConfig(
		display->display, vi,
		GLX_DOUBLEBUFFER, &vi_val
	    ))
		display->has_double_buffer = (vi_val ? True : False);

	    if(!glXGetConfig(
		display->display, vi,
		GLX_ALPHA_SIZE, &vi_val
	    ))
		display->alpha_channel_bits = vi_val;
	}

	/* Failed to find a useable Visual? */
	if(vi == NULL)
	{
	    fprintf(
		stderr,
"Unable to find a suitable X Visual.\n\
\n\
Please verify that you have an X Visual suitable for OpenGL rendering\n\
by running \"xdpyinfo\". Also note that defining multiple Visuals\n\
may confuse the selection of a proper Visual (in which case try defining\n\
only one Visual).\n\
\n\
Your hardware may have a Visual that is not compatible with this program.\n"
	    );
	    GWShutdown(display);
	    return(NULL);
	}


	/* Begin query for X extensions */

	/* XShape */
	if(XShapeQueryVersion(
	    display->display, &major_version, &minor_version
	))
	{
	   if(gw_debug)
		printf(GW_DEBUG_PREFIX
 "Shape extension version: major=%i minor=%i\n",
		    major_version, minor_version
		);
	}
	else
	{
	    if(gw_debug)
		printf(GW_DEBUG_PREFIX
 "XShapeQueryVersion(): Shape extension not available.\n"
		);
	}

	/* Screen Saver */
/* TODO
	Status XScreenSaverQueryVersion(
    Display *dpy,
    int *major,       
    int *minor
	);
 */

#ifdef XF86VIDMODE_H
	/* XF86 VidMode */
	if(XF86VidModeQueryExtension(
	    display->display, &vm_event_base, &vm_error_base
	))
	{
	    if(XF86VidModeQueryVersion(
		display->display, &major_version, &minor_version
	    ))
	    {
		if(gw_debug)
		    printf(GW_DEBUG_PREFIX
 "XF86VidMode extension version: major=%i minor=%i\n",
			major_version, minor_version
		    );
	    }
	    if(gw_debug)
	    {
		Display *dpy = display->display;
		int scr = DefaultScreen(dpy);
		XF86VidModeModeInfo **mode_line;
		int total_mode_lines;
		int vp_x = 0, vp_y = 0;

		/* Get current viewport position */
		XF86VidModeGetViewPort(dpy, scr, &vp_x, &vp_y);

		if(XF86VidModeGetAllModeLines(
		    dpy, scr,
		    &total_mode_lines, &mode_line
		))
		{
		    /* First mode is the current mode */
		    XF86VidModeModeInfo *m = (total_mode_lines > 0) ?
			mode_line[0] : NULL;
		    if(m != NULL)
			printf(
			    "\tCurrent Video Mode: %ix%i  Viewport: %i %i\n",
			    m->hdisplay, m->vdisplay,
			    vp_x, vp_y
			);

		    /* Print info for subsequent modes */
		    for(i = 1; i < total_mode_lines; i++)
		    {
			m = mode_line[i];
			if(m == NULL)
			    continue;

			printf(
			    "\tVideo Mode #%i: %ix%i\n",
			    i, m->hdisplay, m->vdisplay
			);
		    }

		    /* Deallocate memory on each mode info structure */
		    for(i = 0; i < total_mode_lines; i++)
		    {
			m = mode_line[i];
			if(m == NULL)
			    continue;

			/* If private is not NULL then it needs to be
			 * deallocated.  Currently this only occures for
			 * the S3 servers.
			 */
			if((m->privsize > 0) && (m->private != NULL))
			{
			    XFree(m->private);
			    m->private = NULL;
			    m->privsize = 0;
			}

			/* Do not free each structure */
/*			XFree(m); */
		    }
		    XFree(mode_line);
		}
	    }
	    display->has_vidmode_ext = True;
	    display->vidmode_ext_event_offset = vm_event_base;
	}
	else
	{
	    /* XF86 VidMode not available */
	    if(gw_debug)
		printf(GW_DEBUG_PREFIX
 "XF86VidModeQueryExtension(): XF86 VidMode extension not available.\n"
		);
	    display->has_vidmode_ext = False;
	    display->vidmode_ext_event_offset = 0;
	}
#else
	display->has_vidmode_ext = False;
	display->vidmode_ext_event_offset = 0;
#endif	/* XF86VIDMODE_H */


	/* Get other default values */
	display->aspect_offset = aspect_offset;
	display->direct_rendering = direct_rendering;
	display->visual = DefaultVisual(display->display, vi->screen);
	display->white_pix = WhitePixel(display->display, vi->screen);
	display->black_pix = BlackPixel(display->display, vi->screen);
	display->depth = vi->depth;
	if(gw_debug)
	    printf(GW_DEBUG_PREFIX
 "Using X Depth %i bits.\n", display->depth
		);                  

	display->gc = DefaultGC(display->display, vi->screen);
	display->current_font = NULL;
#if 0
/* We do not use X fonts to draw strings, so do not set any of this
 *
 * The default xfont name will be coppied to the display structure
 * further below
 */
	 = XLoadFont(display->display, font_name);
	XSetFont(display->display, display->gc, font);
#endif

	XSetForeground(display->display, display->gc, display->black_pix);
	display->current_font = default_fnt;
	display->atom_wm_motif_all_clients = XInternAtom(
	    display->display,
	    "_MOTIF_WM_ALL_CLIENTS", 
	    False
	);
	display->atom_wm_motif_hints = XInternAtom(
	    display->display,
	    "_MOTIF_WM_HINTS",
	    False
	);
	display->atom_wm_motif_info = XInternAtom(
	    display->display,
	    "_MOTIF_WM_INFO",
	    False
	);
	display->atom_wm_motif_menu = XInternAtom(
	    display->display,
	    "_MOTIF_WM_MENU",
	    False
	);
	display->atom_wm_motif_messages = XInternAtom(
	    display->display,
	    "_MOTIF_WM_MESSAGES",
	    False
	);
	display->atom_wm_motif_offset = XInternAtom(
	    display->display,
	    "_MOTIF_WM_OFFSET",
	    False
	);
	display->atom_wm_motif_query = XInternAtom(
	    display->display,
	    "_MOTIF_WM_QUERY",
	    False
	);
	display->atom_wm_close_window = XInternAtom(
	    display->display,
	    "_NET_CLOSE_WINDOW",
	    False
	);
	display->atom_wm_delete_window = XInternAtom(
	    display->display,
	    "WM_DELETE_WINDOW", 
	    False
	);
	display->atom_wm_ping = XInternAtom(
	    display->display,
	    "_NET_WM_PING",
	    False
	);
	display->atom_wm_save_yourself = XInternAtom(
	    display->display,
	    "WM_SAVE_YOURSELF", 
	    False
	);
	display->atom_wm_state = XInternAtom(
	    display->display,
	    "WM_STATE",
	    False
	);
	display->atom_wm_take_focus = XInternAtom(
	    display->display,
	    "WM_TAKE_FOCUS",
	    False
	);
	display->atom_wm_workarea = XInternAtom(
	    display->display,
	    "_NET_WORKAREA",
	    False
	);


	/* Get root window and its geometry */
	display->root = RootWindow(display->display, vi->screen);
	display->root_width = DisplayWidth(display->display, vi->screen);
	display->root_height = DisplayHeight(display->display, vi->screen);
	if(display->root == None)
	{
	    if(gw_debug)
		printf(GW_DEBUG_PREFIX
 "Cannot find root window (the desktop).\n"
		);
	}


	/* Create X Colormap */
	xerrval = 0;
	display->colormap = XCreateColormap(
	    display->display,
	    display->root,
	    vi->visual, AllocNone
	);
	if(xerrval || (display->colormap == None))
	{
	    if(gw_debug)
		printf(GW_DEBUG_PREFIX
 "XCreateColormap(): Cannot create X colormap.\n"
		);
	}


	/* Sanitize the geometry for creation of the first toplevel
	 * window to fall within the root window's size
	 */
	if((x + width) < off_screen_margin)
	    x = off_screen_margin - width;
	if((y + height) < off_screen_margin)
	    y = off_screen_margin - height;

	if(x > (display->root_width - off_screen_margin))
	    x = display->root_width - off_screen_margin;
	if(y > (display->root_height - off_screen_margin))
	    y = display->root_height - off_screen_margin;


	/* Create first GL context and toplevel Window */
	display->gl_context_num = -1;
	display->total_gl_contexts = 0;
	display->glx_context = NULL;
	display->toplevel = NULL;
	display->toplevel_geometry = NULL;
	display->draw_count = NULL;
	i = GWContextNew(
	    display, x, y, width, height,
	    title, icon_path, icon_name, no_windows
	);
	GWContextSet(display, i);


	/* At this point we have the glX context created and set to our
	 * main toplevel window, now we can get the GL version
	 */
	arg = (const char *)glGetString(GL_VERSION);
	if(arg != NULL)
	{
	    char *gl_vs_ptr;
	    char *gl_vs = STRDUP(arg);
	    if(gl_vs != NULL)
	    {
		/* Deliminate at space which separates version and vendor
		 * name.
		 */
		gl_vs_ptr = strchr(gl_vs, ' ');
		if(gl_vs_ptr != NULL)
		    *gl_vs_ptr = '\0';

		gl_vs_ptr = gl_vs;
		if(gl_vs_ptr != NULL)
		{
		    display->gl_version_major = ATOI(gl_vs_ptr);
		    gl_vs_ptr = strchr(gl_vs_ptr, '.');
		    if(gl_vs_ptr != NULL)
			gl_vs_ptr++;
		}
		if(gl_vs_ptr != NULL)
		{
		    display->gl_version_minor = ATOI(gl_vs_ptr);
		    gl_vs_ptr = strchr(gl_vs_ptr, '.');
		    if(gl_vs_ptr != NULL)
			gl_vs_ptr++;
		}

		free(gl_vs);
	    }

	    if(gw_debug)
		printf(GW_DEBUG_PREFIX
 "OpenGL implementation version: major=%i minor=%i\n",
		    display->gl_version_major, display->gl_version_minor
		);
	}
	else
	{
	    if(gw_debug)
		printf(GW_DEBUG_PREFIX
"glGetString(GL_VERSION): Returned NULL, cannot get OpenGL implementation version.\n"
		);
	    fprintf(stderr, "Cannot obtain OpenGL implementation version.\n");
	}

	/* Font name specified? */
	if(font_name != NULL)
	{
	    /* Load font to see if it exists */
	    XFontStruct *xfont = XLoadQueryFont(
		display->display, font_name
	    );
	    if(xfont == NULL)
	    {
		/* Fall back to something safe */
		font_name =
		    "-*-clean-*-*-*-*-*-*-*-*-*-*-*-*";
	    }
	    else
	    {
		/* Do not keep the loaded X font */
		XFreeFont(display->display, xfont);
	    }

	    /* Record font name */
	    display->def_xfont_name = STRDUP(font_name);
	}

	/* Get maximum texture lengths, in pixels */
	if(True)
	{
	    GLint v[1];
	    glGetIntegerv(GL_MAX_TEXTURE_SIZE, v);
	    display->texture_1d_max = display->texture_2d_max =
		display->texture_3d_max = v[0];
	}

	/* Turn on keyboard autorepeat */
	GWKeyboardAutoRepeat(display, True);

	/* Reset GL states */
	StateGLResetAll(&display->state_gl);

	/* Set pixel storage format (for rastering bitmaps) */
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glPixelStorei(GL_PACK_ALIGNMENT, 1);

	/* Set other GL defaults that we want. Some of these might
	 * default to on but we want them off to improve performance
	 */
	StateGLDisable(&display->state_gl, GL_DITHER);


	/* Load cursors */
	if(True)
	{
	    Display *dpy = display->display;
	    Cursor cur;
/*	    XColor c_fg, c_bg; */
	    int width = 8, height = 8;
	    u_int8_t *data = (u_int8_t *)calloc(
		width * height, 4 * sizeof(u_int8_t)
	    );

#define CREATE_FONT_CURSOR(i)			\
{						\
 xerrval = 0;					\
 cur = XCreateFontCursor(dpy, i);		\
/* if((cur != None) && !xerrval) {		\
  xerrval = 0;					\
  XRecolorCursor(dpy, cur, &c_fg, &c_bg);	\
 } */						\
}
/*
	    XCOLOR_SET_RGB_COEFF(&c_fg, 1.0, 1.0, 1.0);
	    XCOLOR_SET_RGB_COEFF(&c_bg, 0.0, 0.0, 0.0);
 */
	    cur = None;		/* Standard cursor is None */
	    display->cursor_standard = cur;

/*
	    XCOLOR_SET_RGB_COEFF(&c_fg, 0.0, 0.0, 0.0);
	    XCOLOR_SET_RGB_COEFF(&c_bg, 1.0, 1.0, 1.0);
 */
	    CREATE_FONT_CURSOR(XC_watch);
	    display->cursor_busy = cur;

	    CREATE_FONT_CURSOR(XC_xterm);
	    display->cursor_text = cur;

	    CREATE_FONT_CURSOR(XC_fleur);
	    display->cursor_translate = cur;

	    CREATE_FONT_CURSOR(XC_sizing);
	    display->cursor_zoom = cur;

	    display->cursor_invisible = (Cursor)GWCursorNew(
		display, width, height, width / 2, height / 2,
		0, data
	    );

	    free(data);
#undef CREATE_FONT_CURSOR
	}

	/* Initialize dialogs */
	if(!no_windows)
	{
	    GWXDialogCreate(
	        display, &display->mesg_dialog,
		GWX_DIALOG_TYPE_MESG
	    );
	    GWXDialogCreate(
		display, &display->conf_dialog,
		GWX_DIALOG_TYPE_CONF_CANCEL
	    );
	}

	/* Go to full screen mode on start up? */
	if(fullscreen)
	{
	    GWContextFullScreen(display, True);
	}

	if(gw_debug)
	    printf(GW_DEBUG_PREFIX
"Initialization done.\n"
	    );

	return(display);
}
#endif  /* X_H */

#ifdef X_H
/*
 *	Graphics wrapper management function, called once per loop.
 */
void GWManage(gw_display_struct *display)
{
	XEvent event;
	XClientMessageEvent *cm;
	Window w;
	int i, key, events_handled;

	if(display == NULL)
	    return;

	if(display->display != NULL)
	{
	    Display *dpy = display->display;

	    /* Fetch events while there are events pending */
	    while(
		XPending(dpy)
/*		XEventsQueued(dpy, QueuedAlready) > 0 */
/*		XEventsQueued(dpy, QueuedAfterFlush) > 0 */
	    )
	    {
		/* Get next event */
		xerrval = 0;
		XNextEvent(dpy, &event);
		if(xerrval)
		    break;

		/* Get window for this event */
		w = event.xany.window;

		events_handled = 0;

		/* Handle event by type */
		switch(event.type)
		{
		  case Expose:
		    for(i = 0; i < display->total_gl_contexts; i++)
		    {
			if((display->toplevel[i] == w) &&
			   (display->func_draw != NULL)
			)
			{
			    display->func_draw(
				i, display->func_draw_data
			    );
			    events_handled++;
			    break;
			}
		    }
		    break;

		  case ConfigureNotify:
		    if(gw_debug)
			printf(GW_DEBUG_PREFIX
"Got XEvent ConfigureNotify: window=0x%.8x x=%i y=%i width=%i height=%i\n",
			    (u_int32_t)w,
			    event.xconfigure.x,
			    event.xconfigure.y,
			    (int)event.xconfigure.width,
			    (int)event.xconfigure.height
			);
		    for(i = 0; i < display->total_gl_contexts; i++)
		    {
			if(display->toplevel[i] == w)
			{
			    XRectangle *rect = &display->toplevel_geometry[i];
			    rect->x = event.xconfigure.x;
			    rect->y = event.xconfigure.y;
			    rect->width = event.xconfigure.width;
			    rect->height = event.xconfigure.height;
#ifdef XF86VIDMODE_H
			    /* Move viewport if in full screen mode */
			    if(display->fullscreen &&
			       (display->fullscreen_context_num == i)
			    )
			    {
				int scr = DefaultScreen(dpy);
				XF86VidModeSetViewPort(
				    dpy, scr, rect->x, rect->y
				);
			    }
#endif	/* XF86VIDMODE_H */
			    /* Call resize callback */
			    if(display->func_resize != NULL)
				display->func_resize(
				    i, display->func_resize_data,
				    event.xconfigure.x,
				    event.xconfigure.y,
				    (int)event.xconfigure.width,
				    (int)event.xconfigure.height
				);
			    events_handled++;
			    break;
			}
		    }
		    break;

		  case KeyPress:
		    if(gw_debug)
			printf(GW_DEBUG_PREFIX
"Got XEvent KeyPress: window=0x%.8x keycode=0x%.8x\n",
			    (u_int32_t)w,
			    (u_int32_t)event.xkey.keycode
			);
		    key = GWGetCharacterFromKeyCode(
			display,
			event.xkey.keycode
		    );
		    /* First handle key globally (independent of window) */
		    switch(key)
		    {
		      case GWKeyAlt:
			display->alt_key_state = True;
			break;
		      case GWKeyCtrl:
			display->ctrl_key_state = True;
			break;
		      case GWKeyShift:
			display->shift_key_state = True;
			break;
		    }
		    /* Now see which window belongs to this key event */
		    for(i = 0; i < display->total_gl_contexts; i++)
		    {
			if((display->toplevel[i] == w) &&
			   (display->func_keyboard != NULL)
			)
			{
			    display->func_keyboard(
				display->func_keyboard_data,
				key,
				True,
				event.xkey.time
			    );
			    events_handled++;
			    break;
			}
		    }
#if 0
/* This does not work well with dialogs mapped */
		    /* Key event not matched to any window? Then send it "globally" */
		    if(events_handled <= 0)
		    {
			if(display->func_keyboard != NULL)
			{
			    display->func_keyboard(
				display->func_keyboard_data,
				key,
				True,
				event.xkey.time
			    );
			    events_handled++;
			}
		    }
#endif
		    break;

		  case KeyRelease:
		    if(gw_debug)
			printf(GW_DEBUG_PREFIX
"Got XEvent KeyRelease: window=0x%.8x keycode=0x%.8x\n",
			    (u_int32_t)w,
			    (u_int32_t)event.xkey.keycode
			);
		    key = GWGetCharacterFromKeyCode(
			display,
			event.xkey.keycode
		    );
		    /* First handle key globally (independent of window) */
		    switch(key)
		    {
		      case GWKeyAlt:
			display->alt_key_state = False;
			break;
		      case GWKeyCtrl:
			display->ctrl_key_state = False;
			break;
		      case GWKeyShift:
			display->shift_key_state = False;
			break;
		    }
		    /* Now see which window belongs to this key event */
		    for(i = 0; i < display->total_gl_contexts; i++)
		    {
			if((display->toplevel[i] == w) &&
			   (display->func_keyboard != NULL)
			)
			{
			    display->func_keyboard(
				display->func_keyboard_data,
				key,
				False,
				event.xkey.time
			    );
			    events_handled++;
			    break;
			}
		    }
#if 0
/* This does not work well with dialogs mapped */
		    /* Key event not matched to any window? Then send it "globally" */
		    if(events_handled <= 0)
		    {
			if(display->func_keyboard != NULL)
			{
			    display->func_keyboard(
				display->func_keyboard_data,
				key,
				False,
				event.xkey.time
			    );
			    events_handled++;
			}
		    }
#endif
		    break;

		  case ButtonPress:
		    if(gw_debug)
			printf(GW_DEBUG_PREFIX
"Got XEvent ButtonPress: window=0x%.8x button=%i x=%i y=%i\n",
			    (u_int32_t)w,  
			    event.xbutton.button,
			    event.xbutton.x,
			    event.xbutton.y
			);
		    for(i = 0; i < display->total_gl_contexts; i++)
		    {
			if((display->toplevel[i] == w) &&
			   (display->func_pointer != NULL)
			)
			{
			    int		btn_num = 0,
					button_press_count = display->button_press_count;
			    unsigned long	ev_last_time = display->last_button_press,
						ev_time = event.xbutton.time;
			    switch(event.xbutton.button)
			    {
			      case Button1:
				btn_num = 1;
				break;
			      case Button2:
				btn_num = 2;
				break;
			      case Button3:
				btn_num = 3;
				break;
			      case Button4:
				btn_num = 4;
				break;
			      case Button5:
				btn_num = 5;
				break;
			    }
			    display->func_pointer(
				i, display->func_pointer_data,
				event.xbutton.x,
				event.xbutton.y,
				GWEventTypeButtonPress,
				btn_num,
				ev_time
			    );

			    /* Check double click */
			    if((button_press_count == 0) &&
			       (ev_last_time > 0) &&
			       (ev_last_time <= ev_time) &&
		((ev_time - ev_last_time) <= GW_2BUTTON_PRESS_INTERVAL)
			    )
			    {
				display->func_pointer(
				    i, display->func_pointer_data,
				    event.xbutton.x,
				    event.xbutton.y,
				    GWEventType2ButtonPress,
				    btn_num,
				    ev_time
				);
				display->button_press_count++;
			    }
			    /* Check triple click */
			    else if((button_press_count == 1) &&
				    (ev_last_time > 0) &&
				    (ev_last_time <= ev_time) &&
		((ev_time - ev_last_time) <= GW_3BUTTON_PRESS_INTERVAL)
			    )
			    {
				display->func_pointer(
				    i, display->func_pointer_data,
				    event.xbutton.x,
				    event.xbutton.y,
				    GWEventType3ButtonPress,
				    btn_num,
				    ev_time
				);
				display->button_press_count++;
			    }
			    else
			    {
				display->button_press_count = 0;
				display->last_button_press = ev_time;
			    }

			    events_handled++;
			    break;
			}
		    }
		    break;

		  case ButtonRelease:
		    if(gw_debug)
			printf(GW_DEBUG_PREFIX
"Got XEvent ButtonRelease: window=0x%.8x button=%i x=%i y=%i\n",
			    (u_int32_t)w,
			    event.xbutton.button,
			    event.xbutton.x,     
			    event.xbutton.y     
			);
		    for(i = 0; i < display->total_gl_contexts; i++)
		    {
			if((display->toplevel[i] == w) &&
			   (display->func_pointer != NULL)
			)
			{
			    int btn_num = 0;
			    switch(event.xbutton.button)
			    {
			      case Button1:
				btn_num = 1;
				break;
			      case Button2:
				btn_num = 2;
				break;
			      case Button3:
				btn_num = 3;
				break;
			      case Button4:
				btn_num = 4;
				break;
			      case Button5:
				btn_num = 5;
				break;
			    }
			    display->func_pointer(
				i, display->func_pointer_data,
				event.xbutton.x,
				event.xbutton.y,
				GWEventTypeButtonRelease,
				btn_num,
				event.xbutton.time
			    );
			    events_handled++;
			    break;
			}
		    }
		    break;

		  case MotionNotify:
		    for(i = 0; i < display->total_gl_contexts; i++)
		    {
			if((display->toplevel[i] == w) &&
			   (display->func_pointer != NULL)
			)
			{
			    display->func_pointer(
				i, display->func_pointer_data,
				event.xmotion.x,
				event.xmotion.y,
				GWEventTypePointerMotion,
				0,
				event.xmotion.time
			    );
			    events_handled++;
			    break;
			}
		    }
		    break;

		  case VisibilityNotify:
		    if(gw_debug)
			printf(GW_DEBUG_PREFIX
"Got XEvent VisibilityNotify: window=0x%.8x state=%i\n",
			    (u_int32_t)w,
			    event.xvisibility.state
			);
		    for(i = 0; i < display->total_gl_contexts; i++)
		    {
			if((display->toplevel[i] == w) &&
			   (display->func_visibility != NULL)
			)
			{
			    switch(event.xvisibility.state)
			    {
			      case VisibilityFullyObscured:
			        display->func_visibility(
				    i, display->func_visibility_data,
				    GWVisibilityFullyObscured
				);
				break;
			      case VisibilityPartiallyObscured:
				display->func_visibility(
				    i, display->func_visibility_data,
				    GWVisibilityPartiallyObscured
				);
				break;
			      default:
				display->func_visibility(
				    i, display->func_visibility_data,
				    GWVisibilityUnobscured
				);
				break;
			    }
			    events_handled++;
			    break;
			}
		    }
		    break;

		  case ClientMessage:
		    cm = &event.xclient;

		    if(gw_debug && (cm->format == 32))
		    {
			char *atom_name = XGetAtomName(
			    display->display,
			    cm->message_type
			);
			printf(GW_DEBUG_PREFIX
"Got XEvent ClientMessage: window=0x%.8x message_type=0x%.8x atom_name=\"%s\"\n",
			    (u_int32_t)cm->window,
			    (u_int32_t)cm->message_type,
			    atom_name
			);
			XFree(atom_name);
		    }

		    /* Note, NEWER client messages have member
		     * message_type set to the atom to determine the
		     * type of message, while OLDER client messages use
		     * member data.l[0] to determine message type and
		     * message_type is always set to WM_PROTOCOLS
		     */

		    /* WM close (from WM_DELETE_WINDOW) */
		    if((cm->format == 32) &&
		       (cm->data.l[0] == display->atom_wm_delete_window)
		    )
		    {
			for(i = 0; i < display->total_gl_contexts; i++)
			{
			    if(display->toplevel[i] == cm->window)
			    {
				glXWaitGL();
				glXWaitX();
                                // glFinish();
                                // XSync(display->display, False);

			        /* Call close callback */
			        if(display->func_close != NULL)
			            display->func_close(
				        i, display->func_close_data,
				        (void *)w
			            );

				events_handled++;
				break;
		            }
			}
		    }
		    /* WM close (from _NET_CLOSE_WINDOW) */
		    else if((cm->format == 32) &&
			    (cm->message_type == display->atom_wm_close_window)
		    )
		    {
			for(i = 0; i < display->total_gl_contexts; i++)
			{
			    if(display->toplevel[i] == cm->window)
			    {
				glXWaitGL();
				glXWaitX();
                                // glFinish();
                                // XSync(display->display, False);

				/* Call close callback */
				if(display->func_close != NULL)
				    display->func_close(
					i, display->func_close_data,
					(void *)w
				    );

				events_handled++;
				break;
			    }
			}
		    }
		    /* WM ping */
		    else if((cm->format == 32) &&
			    (cm->data.l[0] == display->atom_wm_ping)
		    )
		    {
/*
			const long timestamp = cm->data.l[1];
 */
			for(i = 0; i < display->total_gl_contexts; i++)
			{
			    if(display->toplevel[i] == cm->window)
			    {
			        /* Respond to ping */
			        event.xany.window = display->root;
			        event.xclient.window = display->root;
			        XSendEvent(
				    display->display,
				    display->root,
				    False,
				    SubstructureNotifyMask | SubstructureRedirectMask,
				    &event
				);

				events_handled++;
				break;
			    }
			}
		    }
		    /* Save yourself */
		    else if((cm->format == 32) &&
			    (cm->data.l[0] == display->atom_wm_save_yourself)
		    )
		    {
			for(i = 0; i < display->total_gl_contexts; i++)
			{
			    if(display->toplevel[i] == cm->window)
			    {
				glXWaitGL();
				glXWaitX();
                                // glFinish();
                                // XSync(display->display, False);

				/* Call save yourself callback */
				if(display->func_save_yourself != NULL)
				    display->func_save_yourself(
					i, display->func_save_yourself_data
				    );

				events_handled++;
				break;
			    }
			}
		    }
		    /* Take focus */
		    else if((cm->format == 32) &&
			    (cm->data.l[0] == display->atom_wm_take_focus)
		    )
		    {
			for(i = 0; i < display->total_gl_contexts; i++)
			{
			    if(display->toplevel[i] == cm->window)
			    {
#if 0
/* Don't need to rebind the GL context */
				glXWaitX();
			        glXMakeCurrent(
			            display->display,
			            display->toplevel[i],
			            display->glx_context[i]
			        );
				glXWaitGL();
#endif
				events_handled++;
				break;
			    }
			}
		    }
		    /* Work area (viewport, from _NET_WORKAREA) */
		    else if((cm->format == 32) &&
			    (cm->message_type == display->atom_wm_workarea)
		    )
		    {
			for(i = 0; i < display->total_gl_contexts; i++)
			{
			    if(display->toplevel[i] == cm->window)
			    {
			        display->viewport_x = (int)cm->data.l[0];
				display->viewport_y = (int)cm->data.l[1];
				display->viewport_width = (int)cm->data.l[2];
				display->viewport_height = (int)cm->data.l[3];

				events_handled++;
				break;
			    }
			}
		    }
		    break;

		  default:
		    /* Some extension type event */
#ifdef XF86VIDMODE_H
		    /* XF86 VidMode change? */
		    if(display->has_vidmode_ext)
		    {
#ifdef XF86VidModeNotify
			if(event.type == (display->vidmode_ext_event_offset +
			    XF86VidModeNotify)
			)
			{
			    if(gw_debug)
				printf(
				    GW_DEBUG_PREFIX
			"Got event %i XF86VidModeNotify\n",
				    event.type
				);
			}
#endif	/* XF86VidModeNotify */
#ifdef XF86VidModeModeChange
			if(event.type == (display->vidmode_ext_event_offset +   
			    XF86VidModeModeChange)
			)
			{

			    if(gw_debug)
				printf(
				    GW_DEBUG_PREFIX
			"Got event %i XF86VidModeModeChange\n",
				    event.type
				);
			}
#endif	/* XF86VidModeModeChange */
		    }
#endif	/* XF86VIDMODE_H */
		    break;
		}

		/* Hand event message off to dialog management functions */
		GWXDialogManage(
		    display, &display->mesg_dialog, &event
		);
		GWXDialogManage(
		    display, &display->conf_dialog, &event
		);

	    }
	}

	/* Any pending posted draws for the current context? */
	if(display->gl_context_num > -1)
	{
	    int ctx_num = display->gl_context_num;
	    if((ctx_num >= 0) && (ctx_num < display->total_gl_contexts))
	    {
		if(display->draw_count[ctx_num] > 0)
		{
		    /* Posted draws are pending, call draw function
		     * and reset draw count.
		     */
		    if(display->func_draw != NULL)
			display->func_draw(
			    ctx_num, display->func_draw_data
			);
		    display->draw_count[ctx_num] = 0;
		}
	    }
	}

	/* Always call the timeout function once per call */
	if(display->func_timeout != NULL)
	    display->func_timeout(display->func_timeout_data);
}
#endif	/* X_H */

#ifdef X_H
/*
 *	Graphics wrapper shutdown.
 */
void GWShutdown(gw_display_struct *display)
{
	Display *dpy;

	if(display == NULL)
	    return;

	if(gw_debug)
	    printf(GW_DEBUG_PREFIX
"Beginning shutdown...\n"
	    );

        /* Turn the keyboard auto repeat back on to avoid
           messing with the operating system.
           - Jesse
        */
         XAutoRepeatOn(display->display);

	/* If was in full screen mode then switch back to GUI mode
	 * first before shutdown
	 */
	if(display->fullscreen)
	{
	    if(display->fullscreen_context_num != display->gl_context_num)
		GWContextSet(display, display->fullscreen_context_num);
	    GWContextFullScreen(display, False);
	}

	/* Get pointer to X display structure */
	dpy = display->display;
	if(dpy != NULL)
	{
	    int i;
	    Cursor *cursor;


	    /* Destroy dialog widgets and resources */
	    GWXDialogDestroy(
		display, &display->mesg_dialog
	    );
	    GWXDialogDestroy(
		display, &display->conf_dialog
	    );


	    /* Destroy all gl contexts and toplevel Windows, current
	     * gl context will be disabled first as needed.
	     */
	    for(i = display->total_gl_contexts - 1; i >= 0; i--)
		GWContextDelete(display, i);

	    /* Deallocate pointer arrays for all gl contexts and
	     * toplevel Windows.
	     */
	    free(display->glx_context);
	    display->glx_context = NULL;
	    free(display->toplevel);
	    display->toplevel = NULL;
	    free(display->toplevel_geometry);
	    display->toplevel_geometry = NULL;
	    free(display->draw_count);
	    display->draw_count = NULL;
	    display->total_gl_contexts = 0;
	    display->gl_context_num = -1;

	    /* Ungrab pointer once more just in case */
	    GWUngrabPointer(display);

	    /* Destroy cursors */
#define DO_DESTROY_CURSOR				\
{ if(cursor != NULL) {					\
 if(*cursor != None) {					\
  xerrval = 0;						\
  XFreeCursor(dpy, *cursor);				\
  if(xerrval && gw_debug)				\
   printf(GW_DEBUG_PREFIX				\
"XFreeCursor(): Error destroying Cursor 0x%.8x.\n",	\
    (u_int32_t)(*cursor)				\
   );							\
  *cursor = None;					\
} } }
	    cursor = &display->cursor_standard;
	    DO_DESTROY_CURSOR
	    cursor = &display->cursor_busy;
	    DO_DESTROY_CURSOR
	    cursor = &display->cursor_text;
	    DO_DESTROY_CURSOR
	    cursor = &display->cursor_translate;
	    DO_DESTROY_CURSOR
	    cursor = &display->cursor_zoom;
	    DO_DESTROY_CURSOR
	    cursor = &display->cursor_invisible;
	    DO_DESTROY_CURSOR
#undef DO_DESTROY_CURSOR

	    /* Turn on keyboard autorepeat */
	    /* We do this above.
               - Jesse
               GWKeyboardAutoRepeat(display, True); */

	    if(gw_debug)
		printf(GW_DEBUG_PREFIX
"Closing X display...\n" 
		);

	    /* Free Colormap */
	    if(display->colormap != None)
	    {
		XFreeColormap(dpy, display->colormap);
		display->colormap = None;
	    }

	    /* Destroy XVisualInfo structure */
	    if(display->visual_info != NULL)
	    {
		XFree(display->visual_info);
		display->visual_info = NULL;
	    }

	    /* Close connection to X server */
	    XCloseDisplay(dpy);
	    display->display = dpy = NULL;
	}

	/* Deallocate other display structure resources */
	free(display->def_xfont_name);

	/* Deallocate graphics wrapper display structure itself */
	free(display);

	if(gw_debug)
	    printf(GW_DEBUG_PREFIX
"Shutdown done.\n"
	    );
}
#endif  /* X_H */


#ifdef X_H
/*
 *	Flushes output.
 */
void GWFlush(gw_display_struct *display)
{
	if(display == NULL)
	    return;

	XFlush(display->display);
	glXWaitGL();
        // glFinish();
}
#endif	/* X_H */

#ifdef X_H
/*
 *	Sync output.
 */
void GWSync(gw_display_struct *display)
{
	if(display == NULL)
	    return;

	XSync(display->display, False);
	glXWaitGL();
        // glFinish();
}
#endif  /* X_H */

#ifdef X_H
/*
 *	Returns the number of events pending.
 */
int GWEventsPending(gw_display_struct *display)
{
	return((display != NULL) ? XPending(display->display) : 0);
}
#endif  /* X_H */


#ifdef X_H
/*
 *	Output message.
 */
void GWOutputMessage(
	gw_display_struct *display,
	int type,       /* One of GWOutputMessageType* */
	const char *subject,
	const char *message,
	const char *help_details
)
{
	gwx_dialog_struct *dialog;
	gwx_button_struct *btn;

	if(display == NULL)
	    return;

	dialog = &display->mesg_dialog;

	switch(type)
	{
	  case GWOutputMessageTypeWarning:
	    GWXDialogLoadIcon(
		display, dialog,
		GWX_ICON_WARNING
	    );
	    break;

	  case GWOutputMessageTypeError:
	    GWXDialogLoadIcon(
		display, dialog,
		GWX_ICON_ERROR
	    );
	    break;

	  case GWOutputMessageTypeQuestion:
	    GWXDialogLoadIcon(
		display, dialog,
		GWX_ICON_QUESTION
	    );
	    break;

	  default:
	    GWXDialogLoadIcon(
		display, dialog,
		GWX_ICON_INFO
	    );
	    break;
	}

	/* Set dialog message */
	GWXDialogSetMesg(
	    display, dialog,
	    subject, message, help_details
	);

	/* Set OK button as the default button */
	btn = &dialog->ok_btn;
	GWXWidgetGrabDefault(display, GWX_WIDGET(btn));
	GWXWidgetFocus(display, GWX_WIDGET(btn));

	/* Map and draw dialog */
	GWXDialogMap(display, dialog);
}
#endif  /* X_H */

#ifdef X_H
/*
 *	Blocks client execution until confirmation is returned.
 *	Returns one of GWConfirmation*.
 *
 *	If this function is called while already blocking then
 *	GWConfirmationNotAvailable will be returned immediatly.
 */
int GWConfirmation(
	gw_display_struct *display,
	int type,		/* One of GWOutputMessageType* */
	const char *subject,
	const char *message,
	const char *help_details,
	int default_response	/* One of GWConfirmation* */
)
{
	static Boolean reenterant = False;
	int status;
	gwx_dialog_struct *dialog;
	gwx_button_struct *btn;


	if(reenterant)
	    return(GWConfirmationNotAvailable);
	else
	    reenterant = True;

	/* Is display valid? */
	if(display == NULL)
	{
	    /* No display, so return with GWConfirmationNotAvailable */
	    reenterant = False;
	    return(GWConfirmationNotAvailable);
	}
	else
	{
	    /* Get pointer to dialog */
	    dialog = &display->conf_dialog;
	}

	/* Load icon */
	switch(type)
	{
	  case GWOutputMessageTypeWarning:
	    GWXDialogLoadIcon(
		display, dialog,
		GWX_ICON_WARNING
	    );
	    break;

	  case GWOutputMessageTypeError:
	    GWXDialogLoadIcon(
		display, dialog,
		GWX_ICON_ERROR
	    );
	    break; 

	  case GWOutputMessageTypeQuestion:
	    GWXDialogLoadIcon(
		display, dialog,
		GWX_ICON_QUESTION
	    );
	    break;

	  default:
	    GWXDialogLoadIcon(
		display, dialog,
		GWX_ICON_INFO
	    );
	    break;
	}

	/* Set message */
	GWXDialogSetMesg(
	    display, dialog,
	    subject, message, help_details
	);

	/* Set default button in accordance with the confirmation */
	btn = &dialog->yes_btn;
	GWXWidgetUngrabDefault(display, GWX_WIDGET(btn));
	GWXWidgetUnfocus(display, GWX_WIDGET(btn));
	btn = &dialog->no_btn;
	GWXWidgetUngrabDefault(display, GWX_WIDGET(btn));
	GWXWidgetUnfocus(display, GWX_WIDGET(btn));
	btn = &dialog->cancel_btn;
	GWXWidgetUngrabDefault(display, GWX_WIDGET(btn));
	GWXWidgetUnfocus(display, GWX_WIDGET(btn));
	switch(default_response)
	{
	  case GWConfirmationNo:
	    btn = &dialog->no_btn;
	    break;
	  case GWConfirmationYes:
	    btn = &dialog->yes_btn;
	    break;
	  case GWConfirmationCancel:
	    btn = &dialog->no_btn;
	    break;
	}
	GWXWidgetGrabDefault(display, GWX_WIDGET(btn));
	GWXWidgetFocus(display, GWX_WIDGET(btn));

	GWXDialogMap(display, dialog);

	/* Block until confirmation is recieved */
	status = GWXDoBlockUntilConf(display);

	/* Unmap confirmation dialog */
	GWXWidgetUnmap(display, GWX_WIDGET(dialog));

	reenterant = False;
	return(status);
}
#endif	/* X_H */

#ifdef X_H
/*
 *      Block client execution until confirmation is returned.
 *      Returns one of GWConfirmation*.
 */
int GWConfirmationSimple(
	gw_display_struct *display, const char *message 
)
{
	return(GWConfirmation(
	    display,
	    GWOutputMessageTypeQuestion,
	    "Confirmation",
	    message,
	    NULL,
	    GWConfirmationYes
	));
}
#endif	/* X_H */

#ifdef X_H
/*
 *	Sets redraw (expose) callback function:
 */
void GWSetDrawCB(
	gw_display_struct *display,
	void (*func)(int, void *),
	void *data
)
{
	if(display == NULL)
	    return;

	display->func_draw = func;
	display->func_draw_data = data;
}

/*
 *	Sets resize (configure notify) callback function:
 */
void GWSetResizeCB(
	gw_display_struct *display, 
	void (*func)(int, void *, int, int, int, int),
	void *data
)
{
	if(display == NULL)
	    return;

	display->func_resize = func;
	display->func_resize_data = data;
}

/*
 *	Sets keyboard event callback function:
 */
void GWSetKeyboardCB(
	gw_display_struct *display,
	void (*func)(void *, int, Boolean, unsigned long),
	void *data
)
{
	if(display == NULL)
	    return;

	display->func_keyboard = func;
	display->func_keyboard_data = data;
}

/*
 *	Sets visibility notify callback function:
 */
void GWSetVisibilityCB(
	gw_display_struct *display,
	void (*func)(int, void *, gw_visibility),
	void *data
)
{
	if(display == NULL)
	    return;

	display->func_visibility = func;
	display->func_visibility_data = data;
}

/*
 *	Sets pointer event callback function:
 */
void GWSetPointerCB(
	gw_display_struct *display,
	void (*func)(int, void *, int, int, gw_event_type, int, unsigned long),
	void *data 
)
{
	if(display == NULL)
	    return;
 
	display->func_pointer = func;
	display->func_pointer_data = data;
}

/*
 *	Sets save yourself callback function:
 */
void GWSetSaveYourselfCB(
	gw_display_struct *display,
	void (*func)(int, void *),
	void *data
)
{
	if(display == NULL)
	    return;

	display->func_save_yourself = func;
	display->func_save_yourself_data = data;
}

/*
 *	Sets window close (delete event) callback function:
 */
void GWSetCloseCB(
	gw_display_struct *display,
	void (*func)(int, void *, void *),
	void *data
)
{
	if(display == NULL)
	    return;

	display->func_close = func;
	display->func_close_data = data;
}

/*
 *	Sets timeout (idle) callback function:
 */
void GWSetTimeoutCB(
	gw_display_struct *display,
	void (*func)(void *),
	void *data
)
{
	if(display == NULL)
	    return;

	display->func_timeout = func;
	display->func_timeout_data = data;
}
#endif	/* X_H */


#ifdef X_H
/*
 *	Creates a new GL context and window, returning the context
 *	number or -1 on error.
 */
int GWContextNew(
	gw_display_struct *display,
	int x, int y, int width, int height,
	const char *title,
	const char *icon_path, const char *icon_name,
	Bool no_windows
)
{
	Display *dpy;
	Window w;
	GLXContext glx_context;
	XRectangle *rect;
	int ctx_num;


	if(display == NULL)
	    return(-1);

	dpy = display->display;
	if(dpy == NULL)
	    return(-1);

	/* Search for an available GL context index number */
	for(ctx_num = 0; ctx_num < display->total_gl_contexts; ctx_num++)
	{
	    w = display->toplevel[ctx_num];
	    glx_context = display->glx_context[ctx_num];

	    /* Is this GL context index available? */
	    if((w == None) && (glx_context == NULL))
		break;
	}
	/* No available GL contexts? */
	if(ctx_num >= display->total_gl_contexts)
	{
	    /* Allocate a new gl context */

	    ctx_num = MAX(display->total_gl_contexts, 0);
	    display->total_gl_contexts = ctx_num + 1;

	    display->glx_context = (GLXContext *)realloc(
		display->glx_context,
		display->total_gl_contexts * sizeof(GLXContext)
	    );
	    display->toplevel = (Window *)realloc(
		display->toplevel,
		display->total_gl_contexts * sizeof(Window)
	    );
	    display->toplevel_geometry = (XRectangle *)realloc(
		display->toplevel_geometry,
		display->total_gl_contexts * sizeof(XRectangle)
	    );
	    display->draw_count = (int *)realloc(
		display->draw_count,
		display->total_gl_contexts * sizeof(int)
	    );
	    if((display->glx_context == NULL) || (display->toplevel == NULL) ||
	       (display->toplevel_geometry == NULL) || (display->draw_count == NULL)
	    )
	    {
		display->gl_context_num = -1;
		display->total_gl_contexts = 0;
		return(-1);
	    }
	}

	/* At this point we have an available GL context specified by
	 * ctx_num
	 */

	/* Create GL rendering context */
	display->glx_context[ctx_num] = glx_context = glXCreateContext(
	    dpy,
	    display->visual_info,	/* Visual info */
	    NULL,			/* Share list (none) */
	    (display->direct_rendering) ? GL_TRUE : GL_FALSE    /* Direct rendering? */
	);
	if(glx_context == NULL)
	{
	    if(gw_debug)
		printf(GW_DEBUG_PREFIX
 "glXCreateContext(): Cannot create glX %s rendering context.\n",
		    (display->direct_rendering) ? "hardware" : "software"
		);

	    fprintf(
		stderr,
		"Error creating glX %s rendering context.\n",
		(display->direct_rendering) ? "hardware" : "software"
	    );
	    if(display->direct_rendering)
		fprintf(
		    stderr,
 "Try software rendering instead (add argument \"--software\")?\n"
		);
	}
	else
	{
	    glXWaitGL();	/* Wait for GL context to be created */
	    // glFinish();
            if(gw_debug)
		printf(GW_DEBUG_PREFIX
 "glXCreateContext(): Created glX %s rendering context.\n",
		    (display->direct_rendering) ? "hardware" : "software"
		);
	}

	/* Create first toplevel window and buffer */
	if(!no_windows)
	{
	    /* Set initial geometry of new Window before creation */
	    rect = &display->toplevel_geometry[ctx_num];
	    rect->x = x;
	    rect->y = y;
	    rect->width = (unsigned int)width;
	    rect->height = (unsigned int)height;

	    /* Create first toplevel Window */
	    display->toplevel[ctx_num] = w = GWCreateWindow(
		display,
		display->root,
		x, y,
		width, height,
		title
	    );
	    if(w == None)
	    {
		/* Error creating Window */
		fprintf(
		    stderr,
 "Error creating toplevel window for GL context %i.\n",
		    ctx_num
		);
		return(-1);
	    }
	    else
	    {
		XTextProperty *t;
		XClassHint *class_hint;

		/* Set the Window's WM Name */
		t = (XTextProperty *)malloc(sizeof(XTextProperty));
		if(t != NULL)
		{
		    if(!STRISEMPTY(title))
		    {
			t->value = (unsigned char *)title;
			t->encoding = XA_STRING;
			t->format = 8;
			t->nitems = STRLEN(title);
			xerrval = 0;
			XSetWMName(dpy, w, t);
			if(xerrval && gw_debug)
			    printf(GW_DEBUG_PREFIX
"GWContextNew(): Error setting the WM name for window 0x%.8x.\n",
			        (u_int32_t)w
			    );
		    }
		    free(t);
		}

		/* Set the Window's WM Class Hint */
		class_hint = XAllocClassHint();
		if(class_hint != NULL)
		{
		    if(!STRISEMPTY(title))
		    {
			class_hint->res_name = "toplevel";
			class_hint->res_class = (char *)title;
			xerrval = 0;
			XSetClassHint(dpy, w, class_hint);
			if(xerrval && gw_debug)
			    printf(GW_DEBUG_PREFIX
"GWContextNew(): Error setting the WM class for window 0x%.8x.\n",
				(u_int32_t)w
			    );
		    }
		    XFree(class_hint);
		}

		/* Set the Window's WM Icon */
		GWSetWindowIconFile(
		    display, ctx_num, icon_path, icon_name
		);

		/* Map the Window */
		xerrval = 0;
		XMapRaised(dpy, w);
		if(xerrval && gw_debug)
		    printf(GW_DEBUG_PREFIX
"XMapRaised(): Error maping and raising Window 0x%.8x.\n",
			(u_int32_t)w
		    );

		/* Need to move window after mapping it, because the WM
		 * often places it at the wrong sport despite giving
		 * proper hints as to the creation position
		 */
		xerrval = 0;
		XMoveWindow(dpy, w, x, y);
		if(xerrval && gw_debug)
		    printf(GW_DEBUG_PREFIX
"XMoveWindow(): Error moving Window 0x%.8x.\n",
			(u_int32_t)w
		    );

		glXWaitX();	/* Wait for X window creation commands to finish */
                // XSync(display->display, False);
	    }
	    display->draw_count[ctx_num] = 0;
	}
	else
	{
	    rect = &display->toplevel_geometry[ctx_num];
	    memset(rect, 0x00, sizeof(XRectangle));
	    display->toplevel[ctx_num] = None;
	    display->draw_count[ctx_num] = 0;
	}
	if(gw_debug)
	    printf(GW_DEBUG_PREFIX
"Created gl context number %i.\n",
		ctx_num
	    );

	return(ctx_num);
}
#endif	/* X_H */

#ifdef X_H
/*
 *	Destroys the gl context and window given by the context number
 *	ctx_num. If the specified gl context is the current gl context
 *	then the current gl context will be no longer valid.
 */
void GWContextDelete(
	gw_display_struct *display, int ctx_num
)
{
	Display *dpy;
	Window w;
	GLXContext glx_context;
	XRectangle *rect;


	if(display == NULL)
	    return;

	dpy = display->display;
	if(dpy == NULL)
	    return;

	if((ctx_num < 0) || (ctx_num >= display->total_gl_contexts))
	    return;

	if(gw_debug)
	    printf(GW_DEBUG_PREFIX
"Destroying gl context number %i...\n",
		ctx_num
	    );

	/* Is the given gl context the current gl context? */
	if(ctx_num == display->gl_context_num)
	{
	    display->gl_context_num = -1;

	    if(gw_debug)
		printf(GW_DEBUG_PREFIX
"Disabling current gl context because it is about to be destroyed...\n"
		);

	    glXWaitX();
            // XSync(display->display, False);
	    glXMakeCurrent(dpy, None, NULL);
	    glXWaitGL();
            // glFinish();
	}

	/* Begin destroying the Window */
	w = display->toplevel[ctx_num];
	if(w != None)
	{
	    XWMHints *wm_hints;

	    /* If this context's window is grabbing the pointer then
	     * ungrab the pointer before destroying the window.
	     */
	    if(display->grab_window == w)
		GWUngrabPointer(display);

	    /* Get WM hints of window */
	    xerrval = 0;
	    wm_hints = XGetWMHints(dpy, w);
	    if(!xerrval && (wm_hints != NULL))
	    {
		/* Deallocate resources of window specified in the WM
		 * hints structure.
		 */
		if(wm_hints->flags & IconWindowHint)
		    if(wm_hints->icon_window != None)
			DESTROY_WINDOW(&wm_hints->icon_window);
		if(wm_hints->flags & IconPixmapHint)
		    if(wm_hints->icon_pixmap != None)
			FREE_PIXMAP(&wm_hints->icon_pixmap);
		if(wm_hints->flags & IconMaskHint)
		    if(wm_hints->icon_mask != None)
			FREE_BITMAP(&wm_hints->icon_mask);

		/* Deallocate recieved WM hints structure */
		XFree(wm_hints);
	    }

	    /* Destroy the toplevel Window */
	    xerrval = 0;
	    DESTROY_WINDOW(&w);
	    if(xerrval && gw_debug)
		printf(GW_DEBUG_PREFIX
"XDestroyWindow(): Error destroying window 0x%.8x.\n",
		    (u_int32_t)display->toplevel[ctx_num]
		);
	    else if(gw_debug)
		printf(GW_DEBUG_PREFIX
"Destroyed window 0x%.8x.\n",
		    (u_int32_t)display->toplevel[ctx_num]
		);
	    display->toplevel[ctx_num] = None;

	    glXWaitX();	/* Wait for X window destroy commands to finish */
            // XSync(display->display, False);
	}

	/* Begin destroying the gl context */
	glx_context = display->glx_context[ctx_num];
	if(glx_context != NULL)
	{
	    /* Destroy gl context */
	    glXDestroyContext(dpy, glx_context);
	    if(gw_debug)
		printf(GW_DEBUG_PREFIX
"Destroyed glX context 0x%.8x.\n",
		    (u_int32_t)glx_context
		);
	    display->glx_context[ctx_num] = glx_context = NULL;

	    // glXWaitGL();	/* Wait for GL context destroy commands to finish */
	}

	/* Clear the toplevel rectangle */
	rect = &display->toplevel_geometry[ctx_num];
	memset(rect, 0x00, sizeof(XRectangle));

	/* Reset draw count */
	display->draw_count[ctx_num] = 0;

	if(gw_debug)
	    printf(GW_DEBUG_PREFIX
"Destroyed gl context number %i.\n",
		ctx_num
	    );
}
#endif /* X_H */

#ifdef X_H
/*
 *	Returns the current GL context number or -1 on error.
 */
int GWContextCurrent(gw_display_struct *display)
{
	return((display != NULL) ?
	    MIN(display->gl_context_num, display->total_gl_contexts - 1) : -1
	);
}
#endif /* X_H */

#ifdef X_H
/*
 *	Gets the opaque handle of the Window and the GL rendering
 *	context of the specified GL context.
 *
 *	Returns non-zero on error.
 */
int GWContextGet(
	gw_display_struct *display, int ctx_num,
	void **window_id_rtn, void **gl_context_rtn,
	int *x_rtn, int *y_rtn, int *width_rtn, int *height_rtn
)
{
	Display *dpy = (display != NULL) ? display->display : NULL;
	Window w;
	GLXContext glx_context;
	const XRectangle *rect;


	if(window_id_rtn != NULL)
	    *window_id_rtn = NULL;
	if(gl_context_rtn != NULL)
	    *gl_context_rtn = NULL;
	if(x_rtn != NULL)
	    *x_rtn = 0;
	if(y_rtn != NULL)
	    *y_rtn = 0;
	if(width_rtn != NULL)
	    *width_rtn = 0;
	if(height_rtn != NULL)
	    *height_rtn = 0;

	if(dpy == NULL)
	    return(-1);

	if((ctx_num < 0) || (ctx_num >= display->total_gl_contexts))
	    return(-1);

	w = display->toplevel[ctx_num];
	glx_context = display->glx_context[ctx_num];
	rect = &display->toplevel_geometry[ctx_num];

	if((w == None) || (glx_context == NULL))
	    return(-1);


	if(window_id_rtn != NULL)
	    *window_id_rtn = (void *)w;
	if(gl_context_rtn != NULL)
	    *gl_context_rtn = (void *)glx_context;

	if(x_rtn != NULL)
	    *x_rtn = (int)rect->x;
	if(y_rtn != NULL)
	    *y_rtn = (int)rect->y;
	if(width_rtn != NULL)
	    *width_rtn = (int)rect->width;
	if(height_rtn != NULL)
	    *height_rtn = (int)rect->height;

	return(0);
}
#endif  /* X_H */

#ifdef X_H
/*
 *	Puts the GL rendering context specified by ctx_num as the
 *	current rendering GL context.
 *
 *	Returns non-zero on error.
 */
int GWContextSet(gw_display_struct *display, int ctx_num)
{
	Display *dpy;
	Window w;
	GLXContext glx_context;


	if(display == NULL)
	    return(-1);

	dpy = display->display;
	if(dpy == NULL)
	    return(-1);

	if((ctx_num < 0) || (ctx_num >= display->total_gl_contexts))
	    return(-1);

	/* Specified gl context already the current gl context? */
	if(ctx_num == display->gl_context_num)
	    return(0);


	/* Get Window and gl context of the specified context */
	w = display->toplevel[ctx_num];
	glx_context = display->glx_context[ctx_num];

	if((w == None) || (glx_context == NULL))
	    return(-1);

	glXWaitX();	/* Wait for X commands to finish */
        // XSync(display->display, False);

	/* Make selected GL context current */
	if(glXMakeCurrent(dpy, (GLXDrawable)w, glx_context))
	{
	    display->gl_context_num = ctx_num;
	    glXWaitGL();	/* Wait for GL context switch commands to finish */
            // glFinish();
	    return(0);
	}
	else
	{
	    return(-1);
	}
}
#endif	/* X_H */

#ifdef X_H
/*
 *	Sets the position of the current GL rendering context.
 */
void GWContextPosition(
	gw_display_struct *display, int x, int y
)
{
	Display *dpy = (display != NULL) ? display->display : NULL;
	int ctx_num;
	Window w;
	XRectangle *rect;
	if(dpy == NULL)
	    return;

	ctx_num = display->gl_context_num;
	if((ctx_num >= 0) && (ctx_num < display->total_gl_contexts))
	{
	    w = display->toplevel[ctx_num];
	    rect = &display->toplevel_geometry[ctx_num];
	}
	else
	{
	    w = None;
	    rect = NULL;
	}
	if(w == None)
	    return;

	if((x != rect->x) || (y != rect->y))
	{
	    glXWaitGL();
            // glFinish();

	    xerrval = 0;
	    XMoveWindow(dpy, w, x, y);
	    if(xerrval && gw_debug)
		printf(GW_DEBUG_PREFIX
"XMoveWindow(): Error moving Window 0x%.8x.\n",
		    (u_int32_t)w
		);

	    // glXWaitX();
            XSync(display->display, False);

	    /* Need to update position immediately */
	    rect->x = x;
	    rect->y = y;
	}
}
#endif	/* X_H */

#ifdef X_H
/*
 *	Sets the size of the current GL rendering context.
 *
 *	If currently in full screen then the calling function needs
 *	to call GWContextFullScreen(display, True) after calling this
 *	function so that the full screen is adjusted to the new size.
 */
void GWContextSize(
	gw_display_struct *display, int width, int height
)
{
	Display *dpy = (display != NULL) ? display->display : NULL;
	int ctx_num;
	Window w;
	XRectangle *rect;
	if(dpy == NULL)
	    return;

	ctx_num = display->gl_context_num;
	if((ctx_num >= 0) && (ctx_num < display->total_gl_contexts))
	{
	    w = display->toplevel[ctx_num];
	    rect = &display->toplevel_geometry[ctx_num];
	}
	else
	{
	    w = None;
	    rect = NULL;
	}
	if(w == None)
	    return;

	if((((int)rect->width != width) || ((int)rect->height != height)) &&
	    ((width >= GW_MIN_TOPLEVEL_WIDTH) && (height >= GW_MIN_TOPLEVEL_HEIGHT))
	)
	{
	    int new_width = (int)MAX(
		    ((width > 0) ? width : rect->width),
		    GW_MIN_TOPLEVEL_WIDTH
		),
		new_height = (int)MAX(
		    ((height > 0) ? height : rect->height),
		    GW_MIN_TOPLEVEL_HEIGHT
		);

	    glXWaitGL();
            // glFinish();
	    xerrval = 0;
	    XResizeWindow(dpy, w, new_width, new_height);
	    glXWaitX();
            // XSync(display->display, False);

	    if(xerrval && gw_debug)
		printf(GW_DEBUG_PREFIX
"XResizeWindow(): Error resizing Window 0x%.8x.\n",
		    (u_int32_t)w
		);

	    /* Need to update size immediately */
	    rect->width = new_width;
	    rect->height = new_height;
	}
}
#endif  /* X_H */

#ifdef X_H
/*
 *	Returns True if the current context is currently full screen.
 */
Boolean GWContextIsFullScreen(gw_display_struct *display)
{
	return((display != NULL) ? display->fullscreen : False);
}
#endif  /* X_H */

#ifdef X_H
/*
 *	Sets the current context into full screen if state is True
 *	or returns to GUI mode if state is False.
 *
 *	The full screen size depends on the current size of the GL
 *	context so GWContextSize() should be called prior to going
 *	to full screen.
 *
 *	For X the size of the toplevel window must match the size of
 *	a vidmode defined by the X server or else -2 is returned.
 *
 *	Returns:
 *
 *	0	Success
 *	-1	General error
 *	-2	Unable to find valid mode
 *	-3	System error
 *	-5	This function is not supported
 */
int GWContextFullScreen(gw_display_struct *display, Boolean state)
{
#ifdef XF86VIDMODE_H
	Display *dpy;
	int ctx_num;
	int scr;
	Window w;
	XRectangle *rect, *last_vidmode_gui_rect;


	if(display == NULL)
	    return(-1);

	if(!display->has_vidmode_ext)
	    return(-5);

	dpy = display->display;
	if(dpy == NULL)
	    return(-1);

	ctx_num = display->gl_context_num;
	if((ctx_num < 0) || (ctx_num >= display->total_gl_contexts))
	    return(-1);

	scr = DefaultScreen(dpy);
	w = display->toplevel[ctx_num];
	rect = &display->toplevel_geometry[ctx_num];
	last_vidmode_gui_rect = &display->vidmode_last_gui_geometry;
	if(w == None)
	    return(-3);

	/* Go to full screen mode? */
	if(state)
	{
	    Boolean switched_modes = False;
	    XF86VidModeModeLine mode_line;
	    XF86VidModeModeInfo	**mode_line_info, *m;
	    int i, n, dotclock;
	    int	vidmode_last_x = 0,
		vidmode_last_y = 0;
	    unsigned int	vidmode_last_width = 0,
				vidmode_last_height = 0;

	    /* Get current video mode geometry values */
	    XF86VidModeGetViewPort(
		dpy, scr, &vidmode_last_x, &vidmode_last_y
	    );
	    if(XF86VidModeGetModeLine(
		dpy, scr, &dotclock, &mode_line
	    ))
	    {
		vidmode_last_width = (unsigned int)mode_line.hdisplay;
		vidmode_last_height = (unsigned int)mode_line.vdisplay;
	    }

	    /* Search for suitable video mode that matches the current
	     * toplevel window size
	     */
	    if(XF86VidModeGetAllModeLines(
		dpy, scr, &n, &mode_line_info
	    ))
	    {
		for(i = 0; i < n; i++)
		{
		    m = mode_line_info[i];
		    if(m == NULL)
			continue;

		    /* This mode suitable? */
		    if(!switched_modes &&
		       (m->hdisplay == rect->width) &&
		       (m->vdisplay == rect->height)
		    )
		    {
			/* Record last position of toplevel window in
			 * GUI mode
			 */
			if(!display->fullscreen)
			{
			    display->vidmode_last_gui_wx = rect->x;
			    display->vidmode_last_gui_wy = rect->y;
			}

			/* Begin switching to new vidmode */

			/* Raise window above other windows */
			xerrval = 0;
			XMapRaised(dpy, w);
			if(xerrval && gw_debug)
			    printf(GW_DEBUG_PREFIX
"XMapRaised(): Error mapping and raising Window 0x%.8x.\n",
				(u_int32_t)w
			    );

			/* Move window to the center of the desktop */
			rect->x = (int)(display->root_width - rect->width) / 2;
			rect->y = (int)(display->root_height - rect->height) / 2;
			xerrval = 0;
			XMoveWindow(dpy, w, rect->x, rect->y);
			if(xerrval && gw_debug)
			    printf(GW_DEBUG_PREFIX
"XMoveWindow(): Error moving Window 0x%.8x.\n",
				(u_int32_t)w
			    );

			/* Move pointer to the center of the window */
			xerrval = 0;
			XWarpPointer(
			    dpy, None, w,
			    0, 0, 0, 0,
			    (int)rect->width / 2,
			    (int)rect->height / 2
			);
			if(xerrval && gw_debug)
			    printf(GW_DEBUG_PREFIX
"XWarpPointer(): Error moving pointer.\n"
			    );

			/* Grab the pointer so that it is confined to
			 * the window's area
			 */
			GWGrabPointer(display, w, True);

			/* Switch vidmode */
			if(XF86VidModeSwitchToMode(dpy, scr, m))
			    switched_modes = True;

#if 0
/* Move the virtual screen when the ConfigureNotify event is received */
			XF86VidModeSetViewPort(
			    dpy, scr, rect->x, rect->y
			);
#endif

			if(!switched_modes && gw_debug)
			    printf(GW_DEBUG_PREFIX
"Unable to switch to video mode %ix%i for full screen mode.\n",
				m->hdisplay, m->vdisplay
			    );
		    }

		    /* If private is not NULL then it needs to be
		     * deleted
		     *
		     * Currently, this is only needed for the S3 servers
		     */
		    if((m->privsize > 0) && (m->private != NULL))
		    {
			XFree(m->private);
			m->private = NULL;
			m->privsize = 0;
		    }

		    /* Do not delete each structure */
/*		    XFree(m); */
		}
		XFree(mode_line_info);
	    }

	    /* Successfully switched modes? */
	    if(switched_modes)
	    {
		/* Record previous GUI values */
		if(!display->fullscreen)
		{
		    last_vidmode_gui_rect->x = vidmode_last_x;
		    last_vidmode_gui_rect->y = vidmode_last_y;
		    last_vidmode_gui_rect->width = vidmode_last_width;
		    last_vidmode_gui_rect->height = vidmode_last_height;
		}
		display->fullscreen = True;
		display->fullscreen_context_num = ctx_num;
		return(0);
	    }
	    else
	    {
		return(-2);
	    }
	}
	else
	{
	    /* Go to GUI mode */
	    Boolean switched_modes = False;
	    XF86VidModeModeInfo **mode_line_info, *m;
	    int i, n;

	    /* If this context's window is grabbing the pointer then
	     * ungrab the pointer before switching video modes.
	     */
	    if(display->grab_window == w)
		GWUngrabPointer(display);

	    /* Search for suitable video mode that matches the previous
	     * GUI geometry.
	     */
	    if(XF86VidModeGetAllModeLines(
		dpy, scr, &n, &mode_line_info
	    ))
	    {
		for(i = 0; i < n; i++)
		{
		    m = mode_line_info[i];
		    if(m == NULL)
			continue;

		    /* This mode suitable? */
		    if(!switched_modes &&
		       (m->hdisplay == last_vidmode_gui_rect->width) &&
		       (m->vdisplay == last_vidmode_gui_rect->height)
		    )
		    {
			/* Switch vidmode */
			if(XF86VidModeSwitchToMode(dpy, scr, m))
			    switched_modes = True;

			if(switched_modes)
			{
			    /* Move virtual screen to the last recorded
			     * position it was in GUI mode
			     */
			    XF86VidModeSetViewPort(
				dpy, scr,
				last_vidmode_gui_rect->x,
				last_vidmode_gui_rect->y
			    );
			    /* Move window to the last recorded
			     * position it was in the GUI mode
			     */
			    if((rect->x != display->vidmode_last_gui_wx) ||
			       (rect->y != display->vidmode_last_gui_wy)
			    )
			    {
				rect->x = display->vidmode_last_gui_wx;
				rect->y = display->vidmode_last_gui_wy;
				xerrval = 0;
				XMoveWindow(
				    dpy, w, rect->x, rect->y
				);
				if(xerrval && gw_debug)
				    printf(GW_DEBUG_PREFIX
"XMoveWindow(): Error moving Window 0x%.8x.\n",
					(u_int32_t)w
				    );
			    }

			    /* Move pointer to center of window */
			    xerrval = 0;
			    XWarpPointer(
				dpy, None, w,
				0, 0, 0, 0,
				(int)rect->width / 2,
				(int)rect->height / 2
			    );
			    if(xerrval && gw_debug)
				printf(GW_DEBUG_PREFIX
"XWarpPointer(): Error moving pointer.\n"
				);
			}

			if(!switched_modes && gw_debug)
			    printf(GW_DEBUG_PREFIX
"Unable to switch to video mode %ix%i to restore GUI mode.\n",
				m->hdisplay, m->vdisplay
			    );
		    }

		    /* If private is not NULL then it needs to be
		     * deallocated.  Currently this only occures for
		     * the S3 servers.
		     */
		    if((m->privsize > 0) && (m->private != NULL))
		    {
			XFree(m->private);
			m->private = NULL;
			m->privsize = 0;
		    }

		    /* Do not free each structure */
/*                  XFree(m); */
		}
		XFree(mode_line_info);
	    }

	    /* Successfully switched modes? */
	    if(switched_modes)
	    {
		/* Mark that we are no longer in full screen mode */
		display->fullscreen = False;
		display->fullscreen_context_num = -1;
		return(0);
	    }
	    else
	    {
		return(-2);
	    }
	}
#endif
	return(-5);
}
#endif	/* X_H */


#ifdef X_H
/*
 *	Sends a (synthetic) redraw event for the current context.
 */
void GWPostRedraw(gw_display_struct *display)
{
	int ctx_num = GWContextCurrent(display);
	if(ctx_num < 0)
	    return;

#if 0
/* This makes drawing async and some progrmas want to redraw
 * immediately which causes problems
 */
	/* Increment queued draw count */
	display->draw_count[ctx_num] = MAX(
	    display->draw_count[ctx_num] + 1, 0
	);
#else
	/* Call draw callback to draw */
	if(display->func_draw != NULL)
	    display->func_draw(
		ctx_num, display->func_draw_data
	    );
#endif
}
#endif	/* X_H */

#ifdef X_H
/*
 *	Swaps buffers (if using double buffers), all GL operations are
 *	gauranteed to be completed after this call.
 */
void GWSwapBuffer(gw_display_struct *display)
{
	int ctx_num;
	Display *dpy;
	GLXDrawable gl_drawable;

	if(display == NULL)
	    return;

	dpy = display->display;
	if(dpy == NULL)
	    return;

	ctx_num = display->gl_context_num;
	if((ctx_num >= 0) && (ctx_num < display->total_gl_contexts))
	    gl_drawable = (GLXDrawable)display->toplevel[ctx_num];
	else
	    gl_drawable = None;
	if(gl_drawable == None)
	    return;

	/* If using double buffers then we need to swap buffers, this
	 * will flush all GL commands.  If we are using single buffer
	 * then do not swap buf still wait for GL commands to be
	 * executed since this call should have all GL commands flushed.
	 */
	if(display->has_double_buffer)
	    glXSwapBuffers(dpy, gl_drawable);
	else
	    glXWaitGL();
            // glFinish();

	/* Reset draw count since we've just drawn */
	display->draw_count[ctx_num] = 0;
}
#endif  /* X_H */
  
#ifdef X_H
/*
 *	Turns keyboard auto repeat on/off.
 */
void GWKeyboardAutoRepeat(gw_display_struct *display, Boolean b)
{
	if(display == NULL)
	    return;
	if(display->display == NULL)
	    return;

	xerrval = 0;
        // XAutoRepeatOff(display->display);
/*
        Getting rid of this as it causes all sorts of trouble
        on Ubuntu. The key does not stop repeating for some reason.
        Will turn it back on at the end of the game to avoid
        messing with the operating system.
        - Jesse <jessefrgsmith@yahoo.ca>

        Update: Leaving it off causes problems for people who
        use other apps while SAR is running. Will turn ON
        auto-repeat UNLESS --no-repeat flag is set.
        -- Jesse
*/
        if (display->allow_autorepeat)
        {
	   if(b)
           {
	      XAutoRepeatOn(display->display);
           }
	   else
           {
	      XAutoRepeatOff(display->display);
           }
        }
        else // auto repeat is forbidden so...
            XAutoRepeatOff(display->display);


	if(xerrval && gw_debug)
	    printf(GW_DEBUG_PREFIX
"Cannot switch keyboard auto repeat.\n"
	    );
}
#endif  /* X_H */


#ifdef X_H
/*
 *	Returns a list of video modes, the number of video modes
 *	will be set to the storage pointer to by n.
 *
 *	Can return NULL on error or information not available.
 *
 *	The calling function needs to free() the returned pointer.
 */
gw_vidmode_struct *GWVidModesGet(
	gw_display_struct *display, int *n
)
{
#ifdef XF86VIDMODE_H
	Display *dpy;
	Boolean is_dup;
	int i, j, k, t = 0, scr;
	XF86VidModeModeInfo **mode_line_info, *m;
	gw_vidmode_struct *vm = NULL, *vm_ptr;

	if(n != NULL)
	    *n = t;

	if(display == NULL)
	    return(vm);

	dpy = display->display;
	if(dpy == NULL)
	    return(vm);

	scr = DefaultScreen(dpy);

	if(XF86VidModeGetAllModeLines(
	    dpy, scr, &t, &mode_line_info
	))
	{
	    for(i = 0, j = 0; i < t; i++)
	    {
		m = mode_line_info[i];
		if(m == NULL)
		    continue;

		/* Check for duplicate video mode */
		for(is_dup = False, k = 0; k < j; k++)
		{
		    vm_ptr = &vm[k];
		    if((vm_ptr->width == m->hdisplay) &&
		       (vm_ptr->height == m->vdisplay) &&
		       (vm_ptr->vwidth == m->htotal) &&
		       (vm_ptr->vheight == m->vtotal)
		    )
		    {
			is_dup = True;
			break;
		    }
		}
		if(!is_dup)
		{
		    /* Allocate a new video mode */
		    vm = (gw_vidmode_struct *)realloc(
			vm, (j + 1) * sizeof(gw_vidmode_struct)
		    );
		    if(vm != NULL)
		    {
			vm_ptr = &vm[j];
			vm_ptr->width = m->hdisplay;
			vm_ptr->height = m->vdisplay;
			vm_ptr->vwidth = m->htotal;
			vm_ptr->vheight = m->vtotal;
			j++;
		    }
		}

		/* If private is not NULL then it needs to be
		 * deallocated.  Currently this only occures for
		 * the S3 servers.
		 */
		if((m->privsize > 0) && (m->private != NULL))
		{
		    XFree(m->private);
		    m->private = NULL;
		    m->privsize = 0;
		}

		/* Do not free each structure */
/*		XFree(m); */
	    }
	    XFree(mode_line_info);
	}
	if(n != NULL)
	    *n = t;
	return(vm);
#else
	if(n != NULL)
	    *n = 0;
	return(NULL);
#endif
}
#endif  /* X_H */

/*
 *	Deletes a list of video modes obtained from GWVidModesGet().
 */
void GWVidModesFree(gw_vidmode_struct *vidmode, int n)
{
	if(vidmode == NULL)
	    return;

	free(vidmode);
}

/*
 *	Creates a new accelerator structure, can return NULL on error.
 */
gw_accelerator_struct *GWAcceleratorNew(
	int key, int modifier
)
{
	gw_accelerator_struct *a = (gw_accelerator_struct *)calloc(
	    1, sizeof(gw_accelerator_struct)
	);
	if(a == NULL)
	    return(NULL);

	a->key = key;
	a->modifier = modifier;

	return(a);
}

/*
 *	Deallocates the given accelerator structure.
 */
void GWAcceleratorDelete(gw_accelerator_struct *a)
{
	if(a == NULL)
	    return;

	free(a);
}

/*
 *	Appends a new accelerator structure to the list. Returns the
 *	index to the new accelerator structure or -1 on error.
 */
int GWAcceleratorListAdd(
	gw_accelerator_struct ***a, int *total,
	int key, int modifier
)
{
	int n;
	gw_accelerator_struct *akey;


	if((a == NULL) || (total == NULL))
	    return(-1);

	if(*total < 0)
	    *total = 0;

	n = *total;
	*total = n + 1;
	*a = (gw_accelerator_struct **)realloc(
	    *a,
	    (*total) * sizeof(gw_accelerator_struct *)
	);
	if(*a == NULL)
	{
	    *total = 0;
	    return(-1);
	}

	(*a)[n] = akey = GWAcceleratorNew(key, modifier);
	if(akey == NULL)
	    return(-1);
	else
	    return(n);
}

/*
 *	Deallocates the list of accelerators.
 */
void GWAcceleratorListDelete(
	gw_accelerator_struct ***a, int *total
)
{
	int i;

	if((a == NULL) || (total == NULL))
	    return;

	for(i = 0; i < *total; i++)
	    GWAcceleratorDelete((*a)[i]);

	free(*a);
	*a = NULL;
	*total = 0;
}

/*
 *	Returns True if the given key and modifier match one of the
 *	accelerators in the given list.
 */
Boolean GWAcceleratorListCheck(    
	gw_accelerator_struct **a, int total,
	int key, int modifier
)
{
	int i;
	gw_accelerator_struct *a_ptr;

	if((a == NULL) || (total <= 0))
	    return(False);

	/* Iterate through all accelerator keys in the list */
	for(i = 0; i < total; i++)
	{
	    a_ptr = a[i];
	    if(a_ptr == NULL)
		continue;

	    /* Does the given accelerator key and modifier key match
	     * this accelerator key and modifier key?
	     */
	    if((a_ptr->key == key) &&
	       (a_ptr->modifier == modifier)
	    )
		return(True);
	}

	return(False);
}



#ifdef X_H
/*
 *      Creates a new pointer cursor from the given RGBA data.
 *
 *	In X all alpha values less than 0x80 is considered transparent
 *	and values above that are solid.
 */
void *GWCursorNew(
	gw_display_struct *display,
	int width, int height,	/* In pixels */
	int hot_x, int hot_y,	/* In pixels */
	int bpl,		/* Bytes per line, 0 for autocalc */
	const void *data	/* RGBA data (4 bytes per pixel) */
)
{
	Display *dpy = (display != NULL) ? display->display : NULL;
	const u_int8_t *v, *ptr8 = (const u_int8_t *)data;
	int x, y, ctx_num;
	unsigned int best_width, best_height;
	Drawable d;
	Pixmap pixmap, mask;
	Pixel black_pix, white_pix;
	GC gc;
	XGCValues gcv;
	Cursor c;
	XColor fg_c, bg_c;
	if((dpy == NULL) || (ptr8 == NULL))
	    return(NULL);

	if((width <= 0) || (height <= 0))
	    return(NULL);

	if(bpl <= 0)
	    bpl = width * 4 * sizeof(u_int8_t);

	black_pix = display->black_pix;
	white_pix = display->white_pix;

	ctx_num = display->gl_context_num;
	if((ctx_num >= 0) && (ctx_num < display->total_gl_contexts))
	    d = (Drawable)display->toplevel[ctx_num];
	else
	    d = None;
	if(d == None)
	    return(NULL);

	/* Query cursor size */
	xerrval = 0;
	if(XQueryBestCursor(
	    dpy, d, width, height, &best_width, &best_height
	))
	{
	    if(width > best_width)
	    {
		fprintf(
		    stderr,
"GWCursorNew(): Cursor width %i is too big,\
 reducing to recommend width %i.\n",
		    width, best_width
		);
		width = best_width;
	    }
	    if(height > best_height)
	    {
		fprintf(
		    stderr,
"GWCursorNew(): Cursor height %i is too big,\
 reducing to recommend height %i.\n",
		    height, best_height
		);
		height = best_height;
	    }
	}
	if(xerrval && gw_debug)
	    printf(GW_DEBUG_PREFIX
"XQueryBestCursor(): Unable to obtain best cursor size for\
 Window 0x%.8x.\n",
		(u_int32_t)d
	    );

	/* Set up foreground and background RGB colors for the
	 * cursor.
	 */
	XCOLOR_SET_RGB_COEFF(&fg_c, 1.0, 1.0, 1.0);
	fg_c.pixel = white_pix;
	XCOLOR_SET_RGB_COEFF(&bg_c, 0.0, 0.0, 0.0);
	bg_c.pixel = black_pix;

	/* Create pixmap and mask that will be used to create the
	 * cursor. Both the pixmap and mask must have a depth of 1
	 * bit per pixel.
	 */
	xerrval = 0;
	pixmap = XCreatePixmap(dpy, d, width, height, 1);
	if((pixmap == None) || xerrval)
	{
	    if(gw_debug)
		printf(GW_DEBUG_PREFIX
"XCreatePixmap(): Unable to create Pixmap of size %ix%i\
 and %i bits per pixel.\n",
		    width, height, 1
		);
	    return(NULL);
	}
	xerrval = 0;
	mask = XCreatePixmap(dpy, d, width, height, 1);
	if((mask == None) || xerrval)
	{
	    if(gw_debug)
		printf(GW_DEBUG_PREFIX
"XCreatePixmap(): Unable to create Pixmap of size %ix%i\
 and %i bits per pixel.\n",
		    width, height, 1
		);
	    FREE_PIXMAP(&pixmap);
	    return(NULL);
	}

	/* Create GC for the pixmap and mask */
	gcv.function = GXcopy;
	gcv.plane_mask = AllPlanes;
	gcv.foreground = white_pix;
	gcv.background = black_pix;
	xerrval = 0;
	gc = XCreateGC(
	    dpy, pixmap,
	    GCFunction | GCPlaneMask |
	    GCForeground | GCBackground,
	    &gcv
	);
	if((gc == None) || xerrval)
	{
	    if(gw_debug)
		printf(GW_DEBUG_PREFIX
"XCreateGC(): Unable to create GC for Pixmap 0x%.8x.\n",
		    (u_int32_t)pixmap
		);
	    FREE_PIXMAP(&pixmap);
	    FREE_BITMAP(&mask);
	    return(NULL);
	}

	/* Copy the given data to the pixmap and mask, we will draw the
	 * pixmap and mask with GC functions GXset and GXclear since
	 * everything is in black and white.
	 */
	for(y = 0; y < height; y++)
	{
	    for(x = 0; x < width; x++)
	    {
		/* Get current RGBA pixel value */
		v = &ptr8[(y * bpl) + (x * 4)];

		/* Check alpha value and draw mask pixel */
		XSetFunction(
		    dpy, gc,
		    (v[3] >= 0x80) ? GXset : GXclear
		);
		XDrawPoint(dpy, mask, gc, x, y);

		/* Check RGB value and draw pixmap pixel */
		XSetFunction(
		    dpy, gc,
		    (((v[0] + v[1] + v[2]) / 3) >= 0x80) ?
			GXset : GXclear
		);
		XDrawPoint(dpy, pixmap, gc, x, y);
	    }
	}

	/* Create cursor */
	xerrval = 0;
	c = XCreatePixmapCursor(
	    dpy, pixmap, mask,
	    &fg_c, &bg_c,
	    hot_x, hot_y
	);
	if((c == None) || xerrval)
	{
	    if(gw_debug)
		printf(GW_DEBUG_PREFIX
"XCreatePixmapCursor(): Unable to create Cursor of size %ix%i from\
 Pixmap 0x%.8x and mask 0x%.8x.\n",
		    width, height,
		    (u_int32_t)pixmap,
		    (u_int32_t)mask
		);
	}

	/* Delete pixmap, mask and the GC since they are no longer
	 * needed after the creation of the cursor.
	 */
	FREE_PIXMAP(&pixmap);
	FREE_BITMAP(&mask);
	XFreeGC(dpy, gc);

	return((c != None) ? (void *)c : NULL);
}
#endif  /* X_H */

#ifdef X_H
/*
 *      Sets the pointer cursor for the current toplevel window in
 *	context.
 *
 *	The pointer cursor must be one that has been returned by
 *	GWCursorNew().
 */
void GWCursorSet(gw_display_struct *display, void *cursor_ptr)
{
	Display *dpy = (display != NULL) ? display->display : NULL;
	Cursor c = (Cursor)((cursor_ptr != NULL) ? cursor_ptr : None);
	int ctx_num;
	Window w;
	if(dpy == NULL)
	    return;

	ctx_num = display->gl_context_num;
	if((ctx_num >= 0) && (ctx_num < display->total_gl_contexts))
	    w = display->toplevel[ctx_num];
	else
	    w = None;
	if(w == None)
	    return;

	if(c == None)
	{
	    xerrval = 0;
	    XUndefineCursor(dpy, w);
	    if(xerrval && gw_debug)
		printf(GW_DEBUG_PREFIX
"XUndefineCursor(): Unable to set default Cursor for Window 0x%.8x.\n",
		    (u_int32_t)w
		);
	}
	else
	{
	    xerrval = 0;
	    XDefineCursor(dpy, w, c);
	    if(xerrval && gw_debug)
		printf(GW_DEBUG_PREFIX
"XDefineCursor(): Unable to set Cursor 0x%.8x for Window 0x%.8x.\n",
		    (u_int32_t)c, (u_int32_t)w
		);
	}
	XFlush(dpy);
}
#endif	/* X_H */

#ifdef X_H
/*
 *	Deletes a pointer cursor created by GWCursorNew().
 */
void GWCursorDelete(gw_display_struct *display, void *cursor_ptr)
{
	Display *dpy = (display != NULL) ? display->display : NULL;
	Cursor c = (Cursor)((cursor_ptr != NULL) ? cursor_ptr : None);
	if((dpy == NULL) || (c == None))
	    return;

	XFreeCursor(dpy, c);
}
#endif  /* X_H */

#ifdef X_H
/*
 *	Sets the pointer cursor for the current toplevel window in
 *	context.
 */
void GWSetPointerCursor(
	gw_display_struct *display, gw_pointer_cursor cursor
)
{
	Display *dpy = (display != NULL) ? display->display : NULL;
	int ctx_num;
	Cursor c = None;
	Window w;
	if(dpy == NULL)
	    return;

	ctx_num = display->gl_context_num;
	if((ctx_num < 0) || (ctx_num >= display->total_gl_contexts))
	    return;

	w = display->toplevel[ctx_num];
	if(w == None)
	    return;

	switch(cursor)
	{
	  case GWPointerCursorStandard:
	    c = display->cursor_standard;
	    break;
	  case GWPointerCursorBusy:
	    c = display->cursor_busy;
	    break;
	  case GWPointerCursorText:
	    c = display->cursor_text;
	    break;
	  case GWPointerCursorTranslate:
	    c = display->cursor_translate;
	    break;
	  case GWPointerCursorZoom:
	    c = display->cursor_zoom;
	    break;
	  case GWPointerCursorInvisible:
	    c = display->cursor_invisible;
	    break;
	}

	if(c == None)
	{
	    xerrval = 0;
	    XUndefineCursor(dpy, w);
	    if(xerrval && gw_debug)
		printf(GW_DEBUG_PREFIX
"XUndefineCursor(): Unable to set default Cursor for Window 0x%.8x.\n",
		    (u_int32_t)w
		);
	}
	else
	{
	    xerrval = 0;
	    XDefineCursor(dpy, w, c);
	    if(xerrval && gw_debug)
		printf(GW_DEBUG_PREFIX
"XDefineCursor(): Unable to set Cursor 0x%.8x for Window 0x%.8x.\n",
		    (u_int32_t)c, (u_int32_t)w
		);
	}
	XFlush(dpy);
}
#endif	/* X_H */

#ifdef X_H
/*
 *	Shows the pointer cursor.
 */
void GWShowCursor(gw_display_struct *display)
{
	if(display == NULL)
	    return;

	if(display->cursor_shown)
	    return;

	GWSetPointerCursor(display, GWPointerCursorStandard);
	display->cursor_shown = True;
}
#endif  /* X_H */

#ifdef X_H
/*
 *	Hides the pointer cursor.
 */
void GWHideCursor(gw_display_struct *display)
{
	if(display == NULL)
	    return;

	if(!display->cursor_shown)
	    return;

	GWSetPointerCursor(display, GWPointerCursorInvisible);
	display->cursor_shown = False;
}
#endif  /* X_H */

#ifdef X_H
/*
 *	Returns True if the cursor is shown.
 */
Boolean GWIsCursorShown(gw_display_struct *display)
{
	return((display != NULL) ? display->cursor_shown : False);
}
#endif  /* X_H */

#ifdef X_H
/*
 *	Sets pointer cursor as busy and ignores input.
 */
void GWSetInputBusy(gw_display_struct *display)
{
	if(!GWIsCursorShown(display))
	    return;

	GWSetPointerCursor(display, GWPointerCursorBusy);
}
#endif	/* X_H */

#ifdef X_H
/*
 *	Sets pointer cursor as ready and resumes allowing input.
 */
void GWSetInputReady(gw_display_struct *display)
{
	if(!GWIsCursorShown(display))
	    return;

	GWSetPointerCursor(display, GWPointerCursorStandard);
}
#endif  /* X_H */


#ifdef X_H
/*
 *	Set up gl projection matrix for 2d drawing.
 */
void GWOrtho2D(gw_display_struct *display)
{
	int ctx_num;
	const XRectangle *rect;


	if(display == NULL)
	    return;

	/* Get current gl context */
	ctx_num = display->gl_context_num;
	if((ctx_num < 0) || (ctx_num >= display->total_gl_contexts))
	    return;

	/* Get size of current gl context */
	rect = &display->toplevel_geometry[ctx_num];

	/* Set up gl projection matrix for 2d drawing */
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(  
	    0,		/* Left, right coordinate values */
	    MAX(rect->width - 1, 1),
	    0,		/* Top, bottom coordinate values */
	    MAX(rect->height - 1, 1)
	);      
	glMatrixMode(GL_MODELVIEW);   
	glLoadIdentity();
}
#endif  /* X_H */

#ifdef X_H
/*
 *      Set up gl projection matrix for 2d drawing with specific
 *	coordinates.
 */
void GWOrtho2DCoord(
	gw_display_struct *display,
	float left, float right, float top, float bottom
)
{
	if(display == NULL)
	    return;

	if((left == right) || (top == bottom))
	    return;

	/* Set up gl projection matrix for 2d drawing */
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(
	    left, right,
	    bottom, top
	);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}
#endif  /* X_H */

/*
 *	Sets font to the foreground of the graphics context.
 */
void GWSetFont(gw_display_struct *display, GWFont *font)
{
	if(display == NULL)
	    return;

	display->current_font = font;
}

/*
 *	Gets font sizes.
 *
 *	If an input is NULL, then its value will not be set.
 *
 *	Returns 0 on success or non-zero on error.
 */
int GWGetFontSize(
	GWFont *font, 
	int *width, int *height,  
	int *character_spacing, int *line_spacing  
)
{
	if(font == NULL)
	    return(-1);

	/* Format, header is first 32 bytes:
	 *
	 * Address       Desc
	 * 0             Width of each font
	 * 1             Height of each font
	 * 2             Character width spacing
	 * 3             Line spacing
	 * 4             Bytes per line
	 */

	if(width != NULL)
	    *width = (int)((u_int8_t)font[0]);

	if(height != NULL)
	    *height = (int)((u_int8_t)font[1]);

	if(character_spacing != NULL)
	    *character_spacing = (int)((u_int8_t)font[2]);

	if(line_spacing != NULL)
	    *line_spacing = (int)((u_int8_t)font[3]);

	return(0);
}

/*
 *	Draws string.
 *
 *      GWOrtho2D() must be called to transform orientations before 
 *      using this function.
 */
#ifdef USE_OLD_GWDRAWSTRING
void GWDrawString(
	gw_display_struct *display,
	int x, int y,
	const char *string
)
{
	u_int8_t *font_header, *font_data;

	int font_width, font_height;		/* In pixels */
	int width_spacing, line_spacing;	/* In pixels */
	int bytes_per_line;	/* Bytes per line (per width) */
	int bytes_per_char_len;	/* Bytes per character */

	int win_height;


	if((display == NULL) || (string == NULL))
	    return;

	win_height = display->height;

	/* Get pointer to font header data */
	font_header = display->current_font;
	if(font_header == NULL)
	    return;

	/* Seek past header and get pointer to start of font data */
	font_data = font_header + 32;

	/* Format, header is first 32 bytes:
	 *
	 * Address       Desc
	 * 0             Width of each font
	 * 1             Height of each font
	 * 2             Character width spacing
	 * 3             Line spacing
	 * 4             Bytes per line
	 */

	font_width = font_header[0];
	font_height = font_header[1];
	width_spacing = font_header[2];
	line_spacing = font_header[3];
	bytes_per_line = font_header[4];

	/* Calculate bytes per character */
	bytes_per_char_len = bytes_per_line * font_height;

	/* Convert y position */
	y = win_height - y - font_height;

	while(*string != '\0')
	{
	    glRasterPos2i(x, y);
	    glBitmap(
		font_width, font_height,
		0.0, 0.0,
		width_spacing, 0.0,
		&font_data[
		    bytes_per_char_len * (char)(*string)
		]
	    );

	    x += width_spacing;
	    string++;
	}
}
#else	/* USE_OLD_GWDRAWSTRING */
void GWDrawString(
	gw_display_struct *display,
	int x, int y,
	const char *string
)
{
	u_int8_t *font_header, *font_data;
	int width, height;
	int font_width, font_height;            /* In pixels */
	int width_spacing, line_spacing;        /* In pixels */
	int bytes_per_line;     /* Bytes per line (per width) */
	int bytes_per_char;     /* Bytes per character */
	int char_offset;        /* Current char's offset in the font data */

	int chars_hidden = 0;	/* # of chars entirely outside viewport */
	float xorig = 0.0f;
	float yorig = 0.0f;


	if((display == NULL) || (string == NULL))
	    return;

	GWContextGet(
	    display, display->gl_context_num,
	    NULL, NULL,
	    NULL, NULL,
	    &width, &height
	);

	/* Get pointer to font header data */
	font_header = display->current_font;
	if(font_header == NULL)
	    return;

	/* Seek past header and get pointer to start of font data */
	font_data = font_header + 32;

	/* Format, header is first 32 bytes:
	 *
	 * Address       Desc
	 * 0             Width of each font
	 * 1             Height of each font
	 * 2             Character width spacing
	 * 3             Line spacing
	 * 4             Bytes per line   
	 */     

	font_width = font_header[0];
	font_height = font_header[1];
	width_spacing = font_header[2];
	line_spacing = font_header[3];
	bytes_per_line = font_header[4];

	/* Flip y values */
	y = height - line_spacing - y + 1;

	/* Calculate bytes per character */
	bytes_per_char = bytes_per_line * font_height;

	/* Adjust string index in case x < 0 */
	if(x < 0)
	{
	    chars_hidden = (-x) / width_spacing;
	    xorig = (float)((-x) - (chars_hidden * width_spacing));

	    if(chars_hidden >= STRLEN(string))
		return;

	    x = 0;
	    string += chars_hidden;
	} 
	if(y < 0)
	{
	    if(y < -line_spacing)
		return;

	    yorig = (float)-y;
	    y = 0;
	}

	glRasterPos2i(x, y);
	for(; *string; string++)
	{
	    char_offset = bytes_per_char * (*string);

	    glBitmap(
		font_width, font_height,
		(GLfloat)xorig, (GLfloat)yorig,
		(GLfloat)width_spacing, 0.0,
		font_data + char_offset
	    );
	}
}
#endif	/* USE_OLD_GWDRAWSTRING */

/*
 *	Same as GWDrawString() except that it draws just the specified
 *	character.
 */
void GWDrawCharacter(
	gw_display_struct *display,
	int x, int y,   
	char c
)
{
	u_int8_t *font_header, *font_data;
	int width, height;
	int font_width, font_height;            /* In pixels */
	int width_spacing, line_spacing;        /* In pixels */
	int bytes_per_line;     /* Bytes per line (per width) */
	int bytes_per_char_len; /* Bytes per character */


	if((display == NULL) || (c == '\0'))
	    return;

	GWContextGet(
	    display, display->gl_context_num,
	    NULL, NULL,
	    NULL, NULL,
	    &width, &height
	);
	    
	/* Get pointer to font header data */
	font_header = display->current_font;
	if(font_header == NULL)
	    return;
		
	/* Seek past header and get pointer to start of font data */
	font_data = font_header + 32;

	/* Format, header is first 32 bytes:
	 *
	 * Address       Desc
	 * 0             Width of each font
	 * 1             Height of each font
	 * 2             Character width spacing
	 * 3             Line spacing
	 * 4             Bytes per line
	 */

	font_width = font_header[0];
	font_height = font_header[1];
	width_spacing = font_header[2];
	line_spacing = font_header[3];
	bytes_per_line = font_header[4];

	/* Calculate bytes per character */
	bytes_per_char_len = bytes_per_line * font_height;

	/* Convert y position */
	y = height - y - font_height;

	glRasterPos2i(x, y);
	glBitmap(
	    font_width, font_height,
	    0.0, 0.0,
	    (GLfloat)width_spacing, 0.0,
	    &font_data[
		bytes_per_char_len * c
	    ]
	);
}


/*
 *	Reads the header from the given image data.
 *
 *	Returns non-zero on error.
 */
int GWImageLoadHeaderFromData(
	gw_display_struct *display,
	int *width, int *height, int *bpl, int *bpp,
	const void *data
)
{
	const u_int8_t *src8;

	if((display == NULL) || (data == NULL))
	{
	    if(width != NULL) *width = 0;
	    if(height != NULL) *height = 0;
	    if(bpl != NULL) *bpl = 0;
	    if(bpp != NULL) *bpp = 0;
	    return(-1);
	}

	/* First 32 bytes are header, format:
	 *
	 * <width32> <height32> <bpl32> <bpp32>
	 */
	src8 = (const u_int8_t *)data + (0 * sizeof(u_int8_t));

	if(width != NULL)
	    *width = *(u_int32_t *)src8;
	src8 += sizeof(u_int32_t);

	if(height != NULL)
	    *height = *(u_int32_t *)src8;
	src8 += sizeof(u_int32_t);

	if(bpl != NULL)
	    *bpl = *(u_int32_t *)src8;
	src8 += sizeof(u_int32_t);

	if(bpp != NULL)
	    *bpp = *(u_int32_t *)src8;

	return(0);
}

/*
 *      Loads image RGB (3 bytes per pixel) image data.
 */
u_int8_t *GWImageLoadFromDataRGB(
	gw_display_struct *display,
	int *width, int *height, int *bpl,
	const void *data
)
{
	int w, h, lbpl, tar8_len;
	const u_int8_t *src8;
	u_int8_t *tar8;


	/* Get image data */
	src8 = GWImageLoadFromDataSharedRGB(
	    display, &w, &h, &lbpl, data
	);
	if(src8 == NULL)
	{
	    if(width != NULL) *width = 0;
	    if(height != NULL) *height = 0;
	    if(bpl != NULL) *bpl = 0;
	    return(NULL);
	}

	/* Image size too small? */
	if((w <= 0) || (h <= 0) || (lbpl <= 0))
	{
	    if(width != NULL) *width = 0;
	    if(height != NULL) *height = 0;
	    if(bpl != NULL) *bpl = 0;
	    return(NULL);
	}

	/* Update returns */
	if(width != NULL) *width = w;
	if(height != NULL) *height = h;
	if(bpl != NULL) *bpl = lbpl;

	/* Calculate size of image data in bytes */
	tar8_len = h * lbpl;

	/* Allocate target image data */
	tar8 = (u_int8_t *)malloc(tar8_len);
	if(tar8 == NULL)
	    return(NULL);

	/* Copy source image data to target image data */
	memcpy(tar8, src8, tar8_len);

	return(tar8);
}

/*
 *      Loads image RGB (3 bytes per pixel) image data.
 *
 *      The returned pointer must not be deallocated since it points to
 *      a position within the given data.
 */
const u_int8_t *GWImageLoadFromDataSharedRGB(
	gw_display_struct *display,
	int *width, int *height, int *bpl,
	const void *data
)
{
	int w, h, lbpl, lbpp;

	/* Read header */
	if(GWImageLoadHeaderFromData(
	    display, &w, &h, &lbpl, &lbpp, data
	))
	{
	    if(width != NULL) *width = 0;
	    if(height != NULL) *height = 0;
	    if(bpl != NULL) *bpl = 0;
	    return(NULL);
	}

	/* Bytes per pixel does not match? */
	if(lbpp != 3)
	{
	    fprintf(
		stderr,
"GWImageLoadFromDataSharedRGB(): Warning: Bytes per pixel %i is not 3.\n",
		lbpp
	    );
	    if(width != NULL) *width = 0;
	    if(height != NULL) *height = 0;
	    if(bpl != NULL) *bpl = 0;
	    return(NULL);
	}

	/* Need to calculate bytes per line? */
	if(lbpl <= 0)
	    lbpl = w * lbpp;

	/* Update returns */
	if(width != NULL) *width = w;
	if(height != NULL) *height = h;
	if(bpl != NULL) *bpl = lbpl;

	/* Return image data */
	return((const u_int8_t *)data + (32 * sizeof(u_int8_t)));
}

/*
 *      Loads image RGBA (4 bytes per pixel) image data.
 */
u_int8_t *GWImageLoadFromDataRGBA(
	gw_display_struct *display,
	int *width, int *height, int *bpl,
	const void *data
)
{
	int w, h, lbpl, tar8_len;
	const u_int8_t *src8;
	u_int8_t *tar8;


	/* Get image data */
	src8 = GWImageLoadFromDataSharedRGBA(
	    display, &w, &h, &lbpl, data
	);
	if(src8 == NULL)
	{
	    if(width != NULL) *width = 0;
	    if(height != NULL) *height = 0;
	    if(bpl != NULL) *bpl = 0;
	    return(NULL);
	}

	/* Image size too small? */
	if((w <= 0) || (h <= 0) || (lbpl <= 0))
	{
	    if(width != NULL) *width = 0;
	    if(height != NULL) *height = 0;
	    if(bpl != NULL) *bpl = 0;
	    return(NULL);
	}

	/* Update returns */
	if(width != NULL) *width = w;
	if(height != NULL) *height = h;
	if(bpl != NULL) *bpl = lbpl;

	/* Calculate size of image data in bytes */
	tar8_len = h * lbpl;

	/* Allocate target image data */
	tar8 = (u_int8_t *)malloc(tar8_len);
	if(tar8 == NULL)
	    return(NULL);

	/* Copy source image data to target image data */
	memcpy(tar8, src8, tar8_len);

	return(tar8);
}

/*
 *	Loads image RGBA (4 bytes per pixel) image data.
 *
 *	The returned pointer must not be deallocated since it points to
 *	a position within the given data.
 */
const u_int8_t *GWImageLoadFromDataSharedRGBA(
	gw_display_struct *display,
	int *width, int *height, int *bpl,
	const void *data
)
{
	int w, h, lbpl, lbpp;

	/* Read header */
	if(GWImageLoadHeaderFromData(
	    display, &w, &h, &lbpl, &lbpp, data
	))
	{
	    if(width != NULL) *width = 0;
	    if(height != NULL) *height = 0;
	    if(bpl != NULL) *bpl = 0;
	    return(NULL);
	}

	/* Bytes per pixel does not match? */
	if(lbpp != 4)
	{
	    fprintf(
		stderr,
"GWImageLoadFromDataSharedRGBA(): Warning: Bytes per pixel %i is not 4.\n",
		lbpp
	    );
	    if(width != NULL) *width = 0;
	    if(height != NULL) *height = 0;
	    if(bpl != NULL) *bpl = 0;
	    return(NULL);
	}

	/* Need to calculate bytes per line? */
	if(lbpl <= 0)
	    lbpl = w * lbpp;

	/* Update returns */
	if(width != NULL) *width = w;
	if(height != NULL) *height = h;
	if(bpl != NULL) *bpl = lbpl;

	/* Return image data */
	return((const u_int8_t *)data + (32 * sizeof(u_int8_t)));
}


/*
 *	Called by GWImageDrawFromData*() to draw the image.
 *
 *	img_format may be GL_RGB or GL_RGBA.
 */
static void GWImageDrawFromDataNexus(
	gw_display_struct *display, const u_int8_t *img_data,
	int x, int y, int width, int height,
	int img_width, int img_height,
	GLenum img_format
)
{
	StateGLBoolean alpha_test;
	GLint x2, y2;
	GLfloat x_zoom_factor, y_zoom_factor;
	int win_width, win_height;

	GWContextGet(
	    display, GWContextCurrent(display),
	    NULL, NULL,
	    NULL, NULL,
	    &win_width, &win_height
	);

	/* If no requested image size set, then use actual image size */
	if(width < 1)
	    width = img_width;
	if(height < 1)
	    height = img_height;

	/* Calculate GL converted coordinate positions */
	x2 = (GLint)MAX(x, 0);
	y2 = (GLint)MAX((win_height - y - 1), 0);

	/* Calculate zoom factor based on the requested image size
	 * and the actual image data size.
	 */
	x_zoom_factor = (GLfloat)width / (GLfloat)img_width;
	y_zoom_factor = -(GLfloat)height / (GLfloat)img_height;

	/* Set image drawing coordinates */
	glRasterPos2i(
	    CLIP(x2, 0, win_width - 1),
	    CLIP(y2, 0, win_height - 1)
	);

	/* Set zoom factor of image */
	glPixelZoom(x_zoom_factor, y_zoom_factor);

	/* Enable GL alpha testing if the image data has an alpha
	 * channel.
	 */
	alpha_test = display->state_gl.alpha_test;
	if(img_format == GL_RGBA)
	{
	    StateGLEnable(&display->state_gl, GL_ALPHA_TEST);
	    StateGLAlphaFunc(&display->state_gl, GL_GREATER, 0.5f);
	}

	/* Draw the image */
	glDrawPixels(
	    img_width, img_height,
	    img_format, GL_UNSIGNED_BYTE,
	    (const GLvoid *)img_data
	);

	/* Restore GL states */
	if(!alpha_test)
	    StateGLDisable(&display->state_gl, GL_ALPHA_TEST);

	/* Restore zoom */
	glPixelZoom(1.0f, 1.0f);
}


/*
 *	Draws the image specified by data, which must be in RGB format.
 */
void GWImageDrawFromDataRGB(
	gw_display_struct *display, const void *data,
	int x, int y, int width, int height
)
{
	int img_width, img_height, bpl;
	const u_int8_t *img_data = GWImageLoadFromDataSharedRGB(
	    display, &img_width, &img_height, &bpl,
	    data
	);
	if(img_data == NULL)
	    return;

	GWImageDrawFromDataNexus(
	    display, img_data,
	    x, y, width, height,
	    img_width, img_height,
	    GL_RGB
	);
}

/*
 *      Draws the image specified by data, which must be in RGBA format.
 */
void GWImageDrawFromDataRGBA(
	gw_display_struct *display, const void *data,
	int x, int y, int width, int height
)
{
	int img_width, img_height, bpl;
	const u_int8_t *img_data = GWImageLoadFromDataSharedRGBA(
	    display, &img_width, &img_height, &bpl,
	    data
	);
	if(img_data == NULL)
	    return;

	GWImageDrawFromDataNexus(
	    display, img_data,
	    x, y, width, height,
	    img_width, img_height,
	    GL_RGBA
	);
}

