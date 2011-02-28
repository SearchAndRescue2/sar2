#ifdef __MSW__

#ifndef STRICT
# define STRICT
#endif

#ifndef WIN32_LEAN_AND_MEAN
# define WIN32_LEAN_AND_MEAN
#endif

#include <stdio.h>
#include <stdlib.h>

#include <windows.h>

#include <GL\gl.h>
#include <GL\glu.h>
//#include <GL\glaux.h>		// GLAux Library

#include "gw.h"
#include "stategl.h"
#include "resource.h"

unsigned long GWWGetCurMilliTime(void);
int GWStrCaseCmp(const char *s1, const char *s2);
Boolean GWParseGeometry(
	const char *geometry,
	int *x, int *y, int *width, int *height
);
int GWGetKeyValueFromWParam(gw_display_struct *display, WPARAM wParam);
void GWSetWindowIconFile(
	gw_display_struct *dpy, int ctx_num,
	const char *icon_path, const char *icon_name
);
int GWCreateWindow(
	gw_display_struct *dpy,
	void *parent,
	void **hwnd_rtn,
	void **dc_rtn,
	void **rc_rtn,
	int x, int y,
	int width, int height,
	int depth,		/* In bits. */
	const char *title
);

gw_display_struct *GWInit(int argc, char **argv);
LRESULT CALLBACK WndProc(
	HWND hWnd,      // Handle For This Window 
	UINT uMsg,      // Message For This Window
	WPARAM wParam,  // Additional Message Information
	LPARAM lParam   // Additional Message Information
);
void GWManage(gw_display_struct *display);
void GWShutdown(gw_display_struct *display);

void GWFlush(gw_display_struct *display);
void GWSync(gw_display_struct *display);
int GWEventsPending(gw_display_struct *display);

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
	gw_display_struct *display,
	const char *message
);

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
	void (*func)(int, void *, int, gw_event_type, int, unsigned long),
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

void GWPostRedraw(gw_display_struct *display);
void GWSwapBuffer(gw_display_struct *display);

void GWKeyboardAutoRepeat(gw_display_struct *display, Boolean b);

gw_vidmode_struct *GWVidModesGet(
	gw_display_struct *display, int *n
);

void *GWCursorNew(
	gw_display_struct *display,
	int width, int height,  /* In pixels. */
	int hot_x, int hot_y,   /* In pixels. */
	int bpl,                /* Bytes per line, 0 for autocalc. */
	const void *data        /* RGBA data (4 bytes per pixel). */
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

void GWOrtho2D(gw_display_struct *display);


#define ATOI(s)         (((s) != NULL) ? atoi(s) : 0)
#define ATOL(s)         (((s) != NULL) ? atol(s) : 0)
#define ATOF(s)         (((s) != NULL) ? (float)atof(s) : 0.0f)
#define STRDUP(s)       (((s) != NULL) ? strdup(s) : NULL)

#define MAX(a,b)        (((a) > (b)) ? (a) : (b))
#define MIN(a,b)        (((a) < (b)) ? (a) : (b))
#define CLIP(a,l,h)     (MIN(MAX((a),(l)),(h)))
#define IS_STRING_EMPTY(s)      (((s) != NULL) ? ((s) == '\0') : True)
#define STRCASECMP(a,b)	(GWStrCaseCmp((a),(b)))

#define RADTODEG(r)     ((r) * 180 / PI)
#define DEGTORAD(d)     ((d) * PI / 180)


/* Toplevel window style */
#if 1
// (WS_THICKFRAME | WS_POPUP | WS_CAPTION | WS_SYSMENU)
// (WS_EX_NODRAG | WS_EX_CAPTIONOKBTN | WS_EX_WINDOWEDGE)
# define GWW_TOPLEVEL_WINDOW_STYLE	(WS_OVERLAPPEDWINDOW)
# define GWW_TOPLEVEL_WINDOW_EXSTYLE	(WS_EX_APPWINDOW | WS_EX_WINDOWEDGE)
#else
# define GWW_TOPLEVEL_WINDOW_STYLE	(WS_POPUP)
# define GWW_TOPLEVEL_WINDOW_EXSTYLE	(WS_EX_APPWINDOW)
#endif


/*
 *	Windows key value to GW key value lookup table.
 *
 *	Format for each key contains 3 members as follows:
 *
 *	Windows_value, key_val_lower, key_val_upper
 *
 *	Last 3 members are always 0's.
 */
#define WINDOWS_TO_GW_KEYTABLE				\
{							\
	VK_MENU,	GWKeyAlt,	GWKeyAlt,	\
	VK_LMENU,	GWKeyAlt,	GWKeyAlt,	\
	VK_RMENU,	GWKeyAlt,	GWKeyAlt,	\
	VK_CONTROL,	GWKeyCtrl,	GWKeyCtrl,	\
	VK_LCONTROL,	GWKeyCtrl,	GWKeyCtrl,	\
	VK_RCONTROL,	GWKeyCtrl,	GWKeyCtrl,	\
	VK_SHIFT,	GWKeyShift,	GWKeyShift,	\
	VK_LSHIFT,	GWKeyShift,	GWKeyShift,	\
	VK_RSHIFT,	GWKeyShift,	GWKeyShift,	\
							\
	VK_NUMPAD0,	GWKeyInsert,	GWKeyInsert,	\
	VK_NUMPAD1,	GWKeyEnd,	GWKeyEnd,	\
	VK_NUMPAD2,	GWKeyDown,	GWKeyDown,	\
	VK_NUMPAD3,	GWKeyPageDown,	GWKeyPageDown,	\
	VK_NUMPAD4,	GWKeyLeft,	GWKeyLeft,	\
	VK_NUMPAD5,	GWKeyCenter,	GWKeyCenter,	\
	VK_NUMPAD6,	GWKeyRight,	GWKeyRight,	\
	VK_NUMPAD7,	GWKeyHome,	GWKeyHome,	\
	VK_NUMPAD8,	GWKeyUp,	GWKeyUp,	\
	VK_NUMPAD9,	GWKeyPageUp,	GWKeyPageUp,	\
	VK_MULTIPLY,	'*',	'*',	\
	VK_ADD,		'+',	'+',	\
	VK_SEPARATOR,	'=',	'=',	\
	VK_SUBTRACT,	'-',	'-',	\
	VK_DECIMAL,	'.',	'.',	\
	VK_DIVIDE,	'/',	'/',	\
					\
	VK_F1,		GWKeyF1,	GWKeyF1,	\
	VK_F2,		GWKeyF2,	GWKeyF2,	\
	VK_F3,		GWKeyF3,	GWKeyF3,	\
	VK_F4,		GWKeyF4,	GWKeyF4,	\
	VK_F5,		GWKeyF5,	GWKeyF5,	\
	VK_F6,		GWKeyF6,	GWKeyF6,	\
	VK_F7,		GWKeyF7,	GWKeyF7,	\
	VK_F8,		GWKeyF8,	GWKeyF8,	\
	VK_F9,		GWKeyF9,	GWKeyF9,	\
	VK_F10,		GWKeyF10,	GWKeyF10,	\
	VK_F11,		GWKeyF11,	GWKeyF11,	\
	VK_F12,		GWKeyF12,	GWKeyF12,	\
	VK_F13,		GWKeyF13,	GWKeyF13,	\
	VK_F14,		GWKeyF14,	GWKeyF14,	\
	VK_F15,		GWKeyF15,	GWKeyF15,	\
	VK_F16,		GWKeyF16,	GWKeyF16,	\
	VK_F17,		GWKeyF17,	GWKeyF17,	\
	VK_F18,		GWKeyF18,	GWKeyF18,	\
	VK_F19,		GWKeyF19,	GWKeyF19,	\
	VK_F20,		GWKeyF20,	GWKeyF20,	\
							\
	VK_SPACE,	' ',		' ',		\
	VK_ESCAPE,	0x1b,		0x1b,		\
	VK_UP,		GWKeyUp,	GWKeyUp,	\
	VK_DOWN,	GWKeyDown,	GWKeyDown,	\
	VK_LEFT,	GWKeyLeft,	GWKeyLeft,	\
	VK_RIGHT,	GWKeyRight,	GWKeyRight,	\
/*	VK_CENTER,	GWKeyCenter,	GWKeyCenter, */	\
	VK_HOME,	GWKeyHome,	GWKeyHome,	\
	VK_END,		GWKeyEnd,	GWKeyEnd,	\
	VK_PRIOR,	GWKeyPageUp,	GWKeyPageUp,	\
	VK_NEXT,	GWKeyPageDown,	GWKeyPageDown,	\
							\
	VK_TAB,		'\t',		'\t',		\
	VK_RETURN,	'\n',		'\n',		\
	VK_BACK,	GWKeyBackSpace,	GWKeyBackSpace,	\
	VK_DELETE,	GWKeyDelete,	GWKeyDelete,	\
	VK_INSERT,	GWKeyInsert,	GWKeyInsert,	\
							\
	VK_PAUSE,	GWKeyPause,	GWKeyPause,	\
	VK_SCROLL,	GWKeyScrollLock,GWKeyScrollLock,\
	VK_PRINT,	GWKeySysReq,	GWKeySysReq,	\
							\
	/* Numeric & punctuations. */	\
	48,	'0',	')',	\
	49,	'1',	'!',	\
	50,	'2',	'@',	\
	51,	'3',	'#',	\
	52,	'4',	'$',	\
	53,	'5',	'%',	\
	54,	'6',	'^',	\
	55,	'7',	'&',	\
	56,	'8',	'*',	\
	57,	'9',	'(',	\
	/* Punctuations set 1. */	\
	96,	'`',	'~',	\
	189,	'-',	'_',	\
	187,	'=',	'+',	\
	220,	'\\',	'|',	\
	91,	'[',	'{',	\
	93,	']',	'}',	\
	59,	';',	':',	\
	39,	'\'',	'"',	\
	188,	',',	'<',	\
	190,	'.',	'>',	\
	47,	'/',	'?',	\
	/* Punctuations set 2. */	\
	192,	'`',	'~',	\
	45,	'-',	'_',	\
	61,	'=',	'+',	\
	92,	'\\',	'|',	\
	219,	'[',	'{',	\
	221,	']',	'}',	\
	186,	';',	':',	\
	222,	'\'',	'"',	\
	44,	',',	'<',	\
	46,	'.',	'>',	\
	191,	'/',	'?',	\
				\
	0, 0, 0	/* Windows key value of 0 marks end of table. */ \
}



/*
 *	Returns the current time in milliseconds, used for time
 *	stamps on event message returns.
 */
unsigned long GWWGetCurMilliTime(void)
{
	SYSTEMTIME t;
	GetSystemTime(&t);
	return(
	    (unsigned long)(
		(((((t.wHour * 60.0) + t.wMinute) * 60.0) + t.wSecond) * 1000.0) +
		t.wMilliseconds
	    )
	);
}

/*
 *	Simplified form of the ANSI C strcasecmp().
 */
int GWStrCaseCmp(const char *s1, const char *s2)
{
	if((s1 == NULL) || (s2 == NULL))
	    return(1);	/* False. */

	while((*s1 != '\0') && (*s2 != '\0'))
	{
	    if(toupper(*s1) != toupper(*s2))
		return(1);	/* False. */

	    s1++;
	    s2++;
	}
	if(*s1 == *s2)
	    return(0);	/* True. */
	else
	    return(1);	/* False. */
}

/*
 *	Parses the given geometry string which is of the format
 *	"WxH+X+Y" (variables are uppercase). Alternate geometry
 *	string format "WxH" is also acceptable and the returns for
 *	x and y will be set to 0 and 0 (respectivly) in such case.
 *
 *	Returns True on success and False on error.
 */
Boolean GWParseGeometry(
	const char *geometry,
	int *x, int *y, int *width, int *height
)
{
	const char *s = geometry;

	if(x != NULL)
	    *x = 0;
	if(y != NULL)
	    *y = 0;
	if(width != NULL)
	    *width = 0;
	if(height != NULL)
	    *height = 0;

	if(s == NULL)
	    return(False);

	/* Get width. */
	if(width != NULL)
	    *width = atoi(s);

	/* Seek to dimensions deliminator 'x'. */
	s = strchr(s, 'x');
	if(s == NULL)
	{
	    /* Must have complete dimensions, so this would be an
	     * error.
	     */
	    return(False);
	}
	else
	{
	    /* Seek past deliminator. */
	    s += 1;
	}

	/* Get height. */
	if(height != NULL)
	    *height = atoi(s);

	/* Now check if coordinates are given. */
	while((*s != '\0') && (*s != '+') && (*s != '-'))
	    s++;
	if(*s == '\0')
	{
	    /* No coordinates in the given geometry string,
	     * return success though.
	     */
	    return(True);
	}
	else
	{
	    /* Seek past deliminator (but not if its '-'). */
	    if(*s == '+')
		s += 1;
	}

	/* Get x coordinate. */
	if(x != NULL)
	    *x = atoi(s);

	/* Seek to next coordinate. */
	s += 1;
	while((*s != '\0') && (*s != '+') && (*s != '-'))
	    s++;
	if(*s == '\0')
	{
	    /* Second coordinate not given, this is an error. */
	    return(False);
	}
	else
	{
	    /* Seek past deliminator (but not if its '-'). */
	    if(*s == '+')
		s += 1;
	}

	/* Get y coordinate. */
	if(y != NULL)
	    *y = atoi(s);

	return(True);
}

/*
 *	Returns a GW key value from the given wParam.
 */
int GWGetKeyValueFromWParam(gw_display_struct *display, WPARAM wParam)
{
	const unsigned int keytable[] = WINDOWS_TO_GW_KEYTABLE;
	const unsigned int *kt_ptr = &(keytable[0]);


	if(display == NULL)
	    return('\0');

	while(*kt_ptr != 0)
	{
	    if(*kt_ptr++ == wParam)
		return((int)((display->shift_key_state) ?
		    kt_ptr[1] : kt_ptr[0]
		));

	    kt_ptr += 2;
	}

	/* No match, return as character literal (make sure lower case). */
	return(tolower((int)wParam));
}


/*
 *	Loads an icon for the toplevel window (does nothing for now).
 */
void GWSetWindowIconFile(
	gw_display_struct *dpy, int ctx_num,
	const char *icon_path, const char *icon_name
)
{
	return;
}


/*
 *	Creates a new toplevel window and sets it up for OpenGL rendering.
 *
 *	All inputs must be valid unless otherwise indicated.
 *
 *	Returns 0 on success or -1 on error.
 */
int GWCreateWindow(
	gw_display_struct *dpy,
	void *parent,		/* Can be NULL to indicate root. */
	void **hwnd_rtn,
	void **dc_rtn,
	void **rc_rtn,
	int x, int y,
	int width, int height,
	int depth,		/* In bits. */
	const char *title
)
{
	void *w, *dc, *rc;	/* Window, device context, and rendering context. */
	HWND hWnd;
	HINSTANCE hInst;
	GLuint PixelFormat;	// Holds The Results After Searching For A Match
	WNDCLASSEX wc;		// Windows Class Structure
	DWORD dwStyle = GWW_TOPLEVEL_WINDOW_STYLE;
	DWORD dwExStyle = GWW_TOPLEVEL_WINDOW_EXSTYLE;
	RECT ws;		// Grabs Rectangle Upper Left / Lower Right Values


	if(hwnd_rtn != NULL)
	    *hwnd_rtn = NULL;
	if(dc_rtn != NULL)
	    *dc_rtn = NULL;
	if(rc_rtn != NULL)
	    *rc_rtn = NULL;

	if((hwnd_rtn == NULL) || (dc_rtn == NULL) || (rc_rtn == NULL))
	    return(-1);

	hInst = (HINSTANCE)dpy->pid;

	/* Set window size (left and top are not positions). */
	ws.left = (long)0;
	ws.right = (long)width;
	ws.top = (long)0;
	ws.bottom = (long)height;

	/* Begin setting up window class. */
	/* Size of this structure. */
	wc.cbSize = sizeof(WNDCLASSEX);

	/* Class style, redraw on resize and own dc for the window. */
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;

	/* Set WndProc() as callback for message (event) handling. */
	wc.lpfnWndProc = (WNDPROC)WndProc;

	/* No extra window class data. */
	wc.cbClsExtra = 0;

	/* Extra window data for client data. */
	wc.cbWndExtra = sizeof(void *);

	/* Set the instance. */
	wc.hInstance = hInst;

	/* ALT + TAB icon. */
	wc.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(GWW_ICON_ALT_TAB));
/*	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION); */

	/* Load arrow pointer. */
/*	wc.hCursor = LoadCursor(NULL, IDC_ARROW); */
	wc.hCursor = NULL;	/* We set our own cursor later */

	/* No background. */
	wc.hbrBackground = NULL;

	/* No menu. */
	wc.lpszMenuName = NULL;

	/* Class name. */
	wc.lpszClassName = "OpenGL";

	/* Title bar icon. */
	wc.hIconSm = LoadIcon(hInst, MAKEINTRESOURCE(GWW_ICON_TITLE_BAR));


	/* Try to register the window class. */
	if(!RegisterClassEx(&wc))
	{
	    MessageBox(
		NULL,
		"Failed To Register The Window Class.",
		"ERROR",
		MB_OK | MB_ICONEXCLAMATION
	    );
	    return(-1);
	}

	/* Adjust window size to take into account its
	 * style's decorations.
	 */
	AdjustWindowRectEx(&ws, dwStyle, FALSE, dwExStyle);

	/* Create the window. */
	hWnd = CreateWindowEx(
	    dwExStyle,		// Extended Style For The Window
	    "OpenGL",		// Class Name
	    title,		// Window Title
	    dwStyle |		// Defined Window Style
	    WS_CLIPSIBLINGS |	// Required Window Style
	    WS_CLIPCHILDREN,	// Required Window Style
	    x, y,		// Window Position
	    ws.right - ws.left,	// Calculate Window Width
	    ws.bottom - ws.top,	// Calculate Window Height
	    NULL,		// No Parent Window
	    NULL,		// No Menu
	    (HINSTANCE)dpy->pid,	// Instance
	    dpy			/* Client data. */
	);
	w = hWnd;
	if(hWnd == NULL)
	{
	    MessageBox(
		NULL,
		"Window Creation Error.",
		"ERROR",
		MB_OK | MB_ICONEXCLAMATION
	    );
	    return(-1);
	}
	if(hwnd_rtn != NULL)
	    *hwnd_rtn = w;


	/* Set up pixel format descriptor, this defines the per
	 * pixel specifications.
	 */
	static PIXELFORMATDESCRIPTOR pfd =
	{
	    sizeof(PIXELFORMATDESCRIPTOR),	/* Size of this structure. */
	    1,				/* Version number. */
	    PFD_DRAW_TO_WINDOW | 	/* Format must support window. */
	    PFD_SUPPORT_OPENGL |	/* Format must support OpenGL. */
	    PFD_DOUBLEBUFFER,		/* Must support double buffer. */
	    PFD_TYPE_RGBA,		/* Request RGBA format. */
	    ((depth > 16) ? 24 : depth),	/* Color depth. */
	    0, 0, 0, 0, 0, 0,		/* Ignore colors bits. */
	    8,				/* Alpha bits. */
	    0,				/* Alpha shift. */
	    0,				/* Accumulation buffer bits. */
	    0, 0, 0, 0,			/* Accumulation bits (RGBA). */
	    16,				/* Z depth buffer bits. */
	    8,				/* Stencil buffer bits. */
	    0,				/* Auxiliary buffer bits. */
	    PFD_MAIN_PLANE,		/* Main drawing layer. */
	    0,				/* Reserved. */
	    0, 0, 0			/* Layer masks ignored. */
	};

	/* Create the device context. */
	dc = GetDC(hWnd);
	if(dc == NULL)
	{
	    MessageBox(
		hWnd,
		"Can't Create A GL Device Context.",
		"ERROR",
		MB_OK | MB_ICONEXCLAMATION
	    );
	    return(-1);
	}
	if(dc_rtn != NULL)
	    *dc_rtn = dc;


	/* Select pixel format descriptor. */
	PixelFormat = ChoosePixelFormat((HDC)dc, &pfd);
	if(PixelFormat)
	{
	    dpy->has_double_buffer = True;
	    dpy->alpha_channel_bits = 8;
	}
	else
	{
	    dpy->has_double_buffer = False;
	    dpy->alpha_channel_bits = 0;
	    MessageBox(
		hWnd,
		"Cannot Find A Suitable PixelFormat.",
		"ERROR",
		MB_OK | MB_ICONEXCLAMATION
	    );
	    return(-1);
	}
/* Need to try other pixel formats? */

	/* Able to set pixel format? */
	if(!SetPixelFormat((HDC)dc, PixelFormat, &pfd))
	{
	    MessageBox(
		hWnd,
		"SetPixelFormat(): Unable to set PixelFormat",
		"ERROR",
		MB_OK | MB_ICONEXCLAMATION
	    );
	    return(-1);
	}

	/* Create rendering context from the hardware device context. */
	rc = wglCreateContext((HDC)dc);
	if(rc == NULL)
	{
	    MessageBox(
		hWnd,
		"Can't Create A GL Rendering Context.",
		"ERROR",
		MB_OK | MB_ICONEXCLAMATION
	    );
	    return(-1);
	}
	if(rc_rtn != NULL)
	    *rc_rtn = rc;


#if 1
	if(TRUE)
	{
	    POINT *pPt;
	    RECT *pRect;
	    WINDOWPLACEMENT wp;

	    wp.length = sizeof(WINDOWPLACEMENT);
	    wp.flags = 0;
	    wp.showCmd = SW_SHOW;	/* Map the window too */
	    pPt = &wp.ptMaxPosition;
	    pPt->x = 0;
	    pPt->y = 0;
	    pPt = &wp.ptMinPosition;
	    pPt->x = 0;
	    pPt->y = 0;
	    pRect = &wp.rcNormalPosition;
	    pRect->left = x + ws.left;
	    pRect->right = x + ws.right;
	    pRect->top = y + ws.top;
	    pRect->bottom = y + ws.bottom;
	    SetWindowPlacement(hWnd, &wp);
	}
#else
	MoveWindow(
	    hWnd,
	    x, y,
	    ws.right - ws.left,
	    ws.bottom - ws.top,
	    FALSE
	);
	ShowWindow(hWnd, SW_SHOW);	/* Show the window */
#endif
	SetForegroundWindow(hWnd);	/* Slightly higher priority */
	SetFocus(hWnd);		/* Focus window */
	UpdateWindow(hWnd);

	return(0);
}

/*
 *	Initializes the graphics wrapper.
 */
gw_display_struct *GWInit(int argc, char **argv)
{
	int i, ctx_num;
	const char *arg, *cstrptr;
	HINSTANCE hInst;
	Boolean no_windows = False;
	int x = 0, y = 0;
	int width = 320, height = 240;
	int depth = 16;
	const char *title = "Untitled";
	gw_display_struct *display;
	float aspect_offset = 0.0f;
	Boolean fullscreen = False;


	/* Parse arguments. */
	for(i = 0; i < argc; i++)
	{
	    arg = argv[i];
	    if(arg == NULL)
		continue;

	    /* Window title. */
	    if(!STRCASECMP(arg, "--title") ||
	       !STRCASECMP(arg, "-title")
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
#if 0
		    fprintf(
			stderr,
			"%s: Requires argument.\n",
			argv[i - 1]
		    );
#endif
		}
	    }
	    /* Use full screen mode? */
	    else if(!STRCASECMP(arg, "--fullscreen") ||
		    !STRCASECMP(arg, "-fullscreen") ||
		    !STRCASECMP(arg, "--full_screen") ||
		    !STRCASECMP(arg, "-full_screen")
	    )
	    {
		fullscreen = True;
	    }
	    /* Position and size of toplevel window. */
	    else if(!STRCASECMP(arg, "--geometry") ||
		    !STRCASECMP(arg, "-geometry")
	    )
	    {
		i++;
		arg = (i < argc) ? argv[i] : NULL;
		if(arg != NULL)
		{
		    GWParseGeometry(arg, &x, &y, &width, &height);
		}
		else
		{
#if 0
		    fprintf(
			stderr,
			"%s: Requires argument.\n",
			argv[i - 1]
		    );
#endif
		}
	    }
	    /* Aspect offset. */
	    else if(!STRCASECMP(arg, "--aspect_offset") ||
		    !STRCASECMP(arg, "-aspect_offset") ||
		    !STRCASECMP(arg, "--aspect-offset") ||
		    !STRCASECMP(arg, "-aspect-offset") ||
		    !STRCASECMP(arg, "--aspectoffset") ||
		    !STRCASECMP(arg, "-aspectoffset")
	    )
	    {
		i++;
		arg = (i < argc) ? argv[i] : NULL;
		if(arg != NULL)
		{
		    aspect_offset = (float)ATOF(arg);
		}
		else
		{
#if 0
		    fprintf(
			stderr,
			"%s: Requires argument.\n",
			argv[i - 1]
		    );
#endif
		}
	    }
	    /* No window, this is a special argument to specify that no
	     * windows (or dialogs) be created. 
	     */
	    else if(!STRCASECMP(arg, "--no_windows") ||
		    !STRCASECMP(arg, "-no_windows") ||
		    !STRCASECMP(arg, "--nowindows") ||
		    !STRCASECMP(arg, "-nowindows")
	    )
	    {
		no_windows = True;
	    }
	}


	/* Allocate a new graphics wrapper display structure. */
	display = GW_DISPLAY(calloc(
	    1, sizeof(gw_display_struct)
	));
	if(display == NULL)
	    return(display);

	display->toplevel = NULL;
	display->dc = NULL;
	display->rc = NULL;
	display->toplevel_geometry = NULL;
	display->total_gl_contexts = 0;
	display->gl_context_num = -1;

	/* Get HINSTANCE. */
	display->pid = hInst = GetModuleHandle(NULL);

	/* Keyboard auto repeat. */
	display->keyboard_autorepeat = True;

	/* Clear key state table. */
	memset(
	    &display->key_state,
	    0x00,
	    GW_KEY_STATE_TABLE_MAX_KEYS * sizeof(Boolean)
	);

	display->last_button_press = 0;

	/* Depth (in bits). */
	display->depth = depth;

	/* Set aspect offset. */
	display->aspect_offset = aspect_offset;

	display->fullscreen = False;
	display->last_gui_wx = 0;
	display->last_gui_wy = 0;
	display->last_gui_wwidth = 0;
	display->last_gui_wheight = 0;

	display->cursor_shown = True;
	display->cursor_current = NULL;
#define LOAD_CURSOR(s)	LoadCursor(NULL, (s))
	display->cursor_standard = LOAD_CURSOR(IDC_ARROW);
	display->cursor_busy = LOAD_CURSOR(IDC_WAIT);
	display->cursor_text = LOAD_CURSOR(IDC_IBEAM);
	display->cursor_translate = LOAD_CURSOR(IDC_SIZEALL);
	display->cursor_zoom = LOAD_CURSOR(IDC_SIZENWSE);
#undef LOAD_CURSOR

	/* Get desktop window values. */
	display->root = (void *)HWND_DESKTOP;
#if 1
	display->root_width = GetSystemMetrics(SM_CXMAXIMIZED);
	display->root_height = GetSystemMetrics(SM_CYMAXIMIZED);
#else
	display->root_width = GetSystemMetrics(SM_CXMAXTRACK);
	display->root_height = GetSystemMetrics(SM_CYMAXTRACK);
#endif


	/* Create the first toplevel window and gl context and set it
	 * into context.
	 */
	ctx_num = GWContextNew(
	    display,
	    x, y, width, height,
	    title,
	    NULL,	/* Icon path. */
	    NULL,	/* Icon name. */
	    no_windows
	);
	if(ctx_num > -1)
	{
	    GWContextSet(display, ctx_num);
	}
	else
	{
	    /* Failed to create the first context. */
	}


	/* Get GL version. */
	cstrptr = (const char *)glGetString(GL_VERSION);
	if(cstrptr != NULL)
	{
	    char *gl_vs_ptr;
	    char *gl_vs = strdup(cstrptr);
	    if(gl_vs != NULL)
	    {
		/* Deliminate at space which separates version and vendor
		 * name.
		 */
		gl_vs_ptr = strchr(gl_vs, ' ');
		if(gl_vs_ptr != NULL)
		    (*gl_vs_ptr) = '\0';

		gl_vs_ptr = gl_vs;
		if(gl_vs_ptr != NULL)
		{
		    display->gl_version_major = atoi(gl_vs_ptr);
		    gl_vs_ptr = strchr(gl_vs_ptr, '.');
		    if(gl_vs_ptr != NULL)
			gl_vs_ptr++;
		}
		if(gl_vs_ptr != NULL)
		{
		    display->gl_version_minor = atoi(gl_vs_ptr);
		    gl_vs_ptr = strchr(gl_vs_ptr, '.');
		    if(gl_vs_ptr != NULL)
			gl_vs_ptr++;
		}

		free(gl_vs);
	    }
	}
    
	/* Get maximum texture sizes. */
	if(TRUE)
	{
	    GLint v[1];
	    glGetIntegerv(GL_MAX_TEXTURE_SIZE, v);
	    display->texture_1d_max = display->texture_2d_max =
		display->texture_3d_max = v[0];
	}

	/* Reset all other OpenGL states on record. */
	StateGLResetAll(&display->state_gl);

	/* Reset other OpenGL states. */
/*	glShadeModel(GL_SMOOTH);
	glClearColor(0.0f, 0.0f, 0.0f, 0.5f);
	glClearDepth(1.0f);
	StateGLEnable(&display->state_gl, GL_DEPTH_TEST);
	StateGLDepthFunc(&display->state_gl, GL_LEQUAL);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
 */

	/* Set pixel storage format (for rastering font bitmaps). */
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glPixelStorei(GL_PACK_ALIGNMENT, 1);

	/* Set other GL defaults that we want. Some of these might default
	 * to on but we want them off to improve performance.
	 */
	StateGLDisable(&display->state_gl, GL_DITHER);

	/* Need to go to full screen? */
	if(fullscreen)
	    GWContextFullScreen(display, True);

	return(display);
}

/*
 *	Windows event management (`processing').
 */
LRESULT CALLBACK WndProc(
	HWND hWnd,	/* Window ID. */
	UINT uMsg,	/* Message for the window. */
	WPARAM wParam,	/* Message values. */
	LPARAM lParam
)
{
	int ctx_num;
	gw_display_struct *dpy = NULL;

#define DO_MATCH_CTXNUM_WITH_HWND	\
{ \
 /* Match context number with the given window id. */ \
 for(ctx_num = 0; ctx_num < dpy->total_gl_contexts; ctx_num++) \
 { \
  if(dpy->toplevel[ctx_num] == (void *)hWnd) \
   break; \
 } \
 if(ctx_num >= dpy->total_gl_contexts) \
  ctx_num = -1; \
}

	/* Handle by message type. */
	switch(uMsg)
	{
	  /* Create message. */
	  case WM_CREATE:
	    /* Get pointer to display structure. */
	    dpy = GW_DISPLAY(
		((CREATESTRUCT *)lParam)->lpCreateParams
	    );
	    SetWindowLong(hWnd, 0, (LONG)dpy);
/*	    return(FALSE); */
	    break;

	  /* Activate message. */
	  case WM_ACTIVATE:
	    dpy = GW_DISPLAY(GetWindowLong(hWnd, 0));
	    DO_MATCH_CTXNUM_WITH_HWND
	    if((dpy != NULL) && (ctx_num > -1))
	    {
		if(dpy->func_visibility != NULL)
		{
	            /* HIWORD(wParam) if true means program is minimized. */
		    if(HIWORD(wParam))
			dpy->func_visibility(
			    ctx_num,
			    dpy->func_visibility_data,
			    GWVisibilityFullyObscured
			);
		    else
			dpy->func_visibility(
			    ctx_num,
			    dpy->func_visibility_data,
			    GWVisibilityUnobscured
			);
		}
		return(FALSE);
	    }
	    break;

	  /* Display change. */
	  case WM_DISPLAYCHANGE:
	    dpy = GW_DISPLAY(GetWindowLong(hWnd, 0));
	    DO_MATCH_CTXNUM_WITH_HWND
	    if((dpy != NULL) && (ctx_num > -1))
	    {
#if 0
		int	width = LOWORD(lParam),
			height = HIWORD(lParam),
			depth = wParam;
#endif
		return(FALSE);
	    }
	    break;

	  /* Window focus in. */
	  case WM_SETFOCUS:
	    dpy = GW_DISPLAY(GetWindowLong(hWnd, 0));
	    DO_MATCH_CTXNUM_WITH_HWND
	    if((dpy != NULL) && (ctx_num > -1))
	    {
//		HWND hWndPrev = (HWND)wParam;
		return(FALSE);
	    }
	    break;

	  /* Window focus out (ALT+TAB). */
	  case WM_KILLFOCUS:
	    dpy = GW_DISPLAY(GetWindowLong(hWnd, 0));
	    DO_MATCH_CTXNUM_WITH_HWND
	    if((dpy != NULL) && (ctx_num > -1))
	    {
		if(GWContextIsFullScreen(dpy))
		    GWContextFullScreen(dpy, False);
		return(FALSE);
	    }
	    break;

	  /* Redraw. */
	  case WM_PAINT:
	    dpy = GW_DISPLAY(GetWindowLong(hWnd, 0));
	    DO_MATCH_CTXNUM_WITH_HWND
	    if((dpy != NULL) && (ctx_num > -1))
	    {
		if(dpy->func_draw != NULL)
		    dpy->func_draw(ctx_num, dpy->func_draw_data);
		/* Continue to default handler, do not return(). */
	    }
	    break;

	  /* System command. */
	  case WM_SYSCOMMAND:
	    /* Handle by system call type. */
	    dpy = GW_DISPLAY(GetWindowLong(hWnd, 0));
	    switch(wParam)
	    {
	      /* Screen saver wanting to start. */
	      case SC_SCREENSAVE:
		return(FALSE);
	        break;

	      /* Monitor entering power save. */
	      case SC_MONITORPOWER:
		return(FALSE);
		break;
	    }
	    break;

	  /* Window close. */
	  case WM_CLOSE:
	    dpy = GW_DISPLAY(GetWindowLong(hWnd, 0));
	    DO_MATCH_CTXNUM_WITH_HWND
	    if((dpy != NULL) && (ctx_num > -1))
	    {
		PostQuitMessage(0);	/* Send WM_QUIT message. */
		return(FALSE);
	    }
	    break;

	  /* Key press. */
	  case WM_KEYDOWN:
	    /* wParam is a 0 to 255 value indicating the key value.
	     */
	    dpy = GW_DISPLAY(GetWindowLong(hWnd, 0));
	    DO_MATCH_CTXNUM_WITH_HWND
	    if((dpy != NULL) && (ctx_num > -1))
	    {
		int i, k;
		Boolean actually_changed = False;

		i = (int)CLIP(wParam, 0, GW_KEY_STATE_TABLE_MAX_KEYS - 1);

		/* Check if keyboard auto repeat is off. */
		if(dpy->keyboard_autorepeat)
		{
		    if(!(dpy->key_state[i]))
			actually_changed = True;
		}
		else
		{
		    if(dpy->key_state[i])
			return(FALSE);
		    else
			actually_changed = True;
		}
		dpy->key_state[i] = True;

		/* Get GW key value from wParam. */
		k = GWGetKeyValueFromWParam(dpy, wParam);
#if 0
if(k != '\0')
{
 char s[80];
 sprintf(s, "Key 0x%.8x %i", k, k);
 MessageBox(
  NULL, s, "Key Down",
  MB_OK | MB_ICONINFORMATION
 );
}
#endif
		/* Skip if key is a modifier and state did not actually change. */
		if(!actually_changed)
		{
		    if((k == GWKeyAlt) ||
		       (k == GWKeyCtrl) ||
		       (k == GWKeyShift)
		    )
			return(FALSE);
		}

	        if(dpy->func_keyboard != NULL)
		{
		    unsigned long t = GWWGetCurMilliTime();

		    dpy->func_keyboard(
			dpy->func_keyboard_data,
			k,
			True,	/* Key press */
			t
		    );
		}
		return(FALSE);
	    }
	    break;

	  /* Key release. */
	  case WM_KEYUP:
	    /* wParam is a 0 to 255 value indicating the key value.
	     */
	    dpy = GW_DISPLAY(GetWindowLong(hWnd, 0));
	    DO_MATCH_CTXNUM_WITH_HWND
	    if((dpy != NULL) && (ctx_num > -1))
	    {
		int i, k;
		Boolean actually_changed = False;

		i = (int)CLIP(wParam, 0, GW_KEY_STATE_TABLE_MAX_KEYS - 1);

		/* Check if keyboard auto repeat is off. */
		if(dpy->keyboard_autorepeat)
		{
		    if(dpy->key_state[i])
			actually_changed = True;
		}
		else
		{
		    if(dpy->key_state[i])
			actually_changed = True;
		    else
			return(FALSE);
		}
		dpy->key_state[i] = False;

		/* Get GW key value from wParam. */
		k = GWGetKeyValueFromWParam(dpy, wParam);

		/* Skip if key is a modifier and state did not actually change. */
		if(!actually_changed)
		{
		    if((k == GWKeyAlt) ||
		       (k == GWKeyCtrl) ||
		       (k == GWKeyShift)
		    )
			return(FALSE);
		}

		if(dpy->func_keyboard != NULL)
		{
		    unsigned long t = GWWGetCurMilliTime();
 
		    dpy->func_keyboard(
			dpy->func_keyboard_data,
			k,
			False,	/* Key release */
			t
		    );
		}
		return(FALSE);
	    }
	    break;

	  /* Window move. */
	  case WM_MOVE:
	    /* LoWord = X, HiWord = Y. */
	    dpy = GW_DISPLAY(GetWindowLong(hWnd, 0));
	    DO_MATCH_CTXNUM_WITH_HWND
	    if((dpy != NULL) && (ctx_num > -1))
	    {
		WRectangle *rect = &dpy->toplevel_geometry[ctx_num];
		rect->x = LOWORD(lParam);
		rect->y = HIWORD(lParam);

		/* Call resize callback. */
		if(dpy->func_resize != NULL)
		    dpy->func_resize(
			ctx_num, dpy->func_resize_data,
			rect->x, rect->y,
			rect->width, rect->height
		    );

		return(FALSE);
	    }
	    break;

	  /* Window resize. */
	  case WM_SIZE:
	    /* LoWord = Width, HiWord = Height. */
	    dpy = GW_DISPLAY(GetWindowLong(hWnd, 0));
	    DO_MATCH_CTXNUM_WITH_HWND
	    if((dpy != NULL) && (ctx_num > -1))
	    {
		WRectangle *rect = &dpy->toplevel_geometry[ctx_num];
		rect->width = LOWORD(lParam);
		rect->height = HIWORD(lParam);

		/* Call resize callback. */
		if(dpy->func_resize != NULL)
		    dpy->func_resize(
			ctx_num, dpy->func_resize_data,
			rect->x, rect->y,
			rect->width, rect->height
		    );

		/* Call redraw callback as well. */
		if(dpy->func_draw != NULL)
		    dpy->func_draw(ctx_num, dpy->func_draw_data);

		return(FALSE);
	    }
	    break;

	  /* Window map/unmap.*/
	  case WM_SHOWWINDOW:
	    dpy = GW_DISPLAY(GetWindowLong(hWnd, 0));
	    DO_MATCH_CTXNUM_WITH_HWND
	    if((dpy != NULL) && (ctx_num > -1))
	    {
#if 0
		if(wParam)
		    /* Window mapped */
		else
		    /* Window unmapped */
#endif
		return(FALSE);
	    }
	    break;

	  /* Pointer button press. */
	  case WM_LBUTTONDOWN:
	    dpy = GW_DISPLAY(GetWindowLong(hWnd, 0));
	    DO_MATCH_CTXNUM_WITH_HWND
	    if((dpy != NULL) && (ctx_num > -1))
	    {
		if(dpy->func_pointer != NULL)
		{
		    int x = (int)((unsigned int)lParam & 0x0000ffff);
		    int y = (int)(((unsigned int)lParam & 0xffff0000) >> 16);
		    int button_press_count = dpy->button_press_count;
		    unsigned long	last_t = dpy->last_button_press,
					t = GWWGetCurMilliTime();

		    dpy->func_pointer(
			ctx_num, dpy->func_pointer_data,
			x, y, GWEventTypeButtonPress, 1, t
		    );

		    /* Check double click */
		    if((button_press_count == 0) &&
		       (last_t > 0) &&
		       (last_t <= t) &&
	((t - last_t) <= GW_2BUTTON_PRESS_INTERVAL)
		    )
		    {
			dpy->func_pointer(
			    ctx_num, dpy->func_pointer_data,
			    x, y, GWEventType2ButtonPress, 1, t
			);
			dpy->button_press_count++;
		    }
		    /* Check triple click */
		    else if((button_press_count == 1) &&
		            (last_t > 0) &&
		            (last_t <= t) &&
	((t - last_t) <= GW_3BUTTON_PRESS_INTERVAL)
		    )
		    {
			dpy->func_pointer(
			    ctx_num, dpy->func_pointer_data,
			    x, y, GWEventType3ButtonPress, 1, t
			);
			dpy->button_press_count++;
		    }
		    else
		    {
			dpy->button_press_count = 0;
			dpy->last_button_press = t;
		    }
		}
		return(FALSE);
	    }
	    break;

	  case WM_RBUTTONDOWN:
	    dpy = GW_DISPLAY(GetWindowLong(hWnd, 0));
	    DO_MATCH_CTXNUM_WITH_HWND
	    if((dpy != NULL) && (ctx_num > -1))
	    {
		if(dpy->func_pointer != NULL)
		{
		    int x = (int)((unsigned int)lParam & 0x0000ffff);
		    int y = (int)(((unsigned int)lParam & 0xffff0000) >> 16);
		    long t = (long)GWWGetCurMilliTime();

		    dpy->func_pointer(
			ctx_num, dpy->func_pointer_data,
			x, y, GWEventTypeButtonPress, 3, t
		    );
		}
		return(FALSE);
	    }
	    break;

	  /* Pointer button release. */
	  case WM_LBUTTONUP:
	    dpy = GW_DISPLAY(GetWindowLong(hWnd, 0));
	    DO_MATCH_CTXNUM_WITH_HWND
	    if((dpy != NULL) && (ctx_num > -1))
	    {
		if(dpy->func_pointer != NULL)
		{
		    int x = (int)((unsigned int)lParam & 0x0000ffff);
		    int y = (int)(((unsigned int)lParam & 0xffff0000) >> 16);
		    long t = (long)GWWGetCurMilliTime();

		    dpy->func_pointer(
			ctx_num, dpy->func_pointer_data,
			x, y, GWEventTypeButtonRelease, 1, t
		    );
		}
		return(FALSE);
	    }
	    break;

	  case WM_RBUTTONUP:
	    dpy = GW_DISPLAY(GetWindowLong(hWnd, 0));
	    DO_MATCH_CTXNUM_WITH_HWND
	    if((dpy != NULL) && (ctx_num > -1))
	    {
		if(dpy->func_pointer != NULL)
		{
		    int x = (int)((unsigned int)lParam & 0x0000ffff);
		    int y = (int)(((unsigned int)lParam & 0xffff0000) >> 16);
		    long t = (long)GWWGetCurMilliTime();

		    dpy->func_pointer(
			ctx_num, dpy->func_pointer_data,
			x, y, GWEventTypeButtonRelease, 3, t
		    );
		}
		return(FALSE);
	    }
	    break;

	  /* Pointer motion. */
	  case WM_MOUSEMOVE:
	    dpy = GW_DISPLAY(GetWindowLong(hWnd, 0));
	    DO_MATCH_CTXNUM_WITH_HWND
	    if((dpy != NULL) && (ctx_num > -1))
	    {
		if(dpy->func_pointer != NULL)
		{
		    int x = (int)((unsigned int)lParam & 0x0000ffff);
		    int y = (int)(((unsigned int)lParam & 0xffff0000) >> 16);
		    long t = (long)GWWGetCurMilliTime();

		    dpy->func_pointer(
			ctx_num, dpy->func_pointer_data,
			x, y, GWEventTypePointerMotion, 0, t
		    );
		}
		return(FALSE);
	    }
	    break;

	  /* Set cursor. */
	  case WM_SETCURSOR:
	    dpy = GW_DISPLAY(GetWindowLong(hWnd, 0));
	    DO_MATCH_CTXNUM_WITH_HWND
	    if((dpy != NULL) && (ctx_num > -1))
	    {
		return(FALSE);	// Allow further processing of WM_SETCURSOR
	    }
	    break;

	  /* Timeout callback. */
	  case WM_TIMER:
	    dpy = GW_DISPLAY(GetWindowLong(hWnd, 0));
	    DO_MATCH_CTXNUM_WITH_HWND
	    if((dpy != NULL) && (ctx_num > -1))
	    {
		return(FALSE);
	    }
	    break;

	}

#undef DO_MATCH_CTXNUM_WITH_HWND

	/* Forward unhandled message to DefWindowProc() */
	return(DefWindowProc(hWnd, uMsg, wParam, lParam));
}

/*
 *	Graphics wrapper management.
 */
void GWManage(gw_display_struct *display)
{
	int ctx_num;
	MSG msg;


	if(display == NULL)
	    return;

#define DO_MATCH_CTXNUM_WITH_HWND	\
{ \
 /* Match context number with the given window id. */ \
 for(ctx_num = 0; ctx_num < display->total_gl_contexts; ctx_num++) \
 { \
  if(display->toplevel[ctx_num] == (void *)hWnd) \
   break; \
 } \
 if(ctx_num >= display->total_gl_contexts) \
  ctx_num = -1; \
}

	/* Update modifier key states. */
	display->alt_key_state = (GetAsyncKeyState(VK_MENU)) ? True : False;
	display->ctrl_key_state = (GetAsyncKeyState(VK_CONTROL)) ? True : False;
	display->shift_key_state = (GetAsyncKeyState(VK_SHIFT)) ? True : False;

	/* Manage event messages. */
	if(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
	{
	    HWND hWnd = msg.hwnd;

	    switch(msg.message)
	    {
	      case WM_QUIT:
		if((void *)hWnd == NULL)
		{
		    /* Call close callback for all contexts. */
		    if(display->func_close != NULL)
		    {
			for(ctx_num = 0; ctx_num < display->total_gl_contexts; ctx_num++)
			    display->func_close(
				ctx_num,
				display->func_close_data,
				NULL
			    );
		    }
		}
		else
		{
		    DO_MATCH_CTXNUM_WITH_HWND
		    if((display->func_close != NULL) && (ctx_num > -1))
		    {
			display->func_close(
			    ctx_num,
			    display->func_close_data,
			    (void *)hWnd
			);
		    }
		}
		break;
				      
	      default:
		TranslateMessage(&msg);	// Translate message
		DispatchMessage(&msg);	// Dispatch message
		break;
	    }
	}
	else
	{
	    /* Did not get any new messages, call client timeout function. */
	    if(display->func_timeout != NULL)
		display->func_timeout(
		    display->func_timeout_data
		);
	}

#undef DO_MATCH_CTXNUM_WITH_HWND
}

/*
 *	Graphics wrapper shutdown.
 */
void GWShutdown(gw_display_struct *display)
{
	int i;
	HINSTANCE hInst;

	if(display == NULL)
	    return;

	hInst = (HINSTANCE)display->pid;

	/* If we are still in full screen mode then we need to
	 * return to GUI mode.
	 */
	if(GWContextIsFullScreen(display))
	    GWContextFullScreen(display, False);

	/* Make sure cursor is shown. */
	GWShowCursor(display);

	if(TRUE)
	{
	    /* Destroy all gl contexts and toplevel Windows, current
	     * gl context will be disabled first as needed.
	     */
	    for(i = display->total_gl_contexts - 1; i >= 0; i--)
		GWContextDelete(display, i);

	    /* Deallocate pointer arrays for all gl contexts and
	     * toplevel Windows.
	     */
	    free(display->toplevel);
	    display->toplevel = NULL;
	    free(display->dc);
	    display->dc = NULL;
	    free(display->rc);
	    display->rc = NULL;
	    free(display->toplevel_geometry);
	    display->toplevel_geometry = NULL;
	    display->total_gl_contexts = 0;
	    display->gl_context_num = -1;
	}


	/* Are we able to unregister class? */
	if(!UnregisterClass("OpenGL", (HINSTANCE)display->pid))
	{
	    MessageBox(
		NULL,
		"Could Not Unregister Class.",
		"SHUTDOWN ERROR",
		MB_OK | MB_ICONINFORMATION
	    );
	}

#define DESTROY_CURSOR(p)	\
{ if(*(p) != NULL) { DestroyCursor((HCURSOR)*(p)); *(p) = NULL; } }
	display->cursor_current = NULL;
	DESTROY_CURSOR(&display->cursor_standard);
	DESTROY_CURSOR(&display->cursor_busy);
	DESTROY_CURSOR(&display->cursor_text);
	DESTROY_CURSOR(&display->cursor_translate);
	DESTROY_CURSOR(&display->cursor_zoom);
#undef DESTROY_CURSOR

	display->pid = hInst = NULL;

	/* Deallocate display structure. */
	free(display);
}


/*
 *      Flushes output.
 */
void GWFlush(gw_display_struct *display)
{

}

/*
 *      Sync output.
 */
void GWSync(gw_display_struct *display)
{

}

/*
 *      Returns the number of events pending.
 */
int GWEventsPending(gw_display_struct *display)
{
	MSG mMsg;

	if(display == NULL)
	    return(0);

	if(PeekMessage(&mMsg, NULL, 0, 0, PM_NOREMOVE))
	    return(1);
	else
	    return(0);
}


/*
 *	Output message in dialog form.
 */
void GWOutputMessage(
	gw_display_struct *display,
	int type,       /* One of GWOutputMessageType*. */
	const char *subject,
	const char *message,
	const char *help_details
)
{
	int ctx_num;
	HWND hwnd;
	UINT uType = MB_OK;


	if(display == NULL)
	    return;

	ctx_num = display->gl_context_num;
	if((ctx_num >= 0) && (ctx_num < display->total_gl_contexts))
	    hwnd = (HWND)display->toplevel[ctx_num];
	else
	    hwnd = NULL;

	switch(type)
	{
	  case GWOutputMessageTypeGeneral:
	    uType |= MB_ICONINFORMATION;
	    break;
	  case GWOutputMessageTypeWarning:
	    uType |= MB_ICONWARNING;
	    break;
	  case GWOutputMessageTypeError:
	    uType |= MB_ICONERROR;
	    break;
	  case GWOutputMessageTypeQuestion:
	    uType |= MB_ICONQUESTION;
	    break;
	  default:	/* GWOutputMessageTypeInformation */
	    uType |= MB_ICONINFORMATION;
	    break;
	}

	MessageBox(
	    hwnd,	/* Window handle. */
	    message,	/* Text. */
	    subject,	/* Caption. */
	    uType
	);
}

/*
 *	Map confirmation dialog and block client execution.
 */
int GWConfirmation(
	gw_display_struct *display,
	int type,       /* One of GWOutputMessageType*. */
	const char *subject,   
	const char *message,
	const char *help_details,
	int default_response
)
{
	int ctx_num, status;
	HWND hwnd;
	UINT uType = MB_YESNOCANCEL | MB_ICONINFORMATION;


	if(display == NULL)
	    return(GWConfirmationNotAvailable);

	ctx_num = display->gl_context_num;
	if((ctx_num >= 0) && (ctx_num < display->total_gl_contexts))
	    hwnd = (HWND)display->toplevel[ctx_num];
	else
	    hwnd = NULL;

	switch(type)
	{
	  case GWOutputMessageTypeGeneral:
	    uType |= MB_ICONINFORMATION;
	    break;
	  case GWOutputMessageTypeWarning:
	    uType |= MB_ICONWARNING;
	    break;
	  case GWOutputMessageTypeError:
	    uType |= MB_ICONERROR;
	    break;
	  case GWOutputMessageTypeQuestion:
	    uType |= MB_ICONQUESTION;
	    break;
	  default:      /* GWOutputMessageTypeInformation */
	    uType |= MB_ICONINFORMATION;
	    break;
	}

	switch(MessageBox(
	    hwnd,       /* Window handle. */
	    message,    /* Text. */
	    subject,    /* Caption. */
	    uType
	))
	{
	  case IDOK: case IDYES:
	    status = GWConfirmationYes;
	    break;
	  case IDCANCEL: case IDABORT: case IDIGNORE: case IDCLOSE:
	    status = GWConfirmationCancel;
	    break;
	  case IDNO:
	    status = GWConfirmationNo;
	    break;
	  default:
	    status = GWConfirmationNotAvailable;
	    break;
	}
	return(status);
}

/*
 *	Simplified version of GWConfirmation().
 */
int GWConfirmationSimple(
	gw_display_struct *display,
	const char *message
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

/* Set draw callback function, needs to be set before the resize function! */
void GWSetDrawCB(
	gw_display_struct *display,
	void (*func)(int, void *),
	void *data
)
{
	if(display != NULL)
	{
	    display->func_draw = func;
	    display->func_draw_data = data;
	}
}


/* Set resize callback function.

   Reminder, when resize callback is set, a call must be made to it
   immediatly because under Windows, the callback needs
   to be set after the Windows is created and Windows may not send
   a resize request later.

   So this function needs to be set after the draw function.
 */
void GWSetResizeCB(
	gw_display_struct *display, 
	void (*func)(int, void *, int, int, int, int),
	void *data
)
{
	if(display != NULL)
	{
	    int width, height;

	    display->func_resize = func;
	    display->func_resize_data = data;

	    GWContextGet(
		display, display->gl_context_num,
		NULL, NULL,
		NULL, NULL,
		&width, &height
	    );

	    /* Now call the resize function. */
	    if(display->func_resize != NULL)
		display->func_resize(
		    display->gl_context_num,
		    display->func_resize_data,
		    0, 0,
		    width, height
		);
	}
}

/* Set keyboard callback function. */
void GWSetKeyboardCB(
	gw_display_struct *display,
	void (*func)(void *, int, Boolean, unsigned long),
	void *data
)
{
	if(display != NULL)
	{
	    display->func_keyboard = func;
	    display->func_keyboard_data = data;
	}
}

/* Set pointer callback function. */
void GWSetPointerCB(
	gw_display_struct *display,
	void (*func)(int, void *, int, int, gw_event_type, int, unsigned long),
	void *data
)
{
	if(display != NULL)
	{
	    display->func_pointer = func;
	    display->func_pointer_data = data;
	}
}

/* Set visibility callback function. */
void GWSetVisibilityCB(
	gw_display_struct *display,
	void (*func)(int, void *, gw_visibility),
	void *data
)
{
	if(display != NULL)
	{
	    display->func_visibility = func;
	    display->func_visibility_data = data;

	    /* Need to send visibility notify upon setting of function,
	     * assume visibility unobscured.
	     */
	    if(display->func_visibility != NULL)
	    {
		display->func_visibility(
		    display->gl_context_num,
		    display->func_visibility_data,
		    GWVisibilityUnobscured
		);
	    }
	}
}

/* Set save yourself callback function. */
void GWSetSaveYourselfCB(
	gw_display_struct *display,
	void (*func)(int, void *),
	void *data
)
{
	if(display != NULL)
	{
	    display->func_save_yourself = func;
	    display->func_save_yourself_data = data;
	}
}

/* Set close callback function. */
void GWSetCloseCB(
	gw_display_struct *display,
	void (*func)(int, void *, void *),
	void *data
)
{
	if(display != NULL)
	{
	    display->func_close = func;
	    display->func_close_data = data;
	}
}

/* Set timeout callback function. */
void GWSetTimeoutCB(
	gw_display_struct *display,
	void (*func)(void *),
	void *data
)
{
	if(display != NULL)
	{
	    display->func_timeout = func;
	    display->func_timeout_data = data;
	}
}


/*
 *	Creates a new gl context and window, returning the context
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
	int ctx_num;
	void *w, *dc, *rc;
	WRectangle *rect;


	if(display == NULL)
	    return(-1);


	/* Search for an available gl context index number. */
	for(ctx_num = 0; ctx_num < display->total_gl_contexts; ctx_num++)
	{
	    w = display->toplevel[ctx_num];
	    dc = display->dc[ctx_num];
	    rc = display->rc[ctx_num];

	    /* This gl context index available? */
	    if((w == NULL) && (dc == NULL) && (rc == NULL))
		break;
	}
	/* No available gl contexts? */
	if(ctx_num >= display->total_gl_contexts)
	{
	    /* Allocate a new gl context. */

	    ctx_num = display->total_gl_contexts;
	    display->total_gl_contexts = ctx_num + 1;

	    display->toplevel = (void **)realloc(
		display->toplevel,
		display->total_gl_contexts * sizeof(void *)
	    );
	    display->dc = (void **)realloc(
		display->dc,
		display->total_gl_contexts * sizeof(void *)
	    );
	    display->rc = (void **)realloc(
		display->rc,
		display->total_gl_contexts * sizeof(void *)
	    );
	    display->toplevel_geometry = (WRectangle *)realloc(
		display->toplevel_geometry,
		display->total_gl_contexts * sizeof(WRectangle)
	    );
	    if((display->toplevel == NULL) || (display->dc == NULL) ||
	       (display->rc == NULL) || (display->toplevel_geometry == NULL)
	    )
	    {
		display->gl_context_num = -1;
		display->total_gl_contexts = 0;
		return(-1);
	    }
	}

	/* At this point we have an available gl context specified by
	 * ctx_num.
	 */


	/* Create first toplevel window and buffer. */
	if(!no_windows)
	{
	    /* Set initial geometry of new Window before creation. */
	    rect = &display->toplevel_geometry[ctx_num];
	    rect->x = x;
	    rect->y = y;
	    rect->width = width;
	    rect->height = height;

	    /* Create gl rendering context */
	    if(GWCreateWindow(
		display,
		NULL,		/* No parent, since creating a toplevel window. */
		&display->toplevel[ctx_num],
		&display->dc[ctx_num],
		&display->rc[ctx_num],
		x, y, width, height,
		display->depth,	/* Depth in bits. */
		title
	    ))
	    {
		/* Failed to create window. */
		return(-1);
	    }
	}

	return(ctx_num);
}

/*
 *	Destroys the gl context and window given by the context number
 *	ctx_num. If the specified gl context is the current gl context
 *	then the current gl context will be no longer valid.
 */
void GWContextDelete(
	gw_display_struct *display, int ctx_num
)
{
	WRectangle *rect;


	if(display == NULL)
	    return;

	if((ctx_num < 0) || (ctx_num >= display->total_gl_contexts))
	    return;

 	/* Is the given gl context the current gl context? */
	if(ctx_num == display->gl_context_num)
	{
	    display->gl_context_num = -1;

 	    if(!wglMakeCurrent(NULL, NULL))
	    {
		MessageBox(
		    NULL,
		    "Unable To Disable Current GL Context.",
		    "GL Error",
		    MB_OK | MB_ICONERROR
		);
	    }
	}

	/* Release the rendering context. */
	if(display->rc[ctx_num] != NULL)
	{
	    if(!wglDeleteContext((HGLRC)display->rc[ctx_num]))
	    {
		MessageBox(
		    NULL,
		    "Unable To Release Rendering Context.",
		    "GL Error",
		    MB_OK | MB_ICONERROR
		);
	    }
	    display->rc[ctx_num] = NULL;
	}

	/* Release the device context. */
	if((display->toplevel[ctx_num] != NULL) &&
	   (display->dc[ctx_num] != NULL)
	)
	{
	    if(!ReleaseDC(
		(HWND)display->toplevel[ctx_num],
		(HDC)display->dc[ctx_num]
	    ))
	    {
		MessageBox(
		    NULL,
		    "Unable To Release Device Context.",
		    "Windows Error",
		    MB_OK | MB_ICONERROR
		);
	    }
	    display->dc[ctx_num] = NULL;
	}

	/* Destroy the toplevel window. */
	if(display->toplevel[ctx_num] != NULL)
	{
	    if(!DestroyWindow((HWND)display->toplevel[ctx_num]))
	    {
		MessageBox(
		    NULL,
		    "Unable To Destroy Toplevel Window.",
		    "Windows Error",
		    MB_OK | MB_ICONERROR
		);
	    }
	    display->toplevel[ctx_num] = NULL;	/* Set hWnd To NULL */
	}


	/* Clear the toplevel rectangle. */
	rect = &display->toplevel_geometry[ctx_num];
	memset(rect, 0x00, sizeof(WRectangle));

}

/*
 *      Returns the current GL context number or -1 on error.
 */
int GWContextCurrent(gw_display_struct *display)
{
	return((display != NULL) ?
	    MIN(display->gl_context_num, display->total_gl_contexts - 1) : -1
	);
}

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
	void *w;
	void *rc;
	const WRectangle *rect;


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

	if(display == NULL)
	    return(-1);

	if((ctx_num < 0) || (ctx_num >= display->total_gl_contexts))
	    return(-1);

	w = display->toplevel[ctx_num];
	rc = display->rc[ctx_num];
	rect = &display->toplevel_geometry[ctx_num];

	if((w == NULL) || (rc == NULL))
	    return(-1);


	if(window_id_rtn != NULL)
	    *window_id_rtn = (void *)w;
	if(gl_context_rtn != NULL)
	    *gl_context_rtn = (void *)rc;

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

/*
 *	Puts the GL rendering context specified by ctx_num as the
 *	current rendering GL context.
 *
 *	Returns non-zero on error.
 */
int GWContextSet(gw_display_struct *display, int ctx_num)
{
	void *dc, *rc;


	if(display == NULL)
	    return(-1);

	if((ctx_num < 0) || (ctx_num >= display->total_gl_contexts))
	    return(-1);

	/* Specified gl context already the current gl context? */
	if(ctx_num == display->gl_context_num)
	    return(0);


	/* Get device context and rendering context of the specified context. */
	dc = display->dc[ctx_num];
	rc = display->rc[ctx_num];

	if((dc == NULL) || (rc == NULL))
	    return(-1);

	/* Activate the rendering context, thus putting it into GL context. */
	if(!wglMakeCurrent((HDC)dc, (HGLRC)rc))
	{
	    return(-1);
	}
	else
	{
	    display->gl_context_num = ctx_num;
	    return(0);
	}
}

/*
 *      Sets the position of the current GL rendering context.
 */
void GWContextPosition(
	gw_display_struct *display, int x, int y
)
{
	int ctx_num;
	HWND w;
	WRectangle *rect;
	if(display == NULL)
	    return;

	ctx_num = display->gl_context_num;
	if((ctx_num >= 0) && (ctx_num < display->total_gl_contexts))
	{
	    w = (HWND)display->toplevel[ctx_num];
	    rect = &display->toplevel_geometry[ctx_num];
	}
	else
	{
	    w = NULL;
	    rect = NULL;
	}
	if(w == NULL)
	    return;

	if((x != rect->x) || (y != rect->y))
	{
#if 1
	    POINT *pPt;
	    RECT *pRect, ws;
	    WINDOWPLACEMENT wp;
	    DWORD dwStyle = GWW_TOPLEVEL_WINDOW_STYLE;
	    DWORD dwExStyle = GWW_TOPLEVEL_WINDOW_EXSTYLE;

	    rect->x = x;
	    rect->y = y;

	    /* Set window size (left and top are not positions). */
	    ws.left = (long)0;
	    ws.right = (long)rect->width;
	    ws.top = (long)0;
	    ws.bottom = (long)rect->height;

	    /* Adjust window size to take into account its
	     * style's decorations.
	     */
	    AdjustWindowRectEx(&ws, dwStyle, FALSE, dwExStyle);

	    wp.length = sizeof(WINDOWPLACEMENT);
	    wp.flags = 0;
	    wp.showCmd = SW_SHOWNA;
	    pPt = &wp.ptMaxPosition;
	    pPt->x = 0;
	    pPt->y = 0;
	    pPt = &wp.ptMinPosition;
	    pPt->x = 0;
	    pPt->y = 0;
	    pRect = &wp.rcNormalPosition;
	    pRect->left = rect->x + ws.left;
	    pRect->right = rect->x + ws.right;
	    pRect->top = rect->y + ws.top;
	    pRect->bottom = rect->y + ws.bottom;
	    SetWindowPlacement(w, &wp);
#else
	    DWORD dwStyle = GWW_TOPLEVEL_WINDOW_STYLE;
	    DWORD dwExStyle = GWW_TOPLEVEL_WINDOW_EXSTYLE;
	    RECT ws;

	    /* Set window size (left and top are not positions). */
	    ws.left = (long)0;
	    ws.right = (long)rect->width;
	    ws.top = (long)0;
	    ws.bottom = (long)rect->height;

	    /* Adjust window size to take into account its
	     * style's decorations.
	     */
	    AdjustWindowRectEx(&ws, dwStyle, FALSE, dwExStyle);

	    rect->x = x;
	    rect->y = y;

	    MoveWindow(
		w,
		rect->x + ws.left, rect->y + ws.top,
		ws.right - ws.left, ws.bottom - ws.top,
		TRUE
	    );
	    UpdateWindow(w);
#endif
 	}
}

/*
 *      Sets the size of the current GL rendering context.
 *
 *      If currently in full screen then the calling function needs
 *      to call GWContextFullScreen(display, True) after calling this
 *      function so that the full screen is adjusted to the new size.
 */
void GWContextSize(
	gw_display_struct *display, int width, int height
)
{
	int ctx_num;
	HWND w;
	WRectangle *rect;
	if(display == NULL)
	    return;

	ctx_num = display->gl_context_num;
	if((ctx_num >= 0) && (ctx_num < display->total_gl_contexts))
	{
	    w = (HWND)display->toplevel[ctx_num];
	    rect = &display->toplevel_geometry[ctx_num];
	}
	else
	{
	    w = NULL;
	    rect = NULL;
	}
	if(w == NULL)
	    return;

	if(((width != rect->width) || (height != rect->height)) &&
	   ((width >= GW_MIN_TOPLEVEL_WIDTH) && (height >= GW_MIN_TOPLEVEL_HEIGHT))
	)
	{
#if 1
	    int	new_width = (int)MAX(
		    ((width > 0) ? width : rect->width),
		    GW_MIN_TOPLEVEL_WIDTH
		),
		new_height = (int)MAX(
		    ((height > 0) ? height : rect->height),
		    GW_MIN_TOPLEVEL_HEIGHT
		);
	    POINT *pPt;
	    RECT *pRect, ws;
	    WINDOWPLACEMENT wp;
	    DWORD dwStyle = GWW_TOPLEVEL_WINDOW_STYLE;
	    DWORD dwExStyle = GWW_TOPLEVEL_WINDOW_EXSTYLE;

	    rect->width = new_width;
	    rect->height = new_height;

	    /* Set window size (left and top are not positions). */
	    ws.left = (long)0;
	    ws.right = (long)new_width;
	    ws.top = (long)0;
	    ws.bottom = (long)new_height;

	    /* Adjust window size to take into account its
	     * style's decorations.
	     */
	    AdjustWindowRectEx(&ws, dwStyle, FALSE, dwExStyle);

	    wp.length = sizeof(WINDOWPLACEMENT);
	    wp.flags = 0;
	    wp.showCmd = SW_SHOWNA;
	    pPt = &wp.ptMaxPosition;
	    pPt->x = 0;
	    pPt->y = 0;
	    pPt = &wp.ptMinPosition;
	    pPt->x = 0;
	    pPt->y = 0;
	    pRect = &wp.rcNormalPosition;
	    pRect->left = rect->x + ws.left;
	    pRect->right = rect->x + ws.right;
	    pRect->top = rect->y + ws.top;
	    pRect->bottom = rect->y + ws.bottom;
	    SetWindowPlacement(w, &wp);
#else
	    DWORD dwStyle = GWW_TOPLEVEL_WINDOW_STYLE;
	    DWORD dwExStyle = GWW_TOPLEVEL_WINDOW_EXSTYLE;
	    RECT ws;
	    int	new_width = (int)MAX(
		    ((width > 0) ? width : rect->width),
		    GW_MIN_TOPLEVEL_WIDTH
		),
		new_height = (int)MAX(
		    ((height > 0) ? height : rect->height),
		    GW_MIN_TOPLEVEL_HEIGHT
		);

	    /* Set window size (left and top are not positions). */
	    ws.left = (long)0;
	    ws.right = (long)new_width;
	    ws.top = (long)0;
	    ws.bottom = (long)new_height;

	    /* Adjust window size to take into account its
	     * style's decorations.
	     */
	    AdjustWindowRectEx(&ws, dwStyle, FALSE, dwExStyle);

	    rect->width = new_width;
	    rect->height = new_height;

	    MoveWindow(
		w,
		rect->x + ws.left, rect->y + ws.top,
		ws.right - ws.left, ws.bottom - ws.top,
		TRUE
	    );
	    UpdateWindow(w);
#endif
	}
}

/*
 *      Returns True if the current context is currently full screen.
 */
Boolean GWContextIsFullScreen(gw_display_struct *display)
{
	return((display != NULL) ? display->fullscreen : False);
}

/*
 *      Sets the current context into full screen if state is True
 *      or returns to GUI mode if state is False.
 *
 *      The full screen size depends on the current size of the GL
 *      context so GWContextSize() should be called prior to going
 *      to full screen.
 *
 *      Returns:
 *
 *      0       Success
 *      -1      General error
 *      -2      Unable to find valid mode
 *      -3      System error
 *      -5      This function is not supported
 */
int GWContextFullScreen(gw_display_struct *display, Boolean state)
{
	int ctx_num;
	HWND w;
	WRectangle *rect;


	if(display == NULL)
	    return(-1);

	ctx_num = display->gl_context_num;
	if((ctx_num >= 0) && (ctx_num < display->total_gl_contexts))
	{
	    w = (HWND)display->toplevel[ctx_num];
	    rect = &display->toplevel_geometry[ctx_num];
	}
	else
	{
	    w = NULL;
	    rect = NULL;
	}
	if(w == NULL)
	    return(-1);

	/* Go to full screen mode? */
	if(state)
	{
	    /* Set up device mode for specifying full screen
	     * parameters.
	     */
	    DEVMODE dms;

	    /* Reset device mode structure values. */
	    memset(&dms, 0x00, sizeof(dms));

	    /* Set values for device mode structure. */
	    dms.dmSize = sizeof(dms);
	    dms.dmFields =	DM_PELSWIDTH |
				DM_PELSHEIGHT |
				DM_BITSPERPEL;
	    dms.dmPelsWidth = rect->width;
	    dms.dmPelsHeight = rect->height;
	    dms.dmBitsPerPel = display->depth;

	    /* Switch to full screen mode. */
	    if(ChangeDisplaySettings(&dms, CDS_FULLSCREEN)
		== DISP_CHANGE_BADMODE
	    )
	    {
		/* Unable to switch to full screen mode, it is
		 * possible that the size of the context was not
		 * suitable so instead we find a more reasonable
		 * size.
		 */
		int width, height;

		if(1280 <= rect->width)
		    width = 1280;
		else if(1024 <= rect->width)
		    width = 1024;
		else if(800 <= rect->width)
		    width = 800;
		else if(640 <= rect->width)
		    width = 640;
		else
		    width = 320;
		height = (int)(width * 0.75);

		dms.dmPelsWidth = width;
		dms.dmPelsHeight = height;

		/* Try again to switch to full screen mode
		 * using a more reliable mode size.
		 */
		if(ChangeDisplaySettings(&dms, CDS_FULLSCREEN)
		    != DISP_CHANGE_SUCCESSFUL
		)
		    return(-2);
	    }

/*	    ShowCursor(TRUE); */

	    /* Was not in full screen mode? */
	    if(!display->fullscreen)
	    {
		/* Mark that we are in full screen mode now */
		display->fullscreen = True;

		/* Record last window position in GUI mode */
		display->last_gui_wx = rect->x;
		display->last_gui_wy = rect->y;
		display->last_gui_wwidth = rect->width;
		display->last_gui_wheight = rect->height;

		/* Move window to upper left corner */
		GWContextPosition(display, 0, 0);

		/* Set window as topmost */
		SetWindowPos(
		    w, HWND_TOPMOST,
		    0, 0, 0, 0,
		    SWP_NOMOVE | SWP_NOSIZE
		);
	    }
	    return(0);
	}
	else
	{
	    /* Go to GUI mode. */
	    if(display->fullscreen)
	    {
		display->fullscreen = False;

		/* Restore display settings to their defaults */
		ChangeDisplaySettings(NULL, 0);

		/* Set the window as no longer being topmost */
		SetWindowPos(
		    w, HWND_NOTOPMOST,
		    0, 0, 0, 0,
		    SWP_NOMOVE | SWP_NOSIZE
		);

/*		ShowCursor(TRUE); */

		/* Restore window position and size that it last was in GUI mode */
		GWContextPosition(
		    display, display->last_gui_wx, display->last_gui_wy
		);
		GWContextSize(
		    display, display->last_gui_wwidth, display->last_gui_wheight
		);
	    }
	    return(0);
	}
}


/*
 *	Sends a redraw event (calls redraw callback function immediatly).
 */
void GWPostRedraw(gw_display_struct *display)
{
	int ctx_num;

	if(display == NULL)
	    return;

	ctx_num = display->gl_context_num;
	if((ctx_num < 0) || (ctx_num >= display->total_gl_contexts))
	    return;

	/* Call redraw function. */
	if(display->func_draw != NULL)
	    display->func_draw(
		ctx_num, display->func_draw_data
	    );
}

/*
 *	Swaps buffers.
 */
void GWSwapBuffer(gw_display_struct *display)
{
	int ctx_num;


	if(display == NULL)
	    return;

	ctx_num = display->gl_context_num;
	if((ctx_num < 0) || (ctx_num >= display->total_gl_contexts))
	    return;

	if(display->dc[ctx_num] != NULL)
	    SwapBuffers((HDC)display->dc[ctx_num]);
}


/*
 *	Turns keyboard auto repeat on/off.
 */
void GWKeyboardAutoRepeat(gw_display_struct *display, Boolean b)
{
	if(display != NULL)
	    display->keyboard_autorepeat = b;
}


/*
 *      Returns a list of video modes, the number of video modes
 *      will be set to the storage pointer to by n.
 *
 *      Can return NULL on error or information not available.
 *
 *      The calling function needs to free() the returned pointer.
 */
gw_vidmode_struct *GWVidModesGet(
	gw_display_struct *display, int *n
)
{
	int i, j, total_modes = 0;
	Boolean is_dup;
	gw_vidmode_struct *mode = NULL, *m;
	DWORD dwFields;
	DEVMODE dmSettings;

	if(n != NULL)
	    *n = 0;

	dmSettings.dmSize = sizeof(DEVMODE);

	/* Iterate through all display devices */
	for(
	    i = 0;
	    EnumDisplaySettings(
		NULL,		// Current display device name
		i,		// Device number
		&dmSettings	// Return values
	    );
	    i++
	)
	{
	    /* Check for duplicate */
	    for(is_dup = False, j = 0; j < total_modes; j++)
	    {
		m = &mode[j];
		if((m->vwidth == (int)dmSettings.dmPelsWidth) &&
		   (m->vheight == (int)dmSettings.dmPelsHeight)
		)
		{
		    is_dup = True;
		    break;
		}
	    }
	    if(is_dup)
		continue;

	    /* Append a new video mode values structure */
	    total_modes++;
	    mode = (gw_vidmode_struct *)realloc(
		mode, total_modes * sizeof(gw_vidmode_struct)
	    );
	    if(mode == NULL)
	    {
		total_modes = 0;
		break;
	    }

	    /* Get pointer to the new video mode values structure */
	    m = &mode[total_modes - 1];

	    /* Get flags for which members in dmSettings are defined */
	    dwFields = dmSettings.dmFields;

	    /* Begin setting video mode values */
	    m->vwidth = (dwFields & DM_PELSWIDTH) ?
		dmSettings.dmPelsWidth : 0;
	    m->vheight = (dwFields & DM_PELSHEIGHT) ?
		dmSettings.dmPelsHeight : 0;
	    m->width = m->vwidth;
	    m->height = m->vheight;
	}

	/* Update returns */
	if(n != NULL)
	    *n = total_modes;

	return(mode);
}


/*
 *      Creates a new pointer cursor from the given RGBA data.
 */
void *GWCursorNew(
	gw_display_struct *display,
	int width, int height,  /* In pixels. */
	int hot_x, int hot_y,   /* In pixels. */
	int bpl,                /* Bytes per line, 0 for autocalc. */
	const void *data        /* RGBA data (4 bytes per pixel). */
)
{
	return(NULL);
}

/*
 *      Sets the pointer cursor for the current toplevel window in
 *      context.
 *
 *      The pointer cursor must be one that has been returned by
 *      GWCursorNew().
 */
void GWCursorSet(gw_display_struct *display, void *cursor_ptr)
{

}

/*
 *      Deletes a pointer cursor created by GWCursorNew().
 */
void GWCursorDelete(gw_display_struct *display, void *cursor_ptr)
{

}

/*
 *	Sets the pointer cursor to the one specified by cursor.
 */
void GWSetPointerCursor(   
	gw_display_struct *display, gw_pointer_cursor cursor
)
{
	void *cur;

	if(display == NULL)
	    return;

	switch(cursor)
	{
	  case GWPointerCursorStandard:
	    cur = display->cursor_standard;
	    break;
	  case GWPointerCursorBusy:
	    cur = display->cursor_busy;
	    break;
	  case GWPointerCursorText:
	    cur = display->cursor_text;
	    break;
	  case GWPointerCursorTranslate:
	    cur = display->cursor_translate;
	    break;
	  case GWPointerCursorZoom:
	    cur = display->cursor_zoom;
	    break;
	  case GWPointerCursorInvisible:
	    cur = NULL;
	    break;
	  default:
	    cur = display->cursor_standard;
	    break;
	}

	if(cur == NULL)
	{
	    display->cursor_current = NULL;
	    GWHideCursor(display);
	    return;
	}

	if(cur == display->cursor_current)
	    return;

	display->cursor_current = cur;
	SetCursor((HCURSOR)cur);
}

/*
 *	Shows the pointer cursor.
 */
void GWShowCursor(gw_display_struct *display)
{
	if(display == NULL)
	    return;

	if(!display->cursor_shown)
	{
	    ShowCursor(TRUE);
	    display->cursor_shown = True;
	}
}

/*
 *	Hides the pointer cursor.
 */
void GWHideCursor(gw_display_struct *display)
{
	if(display == NULL)
	    return;

	if(display->cursor_shown)
	{
	    ShowCursor(FALSE);
	    display->cursor_shown = False;
	}
}

/*
 *      Returns True if the cursor is shown.
 */
Boolean GWIsCursorShown(gw_display_struct *display)
{
	return((display != NULL) ? display->cursor_shown : False);
}

/*
 *	Sets pointer cursor as busy and ignores input.
 */
void GWSetInputBusy(gw_display_struct *display)
{
	if(!GWIsCursorShown(display))
	    return;

	GWSetPointerCursor(display, GWPointerCursorBusy);
}

/*
 *      Sets pointer cursor as ready and resumes allowing input.
 */
void GWSetInputReady(gw_display_struct *display)
{
	if(!GWIsCursorShown(display))
	    return;

	GWSetPointerCursor(display, GWPointerCursorStandard);
}

/*
 *      Set up gl projection matrix for 2d drawing.
 */
void GWOrtho2D(gw_display_struct *display)
{
	int ctx_num;
	const WRectangle *rect;


	if(display == NULL)
	    return;

	/* Get current gl context. */
	ctx_num = display->gl_context_num;
	if((ctx_num < 0) || (ctx_num >= display->total_gl_contexts))
	    return;

	/* Get size of current gl context. */
	rect = &display->toplevel_geometry[ctx_num];

	/* Set up gl projection matrix for 2d drawing. */
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(
	    0,          /* Left, right coordinate values. */
	    MAX(rect->width - 1, 1),
	    0,          /* Top, bottom coordinate values. */
	    MAX(rect->height - 1, 1)
	);      
	glMatrixMode(GL_MODELVIEW);   
	glLoadIdentity();
}

/*
 *      Set up gl projection matrix for 2d drawing with specific
 *      coordinates.
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

	/* Set up gl projection matrix for 2d drawing. */
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(
	    left, right,
	    bottom, top
	);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

#endif	/* __MSW__ */
