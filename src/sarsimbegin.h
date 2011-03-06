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
	SAR Simulation Starting

	Switches from menus to simulation.
 */

#ifndef SARSIMBEGIN_H
#define SARSIMBEGIN_H

#include "obj.h"
#include "sar.h"

extern int SARSimBeginMission(
	sar_core_struct *core_ptr,
	const char *mission_file
);
extern int SARSimBeginFreeFlight(
	sar_core_struct *core_ptr,
	const char *scene_file,
	const char *player_file,                /* Player aircraft. */
	sar_position_struct *start_pos,
	sar_direction_struct *start_dir,
	const char *weather_preset_name,
	Boolean free_flight_system_time
);

#endif	/* SARSIMBEGIN_H */
