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

void SARMenuOptionsJoystickReinit(sar_core_struct *core_ptr);
void SARMenuOptionsJoystickMappingReset(sar_core_struct *core_ptr, int gc_js_nums);
void SARMenuOptionsJoystickTestDrawCB(
    void *dpy,		/* Display */
    void *menu,		/* Menu */
    void *o,		/* This Object */
    int id,		/* ID Code */
    void *client_data,	/* Data */
    int x_min, int y_min, int x_max, int y_max
    );
void SARMenuOptionsJoystickMappingPrepare(sar_core_struct *core_ptr);
void SARMenuOptionsJoystickMappingExit(sar_core_struct *core_ptr);

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
void SARMenuOptionsJoystickReinit(sar_core_struct *core_ptr)
{
//fprintf(stderr, "%s:%d : Entering SARMenuOptionsJoystickReinit()\n", __FILE__, __LINE__);
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

    if(
	(opt->js0_axis_heading < 0) &&
	(opt->js0_axis_pitch < 0) &&
	(opt->js0_axis_bank < 0) &&
	(opt->js0_axis_throttle < 0) &&
	(opt->js0_pov_hat < 0) &&
	(opt->js0_axis_hat_x < 0) &&
	(opt->js0_axis_hat_y < 0) &&
	(opt->js0_axis_brake_left < 0) &&
	(opt->js0_axis_brake_right < 0) &&
	(opt->js0_btn_rotate < 0) &&
	(opt->js0_btn_air_brakes < 0) &&
	(opt->js0_btn_wheel_brakes < 0) &&
	(opt->js0_btn_zoom_in < 0) &&
	(opt->js0_btn_zoom_out < 0) &&
	(opt->js0_btn_hoist_up < 0) &&
	(opt->js0_btn_hoist_down < 0)
      )
	strcpy(opt->js0_sdl_guid_s, "");
    if(
	(opt->js1_axis_heading < 0) &&
	(opt->js1_axis_pitch < 0) &&
	(opt->js1_axis_bank < 0) &&
	(opt->js1_axis_throttle < 0) &&
	(opt->js1_pov_hat < 0) &&
	(opt->js1_axis_hat_x < 0) &&
	(opt->js1_axis_hat_y < 0) &&
	(opt->js1_axis_brake_left < 0) &&
	(opt->js1_axis_brake_right < 0) &&
	(opt->js1_btn_rotate < 0) &&
	(opt->js1_btn_air_brakes < 0) &&
	(opt->js1_btn_wheel_brakes < 0) &&
	(opt->js1_btn_zoom_in < 0) &&
	(opt->js1_btn_zoom_out < 0) &&
	(opt->js1_btn_hoist_up < 0) &&
	(opt->js1_btn_hoist_down < 0)
      )
	strcpy(opt->js1_sdl_guid_s, "");
    if(
	(opt->js2_axis_heading < 0) &&
	(opt->js2_axis_pitch < 0) &&
	(opt->js2_axis_bank < 0) &&
	(opt->js2_axis_throttle < 0) &&
	(opt->js2_pov_hat < 0) &&
	(opt->js2_axis_hat_x < 0) &&
	(opt->js2_axis_hat_y < 0) &&
	(opt->js2_axis_brake_left < 0) &&
	(opt->js2_axis_brake_right < 0) &&
	(opt->js2_btn_rotate < 0) &&
	(opt->js2_btn_air_brakes < 0) &&
	(opt->js2_btn_wheel_brakes < 0) &&
	(opt->js2_btn_zoom_in < 0) &&
	(opt->js2_btn_zoom_out < 0) &&
	(opt->js2_btn_hoist_up < 0) &&
	(opt->js2_btn_hoist_down < 0)
      )
	strcpy(opt->js2_sdl_guid_s, "");

    /* Check if joystick(s) need to be enabled */
    if(strlen(opt->js0_sdl_guid_s) != 0 ||
	strlen(opt->js1_sdl_guid_s) != 0 ||
	strlen(opt->js2_sdl_guid_s) != 0)
    {
	/* Add joystick to game controllers mask */
	opt->gctl_controllers |= GCTL_CONTROLLER_JOYSTICK;
    }

    /* Reinitialize game controller to the new type */
    GWSetInputBusy(core_ptr->display);
    SARInitGCTL(core_ptr);		/* Realize changes */
    GWSetInputReady(core_ptr->display);
//fprintf(stderr, "%s:%d : Exiting SARMenuOptionsJoystickReinit()\n", __FILE__, __LINE__);
}


/*
 * 	Reset joystick(s) mapping spins and axis inversion switches as needed:
 *	if gc_js_nums bit 'i' is set, then joystick 'i' mapping will be reset.
 *
 * 	This will only reset spins to 'None' and switches to 'Off' position.
 *	opt->js*_* values modifications are done by SARMenuOptionsSpinCB(...)
 *	and SARMenuOptionsSwitchCB(...) call backs.
 */
void SARMenuOptionsJoystickMappingReset(sar_core_struct *core_ptr, int gc_js_nums)
{
//fprintf(stderr, "%s:%d : Entering SARMenuOptionsJoystickMappingReset()\n", __FILE__, __LINE__);
    //gctl_struct *gc = core_ptr->gctl;
    //sar_option_struct *opt;
    //gctl_js_struct *gc_js;
    gw_display_struct *display;
    sar_menu_struct *menu;
    sar_menu_spin_struct *spin;
    sar_menu_switch_struct *swtch;
    int i, jsLastSpin, jsFirstSpin, jsnum, switch_id, switch_num;
    //void *menu_object;
    //sar_menu_label_struct *label;

    if(gc_js_nums <= 0)
	return;

    if(core_ptr == NULL)
	return;
    //opt = &core_ptr->option;

    menu = SARMatchMenuByNamePtr(
	    core_ptr,
	    SAR_MENU_NAME_OPTIONS_CONTROLLER_JS_MAPPING
	    );
    if(menu == NULL)
	return;

    display = core_ptr->display;
    if(display == NULL)
	return;

    for(i = 0; i < menu->total_objects; i++){
	if(SAR_MENU_IS_SPIN(menu->object[i])){
	    spin = menu->object[i];
	    jsLastSpin = (int)((spin->total_values - 1)/MAX_JOYSTICKS);
	    jsFirstSpin = jsLastSpin - (int)((spin->total_values - 1)/MAX_JOYSTICKS) + 1;
	    jsnum = -1;

	    if(spin->cur_value >= jsFirstSpin && spin->cur_value <= jsLastSpin)
		jsnum = 0;
	    else if(spin->cur_value > jsLastSpin && spin->cur_value <= jsLastSpin * 2)
		jsnum = 1;
	    else if(spin->cur_value > jsLastSpin * 2 && spin->cur_value <= jsLastSpin * 3)
		jsnum = 2;

	    /* This spin to reset? */
	    if( (jsnum == 0 && (gc_js_nums & 1<<0)) ||
		(jsnum == 1 && (gc_js_nums & 1<<1)) ||
		(jsnum == 2 && (gc_js_nums & 1<<2))
	    ){
		/* Reset spin */

		SARMenuSpinSelectValueIndex(
		    display, menu, i,
		    0,		// set spin value index to 0
		    True
		);

		/* Reset inversion switch */

		switch(spin->id){
		    case SAR_MENU_ID_OPT_HEADING_AXIS:
			switch_id = SAR_MENU_ID_OPT_HEADING_AXIS_INV;
			break;
		    case SAR_MENU_ID_OPT_PITCH_AXIS:
			switch_id = SAR_MENU_ID_OPT_PITCH_AXIS_INV;
			break;
		    case SAR_MENU_ID_OPT_BANK_AXIS:
			switch_id = SAR_MENU_ID_OPT_BANK_AXIS_INV;
			break;
		    case SAR_MENU_ID_OPT_THROTTLE_AXIS:
			switch_id = SAR_MENU_ID_OPT_THROTTLE_AXIS_INV;
			break;
		    case SAR_MENU_ID_OPT_POV_HAT_X:
			switch_id = SAR_MENU_ID_OPT_POV_HAT_X_INV;
			break;
		    case SAR_MENU_ID_OPT_POV_HAT_Y:
			switch_id = SAR_MENU_ID_OPT_POV_HAT_Y_INV;
			break;
		    case SAR_MENU_ID_OPT_POV_HAT:
			switch_id = SAR_MENU_ID_OPT_POV_HAT_INV;
			break;
		    case SAR_MENU_ID_OPT_BRAKE_LEFT_AXIS:
			switch_id = SAR_MENU_ID_OPT_BRAKE_LEFT_AXIS_INV;
			break;
		    case SAR_MENU_ID_OPT_BRAKE_RIGHT_AXIS:
			switch_id = SAR_MENU_ID_OPT_BRAKE_RIGHT_AXIS_INV;
			break;
		    default:
			switch_id = -1;
			break;
		}

		if(switch_id >= 0){
		    swtch = SARMenuGetObjectByID(menu, switch_id, &switch_num);
		    SARMenuSwitchSetValue(
			display, menu, switch_num,
			False,	// set switch value to False
			True
		    );
		}
	    }
	}
    }
//fprintf(stderr, "%s:%d : Exiting SARMenuOptionsJoystickMappingReset()\n", __FILE__, __LINE__);
}



/*
 * 	Allocates gc->js_mapping_axes_values before entering joystick mapping menu.
 */
void SARMenuOptionsJoystickMappingPrepare(sar_core_struct *core_ptr)
{
//fprintf(stderr, "%s:%d : Entering SARMenuOptionsJoystickMappingPrepare()\n", __FILE__, __LINE__);
    gctl_struct *gc = core_ptr->gctl;
    gctl_js_mapping_menu_data_struct *axis;
    int  i, j;

    gc->js_mapping_axes_values = GCTL_JS_MAPPING_MENU_DATA(
			malloc(MAX_JOYSTICKS * MAX_JOYSTICK_AXES *
			sizeof(gctl_js_mapping_menu_data_struct))
    );

    for(i = 0; i < MAX_JOYSTICKS; i++)
    {
	for(j = 0; j < MAX_JOYSTICK_AXES; j++)
	{
	    axis = &gc->js_mapping_axes_values[i * MAX_JOYSTICK_AXES + j];
	    axis->min = 0;
	    axis->max = 0;
	}
    }

//fprintf(stderr, "%s:%d : Exiting SARMenuOptionsJoystickMappingPrepare()\n", __FILE__, __LINE__);
}

/*
 * 	Frees gc->js_mapping_axes_values while exiting joystick mapping menu.
 */
void SARMenuOptionsJoystickMappingExit(sar_core_struct *core_ptr)
{
//fprintf(stderr, "%s:%d : Entering SARMenuOptionsJoystickMappingExit()\n", __FILE__, __LINE__);
    gctl_struct *gc = core_ptr->gctl;

    free(gc->js_mapping_axes_values);

//fprintf(stderr, "%s:%d : Exiting SARMenuOptionsJoystickMappingExit()\n", __FILE__, __LINE__);
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
    SDL_Event event;
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

    /* Button pressed icon bitmap 22x22 (circle) */
/*  const u_int8_t btn_pressed_bm[] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfc, 0x00, 0x03, 0x03, 0x00, 0x04,
	0x00, 0x80, 0x08, 0x00, 0x40, 0x08, 0x00, 0x40, 0x10, 0x00, 0x20, 0x10, 0x00, 0x20, 0x10, 0x00,
	0x20, 0x10, 0x00, 0x20, 0x10, 0x00, 0x20, 0x10, 0x00, 0x20, 0x08, 0x00, 0x40, 0x08, 0x00, 0x40,
	0x04, 0x00, 0x80, 0x03, 0x03, 0x00, 0x00, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00
    };
    */

    /* Button pressed icon bitmap 22x22 (disc) */
    const u_int8_t btn_pressed_bm[] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfc, 0x00, 0x03, 0xff, 0x00, 0x07,
	0xff, 0x80, 0x0f, 0xff, 0xc0, 0x0f, 0xff, 0xc0, 0x1f, 0xff, 0xe0, 0x1f, 0xff, 0xe0, 0x1f, 0xff,
	0xe0, 0x1f, 0xff, 0xe0, 0x1f, 0xff, 0xe0, 0x1f, 0xff, 0xe0, 0x0f, 0xff, 0xc0, 0x0f, 0xff, 0xc0,
	0x07, 0xff, 0x80, 0x03, 0xff, 0x00, 0x00, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00
    };


    /* Button released icon bitmap 22x22 */
    const u_int8_t btn_released_bm[] = {
	0x00, 0x00, 0x00, 0x01, 0xfe, 0x00, 0x06, 0x01, 0x80, 0x08, 0x00, 0x40, 0x10, 0x00, 0x20, 0x20,
	0x00, 0x10, 0x20, 0x00, 0x10, 0x40, 0x00, 0x08, 0x40, 0x00, 0x08, 0x40, 0x00, 0x08, 0x40, 0x00,
	0x08, 0x40, 0x00, 0x08, 0x40, 0x00, 0x08, 0x40, 0x00, 0x08, 0x40, 0x00, 0x08, 0x20, 0x00, 0x10,
	0x20, 0x00, 0x10, 0x10, 0x00, 0x20, 0x08, 0x00, 0x40, 0x06, 0x01, 0x80, 0x01, 0xfe, 0x00, 0x00,
	0x00, 0x00
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

#define SET_VERTEX(x,y)	glVertex2i(             \
	(GLint)((x) + x_min),			\
	(GLint)(height - ((y) + y_min))         \
	)
#define DRAW_STRING(x,y,s) GWDrawString(        \
	display,				\
	(int)((x) + x_min),			\
	(int)((y) + y_min),			\
	(s)					\
	)
#define COLOR_YELLOW glColor3f(1.0f, 1.0f, 0.0f);
#define COLOR_GREEN glColor3f(0.0f, 1.0f, 0.0f);
#define COLOR_LIGHT_GREEN glColor3f(0.0f, 0.60f, 0.0f);
#define COLOR_GRAY glColor3f(0.50f, 0.50f, 0.50f);
#define COLOR_RED glColor3f(1.0f, 0.0f, 0.0f);
#define COLOR_BLACK glColor3f(0.0f, 0.0f, 0.0f);

    if(gc == NULL)
	return;

    int x, y;
    int x_border = 30, y_border = 30;
    int x_offset, y_offset, i;
    int wc, hc;
    const char* s;
    gctl_js_struct *gc_joystick;
    Boolean gcjs0_set = False, gcjs1_set = False, gcjs2_set = False;


    /* Open sdl joysticks and map them to gc joysticks */
    GctlJsOpenAndMapp(gc, NULL);
    /* Update sdl joysticks axes and buttons values */
    SDL_JoystickUpdate();

    gc_joystick = &gc->joystick[0];
    if(strlen(gc_joystick->sdl_guid_s) > 0)
	gcjs0_set = True;

    gc_joystick = &gc->joystick[1];
    if(strlen(gc_joystick->sdl_guid_s) > 0)
	gcjs1_set = True;

    gc_joystick = &gc->joystick[2];
    if(strlen(gc_joystick->sdl_guid_s) > 0)
	gcjs2_set = True;


    /* Calculate bounds for hat grid box */
    x_offset = x_border;
    y_offset = y_border;
    wc = 50;
    hc = 50;

    /* Draw hat grid box */
    if((gcjs0_set && gc->joystick[0].pov_hat > -1)
	|| (gcjs1_set && gc->joystick[1].pov_hat > -1)
	|| (gcjs2_set && gc->joystick[2].pov_hat > -1)
	|| (gcjs0_set && gc->joystick[0].axis_hat_x > -1)
	|| (gcjs0_set && gc->joystick[0].axis_hat_y > -1)
	|| (gcjs1_set && gc->joystick[1].axis_hat_x > -1)
	|| (gcjs1_set && gc->joystick[1].axis_hat_y > -1)
	|| (gcjs2_set && gc->joystick[2].axis_hat_x > -1)
	|| (gcjs2_set && gc->joystick[2].axis_hat_y > -1)
    )
	COLOR_GREEN
    else
	COLOR_GRAY
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
    if((gcjs0_set && gc->joystick[0].pov_hat > -1)
	|| (gcjs1_set && gc->joystick[1].pov_hat > -1)
	|| (gcjs2_set && gc->joystick[2].pov_hat > -1)
	|| (gcjs0_set && gc->joystick[0].axis_hat_x > -1)
	|| (gcjs0_set && gc->joystick[0].axis_hat_y > -1)
	|| (gcjs1_set && gc->joystick[1].axis_hat_x > -1)
	|| (gcjs1_set && gc->joystick[1].axis_hat_y > -1)
	|| (gcjs2_set && gc->joystick[2].axis_hat_x > -1)
	|| (gcjs2_set && gc->joystick[2].axis_hat_y > -1)
    )
	COLOR_LIGHT_GREEN
    else
	COLOR_GRAY
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
    if((gcjs0_set && gc->joystick[0].pov_hat > -1)
	|| (gcjs1_set && gc->joystick[1].pov_hat > -1)
	|| (gcjs2_set && gc->joystick[2].pov_hat > -1)
	|| (gcjs0_set && gc->joystick[0].axis_hat_x > -1)
	|| (gcjs0_set && gc->joystick[0].axis_hat_y > -1)
	|| (gcjs1_set && gc->joystick[1].axis_hat_x > -1)
	|| (gcjs1_set && gc->joystick[1].axis_hat_y > -1)
	|| (gcjs2_set && gc->joystick[2].axis_hat_x > -1)
	|| (gcjs2_set && gc->joystick[2].axis_hat_y > -1)
    )
	COLOR_YELLOW
    else
	COLOR_GRAY
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
     * note that x_offset was already set when drawing the
     * hat.
     */

    /* Joysticks buttons roles.
     * If role is not assigned, role name color will be gray
     * If role is assigned and button released, role name color will be light green
     * If role is assigned and button pressed, role name color will be yellow
    */
    x = wc + fw;
    y_offset = y_border;
    y = 0;

    s = "Zoom in";
    if((gcjs0_set && gc->joystick[0].button_zoom_in > -1)
	|| (gcjs1_set && gc->joystick[1].button_zoom_in > -1)
	|| (gcjs2_set && gc->joystick[2].button_zoom_in > -1)
    ){
	if(gc->zoom_in_state)
	    COLOR_YELLOW
	else
	    COLOR_LIGHT_GREEN
    }
    else
	COLOR_GRAY
    DRAW_STRING(x + x_offset, y + y_offset, s);
    y += fh;

    s = "Zoom out";
    if((gcjs0_set && gc->joystick[0].button_zoom_out > -1)
	|| (gcjs1_set && gc->joystick[1].button_zoom_out > -1)
	|| (gcjs2_set && gc->joystick[2].button_zoom_out > -1)
    ){
	if(gc->zoom_out_state)
	    COLOR_YELLOW
	else
	    COLOR_LIGHT_GREEN
    }
    else
	COLOR_GRAY
    DRAW_STRING(x + x_offset, y + y_offset, s);
    y += fh;

    s = "Rotate  ";
    if((gcjs0_set && gc->joystick[0].button_rotate > -1)
	|| (gcjs1_set && gc->joystick[1].button_rotate > -1)
	|| (gcjs2_set && gc->joystick[2].button_rotate > -1)
    ){
	if(gc->ctrl_state)
	    COLOR_YELLOW
	else
	    COLOR_LIGHT_GREEN
    }
    else
	COLOR_GRAY
    DRAW_STRING(x + x_offset, y + y_offset, s);
    x += STRLEN(s) * fw + 2 * fw;
    y -= 2*fh;

    s = "Hoist up";
    if((gcjs0_set && gc->joystick[0].button_hoist_up > -1)
	|| (gcjs1_set && gc->joystick[1].button_hoist_up > -1)
	|| (gcjs2_set && gc->joystick[2].button_hoist_up > -1)
    ){
	if(gc->hoist_up_state)
	    COLOR_YELLOW
	else
	    COLOR_LIGHT_GREEN
    }
    else
	COLOR_GRAY
    DRAW_STRING(x + x_offset, y + y_offset, s);
    y += fh;

    s = "Hoist down";
    if((gcjs0_set && gc->joystick[0].button_hoist_down > -1)
	|| (gcjs1_set && gc->joystick[1].button_hoist_down > -1)
	|| (gcjs2_set && gc->joystick[2].button_hoist_down > -1)
    ){
	if(gc->hoist_down_state)
	    COLOR_YELLOW
	else
	    COLOR_LIGHT_GREEN
    }
    else
	COLOR_GRAY
    DRAW_STRING(x + x_offset, y + y_offset, s);
    x += STRLEN(s) * fw + 2 * fw;
    y -= fh;

    if((gcjs0_set && gc->joystick[0].button_air_brakes > -1)
	|| (gcjs1_set && gc->joystick[1].button_air_brakes > -1)
	|| (gcjs2_set && gc->joystick[2].button_air_brakes > -1)
    ){
	if(gc->air_brakes_state){
	    s = "Air brakes [ON]";
	    COLOR_YELLOW
	}
	else{
	    s = "Air brakes [OFF]";
	    COLOR_LIGHT_GREEN
	}
    }
    else{
	s = "Air brakes";
	COLOR_GRAY
    }
    DRAW_STRING(x + x_offset, y + y_offset, s);
    y += fh;

    /* Wheel Brakes, as button or axes */

    if((gcjs0_set && gc->joystick[0].button_wheel_brakes > -1)
	|| (gcjs1_set && gc->joystick[1].button_wheel_brakes > -1)
	|| (gcjs2_set && gc->joystick[2].button_wheel_brakes > -1)
    ){
	/* Note 1: shift key state detection works only if window has focus.
	 * Note 2: wheel brakes to parking brake state transition don't work
	 *        very good on joystick test page but works fine while in game.
	 */

	/* As we are in a menu, shift key state is not handled by Game
	 * Controller, thus we read it directly from display.
	 */
	if(display->shift_key_state && gc->wheel_brakes_state == 1)
	    gc->wheel_brakes_state = 2;

	switch(gc->wheel_brakes_state){
	    case 0:
		s = "Wheel brakes [OFF]";
		COLOR_LIGHT_GREEN
		break;
	    case 1:
		s = "Wheel brakes [ON]";
		COLOR_YELLOW
		break;
	    case 2:
		s = "Parking brake [ON]";
		COLOR_YELLOW
		break;
	    default:
		break;
	}
    }
    else if((gcjs0_set && gc->joystick[0].axis_brake_left > -1)
	|| (gcjs0_set && gc->joystick[0].axis_brake_right > -1)
	|| (gcjs1_set && gc->joystick[1].axis_brake_left > -1)
	|| (gcjs1_set && gc->joystick[1].axis_brake_right > -1)
	|| (gcjs2_set && gc->joystick[2].axis_brake_left > -1)
	|| (gcjs2_set && gc->joystick[2].axis_brake_right > -1)
    ){
	/* Note : left and right wheel brake values are handled to an unique
	 * brake coefficient (see GCtlUpdate()).
	 */

	int wheel_brakes_coeff_percents = gc->wheel_brakes_coeff * 100;

	/* As we are in a menu, shift key state is not handled by Game
	 * Controller, thus we read it directly from display.
	 */
	if(display->shift_key_state && wheel_brakes_coeff_percents >= 100)
	    gc->wheel_brakes_state = 2;

	switch(gc->wheel_brakes_state){
	    case 0:
		s = "Wheel brakes [0%]";
		COLOR_LIGHT_GREEN
		break;
	    case 1:
		char ns[48+1];
		snprintf(ns, sizeof(ns), "Wheel brakes [%3d%%]", wheel_brakes_coeff_percents);
		s = ns;
		if(wheel_brakes_coeff_percents == 0)
		    COLOR_LIGHT_GREEN
		else
		    COLOR_YELLOW
		break;
	    case 2:
		s = "Parking brake [ON]";
		COLOR_YELLOW
		break;
	}
    }
    else
    {
	s = "Wheel brakes";
	COLOR_GRAY
    }
    DRAW_STRING(x + x_offset, y + y_offset, s);
    y -= fh;

    x_offset = x_border;
    y_offset += wc + 15;

    /* Calculate bounds for pitch and bank axis grid box */

/* Max MAX_JOYSTICK_AXES axes per joystick and 6 characters per axis value */
#define MAX_AXIS_TABLE_PIXELS_WIDTH	MAX_JOYSTICK_AXES * 6 * fw
    if(w - MAX_AXIS_TABLE_PIXELS_WIDTH > h - hc)
    {
	hc = (int)(h - MAX((2 * y_border) + 20, 0) - wc - x_border/2);
	wc = hc;
    }
    else
    {
	wc = (int)(w - MAX((2 * x_border) + 20, 0) - MAX_AXIS_TABLE_PIXELS_WIDTH - x_border/2);
	hc = wc;
    }
#undef MAX_AXIS_TABLE_PIXELS_WIDTH

    if((gcjs0_set && gc->joystick[0].axis_pitch > -1)
	|| (gcjs1_set && gc->joystick[1].axis_pitch > -1)
	|| (gcjs2_set && gc->joystick[2].axis_pitch > -1)
	|| (gcjs0_set && gc->joystick[0].axis_bank > -1)
	|| (gcjs1_set && gc->joystick[1].axis_bank > -1)
	|| (gcjs2_set && gc->joystick[2].axis_bank > -1)
    )
	COLOR_GREEN
    else
	COLOR_GRAY

    /* Pitch and bank grid box */
    glBegin(GL_LINE_LOOP);
    {
	SET_VERTEX(x_offset, y_offset);
	SET_VERTEX(x_offset, hc + y_offset);
	SET_VERTEX(wc + x_offset, hc + y_offset);
	SET_VERTEX(wc + x_offset, y_offset);
    }
    glEnd();

    /* Rudder (heading) grid box */
    if((gcjs0_set && gc->joystick[0].axis_heading > -1)
	|| (gcjs1_set && gc->joystick[1].axis_heading > -1)
	|| (gcjs2_set && gc->joystick[2].axis_heading > -1)
	|| (gcjs0_set && gc->joystick[0].button_rotate > -1 && gc->joystick[0].axis_bank > -1)
	|| (gcjs1_set && gc->joystick[1].button_rotate > -1 && gc->joystick[1].axis_bank > -1)
	|| (gcjs2_set && gc->joystick[2].button_rotate > -1 && gc->joystick[2].axis_bank > -1)
    )
	COLOR_GREEN
    else
	COLOR_GRAY
    glBegin(GL_LINE_STRIP);
    {
	SET_VERTEX(x_offset, hc + y_offset);
	SET_VERTEX(x_offset, hc + 15 + y_offset);
	SET_VERTEX(wc + x_offset, hc + 15 + y_offset);
	SET_VERTEX(wc + x_offset, hc + y_offset);
    }
    glEnd();

    /* Throttle grid box */
    if((gcjs0_set && gc->joystick[0].axis_throttle > -1)
	|| (gcjs1_set && gc->joystick[1].axis_throttle > -1)
	|| (gcjs2_set && gc->joystick[2].axis_throttle > -1)
    )
	COLOR_GREEN
    else
	COLOR_GRAY
    glBegin(GL_LINE_STRIP);
    {
	SET_VERTEX(wc + x_offset, y_offset);
	SET_VERTEX(wc + 20 + x_offset, y_offset);
	SET_VERTEX(wc + 20 + x_offset, hc + 15 + y_offset);
	SET_VERTEX(wc + x_offset, hc + 15 + y_offset);
    }
    glEnd();

    /* Pitch center lines */
    x = (int)(wc / 2);
    y = (int)(hc / 2);
    /* Pitch (horizontal) 25% lines */
    if((gcjs0_set && gc->joystick[0].axis_pitch > -1)
	|| (gcjs1_set && gc->joystick[1].axis_pitch > -1)
	|| (gcjs2_set && gc->joystick[2].axis_pitch > -1)
    )
	COLOR_LIGHT_GREEN
    else
	COLOR_GRAY
    glBegin(GL_LINES);
    {
	SET_VERTEX(x_offset, y + y_offset);
	SET_VERTEX(wc + x_offset, y + y_offset);
    }
    glEnd();
    /* Bank center lines */
    x = (int)(wc / 2);
    y = (int)(hc / 2);
    /* Pitch (horizontal) 25% lines */
    if((gcjs0_set && gc->joystick[0].axis_bank > -1)
	|| (gcjs1_set && gc->joystick[1].axis_bank > -1)
	|| (gcjs2_set && gc->joystick[2].axis_bank > -1)
    )
	COLOR_LIGHT_GREEN
    else
	COLOR_GRAY
    glBegin(GL_LINES);
    {
	SET_VERTEX(x + x_offset, y_offset);
	SET_VERTEX(x + x_offset, hc + 0 + y_offset);
    }
    glEnd();
    /* Rudder (heading) center line */
    if((gcjs0_set && gc->joystick[0].axis_heading > -1)
	|| (gcjs1_set && gc->joystick[1].axis_heading > -1)
	|| (gcjs2_set && gc->joystick[2].axis_heading > -1)
	|| (gcjs0_set && gc->joystick[0].button_rotate > -1)
	|| (gcjs1_set && gc->joystick[1].button_rotate > -1)
	|| (gcjs2_set && gc->joystick[2].button_rotate > -1)
    )
	COLOR_GREEN
    else
	COLOR_GRAY
    glBegin(GL_LINES);
    {
	SET_VERTEX(x + x_offset, hc + 0 + y_offset);
	SET_VERTEX(x + x_offset, hc + 15 + y_offset);
    }
    glEnd();
    /* Pitch (horizontal) 25% lines */
    if((gcjs0_set && gc->joystick[0].axis_pitch > -1)
	|| (gcjs1_set && gc->joystick[1].axis_pitch > -1)
	|| (gcjs2_set && gc->joystick[2].axis_pitch > -1)
    )
	COLOR_LIGHT_GREEN
    else
	COLOR_GRAY
    glBegin(GL_LINES);
    {
	y = (int)(hc * 0.25);
	SET_VERTEX(x_offset, y + y_offset);
	SET_VERTEX(wc + x_offset, y + y_offset);
	y = (int)(hc * 0.75);
	SET_VERTEX(x_offset, y + y_offset);
	SET_VERTEX(wc + x_offset, y + y_offset);
    }
    glEnd();
    /* Bank (vertical) 25% lines */
    if((gcjs0_set && gc->joystick[0].axis_bank > -1)
	|| (gcjs1_set && gc->joystick[1].axis_bank > -1)
	|| (gcjs2_set && gc->joystick[2].axis_bank > -1)
    )
	COLOR_LIGHT_GREEN
    else
	COLOR_GRAY
    glBegin(GL_LINES);
    {
	x = (int)(wc * 0.25);
	SET_VERTEX(x + x_offset, y_offset);
	SET_VERTEX(x + x_offset, hc + 0 + y_offset);
	x = (int)(wc * 0.75);
	SET_VERTEX(x + x_offset, y_offset);
	SET_VERTEX(x + x_offset, hc + 0 + y_offset);
    }
    glEnd();


    /* Draw pitch and bank crosshair */
    x = (int)(CLIP(gc->bank, -1, 1) * wc / 2) + (wc / 2);
    y = (int)(-CLIP(gc->pitch, -1, 1) * hc / 2) + (hc / 2);
    if((gcjs0_set && gc->joystick[0].axis_pitch > -1)
	|| (gcjs1_set && gc->joystick[1].axis_pitch > -1)
	|| (gcjs2_set && gc->joystick[2].axis_pitch > -1)
	|| (gcjs0_set && gc->joystick[0].axis_bank > -1)
	|| (gcjs1_set && gc->joystick[1].axis_bank > -1)
	|| (gcjs2_set && gc->joystick[2].axis_bank > -1)
    )
	COLOR_YELLOW
    else
	COLOR_GRAY
    glRasterPos2i(
	x - 7 + x_offset + x_min,
	height - (y + 7 + y_offset + y_min)
	);
    glBitmap(15, 15, 0.0f, 0.0f, 15.0f, 0.0f, crosshairs_bm);


    /* Draw throttle axis */
    hc += 15;
    if((gcjs0_set && gc->joystick[0].axis_throttle > -1)
	|| (gcjs1_set && gc->joystick[1].axis_throttle > -1)
	|| (gcjs2_set && gc->joystick[2].axis_throttle > -1)
    ){
	/* Calculate bounds for throttle axis */
	x_offset = x_border + wc;
	y = (int)(CLIP(1.0 - gc->throttle, 0, 1) * hc);
	COLOR_YELLOW
	glBegin(GL_QUADS);
	{
	    SET_VERTEX(1 + x_offset, y + y_offset);
	    SET_VERTEX(1 + x_offset, hc + y_offset);
	    SET_VERTEX(20 + x_offset, hc + y_offset);
	    SET_VERTEX(20 + x_offset, y + y_offset);
	}
	glEnd();
    }

    hc += 50;

    /* Calculate bounds for rudder axis */
    x_offset = x_border;
    y_offset = y_border + hc;
    x = (int)(CLIP(gc->heading, -1, 1) * (wc / 2)) + (wc / 2);

    /* Draw rudder (heading) axis */
    if((gcjs0_set && gc->joystick[0].axis_heading > -1)
	|| (gcjs1_set && gc->joystick[1].axis_heading > -1)
	|| (gcjs2_set && gc->joystick[2].axis_heading > -1)
	|| (gcjs0_set && gc->joystick[0].button_rotate > -1 && gc->joystick[0].axis_bank > -1)
	|| (gcjs1_set && gc->joystick[1].button_rotate > -1 && gc->joystick[1].axis_bank > -1)
	|| (gcjs2_set && gc->joystick[2].button_rotate > -1 && gc->joystick[1].axis_bank > -1)
    )
	COLOR_YELLOW
    else
	COLOR_GRAY
    glRasterPos2i(
	x - 7 + x_offset + x_min,
	height - (15 + y_offset + y_min)
	);
    glBitmap(15, 15, 0.0f, 0.0f, 15.0f, 0.0f, rudder_bm);

    x_offset = x_border + wc + fw + x_border;
    y_offset = y_border + 50 + fh;
    x = 0;
    y = 0;

    SDL_Joystick *sdlstick;
    int joystick_axes = 0;

    /* Joystick number, name, axes, buttons, and hats. */

    /* Iterate through all gc joysticks */
    for(i = 0; i < gc->total_joysticks; i++)
    {
	sdlstick = gc->sdljoystick[i];

	if(sdlstick != NULL)
	{
	    char ns[48+1];
	    x = 0;

	    if(gc->total_joysticks > i)
		gc_joystick = &gc->joystick[i];
	    else
		gc_joystick = NULL;

	    /* Joystick number and name */

	    /* This joystick mapped? */
	    if( (i == 0 && strlen(opt->js0_sdl_guid_s) > 0) ||
		(i == 1 && strlen(opt->js1_sdl_guid_s) > 0) ||
		(i == 2 && strlen(opt->js2_sdl_guid_s) > 0)
	    ){
		COLOR_GREEN
		s = "Joystick #";
		DRAW_STRING(x + x_offset, y + y_offset, s);
		x += STRLEN(s) * fw;
		COLOR_YELLOW
		snprintf(ns, sizeof(ns), "%1d", i + 1);
		s = ns;
		DRAW_STRING(x + x_offset, y + y_offset, s);
		x += STRLEN(s) * fw;
	    }
	    else{
		COLOR_RED
		s = "[NOT MAPPED]";
		DRAW_STRING(x + x_offset, y + y_offset, s);
		x += STRLEN(s) * fw;
	    }
	    COLOR_GREEN
	    s = ": ";
	    DRAW_STRING(x + x_offset, y + y_offset, s);
	    x += STRLEN(s) * fw;
	    COLOR_YELLOW
	    snprintf(ns, sizeof(ns), "%s", SDL_JoystickName(sdlstick));
	    s = ns;
	    DRAW_STRING(x + x_offset, y + y_offset, s);

	    y += fh;

	    /* Axes number */
	    COLOR_GREEN
	    x = 0;
	    s = "Axes: ";
	    DRAW_STRING(x + x_offset, y + y_offset, s);
	    x += STRLEN(s) * fw;
	    COLOR_YELLOW
	    snprintf(ns, sizeof(ns), "%i", SDL_JoystickNumAxes(sdlstick));
	    s = ns;
	    DRAW_STRING(x + x_offset, y + y_offset, s);
	    x += STRLEN(s) * fw;

	    /* Buttons number*/
	    COLOR_GREEN
	    s = "   Buttons: ";
	    DRAW_STRING(x + x_offset, y + y_offset, s);
	    x += STRLEN(s) * fw;
	    COLOR_YELLOW
	    snprintf(ns, sizeof(ns), "%i", SDL_JoystickNumButtons(sdlstick));
	    s = ns;
	    DRAW_STRING(x + x_offset, y + y_offset, s);
	    x += STRLEN(s) * fw;

	    /* Hats number */
	    COLOR_GREEN
	    s = "   POV hats: ";
	    DRAW_STRING(x + x_offset, y + y_offset, s);
	    x += STRLEN(s) * fw;
	    COLOR_YELLOW
	    snprintf(ns, sizeof(ns), "%i", SDL_JoystickNumHats(sdlstick));
	    s = ns;
	    DRAW_STRING(x + x_offset, y + y_offset, s);
	    x += STRLEN(s) * fw;

	    /* Mapped hat number */
	    if(gc_joystick != NULL && gc_joystick->pov_hat > -1){
		for(int k = 0; k < SDL_JoystickNumHats(sdlstick); k++)
		{
		    if(k >= MAX_JOYSTICK_HATS)
			break;

		    if(SDL_JoystickGetHat(sdlstick, k) != SDL_HAT_CENTERED){
			COLOR_YELLOW
			snprintf(ns, sizeof(ns), " [%i]", gc_joystick->pov_hat + 1);
		    }
		    else{
			COLOR_GREEN
			snprintf(ns, sizeof(ns), " (%i)", gc_joystick->pov_hat + 1);
		    }
		}
		s = ns;
		DRAW_STRING(x + x_offset, y + y_offset, s);
		x += STRLEN(s) * fw;
	    }

	    y += 1.2*fh;

	    /* Draw axes table */
#define CHARS_PER_AXIS 6
	    joystick_axes = SDL_JoystickNumAxes(sdlstick);
	    if(joystick_axes > 0 && joystick_axes <= MAX_JOYSTICK_AXES){
		x = 0;
		COLOR_GREEN
		GLint table_length = CHARS_PER_AXIS * joystick_axes * fw + 1;
		/* Horizontal lines */
		glBegin(GL_LINES);
		{
		    SET_VERTEX(x_offset, y + y_offset + 1);
		    SET_VERTEX(table_length + x_offset, y + 0 * fh + y_offset + 1);

		    SET_VERTEX(x_offset, y + 1 * fh + y_offset + 1);
		    SET_VERTEX(table_length + x_offset, y + 1 * fh + y_offset + 1);

		    SET_VERTEX(x_offset, y + 2 * fh + y_offset + 1);
		    SET_VERTEX(table_length + x_offset, y + 2 * fh + y_offset + 1);
		}
		glEnd();
		/* First (left) vertical line */
		glBegin(GL_LINES);
		{
		    SET_VERTEX(x_offset, y + y_offset + 1);
		    SET_VERTEX(x_offset, y + 2 * fh + y_offset + 1);
		}
		glEnd();
		/* Nexts vertical lines */
		for (int k = 1; k <= SDL_JoystickNumAxes(sdlstick); k++){
		    glBegin(GL_LINES);
		    {
			SET_VERTEX(CHARS_PER_AXIS * k * fw + x_offset, y + y_offset + 1);
			SET_VERTEX(CHARS_PER_AXIS * k * fw + x_offset, y + 2 * fh + y_offset + 1);
		    }
		    glEnd();
		}
	    }

	    /* Fill axis table with roles and values */

	    for (int axis_num = 0; axis_num < SDL_JoystickNumAxes(sdlstick); axis_num++){
		if(axis_num > MAX_JOYSTICK_AXES)
		    break;

		/* Print axis role if defined, or just print axis number if not. */
		if(gc_joystick != NULL){
		    COLOR_GREEN
		    /* 's' must have CHARS_PER_AXIS characters */
		    if(axis_num == gc_joystick->axis_heading)
			s = " Head.";
		    else if(axis_num == gc_joystick->axis_bank)
			s = " Bank";
		    else if(axis_num == gc_joystick->axis_pitch)
			s = " Pitch";
		    else if(axis_num == gc_joystick->axis_throttle)
			s = " Thro.";
		    else if(axis_num == gc_joystick->axis_hat_x)
			s = " POV X";
		    else if(axis_num == gc_joystick->axis_hat_y)
			s = " POV Y";
		    else if(axis_num == gc_joystick->axis_brake_left)
			s = " L Brk";
		    else if(axis_num == gc_joystick->axis_brake_right)
			s = " R Brk";
		    else{
			snprintf(ns, sizeof(ns), "  %2i   ", axis_num + 1);
			s = ns;
		    }
		}
		else{
		    COLOR_GRAY
		    snprintf(ns, sizeof(ns), "  %2i   ", axis_num + 1);
		    s = ns;
		}

		DRAW_STRING(x + x_offset, y + y_offset, s);
		y += fh;

		/* Print axes values */
		/* 'ns' must have CHARS_PER_AXIS characters */
		snprintf(ns, sizeof(ns), "%6i", SDL_JoystickGetAxis(sdlstick, axis_num));
		s = ns;
		COLOR_YELLOW
		DRAW_STRING(x + x_offset, y + y_offset, s);
		y -= fh;

		x += STRLEN(s) * fw;
	    }
#undef CHARS_PER_AXIS
	    y += 2.2f * fh + 2;


	    /* Joystick buttons */

	    char s_btn_num[2+1];
	    x = 0;
	    for(int k = 0; k < SDL_JoystickNumButtons(sdlstick); k++)
	    {
		if(k >= MAX_JOYSTICK_BTNS)
		    break;

		GLint posX, posY;

		posX = x + x_offset + x_min - 2;
		posY = height - (2 + y + y_offset + y_min + fh + 1);

		COLOR_GREEN
		glRasterPos2i(posX, posY);
		glBitmap(22, 22, 0.0f, 0.0f, 22.0f, 0.0f, btn_released_bm);

		/* Button pressed ? */
		if(SDL_JoystickGetButton(sdlstick, k))
		{
		    COLOR_YELLOW
		    glRasterPos2i(posX, posY);
		    glBitmap(22, 22, 0.0f, 0.0f, 22.0f, 0.0f, btn_pressed_bm);

		    COLOR_BLACK
		}
		else
		    COLOR_GREEN

		/* Button number */
		snprintf(s_btn_num, sizeof(s_btn_num), "%d", k + 1);
		DRAW_STRING( (float)((2 - strlen(s_btn_num)) * fw)/2.0f + x + x_offset, y + y_offset, s_btn_num);

		x += 3 * fw;
	    }
	    y += 2 * fh;
	}
	else
	{
	    /* This joystick is not connected */

	    /* This joystick not mapped? */
	    if(!( (i == 0 && strlen(opt->js0_sdl_guid_s) > 0) ||
		  (i == 1 && strlen(opt->js1_sdl_guid_s) > 0) ||
		  (i == 2 && strlen(opt->js2_sdl_guid_s) > 0)
		)
	    )
		continue;

	    char ns[48+1];
	    x = 0;

	    COLOR_GREEN
	    s = "Joystick #";
	    DRAW_STRING(x + x_offset, y + y_offset, s);
	    x += STRLEN(s) * fw;
	    COLOR_YELLOW
	    snprintf(ns, sizeof(ns), "%d", i + 1);
	    s = ns;
	    DRAW_STRING(x + x_offset, y + y_offset, s);
	    x += STRLEN(s) * fw;
	    COLOR_GREEN
	    s = ": ";
	    DRAW_STRING(x + x_offset, y + y_offset, s);
	    x += STRLEN(s) * fw;
	    COLOR_YELLOW
	    if(i == 0 && strlen(opt->js0_sdl_guid_s) > 0)
		snprintf(ns, sizeof(ns), "%s", opt->js0_sdl_name);
	    else if(i == 1 && strlen(opt->js1_sdl_guid_s) > 0)
		snprintf(ns, sizeof(ns), "%s", opt->js1_sdl_name);
	    else if(i == 2 && strlen(opt->js2_sdl_guid_s) > 0)
		snprintf(ns, sizeof(ns), "%s", opt->js2_sdl_name);

	    s = ns;
	    DRAW_STRING(x + x_offset, y + y_offset, s);
	    /* x += STRLEN(s) * fw; */
	    y += fh;
	    x = 0;
	    COLOR_RED
	    s = "[WARNING: JOYSTICK MAPPED BUT NOT CONNECTED!]";
	    DRAW_STRING(x + x_offset, y + y_offset, s);
	    /* x += STRLEN(s) * fw; */

	    y += 2 * fh;
	}
    }

    if(gc->total_sdl_joysticks == 0 &&
	strlen(opt->js0_sdl_guid_s) == 0 &&
	strlen(opt->js1_sdl_guid_s) == 0 &&
	strlen(opt->js2_sdl_guid_s) == 0
    )
    {
	const char s0[] = "No Joystick Connected";

	s = s0;
	x = x_offset + (w - x_offset) / 2 - ((STRLEN(s) * fw) / 2);
	y = (h - y_offset) / 2 + y_offset / 2 - (fh / 2);

	COLOR_YELLOW
	DRAW_STRING(x, y, s);
    }

/* It works, but is it a good idea to forbid access to mapping menu when
 * there is no joystick connected?
 */
if(False)
{
    /* Set Mapping button sensivity */

    int MB_obj_num;
    Boolean sensitive;

    if(gc->total_sdl_joysticks == 0)
	sensitive = False;
    else
	sensitive = True;

    /* Get "Mapping" button object number on menu page */
    SARMenuGetObjectByID(
	menu,
	SAR_MENU_ID_GOTO_OPTIONS_CONTROLLER_JS_MAPPING,
	&MB_obj_num
    );

    /* Set Mapping button to nonsensitive */
    SARMenuObjectSetSensitive(
	display, menu, MB_obj_num,
	sensitive, True
	);

    /* Button label object number is button object number - 1 :
     * see SARMenuBuildStandardButton()
     * and SARMenuBuildOptionsButton().
     *
     * Set button label to nonsensitive
     */
    SARMenuObjectSetSensitive(
	display, menu, MB_obj_num - 1,
	sensitive, True
	);
}

    /* Reinit if a joystick is newly plugged or removed */

    Boolean doJoystickReinit = True;
    while(SDL_PollEvent(&event))
    {
	switch (event.type)
	{
	    case SDL_JOYDEVICEADDED:
	    case SDL_JOYDEVICEREMOVED:
		if(doJoystickReinit){
		    SARMenuOptionsJoystickReinit(core_ptr);

		    /* Do not reinit one more time during this loop
		     * if an other joystick was plugged / unplugged .
		     */
		    doJoystickReinit = False;
		}
		break;
	    default:
		break;
	}
    }

#undef COLOR_BLACK
#undef COLOR_RED
#undef COLOR_LIGHT_GREEN
#undef COLOR_GREEN
#undef COLOR_YELLOW
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
		GWVidModesSort(m,n);
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
    //const sar_option_struct *opt;
    if((display == NULL) || (m == NULL))
	return;

    //opt = &core_ptr->option;

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
	switch (recorder->type){
	    case SNDSERV_TYPE_OPENAL:
		mesg = strcatalloc(mesg, "OpenAL");
		break;
	}

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
    /*
    mesg = strcatalloc(mesg, "Sound Support:<bold>");
    mesg = strcatalloc(mesg, " SDL"); */
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

	/* Wind */
	sw = SARMenuOptionsGetSwitchByID(
	    m, SAR_MENU_ID_OPT_WIND, NULL
	    );
	if(sw != NULL)
	    sw->state = opt->wind;
    }

    /* Menu: Options->Controller->Joystick->Mapping */
    i = SARMatchMenuByName(core_ptr, SAR_MENU_NAME_OPTIONS_CONTROLLER_JS_MAPPING);
    m = ((i < 0) ? NULL : core_ptr->menu[i]);
    if(m != NULL)
    {
	/* Axis mapping spins */

#define JS0_FIRST_SPIN_VALUE	1
#define JS1_FIRST_SPIN_VALUE	MAX_JOYSTICK_AXES + 1
#define JS2_FIRST_SPIN_VALUE	2 * MAX_JOYSTICK_AXES + 1

	spin = SARMenuOptionsGetSpinByID(
	    m, SAR_MENU_ID_OPT_HEADING_AXIS, NULL
	    );
	if(spin != NULL)
	{
	    if(opt->js0_axis_heading > -1)
		spin->cur_value = opt->js0_axis_heading + JS0_FIRST_SPIN_VALUE;
	    else if(opt->js1_axis_heading > -1)
		spin->cur_value = opt->js1_axis_heading + JS1_FIRST_SPIN_VALUE;
	    else if(opt->js2_axis_heading > -1)
		spin->cur_value = opt->js2_axis_heading + JS2_FIRST_SPIN_VALUE;
	    else
		spin->cur_value = 0;
	}

	spin = SARMenuOptionsGetSpinByID(
	    m, SAR_MENU_ID_OPT_PITCH_AXIS, NULL
	    );
	if(spin != NULL)
	{
	    if(opt->js0_axis_pitch > -1)
		spin->cur_value = opt->js0_axis_pitch + JS0_FIRST_SPIN_VALUE;
	    else if(opt->js1_axis_pitch > -1)
		spin->cur_value = opt->js1_axis_pitch + JS1_FIRST_SPIN_VALUE;
	    else if(opt->js2_axis_pitch > -1)
		spin->cur_value = opt->js2_axis_pitch + JS2_FIRST_SPIN_VALUE;
	    else
		spin->cur_value = 0;
	}

	spin = SARMenuOptionsGetSpinByID(
	    m, SAR_MENU_ID_OPT_BANK_AXIS, NULL
	    );
	if(spin != NULL)
	{
	    if(opt->js0_axis_bank > -1)
		spin->cur_value = opt->js0_axis_bank + JS0_FIRST_SPIN_VALUE;
	    else if(opt->js1_axis_bank > -1)
		spin->cur_value = opt->js1_axis_bank + JS1_FIRST_SPIN_VALUE;
	    else if(opt->js2_axis_bank > -1)
		spin->cur_value = opt->js2_axis_bank + JS2_FIRST_SPIN_VALUE;
	    else
		spin->cur_value = 0;
	}

	spin = SARMenuOptionsGetSpinByID(
	    m, SAR_MENU_ID_OPT_THROTTLE_AXIS, NULL
	    );
	if(spin != NULL)
	{
	    if(opt->js0_axis_throttle > -1)
		spin->cur_value = opt->js0_axis_throttle + JS0_FIRST_SPIN_VALUE;
	    else if(opt->js1_axis_throttle > -1)
		spin->cur_value = opt->js1_axis_throttle + JS1_FIRST_SPIN_VALUE;
	    else if(opt->js2_axis_throttle > -1)
		spin->cur_value = opt->js2_axis_throttle + JS2_FIRST_SPIN_VALUE;
	    else
		spin->cur_value = 0;
	}

	spin = SARMenuOptionsGetSpinByID(
	    m, SAR_MENU_ID_OPT_BRAKE_LEFT_AXIS, NULL
	    );
	if(spin != NULL)
	{
	    if(opt->js0_axis_brake_left > -1)
		spin->cur_value = opt->js0_axis_brake_left + JS0_FIRST_SPIN_VALUE;
	    else if(opt->js1_axis_brake_left > -1)
		spin->cur_value = opt->js1_axis_brake_left + JS1_FIRST_SPIN_VALUE;
	    else if(opt->js2_axis_brake_left > -1)
		spin->cur_value = opt->js2_axis_brake_left + JS2_FIRST_SPIN_VALUE;
	    else
		spin->cur_value = 0;
	}

	spin = SARMenuOptionsGetSpinByID(
	    m, SAR_MENU_ID_OPT_BRAKE_RIGHT_AXIS, NULL
	    );
	if(spin != NULL)
	{
	    if(opt->js0_axis_brake_right > -1)
		spin->cur_value = opt->js0_axis_brake_right + JS0_FIRST_SPIN_VALUE;
	    else if(opt->js1_axis_brake_right > -1)
		spin->cur_value = opt->js1_axis_brake_right + JS1_FIRST_SPIN_VALUE;
	    else if(opt->js2_axis_brake_right > -1)
		spin->cur_value = opt->js2_axis_brake_right + JS2_FIRST_SPIN_VALUE;
	    else
		spin->cur_value = 0;
	}

	spin = SARMenuOptionsGetSpinByID(
	    m, SAR_MENU_ID_OPT_POV_HAT_X, NULL
	    );
	if(spin != NULL)
	{
	    if(opt->js0_axis_hat_x > -1)
		spin->cur_value = opt->js0_axis_hat_x + JS0_FIRST_SPIN_VALUE;
	    else if(opt->js1_axis_hat_x > -1)
		spin->cur_value = opt->js1_axis_hat_x + JS1_FIRST_SPIN_VALUE;
	    else if(opt->js2_axis_hat_x > -1)
		spin->cur_value = opt->js2_axis_hat_x + JS2_FIRST_SPIN_VALUE;
	    else
		spin->cur_value = 0;
	}

	spin = SARMenuOptionsGetSpinByID(
	    m, SAR_MENU_ID_OPT_POV_HAT_Y, NULL
	    );
	if(spin != NULL)
	{
	    if(opt->js0_axis_hat_y > -1)
		spin->cur_value = opt->js0_axis_hat_y + JS0_FIRST_SPIN_VALUE;
	    else if(opt->js1_axis_hat_y > -1)
		spin->cur_value = opt->js1_axis_hat_y + JS1_FIRST_SPIN_VALUE;
	    else if(opt->js2_axis_hat_y > -1)
		spin->cur_value = opt->js2_axis_hat_y + JS2_FIRST_SPIN_VALUE;
	    else
		spin->cur_value = 0;
	}
#undef JS2_FIRST_SPIN_VALUE
#undef JS1_FIRST_SPIN_VALUE
#undef JS0_FIRST_SPIN_VALUE

#define JS0_FIRST_SPIN_VALUE	1
#define JS1_FIRST_SPIN_VALUE	MAX_JOYSTICK_HATS + 1
#define JS2_FIRST_SPIN_VALUE	2 * MAX_JOYSTICK_HATS + 1
	spin = SARMenuOptionsGetSpinByID(
	    m, SAR_MENU_ID_OPT_POV_HAT, NULL
	    );
	if(spin != NULL)
	{
	    if(opt->js0_pov_hat > -1)
		spin->cur_value = opt->js0_pov_hat + JS0_FIRST_SPIN_VALUE;
	    else if(opt->js1_pov_hat > -1)
		spin->cur_value = opt->js1_pov_hat + JS1_FIRST_SPIN_VALUE;
	    else if(opt->js2_pov_hat > -1)
		spin->cur_value = opt->js2_pov_hat + JS2_FIRST_SPIN_VALUE;
	    else
		spin->cur_value = 0;
	}
#undef JS2_FIRST_SPIN_VALUE
#undef JS1_FIRST_SPIN_VALUE
#undef JS0_FIRST_SPIN_VALUE

	/* Axis mapping switches */

	/* Heading axis inversion */
	sw = SARMenuOptionsGetSwitchByID(
	    m, SAR_MENU_ID_OPT_HEADING_AXIS_INV, NULL
	    );
	if(sw != NULL)
	    sw->state = opt->js_axes_inversion_bits & GCTL_JS_AXIS_INV_HEADING;

	/* Pitch axis inversion */
	sw = SARMenuOptionsGetSwitchByID(
	    m, SAR_MENU_ID_OPT_PITCH_AXIS_INV, NULL
	    );
	if(sw != NULL)
	    sw->state = opt->js_axes_inversion_bits & GCTL_JS_AXIS_INV_PITCH;

	/* Bank axis inversion */
	sw = SARMenuOptionsGetSwitchByID(
	    m, SAR_MENU_ID_OPT_BANK_AXIS_INV, NULL
	    );
	if(sw != NULL)
	    sw->state = opt->js_axes_inversion_bits & GCTL_JS_AXIS_INV_BANK;

	/* Throtle axis inversion */
	sw = SARMenuOptionsGetSwitchByID(
	    m, SAR_MENU_ID_OPT_THROTTLE_AXIS_INV, NULL
	    );
	if(sw != NULL)
	    sw->state = opt->js_axes_inversion_bits & GCTL_JS_AXIS_INV_THROTTLE;

	/* POV Hat X axis inversion */
	sw = SARMenuOptionsGetSwitchByID(
	    m, SAR_MENU_ID_OPT_POV_HAT_X_INV, NULL
	    );
	if(sw != NULL)
	    sw->state = opt->js_axes_inversion_bits & GCTL_JS_AXIS_INV_HAT_X;

	/* POV Hat Y axis inversion */
	sw = SARMenuOptionsGetSwitchByID(
	    m, SAR_MENU_ID_OPT_POV_HAT_Y_INV, NULL
	    );
	if(sw != NULL)
	    sw->state = opt->js_axes_inversion_bits & GCTL_JS_AXIS_INV_HAT_Y;

	/* Joystick POV Hat inversion */
	sw = SARMenuOptionsGetSwitchByID(
	    m, SAR_MENU_ID_OPT_POV_HAT_INV, NULL
	    );
	if(sw != NULL)
	    sw->state = opt->js_axes_inversion_bits & GCTL_JS_AXIS_INV_POV_HAT;

	/* Brake Left axis inversion */
	sw = SARMenuOptionsGetSwitchByID(
	    m, SAR_MENU_ID_OPT_BRAKE_LEFT_AXIS_INV, NULL
	    );
	if(sw != NULL)
	    sw->state = opt->js_axes_inversion_bits & GCTL_JS_AXIS_INV_BRAKE_LEFT;

	/* Brake Right axis inversion */
	sw = SARMenuOptionsGetSwitchByID(
	    m, SAR_MENU_ID_OPT_BRAKE_RIGHT_AXIS_INV, NULL
	    );
	if(sw != NULL)
	    sw->state = opt->js_axes_inversion_bits & GCTL_JS_AXIS_INV_BRAKE_RIGHT;


	/* Buttons mapping spins */
#define JS0_BTN_FIRST_SPIN_VALUE	1
#define JS1_BTN_FIRST_SPIN_VALUE	MAX_JOYSTICK_BTNS + 1
#define JS2_BTN_FIRST_SPIN_VALUE	2 * MAX_JOYSTICK_BTNS + 1
	spin = SARMenuOptionsGetSpinByID(
	    m, SAR_MENU_ID_OPT_ROTATE_BUTTON, NULL
	    );
	if(spin != NULL)
	{
	    if(opt->js0_btn_rotate > -1)
		spin->cur_value = opt->js0_btn_rotate + JS0_BTN_FIRST_SPIN_VALUE;
	    else if(opt->js1_btn_rotate > -1)
		spin->cur_value = opt->js1_btn_rotate + JS1_BTN_FIRST_SPIN_VALUE;
	    else if(opt->js2_btn_rotate > -1)
		spin->cur_value = opt->js2_btn_rotate + JS2_BTN_FIRST_SPIN_VALUE;
	    else
		spin->cur_value = 0;
	}

	spin = SARMenuOptionsGetSpinByID(
	    m, SAR_MENU_ID_OPT_AIR_BRAKES_BUTTON, NULL
	    );
	if(spin != NULL)
	{
	    if(opt->js0_btn_air_brakes > -1)
		spin->cur_value = opt->js0_btn_air_brakes + JS0_BTN_FIRST_SPIN_VALUE;
	    else if(opt->js1_btn_air_brakes > -1)
		spin->cur_value = opt->js1_btn_air_brakes + JS1_BTN_FIRST_SPIN_VALUE;
	    else if(opt->js2_btn_air_brakes > -1)
		spin->cur_value = opt->js2_btn_air_brakes + JS2_BTN_FIRST_SPIN_VALUE;
	    else
		spin->cur_value = 0;
	}

	spin = SARMenuOptionsGetSpinByID(
	    m, SAR_MENU_ID_OPT_WHEEL_BRAKES_BUTTON, NULL
	    );
	if(spin != NULL)
	{
	    if(opt->js0_btn_wheel_brakes > -1)
		spin->cur_value = opt->js0_btn_wheel_brakes + JS0_BTN_FIRST_SPIN_VALUE;
	    else if(opt->js1_btn_wheel_brakes > -1)
		spin->cur_value = opt->js1_btn_wheel_brakes + JS1_BTN_FIRST_SPIN_VALUE;
	    else if(opt->js2_btn_wheel_brakes > -1)
		spin->cur_value = opt->js2_btn_wheel_brakes + JS2_BTN_FIRST_SPIN_VALUE;
	    else
		spin->cur_value = 0;
	}

	spin = SARMenuOptionsGetSpinByID(
	    m, SAR_MENU_ID_OPT_ZOOM_IN_BUTTON, NULL
	    );
	if(spin != NULL)
	{
	    if(opt->js0_btn_zoom_in > -1)
		spin->cur_value = opt->js0_btn_zoom_in + JS0_BTN_FIRST_SPIN_VALUE;
	    else if(opt->js1_btn_zoom_in > -1)
		spin->cur_value = opt->js1_btn_zoom_in + JS1_BTN_FIRST_SPIN_VALUE;
	    else if(opt->js2_btn_zoom_in > -1)
		spin->cur_value = opt->js2_btn_zoom_in + JS2_BTN_FIRST_SPIN_VALUE;
	    else
		spin->cur_value = 0;
	}

	spin = SARMenuOptionsGetSpinByID(
	    m, SAR_MENU_ID_OPT_ZOOM_OUT_BUTTON, NULL
	    );
	if(spin != NULL)
	{
	    if(opt->js0_btn_zoom_out > -1)
		spin->cur_value = opt->js0_btn_zoom_out + JS0_BTN_FIRST_SPIN_VALUE;
	    else if(opt->js1_btn_zoom_out > -1)
		spin->cur_value = opt->js1_btn_zoom_out + JS1_BTN_FIRST_SPIN_VALUE;
	    else if(opt->js2_btn_zoom_out > -1)
		spin->cur_value = opt->js2_btn_zoom_out + JS2_BTN_FIRST_SPIN_VALUE;
	    else
		spin->cur_value = 0;
	}

	spin = SARMenuOptionsGetSpinByID(
	    m, SAR_MENU_ID_OPT_HOIST_UP_BUTTON, NULL
	    );
	if(spin != NULL)
	{
	    if(opt->js0_btn_hoist_up > -1)
		spin->cur_value = opt->js0_btn_hoist_up + JS0_BTN_FIRST_SPIN_VALUE;
	    else if(opt->js1_btn_hoist_up > -1)
		spin->cur_value = opt->js1_btn_hoist_up + JS1_BTN_FIRST_SPIN_VALUE;
	    else if(opt->js2_btn_hoist_up > -1)
		spin->cur_value = opt->js2_btn_hoist_up + JS2_BTN_FIRST_SPIN_VALUE;
	    else
		spin->cur_value = 0;
	}

	spin = SARMenuOptionsGetSpinByID(
	    m, SAR_MENU_ID_OPT_HOIST_DOWN_BUTTON, NULL
	    );
	if(spin != NULL)
	{
	    if(opt->js0_btn_hoist_down > -1)
		spin->cur_value = opt->js0_btn_hoist_down + JS0_BTN_FIRST_SPIN_VALUE;
	    else if(opt->js1_btn_hoist_down > -1)
		spin->cur_value = opt->js1_btn_hoist_down + JS1_BTN_FIRST_SPIN_VALUE;
	    else if(opt->js2_btn_hoist_down > -1)
		spin->cur_value = opt->js2_btn_hoist_down + JS2_BTN_FIRST_SPIN_VALUE;
	    else
		spin->cur_value = 0;
	}
#undef JS2_BTN_FIRST_SPIN_VALUE
#undef JS1_BTN_FIRST_SPIN_VALUE
#undef JS0_BTN_FIRST_SPIN_VALUE
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
		h = opt->last_height,
		sv = 0,
		i,n;

	    gw_vidmode_struct
		*vm = GWVidModesGet(display, &n),
		*vm_ptr;

	    GWVidModesSort(vm,n);
	    for (i = 0; i < n; i++)
	    {
		vm_ptr = &vm[i];
		if (v == vm_ptr->width && h == vm_ptr->height)
		{
		    sv = i;
		    break;
		}
	    }
	    GWVidModesFree(vm,n);
	    spin->cur_value = CLIP(sv, 0, spin->total_values - 1);
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
//fprintf(stderr, "%s:%d: Entering SARMenuOptionsSwitchCB()\n", __FILE__, __LINE__);
    sar_core_struct *core_ptr = SAR_CORE(client_data);
    //gw_display_struct *display;
    sar_menu_switch_struct *sw = (sar_menu_switch_struct *)object;
    sar_option_struct *opt;
    if((core_ptr == NULL) || (sw == NULL))
	return;

    //display = core_ptr->display;
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

	case SAR_MENU_ID_OPT_WIND:
	    opt->wind = state;
	    break;

	case SAR_MENU_ID_OPT_HEADING_AXIS_INV:
	    if(state)
		opt->js_axes_inversion_bits |= GCTL_JS_AXIS_INV_HEADING;
	    else
		opt->js_axes_inversion_bits &= ~GCTL_JS_AXIS_INV_HEADING;
	    break;

	case SAR_MENU_ID_OPT_PITCH_AXIS_INV:
	    if(state)
		opt->js_axes_inversion_bits |= GCTL_JS_AXIS_INV_PITCH;
	    else
		opt->js_axes_inversion_bits &= ~GCTL_JS_AXIS_INV_PITCH;
	    break;

	case SAR_MENU_ID_OPT_BANK_AXIS_INV:
	    if(state)
		opt->js_axes_inversion_bits |= GCTL_JS_AXIS_INV_BANK;
	    else
		opt->js_axes_inversion_bits &= ~GCTL_JS_AXIS_INV_BANK;
	    break;

	case SAR_MENU_ID_OPT_THROTTLE_AXIS_INV:
	    if(state)
		opt->js_axes_inversion_bits |= GCTL_JS_AXIS_INV_THROTTLE;
	    else
		opt->js_axes_inversion_bits &= ~GCTL_JS_AXIS_INV_THROTTLE;
	    break;

	case SAR_MENU_ID_OPT_BRAKE_LEFT_AXIS_INV:
	    if(state)
		opt->js_axes_inversion_bits |= GCTL_JS_AXIS_INV_BRAKE_LEFT;
	    else
		opt->js_axes_inversion_bits &= ~GCTL_JS_AXIS_INV_BRAKE_LEFT;
	    break;

	case SAR_MENU_ID_OPT_BRAKE_RIGHT_AXIS_INV:
	    if(state)
		opt->js_axes_inversion_bits |= GCTL_JS_AXIS_INV_BRAKE_RIGHT;
	    else
		opt->js_axes_inversion_bits &= ~GCTL_JS_AXIS_INV_BRAKE_RIGHT;
	    break;

	case SAR_MENU_ID_OPT_POV_HAT_INV:
	    if(state)
		opt->js_axes_inversion_bits |= GCTL_JS_AXIS_INV_POV_HAT;
	    else
		opt->js_axes_inversion_bits &= ~GCTL_JS_AXIS_INV_POV_HAT;
	    break;

	case SAR_MENU_ID_OPT_POV_HAT_X_INV:
	    if(state)
		opt->js_axes_inversion_bits |= GCTL_JS_AXIS_INV_HAT_X;
	    else
		opt->js_axes_inversion_bits &= ~GCTL_JS_AXIS_INV_HAT_X;
	    break;

	case SAR_MENU_ID_OPT_POV_HAT_Y_INV:
	    if(state)
		opt->js_axes_inversion_bits |= GCTL_JS_AXIS_INV_HAT_Y;
	    else
		opt->js_axes_inversion_bits &= ~GCTL_JS_AXIS_INV_HAT_Y;
	    break;
    }
//fprintf(stderr, "%s:%d: Exiting SARMenuOptionsSwitchCB()\n", __FILE__, __LINE__);
}

/*
 *      Options menu spin callback.
 */
void SARMenuOptionsSpinCB(
    void *object, int id, void *client_data,
    char *value
    )
{
//fprintf(stderr, "%s:%d: Entering SARMenuOptionsSpinCB()\n", __FILE__, __LINE__);
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

#define JS1_FIRST_SPIN_VALUE	MAX_JOYSTICK_AXES + 1
#define JS2_FIRST_SPIN_VALUE	2 * MAX_JOYSTICK_AXES + 1
#define JS2_LAST_SPIN_VALUE	3 * MAX_JOYSTICK_AXES
	case SAR_MENU_ID_OPT_HEADING_AXIS:
	    if(spin->cur_value == 0){
		opt->js0_axis_heading = -1;
		opt->js1_axis_heading = -1;
		opt->js2_axis_heading = -1;
		break;
	    }
	    else{
		/* As Heading is mapped to an axis, force Rotate button
		* spin value to None.
		*/

		/* Get Rotate button spin object number on menu page */
		int spin_num;
		SARMenuGetObjectByID(
		    m, SAR_MENU_ID_OPT_ROTATE_BUTTON, &spin_num
		    );
		/* Set Rotate spin value to 0 */
		SARMenuSpinSelectValueIndex(
		    display, m, spin_num, 0, True
		    );

		if(spin->cur_value < JS1_FIRST_SPIN_VALUE){
		    opt->js0_axis_heading = spin->cur_value - 1;
		    opt->js1_axis_heading = -1;
		    opt->js2_axis_heading = -1;
		}
		else if(spin->cur_value < JS2_FIRST_SPIN_VALUE){
		    opt->js0_axis_heading = -1;
		    opt->js1_axis_heading = spin->cur_value - JS1_FIRST_SPIN_VALUE - 2;
		    opt->js2_axis_heading = -1;
		}
		else if(spin->cur_value <= JS2_LAST_SPIN_VALUE){
		    opt->js0_axis_heading = -1;
		    opt->js1_axis_heading = -1;
		    opt->js2_axis_heading = spin->cur_value - JS2_FIRST_SPIN_VALUE - 2;
		}
		else
		    ;
	    }
	    break;

	case SAR_MENU_ID_OPT_PITCH_AXIS:
	    if(spin->cur_value == 0){
		opt->js0_axis_pitch = -1;
		opt->js1_axis_pitch = -1;
		opt->js2_axis_pitch = -1;
		break;
	    }
	    else if(spin->cur_value < JS1_FIRST_SPIN_VALUE){
		opt->js0_axis_pitch = spin->cur_value - 1;
		opt->js1_axis_pitch = -1;
		opt->js2_axis_pitch = -1;
	    }
	    else if(spin->cur_value < JS2_FIRST_SPIN_VALUE){
		opt->js0_axis_pitch = -1;
		opt->js1_axis_pitch = spin->cur_value - JS1_FIRST_SPIN_VALUE - 2;
		opt->js2_axis_pitch = -1;
	    }
	    else if(spin->cur_value <= JS2_LAST_SPIN_VALUE){
		opt->js0_axis_pitch = -1;
		opt->js1_axis_pitch = -1;
		opt->js2_axis_pitch = spin->cur_value - JS2_FIRST_SPIN_VALUE - 2;
	    }
	    else
		;
	    break;

	case SAR_MENU_ID_OPT_BANK_AXIS:
	    if(spin->cur_value == 0){
		opt->js0_axis_bank = -1;
		opt->js1_axis_bank = -1;
		opt->js2_axis_bank = -1;
		break;
	    }
	    else if(spin->cur_value < JS1_FIRST_SPIN_VALUE){
		opt->js0_axis_bank = spin->cur_value - 1;
		opt->js1_axis_bank = -1;
		opt->js2_axis_bank = -1;
	    }
	    else if(spin->cur_value < JS2_FIRST_SPIN_VALUE){
		opt->js0_axis_bank = -1;
		opt->js1_axis_bank = spin->cur_value - JS1_FIRST_SPIN_VALUE - 2;
		opt->js2_axis_bank = -1;
	    }
	    else if(spin->cur_value <= JS2_LAST_SPIN_VALUE){
		opt->js0_axis_bank = -1;
		opt->js1_axis_bank = -1;
		opt->js2_axis_bank = spin->cur_value - JS2_FIRST_SPIN_VALUE - 2;
	    }
	    else
		;
	    break;

	case SAR_MENU_ID_OPT_THROTTLE_AXIS:
	    if(spin->cur_value == 0){
		opt->js0_axis_throttle = -1;
		opt->js1_axis_throttle = -1;
		opt->js2_axis_throttle = -1;
		break;
	    }
	    else if(spin->cur_value < JS1_FIRST_SPIN_VALUE){
		opt->js0_axis_throttle = spin->cur_value - 1;
		opt->js1_axis_throttle = -1;
		opt->js2_axis_throttle = -1;
	    }
	    else if(spin->cur_value < JS2_FIRST_SPIN_VALUE){
		opt->js0_axis_throttle = -1;
		opt->js1_axis_throttle = spin->cur_value - JS1_FIRST_SPIN_VALUE - 2;
		opt->js2_axis_throttle = -1;
	    }
	    else if(spin->cur_value <= JS2_LAST_SPIN_VALUE){
		opt->js0_axis_throttle = -1;
		opt->js1_axis_throttle = -1;
		opt->js2_axis_throttle = spin->cur_value - JS2_FIRST_SPIN_VALUE - 2;
	    }
	    else
		;
	    break;

	case SAR_MENU_ID_OPT_BRAKE_LEFT_AXIS:
	    if(spin->cur_value == 0){
		opt->js0_axis_brake_left = -1;
		opt->js1_axis_brake_left = -1;
		opt->js2_axis_brake_left = -1;
		break;
	    }
	    else{
		/* As Left Wheel Break is mapped to an axis, force Wheel Break
		* button spin value to None.
		*/

		/* Get Wheel Brake button spin object number on menu page */
		int spin_num;
		SARMenuGetObjectByID(
		    m, SAR_MENU_ID_OPT_WHEEL_BRAKES_BUTTON, &spin_num
		    );
		/* Set Wheel Brake spin value to 0 */
		SARMenuSpinSelectValueIndex(
		    display, m, spin_num, 0, True
		    );

		if(spin->cur_value < JS1_FIRST_SPIN_VALUE){
		    opt->js0_axis_brake_left = spin->cur_value - 1;
		    opt->js1_axis_brake_left = -1;
		    opt->js2_axis_brake_left = -1;
		}
		else if(spin->cur_value < JS2_FIRST_SPIN_VALUE){
		    opt->js0_axis_brake_left = -1;
		    opt->js1_axis_brake_left = spin->cur_value - JS1_FIRST_SPIN_VALUE - 2;
		    opt->js2_axis_brake_left = -1;
		}
		else if(spin->cur_value <= JS2_LAST_SPIN_VALUE){
		    opt->js0_axis_brake_left = -1;
		    opt->js1_axis_brake_left = -1;
		    opt->js2_axis_brake_left = spin->cur_value - JS2_FIRST_SPIN_VALUE - 2;
		}
		else
		    ;
	    }
	    break;

	case SAR_MENU_ID_OPT_BRAKE_RIGHT_AXIS:
	    if(spin->cur_value == 0){
		opt->js0_axis_brake_right = -1;
		opt->js1_axis_brake_right = -1;
		opt->js2_axis_brake_right = -1;
		break;
	    }
	    else{
		/* As right Wheel Break is mapped to an axis, force Wheel Break
		* button spin value to None.
		*/

		/* Get Wheel Brake button spin object number on menu page */
		int spin_num;
		SARMenuGetObjectByID(
		    m, SAR_MENU_ID_OPT_WHEEL_BRAKES_BUTTON, &spin_num
		    );
		/* Set Wheel Brake spin value to 0 */
		SARMenuSpinSelectValueIndex(
		    display, m, spin_num, 0, True
		    );

		if(spin->cur_value < JS1_FIRST_SPIN_VALUE){
		    opt->js0_axis_brake_right = spin->cur_value - 1;
		    opt->js1_axis_brake_right = -1;
		    opt->js2_axis_brake_right = -1;
		}
		else if(spin->cur_value < JS2_FIRST_SPIN_VALUE){
		    opt->js0_axis_brake_right = -1;
		    opt->js1_axis_brake_right = spin->cur_value - JS1_FIRST_SPIN_VALUE - 2;
		    opt->js2_axis_brake_right = -1;
		}
		else if(spin->cur_value <= JS2_LAST_SPIN_VALUE){
		    opt->js0_axis_brake_right = -1;
		    opt->js1_axis_brake_right = -1;
		    opt->js2_axis_brake_right = spin->cur_value - JS2_FIRST_SPIN_VALUE - 2;
		}
		else
		    ;
	    }
	    break;

	case SAR_MENU_ID_OPT_POV_HAT_X:
	    if(spin->cur_value == 0){
		opt->js0_axis_hat_x = -1;
		opt->js1_axis_hat_x = -1;
		opt->js2_axis_hat_x = -1;
	    }
	    else{
		/* As POV X is mapped to an analog axis, force "POV Hat" spin
		 * value to None.
		 */

		/* Get "POV Hat" spin object number on menu page */
		int spin_num;
		SARMenuGetObjectByID(
		    m, SAR_MENU_ID_OPT_POV_HAT, &spin_num
		    );
		/* Set "POV Hat" spin value to 0 */
		SARMenuSpinSelectValueIndex(
		    display, m, spin_num, 0, True
		    );

		/* Set option value */
		if(spin->cur_value < JS1_FIRST_SPIN_VALUE){
		    opt->js0_axis_hat_x = spin->cur_value - 1;
		    opt->js1_axis_hat_x = -1;
		    opt->js2_axis_hat_x = -1;
		}
		else if(spin->cur_value < JS2_FIRST_SPIN_VALUE)
		{
		    opt->js0_axis_hat_x = -1;
		    opt->js1_axis_hat_x = spin->cur_value - JS1_FIRST_SPIN_VALUE - 2;
		    opt->js2_axis_hat_x = -1;
		}
		else if(spin->cur_value <= JS2_LAST_SPIN_VALUE)
		{
		    opt->js0_axis_hat_x = -1;
		    opt->js1_axis_hat_x = -1;
		    opt->js2_axis_hat_x = spin->cur_value - JS2_FIRST_SPIN_VALUE - 2;
		}

		/* Clear standard hat values */
		opt->js0_pov_hat = -1;
		opt->js1_pov_hat = -1;
		opt->js2_pov_hat = -1;
	    }
	    break;

	case SAR_MENU_ID_OPT_POV_HAT_Y:
	    if(spin->cur_value == 0){
		opt->js0_axis_hat_y = -1;
		opt->js1_axis_hat_y = -1;
		opt->js2_axis_hat_y = -1;
	    }
	    else{
		/* As POV Y is mapped to an analog axis, force "POV Hat" spin
		* value to None.
		*/

		/* Get "POV Hat" spin object number on menu page */
		int spin_num;
		SARMenuGetObjectByID(
		    m, SAR_MENU_ID_OPT_POV_HAT, &spin_num
		    );
		/* Set "POV Hat" spin value to 0 */
		SARMenuSpinSelectValueIndex(
		    display, m, spin_num, 0, True
		    );

		/* Set option value */
		if(spin->cur_value < JS1_FIRST_SPIN_VALUE){
		    opt->js0_axis_hat_y = spin->cur_value - 1;
		    opt->js1_axis_hat_y = -1;
		    opt->js2_axis_hat_y = -1;
		}
		else if(spin->cur_value < JS2_FIRST_SPIN_VALUE){
		    opt->js0_axis_hat_y = -1;
		    opt->js1_axis_hat_y = spin->cur_value - JS1_FIRST_SPIN_VALUE - 2;
		    opt->js2_axis_hat_y = -1;
		}
		else if(spin->cur_value <= JS2_LAST_SPIN_VALUE){
		    opt->js0_axis_hat_y = -1;
		    opt->js1_axis_hat_y = -1;
		    opt->js2_axis_hat_y = spin->cur_value - JS2_FIRST_SPIN_VALUE - 2;
		}

		/* Clear standard hat values */
		opt->js0_pov_hat = -1;
		opt->js1_pov_hat = -1;
		opt->js2_pov_hat = -1;
	    }
	    break;
#undef JS2_LAST_SPIN_VALUE
#undef JS2_FIRST_SPIN_VALUE
#undef JS1_FIRST_SPIN_VALUE

#define JS1_FIRST_SPIN_VALUE	MAX_JOYSTICK_HATS + 1
#define JS2_FIRST_SPIN_VALUE	2 * MAX_JOYSTICK_HATS + 1
#define JS2_LAST_SPIN_VALUE	3 * MAX_JOYSTICK_HATS
	case SAR_MENU_ID_OPT_POV_HAT:
	    if(spin->cur_value == 0){
		opt->js0_pov_hat = -1;
		opt->js1_pov_hat = -1;
		opt->js2_pov_hat = -1;
		break;
	    }
	    else{
		/* As POV Hat is mapped to a joystick standard hat, force
		 * "POV X Axis" and "POV Y Axis" spins value to None.
		 */

		/* Get "POV X Axis" spin object number */
		int spin_num;
		SARMenuGetObjectByID(
		    m, SAR_MENU_ID_OPT_POV_HAT_X, &spin_num
		    );
		/* Set "POV X Axis" spin value to 0 */
		SARMenuSpinSelectValueIndex(
		    display, m, spin_num, 0, True
		    );
		/* Get "POV Y Axis" spin object number */
		SARMenuGetObjectByID(
		    m, SAR_MENU_ID_OPT_POV_HAT_Y, &spin_num
		    );
		/* Set "POV Y axis" spin value to 0 */
		SARMenuSpinSelectValueIndex(
		    display, m, spin_num, 0, True
		    );

		/* Set option value */
		if(spin->cur_value < JS1_FIRST_SPIN_VALUE){
		    /* Joystick 1 Hat 1 or Hat 2 */
		    opt->js0_pov_hat = spin->cur_value - 1;
		    opt->js1_pov_hat = -1;
		    opt->js2_pov_hat = -1;
		}
		else if(spin->cur_value < JS2_FIRST_SPIN_VALUE){
		    /* Joystick 2 Hat 1 or Hat 2 */
		    opt->js0_pov_hat = -1;
		    opt->js1_pov_hat = spin->cur_value - JS1_FIRST_SPIN_VALUE - 2;
		    opt->js2_pov_hat = -1;
		}
		else if(spin->cur_value <= JS2_LAST_SPIN_VALUE){
		    /* Joystick 3 Hat 1 or Hat 2 */
		    opt->js0_pov_hat = -1;
		    opt->js1_pov_hat = -1;
		    opt->js2_pov_hat = spin->cur_value - JS2_FIRST_SPIN_VALUE - 2;
		}

		/* Clear x and y axes values */
		opt->js0_axis_hat_x = -1;
		opt->js0_axis_hat_y = -1;
		opt->js1_axis_hat_x = -1;
		opt->js1_axis_hat_y = -1;
		opt->js2_axis_hat_x = -1;
		opt->js2_axis_hat_y = -1;
	    }
	    break;
#undef JS2_LAST_SPIN_VALUE
#undef JS2_FIRST_SPIN_VALUE
#undef JS1_FIRST_SPIN_VALUE

#define JS1_FIRST_SPIN_VALUE	MAX_JOYSTICK_BTNS + 1
#define JS2_FIRST_SPIN_VALUE	2 * MAX_JOYSTICK_BTNS + 1
#define JS2_LAST_SPIN_VALUE	3 * MAX_JOYSTICK_BTNS
	case SAR_MENU_ID_OPT_ROTATE_BUTTON:
	    if(spin->cur_value == 0){
		opt->js0_btn_rotate = -1;
		opt->js1_btn_rotate = -1;
		opt->js2_btn_rotate = -1;
	    }
	    else{
		/* As Rotate function is mapped to a button, force Heading axis
		 * spin value to "None".
		 */

		/* Get Heading axis spin object number on menu page */
		int spin_num;
		SARMenuGetObjectByID(
		    m, SAR_MENU_ID_OPT_HEADING_AXIS, &spin_num
		    );
		/* Set Heading spin value to 0 */
		SARMenuSpinSelectValueIndex(
		    display, m, spin_num, 0, True
		    );

		/* Set option value */
		if(spin->cur_value < JS1_FIRST_SPIN_VALUE){
		    opt->js0_btn_rotate = spin->cur_value - 1;
		    opt->js1_btn_rotate = -1;
		    opt->js2_btn_rotate = -1;
		}
		else if(spin->cur_value < JS2_FIRST_SPIN_VALUE)
		{
		    opt->js0_btn_rotate = -1;
		    opt->js1_btn_rotate = spin->cur_value - JS1_FIRST_SPIN_VALUE - 2;
		    opt->js2_btn_rotate = -1;
		}
		else if(spin->cur_value <= JS2_LAST_SPIN_VALUE)
		{
		    opt->js0_btn_rotate = -1;
		    opt->js1_btn_rotate = -1;
		    opt->js2_btn_rotate = spin->cur_value - JS2_FIRST_SPIN_VALUE - 2;
		}
	    }
	    break;

	case SAR_MENU_ID_OPT_AIR_BRAKES_BUTTON:
	    if(spin->cur_value == 0){
		opt->js0_btn_air_brakes = -1;
		opt->js1_btn_air_brakes = -1;
		opt->js2_btn_air_brakes = -1;
	    }
	    else{
		/* Set option value */
		if(spin->cur_value < JS1_FIRST_SPIN_VALUE){
		    opt->js0_btn_air_brakes = spin->cur_value - 1;
		    opt->js1_btn_air_brakes = -1;
		    opt->js2_btn_air_brakes = -1;
		}
		else if(spin->cur_value < JS2_FIRST_SPIN_VALUE)
		{
		    opt->js0_btn_air_brakes = -1;
		    opt->js1_btn_air_brakes = spin->cur_value - JS1_FIRST_SPIN_VALUE - 2;
		    opt->js2_btn_air_brakes = -1;
		}
		else if(spin->cur_value <= JS2_LAST_SPIN_VALUE)
		{
		    opt->js0_btn_air_brakes = -1;
		    opt->js1_btn_air_brakes = -1;
		    opt->js2_btn_air_brakes = spin->cur_value - JS2_FIRST_SPIN_VALUE - 2;
		}
	    }
	    break;

	case SAR_MENU_ID_OPT_WHEEL_BRAKES_BUTTON:
	    if(spin->cur_value == 0){
		opt->js0_btn_wheel_brakes = -1;
		opt->js1_btn_wheel_brakes = -1;
		opt->js2_btn_wheel_brakes = -1;
	    }
	    else{
		/* As Wheel Brake function is mapped to a button, force Wheel
		 * Brakes axes spin values to "None".
		 */

		int spin_num;
		/* Get Left Brake axis spin object number on menu page */
		SARMenuGetObjectByID(
		    m, SAR_MENU_ID_OPT_BRAKE_LEFT_AXIS, &spin_num
		    );
		/* Set Left Brake spin value to 0 */
		SARMenuSpinSelectValueIndex(
		    display, m, spin_num, 0, True
		    );
		/* Get Right Brake axis spin object number on menu page */
		SARMenuGetObjectByID(
		    m, SAR_MENU_ID_OPT_BRAKE_RIGHT_AXIS, &spin_num
		    );
		/* Set Right Brake spin value to 0 */
		SARMenuSpinSelectValueIndex(
		    display, m, spin_num, 0, True
		    );

		/* Set option value */
		if(spin->cur_value < JS1_FIRST_SPIN_VALUE){
		    opt->js0_btn_wheel_brakes = spin->cur_value - 1;
		    opt->js1_btn_wheel_brakes = -1;
		    opt->js2_btn_wheel_brakes = -1;
		}
		else if(spin->cur_value < JS2_FIRST_SPIN_VALUE)
		{
		    opt->js0_btn_wheel_brakes = -1;
		    opt->js1_btn_wheel_brakes = spin->cur_value - JS1_FIRST_SPIN_VALUE - 2;
		    opt->js2_btn_wheel_brakes = -1;
		}
		else if(spin->cur_value <= JS2_LAST_SPIN_VALUE)
		{
		    opt->js0_btn_wheel_brakes = -1;
		    opt->js1_btn_wheel_brakes = -1;
		    opt->js2_btn_wheel_brakes = spin->cur_value - JS2_FIRST_SPIN_VALUE - 2;
		}
	    }
	    break;

	case SAR_MENU_ID_OPT_ZOOM_IN_BUTTON:
	    if(spin->cur_value == 0){
		opt->js0_btn_zoom_in = -1;
		opt->js1_btn_zoom_in = -1;
		opt->js2_btn_zoom_in = -1;
	    }
	    else{
		/* Set option value */
		if(spin->cur_value < JS1_FIRST_SPIN_VALUE){
		    opt->js0_btn_zoom_in = spin->cur_value - 1;
		    opt->js1_btn_zoom_in = -1;
		    opt->js2_btn_zoom_in = -1;
		}
		else if(spin->cur_value < JS2_FIRST_SPIN_VALUE)
		{
		    opt->js0_btn_zoom_in = -1;
		    opt->js1_btn_zoom_in = spin->cur_value - JS1_FIRST_SPIN_VALUE - 2;
		    opt->js2_btn_zoom_in = -1;
		}
		else if(spin->cur_value <= JS2_LAST_SPIN_VALUE)
		{
		    opt->js0_btn_zoom_in = -1;
		    opt->js1_btn_zoom_in = -1;
		    opt->js2_btn_zoom_in = spin->cur_value - JS2_FIRST_SPIN_VALUE - 2;
		}
	    }
	    break;

	case SAR_MENU_ID_OPT_ZOOM_OUT_BUTTON:
	    if(spin->cur_value == 0){
		opt->js0_btn_zoom_out = -1;
		opt->js1_btn_zoom_out = -1;
		opt->js2_btn_zoom_out = -1;
	    }
	    else{
		/* Set option value */
		if(spin->cur_value < JS1_FIRST_SPIN_VALUE){
		    opt->js0_btn_zoom_out = spin->cur_value - 1;
		    opt->js1_btn_zoom_out = -1;
		    opt->js2_btn_zoom_out = -1;
		}
		else if(spin->cur_value < JS2_FIRST_SPIN_VALUE)
		{
		    opt->js0_btn_zoom_out = -1;
		    opt->js1_btn_zoom_out = spin->cur_value - JS1_FIRST_SPIN_VALUE - 2;
		    opt->js2_btn_zoom_out = -1;
		}
		else if(spin->cur_value <= JS2_LAST_SPIN_VALUE)
		{
		    opt->js0_btn_zoom_out = -1;
		    opt->js1_btn_zoom_out = -1;
		    opt->js2_btn_zoom_out = spin->cur_value - JS2_FIRST_SPIN_VALUE - 2;
		}
	    }
	    break;

	case SAR_MENU_ID_OPT_HOIST_UP_BUTTON:
	    if(spin->cur_value == 0){
		opt->js0_btn_hoist_up = -1;
		opt->js1_btn_hoist_up = -1;
		opt->js2_btn_hoist_up = -1;
	    }
	    else{
		/* Set option value */
		if(spin->cur_value < JS1_FIRST_SPIN_VALUE){
		    opt->js0_btn_hoist_up = spin->cur_value - 1;
		    opt->js1_btn_hoist_up = -1;
		    opt->js2_btn_hoist_up = -1;
		}
		else if(spin->cur_value < JS2_FIRST_SPIN_VALUE)
		{
		    opt->js0_btn_hoist_up = -1;
		    opt->js1_btn_hoist_up = spin->cur_value - JS1_FIRST_SPIN_VALUE - 2;
		    opt->js2_btn_hoist_up = -1;
		}
		else if(spin->cur_value <= JS2_LAST_SPIN_VALUE)
		{
		    opt->js0_btn_hoist_up = -1;
		    opt->js1_btn_hoist_up = -1;
		    opt->js2_btn_hoist_up = spin->cur_value - JS2_FIRST_SPIN_VALUE - 2;
		}
	    }
	    break;

	case SAR_MENU_ID_OPT_HOIST_DOWN_BUTTON:
	    if(spin->cur_value == 0){
		opt->js0_btn_hoist_down = -1;
		opt->js1_btn_hoist_down = -1;
		opt->js2_btn_hoist_down = -1;
	    }
	    else{
		/* Set option value */
		if(spin->cur_value < JS1_FIRST_SPIN_VALUE){
		    opt->js0_btn_hoist_down = spin->cur_value - 1;
		    opt->js1_btn_hoist_down = -1;
		    opt->js2_btn_hoist_down = -1;
		}
		else if(spin->cur_value < JS2_FIRST_SPIN_VALUE)
		{
		    opt->js0_btn_hoist_down = -1;
		    opt->js1_btn_hoist_down = spin->cur_value - JS1_FIRST_SPIN_VALUE - 2;
		    opt->js2_btn_hoist_down = -1;
		}
		else if(spin->cur_value <= JS2_LAST_SPIN_VALUE)
		{
		    opt->js0_btn_hoist_down = -1;
		    opt->js1_btn_hoist_down = -1;
		    opt->js2_btn_hoist_down = spin->cur_value - JS2_FIRST_SPIN_VALUE - 2;
		}
	    }
	    break;

#undef JS2_LAST_SPIN_VALUE
#undef JS2_FIRST_SPIN_VALUE
#undef JS1_FIRST_SPIN_VALUE

	case SAR_MENU_ID_OPT_VISIBILITY_MAX:
	    /* These values should correspond to those used to
	     * set the values in SARBuildMenus().
	     */
	    opt->visibility_max = spin->cur_value;
	    break;

	case SAR_MENU_ID_OPT_RESOLUTION:
	    if(display != NULL)
	    {
		int n;
		gw_vidmode_struct *vm = GWVidModesGet(display, &n);
		GWVidModesSort(vm,n);
		if (vm != NULL)
		    SARResolution(display, vm[spin->cur_value].width, vm[spin->cur_value].height);
		GWVidModesFree(vm,n);
	    }
	    break;

/* Macro to turn sound on as needed by checking if the core structure's
 * recorder is initialized. If the recorder is NULL then an attempt to
 * connect to the sound server will be made using the argument
 * specified by sound_server_connect_arg.
 */
#define SND_ON_AS_NEEDED                                                \
	    { if(core_ptr->recorder == NULL) {                          \
		    void *window;                                       \
		    GWContextGet(display, GWContextCurrent(display),	\
				 &window, NULL,                         \
				 NULL, NULL, NULL, NULL                 \
			);                                              \
		    GWSetInputBusy(display);				\
		    core_ptr->recorder = SoundInit(			\
			core_ptr, SOUND_DEFAULT,                        \
			sound_server_connect_arg,                       \
			NULL,		/* Do not start sound server */	\
			window						\
			);                                              \
		    if(core_ptr->recorder == NULL)	{               \
			char *buf = (char *)malloc(                     \
			    (512 + STRLEN(sound_server_connect_arg)) * sizeof(char) \
			    );                                          \
			sprintf(                                        \
			    buf,                                        \
			    "Unable to connect to sound server at:\n\n    %s\n\n\
Make sure that the address is correct,\n\
use the argument --recorder to specify\n\
an alternate address as needed.\n\n\
To start the sound server in most cases\n\
use `starty'",                                                          \
			    sound_server_connect_arg                    \
			    );                                          \
			GWOutputMessage(                                \
			    display,                                    \
			    GWOutputMessageTypeError,                   \
			    "Sound initialization failed!",             \
			    buf,                                        \
			    "Please check to make sure that your sound server\n\
is running and available at the specified address.\n\
Also make sure that you have sufficient permission\n\
to connect to it. To start in most cases run `starty'."                 \
			    );                                          \
			free(buf);                                      \
			SARMenuOptionsSoundRefresh(core_ptr);           \
		    }							\
		    else                                                \
		    {							\
			/* Sound initialization successful */		\
			SoundChangeMode(                                \
			    core_ptr->recorder, core_ptr->audio_mode_name \
			    );                                          \
			SARMenuOptionsSoundRefresh(core_ptr);           \
		    }							\
		    GWSetInputReady(display);				\
		} }

#define SND_OFF_AS_NEEDED                                       \
	    { if(core_ptr->recorder != NULL) {			\
		    GWSetInputBusy(display);                    \
		    SoundShutdown(core_ptr->recorder);          \
		    core_ptr->recorder = NULL;                  \
		    SARMenuOptionsSoundRefresh(core_ptr);       \
		    GWSetInputReady(display);                   \
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
//fprintf(stderr, "%s:%d: Exiting SARMenuOptionsSpinCB()\n", __FILE__, __LINE__);
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
