#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/disk.h"

#include "gw.h"
#include "sound.h"
#include "sarreality.h"
#include "messages.h"
#include "obj.h"
#include "objutils.h"
#include "gctl.h"
#include "simop.h"
#include "simutils.h"
#include "textinput.h"
#include "cmd.h"
#include "sar.h"
#include "scenesound.h"
#include "sardrawselect.h"
#include "sarmenuop.h"
#include "sarmenucodes.h"
#include "sarsimend.h"
#include "sarkey.h"
#include "config.h"


/* Prototype for all SARKey*() functions */
#define SAR_KEY_FUNC_PROTOTYPE					\
	sar_core_struct *core_ptr, gw_display_struct *display,	\
	sar_scene_struct *scene, Boolean state


static void SARKeyCameraRefCockpit(SAR_KEY_FUNC_PROTOTYPE);
static void SARKeyCameraRefHoist(SAR_KEY_FUNC_PROTOTYPE);
static void SARKeyCameraRefMap(SAR_KEY_FUNC_PROTOTYPE);
static void SARKeyCameraRefSpot(SAR_KEY_FUNC_PROTOTYPE);
static void SARKeyCameraRefTower(SAR_KEY_FUNC_PROTOTYPE);
static void SARKeyCommand(SAR_KEY_FUNC_PROTOTYPE);

static void SARKeyEscape(SAR_KEY_FUNC_PROTOTYPE);

static void SARKeyUnits(SAR_KEY_FUNC_PROTOTYPE);
static void SARKeyGraphicsAtmosphere(SAR_KEY_FUNC_PROTOTYPE);
static void SARKeyGraphicsTexturedClouds(SAR_KEY_FUNC_PROTOTYPE);
static void SARKeyGraphicsTexturedGround(SAR_KEY_FUNC_PROTOTYPE);
static void SARKeyGraphicsTexturedObjects(SAR_KEY_FUNC_PROTOTYPE);
static void SARKeySoundLevel(SAR_KEY_FUNC_PROTOTYPE);
static void SARKeyMusic(SAR_KEY_FUNC_PROTOTYPE);

static void SARKeyHelpDisplay(SAR_KEY_FUNC_PROTOTYPE);
static void SARKeyPrintScore(SAR_KEY_FUNC_PROTOTYPE);
static void SARKeyTimeCompression(SAR_KEY_FUNC_PROTOTYPE);
static void SARKeyViewNormalize(SAR_KEY_FUNC_PROTOTYPE);
static void SARKeyHoistContact(SAR_KEY_FUNC_PROTOTYPE);
static void SARKeyFlightPhysicsDifficulty(SAR_KEY_FUNC_PROTOTYPE);
static void SARKeyVisibility(SAR_KEY_FUNC_PROTOTYPE);

static void SARKeyAutoPilot(SAR_KEY_FUNC_PROTOTYPE);
static void SARKeyLandingGear(SAR_KEY_FUNC_PROTOTYPE);
static void SARKeyLights(SAR_KEY_FUNC_PROTOTYPE);
static void SARKeyStrobes(SAR_KEY_FUNC_PROTOTYPE);
static void SARKeyFLIR(SAR_KEY_FUNC_PROTOTYPE);
static void SARKeyTextColor(SAR_KEY_FUNC_PROTOTYPE);
static void SARKeyWeatherChange(SAR_KEY_FUNC_PROTOTYPE);
static void SARKeyInterceptWayPoint(SAR_KEY_FUNC_PROTOTYPE);
static void SARKeyEngine(SAR_KEY_FUNC_PROTOTYPE);
static void SARKeyElevatorTrimUp(SAR_KEY_FUNC_PROTOTYPE);
static void SARKeyElevatorTrimDown(SAR_KEY_FUNC_PROTOTYPE);
static void SARKeyTiltRotors(SAR_KEY_FUNC_PROTOTYPE);
static void SARKeyHoistRopeEndSelect(SAR_KEY_FUNC_PROTOTYPE);
static void SARKeyDoor(SAR_KEY_FUNC_PROTOTYPE);

static int SARKeyRestart(SAR_KEY_FUNC_PROTOTYPE);
static void SARKeyRefuelRepair(SAR_KEY_FUNC_PROTOTYPE);
static void SARKeyFuel(SAR_KEY_FUNC_PROTOTYPE);

static void SARKeySendMessage(SAR_KEY_FUNC_PROTOTYPE);

static void SARKeyTimeOfDay(SAR_KEY_FUNC_PROTOTYPE);

void SARKey(
	sar_core_struct *core_ptr,
	int c, Boolean state, long t
);


#define MAX(a,b)        (((a) > (b)) ? (a) : (b))
#define MIN(a,b)        (((a) < (b)) ? (a) : (b))
#define CLIP(a,l,h)     (MIN(MAX((a),(l)),(h)))
#define STRDUP(s)	(((s) != NULL) ? strdup(s) : NULL)

#define PLAY_SOUND(p)						\
{ if((opt->event_sounds) && (recorder != NULL)) {		\
 char *full_path = STRDUP((ISPATHABSOLUTE(p)) ?			\
  (p) : PrefixPaths(dname.global_data, (p))			\
 );								\
 SoundStartPlayVoid(                                            \
  recorder, full_path, 1.0, 1.0, 0, 0                           \
 );                                                             \
 free(full_path);                                               \
} }



/*
 *	Switch camera reference on scene structure to cockpit.
 */
static void SARKeyCameraRefCockpit(SAR_KEY_FUNC_PROTOTYPE)
{
	int prev_camera_ref;

	if((scene == NULL) || !state)
	    return;

	prev_camera_ref = scene->camera_ref;
	scene->camera_ref = SAR_CAMERA_REF_COCKPIT;
	scene->camera_target = scene->player_obj_num;

	if(prev_camera_ref != scene->camera_ref)
	    SARCameraRefTitleSet(scene, "Cockpit");
}

/*                     
 *      Switch camera reference on scene structure to hoist.
 */
static void SARKeyCameraRefHoist(SAR_KEY_FUNC_PROTOTYPE)
{
	int prev_camera_ref;

	if((scene == NULL) || !state)
	    return;

	prev_camera_ref = scene->camera_ref;
	scene->camera_ref = SAR_CAMERA_REF_HOIST;
	scene->camera_target = scene->player_obj_num;

	if(prev_camera_ref != scene->camera_ref)
	    SARCameraRefTitleSet(scene, "Hoist");
}

/*
 *      Switch camera reference on scene structure to map.
 */
static void SARKeyCameraRefMap(SAR_KEY_FUNC_PROTOTYPE)
{
	sar_object_struct *obj_ptr;

	if((scene == NULL) || !state)
	    return;

	scene->camera_ref = SAR_CAMERA_REF_MAP;
	scene->camera_target = -1;

	obj_ptr = scene->player_obj_ptr;
	if(obj_ptr != NULL)
	{
	    sar_position_struct *pos_src = &obj_ptr->pos;
	    sar_position_struct *pos_tar = &scene->camera_map_pos;

	    /* Set initial camera position at location of object */
	    pos_tar->x = pos_src->x;
	    pos_tar->y = pos_src->y;

	    if(pos_tar->z < 1000.0)
		pos_tar->z = 1000.0;
	}

	/* Set mission title as the map camera reference title? */
	if(core_ptr->mission != NULL)
	{
	    sar_mission_struct *mission = core_ptr->mission;
	    if((mission->title != NULL) ? (*mission->title != '\0') : False)
		SARCameraRefTitleSet(scene, mission->title);
	}
	else if((scene->title != NULL) ? (*scene->title != '\0') : False)
	{
	    SARCameraRefTitleSet(scene, scene->title);
	}
}


/*
 *      Switch camera reference on scene structure to spot.
 *
 *	If the camera reference is already set to spot then the `next'
 *	object will be set as the camera target.
 */
static void SARKeyCameraRefSpot(SAR_KEY_FUNC_PROTOTYPE)
{
	int obj_num = -1;
	sar_object_struct *obj_ptr;


	if((scene == NULL) || !state)
	    return;

	/* Previous camera reference is spot with set target? */
	if((scene->camera_ref == SAR_CAMERA_REF_SPOT) &&
	   (scene->camera_target > -1)
	)
	{
	    /* Camera reference was already spot, so cycle camera target
	     * to the `next' object.
	     */
	    int i, n, type;
	    Boolean seek_backwards = display->shift_key_state;

	    /* Go through objects list twice */
	    for(n = 0; n < 2; n++)
	    {
		/* Go through objects list, starting from the
		 * index value as the current value for the camera
		 * target + 1.
		 */
		i = scene->camera_target + (seek_backwards ? -1 : 1);
		while((i >= 0) && (i < core_ptr->total_objects))
		{
		    obj_ptr = core_ptr->object[i];
		    if(obj_ptr == NULL)
		    {
			seek_backwards ? i-- : i++;
			continue;
		    }

		    /* Get object type */
		    type = obj_ptr->type;

		    /* Match following object types */
		    if((type == SAR_OBJ_TYPE_STATIC) ||
		       (type == SAR_OBJ_TYPE_AUTOMOBILE) ||
		       (type == SAR_OBJ_TYPE_WATERCRAFT) ||
		       (type == SAR_OBJ_TYPE_AIRCRAFT) ||
/*
		       (type == SAR_OBJ_TYPE_RUNWAY) ||
		       (type == SAR_OBJ_TYPE_HELIPAD) ||
 */
		       (type == SAR_OBJ_TYPE_FUELTANK) ||
		       (type == SAR_OBJ_TYPE_HUMAN) ||
		       (type == SAR_OBJ_TYPE_PREMODELED)
		    )
			break;

		    /* No match, seek next index */
		    seek_backwards ? i-- : i++;
		}
		/* Got matched object? */
		if((i >= 0) && (i < core_ptr->total_objects))
		{
		    scene->camera_target = obj_num = i;
		    break;
		}
		else if(i < 0)
		{
		    /* Seeked past bottom, warp to top */
		    scene->camera_target = core_ptr->total_objects;
		}
		else if(i >= core_ptr->total_objects)
		{
		    scene->camera_target = -1;
		}
		else
		{
		    scene->camera_target = -1;
		}
	    }
	}
	else
	{
	    /* Previous camera reference was not spot, so initially
	     * set to spot and target the player object.
	     */
	    scene->camera_ref = SAR_CAMERA_REF_SPOT;
	    scene->camera_target = obj_num = scene->player_obj_num;
	}

	/* At this point obj_num should now be the matched object or -1
	 * on failed match.
	 */

	/* Got matched object? */
	obj_ptr = SARObjGetPtr(
	    core_ptr->object, core_ptr->total_objects, obj_num
	);
	if((obj_ptr != NULL) ? (obj_ptr->name != NULL) : 0)
	{
	    SARCameraRefTitleSet(scene, obj_ptr->name);
	}
	else if(obj_num > -1)
	{
	    char s[80];
	    sprintf(s, "Object #%i", obj_num);
	    SARCameraRefTitleSet(scene, s);
	}
}

/*
 *      Switch camera reference on scene structure to tower.
 *
 *	The position of the tower will be calculated by a call to
 *	SARSimSetFlyByPosition().
 */
static void SARKeyCameraRefTower(SAR_KEY_FUNC_PROTOTYPE)
{
	int prev_camera_ref;


	if((scene == NULL) || !state)
	    return;

	prev_camera_ref = scene->camera_ref;
	scene->camera_ref = SAR_CAMERA_REF_TOWER;
	scene->camera_target = scene->player_obj_num;

	SARSimSetFlyByPosition(
	    scene,
	    &core_ptr->object, &core_ptr->total_objects,
	    scene->player_obj_ptr,
	    &scene->camera_tower_pos
	);

	if(prev_camera_ref != scene->camera_ref)
	    SARCameraRefTitleSet(scene, "Tower");
}

/*
 *	Maps the command prompt, future key events sent to SARKey()
 *	will then be forwarded to SARTextInputHandleKey() until
 *	the command argument is typed in and processed or aborted.
 */
static void SARKeyCommand(SAR_KEY_FUNC_PROTOTYPE)
{
	if(!state)
	    return;

	SARTextInputMap(
	    core_ptr->text_input,
	    "Command", NULL,
	    SARCmdTextInputCB,
	    core_ptr
	);
}


/*
 *	Handles escape key depending on the situation which is checked
 *	in a specific order. The conditions are as follows:
 *
 *	1. Check is help screen is displayed, if so then hide it.
 *
 *	2. Check if any messages are currently being displayed by the
 *	   scene.
 *
 *	3. Check if a mission is in progress, and if so map the prompt
 *	   for confirmation.
 *
 *	4. All else prompt to end simulation.
 */
static void SARKeyEscape(SAR_KEY_FUNC_PROTOTYPE)
{
	if(!state)
	    return;

	/* Displaying help? */
	if(core_ptr->display_help > 0)
	{
	    core_ptr->display_help = 0;
	}
	/* Scene displaying message? */
	else if((scene != NULL) ? (scene->message_display_until > 0) : False)
	{
	    /* Reset message display until time to 0 and delete
	     * all messages on the scene. This will not change
	     * the size of the message pointer array.
	     */
	    SARMessageClearAll(scene);
	}
	/* Scene displaying camera reference title? */
	else if((scene != NULL) ? (scene->camera_ref_title_display_until > 0) : False)
	{
	    SARCameraRefTitleSet(scene, NULL);
	}
	/* Prompt to end mission? */
	else if(core_ptr->mission != NULL)
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


/*
 *	Change units.
 */
static void SARKeyUnits(SAR_KEY_FUNC_PROTOTYPE)
{
	const char *message = NULL;
	sar_option_struct *opt = &core_ptr->option;

	if((scene == NULL) || !state)
	    return;

	switch(opt->units)
	{
	  case SAR_UNITS_ENGLISH:
	    opt->units = SAR_UNITS_METRIC;
	    message = "Units: Metric";
	    break;
	  case SAR_UNITS_METRIC:
	    opt->units = SAR_UNITS_METRIC_ALT_FEET;
	    message = "Units: Metric (Altitude In Feet)";
	    break;
	  case SAR_UNITS_METRIC_ALT_FEET:
	    opt->units = SAR_UNITS_ENGLISH;
	    message = "Units: English";
	    break;
	}

	SARMessageAdd(scene, message);
}

/*
 *      Toggles atmosphere in the global options structure on/off.
 */
static void SARKeyGraphicsAtmosphere(SAR_KEY_FUNC_PROTOTYPE)
{
	const char *message = NULL;
	sar_option_struct *opt = &core_ptr->option;

	if((scene == NULL) || !state)
	    return;

	/* If shift key is held then operation is alternated to be the
	 * celestial toggle.
	 */
	if(display->shift_key_state)
	{
	    if(opt->celestial_objects)
	    {
		opt->celestial_objects = False;
		message = "Celestial Objects: Off";
	    }
	    else
	    {
		opt->celestial_objects = True;
		message = "Celestial Objects: On";
	    }
	}
	else
	{
	    if(opt->atmosphere)
	    {
		opt->atmosphere = False;
		message = "Atmosphere: Off";
	    }
	    else
	    {
		opt->atmosphere = True;
		message = "Atmosphere: On";
	    }
	}

	SARMessageAdd(scene, message);
}

/*
 *	Toggles textured cloud layers in the global options structure
 *	on/off.
 *
 *	Note that this basically specifies if cloud layers should be
 *	drawn since cloud layers are always textured. If camera is
 *	above highest cloud layer then the highest cloud layer will
 *	always be drawn.
 */
static void SARKeyGraphicsTexturedClouds(SAR_KEY_FUNC_PROTOTYPE)
{
	const char *message = NULL;
	sar_option_struct *opt = &core_ptr->option;

	if((scene == NULL) || !state)
	    return;

	/* If shift key is held then operation is alternated to be the
	 * prop wash toggle.
	 */
	if(display->shift_key_state)
	{
	    if(opt->prop_wash)
	    {
		opt->prop_wash = False;
		message = "Prop Wash: Off";
	    }
	    else
	    {
		opt->prop_wash = True;
		message = "Prop Wash: On";
	    }
	}
	else
	{
	    if(opt->textured_clouds)
	    {
		opt->textured_clouds = False;
		message = "Textured Clouds: Off";
	    }
	    else
	    {
		opt->textured_clouds = True;
		message = "Textured Clouds: On";
	    }
	}

	SARMessageAdd(scene, message);
}

/*
 *	Toggles textured ground in the global options structure on/off.
 */
static void SARKeyGraphicsTexturedGround(SAR_KEY_FUNC_PROTOTYPE)
{
	const char *message = NULL;
	sar_option_struct *opt = &core_ptr->option;

	if((scene == NULL) || !state)
	    return;

	/* If shift key is held then operation is alternated to be the
	 * dual pass depth toggle.
	 */
	if(display->shift_key_state)
	{
	    if(opt->dual_pass_depth)
	    {
		opt->dual_pass_depth = False;
		message = "Dual Pass Depth: Off";
	    }
	    else
	    {
		opt->dual_pass_depth = True;
		message = "Dual Pass Depth: On";
	    }
	}
	else
	{
	    if(opt->textured_ground)
	    {
		opt->textured_ground = False;
		message = "Ground Texture: Off";
	    }
	    else
	    {
		opt->textured_ground = True;
		message = "Ground Texture: On";
	    }
	}

	SARMessageAdd(scene, message);
}

/*
 *      Toggles textured objects in the global options structure on/off.
 */
static void SARKeyGraphicsTexturedObjects(SAR_KEY_FUNC_PROTOTYPE)
{
	const char *message = NULL;
	sar_option_struct *opt = &core_ptr->option;

	if((scene == NULL) || !state)
	    return;

	/* If shift key is held then operation is alternated to be the
	 * smoke trails toggle.
	 */
	if(display->shift_key_state)
	{
	    if(opt->smoke_trails)
	    {
		opt->smoke_trails = False;
		message = "Smoke Trails: Off";
	    }
	    else
	    {
		opt->smoke_trails = True;
		message = "Smoke Trails: On";
	    }
	}
	else
	{
	    if(opt->textured_objects)
	    {
		opt->textured_objects = False;
		message = "Object Texture: Off";
	    }
	    else
	    {
		opt->textured_objects = True;
		message = "Object Texture: On";
	    }
	}

	SARMessageAdd(scene, message);
}

/*
 *	Changes the sound level.
 */
static void SARKeySoundLevel(SAR_KEY_FUNC_PROTOTYPE)
{
	const char *message = NULL;
	sar_option_struct *opt = &core_ptr->option;

	if((scene == NULL) || !state)
	    return;

	/* Was all off? */
	if(!opt->engine_sounds && !opt->event_sounds &&
	   !opt->voice_sounds
	)
	{
	    opt->event_sounds = True;
	    message = "Sound: Events";
	}
	/* Was events only? */
	else if(!opt->engine_sounds && opt->event_sounds &&
		!opt->voice_sounds 
	)
	{
	    opt->engine_sounds = True;
	    message = "Sound: Events and Engine";
	}
	/* Was events and engine? */
	else if(opt->engine_sounds && opt->event_sounds &&
		!opt->voice_sounds
	)
	{
	    opt->voice_sounds = True;
	    message = "Sound: Events, Engine, and Voice";
	}
	/* All else assume all on */
	else
	{
	    opt->engine_sounds = False;
	    opt->event_sounds = False;
	    opt->voice_sounds = False;
	    message = "Sound: Off";
	}

	SARSceneSoundUpdate(
	    core_ptr,
	    opt->engine_sounds,
	    opt->event_sounds,
	    opt->voice_sounds,
	    opt->music
	);
	SARMessageAdd(scene, message);
}

/*
 *	Toggles music on/off.
 */
static void SARKeyMusic(SAR_KEY_FUNC_PROTOTYPE)
{
	const char *message = NULL;
	sar_option_struct *opt = &core_ptr->option;

	if((scene == NULL) || !state)
	    return;

	/* Was music on? */
	if(opt->music)
	{
	    opt->music = False;
	    message = "Music: Off";
	}
	else
	{
	    opt->music = True;
	    message = "Music: On";
	}

	SARSceneSoundUpdate(
	    core_ptr,
	    opt->engine_sounds,
	    opt->event_sounds,
	    opt->voice_sounds,
	    opt->music
	);
	SARMessageAdd(scene, message);
}

/*
 *	Toggles the value display_help on the core structure to display
 *	the next page.
 *
 *	This specifies if the help screen should be drawn or not and which
 *	page number.
 */
static void SARKeyHelpDisplay(SAR_KEY_FUNC_PROTOTYPE)
{
	if(!state)
	    return;

	/* Increment or decrement display_help value to indicate current
	 * page being displayed.
	 *
	 * Remember that help page index starts at 1 not 0.
	 *
	 * If increment exceeds maximum page, then it will be sanitized
	 * in the SARDraw*() functions (not here).
	 */
	if(display->shift_key_state)
	    core_ptr->display_help--;
	else
	    core_ptr->display_help++;

	if(core_ptr->display_help < 0)
	    core_ptr->display_help = 0;
}

/*
 *	Prints score, mission status, and passengers on board player
 *	aircraft.
 */
static void SARKeyPrintScore(SAR_KEY_FUNC_PROTOTYPE)
{
	sar_object_struct *obj_ptr;

	if((scene == NULL) || !state)
	    return;

	/* Get player object pointer from scene structure */
	obj_ptr = scene->player_obj_ptr;
	if(obj_ptr != NULL)
	{
	    if(display->shift_key_state)
	    {
		/* Debug case, print player position stats to stdout */
		sar_direction_struct *dir = &obj_ptr->dir;
		sar_position_struct *pos = &obj_ptr->pos;

		/* Print position (for debugging) */
		printf(
 "%i:%2i Player: (hpb) %.0f %.0f %.0f  (xyz) %.2f %.2f %.2f(%.2f feet)\n",
		    (int)(scene->tod / 3600),
		    (int)((int)((int)scene->tod / 60) % 60),
		    SFMRadiansToDegrees(dir->heading),
		    SFMRadiansToDegrees(dir->pitch),
		    SFMRadiansToDegrees(dir->bank),
		    pos->x, pos->y, pos->z,
		    SFMMetersToFeet(pos->z)
		);
	    }
	    else
	    {
		/* Print score, mission status, and occupants on
		 * the given player object obj_ptr. If core_ptr->mission
		 * is NULL then only occupants will be printed.
		 */
		SARMissionPrintStats(
		    core_ptr, scene, core_ptr->mission,
		    obj_ptr
		);
	    }
	}
}

/*
 *	Adjusts the time compression.
 */
static void SARKeyTimeCompression(SAR_KEY_FUNC_PROTOTYPE)
{
	float *tc = &time_compression;

	if((scene == NULL) || !state)
	    return;

	/* Restore time compression to 1.0? */
	if(display->ctrl_key_state)
	{
	    *tc = 1.0f;
	}
	else
	{
	    if(display->shift_key_state)
	    {
		/* Increase time compression */
		if(*tc >= 1.0f)
		{
		    *tc += 1.0f;
		}
		else
		{
		    *tc += 0.25f;
		    if(*tc > 1.0f)
			*tc = 1.0f;
		}
	    }
	    else
	    {
		/* Decrease time compression */
		if(*tc > 1.0f)
		{
		    *tc -= 1.0f;
		    if(*tc < 1.0f)
			*tc = 1.0f;
		}
		else
		{
		    *tc -= 0.25f;
		}
	    }
	}

	/* Clip time compression */
	if(*tc < 0.25f)
	    *tc = 0.25f;
	else if(*tc > 8.0f)
	    *tc = 8.0f;
}

/*
 *	Normalizes the zoom and position of the view with respect to
 *	the current camera_ref on the scene.
 */
static void SARKeyViewNormalize(SAR_KEY_FUNC_PROTOTYPE)
{
	sar_object_struct *obj_ptr;

	if((scene == NULL) || !state)
	    return;

	obj_ptr = scene->player_obj_ptr;	/* Get player object */

	/* Handle by camera reference */
	switch(scene->camera_ref)
	{
	  case SAR_CAMERA_REF_HOIST:
	    if(obj_ptr != NULL)
	    {
		float contact_radius = (float)SARSimGetFlatContactRadius(obj_ptr);
		sar_direction_struct *dir = &scene->camera_hoist_dir;

		scene->camera_hoist_dist = (float)MAX(contact_radius, 10.0);
		dir->heading = (float)(1.0f * PI);
		dir->pitch = (float)(1.95f * PI);
		dir->bank = (float)(0.0f * PI);
	    }
	    break;

	  case SAR_CAMERA_REF_MAP:
	    if(obj_ptr != NULL)
	    {
		sar_position_struct *pos_src = &obj_ptr->pos;
		sar_position_struct *pos_tar = &scene->camera_map_pos;

		/* Set camera position at location of object and keep
		 * the camera position at least 1000 meters up
		 */
		pos_tar->x = pos_src->x;
		pos_tar->y = pos_src->y;

		if(pos_tar->z < 1000.0f)
		    pos_tar->z = 1000.0f;
	    }
	    break;

	  case SAR_CAMERA_REF_TOWER:
	    if(obj_ptr != NULL)
	    {
		SARSimSetFlyByPosition(
		    scene,
		    &core_ptr->object, &core_ptr->total_objects,
		    obj_ptr,
		    &scene->camera_tower_pos
		);
	    }
	    break;

	  case SAR_CAMERA_REF_SPOT:
	    if(obj_ptr != NULL)
	    {
		float contact_radius = SARSimGetFlatContactRadius(obj_ptr);
		sar_direction_struct *dir = &scene->camera_spot_dir;

		scene->camera_spot_dist = (float)MAX(contact_radius, 10.0);
		dir->heading = (float)(1.0f * PI);
		dir->pitch = (float)(1.95f * PI);
		dir->bank = (float)(0.0f * PI);
	    }
	    break;

	  case SAR_CAMERA_REF_COCKPIT:
	    if(obj_ptr != NULL)
	    {
		sar_direction_struct *dir = &scene->camera_cockpit_dir;

		dir->heading = (float)(0.0f * PI);
		dir->pitch = (float)(0.0f * PI);
		dir->bank = (float)(0.0f * PI);
	    }
	    break;
	}
}

/*
 *	Adjusts hoist contact.
 */
static void SARKeyHoistContact(SAR_KEY_FUNC_PROTOTYPE)
{
	const char *mesg = SAR_MESG_FLIGHT_PHYSICS_UNSUPPORTED;
	sar_option_struct *opt = &core_ptr->option;
	float *hoist_contact = &opt->hoist_contact_expansion_coeff;

	if((scene == NULL) || !state)
	    return;

	/* Easy? */
	if(*hoist_contact >= 4.0f)
	{
	    *hoist_contact = 2.0f;
	    mesg = "Hoist Contact: Moderate";
	}
	/* Moderate? */
	else if(*hoist_contact >= 2.0f)
	{
	    *hoist_contact = 1.0f;
	    mesg = "Hoist Contact: Authentic";
	}
	/* All else switch to easy */
	else
	{
	    *hoist_contact = 4.0f;
	    mesg = "Hoist Contact: Easy";
	}

	SARMessageAdd(scene, mesg);
}

/*
 *	Adjusts flight physics difficulty.
 */
static void SARKeyFlightPhysicsDifficulty(SAR_KEY_FUNC_PROTOTYPE)
{
	const char *mesg = SAR_MESG_FLIGHT_PHYSICS_UNSUPPORTED;
	sar_option_struct *opt = &core_ptr->option;

	if((scene == NULL) || !state)
	    return;

	switch(opt->flight_physics_level)
	{
	  case FLIGHT_PHYSICS_EASY:
	    opt->flight_physics_level = FLIGHT_PHYSICS_MODERATE;
	    mesg = SAR_MESG_FLIGHT_PHYSICS_MODERATE;
	    break;
	  case FLIGHT_PHYSICS_MODERATE:
	    opt->flight_physics_level = FLIGHT_PHYSICS_REALISTIC;
	    mesg = SAR_MESG_FLIGHT_PHYSICS_REALISTIC;
	    break;
	  case FLIGHT_PHYSICS_REALISTIC:
	    opt->flight_physics_level = FLIGHT_PHYSICS_EASY;
	    mesg = SAR_MESG_FLIGHT_PHYSICS_EASY;
	    break;
	}

	SARMessageAdd(scene, mesg);
}

/*
 *	Sets maximum visibility.
 */
static void SARKeyVisibility(SAR_KEY_FUNC_PROTOTYPE)
{
	char text[256];
	sar_option_struct *opt = &core_ptr->option;

	if((scene == NULL) || !state)
	    return;

	opt->visibility_max += (display->shift_key_state) ? 1 : -1;
	if(opt->visibility_max > 6)
	    opt->visibility_max = 0;
	else if(opt->visibility_max < 0)
	    opt->visibility_max = 6;

	/* The equation to calculate visibility max is:
	 * miles = 3 + (opt->visibility_max * 3)
	 */
	switch(opt->units)
	{
	  case SAR_UNITS_METRIC:
	  case SAR_UNITS_METRIC_ALT_FEET:
	    sprintf(
		text,
		"Visibility Max: %.1f km",
		SFMMilesToMeters(3 + (opt->visibility_max * 3)) / 1000.0f
	    );
	    break;
	  default:	/* SAR_UNITS_ENGLISH */
	    sprintf(
		text,
		"Visibility Max: %i Miles",
		3 + (opt->visibility_max * 3)
	    );
	    break;
	}
	SARMessageAdd(scene, text);
}


/*
 *	Toggle autopilot/autohover or toggle slew mode.
 */
static void SARKeyAutoPilot(SAR_KEY_FUNC_PROTOTYPE)
{
	sar_object_struct *obj_ptr;


	if((scene == NULL) || !state)
	    return;

	/* If ctrl key is held then operation is alternated to be the
	 * slew toggle.
	 */
	if(display->ctrl_key_state)
	{
	    sar_object_aircraft_struct *aircraft;


	    /* Get player object */
	    obj_ptr = scene->player_obj_ptr;
	    aircraft = SAR_OBJ_GET_AIRCRAFT(obj_ptr);

	    /* Object must be an aircraft in order to toggle slew */
	    if(aircraft != NULL)
	    {
		int is_slew = SARSimIsSlew(obj_ptr);
		if(is_slew)
		{
		    /* Was in slew mode, now go into previous flight mode */
		    SARSimSetSlew(obj_ptr, 0);
		}
		else
		{
		    /* Was in some other flight mode, now wanting to enter
		     * enter slew mode. Make sure flight worthyness allows
		     * allows aircraft to go into slew mode
		     */
		    if(aircraft->air_worthy_state == SAR_AIR_WORTHY_FLYABLE)
			SARSimSetSlew(obj_ptr, 1);
		}
	    }
	}
	else
	{
	    sar_object_aircraft_struct *aircraft;


	    /* Get player object */
	    obj_ptr = scene->player_obj_ptr;
	    aircraft = SAR_OBJ_GET_AIRCRAFT(obj_ptr);

	    /* Object must be an aircraft in order to toggle autopilot */
	    if(aircraft != NULL)
	    {
		if(aircraft->autopilot_state == SAR_AUTOPILOT_ON)
		{
		    aircraft->autopilot_state = SAR_AUTOPILOT_OFF;
		}
		else
		{
		    aircraft->autopilot_state = SAR_AUTOPILOT_ON;
		    aircraft->autopilot_altitude = obj_ptr->pos.z;
		}
	    }
	}
}

/*
 *	Raise/lower landing gears on player object.
 */
static void SARKeyLandingGear(SAR_KEY_FUNC_PROTOTYPE)
{
	sar_object_struct *obj_ptr;
	const sar_option_struct *opt = &core_ptr->option;

	if((scene == NULL) || !state)
	    return;

	obj_ptr = scene->player_obj_ptr;
	if(obj_ptr != NULL)
	{
	    int state = SARObjLandingGearState(obj_ptr);
	    if(state > -1)
		SARSimOpLandingGear(
		    scene,
		    &core_ptr->object, &core_ptr->total_objects,
		    obj_ptr, (state == 0) ? 1 : 0,
		    core_ptr->recorder, opt->event_sounds,
		    SAR_IS_EAR_IN_COCKPIT(scene)
		);
	}
}

/*
 *      Turns lights (vector lights and spot lights) on/off on the player
 *	object.
 */
static void SARKeyLights(SAR_KEY_FUNC_PROTOTYPE)
{
	sar_object_struct *obj_ptr;
	sar_object_aircraft_struct *aircraft;

	if((scene == NULL) || !state)
	    return;

	obj_ptr = scene->player_obj_ptr;
	aircraft = SAR_OBJ_GET_AIRCRAFT(obj_ptr);
	if(aircraft != NULL)
	{
	    if(aircraft->air_worthy_state != SAR_AIR_WORTHY_NOT_FLYABLE)
	    {
		if(display->ctrl_key_state)
		{
		    /* Reset spot light direction */
		    sar_direction_struct *dir = &aircraft->spotlight_dir;
		    dir->heading = (float)SFMDegreesToRadians(
			SAR_DEF_SPOTLIGHT_HEADING
		    );
		    dir->pitch = (float)SFMDegreesToRadians(
			SAR_DEF_SPOTLIGHT_PITCH
		    );
		    dir->bank = (float)SFMDegreesToRadians(
			SAR_DEF_SPOTLIGHT_BANK
		    );
		}
		else if(display->shift_key_state)
		{
		    /* Toggle spot light only */
		    int state = SARSimGetAttenuateState(obj_ptr);
		    SARSimOpAttenuate(obj_ptr, !state);
		}
		else
		{
		    /* Toggle lighting and spot light */
		    int state = SARSimGetLightsState(obj_ptr);
		    SARSimOpLights(obj_ptr, !state);
		    SARSimOpAttenuate(obj_ptr, !state);
		}
	    }
	}
}

/*
 *	Turns strobe lights on/off on the player object.
 */
static void SARKeyStrobes(SAR_KEY_FUNC_PROTOTYPE)
{
	sar_object_struct *obj_ptr;
	sar_object_aircraft_struct *aircraft;

	if((scene == NULL) || !state)
	    return;

	obj_ptr = scene->player_obj_ptr;
	aircraft = SAR_OBJ_GET_AIRCRAFT(obj_ptr);
	if(aircraft != NULL)
	{
	    if(aircraft->air_worthy_state != SAR_AIR_WORTHY_NOT_FLYABLE)
	    {
		int state = SARSimGetStrobesState(obj_ptr);
		SARSimOpStrobes(obj_ptr, !state);
	    }
	}
}

/*
 *	Toggles FLIR (night vision) on/off.
 */
static void SARKeyFLIR(SAR_KEY_FUNC_PROTOTYPE)
{
	snd_recorder_struct *recorder = core_ptr->recorder;
	const sar_option_struct *opt = &core_ptr->option;

	if(!state)
	    return;

	/* Toggle FLIR */
	core_ptr->flir = !core_ptr->flir;

	/* Play sound */
	PLAY_SOUND(
	    (core_ptr->flir) ?
		SAR_DEF_SOUND_FLIR_ON : SAR_DEF_SOUND_FLIR_OFF
	);
}

/*
 *	Brightens or darkens text color on scene.
 */
static void SARKeyTextColor(SAR_KEY_FUNC_PROTOTYPE)
{
	float g;		/* Gamma and base coeff */
	sar_color_struct *c;
	sar_option_struct *opt = &core_ptr->option;

	if((scene == NULL) || !state)
	    return;

	/* Adjust message text color, get just the gamma from the
	 * red compoent (since it should be greyscale).
	 */
	c = &opt->message_color;
	if(display->shift_key_state)
	    g = MAX(c->r - 0.1f, 0.0f);
	else
	    g = MIN(c->r + 0.1f, 1.0f);

	/* Set new hud and message text color */
	SARSetGlobalTextColorBrightness(core_ptr, g);
}

/*
 *	Change weather.
 */
static void SARKeyWeatherChange(SAR_KEY_FUNC_PROTOTYPE)
{
	int i, spin_num;
	sar_weather_data_struct *weather_data;
	sar_weather_data_entry_struct *wdp_ptr;
	sar_menu_struct *m;
	sar_menu_spin_struct *spin_ptr;
	sar_option_struct *opt = &core_ptr->option;

	if((scene == NULL) || !state)
	    return;

	/* Get list of weather settings */
	weather_data = core_ptr->weather_data;
	if(weather_data != NULL)
	{
	    /* Get current weather preset index and increment by 1 */
	    if(display->shift_key_state)
		i = opt->last_selected_ffweather - 1;
	    else
		i = opt->last_selected_ffweather + 1;

	    /* Cycle selection? */
	    if(i >= weather_data->total_presets)
		i = 0;

	    if((i >= 0) && (i < weather_data->total_presets))
		wdp_ptr = weather_data->preset[i];
	    else
		wdp_ptr = NULL;

	    /* Weather data preset structure valid and has a name? */
	    if((wdp_ptr != NULL) ? (wdp_ptr->name != NULL) : False)
	    {
		char text[256 + 80];

		SARWeatherSetScenePreset(
		    weather_data, scene, wdp_ptr->name
		);

		/* Print new weather preset name */
		strcpy(text, "Weather: ");
		strncat(text, wdp_ptr->name, 256);
		if((*text) != '\0')
		    SARMessageAdd(scene, text);
	    }

	    /* Rcord new value */
	    opt->last_selected_ffweather = i;

	    /* Get free flight weather menu */
	    i = SARMatchMenuByName(core_ptr, SAR_MENU_NAME_FREE_FLIGHT_WEATHER);
	    m = (i > -1) ? core_ptr->menu[i] : NULL;

	    spin_ptr = SAR_MENU_SPIN(SARMenuGetObjectByID(
		m, SAR_MENU_ID_MENU_FREE_FLIGHT_WEATHER_CONDITION, &spin_num
	    ));

	    /* Restore previously selected item */
	    spin_ptr->cur_value = opt->last_selected_ffweather;
	}
}


/*
 *	Select next intercept way point.
 */
static void SARKeyInterceptWayPoint(SAR_KEY_FUNC_PROTOTYPE)
{
	sar_object_struct *obj_ptr;

	if((scene == NULL) || !state)
	    return;

	/* If ctrl key is pressed then interprite this as a weather 
	 * change.
	 */
	if(display->ctrl_key_state)
	{
	    SARKeyWeatherChange(core_ptr, display, scene, state);
	    return;
	}


	/* Otherwise change waypoint on player object */

	obj_ptr = scene->player_obj_ptr;
	if(obj_ptr != NULL)
	{
	    int *cur_intercept = NULL, *total_intercepts = NULL;
	    sar_object_aircraft_struct *aircraft;
	    char text[256];


	    *text = '\0';
	    switch(obj_ptr->type)
	    {
	      case SAR_OBJ_TYPE_GARBAGE:
	      case SAR_OBJ_TYPE_STATIC:
	      case SAR_OBJ_TYPE_AUTOMOBILE:
	      case SAR_OBJ_TYPE_WATERCRAFT:
		break;
	      case SAR_OBJ_TYPE_AIRCRAFT:
		aircraft = SAR_OBJ_GET_AIRCRAFT(obj_ptr);
		if(aircraft != NULL)
		{
		    cur_intercept = &aircraft->cur_intercept;
		    total_intercepts = &aircraft->total_intercepts;
		}
		break;
	      case SAR_OBJ_TYPE_GROUND:
	      case SAR_OBJ_TYPE_RUNWAY:
	      case SAR_OBJ_TYPE_HELIPAD:
	      case SAR_OBJ_TYPE_HUMAN:
	      case SAR_OBJ_TYPE_SMOKE:
	      case SAR_OBJ_TYPE_FIRE:
	      case SAR_OBJ_TYPE_EXPLOSION:
	      case SAR_OBJ_TYPE_CHEMICAL_SPRAY:
	      case SAR_OBJ_TYPE_FUELTANK:
	      case SAR_OBJ_TYPE_PREMODELED:
		break;
	    }

	    if((cur_intercept != NULL) && (total_intercepts != NULL))
	    {
		/* Change intercept number */
		if(display->shift_key_state)
		    *cur_intercept = (*cur_intercept) - 1;
		else
		    *cur_intercept = (*cur_intercept) + 1;

		/* Cycle */
		if(*cur_intercept >= *total_intercepts)
		    *cur_intercept = 0;
		else if(*cur_intercept < 0)
		    *cur_intercept = *total_intercepts - 1;

		if(*cur_intercept < 0)
		    *cur_intercept = 0;

		if(*total_intercepts > 0)
		{
		    int i = (*cur_intercept) + 1;
		    sprintf(
			text,
			"Selected waypoint %i%s",
			i,
			(i == (*total_intercepts)) ? " (last)" : ""
		    );
		}
		else
		    strcpy(text, "No waypoints set");

		if((*text) != '\0')
		    SARMessageAdd(scene, text);
	    }
	}
}

/*
 *	Turns engine off or initializes engine on the player object.
 */
static void SARKeyEngine(SAR_KEY_FUNC_PROTOTYPE)
{
	sar_object_struct *obj_ptr;
	sar_object_aircraft_struct *aircraft;
	sar_option_struct *opt = &core_ptr->option;

	if((scene == NULL) || !state)
	    return;

	/* Get player object */
	obj_ptr = scene->player_obj_ptr;
	aircraft = SAR_OBJ_GET_AIRCRAFT(obj_ptr);
	if((obj_ptr != NULL) && (aircraft != NULL))
	{
	    /* Get current engine state and then change it */
	    sar_engine_state engine_state = SARSimGetEngineState(obj_ptr);
	    if(((engine_state == SAR_ENGINE_ON) ||
	        (engine_state == SAR_ENGINE_INIT)) &&
	       display->shift_key_state
	    )
	    {
		SARSimOpEngine(
		    scene,
		    &core_ptr->object, &core_ptr->total_objects,
		    obj_ptr, SAR_ENGINE_OFF,
		    core_ptr->recorder, opt->event_sounds,
		    SAR_IS_EAR_IN_COCKPIT(scene)
		);
	    }
	    else if(!display->shift_key_state)
	    {
		SARSimOpEngine(
		    scene,
		    &core_ptr->object, &core_ptr->total_objects,
		    obj_ptr, SAR_ENGINE_ON,
		    core_ptr->recorder, opt->event_sounds,
		    SAR_IS_EAR_IN_COCKPIT(scene)
		);
	    }
	}
}

/*
 *	Increases elevator trim on player object.
 */
static void SARKeyElevatorTrimUp(SAR_KEY_FUNC_PROTOTYPE)
{
	sar_object_struct *obj_ptr;

	if((scene == NULL) || !state)
	    return;

	obj_ptr = scene->player_obj_ptr;
	if(obj_ptr != NULL)
	{
	    sar_object_aircraft_struct *aircraft;

	    switch(obj_ptr->type)
	    {
	      case SAR_OBJ_TYPE_GARBAGE:
	      case SAR_OBJ_TYPE_STATIC:
	      case SAR_OBJ_TYPE_AUTOMOBILE:
	      case SAR_OBJ_TYPE_WATERCRAFT:
		break;
	      case SAR_OBJ_TYPE_AIRCRAFT:
		aircraft = SAR_OBJ_GET_AIRCRAFT(obj_ptr);
		if(aircraft != NULL)
		{
		    /* In slew mode, this function moves the aircraft down */
		    if(aircraft->flight_model_type == SAR_FLIGHT_MODEL_SLEW)
		    {
			obj_ptr->pos.z -= (float)SFMFeetToMeters(
			    (display->shift_key_state) ? 250.0 : 25.0
			);
			SARSimWarpObject(
			    scene, obj_ptr,
			    &obj_ptr->pos, &obj_ptr->dir
			);
		    }
		    /* All else is regular elevator trim adjusting */
		    else
		    {
			if(display->ctrl_key_state)
			    aircraft->elevator_trim = 0.0f;
			else
			    aircraft->elevator_trim -= (float)(
				(display->shift_key_state) ? 0.1 : 0.025
			    );
			if(aircraft->elevator_trim < -1.0f)
			    aircraft->elevator_trim = -1.0f;
		    }
		}
		break;
	      case SAR_OBJ_TYPE_GROUND:
	      case SAR_OBJ_TYPE_RUNWAY:
	      case SAR_OBJ_TYPE_HELIPAD:
	      case SAR_OBJ_TYPE_HUMAN:
	      case SAR_OBJ_TYPE_SMOKE:
	      case SAR_OBJ_TYPE_FIRE:
	      case SAR_OBJ_TYPE_EXPLOSION:
	      case SAR_OBJ_TYPE_CHEMICAL_SPRAY:
	      case SAR_OBJ_TYPE_FUELTANK:
	      case SAR_OBJ_TYPE_PREMODELED:
		break;
	    }
	}
}

/*
 *	Decreases elevator trim on player object.
 */
static void SARKeyElevatorTrimDown(SAR_KEY_FUNC_PROTOTYPE)
{
	sar_object_struct *obj_ptr;

	if((scene == NULL) || !state)
	    return;

	obj_ptr = scene->player_obj_ptr;
	if(obj_ptr != NULL)
	{
	    sar_object_aircraft_struct *aircraft;
  
	    switch(obj_ptr->type)
	    {
	      case SAR_OBJ_TYPE_GARBAGE:
	      case SAR_OBJ_TYPE_STATIC:
	      case SAR_OBJ_TYPE_AUTOMOBILE:
	      case SAR_OBJ_TYPE_WATERCRAFT:
		break;
	      case SAR_OBJ_TYPE_AIRCRAFT:
		aircraft = SAR_OBJ_GET_AIRCRAFT(obj_ptr);
		if(aircraft != NULL)
		{
		    /* In slew mode, this function moves the aircraft up */
		    if(aircraft->flight_model_type == SAR_FLIGHT_MODEL_SLEW)
		    {
			obj_ptr->pos.z += (float)SFMFeetToMeters(
			    (display->shift_key_state) ? 250.0 : 25.0
			);
			SARSimWarpObject(
			    scene, obj_ptr,
			    &obj_ptr->pos, &obj_ptr->dir
			);
		    }
		    /* All else is regular elevator trim adjusting */
		    else
		    {
			if(display->ctrl_key_state)
			    aircraft->elevator_trim = 0.0f;
			else
			    aircraft->elevator_trim += (float)(
				(display->shift_key_state) ? 0.1 : 0.025
			    );
			if(aircraft->elevator_trim > 1.0f)
			    aircraft->elevator_trim = 1.0f;
		    }
		}
		break;
	      case SAR_OBJ_TYPE_GROUND:
	      case SAR_OBJ_TYPE_RUNWAY:
	      case SAR_OBJ_TYPE_HELIPAD:
	      case SAR_OBJ_TYPE_HUMAN:
	      case SAR_OBJ_TYPE_SMOKE:
	      case SAR_OBJ_TYPE_FIRE:
	      case SAR_OBJ_TYPE_EXPLOSION:
	      case SAR_OBJ_TYPE_CHEMICAL_SPRAY:
	      case SAR_OBJ_TYPE_FUELTANK:
	      case SAR_OBJ_TYPE_PREMODELED:
		break;
	    }
	}
}


/*
 *	Tilt rotors/pitch engines.
 */
static void SARKeyTiltRotors(SAR_KEY_FUNC_PROTOTYPE)
{
	sar_object_struct *obj_ptr;


	if((scene == NULL) || !state)
	    return;

	obj_ptr = scene->player_obj_ptr;
	if(obj_ptr != NULL)
	{
	    sar_object_aircraft_struct *aircraft = SAR_OBJ_GET_AIRCRAFT(obj_ptr);
	    if(aircraft != NULL)
	    {
		/* Can rotors pitch? */
		if(aircraft->engine_can_pitch == 1)
		{
		    /* Change flight model */
		    switch(aircraft->flight_model_type)
		    {
		      case SAR_FLIGHT_MODEL_AIRPLANE:
			SARSimPitchEngine(
			    scene,
			    &core_ptr->object, &core_ptr->total_objects,
			    obj_ptr,
			    0		/* Pitch up */
			);
/* SARMessageAdd(scene, "Helicopter Mode"); */
			break;

		       /* All else assume helicopter */
		       default:
			SARSimPitchEngine(
			    scene,
			    &core_ptr->object, &core_ptr->total_objects,
			    obj_ptr,
			    1           /* Pitch forward */
			);
/* SARMessageAdd(scene, "Airplane Mode"); */
			break;
		    }
		}
	    }
	}
}

/*
 *	Selects different thing (rescue basket or diver) to put at the
 *	end of the player object's rescue hoist rope.
 */
static void SARKeyHoistRopeEndSelect(SAR_KEY_FUNC_PROTOTYPE)
{
	sar_object_struct *obj_ptr;


	if((scene == NULL) || !state)
	    return;

	obj_ptr = scene->player_obj_ptr;
	if(obj_ptr != NULL)
	{
	    sar_obj_hoist_struct *hoist = SARObjGetHoistPtr(obj_ptr, 0, NULL);
	    if(hoist != NULL)
	    {
		sar_hoist_deployment_flags	deployments = hoist->deployments,
						cur_deployment = hoist->cur_deployment;
		const char *message = NULL;

		/* Unable to select if hoist's rope is already out */
		if(hoist->rope_cur > 0.0f)
		{
		    message = SAR_MESG_HOIST_END_SELECT_ROPE_OUT;
		}
		else
		{
		    if(cur_deployment == SAR_HOIST_DEPLOYMENT_BASKET)
		    {
			if(deployments & SAR_HOIST_DEPLOYMENT_DIVER)
			    hoist->cur_deployment = SAR_HOIST_DEPLOYMENT_DIVER;
			else if(deployments & SAR_HOIST_DEPLOYMENT_HOOK)
			    hoist->cur_deployment = SAR_HOIST_DEPLOYMENT_HOOK;
		    }
		    else if(cur_deployment == SAR_HOIST_DEPLOYMENT_DIVER)
		    {
			if(deployments & SAR_HOIST_DEPLOYMENT_HOOK)
			    hoist->cur_deployment = SAR_HOIST_DEPLOYMENT_HOOK;
			else if(deployments & SAR_HOIST_DEPLOYMENT_BASKET)
			    hoist->cur_deployment = SAR_HOIST_DEPLOYMENT_BASKET;
		    }
		    else if(cur_deployment == SAR_HOIST_DEPLOYMENT_HOOK)
		    {
			if(deployments & SAR_HOIST_DEPLOYMENT_BASKET)
			    hoist->cur_deployment = SAR_HOIST_DEPLOYMENT_BASKET;
			else if(deployments & SAR_HOIST_DEPLOYMENT_DIVER)
			    hoist->cur_deployment = SAR_HOIST_DEPLOYMENT_DIVER;
		    }
		    cur_deployment = hoist->cur_deployment;

		    switch(cur_deployment)
		    {
		      case SAR_HOIST_DEPLOYMENT_BASKET:
			message = SAR_MESG_HOIST_END_SELECT_BASKET;
			break;
		      case SAR_HOIST_DEPLOYMENT_DIVER:
			message = SAR_MESG_HOIST_END_SELECT_DIVER;
			break;
		      case SAR_HOIST_DEPLOYMENT_HOOK:
			message = SAR_MESG_HOIST_END_SELECT_HOOK;
			break;
		    }
		}

		if(message != NULL)
		    SARMessageAdd(scene, message);
	    }
	}
}

/*
 *	Open/close main door on player object.
 */
static void SARKeyDoor(SAR_KEY_FUNC_PROTOTYPE)
{
	sar_object_struct *obj_ptr;


	if((scene == NULL) || !state)
	    return;

	obj_ptr = scene->player_obj_ptr;
	if(obj_ptr != NULL)
	{
	    sar_obj_hoist_struct *hoist = SARObjGetHoistPtr(obj_ptr, 0, NULL);
	    sar_obj_part_struct *door_ptr = SARObjGetPartPtr(
		obj_ptr, SAR_OBJ_PART_TYPE_DOOR_RESCUE, 0
	    );

	    if(door_ptr != NULL)
	    {
		/* Door opened and hoist out? */
		if(((hoist == NULL) ? 0 : (hoist->rope_cur > 0.0)) &&
		   (door_ptr->flags & SAR_OBJ_PART_FLAG_STATE)
		)
		{
		    /* Door is opened and hoist is out, cannot close
		     * door.
		     */
		    SARMessageAdd(
			scene,
			SAR_MESG_CANNOT_CLOSE_DOOR_BASKET
		    );
		}
		else
		{
		    /* Check if door was opened? */
		    if(door_ptr->flags & SAR_OBJ_PART_FLAG_STATE)
		    {
			/* Since closing door we need to abort any
			 * passengers leaving.
			 */
			SARSimOpPassengersSetLeave(
			    scene, obj_ptr, 0, 0
			);
		    }

		    /* This is an explicit door operation, so if opening
		     * the door we need to set the the
		     * SAR_OBJ_PART_FLAG_DOOR_STAY_OPEN flag on it.
		     */
		    if(door_ptr->flags & SAR_OBJ_PART_FLAG_STATE)
		    {
			/* Door is already opened, so we're closing
			 * it. When we close the door it will 
			 * automatically remove the 
			 * SAR_OBJ_PART_FLAG_DOOR_STAY_OPEN. flag.
			 */
		    }
		    else
		    {
			/* Door is closed, open it and set the
			 * SAR_OBJ_PART_FLAG_DOOR_STAY_OPEN flag, marking
			 * that the player explicitly opened the door and
			 * it should not automatically start to close on
			 * the next loop.
			 */
			door_ptr->flags |= SAR_OBJ_PART_FLAG_DOOR_STAY_OPEN;
		    }

		    /* Set up rescue door values to begin opening or
		     * closing it.
		     */
		    SARSimOpDoorRescue(
			scene,
			&core_ptr->object, &core_ptr->total_objects,
			obj_ptr,
			!(door_ptr->flags & SAR_OBJ_PART_FLAG_STATE)
		    );
		}
	    }
	}
}

/*
 *      Restart from nearest restarting point or end mission.
 *
 *	Checks if a mission exists but is no longer in progress
 *	(marked success or failed), in which case will end
 *	simulation.
 *
 *	If no mission exists then checks if player object is marked
 *	crashed and moves player to nearest restarting point.
 *
 *	This function returns 1 if the event was handled or 0 if it
 *	was not or error.
 */
static int SARKeyRestart(SAR_KEY_FUNC_PROTOTYPE)
{
	int obj_num;
	sar_object_struct *obj_ptr;


	if((scene == NULL) || !state)
	    return(0);

	/* Begn restarting simulation by the simulation type */

	/* Was there a mission? */
	if(core_ptr->mission != NULL)
	{
	    sar_mission_struct *m = core_ptr->mission;

	    /* Handle by mission state */
	    switch(m->state)
	    {
	      case MISSION_STATE_IN_PROGRESS:
		/* Mission still in progress, do nothing */
		break;

	      case MISSION_STATE_FAILED:
	      case MISSION_STATE_ACCOMPLISHED:
		/* End simulation and tabulate mission results */
		SARSimEnd(core_ptr);
		/* Return indicating event was handled */
		return(1);
		break;

	      default:
		fprintf(
		    stderr,
 "SARKeyRestart(): Unsupported mission state `%i'\n",
		    m->state
	        );
		break;
	    }
	}
	/* No mission (assume this is a free flight), check if player
	 * object is marked crashed.
	 */
	else if(scene->player_has_crashed)
	{
	    /* Reset game controller states */
	    GCtlResetValues(core_ptr->gctl);

	    /* Get references to player objects */
	    obj_num = scene->player_obj_num;
	    obj_ptr = scene->player_obj_ptr;
	    if(obj_ptr != NULL)
	    {
		/* Move player object to nearest restarting point and
		 * repair it.
		 */
		SARSimRestart(
		    core_ptr, scene,
		    &core_ptr->object, &core_ptr->total_objects,
		    obj_num, obj_ptr
		);
		/* Return indicating event was handled */
		return(1);
	    }
	}

	/* Return indicating no event handled */
	return(0);
}

/*
 *	Refuel and repair aircraft.
 */
static void SARKeyRefuelRepair(SAR_KEY_FUNC_PROTOTYPE)
{
	sar_object_struct *obj_ptr;

 
	if((scene == NULL) || !state)
	    return;

	obj_ptr = scene->player_obj_ptr;
	if(obj_ptr != NULL)
	{
	    Boolean	obj_ok = False,
			loc_ok = False,
			can_refuel = False,
			can_repair = False,
			can_dropoff = False;
	    sar_object_aircraft_struct *aircraft;

	    /* Check if object is okay to refuel or repair by its
	     * type
	     */
	    switch(obj_ptr->type)
	    {
	      case SAR_OBJ_TYPE_GARBAGE:
	      case SAR_OBJ_TYPE_STATIC:
	      case SAR_OBJ_TYPE_AUTOMOBILE:
	      case SAR_OBJ_TYPE_WATERCRAFT:
		break;
	      case SAR_OBJ_TYPE_AIRCRAFT:
		aircraft = SAR_OBJ_GET_AIRCRAFT(obj_ptr);
		if(aircraft != NULL)
		{
		    if(aircraft->landed &&
		       (aircraft->air_worthy_state != SAR_AIR_WORTHY_NOT_FLYABLE)
		    )
			obj_ok = True;
		}
		break;
	      case SAR_OBJ_TYPE_GROUND:
	      case SAR_OBJ_TYPE_RUNWAY:
	      case SAR_OBJ_TYPE_HELIPAD:
	      case SAR_OBJ_TYPE_HUMAN:
	      case SAR_OBJ_TYPE_SMOKE:
	      case SAR_OBJ_TYPE_FIRE:
	      case SAR_OBJ_TYPE_EXPLOSION:
	      case SAR_OBJ_TYPE_CHEMICAL_SPRAY:
	      case SAR_OBJ_TYPE_FUELTANK:
	      case SAR_OBJ_TYPE_PREMODELED:
		break;
	    }

	    /* Is object okay to be refueled or repaired? */
	    if(obj_ok)
	    {
		/* Check if object is at a helipad and get the
		 * facilities (refuel or repair)
		 */
		int i, hit_obj_num, list_total, *list;
		sar_object_struct *hit_obj_ptr;
		sar_object_helipad_struct *obj_helipad_ptr;

		/* Get list of objects at the object's location */
		list = SARGetGCCHitList(
		    core_ptr, scene,
		    &core_ptr->object, &core_ptr->total_objects,
		    scene->player_obj_num,
		    &list_total
		);
		/* Iterate through objects at this object's location
		 * and update the refuel, repair, and dropoff states
		 */
		for(i = 0; i < list_total; i++)
		{
		    hit_obj_num = list[i];
		    hit_obj_ptr = SARObjGetPtr(
			core_ptr->object, core_ptr->total_objects,
			hit_obj_num
		    );
		    if(hit_obj_ptr == NULL)
			continue;

		    switch(hit_obj_ptr->type)
		    {
		      case SAR_OBJ_TYPE_GARBAGE:
		      case SAR_OBJ_TYPE_STATIC:
		      case SAR_OBJ_TYPE_AUTOMOBILE:
		      case SAR_OBJ_TYPE_WATERCRAFT:
		      case SAR_OBJ_TYPE_AIRCRAFT:
		      case SAR_OBJ_TYPE_GROUND:
		      case SAR_OBJ_TYPE_RUNWAY:
			break;
		      case SAR_OBJ_TYPE_HELIPAD:
			obj_helipad_ptr = SAR_OBJ_GET_HELIPAD(hit_obj_ptr);
			if(obj_helipad_ptr != NULL)
			{
			    const sar_helipad_flags flags = obj_helipad_ptr->flags;
			    if(flags & SAR_HELIPAD_FLAG_FUEL)
				can_refuel = True;
			    if(flags & SAR_HELIPAD_FLAG_REPAIR)
				can_repair = True;
			    if(flags & SAR_HELIPAD_FLAG_DROPOFF)
				can_dropoff = True;
			}
			loc_ok = True;
			break;
		      case SAR_OBJ_TYPE_HUMAN:
		      case SAR_OBJ_TYPE_SMOKE:
		      case SAR_OBJ_TYPE_FIRE:
		      case SAR_OBJ_TYPE_EXPLOSION:
		      case SAR_OBJ_TYPE_CHEMICAL_SPRAY:
		      case SAR_OBJ_TYPE_FUELTANK:
		      case SAR_OBJ_TYPE_PREMODELED:
			break;
		    }
		}
		free(list);

		/* Begin refuel and repair of object depending on
		 * available facilities.
		 */
		if(loc_ok)
		{
		    const char *s;

		    if(can_refuel)
			SARSimOpRefuel(scene, obj_ptr);
		    if(can_repair)
			SARSimOpRepair(scene, obj_ptr);

		    if(can_refuel && can_repair)
			s = SAR_MESG_REFUELING_REPAIRS_COMPLETE;
		    else if(can_refuel)
			s = SAR_MESG_REFUELING_COMPLETE;
		    else if(can_repair)
			s = SAR_MESG_REPAIRS_COMPLETE;
		    else
			s = SAR_MESG_NO_FACILITIES;
		    SARMessageAdd(scene, s);

		    /* Can drop off passengers and not playing a mission? */
		    if(can_dropoff && (core_ptr->mission == NULL))
		    {
			int passengers_unloaded = SARSimOpPassengersUnloadAll(
			    scene, obj_ptr
			);
			if(passengers_unloaded > 0)
			{
			    char s[256];
			    sprintf(s, "Dropped off %i passengers", passengers_unloaded);
			    SARMessageAdd(scene, s);
			}
		    }
		}
		else
		{
		    SARMessageAdd(
			scene, SAR_MESG_NOT_REFUELABLE
		    );
		}
	    }
	    else
	    {
		SARMessageAdd(
		    scene, SAR_MESG_NOT_REFUELABLE
		);
	    }
	}
}

/*
 *	Prints remaining fuel and statistics, transfers fuel from reserved
 *	tanks, or drop fuel tank.
 */
static void SARKeyFuel(SAR_KEY_FUNC_PROTOTYPE)
{
	int obj_num;
	sar_object_struct *obj_ptr;
	const sar_option_struct *opt = &core_ptr->option;

	if((scene == NULL) || !state)
	    return;

	obj_num = scene->player_obj_num;
	obj_ptr = scene->player_obj_ptr;
	if(obj_ptr != NULL)
	{
	    sar_object_aircraft_struct *aircraft;

	    switch(obj_ptr->type)
	    {
	      case SAR_OBJ_TYPE_GARBAGE:
	      case SAR_OBJ_TYPE_STATIC:
	      case SAR_OBJ_TYPE_AUTOMOBILE:
	      case SAR_OBJ_TYPE_WATERCRAFT:
		break;
	      case SAR_OBJ_TYPE_AIRCRAFT:
		aircraft = SAR_OBJ_GET_AIRCRAFT(obj_ptr);
		if(aircraft != NULL)
		{
		    if(display->ctrl_key_state)
		    {
			/* Drop next fuel tank */
			SARSimOpDropFuelTankNext(
			    scene,
			    &core_ptr->object, &core_ptr->total_objects,
			    obj_num, obj_ptr
			);
		    }
		    else if(display->shift_key_state)
		    {
			char text[256];

			/* Transfer fuel */
			float fuel_transfered = SARSimOpTransferFuelFromTanks(
			    scene, obj_ptr
			);
			if(fuel_transfered > 0.0f)
			{
			    switch(opt->units)
			    {
			      case SAR_UNITS_METRIC:
			      case SAR_UNITS_METRIC_ALT_FEET:
				sprintf(
				    text,
				    SAR_MESG_RESERVE_FUEL_TRANSFERED,
				    fuel_transfered,
				    "kg"
				);
				break;
			      default:	/* SAR_UNITS_ENGLISH */
				sprintf(
				    text,
				    SAR_MESG_RESERVE_FUEL_TRANSFERED,
				    SFMKGToLBS(fuel_transfered),
				    "lbs"
				);
				break;
			    }
			}
			else
			{
			    strcpy(text, SAR_MESG_NO_RESERVE_FUEL_TO_TRANSFER);
			}
			SARMessageAdd(scene, text);
		    }
		    else
		    {
			/* Print fuel stats */
			int i;
			sar_external_fueltank_struct *eft_ptr;
			float throttle_output = SARSimThrottleOutputCoeff(
		(aircraft->flight_model_type != SAR_FLIGHT_MODEL_SLEW) ?
		    aircraft->flight_model_type :
		    aircraft->last_flight_model_type
		,
		aircraft->throttle,
		aircraft->collective,
		aircraft->collective_range
			);
			float fuel = aircraft->fuel;	/* In kg */
			float fuel_rate = aircraft->fuel_rate;	/* In kg/cycle */
			float aloft_time;			/* In seconds */
			float external_fuel = 0.0, external_fuel_max = 0.0;
			char text[256];


			/* Add up fuel from external reserved tanks */
			for(i = 0; i < aircraft->total_external_fueltanks; i++)
			{
			    eft_ptr = aircraft->external_fueltank[i];
			    if(eft_ptr == NULL)
				continue;

			    if(eft_ptr->flags & SAR_EXTERNAL_FUELTANK_FLAG_ONBOARD)
			    {
				external_fuel += eft_ptr->fuel;
				external_fuel_max += eft_ptr->fuel_max;
			    }
			}

			/* Calculate time left aloft (include fuel
			 * from external tanks)
			 */
			if((fuel_rate * throttle_output) > 0.0f)
			    aloft_time = (float)(
				((fuel + external_fuel) /
				(fuel_rate * throttle_output)) * 
				SAR_CYCLE_TO_SEC_COEFF
			    );
			else
			    aloft_time = 0.0f;

			if(external_fuel_max > 0.0f)
			{
			    switch(opt->units)
			    {
			      case SAR_UNITS_METRIC:
			      case SAR_UNITS_METRIC_ALT_FEET:
				sprintf(
				    text,
"Int: %.0f(%.0f) kg  Ext: %.0f(%.0f) kg  Endur: %s",
				    aircraft->fuel,
				    aircraft->fuel_max,
				    external_fuel,
				    external_fuel_max,
				    SARDeltaTimeString(
					core_ptr,
					(time_t)aloft_time
				    )
				);
				break;
			      default:	/* SAR_UNITS_ENGLISH */
				sprintf(
				    text,
"Int: %.0f(%.0f) lbs  Ext: %.0f(%.0f) lbs  Endur: %s",
				    SFMKGToLBS(aircraft->fuel),
				    SFMKGToLBS(aircraft->fuel_max),
				    SFMKGToLBS(external_fuel),
				    SFMKGToLBS(external_fuel_max),
				    SARDeltaTimeString(
					core_ptr,
					(time_t)aloft_time
				    )
				);
				break;
			    }
			}
			else
			{
			    switch(opt->units)
			    {
			      case SAR_UNITS_METRIC:
			      case SAR_UNITS_METRIC_ALT_FEET:
				sprintf(
				    text,
"Int: %.0f(%.0f) kg  Endur: %s",
				    aircraft->fuel,
				    aircraft->fuel_max,
				    SARDeltaTimeString(
					core_ptr,
					(time_t)aloft_time
				    )
				);
				break;
			      default:  /* SAR_UNITS_ENGLISH */
				sprintf(
				    text,
"Int: %.0f(%.0f) lbs  Endur: %s",
				    SFMKGToLBS(aircraft->fuel),
				    SFMKGToLBS(aircraft->fuel_max),
				    SARDeltaTimeString(
					core_ptr,
					(time_t)aloft_time
				    )
				);
				break;
			    }
			}
			SARMessageAdd(scene, text);
		    }
		}
		break;
	      case SAR_OBJ_TYPE_GROUND:
	      case SAR_OBJ_TYPE_RUNWAY:
	      case SAR_OBJ_TYPE_HELIPAD:
	      case SAR_OBJ_TYPE_HUMAN:
	      case SAR_OBJ_TYPE_SMOKE:
	      case SAR_OBJ_TYPE_FIRE:
	      case SAR_OBJ_TYPE_EXPLOSION:
	      case SAR_OBJ_TYPE_CHEMICAL_SPRAY:
	      case SAR_OBJ_TYPE_FUELTANK:
	      case SAR_OBJ_TYPE_PREMODELED:
		break;
 	    }
	}
}

/*
 *	Sends a message.
 */
static void SARKeySendMessage(SAR_KEY_FUNC_PROTOTYPE)
{
	if(!state)
	    return;

	/* Map prompt for entering a message to send */
	SARTextInputMap(
	    core_ptr->text_input,
	    "Message", NULL,
	    SARTextInputCBSendMessage,
	    core_ptr
	);
}

/*
 *	Adjusts the time of day on the scene structure.
 */
static void SARKeyTimeOfDay(SAR_KEY_FUNC_PROTOTYPE)
{
	const char *time_str;
	char text[256];

	if((scene == NULL) || !state)
	    return;

	/* Increase or decrease time of day */
	if(display->shift_key_state)
	    scene->tod -= (float)(0.25 * 3600.0);
	else
	    scene->tod += (float)(0.25 * 3600.0);

	time_str = (const char *)SARTimeOfDayString(core_ptr, scene->tod);
	sprintf(
	    text,
	    "%s %s",
	    SAR_MESG_TIME_OF_DAY,
	    time_str
	);
	SARMessageAdd(scene, text);
}



/*
 *	SAR keyboard event handler. Front end for handling all in game
 *	keyboard events.
 *
 *	If the in game text input prompt is mapped then the key event
 *	will be forwarded to the prompt input handler.
 *
 *	If key event key code c does not match any key operation to be
 *	handled then the keyboard event will be forwarde to
 *	GCtlHandleKey().
 */
void SARKey(
	sar_core_struct *core_ptr,
	int c, Boolean state, long t
)
{
	gw_display_struct *display = core_ptr->display;
	sar_scene_struct *scene = core_ptr->scene;

/* Inputs for all SARKey*() functions */
#define SAR_KEY_FUNC_INPUT	core_ptr, display, scene, state

/* Turns off autorepeat when key is pressed and turns autorepeat
 * back on when key is released.
 */
#define DO_HAS_NO_AUTOREPEAT	{			\
 GWKeyboardAutoRepeat(display, (Boolean)!state);	\
}

	/* Initial simulation key checks come first */

	/* Space bar initially `continues' if mission failed or
	 * ended, or moves player to nearest restarting point on
	 * free flights.
	 */
	if(c == ' ')
	{
	    /* Space bar always implicitly clears scene's sticky banner
	     * message.
	     */
	    SARBannerMessageAppend(scene, NULL);

	    /* Do `restart' procedure, conditions will be checked by
	     * the function. Returns positive if event was handled.
	     */
	    if(SARKeyRestart(SAR_KEY_FUNC_INPUT) > 0)
	    {
		/* Event was handled, return immediatly. The simulation
		 * may have ended or other memory greatly changed.
		 */
		return;
	    }
	}


	/* Handle key as a regular simulation action */
	switch(c)
	{
/* Debug value adjustment */
case 'x': case 'X':
 if((scene != NULL) && (display != NULL) && state)
 {
  debug_value += (float)((display->shift_key_state) ? 0.25 : -0.25);
fprintf(stderr, "Debug value set to %.2f\n", debug_value);
 }
 break;

	  case 's': case 'S':	/* Print score */
	    DO_HAS_NO_AUTOREPEAT
	    if(display->ctrl_key_state)
		SARKeySoundLevel(SAR_KEY_FUNC_INPUT);
	    else
		SARKeyPrintScore(SAR_KEY_FUNC_INPUT);
	    break;

	  case GWKeyBackSpace:	/* Re-center/normalize view */
	    DO_HAS_NO_AUTOREPEAT
	    SARKeyViewNormalize(SAR_KEY_FUNC_INPUT);
	    break;

	  case GWKeyF1:		/* Help display toggle */
	    DO_HAS_NO_AUTOREPEAT
	    SARKeyHelpDisplay(SAR_KEY_FUNC_INPUT);
	    break;

	  case GWKeyF2:		/* Switch camera ref to cockpit */
	    DO_HAS_NO_AUTOREPEAT
	    SARKeyCameraRefCockpit(SAR_KEY_FUNC_INPUT);
	    break;

	  case GWKeyF3:		/* Switch camera ref to spot */
	    DO_HAS_NO_AUTOREPEAT
	    SARKeyCameraRefSpot(SAR_KEY_FUNC_INPUT);
	    break;

	  case GWKeyF4:		/* Switch camera ref to tower */
	    DO_HAS_NO_AUTOREPEAT
	    SARKeyCameraRefTower(SAR_KEY_FUNC_INPUT);
	    break;

	  case GWKeyF5:		/* Switch camera ref to hoist's rescue basket */
	    DO_HAS_NO_AUTOREPEAT
	    SARKeyCameraRefHoist(SAR_KEY_FUNC_INPUT);
	    break;

	  case 'm': case 'M':	/* Switch camera ref to map */
	    DO_HAS_NO_AUTOREPEAT
	    if(display->ctrl_key_state)
		SARKeyMusic(SAR_KEY_FUNC_INPUT);
	    else
		SARKeyCameraRefMap(SAR_KEY_FUNC_INPUT);
	    break;

	  case 'u':
	    if(display->ctrl_key_state)
		SARKeyUnits(SAR_KEY_FUNC_INPUT);
	    break;

	  case GWKeyF9:		/* Toggle textured ground or dual pass depth */
	    DO_HAS_NO_AUTOREPEAT
	    SARKeyGraphicsTexturedGround(SAR_KEY_FUNC_INPUT);
	    break;

	  case GWKeyF10:	/* Toggle atmosphere */
	    DO_HAS_NO_AUTOREPEAT
	    SARKeyGraphicsAtmosphere(SAR_KEY_FUNC_INPUT);
	    break;

	  case GWKeyF11:	/* Toggle textured objects */
	    DO_HAS_NO_AUTOREPEAT
	    SARKeyGraphicsTexturedObjects(SAR_KEY_FUNC_INPUT);
	    break;

	  case GWKeyF12:	/* Toggle textured clouds or prop wash */
	    DO_HAS_NO_AUTOREPEAT
	    SARKeyGraphicsTexturedClouds(SAR_KEY_FUNC_INPUT);
	    break;

	  case 0x1b:		/* Escape */
	    DO_HAS_NO_AUTOREPEAT
	    SARKeyEscape(SAR_KEY_FUNC_INPUT);
	    break;

	  case '/':		/* Command prompt map */
	    DO_HAS_NO_AUTOREPEAT
	    SARKeyCommand(SAR_KEY_FUNC_INPUT);
	    break;

	  case 'z': case 'Z':	/* Adjust time compression */
	    DO_HAS_NO_AUTOREPEAT
	    SARKeyTimeCompression(SAR_KEY_FUNC_INPUT);
	    break;


	  case 'a': case 'A':	/* Slew toggle */
/* This will later include autopilot/autohover */
	    DO_HAS_NO_AUTOREPEAT
	    SARKeyAutoPilot(SAR_KEY_FUNC_INPUT);
	    break;

	  /* Raise/lower landing gears on player object */
	  case 'g': case 'G':
	    DO_HAS_NO_AUTOREPEAT
	    SARKeyLandingGear(SAR_KEY_FUNC_INPUT);
	    break;

	  /* Turn lights on/off on player object */
	  case 'l': case 'L':
	    DO_HAS_NO_AUTOREPEAT
	    SARKeyLights(SAR_KEY_FUNC_INPUT);
	    break;

	  /* Turn strobe lights on/off on player object */
	  case 'o': case 'O':
	    DO_HAS_NO_AUTOREPEAT
	    SARKeyStrobes(SAR_KEY_FUNC_INPUT);
	    break;

	  /* Toggle FLIR (night vision) on/off */
	  case 'i': case 'I':
	    DO_HAS_NO_AUTOREPEAT
	    SARKeyFLIR(SAR_KEY_FUNC_INPUT);
	    break;

	  case 'h': case 'H':
	    if(display->ctrl_key_state)
	    {
		/* Adjust hoist contact expansion coefficient */
		DO_HAS_NO_AUTOREPEAT
		SARKeyHoistContact(SAR_KEY_FUNC_INPUT);
	    }
	    else
	    {
		/* Adjust text color */
		SARKeyTextColor(SAR_KEY_FUNC_INPUT);
	    }
	    break;

	  /* Select next intercept way point */
	  case 'w': case 'W':
	    DO_HAS_NO_AUTOREPEAT
	    SARKeyInterceptWayPoint(SAR_KEY_FUNC_INPUT);
	    break;

	  /* Engine off/init */
	  case 'e': case 'E':
	    DO_HAS_NO_AUTOREPEAT
	    SARKeyEngine(SAR_KEY_FUNC_INPUT);
	    break;

	  /* Elevator trim down */
	  case GWKeyHome:
	    SARKeyElevatorTrimDown(SAR_KEY_FUNC_INPUT);
	    break;

	  /* Elevator trim */
	  case GWKeyEnd:
	    SARKeyElevatorTrimUp(SAR_KEY_FUNC_INPUT);
	    break;

	  /* Send message */
	  case '\'':
	    DO_HAS_NO_AUTOREPEAT
	    SARKeySendMessage(SAR_KEY_FUNC_INPUT);
	    break;

	  /* Adjust time of day */
	  case 't': case 'T':
	    SARKeyTimeOfDay(SAR_KEY_FUNC_INPUT);
	    break;

	  /* Select rescue hoist rope end deployment type */
	  case 'p': case 'P':
	    DO_HAS_NO_AUTOREPEAT
	    SARKeyHoistRopeEndSelect(SAR_KEY_FUNC_INPUT);
	    break;

	  case 'd': case 'D':
	    if(display->ctrl_key_state)
	    {
		/* Adjust flight physics dificulty */
		DO_HAS_NO_AUTOREPEAT
		SARKeyFlightPhysicsDifficulty(SAR_KEY_FUNC_INPUT);
	    }
	    else
	    {
		/* Open/close rescue door */
		DO_HAS_NO_AUTOREPEAT
		SARKeyDoor(SAR_KEY_FUNC_INPUT);
	    }
	    break;

	  case 'v': case 'V':
	    SARKeyVisibility(SAR_KEY_FUNC_INPUT);
	    break;

	  case 'c': case 'C':
	    DO_HAS_NO_AUTOREPEAT

	    break;

	  case 'y': case 'Y':	/* Tilt rotors/pitch engine */
	    DO_HAS_NO_AUTOREPEAT
	    SARKeyTiltRotors(SAR_KEY_FUNC_INPUT);
	    break;

	  case 'r': case 'R':
	    DO_HAS_NO_AUTOREPEAT
	    SARKeyRefuelRepair(SAR_KEY_FUNC_INPUT);
	    break;

	  case 'f': case 'F':
	    DO_HAS_NO_AUTOREPEAT  
	    SARKeyFuel(SAR_KEY_FUNC_INPUT);
	    break;

	  default:
	    /* Some other key, let the game controller handle it */
	    GCtlHandleKey(
		display,
		core_ptr->gctl,
		c, state,
		display->alt_key_state,
		display->ctrl_key_state,
		display->shift_key_state,
		t,
		lapsed_millitime
	    );
	    break;
	}

/* Calling function will redraw whenever a key event is recieved
	GWPostRedraw((gw_display_struct *)ptr);
 */

#undef SAR_KEY_FUNC_INPUT
}

#undef SAR_KEY_FUNC_PROTOTYPE
