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

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <SDL2/SDL.h>
#include "gw.h"		/* Need to know about GW key codes */
#include "gctl.h"


static char *last_gctl_error = NULL;


char *GCtlGetError(void);
void GctlJsOpenAndMapp(gctl_struct *gc, gctl_values_struct *v);
int GetSdlJsIndexFromGuidString(char *guid);
int GetSdlJsIndexFromGcSdlJoystick(gctl_struct *gc, int i);
gctl_struct *GCtlNew(gctl_values_struct *v);
void GCtlUpdate(
    gctl_struct *gc,
    Boolean heading_has_nz,	/* Include heading pitch nullzone */
    Boolean pitch_has_nz,	/* Include pitch nullzone */
    Boolean bank_has_nz,	/* Include bank nullzone */
    time_t cur_ms, time_t lapsed_ms,
    float time_compensation
    );
void GCtlHandlePointer(
    gw_display_struct *display, gctl_struct *gc,
    gw_event_type type,
    int button,		/* Button number */
    int x, int y,
    time_t t,		/* Time stamp (in ms) */
    time_t lapsed_ms
    );
void GCtlHandleKey(
    gw_display_struct *display, gctl_struct *gc,
    int k, Boolean state,	/* Key code and state */
    Boolean alt_state,	/* Modifier key states */
    Boolean ctrl_state,
    Boolean shift_state,
    time_t t,		/* Time stamp (in ms) */
    time_t lapsed_ms
    );
void GCtlResetValues(gctl_struct *gc);
void GCtlResetTimmers(gctl_struct *gc);
void GCtlDelete(gctl_struct *gc);


#define ATOI(s)         (((s) != NULL) ? atoi(s) : 0)
#define ATOL(s)         (((s) != NULL) ? atol(s) : 0)
#define ATOF(s)         (((s) != NULL) ? atof(s) : 0.0f)
#define STRDUP(s)       (((s) != NULL) ? strdup(s) : NULL)

#define MAX(a,b)        (((a) > (b)) ? (a) : (b))
#define MIN(a,b)        (((a) < (b)) ? (a) : (b))
#define CLIP(a,l,h)     (MIN(MAX((a),(l)),(h)))
#define STRLEN(s)       (((s) != NULL) ? strlen(s) : 0)
#define STRISEMPTY(s)   (((s) != NULL) ? (*(s) == '\0') : 1)


/*
 *	Returns a pointer to a string describing the last error that
 *	occured while calling a GCtl*() function or NULL if there was
 *	no error.
 */
char *GCtlGetError(void)
{
    return(last_gctl_error);
}


/*
 *	(Re)open game controller joysticks and open/map SDL joysticks to
 * 	relevant game controller joystick.
 *
 * 	* If not yet done:
 * 	   - Allocates gc->joystick structure
 * 	   - Initializes SDL library joystick subsystem
 * 	   - Allocates gc->sdljoystick structure
 *
 * 	* Depending of calling function:
 * 	   - if 'v' pointer is not null, initializes the Game Controller with
 *	     the specified 'v' values.
 * 	   - if 'v' pointer is null, open all plugged sdl joysticks which are
 *	     mapped to at least one role.
 *
 * 	* Always:
 * 	   - Mapps SDL joysticks to the right gc->joysticks. This is necessary
 * 	     for joystick hot plug because SDL_Joystick device_index changes
 * 	     each time that a joystick is unplugged / replugged.
 * 	   - Open all (up to MAX_JOYSTICKS) plugged SDL joysticks which are not
 * 	     yet been mapped or opened.
 *
 */
void GctlJsOpenAndMapp(gctl_struct *gc, gctl_values_struct *v)
{
//fprintf(stderr, "%s:%d : Entering GctlJsOpenAndMapp()\n", __FILE__, __LINE__);
    gctl_values_js_struct *js_value;
    gctl_js_struct *gc_joystick;
    SDL_Joystick *sdljoystick;
    int i, device_index;
    last_gctl_error = NULL;

    /* As SDL_JoystickOpened() function as been removed from SDL library,
     * add a bit array to store which sdl joysticks are already opened.
     */
    Boolean sdl_joystick_opened[MAX_JOYSTICKS] = { False };

    if(gc == NULL)
	return;

    /* If not yet done, allocate gc->joysticks structure */

    if(gc->joystick == NULL)
    {
	gc->total_joysticks = MAX_JOYSTICKS;

	gc->joystick = GCTL_JS(calloc(gc->total_joysticks, sizeof(gctl_js_struct)));
	if(gc->joystick == NULL)
	{
	    fprintf(stderr, "%s:%d: Can't allocate gc->joystick\n", __FILE__, __LINE__);
	    gc->total_joysticks = 0;
	    return;
	}
	else
	{
	    /* Clear gc joysticks mappings */

	    for(i = 0; i < gc->total_joysticks; i++)
	    {
		gc_joystick = &gc->joystick[i];

		strcpy(gc_joystick->sdl_guid_s, "");

		gc_joystick->axes_inversion_bits = 0;

		gc_joystick->axis_heading = -1;
		gc_joystick->axis_pitch = -1;
		gc_joystick->axis_bank = -1;
		gc_joystick->axis_throttle = -1;
		gc_joystick->pov_hat = -1;
		gc_joystick->axis_hat_x = -1;
		gc_joystick->axis_hat_y = -1;
		gc_joystick->axis_brake_left = -1;
		gc_joystick->axis_brake_right = -1;

		gc_joystick->button_rotate = -1;
		gc_joystick->button_air_brakes = -1;
		gc_joystick->button_wheel_brakes = -1;
		gc_joystick->button_zoom_in = -1;
		gc_joystick->button_zoom_out = -1;
		gc_joystick->button_hoist_up = -1;
		gc_joystick->button_hoist_down = -1;
	    }
	}
    }

    /* If not yet done, initialize SDL library joystick and events subsystems */

    if(!SDL_WasInit(SDL_INIT_JOYSTICK))
    {
	/* Attempt to initialize SDL joystick and events subsystems. */
	if(SDL_Init(SDL_INIT_JOYSTICK) == 0)
	{
	    /* SDL is ready */

	    /* Because joystick events subsystem is automatically set by
	     * SDL_Init(SDL_INIT_JOYSTICK) call, from now SDL_PollEvent(...)
	     * can be called to read joystick events.
	     * Joystick events will be shutted off later when we will not need
	     * them anymore (i.e. at joystick test menu exit) by calling
	     * SDL_JoystickEventState() function.
	     */
	}
	else
	{
	    sprintf(last_gctl_error, "Unable to initialize SDL joysticks: %s", SDL_GetError());
	    gc->sdljoystick = NULL;
	    gc->total_joysticks = 0;
	    gc->total_sdl_joysticks = 0;
	    return;
	}
    }

    /* If not yet done, allocate gc->sdljoystick structure.
     *
     * Note: as size of SDL_Joystick* is very small (it's only a pointer),
     * don't waste time to allocate / reallocate it depending of real number
     * of plugged joysticks (i.e. SDL_NumJoysticks()).
     */

    gc->total_sdl_joysticks = MAX_JOYSTICKS;

    if(gc->sdljoystick == NULL)
    {
	gc->sdljoystick = calloc(gc->total_sdl_joysticks, sizeof(SDL_Joystick*));
	if(gc->sdljoystick == NULL)
	{
	    fprintf(stderr, "%s:%d: Can't allocate gc->sdljoystick\n", __FILE__, __LINE__);
	    gc->total_sdl_joysticks = 0;
	    return;
	}
    }

    /* Mapp each SDL joystick to the right gc->joystick (if any). This is
     * necessary for joystick hot plug because a SDL joystick device_index
     * can change each time this joystick is unplugged / replugged.
     *
     * For example if we connect a stick then a throttle,
     * SDL joystick device index will be 0 for stick and 1 for throttle,
     * but if we disconnect them then reconnect the throttle then the stick,
     * SDL joystick device index will be 1 for stick and 0 for throttle.
     */

    /* Initialize the Game Controller with the specified values? */
    if( v != NULL)
    {
	/* Open all plugged sdl joysticks which are referenced by v */

	for(i = 0; i < gc->total_joysticks; i++)
	{
	    js_value = &v->joystick[i];
	    /* Iterate through all plugged sdl joysticks */
	    for(device_index = 0; device_index < gc->total_sdl_joysticks; device_index++)
	    {
		/* This v->joystick identical to current SDL joystick ? */
		if(GetSdlJsIndexFromGuidString(js_value->sdl_guid_s) == device_index)
		{
		    sdljoystick = SDL_JoystickOpen(device_index);
		    if(sdljoystick != NULL){
			sdl_joystick_opened[device_index] = True;
			gc->sdljoystick[i] = sdljoystick;

			fprintf(stderr,
			    "Open SDL joystick index #%d as: Joystick #%d, %s, vendorID = 0x%04x, productID = 0x%04x\n",
			    device_index,
			    i + 1,
			    SDL_JoystickName(sdljoystick),
			    SDL_JoystickGetVendor(sdljoystick),
			    SDL_JoystickGetProduct(sdljoystick)
			);
			break;
		    }
		    else
		    {
			fprintf(stderr, "Can't open SDL joystick #%d (of %d)\n",
			    device_index, SDL_NumJoysticks()
			);

			gc->sdljoystick[i] = NULL;
			break;
		    }
		}
	    }
	}
	return;
    }
    else
    {
	/* Open all plugged sdl joysticks which are already mapped. */

	for(i = 0; i < gc->total_joysticks; i++)
	{
	    gc_joystick = &gc->joystick[i];

	    /* Iterate through plugged sdl joysticks */
	    for(device_index = 0; device_index < gc->total_sdl_joysticks; device_index++)
	    {
		/* This sdl joystick same as this gc joystick? */
		if(GetSdlJsIndexFromGuidString(gc_joystick->sdl_guid_s) == device_index)
		{
		    sdljoystick = SDL_JoystickOpen(device_index);
		    if(sdljoystick != NULL)
		    {
			sdl_joystick_opened[device_index] = True;
			gc->sdljoystick[i] = sdljoystick;
			break;
		    }
		}
	    }
	}
    }

    /* If there is enough free gc joysticks left, open all plugged sdl
    * joysticks which have not yet been mapped or opened.
    */

    for(i = 0; i < gc->total_joysticks; i++)
    {
	gc_joystick = &gc->joystick[i];

	/* This gc joystick not yet mapped? */
	if(strlen(gc_joystick->sdl_guid_s) == 0)
	{
	    for(device_index = 0; device_index < gc->total_sdl_joysticks; device_index++)
	    {
		if(sdl_joystick_opened[device_index] == False)
		{
		    sdljoystick = SDL_JoystickOpen(device_index);
		    if(sdljoystick != NULL)
		    {
			sdl_joystick_opened[device_index] = True;
			gc->sdljoystick[i] = sdljoystick;
			break;
		    }
		}
	    }
	}
    }
//fprintf(stderr, "%s:%d : Exiting GctlJsOpenAndMapp()\n", __FILE__, __LINE__);
}

/*
 *	Returns the sdl joystick device index wich corresponds
 *	to the given sdl joystick guid string.
 * 	Returns -1 if no corespondence found.
 */
int GetSdlJsIndexFromGuidString(char *guid)
{
    SDL_JoystickGUID sdl_guid;
    char sdl_js_guid_s[33];
    int i, device_index = -1;

    if(strlen(guid) == 0)
	return device_index;

    for(i = 0; i < SDL_NumJoysticks(); i++){
	sdl_guid = SDL_JoystickGetDeviceGUID(i);
	SDL_JoystickGetGUIDString(sdl_guid, sdl_js_guid_s,
				  sizeof(sdl_js_guid_s));

	if(!strcmp(guid, sdl_js_guid_s)){
	    device_index = i;
	    break;
	}
    }

    return device_index;
}

/*
 *	Returns the sdl joystick device index wich corresponds
 *	to the given gc->sdljoystick.
 * 	Returns -1 if no corespondence found.
 */
int GetSdlJsIndexFromGcSdlJoystick(gctl_struct *gc, int i)
{
    int j, device_index = -1;

    if(gc != NULL && gc->sdljoystick[i] != NULL)
    {
	for(j = 0; j < gc->total_sdl_joysticks; j++)
	{
	    if(gc->sdljoystick[i] == SDL_JoystickOpen(j)){
		device_index = j;
		break;
	    }
	}
    }

    return device_index;
}

/*
 *	Initializes the Game Controller with the specified values.
 */
gctl_struct *GCtlNew(gctl_values_struct *v)
{
//fprintf(stderr, "%s:%d : Entering GCtlNew()\n", __FILE__, __LINE__);
    int i;
    gctl_js_struct *joystick;
    gctl_values_js_struct *js_value;
    gctl_struct *gc = GCTL(calloc(1, sizeof(gctl_struct)));

    last_gctl_error = NULL;

    /* Game Controller Values not specified? */
    if(v == NULL)
    {
	free(gc);
	last_gctl_error = "Game Controller Values not specified";
	return(NULL);
    }

    /* Set controller(s) and option(s) */
    gc->controllers = v->controllers;
    gc->options = v->options;

    /* Initialize joystick? */
    if(v->controllers & GCTL_CONTROLLER_JOYSTICK)
    {
	/* Open SDL joysticks and map them to relevant gc joysticks */
	GctlJsOpenAndMapp(gc, v);

	/* Iterate through each joystick */
	for(i = 0; i < gc->total_joysticks; i++)
	{
	    /* Get pointer to current Game Controller Joystick and
	     * Joystick Values
	     */
	    joystick = &gc->joystick[i];
	    js_value = &v->joystick[i];

	    /* Begin setting up joystick mappings */
	    /* Copy joystick mappings from js_value structure */

	    strcpy(joystick->sdl_guid_s, js_value->sdl_guid_s);

	    joystick->axes_inversion_bits = js_value->js_axes_inversion_bits;

	    joystick->axis_heading = js_value->axis_heading;
	    joystick->axis_pitch = js_value->axis_pitch;
	    joystick->axis_bank = js_value->axis_bank;
	    joystick->axis_throttle = js_value->axis_throttle;
	    joystick->pov_hat = js_value->pov_hat;
	    joystick->axis_hat_x = js_value->axis_hat_x;
	    joystick->axis_hat_y = js_value->axis_hat_y;
	    joystick->axis_brake_left = js_value->axis_brake_left;
	    joystick->axis_brake_right = js_value->axis_brake_right;

	    joystick->button_rotate = js_value->button_rotate;
	    joystick->button_air_brakes = js_value->button_air_brakes;
	    joystick->button_wheel_brakes = js_value->button_wheel_brakes;
	    joystick->button_zoom_in = js_value->button_zoom_in;
	    joystick->button_zoom_out = js_value->button_zoom_out;
	    joystick->button_hoist_up = js_value->button_hoist_up;
	    joystick->button_hoist_down = js_value->button_hoist_down;
	}	/* Iterate through each joystick */
    }	/* Initialize joystick? */


	/* Initialize keyboard? */
    if(v->controllers & GCTL_CONTROLLER_KEYBOARD)
    {
	/* Begin resetting keyboard values */
	/* Keyboard states */
	gc->heading_kb_state = False;
	gc->pitch_kb_state = False;
	gc->bank_kb_state = False;
	gc->throttle_kb_state = False;
	gc->hat_x_kb_state = False;
	gc->hat_y_kb_state = False;

	/* Last keypress time stamps */
	gc->heading_kb_last = 0;
	gc->pitch_kb_last = 0;
	gc->bank_kb_last = 0;
	gc->throttle_kb_last = 0;
	gc->hat_x_kb_last = 0;
	gc->hat_y_kb_last = 0;

	gc->axis_kb_state = False;
	gc->button_kb_state = False;
    }   /* Initialize keyboard? */

	/* Initialize pointer? */
    if(v->controllers & GCTL_CONTROLLER_POINTER)
    {
	/* Begin resetting pointer values */
	/* Pressed buttons mask */
	gc->pointer_buttons = 0x00000000;
	/* Set default pointer box size */
	gc->pointer_box_x = 0;
	gc->pointer_box_y = 0;
	gc->pointer_box_width = 100;
	gc->pointer_box_height = 70;
	/* Reset pointer coordinates */
	gc->pointer_x = (gc->pointer_box_width / 2);
	gc->pointer_y = (gc->pointer_box_height / 2);
    }   /* Initialize pointer? */
//fprintf(stderr, "%s:%d : Exiting GCtlNew()\n", __FILE__, __LINE__);
    return(gc);
}

/*
 *      Updates the game controller structure with new (current)
 *      controller values.
 *
 *      Keyboard handling is performed in GCtlHandleKey().
 *
 *      Pointer handling is performed in GCtlHandlePointer().
 */
void GCtlUpdate(
    gctl_struct *gc,
    Boolean heading_has_nz,     /* Include heading pitch nullzone */
    Boolean pitch_has_nz,       /* Include pitch nullzone */
    Boolean bank_has_nz,        /* Include bank nullzone */
    time_t cur_ms, time_t lapsed_ms,
    float time_compensation
    )
{
    int i;
    gctl_controllers controllers;
    gctl_options options;
    time_t gc_last_updated_ms, dms;
    gctl_js_struct *joystick;
    Boolean js_def_heading = False,
	js_def_pitch = False,
	js_def_bank = False,
	js_def_hat_x = False,
	js_def_hat_y = False,
	js_def_zoom_in = False,
	js_def_zoom_out = False,
	js_def_air_brakes = False,
	js_def_wheel_brakes = False;
    Boolean     *btn_kb_state;
    float       *btn_kb_coeff;
    time_t      *btn_kb_last;
    float       btn_kb_decell_coeff;
    SDL_Joystick *sdljoystick;

    if(gc == NULL)
	return;

    /* Get game controllers and general options flags */
    controllers = gc->controllers;
    options = gc->options;

    /* Get timings */
    gc_last_updated_ms = gc->last_updated;
    dms = (time_t)MAX(cur_ms - gc_last_updated_ms, 0);


    /* Check if joystick updating is needed and allowed */
    if((controllers & GCTL_CONTROLLER_JOYSTICK) &&
       (gc->joystick != NULL)
	)
    {
	SDL_JoystickUpdate();
	/* Iterate through each joystick */
	for(i = 0; i < gc->total_joysticks; i++)
	{
	    joystick = &gc->joystick[i];
	    if(joystick == NULL)
		continue;

	    sdljoystick = gc->sdljoystick[i];
	    if(sdljoystick == NULL)
		continue;

	    /* Update defined joystick operations for this joystick.
	     * This is to tabulate a list of functions that were
	     * handled/controlled by any of the joystick(s) so the
	     * function does not get updated again by the keyboard
	     * check farther below
	     */
	    if(joystick->axis_heading > -1)
		js_def_heading = True;
	    if(joystick->axis_pitch > -1)
		js_def_pitch = True;
	    if(joystick->axis_bank > -1)
		js_def_bank = True;
	    if(joystick->axis_hat_x > -1)
		js_def_hat_x = True;
	    if(joystick->axis_hat_y > -1)
		js_def_hat_y = True;
	    /* If ctrl state modifier is on then bank becomes heading */
	    if((joystick->button_rotate > -1) &&
	       gc->ctrl_state
		)
	    {
		if(js_def_bank)
		{
		    js_def_heading = True;
		    js_def_bank = False;
		}
	    }
	    if(joystick->button_zoom_in > -1)
		js_def_zoom_in = True;
	    if(joystick->button_zoom_out > -1)
		js_def_zoom_out = True;
	    if(joystick->button_air_brakes > -1)
		js_def_air_brakes = True;
	    if(joystick->button_wheel_brakes > -1 ||
	       joystick->axis_brake_left > -1 ||
	       joystick->axis_brake_right > -1)
		js_def_wheel_brakes = True;


	    /* Begin handling joystick */

	    int axis_heading = joystick->axis_heading,
		axis_bank = joystick->axis_bank,
		axis_pitch = joystick->axis_pitch,
		axis_throttle = joystick->axis_throttle,
		pov_hat = joystick->pov_hat,
		axis_hat_x = joystick->axis_hat_x,
		axis_hat_y = joystick->axis_hat_y,
		axis_brake_left = joystick->axis_brake_left,
		axis_brake_right = joystick->axis_brake_right,
		axes_inversion_bits = joystick->axes_inversion_bits;

	    /* Check bank to heading modifier */
	    if((joystick->button_rotate > -1) &&
	       gc->ctrl_state
		)
	    {
		/* Bank axis becomes the heading axis */
		if(axis_bank > -1)
		{
		    axis_heading = axis_bank;
		    axis_bank = -1;
		}
	    }

	    /* Heading */
	    if((axis_heading > -1) &&
	       !gc->axis_kb_state){
		gc->heading = ((float)SDL_JoystickGetAxis(sdljoystick, axis_heading) / 32768.0);

		if(axes_inversion_bits & GCTL_JS_AXIS_INV_HEADING)
		    gc->heading = -gc->heading;
	    }

	    /* Bank */
	    if((axis_bank > -1) &&
	       !gc->axis_kb_state){
		gc->bank = ((float)SDL_JoystickGetAxis(sdljoystick, axis_bank) / 32768.0);

		if(axes_inversion_bits & GCTL_JS_AXIS_INV_BANK)
		    gc->bank = -gc->bank;
	    }

	    /* Pitch */
	    if((axis_pitch > -1) &&
	       !gc->axis_kb_state){
		gc->pitch = - ((float)SDL_JoystickGetAxis(sdljoystick, axis_pitch) / 32768.0);

		if(axes_inversion_bits & GCTL_JS_AXIS_INV_PITCH)
		    gc->pitch = -gc->pitch;
	    }

	    /* Throttle */
	    if((axis_throttle > -1) &&
	       !gc->axis_kb_state){
		gc->throttle = 1.0 - (((float)SDL_JoystickGetAxis(sdljoystick, axis_throttle) + 32768.0) / 65536.0);

		if(axes_inversion_bits & GCTL_JS_AXIS_INV_THROTTLE)
		    gc->throttle = -gc->throttle + 1.0;
	    }

	    /* Wheel brakes: we do not make a distinction between left and right and handle both */
	    float brake_coeff_left = 0;
	    float brake_coeff_right = 0;
	    if((axis_brake_left > -1) && !gc->axis_kb_state)
	    {
		int axis_brake_left_val = SDL_JoystickGetAxis(sdljoystick, axis_brake_left);
		brake_coeff_left = 1.0 - ((float)axis_brake_left_val + 32768.0) / 65536.0;

		if(axes_inversion_bits & GCTL_JS_AXIS_INV_BRAKE_LEFT)
		    brake_coeff_left = -brake_coeff_left;
	    }

	    if((axis_brake_right > -1) && !gc->axis_kb_state)
	    {
		int axis_brake_right_val = SDL_JoystickGetAxis(sdljoystick, axis_brake_right);
		brake_coeff_right = 1.0 - ((float)axis_brake_right_val + 32768.0) / 65536.0;

		if(axes_inversion_bits & GCTL_JS_AXIS_INV_BRAKE_RIGHT)
		    brake_coeff_right = -brake_coeff_right;
	    }

	    if (axis_brake_left > -1 || axis_brake_right > -1)
	    {
		float brake_coeff = roundf(MAX(brake_coeff_left, brake_coeff_right) * 100) / 100;

		switch(gc->wheel_brakes_state){
		    case 0:
		    case 1:
			if(brake_coeff > 0)
			{
			    gc->wheel_brakes_state = 1;
			    gc->wheel_brakes_coeff = brake_coeff;
			} else {
			    gc->wheel_brakes_state = 0;
			    gc->wheel_brakes_coeff = 0;
			}

			if(brake_coeff >= 1.0 && gc->shift_state)
			    gc->wheel_brakes_state = 2;
			break;
		    case 2:
			if(!gc->shift_state && brake_coeff >= 0.01)
			    gc->wheel_brakes_state = 1;
			break;
		}
	    }


	    /* Point Of View */

	    /* POV as a standard hat (switches) */
	    if (pov_hat > -1)
	    {
		int pos = SDL_JoystickGetHat(sdljoystick, pov_hat);
		switch (pos) {
		    case SDL_HAT_CENTERED:
			gc->hat_y= 0.0f;
			gc->hat_x= 0.0f;
			break;
		    case SDL_HAT_UP:
			gc->hat_y= 1.0f;
			gc->hat_x= 0.0f;
			break;
		    case SDL_HAT_RIGHT:
			gc->hat_y= 0.0f;
			gc->hat_x= 1.0f;
			break;
		    case SDL_HAT_DOWN:
			gc->hat_y= -1.0f;
			gc->hat_x= 0.0f;
			break;
		    case SDL_HAT_LEFT:
			gc->hat_y= 0.0f;
			gc->hat_x= -1.0f;
			break;
		    case SDL_HAT_RIGHTUP:
			gc->hat_y= 1.0f;
			gc->hat_x= 1.0f;
			break;
		    case SDL_HAT_RIGHTDOWN:
			gc->hat_y= -1.0f;
			gc->hat_x= 1.0f;
			break;
		    case SDL_HAT_LEFTUP:
			gc->hat_y= 1.0f;
			gc->hat_x= -1.0f;
			break;
		    case SDL_HAT_LEFTDOWN:
			gc->hat_y= -1.0f;
			gc->hat_x= -1.0f;
			break;
		    default:
			gc->hat_y= 0.0f;
			gc->hat_x= 0.0f;
			break;
		}
		if(axes_inversion_bits & GCTL_JS_AXIS_INV_POV_HAT){
		    gc->hat_x = -gc->hat_x;
		    gc->hat_y = -gc->hat_y;
		}
	    }
	    /* POV as 2 analog axes */
	    else if ((axis_hat_x > -1) && (axis_hat_y > -1))
	    {
		gc->hat_x = (float)SDL_JoystickGetAxis(sdljoystick, axis_hat_x) / 32768.0;
		gc->hat_y = - ((float)SDL_JoystickGetAxis(sdljoystick, axis_hat_y) / 32768.0);

		/* Add a +/-1% dead band to avoid POV drift */
		if(gc->hat_x < 0.01f && gc->hat_x > -0.01f)
		    gc->hat_x = 0.0f;
		if(gc->hat_y < 0.01f && gc->hat_y > -0.01f)
		    gc->hat_y = 0.0f;

		if(axes_inversion_bits & GCTL_JS_AXIS_INV_HAT_X){
		    gc->hat_x = -gc->hat_x;
		}
		if(axes_inversion_bits & GCTL_JS_AXIS_INV_HAT_Y){
		    gc->hat_y = -gc->hat_y;
		}
	    }


	    /* Handle buttons */



	    /* Zoom */
	    if (joystick->button_zoom_in > -1 && !gc->button_kb_state)
	    {
		gc->zoom_in_state = SDL_JoystickGetButton(
		    sdljoystick,
		    joystick->button_zoom_in) ? True : False;
		gc->zoom_in_coeff = (float)((gc->zoom_in_state) ? 1.0 : 0.0);
	    }

	    if (joystick->button_zoom_out > -1 && !gc->button_kb_state)
	    {
		gc->zoom_out_state = SDL_JoystickGetButton(
		    sdljoystick,
		    joystick->button_zoom_out) ? True : False;
		gc->zoom_out_coeff = (float)((gc->zoom_out_state) ? 1.0 : 0.0);
	    }

	    /*Hoist*/
	    if (joystick->button_hoist_up > -1 && !gc->button_kb_state)
	    {
		gc->hoist_up_state = SDL_JoystickGetButton(
		    sdljoystick,
		    joystick->button_hoist_up) ? True : False;
		gc->hoist_up_coeff = (float)((gc->hoist_up_state) ? 1.0 : 0.0);
	    }

	    if (joystick->button_hoist_down > -1 && !gc->button_kb_state){
		gc->hoist_down_state = SDL_JoystickGetButton(
		    sdljoystick,
		    joystick->button_hoist_down) ? True : False;
		gc->hoist_down_coeff = (float)((gc->hoist_down_state) ? 1.0 : 0.0);
	    }
	    /* Update ctrl modifier state (for switching
	     * bank axis to act as heading axis), but update
	     * this modifier only if the button mapped to it
	     * has been defined
	     */
	    if(joystick->button_rotate > -1 && !gc->button_kb_state)
		gc->ctrl_state = (SDL_JoystickGetButton(sdljoystick, joystick->button_rotate)) ? True : False;

	    /* Air brakes */
	    if (joystick->button_air_brakes > -1 && !gc->button_kb_state)
	    {
		/* As air brakes are toggled on/off, do a rising edge detection
		 * to avoid state jiggling.
		 */

		/* Joystick button pressed? */
		if ( SDL_JoystickGetButton(
			 sdljoystick,
			 joystick->button_air_brakes)
		    )
		{
		    /* Joystick button was previously released? */
		    if(gc->air_brakes_js_btn_released){

			/* Button is pressed and was previously released,
			 * we can toggle state.
			 */

			gc->air_brakes_state = gc->air_brakes_state ? False : True;
			gc->air_brakes_coeff = gc->air_brakes_state ? 1.0f : 0.0f;

			/* Joystick button is now pressed */
			gc->air_brakes_js_btn_released = False;
		    }
		}
		/* Joystick button not pressed */
		else
		    gc->air_brakes_js_btn_released = True;
	    }

	    /* Wheel brakes - only if not controlled by axis and enabled already */
	    if (joystick->button_wheel_brakes > -1 && !gc->button_kb_state)
	    {
		int wheel_brake_btn_pressed = SDL_JoystickGetButton(
		    sdljoystick,
		    joystick->button_wheel_brakes) ? True : False;
		gc->wheel_brakes_coeff = gc->wheel_brakes_state ? 1.0f : 0.0f;

		/* Wheel brakes states:
		 * 0 : wheel brakes OFF
		 * 1 : wheel brakes ON
		 * 2 : parking brake ON
		 *
		 * Push button to apply wheel brakes, release button to
		 * release wheel brakes.
		 * Push button and press <shift> key then release button
		 * 	before release <shift> key to activate parking brake.
		 * When in parking brake state, push button to release brakes.
		 */
		switch(gc->wheel_brakes_state){
		    case 0:
			if (wheel_brake_btn_pressed && !gc->shift_state)
			    gc->wheel_brakes_state = 1;
			else if (wheel_brake_btn_pressed && gc->shift_state)
			    gc->wheel_brakes_state = 2;
			break;
		    case 1:
			if (!wheel_brake_btn_pressed && !gc->shift_state)
			    gc->wheel_brakes_state = 0;
			else if (wheel_brake_btn_pressed && gc->shift_state)
			    gc->wheel_brakes_state = 2;
			break;
		    case 2:
			if (wheel_brake_btn_pressed && !gc->shift_state)
			    gc->wheel_brakes_state = 0;
			break;
		}
	    }
	}	/* Iterate through each joystick */
    }	/* Check if joystick updating is needed and allowed */


	/* Check if keyboard updating is needed and allowed
	 *
	 * Handle keyboard only if pointer or joystick are not present
	 */
    if(controllers & GCTL_CONTROLLER_KEYBOARD)
    {

/* Reduces the btn_kb_coeff towards 0.0 if btn_kb_state is false.
 * The bigger btn_kb_decell_coeff is, the faster btn_kb_coeff reaches
 * 0.0.
 */
#define DO_KB_BTN_UPDATE                                                \
	{                                                               \
	    if(!(*btn_kb_state))					\
	    {                                                           \
		if(*btn_kb_coeff > 0.0f)				\
		    *btn_kb_coeff = (float)MAX(				\
			(*btn_kb_coeff) -                               \
			(btn_kb_decell_coeff * time_compensation),      \
			0.0                                             \
			);                                              \
		else							\
		    *btn_kb_coeff = (float)MIN(				\
			(*btn_kb_coeff) +                               \
			(btn_kb_decell_coeff * time_compensation),      \
			0.0                                             \
			);                                              \
	    }                                                           \
	}

	/* Begin reducing coefficients for keyboard key states.
	 *
	 * Some key states will be skipped if their corresponding
	 * functions were already handled by the joystick.
	 */
	/* Heading */
	if(!js_def_heading || gc->axis_kb_state)
	{
	    btn_kb_decell_coeff = 3.0f;
	    btn_kb_state = &gc->heading_kb_state;
	    btn_kb_last = &gc->heading_kb_last;
	    btn_kb_coeff = &gc->heading;
	    DO_KB_BTN_UPDATE
		}
	/* Pitch */
	if(!js_def_pitch || gc->axis_kb_state)
	{
	    btn_kb_decell_coeff = 3.0f;
	    btn_kb_state = &gc->pitch_kb_state;
	    btn_kb_last = &gc->pitch_kb_last;
	    btn_kb_coeff = &gc->pitch;
	    DO_KB_BTN_UPDATE
		}
	/* Bank */
	if(!js_def_bank || gc->axis_kb_state)
	{
	    /* Bank */
	    btn_kb_decell_coeff = 3.0f;
	    btn_kb_state = &gc->bank_kb_state;
	    btn_kb_last = &gc->bank_kb_last;
	    btn_kb_coeff = &gc->bank;
	    DO_KB_BTN_UPDATE
		}

	/* Leave throttle alone */

	/* Hat X */
	if(!js_def_hat_x || gc->axis_kb_state)
	{
	    btn_kb_decell_coeff = 2.0f;
	    btn_kb_state = &gc->hat_x_kb_state;
	    btn_kb_last = &gc->hat_x_kb_last;
	    btn_kb_coeff = &gc->hat_x;
	    DO_KB_BTN_UPDATE
		}
	/* Hat Y */
	if(!js_def_hat_y || gc->axis_kb_state)
	{
	    btn_kb_decell_coeff = 2.0f;
	    btn_kb_state = &gc->hat_y_kb_state;
	    btn_kb_last = &gc->hat_y_kb_last;
	    btn_kb_coeff = &gc->hat_y;
	    DO_KB_BTN_UPDATE
		}

	/* Wheel brakes */
	if(!js_def_wheel_brakes || gc->button_kb_state)
	{
	    Boolean b = (Boolean)gc->wheel_brakes_state;
	    btn_kb_decell_coeff = 0.5f;
	    btn_kb_state = &b;
	    btn_kb_last = &gc->wheel_brakes_kb_last;
	    btn_kb_coeff = &gc->wheel_brakes_coeff;
	    DO_KB_BTN_UPDATE
		gc->wheel_brakes_state = (int)*btn_kb_state;
	}
	/* Air brakes */
	if(!js_def_air_brakes || gc->button_kb_state)
	{
	    /* Leave air brakes alone */
	}

	/* Zoom in */
	if(!js_def_zoom_in || gc->button_kb_state)
	{
	    btn_kb_decell_coeff = 1.6f;
	    btn_kb_state = &gc->zoom_in_state;
	    btn_kb_last = &gc->zoom_in_kb_last;
	    btn_kb_coeff = &gc->zoom_in_coeff;
	    DO_KB_BTN_UPDATE
		}
	/* Zoom out */
	if(!js_def_zoom_out || gc->button_kb_state)
	{
	    btn_kb_decell_coeff = 1.6f;
	    btn_kb_state = &gc->zoom_out_state;
	    btn_kb_last = &gc->zoom_out_kb_last;
	    btn_kb_coeff = &gc->zoom_out_coeff;
	    DO_KB_BTN_UPDATE
		}

#undef DO_KB_BTN_UPDATE
    }	/* Check if keyboard updating is needed and allowed */


	/* Check if pointer updating is needed and allowed */
    if(controllers & GCTL_CONTROLLER_POINTER)
    {



    }

    /* Update last gc time updated in ms to the current time */
    gc->last_updated = cur_ms;
}

/*
 *	Handles a pointer event with respect to the given game
 *	controller.
 */
void GCtlHandlePointer(
    gw_display_struct *display, gctl_struct *gc,
    gw_event_type type,
    int button,		/* Button number */
    int x, int y,
    time_t t,		/* Time stamp (in ms) */
    time_t lapsed_ms
    )
{
    if(gc == NULL)
	return;

    if(!(gc->controllers & GCTL_CONTROLLER_POINTER))
	return;

/* TODO */

}


/*
 *	Handles the key, where state indicates if it is pressed or
 *	released.
 */
void GCtlHandleKey(
    gw_display_struct *display, gctl_struct *gc,
    int k, Boolean state,	/* Key value and state */
    Boolean alt_state,	/* Modifier key states */
    Boolean ctrl_state,
    Boolean shift_state,
    time_t t,		/* Time stamp (in ms) */
    time_t lapsed_ms
    )
{
    Boolean	*btn_kb_state;
    float	*btn_kb_coeff;
    time_t	*btn_kb_last;
    float	btn_kb_to_coeff;


    if(gc == NULL)
	return;

    if(!(gc->controllers & GCTL_CONTROLLER_KEYBOARD))
	return;

    /* Alt key state */
    gc->alt_state = (alt_state) ? True : False;
    gc->ctrl_state = (ctrl_state) ? True : False;
    gc->shift_state = (shift_state) ? True : False;


/* Macro to switch off keyboard repeats if the key is held down */
#define DO_HAS_NO_AUTOREPEAT                            \
    GWKeyboardAutoRepeat(display, (Boolean)!state);

/* Update the keyboard key based on the value of the pointers to
 * three variables representing the key's state, coefficient, and
 * last event time (btn_kb_state, btn_kb_coeff, and btn_kb_last
 * respectivly).
 */
#define DO_BTN_KB_UPDATE                                                \
    if(state != (*btn_kb_state))					\
    {                                                                   \
	time_t dt = MAX(t - (*btn_kb_last), 0);                         \
	if(state)							\
	{								\
	    /* Pressed down */						\
	    *btn_kb_state = True;                                       \
	    *btn_kb_coeff = 1.0f;                                       \
	    *btn_kb_last = t;						\
	}								\
	else if(dt > lapsed_ms)                                         \
	{								\
	    /* Released up */						\
	    *btn_kb_state = False;					\
	    *btn_kb_coeff = 0.0f;                                       \
	    *btn_kb_last = t;						\
	}								\
	else								\
	{								\
	    /* Released up quickly from a prior press which occured	\
	     * during this cycle.                                       \
	     */								\
	    *btn_kb_state = False;					\
	    *btn_kb_coeff = (lapsed_ms > 0) ?				\
		((float)dt / (float)lapsed_ms) : 0.0f;			\
	    *btn_kb_last = t;						\
	}								\
    }

/* Update the keyboard key for an axis based on the value of the
 * pointers to three variables representing the key's state,
 * coefficient, and last event time (btn_kb_state, btn_kb_coeff,
 * and btn_kb_last respectivly).
 */
#define DO_BTN_KB_AXIS_UPDATE                                           \
    if(state != (*btn_kb_state))					\
    {                                                                   \
	time_t dt = MAX(t - (*btn_kb_last), 0);                         \
	if(state)							\
	{								\
	    /* Pressed down */						\
	    *btn_kb_state = True;                                       \
	    *btn_kb_coeff = btn_kb_to_coeff;				\
	    *btn_kb_last = t;						\
	}								\
	else if(dt > lapsed_ms)                                         \
	{								\
	    /* Released up */						\
	    *btn_kb_state = False;					\
	    *btn_kb_coeff = 0.0f;                                       \
	    *btn_kb_last = t;						\
	}								\
	else								\
	{								\
	    /* Released up quickly from a prior press which occured	\
	     * during this cycle.                                       \
	     */								\
	    *btn_kb_state = False;					\
	    *btn_kb_coeff = btn_kb_to_coeff * ((lapsed_ms > 0) ?	\
					       ((float)dt / (float)lapsed_ms) : 0.0f); \
	    *btn_kb_last = t;						\
	}								\
    }


    switch(k)
    {
	case GWKeyLeft:
	    DO_HAS_NO_AUTOREPEAT
		btn_kb_to_coeff = -1.0f;
	    if(gc->ctrl_state)
	    {
		btn_kb_state = &gc->heading_kb_state;
		btn_kb_coeff = &gc->heading;
		btn_kb_last = &gc->heading_kb_last;
	    }
	    else if(gc->shift_state)
	    {
		btn_kb_state = &gc->hat_x_kb_state;
		btn_kb_coeff = &gc->hat_x;
		btn_kb_last = &gc->hat_x_kb_last;
	    }
	    else
	    {
		btn_kb_state = &gc->bank_kb_state;
		btn_kb_coeff = &gc->bank;
		btn_kb_last = &gc->bank_kb_last;
	    }
	    DO_BTN_KB_AXIS_UPDATE
		if(!state)
		{
		    gc->heading_kb_state = False;
		    gc->hat_x_kb_state = False;
		    gc->bank_kb_state = False;
		}
	    gc->axis_kb_state = state;
	    break;

	case GWKeyRight:
	    DO_HAS_NO_AUTOREPEAT
		btn_kb_to_coeff = 1.0f;
	    if(gc->ctrl_state)
	    {
		btn_kb_state = &gc->heading_kb_state;
		btn_kb_coeff = &gc->heading;
		btn_kb_last = &gc->heading_kb_last;
	    }
	    else if(gc->shift_state)
	    {
		btn_kb_state = &gc->hat_x_kb_state;
		btn_kb_coeff = &gc->hat_x;
		btn_kb_last = &gc->hat_x_kb_last;
	    }
	    else
	    {
		btn_kb_state = &gc->bank_kb_state;
		btn_kb_coeff = &gc->bank;
		btn_kb_last = &gc->bank_kb_last;
	    }
	    DO_BTN_KB_AXIS_UPDATE
		if(!state)
		{
		    gc->heading_kb_state = False;
		    gc->hat_x_kb_state = False;
		    gc->bank_kb_state = False;
		}
	    gc->axis_kb_state = state;
	    break;

	case GWKeyUp:
	    DO_HAS_NO_AUTOREPEAT
		btn_kb_to_coeff = 1.0f;
	    if(gc->shift_state)
	    {
		btn_kb_state = &gc->hat_y_kb_state;
		btn_kb_coeff = &gc->hat_y;
		btn_kb_last = &gc->hat_y_kb_last;
	    }
	    else
	    {
		btn_kb_state = &gc->pitch_kb_state;
		btn_kb_coeff = &gc->pitch;
		btn_kb_last = &gc->pitch_kb_last;
	    }
	    DO_BTN_KB_AXIS_UPDATE
		if(!state)
		{
		    gc->pitch_kb_state = False;
		    gc->hat_y_kb_state = False;
		}
	    gc->axis_kb_state = state;
	    break;

	case GWKeyDown:
	    DO_HAS_NO_AUTOREPEAT
		btn_kb_to_coeff = -1.0f;
	    if(gc->shift_state)
	    {
		btn_kb_state = &gc->hat_y_kb_state;
		btn_kb_coeff = &gc->hat_y;
		btn_kb_last = &gc->hat_y_kb_last;
	    }
	    else
	    {
		btn_kb_state = &gc->pitch_kb_state;
		btn_kb_coeff = &gc->pitch;
		btn_kb_last = &gc->pitch_kb_last;
	    }
	    DO_BTN_KB_AXIS_UPDATE
		if(!state)
		{
		    gc->pitch_kb_state = False;
		    gc->hat_y_kb_state = False;
		}
	    gc->axis_kb_state = state;
	    break;

	    /* Throttle */
	case GWKeyPageUp:
	    if(state)
	    {
		gc->throttle += (float)((gc->shift_state) ? 0.1 : 0.01);
		if(gc->throttle > 1.0)
		    gc->throttle = 1.0f;
	    }
	    gc->axis_kb_state = state;
	    break;
	case GWKeyPageDown:
	    if(state)
	    {
		gc->throttle -= (float)((gc->shift_state) ? 0.1 : 0.01);
		if(gc->throttle < 0.0)
		    gc->throttle = 0.0f;
	    }
	    gc->axis_kb_state = state;
	    break;

	    /* adding some more throttle short-cuts here. -- Jesse */
	case '1': case '2': case '3':
	case '4': case '5': case '6':
	case '7': case '8': case '9':
	    gc->throttle = (float) ( ( k - '0') / 10.0);
	    break;
	case '0':
	    gc->throttle = 1.0;
	    break;

	    /* Wheel brakes */
	case GWKeyDelete: case '.': case '>':
	    DO_HAS_NO_AUTOREPEAT
		if(True)
		{
		    Boolean b = (Boolean)gc->wheel_brakes_state;
		    btn_kb_state = &b;
		    btn_kb_coeff = &gc->wheel_brakes_coeff;
		    btn_kb_last = &gc->wheel_brakes_kb_last;
		    DO_BTN_KB_UPDATE
		    gc->wheel_brakes_state = (int)*btn_kb_state;
		}
	    gc->button_kb_state = state;
	    break;

	    /* Air brakes */
	case 'b':
	    if(state)
	    {
		/* Air brakes are toggled on/off */
		if(gc->air_brakes_state)
		{
		    gc->air_brakes_state = False;
		    gc->air_brakes_coeff = 0.0f;
		}
		else
		{
		    gc->air_brakes_state = True;
		    gc->air_brakes_coeff = 1.0f;
		}
	    }
	    gc->button_kb_state = state;
	    break;

	    /* Zoom in */
	case '=': case '+':
	    DO_HAS_NO_AUTOREPEAT
		btn_kb_state = &gc->zoom_in_state;
	    btn_kb_coeff = &gc->zoom_in_coeff;
	    btn_kb_last = &gc->zoom_in_kb_last;
	    DO_BTN_KB_UPDATE
		gc->button_kb_state = state;
	    break;

	    /* Zoom out */
	case '-': case '_':
	    DO_HAS_NO_AUTOREPEAT
		btn_kb_state = &gc->zoom_out_state;
	    btn_kb_coeff = &gc->zoom_out_coeff;
	    btn_kb_last = &gc->zoom_out_kb_last;
	    DO_BTN_KB_UPDATE
		gc->button_kb_state = state;
	    break;

    }

#undef DO_BTN_KB_UPDATE
#undef DO_BTN_KB_AXIS_UPDATE
#undef DO_HAS_NO_AUTOREPEAT
}


/*
 *	Resets all toggle state dependent values to defaults on the
 *	given game controller structure.
 */
void GCtlResetValues(gctl_struct *gc)
{
    if(gc == NULL)
	return;

    /* Air brakes are toggled, so its value needs to be reset */
    gc->air_brakes_state = False;
    gc->air_brakes_kb_last = 0;
    gc->air_brakes_coeff = 0.0f;


}

/*
 *	Resets all timmer values on the given game controller structure.
 */
void GCtlResetTimmers(gctl_struct *gc)
{
    if(gc == NULL)
	return;

    gc->last_updated = 0;

    gc->heading_kb_last = 0;
    gc->pitch_kb_last = 0;
    gc->bank_kb_last = 0;
    gc->throttle_kb_last = 0;
    gc->hat_x_kb_last = 0;
    gc->hat_y_kb_last = 0;

    gc->zoom_in_kb_last = 0;
    gc->zoom_out_kb_last = 0;

    gc->hoist_up_kb_last = 0;
    gc->hoist_down_kb_last = 0;

    gc->air_brakes_kb_last = 0;
    gc->wheel_brakes_kb_last = 0;
}

/*
 *	Shuts down the Game Controller.
 */
void GCtlDelete(gctl_struct *gc)
{
    last_gctl_error = NULL;

    if(gc == NULL)
	return;

    /* Joystick(s) initialized? */
    if(gc->joystick != NULL)
    {
	/* Delete joysticks */
	free(gc->joystick);
	gc->joystick = NULL;
	gc->total_joysticks = 0;
    }

    free(gc);
}
