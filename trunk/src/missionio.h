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
			 SAR Mission File IO
 */

#ifndef MISSIONIO_H
#define MISSIONIO_H


#include <sys/types.h>
#include "menu.h"
#include "mission.h"
#include "sar.h"


extern char *SARMissionLoadDescription(const char *filename);
extern void SARMissionLoadSceneToMenuMap(
	sar_core_struct *core_ptr,
	sar_menu_struct *m,
	const char *filename            /* Mission file name. */
);
extern void SARMissionLoadMissionLogToMenuMap(
	sar_core_struct *core_ptr,
	sar_menu_struct *m,
	const char *filename            /* Mission log file name. */
);

extern sar_mission_struct *SARMissionLoadFromFile(
	sar_core_struct *core_ptr,
	const char *filename,
	void *client_data,
	int (*progress_func)(void *, long, long)
);

extern void SARMissionLogReset(
	sar_core_struct *core_ptr, sar_mission_struct *mission,
	const char *filename            /* Mission log file name. */
);
extern void SARMissionLogEvent(
	sar_core_struct *core_ptr, sar_mission_struct *mission,
	int event_type,                 /* One of SAR_LOG_EVENT_*. */
	float tod,			/* Time of day in seconds since midnight. */
	sar_position_struct *pos,	/* Position of event. */
	const float *value,		/* Additional values. */
	int total_values,               /* Total number of additional values. */
	const char *message,            /* The message. */
	const char *filename            /* Mission log file name. */
);


#endif	/* MISSIONIO_H */
