#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <math.h>

#include "../include/string.h"
#include "../include/disk.h"

#include "sound.h"
#include "menu.h"
#include "obj.h"
#include "objutils.h"
#include "sar.h"
#include "sarmusic.h"
#include "sarmenucodes.h"
#include "config.h"


void SARMusicUpdate(sar_core_struct *core_ptr);


#define MAX(a,b)	(((a) > (b)) ? (a) : (b))
#define MIN(a,b)	(((a) < (b)) ? (a) : (b))
#define CLIP(a,l,h)	(MIN(MAX((a),(l)),(h)))


/*
 *	Checks the current situation (current music, current menu or
 *	simulation, player object state, etc) and changes the music
 *	as needed.
 *
 *	This function should be called once per loop or whenever the
 *	music is suspected to need to be changed.
 *
 *	The global opt->music state will be checked and music will
 *	be turned on or off as needed.
 */
void SARMusicUpdate(sar_core_struct *core_ptr)
{
	int prev_id, new_id, new_enter_id;
	sar_menu_struct *cur_menu_ptr;
	snd_recorder_struct *recorder = core_ptr->recorder;
	sar_option_struct *opt = &core_ptr->option;
	if(recorder == NULL)
	    return;

	/* Get current music id (can be -1) and record it as the
	 * previous music id
	 */
	prev_id = core_ptr->cur_music_id;

	/* Is music state switched on in the options? */
	if(opt->music)
	{

	}
	else
	{
	    /* Music is suppose to be off, so turn it off as needed
	     * and return
	     */
	    if(SoundMusicIsPlaying(recorder))
		SoundMusicStopPlay(recorder);
	    return;
	}

	/* ******************************************************** */

	/* Begin checking which music id should be played, for
	 * `repeating' and `enter' styles. After this check, new_id
	 * and new_enter_id will be updated to the id codes of the
	 * songs intended to be played
	 */
	new_id = -1;
	new_enter_id = -1;

	/* First determine if we are in the menu system or in
	 * simulation
	 *
	 * The appropriate new music ids should be choosen (if any)
	 * here
	 */
	cur_menu_ptr = SARGetCurrentMenuPtr(core_ptr);
	if(cur_menu_ptr != NULL)
	{
	    /* In menu system, choose new music ids by checking
	     * which menu we are currently in (check the current
	     * menu's name)
	     */
	    const char *menu_name = (const char *)cur_menu_ptr->name;
	    if(menu_name != NULL)
	    {
		/* Loading simulation menu? */
		if(!strcasecmp(menu_name, SAR_MENU_NAME_LOADING_SIMULATION))
		{
		    new_id = SAR_MUSIC_ID_LOADING_SIMULATION;
		}
		/* Main menu? */
		else if(!strcasecmp(menu_name, SAR_MENU_NAME_MAIN))
		{
		    new_id = SAR_MUSIC_ID_MENUS;
		}
/* TODO Add code to check other menus here */
		else
		{
		    /* Some other menu, use the generic menu music id */
		    new_id = SAR_MUSIC_ID_MENUS;
		}
	    }
	    else
	    {
		/* No menu name available, just use the generic menu
		 * music id then
		 */
		new_id = SAR_MUSIC_ID_MENUS;
	    }
	}       /* In menu system? */
	else
	{
	    /* Not in menu system, probably in simulation.  Check
	     * the state of the scene and player object to choose
	     * the appropriate music id
	     */
	    sar_scene_struct *scene = core_ptr->scene;
	    sar_object_struct *obj_ptr = NULL;


	    /* Scene structure must be available */
	    if(scene != NULL)
	    {
		/* Get the pointer to the player object, the state
		 * of the player object will dictate which music
		 * id will be choosen.
		 */
		obj_ptr = scene->player_obj_ptr;

		/* Player object exists? */
		if(obj_ptr != NULL)
		{
		    sar_object_aircraft_struct *obj_aircraft_ptr;
		    sar_obj_hoist_struct *hoist_ptr;


		    /* Get pointers to object's substructures */
		    obj_aircraft_ptr = SAR_OBJ_GET_AIRCRAFT(obj_ptr);
		    hoist_ptr = SARObjGetHoistPtr(obj_ptr, 0, NULL);

		    /* Begin checking by player object type and its
		     * status to determine the new music id's
		     */
		    if(obj_aircraft_ptr != NULL)
		    {
			if(obj_aircraft_ptr->landed)
			{
			    new_id =
			SAR_MUSIC_ID_SIMULATION_ONGROUND;
			    new_enter_id =
			SAR_MUSIC_ID_SIMULATION_ONGROUND_ENTER;
			}
			else
			{
			    /* In flight */
			    switch(scene->tod_code)
			    {
			      case SAR_TOD_CODE_NIGHT:
				new_id =
			SAR_MUSIC_ID_SIMULATION_INFLIGHT_NIGHT;
				new_enter_id =
			SAR_MUSIC_ID_SIMULATION_INFLIGHT_NIGHT_ENTER;
				break;
			      case SAR_TOD_CODE_DUSK:
			      case SAR_TOD_CODE_DAWN:
			      case SAR_TOD_CODE_DAY:
			      case SAR_TOD_CODE_UNDEFINED:
				new_id =
			SAR_MUSIC_ID_SIMULATION_INFLIGHT_DAY;
				new_enter_id =
			SAR_MUSIC_ID_SIMULATION_INFLIGHT_DAY_ENTER;
				break;
			    }
			}
		    }
		    if(hoist_ptr != NULL)
		    {
			if(hoist_ptr->rope_cur > 0.0)
			{
			    new_id =
				SAR_MUSIC_ID_SIMULATION_RESCUE;
			    new_enter_id =
				SAR_MUSIC_ID_SIMULATION_RESCUE_ENTER;
			}
		    }

		    /* Set music id's to defaults if they were not able
		     * to be determined above.
		     */
		    if(new_id < 0)
			new_id = SAR_MUSIC_ID_SIMULATION_ONGROUND;
		    if(new_enter_id < 0)
			new_enter_id = SAR_MUSIC_ID_SIMULATION_ONGROUND_ENTER;
		}
		else
		{
		    /* No player object available, assume on ground */
		    new_id = SAR_MUSIC_ID_SIMULATION_ONGROUND;
		    new_enter_id = SAR_MUSIC_ID_SIMULATION_ONGROUND_ENTER;
		}
	    }

	}       /* In simulation */


	/* ******************************************************** */

	/* At this point the new music id's should have been
	 * choosen, they can still be -1 to indicate no change.
	 */

/* Macro to return the music reference file as tmp_path from the
 * given music_ref_ptr. Uses variables tmp_path[PATH_MAX + NAME_MAX]
 * and music_ref_ptr.
 */
#define GET_MUSIC_REF_FILE      \
{ \
 *tmp_path = '\0'; \
 if((music_ref_ptr != NULL) ? (music_ref_ptr->filename != NULL) : 0) \
 { \
  struct stat stat_buf; \
  const char *cstrptr = PrefixPaths( \
   dname.local_data, music_ref_ptr->filename \
  ); \
  /* Check local file to see if it exists first */ \
  if((cstrptr != NULL) ? !stat(cstrptr, &stat_buf) : 0) \
  { \
   /* Found local file, put that path value in tmp_path */ \
   strncpy(tmp_path, cstrptr, PATH_MAX + NAME_MAX); \
  } \
  else \
  { \
   /* Check global file */ \
   cstrptr = PrefixPaths( \
    dname.global_data, music_ref_ptr->filename \
   ); \
   if((cstrptr != NULL) ? !stat(cstrptr, &stat_buf) : 0) \
   { \
    /* Found global file, put that path value in tmp_path */ \
    strncpy(tmp_path, cstrptr, PATH_MAX + NAME_MAX); \
   } \
  } \
 } \
}


	/* Change in music id's and new music id is valid? */
	if((new_id != prev_id) && (new_id > -1))
	{
	    int music_ref_num;
	    sar_music_ref_struct *music_ref_ptr;
	    char tmp_path[PATH_MAX + NAME_MAX];


	    if(opt->runtime_debug)
		printf(
"SARMusicUpdate(): Changing to music id %i from id %i\n",
		    new_id, prev_id
		);

	    /* Find music reference in list that matches the music id
	     * specified by new_id.
	     */
	    music_ref_ptr = SARMusicMatchPtr(
		core_ptr->music_ref, core_ptr->total_music_refs,
		new_id, &music_ref_num
	    );
	    /* Found music reference matching new_id? */
	    if(music_ref_ptr != NULL)
	    {
		/* Stop playing previous music (if any) and start
		 * playing the new one.
		 */
		GET_MUSIC_REF_FILE
		if(SoundMusicStartPlay(
		    recorder,
		    tmp_path,
		    (music_ref_ptr->flags & SAR_MUSIC_REF_FLAGS_REPEAT) ?
			-1 : 1
		))
		{
		    /* Error playing this music, need to print warning
		     * and switch off music.
		     */
		    fprintf(
			stderr,
			"%s: Unable to play music, turning music off.\n",
			music_ref_ptr->filename
		    );
		    opt->music = False;
		    core_ptr->cur_music_id = prev_id = new_id = -1;
		}
		else
		{
		    /* Successfully started playing new music, update
		     * current music id.
		     */
		    core_ptr->cur_music_id = prev_id = new_id;
		}
	    }
	    else
	    {
		/* No music reference for the given id, continue playing
		 * previous music and do not update current music id.
		 */
	    }
	}
	/* No change in music id's and new music id is valid? */
	else if(new_id > -1)
	{
	    /* No change in music id, but need to check if music is
	     * still playing. If it not playing then it needs to be
	     * either restarted or changed to the next consecutive
	     * song.
	     */
	    if(!SoundMusicIsPlaying(recorder))
	    {
		int music_ref_num;
		sar_music_ref_struct *music_ref_ptr;
		char tmp_path[PATH_MAX + NAME_MAX];


		if(opt->runtime_debug)
		    printf(
"SARMusicUpdate(): Music id %i has stopped playing.\n",
			new_id
		    );

		/* Update new_id to the `next' song that needs to be
		 * played after the current one has stopped.
		 */
		switch(prev_id)
		{
		  case SAR_MUSIC_ID_SIMULATION_ONGROUND_ENTER:
		    new_id = SAR_MUSIC_ID_SIMULATION_ONGROUND;
		    break;

		  case SAR_MUSIC_ID_SIMULATION_INFLIGHT_DAY_ENTER:
		    new_id = SAR_MUSIC_ID_SIMULATION_INFLIGHT_DAY;
		    break;

		  case SAR_MUSIC_ID_SIMULATION_INFLIGHT_NIGHT_ENTER:
		    new_id = SAR_MUSIC_ID_SIMULATION_INFLIGHT_NIGHT;
		    break;

		  case SAR_MUSIC_ID_SIMULATION_RESCUE_ENTER:
		    new_id = SAR_MUSIC_ID_SIMULATION_RESCUE;
		    break;

		  default:
		    /* All other music id's when stopped should just
		     * re-start as the same music id.
		     */
		    new_id = prev_id;
		    break;
		}

		if(opt->runtime_debug)
		    printf(
"SARMusicUpdate(): Previous music id %i stopped, playing new music id %i.\n",
			prev_id, new_id
		    );

		/* Find music reference in list that matches the music
		 * id specified by new_id.
		 */
		music_ref_ptr = SARMusicMatchPtr(
		    core_ptr->music_ref, core_ptr->total_music_refs,
		    new_id, &music_ref_num
		);
		/* Found music reference matching new_id? */
		if(music_ref_ptr != NULL)
		{
		    /* Start playing new music */
		    GET_MUSIC_REF_FILE
		    if(SoundMusicStartPlay(
			recorder,
			tmp_path,
			(music_ref_ptr->flags & SAR_MUSIC_REF_FLAGS_REPEAT) ?
			    -1 : 1
		    ))
		    {
			/* Error playing this music, need to print warning
			 * and switch off music.
			 */
			fprintf(
			    stderr,
			    "%s: Unable to play music, turning music off.\n",
			    music_ref_ptr->filename
			);
			opt->music = False;
			core_ptr->cur_music_id = prev_id = new_id = -1;
		    }
		    else
		    {
			/* Successfully started playing new music, update
			 * current music id.
			 */
			core_ptr->cur_music_id = prev_id = new_id;
		    }
		}
		else
		{
		    /* No music reference for the given id or music needs
		     * to stop until changed. So do not update the current
		     * music id (even if the previous music has stopped).
		     */
		}
	    }   /* Music has stopped playing? */
	}

#undef GET_MUSIC_REF_FILE
}

