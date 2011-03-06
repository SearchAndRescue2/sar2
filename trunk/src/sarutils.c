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
#include <time.h>
#include <math.h>
#include <sys/types.h>

#ifdef __MSW__
# include <windows.h>
#else
# include <sys/time.h>
# include <unistd.h>
#endif
#include <GL/gl.h>

#include "../include/strexp.h"
#include "../include/string.h"

#include "menu.h"
#include "obj.h"
#include "sar.h"   
#include "sardraw.h"
#include "config.h"


#ifdef __MSW__
static double rint(double x);
#endif	/* __MSW__ */

Boolean SARGetGLVersion(int *major, int *minor, int *release);
char *SARGetGLVendorName(void);
char *SARGetGLRendererName(void);
char **SARGelGLExtensionNames(int *strc);

int SARIsMenuAllocated(sar_core_struct *core_ptr, int n);
int SARMatchMenuByName(
	sar_core_struct *core_ptr, const char *name
);
sar_menu_struct *SARMatchMenuByNamePtr(
	sar_core_struct *core_ptr, const char *name
);
sar_menu_struct *SARGetCurrentMenuPtr(sar_core_struct *core_ptr);

sar_player_stat_struct *SARGetPlayerStatPtr(
	sar_core_struct *core_ptr, int i
);
sar_player_stat_struct *SARPlayerStatNew(
	const char *name,
	int score
);
void SARPlayerStatDelete(sar_player_stat_struct *pstat);
void SARPlayerStatResort(sar_player_stat_struct **list, int total);
sar_player_stat_struct *SARPlayerStatCurrent(
	sar_core_struct *core_ptr, int *n
);

void SARDeleteListItemData(sar_menu_list_item_struct *item_ptr);

char *SARTimeOfDayString(sar_core_struct *core_ptr, float t);
char *SARDeltaTimeString(sar_core_struct *core_ptr, time_t t);

void SARSetGlobalTextColorBrightness(
	sar_core_struct *core_ptr, float g
);

void SARReportGLError(
	sar_core_struct *core_ptr, GLenum error_code
);


#define ATOI(s)         (((s) != NULL) ? atoi(s) : 0)
#define ATOL(s)         (((s) != NULL) ? atol(s) : 0)
#define ATOF(s)         (((s) != NULL) ? atof(s) : 0.0f)
#define STRDUP(s)       (((s) != NULL) ? strdup(s) : NULL)

#define MAX(a,b)        (((a) > (b)) ? (a) : (b))
#define MIN(a,b)        (((a) < (b)) ? (a) : (b))
#define CLIP(a,l,h)     (MIN(MAX((a),(l)),(h)))
#define IS_STRING_EMPTY(s)      (((s) != NULL) ? (*(s) == '\0') : TRUE)

#define RADTODEG(r)     ((r) * 180 / PI)
#define DEGTORAD(d)     ((d) * PI / 180)


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
 *	Returns the version numbers for the GL version, returns
 *	GL_TRUE on success or GL_FALSE on failure.
 *
 *	If any of the inputs are NULL, then that respective input will
 *	not be set.
 */
Boolean SARGetGLVersion(int *major, int *minor, int *release)
{
	const GLubyte *strptr;
	char **strv;
	int strc;


	/* Reset inputs. */
	if(major != NULL)
	    *major = 0;
	if(minor != NULL)
	    *minor = 0;
	if(release != NULL)
	    *release = 0;

	/* Get statically allocated '.' character separated string
	 * containing the version numbers. Format can be either
	 * "major.minor" or "major.minor.release".
	 */
	strptr = glGetString(GL_VERSION);
	if(strptr == NULL)
	    return(GL_FALSE);

	/* Explode version string at the '.' characters. */
	strv = strchrexp((const char *)strptr, '.', &strc);
	if(strv == NULL)
	    return(GL_FALSE);

	if((strc > 0) && (major != NULL))
	    *major = atoi(strv[0]);

	if((strc > 1) && (minor != NULL))
	    *minor = atoi(strv[1]);

	if((strc > 2) && (release != NULL))
	    *release = atoi(strv[2]);

	strlistfree(strv, strc);

	return(GL_TRUE);
}

/*
 *      Returns a dynamically allocated string containing the GL
 *      vendor name. The returned string must be free()'ed by the
 *      calling function.
 *
 *      Can return NULL on error.
 */
char *SARGetGLVendorName(void)
{
	/* Return copy of string describing the GL vendor name. */
	return(STRDUP((const char *)glGetString(GL_VENDOR)));
}

/*
 *	Returns a dynamically allocated string containing the GL
 *	renderer name. The returned string must be free()'ed by the
 *	calling function.
 *
 *	Can return NULL on error.
 */
char *SARGetGLRendererName(void)
{
	/* Return copy of string describing the GL renderer name. */
	return(STRDUP((const char *)glGetString(GL_RENDERER)));
}


/*
 *	Returns a dynamically allocated array of strings containing
 *	the list of GL extension names available.
 *
 *	If strc is not NULL then it will be set to the number of
 *	strings returned.
 *
 *	Can return NULL on error.
 */
char **SARGelGLExtensionNames(int *strc)
{
	const GLubyte *strptr;
	char **strv;
	int lstrc;


	/* Reset inputs. */
	if(strc != NULL)
	    *strc = 0;

	/* Get statically allocated space separated string containing
	 * the GL extensions list.
	 */
	strptr = glGetString(GL_EXTENSIONS);
	if(strptr == NULL)
	    return(NULL);

	/* Explode space separated string. */
	strv = strexp((char *)strptr, &lstrc);
	if(strv == NULL)
	    return(NULL);

	/* Update number of strings reciefved (hence number of
	 * extensions).
	 */
	if(strc != NULL)
	    *strc = lstrc;

	return(strv);
}


/*
 *      Is menu allocated on the core structure.
 */
int SARIsMenuAllocated(sar_core_struct *core_ptr, int n)
{
	if(core_ptr == NULL)
	    return(0);
	else if((n < 0) || (n >= core_ptr->total_menus))
	    return(0);
	else if(core_ptr->menu[n] == NULL)
	    return(0);
	else
	    return(1);
}

/*
 *      Returns the index number of the menu who's name matches the
 *	given name (case insensitive).
 *
 *	Can return -1 for no match or error.
 */
int SARMatchMenuByName(
	sar_core_struct *core_ptr, const char *name
)
{
	int i;
	sar_menu_struct *m;

	if((core_ptr == NULL) || (name == NULL))
	    return(-1);

	for(i = 0; i < core_ptr->total_menus; i++)
	{
	    m = core_ptr->menu[i];
	    if(m == NULL)
		continue;

	    if(m->name == NULL)
		continue;

	    if(!strcasecmp(m->name, name))
		return(i);
	}

	return(-1);
}

/*
 *      Returns the pointer to the menu who's name matches the
 *      given name (case insensitive).
 *
 *      Can return NULL for no match or error.
 */
sar_menu_struct *SARMatchMenuByNamePtr(
	sar_core_struct *core_ptr, const char *name
)
{
	int i;
	sar_menu_struct *m;

	if((core_ptr == NULL) || (name == NULL))
	    return(NULL);

	for(i = 0; i < core_ptr->total_menus; i++)
	{
	    m = core_ptr->menu[i];
	    if(m == NULL)
		continue;

	    if(m->name == NULL)
		continue;

	    if(!strcasecmp(m->name, name))
		return(m);
	}

	return(NULL);
}

/*
 *	Returns the pointer to the current menu or NULL if there
 *	is no current menu.
 */
sar_menu_struct *SARGetCurrentMenuPtr(sar_core_struct *core_ptr)
{
	int n = (core_ptr != NULL) ? core_ptr->cur_menu : -1;
	if(n < 0)
	    return(NULL);
	else if(n >= core_ptr->total_menus)
	    return(NULL);
	else
	    return(core_ptr->menu[n]);
}


/*
 *	Returns the player stats for the player specified by index i.
 */
sar_player_stat_struct *SARGetPlayerStatPtr(
	sar_core_struct *core_ptr, int i
)
{
	if(core_ptr == NULL)
	    return(NULL);
	else if((i < 0) || (i >= core_ptr->total_player_stats))
	    return(NULL);
	else
	    return(core_ptr->player_stat[i]);
}

/*
 *	Creates a new player stat.
 */
sar_player_stat_struct *SARPlayerStatNew(
	const char *name,
	int score
)
{
	sar_player_stat_struct *pstat = SAR_PLAYER_STAT(calloc(
	    1, sizeof(sar_player_stat_struct)
	));
	if(pstat == NULL)
	    return(NULL);

	pstat->name = STRDUP(name);
	pstat->score = score;

	return(pstat);
}

/*
 *	Deletes the player stat.
 */
void SARPlayerStatDelete(sar_player_stat_struct *pstat)
{
	free(pstat->name);
	free(pstat);
}

/*
 *	Resorts the list of player stats.
 */
void SARPlayerStatResort(sar_player_stat_struct **list, int total)
{
	int i, n;
	char **names, *name;
	sar_player_stat_struct **tmp_list, *pstat;

	if((list == NULL) || (total <= 0))
	    return;

	/* Create tempory list for sorting */
	names = (char **)calloc(
	    total, sizeof(char *)
	);
	tmp_list = (sar_player_stat_struct **)calloc(
	    total, sizeof(sar_player_stat_struct *)
	);
	if((names == NULL) || (tmp_list == NULL))
	{
	    free(names);
	    free(tmp_list);
	    return;
	}

	/* Get list of names */
	for(i = 0; i < total; i++)
	    names[i] = (list[i] != NULL) ? list[i]->name : NULL;

	/* Sort names list */
	StringQSort(names, total);

	/* Iterate through sorted names */
	for(i = 0; i < total; i++)
	{
	    name = names[i];
	    if(name == NULL)
		continue;

	    /* Iterate through given list, looking for the item who's
	     * name matches the current one in the sorted names list
	     */
	    for(n = 0; n < total; n++)
	    {
		pstat = list[n];
		if(pstat == NULL)
		    continue;

		if(pstat->name == NULL)
		    continue;

		if(!strcmp(name, pstat->name))
		{
		    tmp_list[i] = pstat;
		    list[n] = NULL;
		    break;
		}
	    }
	    if(n >= total)
	    {
		/* Unable to find match */
		tmp_list[i] = NULL;
	    }

	}

	/* Copy sorted list to the specified list */
	memcpy(list, tmp_list, total * sizeof(sar_player_stat_struct *));

	/* Delete tempory list */
	free(names);
	free(tmp_list);
}

/*
 *	Returns the current selected player stat.
 */
sar_player_stat_struct *SARPlayerStatCurrent(
	sar_core_struct *core_ptr, int *n
)
{
	int i = (core_ptr != NULL) ? 
	    core_ptr->option.last_selected_player : -1;
	if(i < 0)
	{
	    if(n != NULL)
		*n = -1;
	    return(NULL);
	}

	if(i < core_ptr->total_player_stats)
	{
	    if(n != NULL)
		*n = i;
	    return(core_ptr->player_stat[i]);
	}
	else
	{
	    if(n != NULL)
		*n = -1;
	    return(NULL);
	}
}


/*
 *	Deletes the client data structure specified on the menu
 *	list object item pointer.
 */
void SARDeleteListItemData(sar_menu_list_item_struct *item_ptr)
{
	sar_menu_list_item_data_struct *d = SAR_MENU_LIST_ITEM_DATA(
	    (item_ptr != NULL) ? item_ptr->client_data : NULL
	);
	if(d == NULL)
	    return;

	free(d->filename);
	free(d->name);
	free(d);

	item_ptr->client_data = NULL;
}

/*
 *	Returns a statically allocated string containing the time
 *	of day in %h:%m format or by whatever format specified on the
 *	core structure if the core_ptr is not NULL.
 *
 *	Time t is given in seconds from midnight.
 */
char *SARTimeOfDayString(sar_core_struct *core_ptr, float t)
{
	static char str[80];

#define HTOS(h)	((h) * 3600.0f)

	/* Sanitize time. */
	while(t >= HTOS(24.0f))
	    t -= HTOS(24.0f);
	while(t < HTOS(0.0f))
	    t += HTOS(24.0f);

	*str = '\0';

	/* No core structure given? */
	if(core_ptr == NULL)
	{
	    int	h = (int)MAX((t / 3600.0f), 0.0f),
		m = (int)MAX(((int)((int)t / 60) % 60), 0.0f);

	    sprintf(str, "%i:%2.2i", h, m);
	}
	else
	{
	    int h = (int)MAX((t / 3600.0f), 0.0f),
		m = (int)MAX(((int)((int)t / 60) % 60), 0.0f);

	    sprintf(str, "%i:%2.2i", h, m);
	}

#undef HTOS
	return(str);
}

/*
 *      Returns a statically allocated string verbosly stating the
 *      delta time t in accordance to the format specified by the
 *	core_ptr. If core_ptr is NULL then a generic format will be
 *	assumed.
 *
 *      This function never returns NULL.
 */
char *SARDeltaTimeString(sar_core_struct *core_ptr, time_t t)
{
#define len 256
	static char s[len];

	if(t <= 0)
	    return("none");

	if(t < 60)
	{
	    sprintf(s, "%ld second%s",
		t,
		(t == 1) ? "" : "s"
	    );
	}
	else if(t < 3600) 
	{
	    time_t	min = t / 60,
			sec = t % 60;
	    if(sec != 0)
		sprintf(
		    s, "%ld minute%s %ld second%s",
		    min,
		    (min == 1) ? "" : "s",
		    sec,
		    (sec == 1) ? "" : "s"
		);
	    else
		sprintf(
		    s, "%ld minute%s",
		    min,
		    (min == 1) ? "" : "s"
		);
	}
	else if(t < 86400)
	{
	    time_t	hr = t / 3600,
			min = (t / 60) % 60;
	    if(min != 0)
		sprintf(
		    s, "%ld hour%s %ld minute%s",
		    hr,
		    (hr == 1) ? "" : "s",
		    min,
		    (min == 1) ? "" : "s"
		);
	    else
		sprintf(
		    s, "%ld hour%s",
		    hr,
		    (hr == 1) ? "" : "s"
		);
	}

	return(s);
#undef len
}


/*
 *	Updates the global hud and message text color based on the
 *	given gamma g which must be in the domain of 0.0 to 1.0.
 */
void SARSetGlobalTextColorBrightness(
	sar_core_struct *core_ptr, float g
)
{
	sar_color_struct *c;
	sar_option_struct *opt;

	if(core_ptr == NULL)
	    return;

	opt = &core_ptr->option;

	/* Set message text color. */
	c = &opt->message_color;
	c->a = 1.0f;
	c->r = g;
	c->g = g;
	c->b = g;

	/* Set new hud color. */
	c = &opt->hud_color;
	if(g > 0.5f)
	{
	    float ng = (g - 0.5f) / 0.5f;
	    c->a = 1.0f;
	    c->r = ng;
	    c->g = 1.0f;
	    c->b = ng;
	}
	else
	{
	    c->a = 1.0f;
	    c->r = 0.0f;
	    c->g = g * 2.0f;
	    c->b = 0.0f;
	}
}

/*
 *	Reports the given GL error code, does not check if the
 *	error is actually an error and prints explicitly even if
 *	global options specify not to print errors.
 */
void SARReportGLError(
	sar_core_struct *core_ptr, GLenum error_code 
)
{
	const char *error_mesg = NULL, *error_type = NULL;

	switch(error_code)
	{
	  case GL_INVALID_ENUM:
	    error_type = "GL_INVALID_ENUM";
	    error_mesg =
		"Invalid GLenum argument (possibly out of range)";
	    break;

	  case GL_INVALID_VALUE:
	    error_type = "GL_INVALID_VALUE";
	    error_mesg =
		"Numeric argument out of range";
	    break;

	  case GL_INVALID_OPERATION:
	    error_type = "GL_INVALID_OPERATION";
	    error_mesg =
		"Operation illegal in current state";
	    break;

	  case GL_STACK_OVERFLOW:
	    error_type = "GL_STACK_OVERFLOW";
	    error_mesg =
		"Command would cause stack overflow";
	    break;

	  case GL_STACK_UNDERFLOW:
	    error_type = "GL_STACK_UNDERFLOW";
	    error_mesg =
		"Command would cause a stack underflow";
	    break;

	  case GL_OUT_OF_MEMORY:
	    error_type = "GL_OUT_OF_MEMORY";
	    error_mesg =
		"Not enough memory left to execute command";
	    break;

	  default:
	    error_type = "*UNKNOWN*";
	    error_mesg = "Undefined GL error";
	    break;
	}

	fprintf(
	    stderr,
	    "GL Error code %i: %s: %s\n",
	    error_code, error_type, error_mesg
	);
}
