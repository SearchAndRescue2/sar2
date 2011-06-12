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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>

#include "../include/string.h"
#include "../include/disk.h"

#include "sound.h"
#include "sar.h"



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
    float sample_rate,
    snd_flags_t options
    );
void SoundStartPlayVoid(
    snd_recorder_struct *recorder,
    const char *object,
    float volume_left, float volume_right,
    float sample_rate,
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
    float sample_rate
    );

void SoundStopPlay(
    snd_recorder_struct *recorder,
    snd_play_struct *snd_play
    );

void MusicStopPlay(
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

    sar_core_struct *core_ptr = SAR_CORE(core);
    if(core_ptr == NULL)
        return(NULL);

    opt = &core_ptr->option;

    recorder = (snd_recorder_struct *)calloc(
        1, sizeof(snd_recorder_struct)
	);
    if(recorder == NULL)
        return(NULL);

    recorder->type = type;
    recorder->con = NULL;
    recorder->sample_rate = 1;
    recorder->sample_size = 0;
    recorder->channels = 2;
    recorder->bytes_per_cycle = 0;
    recorder->background_music_sndobj = NULL;

    /* Initialize by sound server type. */
    switch(type)
    {
#ifdef SDL_MIXER
        case SNDSERV_TYPE_SDL:
            if (SDL_Init(SDL_INIT_AUDIO) != 0)
            {
                printf("Unable to Initialize SDL sound.\n");
                return NULL;
            }
                
            if ( Mix_OpenAudio(22050, AUDIO_S16SYS,2,4096) != 0) 
            {
                printf("Unable to initialize audio: %s\n", Mix_GetError());
                return NULL;
            }
            Mix_ReserveChannels(1);
#endif
            break;
        case SNDSERV_TYPE_OPENAL:
            if (alutInit(NULL,0) == AL_FALSE){
                printf("Alut error: %s", alutGetErrorString(alutGetError()));
            };
            
            /*Reserve memory space for this. It will
             be only freed on server shutdown*/
            snd_play_struct *snd_play = (snd_play_struct *) calloc(1, sizeof(snd_play_struct));
            snd_play->alSource=NULL;
            snd_play->alBuffer=NULL;
            
            recorder->untracked_sound_obj = snd_play;
            break;

        default:
	    fprintf(
		stderr,
		"SoundInit(): Unsupported sound server type `%i'.\n",
		type
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
#ifdef SDL_MIXER
        case SNDSERV_TYPE_SDL:
            /* not sure if we need something here.... 
               will simply return with OK value */
            return 0;
            break;
#endif
        case SNDSERV_TYPE_OPENAL:
            /* same here... */
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

    ALuint source;
    ALuint buffer;
    
    if(recorder == NULL)
        return;

    /* Stop related resources first. */
    SoundMusicStopPlay(recorder);

    /* Shut down sound server by type. */
    switch(recorder->type)
    {
#ifdef SDL_MIXER
        case SNDSERV_TYPE_SDL:
            Mix_CloseAudio();
            SDL_QuitSubSystem(SDL_INIT_AUDIO);
            break;
#endif
        case SNDSERV_TYPE_OPENAL:
            
            /* If tracking a sound, clean it up */
            source = recorder->untracked_sound_obj->alSource;
            buffer = recorder->untracked_sound_obj->alBuffer;
            if (source) {
                alSourceStop(source);
                alDeleteSources(1,&source);
                alGetError();
            }
            if (buffer){
                alDeleteBuffers(1,&buffer);
                alGetError();
            }
            free(recorder->untracked_sound_obj);

            /* Shutdown OpenAL context */
            alGetError();
            if (!alutExit())
                printf("Error: %s\n", alutGetErrorString(alutGetError()));

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
    float sample_rate,        /* Applied sample rate, can be 0. */
    snd_flags_t options     /* Any of SND_PLAY_OPTION_*. */
    )
{
    snd_play_struct *snd_play = NULL;

#ifdef SDL_MIXER
    /*SDL Mixer*/
    int repeat = 0;
    Mix_Chunk *sound = NULL;
#endif

    ALuint buffer;
    ALuint source;

    if((recorder == NULL) || (object == NULL))
        return(snd_play);

    switch(recorder->type)
    {
#ifdef SDL_MIXER
        case SNDSERV_TYPE_SDL:
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
            snd_play->data = Mix_PlayChannel(-1, sound, repeat);
            snd_play->chunk = sound;

            break;
#endif
        case SNDSERV_TYPE_OPENAL:

            //alGenBuffers(1,&buffer);
            
            buffer = alutCreateBufferFromFile(object);
            if (buffer == AL_NONE)
                return NULL;
            
            alGenSources(1,&source);
            if (alGetError() != AL_NO_ERROR)
                return NULL;
            
            alSourcei(source,AL_BUFFER, buffer);
            alSourcef(source,AL_PITCH, 1.0f);
            alSourcef(source, AL_GAIN, (volume_left + volume_right)/2);
            if (options & SND_PLAY_OPTION_REPEATING)
                alSourcei(source,AL_LOOPING,AL_TRUE);
            else 
                alSourcei(source,AL_LOOPING,AL_FALSE);

            snd_play = (snd_play_struct *) calloc(1, sizeof(snd_play_struct));
            if (! snd_play)
                return NULL;
            snd_play->volume_left = volume_left;
            snd_play->volume_right = volume_right;
            snd_play->sample_rate = sample_rate;
            snd_play->options = options;
            snd_play->alSource = source;
            snd_play->alBuffer = buffer;

            alSourcePlay(source);
            alGetError();
            
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
    float sample_rate,        /* Applied sample rate, can be 0. */
    snd_flags_t options     /* Any of SND_PLAY_OPTION_*. */
    )
{
    /*SDL Mixer*/
#ifdef SDL_MIXER
    Mix_Chunk *sound = NULL;
#endif

    ALuint source;
    ALuint buffer;        

    if((recorder == NULL) || (object == NULL))
        return;

    switch(recorder->type)
    {
#ifdef SDL_MIXER
        case SNDSERV_TYPE_SDL:
            sound = Mix_LoadWAV(object);
            if (sound)
                Mix_PlayChannel(-1, sound, 0);
            break;
#endif
        case SNDSERV_TYPE_OPENAL:

            /* Clean up previous object if any */
            source = recorder->untracked_sound_obj->alSource;
            buffer = recorder->untracked_sound_obj->alBuffer;
            if (source) {
                alSourceStop(source);
                alDeleteSources(1,&source);
                alGetError();
            }
            if (buffer){
                alDeleteBuffers(1,&buffer);
                alGetError();
            }

            recorder->untracked_sound_obj->alSource=NULL;
            recorder->untracked_sound_obj->alBuffer=NULL;
            /* Done cleaning up previous untracked objs */

            buffer = alutCreateBufferFromFile(object);
            if (buffer == AL_NONE)
                return;

            alGenSources(1,&source);
            if (alGetError() != AL_NO_ERROR){
                alDeleteBuffers(1,&buffer);
                alGetError();
                return;
            }

            /* Keep track of this sound in the recorder */
            recorder->untracked_sound_obj->alBuffer = buffer;
            recorder->untracked_sound_obj->alSource = source;
           
            alSourcei(source,AL_BUFFER, buffer);
            alSourcef(source, AL_GAIN, (volume_left + volume_right)/2);
            alSourcei(source,AL_LOOPING,AL_FALSE);
            
            alSourcef(source, AL_GAIN, (volume_left + volume_right)/2);
            if (options & SND_PLAY_OPTION_REPEATING)
                alSourcei(source,AL_LOOPING,AL_TRUE);
            else 
                alSourcei(source,AL_LOOPING,AL_FALSE);

            alSourcePlay(source);
            alGetError();
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
#ifdef SDL_MIXER
        case SNDSERV_TYPE_SDL:
            snd_play->volume_left = volume_left;
            snd_play->volume_right = volume_right;
            if (snd_play->data)
            {
                if (snd_play->options & SND_PLAY_OPTION_MUTE)
                    Mix_Volume(snd_play->data,0);
                else {
                    int my_volume = (int) (((volume_left + volume_right)/2) * MIX_MAX_VOLUME);
                    Mix_Volume((int) snd_play->data, my_volume);
                }
            }
            break;
#endif
        case SNDSERV_TYPE_OPENAL:
            snd_play->volume_left = volume_left;
            snd_play->volume_right = volume_right;
            if (snd_play->alSource)
            {
                if (snd_play->options & SND_PLAY_OPTION_MUTE)
                    alSourcef(snd_play->alSource,AL_GAIN,0.0f);
                else
                    alSourcef(snd_play->alSource,AL_GAIN,(volume_left + volume_right)/2);
                alGetError();
            }
            break;

    }
}

/*
 *      Changes the sample rate of the sound object already playing.
 */
void SoundChangePlaySampleRate(
    snd_recorder_struct *recorder,
    snd_play_struct *snd_play,
    float sample_rate         /* Applied sample rate, can be 0. */
    )
{
    if((recorder == NULL) || (snd_play == NULL))
        return;

    /* Change in applied sample rate? */
    if(sample_rate == snd_play->sample_rate)
        return;

    switch(recorder->type)
    {
#ifdef SDL_MIXER
        //change depending on sndserv type
        case SNDSERV_TYPE_SDL:
            // :( we cannot do this with SDL
            break;
#endif
        case SNDSERV_TYPE_OPENAL:
            snd_play->sample_rate = sample_rate;
            alSourcef(snd_play->alSource,AL_PITCH,sample_rate);
            alGetError();
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
#define DO_FREE_PLAY_STRUCT                                             \
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
#ifdef SDL_MIXER
        case SNDSERV_TYPE_SDL:
            Mix_HaltChannel((int) snd_play->data);
            Mix_FreeChunk(snd_play->chunk); 
            break;
#endif
        case SNDSERV_TYPE_OPENAL:
            alSourceStop(snd_play->alSource);
            alDeleteSources(1,&snd_play->alSource);
            alDeleteBuffers(1,&snd_play->alBuffer);
            
            alGetError();
            break;
    }

    DO_FREE_PLAY_STRUCT
#undef DO_FREE_PLAY_STRUCT
        }

void MusicStopPlay(
    snd_recorder_struct *recorder, snd_play_struct *snd_play
    )
{
#define DO_FREE_PLAY_STRUCT                                             \
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
#ifdef SDL_MIXER
        case SNDSERV_TYPE_SDL:
            Mix_HaltMusic();
            Mix_FreeMusic(snd_play->music); 
            break;
#endif
        case SNDSERV_TYPE_OPENAL:
            alSourceStop(snd_play->alSource);
            alDeleteSources(1,&snd_play->alSource);
            alDeleteBuffers(1,&snd_play->alBuffer);
            alGetError();
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

#ifdef SDL_MIXER
    /*SDL Mixer*/
    Mix_Music *sound;
#endif

    //OpenAL ogg management
    vorbis_info *pInfo;
    OggVorbis_File oggFile;
    ALenum format;
    ALsizei freq;
    int bitStream;
    char* buffer;
    long buffer_ptr;
    long current_size;
    ALuint bufferID;
    ALuint sourceID;
    long bytes;

    if((recorder == NULL) || (object == NULL))
        return(-1);

    /* Stop currently playing background music if any. */
    SoundMusicStopPlay(recorder);

    /* Unable to destroy background music sound object? */
    if(recorder->background_music_sndobj != NULL)
        return(-1);


    switch(recorder->type)
    {
#ifdef SDL_MIXER
        case SNDSERV_TYPE_SDL:
            snd_play = (snd_play_struct *) calloc(1, sizeof(snd_play_struct));
            if (! snd_play)
                return -1;
            sound = Mix_LoadMUS(object);
            if (! sound)
            {
                free(snd_play);
                return -1;
            }
            snd_play->data = Mix_PlayMusic(sound, repeats);
            snd_play->music = sound;
            recorder->background_music_sndobj = snd_play;
            break;
#endif
        case SNDSERV_TYPE_OPENAL:
            
            if (ov_fopen((char*)object,&oggFile) != 0)
            {
                printf("Error opening music file %s for decoding\n",object);
                return -1;
            }

            pInfo = ov_info(&oggFile, -1);

            if (pInfo->channels == 1)
                format = AL_FORMAT_MONO16;
            else
                format = AL_FORMAT_STEREO16;
            freq = pInfo->rate;

            buffer = malloc(BUFFER_SIZE);
            if (! buffer)
            {
                printf("Error allocating buffer\n");
                return -1;
            }

            buffer_ptr=0;
            current_size=BUFFER_SIZE;

            do
            {
                bytes = ov_read(&oggFile, buffer+buffer_ptr, current_size - buffer_ptr, 0, 2, 1, &bitStream);
                    
                if (bytes < 0)
                {
                    ov_clear(&oggFile);
                    printf("Error decoding %s\n",object);
                    return -1;
                }

                if (bytes > 0){
                    buffer_ptr += bytes;
                    if (buffer_ptr == current_size){
                        current_size += BUFFER_SIZE;
                        buffer = realloc(buffer,current_size);
                    }
                }
            }
            while (bytes > 0);

            //our buffer has the info now
            ov_clear(&oggFile);
            alGenBuffers(1,&bufferID);
            alGenSources(1,&sourceID);
            
            alBufferData(bufferID,format,buffer,buffer_ptr,freq);
            alSourcei(sourceID, AL_BUFFER, bufferID);
            
            snd_play = (snd_play_struct *) calloc(1, sizeof(snd_play_struct));
            if (! snd_play)
                return -1;
            snd_play->volume_left = 1.0f;
            snd_play->volume_right = 1.0f;
            snd_play->sample_rate = 1.0f;
            snd_play->alSource = sourceID;
            snd_play->alBuffer = bufferID;

            alSourcePlay(sourceID);
            alGetError();

            

            recorder->background_music_sndobj = snd_play;
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
        MusicStopPlay(
            recorder,
            recorder->background_music_sndobj
	    );
        recorder->background_music_sndobj = NULL;
    }
/* printf("SoundMusicStopPlay()\n"); */
}


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

    }
}
