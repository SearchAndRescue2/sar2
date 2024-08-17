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
#include <math.h>

#include "../include/string.h"

#include "obj.h"
#include "objutils.h"
#include "messages.h"
#include "mission.h"
#include "missionio.h"
#include "simutils.h"
#include "sar.h"
#include "config.h"


#ifdef __MSW__
static double rint(double x);
#endif	/* __MSW__ */
static sar_mission_objective_struct *SARMissionObjectiveGetCurrentPtr(
	sar_mission_struct *mission
);
static int SARMissionGetObjectPassengers(
	sar_core_struct *core_ptr, sar_object_struct *obj_ptr
);
static Boolean SARMissionGroundContactMatchName(
	sar_core_struct *core_ptr, sar_scene_struct *scene,
	sar_object_struct **ptr, int total,
	const int *ground_contact, int total_ground_contacts,
	const char *name
);

sar_mission_struct *SARMissionNew(void);
void SARMissionDelete(sar_mission_struct *mission);

void SARMissionPrintStats(
	sar_core_struct *core_ptr,
	sar_scene_struct *scene,
	sar_mission_struct *mission,
	sar_object_struct *obj_ptr
);

static int SARMissionDoObjectiveSuccess(
	sar_mission_struct *mission, sar_scene_struct *scene
);
static int SARMissionDoObjectiveFailed(
	sar_mission_struct *mission, sar_scene_struct *scene
);

void SARMissionDestroyNotify(
	sar_core_struct *core_ptr, sar_object_struct *obj_ptr
);
void SARMissionHoistInNotify(
	sar_core_struct *core_ptr, sar_object_struct *obj_ptr,
	int hoisted_in		/* Number of humans hoisted in */
);
void SARMissionPassengersEnterNotify(
	sar_core_struct *core_ptr, sar_object_struct *obj_ptr,
	int passengers_entered  /* Number of passengers entered */
);
void SARMissionPassengersLeaveNotify(
	sar_core_struct *core_ptr, sar_object_struct *obj_ptr,
	int passengers_left	/* Number of passengers that left */
);
void SARMissionLandNotify(
	sar_core_struct *core_ptr, sar_object_struct *obj_ptr,
	const int *ground_contact, int total_ground_contacts
);

static void SARMissionObjectiveManage(
	sar_core_struct *core_ptr, sar_mission_struct *mission,
	sar_mission_objective_struct *objective
);
int SARMissionManage(sar_core_struct *core_ptr);


#define ATOI(s)         (((s) != NULL) ? atoi(s) : 0)
#define ATOL(s)         (((s) != NULL) ? atol(s) : 0)
#define ATOF(s)         (((s) != NULL) ? atof(s) : 0.0f)
#define STRDUP(s)       (((s) != NULL) ? strdup(s) : NULL)

#define MAX(a,b)        (((a) > (b)) ? (a) : (b))
#define MIN(a,b)        (((a) < (b)) ? (a) : (b))
#define CLIP(a,l,h)     (MIN(MAX((a),(l)),(h)))
#define STRLEN(s)	(((s) != NULL) ? ((int)strlen(s)) : 0)

#define ISCOMMENT(c)    ((c) == SAR_COMMENT_CHAR)
#define ISCR(c)         (((c) == '\n') || ((c) == '\r'))


/*
 *	Interval between mission end check and leaving simulation to
 *	go back to the menus (in milliseconds):
 */
#define DEF_MISSION_END_INT	5000



#ifdef __MSW__
static double rint(double x)
{
	if((double)((double)x - (int)x) > (double)0.5)
	    return((double)((int)x + (int)1));
	else
	    return((double)((int)x));
}
#endif	/* __MSW__ */


/*
 *	Returns the pointer to the current mission objective structure on
 *	the given mission structure. Can return NULL on error.
 */
static sar_mission_objective_struct *SARMissionObjectiveGetCurrentPtr(
	sar_mission_struct *mission
)
{
	int i;

	if(mission == NULL)
	    return(NULL);

	i = mission->cur_objective;
	if((i < 0) || (i >= mission->total_objectives))
	    return(NULL);
	else
	    return(&mission->objective[i]);
}

/*
 *	Returns the number of passengers on the object, depending on the
 *	object type it may not have any objects. Does not include crew
 *	or any fixed on-board `individuals'.
 *
 *	For any object that has a hoist and rescue basket, any passengers
 *	in the rescue basket are NOT counted.
 *
 *	The object is assumed valid.
 */
static int SARMissionGetObjectPassengers(
	sar_core_struct *core_ptr, sar_object_struct *obj_ptr
)
{
	int *passengers;

	if(obj_ptr == NULL)
	    return(0);

	if(SARObjGetOnBoardPtr(
	    obj_ptr,
	    NULL, &passengers, NULL,
	    NULL,
	    NULL, NULL 
	))
	    return(0);

	if(passengers != NULL)
	     return(MAX(*passengers, 0));
	else
	    return(0);
}

/*
 *	Checks for all objects in the given objects list who's name
 *	matches the given name and who's index is found on the given
 *	ground_contact list.
 *
 *	Any special object numbers in the given ground_contact list will
 *	be ignored.
 *
 *	Returns True on match or False on no match or error.
 */
static Boolean SARMissionGroundContactMatchName(
	sar_core_struct *core_ptr, sar_scene_struct *scene,
	sar_object_struct **ptr, int total,
	const int *ground_contact, int total_ground_contacts,
	const char *name
)
{
	int i, obj_num;
	sar_object_struct *obj_ptr;


	if((name == NULL) || (ground_contact == NULL) || (ptr == NULL))
	    return(False);

	/* Check if landed at right place, iterate through list of   
	 * ground contact objects for matching name
	 */
	for(i = 0; i < total_ground_contacts; i++)
	{
	    obj_num = ground_contact[i];
	    obj_ptr = SARObjGetPtr(ptr, total, obj_num);
	    if((obj_ptr == NULL) ? 1 : (obj_ptr->name == NULL))
		continue;

	    /* Names match? */
	    if(!strcasecmp(obj_ptr->name, name))
		break;
	}
	/* No match? */
	if(i >= total_ground_contacts)
	    return(False);
	else
	    return(True);
}


/*
 *	Allocates a new mission structure with all its members
 *	reset.
 */
sar_mission_struct *SARMissionNew(void)
{
	return(
	    (sar_mission_struct *)calloc(1, sizeof(sar_mission_struct))
	);
}

/*
 *	Deletes the given mission structure and all its substructures,
 *      including the mission structure itself.
 */
void SARMissionDelete(sar_mission_struct *mission)
{
	if(mission == NULL)
	    return;

	/* Need to delete objective(s)? */
	if(mission->objective != NULL)
	{
	    int i;
	    sar_mission_objective_struct *objective;

	    /* Iterate through each objective and delete its members */
	    for(i = mission->total_objectives - 1; i >= 0; i--)
	    {
		objective = &mission->objective[i];

		free(objective->arrive_at_name);
		free(objective->message_success);
		free(objective->message_fail);
	    }

	    /* Delete all objectives */
	    free(mission->objective);
	    mission->objective = NULL;
	    mission->total_objectives = 0;
	}

	/* Delete the rest of the mission structure */
	free(mission->title);
	free(mission->description);

	free(mission->start_location_name);

	free(mission->scene_file);
	free(mission->player_model_file);
	free(mission->player_stats_file);

	free(mission->objective);

	free(mission);
}


/*
 *	Prints mission stats to the scene's message output with respect
 *	to the given object and mission structures.
 */
void SARMissionPrintStats(
	sar_core_struct *core_ptr, sar_scene_struct *scene,
	sar_mission_struct *mission,
	sar_object_struct *obj_ptr
)
{
	int	*passengers = NULL,
		*passengers_max = NULL;
	char text[1024];


	/* First print object statistics if object is valid */
	SARObjGetOnBoardPtr(
	    obj_ptr,
	    NULL, &passengers, &passengers_max,
	    NULL,
	    NULL, NULL
	);

	/* Format passengers on board string */
	sprintf(
	    text,
	    "%s: %i(%i)",
	    SAR_MESG_PASSENGERS,
	    (passengers != NULL) ? *passengers : 0,
	    (passengers_max != NULL) ? *passengers_max : 0
	);
	SARMessageAdd(scene, text);

	/* Valid mission structure given? */
	if(mission != NULL)
	{
	    sar_mission_objective_struct *objective = SARMissionObjectiveGetCurrentPtr(mission);

	    /* Is there a current mission objective? */
	    if(objective != NULL)
	    {
#define tmp_name_len    80
		char tmp_name[tmp_name_len];

		/* Handle by mission objective type */
		switch(objective->type)
		{
		  case SAR_MISSION_OBJECTIVE_ARRIVE_AT:
		    strncpy(
			tmp_name,
			(objective->arrive_at_name != NULL) ?
			    objective->arrive_at_name : "(null)",
			tmp_name_len
		    );
		    tmp_name[tmp_name_len - 1] = '\0';

		    /* Print arrive mission status */
		    sprintf(
			text,
			"%s %s",
			SAR_MESG_MISSION_IN_PROGRESS_ENROUTE, tmp_name
		    );
		    SARMessageAdd(scene, text);

		    /* Is there a time limit? */
		    if(objective->time_left > 0.0)
		    {
			sprintf(text,
			    "%s %s",
			    SAR_MESG_TIME_LEFT,
			    SARDeltaTimeString(
				core_ptr,
				(time_t)objective->time_left
			    )
			);
			SARMessageAdd(scene, text);
		    }
		    break;

		  case SAR_MISSION_OBJECTIVE_PICK_UP:
		  case SAR_MISSION_OBJECTIVE_PICK_UP_ARRIVE_AT:
		    /* Print how many more we need to find */
		    if(objective->humans_need_rescue > 0)
			sprintf(
			    text,
			    "%s, %i %s",
			    SAR_MESG_MISSION_RESCUE_IN_PROGRESS,
			    objective->humans_need_rescue,
			    SAR_MESG_MISSION_MORE_TO_FIND
			);
		    else
			sprintf(
			    text,
			    "%s, %s",
			    SAR_MESG_MISSION_RESCUE_IN_PROGRESS,
			    SAR_MESG_MISSION_ALL_FOUND
			);
		    SARMessageAdd(scene, text);

		    /* Is there a time limit? */
		    if(objective->time_left > 0.0)
		    {
			/* Need to check objective type again */
			switch(objective->type)
			{
			  /* Pick up all and get them to safety */
			  case SAR_MISSION_OBJECTIVE_PICK_UP_ARRIVE_AT:
			    sprintf(
				text,
				"%s %s %s",
				SAR_MESG_TIME_LEFT,
				SARDeltaTimeString(
				    core_ptr,
				    (time_t)objective->time_left
				),
				SAR_MESG_MISSION_TO_GET_ALL_TO_SAFETY
			    );
			    SARMessageAdd(scene, text);
			    break;

			  /* Just pick up all */
			  case SAR_MISSION_OBJECTIVE_PICK_UP:
			    sprintf(
				text,
				"%s %s %s",
				SAR_MESG_TIME_LEFT,
				SARDeltaTimeString(
				    core_ptr,
				    (time_t)objective->time_left
				),
				SAR_MESG_MISSION_TO_PICK_UP_ALL
			    );
			    SARMessageAdd(scene, text);
			    break;
			}
		    }
		    break;
		}

#undef tmp_name_len
	    }	/* Is there a current mission objective? */
	    else
	    {
		/* No current mission objective, implying mission is
		 * completed or failed. Check master mission state.
		 */
		switch(mission->state)
		{
		  case MISSION_STATE_IN_PROGRESS:
		    /* Still in progress but no current objectives? Must be
		     * a delay in the updates (hopefully), print a standby
		     * message.
		     */
		    SARMessageAdd(scene, SAR_MESG_MISSION_STANDBY);
		    break;

		  case MISSION_STATE_FAILED:
		    /* Print generic mission objectives failed message */
		    SARMessageAdd(scene, SAR_MESG_MISSION_FAILED);
		    break;

		  case MISSION_STATE_ACCOMPLISHED:
		    /* Print generic mission accomplished message */
		    SARMessageAdd(scene, SAR_MESG_MISSION_RESCUE_COMPLETE);
		    break;
		}
	    }
	}	/* Valid mission structure given? */
}


/*
 *	Procedure to mark current objective on the given mission structure
 *	as success and go on to the next objective. If there are no more
 *	objectives then the entire mission has been accomplished.
 *
 *	Returns:
 *
 *	-1	General error
 *	0	Went on to next objective
 *	1	All objectives completed, mission accomplished
 */
static int SARMissionDoObjectiveSuccess(
	sar_mission_struct *mission, sar_scene_struct *scene
)
{
	int player_obj_num;
	sar_object_struct *player_obj_ptr;
	sar_mission_objective_struct *objective;

	if((mission == NULL) || (scene == NULL))
	    return(-1);

	/* Get player object references from scene structure */
	player_obj_num = scene->player_obj_num;
	player_obj_ptr = scene->player_obj_ptr;

	/* Get current mission objective */
	objective = SARMissionObjectiveGetCurrentPtr(mission);
	if(objective != NULL)
	{
	    sar_object_aircraft_struct *obj_aircraft_ptr;
	    const char *mesg_ptr = objective->message_success;

	    /* Mark current objective as success */
	    objective->state = SAR_MISSION_OBJECTIVE_STATE_SUCCESS;

	    /* Print success message? */
	    if(mesg_ptr != NULL)
		SARMessageAdd(scene, mesg_ptr);

	    /* Update way point on player object */
	    obj_aircraft_ptr = SAR_OBJ_GET_AIRCRAFT(player_obj_ptr);
	    if(obj_aircraft_ptr != NULL)
	    {
		obj_aircraft_ptr->cur_intercept++;
		if(obj_aircraft_ptr->cur_intercept >= obj_aircraft_ptr->total_intercepts)
		    obj_aircraft_ptr->cur_intercept = obj_aircraft_ptr->total_intercepts - 1;
	    }

	    /* Increment to next objective */
	    mission->cur_objective++;
	}

	/* All objectives completed? */
	if((mission->cur_objective >= mission->total_objectives) ||
	   (objective == NULL)
	)
	{
	    /* Mark mission progress state as accomplished */
	    mission->state = MISSION_STATE_ACCOMPLISHED;

	    /* Set mission accomplished banner */
	    SARBannerMessageAppend(scene, NULL);
	    SARBannerMessageAppend(scene,
		SAR_MESG_MISSION_ACCOMPLISHED_BANNER
	    );
#if 0
	    SARBannerMessageAppend(
		scene,
		((mesg_ptr != NULL) ? mesg_ptr :
		    SAR_MESG_MISSION_ARRIVE_COMPLETE
		)
	    );
#endif

	    /* Do not set continue banner message for mission accomplished */

	    /* Schedual next mission check in 5 seconds which will
	     * detect acomplished state
	     */
	    mission->next_check = cur_millitime + DEF_MISSION_END_INT;

	    /* Return indicating all objectives completed, mission
	     * accomplished
	     */
	    return(1);
	}
	else
	{
	    /* Return indicating went on to next objective */
	    return(0);
	}
}

/*
 *	Procedure to mark current objective on the given mission structure
 *	as failed and mark the entire mission as failed.
 *
 *	Returns:
 *
 *	-1	General error
 *	0	Went on to next objective
 *	1	Entire mission failed
 */
static int SARMissionDoObjectiveFailed(
	sar_mission_struct *mission, sar_scene_struct *scene
)
{
	sar_mission_objective_struct *objective;

	if((mission == NULL) || (scene == NULL))
	    return(-1);

	/* Get current mission objective */
	objective = SARMissionObjectiveGetCurrentPtr(mission);
	if(objective != NULL)
	{
	    const char *mesg_ptr = objective->message_fail;

	    /* Mark current objective as failed */
	    objective->state = SAR_MISSION_OBJECTIVE_STATE_FAILED;

	    /* Print failed message? */
	    if(mesg_ptr != NULL)
		SARMessageAdd(scene, mesg_ptr);

	    /* For now, set current objective to reffer past the
	     * highest objective
	     */
/* This might change later, some objectives might allow fail in
 * the future, thus allowing the next objective to be playable and
 * not have the entire mission fail
 *
 * Check if player has crashed on the scene structure?
 */
	    mission->cur_objective = mission->total_objectives;
	}

	/* All objectives failed? */
	if((mission->cur_objective >= mission->total_objectives) ||
	   (objective == NULL)
	)
	{
	    /* Mark mission progress state as failed */
	    mission->state = MISSION_STATE_FAILED;

	    /* Set mission failed banner */
	    SARBannerMessageAppend(scene, NULL);
	    SARBannerMessageAppend(scene,
		SAR_MESG_MISSION_FAILED_BANNER
	    );
#if 0
	    SARBannerMessageAppend(
		scene,
		(mesg_ptr != NULL) ? mesg_ptr :
		    SAR_MESG_MISSION_ARRIVE_FAILED_TIME
	    );
#endif

	    /* Set continue message */
	    SARBannerMessageAppend(
		scene,
		SAR_MESG_MISSION_POST_FAILED_BANNER
	    );

	    /* Schedual next mission check in 5 seconds which will
	     * detect failed state.
	     */
	    mission->next_check = cur_millitime + DEF_MISSION_END_INT;

	    /* Return indicating entire mission failed */
	    return(1);
	}
	else
	{
	    /* Return indicating went on to next objective */
	    return(0);
	}
}


/*
 *	Destroy notify. This function should be called whenever an
 *	object is *about* to be destroyed.
 */
void SARMissionDestroyNotify(
	sar_core_struct *core_ptr, sar_object_struct *obj_ptr
)
{
	int player_obj_num;
	sar_object_struct *player_obj_ptr;
	sar_mission_objective_struct *objective;
	sar_mission_struct *mission = core_ptr->mission;
	sar_scene_struct *scene = core_ptr->scene;
	const sar_option_struct *opt = &core_ptr->option;
	if((scene == NULL) || (mission == NULL) || (obj_ptr == NULL))
	    return;

	if(opt->runtime_debug)
	    printf(
"SARMissionDestroyNotify():\
 Got destroy notify for object \"%s\".\n",
		obj_ptr->name
	    );

	/* Get references to player object */
	player_obj_num = scene->player_obj_num;
	player_obj_ptr = scene->player_obj_ptr;

	/* Get current mission objective, return if there isn't one */
	objective = SARMissionObjectiveGetCurrentPtr(mission);
	if(objective == NULL)
	    return;

	/* Log object crash event */
	if(True)
	{
	    char *s;
	    const char *name = obj_ptr->name;

	    /* If this is the player object then get player's name */
	    if(player_obj_ptr == obj_ptr)
	    {
		sar_player_stat_struct *pstat = SARPlayerStatCurrent(
		    core_ptr, NULL
		);
		if(pstat != NULL)
		{
		    pstat->crashes++;
		    name = pstat->name;
		}
	    }

	    s = (char *)malloc(
		(80 + STRLEN(name)) * sizeof(char)
	    );
	    sprintf(
		s,
		"%s crashed",
		name
	    );
	    SARMissionLogEvent(
		core_ptr, mission,
		SAR_MISSION_LOG_EVENT_CRASH,
		-1.0,
		&obj_ptr->pos,
		NULL, 0,
		s,
		fname.mission_log
	    );
	    free(s);
	}


	/* Handle by current objective type */
	switch(objective->type)
	{
	  case SAR_MISSION_OBJECTIVE_ARRIVE_AT:
	  case SAR_MISSION_OBJECTIVE_PICK_UP:
	  case SAR_MISSION_OBJECTIVE_PICK_UP_ARRIVE_AT:
	    /* Is the player object the object that was destroyed? */
	    if(player_obj_ptr == obj_ptr)
	    {
		/* Player object has been destroyed, this means that for
		 * this type of objective the objective has failed.
		 * Mark this objective as failed and go on to the next
		 * objective or possibly fail the entire mission
		 */
		SARMissionDoObjectiveFailed(mission, scene);
	    }
	    break;
	}
}

/*
 *	Hoist in notify, this function should be called whenever a number
 *	of human objects have been deleted because they were hoisted 
 *	into the object specified by obj_ptr.
 *
 *	Note that for events of humans entering an object, should call
 *	the notify function SARMissionPassengerEnterNotify().
 */
void SARMissionHoistInNotify(
	sar_core_struct *core_ptr, sar_object_struct *obj_ptr,
	int hoisted_in          /* Number of humans hoisted in */
)
{
	char *s;
	const char *name;
	sar_mission_objective_struct *objective;
	sar_mission_struct *mission = core_ptr->mission;
	sar_scene_struct *scene = core_ptr->scene;
	const sar_option_struct *opt = &core_ptr->option;
	if((scene == NULL) || (mission == NULL) || (obj_ptr == NULL) ||
	   (hoisted_in <= 0)
	)
	    return;

	if(opt->runtime_debug)
	    printf(
"SARMissionHoistInNotify(): Got hoist in notify for object \"%s\" for %i humans.\n",
		obj_ptr->name, hoisted_in
	    );

	/* Get current mission objective, return if there isn't one */
	objective = SARMissionObjectiveGetCurrentPtr(mission);
	if(objective == NULL)
	    return;

	name = obj_ptr->name;

	/* If this is the player then add score */
	if(scene->player_obj_ptr == obj_ptr)
	{
	    sar_player_stat_struct *pstat = SARPlayerStatCurrent(
		core_ptr, NULL
	    );
	    if(pstat != NULL)
	    {
		pstat->score += SAR_POINTS_VICTIM_PICKED_UP * hoisted_in;
		name = pstat->name;
	    }
	}

	/* Log human picked up event */
	s = (char *)malloc((80 + STRLEN(name)) * sizeof(char));
	sprintf(
	    s,
	    "%s picked up %i passenger%s",
	    name,
	    hoisted_in,
	    (hoisted_in == 1) ? "" : "s"
	);
	SARMissionLogEvent(
	    core_ptr, mission,
	    SAR_MISSION_LOG_EVENT_PICKUP,
	    -1.0,
	    &obj_ptr->pos,
	    NULL, 0,
	    s,
	    fname.mission_log
	);
	free(s);

	/* Handle by current objective type */
	switch(objective->type)
	{
	  /* Only need to check for objectives of type
	   * SAR_MISSION_OBJECTIVE_PICK_UP since they get passed the 
	   * instant all humans have been picked up.
	   */
	  case SAR_MISSION_OBJECTIVE_PICK_UP:
	    /* Original comment:
	     * Reduce number of humans that still need rescue, note that
	     * we do not care which object hoisted in these humans.
	     *
	     * New comment:
	     * Do not reduce number of humans that still need rescue
	     * because that has be done earlier by
	     * SARMissionPassengersEnterNotify().
	     */
	    /* objective->humans_need_rescue -= hoisted_in; */

	    /* All humans picked up? */
	    if(objective->humans_need_rescue <= 0)
	    {
		/* This objective has been passed with success, call
		 * procedure to go on to the next objective.
		 */
		SARMissionDoObjectiveSuccess(mission, scene);
	    }
	    break;
	}
}

/*
 *      Passengers enter notify, this function should be called
 *      whenever a number of people have volentarly (not hoisted in)
 *	entered the object specified by obj_ptr.
 *
 *      The value passengers_entered indicates how many passengers
 *      entered the given object (since the object's current passenger
 *      count is not checked).
 */
void SARMissionPassengersEnterNotify(
	sar_core_struct *core_ptr, sar_object_struct *obj_ptr,
	int passengers_entered	/* Number of passengers entered */
)
{
	char *s;
	const char *name;
	sar_mission_objective_struct *objective;
	sar_mission_struct *mission = core_ptr->mission;
	sar_scene_struct *scene = core_ptr->scene;
	const sar_option_struct *opt = &core_ptr->option;
	if((scene == NULL) || (mission == NULL) || (obj_ptr == NULL) ||
	   (passengers_entered <= 0)
	)
	    return;

	if(opt->runtime_debug)
	    printf(
"SARMissionPassengersEnterNotify(): Got enter notify for object \"%s\" for %i humans.\n",
		obj_ptr->name, passengers_entered
	    );

	/* Get current mission objective, return if there isn't one */
	objective = SARMissionObjectiveGetCurrentPtr(mission);
	if(objective == NULL)
	    return;

	name = obj_ptr->name;

	/* If this is the player then add score */
	if(scene->player_obj_ptr == obj_ptr)
	{
	    sar_player_stat_struct *pstat = SARPlayerStatCurrent(
		core_ptr, NULL
	    );
	    if(pstat != NULL)
	    {
		pstat->score += SAR_POINTS_VICTIM_PICKED_UP * passengers_entered;
		name = pstat->name;
	    }
	}

	/* Log human enter event */
	s = (char *)malloc((80 + STRLEN(name)) * sizeof(char));
	sprintf(
	    s,
	    "%s picked up %i passenger%s",
	    name,
	    passengers_entered,
	    (passengers_entered == 1) ? "" : "s"
	);
	SARMissionLogEvent(
	    core_ptr, mission,
	    SAR_MISSION_LOG_EVENT_PICKUP,
	    -1.0,
	    &obj_ptr->pos,
	    NULL, 0,
	    s,
	    fname.mission_log
	);
	free(s);

	/* Handle by current objective type */
	switch(objective->type)
	{
	  /* Only need to check for objectives of type
	   * SAR_MISSION_OBJECTIVE_PICK_UP since they get passed the
	   * instant all humans have been picked up.
	   */
	  case SAR_MISSION_OBJECTIVE_PICK_UP:
	    /* Reduce number of humans that still need rescue, note that
	     * we do not care which object hoisted in these humans.
	     */
	    objective->humans_need_rescue -= passengers_entered;

	    /* All humans picked up? */
	    if(objective->humans_need_rescue <= 0)
	    {
		/* This objective has been passed with success, call
		 * procedure to go on to the next objective.
		 */
		SARMissionDoObjectiveSuccess(mission, scene);
	    }
	    break;
	}
}

/*
 *      Passengers leave notify, this function should be called
 *      whenever the given object safely lets off passengers.
 *
 *      The value passengers_left indicates how many passengers
 *      left the given object (since the object's current passenger
 *      count is not checked).
 */
void SARMissionPassengersLeaveNotify(
	sar_core_struct *core_ptr, sar_object_struct *obj_ptr,
	int passengers_left	/* Number of passengers that left */
)
{
	sar_mission_struct *mission = core_ptr->mission;
	if((mission == NULL) || (obj_ptr == NULL))
	    return;

/* This is not implemented yet, when it does however it will check if
 * the number of passengers let off meets the mission requirements
 * (if this is a rescue mission) and in which case then mark the
 * mission as successfully ended.
 */

}

/*
 *	Land notify callback, this function should be called whenever an
 *	object specified by obj_ptr has landed.
 *
 *	The ground_contact is an array of object indices that are the
 *	indices of objects that the given object has landed on. This value
 *	can be NULL if there are no objects that the given object has
 *	landed on.
 */
void SARMissionLandNotify(
	sar_core_struct *core_ptr, sar_object_struct *obj_ptr,
	const int *ground_contact, int total_ground_contacts
)
{
	int passengers_unloaded;
	sar_mission_objective_struct *objective;
	sar_mission_struct *mission = core_ptr->mission;
	sar_scene_struct *scene = core_ptr->scene;
	const sar_option_struct *opt = &core_ptr->option;
	if((scene == NULL) || (mission == NULL) || (obj_ptr == NULL))
	    return;

	if(opt->runtime_debug)
	    printf(
"SARMissionLandNotify(): Got land notify for object \"%s\".\n",
		    obj_ptr->name
	    );

	/* Get current mission objective, return if there isn't one */
	objective = SARMissionObjectiveGetCurrentPtr(mission);
	if(objective == NULL)
	    return;

#define ACCOUNT_PASSENGERS_DROPPED(i)		\
{ if(obj_ptr != NULL) {				\
  char *s;					\
  const char *name = obj_ptr->name;		\
						\
  /* Add score if this is the player */		\
  if(scene->player_obj_ptr == obj_ptr)		\
  {						\
   sar_player_stat_struct *pstat = SARPlayerStatCurrent(core_ptr, NULL); \
   if(pstat != NULL)				\
   {						\
    pstat->rescues += i;			\
    pstat->score += SAR_POINTS_VICTIM_RESCUED * i;	\
    name = pstat->name;				\
   }						\
  }						\
						\
  /* Log the number of passengers dropped */	\
  s = (char *)malloc(				\
   (256 + STRLEN(obj_ptr->name)) * sizeof(char)	\
  );						\
  sprintf(					\
   s,						\
   "%s dropped off %i passenger%s",		\
   name, i, (i == 1) ? "" : "s"			\
  );						\
  SARMissionLogEvent(				\
   core_ptr, mission,				\
   SAR_MISSION_LOG_EVENT_DROPOFF,		\
   -1.0f,					\
   (obj_ptr != NULL) ? &obj_ptr->pos : NULL,	\
   NULL, 0,					\
   s,						\
   fname.mission_log				\
  );						\
  free(s);					\
} }

	/* Handle by current objective type */
	switch(objective->type)
	{
	  case SAR_MISSION_OBJECTIVE_ARRIVE_AT:
	    /* Not landed at right place? */
	    if(!SARMissionGroundContactMatchName(
		core_ptr, scene,
		core_ptr->object, core_ptr->total_objects,
		ground_contact, total_ground_contacts,
		objective->arrive_at_name
	    ))
		break;

	    /* Unload passengers */
/* We may need to do a more prettier unload and also check for when
 * exactly the passengers are actually unloaded
 */
	    passengers_unloaded = SARSimOpPassengersUnloadAll(
		scene, obj_ptr
	    );
	    if(passengers_unloaded > 0)
	    {
		ACCOUNT_PASSENGERS_DROPPED(passengers_unloaded);
	    }

	    /* Is the given object the player object? */
	    if((scene->player_obj_ptr == obj_ptr) &&
	       (objective->state == SAR_MISSION_OBJECTIVE_STATE_INCOMPLETE) &&
	       (mission->state == MISSION_STATE_IN_PROGRESS)
	    )
	    {
		/* This objective has been passed with success, call
		 * procedure to go on to the next objective.
		 */
		SARMissionDoObjectiveSuccess(mission, scene);
	    }
	    break;

	  case SAR_MISSION_OBJECTIVE_PICK_UP:
	    /* Skip this case, since it has no arrive at object there
	     * is no way to check if landed at the correct place.
	     * Do not unload passengers either.
	     */
	    break;

	  case SAR_MISSION_OBJECTIVE_PICK_UP_ARRIVE_AT:
	    /* Landed at the right place? */
	    if(SARMissionGroundContactMatchName(
		core_ptr, scene,
		core_ptr->object, core_ptr->total_objects,
		ground_contact, total_ground_contacts,
		objective->arrive_at_name
	    ))
	    {
		int	*passengers = NULL,
			*total_passengers = NULL;
		int passengers_onboard;

		/* Get number of passengers on board the given object,
		 * note that the given object may not be the player object
		 * but it is okay because any object is accepted to unload
		 * passengers for this kind of objective
		 */
		SARObjGetOnBoardPtr(
		    obj_ptr,
		    NULL, &passengers, &total_passengers,
		    NULL,
		    NULL, NULL
		);
		passengers_onboard = ((passengers != NULL) ?
		    *passengers : 0
		);

		/* Unload passengers */
/* We may need to do a more prettier unload, letting passengers leave
 * and also check for *when* passengers have all left (but not in this
 * land notify function)
 */
		passengers_unloaded = SARSimOpPassengersUnloadAll(
		    scene, obj_ptr
		);
		if(passengers_unloaded > 0)
		{
		    ACCOUNT_PASSENGERS_DROPPED(passengers_unloaded);
		}

		/* Reduce number of humans that need rescue on the 
		 * objective structure
		 */
		objective->humans_need_rescue -= passengers_onboard;

		/* If no more humans need rescue then this objective is
		 * done and will be marked as success
		 */
		if((objective->humans_need_rescue <= 0) &&
		   (objective->state == SAR_MISSION_OBJECTIVE_STATE_INCOMPLETE) &&
		   (mission->state == MISSION_STATE_IN_PROGRESS)
		)
		{
		    /* This objective has been passed with success, call
		     * procedure to go on to the next objective.
		     */
		    SARMissionDoObjectiveSuccess(mission, scene);
		}
	    }
	    break;

	}	/* Handle by current objective type */

#undef ACCOUNT_PASSENGERS_DROPPED
}


/*
 *	Manages the given mission objective. This function should be 
 *	called from SARMissionManage().
 */
static void SARMissionObjectiveManage(
	sar_core_struct *core_ptr, sar_mission_struct *mission,
	sar_mission_objective_struct *objective
)
{
	int player_obj_num;
	sar_object_struct *player_obj_ptr;
	sar_scene_struct *scene = core_ptr->scene;
	const sar_option_struct *opt = &core_ptr->option;
	if((scene == NULL) || (mission == NULL) || (objective == NULL))
	    return;

	/* Get pointer to player object */
	player_obj_num = scene->player_obj_num;
	player_obj_ptr = scene->player_obj_ptr;

	/* Handle by objective type */
	switch(objective->type)
	{
	  case SAR_MISSION_OBJECTIVE_ARRIVE_AT:
	    /* Is there a time limit? */
	    if(objective->time_left > 0.0f)
	    {
		/* Reduce time left */
		objective->time_left -= (float)lapsed_millitime *
		    time_compression / 1000.0f;

		/* Time limit exceeded? */
		if(objective->time_left <= 0.0f)
		{
		    if(opt->runtime_debug)
			printf(
"SARMissionObjectiveManage(): Mission objective time expired to arrive.\n"
			);

		    /* Time limit exceeded and did not reach arrive at
		     * object. Mark this objective as failed and go on to
		     * the next objective or possibly fail entire mission.
		     */
		    SARMissionDoObjectiveFailed(mission, scene);
		}
	    }
	    break;

	  case SAR_MISSION_OBJECTIVE_PICK_UP:
	    /* Is there a time limit? */
	    if(objective->time_left > 0.0f)
	    {
		/* Reduce time left */
		objective->time_left -= (float)lapsed_millitime *
		    time_compression / 1000.0f;

		/* Time limit exceeded? */
		if(objective->time_left <= 0.0f)
		{
		    /* Time limit has exceeded, check if all humans that
		     * still need to be brought to safety are in the
		     * player object.
		     */
		    int passengers = SARMissionGetObjectPassengers(
			core_ptr, player_obj_ptr
		    );
		    if((objective->humans_need_rescue - passengers) > 0)
		    {
			if(opt->runtime_debug)
			    printf(
"SARMissionObjectiveManage(): Mission objective time expired to pick up.\n"
			    );

			/* There are still more unpicked up humans out
			 * there. Mark this objective as failed and go on
			 * to the next objective or possibly fail entire
			 * mission.
			 */
			SARMissionDoObjectiveFailed(mission, scene);
		    }
		}
	    }
	    break;

	  case SAR_MISSION_OBJECTIVE_PICK_UP_ARRIVE_AT:
	    /* Is there a time limit? */
	    if(objective->time_left > 0.0f)
	    {
		/* Reduce time left */
		objective->time_left -= (float)lapsed_millitime *
		    time_compression / 1000.0f;

		/* Time limit exceeded? */
		if(objective->time_left <= 0.0f)
		{
		    if(opt->runtime_debug)
			printf(
"SARMissionObjectiveManage(): Mission objective time expired to pick up and arrive.\n"
			);

		    /* Time limit exceeded and not all have been rescued
		     * and brought to safety. Mark this objective as
		     * failed and go on to the next objective or possibly
		     * fail entire mission.
		     */
		    SARMissionDoObjectiveFailed(mission, scene);
		}
	    }
	    break;
	}
}

/*
 *	Manages mission, this function should be called once per loop.
 *
 *	Return codes are as follows:
 *
 *	-1	Error.
 *	0	Nothing happened or not time to manage.
 *	1	Mission end success.
 *	2	Mission end failure.
 */
int SARMissionManage(sar_core_struct *core_ptr)
{
	sar_mission_struct *mission = core_ptr->mission;
	sar_mission_objective_struct *objective;
	if(mission == NULL)
	    return(-1);

	/* Update time spent on mission */
	mission->time_spent += (float)lapsed_millitime *
	    time_compression / 1000.0f;

	/* Not time to manage mission yet? */
	if(mission->next_check > cur_millitime)
	{
	    /* Skip and return success */
	    return(0);
	}
	else
	{
	    /* Yes its time to check mission status */

	    /* Schedual next time to check mission.
	     *
	     * Note that mission->next_check may be modified again by
	     * one of the mission manage handler functions farther
	     * below.
	     */
	    mission->next_check = cur_millitime + mission->check_int;
	}

	/* Get current mission objective */
	objective = SARMissionObjectiveGetCurrentPtr(mission);


	/* Mission no longer in progress? */
	if((mission->state != MISSION_STATE_IN_PROGRESS) ||
	   (objective == NULL)
	)
	{
	    /* Check the resulting state of the mission */
	    switch(mission->state)
	    {
	      case MISSION_STATE_ACCOMPLISHED:
		return(1);
		break;

	      case MISSION_STATE_FAILED:
		return(2);
		break;

	      default:
		/* Unsupported mission state, return error */
		fprintf(
		    stderr,
"SARMissionManage(): Unsupported mission state %i, returning as error.\n",
		    mission->state
		);
		return(-1);
		break;
	    }
	}

	/* Mission still in progress, manage current mission objective */
	SARMissionObjectiveManage(core_ptr, mission, objective);

	return(0);
}
