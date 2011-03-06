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

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "../include/string.h"

#include "obj.h"
#include "messages.h"
#include "sar.h"
#include "config.h"


void SARMessageAdd(sar_scene_struct *scene, const char *message);
void SARMessageClearAll(sar_scene_struct *scene);
void SARBannerMessageAppend(sar_scene_struct *scene, const char *message);
void SARCameraRefTitleSet(sar_scene_struct *scene, const char *message);


#define ATOI(s)		(((s) != NULL) ? atoi(s) : 0)
#define ATOL(s)		(((s) != NULL) ? atol(s) : 0)
#define ATOF(s)		(((s) != NULL) ? (float)atof(s) : 0.0f)
#define STRDUP(s)	(((s) != NULL) ? strdup(s) : NULL)

#define MAX(a,b)	(((a) > (b)) ? (a) : (b))
#define MIN(a,b)	(((a) < (b)) ? (a) : (b))
#define CLIP(a,l,h)	(MIN(MAX((a),(l)),(h)))
#define STRLEN(s)	(((s) != NULL) ? (int)strlen(s) : 0)
#define STRISEMPTY(s)	(((s) != NULL) ? (*(s) == '\0') : 1)

#define RADTODEG(r)	((r) * 180.0 / PI)
#define DEGTORAD(d)	((d) * PI / 180.0)


/*
 *	Adds the specified message to the list in the scene.
 *	The new message will be inserted at the last index position.
 *
 *	The message pointer array on the scene will not change in size.
 */
void SARMessageAdd(sar_scene_struct *scene, const char *message)
{
	int i, n;


	if(scene == NULL)
	    return;

	/* No message line pointers allocated? */
	if(scene->total_messages <= 0)
	    return;

	/* Free oldest message */
	free(scene->message[0]);

	/* Get new position to insert new message */
	n = scene->total_messages - 1;

	/* Shift message pointers */
	for(i = 0; i < n; i++)
	    scene->message[i] = scene->message[i + 1];

	/* Copy new message to pointer array */
	scene->message[n] = STRDUP(message);

	/* Update the time the message should be shown until */
	scene->message_display_until = cur_millitime +
	    SAR_MESSAGE_SHOW_INT;
}

/*
 *	Deletes all messages on the message pointer array on the scene
 *	structure and resets all pointers to NULL.
 *
 *	The size of the message pointer array will not be changed.
 *
 *	The message_display_until member on the scene will be reset
 *	to 0.
 */
void SARMessageClearAll(sar_scene_struct *scene)
{
	int i;

	if(scene == NULL)
	    return;

	/* Reset the time to keep displaying messages to 0 so messages
	 * shouldn't be displayed anymore if they were being displayed.
	 */
	scene->message_display_until = 0;

	/* Delete each message and set pointer to NULL */
	for(i = 0; i < scene->total_messages; i++)
	{
	    free(scene->message[i]);
	    scene->message[i] = NULL;
	}
}

/*
 *	Appends given message line to scene's sticky banner message, can
 *	be called multiple times to set several lines.
 *
 *	Passing NULL clears the entire list.
 */
void SARBannerMessageAppend(sar_scene_struct *scene, const char *message)
{
	if(scene == NULL)
	    return;

	if(message == NULL)
	{
	    strlistfree(
		scene->sticky_banner_message, 
		scene->total_sticky_banner_messages 
	    ); 
	    scene->sticky_banner_message = NULL;
	    scene->total_sticky_banner_messages = 0;
	}
	else
	{
	    int i;

	    if(scene->total_sticky_banner_messages < 0)
		scene->total_sticky_banner_messages = 0;

	    i = scene->total_sticky_banner_messages;
	    scene->total_sticky_banner_messages = i + 1;

	    scene->sticky_banner_message = (char **)realloc(
		scene->sticky_banner_message,
		scene->total_sticky_banner_messages * sizeof(char *)
	    );
	    if(scene->sticky_banner_message == NULL)
	    {
		scene->total_sticky_banner_messages = 0;
		return;
	    }

	    scene->sticky_banner_message[i] = STRDUP(message);
	}
}

/*
 *	Replaces the current camera reference title message on the scene
 *	structure.
 *
 *	Passing NULL clears it.
 */
void SARCameraRefTitleSet(sar_scene_struct *scene, const char *message)
{
	if(scene == NULL)
	    return;

	if(message == NULL)
	{
	    free(scene->camera_ref_title);
	    scene->camera_ref_title = NULL;
	    scene->camera_ref_title_display_until = 0;
	}
	else
	{
	    char *s;

	    free(scene->camera_ref_title);
	    scene->camera_ref_title = s = STRDUP(message);

	    /* Make first character upper case */
	    if(s[0] != '\0')
		s[0] = toupper(s[0]);

	    scene->camera_ref_title_display_until = cur_millitime +
		SAR_MESSAGE_SHOW_INT;
	}
}
