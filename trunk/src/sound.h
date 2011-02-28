/*
			       Sound Server IO
 */

#ifndef SOUND_H
#define SOUND_H

#include <sys/types.h>

/*
 *	Sound flags type:
 */
#define snd_flags_t	unsigned int


/*
 *	Sound priority levels:
 */
#define SND_PRIORITY_BACKGROUND		0
#define SND_PRIORITY_FOREGROUND		1
#define SND_PRIORITY_PREEMPT		2


/*
 *	Sound play options:
 */
#define SND_PLAY_OPTION_MUTE		(1 << 0)
#define SND_PLAY_OPTION_REPEATING	(1 << 1)

#ifdef HAVE_SDL_MIXER
#define SDL_H
#include <SDL/SDL.h>
#include <SDL/SDL_mixer.h>
#endif

/*
 *      Sound object play structure:
 *
 *	For keeping track of currently playing sound objects.
 */
typedef struct {

	/* Pointer to sound server specific play id. */
	void *data;

	/* Last set volume (from 0.0 to 1.0). This is used to check
	 * if sound play values actually need to be adjusted when
	 * requested.
	 */
	float volume_left, volume_right;

	/* Applied sample rate. */
	int sample_rate;

	/* Play options. */
	snd_flags_t options;
        #ifdef HAVE_SDL_MIXER
        Mix_Chunk *chunk;
        #endif

} snd_play_struct;

#define SND_PLAY(p)	((snd_play_struct *)(p))


/*
 *	Recorder (sound server connection) structure:
 */
typedef struct {

#define SNDSERV_TYPE_NONE	0
#define SNDSERV_TYPE_Y		1
#define SNDSERV_TYPE_SDL        2
#define SOUND_DEFAULT           99

	/* Sound server type, one of SNDSERV_TYPE_*. */
	int type;

	/* Opaque pointer to the connection to the sound server. */
	void *con;

	/* Current Audio parameters. */
	int sample_rate;
	int sample_size;
	int channels;
	int bytes_per_cycle;

	/* Current background music sound object being played (can be
	 * NULL to indicate no background music being played)
	 */
	snd_play_struct *background_music_sndobj;

} snd_recorder_struct;

#define SND_RECORDER(p)	((snd_recorder_struct *)(p))


extern snd_recorder_struct *SoundInit(
	void *core,
	int type,
	const char *connect_arg,
	const char *start_arg,
	void *window
);
extern int SoundManageEvents(snd_recorder_struct *recorder);
extern void SoundShutdown(snd_recorder_struct *recorder);

extern int SoundChangeMode(
	snd_recorder_struct *recorder, const char *mode_name
);

extern snd_play_struct *SoundStartPlay(
	snd_recorder_struct *recorder,
	const char *object,	/* Full path to object. */
	float volume_left,	/* Volume, from 0.0 to 1.0. */
	float volume_right,
	int sample_rate,	/* Applied sample rate, can be 0. */
	snd_flags_t options	/* Any of SND_PLAY_OPTION_*. */
);
extern void SoundStartPlayVoid(
	snd_recorder_struct *recorder,
	const char *object,	/* Full path to object. */
	float volume_left,     /* Volume, from 0.0 to 1.0. */
	float volume_right,
	int sample_rate,        /* Applied sample rate, can be 0. */
	snd_flags_t options     /* Any of SND_PLAY_OPTION_*. */
);
extern void SoundPlayMute(
	snd_recorder_struct *recorder, snd_play_struct *snd_play,
	int mute  
);
extern void SoundChangePlayVolume(
	snd_recorder_struct *recorder, snd_play_struct *snd_play,
	float volume_left,	/* Volume, from 0.0 to 1.0. */
	float volume_right
);
extern void SoundChangePlaySampleRate(
	snd_recorder_struct *recorder, snd_play_struct *snd_play,
	int sample_rate		/* Applied sample rate, can be 0. */
);
extern void SoundStopPlay(
	snd_recorder_struct *recorder, snd_play_struct *snd_play
);

extern int SoundMusicStartPlay(
	snd_recorder_struct *recorder,
	const char *object,	/* Full path to object. */
	int repeats		/* Number of repeats, -1 for infinate. */
);
extern int SoundMusicIsPlaying(snd_recorder_struct *recorder);
extern void SoundMusicStopPlay(snd_recorder_struct *recorder);

extern void SoundMixerGet(
	snd_recorder_struct *recorder, const char *channel_name,
	float *left, float *right       /* 0.0 to 1.0 */
);
extern void SoundMixerSet(
	snd_recorder_struct *recorder, const char *channel_name,
	float left, float right         /* 0.0 to 1.0 */
);


#endif	/* SOUND_H */
