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

/*
		SAR Mission Management, Callbacks, and Procedures
 */

#ifndef MISSION_H
#define MISSION_H

#include "obj.h"


/*
 *	Mission log codes:
 *
 *	These are codes used to determine the event type found in
 *	mission log events
 */
#define SAR_MISSION_LOG_EVENT_COMMENT		0
#define SAR_MISSION_LOG_EVENT_POSITION		1

#define SAR_MISSION_LOG_EVENT_TAKEOFF		10
#define SAR_MISSION_LOG_EVENT_LAND		11
#define SAR_MISSION_LOG_EVENT_CRASH		12
#define SAR_MISSION_LOG_EVENT_PICKUP		13
#define SAR_MISSION_LOG_EVENT_DROPOFF		14


/*
 *	Mission objective types:
 */
/* Must reach object specified by name arrive_at_name within a (optional)
 * time limit set by time_limit (if positive)
 */
#define SAR_MISSION_OBJECTIVE_ARRIVE_AT			1
/* Must pick up atleast humans_need_rescue many humans within a (optional)
 * time limit set by time_limit (if positive)
 */
#define SAR_MISSION_OBJECTIVE_PICK_UP			2
/* Must pick up atleast humans_need_rescue many humans within a (optional)
 * time limit set by time_limit (if positive) and reach the object
 * specified by arrive_at_name
 */
#define SAR_MISSION_OBJECTIVE_PICK_UP_ARRIVE_AT		3


/*
 *	Mission objective structure:
 */
typedef struct {

	int	type;	/* One of SAR_MISSION_OBJECTIVE_* */

#define SAR_MISSION_OBJECTIVE_STATE_INCOMPLETE	0	/* In progress */
#define SAR_MISSION_OBJECTIVE_STATE_SUCCESS	1
#define SAR_MISSION_OBJECTIVE_STATE_FAILED	2
	int	state;	/* One of SAR_MISSION_OBJECTIVE_STATE_* */


	/* Time left in seconds for this objective to be completed.
	 * Note that type float is used, because the milliseconds
	 * resolution needs to be preserved
	 *
	 * A non-positive value implies that there is no time_limit
	 */
	float	time_left;


	/* Number of humans that still need rescue (haven't been brought
	 * to safety) for this objective, a non-positive total_humans
	 * value implies no humans need to be rescued
	 */
	int	humans_need_rescue;

	/* Total number of humans that needed to be rescued. This only
	 * reflects the number of humans that need to be rescued, not
	 * the total number of humans in the scene. If this value is
	 * not positive then that implies no humans need to be rescued
	 */
	int	total_humans;


	/* Name of object to land at for this objective. The value can be
	 * NULL to indicate n/a, but be careful as a NULL value for
	 * objectives of type SAR_MISSION_OBJECTIVE_ARRIVE_AT will always
	 * fail under that situation
	 */
	char	*arrive_at_name;

	/* Success and fail message for this objective, if this objective
	 * is passed successfully then message_success is printed
	 *
	 * Otherwise message_fail is printed. Any of these values can
	 * be NULL for no message
	 */
	char	*message_success,
		*message_fail;

} sar_mission_objective_struct;


/*
 *	Base mission structure:
 */
typedef struct {

#define MISSION_STATE_IN_PROGRESS	0
#define MISSION_STATE_FAILED		1
#define MISSION_STATE_ACCOMPLISHED	2
	int	state;	/* Mission state, one of MISSION_STATE_* */

	char	*title;			/* Title of this mission */
	char	*description;		/* Description of this mission
					 * objectives
					 */

	char	*start_location_name;	/* Name of starting location */

	char	*scene_file;		/* Full path to scene file */
	char	*player_model_file;	/* Full path to player object model file */
	char	*player_stats_file;	/* Full path to player stats file */

	/* Objectives */
	sar_mission_objective_struct	*objective;
	int	total_objectives;

	/* Current objective */
	int	cur_objective;


	/* Time spent so far this mission, in seconds */
	float	time_spent;

	/* Timmings, in milliseconds */
	time_t	next_check, check_int,
		next_log_position, log_position_int;

} sar_mission_struct;
#define SAR_MISSION(p)		((sar_mission_struct *)(p))


#endif	/* MISSION_H */
