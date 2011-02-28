#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>

#ifdef __MSW__
# include <windows.h>
#else
# include <sys/time.h>
# include <unistd.h>
#endif

#include <GL/gl.h>
#include "../include/disk.h"
#include "../include/strexp.h"
#include "../include/string.h"
#include "gw.h"

#include "menu.h"
#include "messages.h"
#include "textinput.h"
#include "sarreality.h"
#include "obj.h"
#include "simmanage.h"
#include "weather.h"
#include "gctl.h"
#include "sound.h"
#include "mission.h"
#include "sar.h"
#include "sarsplash.h"
#include "sarinstall.h"
#include "sardraw.h"
#include "sardrawselect.h"
#include "sarkey.h"
#include "sarscreenshot.h"
#include "sartime.h"
#include "sarmusic.h"
#include "optionio.h"
#include "playerstatio.h"
#include "texturelistio.h"
#include "sceneio.h"
#include "sarmenuop.h"
#include "sarmenucb.h"
#include "sarmenubuild.h"
#include "sarmenumanage.h"
#include "sarmenucodes.h"
#include "sarsimend.h"
#include "config.h"

#include "fonts/6x10.fnt"
#include "fonts/7x14.fnt"
#include "fonts/banner.fnt"
#include "fonts/menu.fnt"


void SARHandleSignal(int s);

int SARInitGCTL(sar_core_struct *core_ptr);

void SARFullScreen(sar_core_struct *core_ptr);
void SARResolution(gw_display_struct *display, int width, int height);
void SARResolutionIncrease(gw_display_struct *display);
void SARResolutionDecrease(gw_display_struct *display);

int SARLoadProgressCB(void *ptr, long pos, long size);

void SARTextInputCBSendMessage(const char *value, void *data);
void SARTextInputCBQuitSimulation(const char *value, void *data);

void SARDrawCB(int ctx_num, void *ptr);
void SARKeyBoardCB(void *ptr, int c, Boolean state, unsigned long t);
void SARPointerCB(
	int ctx_num, void *ptr,
	int x, int y, gw_event_type type, int btn_num, unsigned long t
);
void SARReshapeCB(int ctx_num, void *ptr, int x, int y, int width, int height);
void SARVisibilityCB(int ctx_num, void *ptr, gw_visibility v);
void SARSaveYourselfCB(int ctx_num, void *ptr);
void SARCloseCB(int ctx_num, void *ptr, void *data);
void SARResetTimmersCB(sar_core_struct *core_ptr, time_t t_new);

sar_core_struct *SARInit(int argc, char **argv);
void SARManage(void *ptr);
void SARShutdown(sar_core_struct *core_ptr);


#define ATOI(s)		(((s) != NULL) ? atoi(s) : 0)
#define ATOL(s)		(((s) != NULL) ? atol(s) : 0)
#define ATOF(s)		(((s) != NULL) ? (float)atof(s) : 0.0f)
#define STRDUP(s)	(((s) != NULL) ? strdup(s) : NULL)

#define MAX(a,b)	(((a) > (b)) ? (a) : (b))
#define MIN(a,b)	(((a) < (b)) ? (a) : (b))
#define CLIP(a,l,h)	(MIN(MAX((a),(l)),(h)))
#define STRLEN(s)	(((s) != NULL) ? (int)strlen(s) : 0)
#define STRISEMPTY(s)	(((s) != NULL) ? (*(s) == '\0') : 1)

#define RADTODEG(r)	((r) * 180.0 / PI)
#define DEGTORAD(d)	((d) * PI / 180.0)


static int segfault_count;


sar_dname_struct dname;
sar_fname_struct fname;
sar_next_struct next;

float debug_value;

int runlevel;

time_t	cur_millitime,
	cur_systime,
	lapsed_millitime;

float	time_compensation,
	time_compression;


/*
 *	Records visibility of window.
 */
static int is_visible = 0;




/*
 *	Signal handler.
 */
void SARHandleSignal(int s)
{
#if defined(__MSW__)

#else
	switch(s)
	{
#ifdef SIGINT
	  case SIGINT:
	    runlevel = 1;
	    break;
#endif
#ifdef SIGTERM
	  case SIGTERM:
	    exit(0);
	    break;  
#endif
#ifdef SIGQUIT
	  case SIGQUIT:
	    exit(0);
	    break;
#endif
#ifdef SIGSEGV
	  case SIGSEGV:
	    fprintf(
		stderr,
		PROG_NAME_FULL " triggered a segmentation fault.\n"
	    );
	    exit(1);
	    break;
#endif
	}
#endif
}

/*
 *	(Re)initializes the Game Controller based on values from the
 *	current options.
 */
int SARInitGCTL(sar_core_struct *core_ptr)
{
	int i, total_joysticks;
	void *w, *rc;
	gctl_js_connection js_connection;
	gctl_struct *gctl;
	gctl_values_struct *v;
	gw_display_struct *display = core_ptr->display;
	const sar_option_struct *opt = &core_ptr->option;

	/* Shutdown Game Controller as needed */
	GCtlDelete(core_ptr->gctl);
	core_ptr->gctl = NULL;

	/* Get pointers to the Window and Rendering Context of the
	 * first GL Context, this is needed by the Game Controller
	 */
	if(GWContextGet(
	    display, GWContextCurrent(display),
	    &w, &rc,
	    NULL, NULL, NULL, NULL
	))
	    return(-1);

	/* Check how many joysticks are specified by the options by
	 * checking if axis roles defined for each joystick in question
	 *
	 * Any joystick with one or more axis roles set means the stick
	 * is enabled
	 */
	total_joysticks = 0;
	if(opt->gctl_js0_axis_roles != 0)
	    total_joysticks++;
	if(opt->gctl_js1_axis_roles != 0)
	    total_joysticks++;


	/* Set up Game Controller Values */
	v = GCTL_VALUES(calloc(1, sizeof(gctl_values_struct)));

	/* Controllers; Keyboard, Joystick, and/or Pointer */
	v->controllers = opt->gctl_controllers;

	/* Options (none at this time) */
	v->options = opt->gctl_options;

	/* Set up Game Controller Joystick Values for each joystick   
	 * that is enabled
	 */
	v->total_joysticks = total_joysticks;
	if(total_joysticks > 0)
	    v->joystick = (gctl_values_js_struct *)calloc(
		total_joysticks, sizeof(gctl_values_js_struct)
	    );
	else
	    v->joystick = NULL;
	/* Joystick #1 */
	i = 0;
	js_connection = opt->js0_connection;
	if((total_joysticks > i) && (opt->gctl_js0_axis_roles != 0))
	{
	    gctl_values_js_struct *js_v = &v->joystick[i];

	    js_v->priority = opt->js_priority;
	    switch(js_connection)
	    {
	      case GCTL_JS_CONNECTION_USB:
		js_v->device = STRDUP("/dev/input/js0");
		break;
	      case GCTL_JS_CONNECTION_STANDARD:
		js_v->device = STRDUP("/dev/js0");
		break;
	    }
	    js_v->connection = js_connection;
	    js_v->window = w;

	    js_v->axis_role = opt->gctl_js0_axis_roles;

	    js_v->button_rotate = opt->js0_btn_rotate;
	    js_v->button_air_brakes = opt->js0_btn_air_brakes;
	    js_v->button_wheel_brakes = opt->js0_btn_wheel_brakes;
	    js_v->button_zoom_in = opt->js0_btn_zoom_in;
	    js_v->button_zoom_out = opt->js0_btn_zoom_out;
	    js_v->button_hoist_up = opt->js0_btn_hoist_up;
	    js_v->button_hoist_down = opt->js0_btn_hoist_down;
	}
	/* Joystick #2 */
	i = 1;
	js_connection = opt->js1_connection;
	if((total_joysticks > i) && (opt->gctl_js1_axis_roles != 0))
	{
	    gctl_values_js_struct *js_v = &v->joystick[i];

	    js_v->priority = opt->js_priority;
	    switch(js_connection)
	    {
	      case GCTL_JS_CONNECTION_USB:
		js_v->device = STRDUP("/dev/input/js1");
		break;
	      case GCTL_JS_CONNECTION_STANDARD:
		js_v->device = STRDUP("/dev/js1");
		break;
	    }
	    js_v->connection = js_connection;
	    js_v->window = w;

	    js_v->axis_role = opt->gctl_js1_axis_roles;

	    js_v->button_rotate = opt->js1_btn_rotate;
	    js_v->button_air_brakes = opt->js1_btn_air_brakes;
	    js_v->button_wheel_brakes = opt->js1_btn_wheel_brakes;
	    js_v->button_zoom_in = opt->js1_btn_zoom_in;
	    js_v->button_zoom_out = opt->js1_btn_zoom_out;
	    js_v->button_hoist_up = opt->js1_btn_hoist_up;
	    js_v->button_hoist_down = opt->js1_btn_hoist_down;
	}


	/* Initialize the Game Controller */
	core_ptr->gctl = gctl = GCtlNew(v);

	/* Delete Game Controller Values */
	for(i = 0; i < v->total_joysticks; i++)
	{
	    gctl_values_js_struct *js_v = &v->joystick[i];
	    free(js_v->device);
	}
	free(v->joystick);
	free(v);


	/* Error initializing the Game Controller? */
	if(gctl == NULL)
	{
	    GWOutputMessage(
		core_ptr->display, GWOutputMessageTypeError,
		"Error Initializing Controller",
		GCtlGetError(),
"The above error was encountered when attempting\n\
to initialize the game controller. Please check\n\
if your controllers are set up properly or try\n\
a different game controller setting."
	    );
	    return(-1);
	}
	else
	{
	    return(0);
	}
}


/*
 *	Toggles the switching between full screen and GUI modes.
 */
void SARFullScreen(sar_core_struct *core_ptr)
{
	gw_display_struct *display = core_ptr->display;
	sar_option_struct *opt = &core_ptr->option;

	/* Currenty in full screen mode? */
	if(GWContextIsFullScreen(display))
	{
	    /* Currently in full screen mode, so switch back to GUI
	     * mode.
	     */
	    GWContextFullScreen(display, False);
	    opt->last_fullscreen = False;
	}
	else
	{
	    /* Switch to full screen mode, first trying with the
	     * current size of the GL context.
	     */
	    int status = GWContextFullScreen(display, True);
	    if(status == 0)
	    {
		/* Successfully switched to full screen mode */
		opt->last_fullscreen = True;
	    }
	    else if(status == -2)
	    {
		/* No full screen mode that matches the current size
		 * of the GL context, so use the next closest size.
		 *
		 * Here we will get a list of valid vidmode sizes (if
		 * any) and set the GL context to the size of the
		 * closest (equal or smaller vidmode size) and then
		 * switch to full screen.
		 */
		int i, n, width, height;
		gw_vidmode_struct	*vm = GWVidModesGet(display, &n),
					*vm_ptr;
		int nwidth = 0, nheight = 0;

		GWContextGet(
		    display, GWContextCurrent(display),
		    NULL, NULL,
		    NULL, NULL,
		    &width, &height
		);

		/* Iterate through vidmodes, finding one that is closest
		 * (equal or smaller) than the size of the GL context
		 * and store its size as nwidth and nheight.
		 */
		for(i = 0; i < n; i++)
		{
		    vm_ptr = &vm[i];
		    if((vm_ptr->width < width) &&
		       (vm_ptr->height < height)
		    )
		    {
			nwidth = MAX(vm_ptr->width, nwidth);
			nheight = MAX(vm_ptr->height, nheight);
		    }
		}
		GWVidModesFree(vm, n);

		/* If still unable to find useable video mode then
		 * use most conservitive resolution 320x240.
		 */
		if((nwidth == 0) || (nheight == 0))
		{
		    nwidth = 320;
		    nheight = 240;
		}
		/* Adjust context to new size that is hopefully suitable
		 * for full screen mode.
		 */
		GWContextSize(display, nwidth, nheight);

		/* Attempt to switch to full screen mode with the new
		 * context size.
		 */
		status = GWContextFullScreen(display, True);
		if(status == 0)
		{
		    /* Successfully switched to full screen mode */
		    opt->last_fullscreen = True;
		}
		else if(status == -2)
		{
		    /* Still no full screen mode that matches the
		     * current size, so print message.
		     */
		    int width, height;
		    char *buf = (char *)malloc(4 * 80 * sizeof(char));
		    GWContextGet(
			display, GWContextCurrent(display),
			NULL, NULL,
			NULL, NULL,
			&width, &height
		    );
		    sprintf(buf,
"Unable to find a suitable video mode to fit a full screen\n\
geometry of %ix%i.  Try adjusting the size of the window and\n\
verify that both your hardware and software can support that\n\
geometry.",
			width, height
		    );
		    GWOutputMessage(
			display,
			GWOutputMessageTypeWarning,
			"Full Screen Failed", buf, NULL
		    );
		    free(buf);
		}
	    }
	    else if(status == -5)
	    {
		/* Full screen is not supported */
		GWOutputMessage(
		    display,
		    GWOutputMessageTypeWarning,
		    "Full Screen Failed",
		    "Full screen is not supported.",
		    NULL
		);
	    }
	}
}

/*
 *	Sets the resolution/size of the window and GL frame buffer.
 *
 *	If there is no change in size from the current size then
 *	nothing will be done.
 */
void SARResolution(gw_display_struct *display, int width, int height)
{
	int x, y, owidth, oheight;

	/* Get current size and position */
	GWContextGet(
	    display, GWContextCurrent(display),
	    NULL, NULL,
	    &x, &y,
	    &owidth, &oheight
	);

	/* Is there a change in window size? */
	if((width != owidth) || (height != oheight))
	{
	    /* Adjust the window position coordinates so that the window
	     * stays in the center.
	     */
	    x += (owidth / 2) - (width / 2);
	    y += (oheight / 2) - (height / 2);

	    /* Make sure that the adjusted window position coordinates
	     * do not place the window completely off the root window.
	     */

	    /* Sanitize width */
	    if((display->root_width > 0) &&
	       ((x + width) > display->root_width)
	    )
		x = display->root_width - width;
	    if(x < 0)
		x = 0;
	    /* Sanitize height */
	    if((display->root_height > 0) &&
	       ((y + height) > display->root_height)
	    )
		y = display->root_height - height;
	    if(y < 0)
		y = 0;

	    /* Check if already in full screen mode */
	    if(GWContextIsFullScreen(display))
	    {
		/* Already in full screen mode, so just adjust the
		 * size and update the full screen mode.
		 */
		GWContextSize(display, width, height);
		GWContextFullScreen(display, True);
	    }
	    else
	    {
		/* In windowed mode, update both the size and the
		 * position.
		 */
		GWContextPosition(display, x, y);
		GWContextSize(display, width, height);
	    }
	}
}

/*
 *	Increases the resolution/size of the window and GL frame buffer
 *	to the "next" resolution/size.
 */
void SARResolutionIncrease(gw_display_struct *display)
{
	int width, height, new_width, new_height;

	GWContextGet(
	    display, GWContextCurrent(display),
	    NULL, NULL,
	    NULL, NULL,
	    &width, &height
	);

	/* 320x240 */
	if(width < 320)
	{
	    new_width = 320;
	    new_height = 240;
	}
	/* 640x480 */
	else if(width < 640)
	{
	    new_width = 640;
	    new_height = 480;
	}
	/* 800x600 */
	else if(width < 800)
	{
	    new_width = 800;
	    new_height = 600;
	}
	/* 1024x768 */
	else if(width < 1024)
	{
	    new_width = 1024;
	    new_height = 768;
	}
        else if (width < 1280)
        {
            new_width = 1280;
            new_height = 768;
        }
        else if (width < 1366)
        {
            new_width = 1366;
            new_height = 768;
        }
	/* 100x70 */
	else
	{
	    new_width = 100;
	    new_height = 70;
	}

	/* Set new resolution */
	SARResolution(display, new_width, new_height);
}

/*
 *	Decreases the resolution/size of the window and GL frame buffer
 *      to the "previous" resolution/size.
 */
void SARResolutionDecrease(gw_display_struct *display)
{
	int width, height, new_width, new_height;

	GWContextGet(
	    display, GWContextCurrent(display),
	    NULL, NULL,
	    NULL, NULL,
	    &width, &height
	);

        if (width > 1366)
        {
            new_width = 1280;
            new_height = 768;
        }
        else if (width > 1280)
        {
            new_width = 1366;
            new_height = 768;
        }
	/* 1024x768 */
	else if(width > 1024)
	{
	    new_width = 1024;
	    new_height = 768;
	}
	/* 800x600 */
	else if(width > 800)
	{
	    new_width = 800;
	    new_height = 600;
	}
	/* 640x480 */
	else if(width > 640)
	{
	    new_width = 640;
	    new_height = 480;
	}
	/* 320x240 */
	else if(width > 320)
	{
	    new_width = 320;
	    new_height = 240;
	}
	/* 100x70 */
	else
	{
	    new_width = 100;
	    new_height = 70;
	}

	/* Set new resolution */
	SARResolution(display, new_width, new_height);
}


/*
 *	Loading Simulation progress callback, this updates the progress
 *	bar in the Loading Simulation menu and redraws.
 *
 *	Returns 0 to indicate continue loading or non-zero to indicate
 *	abort load.
 *
 *	Given input ptr must be of type sar_progress_cb_struct.
 */
int SARLoadProgressCB(void *ptr, long pos, long size)
{
	int i;
	sar_core_struct *core_ptr;
	sar_menu_struct *m;
	void *o;
	gw_display_struct *display;
	sar_progress_cb_struct *cb_data = SAR_PROGRESS_CB(ptr);
	if(cb_data == NULL)
	    return(0);

	core_ptr = SAR_CORE(cb_data->core_ptr);
	if(core_ptr == NULL)
	    return(0);

	display = core_ptr->display;
	if(display == NULL)
	    return(0);

	/* Get pointer to the current menu */
	i = core_ptr->cur_menu;
	if((i >= 0) && (i < core_ptr->total_menus))
	    m = core_ptr->menu[i];
	else
	    m = NULL;
	if(m == NULL)
	    return(0);

	/* Make sure that we have a positive range to indicate a
	 * progress
	 */
	if((size > 0) && (cb_data->coeff_range > 0.0f))
	{
	    /* Update progress bar */
	    for(i = 0; i < m->total_objects; i++)
	    {
		o = m->object[i];
		if(SAR_MENU_IS_PROGRESS(o))
		{
		    GWManage(display);
		    SARMenuProgressSet(
			display, m, i,
			cb_data->coeff_offset +
			    ((float)pos / (float)size *
			    cb_data->coeff_range),
			True		/* Redraw */
		    );
		    break;
		}
	    }
	}

	return(0);
}

/*
 *	Tempory function to handle send message callback.
 */
void SARTextInputCBSendMessage(const char *value, void *data)
{
	sar_core_struct *core_ptr = SAR_CORE(data);
	if((value == NULL) || (core_ptr == NULL))
	    return;

	SARMessageAdd(core_ptr->scene, value);
}

/*
 *	Quick simulation text input callback.
 */
void SARTextInputCBQuitSimulation(const char *value, void *data)
{
	sar_core_struct *core_ptr = SAR_CORE(data);
	if((value == NULL) || (core_ptr == NULL))
	    return;

	if(toupper(*value) == 'Y')
	    SARSimEnd(core_ptr);
}

/*
 *	Redraw callback.
 */
void SARDrawCB(int ctx_num, void *ptr)
{
	int cur_menu;
	sar_core_struct *core_ptr = SAR_CORE(ptr);
	if(core_ptr == NULL)
	    return;

	/* Get current menu number */
	cur_menu = core_ptr->cur_menu;

	/* Check if current menu is valid, if it is then that
	 * implies we are in the menus.
	 */
	if(SARIsMenuAllocated(core_ptr, cur_menu))
	{
	    /* Draw menus */
	    SARMenuDrawAll(
		core_ptr->display,
		core_ptr->menu[cur_menu]
	    );
	    SARTextInputDraw(core_ptr->text_input);
	    GWSwapBuffer(core_ptr->display);
	}
	else
	{
	    /* Draw scene */
	    SARDraw(core_ptr);
	}
}

/*
 *	Keyboard callback.
 */
void SARKeyBoardCB(void *ptr, int c, Boolean state, unsigned long t)
{
	sar_core_struct *core_ptr = SAR_CORE(ptr);
	gw_display_struct *display;
	sar_menu_struct *menu_ptr;
	Boolean alt_key_state, ctrl_key_state, shift_key_state;
	if(core_ptr == NULL)
	    return;

	display = core_ptr->display;
	if(display == NULL)
	    return;

	alt_key_state = display->alt_key_state;
	ctrl_key_state = display->ctrl_key_state;
	shift_key_state = display->shift_key_state;

	/* First check to see if the text input prompt is mapped by
	 * checking if the text input callback function is set (not
	 * NULL).
	 */
	if(SARTextInputIsMapped(core_ptr->text_input))
	{
	    text_input_struct *text_input = core_ptr->text_input;

	    /* Pass key event to text input prompt handler */
	    SARTextInputHandleKey(text_input, c, state);

	    /* If we are in the menus then we need to redraw */
	    if(SARGetCurrentMenuPtr(core_ptr) != NULL)
	    {
		SARTextInputDraw(text_input);
		GWSwapBuffer(display);
	    }

	    /* Return, do not continue. This is so that the key event
	     * is not passed to any other key event handlers.
	     */
	    return;
	}


	/* Intercept special key combinations here before going on to
	 * to regular key handling, if any keys are handled here then
	 * they will not be passed to the regular key handler
	 */
	/* Toggle full screen */
	if((c == GWKeyF11) && ctrl_key_state)
	{
	    if(!state)
		SARFullScreen(core_ptr);
	    return;
	}
	/* Increase resolution */
	if((c == ']') && ctrl_key_state)
	{
	    if(!state)
		SARResolutionIncrease(display);
	    return;
	}
	/* Decrease resolution */
	if((c == '[') && ctrl_key_state)
	{
	    if(!state)
		SARResolutionDecrease(display);
	    return;
	}
	/* Screen shot */
#ifdef __MSW__
	if((c == 'c') && ctrl_key_state)
#else
	if(((c == 'c') && ctrl_key_state) || (c == GWKeySysReq))
#endif
	{
	    if(!state)
		SARScreenShot(core_ptr, NULL, 2);
	    return;
	}

	/* Get current selected menu (if any).  If there is no menu
	 * currently selected then call simulation key handler,
	 * otherwise call menu key handler.
	 */
	menu_ptr = SARGetCurrentMenuPtr(core_ptr);
	if(menu_ptr == NULL)
	{
	    /* In simulation ,so pass key event to simulation key
	     * handler.  Note that this function will not post a
	     * redraw.
	     */
	    SARKey(core_ptr, c, state, t); 
	}
	else
	{
	    /* A menu is selected, call menu key handler */
	    SARMenuManageKey(core_ptr->display, menu_ptr, c, state);
	}
}

/*
 *	Pointer callback.
 */
void SARPointerCB(
	int ctx_num, void *ptr,
	int x, int y, gw_event_type type, int btn_num, unsigned long t
)
{
	sar_core_struct *core_ptr = SAR_CORE(ptr);
	gw_display_struct *display;
	sar_menu_struct *menu_ptr;
	if(core_ptr == NULL)
	    return;

	display = core_ptr->display;
	if(display == NULL)
	    return;

	/* First check to see if the text input prompt is mapped, if
	 * it is then the event should be passed to it
	 */
	if(SARTextInputIsMapped(core_ptr->text_input))
	{
	    text_input_struct *text_input = core_ptr->text_input;

	    /* Pass poionter event to text input prompt handler */
	    SARTextInputHandlePointer(
		text_input, x, y, type, btn_num
	    );

	    /* If we are in the menus then we need to redraw */
	    if(SARGetCurrentMenuPtr(core_ptr) != NULL)
	    {
		SARTextInputDraw(text_input);
		GWSwapBuffer(display);
	    }

	    /* Done handling the event */
	    return;
	}

	/* Get current menu (if any) */
	menu_ptr = SARGetCurrentMenuPtr(core_ptr);
	if(menu_ptr == NULL)
	{
	    /* There is no current menu so it implies we are in
	     * simulation, so forward the event to game controller
	     */
	    GCtlHandlePointer(
		display, core_ptr->gctl,
		type, btn_num,
		x, y, t, lapsed_millitime
	    );
	}
	else
	{
	    /* There is a current menu so Forward the event to the
	     * menu pointer event handler
	     */
	    SARMenuManagePointer(
		display, menu_ptr,
		x, y, type, btn_num
	    );
	}
}

/*
 *	Resize callback, updates the last size of the toplevel window.
 *
 *	If width or height are less than 1 the last glViewport() setting
 *	for width or height will be called again.
 */
void SARReshapeCB(int ctx_num, void *ptr, int x, int y, int width, int height)
{
	sar_core_struct *core_ptr = SAR_CORE(ptr);
	gw_display_struct *display;
	sar_option_struct *opt;

	if(core_ptr == NULL)
	    return;

	display = core_ptr->display;
	if(display == NULL)
	    return;

	opt = &core_ptr->option;

	/* Change width and height if specified */
	if((width > 0) && (height > 0))
	{
	    /* Update last positions of toplevel window on global
	     * options
	     */
	    opt->last_x = x;
	    opt->last_y = y;
	    opt->last_width = width;
	    opt->last_height = height;
	}
	else
	{
	    /* If this function is called with width and height not
	     * positive, then it means it was called synthetically and
	     * that we should use the current size of the context on the
	     * display structure
	     */
	    GWContextGet(
		display, ctx_num,
		NULL, NULL,
		NULL, NULL,
		&width, &height
	    );
	}

	/* Set new view port dimensions */
	if((width > 0) && (height > 0))
	    glViewport(0, 0, width, height);
}


/*
 *	Visibility change callback.
 */
void SARVisibilityCB(int ctx_num, void *ptr, gw_visibility v)
{
	switch(v)
	{
	  case GWVisibilityFullyObscured:
	    is_visible = 0;
	    break;
	  default:
	    is_visible = 1;
	    break;
	}
}

/*
 *	Save yourself callback.
 */
void SARSaveYourselfCB(int ctx_num, void *ptr)
{
	sar_core_struct *core_ptr = SAR_CORE(ptr);
	if(core_ptr == NULL)
	    return;



}

/*
 *	Close window callback.
 */
void SARCloseCB(int ctx_num, void *ptr, void *data)
{
	sar_core_struct *core_ptr = SAR_CORE(ptr);
	if(core_ptr == NULL)
	    return;

	/* Check if currently In a menu, implying that we are currently
	 * in the menu system and not in simulation.
	 */
	if(core_ptr->cur_menu > -1)
	{
	    /* Switch global runlevel to 1, causing the program to begin
	     * shutting down.
	     */
	    runlevel = 1;
	}
	else
	{
	    /* Currently in simulation, so end simulation instead of
	     * switching to runlevel 1.
	     *
	     * First check if the user is already being prompted for,
	     * exit.  If the user is then end simulation, otherwise
	     * prompt for exit.
	     */
	    if(SARTextInputIsMapped(core_ptr->text_input))
	    {
		SARTextInputUnmap(core_ptr->text_input);
		SARSimEnd(core_ptr);
	    }
	    else
	    {
		if(core_ptr->mission != NULL)
		{
		    /* Same as end simulation, just prompt text different */
		    SARTextInputMap(
			core_ptr->text_input,
			"Are you sure you want to abort the mission?",
			NULL,
			SARTextInputCBQuitSimulation,
			core_ptr
		    );
		}
		else
		{
		    SARTextInputMap(
			core_ptr->text_input,
			"Are you sure you want to quit simulation?",
			NULL,
			SARTextInputCBQuitSimulation,
			core_ptr
		    );
		}
	    }
	}
}

/*
 *	Resets global timmers to zero and updates the global
 *	variable cur_millitime to the value of t_new.
 *
 *	Global variable lapsed_millitime will be reset to 0 and
 *	time_compensation will be set to 1.0.
 *
 *	Timings on core structure, scene, objects, and other related
 *	resources will also be reset.
 */
void SARResetTimmersCB(sar_core_struct *core_ptr, time_t t_new)
{
	int i;
	sar_scene_struct *scene;
	sar_object_struct *obj_ptr;
	sar_object_aircraft_struct *obj_aircraft_ptr;
	sar_object_ground_struct *obj_ground_ptr;
	sar_object_human_struct *obj_human_ptr;
	sar_object_smoke_struct *obj_smoke_ptr;
	sar_object_fire_struct *obj_fire_ptr;
	sar_object_explosion_struct *obj_explosion_ptr;
	sar_mission_struct *mission = NULL;


	/* Set global variable cur_millitime to t_new */
	cur_millitime = t_new;

	/* Reset lapsed and time compensation */
	lapsed_millitime = 0l;
	time_compensation = 1.0f;


	/* Update random seed */
	srand((unsigned int)cur_millitime);


	/* Reset global next timmers */
	memset(&next, 0x00, sizeof(sar_next_struct));


	/* Reset game controller timmers */
	GCtlResetTimmers(core_ptr->gctl);

	/* Reset timings on scene */
	scene = core_ptr->scene;
	if(scene != NULL)
	{
	    sar_cloud_bb_struct *cloud_bb;

	    scene->message_display_until = 0l;
	    scene->camera_ref_title_display_until = 0l;

	    /* Cloud `billboard' objects lightening timmers */
	    for(i = 0; i < scene->total_cloud_bbs; i++)
	    {
		cloud_bb = scene->cloud_bb[i];
		if(cloud_bb == NULL)
		    continue;

		cloud_bb->lightening_next_on = 0l;
		cloud_bb->lightening_next_off = 0l;
		cloud_bb->lightening_started_on = 0l;
	    }
	}

	/* Reset timings on mission */
	mission = core_ptr->mission;
	if(mission != NULL)
	{
	    mission->next_check = 0l;
	    mission->next_log_position = 0l;
	}


	/* Reset timmers on objects */
	for(i = 0; i < core_ptr->total_objects; i++)
	{
	    obj_ptr = core_ptr->object[i];
	    if(obj_ptr == NULL)
		continue;

	    /* Any object with a defined life span needs to die whenever
	     * timmers are reset
	     *
	     * Note that we need to reset the value to 1 instead of 0
	     * since 0 means that the object has an infinate life span
	     * (never dies)
	     */
	    if(obj_ptr->life_span > 0l)
		obj_ptr->life_span = 1l;

	    switch(obj_ptr->type)
	    {
	      case SAR_OBJ_TYPE_GARBAGE:
	      case SAR_OBJ_TYPE_STATIC:
	      case SAR_OBJ_TYPE_AUTOMOBILE:
	      case SAR_OBJ_TYPE_WATERCRAFT:
		break;
	      case SAR_OBJ_TYPE_AIRCRAFT:
		obj_aircraft_ptr = SAR_OBJ_GET_AIRCRAFT(obj_ptr);
		if(obj_aircraft_ptr != NULL)
		{
		    obj_aircraft_ptr->next_engine_on = 0l;
		}
		break;
	      case SAR_OBJ_TYPE_GROUND:
		obj_ground_ptr = SAR_OBJ_GET_GROUND(obj_ptr);
		break;
	      case SAR_OBJ_TYPE_RUNWAY:
	      case SAR_OBJ_TYPE_HELIPAD:
		break;
	      case SAR_OBJ_TYPE_HUMAN:
		obj_human_ptr = SAR_OBJ_GET_HUMAN(obj_ptr);
		break;
	      case SAR_OBJ_TYPE_SMOKE:
		obj_smoke_ptr = SAR_OBJ_GET_SMOKE(obj_ptr);
		if(obj_smoke_ptr != NULL)
		{
		    obj_smoke_ptr->respawn_next = 0l;
		}
		break;
	      case SAR_OBJ_TYPE_FIRE:
		obj_fire_ptr = SAR_OBJ_GET_FIRE(obj_ptr);
		if(obj_fire_ptr != NULL)
		{
		    obj_fire_ptr->next_frame_inc = 0l;
		}
		break;
	      case SAR_OBJ_TYPE_EXPLOSION:
		obj_explosion_ptr = SAR_OBJ_GET_EXPLOSION(obj_ptr);
		if(obj_explosion_ptr != NULL)
		{
		    obj_explosion_ptr->next_frame_inc = 0l;
		}
		break;
	      case SAR_OBJ_TYPE_CHEMICAL_SPRAY:
	      case SAR_OBJ_TYPE_FUELTANK:
	      case SAR_OBJ_TYPE_PREMODELED:
		break;
	    }

	    /* Lights */
	    if(obj_ptr->light != NULL)
	    {
		int n;
		sar_light_struct *light;

		for(n = 0; n < obj_ptr->total_lights; n++)
		{
		    light = obj_ptr->light[n];
		    if(light == NULL)
			continue;

		    light->next_off = 0l;
		    light->next_on = light->int_delay_on;
		}
	    }

/* Add other object common timing values that need to be reset here */

	}
}


/*
 *	SAR initialize.
 */
sar_core_struct *SARInit(int argc, char **argv)
{
	int i, strc;
	const char *s, *arg;
	char **strv;
#ifndef __MSW__
	const char *s2;
#endif
#ifdef __MSW__
	HINSTANCE hInst = GetModuleHandle(NULL);
#endif
	sar_color_struct *color;

	sar_core_struct *core_ptr;
	gw_display_struct *dpy;
	sar_option_struct *opt;

	const char	*display_address = NULL;
	const char	*font_name = NULL;
	Boolean		cl_direct_rendering = True;
	int		cl_fullscreen = 0;
        int             allow_key_repeat = True;
	float		aspect_offset = 0.0f;
	Boolean		startup_no_sound = False;
	const char	*sound_server_connect_arg = NULL;

	Boolean notify_install_local_error = False,
		notify_install_local_success = False;

	char cwd[PATH_MAX];
	char tmp_path[PATH_MAX + NAME_MAX];
	char geometry_string[GW_GEOMETRY_STRING_MAX];


	/* Set signal callbacks */
	signal(SIGINT, SARHandleSignal); 
	signal(SIGTERM, SARHandleSignal);
	signal(SIGSEGV, SARHandleSignal);

	/* Seed randon number generator */
	srand((unsigned int)time(NULL));

	/* Get current working directory */
	if(getcwd(cwd, PATH_MAX) != NULL)
	    cwd[PATH_MAX - 1] = '\0';
	else
	    strcpy(cwd, "/");

	*geometry_string = '\0';


	/* Reset globals */
	cur_millitime = 0;
	lapsed_millitime = 0;
	time_compensation = 1.0f;
	time_compression = 1.0f;


	/* Allocate core structure */
	core_ptr = SAR_CORE(calloc(1, sizeof(sar_core_struct)));
	if(core_ptr == NULL)
	    return(NULL);

	opt = &core_ptr->option;

	/* Reset core structure values */
	core_ptr->prog_file = NULL;
	core_ptr->prog_file_full_path = NULL;

	core_ptr->stop_count = 0;

	core_ptr->display = NULL;
	core_ptr->recorder_address = NULL;
	core_ptr->recorder = NULL;
	core_ptr->audio_mode_name = NULL;
	core_ptr->gctl = NULL;

	core_ptr->cur_music_id = -1;
	core_ptr->music_ref = NULL;
	core_ptr->total_music_refs = 0;
	core_ptr->human_data = NULL;
	core_ptr->weather_data = NULL;

	core_ptr->scene = NULL;
	core_ptr->mission = NULL;
	core_ptr->object = NULL;
	core_ptr->total_objects = 0;

	core_ptr->menu = NULL;
	core_ptr->total_menus = 0;
	core_ptr->cur_menu = 0;
/* Bunch of menu images that we arn't resetting, but we can get away
 * with not resetting them.
 */

	core_ptr->texture_list = NULL;
	core_ptr->total_texture_list = 0;

	core_ptr->player_stat = NULL;
	core_ptr->total_player_stats = 0;

	core_ptr->cur_mission_file = NULL;
	core_ptr->cur_player_model_file = NULL;
	core_ptr->cur_scene_file = NULL;

	core_ptr->display_help = 0;

	core_ptr->flir = False;

	core_ptr->drawmap_objname = NULL;
	core_ptr->total_drawmap_objnames = 0;
	core_ptr->drawmap_ghc_result = 0.0f;

	core_ptr->text_input = NULL;

	memset(opt, 0x00, sizeof(sar_option_struct));
	memset(&core_ptr->fps, 0x00, sizeof(sar_fps_struct));


	/* Reset options to defaults */
	opt->menu_backgrounds = True;
	opt->menu_change_affects = True;
	opt->menu_always_full_redraw = False;
	opt->console_quiet = False;
	opt->internal_debug = False;
	opt->runtime_debug = False;
	opt->prioritize_memory = False;
	opt->units = SAR_UNITS_ENGLISH;

	opt->textured_ground = True;
	opt->textured_clouds = True;
	opt->textured_objects = True;
	opt->atmosphere = True;
	opt->dual_pass_depth = True;
	opt->prop_wash = True;
	opt->smoke_trails = True;
	opt->celestial_objects = True;
	opt->gl_polygon_offset_factor = -1.9f;
	opt->gl_shade_model = GL_SMOOTH;
	opt->visibility_max = 4;
	opt->rotor_blur_style = SAR_ROTOR_BLUR_CLOCK_RADIAL;
	opt->graphics_acceleration = 0.0f;

	opt->engine_sounds = False;
	opt->event_sounds = False;
	opt->voice_sounds = False;
	opt->music = False;

	opt->sound_priority = SND_PRIORITY_BACKGROUND;

	opt->system_time_free_flight = False;

	color = &opt->hud_color;
	color->a = 1.0f;
	color->r = 1.0f;
	color->g = 1.0f;
	color->b = 1.0f;

	color = &opt->message_color;
	color->a = 1.0f;
	color->r = 1.0f;
	color->g = 1.0f;
	color->b = 1.0f;

	opt->show_hud_text = True;
	opt->show_outside_text = True;

	opt->explosion_frame_int = SAR_DEF_EXPLOSION_FRAME_INT;
	opt->splash_frame_int = SAR_DEF_SPLASH_FRAME_INT;
	opt->crash_explosion_life_span = SAR_DEF_CRASH_EXPLOSION_LIFE_SPAN;
	opt->fuel_tank_life_span = SAR_DEF_FUEL_TANK_LIFE_SPAN;

	opt->rotor_wash_vis_coeff = (float)SAR_DEF_ROTOR_WASH_VIS_COEFF;

	opt->gctl_controllers = GCTL_CONTROLLER_KEYBOARD |
	    GCTL_CONTROLLER_POINTER;
	opt->gctl_options = 0;
	opt->js_priority = GCTL_JS_PRIORITY_BACKGROUND;

	opt->js0_connection = GCTL_JS_CONNECTION_STANDARD;
	opt->js1_connection = GCTL_JS_CONNECTION_STANDARD;

	opt->js0_btn_rotate = 3;
	opt->js0_btn_air_brakes = 6;
	opt->js0_btn_wheel_brakes = 0;
	opt->js0_btn_zoom_in = 2;
	opt->js0_btn_zoom_out = 1;
	opt->js0_btn_hoist_up = 5;
	opt->js0_btn_hoist_down = 4;

	opt->js1_btn_rotate = 3;
	opt->js1_btn_air_brakes = 6;
	opt->js1_btn_wheel_brakes = 0;
	opt->js1_btn_zoom_in = 2;
	opt->js1_btn_zoom_out = 1;
	opt->js1_btn_hoist_up = 5;
	opt->js1_btn_hoist_down = 4;
/*
	opt->gctl_js0_axis_roles = GCTL_JS_AXIS_ROLE_PITCH |
	    GCTL_JS_AXIS_ROLE_BANK;
 */
	opt->gctl_js0_axis_roles = 0;
	opt->gctl_js1_axis_roles = 0;
/*
	opt->gctl_js1_axis_roles = 
GCTL_JS_AXIS_ROLE_AS_THROTTLE_AND_RUDDER;
 */

	opt->hoist_contact_expansion_coeff = 1.0f;
	opt->damage_resistance_coeff = 1.0f;
	opt->flight_physics_level = FLIGHT_PHYSICS_REALISTIC;	/* Make it hard */

	opt->last_selected_player = 0;
	opt->last_selected_mission = 0;
	opt->last_selected_ffscene = 0;
	opt->last_selected_ffaircraft = 0;
	opt->last_selected_ffweather = 0;
	opt->last_x = 0;
	opt->last_y = 0;
	opt->last_width = SAR_DEF_WINDOW_WIDTH;
	opt->last_height = SAR_DEF_WINDOW_HEIGHT;
	opt->last_fullscreen = False;


	/* Set local game directory */
#ifdef __MSW__
	strncpy(dname.local_data, cwd, PATH_MAX);
#else
	s = getenv("HOME");
	if(s == NULL)
	    s = cwd;
	s2 = PrefixPaths(s, SAR_DEF_LOCAL_DATA_DIR);
	strncpy(
	    dname.local_data,
	    (s2 != NULL) ? s2 : cwd,
	    PATH_MAX
	);
#endif
	dname.local_data[PATH_MAX - 1] = '\0';

	/* Set global game directory */
#if defined(__MSW__)
	strncpy(dname.global_data, cwd, PATH_MAX);
#else
	s = getenv(SAR_DEF_ENV_GLOBAL_DIR);
	if(s != NULL)
	{
	    struct stat stat_buf;

	    if(!ISPATHABSOLUTE(s))
		fprintf(
		    stderr,
"Warning: Environment variable \"%s\" value \"%s\" must be an absolute path.\n",
		    SAR_DEF_ENV_GLOBAL_DIR, s
		);
	    if(stat(s, &stat_buf))
		fprintf(
		    stderr,
"Warning: Environment variable \"%s\" value \"%s\" reffers to a non-existant object.\n",
		    SAR_DEF_ENV_GLOBAL_DIR, s
		);
	}
	strncpy(
	    dname.global_data,
	    (s != NULL) ? s : SAR_DEF_GLOBAL_DATA_DIR,
	    PATH_MAX
	);
#endif
	dname.global_data[PATH_MAX - 1] = '\0';

	/* Set default configuration file */
	s = PrefixPaths(dname.local_data, SAR_DEF_OPTIONS_FILE);
	strncpy(
	    fname.options,
	    (s != NULL) ? s : SAR_DEF_OPTIONS_FILE,
	    PATH_MAX + NAME_MAX
	);
	fname.options[PATH_MAX + NAME_MAX - 1] = '\0';

	/* Set default global human data presets file */
	s = PrefixPaths(dname.global_data, SAR_DEF_HUMAN_FILE);
	strncpy(
	    fname.human,
	    (s != NULL) ? s : SAR_DEF_HUMAN_FILE,
	    PATH_MAX + NAME_MAX
	);
	fname.human[PATH_MAX + NAME_MAX - 1] = '\0';

	/* Set default global music data presets file */
	s = PrefixPaths(dname.global_data, SAR_DEF_MUSIC_FILE);
	strncpy(
	    fname.music,
	    (s != NULL) ? s : SAR_DEF_MUSIC_FILE,
	    PATH_MAX + NAME_MAX
	);
	fname.music[PATH_MAX + NAME_MAX - 1] = '\0';

	/* Set default global textures list file */
	s = PrefixPaths(dname.global_data, SAR_DEF_TEXTURES_FILE);
	strncpy(
	    fname.textures,
	    (s != NULL) ? s : SAR_DEF_TEXTURES_FILE,
	    PATH_MAX + NAME_MAX
	);
	fname.textures[PATH_MAX + NAME_MAX - 1] = '\0';

	/* Set default players list file */
	s = PrefixPaths(dname.local_data, SAR_DEF_PLAYERS_FILE);
	strncpy(
	    fname.players,
	    (s != NULL) ? s : SAR_DEF_PLAYERS_FILE,
	    PATH_MAX + NAME_MAX
	);
	fname.players[PATH_MAX + NAME_MAX - 1] = '\0';

	/* Set default global weather data presets file */
	s = PrefixPaths(dname.global_data, SAR_DEF_WEATHER_FILE);
	strncpy(
	    fname.weather,
	    (s != NULL) ? s : SAR_DEF_WEATHER_FILE,
	    PATH_MAX + NAME_MAX
	);
	fname.weather[PATH_MAX + NAME_MAX - 1] = '\0';

	/* Set default path to mission log file */
	s = PrefixPaths(dname.local_data, SAR_DEF_MISSION_LOG_FILE);
	strncpy(
	    fname.mission_log,
	    (s != NULL) ? s : SAR_DEF_MISSION_LOG_FILE,
	    PATH_MAX + NAME_MAX
	);
	fname.mission_log[PATH_MAX + NAME_MAX - 1] = '\0';


	/* Parse arguments */
#ifdef __MSW__
	/* Win32 parse arguments starting from index 0 */
	for(i = 0; i < argc; i++)
#else
	for(i = 1; i < argc; i++)
#endif
	{
	    arg = argv[i];
	    if(arg == NULL)
		 continue;

	    /* Run time debug */
	    if(!strcasecmp(arg, "--runtime-debug") ||
	       !strcasecmp(arg, "-runtime-debug") ||
	       !strcasecmp(arg, "--runtime_debug") ||
	       !strcasecmp(arg, "-runtime_debug")
	    )
	    {
		opt->runtime_debug = True;
	    }
	    /* Help */
	    else if(!strcasecmp(arg, "--help") ||
		    !strcasecmp(arg, "-help") ||
                    !strcasecmp(arg, "-h") ||
		    !strcasecmp(arg, "-?") ||
		    !strcasecmp(arg, "/h") ||
		    !strcasecmp(arg, "/?") ||
		    !strcasecmp(arg, "?")
	    )
	    {
		/* Print help message */
		printf(PROG_USAGE_MESG);
		return(NULL);
	    }
	    /* Version */
	    else if(!strcasecmp(arg, "--version") ||
		    !strcasecmp(arg, "-version")
	    )
	    {
		/* Print program version and copyright */
		printf(
		    PROG_NAME_FULL " Version " PROG_VERSION "\n"
		    PROG_COPYRIGHT "\n"
		);
		return(NULL);
	    }
	    /* Configuration File */
	    else if(!strcasecmp(arg, "--config") ||
		    !strcasecmp(arg, "-config") ||
		    !strcasecmp(arg, "--rcfile") ||
		    !strcasecmp(arg, "-rcfile") ||
		    !strcasecmp(arg, "-f")
	    )
	    {
		i++;
		arg = (i < argc) ? argv[i] : NULL;
		if(arg != NULL)
		{
		    const char *path = PathSubHome(arg);
		    if(path == NULL)
			path = arg;
		    strncpy(tmp_path, path, PATH_MAX + NAME_MAX);
		    tmp_path[PATH_MAX + NAME_MAX - 1] = '\0';

		    if(!ISPATHABSOLUTE(tmp_path))
		    {
			path = PrefixPaths(cwd, tmp_path);
			if(path != NULL)
			{
			    strncpy(tmp_path, path, PATH_MAX + NAME_MAX);
			    tmp_path[PATH_MAX + NAME_MAX - 1] = '\0';
			}
		    }

		    strncpy(fname.options, tmp_path, PATH_MAX + NAME_MAX);
		    fname.options[PATH_MAX + NAME_MAX - 1] = '\0';
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
	    /* Control */
	    else if(!strcasecmp(arg, "--control") ||
		    !strcasecmp(arg, "-control") ||
		    !strcasecmp(arg, "-c")
	    )
	    {
		i++;
		arg = (i < argc) ? argv[i] : NULL;
		if(arg != NULL)
		{
		    const char *controller = arg;

		    /* Joystick */
		    if(strcasepfx(controller, "j"))
			opt->gctl_controllers = GCTL_CONTROLLER_KEYBOARD |
			    GCTL_CONTROLLER_POINTER | GCTL_CONTROLLER_JOYSTICK;
		    /* Keyboard */
		    else if(strcasepfx(controller, "k"))
			opt->gctl_controllers = GCTL_CONTROLLER_KEYBOARD |
			    GCTL_CONTROLLER_POINTER;
		    else
			fprintf(
			    stderr,
			    "%s: Unsupported argument `%s'\n",
			    argv[i - 1], controller
			);
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
	    /* Software rendering? */
	    else if(!strcasecmp(arg, "--software_rendering") ||
		    !strcasecmp(arg, "-software_rendering") ||
		    !strcasecmp(arg, "--software-rendering") ||
		    !strcasecmp(arg, "-software-rendering") ||
		    !strcasecmp(arg, "--softwarerendering") ||
		    !strcasecmp(arg, "-softwarerendering") ||
		    !strcasecmp(arg, "--software") ||
		    !strcasecmp(arg, "-software")
	    )
	    {
		cl_direct_rendering = False;
	    }
	    /* Always Full Menu Redraws */
	    else if(!strcasecmp(arg, "--full_menu_redraw") ||
		    !strcasecmp(arg, "--full-menu-redraw") ||   
		    !strcasecmp(arg, "-full_menu_redraw") ||
		    !strcasecmp(arg, "-full-menu-redraw")
	    )
	    {
		opt->menu_always_full_redraw = True;
	    }
	    /* Display (connection to X server) */
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
	    /* Full screen */
	    else if(!strcasecmp(arg, "--full_screen") ||
		    !strcasecmp(arg, "-full_screen") ||
		    !strcasecmp(arg, "--fullscreen") ||
		    !strcasecmp(arg, "-fullscreen")
	    )
	    {
		cl_fullscreen = 1;
	    }
            else if ( (!strcasecmp(arg, "--window") ) ||
                      (!strcasecmp(arg, "-window") ) )
            {
                cl_fullscreen = 2;
            }
            else if ( (!strcasecmp(arg, "--no-keyrepeat") ) ||
                      (!strcasecmp(arg, "--no-autorepeat") ) )
            {
                 allow_key_repeat = False;
            }
 
	    /* Font (font name for XFonts, X only) */
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
	    /* Geometry (position and size of toplevel window) */
	    else if(!strcasecmp(arg, "--geometry") ||
		    !strcasecmp(arg, "-geometry")
	    )
	    {
		i++;
		arg = (i < argc) ? argv[i] : NULL;
		if(arg != NULL)
		{
		    strncpy(geometry_string, arg, GW_GEOMETRY_STRING_MAX);
		    geometry_string[GW_GEOMETRY_STRING_MAX - 1] = '\0';
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
	    /* Aspect offset */
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
	    /* Recorder */
	    else if(!strcasecmp(arg, "--recorder") ||
		    !strcasecmp(arg, "-recorder")
	    )
	    {
		i++;
		arg = (i < argc) ? argv[i] : NULL;
		if(arg != NULL)
		{
		    sound_server_connect_arg = arg;
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
	    /* No sound */
	    else if(!strcasecmp(arg, "--no_sound") ||
		    !strcasecmp(arg, "--nosound") ||
		    !strcasecmp(arg, "-no_sound") ||
		    !strcasecmp(arg, "-nosound")
	    )
	    {
		startup_no_sound = True;
	    }
	    /* No menu backgrounds */
	    else if(!strcasecmp(arg, "--nomenubackgrounds") ||
		    !strcasecmp(arg, "--nomenubackground") ||
		    !strcasecmp(arg, "--nomenubkg") ||
		    !strcasecmp(arg, "--nomenubg") ||
		    !strcasecmp(arg, "-nomenubackgrounds") ||
		    !strcasecmp(arg, "-nomenubackground") ||
		    !strcasecmp(arg, "-nomenubkg") ||
		    !strcasecmp(arg, "-nomenubg")
	    )
	    {
		opt->menu_backgrounds = False;
	    }

	}


	/* Check if program has been installed globally.
	 * Win32 note this will be the same as installed locally.
	 */
	if(!SARIsInstalledGlobal(&dname, &fname))
	{
	    /* Not globally installed so it is a critical error */
	    fprintf(
		stderr,
PROG_NAME_FULL " was unable to find the data files in:\n\
\n\
    %s\n",
		dname.global_data
	    );
	    fprintf(
		stderr,
"\nPlease verify that you have installed the program properly. If you \
have installed the global data files in a non-standard location, \
then you should set the environment variable \"%s\" to refer to that \
non-standard location (with an absolute path).\n",
		SAR_DEF_ENV_GLOBAL_DIR
	    );
	    free(core_ptr);
	    return(NULL);
	}
	/* Check if program is installed locally */
	if(!SARIsInstalledLocal(&dname, &fname))
	{
	    /* Not locally installed, so install from global */
	    if(SARDoInstallLocal(&dname, &fname, opt))
	    {
		fprintf(
		    stderr,
PROG_NAME_FULL " was unable to complete the installation of local data files in:\n\
\n\
    %s\n",
		    dname.local_data
		);
		notify_install_local_error = True;
	    }
	    else
	    {
		notify_install_local_success = True;
	    }
	}

	/* Load values from configuration file */
	if(SAROptionsLoadFromFile(opt, fname.options))
	{
	    fprintf(
		stderr,
 "%s: Warning: Error occured while loading configuration.\n",
		fname.options
	    );
	}

	/* Record program file name */
#ifdef __MSW__
	if(hInst != NULL)
	{
	    char prog[PATH_MAX];
	    GetModuleFileName(hInst, prog, sizeof(prog));

	    s = strrchr(prog, DIR_DELIMINATOR);
	    if(s != NULL)
		s++;
	    else
		s = prog;

	    free(core_ptr->prog_file_full_path);
	    core_ptr->prog_file_full_path = STRDUP(prog);

	    free(core_ptr->prog_file);
	    core_ptr->prog_file = STRDUP(s);
	}
#else
	arg = (argc > 0) ? argv[0] : NULL;
	if(arg != NULL)
	{
	    s = strrchr(arg, DIR_DELIMINATOR);
	    if(s != NULL)
		s++;
	    else
		s = arg;

	    free(core_ptr->prog_file_full_path);
	    core_ptr->prog_file_full_path = STRDUP(arg);

	    free(core_ptr->prog_file);
	    core_ptr->prog_file = STRDUP(s);
	}
#endif

	/* Set up arguments for initializing the graphics wrapper */
	strc = 11;
	strv = (char **)calloc(strc, sizeof(char *));
	strv[0] = STRDUP(cl_direct_rendering ?
	    "--hardware_rendering" : "--software_rendering"
	);
	strv[1] = STRDUP("--geometry");
	if(*geometry_string == '\0')
	    sprintf(
		geometry_string,
		"%ix%i%s%i%s%i",
		opt->last_width,
		opt->last_height,
		((opt->last_x < 0) ? "" : "+"),
		opt->last_x,
		((opt->last_y < 0) ? "" : "+"),
		opt->last_y
	    );
	strv[2] = STRDUP(geometry_string);
	strv[3] = STRDUP("--title");
	strv[4] = STRDUP(PROG_NAME_FULL);
	strv[5] = STRDUP("--icon_path");
	strv[6] = STRDUP(SAR_DEF_SAR_ICON_FILE);
	strv[7] = STRDUP("--icon_name");
	strv[8] = STRDUP(PROG_NAME_FULL);
	if(cl_fullscreen == 1)
	    strv[9] = STRDUP("--full_screen");
        else if (cl_fullscreen == 2)
        {
            strv[9] = STRDUP("--window");
            opt->last_fullscreen = False;
        }
	else
	    strv[9] = STRDUP(opt->last_fullscreen ?
		"--fullscreen" : "--windowed"
	    );
        if (! allow_key_repeat)
           strv[10] = STRDUP("--no-keyrepeat");

	if(!STRISEMPTY(display_address))
	{
	    int n = strc;
	    strv = (char **)realloc(strv, strc * sizeof(char *));
	    strv[n + 0] = STRDUP("--display");
	    strv[n + 1] = STRDUP(display_address);
	}
	if(aspect_offset != 0.0)
	{
	    int n = strc;
	    char num_str[80];
	    sprintf(num_str, "%f", aspect_offset);
	    strc = n + 2;
	    strv = (char **)realloc(strv, strc * sizeof(char *));
	    strv[n + 0] = STRDUP("--aspect_offset");
	    strv[n + 1] = STRDUP(num_str);
	}
	if(opt->runtime_debug)
	{
	    int n = strc;
	    strc = n + 1;
	    strv = (char **)realloc(strv, strc * sizeof(char *));
	    strv[n + 0] = STRDUP("--gw_debug");
	}
	if(font_name != NULL)
	{
	    int n = strc;
	    strc = n + 2;
	    strv = (char **)realloc(strv, strc * sizeof(char *));
	    strv[n + 0] = STRDUP("--font");
	    strv[n + 1] = STRDUP(font_name);
	}

	/* Initialize graphics erapper */
	core_ptr->display = GWInit(strc, strv);

	/* Delete arguments used in initializing graphics wrapper */
	strlistfree(strv, strc);
	strv = NULL;
	strc = 0;

	/* Failed to initialize graphics wrapper? */
	if(core_ptr->display == NULL)
	{
	    SARShutdown(core_ptr);
	    return(NULL);
	}
	dpy = core_ptr->display;

	/* Check if we have OpenGL 1.1 or newer */
	if((dpy->gl_version_major < 1) ?
	    True : ((dpy->gl_version_major == 1) ?
		(dpy->gl_version_minor < 1) : False)
	)
	{
	    fprintf(
		stderr,
PROG_NAME_FULL " requires OpenGL version 1.1 or newer, the current \
version that was detected is OpenGL version %i.%i.\n",
		dpy->gl_version_major,
		dpy->gl_version_minor
	    );
	}

	/* Check if we have alpha bits */
	if(dpy->alpha_channel_bits <= 0)
	{
	    fprintf(
		stderr,
PROG_NAME_FULL " requires an alpha channel, the current alpha channel \
that was detected is 0 bits in size.\n"
	    );
	}

	/* Set font data references */
	opt->hud_font = font_6x10;		/* For HUD text */
	opt->message_font = font_7x14;	/* For standard/misc messages */
	opt->menu_font = font_menu;		/* For menus (except for menu values */
	opt->banner_font = font_banner;	/* For the sticky banner messages */

	/* Set graphics wrapper options */
	GWSetDrawCB(dpy, SARDrawCB, core_ptr);
	GWSetKeyboardCB(dpy, SARKeyBoardCB, core_ptr);
	GWSetPointerCB(dpy, SARPointerCB, core_ptr);
	GWSetResizeCB(dpy, SARReshapeCB, core_ptr);
	GWSetVisibilityCB(dpy, SARVisibilityCB, core_ptr);
	GWSetSaveYourselfCB(dpy, SARSaveYourselfCB, core_ptr);
	GWSetCloseCB(dpy, SARCloseCB, core_ptr);
	GWSetTimeoutCB(dpy, SARManage, core_ptr);


	/* Do splash */
	SARSplash(core_ptr);


	/* Set audio mode name for changing of sound server audio mode */
	free(core_ptr->audio_mode_name);
	core_ptr->audio_mode_name = STRDUP("PlayStereo11025");

	/* Initialize sound wrapper */
	if (startup_no_sound) /* ||
	   (!opt->engine_sounds &&
	    !opt->event_sounds &&
	    !opt->voice_sounds &&
	    !opt->music
	   )
	) */
	{
	    /* Do not initialize sound and turn all sound options off */
	    core_ptr->recorder = NULL;
	    opt->engine_sounds = False;
	    opt->event_sounds = False;
	    opt->voice_sounds = False;
	    opt->music = False;
	}
	else
	{
	    void *window;
            int type = SNDSERV_TYPE_NONE; // use this as default

            #ifdef Y_H
            type = SNDSERV_TYPE_Y;
            #endif
            #ifdef SDL_H
            type = SNDSERV_TYPE_SDL;    // hopefully we use this
            #endif

	    GWContextGet(
		dpy, GWContextCurrent(dpy),
		&window, NULL,
		NULL, NULL,
		NULL, NULL
	    );
	    core_ptr->recorder = SoundInit(
		core_ptr,
	        type,
	        sound_server_connect_arg,	/* Connect argument */
		NULL,				/* Start argument */
		window				/* Toplevel window */
	    );

	    if(core_ptr->recorder == NULL)
	    {
	        fprintf(
		    stderr,
PROG_NAME_FULL " could not connect to the Sound Server, please check \
if the Sound Server is currently running.\n"
		);

		opt->engine_sounds = False;
		opt->event_sounds = False;
		opt->voice_sounds = False;
		opt->music = False;
	    }
            #ifdef Y_H
	    else
	    {
		/* Change Y audio mode */
	        SoundChangeMode(
		    core_ptr->recorder, core_ptr->audio_mode_name
		);

		/* Update recorder address */
		if(sound_server_connect_arg != NULL)
		{
		    free(core_ptr->recorder_address);
		    core_ptr->recorder_address = STRDUP(sound_server_connect_arg);
		}
	    }
            #endif
	}


	/* Initialize game controller
	 *
	 * Game controller options need to be set up properly prior to
	 * this call
	 */
	SARInitGCTL(core_ptr);


	/* Load global texture reference names list (these contain only
	 * names and file names of textures to be loaded when a scene
	 * is loaded)
	 */
	SARTextureListLoadFromFile(
	    fname.textures,
	    &core_ptr->texture_list,
	    &core_ptr->total_texture_list
	);

	/* Initialize predefined humans list */
	core_ptr->human_data = SARHumanPresetsInit(core_ptr);
	if(core_ptr->human_data == NULL)
	{
	    fprintf(   
		stderr,
		"Error initializing human presets data.\n"
	    );
	}
	/* Load predefined humans list from file */
	SARHumanLoadFromFile(
	    core_ptr->human_data,
	    fname.human
	);

	/* Reset current music ID so that the music will restart after
	 * the music list is loaded from file
	 */
	core_ptr->cur_music_id = -1;
	/* Load music list from file */
	SARMusicListLoadFromFile(
	    fname.music,
	    &core_ptr->music_ref, &core_ptr->total_music_refs
	);

	/* Initialize predefined weather conditions */
	core_ptr->weather_data = SARWeatherPresetsInit(core_ptr);
	if(core_ptr->weather_data == NULL)
	{
	    fprintf(
		stderr,
		"Error initializing weather presets data.\n"
	    );
	}
	/* Load predefined weather conditions from file */
	SARWeatherLoadFromFile(
	    core_ptr->weather_data,
	    fname.weather
	);

	/* Load players list */
	SARPlayerStatsLoadFromFile(
	    &core_ptr->player_stat,
	    &core_ptr->total_player_stats,
	    fname.players
	);



	/* Reset GL states for menu system */
	SARMenuGLStateReset(core_ptr->display);

	/* Create all menu system resources and each menu */
	SARBuildMenus(core_ptr);

	/* Force select the main menu */
	core_ptr->cur_menu = -1;
	SARMenuSwitchToMenu(core_ptr, SAR_MENU_NAME_MAIN);

	/* Text input prompt */
	core_ptr->text_input = SARTextInputNew(
	    core_ptr->display, opt->message_font
	);

	/* Print messages as needed */
	if(notify_install_local_error)
	{
	    char mesg[1024];

	    sprintf(mesg,
"Cannot install local data files to:\n%s",
		dname.local_data
	    );

	    GWOutputMessage(
		dpy, GWOutputMessageTypeError,
		"Installation Error",
		mesg,
"There was a problem installing local data files, make sure that\n\
global data for this program has been installed properly and that\n\
your home directory exists and has has write permission for this program"
	    );
	}
	else if(notify_install_local_success)
	{
	    char mesg[1024];

	    sprintf(mesg,
"Local data files for this program have been\ninstalled in:\n\n%s",
		dname.local_data
	    );

	    GWOutputMessage(
		dpy, GWOutputMessageTypeGeneral,
		"Local Data Files Installed",
		mesg,
"The data files needed by this program have been installed, this\n\
occured because the program needs them and they were not detected\n\
to exist before."
	    );
	}

	return(core_ptr);
}

/*
 *	SAR management, this is called once per loop as the timeout
 *	function.
 *
 *	The graphics wrapper normally calls this function on timeouts.
 */
void SARManage(void *ptr)
{
	int status;
	int cur_menu;
	time_t t_new;
	sar_scene_struct *scene;
	sar_core_struct *core_ptr = SAR_CORE(ptr);
	const sar_option_struct *opt;
	if(core_ptr == NULL)
	    return;

	opt = &core_ptr->option;

	/* Get current time in milliseconds */
	t_new = SARGetCurMilliTime();

	/* Check if the new current time has "warped" to a smaller value
	 * than the previous current time.
	 */
	if(t_new < cur_millitime)
	{
	    /* Timing has cycled, so we need to reset all timmers */
	    SARResetTimmersCB(core_ptr, t_new);
	}
	else
	{
	    /* Calculate lapsed ms from last loop */
	    lapsed_millitime = t_new - cur_millitime;

	    /* Calculate time compensation coeff */
	    time_compensation = (float)CLIP(
		(float)lapsed_millitime / (float)CYCLE_LAPSE_MS,
		0.0, 1000.0
	    );

	    /* Set new current time in ms */
	    cur_millitime = t_new;
	}

	/* Get current systime seconds */
	cur_systime = time(NULL);


	/* Get new game controller positions */
	GCtlUpdate(
	    core_ptr->gctl,
	    (Boolean)((opt->flight_physics_level != FLIGHT_PHYSICS_REALISTIC) ? True : False),	/* Heading nullzone? */
	    (Boolean)((opt->flight_physics_level == FLIGHT_PHYSICS_EASY) ? True : False),	/* Pitch nullzone? */
	    (Boolean)((opt->flight_physics_level == FLIGHT_PHYSICS_EASY) ? True : False),	/* Bank nullzone? */
	    cur_millitime, lapsed_millitime, time_compensation
	);


	/* Check if a current menu is allocated (hence selected) */
	cur_menu = core_ptr->cur_menu;
	if(SARIsMenuAllocated(core_ptr, cur_menu))
	{
	    /* A menu is selected which implies that we are in the
	     * menus.  Get current menu to see which menu we're
	     * currently on.
	     */
	    SARMenuManage(core_ptr, core_ptr->menu[cur_menu]);
	}
	else
	{
	    /* No menu selected, this implies we are in game and
	     * as such we need to handle simulation updates and
	     * redraw.
	     */
	    scene = core_ptr->scene;

	    SARSimUpdateScene(core_ptr, scene);
	    SARSimUpdateSceneObjects(core_ptr, scene);

	    if(is_visible)
		SARDraw(core_ptr);

	    /* Manage mission */
	    status = SARMissionManage(core_ptr);
	    /* Check mission manage result */
	    switch(status)
	    {
	      case 1:
		/* Mission ended with success, end simulation and
		 * tabulate mission results.
		 */
		SARSimEnd(core_ptr);
		break;

	      case 2:
		/* Mission over and failed. Do not call SARSimEnd(),
		 * instead let user respond to failed message and
		 * manually end mission.
		 */
		break;

	      case -1:
		/* Mission management error */
		break;

	      default:
		/* Nothing eventful */
		break;
	    }
	}

	/* Manage sound events if connected to recorder */
	if(core_ptr->recorder != NULL)
	{
	    /* Manage sound events.  If return is negative then that
	     * means the recorder pointer is no longer valid.
	     */
	    status = SoundManageEvents(core_ptr->recorder);
	    if(status < 0)
		core_ptr->recorder = NULL;
	}

	/* Update music, checks the current run time situation and
	 * changes the music as needed.
	 */
	SARMusicUpdate(core_ptr);

}

/*
 *	Deletes the Core.
 */
void SARShutdown(sar_core_struct *core_ptr)
{
	const char *s;
	sar_option_struct *opt;

	if(core_ptr == NULL)
	    return;

	opt = &core_ptr->option;

	/* Save options */
	s = fname.options;
	if(!STRISEMPTY(s))
	{
	    if(SAROptionsSaveToFile(opt, s))
		fprintf(
		    stderr,
"%s: Warning: Error occured while saving options.\n",
		    s
		);
	}
	/* Save players list */
	s = fname.players;
	if(!STRISEMPTY(s))
	{
	    if(SARPlayerStatsSaveToFile(
		core_ptr->player_stat, core_ptr->total_player_stats,
		s
	    ))
		fprintf(
		    stderr,
"%s: Warning: Error occured while saving pilots list.\n",
		    s
		);
	}


	/* Text input */
	SARTextInputDelete(core_ptr->text_input);
	core_ptr->text_input = NULL;

	/* Global texture file reference list */
	SARTextureListDeleteAll(
	    &core_ptr->texture_list,
	    &core_ptr->total_texture_list
	);

	/* File names */
	free(core_ptr->cur_mission_file);
	core_ptr->cur_mission_file = NULL;

	free(core_ptr->cur_player_model_file);
	core_ptr->cur_player_model_file = NULL;

	free(core_ptr->cur_scene_file);
	core_ptr->cur_scene_file = NULL;


	/* Menus */
	if(core_ptr->total_menus > 0)
	{
	    int i, n;
	    for(i = 0; i < core_ptr->total_menus; i++)
	    {
		sar_menu_struct *m = core_ptr->menu[i];
		if(m == NULL)
		    continue;

		/* Iterate through each object on the menu */
		for(n = 0; n < m->total_objects; n++)
		{
		    void *o = m->object[n];
		    if(o == NULL)
			continue;

		    /* Handle by the menu object's type */
		    switch(SAR_MENU_OBJECT_TYPE(o))
		    {
		      case SAR_MENU_OBJECT_TYPE_LABEL:
		      case SAR_MENU_OBJECT_TYPE_BUTTON:
		      case SAR_MENU_OBJECT_TYPE_PROGRESS:
		      case SAR_MENU_OBJECT_TYPE_MESSAGE_BOX:
		        break;
		      case SAR_MENU_OBJECT_TYPE_LIST:
		        {
			    int j;
			    sar_menu_list_struct *list = SAR_MENU_LIST(o);
			    /* Delete list item data */
			    for(j = 0; j < list->total_items; j++)
			        SARDeleteListItemData(list->item[j]);
			}
			break;
		      case SAR_MENU_OBJECT_TYPE_MDISPLAY:
		      case SAR_MENU_OBJECT_TYPE_SWITCH:
		      case SAR_MENU_OBJECT_TYPE_SPIN:
		      case SAR_MENU_OBJECT_TYPE_SLIDER:
		      case SAR_MENU_OBJECT_TYPE_MAP:
		      case SAR_MENU_OBJECT_TYPE_OBJVIEW:
		        break;
		    }
		}

		/* Delete this menu and all its objects */
		SARMenuDelete(m);
	    }
	    free(core_ptr->menu);
	    core_ptr->menu = NULL;
	    core_ptr->cur_menu = 0;
	    core_ptr->total_menus = 0;
	}

	/* Begin deleted resources used for the menus, these resources
	 * may be shared but since all menus are deleted at this
	 * point it is safe to delete these resources
	 */

	/* Images used by menus */
#define DELETE_IMAGE(_i_)	\
{ if(*(_i_) != NULL) { SARImageDelete(*(_i_)); *(_i_) = NULL; } }
	if(core_ptr->menu_list_bg_img != NULL)
	{
	    int i;
	    for(i = 0; i < 9; i++)
		DELETE_IMAGE(&core_ptr->menu_list_bg_img[i]);
	    free(core_ptr->menu_list_bg_img);
	    core_ptr->menu_list_bg_img = NULL;
	}

	DELETE_IMAGE(&core_ptr->menu_button_armed_img);
	DELETE_IMAGE(&core_ptr->menu_button_unarmed_img);
	DELETE_IMAGE(&core_ptr->menu_button_highlighted_img);
	DELETE_IMAGE(&core_ptr->menu_button_label_img);

	DELETE_IMAGE(&core_ptr->menu_label_bg_img);

	DELETE_IMAGE(&core_ptr->menu_switch_bg_img);
	DELETE_IMAGE(&core_ptr->menu_switch_off_img);
	DELETE_IMAGE(&core_ptr->menu_switch_on_img);

	DELETE_IMAGE(&core_ptr->menu_spin_label_img);
	DELETE_IMAGE(&core_ptr->menu_spin_value_img);
	DELETE_IMAGE(&core_ptr->menu_spin_dec_armed_img);
	DELETE_IMAGE(&core_ptr->menu_spin_dec_unarmed_img);
	DELETE_IMAGE(&core_ptr->menu_spin_inc_armed_img);
	DELETE_IMAGE(&core_ptr->menu_spin_inc_unarmed_img);

	DELETE_IMAGE(&core_ptr->menu_slider_label_img);
	DELETE_IMAGE(&core_ptr->menu_slider_trough_img);
	DELETE_IMAGE(&core_ptr->menu_slider_handle_img);

	DELETE_IMAGE(&core_ptr->menu_progress_bg_img);
	DELETE_IMAGE(&core_ptr->menu_progress_fg_img);

	DELETE_IMAGE(&core_ptr->menu_button_pan_up_armed_img);
	DELETE_IMAGE(&core_ptr->menu_button_pan_up_unarmed_img);
	DELETE_IMAGE(&core_ptr->menu_button_pan_down_armed_img);
	DELETE_IMAGE(&core_ptr->menu_button_pan_down_unarmed_img);
	DELETE_IMAGE(&core_ptr->menu_button_pan_left_armed_img);
	DELETE_IMAGE(&core_ptr->menu_button_pan_left_unarmed_img);
	DELETE_IMAGE(&core_ptr->menu_button_pan_right_armed_img);
	DELETE_IMAGE(&core_ptr->menu_button_pan_right_unarmed_img);

	DELETE_IMAGE(&core_ptr->menu_button_zoom_in_armed_img);
	DELETE_IMAGE(&core_ptr->menu_button_zoom_in_unarmed_img);
	DELETE_IMAGE(&core_ptr->menu_button_zoom_out_armed_img);
	DELETE_IMAGE(&core_ptr->menu_button_zoom_out_unarmed_img);

	DELETE_IMAGE(&core_ptr->menumap_helipad_img);
	DELETE_IMAGE(&core_ptr->menumap_intercept_img);
	DELETE_IMAGE(&core_ptr->menumap_helicopter_img);
	DELETE_IMAGE(&core_ptr->menumap_victim_img);
	DELETE_IMAGE(&core_ptr->menumap_vessel_img);
	DELETE_IMAGE(&core_ptr->menumap_crash_img);

#undef DELETE_IMAGE


	/* Delete mission structure (if any). Note that mission should
	 * have been properly ended and deleted before shutting down
	 */
	SARMissionDelete(core_ptr->mission);
	core_ptr->mission = NULL;

	/* Delete scene */
	SARSceneDestroy(
	    core_ptr,
	    core_ptr->scene,
	    &core_ptr->object,
	    &core_ptr->total_objects
	);
	free(core_ptr->scene);
	core_ptr->scene = NULL;

	/* Draw Map Object Names List */
	SARDrawMapObjNameListDelete(
	    &core_ptr->drawmap_objname,
	    &core_ptr->total_drawmap_objnames
	);

	/* Player Stats */
	if(core_ptr->total_player_stats > 0)
	{
	    int i;
	    for(i = 0; i < core_ptr->total_player_stats; i++)
		SARPlayerStatDelete(core_ptr->player_stat[i]);
	    free(core_ptr->player_stat);
	    core_ptr->player_stat = NULL;
	    core_ptr->total_player_stats = 0;
	}

	/* Human Data */
	SARHumanPresetsShutdown(core_ptr->human_data);
	core_ptr->human_data = NULL;

	/* Weather Data */
	SARWeatherPresetsShutdown(core_ptr->weather_data);
	core_ptr->weather_data = NULL;


	/* Game Controller */
	GCtlDelete(core_ptr->gctl);
	core_ptr->gctl = NULL;

	/* Music List */
	SARMusicListDeleteAll(&core_ptr->music_ref, &core_ptr->total_music_refs);

	/* Sound wrapper */
	SoundShutdown(core_ptr->recorder);
	core_ptr->recorder = NULL;

	free(core_ptr->audio_mode_name);
	core_ptr->audio_mode_name = NULL;

	free(core_ptr->recorder_address);
	core_ptr->recorder_address = NULL;

	/* Graphics wrapper */
	GWShutdown(core_ptr->display);
	core_ptr->display = NULL;

	/* Program name */
	free(core_ptr->prog_file_full_path);
	core_ptr->prog_file_full_path = NULL;
	free(core_ptr->prog_file);
	core_ptr->prog_file = NULL;

	free(core_ptr);
}

#ifdef __MSW__
int WINAPI WinMain(
	HINSTANCE hInstance,		// Instance
	HINSTANCE hPrevInstance,	// Previous Instance
	LPSTR lpCmdLine,		// Command Line Parameters
	int nCmdShow			// Window Show State
)
#else
int main(int argc, char **argv)
#endif
{
#ifdef __MSW__
	int argc = 0;
	char **argv = strexp(lpCmdLine, &argc);
#endif
	sar_core_struct *core_ptr;
	const sar_option_struct *opt;


	/* Reset globals */
	debug_value = 0.0f;
	segfault_count = 0;

#ifdef __MSW__
	/* Initialize COM loaders (needed for DirectX) */
	CoInitialize(NULL);
#endif

	/* Initialize program core */
	core_ptr = SARInit(argc, argv);
	if(core_ptr == NULL)
	    return(1);
	opt = &core_ptr->option;

	/* Main loop */
	runlevel = 2;
	while(runlevel >= 2)
	{
#ifdef __MSW__
	    /* No sleeping for Windows */
#else
	    usleep(
		(unsigned long)CLIP(
		    (8000 * (1.0 - opt->graphics_acceleration)), 0, 1000
		)
	    );
#endif
	    GWManage(core_ptr->display);
	}

	/* Shutdown program core */
	SARShutdown(core_ptr);
	core_ptr = NULL;

#ifdef __MSW__
	/* Free command line arguments */
	strlistfree(argv, argc);

	/* Shutdown COM loader (needed for DirectX) */
	CoUninitialize();
#endif

	return(0);
}
