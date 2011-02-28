/*
	SAR Scene Sound Management

	Updates the continuous playing of sound and music on the scene.
 */

#ifndef SCENESOUND_H
#define SCENESOUND_H

#include "sar.h"

extern void SARSceneSoundUpdate(
	sar_core_struct *core_ptr,
	Boolean engine_sounds,
	Boolean event_sounds,
	Boolean voice_sounds,
	Boolean music
);

#endif	/* SCENESOUND_H */
