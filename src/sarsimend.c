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

#ifdef __MSW__
# include <windows.h>
#else
# include <sys/time.h>
# include <unistd.h>
#endif

#include "gw.h"
#include "messages.h"
#include "obj.h"
#include "mission.h"
#include "sar.h"
#include "missionio.h"
#include "sceneio.h"
#include "sarmenuop.h"
#include "sarmenucodes.h"
#include "sarsimend.h"
#include "config.h"


static void SARSimEndMissionTabulate(
	sar_core_struct *core_ptr, sar_mission_struct *mission
);

void SARSimEnd(sar_core_struct *core_ptr);


#define MAX(a,b)        (((a) > (b)) ? (a) : (b))
#define MIN(a,b)        (((a) < (b)) ? (a) : (b))
#define CLIP(a,l,h)     (MIN(MAX((a),(l)),(h)))
#define STRDUP(s)	(((s) != NULL) ? strdup(s) : NULL)


/*
 *	Updates the mission log map menu with the information from the
 *	current mission (but does switch to the log map menu.
 */
static void SARSimEndMissionTabulate(
	sar_core_struct *core_ptr, sar_mission_struct *mission
)
{
	char *s;
	const char *name;
	sar_player_stat_struct *pstat;
	sar_menu_struct *m;
	sar_scene_struct *scene = core_ptr->scene;
	int player_obj_num;
	sar_object_struct *player_obj_ptr;
	if(scene == NULL)
	    return;

	/* Get player object */
	player_obj_num = scene->player_obj_num;
	player_obj_ptr = scene->player_obj_ptr;
	pstat = SARPlayerStatCurrent(core_ptr, NULL);

	if(pstat != NULL)
	    name = pstat->name;
	else
	    name = (player_obj_ptr != NULL) ? player_obj_ptr->name : NULL;

	/* Enough information to begin tabulating mission? */
	if(True)
	{
	    /* Update player stats and get mission ending statement */
	    switch(mission->state)
	    {
	      case MISSION_STATE_IN_PROGRESS:
		/* Mission was still in progress, aborted */
		if(pstat != NULL)
		    pstat->missions_failed++;	/* Counts as failed */
		s = STRDUP("*** MISSION ABORTED ***");
		break;

	      case MISSION_STATE_FAILED:
		/* Mission failed */
		if(pstat != NULL)
		    pstat->missions_failed++;
		s = STRDUP("*** MISSION FAILED ***");
		break;

	      case MISSION_STATE_ACCOMPLISHED:
		/* Mission accomplished */
		if(pstat != NULL)
		{
		    pstat->missions_succeeded++;
		    pstat->score += SAR_POINTS_MISSION_ACCOMPLISHED;
		}
		s = STRDUP("*** MISSION ACCOMPLISHED ***");
		break;

	      default:
		fprintf(
		    stderr,
 "SARSimEndMissionTabulate(): Unsupported mission progress state code `%i'\n",
		    mission->state
		);
		s = STRDUP("Error tabulating mission results!");
		break;
	    }
	}
	/* Log final mission event */
	if(s != NULL)
	{
	    SARMissionLogEvent(
		core_ptr, mission,
/*		SAR_MISSION_LOG_EVENT_COMMENT, */
		SAR_MISSION_LOG_EVENT_LAND,
		-1.0,
		(player_obj_ptr != NULL) ? &player_obj_ptr->pos : NULL,
		NULL, 0,
		s,
		fname.mission_log
	    );
	    free(s);
	    s = NULL;
	}


	/* Update player stats on the players list menu */
	m = SARMatchMenuByNamePtr(core_ptr, SAR_MENU_NAME_PLAYER);
	if(m != NULL)
	{
	    int list_num;
	    sar_menu_list_struct *list = SAR_MENU_LIST(SARMenuGetObjectByID(
		m, SAR_MENU_ID_PLAYER_LIST, &list_num
	    ));
	    if(list != NULL)
	    {
		int i = list->selected_item;
		list->selected_item = -1;
		SARMenuListSelect(
		    core_ptr->display, m, list_num,
		    i,
		    True,	/* Scroll to */
		    False	/* Redraw */
		);
	    }
	}


	/* Get mission log map menu */
	m = SARMatchMenuByNamePtr(core_ptr, SAR_MENU_NAME_MISSION_LOG_MAP);

	/* Load map and markings from the given mission log file to the
	 * given map object
	 */
	SARMissionLoadMissionLogToMenuMap(
	    core_ptr, m,
	    fname.mission_log
	);

}



/*
 *	Procedure to end simulation, this function should be called as the
 *	procedure to tabulate the end of a mission or simulation and
 *	return to the menus.
 *
 *	The simulation type (campaign, mission, freeflight, etc) will
 *	be checked.
 */
void SARSimEnd(sar_core_struct *core_ptr)
{
	gw_display_struct *display;
	const char *switch_to_menu_name = NULL;
	const sar_option_struct *opt;


	if(core_ptr == NULL)
	    return;

	display = core_ptr->display;
	opt = &core_ptr->option;

	if(opt->runtime_debug)
	    printf("SARSimEnd(): Ending simulation...\n");

	/* Check if a mission structure is defined which implies the
	 * simulation involved a mission (and possibly a campaign)
	 */
	if(core_ptr->mission != NULL)
	{ 
	    /* Mission structure is allocated, implying that there is
	     * a mission going on
	     */
	    sar_mission_struct *mission = core_ptr->mission;

	    if(opt->runtime_debug)
		printf("SARSimEnd(): Tabulating mission results...\n");

	    /* Tabulate mission results */
	    SARSimEndMissionTabulate(core_ptr, mission);	    

	    /* Set mission menu as the menu to switch back to after
	     * the simulation.
	     */
	    switch_to_menu_name = SAR_MENU_NAME_MISSION_LOG_MAP;
 
	    /* Destroy the mission structure, it is no longer needed */
	    SARMissionDelete(mission);
	    core_ptr->mission = mission = NULL;
	}
	else
	{
	    /* No mission structure available, implying this was probably
	     * a free flight
	     */

	    /* Set free flight menu as the menu to switch back to after
	     * the simulation
	     */
	    switch_to_menu_name = SAR_MENU_NAME_FREE_FLIGHT;
	}

	/* Delete scene */
	SARSceneDestroy(
	    core_ptr, core_ptr->scene,
	    &core_ptr->object, &core_ptr->total_objects
	);
	free(core_ptr->scene);
	core_ptr->scene = NULL;


	if(opt->runtime_debug)
	    printf("SARSimEnd(): Switching back to menus...\n");

	/* Show pointer cursor */
	GWShowCursor(display);

	/* Switch to 2D, select main menu (and thus redrawing it) */
	SARMenuGLStateReset(display);

	/* If no menu name to switch to then assume main menu */
	if(switch_to_menu_name == NULL)
	    switch_to_menu_name = SAR_MENU_NAME_MAIN;
	SARMenuSwitchToMenu(core_ptr, switch_to_menu_name);

	/* Restore keyboard auto repeat, this may be needed since a key
	 * may have been pressed to end a mission and while its held down
	 * the mission may end and auto repeat is not reset since the
	 * key release of the key in question is not handled when back in
	 * menus
	 */
	GWKeyboardAutoRepeat(display, True);


	if(opt->runtime_debug)
	    printf("SARSimEnd(): Done.\n");
}
