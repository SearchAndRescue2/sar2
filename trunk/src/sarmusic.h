/*
	                  SAR Music Management

	See sound.c for low-level to sound output library functions.
 */

#ifndef SARMUSIC_H
#define SARMUSIC_H

#include "sound.h"
#include "sar.h"


/*
 *	Music Codes:
 *
 *	These correspond to an index on the core structure's list of
 *	music file references
 */
#define SAR_MUSIC_ID_DEFAULT			0
#define SAR_MUSIC_ID_SPLASH			SAR_MUSIC_CODE_DEFAULT
#define SAR_MUSIC_ID_MENUS			10
#define SAR_MUSIC_ID_LOADING_SIMULATION		50
#define SAR_MUSIC_ID_MISSION_FAILED		60
#define SAR_MUSIC_ID_MISSION_SUCCESS		61

#define SAR_MUSIC_ID_SIMULATION_ONGROUND		100
#define SAR_MUSIC_ID_SIMULATION_ONGROUND_ENTER		101
#define SAR_MUSIC_ID_SIMULATION_INFLIGHT_DAY		110
#define SAR_MUSIC_ID_SIMULATION_INFLIGHT_DAY_ENTER	111
#define SAR_MUSIC_ID_SIMULATION_INFLIGHT_NIGHT		120
#define SAR_MUSIC_ID_SIMULATION_INFLIGHT_NIGHT_ENTER	121
#define SAR_MUSIC_ID_SIMULATION_RESCUE			150
#define SAR_MUSIC_ID_SIMULATION_RESCUE_ENTER		151


extern void SARMusicUpdate(sar_core_struct *core_ptr);

#endif	/* SARMUSIC_H */
