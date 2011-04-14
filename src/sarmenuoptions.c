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
#include <sys/types.h>
#include <sys/stat.h>
#ifdef __MSW__
# include <windows.h>
#else
# include <unistd.h>
#endif

#include "../include/string.h"
#include "../include/fio.h"
#include "../include/disk.h"

#include "gw.h"
#include "textinput.h"
#include "sound.h"
#include "menu.h"
#include "sar.h"
#include "sarmenuoptions.h"
#include "sarmenucodes.h"
#include "config.h"


static sar_menu_switch_struct *SARMenuOptionsGetSwitchByID(
	sar_menu_struct *m, int id, int *n
);
static sar_menu_spin_struct *SARMenuOptionsGetSpinByID(
	sar_menu_struct *m, int id, int *n
);
static sar_menu_slider_struct *SARMenuOptionsGetSliderByID(
	sar_menu_struct *m, int id, int *n
);

static void SARMenuOptionsSaveMessageBox(
	sar_core_struct *core_ptr, const char *mesg,
	const char *path
);
static void SARMenuOptionsJoystickReinit(sar_core_struct *core_ptr);
static void SARMenuOptionsJoystickButtonsRemap(sar_core_struct *core_ptr);
static int SARGetJSButtonFromButtonRoleSpin(
	sar_core_struct *core_ptr,
	sar_menu_spin_struct *spin, int id
);

void SARMenuOptionsJoystickTestDrawCB(
	void *dpy,		/* Display */
	void *menu,		/* Menu */
	void *o,		/* This Object */
	int id,		/* ID Code */	
	void *client_data,	/* Data */
	int x_min, int y_min, int x_max, int y_max
);

void SARMenuOptionsGraphicsInfoRefresh(sar_core_struct *core_ptr);
static void SARMenuOptionsSaveGraphicsInfoCB(const char *v, void *data);
void SARMenuOptionsSoundTest(sar_core_struct *core_ptr);
void SARMenuOptionsSoundRefresh(sar_core_struct *core_ptr);
void SARMenuOptionsSoundInfoRefresh(sar_core_struct *core_ptr);
static void SARMenuOptionsSaveSoundInfoCB(const char *v, void *data);

void SARMenuOptionsApply(sar_core_struct *core_ptr);
void SARMenuOptionsFetch(sar_core_struct *core_ptr);

void SARMenuOptionsButtonCB(
	void *object, int id, void *client_data
);
void SARMenuOptionsSwitchCB(
	void *object, int id, void *client_data,
	Boolean state
);
void SARMenuOptionsSpinCB(
	void *object, int id, void *client_data,
	char *value
);
void SARMenuOptionsSliderCB(
	void *object, int id, void *client_data,
	float value
);


#define ATOI(s)		(((s) != NULL) ? atoi(s) : 0)
#define ATOL(s)		(((s) != NULL) ? atol(s) : 0)
#define ATOF(s)		(((s) != NULL) ? (float)atof(s) : 0.0f)
#define STRDUP(s)	(((s) != NULL) ? strdup(s) : NULL)

#define MAX(a,b)	(((a) > (b)) ? (a) : (b))
#define MIN(a,b)	(((a) < (b)) ? (a) : (b))
#define CLIP(a,l,h)	(MIN(MAX((a),(l)),(h)))

#define DEGTORAD(d)	((d) * PI / 180)
#define RADTODEG(r)	((r) * 180 / PI)

#define STRLEN(s)	(((s) != NULL) ? ((int)strlen(s)) : 0)
#define STRISEMPTY(s)	(((s) != NULL) ? (*(s) == '\0') : 1)


/*
 *	Returns the switch object on menu m who's id matches the
 *	given id.
 */
static sar_menu_switch_struct *SARMenuOptionsGetSwitchByID(
	sar_menu_struct *m, int id, int *n
)
{
	int i;
	void *o;
	sar_menu_switch_struct *sw;

	if(n != NULL)
	    *n = -1;

	if(m == NULL)
	    return(NULL);

	for(i = 0; i < m->total_objects; i++)
	{
	    o = m->object[i];
	    if(SAR_MENU_IS_SWITCH(o))
		sw = SAR_MENU_SWITCH(o);
	    else
		continue;
 
	    if(sw->id == id)
	    {
		if(n != NULL)
		    *n = i;

		return(sw);
	    }
	}

	return(NULL);
}

/*
 *	Returns the spin object on menu m who's id matches the
 *      given id.
 */
static sar_menu_spin_struct *SARMenuOptionsGetSpinByID(
	sar_menu_struct *m, int id, int *n
)
{
	int i;
	void *o;
	sar_menu_spin_struct *spin;


	if(n != NULL)
	    *n = -1;

	if(m == NULL)
	    return(NULL);

	for(i = 0; i < m->total_objects; i++)
	{
	    o = m->object[i];
	    if(SAR_MENU_IS_SPIN(o))
		spin = SAR_MENU_SPIN(o);
	    else
		continue;

	    if(spin->id == id)
	    {
		if(n != NULL)
		    *n = i;

		return(spin);
	    }
	}

	return(NULL);
}

/*
 *      Returns the slider object on menu m who's id matches the
 *      given id.
 */
static sar_menu_slider_struct *SARMenuOptionsGetSliderByID(
	sar_menu_struct *m, int id, int *n
)
{
	int i;
	void *o;
	sar_menu_slider_struct *slider;


	if(n != NULL)
	    *n = -1;

	if(m == NULL)
	    return(NULL);

	for(i = 0; i < m->total_objects; i++)
	{
	    o = m->object[i];
	    if(SAR_MENU_IS_SLIDER(o))
		slider = SAR_MENU_SLIDER(o);
	    else
		continue;

	    if(slider->id == id)
	    {
		if(n != NULL)
		    *n = i;

		return(slider);
	    }
	}

	return(NULL);
}


/*
 *	Saves the message box's message specified by mesg to the
 *	file specified by path.
 */
static void SARMenuOptionsSaveMessageBox(
	sar_core_struct *core_ptr, const char *mesg,
	const char *path
)
{
	struct stat stat_buf;
	FILE *fp;
	const char *s;


	if((core_ptr == NULL) || (mesg == NULL) || (path == NULL))
	    return;

	/* Check if file exists and confirm overwrite */
	if(!stat(path, &stat_buf))
	{
	    int status;
	    char *buf = (char *)malloc(
		(256 + STRLEN(path)) * sizeof(char)
	    );
	    sprintf(
		buf,
"Overwrite existing file:\n\n    %s",
		path
	    );
	    status = GWConfirmation(
		core_ptr->display,
		GWOutputMessageTypeWarning,
		"Confirm Overwrite",
		buf,
		NULL,
		GWConfirmationNo
	    );
	    free(buf);
	    if(status != GWConfirmationYes)
		return;
	}

	/* Open file for writing */
	fp = FOpen(path, "wb");
	if(fp == NULL)
	    return;

	/* Begin writing message to file */
	s = mesg;
	while(*s != '\0')
	{
	    /* Skip tags */
	    if(*s == '<')
	    {
		while((*s != '>') && (*s != '\0'))
		    s++;
		if(*s != '\0')
		    s++;
		else
		    break;
	    }
	    fputc(*s, fp);
	    s++;
	}

	/* Close file */
	FClose(fp);
}

/*
 *	Reinitializes the joystick and the game controller based on
 *	the global options.
 *
 *	This will update the gctl structure on the core structure and
 *	opt->gctl_controllers.
 */
static void SARMenuOptionsJoystickReinit(sar_core_struct *core_ptr)
{
	sar_option_struct *opt;


	if(core_ptr == NULL)
	    return;

	opt = &core_ptr->option;

	/* Set up game controllers mask.
	 *
	 * Start off game controllers mask with just keyboard and pointer.
	 */
	opt->gctl_controllers = GCTL_CONTROLLER_KEYBOARD |
	    GCTL_CONTROLLER_POINTER;
	/* Check if one or more joystick(s) need to be enabled */
	if((opt->gctl_js0_axis_roles != 0) ||
	   (opt->gctl_js1_axis_roles != 0)
	)
	{
	    /* Add joystick to game controllers mask */
	    opt->gctl_controllers |= GCTL_CONTROLLER_JOYSTICK;
	}

	/* Reinitialize game controller to the new type */
	GWSetInputBusy(core_ptr->display);
	SARInitGCTL(core_ptr);		/* Realize changes */
	GWSetInputReady(core_ptr->display);
}

/*
 *	Checks if the gctl structure on the given core structure has
 *	one or more joysticks initialized. If so then the joystick
 *	structures on the gctl structure will have its button mappings
 *	updated with the global options joystick button mappings.
 *
 *	No reinitialization will take place.
 */
static void SARMenuOptionsJoystickButtonsRemap(sar_core_struct *core_ptr)
{
	gctl_struct *gc;
	sar_option_struct *opt;

	if(core_ptr == NULL)
	    return;

	gc = core_ptr->gctl;
	if(gc == NULL)
	    return;

	opt = &core_ptr->option;

	/* Joystick controller defined as one of the game controllers? */
	if(gc->controllers & GCTL_CONTROLLER_JOYSTICK)
	{
	    int i;
	    gctl_js_struct *gc_js;


	    /* First joystick initialized? */
	    i = 0;
	    if(i < gc->total_joysticks)
		gc_js = &gc->joystick[i];
	    else
		gc_js = NULL;
	    if(gc_js != NULL)
	    {
		gc_js->button_rotate = opt->js0_btn_rotate;
		gc_js->button_air_brakes = opt->js0_btn_air_brakes;
		gc_js->button_wheel_brakes =  opt->js0_btn_wheel_brakes;
		gc_js->button_zoom_in = opt->js0_btn_zoom_in;
		gc_js->button_zoom_out = opt->js0_btn_zoom_out;
		gc_js->button_hoist_up = opt->js0_btn_hoist_up;
		gc_js->button_hoist_down = opt->js0_btn_hoist_down;
	    }

	    /* Second joystick initialized? */
	    i = 1;
	    if(i < gc->total_joysticks)
		gc_js = &gc->joystick[i];
	    else
		gc_js = NULL;
	    if(gc_js != NULL)
	    {
		gc_js->button_rotate = opt->js1_btn_rotate;
		gc_js->button_air_brakes = opt->js1_btn_air_brakes;
		gc_js->button_wheel_brakes =  opt->js1_btn_wheel_brakes;
		gc_js->button_zoom_in = opt->js1_btn_zoom_in;
		gc_js->button_zoom_out = opt->js1_btn_zoom_out;
		gc_js->button_hoist_up = opt->js1_btn_hoist_up;
		gc_js->button_hoist_down = opt->js1_btn_hoist_down;
	    }

	}
}


/*
 *	Returns the current joystick button mapping for the given
 *	joystick action spin's current value.
 *
 *	The given id must be one of SAR_OPT_SELECT_JS#_BUTTON_ACTION
 *	where # is the joystick number.
 *
 *	Can return -1 on error or if the value is not set. Calling
 *	function must add 1 to offset when setting to the button spin's
 *	current value since the button spin's value 0 means button number
 *	-1 (none/invalid).
 */
static int SARGetJSButtonFromButtonRoleSpin(
	sar_core_struct *core_ptr,
	sar_menu_spin_struct *spin, int id
)
{
	int val = -1;
	const sar_option_struct *opt;


	if((core_ptr == NULL) || (spin == NULL))
	    return(val);

	opt = &core_ptr->option;

	/* First joystick (js0) */
	if(id == SAR_MENU_ID_OPT_JS0_BUTTON_ACTION)
	{
	    switch(spin->cur_value)
	    {
	      case 0:	/* Rotate Modifier */
		val = opt->js0_btn_rotate;
		break;
	      case 1:	/* Air Brakes */
		val = opt->js0_btn_air_brakes;
		break;
	      case 2:	/* Wheel Brakes */
		val = opt->js0_btn_wheel_brakes;
		break;
	      case 3:	/* Zoom In */
		val = opt->js0_btn_zoom_in;
		break;
	      case 4:	/* Zoom out */
		val = opt->js0_btn_zoom_out;
		break;
	      case 5:	/* Hoist Up */
		val = opt->js0_btn_hoist_up;
		break;
	      case 6:	/* Hoist Down */
		val = opt->js0_btn_hoist_down;
		break;
/* When adding a new button mapping, make sure to add it to the
 * setting of the button mappings to option structure in
 * SARMenuOptionsSpinCB(). 
 *
 * Also make sure to add support for EACH joystick!
 */
	    }
	}
	/* Second joystick (js1) */
	else if(id == SAR_MENU_ID_OPT_JS1_BUTTON_ACTION)
	{
	    switch(spin->cur_value) 
	    {
	      case 0:   /* Rotate Modifier */
		val = opt->js1_btn_rotate;
		break;
	      case 1:   /* Air Brakes */
		val = opt->js1_btn_air_brakes;
		break;
	      case 2:   /* Wheel Brakes */
		val = opt->js1_btn_wheel_brakes;
		break;
	      case 3:   /* Zoom In */
		val = opt->js1_btn_zoom_in;
		break;
	      case 4:   /* Zoom out */
		val = opt->js1_btn_zoom_out;
		break;
	      case 5:   /* Hoist Up */
		val = opt->js1_btn_hoist_up;
		break;
	      case 6:   /* Hoist Down */
		val = opt->js1_btn_hoist_down;
		break;
/* When adding a new button mapping, make sure to add it to the
 * setting of the button mappings to option structure in
 * SARMenuOptionsSpinCB().
 *
 * Also be sure to add support for EACH joystick!
 */
	    }
	}

	if(val < 0)
	    val = -1;

	return(val);
}


/*
 *	Joystick test output multi-purpose display draw callback.
 */
void SARMenuOptionsJoystickTestDrawCB(
	void *dpy,              /* Display */
	void *menu,             /* Menu */
	void *o,                /* This Object */
	int id,            /* ID Code */
	void *client_data,      /* Data */
	int x_min, int y_min, int x_max, int y_max
)
{
	gw_display_struct *display = GW_DISPLAY(dpy);
/*	sar_menu_struct *m = SAR_MENU(menu); */
	sar_menu_mdisplay_struct *mdpy = SAR_MENU_MDISPLAY(o);
	sar_core_struct *core_ptr = SAR_CORE(client_data);
	const sar_option_struct *opt;
	gctl_struct *gc;
	int w = x_max - x_min, h = y_max - y_min;
	int fw, fh, width, height;

	/* Crosshairs bitmap 15x15 */
	const u_int8_t crosshairs_bm[] = { 0x01, 0x00, 0x03, 0x80, 0x02,
0x80, 0x02, 0x80, 0x02, 0x80, 0x02, 0x80, 0x7e, 0xfc, 0xc1, 0x06, 0x7e,
0xfc, 0x02, 0x80, 0x02, 0x80, 0x02, 0x80, 0x02, 0x80, 0x03, 0x80, 0x01,
0x00 };
	/* Rudder bitmap 15x15 */
	const u_int8_t rudder_bm[] = { 0x01, 0x00, 0x03, 0x80, 0x02,
0x80, 0x02, 0x80, 0x02, 0x80, 0x02, 0x80, 0x02, 0x80, 0x02, 0x80, 0x02,
0x80, 0x02, 0x80, 0x02, 0x80, 0x02, 0x80, 0x02, 0x80, 0x03, 0x80, 0x01,
0x00 };
	/* Hat bitmap 15x15 */
	const u_int8_t hat_bm[] = { 0x03, 0x80, 0x1f, 0xf0, 0x3e, 0xf8,
0x7e, 0xfc, 0x7e, 0xfc, 0x7e, 0xfc, 0xfe, 0xfe, 0xc0, 0x06, 0xfe, 0xfe,
0x7e, 0xfc, 0x7e, 0xfc, 0x7e, 0xfc, 0x3e, 0xf8, 0x1f, 0xf0, 0x03, 0x80
};
	/* Button icon bitmap 15x15 */
	const u_int8_t btn_bm[] = { 0x00, 0x00, 0x1f, 0xf0, 0x30, 0x18,
0x67, 0xcc, 0x4f, 0xe4, 0x5f, 0xf4, 0x5f, 0xf4, 0x5f, 0xf4, 0x5f, 0xf4,
0x5f, 0xf4, 0x4f, 0xe4, 0x67, 0xcc, 0x30, 0x18, 0x1f, 0xf0, 0x00, 0x00
};

	if((display == NULL) || (mdpy == NULL) || (core_ptr == NULL))
	    return;

	gc = core_ptr->gctl;
	opt = &core_ptr->option;

	GWContextGet(
	    display, GWContextCurrent(display),
	    NULL, NULL,
	    NULL, NULL,
	    &width, &height
	);

	GWGetFontSize(opt->message_font, NULL, NULL, &fw, &fh);
	GWSetFont(display, opt->message_font);

#define SET_VERTEX(x,y)	glVertex2i(	\
 (GLint)((x) + x_min),			\
 (GLint)(height - ((y) + y_min))	\
)
#define DRAW_STRING(x,y,s) GWDrawString(\
 display,				\
 (int)((x) + x_min),			\
 (int)((y) + y_min),			\
 (s)					\
)

	/* Is game controller initialized to joystick? */
	if((gc != NULL) ? (gc->controllers & GCTL_CONTROLLER_JOYSTICK) : False)
	{
	    int js_num, x, y;
	    int x_border = 30, y_border = 30;
	    int x_offset, y_offset;
	    int wc, hc;

	    /* Calculate bounds for pitch and bank axis grid box */
	    if(w > h)
	    {
		wc = (int)(h - MAX((2 * y_border) + 20, 0));
		hc = (int)(h - MAX((2 * y_border) + 15, 0));
	    }
	    else
	    {
		wc = (int)(w - MAX((2 * x_border) + 20, 0));
		hc = (int)(w - MAX((2 * x_border) + 15, 0));
	    }
	    x_offset = x_border;
	    y_offset = y_border;

	    /* Pitch and bank grid box */
	    glColor3f(0.0f, 0.75f, 0.0f);
	    glBegin(GL_LINE_LOOP);
	    {
		SET_VERTEX(x_offset, y_offset);
		SET_VERTEX(x_offset, hc + y_offset);
		SET_VERTEX(wc + x_offset, hc + y_offset);
		SET_VERTEX(wc + x_offset, y_offset);
	    }
	    glEnd();
	    /* Rudder grid box */
	    glBegin(GL_LINE_STRIP);
	    {
		SET_VERTEX(x_offset, hc + y_offset);
		SET_VERTEX(x_offset, hc + 15 + y_offset);
		SET_VERTEX(wc + x_offset, hc + 15 + y_offset);
		SET_VERTEX(wc + x_offset, hc + y_offset);
	    }
	    glEnd();
	    /* Throttle grid box */
	    glBegin(GL_LINE_STRIP);
	    {
		SET_VERTEX(wc + x_offset, y_offset);
		SET_VERTEX(wc + 20 + x_offset, y_offset);
		SET_VERTEX(wc + 20 + x_offset, hc + 15 + y_offset);
	    }
	    glEnd();

	    /* Center line */
	    x = (int)(wc / 2);
	    y = (int)(hc / 2);
	    glColor3f(0.0f, 1.0f, 0.0f);
	    glBegin(GL_LINES);
	    {
		SET_VERTEX(x + x_offset, y_offset);
		SET_VERTEX(x + x_offset, hc + 15 + y_offset);
		SET_VERTEX(x_offset, y + y_offset);
		SET_VERTEX(wc + x_offset, y + y_offset);
	    }
	    glEnd();
	    /* 25% lines */
	    glColor3f(0.0f, 0.75f, 0.0f);
	    glBegin(GL_LINES);
	    {
		x = (int)(wc * 0.25);
		SET_VERTEX(x + x_offset, y_offset);
		SET_VERTEX(x + x_offset, hc + 15 + y_offset);
		x = (int)(wc * 0.75);
		SET_VERTEX(x + x_offset, y_offset);
		SET_VERTEX(x + x_offset, hc + 15 + y_offset);
		y = (int)(hc * 0.25);
		SET_VERTEX(x_offset, y + y_offset);
		SET_VERTEX(wc + x_offset, y + y_offset);
		y = (int)(hc * 0.75);
		SET_VERTEX(x_offset, y + y_offset);
		SET_VERTEX(wc + x_offset, y + y_offset);
	    }
	    glEnd();


	    /* Draw pitch and bank crosshair */
	    x = (int)(CLIP(gc->bank, -1, 1) * wc / 2) + (wc / 2);
	    y = (int)(-CLIP(gc->pitch, -1, 1) * hc / 2) + (hc / 2);
	    glColor3f(1.0f, 1.0f, 0.0f);
	    glRasterPos2i(
		x - 7 + x_offset + x_min,
		height - (y + 7 + y_offset + y_min)
	    );
	    glBitmap(15, 15, 0.0f, 0.0f, 15.0f, 0.0f, crosshairs_bm);

	    /* Calculate bounds for throttle axis */
	    hc += 15;
	    x_offset = x_border + wc;
	    y_offset = y_border;
	    y = (int)(CLIP(1.0 - gc->throttle, 0, 1) * hc);

	    /* Draw throttle axis */
	    glBegin(GL_QUADS);
	    {
		SET_VERTEX(0 + x_offset, y + y_offset);
		SET_VERTEX(0 + x_offset, hc + y_offset);
		SET_VERTEX(20 + x_offset, hc + y_offset);
		SET_VERTEX(20 + x_offset, y + y_offset);
	    }
	    glEnd();

	    hc -= 15;


	    /* Calculate bounds for rudder axis */
	    x_offset = x_border;
	    y_offset = y_border + hc;
	    x = (int)(CLIP(gc->heading, -1, 1) * (wc / 2)) + (wc / 2);

	    /* Draw rudder axis */
	    glRasterPos2i(
		x - 7 + x_offset + x_min,
		height - (15 + y_offset + y_min)
	    );
	    glBitmap(15, 15, 0.0f, 0.0f, 15.0f, 0.0f, rudder_bm);


	    /* Calculate bounds for hat grid box */
	    x_offset = x_border + wc + 20 + 10;
	    y_offset = y_border;
	    wc = 50;
	    hc = 50;

	    /* Draw hat grid box */
	    glColor3f(0.0f, 0.75f, 0.0f);
	    glBegin(GL_LINE_LOOP);
	    {
		SET_VERTEX(x_offset, y_offset);
		SET_VERTEX(x_offset, hc + y_offset);
		SET_VERTEX(wc + x_offset, hc + y_offset);
		SET_VERTEX(wc + x_offset, y_offset);
	    }
	    glEnd();
	    x = wc / 2;
	    y = hc / 2;
	    glColor3f(0.0f, 1.0f, 0.0f);
	    glBegin(GL_LINES);
	    {
		SET_VERTEX(x_offset, y + y_offset);
		SET_VERTEX(wc + x_offset, y + y_offset);
		SET_VERTEX(x + x_offset, y_offset);
		SET_VERTEX(x + x_offset, hc + y_offset);
	    }
	    glEnd();

	    /* Draw hat */
	    x = (int)((gc->hat_x * wc * 0.25) + (wc / 2));
	    y = (int)((-gc->hat_y * hc * 0.25) + (hc / 2));
	    glColor3f(1.0f, 1.0f, 0.0f);
	    glRasterPos2i(
		x - 7 + x_offset + x_min,
		height - (y + 7 + y_offset + y_min)
	    );
	    glBitmap(15, 15, 0.0f, 0.0f, 15.0f, 0.0f, hat_bm);


	    /* Begin drawing buttons, calculate offsets and starting
	     * coordinates.  Note that the y coordinate will be
	     * repositioned after each draw and does not need to
	     * be reset until the "column" shifts.
	     *
	     * Set the offsets for the start of drawing this column,
	     * ote that x_offset was already set when drawing the
	     * hat.
	     */
/*	    x_offset = x_border + wc + 20 + x_border; */
	    y_offset = y_border + hc + 10;
	    x = 0;
	    y = 0;

	    /* Buttons pressed on joystick */
            js_num = 0;
	    if(gc->total_joysticks > js_num)
	    {
                SDL_Joystick *sdljoystick = gc->sdljoystick[js_num];
                int button = -1;

		glColor3f(0.0f, 1.0f, 0.0f);
		glRasterPos2i(
		    x + x_offset + x_min,
		    height - (15 + y + y_offset + y_min)
		);
		glBitmap(15, 15, 0.0f, 0.0f, 15.0f, 0.0f, btn_bm);
		x += 15 + 5;
		glColor3f(1.0f, 1.0f, 0.0f);

		/* This joystick initialized? */
		if (sdljoystick == NULL && SDL_NumJoysticks() > js_num){
                    sdljoystick = SDL_JoystickOpen(js_num);
                    gc->sdljoystick[js_num] = sdljoystick;
                }
		if (sdljoystick != NULL){
		    int i;
                    
		    /* Look for a currently pressed button */
		    for(i = 0; i < SDL_JoystickNumButtons(sdljoystick); i++)
		    {
			if(SDL_JoystickGetButton(sdljoystick,i))
			{
			    button = i;
			    break;
			}
		    }
		}
		/* Was a button pressed? */
		if(button > -1)
		{
		    char s[80];
		    sprintf(s, "%i", button + 1);
		    DRAW_STRING(
			x + x_offset,
			(15 / 2) - (fh / 2) + y + y_offset,
			s
		    );
		    x += fw * (STRLEN(s) + 1);
		}
		y += 15 + 5;

	    }

            js_num = 1;
	    if(gc->total_joysticks > js_num)
	    {
                SDL_Joystick *sdljoystick = gc->sdljoystick[js_num];
                int button = -1;

		glColor3f(0.0f, 1.0f, 0.0f);
		glRasterPos2i(
		    x + x_offset + x_min,
		    height - (15 + y + y_offset + y_min)
		);
		glBitmap(15, 15, 0.0f, 0.0f, 15.0f, 0.0f, btn_bm);
		x += 15 + 5;
		glColor3f(1.0f, 1.0f, 0.0f);

		/* This joystick initialized? */
		if (sdljoystick == NULL && SDL_NumJoysticks() > js_num){
                    sdljoystick = SDL_JoystickOpen(js_num);
                    gc->sdljoystick[js_num] = sdljoystick;
                }
		if (sdljoystick != NULL){
		    int i;
                    
		    /* Look for a currently pressed button */
		    for(i = 0; i < SDL_JoystickNumButtons(sdljoystick); i++)
		    {
			if(SDL_JoystickGetButton(sdljoystick,i))
			{
			    button = i;
			    break;
			}
		    }
		}
		/* Was a button pressed? */
		if(button > -1)
		{
		    char s[80];
		    sprintf(s, "%i", button + 1);
		    DRAW_STRING(
			x + x_offset,
			(15 / 2) - (fh / 2) + y + y_offset,
			s
		    );
		    x += fw * (STRLEN(s) + 1);
		}
		y += 15 + 5;

	    }


	    /* Joystick names, axises, and buttons */
	    x = 0;
	    y += 5;
	    for(js_num = 0; js_num < gc->total_joysticks; js_num++)
	    {
                SDL_Joystick *sdljoystick = gc->sdljoystick[js_num];
		if(sdljoystick != NULL)
		{
		    const char* s;
		    char ns[40];

		    /* Joystick name */
	
                    sprintf(ns, "SDL Joystick #%i", js_num);
                    s = ns;
                    glColor3f(1.0f, 1.0f, 0.0f);
                    DRAW_STRING(x + x_offset, y + y_offset, s);
                    y += fh + 2;
	       

		    /* Axises */
		    s = "Axises: ";
		    glColor3f(0.0f, 1.0f, 0.0f);
		    DRAW_STRING(x + x_offset, y + y_offset, s);
		    x += STRLEN(s) * fw;

		    sprintf(ns, "%i", SDL_JoystickNumAxes(sdljoystick));
		    s = ns;
		    glColor3f(1.0f, 1.0f, 0.0f);
		    DRAW_STRING(x + x_offset, y + y_offset, s);
		    x += STRLEN(s) * fw;

		    /* Buttons */
		    s = "  Buttons: ";
		    glColor3f(0.0f, 1.0f, 0.0f);
		    DRAW_STRING(x + x_offset, y + y_offset, s);
		    x += STRLEN(s) * fw;

		    sprintf(ns, "%i", SDL_JoystickNumButtons(sdljoystick));
		    s = ns;
		    glColor3f(1.0f, 1.0f, 0.0f);
		    DRAW_STRING(x + x_offset, y + y_offset, s);
		    x += STRLEN(s) * fw;

		    y += fh;
		}

		y += 5;
	    }



	}
	else
	{
	    /* Game controller not initialized to joystick */
	    const char s[] = "Joystick Not Initialized";
	    int	x = (w / 2) - ((STRLEN(s) * fw) / 2),
		y = (h / 2) - (fh / 2);
	    glColor3f(1.0f, 1.0f, 0.0f);
	    DRAW_STRING(x, y, s);
	}

#undef DRAW_STRING
#undef SET_VERTEX
}


/*
 *	Gets graphics info and updates the graphics info menu's message
 *	box.
 */
void SARMenuOptionsGraphicsInfoRefresh(sar_core_struct *core_ptr)
{
	int gl_major, gl_minor, gl_release;
	char *mesg = NULL;
	gw_display_struct *display = (core_ptr != NULL) ?
	    core_ptr->display : NULL;
	int i = SARMatchMenuByName(
	    core_ptr, SAR_MENU_NAME_OPTIONS_GRAPHICS_INFO
	);
	sar_menu_struct *m = (i > -1) ? core_ptr->menu[i] : NULL;
	if((display == NULL) || (m == NULL))
	    return;

	/* Start by getting the GL version */
	if(SARGetGLVersion(&gl_major, &gl_minor, &gl_release))
	{
	    int n, strc, x, y, width, height;
	    void *window, *ctx;
	    char **strv;
	    char s[1024];
#ifdef X_H
	    Display *dpy = display->display;
	    const XVisualInfo *vi = display->visual_info;
#endif

	    /* Get GL context information */
	    GWContextGet(
		display, GWContextCurrent(display),
		&window, &ctx,
		&x, &y,
		&width, &height
	    );

	    /* Graphics Output */
	    sprintf(
		s,
"Graphics Output: <bold>OpenGL Version %i.%i.%i<default>\n",
		gl_major, gl_minor, gl_release
	    );
	    mesg = strcatalloc(mesg, s);

	    /* Vendor */
	    sprintf(
		s, "Vendor: <bold>%s<default>\n",
		SARGetGLVendorName()
	    );
	    mesg = strcatalloc(mesg, s);

	    /* Renderer */
	    sprintf(
		s, "Renderer: <bold>%s<default>\n",
		SARGetGLRendererName()
	    );
	    mesg = strcatalloc(mesg, s);

	    /* Rendering Method */
#ifdef X_H
	    sprintf(
		s, "Rendering Method: <bold>%s<default>\n",
		glXIsDirect(dpy, (GLXContext)ctx) ?
		    "Hardware" : "Software"
	    );
	    mesg = strcatalloc(mesg, s);
#endif
#ifdef __MSW__
/* TODO */
#endif

	    /* Window System */
#ifdef X_H
	    sprintf(s,
"Window System: <bold>X Window Systems Version %i.%i (GLX %i.%i)<default>\n",
		ProtocolVersion(dpy),
		ProtocolRevision(dpy),
		display->glx_version_major,
		display->glx_version_minor
	    );
	    mesg = strcatalloc(mesg, s);
	    sprintf(s,
"Window System Vendor: <bold>%s (Release %i)<default>\n",
		ServerVendor(dpy),
		VendorRelease(dpy)
	    );
	    mesg = strcatalloc(mesg, s);
	    if(True)
	    {
		const char *addr = DisplayString(dpy);
		if(addr == NULL)
		    addr = getenv("DISPLAY");
		sprintf(s,
"Display Address: <bold>%s (Connection #%i)<default>\n",
		    (addr != NULL) ? addr : "localhost",
		    ConnectionNumber(dpy)
		);
		mesg = strcatalloc(mesg, s);
	    }
#endif
#ifdef __MSW__
	    sprintf(s,
"Window System: <bold>Win32 Version %i.%i (GLW %i.%i)<default>\n",
		((WINVER & 0xff00) >> 8),
		((WINVER & 0x00ff) >> 0),
		display->glw_version_major,
		display->glw_version_minor
	    );
	    mesg = strcatalloc(mesg, s);
#endif

#ifdef X_H
	    /* Visual */
	    if(vi != NULL)
	    {
		int cmap_entries = 0;
		const char *class_name = "Unknown";
#if defined(__cplusplus) || defined(c_plusplus)
		switch(vi->c_class)
#else
		switch(vi->class)
#endif
		{
		  case StaticGray:
		    class_name = "StaticGray";
		    cmap_entries = vi->colormap_size;
		    break;
		  case GrayScale:
		    class_name = "GrayScale";
		    break;
		  case StaticColor:
		    class_name = "StaticColor";
		    cmap_entries = vi->colormap_size;
		    break;
		  case PseudoColor:
		    class_name = "PseudoColor";
		    cmap_entries = vi->colormap_size;
		    break;
		  case TrueColor:
		    class_name = "TrueColor";
		    break;
		  case DirectColor:
		    class_name = "DirectColor";
		    break;
		}
		if(cmap_entries > 0)
		    sprintf(
			s,
"Visual Class: <bold>%s (%i Colormap Entries)<default>\n",
			class_name, cmap_entries
		    );
		else
		    sprintf(
			s,
"Visual Class: <bold>%s<default>\n",
			class_name
		    );
		mesg = strcatalloc(mesg, s);
	    }
#endif

	    /* Frame Buffer */
#ifdef X_H
	    sprintf(
		s, "Frame Buffer: <bold>%ix%i RGB%s %ibits %s<default>\n",
		width, height,
		(display->alpha_channel_bits > 0) ? "A" : "",
		display->depth,
		(display->has_double_buffer) ? "Double" : "Single"
	    );
	    mesg = strcatalloc(mesg, s);
#endif
#ifdef __MSW__
	    sprintf(
		s, "Frame Buffer: <bold>%ix%i RGB%s  %ibits %s<default>\n",
		width, height,
		(display->alpha_channel_bits > 0) ? "A" : "",
		display->depth,
		(display->has_double_buffer) ? "Double" : "Single"
	    );
	    mesg = strcatalloc(mesg, s);
#endif

	    /* Desktop */
#ifdef X_H
	    if((display->root_width > 0) &&
	       (display->root_height > 0)
	    )
	    {
		sprintf(
		    s, "Root Window: <bold>%ix%i<default>\n",
		    display->root_width,
		    display->root_height
		);
		mesg = strcatalloc(mesg, s);
	    }
#endif
#ifdef __MSW__
	    if((display->root_width > 0) &&
	       (display->root_height > 0)
	    )
	    {
		sprintf(
		    s, "Desktop: <bold>%ix%i<default>\n",
		    display->root_width,
		    display->root_height
		);
		mesg = strcatalloc(mesg, s);
	    }
#endif

#ifdef X_H
	    /* XF86 video mode extension */
	    sprintf(
		s, "XF86 Video Mode Extension: <bold>%s<default>\n",
		(display->has_vidmode_ext) ? "Available" : "Not Available"
	    );
	    mesg = strcatalloc(mesg, s);
#endif

	    /* Video modes */
	    if(True)
	    {
		int i, n;
		gw_vidmode_struct *m_ptr, *m = GWVidModesGet(display, &n);
		if(m != NULL)
		{
		    strcpy(s, "Video Modes: ");
		    mesg = strcatalloc(mesg, s);

		    for(i = 0; i < n; i++)
		    {
			m_ptr = &m[i];
			sprintf(
			    s, "<bold>%ix%i<default>%s",
			    m_ptr->width, m_ptr->height,
			    (i < (n - 1)) ? " " : ""
			);
			mesg = strcatalloc(mesg, s);
		    }
		    mesg = strcatalloc(mesg, "\n");

		    GWVidModesFree(m, n);
		}
	    }

	    /* GL extensions */
	    strv = SARGelGLExtensionNames(&strc);
	    if(strv != NULL)
	    {
		strcpy(s, "Extensions: ");
		mesg = strcatalloc(mesg, s);
		for(n = 0; n < strc; n++)
		{
		    sprintf(
			s, "<bold>%s<default>%s",
			strv[n],
			(n < (strc - 1)) ? " " : ""
		    );
		    mesg = strcatalloc(mesg, s);
		}
		mesg = strcatalloc(mesg, "\n");
		strlistfree(strv, strc);
	    }
	}
	else
	{
	    char s[1024];

	    /* Unable to get GL version? */
	    strcpy(s, "Unable to obtain graphics information\n");
	    mesg = strcatalloc(mesg, s);
	}

	/* Set message */
	for(i = 0; i < m->total_objects; i++)
	{
	    if(!SAR_MENU_IS_MESSAGE_BOX(m->object[i]))
		continue;

	    SARMenuMessageBoxSet(
		display, m, i, mesg,
		(Boolean)((core_ptr->cur_menu == i) ? True : False)
	    );
	    break;
	}

	free(mesg);
}

/*
 *	Save graphics info test input prompt callback.
 */
static void SARMenuOptionsSaveGraphicsInfoCB(const char *v, void *data)
{
	int i;
	gw_display_struct *display;
	sar_menu_struct *m;
	sar_core_struct *core_ptr = SAR_CORE(data);
	if(core_ptr == NULL)
	    return;

	/* Redraw the current menu since the prompt has been unmapped */
	display = core_ptr->display;
	i = core_ptr->cur_menu;
	if((i >= 0) && (i < core_ptr->total_menus))
	    m = core_ptr->menu[i];
	else
	    m = NULL;
	SARMenuDrawAll(display, m);
	GWSwapBuffer(display);

	if(v != NULL)
	{
	    const char *filename = v;
	    sar_menu_message_box_struct *mesgbox = SAR_MENU_MESSAGE_BOX(
		SARMenuGetObjectByID(
		    m, SAR_MENU_ID_GRAPHICS_INFO_MESG, NULL
		)
	    );
	    if(mesgbox != NULL)
		SARMenuOptionsSaveMessageBox(
		    core_ptr, mesgbox->message, filename
		);
	}
}


/*
 *	Plays a test sound.
 */
void SARMenuOptionsSoundTest(sar_core_struct *core_ptr)
{
	char *sndobj;
	struct stat stat_buf;
	gw_display_struct *display = (core_ptr != NULL) ?
	    core_ptr->display : NULL;
	snd_recorder_struct *recorder = (core_ptr != NULL) ?
	    core_ptr->recorder : NULL;
	const char *sndobj_list[] = {
		SAR_DEF_SOUND_LAND_WHEEL_SKID,
		SAR_DEF_SOUND_CRASH_OBSTRUCTION,
		SAR_DEF_SOUND_SPLASH_AIRCRAFT,
		SAR_DEF_SOUND_LANDING_GEAR_DOWN,
		SAR_DEF_SOUND_LANDING_GEAR_UP,
		SAR_DEF_SOUND_HELICOPTER_ENGINE_START,
		SAR_DEF_SOUND_HELICOPTER_ENGINE_SHUTDOWN
	};
	static int last_sound_i = 0;
	if(display == NULL)
	    return;

	/* Sound off or was unable to connect? */
	if(recorder == NULL)
	{
	    GWOutputMessage(
		display,
		GWOutputMessageTypeError,
		"Sound Test",
"Sound is currently turned off.\n\
\n\
If you were unable to turn on the sound or increase\n\
the Sound Level then make sure that the sound server\n\
is configured properly and currently running.",
		NULL
	    );
	    return;
	}

	/* Generate sound object path */
	sndobj = STRDUP(PrefixPaths(
	    dname.global_data, sndobj_list[last_sound_i]
	));

	/* Increment sound index */
	last_sound_i++;
	if(last_sound_i >= (int)(sizeof(sndobj_list) / sizeof(char *)))
	    last_sound_i = 0;

	/* Check if sound object exists */
	if((sndobj != NULL) ? stat(sndobj, &stat_buf) : True)
	{
	    char *buf = (char *)malloc(
		(80 + STRLEN(sndobj)) * sizeof(char)
	    );
	    sprintf(
		buf,
		"No such file:\n\n    %s",
		sndobj
	    );
	    GWOutputMessage(
		display,
		GWOutputMessageTypeError,
		"Sound Test",
		buf,
		NULL
	    );
	    free(buf);
	    free(sndobj);
	    return;
	}

	/* Attempt to play sound */
	SoundStartPlayVoid(
	    recorder,
	    sndobj,		/* Full path to object */
	    1.0f, 1.0f,		/* Volume, from 0.0 to 1.0 */
	    1.0f,		/* Applied sample rate, can be 0 */
	    0			/* Any of SND_PLAY_OPTION_* */
	);

	free(sndobj);
}

/*
 *	Gets sound values and updates menu objects on the sound menu.
 */
void SARMenuOptionsSoundRefresh(sar_core_struct *core_ptr)
{
	gw_display_struct *display = (core_ptr != NULL) ?
	    core_ptr->display : NULL;
	snd_recorder_struct *recorder = (core_ptr != NULL) ?
	    core_ptr->recorder : NULL;
	Boolean redraw;
	float	master_volume = 0.0f,
		sound_volume = 0.0f,
		music_volume = 0.0f;
	void *o;
	int n;
	int i = SARMatchMenuByName(
	    core_ptr, SAR_MENU_NAME_OPTIONS_SOUND
	);
	sar_menu_struct *m = (i > -1) ? core_ptr->menu[i] : NULL;
	const sar_option_struct *opt;
	if((display == NULL) || (m == NULL))
	    return;

	opt = &core_ptr->option;

	/* Need redraw if the current menu is this menu */
	redraw = (core_ptr->cur_menu == i) ? True : False;

	/* Get volumes */
	if(recorder != NULL)
	{
	    float left, right;
	    SoundMixerGet(recorder, "volume", &left, &right);
	    master_volume = left;
	    SoundMixerGet(recorder, "wav", &left, &right);
	    sound_volume = left;
	    SoundMixerGet(recorder, "midi", &left, &right);
	    music_volume = left;
	}

	/* Iterate through all objects on this menu, refreshing the
	 * values of each one as needed.
	 */
	for(n = 0; n < m->total_objects; n++)
	{
	    o = m->object[n];
	    if(SAR_MENU_IS_SPIN(o))
	    {
		int vi;
		sar_menu_spin_struct *spin = SAR_MENU_SPIN(o);
		switch(spin->id)
		{
		  case SAR_MENU_ID_OPT_SOUND_PRIORITY:
		    switch(opt->sound_priority)
		    {
		      case SND_PRIORITY_FOREGROUND:
			vi = 1;
			break;
		      case SND_PRIORITY_PREEMPT:
			vi = 2;
			break;
		      default:	/* SND_PRIORITY_BACKGROUND */
			vi = 0;
			break;
		    }
		    SARMenuSpinSelectValueIndex(
			display, m, n, vi, redraw
		    );
		    break;
		}
	    }
	    else if(SAR_MENU_IS_SWITCH(o))
	    {
		sar_menu_switch_struct *sw = SAR_MENU_SWITCH(o);
		switch(sw->id)
		{
		  case SAR_MENU_ID_OPT_MUSIC:
		    SARMenuSwitchSetValue(
			display, m, n, opt->music, redraw
		    );
		    break;
		}
	    }
	    else if(SAR_MENU_IS_SLIDER(o))
	    {
		sar_menu_slider_struct *slider = SAR_MENU_SLIDER(o);
		switch(slider->id)
		{
		  case SAR_MENU_ID_OPT_VOLUME_MASTER:
		    SARMenuSliderSetValue(
			display, m, n, master_volume, redraw
		    );
		    break;
		  case SAR_MENU_ID_OPT_VOLUME_SOUND:
		    SARMenuSliderSetValue(
			display, m, n, sound_volume, redraw
		    );
		    break;
		  case SAR_MENU_ID_OPT_VOLUME_MUSIC:
		    SARMenuSliderSetValue(
			display, m, n, music_volume, redraw
		    );
		    break;
		}
	    }
	}
}

/*
 *	Gets sound info and updates the sound info menu's message
 *      box.
 */
void SARMenuOptionsSoundInfoRefresh(sar_core_struct *core_ptr)
{
	char *mesg = NULL;
	gw_display_struct *display = (core_ptr != NULL) ?
	    core_ptr->display : NULL;
	snd_recorder_struct *recorder = (core_ptr != NULL) ?
	    core_ptr->recorder : NULL;
	int i = SARMatchMenuByName(
	    core_ptr, SAR_MENU_NAME_OPTIONS_SOUND_INFO
	);
	sar_menu_struct *m = (i > -1) ? core_ptr->menu[i] : NULL;
	const sar_option_struct *opt;
	if((display == NULL) || (m == NULL))
	    return;

	opt = &core_ptr->option;

	/* Recorder Address */
	if((recorder != NULL) && (core_ptr->recorder_address != NULL))
	{
	    mesg = strcatalloc(mesg, "Recorder Address: <bold>");
	    mesg = strcatalloc(mesg, core_ptr->recorder_address);
	    mesg = strcatalloc(mesg, "<default>\n");
	}

	/* Sound Output */
	mesg = strcatalloc(mesg, "Sound Output: <bold>");
	if(recorder != NULL)
	{
          mesg = strcatalloc(mesg, "SDL Mixer");
	}
	else
	{
	    mesg = strcatalloc(mesg, "*Off*");
	}
	mesg = strcatalloc(mesg, "<default>\n");

	/* Audio Mode */
	if((recorder != NULL) && (core_ptr->audio_mode_name != NULL))
	{
	    mesg = strcatalloc(mesg, "Audio Mode: <bold>");
	    mesg = strcatalloc(mesg, core_ptr->audio_mode_name);
	    mesg = strcatalloc(mesg, "<default>\n");
	}

	/* Sound Support */
	mesg = strcatalloc(mesg, "Sound Support:<bold>");
        mesg = strcatalloc(mesg, " SDL");
	mesg = strcatalloc(mesg, "<default>\n");


	/* Set message */
	for(i = 0; i < m->total_objects; i++)
	{
	    if(!SAR_MENU_IS_MESSAGE_BOX(m->object[i]))
		continue;

	    SARMenuMessageBoxSet(
		display, m, i, mesg,
		(Boolean)((core_ptr->cur_menu == i) ? True : False)
	    );
	    break;
	}

	free(mesg);
}

/*
 *      Save sound info test input prompt callback.
 */
static void SARMenuOptionsSaveSoundInfoCB(const char *v, void *data)
{
	int i;
	gw_display_struct *display;
	sar_menu_struct *m;
	sar_core_struct *core_ptr = SAR_CORE(data);
	if(core_ptr == NULL)
	    return;

	/* Redraw the current menu since the prompt has been unmapped */
	display = core_ptr->display;
	i = core_ptr->cur_menu;
	if((i >= 0) && (i < core_ptr->total_menus))
	    m = core_ptr->menu[i];
	else
	    m = NULL;
	SARMenuDrawAll(display, m);
	GWSwapBuffer(display);

	/* Got file name? */
	if(v != NULL)
	{
	    const char *filename = v;
	    sar_menu_message_box_struct *mesgbox = SAR_MENU_MESSAGE_BOX(
		SARMenuGetObjectByID(
		    m, SAR_MENU_ID_SOUND_INFO_MESG, NULL
		)
	    );
	    if(mesgbox != NULL)
		SARMenuOptionsSaveMessageBox(
		    core_ptr, mesgbox->message, filename
		);
	}
}


/*
 *	Sets global option structure values based on values from the
 *	options menu widget/objects.
 */
void SARMenuOptionsApply(sar_core_struct *core_ptr)
{
	int i;
	sar_menu_struct *m;


	if(core_ptr == NULL)
	    return;

	/* Note that most option values are applied when the menu object
	 * value is modified, so this function is only needed to apply
	 * values from menu object values that are not applied when
 	 * immediatedly modified.
	 */

	/* Options general */
	i = SARMatchMenuByName(core_ptr, SAR_MENU_NAME_OPTIONS);
	m = (i > -1) ? core_ptr->menu[i] : NULL;
	if(m != NULL)
	{

	}

	/* Options controller */
	i = SARMatchMenuByName(core_ptr, SAR_MENU_NAME_OPTIONS_CONTROLLER);
	m = (i > -1) ? core_ptr->menu[i] : NULL;
	if(m != NULL)
	{

	}

	/* Options graphics */
	i = SARMatchMenuByName(core_ptr, SAR_MENU_NAME_OPTIONS_GRAPHICS);
	m = (i > -1) ? core_ptr->menu[i] : NULL;
	if(m != NULL)
	{

	}

	/* Options sounds */
	i = SARMatchMenuByName(core_ptr, SAR_MENU_NAME_OPTIONS_SOUND);
	m = (i > -1) ? core_ptr->menu[i] : NULL;
	if(m != NULL)
	{

	}
}

/*
 *	Gets values from global options and sets them to the objects in
 *	the widget/objects on the options menus.
 */
void SARMenuOptionsFetch(sar_core_struct *core_ptr)
{
	int i, n;
	gw_display_struct *display;
	sar_menu_struct *m;
	sar_menu_switch_struct *sw;
	sar_menu_spin_struct *spin;
	sar_menu_slider_struct *slider;
	const sar_option_struct *opt;

	if(core_ptr == NULL)
	    return;

	display = core_ptr->display;
	opt = &core_ptr->option;

	/* Menu: Options */
	i = SARMatchMenuByName(core_ptr, SAR_MENU_NAME_OPTIONS);
	m = (i > -1) ? core_ptr->menu[i] : NULL;
	if(m != NULL)
	{

	}

	/* Menu: Options->Simulation */
	i = SARMatchMenuByName(core_ptr, SAR_MENU_NAME_OPTIONS_SIMULATION);
	m = (i > -1) ? core_ptr->menu[i] : NULL;
	if(m != NULL)
	{
	    /* Units */
	    spin = SARMenuOptionsGetSpinByID(
		m, SAR_MENU_ID_OPT_UNITS, NULL
	    );
	    if(spin != NULL)
	    {
		switch(opt->units)
		{
		  case SAR_UNITS_ENGLISH:
		    spin->cur_value = 0;
		    break;
		  case SAR_UNITS_METRIC:
		    spin->cur_value = 1;
		    break;
		  case SAR_UNITS_METRIC_ALT_FEET:
		    spin->cur_value = 2;
		    break;
		}
	    }

	    /* Hoist contact, the expansion coefficient from the hoist
	     * deployment's actual size
	     */
	    spin = SARMenuOptionsGetSpinByID(
		m, SAR_MENU_ID_OPT_HOIST_CONTACT, NULL
	    );
	    if(spin != NULL)
	    {
		if(opt->hoist_contact_expansion_coeff >= 4.0f)
		    spin->cur_value = 0;
		else if(opt->hoist_contact_expansion_coeff >= 2.0f)
		    spin->cur_value = 1;
		else if(opt->hoist_contact_expansion_coeff >= 1.0f)
		    spin->cur_value = 2;
		else
		    spin->cur_value = 0;
	    }

	    /* Damage resistance, how much of an impact can player
	     * aircraft tolorate from actual coefficient
	     */
	    spin = SARMenuOptionsGetSpinByID(
		m, SAR_MENU_ID_OPT_DAMAGE_RESISTANCE, NULL
	    );
	    if(spin != NULL)
	    {
		if(opt->damage_resistance_coeff >= 2.0f)
		    spin->cur_value = 0;
		else if(opt->damage_resistance_coeff >= 1.5f)
		    spin->cur_value = 1;
		else
		    spin->cur_value = 2;
	    }

	    /* Flight dynamics difficulty */
	    spin = SARMenuOptionsGetSpinByID(
		m, SAR_MENU_ID_OPT_FLIGHT_PHYSICS, NULL
	    );
	    if(spin != NULL)
	    {
		switch(opt->flight_physics_level)
		{
		  case FLIGHT_PHYSICS_REALISTIC:
		    spin->cur_value = 2;
		    break;
		  case FLIGHT_PHYSICS_MODERATE:
		    spin->cur_value = 1;
		    break;
		  case FLIGHT_PHYSICS_EASY:
		    spin->cur_value = 0;
		    break;
		}
	    }
	}

	/* Menu: Options->Controller */
	i = SARMatchMenuByName(core_ptr, SAR_MENU_NAME_OPTIONS_CONTROLLER);
	m = ((i < 0) ? NULL : core_ptr->menu[i]);
	if(m != NULL)
	{
	    int connection, priority;
	    gctl_js_axis_roles axis_roles;


	    /* Joystick 1 (js0) connection type and axis roles */
	    /* Connection type */
	    connection = opt->js0_connection;
	    spin = SARMenuOptionsGetSpinByID(
		m, SAR_MENU_ID_OPT_JS0_CONNECTION, NULL
	    );
	    if(spin != NULL)
	    {
		switch(connection)
		{
		  case GCTL_JS_CONNECTION_USB:
		    spin->cur_value = 1;
		    break;
		  default:	/* GCTL_JS_CONNECTION_STANDARD */
		    spin->cur_value = 0;
		    break;
		}
	    }
	    /* Axis roles */
	    axis_roles = opt->gctl_js0_axis_roles;
	    spin = SARMenuOptionsGetSpinByID(
		m, SAR_MENU_ID_OPT_JS0_AXISES, NULL
	    );
	    if(spin != NULL)
	    {
		/* These value codes should match those in SARBuildMenus()
		 * that added these values.
		 */
		if(axis_roles)
		{
		    if(axis_roles & GCTL_JS_AXIS_ROLE_AS_THROTTLE_AND_RUDDER)
			spin->cur_value = 9;
		    else if((axis_roles & GCTL_JS_AXIS_ROLE_THROTTLE) &&
			    (axis_roles & GCTL_JS_AXIS_ROLE_HEADING) &&
			    (axis_roles & GCTL_JS_AXIS_ROLE_HAT)
		    )
			spin->cur_value = 8;
		    else if((axis_roles & GCTL_JS_AXIS_ROLE_HEADING) &&
			    (axis_roles & GCTL_JS_AXIS_ROLE_HAT)
		    )
			spin->cur_value = 7;
		    else if((axis_roles & GCTL_JS_AXIS_ROLE_HEADING) &&
			    (axis_roles & GCTL_JS_AXIS_ROLE_THROTTLE)
		    )
			spin->cur_value = 6;
		    else if(axis_roles & GCTL_JS_AXIS_ROLE_HEADING)
			spin->cur_value = 5;
		    else if((axis_roles & GCTL_JS_AXIS_ROLE_THROTTLE) &&
			    (axis_roles & GCTL_JS_AXIS_ROLE_HAT)
		    )
			spin->cur_value = 4;
		    else if(axis_roles & GCTL_JS_AXIS_ROLE_HAT)
			spin->cur_value = 3;
		    else if(axis_roles & GCTL_JS_AXIS_ROLE_THROTTLE) 
			spin->cur_value = 2;
		    else
			spin->cur_value = 1;
		}
		else
		{
		    spin->cur_value = 0;
		}
	    }

	    /* Joystick 2 (js1) connection type and axis roles */
	    /* Connection type */
	    connection = opt->js1_connection;
	    spin = SARMenuOptionsGetSpinByID(
		m, SAR_MENU_ID_OPT_JS1_CONNECTION, NULL
	    );
	    if(spin != NULL)
	    {
		switch(connection)
		{
		  case GCTL_JS_CONNECTION_USB:
		    spin->cur_value = 1;
		    break;
		  default:      /* GCTL_JS_CONNECTION_STANDARD */
		    spin->cur_value = 0;
		    break;
		}
	    }
	    /* Axis roles */
	    axis_roles = opt->gctl_js1_axis_roles;
	    spin = SARMenuOptionsGetSpinByID(
		m, SAR_MENU_ID_OPT_JS1_AXISES, NULL
	    );
	    if(spin != NULL)
	    {
		/* These value codes should match those in SARBuildMenus()
		 * that added these values.
		 */
		if(axis_roles)
		{
		    if(axis_roles & GCTL_JS_AXIS_ROLE_AS_THROTTLE_AND_RUDDER)
			spin->cur_value = 9;
		    else if((axis_roles & GCTL_JS_AXIS_ROLE_THROTTLE) &&
			    (axis_roles & GCTL_JS_AXIS_ROLE_HEADING) &&
			    (axis_roles & GCTL_JS_AXIS_ROLE_HAT)
		    )
			spin->cur_value = 8;
		    else if((axis_roles & GCTL_JS_AXIS_ROLE_HEADING) &&
			    (axis_roles & GCTL_JS_AXIS_ROLE_HAT)
		    )
			spin->cur_value = 7;
		    else if((axis_roles & GCTL_JS_AXIS_ROLE_HEADING) &&
			    (axis_roles & GCTL_JS_AXIS_ROLE_THROTTLE)
		    )
			spin->cur_value = 6;
		    else if(axis_roles & GCTL_JS_AXIS_ROLE_HEADING)
			spin->cur_value = 5;
		    else if((axis_roles & GCTL_JS_AXIS_ROLE_THROTTLE) &&
			    (axis_roles & GCTL_JS_AXIS_ROLE_HAT)
		    )
			spin->cur_value = 4;
		    else if(axis_roles & GCTL_JS_AXIS_ROLE_HAT)
			spin->cur_value = 3;
		    else if(axis_roles & GCTL_JS_AXIS_ROLE_THROTTLE)
			spin->cur_value = 2;
		    else
			spin->cur_value = 1;
		}
		else
		{
		    spin->cur_value = 0;
		}
	    }

	    /* Joystick priority */
	    priority = opt->js_priority;
	    spin = SARMenuOptionsGetSpinByID(
		m, SAR_MENU_ID_OPT_JS_PRIORITY, NULL
	    );
	    if(spin != NULL)
	    {
		switch(priority)
		{
		  case GCTL_JS_PRIORITY_FOREGROUND:
		    spin->cur_value = 1;
		    break;
		  case GCTL_JS_PRIORITY_PREEMPT:
		    spin->cur_value = 2;
		    break;
		  default:
		    spin->cur_value = 0;
		    break;
		}
	    }
	}

	/* Menu: Options->Controller->Buttons */
	i = SARMatchMenuByName(
	    core_ptr, SAR_MENU_NAME_OPTIONS_CONTROLLER_JS_BTN
	);
	m = (i > -1) ? core_ptr->menu[i] : NULL;
	if(m != NULL)
	{
	    sar_menu_spin_struct *btn_action_spin;


	    /* Joystick 1 (js0) button action */
	    btn_action_spin = SARMenuOptionsGetSpinByID(
		m, SAR_MENU_ID_OPT_JS0_BUTTON_ACTION, NULL
	    );
	    spin = SARMenuOptionsGetSpinByID(
		m, SAR_MENU_ID_OPT_JS0_BUTTON_NUMBER, NULL
	    );
	    if((spin != NULL) && (btn_action_spin != NULL))
	    {
		spin->cur_value = SARGetJSButtonFromButtonRoleSpin(
		    core_ptr,
		    btn_action_spin, SAR_MENU_ID_OPT_JS0_BUTTON_ACTION
		) + 1;
		if(spin->cur_value >= spin->total_values)
		    spin->cur_value = 0;
	    }

	    /* Joystick 2 (js1) button actions */
	    btn_action_spin = SARMenuOptionsGetSpinByID(  
		m, SAR_MENU_ID_OPT_JS1_BUTTON_ACTION, NULL
	    );
	    spin = SARMenuOptionsGetSpinByID(
		m, SAR_MENU_ID_OPT_JS1_BUTTON_NUMBER, NULL
	    );
	    if((spin != NULL) && (btn_action_spin != NULL))
	    {
		spin->cur_value = SARGetJSButtonFromButtonRoleSpin(
		    core_ptr,
		    btn_action_spin, SAR_MENU_ID_OPT_JS1_BUTTON_ACTION
		) + 1;
		if(spin->cur_value >= spin->total_values)
		    spin->cur_value = 0;
	    }
	}

	/* Menu: Options->Graphics */
	i = SARMatchMenuByName(core_ptr, SAR_MENU_NAME_OPTIONS_GRAPHICS);
	m = (i > -1) ? core_ptr->menu[i] : NULL;
	if(m != NULL)
	{
	    /* Textured ground switch */
	    sw = SARMenuOptionsGetSwitchByID(
		m, SAR_MENU_ID_OPT_GROUND_TEXTURE, NULL
	    );
	    if(sw != NULL)
		sw->state = opt->textured_ground;

	    /* Textured objects switch */
	    sw = SARMenuOptionsGetSwitchByID(
		m, SAR_MENU_ID_OPT_OBJECT_TEXTURE, NULL
	    );
	    if(sw != NULL)
		sw->state = opt->textured_objects;

	    /* Textured clouds switch */
	    sw = SARMenuOptionsGetSwitchByID(
		m, SAR_MENU_ID_OPT_CLOUDS, NULL
	    );
	    if(sw != NULL)
		sw->state = opt->textured_clouds;

	    /* Atmosphere switch */
	    sw = SARMenuOptionsGetSwitchByID(
		m, SAR_MENU_ID_OPT_ATMOSPHERE, NULL
	    );
	    if(sw != NULL)
		sw->state = opt->atmosphere;

	    /* Dual pass depth switch */
	    sw = SARMenuOptionsGetSwitchByID(
		m, SAR_MENU_ID_OPT_DUAL_PASS_DEPTH, NULL
	    );
	    if(sw != NULL)
		sw->state = opt->dual_pass_depth;

	    /* Prop wash switch */
	    sw = SARMenuOptionsGetSwitchByID(
		m, SAR_MENU_ID_OPT_PROP_WASH, NULL
	    );
	    if(sw != NULL)
		sw->state = opt->prop_wash;

	    /* Smoke trails switch */
	    sw = SARMenuOptionsGetSwitchByID(
		m, SAR_MENU_ID_OPT_SMOKE_TRAILS, NULL
	    );
	    if(sw != NULL)
		sw->state = opt->smoke_trails;

	    /* Celestial objects switch */
	    sw = SARMenuOptionsGetSwitchByID(
		m, SAR_MENU_ID_OPT_CELESTIAL_OBJECTS, NULL
	    );
	    if(sw != NULL)
		sw->state = opt->celestial_objects;

	    /* Visibility */
	    spin = SARMenuOptionsGetSpinByID(
		m, SAR_MENU_ID_OPT_VISIBILITY_MAX, &n
	    );
	    if(spin != NULL)
	    {
		int v = opt->visibility_max;

		SARMenuSpinDeleteAllValues(m, n);
		switch(opt->units)
		{
		  case SAR_UNITS_ENGLISH:
		    SARMenuSpinAddValue(m, n, "3 miles");
		    SARMenuSpinAddValue(m, n, "6 miles");
		    SARMenuSpinAddValue(m, n, "9 miles");
		    SARMenuSpinAddValue(m, n, "12 miles");
		    SARMenuSpinAddValue(m, n, "15 miles");
		    SARMenuSpinAddValue(m, n, "18 miles");
		    SARMenuSpinAddValue(m, n, "21 miles");
		    break;
		  case SAR_UNITS_METRIC:
		  case SAR_UNITS_METRIC_ALT_FEET:
		    SARMenuSpinAddValue(m, n, "4.8 km");
		    SARMenuSpinAddValue(m, n, "9.7 km");
		    SARMenuSpinAddValue(m, n, "14.5 km");
		    SARMenuSpinAddValue(m, n, "19.3 km");
		    SARMenuSpinAddValue(m, n, "24.1 km");
		    SARMenuSpinAddValue(m, n, "29.0 km");
		    SARMenuSpinAddValue(m, n, "33.8 km");
		    break;
		}

		if((v >= 0) && (v < spin->total_values))
		    spin->cur_value = v;
		else
		    spin->cur_value = 0;
	    } 

	    /* Graphics Acceleration */
	    slider = SARMenuOptionsGetSliderByID(
		m, SAR_MENU_ID_OPT_GRAPHICS_ACCELERATION, &n
	    );
	    if(slider != NULL)
	    {
		SARMenuSliderSetValue(
		    display, m, n, opt->graphics_acceleration,
		    False
		);
	    }

	    /* Resolution */
	    spin = SARMenuOptionsGetSpinByID(
		m, SAR_MENU_ID_OPT_RESOLUTION, NULL
	    );
	    if(spin != NULL)
	    {
		int	v = opt->last_width,
			sv;
		if(v <= 100)
		    sv = 0;
		else if(v <= 320)
		    sv = 1;
		else if(v <= 640)
		    sv = 2;
		else if(v <= 800)
		    sv = 3;
		else if(v <= 1024)
		    sv = 4;
                else if (v <= 1280)
                   sv = 5;
                else if (v <- 1366)
                   sv = 6;
		else
		    sv = 0;
		spin->cur_value = CLIP(sv, 0, spin->total_values - 1);;
	    }

	    /* Full screen switch */
	    sw = SARMenuOptionsGetSwitchByID(
		m, SAR_MENU_ID_OPT_FULLSCREEN, NULL
	    );
	    if(sw != NULL)
		sw->state = opt->last_fullscreen;
	}

	/* Menu: Options->Sound */
	i = SARMatchMenuByName(core_ptr, SAR_MENU_NAME_OPTIONS_SOUND);
	m = (i > -1) ? core_ptr->menu[i] : NULL;
	if(m != NULL)
	{
	    /* Sounds level */
	    spin = SARMenuOptionsGetSpinByID(
		m, SAR_MENU_ID_OPT_SOUND_LEVEL, NULL
	    );
	    if(spin != NULL)
	    {
		/* No recorder (implies all sounds off)? */
		if(core_ptr->recorder == NULL)
		{
		    spin->cur_value = 0;
		}
		/* All sounds on? */
		else if(opt->engine_sounds &&
			opt->event_sounds &&
			opt->voice_sounds
		)
		{
		    spin->cur_value = 3;
		}
		/* Engine sounds only? */
		else if(opt->engine_sounds)
		{
		    spin->cur_value = 2;
		}
		/* Event sounds only? */
		else if(opt->event_sounds)
		{
		    spin->cur_value = 1;
		}
		/* All else no sounds */
		else
		{
		    spin->cur_value = 0;
		}
	    }

	    /* Music */
#if 0
	    sw = SARMenuOptionsGetSwitchByID(
		m, SAR_MENU_ID_OPT_MUSIC, NULL
	    );
	    if(sw != NULL)
		sw->state = opt->music;
#endif
	}
}

/*
 *	Options menu button callback.
 *
 *	Buttons that switch menus should use SARMenuButtonCB() instead.
 */
void SARMenuOptionsButtonCB(
	void *object, int id, void *client_data
)
{
	gw_display_struct *display;
	sar_core_struct *core_ptr = SAR_CORE(client_data);
	if((core_ptr == NULL) || (object == NULL))
	    return;

	display = core_ptr->display;

	switch(id)
	{
	  case SAR_MENU_ID_OPT_CONTROLLER_REFRESH:
	    SARMenuOptionsJoystickReinit(core_ptr);
	    break;

	  case SAR_MENU_ID_OPT_GRAPHICS_INFO_SAVE:
	    if(core_ptr->text_input != NULL)
	    {
		text_input_struct *p = core_ptr->text_input;
		if(!SARTextInputIsMapped(p))
		{
		    char path[PATH_MAX + NAME_MAX + 1];
		    if(getcwd(path, PATH_MAX))
			path[PATH_MAX - 1] = '\0';
		    else
			*path = '\0';
		    strncat(path, "/gfxinfo.txt", NAME_MAX);

		    SARTextInputMap(
			p, "File Name", path,
			SARMenuOptionsSaveGraphicsInfoCB, core_ptr
		    );
		    SARTextInputDraw(p);
		    GWSwapBuffer(display);
		}
	    }
	    break;

	  case SAR_MENU_ID_OPT_SOUND_TEST:
	    SARMenuOptionsSoundTest(core_ptr);
	    break;

	  case SAR_MENU_ID_OPT_SOUND_INFO_SAVE:
	    if(core_ptr->text_input != NULL)
	    {
		text_input_struct *p = core_ptr->text_input;
		if(!SARTextInputIsMapped(p))
		{
		    char path[PATH_MAX + NAME_MAX + 1];
		    if(getcwd(path, PATH_MAX))
			path[PATH_MAX - 1] = '\0';
		    else
			*path = '\0';
		    strncat(path, "/sndinfo.txt", NAME_MAX);

		    SARTextInputMap(
			p, "File Name", path,
			SARMenuOptionsSaveSoundInfoCB, core_ptr
		    );
		    SARTextInputDraw(p);
		    GWSwapBuffer(display);
		}
	    }
	    break;

	}
}

/*         
 *      Options menu switch callback.
 */
void SARMenuOptionsSwitchCB(
	void *object, int id, void *client_data,
	Boolean state
)
{
	sar_core_struct *core_ptr = SAR_CORE(client_data);
	gw_display_struct *display;
	sar_menu_switch_struct *sw = (sar_menu_switch_struct *)object;
	sar_option_struct *opt;
	if((core_ptr == NULL) || (sw == NULL))
	    return;

	display = core_ptr->display;
	opt = &core_ptr->option;

	/* Handle by switch's ID code */
	switch(id)
	{
	  case SAR_MENU_ID_OPT_GROUND_TEXTURE:
	    opt->textured_ground = state;
	    break;

	  case SAR_MENU_ID_OPT_OBJECT_TEXTURE:
	    opt->textured_objects = state;
	    if(!state)
	    {
		/* Print warning about object texture if off */
		GWOutputMessage(
		    core_ptr->display, GWOutputMessageTypeWarning,
		    "Object Texture Disabling Warning",
"Disabling object texturing may affect ground plot\n\
objects and make flight visualization extremely difficult.",
"Ground plot objects and visualization important objects\n\
will not be texturized. It will also be extremely difficult\n\
to visually interprite heightfield objects."
		);
	    }
	    break;

	  case SAR_MENU_ID_OPT_CLOUDS:
	    opt->textured_clouds = state;
	    break;

	  case SAR_MENU_ID_OPT_ATMOSPHERE:
	    opt->atmosphere = state;  
	    break;
 
	  case SAR_MENU_ID_OPT_DUAL_PASS_DEPTH:
	    opt->dual_pass_depth = state;
	    break;

	  case SAR_MENU_ID_OPT_PROP_WASH:
	    opt->prop_wash = state;
	    break;

	  case SAR_MENU_ID_OPT_SMOKE_TRAILS:
	    opt->smoke_trails = state;
	    break;

	  case SAR_MENU_ID_OPT_CELESTIAL_OBJECTS:
	    opt->celestial_objects = state;
	    break;

	  case SAR_MENU_ID_OPT_FULLSCREEN:
	    if(state != opt->last_fullscreen)
		SARFullScreen(core_ptr);
	    break;

	  case SAR_MENU_ID_OPT_MUSIC:
	    opt->music = state;
	    break;
	}
}
 
/*
 *      Options menu spin callback.
 */
void SARMenuOptionsSpinCB(
	void *object, int id, void *client_data,
	char *value
)
{
	const char *sound_server_connect_arg = "127.0.0.1:9433";
	sar_menu_struct *m;
	gw_display_struct *display;
	sar_core_struct *core_ptr = SAR_CORE(client_data);
	sar_menu_spin_struct *spin = SAR_MENU_SPIN(object);               
	sar_option_struct *opt;
	if((core_ptr == NULL) || (spin == NULL))
	    return;

	display = core_ptr->display;
	opt = &core_ptr->option;

	/* Sound server recorder address available? */
	if(core_ptr->recorder_address != NULL)
	    sound_server_connect_arg = (const char *)core_ptr->recorder_address;

	/* Get current menu */
	m = SARGetCurrentMenuPtr(core_ptr);
	if(m == NULL)
	    return;

	/* Handle by spin ID code */
	switch(id)
	{
	  case SAR_MENU_ID_OPT_UNITS:
	    switch(spin->cur_value)
	    {
	      case 0:
		opt->units = SAR_UNITS_ENGLISH;
		break;
	      case 1:
		opt->units = SAR_UNITS_METRIC;
		break;
	      case 2:
		opt->units = SAR_UNITS_METRIC_ALT_FEET;
		break;
	    }
	    /* Need to reget option values since changing units
	     * requires that some values be reloaded
	     */
	    SARMenuOptionsFetch(core_ptr);
	    break;

	  case SAR_MENU_ID_OPT_HOIST_CONTACT:
	    /* These values should correspond to those used to
	     * set the values in SARBuildMenus()
	     */
	    switch(spin->cur_value)
	    {
	      case 1:	/* Moderate */
		opt->hoist_contact_expansion_coeff = 2.0f;
		break;

	      case 2:	/* Authentic/difficult */
		opt->hoist_contact_expansion_coeff = 1.0f;
		break;

	      default:	/* Easy */
		opt->hoist_contact_expansion_coeff = 4.0f;
		break;
	    }
	    break;

	  case SAR_MENU_ID_OPT_DAMAGE_RESISTANCE:
	    /* These values should correspond to those used to
	     * set the values in SARBuildMenus().
	     */
	    switch(spin->cur_value)
	    {
	      case 1:   /* Nominal */
		opt->damage_resistance_coeff = 1.5f;
		break;

	      case 2:   /* Authentic/difficult */
		opt->damage_resistance_coeff = 1.0f;
		break;
		    
	      default:	/* Strong/easy */
		opt->damage_resistance_coeff = 2.0f;
		break;
	    }   
	    break;

	  case SAR_MENU_ID_OPT_FLIGHT_PHYSICS:
	    /* These values should correspond to those used to
	     * set the values in SARBuildMenus()
	     */
	    switch(spin->cur_value)
	    {
	      case 1:   /* Moderate */
		opt->flight_physics_level = FLIGHT_PHYSICS_MODERATE;
		break;
	      case 2:   /* Realistic/difficult */
		opt->flight_physics_level = FLIGHT_PHYSICS_REALISTIC;
		break;
	      default:  /* Easy */
		opt->flight_physics_level = FLIGHT_PHYSICS_EASY;
		break;
	    }
	    break;

	  case SAR_MENU_ID_OPT_JS0_CONNECTION:
	    /* These values should correspond to those used to
	     * set the values in SARBuildMenus().
	     */
#define JS_CONNECTION	opt->js0_connection
	    switch(spin->cur_value)
	    {
	      case 1:   /* USB */
		JS_CONNECTION = GCTL_JS_CONNECTION_USB;
		break;
	      default:	/* Standard */
		JS_CONNECTION = GCTL_JS_CONNECTION_STANDARD;
		break;
	    }
#undef JS_CONNECTION
	    /* Update options.controllers mask and reinitialize the
	     * game controller.
	     */
	    SARMenuOptionsJoystickReinit(core_ptr);
	    break;

	  case SAR_MENU_ID_OPT_JS1_CONNECTION:
	    /* These values should correspond to those used to
	     * set the values in SARBuildMenus().
	     */
#define JS_CONNECTION	opt->js1_connection
	    switch(spin->cur_value)
	    {
	      case 1:   /* USB */
		JS_CONNECTION = GCTL_JS_CONNECTION_USB;
		break;
	      default:  /* Standard */
		JS_CONNECTION = GCTL_JS_CONNECTION_STANDARD;
		break;
	    }
#undef JS_CONNECTION
	    /* Update options.controllers mask and reinitialize the
	     * game controller.
	     */
	    SARMenuOptionsJoystickReinit(core_ptr);
	    break;

	  case SAR_MENU_ID_OPT_JS0_AXISES:
	    /* These values should correspond to those used to
	     * set the values in SARBuildMenus().
	     */
#define JS_AXIS_ROLES	opt->gctl_js0_axis_roles
	    JS_AXIS_ROLES  = 0;
	    switch(spin->cur_value)
	    {
	      case 1:   /* 2D */
		JS_AXIS_ROLES |= GCTL_JS_AXIS_ROLE_PITCH |
		    GCTL_JS_AXIS_ROLE_BANK;
		break;

	      case 2:   /* 2D with throttle */ 
		JS_AXIS_ROLES |= GCTL_JS_AXIS_ROLE_PITCH |
		    GCTL_JS_AXIS_ROLE_BANK | GCTL_JS_AXIS_ROLE_THROTTLE;
		break;

	      case 3:   /* 2D with hat */
		JS_AXIS_ROLES |= GCTL_JS_AXIS_ROLE_PITCH |
		    GCTL_JS_AXIS_ROLE_BANK | GCTL_JS_AXIS_ROLE_HAT;
		break;

	      case 4:   /* 2D with throttle & hat */
		JS_AXIS_ROLES |= GCTL_JS_AXIS_ROLE_PITCH |
		    GCTL_JS_AXIS_ROLE_BANK | GCTL_JS_AXIS_ROLE_THROTTLE |
		    GCTL_JS_AXIS_ROLE_HAT;
		break;
	    
	      case 5:   /* 3D */
		JS_AXIS_ROLES |= GCTL_JS_AXIS_ROLE_PITCH |
		    GCTL_JS_AXIS_ROLE_BANK | GCTL_JS_AXIS_ROLE_HEADING;
		break;
		
	      case 6:   /* 3D with throttle */
		JS_AXIS_ROLES |= GCTL_JS_AXIS_ROLE_PITCH |
		    GCTL_JS_AXIS_ROLE_BANK | GCTL_JS_AXIS_ROLE_HEADING |
		    GCTL_JS_AXIS_ROLE_THROTTLE;
		break;
	
	      case 7:   /* 3D with hat */
		JS_AXIS_ROLES |= GCTL_JS_AXIS_ROLE_PITCH |
		    GCTL_JS_AXIS_ROLE_BANK | GCTL_JS_AXIS_ROLE_HEADING |
		    GCTL_JS_AXIS_ROLE_HAT;
		break;
	      
	      case 8:   /* 3D with throttle & hat */
		JS_AXIS_ROLES |= GCTL_JS_AXIS_ROLE_PITCH |
		    GCTL_JS_AXIS_ROLE_BANK | GCTL_JS_AXIS_ROLE_HEADING |
		    GCTL_JS_AXIS_ROLE_THROTTLE | GCTL_JS_AXIS_ROLE_HAT;
		break;
		
	      case 9:   /* As throttle & rudder */
		JS_AXIS_ROLES |= GCTL_JS_AXIS_ROLE_AS_THROTTLE_AND_RUDDER;
		break;
	    }
#undef JS_AXIS_ROLES
	    /* Update options.controllers mask and reinitialize the
	     * game controller.
	     */
	    SARMenuOptionsJoystickReinit(core_ptr);
	    break;
		    
	  case SAR_MENU_ID_OPT_JS1_AXISES:
	    /* These values should correspond to those used to
	     * set the values in SARBuildMenus().
	     */
#define JS_AXIS_ROLES	opt->gctl_js1_axis_roles
	    JS_AXIS_ROLES = 0;
	    switch(spin->cur_value)
	    {
	      case 1:   /* 2D */
		JS_AXIS_ROLES |= GCTL_JS_AXIS_ROLE_PITCH |
		    GCTL_JS_AXIS_ROLE_BANK;
		break;

	      case 2:   /* 2D with throttle */
		JS_AXIS_ROLES |= GCTL_JS_AXIS_ROLE_PITCH |
		    GCTL_JS_AXIS_ROLE_BANK | GCTL_JS_AXIS_ROLE_THROTTLE;
		break;

	      case 3:   /* 2D with hat */
		JS_AXIS_ROLES |= GCTL_JS_AXIS_ROLE_PITCH |
		    GCTL_JS_AXIS_ROLE_BANK | GCTL_JS_AXIS_ROLE_HAT;
		break;

	      case 4:   /* 2D with throttle & hat */
		JS_AXIS_ROLES |= GCTL_JS_AXIS_ROLE_PITCH |
		    GCTL_JS_AXIS_ROLE_BANK | GCTL_JS_AXIS_ROLE_THROTTLE |
		    GCTL_JS_AXIS_ROLE_HAT;
		break;

	      case 5:   /* 3D */
		JS_AXIS_ROLES |= GCTL_JS_AXIS_ROLE_PITCH |
		    GCTL_JS_AXIS_ROLE_BANK | GCTL_JS_AXIS_ROLE_HEADING;
		break;

	      case 6:   /* 3D with throttle */
		JS_AXIS_ROLES |= GCTL_JS_AXIS_ROLE_PITCH |
		    GCTL_JS_AXIS_ROLE_BANK | GCTL_JS_AXIS_ROLE_HEADING |
		    GCTL_JS_AXIS_ROLE_THROTTLE;
		break;

	      case 7:   /* 3D with hat */
		JS_AXIS_ROLES |= GCTL_JS_AXIS_ROLE_PITCH |
		    GCTL_JS_AXIS_ROLE_BANK | GCTL_JS_AXIS_ROLE_HEADING |
		    GCTL_JS_AXIS_ROLE_HAT;
		break;

	      case 8:   /* 3D with throttle & hat */
		JS_AXIS_ROLES |= GCTL_JS_AXIS_ROLE_PITCH |
		    GCTL_JS_AXIS_ROLE_BANK | GCTL_JS_AXIS_ROLE_HEADING |
		    GCTL_JS_AXIS_ROLE_THROTTLE | GCTL_JS_AXIS_ROLE_HAT;
		break;

	      case 9:   /* As throttle & rudder */
		JS_AXIS_ROLES |= GCTL_JS_AXIS_ROLE_AS_THROTTLE_AND_RUDDER;
		break;
	    }
#undef JS_AXIS_ROLES
	    /* Update options.controllers mask and reinitialize the
	     * game controller.
	     */
	    SARMenuOptionsJoystickReinit(core_ptr);
	    break;

	  case SAR_MENU_ID_OPT_JS_PRIORITY:
	    switch(spin->cur_value)
	    {
	      case 1:
		opt->js_priority = GCTL_JS_PRIORITY_FOREGROUND;
		break;
	      case 2:
		opt->js_priority = GCTL_JS_PRIORITY_PREEMPT;
		break;
	      default:
		opt->js_priority = GCTL_JS_PRIORITY_BACKGROUND;
		break;
	    }
	    /* Update options.controllers mask and reinitialize the
	     * game controller.
	     */
	    SARMenuOptionsJoystickReinit(core_ptr);
	    break;

	  case SAR_MENU_ID_OPT_JS0_BUTTON_ACTION:

	    if(True)
	    {
		int on;
		sar_menu_spin_struct *btn_num_spin = SARMenuOptionsGetSpinByID(
		    m, SAR_MENU_ID_OPT_JS0_BUTTON_NUMBER, &on
		);
		if(btn_num_spin != NULL)
		{
		    int value_num = SARGetJSButtonFromButtonRoleSpin(
			core_ptr,
			spin, SAR_MENU_ID_OPT_JS0_BUTTON_ACTION
		    ) + 1;
		    if(value_num >= btn_num_spin->total_values)
			value_num = 0;
		    SARMenuSpinSelectValueIndex(
			display, m, on, value_num, True
		    );
		}
	    }
	    break;

	  case SAR_MENU_ID_OPT_JS0_BUTTON_NUMBER:
	    if(True)
	    {
		sar_option_struct *opt = &core_ptr->option;
		sar_menu_spin_struct *btn_role_spin = SARMenuOptionsGetSpinByID(
		    m, SAR_MENU_ID_OPT_JS0_BUTTON_ACTION, NULL
		);
		if(btn_role_spin != NULL)
		{
		    int new_val = spin->cur_value - 1;

		    /* Set option for button mapping based on which value
		     * the action spin is on.
		     */
		    switch(btn_role_spin->cur_value)
		    {
		      case 0:	/* Rotate Modifier */
			opt->js0_btn_rotate = new_val;
			break;
		      case 1:	/* Air Brakes */
			opt->js0_btn_air_brakes = new_val;
			break;
		      case 2:	/* Wheel Brakes */
			opt->js0_btn_wheel_brakes = new_val;
			break;
		      case 3:	/* Zoom In */
			opt->js0_btn_zoom_in = new_val;
			break;
		      case 4:	/* Zoom out */
			opt->js0_btn_zoom_out = new_val;
			break;
		      case 5:	/* Hoist Up */
			opt->js0_btn_hoist_up = new_val;
			break;
		      case 6:	/* Hoist Down */
			opt->js0_btn_hoist_down = new_val;
			break;
/* When adding new button mappings, be sure to add support for them
 * in SARGetJSButtonFromButtonRoleSpin().
 *
 * Also remember to add support for EACH joystick!
 */
		    }
		}
	    }
	    /* Update joystick button mappings from global options
	     * values to the gctl structure on the core structure.
	     * No reinitialization will take place.
	     */
	    SARMenuOptionsJoystickButtonsRemap(core_ptr);
	    break;

	  case SAR_MENU_ID_OPT_JS1_BUTTON_ACTION:
	    if(True)
	    {
		int on;
		sar_menu_spin_struct *btn_num_spin = SARMenuOptionsGetSpinByID(
		    m, SAR_MENU_ID_OPT_JS1_BUTTON_NUMBER, &on
		);
		if(btn_num_spin != NULL)
		{
		    int value_num = SARGetJSButtonFromButtonRoleSpin(
			core_ptr,
			spin, SAR_MENU_ID_OPT_JS1_BUTTON_ACTION
		    ) + 1;
		    if(value_num >= btn_num_spin->total_values)
			value_num = 0;
		    SARMenuSpinSelectValueIndex(
			display, m, on, value_num, True
		    );
		}
	    }
	    break;

	  case SAR_MENU_ID_OPT_JS1_BUTTON_NUMBER:
	    if(True)
	    {
		sar_option_struct *opt = &core_ptr->option;
		sar_menu_spin_struct *btn_role_spin = SARMenuOptionsGetSpinByID(
		    m, SAR_MENU_ID_OPT_JS1_BUTTON_ACTION, NULL
		);
		if(btn_role_spin != NULL)
		{
		    int new_val = spin->cur_value - 1;

		    /* Set option for button mapping based on which value
		     * the action spin is on.
		     */
		    switch(btn_role_spin->cur_value)
		    {
		      case 0:   /* Rotate Modifier */
			opt->js1_btn_rotate = new_val;
			break;
		      case 1:   /* Air Brakes */
			opt->js1_btn_air_brakes = new_val;
			break;
		      case 2:   /* Wheel Brakes */
			opt->js1_btn_wheel_brakes = new_val;
			break;
		      case 3:   /* Zoom In */
			opt->js1_btn_zoom_in = new_val;
			break;
		      case 4:   /* Zoom out */
			opt->js1_btn_zoom_out = new_val;
			break;
		      case 5:   /* Hoist Up */
			opt->js1_btn_hoist_up = new_val;
			break;
		      case 6:   /* Hoist Down */
			opt->js1_btn_hoist_down = new_val;
			break;
/* When adding new button mappings, be sure to add support for them
 * in SARGetJSButtonFromButtonRoleSpin().
 * 
 * Also remember to add support for EACH joystick!
 */
		    }
		}
	    }
	    /* Update joystick button mappings from global options
	     * values to the gctl structure on the core structure.
	     * No reinitialization will take place.
	     */
	    SARMenuOptionsJoystickButtonsRemap(core_ptr);
	    break;

	  case SAR_MENU_ID_OPT_VISIBILITY_MAX:
	    /* These values should correspond to those used to
	     * set the values in SARBuildMenus().
	     */
	    opt->visibility_max = spin->cur_value;
	    break;

	  case SAR_MENU_ID_OPT_RESOLUTION:
	    if(display != NULL)
	    {
		int width, height;
		switch(spin->cur_value)
		{
                  case 6:
                    width = 1366;
                    height = 768;
                    break;
                  case 5:
                    width = 1280;
                    height = 768;
                    break;
		  case 4:
		    width = 1024;
		    height = 768;
		    break;
		  case 3:
		    width = 800;
		    height = 600;
		    break;
		  case 2:
		    width = 640;
		    height = 480;
		    break;
		  case 1:
		    width = 320;
		    height = 240;
		    break;
		  default:	/* 0 */
		    width = 100;
		    height = 70;
		    break;
		}
		SARResolution(display, width, height);
	    }
	    break;

/* Macro to turn sound on as needed by checking if the core structure's
 * recorder is initialized. If the recorder is NULL then an attempt to
 * connect to the sound server will be made using the argument
 * specified by sound_server_connect_arg.
 */
#define SND_ON_AS_NEEDED				\
{ if(core_ptr->recorder == NULL) {			\
 void *window;						\
 GWContextGet(display, GWContextCurrent(display),	\
  &window, NULL,					\
  NULL, NULL, NULL, NULL				\
 );							\
 GWSetInputBusy(display);				\
 core_ptr->recorder = SoundInit(			\
  core_ptr, SOUND_DEFAULT,				\
  sound_server_connect_arg,				\
  NULL,		/* Do not start sound server */	\
  window						\
 );							\
 if(core_ptr->recorder == NULL)	{			\
  char *buf = (char *)malloc(				\
   (512 + STRLEN(sound_server_connect_arg)) * sizeof(char) \
  );							\
  sprintf(						\
   buf,							\
"Unable to connect to sound server at:\n\n    %s\n\n\
Make sure that the address is correct,\n\
use the argument --recorder to specify\n\
an alternate address as needed.\n\n\
To start the sound server in most cases\n\
use `starty'", \
    sound_server_connect_arg \
  );							\
  GWOutputMessage(					\
   display,						\
   GWOutputMessageTypeError,				\
   "Sound initialization failed!",			\
   buf,							\
"Please check to make sure that your sound server\n\
is running and available at the specified address.\n\
Also make sure that you have sufficient permission\n\
to connect to it. To start in most cases run `starty'."	\
  );							\
  free(buf);						\
  SARMenuOptionsSoundRefresh(core_ptr);			\
 }							\
 else							\
 {							\
  /* Sound initialization successful */		\
  SoundChangeMode(					\
   core_ptr->recorder, core_ptr->audio_mode_name	\
  );							\
  SARMenuOptionsSoundRefresh(core_ptr);			\
 }							\
 GWSetInputReady(display);				\
} }

#define SND_OFF_AS_NEEDED				\
{ if(core_ptr->recorder != NULL) {			\
 GWSetInputBusy(display);				\
 SoundShutdown(core_ptr->recorder);			\
 core_ptr->recorder = NULL;				\
 SARMenuOptionsSoundRefresh(core_ptr);			\
 GWSetInputReady(display);				\
} }
	  case SAR_MENU_ID_OPT_SOUND_LEVEL:
	    switch(spin->cur_value)
	    {
	      case 1:   /* On, events only */
		opt->engine_sounds = False;
		opt->event_sounds = True;
		opt->voice_sounds = False;
		SND_ON_AS_NEEDED
		break;

	      case 2:   /* On, events and engine */
		opt->engine_sounds = True;
		opt->event_sounds = True;
		opt->voice_sounds = False;
		SND_ON_AS_NEEDED  
		break;
 
	      case 3:   /* On, events, engine, and voice (all on) */
		opt->engine_sounds = True;
		opt->event_sounds = True;
		opt->voice_sounds = True;
		SND_ON_AS_NEEDED
		break;
    
	      default:  /* All else assume off */
		opt->engine_sounds = False;
		opt->event_sounds = False;
		opt->voice_sounds = False;
		if(opt->music)
		    opt->music = False;
		SND_OFF_AS_NEEDED
		break;
	    }
	    break;

	  case SAR_MENU_ID_OPT_SOUND_PRIORITY:
	    switch(spin->cur_value)
	    {
	      case 1:
		opt->sound_priority = SND_PRIORITY_FOREGROUND;
		if(opt->engine_sounds || opt->event_sounds ||
		   opt->voice_sounds
		)
		{
		    SND_OFF_AS_NEEDED
		    SND_ON_AS_NEEDED
		}
		break;
	      case 2:
		opt->sound_priority = SND_PRIORITY_PREEMPT;
		if(opt->engine_sounds || opt->event_sounds ||
		   opt->voice_sounds
		)
		{
		    SND_OFF_AS_NEEDED
		    SND_ON_AS_NEEDED
		}
		break;
	      default:
		opt->sound_priority = SND_PRIORITY_BACKGROUND;
		if(opt->engine_sounds || opt->event_sounds ||
		   opt->voice_sounds
		)
		{
		    SND_OFF_AS_NEEDED
		    SND_ON_AS_NEEDED
		}
		break;
	    }
	    break;
#undef SND_ON_AS_NEEDED
#undef SND_OFF_AS_NEEDED

	}
}

/*
 *      Options menu slider callback.
 */
void SARMenuOptionsSliderCB(
	void *object, int id, void *client_data,
	float value
)
{
	sar_menu_struct *m;
	gw_display_struct *display;
	sar_core_struct *core_ptr = SAR_CORE(client_data);
	sar_menu_slider_struct *slider = SAR_MENU_SLIDER(object);
	sar_option_struct *opt;
	if((core_ptr == NULL) || (slider == NULL))
	    return;

	display = core_ptr->display;
	opt = &core_ptr->option;

	/* Get current menu */
	m = SARGetCurrentMenuPtr(core_ptr);
	if(m == NULL)
	    return;

	/* Handle by slider's ID code */
	switch(id)
	{
	  case SAR_MENU_ID_OPT_GRAPHICS_ACCELERATION:
	    opt->graphics_acceleration = (float)CLIP(value, 0.0, 1.0);
	    break;

	  case SAR_MENU_ID_OPT_VOLUME_MASTER:
	    SoundMixerSet(core_ptr->recorder, "volume", value, value);
	    break;

	  case SAR_MENU_ID_OPT_VOLUME_SOUND:
	    SoundMixerSet(core_ptr->recorder, "wav", value, value);
	    break;

	  case SAR_MENU_ID_OPT_VOLUME_MUSIC:
	    SoundMixerSet(core_ptr->recorder, "midi", value, value);
	    break;
	}
}
