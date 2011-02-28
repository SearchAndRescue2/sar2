#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <sys/types.h>

#include "../include/os.h"
#ifdef __MSW__
# include <windows.h>
#endif

#include <GL/gl.h>

#include "v3dmp.h"

#ifdef MEMWATCH
# include "memwatch.h"
#endif


static void *V3DMPMemDup(const void *buf, int total);

void *V3DMPCreate(int type);
void *V3DMPDup(const void *p);
int V3DMPInsertVertex(
	void *p, int i,
	mp_vertex_struct **v_rtn, mp_vertex_struct **n_rtn,
	mp_vertex_struct **tc_rtn
);
void V3DMPDestroy(void *p);

void *V3DMPListGetPtr(void **list, int total, int i);
void *V3DMPListInsert(
	void ***list, int *total, int i,
	int type
);
void V3DMPListDelete(void ***list, int *total, int i);
void V3DMPListDeleteAll(void ***list, int *total);

int V3DMPGetAttributes(
	void *p, int vertex_num,
	mp_vertex_struct **n,
	mp_vertex_struct **tc,
	mp_vertex_struct **v,
	int *total
);
mp_vertex_struct *V3DMPGetNormal(void *p, int i);
mp_vertex_struct *V3DMPGetVertex(void *p, int i);
mp_vertex_struct *V3DMPGetTexCoord(void *p, int i);
int V3DMPGetTotal(void *p);

int V3DMPFlipWinding(void *p, int flip_normals, int flip_texcoords);
int V3DMPUnitlizeNormal(void *p);


#define SQRT(x)         (((x) > 0.0f) ? sqrt(x) : 0.0f)

#define ATOI(s)         (((s) != NULL) ? atoi(s) : 0)
#define ATOL(s)         (((s) != NULL) ? atol(s) : 0)
#define ATOF(s)         (((s) != NULL) ? atof(s) : 0.0f)
#define STRDUP(s)       (((s) != NULL) ? strdup(s) : NULL)

#define MAX(a,b)        (((a) > (b)) ? (a) : (b))
#define MIN(a,b)        (((a) < (b)) ? (a) : (b))
#define CLIP(a,l,h)     (MIN(MAX((a),(l)),(h)))


/*
 *	Returns a dynamically allocated buffer that is a duplicate
 *	of the given buffer specified as bytes long total.
 *
 *	Can return NULL if total is <= 0 or buf is NULL.
 */
static void *V3DMPMemDup(const void *buf, int total)
{
	void *rtn_buf = NULL;


	if((buf == NULL) || (total <= 0))
	    return(rtn_buf);

	rtn_buf = malloc(total);
	if(rtn_buf == NULL)
	    return(rtn_buf);
	else
	    memcpy(rtn_buf, buf, total);

	return(rtn_buf);
}

/*
 *	Allocates a new model primitive structure of the specified
 *	type and sets its type, the primitive will not be `realized'.
 *
 *	Can return NULL on error.
 */
void *V3DMPCreate(int type)
{
	int size = 0;
	void *p = NULL;

	switch(type)
	{
	  case V3DMP_TYPE_COMMENT:
	    size = sizeof(mp_comment_struct);
	    break;


	  case V3DMP_TYPE_TRANSLATE:
	    size = sizeof(mp_translate_struct);
	    break;

	  case V3DMP_TYPE_UNTRANSLATE:
	    size = sizeof(mp_untranslate_struct);
	    break;

	  case V3DMP_TYPE_ROTATE:
	    size = sizeof(mp_rotate_struct);
	    break;

	  case V3DMP_TYPE_UNROTATE:
	    size = sizeof(mp_unrotate_struct);
	    break;


	  case V3DMP_TYPE_POINT:
	    size = sizeof(mp_point_struct);
	    break;

	  case V3DMP_TYPE_LINE:
	    size = sizeof(mp_line_struct);
	    break;

	  case V3DMP_TYPE_LINE_STRIP:
	    size = sizeof(mp_line_strip_struct);
	    break;

	  case V3DMP_TYPE_LINE_LOOP:
	    size = sizeof(mp_line_loop_struct);
	    break;

	  case V3DMP_TYPE_TRIANGLE:
	    size = sizeof(mp_triangle_struct);
	    break;

	  case V3DMP_TYPE_TRIANGLE_STRIP:
	    size = sizeof(mp_triangle_strip_struct);
	    break;

	  case V3DMP_TYPE_TRIANGLE_FAN:
	    size = sizeof(mp_triangle_fan_struct);
	    break;

	  case V3DMP_TYPE_QUAD:
	    size = sizeof(mp_quad_struct);
	    break;

	  case V3DMP_TYPE_QUAD_STRIP:
	    size = sizeof(mp_quad_strip_struct);
	    break;

	  case V3DMP_TYPE_POLYGON:
	    size = sizeof(mp_polygon_struct);
	    break;


	  case V3DMP_TYPE_COLOR:
	    size = sizeof(mp_color_struct);
	    break;

	  case V3DMP_TYPE_TEXTURE_SELECT:
	    size = sizeof(mp_texture_select_struct);
	    break;

	  case V3DMP_TYPE_TEXTURE_ORIENT_XY:
	    size = sizeof(mp_texture_orient_xy_struct);
	    break;

	  case V3DMP_TYPE_TEXTURE_ORIENT_YZ:
	    size = sizeof(mp_texture_orient_yz_struct);
	    break;

	  case V3DMP_TYPE_TEXTURE_ORIENT_XZ:
	    size = sizeof(mp_texture_orient_xz_struct);
	    break;

	  case V3DMP_TYPE_TEXTURE_OFF:
	    size = sizeof(mp_texture_off_struct);
	    break;

	  case V3DMP_TYPE_HEIGHTFIELD_LOAD:
	    size = sizeof(mp_heightfield_load_struct);
	    break;


	  default:
	    fprintf(
		stderr,
		"V3DMPCreate(): Unsupported primitive type %i\n",
		type
	    );
	    break;
	}

	if(size > 0)
	{
	    p = (void *)calloc(1, size);
	    (*(int *)p) = type;
	}

	return(p);
}

/*
 *	Returns a dynamically allocated duplicate of the given
 *	model primitive but with its substructures not `realized'.
 *
 *	Not all dynamically allocated substructures will be coppied,
 *	this is so that the calling function can decide to `realize'
 *	the duplicated model primitive or not.
 *
 *	Can return NULL on error.
 */
void *V3DMPDup(const void *p)
{
	void *np = NULL;
	int ptype, size = 0;


	if(p == NULL)
	    return(np);
	else
	    ptype = V3DMPGetType(p);


/* Allocates np to the specified size and coppies values over from p.
 * So if p has any members as pointers, they need to be reset!
 */
#define DO_ALLOC	\
{ np = V3DMPMemDup(p, size); }

/* Coppies vertexes on primitive ptr to n_ptr which both have dynamically
 * allocated vertexes. Uses variables; i, n_ptr, ptr, 
 */
#define DO_COPY_DYN_VERTEXES				\
{							\
 n_ptr->total = ptr->total;				\
 if(ptr->total > 0) {					\
  n_ptr->v = (mp_vertex_struct **)calloc(		\
   ptr->total, sizeof(mp_vertex_struct *)		\
  );							\
  n_ptr->n = (mp_vertex_struct **)calloc(		\
   ptr->total, sizeof(mp_vertex_struct *)		\
  );							\
  n_ptr->tc = (mp_vertex_struct **)calloc(		\
   ptr->total, sizeof(mp_vertex_struct *)		\
  );							\
 } else {						\
  n_ptr->v = NULL;					\
  n_ptr->n = NULL;					\
  n_ptr->tc = NULL;					\
 }							\
 if((n_ptr->v == NULL) || (n_ptr->n == NULL) ||		\
    (n_ptr->tc == NULL)					\
 ) {							\
  n_ptr->total = 0;					\
 } else {						\
  for(i = 0; i < n_ptr->total; i++) {			\
   n_ptr->v[i] = (mp_vertex_struct *)V3DMPMemDup(	\
    ptr->v[i], sizeof(mp_vertex_struct)			\
   );							\
   n_ptr->n[i] = (mp_vertex_struct *)V3DMPMemDup(	\
    ptr->n[i], sizeof(mp_vertex_struct)			\
   );							\
   n_ptr->tc[i] = (mp_vertex_struct *)V3DMPMemDup(	\
    ptr->tc[i], sizeof(mp_vertex_struct)		\
   );							\
  }							\
 }							\
}


	switch(ptype)
	{
	  case V3DMP_TYPE_COMMENT:
	    size = sizeof(mp_comment_struct);
	    DO_ALLOC
	    if(np != NULL)
	    {
		int i;
		const mp_comment_struct *ptr = (mp_comment_struct *)p;
		mp_comment_struct *n_ptr = (mp_comment_struct *)np;

		n_ptr->total_lines = ptr->total_lines;
		if(ptr->total_lines > 0)
		    n_ptr->line = (char **)calloc(
			ptr->total_lines, sizeof(char *)
		    );
		else
		    n_ptr->line = NULL;
		if(n_ptr->line == NULL)
		    n_ptr->total_lines = 0;
		for(i = 0; i < n_ptr->total_lines; i++)
		    n_ptr->line[i] = STRDUP(ptr->line[i]);
	    }
	    break;


	  case V3DMP_TYPE_TRANSLATE:
	    size = sizeof(mp_translate_struct);
	    DO_ALLOC
	    break;

	  case V3DMP_TYPE_UNTRANSLATE:   
	    size = sizeof(mp_untranslate_struct);
	    DO_ALLOC
	    break;

	  case V3DMP_TYPE_ROTATE:
	    size = sizeof(mp_rotate_struct);
	    DO_ALLOC
	    break;

	  case V3DMP_TYPE_UNROTATE:
	    size = sizeof(mp_unrotate_struct);
	    DO_ALLOC
	    break;


	  case V3DMP_TYPE_POINT:
	    size = sizeof(mp_point_struct);
	    DO_ALLOC
	    break;
	  
	  case V3DMP_TYPE_LINE:
	    size = sizeof(mp_line_struct);
	    DO_ALLOC
	    break;
		
	  case V3DMP_TYPE_LINE_STRIP:
	    size = sizeof(mp_line_strip_struct);
	    DO_ALLOC
	    if(np != NULL)
	    {
		int i;
		const mp_line_strip_struct *ptr = (mp_line_strip_struct *)p;
		mp_line_strip_struct *n_ptr = (mp_line_strip_struct *)np;
		DO_COPY_DYN_VERTEXES
	    }
	    break;    
	 
	  case V3DMP_TYPE_LINE_LOOP:
	    size = sizeof(mp_line_loop_struct);
	    DO_ALLOC
	    if(np != NULL)
	    {
		int i;
		const mp_line_loop_struct *ptr = (mp_line_loop_struct *)p;
		mp_line_loop_struct *n_ptr = (mp_line_loop_struct *)np;
		DO_COPY_DYN_VERTEXES
	    }
	    break;

	  case V3DMP_TYPE_TRIANGLE:
	    size = sizeof(mp_triangle_struct);
	    DO_ALLOC
	    break;

	  case V3DMP_TYPE_TRIANGLE_STRIP:
	    size = sizeof(mp_triangle_strip_struct);
	    DO_ALLOC
	    if(np != NULL)
	    {
		int i;
		const mp_triangle_strip_struct *ptr = (mp_triangle_strip_struct *)p;
		mp_triangle_strip_struct *n_ptr = (mp_triangle_strip_struct *)np;
		DO_COPY_DYN_VERTEXES
	    }
	    break;
 
	  case V3DMP_TYPE_TRIANGLE_FAN:
	    size = sizeof(mp_triangle_fan_struct);
	    DO_ALLOC
	    if(np != NULL)
	    {
		int i;
		const mp_triangle_fan_struct *ptr = (mp_triangle_fan_struct *)p;
		mp_triangle_fan_struct *n_ptr = (mp_triangle_fan_struct *)np;
		DO_COPY_DYN_VERTEXES
	    }
	    break;

	  case V3DMP_TYPE_QUAD:
	    size = sizeof(mp_quad_struct);
	    DO_ALLOC
	    break;

	  case V3DMP_TYPE_QUAD_STRIP:
	    size = sizeof(mp_quad_strip_struct);
	    DO_ALLOC
	    if(np != NULL)
	    {
		int i;
		const mp_quad_strip_struct *ptr = (mp_quad_strip_struct *)p;
		mp_quad_strip_struct *n_ptr = (mp_quad_strip_struct *)np;
		DO_COPY_DYN_VERTEXES
	    }  
	    break;  

	 case V3DMP_TYPE_POLYGON:
	    size = sizeof(mp_polygon_struct);
	    DO_ALLOC  
	    if(np != NULL)
	    {
		int i;
		const mp_polygon_struct *ptr = (mp_polygon_struct *)p;
		mp_polygon_struct *n_ptr = (mp_polygon_struct *)np;
		DO_COPY_DYN_VERTEXES
	    }
	    break;


	  case V3DMP_TYPE_COLOR:
	    size = sizeof(mp_color_struct);
	    DO_ALLOC
	    break;
	 
	  case V3DMP_TYPE_TEXTURE_SELECT:
	    size = sizeof(mp_texture_select_struct);
	    DO_ALLOC
	    if(np != NULL)
	    {
		const mp_texture_select_struct *ptr = (mp_texture_select_struct *)p;
		mp_texture_select_struct *n_ptr = (mp_texture_select_struct *)np;

		if(ptr->name != NULL)
		    n_ptr->name = STRDUP(ptr->name);

		/* Need to reset client data (unrealizing it). */
		n_ptr->client_data = NULL;
	    }
	    break;
	    
	  case V3DMP_TYPE_TEXTURE_ORIENT_XY:
	    size = sizeof(mp_texture_orient_xy_struct);
	    DO_ALLOC
	    break;

	  case V3DMP_TYPE_TEXTURE_ORIENT_YZ:
	    size = sizeof(mp_texture_orient_yz_struct);
	    DO_ALLOC
	    break;

	  case V3DMP_TYPE_TEXTURE_ORIENT_XZ:
	    size = sizeof(mp_texture_orient_xz_struct);
	    DO_ALLOC
	    break;

	  case V3DMP_TYPE_TEXTURE_OFF:
	    size = sizeof(mp_texture_off_struct);
	    DO_ALLOC
	    break;

	  case V3DMP_TYPE_HEIGHTFIELD_LOAD:
	    size = sizeof(mp_heightfield_load_struct);
	    DO_ALLOC
	    if(np != NULL)
	    {
		const mp_heightfield_load_struct *ptr =
		    (mp_heightfield_load_struct *)p;
		mp_heightfield_load_struct *n_ptr =
		    (mp_heightfield_load_struct *)np;

		if(ptr->path != NULL)
		    n_ptr->path = STRDUP(ptr->path);

		/* Need to reset client set GL list. */
		n_ptr->gl_list = NULL;

		if(ptr->data != NULL)
		    n_ptr->data = (double *)V3DMPMemDup(
			ptr->data, ptr->total_points * sizeof(double)
		    );
	    }
	    break;

	  default:   
	    fprintf(   
		stderr,
		"V3DMPDup(): Unsupported primitive type %i\n",
		ptype
	    );
	    break;
	}
#undef DO_ALLOC
#undef DO_COPY_DYN_VERTEXES
	return(np);
}

/*
 *	Checks if p is a model primitive with variable amount of vertexes
 *	and if so, then inserts a new normal, vertex and texcoord at index
 *	i. If i is negative or greater than the current total
 *	number of vertexes on p then i will be adjusted to append a
 *	texture coordinate and vertex to p.
 *
 *	Returns -1 on error, -2 if primitive cannot have variable
 *	number of vertexes, or the index number of the normal, vertex,
 *	and texcoord actually allocated.
 */
int V3DMPInsertVertex(
	void *p, int i,
	mp_vertex_struct **v_rtn, mp_vertex_struct **n_rtn,
	mp_vertex_struct **tc_rtn
)
{
	mp_vertex_struct	***v_list = NULL,
				***n_list = NULL,
				***tc_list = NULL;
	mp_vertex_struct *v, *n, *tc;
	int j, k, ptype, *total = NULL;

	mp_line_strip_struct *line_strip;
	mp_line_loop_struct *line_loop;
	mp_triangle_strip_struct *triangle_strip;
	mp_triangle_fan_struct *triangle_fan;
	mp_quad_strip_struct *quad_strip;
	mp_polygon_struct *polygon;


	/* Reset returns. */
	if(v_rtn != NULL)
	    (*v_rtn) = NULL;
	if(n_rtn != NULL)
	    (*n_rtn) = NULL;
	if(tc_rtn != NULL)
	    (*tc_rtn) = NULL;

	if(p == NULL)
	    return(-1);

	ptype = V3DMPGetType(p);

	/* Get pointer to v_list and tc_list, thus checking if the
	 * primitive has variable number of vertexes.
	 */
	switch(ptype)
	{
	  case V3DMP_TYPE_LINE_STRIP:
	    line_strip = p;
	    v_list = &line_strip->v;
	    n_list = &line_strip->n;
	    tc_list = &line_strip->tc;
	    total = &line_strip->total;
	    break;

	  case V3DMP_TYPE_LINE_LOOP:
	    line_loop = p;
	    v_list = &line_loop->v;
	    n_list = &line_loop->n;
	    tc_list = &line_loop->tc;
	    total = &line_loop->total;
	    break;

	  case V3DMP_TYPE_TRIANGLE_STRIP:
	    triangle_strip = p;
	    v_list = &triangle_strip->v;
	    n_list = &triangle_strip->n;
	    tc_list = &triangle_strip->tc;
	    total = &triangle_strip->total;
	    break;

	  case V3DMP_TYPE_TRIANGLE_FAN:
	    triangle_fan = p;
	    v_list = &triangle_fan->v;
	    n_list = &triangle_fan->n;
	    tc_list = &triangle_fan->tc;
	    total = &triangle_fan->total;
	    break;

	  case V3DMP_TYPE_QUAD_STRIP:
	    quad_strip = p;
	    v_list = &quad_strip->v;
	    n_list = &quad_strip->n;
	    tc_list = &quad_strip->tc;
	    total = &quad_strip->total;
	    break;

	  case V3DMP_TYPE_POLYGON:
	    polygon = p;
	    v_list = &polygon->v;
	    n_list = &polygon->n;
	    tc_list = &polygon->tc; 
	    total = &polygon->total;
	    break;
	}

	/* Not a variable number of vertex type primitive? */
	if((v_list == NULL) ||
	   (n_list == NULL) ||
	   (tc_list == NULL) ||
	   (total == NULL)
	)
	    return(-2);

	/* Sanitize total. */
	if((*total) < 0)
	    (*total) = 0;

	/* Calculate new vertex insert position k. */

	/* Append? */
	if((i < 0) || (i > (*total)))
	{
	    k = (*total);
	    (*total) = k + 1;
	}
	/* Within bounds. */
	else
	{
	    k = i;
	    (*total) = (*total) + 1;
	}


	/* Allocate more pointers for vertex, normal, and texture
	 * coordinate lists.
	 */
	/* Vertexes. */
	(*v_list) = (mp_vertex_struct **)realloc(
	    *v_list,
	    (*total) * sizeof(mp_vertex_struct *)
	);
	if((*v_list) == NULL)
	{
	    (*v_list) = NULL;
	    (*n_list) = NULL;
	    (*tc_list) = NULL;
	    (*total) = 0;
	    return(-1);
	}

	/* Normals. */
	(*n_list) = (mp_vertex_struct **)realloc(
	    *n_list,
	    (*total) * sizeof(mp_vertex_struct *)
	);
	if((*n_list) == NULL)
	{
	    (*v_list) = NULL;
	    (*n_list) = NULL;
	    (*tc_list) = NULL;
	    (*total) = 0;
	    return(-1);
	}

	/* Texture coordinates. */
	(*tc_list) = (mp_vertex_struct **)realloc(
	    *tc_list,
	    (*total) * sizeof(mp_vertex_struct *)
	);
	if((*tc_list) == NULL)
	{
	    (*v_list) = NULL;
	    (*n_list) = NULL;
	    (*tc_list) = NULL;
	    (*total) = 0;
	    return(-1);
	}

	/* Shift pointers. */
	for(j = (*total) - 1; j > k; j--)
	{
	    (*v_list)[j] = (*v_list)[j - 1];
	    (*n_list)[j] = (*n_list)[j - 1];
	    (*tc_list)[j] = (*tc_list)[j - 1];
	}


	/* Allocate new texture coordinate and vertex pair. */
	v = (mp_vertex_struct *)calloc(1, sizeof(mp_vertex_struct));
	n = (mp_vertex_struct *)calloc(1, sizeof(mp_vertex_struct));
	tc = (mp_vertex_struct *)calloc(1, sizeof(mp_vertex_struct));

	/* Assign to pointers in list. */
	(*v_list)[k] = v;
	(*n_list)[k] = n;
	(*tc_list)[k] = tc;


	/* Allocation error? */
	if((v == NULL) || (n == NULL) || (tc == NULL))
	    return(-1);

	/* Set pointer to returns? */
	if(v_rtn != NULL)
	    (*v_rtn) = v;
	if(n_rtn != NULL)
	    (*n_rtn) = n;
	if(tc_rtn != NULL)
	    (*tc_rtn) = tc;

	return(k);
}

/*
 *	Deallocates the model primitive structure p and any allocated
 *	sub resources.
 */
void V3DMPDestroy(void *p)
{
	int i, type;
	GLuint list;
	mp_comment_struct *comment;
	mp_line_strip_struct *line_strip;
	mp_line_loop_struct *line_loop;
	mp_triangle_strip_struct *triangle_strip;
	mp_triangle_fan_struct *triangle_fan;
	mp_quad_strip_struct *quad_strip;
	mp_polygon_struct *polygon;
	mp_texture_select_struct *texture_select;
	mp_heightfield_load_struct *heightfield_load;


	if(p == NULL)
	    return;
	else
	    type = V3DMPGetType(p);

	/* Deallocate any substructures. */
	switch(type)
	{
	  case V3DMP_TYPE_COMMENT:
	    comment = p;
	    for(i = 0; i < comment->total_lines; i++)
		free(comment->line[i]);
	    free(comment->line);
	    break;

	  case V3DMP_TYPE_LINE_STRIP:
	    line_strip = p;
	    for(i = 0; i < line_strip->total; i++)
	    {
		free(line_strip->v[i]);
		free(line_strip->n[i]);
		free(line_strip->tc[i]);
	    }
	    free(line_strip->v);
	    free(line_strip->n);
	    free(line_strip->tc);
	    break;

	  case V3DMP_TYPE_LINE_LOOP:
	    line_loop = p;
	    for(i = 0; i < line_loop->total; i++)
	    {
		free(line_loop->v[i]);
		free(line_loop->n[i]);
		free(line_loop->tc[i]);
	    }
	    free(line_loop->v);
	    free(line_loop->n);
	    free(line_loop->tc);
	    break;

	  case V3DMP_TYPE_TRIANGLE_STRIP:
	    triangle_strip = p;
	    for(i = 0; i < triangle_strip->total; i++)
	    {
		free(triangle_strip->v[i]);
		free(triangle_strip->n[i]);
		free(triangle_strip->tc[i]);
	    }
	    free(triangle_strip->v);
	    free(triangle_strip->n);
	    free(triangle_strip->tc);
	    break;

	  case V3DMP_TYPE_TRIANGLE_FAN:
	    triangle_fan = p;
	    for(i = 0; i < triangle_fan->total; i++)
	    {
		free(triangle_fan->v[i]);
		free(triangle_fan->n[i]);
		free(triangle_fan->tc[i]);
	    }
	    free(triangle_fan->v);
	    free(triangle_fan->n);
	    free(triangle_fan->tc);
	    break;

	  case V3DMP_TYPE_QUAD_STRIP:
	    quad_strip = p;
	    for(i = 0; i < quad_strip->total; i++)
	    {
		free(quad_strip->v[i]);
		free(quad_strip->n[i]);
		free(quad_strip->tc[i]);
	    }
	    free(quad_strip->v);
	    free(quad_strip->n);
	    free(quad_strip->tc);
	    break;

	  case V3DMP_TYPE_POLYGON:
	    polygon = p;
	    for(i = 0; i < polygon->total; i++)
	    {
		free(polygon->v[i]);
		free(polygon->n[i]);
		free(polygon->tc[i]);
	    }
	    free(polygon->v);
	    free(polygon->n);
	    free(polygon->tc);
	    break;

	  case V3DMP_TYPE_TEXTURE_SELECT:
	    texture_select = p;
	    free(texture_select->name);
	    /* Do not free client data. */
	    break;

	  case V3DMP_TYPE_HEIGHTFIELD_LOAD:
	    heightfield_load = p;
	    free(heightfield_load->path);
	    /* This should actually be unloaded by calling function. */
	    list = (GLuint)heightfield_load->gl_list;
	    if(list != 0)
		glDeleteLists(list, 1);
	    free(heightfield_load->data);
	    break;
	}

	/* Free structure itself. */
	free(p);

	return;
}


/*
 *	Checks if item i exists in the list and returns its pointer
 *	if it exists or NULL if it does not.
 */
void *V3DMPListGetPtr(void **list, int total, int i)
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
 *	Inserts a new model primitive item at list position i and
 *	returns the pointer to the model primitive or NULL on failure.
 *
 *	If i is negative then the item will be appended to the list.
 *
 *	If i is greater or equal to (*total) + 1 then i will
 *	be set to the value of (*total).
 *
 *	Both list and total need to be non NULL.
 */
void *V3DMPListInsert(
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

	    (*list)[i] = V3DMPCreate(type);
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

	    (*list)[i] = V3DMPCreate(type);
	}

	return((*list)[i]);
}

/*
 *	Deletes the item i from the list.  Item pointer i on the list
 *	will then be set to NULL.
 */
void V3DMPListDelete(void ***list, int *total, int i)
{
	void *p;

	if((list == NULL) ||
	   (total == NULL)
	)
	    return;

	/* Item exists and is allocated? */
	p = V3DMPListGetPtr(*list, *total, i);
	if(p != NULL)
	{
	    V3DMPDestroy(p);
	    (*list)[i] = NULL;	/* Reset item pointer to NULL. */
	}

	return;
}

/*
 *	Deletes the entire list and resets the pointers list
 *	and total to NULL.
 */
void V3DMPListDeleteAll(void ***list, int *total)
{
	int i;

	if((list == NULL) ||
	   (total == NULL)
	)
	    return;

	for(i = 0; i < (*total); i++)
	    V3DMPListDelete(list, total, i);

	free(*list);
	(*list) = NULL;

	(*total) = 0;

	return;
}


/*
 *	Returns the requested information, pointers returned should
 *	not be free'ed.
 *
 *	Returns non-zero if i is out of range, on error, or if the
 *	primitive's type does not have vertexes.
 */
int V3DMPGetAttributes(
	void *p, int vertex_num,
	mp_vertex_struct **n,	/* Normal pointer. */
	mp_vertex_struct **tc,	/* Texture coordinate pointer. */
	mp_vertex_struct **v,	/* Vertex pointer. */
	int *total		/* Total vertexes on primitive. */
)
{
	int i = vertex_num, t;

	mp_point_struct *point;
	mp_line_struct *line;
	mp_line_strip_struct *line_strip;
	mp_line_loop_struct *line_loop;
	mp_triangle_struct *triangle;
	mp_triangle_strip_struct *triangle_strip;
	mp_triangle_fan_struct *triangle_fan;
	mp_quad_struct *quad;
	mp_quad_strip_struct *quad_strip;
	mp_polygon_struct *polygon;


	if((p == NULL) ||
	   (i < 0)
	)
	    return(-1);

	/* Reset values. */
	if(n != NULL)
	    (*n) = NULL;
	if(tc != NULL)
	    (*tc) = NULL;
	if(v != NULL)
	    (*v) = NULL;
	if(total != NULL)
	    (*total) = 0;

	/* Begin fetching attributes based on primitive type. */
	switch(V3DMPGetType(p))
	{
	  case V3DMP_TYPE_POINT:
	    point = p;
	    t = V3DMP_POINT_NVERTEX;
	    if(i < t)
	    {
		if(v != NULL)
		    (*v) = &point->v[i];
		if(n != NULL)
		    (*n) = &point->n[i];
		if(tc != NULL)
		    (*tc) = &point->tc[i];
	    }
	    if(total != NULL)
		(*total) = t;
	    break;

	  case V3DMP_TYPE_LINE:
	    line = p;
	    t = V3DMP_LINE_NVERTEX;
	    if(i < t)
	    {
		if(v != NULL)
		    (*v) = &line->v[i];
		if(n != NULL)
		    (*n) = &line->n[i];
		if(tc != NULL)
		    (*tc) = &line->tc[i];
	    }
	    if(total != NULL)
		(*total) = t;
	    break;

	  case V3DMP_TYPE_LINE_STRIP:
	    line_strip = p;
	    t = line_strip->total;
	    if(i < t)
	    {
		if(v != NULL)
		    (*v) = line_strip->v[i];
		if(n != NULL)
		    (*n) = line_strip->n[i];
		if(tc != NULL)
		    (*tc) = line_strip->tc[i];
	    }
	    if(total != NULL)
		(*total) = t;
	    break;

	  case V3DMP_TYPE_LINE_LOOP:
	    line_loop = p; 
	    t = line_loop->total;
	    if(i < t)
	    {
		if(v != NULL)
		    (*v) = line_loop->v[i];
		if(n != NULL)
		    (*n) = line_loop->n[i];
		if(tc != NULL)
		    (*tc) = line_loop->tc[i];
	    }
	    if(total != NULL)
		(*total) = t;
	    break;

	  case V3DMP_TYPE_TRIANGLE:
	    triangle = p;
	    t = V3DMP_TRIANGLE_NVERTEX;
	    if(i < t)
	    {
		if(v != NULL)
		    (*v) = &triangle->v[i];
		if(n != NULL)
		    (*n) = &triangle->n[i];
		if(tc != NULL)
		    (*tc) = &triangle->tc[i];
	    }
	    if(total != NULL)
		(*total) = t;
	    break;

	  case V3DMP_TYPE_TRIANGLE_STRIP:
	    triangle_strip = p;
	    t = triangle_strip->total;
	    if(i < t)
	    {
		if(v != NULL)
		    (*v) = triangle_strip->v[i];
		if(n != NULL)
		    (*n) = triangle_strip->n[i];
		if(tc != NULL)
		    (*tc) = triangle_strip->tc[i];
	    }
	    if(total != NULL)
		(*total) = t;
	    break;

	  case V3DMP_TYPE_TRIANGLE_FAN:
	    triangle_fan = p;
	    t = triangle_fan->total;
	    if(i < t)
	    {
		if(v != NULL)
		    (*v) = triangle_fan->v[i];
		if(n != NULL)
		    (*n) = triangle_fan->n[i];
		if(tc != NULL)
		    (*tc) = triangle_fan->tc[i];
	    }
	    if(total != NULL)
		(*total) = t;
	    break;

	  case V3DMP_TYPE_QUAD:
	    quad = p;
	    t = V3DMP_QUAD_NVERTEX;
	    if(i < t)
	    {
		if(v != NULL)
		    (*v) = &quad->v[i];
		if(n != NULL)
		    (*n) = &quad->n[i];
		if(tc != NULL)
		    (*tc) = &quad->tc[i];
	    }
	    if(total != NULL)
		(*total) = t;
	    break;

	  case V3DMP_TYPE_QUAD_STRIP:
	    quad_strip = p;
	    t = quad_strip->total;
	    if(i < t)
	    {
		if(v != NULL)
		    (*v) = quad_strip->v[i];
		if(n != NULL)
		    (*n) = quad_strip->n[i];
		if(tc != NULL)
		    (*tc) = quad_strip->tc[i];
	    }
	    if(total != NULL)
		(*total) = t;
	    break;

	  case V3DMP_TYPE_POLYGON:
	    polygon = p;
	    t = polygon->total;
	    if(i < t)
	    {
		if(v != NULL)
		    (*v) = polygon->v[i];
		if(n != NULL)
		    (*n) = polygon->n[i];
		if(tc != NULL)
		    (*tc) = polygon->tc[i];
	    }
	    if(total != NULL)
		(*total) = t;
	    break;
	}

	return(0);
}


mp_vertex_struct *V3DMPGetNormal(void *p, int i)
{
	mp_vertex_struct *n = NULL;

	V3DMPGetAttributes(
	    p, i, &n, NULL, NULL, NULL
	);

	return(n);
}

mp_vertex_struct *V3DMPGetVertex(void *p, int i)
{
	mp_vertex_struct *v = NULL;

	V3DMPGetAttributes(
	    p, i, NULL, NULL, &v, NULL
	);

	return(v);
}

mp_vertex_struct *V3DMPGetTexCoord(void *p, int i)
{
	mp_vertex_struct *tc = NULL;

	V3DMPGetAttributes(
	    p, i, NULL, &tc, NULL, NULL
	);

	return(tc);
}

int V3DMPGetTotal(void *p)
{
	int total = 0;

	V3DMPGetAttributes(
	    p, 0, NULL, NULL, NULL, &total
	);

	return(total);
}


/*
 *	Sets the normals on all vertexes on the given primitive p
 *	to unit normals.
 *
 *	Returns 0 on success, -1 on error, or -2 if the primitive
 *	does not have vertexes to flip (which can be a valid primitive
 *      that *could* have vertexes but just didn't have any).
 */
int V3DMPUnitlizeNormal(void *p)
{
	mp_vertex_struct *sn = NULL, **dn = NULL;
	int ptype, total = 0;

	mp_point_struct *point;
	mp_line_struct *line;
	mp_line_strip_struct *line_strip;
	mp_line_loop_struct *line_loop;
	mp_triangle_struct *triangle;
	mp_triangle_strip_struct *triangle_strip;
	mp_triangle_fan_struct *triangle_fan;
	mp_quad_struct *quad;
	mp_quad_strip_struct *quad_strip;
	mp_polygon_struct *polygon;

	if(p == NULL)
	    return(-1);

	ptype = V3DMPGetType(p);
	switch(ptype)
	{
	  case V3DMP_TYPE_POINT:
	    point = p;
	    sn = &point->n[0];
	    total = V3DMP_POINT_NVERTEX;
	    break;

	  case V3DMP_TYPE_LINE:
	    line = p;
	    sn = &line->n[0];
	    total = V3DMP_LINE_NVERTEX;
	    break;

	  case V3DMP_TYPE_LINE_STRIP:
	    line_strip = p;
	    dn = line_strip->n;
	    total = line_strip->total;
	    break;

	  case V3DMP_TYPE_LINE_LOOP:
	    line_loop = p;
	    dn = line_loop->n;
	    total = line_loop->total;
	    break;

	  case V3DMP_TYPE_TRIANGLE:
	    triangle = p;
	    sn = &triangle->n[0];
	    total = V3DMP_TRIANGLE_NVERTEX;
	    break;

	  case V3DMP_TYPE_TRIANGLE_STRIP:
	    triangle_strip = p;
	    dn = triangle_strip->n;
	    total = triangle_strip->total;
	    break;

	  case V3DMP_TYPE_TRIANGLE_FAN:
	    triangle_fan = p;
	    dn = triangle_fan->n;
	    total = triangle_fan->total;
	    break;

	  case V3DMP_TYPE_QUAD:
	    quad = p;
	    sn = &quad->n[0];
	    total = V3DMP_QUAD_NVERTEX;
	    break;

	  case V3DMP_TYPE_QUAD_STRIP:
	    quad_strip = p;
	    dn = quad_strip->n;
	    total = quad_strip->total;
	    break;

	  case V3DMP_TYPE_POLYGON:
	    polygon = p;
	    dn = polygon->n;
	    total = polygon->total;
	    break;
	}

	/* Finite vertex primitive? */
	if((sn != NULL) && (total > 1))
	{
	    int i;
	    double len;
	    mp_vertex_struct *n;

	    for(i = 0; i < total; i++)
	    {
		n = &sn[i];
		len = SQRT(
		    (n->x * n->x) + (n->y * n->y) + (n->z * n->z)
		);
		if(len > 0.0)
		{
		    n->x = n->x / len;
		    n->y = n->y / len;
		    n->z = n->z / len;
		}
	    }
	}
	/* Infinite vertex primitive? */
	else if((dn != NULL) && (total > 1))
	{
	    int i;
	    double len;
	    mp_vertex_struct *n;

	    for(i = 0; i < total; i++)
	    {
		n = dn[i];
		if(n == NULL)
		    continue;

		len = SQRT(
		    (n->x * n->x) + (n->y * n->y) + (n->z * n->z)
		);
		if(len > 0.0)
		{
		    n->x = n->x / len;
		    n->y = n->y / len;
		    n->z = n->z / len;
		}
	    }
	}


	return(0);
}

/*
 *	Flips the winding of the vertexes on the given primitive p.
 *	Returns 0 on success, -1 on error, or -2 if the primitive
 *	does not have vertexes to flip (which can be a valid primitive
 *	that *could* have vertexes but just didn't have any).
 */
int V3DMPFlipWinding(void *p, int flip_normals, int flip_texcoords)
{
	mp_vertex_struct *sv = NULL, **dv = NULL;
	mp_vertex_struct *sn = NULL, **dn = NULL;
	mp_vertex_struct *stc = NULL, **dtc = NULL;
	int ptype, total = 0;

	mp_point_struct *point;
	mp_line_struct *line;
	mp_line_strip_struct *line_strip;
	mp_line_loop_struct *line_loop;
	mp_triangle_struct *triangle;
	mp_triangle_strip_struct *triangle_strip;
	mp_triangle_fan_struct *triangle_fan;
	mp_quad_struct *quad;
	mp_quad_strip_struct *quad_strip;
	mp_polygon_struct *polygon;


	if(p == NULL)
	    return(-1);

	ptype = V3DMPGetType(p);
	switch(ptype)
	{
	  case V3DMP_TYPE_POINT:
	    point = p;
	    sv = &point->v[0];
	    sn = &point->n[0];
	    stc = &point->tc[0];
	    total = V3DMP_POINT_NVERTEX;
	    break;

	  case V3DMP_TYPE_LINE:
	    line = p;
	    sv = &line->v[0];
	    sn = &line->n[0];
	    stc = &line->tc[0];
	    total = V3DMP_LINE_NVERTEX;
	    break;

	  case V3DMP_TYPE_LINE_STRIP:
	    line_strip = p;
	    dv = line_strip->v;
	    dn = line_strip->n;
	    dtc = line_strip->tc;
	    total = line_strip->total;
	    break;

	  case V3DMP_TYPE_LINE_LOOP:
	    line_loop = p;
	    dv = line_loop->v;
	    dn = line_loop->n;
	    dtc = line_loop->tc;
	    total = line_loop->total;
	    break;

	  case V3DMP_TYPE_TRIANGLE:
	    triangle = p;
	    sv = &triangle->v[0];
	    sn = &triangle->n[0];
	    stc = &triangle->tc[0];
	    total = V3DMP_TRIANGLE_NVERTEX;
	    break;

	  case V3DMP_TYPE_TRIANGLE_STRIP:
	    triangle_strip = p;
	    dv = triangle_strip->v;
	    dn = triangle_strip->n;
	    dtc = triangle_strip->tc;
	    total = triangle_strip->total;
	    break;

	  case V3DMP_TYPE_TRIANGLE_FAN:
	    triangle_fan = p;
	    dv = triangle_fan->v;
	    dn = triangle_fan->n;
	    dtc = triangle_fan->tc;
	    total = triangle_fan->total;
	    break;

	  case V3DMP_TYPE_QUAD:
	    quad = p;
	    sv = &quad->v[0];
	    sn = &quad->n[0];
	    stc = &quad->tc[0];
	    total = V3DMP_QUAD_NVERTEX;
	    break;

	  case V3DMP_TYPE_QUAD_STRIP:
	    quad_strip = p;
	    dv = quad_strip->v;
	    dn = quad_strip->n;
	    dtc = quad_strip->tc;
	    total = quad_strip->total;
	    break;

	  case V3DMP_TYPE_POLYGON:
	    polygon = p;
	    dv = polygon->v;
	    dn = polygon->n;
	    dtc = polygon->tc;
	    total = polygon->total;
	    break;
	}

	/* Finite vertex primitive? */
	if((sv != NULL) && (sn != NULL) && (stc != NULL) && (total > 1))
	{
	    mp_vertex_struct v_tmp;
	    mp_vertex_struct *v_l, *v_h, *n_l, *n_h, *tc_l, *tc_h;
	    int i, n, half_total = total / 2;

	    for(i = 0; i < half_total; i++)
	    {
		n = total - 1 - i;

		v_l = &sv[i];
		v_h = &sv[n];
		memcpy(&v_tmp, v_l, sizeof(mp_vertex_struct));
		memcpy(v_l, v_h, sizeof(mp_vertex_struct));
		memcpy(v_h, &v_tmp, sizeof(mp_vertex_struct));

		if(flip_normals)
		{
		 n_l = &sn[i];
		 n_h = &sn[n];
		 memcpy(&v_tmp, n_l, sizeof(mp_vertex_struct));
		 memcpy(n_l, n_h, sizeof(mp_vertex_struct));
		 memcpy(n_h, &v_tmp, sizeof(mp_vertex_struct));
		}

		if(flip_texcoords)
		{
		 tc_l = &stc[i];
		 tc_h = &stc[n];
		 memcpy(&v_tmp, tc_l, sizeof(mp_vertex_struct));
		 memcpy(tc_l, tc_h, sizeof(mp_vertex_struct));
		 memcpy(tc_h, &v_tmp, sizeof(mp_vertex_struct));
		}
	    }
	}
	/* Infinite vertex primitive? */
	else if((dv != NULL) && (dn != NULL) && (dtc != NULL) && (total > 1))
	{
	    mp_vertex_struct v_tmp;
	    mp_vertex_struct *v_l, *v_h, *n_l, *n_h, *tc_l, *tc_h;
	    int i, n, half_total = total / 2;

	    for(i = 0; i < half_total; i++)
	    {
		n = total - 1 - i;

		v_l = dv[i];
		v_h = dv[n];
		if((v_l != NULL) && (v_h != NULL))
		{
		    memcpy(&v_tmp, v_l, sizeof(mp_vertex_struct));
		    memcpy(v_l, v_h, sizeof(mp_vertex_struct));
		    memcpy(v_h, &v_tmp, sizeof(mp_vertex_struct));
		}

		n_l = dn[i];
		n_h = dn[n];
		if((n_l != NULL) && (n_h != NULL) && flip_normals)
		{
		    memcpy(&v_tmp, n_l, sizeof(mp_vertex_struct));
		    memcpy(n_l, n_h, sizeof(mp_vertex_struct));
		    memcpy(n_h, &v_tmp, sizeof(mp_vertex_struct));
		}

		tc_l = dtc[i];
		tc_h = dtc[n];
		if((tc_l != NULL) && (tc_h != NULL) && flip_texcoords)
		{
		    memcpy(&v_tmp, tc_l, sizeof(mp_vertex_struct));
		    memcpy(tc_l, tc_h, sizeof(mp_vertex_struct));
		    memcpy(tc_h, &v_tmp, sizeof(mp_vertex_struct));
		}
	    }
	}

	return(0);
}
