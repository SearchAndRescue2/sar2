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

#include "v3dmh.h"

#ifdef MEMWATCH
# include "memwatch.h"
#endif


void *V3DMHCreate(int type);
void V3DMHDestroy(void *p);

void *V3DMHListGetPtr(void **list, int total, int i);
void *V3DMHListInsert(
	void ***list, int *total, int i,
	int type
);
void V3DMHListDelete(void ***list, int *total, int i);
void V3DMHListDeleteAll(void ***list, int *total);

int V3DMHTextureBaseDirectorySet(
	void ***list, int *total, const char *path
);
char *V3DMHTextureBaseDirectoryGet(void **list, int total);

int V3DMHHeightfieldBaseDirectorySet(
	void ***list, int *total, const char *path
);
char *V3DMHHeightfieldBaseDirectoryGet(void **list, int total);


#define ATOI(s)         (((s) != NULL) ? atoi(s) : 0)
#define ATOL(s)         (((s) != NULL) ? atol(s) : 0)
#define ATOF(s)         (((s) != NULL) ? atof(s) : 0.0f)
#define STRDUP(s)       (((s) != NULL) ? strdup(s) : NULL)

#define MAX(a,b)        (((a) > (b)) ? (a) : (b))
#define MIN(a,b)        (((a) < (b)) ? (a) : (b))
#define CLIP(a,l,h)     (MIN(MAX((a),(l)),(h)))


/*
 *	Allocates a new model header item structure of the specified
 *	type and sets its type.
 *
 *	Can return NULL on error.
 */
void *V3DMHCreate(int type)
{
	int size = 0;
	void *p = NULL;

	switch(type)
	{
	  case V3DMH_TYPE_COMMENT:
	    size = sizeof(mh_comment_struct);
	    break;


	  case V3DMH_TYPE_VERSION:
	    size = sizeof(mh_version_struct);
	    break;

	  case V3DMH_TYPE_CREATOR:
	    size = sizeof(mh_creator_struct);
	    break;

	  case V3DMH_TYPE_AUTHOR:
	    size = sizeof(mh_author_struct);
	    break;


	  case V3DMH_TYPE_HEIGHTFIELD_BASE_DIRECTORY:
	    size = sizeof(mh_heightfield_base_directory_struct);
	    break;

	  case V3DMH_TYPE_TEXTURE_BASE_DIRECTORY:
	    size = sizeof(mh_texture_base_directory_struct);
	    break;

	  case V3DMH_TYPE_TEXTURE_LOAD:
	    size = sizeof(mh_texture_load_struct);
	    break;

	  case V3DMH_TYPE_COLOR_SPECIFICATION:
	    size = sizeof(mh_color_specification_struct);
	    break;

	  default:
	    fprintf(
		stderr,
		"V3DMHCreate(): Unsupported primitive type %i\n",
		type
	    );
	}

	if(size > 0)
	{
	    p = (void *)calloc(1, size);
	    (*(int *)p) = type;
	}

	return(p);
}

/*
 *	Deallocates the model header item structure p and any allocated
 *	sub resources.
 */
void V3DMHDestroy(void *p)
{
	int i, type;
	mh_comment_struct *comment;
	mh_creator_struct *creator;
	mh_author_struct *author;
	mh_heightfield_base_directory_struct *heightfield_base_directory;
	mh_texture_base_directory_struct *texture_base_directory;
	mh_texture_load_struct *texture_load;
	mh_color_specification_struct *color_specification;


	if(p == NULL)
	    return;
	else
	    type = (*(int *)p);

	/* Deallocate any substructures. */
	switch(type)
	{
	  case V3DMH_TYPE_COMMENT:
	    comment = p;
	    for(i = 0; i < comment->total_lines; i++)
		free(comment->line[i]);
	    free(comment->line);
	    break;

	  case V3DMH_TYPE_CREATOR:
	    creator = p;
	    free(creator->creator);
	    break;

	  case V3DMH_TYPE_AUTHOR:
	    author = p;
	    free(author->author);
	    break;


	  case V3DMH_TYPE_HEIGHTFIELD_BASE_DIRECTORY:
	    heightfield_base_directory = p;
	    free(heightfield_base_directory->path);
	    break;

	  case V3DMH_TYPE_TEXTURE_BASE_DIRECTORY:
	    texture_base_directory = p;
	    free(texture_base_directory->path);
	    break;

	  case V3DMH_TYPE_TEXTURE_LOAD:
	    texture_load = p;
	    free(texture_load->name);
	    free(texture_load->path);
	    break;

	  case V3DMH_TYPE_COLOR_SPECIFICATION:
	    color_specification = p;
	    free(color_specification->name);
	    break;
	}

	free(p);

	return;
}


/*
 *	Checks if item i exists in the list and returns its pointer
 *	if it exists or NULL if it does not.
 */
void *V3DMHListGetPtr(void **list, int total, int i)
{
	if((list == NULL) ||
	   (i < 0) ||
	   (i >= total)
	)
	    return(NULL);
	else
	    return(list[i]);
}

/*
 *	Inserts a new model header item at list position i and
 *	returns the pointer to the model primitive or NULL on failure.
 *
 *	If i is negative then the item will be appended to the list.
 *
 *	If i is greater or equal to (*total) + 1 then i will
 *	be set to the value of (*total).
 *
 *	Both list and total need to be non NULL.
 */
void *V3DMHListInsert(
	void ***list, int *total, int i,
	int type
)
{
	if((list == NULL) ||
	   (total == NULL)
	)
	    return(NULL);

	if((*total) < 0)
	    (*total) = 0;

	/* Allocate more pointers for the list. */
	(*total) = (*total) + 1;
	(*list) = (void **)realloc(
	    *list,
	    (*total) * sizeof(void *)
	);
	if((*list) == NULL)
	{
	    (*total) = 0;
	    return(NULL);
	}

	/* Append or insert? */
	if(i < 0)
	{
	    /* Append. */

	    i = (*total) - 1;

	    (*list)[i] = V3DMHCreate(type);
	}
	else
	{
	    /* Insert. */
	    int n;

	    if(i >= (*total))
		i = (*total) - 1;

	    /* Shift pointers. */
	    for(n = (*total) - 1; n > i; n--)
		(*list)[n] = (*list)[n - 1];

	    (*list)[i] = V3DMHCreate(type);
	}

	return((*list)[i]);
}

/*
 *	Deletes the item i from the list.  Item pointer i on the list
 *	will then be set to NULL.
 */
void V3DMHListDelete(void ***list, int *total, int i)
{
	void *p;

	if((list == NULL) ||
	   (total == NULL)
	)
	    return;

	/* Item exists and is allocated? */
	p = V3DMHListGetPtr(*list, *total, i);
	if(p != NULL)
	{
	    V3DMHDestroy(p);
	    (*list)[i] = NULL;	/* Reset item pointer to NULL. */
	}

	return;
}

/*
 *	Deletes the entire list and resets the pointers list
 *	and total to NULL.
 */
void V3DMHListDeleteAll(void ***list, int *total)
{
	int i;

	if((list == NULL) ||
	   (total == NULL)
	)
	    return;

	for(i = 0; i < (*total); i++)
	    V3DMHListDelete(list, total, i);

	free(*list);
	*list = NULL;

	*total = 0;

	return;
}

/*
 *	Sets the texture base directory in the given list of model header
 *	items.
 *
 *	A new item will be prepended if no existing 
 *	mh_texture_base_directory_struct is found in the list.
 *
 *	Returns the newly added item index or -1 on failure.
 */
int V3DMHTextureBaseDirectorySet(
	void ***list, int *total, const char *path
)
{
	int i, n;
	void *h;
	mh_texture_base_directory_struct *h_texture_base_dir;

	if((list == NULL) || (total == NULL))
	    return(-1);

	/* Check for existing item and set it if found. */
	for(i = 0, n = -1; i < (*total); i++)
	{
	    h = (*list)[i];
	    if(h == NULL)
		continue;

	    if((*(int *)h) == V3DMH_TYPE_TEXTURE_BASE_DIRECTORY)
	    {
		h_texture_base_dir = (mh_texture_base_directory_struct *)h;

		free(h_texture_base_dir->path);
		h_texture_base_dir->path = STRDUP(path);

		n = i;
	    }
	}
	/* Set any above? */
	if(n > -1)
	    return(n);

	/* Allocate more pointers and shift. */
	(*total) = (*total) + 1;
	(*list) = (void **)realloc(
	    *list, (*total) * sizeof(void *)
	);
	if((*list) == NULL)
	{
	    (*total) = 0;
	    return(-1);
	}

	for(n = (*total) - 1; n > 0; n--)
	    (*list)[n] = (*list)[n - 1];

	n = 0;

	/* Prepend a new item. */
	h_texture_base_dir = (mh_texture_base_directory_struct *)V3DMHCreate(
	    V3DMH_TYPE_TEXTURE_BASE_DIRECTORY
	);
	if(h_texture_base_dir == NULL)
	    return(-1);

	/* Set new path. */
	free(h_texture_base_dir->path);
	h_texture_base_dir->path = STRDUP(path);

	(*list)[n] = h_texture_base_dir;

	return(n);
}

/*
 *	Returns the texture base directory if specified as the first
 *	mh_texture_base_directory_struct structure found in the
 *	given list.
 *
 *	Can return NULL on error, the returned pointer should not be
 *	deallocated as it comes from the header item.
 */
char *V3DMHTextureBaseDirectoryGet(void **list, int total)
{
	int i;
	void *h;
	mh_texture_base_directory_struct *h_texture_base_dir;

	if(list == NULL)
	    return(NULL);

	/* Check for existing item. */
	for(i = 0; i < total; i++)
	{
	    h = list[i];
	    if(h == NULL)
		continue;

	    if((*(int *)h) == V3DMH_TYPE_TEXTURE_BASE_DIRECTORY)
	    {
		h_texture_base_dir = (mh_texture_base_directory_struct *)h;

		return(h_texture_base_dir->path);
	    }
	}

	return(NULL);
}

/*
 *      Sets the heightfield base directory in the given list of model
 *	header items.
 *
 *      A new item will be prepended if no existing
 *      mh_heightfield_base_directory_struct is found in the list.
 *
 *      Returns the newly added item index or -1 on failure.
 */
int V3DMHHeightfieldBaseDirectorySet(
	void ***list, int *total, const char *path
)
{
	int i, n;
	void *h;
	mh_heightfield_base_directory_struct *h_heightfield_base_dir;

	if((list == NULL) || (total == NULL))
	    return(-1);

	/* Check for existing item and set it if found. */
	for(i = 0, n = -1; i < (*total); i++)
	{
	    h = (*list)[i];
	    if(h == NULL)
		continue;

	    if((*(int *)h) == V3DMH_TYPE_HEIGHTFIELD_BASE_DIRECTORY)
	    {
		h_heightfield_base_dir = (mh_heightfield_base_directory_struct *)h;

		free(h_heightfield_base_dir->path);
		h_heightfield_base_dir->path = STRDUP(path);

		n = i;
	    }
	}
	/* Set any above? */
	if(n > -1)
	    return(n);

	/* Allocate more pointers and shift. */
	(*total) = (*total) + 1;
	(*list) = (void **)realloc(
	    *list, (*total) * sizeof(void *)
	);
	if((*list) == NULL)
	{
	    (*total) = 0;
	    return(-1);
	}

	for(n = (*total) - 1; n > 0; n--)
	    (*list)[n] = (*list)[n - 1];

	n = 0;

	/* Prepend a new item. */
	h_heightfield_base_dir = (mh_heightfield_base_directory_struct *)V3DMHCreate(
	    V3DMH_TYPE_HEIGHTFIELD_BASE_DIRECTORY
	);
	if(h_heightfield_base_dir == NULL)
	    return(-1);

	/* Set new path. */
	free(h_heightfield_base_dir->path);
	h_heightfield_base_dir->path = STRDUP(path);

	(*list)[n] = h_heightfield_base_dir;

	return(n);
}       

/*
 *      Returns the heightfield base directory if specified as the first
 *      mh_heightfield_base_directory_struct structure found in the
 *      given list.
 *
 *      Can return NULL on error, the returned pointer should not be
 *      deallocated as it comes from the header item.
 */
char *V3DMHHeightfieldBaseDirectoryGet(void **list, int total)
{
	int i;
	void *h;
	mh_heightfield_base_directory_struct *h_heightfield_base_dir;

	if(list == NULL)
	    return(NULL);

	/* Check for existing item. */
	for(i = 0; i < total; i++)
	{
	    h = list[i];
	    if(h == NULL)
		continue;

	    if((*(int *)h) == V3DMH_TYPE_HEIGHTFIELD_BASE_DIRECTORY)
	    {
		h_heightfield_base_dir = (mh_heightfield_base_directory_struct *)h;

		return(h_heightfield_base_dir->path);
	    }
	}

	return(NULL);
}

