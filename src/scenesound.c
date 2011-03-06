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

#ifdef __MSW__
# include <windows.h>
#endif

#include "obj.h"
#include "objsound.h"
#include "objutils.h"
#include "sound.h"
#include "sar.h"
#include "scenesound.h"
#include "config.h"


void SARSceneSoundUpdate(
	sar_core_struct *core_ptr,
	Boolean engine_sounds,
	Boolean event_sounds,
	Boolean voice_sounds,
	Boolean music
);


#define ATOI(s)         (((s) != NULL) ? atoi(s) : 0)
#define ATOL(s)         (((s) != NULL) ? atol(s) : 0)
#define ATOF(s)         (((s) != NULL) ? atof(s) : 0.0f)
#define STRDUP(s)       (((s) != NULL) ? strdup(s) : NULL)

#define MAX(a,b)        (((a) > (b)) ? (a) : (b))
#define MIN(a,b)        (((a) < (b)) ? (a) : (b))
#define CLIP(a,l,h)     (MIN(MAX((a),(l)),(h)))
#define STRLEN(s)       (((s) != NULL) ? ((int)strlen(s)) : 0)


/*
 *	Updates all sound resources on the core's scene structure and
 *	its objects with respect to the given sound conditions. Purpose
 *	for this function is to start or stop sounds at startup or as the
 *	sound options change in the middle of a game.
 *
 *	The core structure's scene must be valid (not NULL).
 *	If the core structure's recorder is NULL and any given inputs
 *	suggest sound is to be enabled then the recorder will be
 *	initialized.
 *
 *	The core structure's recorder will never be shut down by this
 *	function.
 */
void SARSceneSoundUpdate(
	sar_core_struct *core_ptr,
	Boolean engine_sounds,
	Boolean event_sounds,
	Boolean voice_sounds,
	Boolean music 
)
{
	int obj_num;
	const char *name;
	int sndsrc_num;
	sar_object_struct *obj_ptr;
	sar_object_aircraft_struct *aircraft;
	void **snd_play_rtn;
	const char *sound_server_connect_arg = "127.0.0.1:9433";
	gw_display_struct *display = core_ptr->display;
	sar_scene_struct *scene = core_ptr->scene;
	snd_recorder_struct *recorder = core_ptr->recorder;

	/* Is recorder NULL? */
	if(recorder == NULL)
	{
	    /* Turn sound on? */
	    if(engine_sounds || event_sounds || voice_sounds ||
	       music
	    )
	    {
		/* Need to initialize recorder. */
		void *window;
                int type;

                type = SNDSERV_TYPE_SDL;

		GWContextGet(
		    display, GWContextCurrent(display),
		    &window, NULL,
		    NULL, NULL, NULL, NULL
		);

		GWSetInputBusy(core_ptr->display);
		recorder = SoundInit(
		    core_ptr, type,
		    sound_server_connect_arg,
		    NULL,          /* Do not start sound server. */
		    window
		);
		if(recorder == NULL)
		{
		    const char *con_arg = sound_server_connect_arg;
		    char *buf = (char *)malloc(
			(256 + STRLEN(con_arg)) * sizeof(char)
		    );
		    sprintf(
			buf,
"Unable to connect to sound server at:\n\n    %s",
			con_arg
		    );
		    GWOutputMessage(
			core_ptr->display,
			GWOutputMessageTypeError,
			"Sound initialization failed!",
			buf,
"Please check to make sure that your sound server\n\
is running and available at the specified address.\n\
Also make sure that you have sufficient permission\n\
to connect to it."
		    );
		    free(buf);
		}
		else
		{
		    core_ptr->recorder = recorder;

		    SoundChangeMode(
			core_ptr->recorder, core_ptr->audio_mode_name
		    );
		}
		GWSetInputReady(core_ptr->display);
	    }
	    else
	    {
		/* Recorder not initialized and sound is turned off,
		 * there is nothing further to do.
		 */
		return;
	    }
	}

	/* Scene structure must be valid. */
	if(scene == NULL)
	    return;


	/* Update sounds on the scene structure. */




	/* Update sounds on each object. */
	for(obj_num = 0; obj_num < core_ptr->total_objects; obj_num++)
	{
	    obj_ptr = core_ptr->object[obj_num];
	    if(obj_ptr == NULL)
		continue;

	    switch(obj_ptr->type)
	    {
	      case SAR_OBJ_TYPE_GARBAGE:
	      case SAR_OBJ_TYPE_STATIC:
	      case SAR_OBJ_TYPE_AUTOMOBILE:
	      case SAR_OBJ_TYPE_WATERCRAFT:
		break;
	      case SAR_OBJ_TYPE_AIRCRAFT:
		aircraft = SAR_OBJ_GET_AIRCRAFT(obj_ptr);
		if(aircraft != NULL)
		{
#define DO_LOAD_ENGINE_SOUND				\
{ if(*snd_play_rtn == NULL)				\
  *snd_play_rtn = SARSoundSourcePlayFromListRepeating(	\
   recorder, obj_ptr->sndsrc, obj_ptr->total_sndsrcs,	\
   name, &sndsrc_num					\
  );							\
}
#define DO_UNLOAD_ENGINE_SOUND				\
{ if(*snd_play_rtn != NULL)				\
 {							\
  SoundStopPlay(recorder, *snd_play_rtn);		\
  *snd_play_rtn = NULL;					\
 }							\
}
		    /* Load/unload engine sounds. */
		    if(engine_sounds)
		    {
			/* Turn engine sounds on as needed. */

			name = "engine_inside";
			snd_play_rtn = &aircraft->engine_inside_sndplay;
			DO_LOAD_ENGINE_SOUND
			aircraft->engine_inside_sndsrc = sndsrc_num;

			name = "engine_outside";
			snd_play_rtn = &aircraft->engine_outside_sndplay;
			DO_LOAD_ENGINE_SOUND
			aircraft->engine_outside_sndsrc = sndsrc_num;
		    }
		    else
		    {
			/* Turn engine sounds off as needed. */

			snd_play_rtn = &aircraft->engine_inside_sndplay;
			DO_UNLOAD_ENGINE_SOUND

			snd_play_rtn = &aircraft->engine_outside_sndplay;
			DO_UNLOAD_ENGINE_SOUND
		    }
#undef DO_UNLOAD_ENGINE_SOUND
#undef DO_LOAD_ENGINE_SOUND

#define DO_GET_SNDSRC_INDEX	{			\
 sndsrc_num = SARSoundSourceMatchFromList(		\
  obj_ptr->sndsrc, obj_ptr->total_sndsrcs,		\
  name							\
 );							\
}
		    /* Get sound source index of warning sounds. */
		    if(event_sounds)
		    {
			name = "stall";
			DO_GET_SNDSRC_INDEX
			aircraft->stall_sndsrc = sndsrc_num;

			name = "overspeed";
			DO_GET_SNDSRC_INDEX
			aircraft->overspeed_sndsrc = sndsrc_num;

		    }
#undef DO_GET_SNDSRC_INDEX
		}
		break;
	      case SAR_OBJ_TYPE_GROUND:
	      case SAR_OBJ_TYPE_RUNWAY:
	      case SAR_OBJ_TYPE_HELIPAD:
	      case SAR_OBJ_TYPE_HUMAN:
	      case SAR_OBJ_TYPE_SMOKE:
	      case SAR_OBJ_TYPE_FIRE:
	      case SAR_OBJ_TYPE_EXPLOSION:
	      case SAR_OBJ_TYPE_CHEMICAL_SPRAY:
	      case SAR_OBJ_TYPE_FUELTANK:
	      case SAR_OBJ_TYPE_PREMODELED:
		break;
	    }
	}


}
