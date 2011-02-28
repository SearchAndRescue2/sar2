#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
//#define SDLJS
#ifdef SDLJS
#include <SDL/SDL.h>
#endif
#ifdef JS_SUPPORT
# include <jsw.h>
#endif
#include "gw.h"		/* Need to know about GW key codes */
#include "gctl.h"


static char *last_gctl_error = NULL;


char *GCtlGetError(void);
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
 *	Initializes the Game Controller with the specified values.
 */
gctl_struct *GCtlNew(gctl_values_struct *v)
{
	int i;
#if defined(JSW_H)
        int status = 0;
#endif
	gctl_js_struct *joystick;
	gctl_values_js_struct *js_value;
	gctl_struct *gc = GCTL(calloc(1, sizeof(gctl_struct)));

	last_gctl_error = NULL;

	if(gc == NULL)
	{
	    last_gctl_error = "Memory allocation error";
	    return(NULL);
	}

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

	/* Begin initializing */

 #ifdef SDLJS
   /* Attempt to initialize the joystick subsystem. */
   if (SDL_InitSubSystem(SDL_INIT_JOYSTICK) != 0)  {
     fprintf(stderr, "Unable to initialize SDL joystick: %s\n", SDL_GetError());
     gc->sdljoystick = NULL;
   } else {
     if (SDL_NumJoysticks() > 0) {
       fprintf(stderr, "At least one SDL joystick available.\n");
       //SDL_JoystickEventState(SDL_ENABLE);
       gc->sdljoystick = SDL_JoystickOpen(0);
       gc->sdlnumaxes = SDL_JoystickNumAxes(gc->sdljoystick);
     } else {
       fprintf(stderr, "No SDL joysticks available.\n");
       gc->sdljoystick = NULL;
     }
   }
 #endif

	/* Initialize the joystick? */
	if(v->controllers & GCTL_CONTROLLER_JOYSTICK)
	{
	    const char *device;
	    gctl_js_axis_roles axis_role;

	    /* Allocate Game Controller Joysticks */
	    gc->total_joysticks = v->total_joysticks;
	    gc->joystick = GCTL_JS(calloc(
		gc->total_joysticks, sizeof(gctl_js_struct)
	    ));
	    if(gc->joystick == NULL)
	    {
		gc->total_joysticks = 0;
	    }

	    /* Iterate through each joystick */
	    for(i = 0; i < gc->total_joysticks; i++)
	    {
		/* Get pointer to current Game Controller Joystick and
		 * Joystick Values
		 */
		joystick = &gc->joystick[i];
		js_value = &v->joystick[i];

		device = js_value->device;
		axis_role = js_value->axis_role;

#if defined(JSW_H)
		if(!STRISEMPTY(device))
		{
		    js_data_struct *jsd;

		    /* Allocate libjsw joystick data */
		    joystick->data = jsd = (js_data_struct *)calloc(
			1, sizeof(js_data_struct)
		    );
		    if(jsd != NULL)
		    {
			/* Initialize libjsw joystick */
			int jsstatus = JSInit(
			    jsd,		/* Joystick Data */
			    device,		/* Device */
			    NULL,		/* Alternate calibration file */
			    JSFlagNonBlocking	/* Options */
			);
			if(jsstatus != JSSuccess)
			{
			    /* Failed to initialize joystick */

			    /* Try to close and delete the libjsw
			     * joystick data
			     */
			    JSClose(jsd);
			    free(jsd);
			    joystick->data = jsd = NULL;

			    /* Set error status and message */
			    status = -1;
			    switch(jsstatus)
			    {
			      case JSError:
				last_gctl_error =
"Error encountered while attempting to initialize joystick";
				break;
			      case JSBadValue:
				last_gctl_error =
"Bad value encountered while attempting to initialize joystick";
			        break;
			      case JSNoAccess:
				last_gctl_error =
"Joystick does not exist or insufficient permission to access it";
				break;
			      case JSNoBuffers:
				last_gctl_error =
"Memory allocation error";
				break;
			      default:
				last_gctl_error =
"libjsw reported an unknown error while initlaizing joystick";
				break;
			    }
			}
			else
			{
			    /* Initialized joystick successfully */
#if defined(__MSW__)
			    /* Windows version of libjsw requires that
			     * we set the cooperation level with respect
			     * to the toplevel window
			     */
			    switch(js_value->priority)
			    {
			      case GCTL_JS_PRIORITY_PREEMPT:
				JSSetCooperativeLevelWin32(jsd, 2, js_value->window);
				break;
			      case GCTL_JS_PRIORITY_FOREGROUND:
				JSSetCooperativeLevelWin32(jsd, 1, js_value->window);
				break;
			      default:	/* GCTL_JS_PRIORITY_BACKGROUND */
				JSSetCooperativeLevelWin32(jsd, 0, js_value->window);
				break;
			    }
#endif	/* __MSW__ */
			}
		    }
		    else
		    {
			last_gctl_error = "Memory allocation error";
			status = -1;
		    }
		}
#else
		last_gctl_error =
"Unable to initialize joystick(s): Joystick support not compiled";
#endif


		/* Begin setting up joystick axis mappings */

		/* Reset all axis mappings first */
		joystick->axis_heading = -1;
		joystick->axis_bank = -1;
		joystick->axis_pitch = -1;
		joystick->axis_throttle = -1;
		joystick->axis_hat_x = -1;
		joystick->axis_hat_y = -1;

		/* Throttle and rudder unit? */
		if(axis_role & GCTL_JS_AXIS_ROLE_AS_THROTTLE_AND_RUDDER)
		{
		    joystick->axis_heading = 0;
		    joystick->axis_throttle = 1;
		}
		/* Joystick with no axises (only buttons) */
		else if(axis_role & GCTL_JS_AXIS_ROLE_NONE)
		{

		}
		/* Joystick as a standard composite unit */
		else
		{
		    /* Get bank axis */
		    if(axis_role & GCTL_JS_AXIS_ROLE_BANK)
			joystick->axis_bank = 0;
		    /* Get heading axis */
		    if(axis_role & GCTL_JS_AXIS_ROLE_HEADING)
			joystick->axis_heading = 2;
		    /* Get pitch axis */
		    if(axis_role & GCTL_JS_AXIS_ROLE_PITCH)
			joystick->axis_pitch = 1;

		    /* Get throttle axis */
		    if(axis_role & GCTL_JS_AXIS_ROLE_THROTTLE)
		    {
			/* Check if heading axis is valid, if not then
			 * throttle is axis 2
			 */
			if(joystick->axis_heading < 0)
			    joystick->axis_throttle = 2;
			else
			    joystick->axis_throttle = 3;
		    }

		    /* Get hat x and y axis */
		    if(axis_role & GCTL_JS_AXIS_ROLE_HAT)
		    {
			/* Hat x */
			if(joystick->axis_heading < 0)
			{
			    if(joystick->axis_throttle < 0)
				joystick->axis_hat_x = 2;
			    else
				joystick->axis_hat_x = 3;
			}
			else if(joystick->axis_throttle < 0)
			{
			    joystick->axis_hat_x = 3;
			}
			else
			{
			    joystick->axis_hat_x = 4;
			}
			/* Hat y (the axis after hat x) */
			joystick->axis_hat_y =
			    joystick->axis_hat_x + 1;
		    }
		}	/* Joystick as a standard composite unit */


		/* Begin setting up joystick button mappings */

		/* Copy joystick button mappings from js_value structure */
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
	}	/* Initialize keyboard? */

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
	}	/* Initialize pointer? */


	return(gc);
}

/*
 *	Updates the game controller structure with new (current)
 *	controller values.
 *
 *	Keyboard handling is performed in GCtlHandleKey().
 *
 *	Pointer handling is performed in GCtlHandlePointer().
 */
void GCtlUpdate(
	gctl_struct *gc,
	Boolean heading_has_nz,	/* Include heading pitch nullzone */
	Boolean pitch_has_nz,	/* Include pitch nullzone */
	Boolean bank_has_nz,	/* Include bank nullzone */
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
	Boolean	*btn_kb_state;
	float	*btn_kb_coeff;
	time_t	*btn_kb_last;
	float	btn_kb_decell_coeff;

	if(gc == NULL)
	    return;

	/* Get game controllers and general options flags */
	controllers = gc->controllers;
	options = gc->options;

	/* Get timings */
	gc_last_updated_ms = gc->last_updated;
	dms = (time_t)MAX(cur_ms - gc_last_updated_ms, 0);

       /* SDL joystick? */
 #ifdef SDLJS
       if (gc->sdljoystick) {
         // So we've got an SDL joystick structure.
         // For now, we treat
         //  Axis 0: -ve bank left, +ve bank right
         //  Axis 1: -ve pitch nose down, +ve pitch nose up
         //  Axis 2: -ve throttle 0%, +ve throttle 100%
         SDL_JoystickUpdate();
         if (gc->sdlnumaxes >= 1) {
           // Bank.
           gc->bank = ((float)SDL_JoystickGetAxis(gc->sdljoystick, 0) / 32768.0);
           js_def_bank = True;
         }
         if (gc->sdlnumaxes >= 2) {
           // Pitch.
           gc->pitch = - ((float)SDL_JoystickGetAxis(gc->sdljoystick, 1) / 32768.0);
           js_def_pitch = True;
         }
         if (gc->sdlnumaxes >= 3) {
           // Throttle.
           gc->throttle = (((float)SDL_JoystickGetAxis(gc->sdljoystick, 2) + 32768.0) / 65536.0);
         }
       }
 #endif



	/* Check if joystick updating is needed and allowed */
	if((controllers & GCTL_CONTROLLER_JOYSTICK) &&
	   (gc->joystick != NULL)
	)
	{
#ifdef JSW_H
	    js_data_struct *jsd;
#endif  /* JSW_H */

	    /* Iterate through each joystick */
	    for(i = 0; i < gc->total_joysticks; i++)
	    {
		joystick = &gc->joystick[i];

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
		if(joystick->button_wheel_brakes > -1)
		    js_def_wheel_brakes = True;


		/* Begin handling joystick */
#if defined(JSW_H)
		jsd = (js_data_struct *)joystick->data;
		if(jsd != NULL)
		{
		    int status;
		    int axis_num, button_num;
		    int	axis_heading = joystick->axis_heading,
			axis_bank = joystick->axis_bank,
			axis_pitch = joystick->axis_pitch,
			axis_throttle = joystick->axis_throttle,
			axis_hat_x = joystick->axis_hat_x,
			axis_hat_y = joystick->axis_hat_y;
		    js_axis_struct *axis_ptr;
		    js_button_struct *button_ptr;

/* Is hat value x centered at center y (both in raw units)? */
#define HAT_IS_CEN_RAW(v,c)	((v) == (c))
/* Is hat center at coefficient 0.0? */
#define HAT_IS_CEN_COEFF(v)	((v) == 0.0f)

/* Calculates the coefficient of the current time (c) minus the previous (l) time
 * divided by delta time (d). If (d) is not positive then 0.0 is returned.
 */
#define DELTA_TIME_TO_COEFF(c,l,d)	(((d) > 0) ? \
	(float)CLIP((float)((c) - (l)) / (float)(d), 0.0, 1.0) : 0.0 \
)

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

		    /* Get next event from joystick if any */
		    status = JSUpdate(jsd);
		    if(status == JSGotEvent)
		    {
			/* Got event, begin fetching values for each axis,
			 * any axis number that is -1 will be ignored
			 */
			/* Heading */
			if((axis_heading > -1) &&
			   !gc->axis_kb_state
			)
			    gc->heading = (float)((heading_has_nz) ?
						  (JSGetAxisCoeffNZ(jsd, axis_heading)*2000000/7800) -1 :
						  (JSGetAxisCoeff(jsd, axis_heading)*2000000/7800) -1
			    );
			//printf("Heading: %f\n", gc->heading);
			/* Bank */
			if((axis_bank > -1) &&
			   !gc->axis_kb_state
			)
			    gc->bank = (float)((bank_has_nz) ?
					       (JSGetAxisCoeffNZ(jsd, axis_bank)*2000000/7800) -1 :
					       (JSGetAxisCoeff(jsd, axis_bank)*2000000/7800) -1
			    );
			
			//printf("Bank: %f\n", gc->bank);
			/* Pitch */
			if((axis_pitch > -1) &&
			   !gc->axis_kb_state
			)
			    gc->pitch = (float)((pitch_has_nz) ?
						(-JSGetAxisCoeffNZ(jsd, axis_pitch)*2000000/7800) + 1 :
						(-JSGetAxisCoeff(jsd, axis_pitch)*2000000/7800) + 1			    
			    );
			//printf("Pitch: %f\n", gc->pitch);
			
			/* Throttle */
			if((axis_throttle > -1) &&
			   !gc->axis_kb_state
			)
			{
			    gc->throttle = (float)(
				1 - ((JSGetAxisCoeff(jsd, axis_throttle)*1000000 / 4000) / 2) 
			    );
//!!!!!!!!!!!!!!!
			    //printf("Throttle: %f\n", gc->throttle);
			    if(gc->throttle > 1.0f)
				gc->throttle = 1.0f;
			    else if(gc->throttle < 0.0f)
				gc->throttle = 0.0f;
			}
			/* Hat x */
			if((axis_hat_x > -1) &&
			   !gc->axis_kb_state
			)
			{
			    axis_num = axis_hat_x;
			    if(JSIsAxisAllocated(jsd, axis_num))
			    {
			    	axis_ptr = jsd->axis[axis_num];
				gc->hat_x = (float)JSGetAxisCoeff(jsd, axis_num)*1000000/30.518509;
				//printf("Hat_x: %f\n", gc->hat_x);
			    }								
			    /* if(JSIsAxisAllocated(jsd, axis_num)) */
			    /* { */
			    /* 	axis_ptr = jsd->axis[axis_num]; */
			    /* 	if(HAT_IS_CEN_RAW(axis_ptr->cur, axis_ptr->cen) && */
			    /* 	   !HAT_IS_CEN_RAW(axis_ptr->prev, axis_ptr->cen) && */
			    /* 	   HAT_IS_CEN_COEFF(gc->hat_x) */
			    /* 	) */
			    /* 	{ */
			    /* 	    /\* Pressed and released during busy loop *\/ */
			    /* 	    gc->hat_x = (float)DELTA_TIME_TO_COEFF( */
			    /* 		axis_ptr->time, axis_ptr->last_time, dms */
			    /* 	    ) * (float)( */
			    /* 		(axis_ptr->prev > axis_ptr->cen) ? 1.0 : -1.0 */
			    /* 	    ); */
			    /* 	    if(axis_ptr->flags & JSAxisFlagFlipped) */
			    /* 		gc->hat_x *= -1.0f; */
			    /* 	} */
			    /* 	else */
			    /* 	{ */
			    /* 	    gc->hat_x = (float)JSGetAxisCoeffNZ(jsd, axis_num); */
			    /* 	} */
			    /*} */

			}

			/* Hat y */
			if((axis_hat_y > -1) &&
			   !gc->axis_kb_state
			)
			{
			    axis_num = axis_hat_y;
			    if(JSIsAxisAllocated(jsd, axis_num))
			    {
			    	axis_ptr = jsd->axis[axis_num];
				gc->hat_y=-JSGetAxisCoeff(jsd,axis_num)*1000000/30.518509;
				//printf("Hat_y: %f\n", gc->hat_y);				
			    }
			    /* if(JSIsAxisAllocated(jsd, axis_num)) */
			    /* { */
			    /* 	axis_ptr = jsd->axis[axis_num]; */

			    /* 	if(HAT_IS_CEN_RAW(axis_ptr->cur, axis_ptr->cen) && */
			    /* 	   !HAT_IS_CEN_RAW(axis_ptr->prev, axis_ptr->cen) && */
			    /* 	   HAT_IS_CEN_COEFF(gc->hat_y) */
			    /* 	) */
			    /* 	{ */
			    /* 	    /\* Pressed and released during busy loop *\/ */
			    /* 	    gc->hat_y = (float)DELTA_TIME_TO_COEFF( */
			    /* 		axis_ptr->time, axis_ptr->last_time, dms */
			    /* 	    ) * (float)( */
			    /* 		(axis_ptr->prev > axis_ptr->cen) ? 1.0 : -1.0 */
			    /* 	    ); */
			    /* 	    if(axis_ptr->flags & JSAxisFlagFlipped) */
			    /* 		gc->hat_y *= -1.0f; */
			    /* 	} */
			    /* 	else */
			    /* 	{ */
			    /* 	    gc->hat_y = (float)JSGetAxisCoeffNZ(jsd, axis_num); */

			    /* 	} */
			    /* } */

			}

			/* Begin handling buttons */

			/* Zoom in button state and coefficient */
			button_num = joystick->button_zoom_in;
			if(JSIsButtonAllocated(jsd, button_num) &&
			   !gc->button_kb_state)
			{
			    button_ptr = jsd->button[button_num];

			    if((button_ptr->changed_state == JSButtonChangedStateOnToOff) &&
			       !gc->zoom_in_state
			    )
			    {
				/* Pressed and released during busy loop */
				gc->zoom_in_coeff = (float)DELTA_TIME_TO_COEFF(
				    button_ptr->time, button_ptr->last_time, dms
				);
				gc->zoom_in_state = (button_ptr->state) ? True : False;
			    }
			    else
			    {
				gc->zoom_in_state = (button_ptr->state) ? True : False;
				gc->zoom_in_coeff = (float)((button_ptr->state) ? 1.0 : 0.0);
			    }
			}
			/* Zoom out button state and coefficient */
			button_num = joystick->button_zoom_out;
			if(JSIsButtonAllocated(jsd, button_num) &&
			   !gc->button_kb_state
			)
			{
			    button_ptr = jsd->button[button_num];

			    if((button_ptr->changed_state == JSButtonChangedStateOnToOff) &&
			       !gc->zoom_out_state
			    )
			    {
				/* Pressed and released during busy loop */
				gc->zoom_out_coeff = (float)DELTA_TIME_TO_COEFF(
				    button_ptr->time, button_ptr->last_time, dms
				);
				gc->zoom_out_state = (button_ptr->state) ? True : False;
			    }
			    else
			    {
				gc->zoom_out_state = (button_ptr->state) ? True : False;
				gc->zoom_out_coeff = (float)((button_ptr->state) ? 1.0 : 0.0);
			    }
			}
			/* Hoist up button state and coefficient */
			button_num = joystick->button_hoist_up;
			if(JSIsButtonAllocated(jsd, button_num) &&
			   !gc->button_kb_state
			)
			{
			    button_ptr = jsd->button[button_num];

			    if((button_ptr->changed_state == JSButtonChangedStateOnToOff) &&
			       !gc->hoist_up_state
			    )
			    {
				/* Pressed and released during busy loop */
				gc->hoist_up_coeff = (float)DELTA_TIME_TO_COEFF(
				    button_ptr->time, button_ptr->last_time, dms
				);
				gc->hoist_up_state = (button_ptr->state) ? True : False;
			    }
			    else
			    {
				gc->hoist_up_state = (button_ptr->state) ? True : False;
				gc->hoist_up_coeff = (float)((button_ptr->state) ? 1.0 : 0.0);
			    }
			}
			/* Hoist down button state and coefficient */
			button_num = joystick->button_hoist_down;
			if(JSIsButtonAllocated(jsd, button_num) &&
			   !gc->button_kb_state
			)
			{
			    button_ptr = jsd->button[button_num];

			    if((button_ptr->changed_state == JSButtonChangedStateOnToOff) &&
			       !gc->hoist_down_state
			    )
			    {
				/* Pressed and released during busy loop */
				gc->hoist_down_coeff = (float)DELTA_TIME_TO_COEFF(
				    button_ptr->time, button_ptr->last_time, dms
				);
				gc->hoist_down_state = (button_ptr->state) ? True : False;
			    }
			    else
			    {
				gc->hoist_down_state = (button_ptr->state) ? True : False;
				gc->hoist_down_coeff = (float)((button_ptr->state) ? 1.0 : 0.0);
			    }
			}

			/* Update ctrl modifier state (for switching
			 * bank axis to act as heading axis), but update
			 * this modifier only if the button mapped to it
			 * has been defined
			 */
			if((joystick->button_rotate > -1) &&
			   !gc->button_kb_state
			)
			    gc->ctrl_state = (JSGetButtonState(
				jsd, joystick->button_rotate
			    )) ? True : False;

			/* Air brakes */
			button_num = joystick->button_air_brakes;
			if(JSIsButtonAllocated(jsd, button_num) &&
			   !gc->button_kb_state
			)
			{
			    button_ptr = jsd->button[button_num];

			    /* Air brakes (unlike wheel brakes) are toggled
			     * on and off
			     */
			    if(button_ptr->changed_state == JSButtonChangedStateOffToOn)
			    {
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
			}

			/* Wheel brakes */
			button_num = joystick->button_wheel_brakes;
			if(JSIsButtonAllocated(jsd, button_num) &&
			   !gc->button_kb_state
			)
			{
			    button_ptr = jsd->button[button_num];

			    if((button_ptr->changed_state == JSButtonChangedStateOnToOff) &&
			       !gc->wheel_brakes_state
			    )
			    {
				/* Pressed and released during busy loop */
				gc->wheel_brakes_coeff = (float)DELTA_TIME_TO_COEFF(
				    button_ptr->time, button_ptr->last_time, dms
				);
				/* Wheel brakes state updated below */
			    }
			    else
			    {
				/* Wheel brakes state updated below */
				gc->wheel_brakes_coeff = (float)((button_ptr->state) ? 1.0 : 0.0);
			    }
			}
			if(gc->wheel_brakes_coeff > 0.0f)
		        {
		            if(gc->shift_state)
			        gc->wheel_brakes_state = 2;	/* Parking brakes */
		            else
			        gc->wheel_brakes_state = 1;	/* Brakes */
		        }
		        else
		        {
		            if(gc->wheel_brakes_state != 2)
			        gc->wheel_brakes_state = 0;	/* Off */
		        }
		    }
		    else
		    {
		        /* No events to report, reset some place holding
			 * variables
			 */

		        /* Reset hat values (this is needed) */
			axis_num = axis_hat_x;
			if(JSIsAxisAllocated(jsd, axis_num) &&
			   !gc->axis_kb_state
			)
			{
			    axis_ptr = jsd->axis[axis_num];

			    if(HAT_IS_CEN_RAW(axis_ptr->cur, axis_ptr->cen) &&
			       (gc->hat_x != 0.0f)
			    )
				gc->hat_x = 0.0f;
			}
			axis_num = axis_hat_y;
			if(JSIsAxisAllocated(jsd, axis_num) &&
			   !gc->axis_kb_state
			)
			{
			    axis_ptr = jsd->axis[axis_num];

			    if(HAT_IS_CEN_RAW(axis_ptr->cur, axis_ptr->cen) &&
			       (gc->hat_y != 0.0f)
			    )
				gc->hat_y = 0.0f;
			}
		    }
		}	/* libjsw data structure allocated? */

#undef HAT_IS_CEN_RAW
#undef HAT_IS_CEN_COEFF
#undef DELTA_TIME_TO_COEFF

#endif	/* JSW_H */

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
#define DO_KB_BTN_UPDATE				\
{ 							\
 if(!(*btn_kb_state))					\
 {							\
  if(*btn_kb_coeff > 0.0f)				\
   *btn_kb_coeff = (float)MAX(				\
    (*btn_kb_coeff) -					\
    (btn_kb_decell_coeff * time_compensation),		\
    0.0							\
   );							\
  else							\
   *btn_kb_coeff = (float)MIN(				\
    (*btn_kb_coeff) +					\
    (btn_kb_decell_coeff * time_compensation),		\
    0.0							\
   );							\
 }							\
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
#define DO_HAS_NO_AUTOREPEAT    \
GWKeyboardAutoRepeat(display, (Boolean)!state);

/* Update the keyboard key based on the value of the pointers to
 * three variables representing the key's state, coefficient, and
 * last event time (btn_kb_state, btn_kb_coeff, and btn_kb_last
 * respectivly).
 */
#define DO_BTN_KB_UPDATE					\
if(state != (*btn_kb_state))					\
{								\
 time_t dt = MAX(t - (*btn_kb_last), 0);			\
 if(state)							\
 {								\
  /* Pressed down */						\
  *btn_kb_state = True;						\
  *btn_kb_coeff = 1.0f;						\
  *btn_kb_last = t;						\
 }								\
 else if(dt > lapsed_ms)					\
 {								\
  /* Released up */						\
  *btn_kb_state = False;					\
  *btn_kb_last = t;						\
 }								\
 else								\
 {								\
  /* Released up quickly from a prior press which occured	\
   * during this cycle.						\
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
#define DO_BTN_KB_AXIS_UPDATE					\
if(state != (*btn_kb_state))					\
{								\
 time_t dt = MAX(t - (*btn_kb_last), 0);			\
 if(state)							\
 {								\
  /* Pressed down */						\
  *btn_kb_state = True;						\
  *btn_kb_coeff = btn_kb_to_coeff;				\
  *btn_kb_last = t;						\
 }								\
 else if(dt > lapsed_ms)					\
 {								\
  /* Released up */						\
  *btn_kb_state = False;					\
  *btn_kb_last = t;						\
 }								\
 else								\
 {								\
  /* Released up quickly from a prior press which occured	\
   * during this cycle.						\
   */								\
  *btn_kb_state = False;					\
  *btn_kb_coeff = btn_kb_to_coeff * ((lapsed_ms > 0) ?	\
   ((float)dt / (float)lapsed_ms) : 0.0f);		\
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
	int i;
	gctl_js_struct *joystick;
#if defined(JSW_H)
        js_data_struct *jsd;
#endif  /* JSW_H */

	last_gctl_error = NULL;

	if(gc == NULL)
	    return;

	/* Joystick(s) initialized? */
	if(gc->joystick != NULL)
	{
	    /* Iterate through each joystick */
	    for(i = 0; i < gc->total_joysticks; i++)
	    {
		joystick = &gc->joystick[i];

		/* Begin shutting down this joystick */
#if defined(JSW_H)
		/* Using libjsw, get pointer to libjsw joystick data handle */
		jsd = (js_data_struct *)joystick->data;
		if(jsd != NULL)
		{
		    /* Close and then delete the jsd */
		    JSClose(jsd);
		    free(jsd);
		    joystick->data = jsd = NULL;
		}
#else
		/* No joystick support compiled
		 *
		 * Check if Joystick Data is not NULL, if so then print
		 * warning
		 */
		if(joystick->data != NULL)
		    fprintf(
			stderr,
"GCtlDelete(): Unable to shutdown joystick: Joystick support not compiled"
		    );
#endif	/* JSW_H */
	    }

	    /* Delete joysticks */
	    free(gc->joystick);
	    gc->joystick = NULL;
	    gc->total_joysticks = 0;
	}

	free(gc);
}
