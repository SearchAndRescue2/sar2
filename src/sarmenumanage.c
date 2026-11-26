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
#include <stdlib.h>
#include <string.h>

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

static void SARMenuManageOptionsControllerJSMapping(
        SAR_MENU_MANAGE_PROTOTYPE
);

static void SARMenuManageOptionsControllerJoystick(
        SAR_MENU_MANAGE_PROTOTYPE
);

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

#define DO_REDRAW_OBJECT(d,m,n)	{	\
 if((m)->always_full_redraw)		\
  SARMenuDrawAll((d), (m));		\
 else					\
  SARMenuDrawObject((d),(m),(n));	\
 GWSwapBuffer(d);			\
}


/*
 *	Manages menu Options->Controller->Joystick->Mapping
 *
 *	1) Selects a spin index by moving an axis or clicking on a button.
 * 	   Corresponding opt->js*_* value is set by SARMenuOptionsSpinCB().
 *
 * 	2) Copy moved/clicked joystick SDL GUID string and name
 * 	   to opt->js*_sdl_guid_s and opt->js*_sdl_name.
 */
static void SARMenuManageOptionsControllerJSMapping(
        SAR_MENU_MANAGE_PROTOTYPE
)
{
//fprintf(stderr, "%s:%d : Entering SARMenuManageOptionsControllerJSMapping()\n", __FILE__, __LINE__);
#define FG_COLOR_WHITE  sar_menu_color_struct fg_color = { 1.0, 0.9, 0.9, 0.9 }; // ( a, r, g, b )
#define FG_COLOR_YELLOW sar_menu_color_struct fg_color = { 1.0, 0.9, 0.9, 0.0 }; // ( a, r, g, b )
	gw_display_struct *display = core_ptr->display;
	gctl_struct *gc = core_ptr->gctl;
	gctl_js_mapping_menu_data_struct *axis;
	sar_option_struct *opt = &core_ptr->option;
	int jsnum, i, device_index, activated = 0;
	SDL_Joystick *sdl_joystick = NULL;
	Sint16 axis_value;
	Boolean is_an_axis_spin = False, is_the_pov_hat_spin = False, is_a_button_spin = False;
	sar_menu_label_struct *label_ptr;
	char guid_str[33];
	SDL_JoystickGUID guid;


	GctlJsOpenAndMapp(gc, NULL);
	SDL_JoystickUpdate();

	sar_menu_spin_struct *spin = SAR_MENU_SPIN(
	    SARMenuGetObject(menu, menu->selected_object)
	);

	/* Get spin type */

	switch(spin->id){
	    case SAR_MENU_ID_OPT_HEADING_AXIS:
	    case SAR_MENU_ID_OPT_PITCH_AXIS:
	    case SAR_MENU_ID_OPT_BANK_AXIS:
	    case SAR_MENU_ID_OPT_THROTTLE_AXIS:
	    case SAR_MENU_ID_OPT_BRAKE_LEFT_AXIS:
	    case SAR_MENU_ID_OPT_BRAKE_RIGHT_AXIS:
	    case SAR_MENU_ID_OPT_POV_HAT_X:
	    case SAR_MENU_ID_OPT_POV_HAT_Y:
		is_an_axis_spin = True;
		break;

	    case SAR_MENU_ID_OPT_POV_HAT:
		is_the_pov_hat_spin = True;
		break;

	    case SAR_MENU_ID_OPT_ROTATE_BUTTON:
	    case SAR_MENU_ID_OPT_AIR_BRAKES_BUTTON:
	    case SAR_MENU_ID_OPT_WHEEL_BRAKES_BUTTON:
	    case SAR_MENU_ID_OPT_ZOOM_IN_BUTTON:
	    case SAR_MENU_ID_OPT_ZOOM_OUT_BUTTON:
	    case SAR_MENU_ID_OPT_HOIST_UP_BUTTON:
	    case SAR_MENU_ID_OPT_HOIST_DOWN_BUTTON:
		is_a_button_spin = True;
		break;

	    default:
		break;
	}

	/* Change "help labels" color and text as needed.
	 * Because labels don't have any ID, label detection is made with
	 * label label (i.e. label text). Only the 3 first characters are
	 * used for this detection in order to allow text translation.
	 *
	 * Note that labels are always center aligned thus text width and
	 * number of text lines are important.
	 */

	/* Selected object not a spin? */
	if(!SAR_MENU_IS_SPIN(spin)){
	    for(i = 0; i < menu->total_objects; i++){
		label_ptr = SARMenuGetObject(menu, i);

		/* This object not the good one? */
		if(label_ptr == NULL ||
		    label_ptr->type != SAR_MENU_OBJECT_TYPE_LABEL ||
		    !strstr(label_ptr->label, "1) ")
		    )
		    continue;

		FG_COLOR_YELLOW

		/* Set label color */
		SARMenuLabelSetLabel(display, menu, i, NULL, &fg_color, True);

		break;
	    }
	    for(i = 0; i < menu->total_objects; i++){
		label_ptr = SARMenuGetObject(menu, i);

		/* This object not the good one? */
		if(label_ptr == NULL ||
		    label_ptr->type != SAR_MENU_OBJECT_TYPE_LABEL ||
		    !strstr(label_ptr->label, "2) ")
		    )
		    continue;

		FG_COLOR_WHITE

		/* Set label text and color */
		SARMenuLabelSetLabel(display, menu, i,
#if defined(PROG_LANGUAGE_SPANISH)
"2) Nuevas instrucciones vienen...                                          \n\
                                                                            \n\
                                                                            "
#elif defined(PROG_LANGUAGE_FRENCH)
"2) De nouvelles instructions arrivent...                                   \n\
                                                                            \n\
                                                                            "
#elif defined(PROG_LANGUAGE_GERMAN)
"2) Neue Anweisungen kommen...                                              \n\
                                                                            \n\
                                                                            "
#elif defined(PROG_LANGUAGE_ITALIAN)
"2) Nuove istruzioni in arrivo...                                           \n\
                                                                            \n\
                                                                            "
#elif defined(PROG_LANGUAGE_DUTCH)
"2) Er komen nieuwe instructies...                                          \n\
                                                                            \n\
                                                                            "
#elif defined(PROG_LANGUAGE_PORTUGUESE)
"2) Novas instrucoes a chegar...                                            \n\
                                                                            \n\
                                                                            "
#elif defined(PROG_LANGUAGE_NORWEGIAN)
"2) Nye instruksjoner kommer...                                             \n\
                                                                            \n\
                                                                            "
#else
"2) New instructions coming...                                              \n\
                                                                            \n\
                                                                            "
#endif
		    , &fg_color, True
		);

		break;
	    }
	    return;
	}
	/* Selected object is a spin */
	else{
	    for(i = 0; i < menu->total_objects; i++){
		label_ptr = SARMenuGetObject(menu, i);

		/* This object not the good one? */
		if(label_ptr == NULL ||
		    label_ptr->type != SAR_MENU_OBJECT_TYPE_LABEL ||
		    !strstr(label_ptr->label, "1) ")
		    )
		    continue;

		FG_COLOR_WHITE

		/* Set label color */
		SARMenuLabelSetLabel(display, menu, i, NULL, &fg_color, True);

		break;
	    }
	    for(i = 0; i < menu->total_objects; i++){
		label_ptr = SARMenuGetObject(menu, i);

		/* This object not the good one? */
		if(label_ptr == NULL ||
		    label_ptr->type != SAR_MENU_OBJECT_TYPE_LABEL ||
		    !strstr(label_ptr->label, "2) ")
		    )
		    continue;

		FG_COLOR_YELLOW

		/* Set label text (handled by spin role type) and color */

		if(is_an_axis_spin)
		    SARMenuLabelSetLabel(display, menu, i,
#if defined(PROG_LANGUAGE_SPANISH)
"2) Mueva el eje de un joystick de punta a punta para seleccionarlo,        \n\
   o presione la tecla 'Inicio' del teclado para restablecer la asignacion  \n\
   del cuadro de funciones.                                                 "
#elif defined(PROG_LANGUAGE_FRENCH)
"2) Bougez un axe de joystick de butee en butee pour le selectionner,       \n\
   ou appuyez sur la touche 'Debut' du clavier pour deselectionner ce role. \n\
                                                                            "
#elif defined(PROG_LANGUAGE_GERMAN)
"2) Bewegen Sie eine Joystickachse von Zehe zu Zehe, um sie auszuwahlen,    \n\
   oder drucken Sie die Home-Taste auf der Tastatur, um die                 \n\
   Rollenfeldzuordnung zuruckzusetzen.                                      "
#elif defined(PROG_LANGUAGE_ITALIAN)
"2) Muovi l'asse del joystick da un piede all'altro per selezionarlo,       \n\
   oppure premere il tasto 'Home' della tastiera per reimpostare la         \n\
   mappatura delle caselle dei ruoli.                                       "
#elif defined(PROG_LANGUAGE_DUTCH)
"2) Beweeg de as van een joystick van teen tot teen om deze te selecteren,  \n\
   of druk op de 'Home'-toets op het toetsenbord om de rolboxtoewijzing     \n\
   opnieuw in te stellen.                                                   "
#elif defined(PROG_LANGUAGE_PORTUGUESE)
"2) Mova o eixo do joystick de ponta a ponta para o seleccionar,            \n\
   ou prima a tecla 'Home' do teclado para repor o mapeamento da caixa de   \n\
   funcoes.                                                                 "
#elif defined(PROG_LANGUAGE_NORWEGIAN)
"2) Flytt en joystick-akse fra ta til ta for a velge den,                   \n\
   eller trykk pa 'Hjem'-tasten pa tastaturet for a tilbakestille           \n\
   rollebokskartlegging.                                                    "
#else
"2) Move a joystick axis from toe to toe to select it,                      \n\
   or press keyboard 'Home' key to reset role box mapping.                  \n\
                                                                            "
#endif
			, &fg_color, True
		    );

		else if(is_the_pov_hat_spin)
		    SARMenuLabelSetLabel(display, menu, i,
#if defined(PROG_LANGUAGE_SPANISH)
"Haga clic en un cuadro de funcion y luego mueva el eje del joystick        \n\
    hacia la punta para establecer la funcion del eje o presione un         \n\
    boton del joystick para configurar la funcion del boton.                "
#elif defined(PROG_LANGUAGE_FRENCH)
"2) Bougez un bouton de point de vue d'un joystick pour le selectionner,    \n\
   ou appuyez sur la touche 'Debut' du clavier pour deselectionner ce role. \n\
                                                                            "
#elif defined(PROG_LANGUAGE_GERMAN)
"Klicken Sie auf ein Rollenfeld und bewegen Sie dann die                    \n\
    Joystick-Achse nach vorne, um die Achsenrolle festzulegen oder          \n\
    druecken Sie eine Joystick-Taste, um die Tastenrolle festzulegen.       "
#elif defined(PROG_LANGUAGE_ITALIAN)
"Fare clic su una casella di ruolo, quindi spostare l'asse del              \n\
    joystick in punta per impostare il ruolo dell'asse oppure premere un    \n\
    pulsante del joystick per impostare il ruolo del pulsante.              "
#elif defined(PROG_LANGUAGE_DUTCH)
"Klik op een rolvak en beweeg de as van de joystick naar de teen om         \n\
    de rol van de as in te stellen of druk op een joystickknop om de        \n\
    rol van de knop in te stellen.                                          "
#elif defined(PROG_LANGUAGE_PORTUGUESE)
"Clique em uma caixa de funcao e mova o eixo do joystick para a ponta       \n\
    do pe para definir a funcao do eixo ou pressione um botao do            \n\
    joystick para definir a funcao do botao.                                "
#elif defined(PROG_LANGUAGE_NORWEGIAN)
"Klikk pa en rolleboks og flytt styrespakens akse til ta for a angi         \n\
    akserolle eller trykk pa en styrespakknapp for a angi knapprolle.       \n\
                                                                            "
#else
"2) Move a joystick POV hat to select it,                                   \n\
   or press keyboard 'Home' key to reset role box mapping.                  \n\
                                                                            "
#endif
			, &fg_color, True
		    );

		else if(is_a_button_spin)
		    SARMenuLabelSetLabel(display, menu, i,
#if defined(PROG_LANGUAGE_SPANISH)
"Haga clic en un cuadro de funcion y luego mueva el eje del joystick        \n\
    hacia la punta para establecer la funcion del eje o presione un         \n\
    botón del joystick para configurar la funcion del boton.                "
#elif defined(PROG_LANGUAGE_FRENCH)
"2) Appuyez sur un bouton d'un joystick pour le selectionner,               \n\
   ou appuyez sur la touche 'Debut' du clavier pour deselectionner ce role. \n\
                                                                            "
#elif defined(PROG_LANGUAGE_GERMAN)
"Klicken Sie auf ein Rollenfeld und bewegen Sie dann die                    \n\
    Joystick-Achse nach vorne, um die Achsenrolle festzulegen oder          \n\
    drücken Sie eine Joystick-Taste, um die Tastenrolle festzulegen.        "
#elif defined(PROG_LANGUAGE_ITALIAN)
"Fare clic su una casella di ruolo, quindi spostare l'asse del              \n\
    joystick in punta per impostare il ruolo dell'asse oppure premere un    \n\
    pulsante del joystick per impostare il ruolo del pulsante.              "
#elif defined(PROG_LANGUAGE_DUTCH)
"Klik op een rolvak en beweeg de as van de joystick naar de teen om         \n\
    de rol van de as in te stellen of druk op een joystickknop om de        \n\
    rol van de knop in te stellen.                                          "
#elif defined(PROG_LANGUAGE_PORTUGUESE)
"Clique em uma caixa de funcao e mova o eixo do joystick para a ponta       \n\
    do pe para definir a funcao do eixo ou pressione um botao do            \n\
    joystick para definir a funcao do botao.                                "
#elif defined(PROG_LANGUAGE_NORWEGIAN)
"Klikk pa en rolleboks og flytt styrespakens akse til ta for a angi         \n\
    akserolle eller trykk pa en styrespakknapp for a angi knapprolle.       \n\
                                                                            "
#else
"2) Press a joystick button to select it,                                   \n\
   or press keyboard 'Home' key to reset role box mapping.                  \n\
                                                                            "
#endif
			, &fg_color, True
		    );

		break;
	    }
	}

	/* Set spin value  */

	for(jsnum = 0; jsnum < gc->total_sdl_joysticks; jsnum++){
	    sdl_joystick = gc->sdljoystick[jsnum];

	    if(sdl_joystick == NULL)
		continue;

	    if(is_an_axis_spin){
		/* Look for a currently moved axis. */
		for(i = 0; i < SDL_JoystickNumAxes(sdl_joystick); i++)
		{
		    if(i >= MAX_JOYSTICK_AXES)
			break;

		    axis_value = SDL_JoystickGetAxis(sdl_joystick, i);
		    axis = &gc->js_mapping_axes_values[jsnum * MAX_JOYSTICK_AXES + i];

		    if(axis_value < 0 && axis_value < axis->min)
			axis->min = axis_value;
		    if(axis_value > 0 && axis_value > axis->max)
			axis->max = axis_value;

		    if((int)axis->max - (int)axis->min > 49151)
		    {
			/* Axis widely moved, set spin index to match player
			 * choice.
			 */
			SARMenuSpinSelectValueIndex(
			    display, menu, menu->selected_object,
			    (jsnum * MAX_JOYSTICK_AXES + i) + 1, True
			    );

			/* Mark this joystick as activated */
			activated |= 1 << jsnum;

			/* To go out of 'for(jsnum' loop */
			jsnum = gc->total_sdl_joysticks;

			/* Go out of 'for(i' loop */
			break;
		    }
		}
	    }
	    else if(is_the_pov_hat_spin){
		for(i = 0; i < SDL_JoystickNumHats(sdl_joystick); i++){
		    if(i >= MAX_JOYSTICK_HATS)
			break;

		    /* Hat not centered (thus moved)? */
		    if(SDL_JoystickGetHat(sdl_joystick, i) != SDL_HAT_CENTERED){
			/* Set spin index to match player choice */
			SARMenuSpinSelectValueIndex(
			    display, menu, menu->selected_object,
			    (jsnum * MAX_JOYSTICK_HATS + i) + 1, True
			    );

			/* Mark this joystick as activated */
			activated |= 1 << jsnum;

			/* To go out of 'for(jsnum' loop */
			jsnum = gc->total_sdl_joysticks;

			/* Go out of 'for(i' loop */
			break;
		    }
		}
	    }
	    else if(is_a_button_spin){
		for(i = 0; i < SDL_JoystickNumButtons(sdl_joystick); i++){
		    if(i >= MAX_JOYSTICK_BTNS)
			break;

		    /* Button pressed? */
		    if(SDL_JoystickGetButton(sdl_joystick, i)){
			/* Set spin index to match player choice */
			SARMenuSpinSelectValueIndex(
			    display, menu, menu->selected_object,
			    (jsnum * MAX_JOYSTICK_BTNS + i) + 1, True
			    );

			/* Mark this joystick as activated */
			activated |= 1 << jsnum;

			/* To go out of 'for(jsnum' loop */
			jsnum = gc->total_sdl_joysticks;

			/* Go out of 'for(i' loop */
			break;
		    }
		}
	    }
	}

	/* Copy sdl joystick(s) guid string to core options: this will
	 * signal that this joystick is mapped to at least one role.
	 */

	i = 0;
	if(activated & 1<< i && strlen(opt->js0_sdl_guid_s) == 0){

	    device_index = GetSdlJsIndexFromGcSdlJoystick(gc, i);
	    if(device_index > -1){
		guid = SDL_JoystickGetDeviceGUID(device_index);
		SDL_JoystickGetGUIDString(guid, guid_str, sizeof(guid_str));
		strcpy(opt->js0_sdl_guid_s, guid_str);

		if(opt->js0_sdl_name != NULL)
		    free(opt->js0_sdl_name);
		opt->js0_sdl_name = STRDUP(SDL_JoystickNameForIndex(device_index));
	    }
	}
	i = 1;
	if(activated & 1<< i && strlen(opt->js1_sdl_guid_s) == 0){
	    device_index = GetSdlJsIndexFromGcSdlJoystick(gc, i);
	    if(device_index > -1){
		guid = SDL_JoystickGetDeviceGUID(device_index);
		SDL_JoystickGetGUIDString(guid, guid_str, sizeof(guid_str));
		strcpy(opt->js1_sdl_guid_s, guid_str);

		if(opt->js1_sdl_name != NULL)
		    free(opt->js1_sdl_name);
		opt->js1_sdl_name = STRDUP(SDL_JoystickNameForIndex(device_index));
	    }
	}
	i = 2;
	if(activated & 1<< i && strlen(opt->js2_sdl_guid_s) == 0){
	    device_index = GetSdlJsIndexFromGcSdlJoystick(gc, i);
	    if(device_index > -1){
		guid = SDL_JoystickGetDeviceGUID(device_index);
		SDL_JoystickGetGUIDString(guid, guid_str, sizeof(guid_str));
		strcpy(opt->js2_sdl_guid_s, guid_str);

		if(opt->js2_sdl_name != NULL)
		    free(opt->js2_sdl_name);
		opt->js2_sdl_name = STRDUP(SDL_JoystickNameForIndex(device_index));
	    }
	}

	if(activated != 0 && is_an_axis_spin){
	    /* Clear min and max values of all axes */

	    for(jsnum = 0; jsnum < gc->total_sdl_joysticks; jsnum++){
		for(i = 0; i < MAX_JOYSTICK_AXES; i++){
		    axis = &gc->js_mapping_axes_values[jsnum * MAX_JOYSTICK_AXES + i];
		    axis->min = 0;
		    axis->max = 0;
		}
	    }
	}

//fprintf(stderr, "%s:%d : Exiting SARMenuManageOptionsControllerJSMapping()\n", __FILE__, __LINE__);
}

/*
 *      Manages menu Options->Controller->Joystick
 */
static void SARMenuManageOptionsControllerJoystick(
        SAR_MENU_MANAGE_PROTOTYPE
)
{
        gw_display_struct *display = core_ptr->display;
        gctl_struct *gc = core_ptr->gctl;
	int i;
	void *o;

	if((display == NULL) || (gc == NULL))
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
	DO_REDRAW_OBJECT(display, menu, i);
}


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
	    if(!strcasecmp(name, SAR_MENU_NAME_OPTIONS_CONTROLLER_JS_MAPPING))
		SARMenuManageOptionsControllerJSMapping(SAR_MENU_MANAGE_INPUT);
	    else if(!strcasecmp(name, SAR_MENU_NAME_OPTIONS_CONTROLLER_JOYSTICK))
		SARMenuManageOptionsControllerJoystick(SAR_MENU_MANAGE_INPUT);
#undef SAR_MENU_MANAGE_INPUT
	}
}
#undef SAR_MENU_MANAGE_PROTOTYPE
