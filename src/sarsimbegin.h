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
