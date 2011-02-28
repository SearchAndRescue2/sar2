#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>

#ifdef HAVE_Y2
# include <Y2/Y.h>	/* We get Y_H defined by #including this. */
# include <Y2/Ylib.h>
# include <Y2/Ymixercodes.h>
#endif	/* HAVE_Y2 */

#include "../include/string.h"
#include "../include/disk.h"

#include "sound.h"
#include "sar.h"


#ifdef Y_H
/* Default values for Y. */
# ifndef DEF_Y_CONNECT_ARG
#  define DEF_Y_CONNECT_ARG		"127.0.0.1:9433"
# endif
# ifndef DEF_Y_AUDIO_MODE_NAME
#  define DEF_Y_AUDIO_MODE_NAME		"Default"
# endif
#endif	/* Y_H */



snd_recorder_struct *SoundInit(
	void *core,
	int type,
	const char *connect_arg,
	const char *start_arg,
	void *window
);
int SoundManageEvents(snd_recorder_struct *recorder);
void SoundShutdown(snd_recorder_struct *recorder);

int SoundChangeMode(
	snd_recorder_struct *recorder, const char *mode_name
);

snd_play_struct *SoundStartPlay(
	snd_recorder_struct *recorder,
	const char *object,
	float volume_left, float volume_right,
	int sample_rate,
	snd_flags_t options
);
void SoundStartPlayVoid(
	snd_recorder_struct *recorder,
	const char *object,
	float volume_left, float volume_right,
	int sample_rate,
	snd_flags_t options
);

void SoundPlayMute(
	snd_recorder_struct *recorder,
	snd_play_struct *snd_play,
	int mute
);
void SoundChangePlayVolume(
	snd_recorder_struct *recorder,
	snd_play_struct *snd_play,
	float volume_left, float volume_right
);
void SoundChangePlaySampleRate(
	snd_recorder_struct *recorder,
	snd_play_struct *snd_play,
	int sample_rate
);

void SoundStopPlay(
	snd_recorder_struct *recorder,
	snd_play_struct *snd_play
);

int SoundMusicStartPlay(
	snd_recorder_struct *recorder,
	const char *object,     /* Full path to object. */
	int repeats             /* Number of repeats, -1 for infinate. */
);
int SoundMusicIsPlaying(snd_recorder_struct *recorder);
void SoundMusicStopPlay(snd_recorder_struct *recorder);

#ifdef Y_H
static int SoundGetYMixerCode(const char *channel_name);
#endif
void SoundMixerGet(
	snd_recorder_struct *recorder, const char *channel_name,
	float *left, float *right	/* 0.0 to 1.0 */
);
void SoundMixerSet(
	snd_recorder_struct *recorder, const char *channel_name,
	float left, float right		/* 0.0 to 1.0 */
);


#define MAX(a,b)        (((a) > (b)) ? (a) : (b))
#define MIN(a,b)        (((a) < (b)) ? (a) : (b))
#define CLIP(a,l,h)     (MIN(MAX((a),(l)),(h)))


/*
 *	Initializes sound server and returns a newly allocated
 *	recorder structure.
 */
snd_recorder_struct *SoundInit(
	void *core,
	int type,
	const char *connect_arg,
	const char *start_arg,
	void *window
)
{
	snd_recorder_struct *recorder;
	const sar_option_struct *opt;
        int my_type = type;

        #ifdef SDL_H
        int audio_rate = 22050;
        Uint16 audio_format = AUDIO_S16SYS;
        int audio_channels = 2;
        int audio_buffers = 4096;
        if (type == SOUND_DEFAULT)
            my_type = SNDSERV_TYPE_SDL;
        #endif
        #ifdef Y_H
        if (type == SOUND_DEFAULT)
           my_type = SNDSERV_TYPE_Y;
        #endif

	sar_core_struct *core_ptr = SAR_CORE(core);
	if(core_ptr == NULL)
	    return(NULL);

	opt = &core_ptr->option;

	recorder = (snd_recorder_struct *)calloc(
	    1, sizeof(snd_recorder_struct)
	);
	if(recorder == NULL)
	    return(NULL);

	recorder->type = my_type;
	recorder->con = NULL;
	recorder->sample_rate = 0;
	recorder->sample_size = 0;
	recorder->channels = 0;
	recorder->bytes_per_cycle = 0;
	recorder->background_music_sndobj = NULL;

	/* Initialize by sound server type. */
	switch(my_type)
	{
	  case SNDSERV_TYPE_Y:
#ifdef Y_H
	    if(connect_arg == NULL)
		connect_arg = (const char *)getenv("RECORDER");
	    if(connect_arg == NULL)
		connect_arg = DEF_Y_CONNECT_ARG;

	    /* Connect to sound server. */
#ifdef __MSW__
	    recorder->con = (void *)YOpenConnectionWin32(
		start_arg, connect_arg, opt->sound_priority, window
	    );
#else
	    recorder->con = (void *)YOpenConnection(
		start_arg, connect_arg
	    );
#endif
	    /* Failed to connect? */
	    if(recorder->con == NULL)
	    {
		free(recorder);
		return(NULL);
	    }

	    /* Record sound server type. */
	    recorder->type = type;

	    if(opt->runtime_debug)
		printf(
"SoundInit(): Initialized sound server at \"%s\".\n",
		    connect_arg
		);
#endif	/* Y_H */
	    if(recorder->con == NULL)
	    {
		fprintf(
		    stderr,
		    "SoundInit(): No sound support.\n"
		);
		free(recorder);
		return(NULL);
	    }
	    break;

         case SNDSERV_TYPE_SDL:
         #ifdef SDL_H
              if (SDL_Init(SDL_INIT_AUDIO) != 0)
              {
                  printf("Unable to Initialize SDL sound.\n");
                  return NULL;
              }

              if ( Mix_OpenAudio(audio_rate, audio_format, 
                                 audio_channels, audio_buffers) != 0) 
              {
	           printf("Unable to initialize audio: %s\n", Mix_GetError());
                   return NULL;
              }
          #endif
            break;

	  default:
	    fprintf(
		stderr,
		"SoundInit(): Unsupported sound server type `%i'.\n",
		my_type
	    );
	    free(recorder);
	    return(NULL);
	    break;
	}

	return(recorder);
}


/*
 *	Manages sound server events.
 *
 *	Returns the number of events recieved or -1 on error.
 *
 *	If -1 is returned then recorder has become invalid and should
 *	not be referenced again. This usually means the sound server
 *	has shut down, the connection to the sound server was broken, or 
 *	some other fatal error occured.
 */
int SoundManageEvents(snd_recorder_struct *recorder)
{
	int events_handled = 0;


	if(recorder == NULL)
	    return(-1);

	switch(recorder->type)
	{
	  case SNDSERV_TYPE_Y:
#ifdef Y_H

/* Macro to return the YID of a given pointer to a snd_play_struct. */
#define GET_SND_PLAY_YID(p)		\
 (((p) != NULL) ? (YID)((snd_play_struct *)(p)->data) : YIDNULL)

/* Macro to set the given snd_play_struct pointer's member data to
 * YIDNULL.
 */
#define SET_SND_PLAY_YID_NULL(p)	\
 { if((p) != NULL) (p)->data = (void *)YIDNULL; }

	    if(recorder->con != NULL)
	    {
		YConnection *con = (YConnection *)recorder->con;
		YEvent event;
		YEventSoundKill *kill;
		YEventMixer *mixer;

		/* Begin queued Y events. */
		while(YGetNextEvent(
		    con,
		    &event,
		    False		/* Do not block. */
		) > 0)
		{
		    events_handled++;

		    switch(event.type)
		    {
		      case YDisconnect: case YShutdown:
			/* Y server has disconnected or shutdown, the
			 * connection structure needs to be closed,
			 * deallocated, and set to NULL.
			 */
			YCloseConnection(con, False);
			recorder->con = con = NULL;

if(event.type == YShutdown)
 fprintf(
  stderr,
  "Lost connection to Y server: Y server has shut down.\n"
 );
else
 fprintf(
  stderr,
  "Lost connection to Y server: Y server has disconnected us.\n"
 );

			/* Deallocate recorder structure and return
			 * indicating that the recorder should not be 
			 * referenced again.
			 */
			free(recorder);
			return(-1);
			break;

		      /* Sound object has stopped playing. */
		      case YSoundObjectKill:
			kill = &event.kill;
			if(kill->yid != YIDNULL)
			{
			    YID yid = kill->yid;

			    /* Begin checking which sound object this is. */
			    /* Background music? */
			    if(yid == GET_SND_PLAY_YID(recorder->background_music_sndobj))
			    {
				SET_SND_PLAY_YID_NULL(recorder->background_music_sndobj);
				SoundStopPlay(
				    recorder,
				    recorder->background_music_sndobj
				);
				recorder->background_music_sndobj = NULL;
			    }
			}
			break; 

		      case YMixerChannel:
			mixer = &event.mixer;
			switch(mixer->code)
			{
			  case YMixerCodeVolume:
			    break;
			  case YMixerCodeSynth:
			    break;
			  case YMixerCodePCM:
			    break;
			}
			break;
		    }
		}	/* Handle queued Y events. */
	    }
#undef SET_SND_PLAY_YID_NULL
#undef GET_SND_PLAY_YID
#endif  /* Y_H */
	    break;

          case SNDSERV_TYPE_SDL:
            /* not sure if we need something here.... 
              will simply return with OK value */
            return 0;
            break;
	  default:
	    free(recorder);
	    return(-1);
	    break;
	}

	return(events_handled);
}

/*
 *      Shuts down the sound server, all resources on the given recorder
 *	structure will be deallocated and the recorder structure itself
 *	will be deallocated. The recorder should not be referenced again
 *	after this call.
 */
void SoundShutdown(
	snd_recorder_struct *recorder
)
{
	if(recorder == NULL)
	    return;

	/* Stop related resources first. */
	SoundMusicStopPlay(recorder);

	/* Shut down sound server by type. */
	switch(recorder->type)
	{
	  case SNDSERV_TYPE_Y:
#ifdef Y_H
	    if(recorder->con != NULL)
	    {
		YConnection *con = (YConnection *)recorder->con;

		/* Close connection to server. */
		YCloseConnection(con, False);
		recorder->con = con = NULL;
	    }
#endif  /* Y_H */
	    break;
           case SNDSERV_TYPE_SDL:
           #ifdef SDL_H
           Mix_CloseAudio();
           SDL_QuitSubSystem(SDL_INIT_AUDIO);
           #endif
           break;

	}

	/* Deallocate recorder structure. */
	free(recorder);
}



/*
 *	Changes audio mode to the one specified by mode_name.
 *
 *	Returns 0 on success or -1 on error.
 */
int SoundChangeMode(
	snd_recorder_struct *recorder, const char *mode_name
)
{
	if(recorder == NULL)
	    return(-1);

	switch(recorder->type)
	{
	  case SNDSERV_TYPE_Y:
#ifdef Y_H
	    if(recorder->con != NULL)
	    {
		YConnection *con = (YConnection *)recorder->con;
		YAudioModeValuesStruct **ptr, *yaudio_mode;
		int i, total_yaudio_modes;

		int sample_rate = 0;
		int sample_size = 0;
		int channels = 0;
		int fragment_size_bytes = 0;


		if(mode_name == NULL)
		    mode_name = DEF_Y_AUDIO_MODE_NAME;

		/* Get listing of available Audio mode. */
		ptr = YGetAudioModes(con, &total_yaudio_modes);

		/* Check if specified Audio mode is in list. */
		for(i = 0; i < total_yaudio_modes; i++)  
		{
		    yaudio_mode = ptr[i];
		    if(yaudio_mode == NULL)
			continue;

		    if(yaudio_mode->name == NULL)
			continue;

		    /* Audio mode names match? */
		    if(!strcasecmp(yaudio_mode->name, mode_name))
		    {
			sample_rate = yaudio_mode->sample_rate;
			sample_size = yaudio_mode->sample_size;
			channels = yaudio_mode->sample_rate;
			fragment_size_bytes = yaudio_mode->fragment_size_bytes;
			break;
		    }
		}
		/* Free Audio modes listing. */
		YFreeAudioModesList(ptr, total_yaudio_modes);

	        /* Audio mode name in list? */
	        if(i >= total_yaudio_modes)
		{
		    /* Could not match audio mode name from server's list
		     * of audio mode names. So we need to return -1 here.
		     */
		    return(-1);
		}

	        /* Change Audio mode. */
		if(YChangeAudioModePreset(con, mode_name))
		    return(-1);

		/* Record new Audio values on our recorder structure. */
		recorder->sample_rate = sample_rate;
		recorder->sample_size = sample_size;
		recorder->channels = channels;
		recorder->bytes_per_cycle = fragment_size_bytes;
	    }
#endif	/* Y_H */
	    break;

	}

	return(0);
}


/*
 *	Plays a sound object by given object path. Returns a dynamically
 *	allocated structure containing the play information or NULL on failure.
 */
snd_play_struct *SoundStartPlay(
	snd_recorder_struct *recorder,
	const char *object,	/* Full path to sound object. */
	float volume_left,     /* Volume, from 0.0 to 1.0. */
	float volume_right,
	int sample_rate,        /* Applied sample rate, can be 0. */
	snd_flags_t options     /* Any of SND_PLAY_OPTION_*. */
)
{
        int repeat = 0;
	snd_play_struct *snd_play = NULL;
        #ifdef SDL_H
        Mix_Chunk *sound = NULL;
        #endif

	if((recorder == NULL) || (object == NULL))
	    return(snd_play);

	switch(recorder->type)
	{
	  case SNDSERV_TYPE_Y:
#ifdef Y_H
	    if(recorder->con != NULL)
	    {
		YConnection *con = (YConnection *)recorder->con;
		YID yid;
		YEventSoundPlay value;

		/* Set up sound play values. */
		value.flags = YPlayValuesFlagPosition |
			      YPlayValuesFlagTotalRepeats |
			      YPlayValuesFlagVolume |
			      YPlayValuesFlagSampleRate;
		value.position = 0;
		value.total_repeats = ((options & SND_PLAY_OPTION_REPEATING) ?
		    -1 : 1
		);
		value.left_volume = CLIP(
		    (options & SND_PLAY_OPTION_MUTE) ? 0.0 : volume_left,
		    0.0, 1.0
		);
		value.right_volume = CLIP(
		    (options & SND_PLAY_OPTION_MUTE) ? 0.0 : volume_right,
		    0.0, 1.0
		);
		value.sample_rate = sample_rate;

		/* Start playing sound object. */
		yid = YStartPlaySoundObject(
		    con,
		    object,
		    &value
		);
		if(yid == YIDNULL)
		    return(NULL);

		/* Allocate sound play structure. */
		snd_play = (snd_play_struct *)calloc(
		    1, sizeof(snd_play_struct)
		);
		if(snd_play == NULL)
		    return(NULL);

		/* Record values. */
		snd_play->data = (void *)yid;
		snd_play->volume_left = (float)value.left_volume;
		snd_play->volume_right = (float)value.right_volume;
		snd_play->sample_rate = sample_rate;
		snd_play->options = options;
	    }
#endif	/* Y_H */
	    break;
            case SNDSERV_TYPE_SDL:
            #ifdef SDL_H
            sound = Mix_LoadWAV(object);
               if (! sound)
                  return NULL;
            snd_play = (snd_play_struct *) calloc(1, sizeof(snd_play_struct));
            if (! snd_play)
                return NULL;
            if (options & SND_PLAY_OPTION_MUTE)
            {
               volume_left = 0.0;
               volume_right = 0.0;
            }
            else
            {
               volume_left = 1.0;
               volume_right = 1.0;
            }
            if (options & SND_PLAY_OPTION_REPEATING)
                repeat = -1;
            else
                repeat = 0;
            snd_play->volume_left = volume_left;
            snd_play->volume_right = volume_right;
            snd_play->sample_rate = sample_rate;
            snd_play->options = options;
            snd_play->data = (void *)Mix_PlayChannel(-1, sound, repeat);
            snd_play->chunk = sound;
            #endif
            break;

	}

	return(snd_play);
}

/*
 *	Same as SoundStartPlay() except no reference is returned and there
 *	is no way to control the sound object being played after it has
 *	started playing (except for calling SoundShutdown()).
 */
void SoundStartPlayVoid(
	snd_recorder_struct *recorder,
	const char *object,	/* Full path to sound object. */
	float volume_left,	/* Volume, from 0.0 to 1.0. */
	float volume_right,
	int sample_rate,        /* Applied sample rate, can be 0. */
	snd_flags_t options     /* Any of SND_PLAY_OPTION_*. */
)
{
        #ifdef SDL_H
        Mix_Chunk *sound = NULL;
        #endif

	if((recorder == NULL) || (object == NULL))
	    return;

	switch(recorder->type)
	{
	  case SNDSERV_TYPE_Y:
#ifdef Y_H
	    if(recorder->con != NULL)
	    {
		YConnection *con = (YConnection *)recorder->con;
		YID yid;
		YEventSoundPlay value;

		/* Set up sound play values. */
		value.flags = YPlayValuesFlagPosition |
			      YPlayValuesFlagTotalRepeats |
			      YPlayValuesFlagVolume |
			      YPlayValuesFlagSampleRate;
		value.position = 0;
		value.total_repeats = ((options & SND_PLAY_OPTION_REPEATING) ?
		    -1 : 1
		);
		value.left_volume = CLIP(
		    (options & SND_PLAY_OPTION_MUTE) ? 0.0 : volume_left,
		    0.0, 1.0
		);
		value.right_volume = CLIP(
		    (options & SND_PLAY_OPTION_MUTE) ? 0.0 : volume_right,
		    0.0, 1.0
		);
		value.sample_rate = sample_rate;

		/* Start playing sound object. */
		yid = YStartPlaySoundObject(con, object, &value);
		if(yid == YIDNULL)
		    return;
	    }
#endif  /* Y_H */
	    break;
            case SNDSERV_TYPE_SDL:
            #ifdef SDL_H
            sound = Mix_LoadWAV(object);
            if (sound)
                Mix_PlayChannel(-1, sound, 0);
            #endif
            break;

	}
}

/*
 *	Mutes or unmutes a sound object already playing.
 */
void SoundPlayMute(
	snd_recorder_struct *recorder, snd_play_struct *snd_play,
	int mute
)
{
	if((recorder == NULL) || (snd_play == NULL))
	    return;

	if(mute)
	    snd_play->options |= SND_PLAY_OPTION_MUTE;
	else
	    snd_play->options &= ~SND_PLAY_OPTION_MUTE;

	SoundChangePlayVolume(
	    recorder, snd_play,
	    snd_play->volume_left, snd_play->volume_right
	);
}

/*
 *	Changes the volume of the sound object already playing.
 */
void SoundChangePlayVolume(
	snd_recorder_struct *recorder, snd_play_struct *snd_play,
	float volume_left,     /* Volume, from 0.0 to 1.0. */
	float volume_right
)
{
	if((recorder == NULL) || (snd_play == NULL))
	    return;

	/* Change in volume? */
	if((volume_left == snd_play->volume_left) &&
	   (volume_right == snd_play->volume_right)
	)
	    return;

	switch(recorder->type)   
	{
	  case SNDSERV_TYPE_Y:
#ifdef Y_H
	    if((recorder->con != NULL) &&
	       (snd_play->data != NULL)
	    )
	    {
		YConnection *con = (YConnection *)recorder->con;
		YID yid = (YID)snd_play->data;
		YEventSoundPlay value;


		/* Set up sound play values. */
		value.flags = YPlayValuesFlagVolume;
		value.left_volume = CLIP(
		    (snd_play->options & SND_PLAY_OPTION_MUTE) ? 0.0 : volume_left,
		    0.0, 1.0
		);
		value.right_volume = CLIP(
		    (snd_play->options & SND_PLAY_OPTION_MUTE) ? 0.0 : volume_right,
		    0.0, 1.0
		);

		/* Change play sound object values. */
		YSetPlaySoundObjectValues(con, yid, &value);

		/* Record values. */
		snd_play->volume_left = (float)value.left_volume;
		snd_play->volume_right = (float)value.right_volume;
	    }
#endif  /* Y_H */
	    break;
            case SNDSERV_TYPE_SDL:
            #ifdef SDL_H
                snd_play->volume_left = volume_left;
                snd_play->volume_right = volume_right;
                if (snd_play->data)
                {
                   int my_volume;
                   my_volume = (int) (volume_left * MIX_MAX_VOLUME);
                   Mix_Volume((int) snd_play->data, my_volume);
                }
            #endif
            break;

	}
}

/*
 *      Changes the sample rate of the sound object already playing.
 */
void SoundChangePlaySampleRate(
	snd_recorder_struct *recorder,
	snd_play_struct *snd_play,
	int sample_rate         /* Applied sample rate, can be 0. */
)
{
	if((recorder == NULL) || (snd_play == NULL))
	    return;

	/* Change in applied sample rate? */
	if(sample_rate == snd_play->sample_rate)
	    return;

	switch(recorder->type)
	{
	  case SNDSERV_TYPE_Y:
#ifdef Y_H
	    if((recorder->con != NULL) &&
	       (snd_play->data != NULL)
	    )
	    {
		YConnection *con = (YConnection *)recorder->con;
		YID yid = (YID)snd_play->data;
		YEventSoundPlay value;


		if(sample_rate < 0)
		    sample_rate = 0;

		/* Set up sound play values. */
		value.flags = YPlayValuesFlagSampleRate;
		value.sample_rate = sample_rate;

		/* Change play sound object values. */
		YSetPlaySoundObjectValues(con, yid, &value);

		/* Record values. */
		snd_play->sample_rate = sample_rate;
	    }
#endif  /* Y_H */
	    break;

	}
}

/*
 *	Stops a sound object already playing and deallocates the given
 *	snd_play structure. The snd_play structure should not be
 *	referenced again after this call. The snd_play structure will be
 *	deallocated even if recorder is NULL. However recorder should 
 *	always be given if available.
 */
void SoundStopPlay(
	snd_recorder_struct *recorder, snd_play_struct *snd_play
)
{
#define DO_FREE_PLAY_STRUCT	\
{ if(snd_play != NULL) { free(snd_play); snd_play = NULL; } }

	if(recorder == NULL)
	{
	    DO_FREE_PLAY_STRUCT
	    return;
	}

	if(snd_play == NULL)
	    return;

	switch(recorder->type)
	{       
	  case SNDSERV_TYPE_Y:
#ifdef Y_H
	    if((recorder->con != NULL) &&
	       ((YID)snd_play->data != YIDNULL)
	    )
	    {
		YConnection *con = (YConnection *)recorder->con;
		YID yid = (YID)snd_play->data;

		YDestroyPlaySoundObject(con, yid);
		snd_play->data = (void *)YIDNULL;
	    }
#endif	/* Y_H */
	    break;
            case SNDSERV_TYPE_SDL:
            #ifdef SDL_H
                 Mix_HaltChannel((int) snd_play->data);
                 Mix_FreeChunk(snd_play->chunk); 
            #endif
            break;
	}

	DO_FREE_PLAY_STRUCT
#undef DO_FREE_PLAY_STRUCT
}


/*
 *	Starts playing the given background music sound object specified
 *	by object. If an existing background music sound object is being
 *	played then it will be stopped first.
 *
 *	The given repeats can be -1 for infinate repeating (only stopped
 *	when another background music sound object begins to play or when
 *	the sound server shuts down).
 *
 *	Returns non-zero on error.
 */
int SoundMusicStartPlay(
	snd_recorder_struct *recorder,
	const char *object,     /* Full path to object. */
	int repeats             /* Number of repeats, -1 for infinate. */
)
{
	snd_play_struct *snd_play = NULL;

        #ifdef SDL_H
        Mix_Chunk *sound;
        #endif

	if((recorder == NULL) || (object == NULL))
	    return(-1);

	/* Stop currently playing background music if any. */
	SoundMusicStopPlay(recorder);

	/* Unable to destroy background music sound object? */
	if(recorder->background_music_sndobj != NULL)
	    return(-1);


	switch(recorder->type)
	{
	  case SNDSERV_TYPE_Y:
#ifdef Y_H
	    if(recorder->con != NULL)
	    {
		YConnection *con = (YConnection *)recorder->con;
		YID yid;
		YEventSoundPlay value;


		/* Set up sound play values. */
		value.flags = YPlayValuesFlagPosition |
			      YPlayValuesFlagTotalRepeats |
			      YPlayValuesFlagVolume |
			      YPlayValuesFlagSampleRate;
		value.position = 0;
		value.total_repeats = repeats;
		value.left_volume = 1.0;
		value.right_volume = 1.0;
		value.sample_rate = recorder->sample_rate;

		/* Start playing sound object. */
		yid = YStartPlaySoundObject(con, object, &value);
		if(yid == YIDNULL)
		    return(-1);

		/* Allocate background music sound play structure. */
		recorder->background_music_sndobj = snd_play = (snd_play_struct *)calloc(
		    1, sizeof(snd_play_struct)
		);
		if(snd_play == NULL)
		    return(-1);

		/* Record values. */
		snd_play->data = (void *)yid;
		snd_play->volume_left = (float)value.left_volume;
		snd_play->volume_right = (float)value.right_volume;
		snd_play->sample_rate = value.sample_rate;
		snd_play->options = 0;
	    }
#endif  /* Y_H */
	    break;
            case SNDSERV_TYPE_SDL:
            #ifdef SDL_H
                snd_play = (snd_play_struct *) calloc(1, sizeof(snd_play_struct));
                if (! snd_play)
                  return -1;
                sound = Mix_LoadWAV(object);
                if (! sound)
                {
                    free(snd_play);
                    return -1;
                }
                snd_play->data = (void *) Mix_PlayChannel(-1, sound, 0);
                recorder->background_music_sndobj = snd_play;
            #endif
            break;

	}

	return(0);
}

/*
 *	Returns true if there is a background music sound object playing
 *	on the specified recorder.
 */
int SoundMusicIsPlaying(snd_recorder_struct *recorder)
{
	return((recorder != NULL) ?
	    (recorder->background_music_sndobj != NULL) : 0
	);
}

/*
 *	Stops playing the background music sound object (if any).
 */
void SoundMusicStopPlay(snd_recorder_struct *recorder)
{
	if(recorder == NULL)
	    return;

	if(recorder->background_music_sndobj != NULL)
	{
	    SoundStopPlay(
		recorder,
		recorder->background_music_sndobj
	    );
	    recorder->background_music_sndobj = NULL;
	}
/* printf("SoundMusicStopPlay()\n"); */
}


#ifdef Y_H
/*
 *	Returns one of YMixerCode* based on channel_name or -1 on
 *	error.
 */
static int SoundGetYMixerCode(const char *channel_name)
{
	if(channel_name == NULL)
	    return(-1);

	if(!strcasecmp(channel_name, "volume"))
	    return(YMixerCodeVolume);
	else if(!strcasecmp(channel_name, "midi"))
	    return(YMixerCodeSynth);
	else if(!strcasecmp(channel_name, "wav"))
	    return(YMixerCodePCM);
	else
	    return(-1);
}
#endif

/*
 *	Obtains the mixer channel's values.
 */
void SoundMixerGet(
	snd_recorder_struct *recorder, const char *channel_name,
	float *left, float *right       /* 0.0 to 1.0 */
)
{
	if(left != NULL)
	    *left = 0.0f;
	if(right != NULL)
	    *right = 0.0f;

	if(recorder == NULL)
	    return;

	switch(recorder->type)
	{
	  case SNDSERV_TYPE_Y:
#ifdef Y_H
	    if(recorder->con != NULL)
	    {
		YConnection *con = (YConnection *)recorder->con;
		int mixer_channel_code = SoundGetYMixerCode(channel_name);
		Coefficient v[2];

		if(mixer_channel_code > -1)
		{
		    YGetMixerChannel(
			con, mixer_channel_code,
			&v[0], &v[1]
		    );
		    if(left != NULL)
			*left = (float)v[0];
		    if(right != NULL)
			*right = (float)v[1];
		}
	    }
#endif	/* Y_H */
	    break;
	}
}

/*
 *	Sets the mixer channel value.
 */
void SoundMixerSet(
	snd_recorder_struct *recorder, const char *channel_name,
	float left, float right         /* 0.0 to 1.0 */
)
{
	if(recorder == NULL)
	    return;

	switch(recorder->type)
	{
	  case SNDSERV_TYPE_Y:
#ifdef Y_H
	    if(recorder->con != NULL)
	    {
		YConnection *con = (YConnection *)recorder->con;
		int mixer_channel_code = SoundGetYMixerCode(channel_name);
		Coefficient v[2];

		if(mixer_channel_code > -1)
		{
		    v[0] = left;
		    v[1] = right;
		    YSetMixerChannel(
			con, mixer_channel_code,
			v[0], v[1]
		    );
/*printf("Set %i: %.2f %.2f\n", mixer_channel_code, v[0], v[1]);*/
		}
	    }
#endif  /* Y_H */
	    break;
	}
}
