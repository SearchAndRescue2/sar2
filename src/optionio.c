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
#include <sys/stat.h>

#include "../include/fio.h"
#include "../include/string.h"

#include "gctl.h"
#include "sar.h"
#include "optionio.h"
#include "config.h"


int SAROptionsLoadFromFile(
	sar_option_struct *opt, const char *filename
);
int SAROptionsSaveToFile(
	const sar_option_struct *opt, const char *filename
);


#define ATOI(s)         (((s) != NULL) ? atoi(s) : 0)
#define ATOL(s)         (((s) != NULL) ? atol(s) : 0)
#define ATOF(s)         (((s) != NULL) ? atof(s) : 0.0f)
#define STRDUP(s)       (((s) != NULL) ? strdup(s) : NULL)

#define MAX(a,b)        (((a) > (b)) ? (a) : (b))
#define MIN(a,b)        (((a) < (b)) ? (a) : (b))
#define CLIP(a,l,h)     (MIN(MAX((a),(l)),(h)))

#ifndef SAR_COMMENT_CHAR
# define SAR_COMMENT_CHAR	'#'
#endif

#ifndef SAR_CFG_DELIM_CHAR
# define SAR_CFG_DELIM_CHAR	'='
#endif

/*
 *	Loads options from file.
 */
int SAROptionsLoadFromFile(
	sar_option_struct *opt, const char *filename
)
{
	FILE *fp;
	char *buf = NULL;
	double version[3];
	struct stat stat_buf;


	if((opt == NULL) || (filename == NULL))
	    return(-1);

	if(stat(filename, &stat_buf))
	{
	    fprintf(stderr, "%s: No such file.\n", filename);
	    return(-1);
	}
#ifdef S_ISREG
	if(!S_ISREG(stat_buf.st_mode))
	{
	    fprintf(stderr,
		"%s: Not a file.\n",
		filename
	    );
	    return(-1);
	}
#endif	/* S_ISREG */

	fp = FOpen(filename, "rb");
	if(fp == NULL)
	{
	    fprintf(stderr, "%s: Cannot open.\n", filename);
	    return(-1);
	}

	do
	{
	    buf = FSeekNextParm(
		fp,
		buf,
		SAR_COMMENT_CHAR,
		SAR_CFG_DELIM_CHAR
	    );
	    if(buf == NULL)
		break;

	    /* Version */
	    if(!strcasecmp(buf, "Version"))
	    {
		FGetValuesF(fp, version, 3);
	    }

	    /* MenuBackgrounds */
	    else if(!strcasecmp(buf, "MenuBackgrounds"))
	    {
		double vf[1];
		FGetValuesF(fp, vf, 1);
/* Depreciated */
	    }

	    /* Units */
	    else if(!strcasecmp(buf, "Units"))
	    {
		double vf[1];
		FGetValuesF(fp, vf, 1);
		opt->units = (sar_units)vf[0];
	    }

	    /* TexturedGround */
	    else if(!strcasecmp(buf, "TexturedGround"))
	    {
		double vf[1];
		FGetValuesF(fp, vf, 1);
		opt->textured_ground = ((int)vf[0]) ? True : False;
	    }
	    /* TexturedObjects */
	    else if(!strcasecmp(buf, "TexturedObjects"))
	    {
		double vf[1];
		FGetValuesF(fp, vf, 1);
		opt->textured_objects = ((int)vf[0]) ? True : False;
	    }
	    /* TexturedClouds */
	    else if(!strcasecmp(buf, "TexturedClouds"))
	    {
		double vf[1];
		FGetValuesF(fp, vf, 1);
		opt->textured_clouds = ((int)vf[0]) ? True : False;
	    }
	    /* Atmosphere */
	    else if(!strcasecmp(buf, "Atmosphere"))
	    {
		double vf[1];
		FGetValuesF(fp, vf, 1);
		opt->atmosphere = ((int)vf[0]) ? True : False;
	    }
	    /* DualPassDepth */
	    else if(!strcasecmp(buf, "DualPassDepth"))
	    {
		double vf[1];
		FGetValuesF(fp, vf, 1);
		opt->dual_pass_depth = ((int)vf[0]) ? True : False;
	    }
	    /* PropWash */
	    else if(!strcasecmp(buf, "PropWash"))
	    {
		double vf[1];
		FGetValuesF(fp, vf, 1);
		opt->prop_wash = ((int)vf[0]) ? True : False;
	    }
	    /* SmokeTrails */
	    else if(!strcasecmp(buf, "SmokeTrails"))
	    {
		double vf[1];
		FGetValuesF(fp, vf, 1);
		opt->smoke_trails = ((int)vf[0]) ? True : False;
	    }
	    /* GLPolygonOffsetFactor */
	    else if(!strcasecmp(buf, "GLPolygonOffsetFactor"))
	    {
		double vf[1];
		FGetValuesF(fp, vf, 1);
		opt->gl_polygon_offset_factor = (float)vf[0];
	    }
	    /* VisibilityMax */
	    else if(!strcasecmp(buf, "VisibilityMax"))
	    {
		double vf[1];
		FGetValuesF(fp, vf, 1);
		opt->visibility_max = (int)vf[0];
	    }
	    /* RotorBlurStyle */
	    else if(!strcasecmp(buf, "RotorBlurStyle"))
	    {
		double vf[1];
		FGetValuesF(fp, vf, 1);
		opt->rotor_blur_style = (sar_rotor_blur_style)vf[0];
	    }

	    /* GraphicsAcceleration */
	    else if(!strcasecmp(buf, "GraphicsAcceleration"))
	    {
		double vf[1];
		FGetValuesF(fp, vf, 1);
		opt->graphics_acceleration = (float)vf[0];
	    }

	    /* EngineSounds */
	    else if(!strcasecmp(buf, "EngineSounds"))
	    {
		double vf[1];
		FGetValuesF(fp, vf, 1);
		opt->engine_sounds = ((int)vf[0]) ? True : False;
	    }
	    /* EventSounds */
	    else if(!strcasecmp(buf, "EventSounds"))
	    {
		double vf[1];
		FGetValuesF(fp, vf, 1);
		opt->event_sounds = ((int)vf[0]) ? True : False;
	    }
	    /* VoiceSounds */
	    else if(!strcasecmp(buf, "VoiceSounds"))
	    {
		double vf[1];
		FGetValuesF(fp, vf, 1);
		opt->voice_sounds = ((int)vf[0]) ? True : False;
	    }
	    /* Music */
	    else if(!strcasecmp(buf, "Music"))
	    {
		double vf[1];
		FGetValuesF(fp, vf, 1);
		opt->music = ((int)vf[0]) ? True : False;
	    }
	    /* SoundPriority */
	    else if(!strcasecmp(buf, "SoundPriority"))
	    {
		double vf[1];
		FGetValuesF(fp, vf, 1);
		opt->sound_priority = (int)vf[0];
	    }

	    /* SystemTimeFreeFlight */
	    else if(!strcasecmp(buf, "SystemTimeFreeFlight"))
	    {
		double vf[1];
		FGetValuesF(fp, vf, 1);
		opt->system_time_free_flight = ((int)vf[0]) ? True : False;
	    }

	    /* HUDColor */
	    else if(!strcasecmp(buf, "HUDColor"))
	    {
		sar_color_struct *c = &opt->hud_color;
		double vf[4];
		FGetValuesF(fp, vf, 4);

		/* ARGB */
		c->a = (float)CLIP(vf[0], 0.0, 1.0);
		c->r = (float)CLIP(vf[1], 0.0, 1.0);
		c->g = (float)CLIP(vf[2], 0.0, 1.0);
		c->b = (float)CLIP(vf[3], 0.0, 1.0);
	    }
	    /* MessageColor */
	    else if(!strcasecmp(buf, "MessageColor"))
	    {
		sar_color_struct *c = &opt->message_color;
		double vf[4];
		FGetValuesF(fp, vf, 4);

		/* ARGB */
		c->a = (float)CLIP(vf[0], 0.0, 1.0);
		c->r = (float)CLIP(vf[1], 0.0, 1.0);
		c->g = (float)CLIP(vf[2], 0.0, 1.0);
		c->b = (float)CLIP(vf[3], 0.0, 1.0);
	    }

	    /* SmokeSpawnInterval */
	    else if(!strcasecmp(buf, "SmokeSpawnInterval"))
	    {
		double vf[1];
		FGetValuesF(fp, vf, 1);
/* Depreciated */
	    }
	    /* ExplosionFrameInterval */
	    else if(!strcasecmp(buf, "ExplosionFrameInterval"))
	    {
		double vf[1];
		FGetValuesF(fp, vf, 1);
		opt->explosion_frame_int = (time_t)vf[0];
		if(opt->explosion_frame_int < 1l)
		{
		    fprintf(
			stderr,
 "%s: ExplosionFrameInterval: warning: Value out of range, suggest %ld.\n",
			filename, SAR_DEF_EXPLOSION_FRAME_INT
		    );
		    opt->explosion_frame_int = 1l;
		}
	    }
	    /* SplashFrameInterval */
	    else if(!strcasecmp(buf, "SplashFrameInterval"))
	    {
		double vf[1];
		FGetValuesF(fp, vf, 1);
		opt->splash_frame_int = (time_t)vf[0];
		if(opt->splash_frame_int < 1l)
		{
		    fprintf(
			stderr,
 "%s: SplashFrameInterval: warning: Value out of range, suggest %ld.\n",
			filename, SAR_DEF_SPLASH_FRAME_INT
		    );
		    opt->splash_frame_int = 1l;
		}
	    }

	    /* SmokeLifeSpan */
	    else if(!strcasecmp(buf, "SmokeLifeSpan"))
	    {
		double vf[1];
		FGetValuesF(fp, vf, 1);
/* Depreciated */
	    }
	    /* CrashExplosionLifeSpan */
	    else if(!strcasecmp(buf, "CrashExplosionLifeSpan"))
	    {
		double vf[1];
		FGetValuesF(fp, vf, 1);
		opt->crash_explosion_life_span = (time_t)vf[0];
		if(opt->crash_explosion_life_span < 1l)
		{
		    fprintf(
			stderr,
 "%s: CrashExplosionLifeSpan: warning: Value out of range, suggest %ld.\n",
			filename, SAR_DEF_CRASH_EXPLOSION_LIFE_SPAN
		    );
		    opt->crash_explosion_life_span = 1l;
		}
	    }
	    /* FuelTankLifeSpan */
	    else if(!strcasecmp(buf, "FuelTankLifeSpan"))
	    {
		double vf[1];
		FGetValuesF(fp, vf, 1);
		opt->fuel_tank_life_span = (time_t)vf[0];
		if(opt->fuel_tank_life_span < 1l)
		{
		    fprintf(
			stderr,
 "%s: FuelTankLifeSpan: warning: Value out of range, suggest %ld.\n",
			filename, SAR_DEF_FUEL_TANK_LIFE_SPAN
		    );
		    opt->fuel_tank_life_span = 1l;
		}
	    }

	    /* RotorWashVisCoeff */
	    else if(!strcasecmp(buf, "RotorWashVisCoeff"))
	    {
		double vf[1];
		FGetValuesF(fp, vf, 1);
		opt->rotor_wash_vis_coeff = (float)CLIP(vf[0], 0.0, 1.0);
	    }

	    /* GameControllers */
	    else if(!strcasecmp(buf, "GameControllers") ||
		    !strcasecmp(buf, "GameControllerType")
	    )
	    {
		double vf[1];
		FGetValuesF(fp, vf, 1);
		opt->gctl_controllers = (gctl_controllers)vf[0];

	    }
	    /* Joystick #1 (js0) SDL_NAME */
	    else if(!strcasecmp(buf, "Joystick0Name"))
	    {
		char *s = FGetString(fp);
		opt->js0_sdl_name = STRDUP(s);
	    }
	    /* Joystick #1 (js0) SDL_GUID */
	    else if(!strcasecmp(buf, "Joystick0Guid"))
	    {
		char *s = FGetString(fp);
		if(strlen(s) == 32){
		    strcpy(opt->js0_sdl_guid_s, s);
		}
		else
		    strcpy(opt->js0_sdl_guid_s, "");
	    }
	    /* Joystick #2 (js1) SDL_NAME */
	    else if(!strcasecmp(buf, "Joystick1Name"))
	    {
		char *s = FGetString(fp);
		opt->js1_sdl_name = STRDUP(s);
	    }
	    /* Joystick #2 (js1) SDL_GUID */
	    else if(!strcasecmp(buf, "Joystick1Guid"))
	    {
		char *s = FGetString(fp);
		if(strlen(s) == 32){
		    strcpy(opt->js1_sdl_guid_s, s);
		}
		else
		    strcpy(opt->js1_sdl_guid_s, "");
	    }
	    /* Joystick #3 (js2) SDL_NAME */
	    else if(!strcasecmp(buf, "Joystick2Name"))
	    {
		char *s = FGetString(fp);
		opt->js2_sdl_name = STRDUP(s);
	    }
	    /* Joystick #3 (js2) SDL_GUID */
	    else if(!strcasecmp(buf, "Joystick2Guid"))
	    {
		char *s = FGetString(fp);
		if(strlen(s) == 32){
		    strcpy(opt->js2_sdl_guid_s, s);
		}
		else
		    strcpy(opt->js2_sdl_guid_s, "");
	    }
	    /* Joystick #1 (js0) button mappings */
	    else if(!strcasecmp(buf, "Joystick0ButtonMappings") ||
		    !strcasecmp(buf, "JoystickButtonMappings")
	    )
	    {
		/* Format:
		 *
		 * <rotate> <air brakes> <wheel brakes> <zoom in>
		 * <zoom out> <hoist up> <hoist_down>
		 */
		double vf[7];
		FGetValuesF(fp, vf, 7);

		opt->js0_btn_rotate = (int)vf[0];
		opt->js0_btn_air_brakes = (int)vf[1];
		opt->js0_btn_wheel_brakes = (int)vf[2];
		opt->js0_btn_zoom_in = (int)vf[3];
		opt->js0_btn_zoom_out = (int)vf[4];
		opt->js0_btn_hoist_up = (int)vf[5];
		opt->js0_btn_hoist_down = (int)vf[6];
	    }
	    /* Joystick #2 (js1) button mappings */
	    else if(!strcasecmp(buf, "Joystick1ButtonMappings"))
	    {
		/* Format:
		 *
		 * <rotate> <air brakes> <wheel brakes> <zoom in>
		 * <zoom out> <hoist up> <hoist_down>
		 */
		double vf[7];
		FGetValuesF(fp, vf, 7);

		opt->js1_btn_rotate = (int)vf[0];
		opt->js1_btn_air_brakes = (int)vf[1];
		opt->js1_btn_wheel_brakes = (int)vf[2];
		opt->js1_btn_zoom_in = (int)vf[3];
		opt->js1_btn_zoom_out = (int)vf[4];
		opt->js1_btn_hoist_up = (int)vf[5];
		opt->js1_btn_hoist_down = (int)vf[6];
	    }
	    /* Joystick #3 (js2) button mappings */
	    else if(!strcasecmp(buf, "Joystick2ButtonMappings"))
	    {
		/* Format:
		 *
		 * <rotate> <air brakes> <wheel brakes> <zoom in>
		 * <zoom out> <hoist up> <hoist_down>
		 */
		double vf[7];
		FGetValuesF(fp, vf, 7);

		opt->js2_btn_rotate = (int)vf[0];
		opt->js2_btn_air_brakes = (int)vf[1];
		opt->js2_btn_wheel_brakes = (int)vf[2];
		opt->js2_btn_zoom_in = (int)vf[3];
		opt->js2_btn_zoom_out = (int)vf[4];
		opt->js2_btn_hoist_up = (int)vf[5];
		opt->js2_btn_hoist_down = (int)vf[6];
	    }
	    /* Joystick #1 (js0) axis roles.
	     * Should only happen when opening sar2.ini file version <= 2.6.0.
	     */
	    else if(!strcasecmp(buf, "Joystick0AxisRoles"))
	    {
		/* Value is stored as bits (unsigned long) */
		double vf[1];
		/* Read value to avoid complain (backward compatibility). */
		FGetValuesF(fp, vf, 1);
		/* opt->gctl_js0_axis_roles = (gctl_js_axis_roles)vf[0]; */

		if(vf[0] != 0){
		    fprintf(stderr, "It seems that your %s configuration file \
is not up to date.\nPlease go to Options/Controller/Joystick then remap your \
joystick(s).\n", filename);

		    /* Reset old sar2.ini file version values */
		    opt->js0_btn_rotate = -1;
		    opt->js0_btn_air_brakes = -1;
		    opt->js0_btn_wheel_brakes = -1;
		    opt->js0_btn_zoom_in = -1;
		    opt->js0_btn_zoom_out = -1;
		    opt->js0_btn_hoist_up = -1;
		    opt->js0_btn_hoist_down = -1;
		    opt->js0_pov_hat = -1;

		    opt->js1_btn_rotate = -1;
		    opt->js1_btn_air_brakes = -1;
		    opt->js1_btn_wheel_brakes = -1;
		    opt->js1_btn_zoom_in = -1;
		    opt->js1_btn_zoom_out = -1;
		    opt->js1_btn_hoist_up = -1;
		    opt->js1_btn_hoist_down = -1;
		    opt->js1_pov_hat = -1;

		    /* No need to reset js0/1/2 axis mappings and js2 buttons
		     * values because they don't exist in old sar2.ini files.
		     */
		}
	    }
	    /* Joystick #2 (js1) axis roles. Sar2 versions <= 2.6.0 only! */
	    else if(!strcasecmp(buf, "Joystick1AxisRoles"))
	    {
		/* Value is stored as bits (unsigned long) */
		double vf[1];
		/* Read value to avoid complain (backward compatibility). */
		FGetValuesF(fp, vf, 1);
		/* opt->gctl_js1_axis_roles = (gctl_js_axis_roles)vf[0]; */
	    }
	    /* Joystick #1 (js0) axes mappings */
	    else if(!strcasecmp(buf, "Joystick0AxisMappings"))
	    {
		/* Format:
		 *
		 * <heading> <pitch> <bank> <throttle>
		 * <hat_x> <hat_y> <pov_hat> <brake_left> <brake_right>
		 */
		int value[9];
		FGetValuesI(fp, value, 9);

		opt->js0_axis_heading = value[0];
		opt->js0_axis_pitch = value[1];
		opt->js0_axis_bank = value[2];
		opt->js0_axis_throttle = value[3];
		opt->js0_axis_hat_x = value[4];
		opt->js0_axis_hat_y = value[5];
		opt->js0_pov_hat = value[6];
		opt->js0_axis_brake_left = value[7];
		opt->js0_axis_brake_right = value[8];
	    }
	    /* Joystick #2 (js1) axes mappings */
	    else if(!strcasecmp(buf, "Joystick1AxisMappings"))
	    {
		/* Format:
		 *
		 * <heading> <pitch> <bank> <throttle>
		 * <hat_x> <hat_y> <pov_hat> <brake_left> <brake_right>
		 */
		int value[9];
		FGetValuesI(fp, value, 9);

		opt->js1_axis_heading = value[0];
		opt->js1_axis_pitch = value[1];
		opt->js1_axis_bank = value[2];
		opt->js1_axis_throttle = value[3];
		opt->js1_axis_hat_x = value[4];
		opt->js1_axis_hat_y = value[5];
		opt->js1_pov_hat = value[6];
		opt->js1_axis_brake_left = value[7];
		opt->js1_axis_brake_right = value[8];
	    }
	    /* Joystick #3 (js2) axes mappings */
	    else if(!strcasecmp(buf, "Joystick2AxisMappings"))
	    {
		/* Format:
		 *
		 * <heading> <pitch> <bank> <throttle>
		 * <hat_x> <hat_y> <pov_hat> <brake_left> <brake_right>
		 */
		int value[9];
		FGetValuesI(fp, value, 9);

		opt->js2_axis_heading = value[0];
		opt->js2_axis_pitch = value[1];
		opt->js2_axis_bank = value[2];
		opt->js2_axis_throttle = value[3];
		opt->js2_axis_hat_x = value[4];
		opt->js2_axis_hat_y = value[5];
		opt->js2_pov_hat = value[6];
		opt->js2_axis_brake_left = value[7];
		opt->js2_axis_brake_right = value[8];
	    }
	    /* Joysticks axis inversion */
	    else if(!strcasecmp(buf, "JoysticksAxisInversion"))
	    {
		/* Format:
		 *
		 * <heading> <pitch> <bank> <throttle>
		 * <hat_x> <hat_y> <pov_hat> <brake_left> <brake_right>
		 */
		int value[9];
		FGetValuesI(fp, value, 9);

		opt->js_axes_inversion_bits = 0;

		/* Heading */
		if(value[0] == 1)
		    opt->js_axes_inversion_bits |= GCTL_JS_AXIS_INV_HEADING;
		else
		    opt->js_axes_inversion_bits &= ~GCTL_JS_AXIS_INV_HEADING;

		/* Pitch */
		if(value[1] == 1)
		    opt->js_axes_inversion_bits |= GCTL_JS_AXIS_INV_PITCH;
		else
		    opt->js_axes_inversion_bits &= ~GCTL_JS_AXIS_INV_PITCH;

		/* Bank */
		if(value[2] == 1)
		    opt->js_axes_inversion_bits |= GCTL_JS_AXIS_INV_BANK;
		else
		    opt->js_axes_inversion_bits &= ~GCTL_JS_AXIS_INV_BANK;

		/* Throttle */
		if(value[3] == 1)
		    opt->js_axes_inversion_bits |= GCTL_JS_AXIS_INV_THROTTLE;
		else
		    opt->js_axes_inversion_bits &= ~GCTL_JS_AXIS_INV_THROTTLE;

		/* Hat x */
		if(value[4] == 1)
		    opt->js_axes_inversion_bits |= GCTL_JS_AXIS_INV_HAT_X;
		else
		    opt->js_axes_inversion_bits &= ~GCTL_JS_AXIS_INV_HAT_X;

		/* Hat y */
		if(value[5] == 1)
		    opt->js_axes_inversion_bits |= GCTL_JS_AXIS_INV_HAT_Y;
		else
		    opt->js_axes_inversion_bits &= ~GCTL_JS_AXIS_INV_HAT_Y;

		/* POV Hat */
		if(value[6] == 1)
		    opt->js_axes_inversion_bits |= GCTL_JS_AXIS_INV_POV_HAT;
		else
		    opt->js_axes_inversion_bits &= ~GCTL_JS_AXIS_INV_POV_HAT;

		/* Brake left */
		if(value[7] == 1)
		    opt->js_axes_inversion_bits |= GCTL_JS_AXIS_INV_BRAKE_LEFT;
		else
		    opt->js_axes_inversion_bits &= ~GCTL_JS_AXIS_INV_BRAKE_LEFT;

		/* Brake right */
		if(value[8] == 1)
		    opt->js_axes_inversion_bits |= GCTL_JS_AXIS_INV_BRAKE_RIGHT;
		else
		    opt->js_axes_inversion_bits &= ~GCTL_JS_AXIS_INV_BRAKE_RIGHT;
	    }
	    /* GameControllerOptions */
	    else if(!strcasecmp(buf, "GameControllerOptions"))
	    {
		/* Value is stored as bits (unsigned long) */
		double vf[1];
		FGetValuesF(fp, vf, 1);
		opt->gctl_options = (gctl_options)vf[0];
	    }

	    /* HoistContactExpansionCoefficient */
	    else if(!strcasecmp(buf, "HoistContactExpansionCoefficient"))
	    {
		double vf[1];
		FGetValuesF(fp, vf, 1);
		opt->hoist_contact_expansion_coeff = (float)MAX(vf[0], 1.0);
	    }
	    /* DamageResistanceCoefficient */
	    else if(!strcasecmp(buf, "DamageResistanceCoefficient"))
	    {
		double vf[1];
		FGetValuesF(fp, vf, 1);
		opt->damage_resistance_coeff = (float)MAX(vf[0], 1.0);
	    }
	    /* FlightPhysics */
	    else if(!strcasecmp(buf, "FlightPhysics"))
	    {
		double vf[1];
		FGetValuesF(fp, vf, 1);
		opt->flight_physics_level = (sar_flight_physics_level)vf[0];
	    }
	    /* Wind */
	    else if(!strcasecmp(buf, "Wind"))
	    {
		double vf[1];
		FGetValuesF(fp, vf, 1);
		opt->wind = (Boolean)vf[0];
	    }

	    /* Last selected player */
	    else if(!strcasecmp(buf, "LastSelectedPlayer"))
	    {
		double vf[1];
		FGetValuesF(fp, vf, 1);
		opt->last_selected_player = (int)vf[0];
	    }
	    /* Last selected mission */
	    else if(!strcasecmp(buf, "LastSelectedMission"))
	    {
		double vf[1];
		FGetValuesF(fp, vf, 1);
		opt->last_selected_mission = (int)vf[0];
	    }
	    /* Last selected free flight scene */
	    else if(!strcasecmp(buf, "LastSelectedFFScene"))
	    {
		double vf[1];
		FGetValuesF(fp, vf, 1);
		opt->last_selected_ffscene = (int)vf[0];
	    }
	    /* Last selected free flight aircraft */
	    else if(!strcasecmp(buf, "LastSelectedFFAircraft"))
	    {
		double vf[1];
		FGetValuesF(fp, vf, 1);
		opt->last_selected_ffaircraft = (int)vf[0];
	    }
	    /* Last selected free flight weather preset */
	    else if(!strcasecmp(buf, "LastSelectedFFWeather"))
	    {
		double vf[1];
		FGetValuesF(fp, vf, 1);
		opt->last_selected_ffweather = (int)vf[0];
	    }
	    /* Last toplevel window position */
	    else if(!strcasecmp(buf, "LastToplevelPosition"))
	    {
		double vf[4];
		FGetValuesF(fp, vf, 4);
		opt->last_x = (int)vf[0];
		opt->last_y = (int)vf[1];
		opt->last_width = (int)vf[2];
		opt->last_height = (int)vf[3];
	    }
	    /* Last fullscreen */
	    else if(!strcasecmp(buf, "LastFullScreen"))
	    {
		double vf[1];
		FGetValuesF(fp, vf, 1);
		opt->last_fullscreen = (Boolean)vf[0];
	    }

	    else
	    {
		fprintf(
		    stderr,
		    "%s: Unsupported parameter \"%s\".\n",
		    filename, buf
		);
		FSeekNextLine(fp);
	    }

	} while(1);

	FClose(fp);

	return(0);
}

/*
 *	Saves options to file.
 */
int SAROptionsSaveToFile(
	const sar_option_struct *opt, const char *filename
)
{
	FILE *fp;
	struct stat stat_buf;


	if((opt == NULL) || (filename == NULL))
	    return(-1);

	if(!stat(filename, &stat_buf))
	{
#ifdef S_ISDIR
	    if(S_ISDIR(stat_buf.st_mode))
	    {
		fprintf(stderr,
"%s: Unable to write file, object exists as a directory.\n",
		    filename
		);
		return(-1);
	    }
#endif	/* S_ISDIR */
	}

	/* Open file for writing */
	fp = FOpen(filename, "wb");
	if(fp == NULL)
	    return(-1);

	/* Change permissions so that only owner may read and write */
#if defined(__MSW__)

#else
	if(fchmod(fileno(fp), S_IRUSR | S_IWUSR))
	{
	    fprintf(
		stderr,
		"%s: Warning: Unable to set permissions.\n",
		filename
	    );
	}
#endif

#define PUTCR	fputc('\n', fp);

	/* Write header */
	fprintf(fp,
"# %s - Options\n\
#\n\
#       This file is automatically generated, you should only modify\n\
#       these values from the Options menu.\n\
#\n\
\n",
	    PROG_NAME_FULL
	);

	/* Version */
	fprintf(
	    fp,
	    "Version = %i %i %i",
	    PROG_VERSION_MAJOR, PROG_VERSION_MINOR, PROG_VERSION_RELEASE
	);
	PUTCR

	/* Show menu backgrounds */
	fprintf(
	    fp,
	    "MenuBackgrounds = %i",
	    ((opt->menu_backgrounds) ? 1 : 0)
	);
	PUTCR

	/* Units */
	fprintf(
	    fp,
	    "Units = %i",
	    opt->units
	);
	PUTCR

	/* Textured ground base */
	fprintf(
	    fp,
	    "TexturedGround = %i",
	    opt->textured_ground ? 1 : 0
	);
	PUTCR
	/* Textured objects */
	fprintf(
	    fp,
	    "TexturedObjects = %i",
	    opt->textured_objects ? 1 : 0
	);
	PUTCR
	/* Textured clouds */
	fprintf(
	    fp,
	    "TexturedClouds = %i",
	    opt->textured_clouds ? 1 : 0
	);
	PUTCR
	/* Atmosphere */
	fprintf(
	    fp,
	    "Atmosphere = %i",
	    opt->atmosphere ? 1 : 0
	);
	PUTCR
	/* Dual pass depth */
	fprintf(
	    fp,
	    "DualPassDepth = %i",
	    opt->dual_pass_depth ? 1 : 0
	);
	PUTCR
	/* Prop wash */
	fprintf(
	    fp,
	    "PropWash = %i",
	    opt->prop_wash ? 1 : 0
	);
	PUTCR
	/* Smoke trails */
	fprintf(
	    fp,
	    "SmokeTrails = %i",
	    opt->smoke_trails ? 1 : 0
	);
	PUTCR
	/* GL polygon offset factor */
	fprintf(
	    fp,
	    "GLPolygonOffsetFactor = %f",
	    opt->gl_polygon_offset_factor
	);
	PUTCR
	/* VisibilityMax */
	fprintf(
	    fp,
"# Maximum visibility is computed: miles = 3 + (visibility_max * 3)"
	);
	PUTCR
	fprintf(
	    fp,
	    "VisibilityMax = %i",
	    opt->visibility_max
	);
	PUTCR
	/* RotorBlurStyle */
	fprintf(
	    fp,
	    "RotorBlurStyle = %i",
	    (int)opt->rotor_blur_style
	);
	PUTCR
	/* GraphicsAcceleration */
	fprintf(
	    fp,
"# Graphics acceleration (1.0 = max  0.0 = none/min)"
	);
	PUTCR
	fprintf(
	    fp,
	    "GraphicsAcceleration = %f",
	    opt->graphics_acceleration
	);
	PUTCR
	PUTCR

	/* Engine sounds */
	fprintf(
	    fp,
	    "EngineSounds = %i",
	    opt->engine_sounds ? 1 : 0
	);
	PUTCR
	/* Event sounds */
	fprintf(
	    fp,
	    "EventSounds = %i",
	    opt->event_sounds ? 1 : 0
	);
	PUTCR
	/* Voice sounds */
	fprintf(
	    fp,
	    "VoiceSounds = %i",
	    opt->voice_sounds ? 1 : 0
	);
	PUTCR
	/* Music */
	fprintf(
	    fp,
	    "Music = %i",
	    opt->music ? 1 : 0
	);
	PUTCR
	/* Sound priority */
	fprintf(
	    fp,
	    "SoundPriority = %i",
	    opt->sound_priority
	);
	PUTCR
	PUTCR

	/* Use system time when starting free flights */
	fprintf(
	    fp,
	    "SystemTimeFreeFlight = %i",
	    opt->system_time_free_flight ? 1 : 0
	);
	PUTCR
	PUTCR

	/* HUD color */
	fprintf(
	    fp,
	    "HUDColor = %f %f %f %f",
	    opt->hud_color.a,
	    opt->hud_color.r,
	    opt->hud_color.g,
	    opt->hud_color.b
	);
	PUTCR
	/* Message color */
	fprintf(
	    fp,
	    "MessageColor = %f %f %f %f",
	    opt->message_color.a,
	    opt->message_color.r,
	    opt->message_color.g,
	    opt->message_color.b
	);
	PUTCR
	PUTCR

	/* Smoke puff spawn interval (in miliseconds) */
/* SmokeSpawnInterval is depreciated */
	/* Explosion frame increment interval (in milliseconds) */
	fprintf(
	    fp,
	    "ExplosionFrameInterval = %ld",
	    opt->explosion_frame_int
	);
	PUTCR
	/* Splash frame increment interval (in milliseconds) */
	fprintf(
	    fp,
	    "SplashFrameInterval = %ld",
	    opt->splash_frame_int
	);
	PUTCR
	PUTCR

	/* Smoke puff life span (in milliseconds) */
/* SmokeLifeSpan is depreciated */
	/* Crash caused explosion life span (in milliseconds) */
	fprintf(
	    fp,
	    "CrashExplosionLifeSpan = %ld",
	    opt->crash_explosion_life_span
	);
	PUTCR
	/* Dropped fuel tanks, after landing on ground (in milliseconds) */
	fprintf(
	    fp,
	    "FuelTankLifeSpan = %ld",
	    opt->fuel_tank_life_span
	);
	PUTCR
	PUTCR

	/* Rotor wash visibility coefficient */
	fprintf(
	    fp,
	    "RotorWashVisCoeff = %f",
	    opt->rotor_wash_vis_coeff
	);
	PUTCR
	PUTCR

	/* Game controllers mask */
	fprintf(
	    fp,
	    "# Mask of game controllers; (1 << 0) = keyboard, (1 << 1) = pointer,"
	);
	PUTCR
	fprintf(
	    fp,
	    "# (1 << 2) = joystick"
	);
	PUTCR
	fprintf(
	    fp,
	    "GameControllers = %i",
	    opt->gctl_controllers
	);
	PUTCR
	fprintf(
	    fp,
	    "# Joystick name and SDL GUID. SDL GUID must be unique."
	);
	PUTCR
	if(opt->js0_sdl_name != NULL && strlen(opt->js0_sdl_guid_s) != 0)
	    fprintf(
		fp,
		"Joystick0Name = %s",
		opt->js0_sdl_name
	    );
	else
	    fprintf(
		fp,
		"Joystick0Name = "
	    );
	PUTCR
	fprintf(
	    fp,
	    "Joystick0Guid = %s",
	    opt->js0_sdl_guid_s
	);
	PUTCR
	if(opt->js1_sdl_name != NULL && strlen(opt->js1_sdl_guid_s) != 0)
	    fprintf(
		fp,
		"Joystick1Name = %s",
		opt->js1_sdl_name
	    );
	else
	    fprintf(
		fp,
		"Joystick1Name = "
	    );
	PUTCR
	fprintf(
	    fp,
	    "Joystick1Guid = %s",
	    opt->js1_sdl_guid_s
	);
	PUTCR
	if(opt->js2_sdl_name != NULL && strlen(opt->js2_sdl_guid_s) != 0)
	    fprintf(
		fp,
		"Joystick2Name = %s",
		opt->js2_sdl_name
	    );
	else
	    fprintf(
		fp,
		"Joystick2Name = "
	    );
	PUTCR
	fprintf(
	    fp,
	    "Joystick2Guid = %s",
	    opt->js2_sdl_guid_s
	);
	PUTCR
	/* Joystick button mappings (7 values) */
	fprintf(
	    fp,
"\
# Joystick button mappings.\n\
# Set button number between 0 and 15 included if role is assigned,\n\
# or set to -1 if role is not assigned.\n\
# <rot> <air_brk> <whl_brk> <zm_in> <zm_out> <hoist_up> <hoist_dn>"
	);
	PUTCR
	if(strlen(opt->js0_sdl_guid_s)!= 0)
	    fprintf(
		fp,
		"Joystick0ButtonMappings = %2i %2i %2i %2i %2i %2i %2i",
		opt->js0_btn_rotate,
		opt->js0_btn_air_brakes,
		opt->js0_btn_wheel_brakes,
		opt->js0_btn_zoom_in,
		opt->js0_btn_zoom_out,
		opt->js0_btn_hoist_up,
		opt->js0_btn_hoist_down
	    );
	else
	    fprintf(
		fp,
		"Joystick0ButtonMappings = -1 -1 -1 -1 -1 -1 -1"
	    );
	PUTCR
	if(strlen(opt->js1_sdl_guid_s)!= 0)
	    fprintf(
		fp,
		"Joystick1ButtonMappings = %2i %2i %2i %2i %2i %2i %2i",
		opt->js1_btn_rotate,
		opt->js1_btn_air_brakes,
		opt->js1_btn_wheel_brakes,
		opt->js1_btn_zoom_in,
		opt->js1_btn_zoom_out,
		opt->js1_btn_hoist_up,
		opt->js1_btn_hoist_down
	    );
	else
	    fprintf(
		fp,
		"Joystick1ButtonMappings = -1 -1 -1 -1 -1 -1 -1"
	    );
	PUTCR
	if(strlen(opt->js2_sdl_guid_s)!= 0)
	    fprintf(
		fp,
		"Joystick2ButtonMappings = %2i %2i %2i %2i %2i %2i %2i",
		opt->js2_btn_rotate,
		opt->js2_btn_air_brakes,
		opt->js2_btn_wheel_brakes,
		opt->js2_btn_zoom_in,
		opt->js2_btn_zoom_out,
		opt->js2_btn_hoist_up,
		opt->js2_btn_hoist_down
	    );
	else
	    fprintf(
		fp,
		"Joystick2ButtonMappings = -1 -1 -1 -1 -1 -1 -1"
	    );
	PUTCR
	/* Joystick axes mappings (9 values) */
	fprintf(
	    fp,
"\
# Joystick axis mappings.\n\
# Set axis number between 0 and 7 included if role is assigned,\n\
# or set to -1 if role is not assigned.\n\
# If POV control use a standard POV hat switch,\n\
# set <hat_x> and <hat_y> to -1 and <pov_hat> to hat number (0 or 1).\n\
# If POV control use analog axes,\n\
# set <hat_x> and <hat_y> to axis number and <pov_hat> to -1.\n\
# If axis direction must be inverted, set corresponding inversion value to 1.\n\
# <heading> <pitch> <bank> <throttle> <hat_x> <hat_y> <pov_hat> <brake_left> <brake_right>"
	);
	PUTCR
	if(strlen(opt->js0_sdl_guid_s)!= 0)
	    fprintf(
		fp,
		"Joystick0AxisMappings = %2i %2i %2i %2i %2i %2i %2i %2i %2i",
		opt->js0_axis_heading,
		opt->js0_axis_pitch,
		opt->js0_axis_bank,
		opt->js0_axis_throttle,
		opt->js0_axis_hat_x,
		opt->js0_axis_hat_y,
		opt->js0_pov_hat,
		opt->js0_axis_brake_left,
		opt->js0_axis_brake_right
	    );
	else
	    fprintf(
		fp,
		"Joystick0AxisMappings = -1 -1 -1 -1 -1 -1 -1 -1 -1"
	    );
	PUTCR
	if(strlen(opt->js1_sdl_guid_s)!= 0)
	    fprintf(
		fp,
		"Joystick1AxisMappings = %2i %2i %2i %2i %2i %2i %2i %2i %2i",
		opt->js1_axis_heading,
		opt->js1_axis_pitch,
		opt->js1_axis_bank,
		opt->js1_axis_throttle,
		opt->js1_axis_hat_x,
		opt->js1_axis_hat_y,
		opt->js1_pov_hat,
		opt->js1_axis_brake_left,
		opt->js1_axis_brake_right
	    );
	else
	    fprintf(
		fp,
		"Joystick1AxisMappings = -1 -1 -1 -1 -1 -1 -1 -1 -1"
	    );
	PUTCR
	if(strlen(opt->js2_sdl_guid_s)!= 0)
	    fprintf(
		fp,
		"Joystick2AxisMappings = %2i %2i %2i %2i %2i %2i %2i %2i %2i",
		opt->js2_axis_heading,
		opt->js2_axis_pitch,
		opt->js2_axis_bank,
		opt->js2_axis_throttle,
		opt->js2_axis_hat_x,
		opt->js2_axis_hat_y,
		opt->js2_pov_hat,
		opt->js2_axis_brake_left,
		opt->js2_axis_brake_right
	    );
	else
	    fprintf(
		fp,
		"Joystick2AxisMappings = -1 -1 -1 -1 -1 -1 -1 -1 -1"
	    );
	PUTCR

	fprintf(fp, "JoysticksAxisInversion =");
	if( strlen(opt->js0_sdl_guid_s) != 0 ||
	    strlen(opt->js1_sdl_guid_s) != 0 ||
	    strlen(opt->js2_sdl_guid_s) != 0
	){
	    if(opt->js_axes_inversion_bits & GCTL_JS_AXIS_INV_HEADING)
		fprintf(fp, " 1");
	    else
		fprintf(fp, " 0");
	    if(opt->js_axes_inversion_bits & GCTL_JS_AXIS_INV_PITCH)
		fprintf(fp, "  1");
	    else
		fprintf(fp, "  0");
	    if(opt->js_axes_inversion_bits & GCTL_JS_AXIS_INV_BANK)
		fprintf(fp, "  1");
	    else
		fprintf(fp, "  0");
	    if(opt->js_axes_inversion_bits & GCTL_JS_AXIS_INV_THROTTLE)
		fprintf(fp, "  1");
	    else
		fprintf(fp, "  0");
	    if(opt->js_axes_inversion_bits & GCTL_JS_AXIS_INV_HAT_X)
		fprintf(fp, "  1");
	    else
		fprintf(fp, "  0");
	    if(opt->js_axes_inversion_bits & GCTL_JS_AXIS_INV_HAT_Y)
		fprintf(fp, "  1");
	    else
		fprintf(fp, "  0");
	    if(opt->js_axes_inversion_bits & GCTL_JS_AXIS_INV_POV_HAT)
		fprintf(fp, "  1");
	    else
		fprintf(fp, "  0");
	    if(opt->js_axes_inversion_bits & GCTL_JS_AXIS_INV_BRAKE_LEFT)
		fprintf(fp, "  1");
	    else
		fprintf(fp, "  0");
	    if(opt->js_axes_inversion_bits & GCTL_JS_AXIS_INV_BRAKE_RIGHT)
		fprintf(fp, "  1");
	    else
		fprintf(fp, "  0");
	}
	else
	    fprintf(fp, " 0  0  0  0  0  0  0  0  0");

	PUTCR
	/* Game controller options */
	fprintf(
	    fp,
	    "GameControllerOptions = %i",
	    opt->gctl_options
	);
	PUTCR
	PUTCR


	/* Hoist contact expansion coefficient */
	fprintf(
	    fp,
	    "HoistContactExpansionCoefficient = %f",
	    opt->hoist_contact_expansion_coeff
	);
	PUTCR
	/* Damage resistance coefficient */
	fprintf(
	    fp,
	    "DamageResistanceCoefficient = %f",
	    opt->damage_resistance_coeff
	);
	PUTCR
	/* Flight physics */
	fprintf(
	    fp,
	    "FlightPhysics = %i",
	    (int)opt->flight_physics_level
	);
	/* Wind */
	PUTCR
	fprintf(
	    fp,
	    "Wind = %i",
	    opt->wind
	);
	PUTCR
	PUTCR

	/* Last selected player */
	fprintf(
	    fp,
	    "LastSelectedPlayer = %i",
	    opt->last_selected_player
	);
	PUTCR
	/* Last selected mission */
	fprintf(
	    fp,
	    "LastSelectedMission = %i",
	    opt->last_selected_mission
	);
	PUTCR
	/* Last selected free flight scene */
	fprintf(
	    fp,
	    "LastSelectedFFScene = %i",
	    opt->last_selected_ffscene
	);
	PUTCR
	/* Last selected free flight aircraft */
	fprintf(
	    fp,
	    "LastSelectedFFAircraft = %i",
	    opt->last_selected_ffaircraft
	);
	PUTCR
	/* Last selected free flight weather preset */
	fprintf(
	    fp,
	    "LastSelectedFFWeather = %i",
	    opt->last_selected_ffweather
	);
	PUTCR
	/* Last toplevel window position */
	fprintf(
	    fp,
	    "LastToplevelPosition = %i %i %i %i",
	    opt->last_x, opt->last_y, opt->last_width, opt->last_height
	);
	PUTCR
	/* Last full screen */
	fprintf(
	    fp,
	    "LastFullScreen = %i",
	    opt->last_fullscreen
	);
	PUTCR

#undef PUTCR


	/* Close file */
	FClose(fp);

	return(0);
}
