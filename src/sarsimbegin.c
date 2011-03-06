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
#include <time.h>

#ifdef __MSW__
# include <windows.h>
#else
# include <sys/time.h>
# include <unistd.h>
#endif

#include "gw.h"
#include "messages.h"
#include "sarreality.h"
#include "obj.h"
#include "mission.h"
#include "sar.h"
#include "sartime.h"
#include "sarmusic.h"
#include "scenesound.h"
#include "objio.h"
#include "missionio.h"
#include "sceneio.h"
#include "sarmenuop.h"
#include "sarmenucodes.h"
#include "sarsimbegin.h"


static void SARSimBeginResetOptions(sar_core_struct *core_ptr);

int SARSimBeginMission(
	sar_core_struct *core_ptr,
	const char *mission_file
);
int SARSimBeginFreeFlight(
	sar_core_struct *core_ptr,
	const char *scene_file,
	const char *player_file,                /* Player aircraft. */
	sar_position_struct *start_pos,
	sar_direction_struct *start_dir,
	const char *weather_preset_name,
	Boolean free_flight_system_time
);


#define ATOI(s)         (((s) != NULL) ? atoi(s) : 0)
#define ATOL(s)         (((s) != NULL) ? atol(s) : 0)
#define ATOF(s)         (((s) != NULL) ? atof(s) : 0.0f)
#define STRDUP(s)       (((s) != NULL) ? strdup(s) : NULL)

#define MAX(a,b)        (((a) > (b)) ? (a) : (b))
#define MIN(a,b)        (((a) < (b)) ? (a) : (b))
#define CLIP(a,l,h)     (MIN(MAX((a),(l)),(h)))
#define STRLEN(s)       (((s) != NULL) ? ((int)strlen(s)) : 0)


/*
 *	Resets options for simulation.
 */
static void SARSimBeginResetOptions(sar_core_struct *core_ptr)
{
	sar_option_struct *opt = &core_ptr->option;

	core_ptr->display_help = 0;
	core_ptr->flir = False;

	opt->show_hud_text = True;
	opt->show_outside_text = True;

	time_compression = 1.0f;
}

/*
 *      Enters mission simulation from the menu system.
 *
 *      Returns 0 on success or -1 on failure. On failure this function
 *      will return the menu system.
 */
int SARSimBeginMission(
	sar_core_struct *core_ptr,
	const char *mission_file
)
{
	gw_display_struct *display;
	sar_scene_struct *scene;
	sar_mission_struct *mission;
	sar_progress_cb_struct *cb_data;
	const sar_option_struct *opt;


	if(core_ptr == NULL)
	    return(-1);

	display = core_ptr->display;
	if(display == NULL)
	    return(-1);

	opt = &core_ptr->option;

	/* Check inputs */
	if(mission_file == NULL)
	{
	    GWOutputMessage(
		display,
		GWOutputMessageTypeWarning,
		"Missing Information",
		"No mission selected",
		"You need to select a mission"
	    );
	    return(-1);
	}


	/* Begin switching from menus to mission simulation */

	/* Switch to load simulation progress menu */
	SARMenuSwitchToMenu(
	    core_ptr, SAR_MENU_NAME_LOADING_SIMULATION
	);
	/* Check if music needs to be updated and update it as needed.
	 * This will detect that the current menu is the loading
	 * simulation menu and switch to the loading simulation music.
	 */
	SARMusicUpdate(core_ptr);

	GWSetInputBusy(display);

	core_ptr->stop_count = 0;

	/* Allocate scene structure as needed */
	if(core_ptr->scene == NULL)
	{
	    core_ptr->scene = (sar_scene_struct *)calloc(
		1, sizeof(sar_scene_struct)
	    );
	    if(core_ptr->scene == NULL)
	    {
		/* Scene allocation failed, switch back to main menu,
		 * mark input as ready, and return
		 */
		SARMenuSwitchToMenu(
		    core_ptr, SAR_MENU_NAME_MAIN
		);
		GWSetInputReady(display);
		return(-1);
	    }
	}
	scene = core_ptr->scene;

	/* Unload mission (in case it's already loaded) */
	SARMissionDelete(core_ptr->mission);
	core_ptr->mission = NULL;

	/* Unload scene (in case it's already loaded) */
	SARSceneDestroy(
	    core_ptr, scene,
	    &core_ptr->object, &core_ptr->total_objects
	);

	/* Reset game controller values */
	GCtlResetValues(core_ptr->gctl);

	/* Set up load progress callback data */
	cb_data = SAR_PROGRESS_CB(calloc(1, sizeof(sar_progress_cb_struct)));
	cb_data->core_ptr = core_ptr;
	cb_data->coeff_offset = 0.0f;
	cb_data->coeff_range = 1.0f;
	cb_data->can_abort = False;

	/* Load mission */
	core_ptr->mission = mission = SARMissionLoadFromFile(
	    core_ptr, mission_file,
	    cb_data, SARLoadProgressCB
	);

	/* Failed to load mission? */
	if(mission == NULL)
	{
	    /* Failed to load mission */
	    SARMissionDelete(mission);
	    core_ptr->mission = mission = NULL;

	    SARSceneDestroy(
		core_ptr, scene,
		&core_ptr->object, &core_ptr->total_objects
	    );
	    free(core_ptr->scene);
	    core_ptr->scene = scene = NULL;

	    free(cb_data);

	    SARMenuSwitchToMenu(
		core_ptr, SAR_MENU_NAME_MAIN
	    );
	    GWSetInputReady(display);
	    return(-1);
	}
	else
	{
	    char *s;
	    const char *name;
	    sar_player_stat_struct *pstat = SARPlayerStatCurrent(core_ptr, NULL);
	    sar_object_struct *player_obj_ptr = scene->player_obj_ptr;

	    if(pstat != NULL)
		name = pstat->name;
	    else
		name = (player_obj_ptr != NULL) ? player_obj_ptr->name : NULL;

	    /* Reset mission log file, erasing the old log file and
	     * generating a new one with a header based on the values
	     * from the specified mission
	     */
	    SARMissionLogReset(
		core_ptr, mission,
		fname.mission_log
	    );

	    /* Log mission start event */
	    s = (char *)malloc(
		(80 + STRLEN(name) +
		    STRLEN(mission->start_location_name)
		) * sizeof(char)
	    );
	    sprintf(
		s,
		"%s took off at %s",
		name,
		mission->start_location_name
	    );
	    SARMissionLogEvent(
		core_ptr, core_ptr->mission,
		SAR_MISSION_LOG_EVENT_TAKEOFF,
		-1.0,
		(player_obj_ptr != NULL) ? &player_obj_ptr->pos : NULL,
		NULL, 0,
		s,
		fname.mission_log
	    );
	    free(s);

	    /* Update message text color and hud color based on the
	     * initial time of day set on the scene
	     */
	    SARSetGlobalTextColorBrightness(
		core_ptr,
		(float)(
		    (scene->tod > 43200.0f) ?
		    MAX(1.0 - ((scene->tod - 43200.0) / 43200.0), 0.4) :
		    MAX(scene->tod / 43200.0, 0.4)
		)
	    );

	    /* Switch off menus */
	    SARMenuSwitchToMenu(core_ptr, NULL);

	    /* Update sound on scene */
	    SARSceneSoundUpdate(
		core_ptr,
		opt->engine_sounds,
		opt->event_sounds,
		opt->voice_sounds,
		opt->music
	    );
	}

	/* Need to reset timmers since the loading may have consumed
	 * long amount of time and if the lapsed_millitime is too long
	 * simulations and other timings can get out of sync
	 */
	SARResetTimmersCB(core_ptr, SARGetCurMilliTime());

	/* Reset option values before starting simulation */
	SARSimBeginResetOptions(core_ptr);

	GWSetInputReady(display);

	/* Hide pointer cursor */
	GWHideCursor(display);

        /* Show welcome message */
        SARMessageAdd(core_ptr->scene,core_ptr->scene->welcome_message);

	free(cb_data);

   	return(0);
}

/*
 *	Enters free flight simulation from the menu system.
 *
 *	Returns 0 on success or -1 on failure. On failure this function
 *	will return the menu system.
 */
int SARSimBeginFreeFlight(
	sar_core_struct *core_ptr,
	const char *scene_file,
	const char *player_file,                /* Player aircraft. */
	sar_position_struct *start_pos,
	sar_direction_struct *start_dir,
	const char *weather_preset_name,
	Boolean free_flight_system_time
)
{
	gw_display_struct *display;
	sar_scene_struct *scene;
	sar_progress_cb_struct *cb_data;
	const sar_option_struct *opt;

	if(core_ptr == NULL)
	    return(-1);

	display = core_ptr->display;
	if(display == NULL)
	    return(-1);

	opt = &core_ptr->option;

	/* Check inputs */
	if(player_file == NULL)
	{
	    GWOutputMessage(
		display,
		GWOutputMessageTypeWarning,
		"Missing Information",
		"No aircraft selected",
		"You need to select an aircraft for free flight"
	    );
	    return(-1);
	}
	if(scene_file == NULL)
	{
	    GWOutputMessage(
		display,
		GWOutputMessageTypeWarning,
		"Missing Information",
		"No scenery selected",
		"You need to select a scenery to fly in"
	    );
	    return(-1);
	}


	/* Begin switching from menus to free flight simulation */

	/* Switch to load simulation progress menu */
	SARMenuSwitchToMenu(
	    core_ptr, SAR_MENU_NAME_LOADING_SIMULATION
	);
	/* Check if music needs to be updated and update it as needed.
	 * This will detect that the current menu is the loading
	 * simulation menu and switch to the loading simulation music.
	 */
	SARMusicUpdate(core_ptr);

	GWSetInputBusy(display);

	core_ptr->stop_count = 0;

	/* Allocate scene structure as needed. */
	if(core_ptr->scene == NULL)
	{
	    core_ptr->scene = (sar_scene_struct *)calloc(
		1, sizeof(sar_scene_struct)
	    );
	    if(core_ptr->scene == NULL)
	    {
		SARMenuSwitchToMenu(
		    core_ptr, SAR_MENU_NAME_MAIN
		);
		GWSetInputReady(display);
		return(-1);
	    }
	}
	scene = core_ptr->scene;

	/* Unload scene (incase it's already loaded/allocated). */
	SARSceneDestroy(
	    core_ptr, scene,
	    &core_ptr->object,
	    &core_ptr->total_objects
	);

	/* Reset game controller values. */
	GCtlResetValues(core_ptr->gctl);

	/* Set up load progress callback data. */
	cb_data = SAR_PROGRESS_CB(calloc(1, sizeof(sar_progress_cb_struct)));
	cb_data->core_ptr = core_ptr;
	cb_data->coeff_offset = 0.0f;
	cb_data->coeff_range = 0.9f;
	cb_data->can_abort = False;

	/* Load scene */
	if(SARSceneLoadFromFile(
	    core_ptr, scene, scene_file,
	    weather_preset_name,
	    cb_data, SARLoadProgressCB
	))
	{
	    /* Failed to load scene
	     *
	     * Delete any loaded portions of the scene, switch back to
	     * the menus, and return indicating error
	     */
	    SARSceneDestroy(
		core_ptr, scene,
		&core_ptr->object, &core_ptr->total_objects
	    );
	    free(scene);
	    core_ptr->scene = scene = NULL;

	    free(cb_data);

	    SARMenuSwitchToMenu(
		core_ptr, SAR_MENU_NAME_MAIN
	    );
	    GWSetInputReady(display);
	    return(-1);
	}
	else
	{
	    /* Successfully loaded scene */

	    /* Add player object to scene */
	    cb_data->coeff_offset = 0.9f;
	    cb_data->coeff_range = 0.1f;
	    SARLoadProgressCB(cb_data, 0, 1);
	    SARSceneAddPlayerObject(
		core_ptr, scene, player_file,
		start_pos, start_dir
	    );
	    SARLoadProgressCB(cb_data, 1, 1);

	    /* Set free flight time from system time? */
	    if(free_flight_system_time)
	    {
		time_t t = time(NULL);
		struct tm *tm_ptr = localtime(&t);
		if(tm_ptr != NULL)
		{
		    scene->tod = (float)(
			(tm_ptr->tm_hour * 3600) +
			(tm_ptr->tm_min * 60) +
			(tm_ptr->tm_sec)
		    );
		}
	    }

    	    /* Update message text color and hud color based on the
	     * initial time of day set on the scene
	     */
	    SARSetGlobalTextColorBrightness(
		core_ptr,
		(float)(
		    (scene->tod > 43200.0f) ?
		    MAX(1.0 - ((scene->tod - 43200.0) / 43200.0), 0.4) :
		    MAX(scene->tod / 43200.0, 0.4)
		)
	    );

	    /* Switch off menus */
	    SARMenuSwitchToMenu(core_ptr, NULL);

	    /* Update sound on scene */
	    SARSceneSoundUpdate(
		core_ptr,
		opt->engine_sounds,
		opt->event_sounds,
		opt->voice_sounds,
		opt->music
	    );
	}

	/* Need to reset timmers since the loading may have consumed
	 * long amount of time and if the lapsed_millitime is too long
	 * simulations and other timings can get out of sync
	 */
	SARResetTimmersCB(core_ptr, SARGetCurMilliTime());

	/* Reset option values before starting simulation */
	SARSimBeginResetOptions(core_ptr);

	GWSetInputReady(display);

	/* Hide pointer cursor */
	GWHideCursor(display);

        /* Show scene welcome message */
        SARMessageAdd(core_ptr->scene,core_ptr->scene->welcome_message);

	free(cb_data);

	return(0);
}
