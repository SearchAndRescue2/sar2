/*
	Object sound source utilities and updating of object sounds.
 */

#ifndef OBJSOUND_H
#define OBJSOUND_H

#include "sound.h"
#include "obj.h"

/* Object sound source */
extern sar_sound_source_struct *SARSoundSourceNew(
	const char *name,
	const char *filename,
	const char *filename_far,
	float range,			/* In meters */
	float range_far,                /* In meters */
	const sar_position_struct *pos,
	float cutoff,			/* In radians */
	const sar_direction_struct *dir,
	int sample_rate_limit		/* In Hz */
);
extern void SARSoundSourceDelete(sar_sound_source_struct *sndsrc);
extern int SARSoundSourceMatchFromList(
	sar_sound_source_struct **list, int total,
	const char *name
);
extern void SARSoundSourcePlayFromList(
	snd_recorder_struct *recorder,
	sar_sound_source_struct **list, int total,
	const char *name,
	const sar_position_struct *pos,		/* Object position */
	const sar_direction_struct *dir,	/* Object direction */
	const sar_position_struct *ear_pos
);
extern snd_play_struct *SARSoundSourcePlayFromListRepeating(
	snd_recorder_struct *recorder,
	sar_sound_source_struct **list, int total,
	const char *name, int *sndsrc_num
);

/* Sound updating */
extern void SARSoundEngineMute(
	snd_recorder_struct *recorder,
	snd_play_struct *inside_snd_play,
	snd_play_struct *outside_snd_play
);
extern void SARSoundEngineUpdate(
	snd_recorder_struct *recorder,
	sar_sound_source_struct **list, int total,
	snd_play_struct *inside_snd_play,
	int inside_sndsrc_num,
	snd_play_struct *outside_snd_play,
	int outside_sndsrc_num,
	char ear_in_cockpit,
	sar_engine_state engine_state,	/* One of SAR_ENGINE_STATE_* */
	float throttle,			/* Throttle 0.0 to 1.0 */
	float distance_to_camera	/* In meters */
);
extern int SARSoundStallUpdate(
	snd_recorder_struct *recorder,
	sar_sound_source_struct **list, int total,
	void **stall_snd_play,
	int stall_sndsrc_num,
	char ear_in_cockpit,
	sar_flight_model_type flight_model_type,
	char is_landed,
	float speed, float speed_stall
);
extern void SARSoundOverSpeedUpdate(
	snd_recorder_struct *recorder,
	sar_sound_source_struct **list, int total,
	void **overspeed_snd_play,
	int overspeed_sndsrc_num,
	char ear_in_cockpit, char is_overspeed
);

#endif	/* OBJSOUND_H */
