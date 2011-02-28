#include <string.h>
#include <stdlib.h>
#include <sys/types.h>

#include "../include/string.h"
#include "../include/strexp.h"

#include "gw.h"
#include "messages.h"
#include "cmd.h"
#include "sar.h"
#include "scenesound.h"
#include "sartime.h"


void SARCmdOption(SAR_CMD_PROTOTYPE);


#define ATOI(s)         (((s) != NULL) ? atoi(s) : 0)
#define ATOL(s)         (((s) != NULL) ? atol(s) : 0)
#define ATOF(s)         (((s) != NULL) ? (float)atof(s) : 0.0f)
#define STRDUP(s)       (((s) != NULL) ? strdup(s) : NULL)

#define MAX(a,b)        (((a) > (b)) ? (a) : (b))
#define MIN(a,b)        (((a) < (b)) ? (a) : (b))
#define CLIP(a,l,h)     (MIN(MAX((a),(l)),(h)))
#define STRLEN(s)       (((s) != NULL) ? ((int)strlen(s)) : 0)

#define RADTODEG(r)     ((r) * 180.0 / PI)
#define DEGTORAD(d)     ((d) * PI / 180.0)

#define STR_IS_YES(s)	(StringIsYes(s))

#define NOTIFY(s)			\
{ if(SAR_CMD_IS_VERBOSE(flags) &&	\
     (scene != NULL) && ((s) != NULL)	\
  ) { SARMessageAdd(scene, (s)); }	\
}


static char *STRDUP_NUMI(int v, const char *units)
{
	char *s = (char *)malloc(40 + STRLEN(units));
	if(units != NULL)
	    sprintf(s, "%i %s", v, units);
	else
	    sprintf(s, "%i", v);
	return(s);
}
static char *STRDUP_NUMUL(unsigned long v, const char *units)
{
	char *s = (char *)malloc(40 + STRLEN(units));
	if(units != NULL)
	    sprintf(s, "%ld %s", v, units);
	else
	    sprintf(s, "%ld", v);
	return(s);
}
static char *STRDUP_NUMF(float v, const char *units)
{
	char *s = (char *)malloc(40 + STRLEN(units));
	if(units != NULL)
	    sprintf(s, "%f %s", v, units);
	else
	    sprintf(s, "%f", v);
	return(s);
}

/*
 *	Global option modification.
 */
void SARCmdOption(SAR_CMD_PROTOTYPE)
{
	int argc;
	char **argv;
	char *parm, *val, *new_val = NULL;
	sar_core_struct *core_ptr = SAR_CORE(data);
	sar_scene_struct *scene = core_ptr->scene;
	sar_option_struct *opt = &core_ptr->option;

	/* No argument? */
	if(*arg == '\0')
	{
	    NOTIFY(
		"Usage: option <parameter>=<value>"
	    );
	    return;
	}

	/* Parse arguments, format:
	 *
	 * <parameter>=<value>
	 */
	argv = strchrexp(arg, '=', &argc);
	if(argv == NULL)
	    return;

	/* First argument is option/parameter. */
	if(argc < 2)
	{
	    NOTIFY(
		"Usage: option <parameter>=<value>"
	    );
	    strlistfree(argv, argc);
	    return;
	}

	/* Get links to parm and val in the strv array. */
	if(argc > 0)
	    parm = argv[0];
	else
	    parm = NULL;

	if(argc > 1)
	    val = argv[1];
	else
	    val = NULL;

	if((parm == NULL) || (val == NULL))
	{
	    NOTIFY(
		"Usage: option <parameter>=<value>"
	    );
	    strlistfree(argv, argc);
	    return;
	} 

	strstrip(parm);
	strstrip(val);


	/* Begin handling parameter. */

	/* menu_backgrounds */
	if(!strcasecmp(parm, "menu_backgrounds"))
	{
	    Boolean b = STR_IS_YES(val);
	    opt->menu_backgrounds = b;
	    new_val = STRDUP(b ? "On" : "Off");
	}
	/* menu_change_affects */
	else if(!strcasecmp(parm, "menu_change_affects"))
	{
	    Boolean b = STR_IS_YES(val);
	    opt->menu_change_affects = b;
	    new_val = STRDUP(b ? "On" : "Off");
	}
	/* console_quiet */
	else if(!strcasecmp(parm, "console_quiet"))
	{
	    Boolean b = STR_IS_YES(val);
	    opt->console_quiet = b;
	    new_val = STRDUP(b ? "On" : "Off");
	}
	/* internal_debug */
	else if(!strcasecmp(parm, "internal_debug"))
	{
	    Boolean b = STR_IS_YES(val);
	    opt->internal_debug = b;
	    new_val = STRDUP(b ? "On" : "Off");
	}
	/* runtime_debug */
	else if(!strcasecmp(parm, "runtime_debug"))
	{
	    Boolean b = STR_IS_YES(val);
	    opt->runtime_debug = b;
	    new_val = STRDUP(b ? "On" : "Off");
	}
	/* prioritize_memory */
	else if(!strcasecmp(parm, "prioritize_memory"))
	{
	    Boolean b = STR_IS_YES(val);
	    opt->prioritize_memory = b;
	    new_val = STRDUP(b ? "On" : "Off");
	}

	/* units */
	else if(!strcasecmp(parm, "units"))
	{
	    const char *s = val;
	    if(!strcasecmp(s, "metric_") ||
	       !strcasecmp(s, "metric")
	    )
	    {
		opt->units = SAR_UNITS_METRIC_ALT_FEET;
		new_val = STRDUP("METRIC_ALT_FEET");
	    }
	    else if(!strcasecmp(s, "metric"))
	    {
		opt->units = SAR_UNITS_METRIC;
		new_val = STRDUP("METRIC");
	    }
	    else
	    {
		opt->units = SAR_UNITS_ENGLISH;
		new_val = STRDUP("ENGLISH");
	    }
	}

	/* textured_ground */
	else if(!strcasecmp(parm, "textured_ground"))
	{
	    Boolean b = STR_IS_YES(val);
	    opt->textured_ground = b;
	    new_val = STRDUP(b ? "On" : "Off");
	}
	/* textured_clouds */
	else if(!strcasecmp(parm, "textured_clouds"))
	{
	    Boolean b = STR_IS_YES(val);
	    opt->textured_clouds = b;
	    new_val = STRDUP(b ? "On" : "Off");
	}
	/* textured_objects */
	else if(!strcasecmp(parm, "textured_objects"))
	{
	    Boolean b = STR_IS_YES(val);
	    opt->textured_objects = b;
	    new_val = STRDUP(b ? "On" : "Off");
	}
	/* atmosphere */
	else if(!strcasecmp(parm, "atmosphere"))
	{
	    Boolean b = STR_IS_YES(val);
	    opt->atmosphere = b;
	    new_val = STRDUP(b ? "On" : "Off");
	}
	/* dual_pass_depth */
	else if(!strcasecmp(parm, "dual_pass_depth"))
	{
	    Boolean b = STR_IS_YES(val);
	    opt->dual_pass_depth = b;
	    new_val = STRDUP(b ? "On" : "Off");
	}
	/* prop_wash */
	else if(!strcasecmp(parm, "prop_wash"))
	{
	    Boolean b = STR_IS_YES(val);
	    opt->prop_wash = b;
	    new_val = STRDUP(b ? "On" : "Off");
	}
	/* smoke_trails */
	else if(!strcasecmp(parm, "smoke_trails"))
	{
	    Boolean b = STR_IS_YES(val);
	    opt->smoke_trails = b;
	    new_val = STRDUP(b ? "On" : "Off");
	}
	/* celestial_objects */
	else if(!strcasecmp(parm, "celestial_objects"))
	{
	    Boolean b = STR_IS_YES(val);
	    opt->celestial_objects = STR_IS_YES(val);
	    new_val = STRDUP(b ? "On" : "Off");
	}
	/* gl_polygon_offset_factor */
	else if(!strcasecmp(parm, "gl_polygon_offset_factor") ||
		!strcasecmp(parm, "gl_polygon_offset")
	)
	{
	    opt->gl_polygon_offset_factor = ATOF(val);
	    new_val = STRDUP_NUMF(opt->gl_polygon_offset_factor, NULL);
	}
	/* gl_shade_model */
	else if(!strcasecmp(parm, "gl_shade_model"))
	{
	    const char *s = val;
	    if(!strcasecmp(s, "GL_SMOOTH"))
	    {
		opt->gl_shade_model = GL_SMOOTH;
		new_val = STRDUP("GL_SMOOTH");
	    }
	    else
	    {
		opt->gl_shade_model = GL_FLAT;
		new_val = STRDUP("GL_FLAT");
	    }
	}
	/* visibility_max */
	else if(!strcasecmp(parm, "visibility_max"))
	{
	    /* Visibility in miles = (n * 3) + 3 */
	    opt->visibility_max = (int)CLIP(
		(ATOF(val) - 3) / 3,
		0, 6
	    );
	    new_val = STRDUP_NUMI(
		(opt->visibility_max * 3) + 3,
		"miles"
	    );
	}
	/* graphics_acceleration */
	else if(!strcasecmp(parm, "graphics_acceleration"))
	{
	    opt->graphics_acceleration = (float)CLIP(
		ATOF(val), 0.0, 1.0
	    );
	    new_val = STRDUP_NUMF(opt->graphics_acceleration, NULL);
	}

#define UPDATE_SCENE_SOUND	\
{ if(scene != NULL) {		\
 SARSceneSoundUpdate(		\
  core_ptr,			\
  opt->engine_sounds,		\
  opt->event_sounds,		\
  opt->voice_sounds,		\
  opt->music			\
 );				\
} }
	/* engine_sounds */
	else if(!strcasecmp(parm, "engine_sounds"))
	{
	    Boolean b = STR_IS_YES(val);
	    opt->engine_sounds = b;
	    UPDATE_SCENE_SOUND
	    new_val = STRDUP(b ? "On" : "Off");
	}
	/* event_sounds */
	else if(!strcasecmp(parm, "event_sounds"))
	{
	    Boolean b = STR_IS_YES(val);
	    opt->event_sounds = b;
	    UPDATE_SCENE_SOUND
	    new_val = STRDUP(b ? "On" : "Off");
	}
	/* voice_sounds */
	else if(!strcasecmp(parm, "voice_sounds"))
	{
	    Boolean b = STR_IS_YES(val);
	    opt->voice_sounds = b;
	    UPDATE_SCENE_SOUND
	    new_val = STRDUP(b ? "On" : "Off");
	}
	/* music */
	else if(!strcasecmp(parm, "music"))
	{
	    Boolean b = STR_IS_YES(val);
	    opt->music = b;
	    UPDATE_SCENE_SOUND
	    new_val = STRDUP(b ? "On" : "Off");
	}
	/* sound_priority */
	else if(!strcasecmp(parm, "sound_priority"))
	{
	    const char *s = val;
	    if(!strcasecmp(s, "preempt"))
	    {
		opt->sound_priority = SND_PRIORITY_PREEMPT;
		new_val = STRDUP("PREEMPT");
	    }
	    else if(!strcasecmp(s, "foreground"))
	    {
		opt->sound_priority = SND_PRIORITY_FOREGROUND;
		new_val = STRDUP("FOREGROUND");
	    }
	    else
	    {
		opt->sound_priority = SND_PRIORITY_BACKGROUND;
		new_val = STRDUP("BACKGROUND");
	    }
	    UPDATE_SCENE_SOUND
	}
#undef UPDATE_SCENE_SOUND

	/* system_time_free_flight */
	else if(!strcasecmp(parm, "system_time_free_flight"))
	{
	    Boolean b = STR_IS_YES(val);
	    opt->system_time_free_flight = b;
	    new_val = STRDUP(b ? "On" : "Off");
	}

	/* show_hud_text */
	else if(!strcasecmp(parm, "show_hud_text"))
	{
	    Boolean b = STR_IS_YES(val);
	    opt->show_hud_text = b;
	    new_val = STRDUP(b ? "On" : "Off");
	}
	/* show_outside_text */
	else if(!strcasecmp(parm, "show_outside_text"))
	{
	    Boolean b = STR_IS_YES(val);
	    opt->show_outside_text = b;
	    new_val = STRDUP(b ? "On" : "Off");
	}

	/* explosion_frame_int */
	else if(!strcasecmp(parm, "explosion_frame_int"))
	{
	    opt->explosion_frame_int = (time_t)MAX(ATOL(val), 0);
	    new_val = STRDUP_NUMUL(opt->explosion_frame_int, "ms");
	}
	/* splash_frame_int */
	else if(!strcasecmp(parm, "splash_frame_int"))
	{
	    opt->splash_frame_int = (time_t)MAX(ATOL(val), 0);
	    new_val = STRDUP_NUMUL(opt->splash_frame_int, "ms");
	}
	/* crash_explosion_life_span */
	else if(!strcasecmp(parm, "crash_explosion_life_span"))
	{
	    opt->crash_explosion_life_span = (time_t)MAX(ATOL(val), 0);
	    new_val = STRDUP_NUMUL(opt->crash_explosion_life_span, "ms");
	}
	/* fuel_tank_life_span */
	else if(!strcasecmp(parm, "fuel_tank_life_span"))
	{
	    opt->fuel_tank_life_span = (time_t)MAX(ATOL(val), 0);
	    new_val = STRDUP_NUMUL(opt->fuel_tank_life_span, "ms");
	}

	/* rotor_wash_vis_coeff */
	else if(!strcasecmp(parm, "rotor_wash_vis_coeff"))
	{
	    opt->rotor_wash_vis_coeff = (float)CLIP(
		ATOF(val), 0.0, 1.0
	    );
	    new_val = STRDUP_NUMF(opt->rotor_wash_vis_coeff, NULL);
	}

	/* hoist_contact_expansion_coeff */
	else if(!strcasecmp(parm, "hoist_contact_expansion_coeff"))
	{
	    opt->hoist_contact_expansion_coeff = ATOF(val);
	    new_val = STRDUP_NUMF(opt->hoist_contact_expansion_coeff, NULL);
	}
	/* damage_resistance_coeff */
	else if(!strcasecmp(parm, "damage_resistance_coeff"))
	{
	    opt->damage_resistance_coeff = ATOF(val);
	    new_val = STRDUP_NUMF(opt->damage_resistance_coeff, NULL);
	}
	/* flight_physics */
	else if(!strcasecmp(parm, "flight_physics"))
	{
	    if(!strcasecmp(val, "realistic"))
	    {
		opt->flight_physics_level = FLIGHT_PHYSICS_REALISTIC;
		new_val = STRDUP("REALISTIC");
	    }
	    else if(!strcasecmp(val, "moderate"))
	    {
		opt->flight_physics_level = FLIGHT_PHYSICS_MODERATE;
		new_val = STRDUP("MODERATE");
	    }
	    else
	    {
		opt->flight_physics_level = FLIGHT_PHYSICS_EASY;
		new_val = STRDUP("EASY");
	    }
	}

/* Add other option parameters here */


	/* Set new value? */
	if(new_val != NULL)
	{
	    char *s = (char *)malloc(
		(80 + STRLEN(parm) + STRLEN(val)) * sizeof(char)
	    );
	    sprintf(
		s,
"Option \"%s\" set to \"%s\".",
		parm, new_val
	    );
	    NOTIFY(s);
	    free(s);
	}
	else
	{
	    char *s = (char *)malloc(
		(80 + STRLEN(parm)) * sizeof(char)
	    );
	    sprintf(
		s,
"%s: No such option.",
		parm
	    );
	    NOTIFY(s);
	    free(s);
	}

	/* Deallocate exploded argument strings. */
	strlistfree(argv, argc);

	/* Links to parm and val strings are now invalid. */
	parm = NULL;
	val = NULL;

	free(new_val);
	new_val = NULL;
}
