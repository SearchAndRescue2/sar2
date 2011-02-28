#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef JS_SUPPORT
# include <jsw.h>
#endif

#include "../include/string.h"

#include "gctl.h"
#include "gw.h"
#include "menu.h"
#include "sar.h"
#include "sarmenuop.h"
#include "sarmenumanage.h"
#include "sarmenucodes.h"
#include "config.h"


#define SAR_MENU_MANAGE_PROTOTYPE	\
	sar_core_struct *core_ptr, sar_menu_struct *menu

#ifdef JS_SUPPORT
static void SARMenuManageOptionsControllerJSButtons(
        SAR_MENU_MANAGE_PROTOTYPE
);
#endif
#ifdef JS_SUPPORT
static void SARMenuManageOptionsControllerTest(
        SAR_MENU_MANAGE_PROTOTYPE
);
#endif

void SARMenuManage(
        sar_core_struct *core_ptr, sar_menu_struct *menu
);


#define ATOI(s)         (((s) != NULL) ? atoi(s) : 0)
#define ATOL(s)         (((s) != NULL) ? atol(s) : 0)
#define ATOF(s)         (((s) != NULL) ? (float)atof(s) : 0.0f)
#define STRDUP(s)       (((s) != NULL) ? strdup(s) : NULL)

#define MAX(a,b)        (((a) > (b)) ? (a) : (b))
#define MIN(a,b)        (((a) < (b)) ? (a) : (b))
#define CLIP(a,l,h)     (MIN(MAX((a),(l)),(h)))


#ifdef JS_SUPPORT
/*
 *	Manages menu Options->Controller->Buttons
 */
static void SARMenuManageOptionsControllerJSButtons(
        SAR_MENU_MANAGE_PROTOTYPE
)
{
	gw_display_struct *display = core_ptr->display;
	gctl_struct *gc = core_ptr->gctl;
	int jsnum;
	int js0_btn_num_spin_num, js1_btn_num_spin_num;
/*
	sar_menu_spin_struct *js0_btn_role_spin = SARMenuGetSpin(
	    menu, 0, NULL
	);
 */
        sar_menu_spin_struct *js0_btn_num_spin = SARMenuGetSpin(
            menu, 1, &js0_btn_num_spin_num
        );
/*
        sar_menu_spin_struct *js1_btn_role_spin = SARMenuGetSpin(
            menu, 2, NULL
        );
 */
        sar_menu_spin_struct *js1_btn_num_spin = SARMenuGetSpin(
            menu, 3, &js1_btn_num_spin_num
        );
	/* Game controller not set to joystick? */
	if((gc != NULL) ? !(gc->controllers & GCTL_CONTROLLER_JOYSTICK) : True)
	    return;

	/* Joystick #0. */
	jsnum = 0;
	if((js0_btn_num_spin != NULL) && (gc->total_joysticks > jsnum))
	{
#ifdef JSW_H
            gctl_js_struct *gc_js = &gc->joystick[jsnum];
	    js_data_struct *jsd = (js_data_struct *)gc_js->data;
	    int i, button = -1;
	    sar_menu_spin_struct *spin = js0_btn_num_spin;

	    /* This joystick initialized? */
	    if((jsd != NULL) ? JSIsInit(jsd) : False)
	    {
		/* Look for a currently pressed button. */
		for(i = 0; i < jsd->total_buttons; i++)
		{
		    if(JSGetButtonState(jsd, i) == JSButtonStateOn)
		    {
			button = i;
			break;
		    }
		}
	    }
	
	    /* Was a button pressed? */
	    if(button > -1)
	    {
		/* Spin button values start at index 1 so button 0
		 * is spin value index 1.
		 */
		i = button + 1;
		/* Change in value? */
		if(spin->cur_value != i)
		    SARMenuSpinSelectValueIndex(
			display, menu, js0_btn_num_spin_num,
			i, True
		    );
            }
#endif	/* JSW_H */
	}

        /* Joystick #1. */
	jsnum = 1;
        if((js1_btn_num_spin != NULL) && (gc->total_joysticks > jsnum))
        {
#ifdef JSW_H
            gctl_js_struct *gc_js = &gc->joystick[jsnum];
            js_data_struct *jsd = (js_data_struct *)gc_js->data;
            int i, button = -1;
            sar_menu_spin_struct *spin = js1_btn_num_spin;

            /* This joystick initialized? */
            if((jsd != NULL) ? JSIsInit(jsd) : False)
            {
		/* Look for a currently pressed button. */
                for(i = 0; i < jsd->total_buttons; i++)
                {
                    if(JSGetButtonState(jsd, i) == JSButtonStateOn)
                    {
                        button = i;
                        break;
                    }
                }
            }

            /* Was a button pressed? */
            if(button > -1)
            {
                /* Spin button values start at index 1 so button 0
                 * is spin value index 1.
                 */
                i = button + 1;
                /* Change in value? */
                if(spin->cur_value != i)
                    SARMenuSpinSelectValueIndex(
                        display, menu, js1_btn_num_spin_num,
                        i, True
                    );
            }
#endif	/* JSW_H */
	}
}
#endif	/* JS_SUPPORT */

#ifdef JS_SUPPORT
/*
 *      Manages menu Options->Controller->Test
 */
static void SARMenuManageOptionsControllerTest(
        SAR_MENU_MANAGE_PROTOTYPE
)
{
        gw_display_struct *display = core_ptr->display;
        gctl_struct *gc = core_ptr->gctl;
	int i;
	void *o;

	if((display == NULL) || (gc == NULL))
	    return;

	/* No joysticks? */
	if(gc->total_joysticks <= 0)
	    return;

	/* Find joystick test multi-purpose display object. */
	for(i = 0; i < menu->total_objects; i++)
	{
	    o = menu->object[i];
	    if(SAR_MENU_IS_MDISPLAY(o))
		break;
	}
	if(i >= menu->total_objects)
	    return;

	/* Redraw joystick test multi-purpose display object. */
	SARMenuDrawObject(display, menu, i);
	GWSwapBuffer(display);
}
#endif  /* JS_SUPPORT */


/*
 *	Called once per cycle to manage the menu system (above the 
 *	graphics wrapper level).
 */
void SARMenuManage(
        sar_core_struct *core_ptr, sar_menu_struct *menu
)
{
	const char *name;

	if(menu == NULL)
	    return;

	/* Handle by which menu we are currently in. */
	name = menu->name;
	if(name != NULL)
	{
#define SAR_MENU_MANAGE_INPUT	core_ptr, menu
#ifdef JS_SUPPORT
	    if(!strcasecmp(name, SAR_MENU_NAME_OPTIONS_CONTROLLER_JS_BTN))
		SARMenuManageOptionsControllerJSButtons(SAR_MENU_MANAGE_INPUT);
	    else if(!strcasecmp(name, SAR_MENU_NAME_OPTIONS_CONTROLLER_TEST))
		SARMenuManageOptionsControllerTest(SAR_MENU_MANAGE_INPUT);
#endif	/* JS_SUPPORT */
#undef SAR_MENU_MANAGE_INPUT
	}

}
#undef SAR_MENU_MANAGE_PROTOTYPE
