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
                    SAR Simulation Utility Functions
 */

#ifndef SIMUTILS_H
#define SIMUTILS_H

#include <sys/types.h>
#include "sfm.h"
#include "sound.h"
#include "obj.h"
#include "sar.h"

extern float SARSimGetFlatContactRadius(sar_object_struct *obj_ptr);
extern sar_object_struct *SARSimMatchObjectFromFDM(
        sar_object_struct **list, int total,
        SFMModelStruct *fdm, int *index
);
extern float SARSimThrottleOutputCoeff(
        int flight_model_type,
        float throttle,
        float collective, float collective_range
);
extern float SARSimStallSpeed(
	const sar_object_aircraft_struct *obj_aircraft_ptr
);

/* Position Realization */
extern void SARSimWarpObjectRelative(   
        sar_scene_struct *scene, sar_object_struct *obj_ptr,
        sar_object_struct **ptr, int total, int ref_obj_num,
        sar_position_struct *offset_pos,
        sar_direction_struct *offset_dir
);
extern void SARSimWarpObject(
        sar_scene_struct *scene, sar_object_struct *obj_ptr,
        sar_position_struct *new_pos,
        sar_direction_struct *new_dir
);

/* Object Parts */
extern void SARSimUpdatePart(
	sar_scene_struct *scene,
        sar_object_struct *obj_ptr, sar_obj_part_struct *part_ptr,
	snd_recorder_struct *recorder, int play_sound, int ear_in_cockpit
);
extern void SARSimOpAirBrakes(
        sar_scene_struct *scene,
        sar_object_struct ***ptr, int *total,
        sar_object_struct *obj_ptr, int air_brakes_state,
	snd_recorder_struct *recorder, int play_sound, int ear_in_cockpit
);
extern void SARSimOpLandingGear(
        sar_scene_struct *scene,
        sar_object_struct ***ptr, int *total,
        sar_object_struct *obj_ptr, int gear_state,
	snd_recorder_struct *recorder, int play_sound, int ear_in_cockpit
);
extern void SARSimOpDoorRescue(
        sar_scene_struct *scene,
        sar_object_struct ***ptr, int *total,
        sar_object_struct *obj_ptr, int state
);

/* Rotor */
extern void SARSimRotorUpdateControls(
        sar_obj_rotor_struct *rotor_ptr,
        float pitch, float bank
);

/* Engine */
extern sar_engine_state SARSimGetEngineState(sar_object_struct *obj_ptr);
extern void SARSimOpEngine(
        sar_scene_struct *scene,
        sar_object_struct ***ptr, int *total,
        sar_object_struct *obj_ptr, sar_engine_state engine_state,
        snd_recorder_struct *recorder, char play_sound, char ear_in_cockpit
);
extern void SARSimPitchEngine(
        sar_scene_struct *scene,
        sar_object_struct ***ptr, int *total,
        sar_object_struct *obj_ptr, int pitch_state
);

/* Object Lights */
extern void SARSimUpdateLights(sar_object_struct *obj_ptr);
extern int SARSimGetLightsState(sar_object_struct *obj_ptr);
extern void SARSimOpLights( 
        sar_object_struct *obj_ptr, int state
);
extern int SARSimGetStrobesState(sar_object_struct *obj_ptr);
extern void SARSimOpStrobes(
        sar_object_struct *obj_ptr, int state
);
extern int SARSimGetAttenuateState(sar_object_struct *obj_ptr);
extern void SARSimOpAttenuate(
        sar_object_struct *obj_ptr, int state
);

/* Repair & Refuel */
extern int SARSimOpRepair(
        sar_scene_struct *scene, sar_object_struct *obj_ptr
);
extern int SARSimOpRefuel(
        sar_scene_struct *scene, sar_object_struct *obj_ptr
);

/* Passenger Operations */
extern int SARSimOpPassengersSetLeave(
        sar_scene_struct *scene, sar_object_struct *obj_ptr,
        int passengers_leave_pending, int passengers_drop_pending
);
extern int SARSimOpPassengersUnloadAll(
        sar_scene_struct *scene, sar_object_struct *obj_ptr
);

/* Fuel */
extern float SARSimOpTransferFuelFromTanks(
        sar_scene_struct *scene, sar_object_struct *obj_ptr
);
extern int SARSimOpDropFuelTankNext(  
        sar_scene_struct *scene,
        sar_object_struct ***ptr, int *total,
        int obj_num, sar_object_struct *obj_ptr
);

/* Smoke Respawn */
extern void SARSimSmokeSpawn(
        sar_core_struct *core_ptr, sar_object_struct *obj_ptr,
        sar_object_smoke_struct *obj_smoke_ptr
);

/* Slew */
extern int SARSimIsSlew(sar_object_struct *obj_ptr);
extern void SARSimSetSlew(
	sar_object_struct *obj_ptr, int enter_slew
);

/* Hoist & Passenger Operations */
extern int SARSimDoPickUpHuman(
	sar_scene_struct *scene,
        sar_object_struct *obj_ptr,
	sar_object_human_struct *obj_human_ptr,
	int human_obj_num
);
extern int SARSimDoHoistIn(
        sar_core_struct *core_ptr,
        sar_object_struct *obj_ptr 
);
extern int SARSimBoardObject(
        sar_core_struct *core_ptr,
        sar_object_struct *tar_obj_ptr, int src_obj_num
);

/* Camera Setting */
extern void SARSimSetFlyByPosition(
	sar_scene_struct *scene,
	sar_object_struct ***ptr, int *total,
	sar_object_struct *obj_ptr,
	sar_position_struct *pos_result
);


#endif	/* SIMUTILS_H */
