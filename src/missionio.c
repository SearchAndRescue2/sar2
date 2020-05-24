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
#include <limits.h>
#include <ctype.h>
#include <sys/stat.h>
#include <math.h>

#include "../include/fio.h"
#include "../include/string.h"
#include "../include/strexp.h"
#include "../include/disk.h"

#include "sfm.h"

#include "obj.h"
#include "objutils.h"
#include "objio.h"
#include "menu.h"
#include "sarreality.h"
#include "weather.h"
#include "mission.h"
#include "simutils.h"
#include "sar.h"
#include "sarfio.h"
#include "sceneio.h"
#include "missionio.h"
#include "config.h"


static int SARMissionGetInterceptNameFromScene(
	const char *filename,
	const char *obj_name,
	sar_intercept_struct *intercept
);
static int SARMissionLoadSceneMapToMenuMap(
	sar_core_struct *core_ptr, sar_menu_struct *m,
	sar_menu_map_struct *map_ptr,
	const char *filename                    /* Scene file */
);
static int SARMissionLoadSceneMapToMenuMapMarkings(
	sar_core_struct *core_ptr,
	sar_menu_struct *m, sar_menu_map_struct *map_ptr,
	const char *scene_filename, const char *mission_filename
);

char *SARMissionLoadDescription(const char *filename);
void SARMissionLoadSceneToMenuMap(
	sar_core_struct *core_ptr,
	sar_menu_struct *m,
	const char *filename		/* Mission file name */
);
void SARMissionLoadMissionLogToMenuMap(
	sar_core_struct *core_ptr,
	sar_menu_struct *m,
	const char *filename            /* Mission log file name */
);

sar_mission_struct *SARMissionLoadFromFile(
	sar_core_struct *core_ptr,
	const char *filename,
	void *client_data,
	int (*progress_func)(void *, long, long)
);

void SARMissionLogReset(
	sar_core_struct *core_ptr, sar_mission_struct *mission,
	const char *filename		/* Mission log file name */
);
void SARMissionLogEvent(
	sar_core_struct *core_ptr, sar_mission_struct *mission,
	int event_type,			/* One of SAR_LOG_EVENT_* */
	float tod,			/* Time of day in seconds since midnight */
	sar_position_struct *pos,	/* Position of event */
	const float *value,		/* Additional values */
	int total_values,		/* Total number of additional values */
	const char *message,		/* The message */
	const char *filename            /* Mission log file name */
);


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

#define ISCOMMENT(c)    ((c) == SAR_COMMENT_CHAR)
#define ISCR(c)         (((c) == '\n') || ((c) == '\r'))


/*
 *	Load intercept by name from scene file, returns 0 on success
 *	or -1 on no match.
 */
static int SARMissionGetInterceptNameFromScene(
	const char *filename, const char *obj_name,
	sar_intercept_struct *intercept
)
{
	int i, status, ptype, total_parms;
	char *cur_obj_name = NULL;
	void *p, **parm;
	sar_parm_version_struct *p_version;
	sar_parm_object_name_struct *p_object_name;
	sar_parm_translate_struct *p_translate;
	sar_parm_rotate_struct *p_rotate;
	Boolean matched_translate = False, matched_rotate = False;

	if(STRISEMPTY(filename) || STRISEMPTY(obj_name) ||
	   (intercept == NULL)
	)
	    return(-1);

	/* Load parms from scene file */
	status = SARParmLoadFromFile(
	    filename, SAR_FILE_FORMAT_SCENE,
	    &parm, &total_parms,
	    -1,
	    NULL, NULL
	);
	if(status)
	    return(-1);

	/* Iterate through loaded parms */
	for(i = 0; i < total_parms; i++)
	{
	    p = parm[i];
	    if(p == NULL)
		continue;

	    /* Handle by parm type */
	    ptype = *(int *)p;
	    switch(ptype)
	    {
	      case SAR_PARM_VERSION:
		p_version = (sar_parm_version_struct *)p;
		if((p_version->major > PROG_VERSION_MAJOR) ||
		   (p_version->minor > PROG_VERSION_MINOR) ||
		   (p_version->release > PROG_VERSION_RELEASE)
		)
		{
		    int need_warn = 0;
		    if(p_version->major > PROG_VERSION_MAJOR)
			need_warn = 1;
		    else if((p_version->major == PROG_VERSION_MAJOR) &&
			    (p_version->minor > PROG_VERSION_MINOR)
		    )
			need_warn = 1;
		    else if((p_version->major == PROG_VERSION_MAJOR) &&
			    (p_version->minor == PROG_VERSION_MINOR) &&
			    (p_version->release == PROG_VERSION_RELEASE)
		    )
			need_warn = 1;
		    if(need_warn)
			fprintf(
			    stderr,
"%s: Warning: File format version %i.%i.%i is newer than program\
 version %i.%i.%i.\n",
			    filename,
			    p_version->major, p_version->minor,
			    p_version->release, PROG_VERSION_MAJOR,
			    PROG_VERSION_MINOR, PROG_VERSION_RELEASE
			);
		}
		break;

	      case SAR_PARM_OBJECT_NAME:
		p_object_name = (sar_parm_object_name_struct *)p;
	        /* Update current object name */
		free(cur_obj_name);
		cur_obj_name = STRDUP(p_object_name->name);
		/* Reset all other context values */
		matched_translate = False;
		matched_rotate = False;
		break;

	      case SAR_PARM_TRANSLATE:
		p_translate = (sar_parm_translate_struct *)p;
		if((cur_obj_name != NULL) && !matched_translate)
		{
		    if(!strcasecmp(cur_obj_name, obj_name))
		    {
			intercept->x = p_translate->translate.x;
			intercept->y = p_translate->translate.y;
			intercept->z = p_translate->translate.z;

			intercept->radius = 40;	/* Explicitly set */

			matched_translate = True;
		    }
		}
		break;

	      case SAR_PARM_ROTATE:
		p_rotate = (sar_parm_rotate_struct *)p;
		if((cur_obj_name != NULL) && !matched_rotate)
		{
		    if(!strcasecmp(cur_obj_name, obj_name))
		    {
/* TODO, not really needed right now */

			matched_rotate = True;
		    }
		}
		break;
	    }
	}

	/* Delete all loaded parms */
	SARParmDeleteAll(&parm, &total_parms);

	free(cur_obj_name);

	return(0);
}

/*
 *	Loads the scene map of the given scene specified by filename
 *	to the given map object as a texture. Any existing texture already
 *	loaded on the given map object will be unloaded first.
 *
 *	Returns -1 on error or 0 on success.
 *
 *      This function is intended to be called by 
 *	SARMissionLoadMissionLogToMenuMap().
 */
static int SARMissionLoadSceneMapToMenuMap(
	sar_core_struct *core_ptr, sar_menu_struct *m,
	sar_menu_map_struct *map_ptr,
	const char *filename                    /* Scene file */
)
{
	int i, status, ptype, total_parms;
	const char *img_path;
	void *p, **parm;
	sar_parm_version_struct *p_version;
	sar_parm_name_struct *p_name;
	sar_parm_scene_map_struct *p_scene_map;


	if(STRISEMPTY(filename) || (map_ptr == NULL))
	    return(-1);

	/* Load parms from scene file */
	status = SARParmLoadFromFile(
	    filename, SAR_FILE_FORMAT_SCENE,
	    &parm, &total_parms,
	    -1,
	    NULL, NULL
	);
	if(status)
	    return(-1);

	/* Iterate through loaded parms */
	for(i = 0; i < total_parms; i++)
	{
	    p = parm[i];
	    if(p == NULL)
		continue;

	    /* Handle by parm type */
	    ptype = *(int *)p;
	    switch(ptype)
	    {
	      case SAR_PARM_VERSION:
		p_version = (sar_parm_version_struct *)p;
		if((p_version->major > PROG_VERSION_MAJOR) ||
		   (p_version->minor > PROG_VERSION_MINOR) ||
		   (p_version->release > PROG_VERSION_RELEASE)
		)
		{
		    int need_warn = 0;
		    if(p_version->major > PROG_VERSION_MAJOR)
			need_warn = 1;
		    else if((p_version->major == PROG_VERSION_MAJOR) &&
			    (p_version->minor > PROG_VERSION_MINOR)
		    )
			need_warn = 1;
		    else if((p_version->major == PROG_VERSION_MAJOR) &&
			    (p_version->minor == PROG_VERSION_MINOR) &&
			    (p_version->release == PROG_VERSION_RELEASE)
		    )
			need_warn = 1;
		    if(need_warn)
			fprintf(
			    stderr,
"%s: Warning: File format version %i.%i.%i is newer than program\
 version %i.%i.%i.\n",
			    filename,
			    p_version->major, p_version->minor,
			    p_version->release, PROG_VERSION_MAJOR,
			    PROG_VERSION_MINOR, PROG_VERSION_RELEASE
			);
		}
		break;

	      case SAR_PARM_NAME:
		p_name = (sar_parm_name_struct *)p;
		free(map_ptr->title);
		map_ptr->title = STRDUP(p_name->name);
		break;

	      case SAR_PARM_SCENE_MAP:
		p_scene_map = (sar_parm_scene_map_struct *)p;
		/* Width and height in meters */
		map_ptr->bg_tex_width = (float)MAX(p_scene_map->width, 10.0);
		map_ptr->bg_tex_height = (float)MAX(p_scene_map->height, 10.0);

		/* Texture image path */
		img_path = p_scene_map->file;
		if(!STRISEMPTY(img_path))
		{
		    char *dpath;

		    if(ISPATHABSOLUTE(img_path))
		    {
			dpath = STRDUP(img_path);
		    }
		    else
		    {
			struct stat stat_buf;
			const char *s = PrefixPaths(dname.local_data, img_path);
			if((s != NULL) ? stat(s, &stat_buf) : True)
			    s = PrefixPaths(
				dname.global_data, img_path
			    );
			dpath = STRDUP(s);
		    }

		    /* Unload menu map's background texture as needed */
		    V3DTextureDestroy(map_ptr->bg_tex);

		    /* Load new texture for menu map's background */
		    map_ptr->bg_tex = V3DTextureLoadFromFile2DPreempt(
			dpath,
			"mission_map_tex",	/* Not used */
			V3D_TEX_FORMAT_RGB	/* Destination format */
		    );
		    V3DTexturePriority(map_ptr->bg_tex, 0.95f);

		    free(dpath);
		}
		break;

	    }   /* Handle by parm type */
	}       /* Iterate through loaded parms */

	/* Delete all loaded parms */
	SARParmDeleteAll(&parm, &total_parms);

	return(0);
}

/*
 *	Called by SARMissionLoadSceneMapToMenuMap().
 *
 *	Loads the scene map and markings of the given scene specified by
 *	filename to the map object as a texture
 *
 *	Intercept points are not loaded.
 */
static int SARMissionLoadSceneMapToMenuMapMarkings(
	sar_core_struct *core_ptr,
	sar_menu_struct *m, sar_menu_map_struct *map_ptr,
	const char *scene_filename, const char *mission_filename
)
{
	int i, n, pass, ptype, total_parms;
	const char *img_path;
	void *p, **parm;
	sar_menu_color_struct fg_color;
	sar_menu_map_marking_struct *marking_ptr;
	Boolean	got_translate,
		got_rotate,
		got_object_name,
		got_object_map_desc;
	sar_parm_version_struct *p_version;
	sar_parm_name_struct *p_name;
	sar_parm_scene_map_struct *p_scene_map;
	sar_parm_new_object_struct *p_new_object;
	sar_parm_new_helipad_struct *p_new_helipad;
	sar_parm_new_runway_struct *p_new_runway;
	sar_parm_new_human_struct *p_new_human;
	sar_parm_new_premodeled_struct *p_new_premodeled;
	sar_parm_translate_struct *p_translate;
	sar_parm_rotate_struct *p_rotate;
	sar_parm_object_name_struct *p_object_name;
	sar_parm_object_map_description_struct *p_object_map_desc;

	if((m == NULL) || (map_ptr == NULL))
	    return(-1);

	/* Load parms from scene file */
	if(SARParmLoadFromFile(
	    scene_filename, SAR_FILE_FORMAT_SCENE,
	    &parm, &total_parms,
	    -1,
	    NULL, NULL
	))
	    return(-1);

	/* Iterate through loaded parms */
	for(i = 0; i < total_parms; i++)
	{
	    p = parm[i];
	    if(p == NULL)
		continue;

	    /* Handle by parm type */
	    ptype = *(int *)p;
	    switch(ptype)
	    {
	      case SAR_PARM_VERSION:
		p_version = (sar_parm_version_struct *)p;
		if((p_version->major > PROG_VERSION_MAJOR) ||
		   (p_version->minor > PROG_VERSION_MINOR) ||
		   (p_version->release > PROG_VERSION_RELEASE)
		)
		{
		    int need_warn = 0;
		    if(p_version->major > PROG_VERSION_MAJOR)
			need_warn = 1;
		    else if((p_version->major == PROG_VERSION_MAJOR) &&
			    (p_version->minor > PROG_VERSION_MINOR)
		    )
			need_warn = 1;
		    else if((p_version->major == PROG_VERSION_MAJOR) &&
			    (p_version->minor == PROG_VERSION_MINOR) &&
			    (p_version->release == PROG_VERSION_RELEASE)
		    )
			need_warn = 1;
		    if(need_warn)
			fprintf(
			    stderr,
"%s: Warning: File format version %i.%i.%i is newer than program\
 version %i.%i.%i.\n",
			    scene_filename,
			    p_version->major, p_version->minor,
			    p_version->release, PROG_VERSION_MAJOR,
			    PROG_VERSION_MINOR, PROG_VERSION_RELEASE
			);
		}
		break;

	      case SAR_PARM_NAME:
		p_name = (sar_parm_name_struct *)p;
		free(map_ptr->title);
		map_ptr->title = STRDUP(p_name->name);
		break;

	      case SAR_PARM_SCENE_MAP:
		p_scene_map = (sar_parm_scene_map_struct *)p;
		/* Width and height in meters */
		map_ptr->bg_tex_width = (float)MAX(p_scene_map->width, 10.0);
		map_ptr->bg_tex_height = (float)MAX(p_scene_map->height, 10.0);

		/* Texture image path */
		img_path = p_scene_map->file;
		if(!STRISEMPTY(img_path))
		{
		    char *dpath;

		    if(ISPATHABSOLUTE(img_path))
		    {
			dpath = STRDUP(img_path);
		    }
		    else
		    {
			struct stat stat_buf;
			const char *s = PrefixPaths(dname.local_data, img_path);
			if((s != NULL) ? stat(s, &stat_buf) : True)
			    s = PrefixPaths(
				dname.global_data, img_path
			    );
			dpath = STRDUP(s);
		    }

		    /* Unload menu map's background texture as needed */
		    V3DTextureDestroy(map_ptr->bg_tex);

		    /* Load new texture for menu map's background */
		    map_ptr->bg_tex = V3DTextureLoadFromFile2DPreempt(
			dpath,
			"mission_map_tex",	/* Not used */
			V3D_TEX_FORMAT_RGB	/* Destination format */
		    );
		    V3DTexturePriority(map_ptr->bg_tex, 0.95f);

		    free(dpath);
		}
		break;

	    }	/* Handle by parm type */
	}	/* Iterate through loaded parms */


/* Resets all local contexts pointers */
#define RESET_CONTEXTS	{		\
 marking_ptr = NULL;			\
 got_translate = False;			\
 got_rotate = False;			\
 got_object_name = False;		\
 got_object_map_desc = False;		\
}

	/* Reset local contexts */
	RESET_CONTEXTS

	/* Perform two iterations, the first on the existing loaded
	 * parms and then reload the parms from the mission file
	 * and load from them. This will ensure that we get map markings
	 * from first the scene file and then the mission file.
	 */
	for(pass = 0; pass < 2; pass++)
	{
	    /* Iterate through loaded parms (second time around) */
	    for(i = 0; i < total_parms; i++)
	    {
		p = parm[i];
		if(p == NULL)
		    continue;

		/* Handle by parm type */
		ptype = *(int *)p;
		switch(ptype)
		{
		  case SAR_PARM_NEW_OBJECT:
		    p_new_object = (sar_parm_new_object_struct *)p;
		    /* Reset local contexts */
		    RESET_CONTEXTS
/* Ignore for now */
		    break;

		  case SAR_PARM_NEW_HELIPAD:
		    p_new_helipad = (sar_parm_new_helipad_struct *)p;
		    /* Reset local contexts */
		    RESET_CONTEXTS
		    /* Set foreground color */
		    fg_color.a = 1.0f;
		    fg_color.r = 0.0f;
		    fg_color.g = 1.0f;
		    fg_color.b = 0.0f;
		    /* Allocate new marking on map for intercept */
		    n = SARMenuMapAppendMarking(
			map_ptr, SAR_MENU_MAP_MARKING_TYPE_ICON,
			&fg_color, 0.0, 0.0, 0.0, 0.0,
			core_ptr->menumap_helipad_img,
			NULL
		    );
		    marking_ptr = (n > -1) ? map_ptr->marking[n] : NULL;
		    break;

		  case SAR_PARM_NEW_RUNWAY:
		    p_new_runway = (sar_parm_new_runway_struct *)p;
		    /* Reset local contexts */
		    RESET_CONTEXTS
/* Ignore for now */ 
		    break;

	          case SAR_PARM_NEW_HUMAN:
		    p_new_human = (sar_parm_new_human_struct *)p;
		    /* Reset local contexts */
		    RESET_CONTEXTS
/* Ignore for now */
		    break;

		  case SAR_PARM_NEW_PREMODELED:
		    p_new_premodeled = (sar_parm_new_premodeled_struct *)p;
		    /* Reset local contexts */
		    RESET_CONTEXTS
/* Ignore for now */
		    break;

		  case SAR_PARM_TRANSLATE:
		    p_translate = (sar_parm_translate_struct *)p;
		    if((marking_ptr != NULL) && !got_translate)
		    {
		        marking_ptr->x = p_translate->translate.x;
		        marking_ptr->y = p_translate->translate.y;
		        got_translate = True;
		    }
		    break;

		  case SAR_PARM_ROTATE:
		    p_rotate = (sar_parm_rotate_struct *)p;
		    if((marking_ptr != NULL) && !got_rotate)
		    {
/* Ignore for now */
		        got_rotate = True;
		    }
		    break;

		  case SAR_PARM_OBJECT_NAME:
		    p_object_name = (sar_parm_object_name_struct *)p;
		    if((marking_ptr != NULL) && !got_object_name)
		    {
/* Ignore for now */
		        got_object_name = True;
		    }
		    break;

	          case SAR_PARM_OBJECT_MAP_DESCRIPTION:
		    p_object_map_desc =
			(sar_parm_object_map_description_struct *)p;
		    if((marking_ptr != NULL) && !got_object_map_desc)
		    {
		        free(marking_ptr->desc);
		        marking_ptr->desc = STRDUP(
			    p_object_map_desc->description
			);
			got_object_map_desc = True;
		    }
		    break;

		}	/* Handle by parm type */
	    }	/* Iterate through loaded parms (second time around) */

	    if(pass >= 1)
		break;

	    /* Delete loaded parms and then load parms from mission file */
	    SARParmDeleteAll(&parm, &total_parms);
	    if(SARParmLoadFromFile(
		mission_filename, SAR_FILE_FORMAT_MISSION,
		&parm, &total_parms,
		-1,
		NULL, NULL
	    ))
		break;

	    RESET_CONTEXTS
	}

	/* Delete loaded parms */
	SARParmDeleteAll(&parm, &total_parms);

#undef RESET_CONTEXTS

	return(0);
}


/*
 *	Returns a pointer to a dynamically allocated description fetched
 *	from the specified mission file.
 *
 *	The calling function needs to deallocate the returned string.
 */
char *SARMissionLoadDescription(const char *filename)
{
	int i, status, ptype, total_parms;
	void *p, **parm;
	sar_parm_description_struct *p_desc;
	char *desc = NULL;

	if(STRISEMPTY(filename))
	    return(desc);

	/* Load parms from mission file */
	status = SARParmLoadFromFile(
	    filename, SAR_FILE_FORMAT_MISSION,
	    &parm, &total_parms,
	    SAR_PARM_DESCRIPTION,
	    NULL, NULL  
	);
	if(status)
	    return(desc);

	/* Iterate through loaded parms */
	for(i = 0; i < total_parms; i++)
	{
	    p = parm[i];
	    if(p == NULL)
		continue;

	    ptype = *(int *)p;
	    switch(ptype)
	    {
	      case SAR_PARM_DESCRIPTION:
		p_desc = (sar_parm_description_struct *)p;
		free(desc);
		desc = STRDUP(p_desc->description);
		break;
	    }
	}

	/* Delete loaded parms */
	SARParmDeleteAll(&parm, &total_parms);

	return(desc);
}

/*
 *	Loads the scene specified by filename to the map object on the
 *	menu.
 *
 *	All relivent objects specified in the scene will be loaded
 *	as markings on the map object.
 */
void SARMissionLoadSceneToMenuMap(
	sar_core_struct *core_ptr,
	sar_menu_struct *m,
	const char *filename		/* Mission file name */
)
{
	int i, n, status, ptype, total_parms;
	void *p, **parm;
	sar_parm_version_struct *p_version;
	sar_parm_mission_scene_file_struct *p_mission_scene_file;
	sar_parm_mission_begin_at_struct *p_mission_begin_at;
	sar_parm_mission_begin_at_pos_struct *p_mission_begin_at_pos;
	sar_parm_mission_arrive_at_struct *p_mission_arrive_at;
	sar_parm_mission_add_intercept_struct *p_mission_add_intercept;
	int map_num = -1;
	sar_menu_map_struct *map_ptr = NULL;
	const char *s;
	Boolean begin_at_set = False;
	char	*scene_filename = NULL,
		*arrive_at_name = NULL,
		*begin_at_name = NULL;
	sar_menu_map_marking_struct	*marking_ptr = NULL,
					*intercept_marking_ptr = NULL;
	int intercepts_set = 0;

	if(m == NULL)
	    return;

	/* Get first map object on the menu */
	for(i = 0; i < m->total_objects; i++)
	{
	    p = m->object[i];
	    if(p == NULL)
		continue;

	    ptype = (*(int *)p);
	    if(ptype == SAR_MENU_OBJECT_TYPE_MAP)
	    {
		map_num = i;
		map_ptr = SAR_MENU_MAP(p);
		break;
	    }
	}
	if(map_ptr == NULL)
	    return;

	/* Delete all markings on menu's map object */
	SARMenuMapDeleteAllMarkings(map_ptr);

	/* If no filename was given then only delete map markings */
	if(STRISEMPTY(filename))
	    return;

	/* Note that order of which parameters appear in the mission
	 * file is important!
	 *
	 * We expect to parse certain parameters before others
	 */

	/* Load parms from mission file */
	status = SARParmLoadFromFile(
	    filename, SAR_FILE_FORMAT_MISSION,
	    &parm, &total_parms,   
	    -1,
	    NULL, NULL
	);
	if(status)
	    return;

	/* Iterate through loaded parms */
	for(i = 0; i < total_parms; i++)
	{
	    p = parm[i];
	    if(p == NULL)
		continue;

	    /* Handle by parm type */
	    ptype = *(int *)p;
	    switch(ptype)
	    {
	      case SAR_PARM_VERSION:
		p_version = (sar_parm_version_struct *)p;
		if((p_version->major > PROG_VERSION_MAJOR) ||
		   (p_version->minor > PROG_VERSION_MINOR) ||
		   (p_version->release > PROG_VERSION_RELEASE)
		)
		{
		    int need_warn = 0;
		    if(p_version->major > PROG_VERSION_MAJOR)
			need_warn = 1;
		    else if((p_version->major == PROG_VERSION_MAJOR) &&
			    (p_version->minor > PROG_VERSION_MINOR)
		    )
			need_warn = 1;
		    else if((p_version->major == PROG_VERSION_MAJOR) &&
			    (p_version->minor == PROG_VERSION_MINOR) &&
			    (p_version->release == PROG_VERSION_RELEASE)
		    )
			need_warn = 1;
		    if(need_warn)
			fprintf(
			    stderr,
"%s: Warning: File format version %i.%i.%i is newer than program\
 version %i.%i.%i.\n",
			    filename,
			    p_version->major, p_version->minor,
			    p_version->release, PROG_VERSION_MAJOR,
			    PROG_VERSION_MINOR, PROG_VERSION_RELEASE
			);
		}
		break;

	      case SAR_PARM_MISSION_SCENE_FILE:
		p_mission_scene_file = (sar_parm_mission_scene_file_struct *)p;
		s = p_mission_scene_file->file;
		if(!STRISEMPTY(s))
		{
		    if(ISPATHABSOLUTE(s))
		    {
			free(scene_filename);
			scene_filename = STRDUP(s);
		    }
		    else
		    {
			struct stat stat_buf;
			char *s2 = PrefixPaths(dname.local_data, s);
			if((s2 != NULL) ? stat(s2, &stat_buf) : True)
			    s2 = PrefixPaths(dname.global_data, s);
			if(s2 != NULL)
			{
			    free(scene_filename);
			    scene_filename = STRDUP(s2);
			}
		    }

		    /* Load information from scene file which are
		     * relivent to the mission map (such as map
		     * background texture file and landmarks)
		     */                 
		    SARMissionLoadSceneMapToMenuMapMarkings(
			core_ptr, m, map_ptr,
			scene_filename,	/* Scene file */
			filename	/* Mission file */
		    );
		}
		break;

	      case SAR_PARM_MISSION_BEGIN_AT:
		p_mission_begin_at = (sar_parm_mission_begin_at_struct *)p;
		if(p_mission_begin_at->name != NULL)
		{
		    sar_menu_color_struct fg_color;
		    sar_intercept_struct intercept;
		    memset(&intercept, 0x00, sizeof(sar_intercept_struct));

		    /* Update begin at name */
		    free(begin_at_name);
		    begin_at_name = STRDUP(p_mission_begin_at->name);

		    /* Match intercept by name and set begin at location (only
		     * if the begin at location has not been set yet)
		     */
		    if(!SARMissionGetInterceptNameFromScene(
			scene_filename, begin_at_name, &intercept
		    ) && !begin_at_set)
		    {
			fg_color.a = 1.0;
			fg_color.r = 0.0;
			fg_color.g = 1.0;
			fg_color.b = 0.0;

			/* Allocate new marking on map for intercept */
			n = SARMenuMapAppendMarking(
			    map_ptr, SAR_MENU_MAP_MARKING_TYPE_INTERCEPT_LINE,
			    &fg_color,
			    intercept.x, intercept.y,
			    0.0, 0.0,
			    NULL, NULL
			);
			marking_ptr = ((n >= 0) ? map_ptr->marking[n] : NULL);
			intercept_marking_ptr = marking_ptr;
			if(marking_ptr != NULL)
			{
			    intercepts_set++;

			    /* Reset meters to pixels coeff zoom */
			    map_ptr->m_to_pixels_coeff = (float)SAR_MAP_DEF_MTOP_COEFF;

			    /* Set initial scroll position on map with
			     * respect to the begin at location
			     */
			    map_ptr->scroll_x = (int)(-intercept.x *
				map_ptr->m_to_pixels_coeff);
			    map_ptr->scroll_y = (int)(-intercept.y *
				map_ptr->m_to_pixels_coeff);

			    /* Reset selected_marking on menu map */
			    map_ptr->selected_marking = -1;
			}

			begin_at_set = True;
		    }
		}
		break;

	     case SAR_PARM_MISSION_BEGIN_AT_POS:
		p_mission_begin_at_pos = (sar_parm_mission_begin_at_pos_struct *)p;
		if(!begin_at_set)
		{
		    sar_menu_color_struct fg_color;
		    sar_position_struct *ipos = &p_mission_begin_at_pos->pos;

		    fg_color.a = 1.0f;
		    fg_color.r = 0.0f;
		    fg_color.g = 1.0f;
		    fg_color.b = 0.0f;

		    /* Allocate new marking on map for intercept */
		    n = SARMenuMapAppendMarking(
			map_ptr, SAR_MENU_MAP_MARKING_TYPE_INTERCEPT_LINE,
			&fg_color,
			ipos->x, ipos->y,
			0.0f, 0.0f,
			NULL, NULL
		    );
		    marking_ptr = (n > -1) ? map_ptr->marking[n] : NULL;
		    intercept_marking_ptr = marking_ptr;
		    if(marking_ptr != NULL)
		    {
			intercepts_set++;

			/* Reset meters to pixels coeff zoom */
			map_ptr->m_to_pixels_coeff = (float)SAR_MAP_DEF_MTOP_COEFF;

			/* Set initial scroll position on map with
			 * respect to the begin at location
			 */
			map_ptr->scroll_x = (int)(-ipos->x *
			    map_ptr->m_to_pixels_coeff);
			map_ptr->scroll_y = (int)(-ipos->y *
			    map_ptr->m_to_pixels_coeff);

			/* Reset selected_marking on menu map */
			map_ptr->selected_marking = -1;
		    }

		    begin_at_set = True;
		}
		break;

	      case SAR_PARM_MISSION_ARRIVE_AT:
		p_mission_arrive_at = (sar_parm_mission_arrive_at_struct *)p;
		if(p_mission_arrive_at->name)
		{
		    free(arrive_at_name);
		    arrive_at_name = STRDUP(p_mission_arrive_at->name);
		}
		break;

	      case SAR_PARM_MISSION_ADD_INTERCEPT:
		p_mission_add_intercept = (sar_parm_mission_add_intercept_struct *)p;
		if(1)
		{
		    sar_intercept_struct intercept;
		    memset(&intercept, 0x00, sizeof(sar_intercept_struct));

		    /* Set intercept by reference code */
		    switch(p_mission_add_intercept->ref_code)
		    {
		      case 3:		/* Arrive at location */
			SARMissionGetInterceptNameFromScene(
			    scene_filename, arrive_at_name,
			    &intercept
			);
			break;

		      case 2:		/* Begin at location */
			SARMissionGetInterceptNameFromScene(
			    scene_filename, begin_at_name,
			    &intercept
			);
			break;

		      default:		/* Standard intercept */
			intercept.x = p_mission_add_intercept->pos.x;
			intercept.y = p_mission_add_intercept->pos.y;
			intercept.z = p_mission_add_intercept->pos.z;
			intercept.radius = p_mission_add_intercept->radius;
			intercept.urgency = p_mission_add_intercept->urgency;
			break;
		    }

		    /* Allocate new map marking structure, note previous
		     * marking structure should already be allocated from
		     * another add intercept parameter or begin at name
		     * parameter
		     */
		    if((marking_ptr != NULL) &&
		       (intercept_marking_ptr != NULL)
		    )
		    {
			sar_menu_map_marking_struct *new_ptr;

			/* Allocate new intercept marking structure and
			 * copy over end position values from previous
			 * marking structure
			 */
			n = SARMenuMapAppendMarking(
			    map_ptr, SAR_MENU_MAP_MARKING_TYPE_INTERCEPT_LINE,
			    NULL, 0.0f, 0.0f, 0.0f, 0.0f,
			    NULL, NULL
			);
			new_ptr = (n > -1) ? map_ptr->marking[n] : NULL;

			/* Set end position on last marking structure */
			marking_ptr->x_end = intercept.x;
			marking_ptr->y_end = intercept.y;

			intercepts_set++;

			/* New marking structure allocated successfully? */
			if(new_ptr != NULL)
			{
			    /* Copy over fg color from previous marking */
			    memcpy(
			        &new_ptr->fg_color,
			        &marking_ptr->fg_color,
			        sizeof(sar_menu_color_struct)
			    );
			    new_ptr->x = intercept.x;
			    new_ptr->y = intercept.y;

			    intercepts_set++;

			    intercept_marking_ptr = new_ptr;
			    marking_ptr = new_ptr;
		        }
			else
			{
			    intercept_marking_ptr = NULL;
			    marking_ptr = NULL;
			}

			/* Allocate another marking structure to represent
			 * way point icon
			 */
			n = SARMenuMapAppendMarking(
			    map_ptr, SAR_MENU_MAP_MARKING_TYPE_ICON,
			    NULL,
			    intercept.x, intercept.y,
			    0.0f, 0.0f,
			    core_ptr->menumap_intercept_img, NULL
			);
			new_ptr = ((n >= 0) ? map_ptr->marking[n] : NULL);
		    }
		}
		break;
	    }	/* Handle parm by type */
	}	/* Iterate through loaded parms */


	/* Delete loaded parms */
	SARParmDeleteAll(&parm, &total_parms);


	/* Delete local context names */
	free(scene_filename);
	scene_filename = NULL;

	free(arrive_at_name);
	arrive_at_name = NULL;

	free(begin_at_name);
	begin_at_name = NULL;


	/* If the number of intercepts set on the map is an odd number,
	 * then that implies that the last intercept line marking is
	 * extraneous
	 */
	if(intercepts_set & 1)
	{
	    /* Delete the last marking pointer of type
	     * SAR_MENU_MAP_MARKING_TYPE_INTERCEPT_LINE
	     *
	     * Iterate from last to first of all markings on the map
	     */
	    for(i = map_ptr->total_markings - 1; i >= 0; i--)
	    {
		marking_ptr = map_ptr->marking[i];
		if(marking_ptr == NULL)
		    continue;

		/* Check marking type */
		if(marking_ptr->type == SAR_MENU_MAP_MARKING_TYPE_INTERCEPT_LINE)
		{
		    /* Need to delete this marking on the map, set the
		     * pointer to NULL after deleting it
		     */
		    free(marking_ptr);
		    map_ptr->marking[i] = NULL;

		    /* Do not delete any more markings */
		    break;
		}
	    }
	}

}

/*
 *	Procedure to load the mission log specified by filename to the
 *	map object on the given menu.
 */
void SARMissionLoadMissionLogToMenuMap(
	sar_core_struct *core_ptr,
	sar_menu_struct *m,
	const char *filename		/* Mission log file name */
)
{
	int i, ptype, status, total_parms;
	void *p, **parm;
	sar_position_struct *pos;
	sar_parm_version_struct *p_version;
	sar_parm_mission_log_header_struct *p_mission_log_header;
	sar_parm_mission_log_event_struct *p_mission_log_event;
	int map_num = -1;
	sar_menu_map_struct *map_ptr = NULL;
	Boolean handled_header = False;
	char	*title = NULL,
		*scene_filename = NULL,
		*player_stats_filename = NULL;
	sar_menu_map_marking_struct *last_intercept_marking = NULL;
	int total_intercept_markings = 0;
	int total_events = 0;

	if(m == NULL)
	    return;

	/* Get first map object on the menu */
	for(i = 0; i < m->total_objects; i++)
	{
	    p = m->object[i];
	    if(p == NULL)
		continue;

	    ptype = (*(int *)p);
	    if(ptype == SAR_MENU_OBJECT_TYPE_MAP)
	    {
		map_num = i;
		map_ptr = SAR_MENU_MAP(p);
		break;
	    }
	}
	if(map_ptr == NULL)
	    return;

	/* Delete all markings on menu's map object */
	SARMenuMapDeleteAllMarkings(map_ptr);

	/* If no filename was given then only delete map markings */
	if(STRISEMPTY(filename))
	    return;

	/* Load parms from mission log file */
	status = SARParmLoadFromFile(
	    filename, SAR_FILE_FORMAT_MISSION_LOG,
	    &parm, &total_parms,
	    -1,
	    NULL, NULL
	);
	if(status)
	    return;


	/* Iterate through loaded parms */
	for(i = 0; i < total_parms; i++)
	{
	    p = parm[i];
	    if(p == NULL)
		continue;

	    /* Handle by parm type */
	    ptype = *(int *)p;
	    switch(ptype)
	    {
	      case SAR_PARM_VERSION:
		p_version = (sar_parm_version_struct *)p;
		if((p_version->major > PROG_VERSION_MAJOR) ||
		   (p_version->minor > PROG_VERSION_MINOR) ||
		   (p_version->release > PROG_VERSION_RELEASE)
		)
		{
		    int need_warn = 0;
		    if(p_version->major > PROG_VERSION_MAJOR)
			need_warn = 1;
		    else if((p_version->major == PROG_VERSION_MAJOR) &&
			    (p_version->minor > PROG_VERSION_MINOR)
		    )
			need_warn = 1;
		    else if((p_version->major == PROG_VERSION_MAJOR) &&
			    (p_version->minor == PROG_VERSION_MINOR) &&
			    (p_version->release == PROG_VERSION_RELEASE)
		    )
			need_warn = 1;
		    if(need_warn)
			fprintf(
			    stderr,
"%s: Warning: File format version %i.%i.%i is newer than program\
 version %i.%i.%i.\n",
			    filename,
			    p_version->major, p_version->minor,
			    p_version->release, PROG_VERSION_MAJOR,
			    PROG_VERSION_MINOR, PROG_VERSION_RELEASE
			);
		}
		break;

	      case SAR_PARM_MISSION_LOG_HEADER:
		p_mission_log_header = (sar_parm_mission_log_header_struct *)p;

		free(title);
		title = STRDUP(p_mission_log_header->title);

		free(scene_filename);
		scene_filename = STRDUP(p_mission_log_header->scene_file);

		free(player_stats_filename);
		player_stats_filename = STRDUP(p_mission_log_header->player_stats_file);

		/* Handle this header if we have not yet handled the
		 * header earlier
		 */
		if(!handled_header)
		{
		    /* Got scene file name from reading the log file? */
		    if(scene_filename != NULL)
			SARMissionLoadSceneMapToMenuMap(
			    core_ptr, m, map_ptr, scene_filename
			);

		    /* Reset map */
		    map_ptr->m_to_pixels_coeff = (float)SAR_MAP_DEF_MTOP_COEFF;
		    /* Reset initial scroll position */
		    map_ptr->scroll_x = (int)(
			0.0f * map_ptr->m_to_pixels_coeff
		    );
		    map_ptr->scroll_y = (int)(
			0.0f * map_ptr->m_to_pixels_coeff
		    );
		    /* Reset selected_marking on menu map */
		    map_ptr->selected_marking = -1;

		    /* Title needs to be set after calling
		     * SARMissionLoadSceneMapToMenuMap() since it will
		     * set the title
		     */
		    free(map_ptr->title);
		    map_ptr->title = STRDUP(title);

		    /* Mark that we handled the header */
		    handled_header = True;
		}
		break;

	      case SAR_PARM_MISSION_LOG_EVENT:
		p_mission_log_event = (sar_parm_mission_log_event_struct *)p;
		pos = &p_mission_log_event->pos;
		if(True)
		{
		    int n;
		    sar_menu_color_struct fg_color;

		    fg_color.a = 1.0f;
		    fg_color.r = 0.0f;
		    fg_color.g = 1.0f;
		    fg_color.b = 0.0f;

		    /* Handle mission log event by its type */
		    switch(p_mission_log_event->event_type)
		    {
		      case SAR_MISSION_LOG_EVENT_COMMENT:
			SARMenuMapAppendMarking(
			    map_ptr, SAR_MENU_MAP_MARKING_TYPE_ICON,
			    &fg_color,
			    pos->x, pos->y,
			    0.0f, 0.0f,
			    NULL,		/* No icon */
			    p_mission_log_event->message
			);
			break;

		      case SAR_MISSION_LOG_EVENT_POSITION:
			/* Was there a prior intercept marking? */
			if(last_intercept_marking != NULL)
			{
			    /* Set last intercept marking's end position
			     * to the position specified by this log event
			     */
			    last_intercept_marking->x_end = pos->x;
			    last_intercept_marking->y_end = pos->y;
			}
			/* Create a new intercept line marking */
			n = SARMenuMapAppendMarking(
			    map_ptr, SAR_MENU_MAP_MARKING_TYPE_INTERCEPT_LINE,
			    &fg_color,
			    pos->x, pos->y,
			    pos->x, pos->y,
			    NULL,	/* No icon */
			    NULL	/* No message */
			);
			/* Record pointer to this intercept line marking */
			last_intercept_marking = (n > -1) ?
			    map_ptr->marking[n] : NULL;
			/* Increment total number of intercept markings */
			total_intercept_markings++;
			break;

		      case SAR_MISSION_LOG_EVENT_TAKEOFF:
			SARMenuMapAppendMarking(
			    map_ptr, SAR_MENU_MAP_MARKING_TYPE_ICON,
			    &fg_color,
			    pos->x, pos->y,
			    0.0f, 0.0f,
			    core_ptr->menumap_helicopter_img,
			    p_mission_log_event->message
			);
			break;

		      case SAR_MISSION_LOG_EVENT_LAND:
			SARMenuMapAppendMarking(
			    map_ptr, SAR_MENU_MAP_MARKING_TYPE_ICON,
			    &fg_color,
			    pos->x, pos->y,
			    0.0f, 0.0f,
			    core_ptr->menumap_helicopter_img,
			    p_mission_log_event->message
			);
			break;

		      case SAR_MISSION_LOG_EVENT_CRASH:
			SARMenuMapAppendMarking(
			    map_ptr, SAR_MENU_MAP_MARKING_TYPE_ICON,
			    &fg_color,
			    pos->x, pos->y,
			    0.0f, 0.0f,
			    core_ptr->menumap_crash_img,
			    p_mission_log_event->message
			);
			break;

		      case SAR_MISSION_LOG_EVENT_PICKUP:
			SARMenuMapAppendMarking(
			    map_ptr, SAR_MENU_MAP_MARKING_TYPE_ICON,
			    &fg_color,
			    pos->x, pos->y,
			    0.0f, 0.0f,
			    core_ptr->menumap_victim_img,
			    p_mission_log_event->message
			);
			break;

		      case SAR_MISSION_LOG_EVENT_DROPOFF:
			SARMenuMapAppendMarking(
			    map_ptr, SAR_MENU_MAP_MARKING_TYPE_ICON,
			    &fg_color,
			    pos->x, pos->y,
			    0.0f, 0.0f,
			    core_ptr->menumap_helicopter_img,
			    p_mission_log_event->message
			);
			break;
		    }
		}

		/* If this is the first event then set map's scroll
		 * position to it
		 */
		if(total_events == 0)
		{
		    map_ptr->scroll_x = (int)(
			-pos->x * map_ptr->m_to_pixels_coeff
		    );
		    map_ptr->scroll_y = (int)(
			-pos->y * map_ptr->m_to_pixels_coeff
		    );
		    map_ptr->selected_marking = 0;
		}

		total_events++;

		break;
	    }
	}

	/* Delete loaded parms */
	SARParmDeleteAll(&parm, &total_parms);

	free(title);
	free(scene_filename);
	free(player_stats_filename);
}

/*
 *	Loads a new mission
 *
 *	Note that the scene structure in the core structure must be
 *	allocated and with all values reset.
 */
sar_mission_struct *SARMissionLoadFromFile(
	sar_core_struct *core_ptr, const char *filename,
	void *client_data,
	int (*progress_func)(void *, long, long)
)
{
	int i, status, ptype, total_parms;
	void *p, **parm;
	sar_parm_version_struct *p_version;
	sar_parm_name_struct *p_name;
	sar_parm_description_struct *p_desc;
	sar_parm_player_model_file_struct *p_player_model_file;
	sar_parm_weather_struct *p_weather;
	sar_parm_time_of_day_struct *p_time_of_day;
	sar_parm_wind_struct *p_wind;
	sar_parm_texture_base_directory_struct *p_texture_base_directory;
	sar_parm_texture_load_struct *p_texture_load;
	sar_parm_mission_new_objective_struct *p_mission_new_objective;
	sar_parm_mission_scene_file_struct *p_mission_scene_file;
	sar_parm_mission_time_left_struct *p_mission_time_left;
	sar_parm_mission_begin_at_struct *p_mission_begin_at;
	sar_parm_mission_begin_at_pos_struct *p_mission_begin_at_pos;
	sar_parm_mission_arrive_at_struct *p_mission_arrive_at;
	sar_parm_mission_message_success_struct *p_mission_message_success;
	sar_parm_mission_message_fail_struct *p_mission_message_fail;
	sar_parm_mission_humans_tally_struct *p_mission_humans_tally;
	sar_parm_mission_add_intercept_struct *p_mission_add_intercept;
	sar_parm_new_object_struct *p_new_object;
	sar_parm_new_helipad_struct *p_new_helipad;
	sar_parm_new_runway_struct *p_new_runway;
	sar_parm_new_human_struct *p_new_human;
	sar_parm_new_fire_struct *p_new_fire;
	sar_parm_new_smoke_struct *p_new_smoke;
	sar_parm_new_premodeled_struct *p_new_premodeled;
	sar_parm_model_file_struct *p_model_file;
	sar_parm_translate_struct *p_translate;
	sar_parm_translate_random_struct *p_translate_random;
	sar_parm_rotate_struct *p_rotate;
	sar_parm_no_depth_test_struct *p_no_depth_test;
	sar_parm_polygon_offset_struct *p_polygon_offset;
	sar_parm_object_name_struct *p_object_name;
	sar_parm_runway_approach_lighting_north_struct *p_runway_applight_n;
	sar_parm_runway_approach_lighting_south_struct *p_runway_applight_s;
	sar_parm_human_message_enter_struct *p_human_message_enter;
	sar_parm_human_reference_struct *p_human_reference;

	Boolean loaded_scene = False;
	sar_mission_struct *mission;
	int objective_num = -1;
	sar_mission_objective_struct *objective = NULL;

	int humans_need_rescue = 0;
	char	*begin_at_name = NULL,
		*arrive_at_name = NULL,
		*weather_preset_name = NULL;

	int obj_num = -1;
	sar_object_struct *obj_ptr = NULL;
	sar_object_aircraft_struct *obj_aircraft_ptr = NULL;
	sar_object_ground_struct *obj_ground_ptr = NULL;
	sar_object_helipad_struct *obj_helipad_ptr = NULL;
	sar_object_runway_struct *obj_runway_ptr = NULL;
	sar_object_human_struct *obj_human_ptr = NULL;
	sar_object_smoke_struct *obj_smoke_ptr = NULL;
	sar_object_fire_struct *obj_fire_ptr = NULL;
	Boolean begin_at_set = False;
	sar_position_struct begin_pos;
	sar_direction_struct begin_dir;

	sar_scene_struct *scene = core_ptr->scene;
	sar_object_struct ***ptr = &core_ptr->object;
	int *total = &core_ptr->total_objects;
	const sar_option_struct *opt = &core_ptr->option;
	if((scene == NULL) || (filename == NULL))
	    return(NULL);

/* Regets object substructure pointers from the specified object */
#define CONTEXT_REGET_SUBSTRUCTURE_PTRS(_o_) {		\
 if((_o_) != NULL) {					\
  obj_aircraft_ptr = SAR_OBJ_GET_AIRCRAFT(_o_);		\
  obj_ground_ptr = SAR_OBJ_GET_GROUND(_o_);		\
  obj_helipad_ptr = SAR_OBJ_GET_HELIPAD(_o_);		\
  obj_runway_ptr = SAR_OBJ_GET_RUNWAY(_o_);		\
  obj_human_ptr = SAR_OBJ_GET_HUMAN(_o_);		\
  obj_smoke_ptr = SAR_OBJ_GET_SMOKE(_o_);		\
  obj_fire_ptr = SAR_OBJ_GET_FIRE(_o_);			\
 }							\
}

	/* Note, order of which parameters appear in the mission file
	 * is important! We expect to parse certain parameters before
	 * others in this function.
	 */

	/* Load parms from mission file */
	status = SARParmLoadFromFile(
	    filename, SAR_FILE_FORMAT_MISSION,
	    &parm, &total_parms,
	    -1,
	    NULL, NULL
	);
	if(status)
	    return(NULL);

	if(opt->runtime_debug)
	    printf(
"SARMissionLoadFromFile(): Creating new mission...\n"
	    );

	/* Allocate a new mission structure */
	mission = (sar_mission_struct *)calloc(
	    1, sizeof(sar_mission_struct)
	);
	if(mission == NULL)
	{
	    SARParmDeleteAll(&parm, &total_parms);
	    return(NULL);
	}

	/* Set up default mission values */
	mission->state = MISSION_STATE_IN_PROGRESS;
	mission->title = NULL;
	mission->description = NULL;
	mission->start_location_name = NULL;
	mission->scene_file = NULL;
	mission->player_model_file = NULL;
	mission->player_stats_file = NULL;
	mission->objective = NULL;
	mission->total_objectives = 0;
	mission->cur_objective = 0;		/* Always start on first objective */
	mission->time_spent = 0.0f;
	mission->check_int = 1000;		/* In milliseconds */
	mission->next_check = cur_millitime + mission->check_int;
	mission->log_position_int = 10000;
	mission->next_log_position = cur_millitime + mission->log_position_int;

	/* Iterate through loaded parms */
	for(i = 0; i < total_parms; i++)
	{
	    p = parm[i];
	    if(p == NULL)
		continue;

	    /* Handle by parm type */
	    ptype = *(int *)p;
	    switch(ptype)
	    {
	      case SAR_PARM_VERSION:
		p_version = (sar_parm_version_struct *)p;
		if((p_version->major > PROG_VERSION_MAJOR) ||
		   (p_version->minor > PROG_VERSION_MINOR) ||
		   (p_version->release > PROG_VERSION_RELEASE)
		)
		{
		    int need_warn = 0;
		    if(p_version->major > PROG_VERSION_MAJOR)
			need_warn = 1;
		    else if((p_version->major == PROG_VERSION_MAJOR) &&
			    (p_version->minor > PROG_VERSION_MINOR)
		    )
			need_warn = 1;
		    else if((p_version->major == PROG_VERSION_MAJOR) &&
			    (p_version->minor == PROG_VERSION_MINOR) &&
			    (p_version->release == PROG_VERSION_RELEASE)
		    )
			need_warn = 1;
		    if(need_warn)
			fprintf(
			    stderr,
"%s: Warning: File format version %i.%i.%i is newer than program\
 version %i.%i.%i.\n",
			    filename,
			    p_version->major, p_version->minor,
			    p_version->release, PROG_VERSION_MAJOR,
			    PROG_VERSION_MINOR, PROG_VERSION_RELEASE
			);
		}
		break;

	      case SAR_PARM_NAME:
		p_name = (sar_parm_name_struct *)p;
		free(mission->title);
		mission->title = STRDUP(p_name->name);
		break;

	      case SAR_PARM_DESCRIPTION:
		p_desc = (sar_parm_description_struct *)p;
		free(mission->description);
		mission->description = STRDUP(p_desc->description);
		break;

	      case SAR_PARM_PLAYER_MODEL_FILE:
		p_player_model_file = (sar_parm_player_model_file_struct *)p;
		if(loaded_scene)
		{
		    const char *player_file = p_player_model_file->file;
		    char *full_path;

		    /* Complete path */
		    if(ISPATHABSOLUTE(player_file))
		    {
			full_path = STRDUP(player_file);
		    }
		    else
		    {
			struct stat stat_buf;
			full_path = STRDUP(PrefixPaths(
			    dname.local_data, player_file
			));
			if((full_path != NULL) ? stat(full_path, &stat_buf) : True)
			{
			    free(full_path);
			    full_path = STRDUP(PrefixPaths(
				dname.global_data, player_file
			    ));
			}
		    }
		    if(full_path == NULL)
			fprintf(
			    stderr,
"%s: Warning: Unable to complete path \"%s\".\n",
			    filename, player_file
			);

		    /* Update player model file name on mission and
		     * core structures.
		     */
		    free(mission->player_model_file);
		    mission->player_model_file = STRDUP(full_path);

		    free(core_ptr->cur_player_model_file);
		    core_ptr->cur_player_model_file = STRDUP(full_path);

		    /* Create the player object since the model file is
		     * now known
		     */
		    SARSceneAddPlayerObject(
			core_ptr, scene,
			full_path,
			&begin_pos, &begin_dir
		    );

		    /* Update object context pointers */
		    obj_num = scene->player_obj_num;
		    obj_ptr = scene->player_obj_ptr;
		    CONTEXT_REGET_SUBSTRUCTURE_PTRS(obj_ptr)

		    free(full_path);
		}
		else
		{
		    fprintf(
			stderr,
 "%s: \"player_model_file\" may not be specified before \"mission_scene_file\".\n",
			filename
		    );
		}
		break;

	      case SAR_PARM_WEATHER:
		p_weather = (sar_parm_weather_struct *)p;
		if(loaded_scene)
		{
		    fprintf(
			stderr,
 "%s: \"weather\" may not be specified after \"mission_scene_file\".\n",
			filename
		    );
		}
		else
		{
		    free(weather_preset_name);
		    weather_preset_name = STRDUP(p_weather->weather_preset_name);
		}
		break;

	      case SAR_PARM_TIME_OF_DAY:
		p_time_of_day = (sar_parm_time_of_day_struct *)p;
		if(loaded_scene)
		{
		    scene->tod = p_time_of_day->tod;
#if 0
/* The tod_code is updated in SARSimUpdateScene() */
		    scene->tod_code = SAR_TOD_CODE_UNDEFINED;
#endif
		}
		else
		{
		    fprintf(
			stderr,
 "%s: \"time_of_day\" may not be specified before \"mission_scene_file\".\n",
			filename
		    );
		}
		break;
	      case SAR_PARM_WIND:
		p_wind = (sar_parm_wind_struct *)p;
		if(loaded_scene)
		{
		    scene->realm->wind_vector.x = sin(p_wind->heading) * p_wind->speed;
		    scene->realm->wind_vector.y = cos(p_wind->heading) * p_wind->speed;
		    scene->realm->wind_vector.z = 0.0;
		    scene->realm->wind_flags = p_wind->flags;
		}
		else
		{
		    fprintf(
			stderr,
 "%s: \"wind\" may not be specified before \"mission_scene_file\".\n",
			filename
		    );
		}
		break;
	      case SAR_PARM_TEXTURE_BASE_DIRECTORY:
		p_texture_base_directory = (sar_parm_texture_base_directory_struct *)p;
		break;

	      case SAR_PARM_TEXTURE_LOAD:
		p_texture_load = (sar_parm_texture_load_struct *)p;
		SARObjLoadTexture(
		    core_ptr, scene, p_texture_load
		);
		break;

	      case SAR_PARM_MISSION_SCENE_FILE:
		p_mission_scene_file = (sar_parm_mission_scene_file_struct *)p;
		if(loaded_scene)
		{
		    fprintf(
			stderr,
"%s: Warning: Redefination of \"mission_scene_file\" ignored.\n",
			filename
		    );
		}
		else
		{
		    const char *scene_file = p_mission_scene_file->file;
		    char *full_path;

		    /* Complete path */
		    if(ISPATHABSOLUTE(scene_file))
		    {
			full_path = STRDUP(scene_file);
		    }
		    else
		    {
			struct stat stat_buf;
			full_path = STRDUP(PrefixPaths(
			    dname.local_data, scene_file
			));
			if((full_path != NULL) ? stat(full_path, &stat_buf) : True)
			{
			    free(full_path);
			    full_path = STRDUP(PrefixPaths(
				dname.global_data, scene_file
			    ));
			}
		    }
		    if(full_path == NULL)
			fprintf(
			    stderr,
"%s: Warning: Unable to complete path \"%s\".\n",
			    filename, scene_file
			);

		    /* Set new scene file on mission structure */
		    free(mission->scene_file);
		    mission->scene_file = STRDUP(full_path);
		    free(full_path);

		    /* Load scene */
		    status = SARSceneLoadFromFile(
			core_ptr, scene,
			mission->scene_file,
			weather_preset_name,
			client_data, progress_func
		    );
		    if(status)
		    {
			/* Failed to load scene */
			fprintf(
			    stderr,
 "%s: Error occured while loading scene \"%s\".\n",
			    filename, mission->scene_file
			);
			SARSceneDestroy(
			    core_ptr, scene,
			    ptr, total
			);
			SARMissionDelete(mission);
			mission = NULL;

			break;
		    }
		    else
		    {
			loaded_scene = True;
		    }
		}
		break;

	     case SAR_PARM_MISSION_NEW_OBJECTIVE:
		p_mission_new_objective = (sar_parm_mission_new_objective_struct *)p;
		/* Create a new objective */
		objective_num = mission->total_objectives;
		mission->total_objectives = objective_num + 1;
		mission->objective = (sar_mission_objective_struct *)realloc(
		    mission->objective,
		    mission->total_objectives * sizeof(sar_mission_objective_struct)
		);
		if(mission->objective == NULL)
		{
		    mission->total_objectives = 0;
		    objective_num = -1;
		    objective = NULL;
		}
		else
		{
		    objective = &mission->objective[objective_num];

		    /* Clear and then set default objective structure values */
		    memset(objective, 0x00, sizeof(sar_mission_objective_struct));
		    objective->type = p_mission_new_objective->objective_type;
		    objective->state = SAR_MISSION_OBJECTIVE_STATE_INCOMPLETE;
		    objective->time_left = 0.0f;
		    objective->humans_need_rescue = 0;
		    objective->total_humans = 0;
		    objective->arrive_at_name = NULL;
		    objective->message_success = NULL;
		    objective->message_fail = NULL;

		    if(opt->runtime_debug)
			printf(
"SARMissionLoadFromFile(): Creating new mission objective of type %i.\n",
			    p_mission_new_objective->objective_type
			);
		}
		break;

	      case SAR_PARM_MISSION_TIME_LEFT:
		p_mission_time_left = (sar_parm_mission_time_left_struct *)p;
		if(objective != NULL)
		{
		    objective->time_left = p_mission_time_left->time_left;
		}
		break;

	      case SAR_PARM_MISSION_BEGIN_AT:
		p_mission_begin_at = (sar_parm_mission_begin_at_struct *)p;
		if(p_mission_begin_at->name != NULL)
		{
		    sar_object_struct *begin_obj_ptr;

		    /* Update mission begin at name */
		    free(begin_at_name);
		    begin_at_name = STRDUP(p_mission_begin_at->name);

		    free(mission->start_location_name);
		    mission->start_location_name = STRDUP(p_mission_begin_at->name);

		    if(!begin_at_set)
		    {
			/* Match the object with the name specified by
			 * begin_at_name.
			 */
			begin_obj_ptr = SARObjMatchPointerByName(
			    scene, *ptr, *total,
			    begin_at_name, NULL
			);
			/* Got match? */
			if(begin_obj_ptr != NULL)
			{
			    /* Record begin at position and direction */
			    memcpy(
				&begin_pos, &begin_obj_ptr->pos,
				sizeof(sar_position_struct)
			    );
/* TODO Need to work on getting begin_dir based on location of matched
 * begin_at_name object
 */
			    memcpy(
				&begin_dir, &begin_obj_ptr->dir,
				sizeof(sar_direction_struct)
			    );
			}
			else
			{
			    fprintf(
				stderr,
"%s: Warning: \"mission_begin_at\" object name \"%s\" not found.\n",
				filename, begin_at_name
			    );
			}

			begin_at_set = True;
		    }
		}
		break;

	      case SAR_PARM_MISSION_BEGIN_AT_POS:
		p_mission_begin_at_pos = (sar_parm_mission_begin_at_pos_struct *)p;
		if(!begin_at_set)
		{
		    /* Record begin at position and direction */
		    memcpy(
			&begin_pos, &p_mission_begin_at_pos->pos,
			sizeof(sar_position_struct)
		    );
		    memcpy(
			&begin_dir, &p_mission_begin_at_pos->dir,
			sizeof(sar_direction_struct)
		    );
		    begin_at_set = True;
		}
		break;

	      case SAR_PARM_MISSION_ARRIVE_AT:
		p_mission_arrive_at = (sar_parm_mission_arrive_at_struct *)p;
		if(p_mission_arrive_at->name != NULL)
		{
		    sar_object_struct *arrive_obj_ptr;

		    /* Update arrive_at_name context */
		    free(arrive_at_name);
		    arrive_at_name = STRDUP(p_mission_arrive_at->name);

		    /* Update arrive_at_name on current objective (if any) */
		    if(objective != NULL)
		    {
			free(objective->arrive_at_name);
			objective->arrive_at_name = STRDUP(p_mission_arrive_at->name);
		    }

		    /* Match arrive at object (just to verify) */
		    arrive_obj_ptr = SARObjMatchPointerByName(
			scene, *ptr, *total,
			arrive_at_name, NULL
		    );
		    if(arrive_obj_ptr == NULL)
		    {
			fprintf(
			    stderr,
"%s: Warning: \"mission_arrive_at\" object name \"%s\" not found.\n",
			    filename, arrive_at_name
			);
		    }
		}
		break;

	      case SAR_PARM_MISSION_MESSAGE_SUCCESS:
		p_mission_message_success = (sar_parm_mission_message_success_struct *)p;
		if(p_mission_message_success->message != NULL)
		{
		    /* Update message_success on current objective (if any) */
		    if(objective != NULL)
		    {
			free(objective->message_success);
			objective->message_success = STRDUP(p_mission_message_success->message);
		    }
		}
		break;

	      case SAR_PARM_MISSION_MESSAGE_FAIL:
		p_mission_message_fail = (sar_parm_mission_message_fail_struct *)p;
		if(p_mission_message_fail->message != NULL)
		{
		    /* Update message_fail on current objective (if any) */
		    if(objective != NULL)
		    {
			free(objective->message_fail);
			objective->message_fail = STRDUP(p_mission_message_fail->message);
		    }
		}
		break;

	      case SAR_PARM_MISSION_HUMANS_TALLY:
		p_mission_humans_tally = (sar_parm_mission_humans_tally_struct *)p;
		if(objective != NULL)
		{
		    objective->humans_need_rescue = 
			p_mission_humans_tally->humans_need_rescue;
		    objective->total_humans =
			p_mission_humans_tally->total_humans;

		    /* Initial value humans_need_rescue cannot be
		     * greater than total_humans.
		     */
		    if(objective->humans_need_rescue > objective->total_humans)
			objective->humans_need_rescue = objective->total_humans;
		}
		break;

	      case SAR_PARM_MISSION_ADD_INTERCEPT:
		p_mission_add_intercept = (sar_parm_mission_add_intercept_struct *)p;
		if(obj_ptr != NULL)
		{
		    sar_intercept_struct intercept;
		    sar_object_struct *tar_obj_ptr;


		    /* Reset intercept structure */
		    memset(&intercept, 0x00, sizeof(sar_intercept_struct));

		    /* Handle by intercept reference code */
		    switch(p_mission_add_intercept->ref_code)
		    {
		      case 3:   /* Intercept arrive at location */   
			tar_obj_ptr = SARObjMatchPointerByName(
			    scene,
			    *ptr, *total,
			    arrive_at_name, NULL
			);
			if(tar_obj_ptr != NULL)
			{
			    intercept.x = tar_obj_ptr->pos.x;
			    intercept.y = tar_obj_ptr->pos.y;
			    intercept.z = tar_obj_ptr->pos.z;
			    intercept.radius = p_mission_add_intercept->radius;
			    intercept.urgency = p_mission_add_intercept->urgency;
			}
			break;

		      case 2:   /* Intercept begin at location */
			tar_obj_ptr = SARObjMatchPointerByName(
			    scene,
			    *ptr, *total,
			    begin_at_name, NULL
			);
			if(tar_obj_ptr != NULL)
			{
			    intercept.x = tar_obj_ptr->pos.x;
			    intercept.y = tar_obj_ptr->pos.y;
			    intercept.z = tar_obj_ptr->pos.z;
			    intercept.radius = p_mission_add_intercept->radius;
			    intercept.urgency = p_mission_add_intercept->urgency;
			}
			break;
		    
		      default:  /* Standard intercept */
			intercept.x = p_mission_add_intercept->pos.x;
			intercept.y = p_mission_add_intercept->pos.y;
			intercept.z = p_mission_add_intercept->pos.z;
			intercept.radius = p_mission_add_intercept->radius;
			intercept.urgency = p_mission_add_intercept->urgency;
			break;
		    }

		    /* Allocate new intercept structure on object */
		    if(obj_aircraft_ptr != NULL)
		    {
			SARObjInterceptNew(
			    scene,
			    &obj_aircraft_ptr->intercept,
			    &obj_aircraft_ptr->total_intercepts,
			    intercept.flags,
			    intercept.x, intercept.y, intercept.z,
			    intercept.radius, intercept.urgency,
			    intercept.name
			);
		    }
		}
		break;

	      case SAR_PARM_NEW_OBJECT:
		p_new_object = (sar_parm_new_object_struct *)p;
		/* Create new object of the specified type */
		obj_num = SARObjNew(
		    scene, ptr, total,
		    p_new_object->object_type
		);
		/* Update object context pointers */
		obj_ptr = (obj_num > -1) ? (*ptr)[obj_num] : NULL;
		CONTEXT_REGET_SUBSTRUCTURE_PTRS(obj_ptr)
		break;

	      case SAR_PARM_NEW_HELIPAD:
		p_new_helipad = (sar_parm_new_helipad_struct *)p;
		/* Create new helipad */
		obj_num = SARObjLoadHelipad(
		    core_ptr, scene, p_new_helipad
		);
		/* Update object context pointers */
		obj_ptr = (obj_num > -1) ? (*ptr)[obj_num] : NULL;
		CONTEXT_REGET_SUBSTRUCTURE_PTRS(obj_ptr)
		break;

	      case SAR_PARM_NEW_RUNWAY:
		p_new_runway = (sar_parm_new_runway_struct *)p;
		/* Create new runway */
		obj_num = SARObjLoadRunway(
		    core_ptr, scene, p_new_runway
		);
		/* Update object context pointers */
		obj_ptr = (obj_num > -1) ? (*ptr)[obj_num] : NULL;
		CONTEXT_REGET_SUBSTRUCTURE_PTRS(obj_ptr)
		break;

	      case SAR_PARM_NEW_HUMAN:
		p_new_human = (sar_parm_new_human_struct *)p;
		/* Create new human */
		obj_num = SARObjLoadHuman(
		    core_ptr, scene, p_new_human
		);
		/* Update object context pointers */
		obj_ptr = (obj_num > -1) ? (*ptr)[obj_num] : NULL;
		CONTEXT_REGET_SUBSTRUCTURE_PTRS(obj_ptr)
		if((obj_ptr != NULL) && (obj_human_ptr != NULL))
		{
		    /* Increment number of humans that need rescue that
		     * this mission file has specified.
		     */
		    if(obj_human_ptr->flags & SAR_HUMAN_FLAG_NEED_RESCUE)
			humans_need_rescue++;
		}
		break;

	      case SAR_PARM_NEW_FIRE:     
		p_new_fire = (sar_parm_new_fire_struct *)p;
		/* Create new fire object */
		obj_num = SARObjLoadFire(
		    core_ptr, scene, p_new_fire
		);
		/* Update object context pointers */
		obj_ptr = (obj_num > -1) ? (*ptr)[obj_num] : NULL;
		CONTEXT_REGET_SUBSTRUCTURE_PTRS(obj_ptr)
		break;

	      case SAR_PARM_NEW_SMOKE:
		p_new_smoke = (sar_parm_new_smoke_struct *)p;
		/* Create new smoke trails object */
		obj_num = SARObjLoadSmoke(
		    core_ptr, scene, p_new_smoke
		);
		/* Update object context pointers */
		obj_ptr = (obj_num > -1) ? (*ptr)[obj_num] : NULL;
		CONTEXT_REGET_SUBSTRUCTURE_PTRS(obj_ptr)
		break;

	      case SAR_PARM_NEW_PREMODELED:
		p_new_premodeled = (sar_parm_new_premodeled_struct *)p;
		/* Create new premodeled object */
		obj_num = SARObjPremodeledNew(
		    core_ptr, scene,
		    p_new_premodeled->model_type,
		    p_new_premodeled->argc,
		    p_new_premodeled->argv
		);
		/* Reget object context pointers */
		obj_ptr = (obj_num > -1) ? (*ptr)[obj_num] : NULL;
		CONTEXT_REGET_SUBSTRUCTURE_PTRS(obj_ptr)
		break;

	      case SAR_PARM_MODEL_FILE:
		p_model_file = (sar_parm_model_file_struct *)p;
		if((p_model_file->file != NULL) && (obj_ptr != NULL))
		    SARObjLoadFromFile(core_ptr, obj_num, p_model_file->file);
		break;

#if 0
	      case SAR_PARM_SELECT_OBJECT_BY_NAME:
		p_select_object_by_name = (sar_parm_select_object_by_name_struct *)p;
		if(p_select_object_by_name->name != NULL)
		{
		    obj_ptr = SARObjMatchPointerByName(
			scene,
			core_ptr->object, core_ptr->total_objects,
			p_select_object_by_name->name, &obj_num
		    );
		}
		break;
#endif
	      case SAR_PARM_TRANSLATE:
		p_translate = (sar_parm_translate_struct *)p;
	        SARObjLoadTranslate(
		    core_ptr, scene,
		    obj_ptr, p_translate
		);
		break;

	      case SAR_PARM_TRANSLATE_RANDOM:
		p_translate_random = (sar_parm_translate_random_struct *)p;
		SARObjLoadTranslateRandom(
		    core_ptr, scene,
		    obj_ptr, p_translate_random
		);
		break;

	      case SAR_PARM_ROTATE:
		p_rotate = (sar_parm_rotate_struct *)p;
		if(obj_ptr != NULL)
		    SARSimWarpObject(
			scene, obj_ptr,
			NULL, &p_rotate->rotate
		    );
		break;

	      case SAR_PARM_NO_DEPTH_TEST:
		p_no_depth_test = (sar_parm_no_depth_test_struct *)p;
		if(obj_ptr != NULL)
		{
		    obj_ptr->flags |= SAR_OBJ_FLAG_NO_DEPTH_TEST;
		}
		break;

	      case SAR_PARM_POLYGON_OFFSET:
		p_polygon_offset = (sar_parm_polygon_offset_struct *)p;
		if(obj_ptr != NULL)
		{
		    obj_ptr->flags |= p_polygon_offset->flags;
		}
		break;

	      case SAR_PARM_OBJECT_NAME:
		p_object_name = (sar_parm_object_name_struct *)p;
		if(obj_ptr != NULL)
		{
		    free(obj_ptr->name);
		    obj_ptr->name = STRDUP(p_object_name->name);
		}
		break;

	      case SAR_PARM_RUNWAY_APPROACH_LIGHTING_NORTH:
		p_runway_applight_n =
		    (sar_parm_runway_approach_lighting_north_struct *)p;
		if((obj_ptr != NULL) && (obj_runway_ptr != NULL))
		{
		    obj_runway_ptr->north_approach_lighting_flags =
			p_runway_applight_n->flags;
		}
		break;

	      case SAR_PARM_RUNWAY_APPROACH_LIGHTING_SOUTH:
		p_runway_applight_s =
		    (sar_parm_runway_approach_lighting_south_struct *)p;
		if((obj_ptr != NULL) && (obj_runway_ptr != NULL))
		{
		    obj_runway_ptr->south_approach_lighting_flags =
			p_runway_applight_s->flags;
		}
		break;

	      case SAR_PARM_HUMAN_MESSAGE_ENTER:
		p_human_message_enter = (sar_parm_human_message_enter_struct *)p;
		if((obj_ptr != NULL) && (obj_human_ptr != NULL))
		{
		    free(obj_human_ptr->mesg_enter);
		    obj_human_ptr->mesg_enter = STRDUP(
			p_human_message_enter->message
		    );
		}
		break;

	      case SAR_PARM_HUMAN_REFERENCE:
		p_human_reference = (sar_parm_human_reference_struct *)p;
		if((obj_ptr != NULL) && (obj_human_ptr != NULL) &&
		   (p_human_reference->reference_name != NULL)
		)
		{
		    int human_ref_obj_num = -1;
		    const char *ref_name = p_human_reference->reference_name;

		    /* Run towards? */
		    if(p_human_reference->flags & SAR_HUMAN_FLAG_RUN_TOWARDS)
		    {
			obj_human_ptr->flags |= SAR_HUMAN_FLAG_RUN_TOWARDS;
		    }
		    /* Run away? */
		    else if(p_human_reference->flags & SAR_HUMAN_FLAG_RUN_AWAY)
		    {
			obj_human_ptr->flags |= SAR_HUMAN_FLAG_RUN_AWAY;
		    }

		    /* Handle reference object name */
		    if(!strcasecmp(ref_name, "player"))
		    {
			/* Set special intercept code to intercept the player */
			obj_human_ptr->intercepting_object = -2;
		    }
		    else
		    {
			/* All else match by object name */
			SARObjMatchPointerByName(
			    scene, *ptr, *total,
			    ref_name, &human_ref_obj_num
			);
			obj_human_ptr->intercepting_object = human_ref_obj_num;
		    }
		}
		break;

	      default:
		break;

	    }	/* Handle by parm type */


	    /* If we lost the mission structure then we need to
	     * give up, there was probably some fatal error
	     * encountered above
	     */
	    if(mission == NULL)
		break;

	}	/* Iterate through loaded parms */


	/* Delete loaded parms */
	SARParmDeleteAll(&parm, &total_parms);

	if(opt->runtime_debug)
	    printf(
"SARMissionLoadFromFile():\
 Loaded new mission with %i objectives.\n",
		mission->total_objectives
	    );

	free(begin_at_name);
	free(arrive_at_name);
	free(weather_preset_name);

#undef CONTEXT_REGET_SUBSTRUCTURE_PTRS

	return(mission);
}

/*
 *	Resets the mission log file specified by filename and rewrites
 *	its header with the information given by the mission structure.
 *
 *	This will overwrite the previous log file (if any) specified
 *	by filename.
 */
void SARMissionLogReset(
	sar_core_struct *core_ptr, sar_mission_struct *mission,
	const char *filename            /* Mission log file name */
)
{
	void *p, **parm = NULL;
	int total_parms = 0;

	if((mission == NULL) || STRISEMPTY(filename))
	    return;

	/* Begin generating parameters for writing mission log */
	p = SARParmNewAppend(
	    SAR_PARM_MISSION_LOG_HEADER,
	    &parm, &total_parms
	);
	if(p != NULL)
	{
	    sar_parm_mission_log_header_struct *pv =
		(sar_parm_mission_log_header_struct *)p;
	    pv->version_major = PROG_VERSION_MAJOR;
	    pv->version_minor = PROG_VERSION_MINOR;
	    pv->version_release = PROG_VERSION_RELEASE;
	    free(pv->title);
	    pv->title = STRDUP(mission->title);
	    free(pv->scene_file);
	    pv->scene_file = STRDUP(mission->scene_file);
	    free(pv->player_stats_file);
	    pv->player_stats_file = STRDUP(mission->player_stats_file);
	}

	/* Write mission log file, this will overwrite the existing
	 * mission log file (if any).
	 */
	SARParmSaveToFile(
	    filename, SAR_FILE_FORMAT_MISSION_LOG,
	    parm, total_parms,
	    NULL, NULL
	);

	/* Delete loaded parms */
	SARParmDeleteAll(&parm, &total_parms);
}

/*
 *	Appends a log entry to the given log file, the log file will be
 *	opened for writing (append) and then closed during this call.
 *
 *	The event_type must be given and cannot be -1.
 *
 *	The tod (time of day) can be either a valid time or -1.0 to indicate
 *	use current time on core's scene structure.
 *
 *	The given file name must reffer to valid file which is writeable.
 *
 *	All other inputs are optional.
 */
void SARMissionLogEvent(
	sar_core_struct *core_ptr, sar_mission_struct *mission,
	int event_type,                 /* One of SAR_LOG_EVENT_* */
	float tod,			/* Time of day in seconds since midnight */
	sar_position_struct *pos,       /* Position of event */
	const float *value,		/* Additional values */
	int total_values,               /* Total number of additional values */
	const char *message,		/* The message */
	const char *filename            /* Mission log file name */
)
{
	int i;
	FILE *fp;
	sar_scene_struct *scene = core_ptr->scene;
	if((scene == NULL) || (mission == NULL) ||
	   STRISEMPTY(filename)
	)
	    return;

	/* Open mission log file for append writing */
	fp = FOpen(filename, "ab");
	if(fp == NULL)
	    return;

	/* Write event type, time and coordinates */
	fprintf(
	    fp,
	    "%i %f %f %f %f%s",
	    event_type,
	    (tod < 0.0f) ? scene->tod : tod,
	    (pos != NULL) ? pos->x : 0.0f,
	    (pos != NULL) ? pos->y : 0.0f,
	    (pos != NULL) ? pos->z : 0.0f,
	    (total_values > 0) ? " " : ""
	);

	/* Write any additional arguments */
	for(i = 0; i < total_values; i++)
	    fprintf(
		fp,
		"%f%s",
		value[i],
		(i < (total_values - 1)) ? " " : ""
	    );

	/* Write values and message deliminator character */
	fputc(':', fp);

	/* Write message? */
	if(message != NULL)
	    fputs(message, fp);

	/* End this log event line with a newline character */
	fputc('\n', fp);

	/* Close mission log file */
	FClose(fp);
}
