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
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <limits.h>
#include <math.h>

#include "../include/os.h"

#ifdef __MSW__
# include <windows.h>
#endif
#include <GL/gl.h>
#include <GL/glu.h>

#include "../include/disk.h"
#include "../include/string.h"

#include "v3dgl.h"
#include "v3dhf.h"
#include "v3dtex.h"

#include "sar.h"

/* GL interpritation structure IO. */
v3d_glinterprite_struct *V3DGLInterpriteNew(void);
void V3DGLInterpriteDelete(v3d_glinterprite_struct *glinterp);

/* GL resource IO and management. */
v3d_glresource_struct *V3DGLResourceNew(void);
v3d_glresource_struct *V3DGLResourceNewFromModelData(
	const v3d_glinterprite_struct *glinterp,
	void **mh_item, int total_mh_items,
	v3d_model_struct **model, int total_models
);
int V3DGLResourceSetInterpritation(
	v3d_glresource_struct *glres,
	const v3d_glinterprite_struct *glinterp
);
v3d_glinterprite_struct *V3DGLResourceGetInterpritation(
	v3d_glresource_struct *glres  
);
void V3DGLResourceDelete(v3d_glresource_struct *glres);

/* V3D model to GL interpritation processing. */
static void V3DGLSetNormal(
	v3d_glinterprite_struct *glinterp, mp_vertex_struct *n
);
static void V3DGLSetTexCoord(
	v3d_glinterprite_struct *glinterp, mp_vertex_struct *v,
	mp_vertex_struct *tc, int texture_binded, void *p_texture_orient
);
static void V3DGLSetVertex(
	v3d_glinterprite_struct *glinterp, mp_vertex_struct *v
);
static void V3DGLProcessModelOtherData(
	v3d_glresource_struct *glres, v3d_glinterprite_struct *glinterp,
	v3d_model_struct *m,
	void *client_data,
	void (*extra_cb)(v3d_model_struct *, const char *, void *)
);
static void V3DGLProcessModelStandard(
	v3d_glresource_struct *glres, v3d_glinterprite_struct *glinterp,
	v3d_model_struct *m,
	void *client_data,
	void (*extra_cb)(v3d_model_struct *, const char *, void *)
);
void V3DGLProcessModelExtra(
	v3d_glresource_struct *glres,
	v3d_model_struct *m,
	void *client_data,
	void (*extra_cb)(v3d_model_struct *, const char *, void *)
);
void V3DGLProcessModel(
	v3d_glresource_struct *glres,
	v3d_model_struct *m
);


#ifndef PI
# define PI	3.14159265359
#endif

#define SQRT(x)         (((x) > 0.0f) ? sqrt(x) : 0.0f)

#define ATOI(s)         (((s) != NULL) ? atoi(s) : 0)
#define ATOL(s)         (((s) != NULL) ? atol(s) : 0)
#define ATOF(s)         (((s) != NULL) ? atof(s) : 0.0f)
#define STRDUP(s)       (((s) != NULL) ? strdup(s) : NULL)

#define MAX(a,b)        (((a) > (b)) ? (a) : (b))
#define MIN(a,b)        (((a) < (b)) ? (a) : (b))
#define CLIP(a,l,h)     (MIN(MAX((a),(l)),(h)))

#define RADTODEG(r)     ((r) * 180 / PI)


/*
 *	Allocates a new gl interpritation structure with all values
 *	reset.
 */
v3d_glinterprite_struct *V3DGLInterpriteNew(void)
{
	v3d_glinterprite_struct *glinterp = (v3d_glinterprite_struct *)calloc(
	    1, sizeof(v3d_glinterprite_struct)
	);
	return(glinterp);
}

/*
 *	Deallocates the given gl interpritation structure and all
 *	its allocated resources.
 */
void V3DGLInterpriteDelete(v3d_glinterprite_struct *glinterp)
{
	unsigned int flags;

	if(glinterp == NULL)
	    return;

	flags = glinterp->flags;

	/* Deallocate any dynamically allocated members. */
	/* Heightfield base directory. */
	if(flags & V3D_GLFLAG_HEIGHTFIELD_BASE_DIR)
	{
	    free(glinterp->heightfield_base_dir);
	    glinterp->heightfield_base_dir = NULL;
	}
	/* Texture base directory. */
	if(flags & V3D_GLFLAG_TEXTURE_BASE_DIR)
	{
	    free(glinterp->texture_base_dir);
	    glinterp->texture_base_dir = NULL;
	}

	/* Deallocate GL interpritation structure itself. */
	free(glinterp);
}


/*
 *	Allocates a new gl resources structure with all values
 *	reset.
 */
v3d_glresource_struct *V3DGLResourceNew(void)
{
	v3d_glresource_struct *glres = (v3d_glresource_struct *)calloc(
	    1, sizeof(v3d_glresource_struct)
	);
	if(glres != NULL)
	{
	    /* Allocate dynamically allocated members of the gl resource
	     * structure.
	     */

	    /* gl interpritation structure. */
	    glres->glinterprite = V3DGLInterpriteNew();
	}

	return(glres);
}

/*
 *	Allocates a new gl resources structure with the values
 *	initialized with respect to the given V3D model data.
 *
 *	If the given v3d_glinterprite_struct *glinterp is not NULL then
 *	its values will be coppied to the new gl resources structure
 *	and overrides any values defined in the model header items.
 *
 *      The correct gl context must be binded before calling this
 *      function.
 */
v3d_glresource_struct *V3DGLResourceNewFromModelData(
	const v3d_glinterprite_struct *glinterp,
	void **mh_item, int total_mh_items,
	v3d_model_struct **model, int total_models
)
{
	int i, n;
	unsigned int interp_flags;
	int tex_dest_fmt = V3D_TEX_FORMAT_RGBA;
	void *h;
	mh_texture_load_struct *h_texture_load;
	const char *texture_base_dir;
	v3d_glresource_struct *glres = V3DGLResourceNew();


	if(glres == NULL)
	    return(NULL);

	/* Get interpritation flags that define what values that are set
	 * in the gl interpritation structure.
	 */
	interp_flags = (glinterp != NULL) ? glinterp->flags : 0;

	/* Get texture base directory. */
	texture_base_dir = (const char *)V3DMHTextureBaseDirectoryGet(
	    mh_item, total_mh_items
	);
	/* Interpritation structure overrides? */
	if(interp_flags & V3D_GLFLAG_TEXTURE_BASE_DIR)
	    texture_base_dir = glinterp->texture_base_dir;


	/* Load textures by iterating through model header items. */
	for(i = 0; i < total_mh_items; i++)
	{
	    h = mh_item[i];
	    if(h == NULL)
		continue;

	    /* This header item specify a texture load? */
	    if((*(int *)h) == V3DMH_TYPE_TEXTURE_LOAD)
	    {
		v3d_texture_ref_struct *t;
		char tmp_path[PATH_MAX + NAME_MAX];


		h_texture_load = (mh_texture_load_struct *)h;

		if(h_texture_load->path == NULL)
		    continue;

		if(ISPATHABSOLUTE(h_texture_load->path))
		{
		    strncpy(tmp_path, h_texture_load->path, PATH_MAX + NAME_MAX);
		}
                else 
                {
                    const char *cstrptr = PrefixPaths(dname.local_data,h_texture_load->path);
                    struct stat stat_buf;
                    if((cstrptr != NULL) ? stat(cstrptr, &stat_buf) : 1)
                        cstrptr = PrefixPaths(dname.global_data, h_texture_load->path);
                    strncpy(tmp_path,cstrptr,PATH_MAX + NAME_MAX);
                }
		/* else if(texture_base_dir != NULL) */
		/* { */
		/*     const char *cstrptr = (const char *)PrefixPaths( */
		/* 	texture_base_dir, h_texture_load->path */
		/*     ); */
		/*     if(cstrptr == NULL) */
		/* 	continue; */

		/*     strncpy(tmp_path, cstrptr, PATH_MAX + NAME_MAX); */
		/* } */
		/* else */
		/* { */
		/*     strncpy(tmp_path, h_texture_load->path, PATH_MAX + NAME_MAX); */
		/* } */
		tmp_path[PATH_MAX + NAME_MAX - 1] = '\0';


		t = V3DTextureLoadFromFile2D(
		    tmp_path,
		    h_texture_load->name,
		    tex_dest_fmt,
		    NULL, NULL
		);
		if(t != NULL)
		{
		    /* Texture loaded successfully. */

		    /* Set texture priority. */
		    V3DTexturePriority(t, h_texture_load->priority);

		    /* Allocate more texture pointers on GL resource. */
		    n = glres->total_textures;
		    glres->total_textures = n + 1;
		    glres->texture = (v3d_texture_ref_struct **)realloc(
			glres->texture,
			glres->total_textures * sizeof(v3d_texture_ref_struct *)
		    );
		    if(glres->texture == NULL)
		    {
			glres->total_textures = 0;
			V3DTextureDestroy(t);
			break;
		    }

		    glres->texture[n] = t;
		    t = NULL;
		}
	    }
	}


	/* Override any GL interpritation values? */
	if(glinterp != NULL)
	    V3DGLResourceSetInterpritation(glres, glinterp);

	return(glres);
}

/*
 *	Sets the given gl resources structure's gl interpritation
 *	legend to the given glinterp.
 *
 *	Returns non-zero on error.
 */
int V3DGLResourceSetInterpritation(
	v3d_glresource_struct *glres,
	const v3d_glinterprite_struct *glinterp
)
{
	unsigned int flags;
	v3d_glinterprite_struct *t_glinterp;


	if((glres == NULL) || (glinterp == NULL))
	    return(-1);

	flags = glinterp->flags;

	/* Get target gl interpritation structure from the given gl
	 * resource structure.
	 */
	t_glinterp = glres->glinterprite;
	if(t_glinterp == NULL)
	    return(-1);

	if(flags & V3D_GLFLAG_COORDINATE_AXIS)
	    t_glinterp->coordinate_axis = glinterp->coordinate_axis;

	if(flags & V3D_GLFLAG_TEXTURE_KEEP)
	    t_glinterp->texture_keep = glinterp->texture_keep;

	if(flags & V3D_GLFLAG_ALLOW_TRANSLATIONS)
	    t_glinterp->allow_translations = glinterp->allow_translations;

	if(flags & V3D_GLFLAG_ALLOW_ROTATIONS)
	    t_glinterp->allow_rotations = glinterp->allow_rotations;


	if(flags & V3D_GLFLAG_FLIP_WINDING)
	    t_glinterp->flip_winding = glinterp->flip_winding;

	if(flags & V3D_GLFLAG_PASS_NORMALS)
	    t_glinterp->pass_normals = glinterp->pass_normals;

	if(flags & V3D_GLFLAG_UNITLIZE_NORMALS)
	    t_glinterp->unitlize_normals = glinterp->unitlize_normals;

	if(flags & V3D_GLFLAG_PASS_TEXCOORDS)
	    t_glinterp->pass_texcoords = glinterp->pass_texcoords;

	if(flags & V3D_GLFLAG_TEXTURE_NAME_CASE_SENSITIVE)
	    t_glinterp->texture_name_case_sensitive =
		glinterp->texture_name_case_sensitive;

	if(flags & V3D_GLFLAG_MATERIAL_PROPERTIES)
	    t_glinterp->material_properties = glinterp->material_properties;

	if(flags & V3D_GLFLAG_FACES)
	    t_glinterp->faces = glinterp->faces;


	if(flags & V3D_GLFLAG_ENABLE_BLENDING)
	    t_glinterp->enable_blending = glinterp->enable_blending;

	if(flags & V3D_GLFLAG_SET_BLEND_FUNC)
	    t_glinterp->set_blend_func = glinterp->set_blend_func;

	if(flags & V3D_GLFLAG_HEIGHTFIELD_BASE_DIR)
	{
	    const char *src_str = glinterp->heightfield_base_dir;

	    free(t_glinterp->heightfield_base_dir);
	    t_glinterp->heightfield_base_dir = STRDUP(src_str);
	}

	if(flags & V3D_GLFLAG_TEXTURE_BASE_DIR)
	{
	    const char *src_str = glinterp->texture_base_dir;

	    free(t_glinterp->texture_base_dir);
	    t_glinterp->texture_base_dir = STRDUP(src_str);
	}


	/* Update flags on the target glinterp. */
	t_glinterp->flags |= flags;

	return(0);
}

/*
 *	Returns the gl interpritation structure from the given gl
 *	resources structure.
 *
 *	The returned value should not be modified or deallocated.
 *
 *	Can return NULL on error.
 */
v3d_glinterprite_struct *V3DGLResourceGetInterpritation(
	v3d_glresource_struct *glres  
)
{
	return((glres != NULL) ? glres->glinterprite : NULL);
}

/*
 *	Deallocates all resources on the given gl resources
 *	structure.
 *
 *      The correct gl context must be binded before calling this
 *      function.
 */
void V3DGLResourceDelete(v3d_glresource_struct *glres)
{
	int i;

	if(glres == NULL)
	    return;

	/* Unload and deallocate all textures. */
	for(i = 0; i < glres->total_textures; i++)
	    V3DTextureDestroy(glres->texture[i]);
	free(glres->texture);
	glres->texture = NULL;
	glres->total_textures = 0;

	/* Deallocate gl interpritation structure. */
	V3DGLInterpriteDelete(glres->glinterprite);
	glres->glinterprite = NULL;

	/* Deallocate structure itself. */
	free(glres);
}


/*
 *	Sets the given normal with respect to the glinterp values.
 */
static void V3DGLSetNormal(
	v3d_glinterprite_struct *glinterp, mp_vertex_struct *n
)
{
	int coordinate_axis = V3D_GLCOORDINATE_AXIS_SCIENTIFIC;
	int pass_normals = V3D_GLPASS_NORMALS_AS_NEEDED;
	int unitlize_normals = TRUE;
	mp_vertex_struct tn;


	if(n == NULL)
	    return;

	if(glinterp != NULL)
	{
	    if(glinterp->flags & V3D_GLFLAG_COORDINATE_AXIS)
		coordinate_axis = glinterp->coordinate_axis;
	    if(glinterp->flags & V3D_GLFLAG_PASS_NORMALS)
		pass_normals = glinterp->pass_normals;
	    if(glinterp->flags & V3D_GLFLAG_UNITLIZE_NORMALS)
		unitlize_normals = glinterp->unitlize_normals;
	}

	/* Never pass normals? */
	if(pass_normals == V3D_GLPASS_NORMALS_NEVER)
	    return;

	/* Check if normal vector is empty. */
	if((n->x == 0.0) && (n->y == 0.0) && (n->z == 0.0))
	{
	    /* It's empty, check if we should pass empty normal
	     * vectors.
	     */
	    if(pass_normals != V3D_GLPASS_NORMALS_ALWAYS)
		return;
	}

	/* Copy normal to our local target normal for later
	 * manipulation.
	 */
	tn.x = n->x;
	tn.y = n->y;
	tn.z = n->z;

	if(unitlize_normals &&
	   ((tn.x != 0.0) || (tn.y != 0.0) || (tn.z != 0.0))
	)
	{
	    double mag = SQRT(
		(tn.x * tn.x) + (tn.y * tn.y) + (tn.z * tn.z)
	    );
	    if(mag > 0.0)
	    {
		tn.x = tn.x / mag;
		tn.y = tn.y / mag;
		tn.z = tn.z / mag;
	    }
	}

	/* Set by coordinate axis type. */
	switch(coordinate_axis)
	{
	  case V3D_GLCOORDINATE_AXIS_GL:
	    glNormal3d(tn.x, tn.y, tn.z);
	    break;

	  case V3D_GLCOORDINATE_AXIS_SCIENTIFIC:
	    glNormal3d(tn.x, tn.z, -tn.y);
	    break;
	}
}

/*
 *      Sets the given texcoord with respect to the glinterp values.
 */
static void V3DGLSetTexCoord(
	v3d_glinterprite_struct *glinterp, mp_vertex_struct *v,
	mp_vertex_struct *tc, int texture_binded, void *p_texture_orient
)
{
	int coordinate_axis = V3D_GLCOORDINATE_AXIS_SCIENTIFIC;
	int pass_texcoords = V3D_GLPASS_TEXCOORDS_AS_NEEDED;

	if(tc == NULL)
	    return;

	if(glinterp != NULL)
	{
	    if(glinterp->flags & V3D_GLFLAG_COORDINATE_AXIS)
		coordinate_axis = glinterp->coordinate_axis;
	    if(glinterp->flags & V3D_GLFLAG_PASS_TEXCOORDS)
		pass_texcoords = glinterp->pass_texcoords;
	}

	/* Never pass texcoords? */
	if(pass_texcoords == V3D_GLPASS_TEXCOORDS_NEVER)
	    return;

	/* No texture is binded? */
	if(!texture_binded)
	{
	    /* No texture binded, check if we should pass texcoord. */
	    if(pass_texcoords != V3D_GLPASS_TEXCOORDS_ALWAYS)
		return;
	}

	/* Orienting texcoord by plane? */
	if((p_texture_orient != NULL) && (v != NULL))
	{
	    mp_texture_orient_xy_struct *orient_xy;
	    mp_texture_orient_yz_struct *orient_yz;
	    mp_texture_orient_xz_struct *orient_xz;
	    double s, t;

	    switch(V3DMPGetType(p_texture_orient))
	    {
	      case V3DMP_TYPE_TEXTURE_ORIENT_XY:
		orient_xy = (mp_texture_orient_xy_struct *)p_texture_orient;
		if(orient_xy->dx > 0.0)
		    s = (v->x - orient_xy->x) / orient_xy->dx;
		else
		    s = 0.0;
		if(orient_xy->dy > 0.0)
		    t = (v->y - orient_xy->y) / orient_xy->dy;
		else
		    t = 0.0;
		/* V3D texture coordinate origin is upper left while GL
		 * is lower left, remember to flip the t.
		 */
		glTexCoord2d(s, 1.0 - t);
		break;

	      case V3DMP_TYPE_TEXTURE_ORIENT_YZ:
		orient_yz = (mp_texture_orient_yz_struct *)p_texture_orient;
		if(orient_yz->dy > 0.0)
		    s = -(v->y - orient_yz->y) / orient_yz->dy;
		else
		    s = 0.0;
		if(orient_yz->dz > 0.0)
		    t = (v->z - orient_yz->z) / orient_yz->dz;
		else
		    t = 0.0;
		/* V3D texture coordinate origin is upper left while GL
		 * is lower left, remember to flip the t.
		 */
		glTexCoord2d(s, 1.0 - t);
		break;

	      case V3DMP_TYPE_TEXTURE_ORIENT_XZ:
		orient_xz = (mp_texture_orient_xz_struct *)p_texture_orient;
		if(orient_xz->dx > 0.0)
		    s = (v->x - orient_xz->x) / orient_xz->dx;
		else
		    s = 0.0;
		if(orient_xz->dz > 0.0)
		    t = (v->z - orient_xz->z) / orient_xz->dz;
		else
		    t = 0.0;
		/* V3D texture coordinate origin is upper left while GL
		 * is lower left, remember to flip the t.
		 */
		glTexCoord2d(s, 1.0 - t);
		break;
	    }
	}
	/* Set texcoord explicitly by the given tc. */
	else
	{
	    double s = tc->x;
	    double t = tc->y;

	    /* V3D texture coordinate origin is upper left while GL
	     * is lower left, remember to flip the t.
	     */
	    switch(coordinate_axis)
	    {
	      case V3D_GLCOORDINATE_AXIS_GL:
		glTexCoord2d(s, t);
		break;

	      case V3D_GLCOORDINATE_AXIS_SCIENTIFIC:
		glTexCoord2d(s, 1.0 - t);
		break;
	    }   
	}
}

/*
 *	Sets the given vertex with respect to the glinterp values.
 */
static void V3DGLSetVertex(
	v3d_glinterprite_struct *glinterp, mp_vertex_struct *v
)
{
	int coordinate_axis = V3D_GLCOORDINATE_AXIS_SCIENTIFIC;

	if(v == NULL)
	    return;

	if(glinterp != NULL)
	{
	    if(glinterp->flags & V3D_GLFLAG_COORDINATE_AXIS)
		coordinate_axis = glinterp->coordinate_axis;
	}

	switch(coordinate_axis)
	{
	  case V3D_GLCOORDINATE_AXIS_GL:
	    glVertex3d(v->x, v->y, v->z);
	    break;

	  case V3D_GLCOORDINATE_AXIS_SCIENTIFIC:
	    glVertex3d(v->x, v->z, -v->y);
	    break;
	}
}


/*
 *	Called by V3DGLProcessModelExtra() to process a model of type
 *	V3D_MODEL_TYPE_OTHER_DATA.
 *
 *	Given inputs glres, glinterp, and m are assumed valid.
 */
static void V3DGLProcessModelOtherData(
	v3d_glresource_struct *glres, v3d_glinterprite_struct *glinterp,
	v3d_model_struct *m,
	void *client_data,
	void (*extra_cb)(v3d_model_struct *, const char *, void *)
)
{
	int line_num;
	const char *line_ptr;


	for(line_num = 0; line_num < m->total_other_data_lines; line_num++)
	{
	    line_ptr = (const char *)m->other_data_line[line_num];
	    if(line_ptr == NULL)
		continue;

	    if(extra_cb != NULL)
		extra_cb(m, line_ptr, client_data);
	}
}

/*
 *      Called by V3DGLProcessModelExtra() to process a model of type
 *      V3D_MODEL_TYPE_STANDARD.
 *
 *      Given inputs glres, glinterp, and m are assumed valid.
 */
static void V3DGLProcessModelStandard(
	v3d_glresource_struct *glres, v3d_glinterprite_struct *glinterp,
	v3d_model_struct *m,
	void *client_data,
	void (*extra_cb)(v3d_model_struct *, const char *, void *)
)
{       
	int pn, line_num;
	const char *line_ptr;   
	void *p;
	int matrix_level = 0;
	int texture_binded = FALSE;
	void *last_p_texture_orient = NULL;
	int coordinate_axis = V3D_GLCOORDINATE_AXIS_SCIENTIFIC;
	int flip_winding = FALSE;
	int pass_normals = V3D_GLPASS_NORMALS_AS_NEEDED;
	int unitlize_normals = TRUE;
	int pass_texcoords = V3D_GLPASS_TEXCOORDS_AS_NEEDED;
	int texture_name_case_sensitive = FALSE;
	int material_properties = V3D_GLPASS_MATERIAL_PROPERTIES_NEVER;
	int enable_blending = V3D_GLENABLE_BLENDING_NEVER;
	int faces = V3D_GLFACES_FRONT_AND_BACK;
	int blending_enabled = FALSE;
	int set_blend_func = FALSE;
	const char *heightfield_base_dir = NULL;
	const char *texture_base_dir = NULL;

	int last_begin_ptype = -1;

	mp_comment_struct *p_comment;
	mp_translate_struct *p_translate;
	mp_untranslate_struct *p_untranslate;
	mp_rotate_struct *p_rotate;
	mp_unrotate_struct *p_unrotate;
	mp_point_struct *p_point;
	mp_line_struct *p_line;
	mp_line_strip_struct *p_line_strip;
	mp_line_loop_struct *p_line_loop;
	mp_triangle_struct *p_triangle;
	mp_triangle_strip_struct *p_triangle_strip;
	mp_triangle_fan_struct *p_triangle_fan;
	mp_quad_struct *p_quad;
	mp_quad_strip_struct *p_quad_strip;
	mp_polygon_struct *p_polygon;
	mp_color_struct *p_color;
	mp_texture_select_struct *p_texture_select;
	mp_texture_orient_xy_struct *p_texture_orient_xy;
	mp_texture_orient_yz_struct *p_texture_orient_yz;
	mp_texture_orient_xz_struct *p_texture_orient_xz;
	mp_texture_off_struct *p_texture_off;
	mp_heightfield_load_struct *p_heightfield_load;


	/* Get some values GL interpritation structure (if set). */
	if(glinterp->flags & V3D_GLFLAG_COORDINATE_AXIS)
	    coordinate_axis = glinterp->coordinate_axis;
	if(glinterp->flags & V3D_GLFLAG_FLIP_WINDING)
	    flip_winding = glinterp->flip_winding;
	if(glinterp->flags & V3D_GLFLAG_PASS_NORMALS)
	    pass_normals = glinterp->pass_normals;
	if(glinterp->flags & V3D_GLFLAG_UNITLIZE_NORMALS)
	    unitlize_normals = glinterp->unitlize_normals;
	if(glinterp->flags & V3D_GLFLAG_PASS_TEXCOORDS)
	    pass_texcoords = glinterp->pass_texcoords;
	if(glinterp->flags & V3D_GLFLAG_TEXTURE_NAME_CASE_SENSITIVE)
	    texture_name_case_sensitive = glinterp->texture_name_case_sensitive;
	if(glinterp->flags & V3D_GLFLAG_MATERIAL_PROPERTIES)
	    material_properties = glinterp->material_properties;
	if(glinterp->flags & V3D_GLFLAG_FACES)
	    faces = glinterp->faces;
	if(glinterp->flags & V3D_GLFLAG_ENABLE_BLENDING)
	    enable_blending = glinterp->enable_blending;
	if(glinterp->flags & V3D_GLFLAG_SET_BLEND_FUNC)
	    set_blend_func = glinterp->set_blend_func;
	if(glinterp->flags & V3D_GLFLAG_HEIGHTFIELD_BASE_DIR)
	    heightfield_base_dir = glinterp->heightfield_base_dir;
	if(glinterp->flags & V3D_GLFLAG_TEXTURE_BASE_DIR)
	    texture_base_dir = glinterp->texture_base_dir;


	/* Iterate through each primitive on the given editor. */
	for(pn = 0; pn < m->total_primitives; pn++)
	{
	    p = m->primitive[pn];
	    if(p == NULL)
		continue;

	    /* Call glEnd() if last_begin_ptype does not match the
	     * new primitive type.
	     */
	    if((last_begin_ptype != V3DMPGetType(p)) &&
	       (last_begin_ptype > -1)
	    )
	    {
		glEnd();
		last_begin_ptype = -1;
	    }


	    /* Handle by primitive type. */
	    switch(V3DMPGetType(p))
	    {
	      case V3DMP_TYPE_COMMENT:
		p_comment = (mp_comment_struct *)p;
		for(line_num = 0; line_num < p_comment->total_lines; line_num++)
		{
		    line_ptr = (const char *)p_comment->line[line_num];
		    if(line_ptr == NULL)
			continue;

		    if(extra_cb != NULL)
			extra_cb(m, line_ptr, client_data);
		}
		break;

	      case V3DMP_TYPE_TRANSLATE:
		p_translate = (mp_translate_struct *)p;
		if(glinterp->flags & V3D_GLFLAG_ALLOW_TRANSLATIONS)
		{
		    if(!glinterp->allow_translations)
			break;
		}
		glPushMatrix();
		matrix_level++;

		switch(coordinate_axis)
		{
		  case V3D_GLCOORDINATE_AXIS_GL:
		    glTranslated(
			p_translate->x, p_translate->y, p_translate->z
		    );
		    break;

		  case V3D_GLCOORDINATE_AXIS_SCIENTIFIC:
		    glTranslated(
			p_translate->x, p_translate->z, -p_translate->y
		    );
		    break;
		}
		break;

	      case V3DMP_TYPE_UNTRANSLATE:
		p_untranslate = (mp_untranslate_struct *)p;
		if(glinterp->flags & V3D_GLFLAG_ALLOW_TRANSLATIONS)
		{
		    if(!glinterp->allow_translations)
			break;
		}
		glPopMatrix();
		matrix_level--;
		break;

	      case V3DMP_TYPE_ROTATE:
		p_rotate = (mp_rotate_struct *)p;
		if(glinterp->flags & V3D_GLFLAG_ALLOW_ROTATIONS)
		{
		    if(!glinterp->allow_rotations)
			break;
		}
		glPushMatrix();
		matrix_level++;

		switch(coordinate_axis)
		{
		  case V3D_GLCOORDINATE_AXIS_GL:
		    glRotated(
			-RADTODEG(p_rotate->heading),
			0.0, 0.0, 1.0
		    );
		    glRotated(
			-RADTODEG(p_rotate->pitch),
			1.0, 0.0, 0.0
		    );
		    glRotated(
		       -RADTODEG(p_rotate->bank),
			0.0, 1.0, 0.0
		    );
		    break;

		  case V3D_GLCOORDINATE_AXIS_SCIENTIFIC:
		    glRotated(
			-RADTODEG(p_rotate->heading),
			0.0, 1.0, 0.0
		    );
		    glRotated(
			-RADTODEG(p_rotate->pitch),
			1.0, 0.0, 0.0
		    );
		    glRotated(
		       -RADTODEG(p_rotate->bank),
			0.0, 0.0, 1.0
		    );
		    break;
		}
		break;

	      case V3DMP_TYPE_UNROTATE:
		p_unrotate = (mp_unrotate_struct *)p;
		if(glinterp->flags & V3D_GLFLAG_ALLOW_ROTATIONS)
		{
		    if(!glinterp->allow_rotations)
			break;
		}
		glPopMatrix();
		matrix_level--;
		break;

#define DO_SET_VERTEX_DYNAMIC	\
{ \
 int i; \
 mp_vertex_struct *v, *n, *tc; \
 mp_vertex_struct *fbn = NULL;	/* Fallback normal vector. */ \
\
 /* Look for first non-empty fallback normal vector. */ \
 if(pass_normals != V3D_GLPASS_NORMALS_NEVER) \
 { \
  for(i = 0; i < V_TOTAL; i++) \
  { \
   n = P_PTR->n[i]; \
   if(n != NULL) \
   { \
    if((n->x != 0.0) || (n->y != 0.0) || (n->z != 0.0)) \
     fbn = n; \
   } \
  } \
 } \
\
 if(flip_winding) \
 { \
  /* Check if we need to set the fallback normal for the first \
   * vertex. \
   */ \
  if((pass_normals != V3D_GLPASS_NORMALS_NEVER) && (V_TOTAL > 0)) \
  { \
   n = P_PTR->n[V_TOTAL - 1]; \
   if(n != NULL) \
   { \
    if((n->x == 0.0) && (n->y == 0.0) && (n->z == 0.0)) \
     V3DGLSetNormal(glinterp, fbn); \
   } \
  } \
 \
  for(i = V_TOTAL - 1; i >= 0; i--) \
  { \
   v = P_PTR->v[i]; \
   n = P_PTR->n[i]; \
   tc = P_PTR->tc[i]; \
   V3DGLSetNormal(glinterp, n); \
   V3DGLSetTexCoord(glinterp, v, tc, texture_binded, last_p_texture_orient); \
   V3DGLSetVertex(glinterp, v); \
  } \
 } \
 else \
 { \
  /* Check if we need to set the fallback normal for the first \
   * vertex. \
   */ \
  if((pass_normals != V3D_GLPASS_NORMALS_NEVER) && (V_TOTAL > 0)) \
  { \
   n = P_PTR->n[0]; \
   if(n != NULL) \
   { \
    if((n->x == 0.0) && (n->y == 0.0) && (n->z == 0.0)) \
     V3DGLSetNormal(glinterp, fbn); \
   } \
  } \
 \
  for(i = 0; i < V_TOTAL; i++) \
  { \
   v = P_PTR->v[i]; \
   n = P_PTR->n[i]; \
   tc = P_PTR->tc[i]; \
   V3DGLSetNormal(glinterp, n); \
   V3DGLSetTexCoord(glinterp, v, tc, texture_binded, last_p_texture_orient); \
   V3DGLSetVertex(glinterp, v); \
  } \
 } \
}

#define DO_SET_VERTEX_STATIC	\
{ \
 int i; \
 mp_vertex_struct *v, *n, *tc; \
 mp_vertex_struct *fbn = NULL; /* Fallback normal vector. */ \
\
 /* Look for first non-empty fallback normal vector. */ \
 if(pass_normals != V3D_GLPASS_NORMALS_NEVER) \
 { \
  for(i = 0; i < V_TOTAL; i++) \
  { \
   n = &P_PTR->n[i]; \
   if(n != NULL) \
   { \
    if((n->x != 0.0) || (n->y != 0.0) || (n->z != 0.0)) \
     fbn = n; \
   } \
  } \
 } \
\
 if(flip_winding) \
 { \
  /* Check if we need to set the fallback normal for the first \
   * vertex. \
   */ \
  if(pass_normals != V3D_GLPASS_NORMALS_NEVER) \
  { \
   n = &P_PTR->n[V_TOTAL - 1]; \
   if((n->x == 0.0) && (n->y == 0.0) && (n->z == 0.0)) \
    V3DGLSetNormal(glinterp, fbn); \
  } \
 \
  for(i = V_TOTAL - 1; i >= 0; i--) \
  { \
   v = &P_PTR->v[i]; \
   n = &P_PTR->n[i]; \
   tc = &P_PTR->tc[i]; \
   V3DGLSetNormal(glinterp, n); \
   V3DGLSetTexCoord(glinterp, v, tc, texture_binded, last_p_texture_orient); \
   V3DGLSetVertex(glinterp, v); \
  } \
 } \
 else \
 { \
  /* Check if we need to set the fallback normal for the first \
   * vertex. \
   */ \
  if(pass_normals != V3D_GLPASS_NORMALS_NEVER) \
  { \
   n = &P_PTR->n[0]; \
   if((n->x == 0.0) && (n->y == 0.0) && (n->z == 0.0)) \
    V3DGLSetNormal(glinterp, fbn); \
  } \
 \
  for(i = 0; i < V_TOTAL; i++) \
  { \
   v = &P_PTR->v[i]; \
   n = &P_PTR->n[i]; \
   tc = &P_PTR->tc[i]; \
   V3DGLSetNormal(glinterp, n); \
   V3DGLSetTexCoord(glinterp, v, tc, texture_binded, last_p_texture_orient); \
   V3DGLSetVertex(glinterp, v); \
  } \
 } \
}

	      case V3DMP_TYPE_POINT:
#define P_PTR	p_point
#define V_TOTAL	V3DMP_POINT_NVERTEX
		P_PTR = (mp_point_struct *)p;
		if(last_begin_ptype != V3DMP_TYPE_POINT)
		    glBegin(GL_POINTS);
		DO_SET_VERTEX_STATIC
		last_begin_ptype = V3DMP_TYPE_POINT;
#undef P_PTR
#undef V_TOTAL
		break;

	      case V3DMP_TYPE_LINE:
#define P_PTR   p_line
#define V_TOTAL V3DMP_LINE_NVERTEX
		P_PTR = (mp_line_struct *)p;
		if(last_begin_ptype != V3DMP_TYPE_LINE)
		    glBegin(GL_LINES);
		DO_SET_VERTEX_STATIC
		last_begin_ptype = V3DMP_TYPE_LINE;
#undef P_PTR
#undef V_TOTAL
		break;

	      case V3DMP_TYPE_LINE_STRIP:
#define P_PTR   p_line_strip
#define V_TOTAL P_PTR->total
		P_PTR = (mp_line_strip_struct *)p;
		glBegin(GL_LINE_STRIP);
		DO_SET_VERTEX_DYNAMIC
		glEnd();
#undef P_PTR
#undef V_TOTAL
		break;

	      case V3DMP_TYPE_LINE_LOOP:
#define P_PTR   p_line_loop
#define V_TOTAL P_PTR->total
		P_PTR = (mp_line_loop_struct *)p;
		glBegin(GL_LINE_LOOP);
		DO_SET_VERTEX_DYNAMIC
		glEnd();
#undef P_PTR
#undef V_TOTAL
		break;

	      case V3DMP_TYPE_TRIANGLE:
#define P_PTR   p_triangle
#define V_TOTAL V3DMP_TRIANGLE_NVERTEX
		P_PTR = (mp_triangle_struct *)p;
		if(last_begin_ptype != V3DMP_TYPE_TRIANGLE)
		    glBegin(GL_TRIANGLES);
		DO_SET_VERTEX_STATIC
		last_begin_ptype = V3DMP_TYPE_TRIANGLE;
#undef P_PTR
#undef V_TOTAL
		break;

	      case V3DMP_TYPE_TRIANGLE_STRIP:
#define P_PTR   p_triangle_strip
#define V_TOTAL P_PTR->total
		P_PTR = (mp_triangle_strip_struct *)p;
		glBegin(GL_TRIANGLE_STRIP);
		DO_SET_VERTEX_DYNAMIC
		glEnd();
#undef P_PTR
#undef V_TOTAL
		break;

	      case V3DMP_TYPE_TRIANGLE_FAN:
#define P_PTR   p_triangle_fan
#define V_TOTAL P_PTR->total
		P_PTR = (mp_triangle_fan_struct *)p;
		glBegin(GL_TRIANGLE_FAN);
		DO_SET_VERTEX_DYNAMIC
		glEnd();
#undef P_PTR
#undef V_TOTAL
		break;

	      case V3DMP_TYPE_QUAD:
#define P_PTR   p_quad
#define V_TOTAL V3DMP_QUAD_NVERTEX
		P_PTR = (mp_quad_struct *)p;
		if(last_begin_ptype != V3DMP_TYPE_QUAD)
		    glBegin(GL_QUADS);
		DO_SET_VERTEX_STATIC
		last_begin_ptype = V3DMP_TYPE_QUAD;
#undef P_PTR
#undef V_TOTAL
		break;

	      case V3DMP_TYPE_QUAD_STRIP:
#define P_PTR   p_quad_strip
#define V_TOTAL P_PTR->total
		P_PTR = (mp_quad_strip_struct *)p;
		glBegin(GL_QUAD_STRIP);
		DO_SET_VERTEX_DYNAMIC
		glEnd();
#undef P_PTR
#undef V_TOTAL
		break;

	      case V3DMP_TYPE_POLYGON:
#define P_PTR   p_polygon
#define V_TOTAL P_PTR->total
		P_PTR = (mp_polygon_struct *)p;
		glBegin(GL_POLYGON);
		DO_SET_VERTEX_DYNAMIC
		glEnd();
#undef P_PTR
#undef V_TOTAL
		break;

#undef DO_SET_VERTEX_STATIC
#undef DO_SET_VERTEX_DYNAMIC


	      case V3DMP_TYPE_COLOR:
		p_color = (mp_color_struct *)p;
		if(p_color->a < 1.0)
		{
		    if(enable_blending)
		    {
			if(!blending_enabled)
			{
			    glEnable(GL_BLEND);
			    glDisable(GL_ALPHA_TEST);
			    blending_enabled = TRUE;
			}
		    }
		    if(set_blend_func)
		    {
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		    }
		}
		else
		{
		    if(enable_blending)
		    {
			if(blending_enabled)
			{   
			    glDisable(GL_BLEND);
			    glEnable(GL_ALPHA_TEST);
			    blending_enabled = FALSE;
			}
		    }
		}
		/* Set color? */
		if(material_properties != V3D_GLPASS_MATERIAL_PROPERTIES_INSTEAD_COLOR)
		{
		    glColor4d(p_color->r, p_color->g, p_color->b, p_color->a);
		}
		/* Set material properties? */
		if(material_properties != V3D_GLPASS_MATERIAL_PROPERTIES_NEVER)
		{
		    GLenum glface;
		    GLfloat glparam[4];

		    switch(faces)
		    {
		      case V3D_GLFACES_FRONT_AND_BACK:
			glface = GL_FRONT_AND_BACK;
			break;
		      case V3D_GLFACES_BACK:
			glface = GL_BACK;
			break;
		      default:	/* V3D_GLFACES_FRONT */
			glface = GL_FRONT;
			break;
		    }

		    glparam[0] = (GLfloat)(p_color->ambient * p_color->r);
		    glparam[1] = (GLfloat)(p_color->ambient * p_color->g);
		    glparam[2] = (GLfloat)(p_color->ambient * p_color->b);
		    glparam[3] = (GLfloat)(p_color->a);
		    glMaterialfv(glface, GL_AMBIENT, glparam);

		    glparam[0] = (GLfloat)(p_color->diffuse * p_color->r);
		    glparam[1] = (GLfloat)(p_color->diffuse * p_color->g);
		    glparam[2] = (GLfloat)(p_color->diffuse * p_color->b);
		    glparam[3] = (GLfloat)(p_color->a);
		    glMaterialfv(glface, GL_DIFFUSE, glparam);

		    glparam[0] = (GLfloat)(p_color->specular * p_color->r);
		    glparam[1] = (GLfloat)(p_color->specular * p_color->g);
		    glparam[2] = (GLfloat)(p_color->specular * p_color->b);
		    glparam[3] = (GLfloat)(p_color->a);
		    glMaterialfv(glface, GL_SPECULAR, glparam);

		    glparam[0] = (GLfloat)(p_color->shininess * 128.0);
		    glMaterialf(glface, GL_SHININESS, glparam[0]);
 
		    glparam[0] = (GLfloat)(p_color->emission * p_color->r);
		    glparam[1] = (GLfloat)(p_color->emission * p_color->g);
		    glparam[2] = (GLfloat)(p_color->emission * p_color->b);
		    glparam[3] = (GLfloat)(p_color->a);
		    glMaterialfv(glface, GL_EMISSION, glparam);
		}
		break;

	      case V3DMP_TYPE_TEXTURE_SELECT:
		p_texture_select = (mp_texture_select_struct *)p;
		if(p_texture_select->name != NULL)
		{
		    const char *tex_name = (const char *)p_texture_select->name;
		    int tn;
		    v3d_texture_ref_struct *t;

		    /* Previous last_p_texture_orient gets reset when
		     * a new texture is binded.
		     */
		    last_p_texture_orient = NULL;

		    /* Iterate through each loaded texture on the 
		     * gl resources structure.
		     */
		    for(tn = 0; tn < glres->total_textures; tn++)
		    {
			t = glres->texture[tn];
			if((t == NULL) ? 1 : (t->name == NULL))
			    continue;

			if(texture_name_case_sensitive)
			{
			    if(!strcmp(t->name, tex_name))
				break;
			}
			else
			{
			    if(!strcasecmp(t->name, tex_name))
				break;
			}
		    }
		    /* Matched texture? */
		    if(tn < glres->total_textures)
			t = glres->texture[tn];
		    else
			t = NULL;

		    if(t == NULL)
		    {
			/* No texture, unbind any existing texture. */
			V3DTextureSelect(NULL);
			texture_binded = FALSE;
		    }
		    else
		    {
			V3DTextureSelect(t);
			texture_binded = TRUE;
		    }
		}
		break;

	      case V3DMP_TYPE_TEXTURE_ORIENT_XY:
		p_texture_orient_xy = (mp_texture_orient_xy_struct *)p;
		if(texture_binded)
		{
		    last_p_texture_orient = p;
		}
		break;

	      case V3DMP_TYPE_TEXTURE_ORIENT_YZ:
		p_texture_orient_yz = (mp_texture_orient_yz_struct *)p;
		if(texture_binded)
		{
		    last_p_texture_orient = p;
		}
		break;

	      case V3DMP_TYPE_TEXTURE_ORIENT_XZ:
		p_texture_orient_xz = (mp_texture_orient_xz_struct *)p;
		if(texture_binded)
		{
		    last_p_texture_orient = p;
		}
		break;

	      case V3DMP_TYPE_TEXTURE_OFF:
		p_texture_off = (mp_texture_off_struct *)p;
		V3DTextureSelect(NULL);
		texture_binded = FALSE;
		last_p_texture_orient = NULL;
		break;

	      case V3DMP_TYPE_HEIGHTFIELD_LOAD:
		p_heightfield_load = (mp_heightfield_load_struct *)p;
		if(p_heightfield_load->path != NULL)
		{
		    int status, widthp, heightp;
		    double x_spacing, y_spacing;
		    const char *cstrptr;
		    v3d_hf_options_struct hfopt;
		    char tmp_path[PATH_MAX + NAME_MAX];

		    if(ISPATHABSOLUTE(p_heightfield_load->path))
		    {
			strncpy(tmp_path, p_heightfield_load->path, PATH_MAX + NAME_MAX);
		    }
		    else if(heightfield_base_dir != NULL)
		    {
			cstrptr = (const char *)PrefixPaths(
			    heightfield_base_dir, p_heightfield_load->path
			);
			strncpy(
			    tmp_path,
			    (cstrptr == NULL) ? p_heightfield_load->path : cstrptr,
			    PATH_MAX + NAME_MAX
			);
		    }
		    else
		    {
			strncpy(tmp_path, p_heightfield_load->path, PATH_MAX + NAME_MAX);
		    }

		    glPushMatrix();
		    switch(coordinate_axis)
		    {
		      case V3D_GLCOORDINATE_AXIS_GL:
			glTranslated(
			    p_heightfield_load->x,
			    p_heightfield_load->y,
			    p_heightfield_load->z
			);
			glRotated(
			    -RADTODEG(p_heightfield_load->heading),
			    0.0, 0.0, 1.0 
			);
			glRotated(
			    -RADTODEG(p_heightfield_load->pitch),  
			    1.0, 0.0, 0.0
			);
			glRotated(
			    -RADTODEG(p_heightfield_load->bank),
			    0.0, 1.0, 0.0
			);
			break;

		      case V3D_GLCOORDINATE_AXIS_SCIENTIFIC:
			glTranslated(
			    p_heightfield_load->x, 
			    p_heightfield_load->z,
			    -p_heightfield_load->y
			);
			glRotated(
			    -RADTODEG(p_heightfield_load->heading),
			    0.0, 1.0, 0.0
			);
			glRotated(
			    -RADTODEG(p_heightfield_load->pitch),
			    1.0, 0.0, 0.0
			);
			glRotated(
			    -RADTODEG(p_heightfield_load->bank),
			    0.0, 0.0, 1.0
			);
			break;
		    }

		    /* Set up heightfield options. */
		    hfopt.flags = (V3D_HF_OPT_FLAG_WINDING |
			V3D_HF_OPT_FLAG_SET_NORMAL | V3D_HF_OPT_FLAG_SET_TEXCOORD
		    );
		    hfopt.winding = ((flip_winding) ?
			V3D_HF_WIND_CCW : V3D_HF_WIND_CW
		    );
		    hfopt.set_normal = ((pass_normals != V3D_GLPASS_NORMALS_NEVER) ?
			V3D_HF_SET_NORMAL_AVERAGED : V3D_HF_SET_NORMAL_NEVER
		    );
		    hfopt.set_texcoord = ((pass_texcoords != V3D_GLPASS_TEXCOORDS_NEVER) ?
			V3D_HF_SET_TEXCOORD_ALWAYS : V3D_HF_SET_TEXCOORD_NEVER
		    );

		    /* Load heightfield from image file and generate GL
		     * commands.
		     */
		    status = V3DHFLoadFromFile(
			tmp_path,
			p_heightfield_load->x_length, 
			p_heightfield_load->y_length,
			p_heightfield_load->z_length,
			&widthp, &heightp,
			&x_spacing, &y_spacing,
			NULL,		/* No allocated z points. */
			0,		/* GL list not important. */
			&hfopt
		    );
		    glPopMatrix();
		}
		break;
	    }
	}

	/* Need to call glEnd() if last_begin_ptype is not -1. */
	if(last_begin_ptype > -1)
	{
	    glEnd();
	    last_begin_ptype = -1;
	}

	/* Pop any extranous matrixes */
	while(matrix_level > 0)
	{
	    glPopMatrix();
	    matrix_level--;
	}

	/* Unbind any textures */
	if(texture_binded)
	    V3DTextureSelect(NULL);

	/* Restore blend state */
	if(blending_enabled)
	{
	    glDisable(GL_BLEND);
	    glEnable(GL_ALPHA_TEST);
	    blending_enabled = FALSE;
	}
}

/*
 *	Interprites the given V3D model to GL commands.
 *
 *	The correct GL context must be binded before calling this 
 *	function.
 */
void V3DGLProcessModelExtra(
	v3d_glresource_struct *glres,
	v3d_model_struct *m,
	void *client_data,
	void (*extra_cb)(v3d_model_struct *, const char *, void *)
)
{
	v3d_glinterprite_struct *glinterp;

	/* No model to process? */
	if(m == NULL)
	    return;

	/* Can't process without a GL resources structure. */
	if(glres == NULL)
	    return;

	glinterp = glres->glinterprite;
	if(glinterp == NULL)
	    return;

	/* Handle by model type. */
	switch(V3DModelGetType(m))
	{
	  case V3D_MODEL_TYPE_OTHER_DATA:
	    V3DGLProcessModelOtherData(
		glres, glinterp, m, client_data, extra_cb
	    ); 
	    break;

	  case V3D_MODEL_TYPE_STANDARD:
	    V3DGLProcessModelStandard(
		glres, glinterp, m, client_data, extra_cb
	    );
	    break;
	}
}

/*
 *	Simplified form of V3DGLProcessModelExtra() without the
 *	extranous data callback.
 */
void V3DGLProcessModel(
	v3d_glresource_struct *glres,
	v3d_model_struct *m
)
{
	V3DGLProcessModelExtra(glres, m, NULL, NULL);
}
