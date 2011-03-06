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

#include "v3dmp.h"
#include "v3dmodel.h"

#ifdef MEMWATCH
# include "memwatch.h"
#endif


v3d_model_struct *V3DModelCreate(int type, const char *name);
v3d_model_struct *V3DModelDup(const v3d_model_struct *m);
void V3DModelDestroy(v3d_model_struct *m);

v3d_model_struct *V3DModelListGetPtr(
	v3d_model_struct **list, int total, int i
);
v3d_model_struct *V3DModelListInsert(
	v3d_model_struct ***list, int *total, int i,
	int type, const char *name
);
void V3DModelListDelete(v3d_model_struct ***list, int *total, int i);
void V3DModelListDeleteAll(v3d_model_struct ***list, int *total);

int V3DModelGetType(v3d_model_struct *m);


#define ATOI(s)         (((s) != NULL) ? atoi(s) : 0)
#define ATOL(s)         (((s) != NULL) ? atol(s) : 0)
#define ATOF(s)         (((s) != NULL) ? atof(s) : 0.0f)
#define STRDUP(s)       (((s) != NULL) ? strdup(s) : NULL)

#define MAX(a,b)        (((a) > (b)) ? (a) : (b))
#define MIN(a,b)        (((a) < (b)) ? (a) : (b))
#define CLIP(a,l,h)     (MIN(MAX((a),(l)),(h)))


/*
 *	Allocates a new model structure, if successful the
 *	given type and name will be set.
 *
 *	Can return NULL on error.
 */
v3d_model_struct *V3DModelCreate(int type, const char *name)
{
	v3d_model_struct *m = (v3d_model_struct *)calloc(
	    1,
	    sizeof(v3d_model_struct)
	);
	if(m == NULL)
	    return(NULL);

	m->type = type;
	m->name = STRDUP(name);

	m->primitive = NULL;
	m->total_primitives = 0;

	m->other_data_line = NULL;
	m->total_other_data_lines = 0;

	return(m);
}

/*
 *      Returns a dynamically allocated duplicate of the given
 *      model and duplicate of all of its substructure data.
 *
 *	The primitives in the coppied model will not be `realized',
 *      that is left to the calling function. See notes on function
 *	V3DMPDup() for more details on what is coppied on the primitives
 *	and what are not.
 *
 *      Can return NULL on error.
 */
v3d_model_struct *V3DModelDup(const v3d_model_struct *m)
{
	int i;
	v3d_model_struct *nm = NULL;
	const void *p;


	if(m == NULL)
	    return(nm);

	nm = (v3d_model_struct *)calloc(1, sizeof(v3d_model_struct));
	if(nm == NULL)
	    return(nm);

	/* Begin duplicating data. */
	nm->type = m->type;
	nm->flags = m->flags;

	/* Copy name as needed. */
	if(m->name != NULL)
	    nm->name = STRDUP(m->name);

	/* Duplicate all primitives (but note they will not be
	 * `realized'), that is up to the calling function.
	 */
	nm->total_primitives = m->total_primitives;
	if(m->total_primitives > 0)
	    nm->primitive = (void **)calloc(
		m->total_primitives, sizeof(void *)
	    );
	else
	    nm->primitive = NULL;
	if(nm->primitive == NULL)
	    nm->total_primitives = 0;
	for(i = 0; i < nm->total_primitives; i++)
	{
	    p = m->primitive[i];
	    if(p == NULL)
		nm->primitive[i] = NULL;
	    else
		nm->primitive[i] = V3DMPDup(p);
	}

	/* Duplicate all other data lines. */
	nm->total_other_data_lines = m->total_other_data_lines;
	if(m->total_other_data_lines > 0)
	    nm->other_data_line = (char **)calloc(
		m->total_other_data_lines, sizeof(char *)
	    );
	else
	    nm->other_data_line = NULL;
	if(nm->other_data_line == NULL)
	    nm->total_other_data_lines = 0;
	for(i = 0; i < nm->total_other_data_lines; i++)
	    nm->other_data_line[i] = STRDUP(m->other_data_line[i]);

	return(nm);
}


/*
 *	Deallocates the model structure and all of its allocated
 *	resources (including its primitives).
 *
 *	Note that the primitives on the model will not be unrealized,
 *	it is up to the calling function to do that.
 */
void V3DModelDestroy(v3d_model_struct *m)
{
	int i;


	if(m == NULL)
	    return;

	/* Deallocate model name. */
	free(m->name);
	m->name = NULL;

	/* Delete all primitives. */
	V3DMPListDeleteAll(&m->primitive, &m->total_primitives);

	/* Deallocate all other data lines. */
	for(i = 0; i < m->total_other_data_lines; i++)
	    free(m->other_data_line[i]);
	free(m->other_data_line);
	m->other_data_line = NULL;
	m->total_other_data_lines = 0;

	/* Deallocate structure itself. */
	free(m);

	return;
}


/*
 *	Checks if model i exists in the list and returns its pointer
 *	if it exists or NULL if it does not.
 */
v3d_model_struct *V3DModelListGetPtr(v3d_model_struct **list, int total, int i)
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
 *      Inserts a new model item at list position i and returns the
 *	pointer to the model or NULL on failure.
 *
 *      If i is negative then the item will be appended to the list.
 *
 *      If i is greater or equal to (*total) + 1 then i will
 *      be set to the value of (*total).
 *
 *      Both list and total need to be non NULL.
 */
v3d_model_struct *V3DModelListInsert(
	v3d_model_struct ***list, int *total, int i,
	int type, const char *name
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
	(*list) = (v3d_model_struct **)realloc(
	    *list,
	    (*total) * sizeof(v3d_model_struct *)
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
 
	    (*list)[i] = V3DModelCreate(type, name);
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

	    (*list)[i] = V3DModelCreate(type, name);
	}

	return((*list)[i]);
}

/*
 *      Deletes the item i from the list.  Item pointer i on the list
 *      will then be set to NULL.
 */
void V3DModelListDelete(v3d_model_struct ***list, int *total, int i)
{
	v3d_model_struct *m;

	if((list == NULL) ||
	   (total == NULL)
	)
	    return;

	/* Item exists and is allocated? */
	m = V3DModelListGetPtr(*list, *total, i);
	if(m != NULL)
	{
	    V3DModelDestroy(m);
	    (*list)[i] = NULL;	/* Reset item pointer to NULL. */
	}

	return;
}

/*
 *      Deletes the entire list and resets the pointers list
 *      and total to NULL.
 */
void V3DModelListDeleteAll(v3d_model_struct ***list, int *total)
{
	int i;

	if((list == NULL) ||
	   (total == NULL)
	)
	    return;

	for(i = 0; i < (*total); i++)
	    V3DModelListDelete(list, total, i);

	free(*list);
	*list = NULL;

	*total = 0;

	return;
}


/*
 *	Returns the model type.
 *
 *	Can return -1 on error.
 */
int V3DModelGetType(v3d_model_struct *m)
{
	if(m == NULL)
	    return(-1);
	else
	    return(m->type);
}

