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
#include <math.h>

#include "matrixmath.h"
#include "sound.h"
#include "obj.h"
#include "objsound.h"
#include "sar.h"
#include "config.h"


#ifdef __MSW__
static double rint(double x);
#endif	/* __MSW__ */


sar_sound_source_struct *SARSoundSourceNew(
    const char *name,
    const char *filename,
    const char *filename_far,
    float range,                    /* In meters */
    float range_far,                /* In meters */
    const sar_position_struct *pos,
    float cutoff,                   /* In radians */
    const sar_direction_struct *dir,
    float sample_rate_limit           /* 1.0 is normal speed */
    );
void SARSoundSourceDelete(sar_sound_source_struct *sndsrc);
int SARSoundSourceMatchFromList(
    sar_sound_source_struct **list, int total,
    const char *name
    );
void SARSoundSourcePlayFromList(
    snd_recorder_struct *recorder,
    sar_sound_source_struct **list, int total,
    const char *name,
    const sar_position_struct *pos,		/* Object position */
    const sar_direction_struct *dir,	/* Object direction */
    const sar_position_struct *ear_pos
    );
snd_play_struct *SARSoundSourcePlayFromListRepeating(
    snd_recorder_struct *recorder,
    sar_sound_source_struct **list, int total,
    const char *name, int *sndsrc_num
    );

void SARSoundEngineMute(
    snd_recorder_struct *recorder,
    snd_play_struct *inside_snd_play,
    snd_play_struct *outside_snd_play 
    );
void SARSoundEngineUpdate(
    snd_recorder_struct *recorder,
    sar_sound_source_struct **list, int total,
    snd_play_struct *inside_snd_play,
    int inside_sndsrc_num,
    snd_play_struct *outside_snd_play,
    int outside_sndsrc_num,
    char ear_in_cockpit,
    sar_engine_state engine_state,
    float throttle,
    float distance_to_camera
    );
int SARSoundStallUpdate(
    snd_recorder_struct *recorder,
    sar_sound_source_struct **list, int total,
    void **stall_snd_play,
    int stall_sndsrc_num,
    char ear_in_cockpit,
    sar_flight_model_type flight_model_type,
    char is_landed,
    float speed, float speed_stall
    );
void SARSoundOverSpeedUpdate(
    snd_recorder_struct *recorder,
    sar_sound_source_struct **list, int total,
    void **overspeed_snd_play,
    int overspeed_sndsrc_num,
    char ear_in_cockpit, char is_overspeed
    );


#define POW(x,y)        (((x) > 0.0f) ? pow(x,y) : 0.0f)

#define MAX(a,b)	(((a) > (b)) ? (a) : (b))
#define MIN(a,b)	(((a) < (b)) ? (a) : (b))
#define CLIP(a,l,h)	(MIN(MAX((a),(l)),(h)))
#define STRDUP(s)	(((s) != NULL) ? strdup(s) : NULL)


#ifdef __MSW__
static double rint(double x)
{
    if((double)((double)x - (int)x) > (double)0.5)
        return((double)((int)x + (int)1));
    else
        return((double)((int)x));
}
#endif	/* __MSW__ */



/*
 *      Creates a new sound source structure.
 */
sar_sound_source_struct *SARSoundSourceNew(
    const char *name,
    const char *filename,
    const char *filename_far,
    float range,			/* In meters */
    float range_far,		/* In meters */
    const sar_position_struct *pos,
    float cutoff,			/* In radians */
    const sar_direction_struct *dir,
    float sample_rate_limit	      
    )
{
    sar_sound_source_struct *sndsrc = SAR_SOUND_SOURCE(
        calloc(1, sizeof(sar_sound_source_struct))
	);
    if(sndsrc == NULL)
        return(NULL);

    sndsrc->name = STRDUP(name);

    sndsrc->filename = STRDUP(filename);
    sndsrc->filename_far = STRDUP(filename_far);

    sndsrc->range = range;
    sndsrc->range_far = range_far;

    if(pos != NULL)
        memcpy(&sndsrc->pos, pos, sizeof(sar_position_struct));

    sndsrc->cutoff = cutoff;

    if(dir != NULL)
        memcpy(&sndsrc->dir, dir, sizeof(sar_direction_struct));

    sndsrc->sample_rate_limit = sample_rate_limit;

    return(sndsrc);
}

/*
 *      Deletes the given sound source structure.
 */
void SARSoundSourceDelete(sar_sound_source_struct *sndsrc)
{
    if(sndsrc == NULL)
        return;

    free(sndsrc->name);
    free(sndsrc->filename);
    free(sndsrc->filename_far);
    free(sndsrc);
}

/*
 *      Returns the index of the sound source found in the given list
 *      that matches the given name.
 *
 *      Can return -1 on failed match.
 */
int SARSoundSourceMatchFromList(
    sar_sound_source_struct **list, int total,
    const char *name
    )
{
    int i;
    sar_sound_source_struct *sndsrc;


    if((list == NULL) || (total <= 0) || (name == NULL))
        return(-1);

    for(i = 0; i < total; i++)
    {
        sndsrc = list[i];
        if(sndsrc == NULL)
            continue;

        if(sndsrc->name == NULL)
            continue;

        if(!strcmp(sndsrc->name, name))
            return(i);
    }

    return(-1);
}

/*
 *      Searches the given list of sound sources for a sound source
 *      who's name matches the given name.
 *
 *      If it is matched then it will be played once.
 */
void SARSoundSourcePlayFromList(
    snd_recorder_struct *recorder,
    sar_sound_source_struct **list, int total,
    const char *name,
    const sar_position_struct *pos,		/* Object position */
    const sar_direction_struct *dir,	/* Object direction */
    const sar_position_struct *ear_pos
    )
{
    int i;
    double a[3 * 1], b[3 * 1];
    float r;
    sar_sound_source_struct *sndsrc;
    sar_position_struct snd_pos;


    if(recorder == NULL)
        return;

    i = SARSoundSourceMatchFromList(list, total, name);
    sndsrc = (i > -1) ? list[i] : NULL;
    if(sndsrc == NULL)
        return;

    if(sndsrc->filename == NULL)
        return;


    /* Found sound source to play */

    /* Calculate world coordinates of sound as snd_pos */
    if((pos != NULL) && (dir != NULL))
    {
        a[0] = sndsrc->pos.x;
        a[1] = sndsrc->pos.y;
        a[2] = sndsrc->pos.z;

        /* Rotate matrix a into b */
        MatrixRotateBank3(a, -dir->bank, b);        /* Our bank is negative,
                                                     * so pass as flipped
                                                     */
        MatrixRotatePitch3(b, dir->pitch, a);
        MatrixRotateHeading3(a, dir->heading, b);

        snd_pos.x = (float)(pos->x + b[0]);
        snd_pos.y = (float)(pos->y + b[1]);
        snd_pos.z = (float)(pos->z + b[2]);
    }
    else if(pos != NULL)
    {
        snd_pos.x = pos->x;
        snd_pos.y = pos->y;
        snd_pos.z = pos->z;
    }
    else
    {
        memset(&snd_pos, 0x00, sizeof(sar_position_struct));
    }


    /* Calculate 3d distance r of source to ear */
    if(ear_pos != NULL)
        r = (float)SFMHypot3(
            snd_pos.x - ear_pos->x,
            snd_pos.y - ear_pos->y,
            snd_pos.z - ear_pos->z
	    );
    else
        r = 0.0f;


    /* Calculate volume due to distance r relative to the
     * maximum range of the sound source.
     */
    if(sndsrc->range > 0.0)
    {
        if(r < sndsrc->range)
        {
            float vol = (float)MAX(
                1.0 - (r / sndsrc->range), 0.0
		);
            SoundStartPlayVoid(
                recorder,
                sndsrc->filename,
                vol, vol,
                1.0f, 0
		);
        }
    }
    else
    {
        float vol = 1.0f;
        SoundStartPlayVoid(
            recorder,
            sndsrc->filename,
            vol, vol,
            1.0f, 0
	    );
    }
}

/*
 *      Searches the given list of sound sources for a sound source
 *      who's name matches the given name.
 *
 *      If it is matched then it will be played repeatedly.
 *
 *      Returns the pointer to the new sound object play structure
 *      or NULL on failure.
 */
snd_play_struct *SARSoundSourcePlayFromListRepeating(
    snd_recorder_struct *recorder,
    sar_sound_source_struct **list, int total,
    const char *name, int *sndsrc_num
    )
{
    int i;
    sar_sound_source_struct *sndsrc;

    if(sndsrc_num != NULL)
        *sndsrc_num = -1;

    if(recorder == NULL)
        return(NULL);

    i = SARSoundSourceMatchFromList(list, total, name);
    sndsrc = (i > -1) ? list[i] : NULL;
    if(sndsrc == NULL)
        return(NULL);

    if(sndsrc->filename == NULL)
        return(NULL);

    if(sndsrc_num != NULL)
        *sndsrc_num = i;

    /* Start playing the sound object repeatedly, the volume
     * should be initially 0.0 (muted). The calling function
     * is responsible for increasing the volume of the returned
     * sound play object.
     */
    return(SoundStartPlay(
               recorder,
               sndsrc->filename,   /* Full path to object */
               0.0, 0.0,           /* Volume, from 0.0 to 1.0 */
               1.0f,                  /* Applied sample rate, can be 0 */
               SND_PLAY_OPTION_REPEATING   /* Options */
               ));
}


/*
 *	Calls SARSoundEngineUpdate() with throttle set to 0.0
 *	and the engine_state set to SAR_ENGINE_OFF.
 *
 *	This effectively mutes the engine sound.
 */
void SARSoundEngineMute(
    snd_recorder_struct *recorder,
    snd_play_struct *inside_snd_play,
    snd_play_struct *outside_snd_play 
    )
{
    SARSoundEngineUpdate(
        recorder,
        NULL, 0,			/* No sound sources needed */
        inside_snd_play,
        -1,
        outside_snd_play,
        -1,
        0,				/* In cockpit? */
        SAR_ENGINE_OFF,	/* Engine state */
        0.0f,			/* Throttle */
        0.0f			/* Distance to camera */
	);
}

/* 
 *      Adjusts the engine volume and sample rate.
 */
void SARSoundEngineUpdate(
    snd_recorder_struct *recorder,
    sar_sound_source_struct **list, int total,
    snd_play_struct *inside_snd_play,
    int inside_sndsrc_num,
    snd_play_struct *outside_snd_play,
    int outside_sndsrc_num,
    char ear_in_cockpit,
    sar_engine_state engine_state,	/* One of SAR_ENGINE_* */
    float throttle,			/* Throttle coeff 0.0 to 1.0 */
    float distance_to_camera	/* In meters */
    )
{
    float sample_rate_limit, new_sample_rate;
    float volume;				/* 0.0 to 1.0 */
    float audiable_radius = 2000.0f;
    sar_sound_source_struct *inside_sndsrc = NULL,
        *outside_sndsrc = NULL;

    /* Not connected to sound server? */
    if(recorder == NULL)
        return;

    /* If the engine is off then mute all sound volumes, this
     * improves efficiency since no sound would be mixed by
     * the sound server if volume is 0.0 but the sound object will
     * still remain valid.
     */
    /* if(engine_state != SAR_ENGINE_ON) */
    /* { */
    /*     SoundChangePlayVolume( */
    /* 	recorder, inside_snd_play, */
    /* 	0.0f, 0.0f */
    /*     ); */
    /*     SoundChangePlayVolume( */
    /* 	recorder, outside_snd_play, */
    /* 	0.0f, 0.0f */
    /*     ); */
    /*     return; */
    /* } */


    /* Get pointers to sound source structures (if possible) */
    if(list != NULL)
    {
        if((inside_sndsrc_num >= 0) &&
           (inside_sndsrc_num < total)
	    )
            inside_sndsrc = list[inside_sndsrc_num];

        if((outside_sndsrc_num >= 0) &&
           (outside_sndsrc_num < total)
	    )
            outside_sndsrc = list[outside_sndsrc_num];
    }


    /* Begin calculating sample rate based on the throttle position,
     * increase to a faster sample rate as throttle increases.
     */

    /* Get lower and upper sample rate bounds and ranges */
    sample_rate_limit = recorder->sample_rate;
    if(ear_in_cockpit && inside_sndsrc != NULL)
    {
        sample_rate_limit = inside_sndsrc->sample_rate_limit;
        audiable_radius = inside_sndsrc->range;
    }
    else if(outside_sndsrc != NULL)
    {
        sample_rate_limit = outside_sndsrc->sample_rate_limit;
        audiable_radius = outside_sndsrc->range;
    }

    /* Calculate new sample rate based on throttle position */
    /* new_sample_rate = (int)rint( */
    /*     ((MAX(sample_rate_limit - sample_rate, 0) * throttle) + */
    /*     sample_rate) / 1000 */
    /* ); */
    /* new_sample_rate = (int)(new_sample_rate * 1000); */

    //If recorder -> sample_rate is the maximum, that is 1, the new sample rate
    //will be the sample_rate * throotle
    new_sample_rate = sample_rate_limit * throttle;

    /* Calculate volume based on (1 - (x^0.5)) curve */
    volume = (float)CLIP(1.0 -
                         POW(distance_to_camera / audiable_radius, 0.5),
                         0.0, 1.0
	);

    if (new_sample_rate == 0) {
        SoundChangePlayVolume(
            recorder, inside_snd_play,
            0.0f, 0.0f
            );
        SoundChangePlayVolume(
            recorder, outside_snd_play,
            0.0f, 0.0f
            );
    }

    /* Is ear inside cockpit? */
    else if (ear_in_cockpit)
    {
        /* Inside cockpit */
        SoundChangePlayVolume(
            recorder, inside_snd_play,
            volume, volume
	    );
        SoundChangePlaySampleRate(
            recorder, inside_snd_play,
            new_sample_rate
	    );
        SoundChangePlayVolume(
            recorder, outside_snd_play,
            0.0f, 0.0f
	    );
        SoundChangePlaySampleRate(
            recorder, outside_snd_play,
            new_sample_rate
	    );
    }
    else
    {
        /* Outside */
        SoundChangePlayVolume(
            recorder, inside_snd_play,
            0.0f, 0.0f
	    );
        SoundChangePlaySampleRate(
            recorder, inside_snd_play,
            new_sample_rate
	    );
        SoundChangePlayVolume(
            recorder, outside_snd_play,
            volume, volume
	    );
        SoundChangePlaySampleRate(
            recorder, outside_snd_play,
            new_sample_rate
	    );
    }
}

/*
 *	Plays or stops the repeating stall warning sound
 *
 *	The speed_stall should have modifications (ie flaps) already
 *	applied to it.
 *
 *	Returns 1 if sound was turned on or 0 if it was not.
 */
int SARSoundStallUpdate(
    snd_recorder_struct *recorder,
    sar_sound_source_struct **list, int total,
    void **stall_snd_play,
    int stall_sndsrc_num,
    char ear_in_cockpit,
    sar_flight_model_type flight_model_type,
    char is_landed,
    float speed, float speed_stall
    )
{
    Boolean stalling = False;
    float speed_threshold;

    if((recorder == NULL) || (stall_snd_play == NULL))
        return(0);

    /* Calculate the speed at which the sound should be turned on
     * if the speed is lower than this value
     */
    // Now that we have 2 stall thresholds we can use the real stall
    // as limit.
    speed_threshold = speed_stall; // * 0.78f;

    /* Stall warning sound should be turned on? */
    if(!is_landed && (flight_model_type == SAR_FLIGHT_MODEL_AIRPLANE))
    {
        if((speed < speed_threshold) && (speed_threshold > 0.0f))
            stalling = True;
    }

    /* Check if the sound needs to be played or stopped? */
    if(stalling && (*stall_snd_play == NULL))
    {
        /* Need to start playing stall sound */
        int i = stall_sndsrc_num;
        sar_sound_source_struct *sndsrc;

        if((i >= 0) && (i < total))
            sndsrc = list[i];
        else
            sndsrc = NULL;

        if(sndsrc != NULL)
            *stall_snd_play = (void *)SoundStartPlay(
                recorder,
                sndsrc->filename,
                1.0f, 1.0f,
                1.0f,
                SND_PLAY_OPTION_REPEATING
		);
    }
    else if(!stalling && (*stall_snd_play != NULL))
    {
        /* Need to stop playing stall sound */
        SoundStopPlay(
            recorder,
            (snd_play_struct *)(*stall_snd_play)
	    );
        *stall_snd_play = NULL;
    }

    return(stalling ? 1 : 0);
}

/*
 *      Plays or stops the repeating overspeed warning sound.
 */
void SARSoundOverSpeedUpdate(
    snd_recorder_struct *recorder,
    sar_sound_source_struct **list, int total,
    void **overspeed_snd_play,
    int overspeed_sndsrc_num,
    char ear_in_cockpit, char is_overspeed
    )
{
    if((recorder == NULL) || (overspeed_snd_play == NULL))
        return;

    /* Check if the sound needs to be played or stopped? */
    if(is_overspeed && (*overspeed_snd_play == NULL))
    {
        /* Need to start playing overspeed sound */
        int i = overspeed_sndsrc_num;
        sar_sound_source_struct *sndsrc;

        if((i >= 0) && (i < total))
            sndsrc = list[i];
        else
            sndsrc = NULL;

        if(sndsrc != NULL)
            *overspeed_snd_play = (void *)SoundStartPlay(
                recorder,
                sndsrc->filename,
                1.0f, 1.0f,
                1.0f,
                SND_PLAY_OPTION_REPEATING
		);
    }
    else if(!is_overspeed && (*overspeed_snd_play != NULL))
    {
        /* Need to stop playing overspeed sound */
        SoundStopPlay(
            recorder,
            (snd_play_struct *)(*overspeed_snd_play)
	    );
        *overspeed_snd_play = NULL;
    }
}
