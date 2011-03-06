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
#ifdef __MSW__
# include <windows.h>
#endif
#include <GL/gl.h>
#include "obj.h"
#include "sar.h"
#include "sardraw.h"
#include "sardrawselect.h"
#include "config.h"


void SARDrawMapProcessHits(
	sar_core_struct *core_ptr, GLint hits, GLuint buffer[]
);

int SARDrawMapObjNameListAppend(
	sar_drawmap_objname_struct ***ptr, int *total,
	GLuint gl_name, const char *obj_name
);
void SARDrawMapObjNameListDelete(
	sar_drawmap_objname_struct ***ptr, int *total
);

int *SARGetGCCHitList(
	sar_core_struct *core_ptr, sar_scene_struct *scene,
	sar_object_struct ***ptr, int *total,
	int obj_num,
	int *hits
);
int SARGetGHCOverWater(
	sar_core_struct *core_ptr, sar_scene_struct *scene,
	sar_object_struct ***ptr, int *total,
	int obj_num, Boolean *got_hit, Boolean *over_water
);


#define ATOI(s)         (((s) != NULL) ? atoi(s) : 0)
#define ATOL(s)         (((s) != NULL) ? atol(s) : 0)
#define ATOF(s)         (((s) != NULL) ? atof(s) : 0.0f)
#define STRDUP(s)       (((s) != NULL) ? strdup(s) : NULL)

#define MAX(a,b)	(((a) > (b)) ? (a) : (b))
#define MIN(a,b)	(((a) < (b)) ? (a) : (b))
#define CLIP(a,l,h)	(MIN(MAX((a),(l)),(h)))
#define STRISEMPTY(s)	(((s) != NULL) ? (*(s) == '\0') : TRUE)

#define RADTODEG(r)	((r) * 180 / PI)
#define DEGTORAD(d)	((d) * PI / 180)


/*
 *	Called by SARDrawMap() to process the hits after drawing the
 *	map in GL_SELECT render mode.
 *
 *	Hits will be recorded on specified core's draw map object names
 *	list.
 *
 *	Inputs assumed valid.
 */
void SARDrawMapProcessHits(
	sar_core_struct *core_ptr, GLint hits, GLuint buffer[]
)
{
	int i, j;
	GLuint gl_name, names, *ptr = buffer;

	/* Iterate through each hit */
	for(i = 0; i < hits; i++)
	{
	    names = *ptr;	/* Number of names on this hit */
	    ptr += 3;		/* Seek to start of names on this hit */

	    /* Iterate through each name on this hit */
	    for(j = 0; j < (int)names; j++)
	    {
		/* Get the GL name which is the object's index number */
		gl_name = *ptr++;

		/* Append this object to the names list */
		SARDrawMapObjNameListAppend(
		    &core_ptr->drawmap_objname,
		    &core_ptr->total_drawmap_objnames,
		    gl_name,		/* Object's index */
		    NULL		/* No object names for now */
		);
	    }
	}
}

/*
 *	Appends an Object Name to the Object Names List.
 *
 *	Returns the index of the new Object Name or negative on error.
 */
int SARDrawMapObjNameListAppend(
	sar_drawmap_objname_struct ***ptr, int *total,
	GLuint gl_name, const char *obj_name
)
{
	int n;
	sar_drawmap_objname_struct *name;

	if((ptr == NULL) || (total == NULL))
	    return(-2);

	/* Increase allocation of the Object Names List */
	n = MAX(*total, 0);
	*total = n + 1;
	*ptr = (sar_drawmap_objname_struct **)realloc(
	    *ptr,
	    (*total) * sizeof(sar_drawmap_objname_struct *)
	);
	if(*ptr == NULL)
	{
	    *total = 0;
	    return(-3);
	}
	/* Allocate a new Object Name */
	(*ptr)[n] = name = (sar_drawmap_objname_struct *)calloc(
	    1, sizeof(sar_drawmap_objname_struct)
	);
	if(name == NULL)
	    return(-3);

	name->gl_name = gl_name;		/* Object's index */
	name->obj_name = STRDUP(obj_name);

	return(n);
}

/*
 *	Deletes the Object Names List.
 */
void SARDrawMapObjNameListDelete(
	sar_drawmap_objname_struct ***ptr, int *total
)
{
	int i;
	sar_drawmap_objname_struct *name;

	if((ptr == NULL) || (total == NULL))
	    return;

	for(i = 0; i < *total; i++)
	{
	    name = (*ptr)[i];
	    if(name == NULL)
		continue;

	    free(name->obj_name);
	    free(name);
	}
	free(*ptr);
	*ptr = NULL;
	*total = 0;
}

/*
 *	Draws the map in GL_SELECT render mode and processes the hits,
 *	storing the hits on the specified core's Object Names List.
 *
 *	Note: Special numbers have the base offset of (*total) added
 *	to them. So any index that is a special number is:
 *
 *		special_index = i - (*total).
 *
 *	The specified obj_num (or -1 for the player object) will be
 *	excluded from the returned index list.
 *
 *      Returns a list of object indices that were hit, the calling
 *	function must delete the returned list.
 */
int *SARGetGCCHitList(
	sar_core_struct *core_ptr, sar_scene_struct *scene,
	sar_object_struct ***ptr, int *total,
	int obj_num,
	int *hits
)
{
	int i, n, hit_obj_num, *list = NULL;
	sar_drawmap_objname_struct *name;

	if(hits != NULL)
	    *hits = 0;

	if((core_ptr == NULL) || (scene == NULL) || (ptr == NULL) ||
	   (total == NULL)
	)
	    return(list);

	/* Draw the map with draw_for_gcc set to True, this will draw
	 * the map with an isolated camera angle looking directly
	 * downward with respect to the specified object (or -1 for the
	 * player object)
	 *
	 * The core's Object Names List will be cleared and updated
	 * with the hits list
	 */
	SARDrawMap(core_ptr, True, False, obj_num);

	/* Get hits list */
	for(i = 0; i < core_ptr->total_drawmap_objnames; i++)
	{
	    name = core_ptr->drawmap_objname[i];
	    if(name == NULL)
		continue;

	    /* Get the hit object number as the gl name
	     *
	     * Note that the hit object number may be >= to
	     * total_objects in which case it has special meaning
	     */
	    hit_obj_num = (int)name->gl_name;

	    /* Skip if the hit object number is the same as given
	     * object number
	     */
	    if(hit_obj_num == obj_num)
		continue;

	    /* Allocate more hits and append this hit object number to
	     * the list
	     */
	    if(hits != NULL)
	    {
		n = MAX(*hits, 0);
		*hits = n + 1;

		list = (int *)realloc(
		    list,
		    (*hits) * sizeof(int)
		);
		if(list == NULL)
		{
		    *hits = 0;
		    break;
		}
		else
		{
		    list[n] = hit_obj_num;
		}
	    }
	}

	return(list);
}

/*
 *      Checks for ground hit contact, relative to the given obj_num.
 *
 *      Checks if there is any object directly under obj_num that is
 *      drawn. If nothing is drawn directly under the object then
 *      got_hit will be set to False, otherwise True.
 *
 *      If the scene's base type is water, then over_water will be set
 *      True if (and only if) got_hit is False.
 *
 *      Returns 0 on success or non-zero on error.
 */
int SARGetGHCOverWater(
	sar_core_struct *core_ptr, sar_scene_struct *scene,
	sar_object_struct ***ptr, int *total,
	int obj_num, Boolean *got_hit, Boolean *over_water
)
{
	if(got_hit != NULL)
	    *got_hit = False;
	if(over_water != NULL)
	    *over_water = False;

	if((core_ptr == NULL) || (scene == NULL) || (ptr == NULL) ||
	   (total == NULL)
	)
	    return(-1);

	/* Draw the map with draw_for_ghc set to True, this will draw
	 * the map with an isolated camera angle looking directly
	 * downward with respect to the specified object (or -1 for the
	 * player object)
	 *
	 * The core's drawmap_ghc_result will be updated
	 */
	SARDrawMap(core_ptr, False, True, obj_num);

	/* Got hit? */
	if(core_ptr->drawmap_ghc_result >= 0.5f)
	{
	    /* Mark that we got a hit */
	    if(got_hit != NULL)
		*got_hit = True;

	    /* Mark that the specified object is not over water */
	    if(over_water != NULL)
		*over_water = False;
	}
	else
	{
	    /* Mark that we did not get a hit */
	    if(got_hit != NULL)
		*got_hit = False;

	    /* Scene's base is water? */
	    if(scene->base_flags & SAR_SCENE_BASE_FLAG_IS_WATER)
	    {
		if(over_water != NULL)
		    *over_water = True;
	    }
	    else
	    {
		if(over_water != NULL)
		    *over_water = False;
	    }
	}

	return(0);
}
