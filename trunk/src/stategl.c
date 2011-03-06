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
#include <sys/types.h>

#include "../include/os.h"
#ifdef __MSW__
# include <windows.h>
#endif
#include <GL/gl.h>

#include "stategl.h"


void StateGLEnableF(state_gl_struct *s, GLenum cap, GLboolean force);
void StateGLEnable(state_gl_struct *s, GLenum cap);
void StateGLDisableF(state_gl_struct *s, GLenum cap, GLboolean force);
void StateGLDisable(state_gl_struct *s, GLenum cap);

void StateGLAlphaFunc(
	state_gl_struct *s,
	GLenum func, GLclampf ref
);
void StateGLBlendFunc(
	state_gl_struct *s,
	GLenum sfactor, GLenum dfactor
);
void StateGLColorMaterial(
	state_gl_struct *gl_state,
	GLenum face, GLenum mode
);
void StateGLDepthFunc(
	state_gl_struct *s, GLenum func
);
void StateGLDepthMask(
	state_gl_struct *s, GLboolean flag
);
void StateGLFrontFace(
	state_gl_struct *s, GLenum mode   
);
void StateGLLineWidth(
	state_gl_struct *s, GLfloat width
);
void StateGLPointSize(
	state_gl_struct *s, GLfloat size
);
void StateGLPolygonOffset(
	state_gl_struct *s, GLfloat factor, GLfloat units
);
void StateGLScissor(
	state_gl_struct *s, GLint x, GLint y, GLsizei width, GLsizei height
);
void StateGLShadeModel(
	state_gl_struct *s, GLenum mode
);
void StateGLStencilFunc(
	state_gl_struct *s, GLenum func, GLint ref, GLuint mask
);
void StateGLStencilOp(
	state_gl_struct *s, GLenum fail, GLenum zfail, GLenum zpass
);
void StateGLTexEnvI(
	state_gl_struct *s,
	GLenum target, GLenum pname, GLint param
);

void StateGLResetAll(state_gl_struct *s);



/*
 *	Enables the OpenGL state specified by cap as needed
 *
 *	If s is NULL then the state will be explicitly enabled
 */
void StateGLEnableF(state_gl_struct *s, GLenum cap, GLboolean force)
{
#if 0
	if(s == NULL)
	{
	    glEnable(cap);
	    return;
	}
#endif
	switch(cap)
	{
	  case GL_ALPHA_TEST:
	    if(!s->alpha_test || force)
	    {
		glEnable(cap);
		s->alpha_test = True;
	    }
	    break;

	  case GL_BLEND:
	    if(!s->blend || force)
	    {
		glEnable(cap);
		s->blend = True;
	    }
	    break;

	  case GL_COLOR_MATERIAL:
	    if(!s->color_material || force)
	    {
		glEnable(cap);
		s->color_material = True;
	    }
	    break;

	  case GL_CULL_FACE:
	    if(!s->cull_face || force)
	    {
		glEnable(cap);
		s->cull_face = True;
	    }
	    break;

	  case GL_DEPTH_TEST:
	    if(!s->depth_test || force)
	    {
		glEnable(cap);
		s->depth_test = True;
	    }
	    break;

	  case GL_DITHER:
	    if(!s->dither || force)
	    {
		glEnable(cap);
		s->dither = True;
	    }
	    break;

	  case GL_FOG:
	    if(!s->fog || force)
	    {
		glEnable(cap);
		s->fog = True;
	    }
	    break;

	  case GL_LIGHTING:
	    if(!s->lighting || force)
	    {
		glEnable(cap);
		s->lighting = True;
	    }
	    break;

	  case GL_LIGHT0:
	    if(!s->light0 || force)
	    {
		glEnable(cap);
		s->light0 = True;
	    }
	    break;

	  case GL_LIGHT1:
	    if(!s->light1 || force)
	    {
		glEnable(cap);
		s->light1 = True;
	    }
	    break;

	  case GL_LIGHT2:
	    if(!s->light2 || force)
	    {
		glEnable(cap);
		s->light2 = True;
	    }
	    break;

	  case GL_LIGHT3:
	    if(!s->light3 || force)
	    {
		glEnable(cap);
		s->light3 = True;
	    }
	    break;

	  case GL_LIGHT4:
	    if(!s->light4 || force)
	    {
		glEnable(cap);
		s->light4 = True;
	    }
	    break;

	  case GL_LIGHT5:
	    if(!s->light5 || force)
	    {
		glEnable(cap);
		s->light5 = True;
	    }
	    break;

	  case GL_LIGHT6:
	    if(!s->light6 || force)
	    {
		glEnable(cap);
		s->light6 = True;
	    }
	    break;

	  case GL_LIGHT7:
	    if(!s->light7 || force)
	    {
		glEnable(cap);
		s->light7 = True;
	    }
	    break;

	  case GL_LINE_SMOOTH:
	    if(!s->line_smooth || force)
	    {
		glEnable(cap);
		s->line_smooth = True;
	    }
	    break;

	  case GL_POINT_SMOOTH:
	    if(!s->point_smooth || force)
	    {
		glEnable(cap);
		s->point_smooth = True;
	    }
	    break;

	  case GL_POLYGON_OFFSET_FILL:
	    if(!s->polygon_offset_fill || force)
	    {
		glEnable(cap);
		s->polygon_offset_fill = True;
	    }
	    break;

	  case GL_POLYGON_OFFSET_LINE:
	    if(!s->polygon_offset_line || force)
	    {
		glEnable(cap);
		s->polygon_offset_line = True;
	    }
	    break;

	  case GL_POLYGON_OFFSET_POINT:
	    if(!s->polygon_offset_point || force)
	    {
		glEnable(cap);
		s->polygon_offset_point = True;
	    }
	    break;

	  case GL_SCISSOR_TEST:
	    if(!s->scissor_test || force)
	    {
		glEnable(cap);
		s->scissor_test = True;
	    }
	    break;

	  case GL_STENCIL_TEST:
	    if(!s->stencil_test || force)
	    {
		glEnable(cap);
		s->stencil_test = True;
	    }
	    break;
#if defined(GL_TEXTURE_1D)
	  case GL_TEXTURE_1D:
	    if(!s->texture_1d || force)
	    {
		glEnable(cap);
		s->texture_1d = True;
	    }
	    break;
#endif
#if defined(GL_TEXTURE_2D)
	  case GL_TEXTURE_2D:
	    if(!s->texture_2d || force)
	    {
		glEnable(cap);
		s->texture_2d = True;
	    }
	    break;
#endif
#if defined(GL_TEXTURE_3D)
	  case GL_TEXTURE_3D:
	    if(!s->texture_3d || force)
	    {
		glEnable(cap);  
		s->texture_3d = True;
	    }
	    break;
#endif
	  default:
	    fprintf(
		stderr,
		"Cannot glEnable() unsupported cap %i.\n",
		cap
	    );
	    break;
	}
}

void StateGLEnable(state_gl_struct *s, GLenum cap)
{
	StateGLEnableF(s, cap, GL_FALSE);
}

/*
 *	Disables the OpenGL state specified by cap as needed
 *
 *	If s is NULL then the state will be explicitly disabled
 */
void StateGLDisableF(state_gl_struct *s, GLenum cap, GLboolean force)
{
#if 0
	if(s == NULL)
	{
	    glDisable(cap);
	    return;
	}
#endif
	switch(cap)
	{
	  case GL_ALPHA_TEST:
	    if(s->alpha_test || force)
	    {
		glDisable(cap);
		s->alpha_test = False;
	    }
	    break;

	  case GL_BLEND:
	    if(s->blend || force)
	    {
		glDisable(cap);
		s->blend = False;
	    }
	    break;

	  case GL_COLOR_MATERIAL:
	    if(s->color_material || force)
	    {
		glDisable(cap);
		s->color_material = False;
	    }
	    break;

	  case GL_CULL_FACE:
	    if(s->cull_face || force)
	    {
		glDisable(cap);
		s->cull_face = False;
	    }
	    break;

	  case GL_DEPTH_TEST:
	    if(s->depth_test || force)
	    {
		glDisable(cap);
		s->depth_test = False;
	    }
	    break;

	  case GL_DITHER:
	    if(s->dither || force)
	    {
		glEnable(cap);
		s->dither = False;
	    }
	    break;

	  case GL_FOG:
	    if(s->fog || force)
	    {
		glDisable(cap);
		s->fog = False;
	    }
	    break;

	  case GL_LIGHTING:
	    if(s->lighting || force)
	    {
		glDisable(cap);
		s->lighting = False;
	    }
	    break;

	  case GL_LIGHT0:
	    if(s->light0 || force)
	    {
		glDisable(cap);
		s->light0 = False;
	    }
	    break;

	  case GL_LIGHT1:
	    if(s->light1 || force)
	    {
		glDisable(cap);
		s->light1 = False;
	    }
	    break;

	  case GL_LIGHT2:
	    if(s->light2 || force)
	    {
		glDisable(cap);
		s->light2 = False;
	    }
	    break;

	  case GL_LIGHT3:
	    if(s->light3 || force)
	    {
		glDisable(cap);
		s->light3 = False;
	    }
	    break;

	  case GL_LIGHT4:
	    if(s->light4 || force)
	    {
		glDisable(cap);
		s->light4 = False;
	    }
	    break;

	  case GL_LIGHT5:
	    if(s->light5 || force)
	    {
		glDisable(cap);
		s->light5 = False;
	    }
	    break;

	  case GL_LIGHT6:
	    if(s->light6 || force)
	    {
		glDisable(cap);
		s->light6 = False;
	    }
	    break;

	  case GL_LIGHT7:
	    if(s->light7 || force)
	    {
		glDisable(cap);
		s->light7 = False;
	    }
	    break;

	  case GL_LINE_SMOOTH:
	    if(s->line_smooth || force)
	    {
		glDisable(cap);
		s->line_smooth = False;
	    }
	    break;

	  case GL_POINT_SMOOTH:
	    if(s->point_smooth || force)
	    {
		glDisable(cap);
		s->point_smooth = False;
	    }
	    break;

	  case GL_POLYGON_OFFSET_FILL:
	    if(s->polygon_offset_fill || force)
	    {
		glDisable(cap);
		s->polygon_offset_fill = False;
	    }
	    break;

	  case GL_POLYGON_OFFSET_LINE:
	    if(s->polygon_offset_line || force)
	    {
		glDisable(cap);
		s->polygon_offset_line = False;
	    }
	    break;

	  case GL_POLYGON_OFFSET_POINT:
	    if(s->polygon_offset_point || force)
	    {
		glDisable(cap);
		s->polygon_offset_point = False;
	    }
	    break;

	  case GL_SCISSOR_TEST:
	    if(s->scissor_test || force)
	    {
		glDisable(cap);
		s->scissor_test = False;
	    }
	    break;

	  case GL_STENCIL_TEST:
	    if(s->stencil_test || force)
	    {
		glDisable(cap);
		s->stencil_test = False;
	    }
	    break;

#if defined(GL_TEXTURE_1D)
	  case GL_TEXTURE_1D:
	    if(s->texture_1d || force)
	    {
		glDisable(cap);
		s->texture_1d = False;
	    }
	    break;
#endif
#if defined(GL_TEXTURE_2D)
	  case GL_TEXTURE_2D:
	    if(s->texture_2d || force)
	    {
		glDisable(cap);
		s->texture_2d = False;
	    }
	    break;
#endif
#if defined(GL_TEXTURE_3D)
	  case GL_TEXTURE_3D:
	    if(s->texture_3d || force)
	    {
		glDisable(cap);
		s->texture_3d = False;
	    }
	    break;
#endif

	  default:
	    fprintf(
		stderr,
		"Cannot glDisable() unsupported cap %i.\n",
		cap
	    );
	    break;
	}
}

void StateGLDisable(state_gl_struct *s, GLenum cap)
{
	StateGLDisableF(s, cap, GL_FALSE);
}

/*
 *	Sets the alpha function
 *
 *	If s is NULL then values will be set explicitly
 */
void StateGLAlphaFunc(
	state_gl_struct *s,
	GLenum func, GLclampf ref
)
{
	if(s != NULL)
	{
	    if((s->alpha_func_func != func) ||
	       (s->alpha_func_ref != ref)
	    )
	    {
		s->alpha_func_func = func;
		s->alpha_func_ref = ref;
	    }
	    else
	    {
		return;
	    }
	}
	glAlphaFunc(func, ref);
}

/*
 *	Sets the blend function
 *
 *	If s is NULL then values will be set explicitly
 */
void StateGLBlendFunc(
	state_gl_struct *s,
	GLenum sfactor, GLenum dfactor
)
{
	if(s != NULL)
	{
	    if((s->blend_func_sfactor != sfactor) ||
	       (s->blend_func_dfactor != dfactor)
	    )
	    {
		s->blend_func_sfactor = sfactor;
		s->blend_func_dfactor = dfactor;
	    }
	    else
	    {
		return;
	    }
	}
	glBlendFunc(sfactor, dfactor);
}

/*
 *	Sets the color material parameters
 *
 *	If s is NULL then values will be set explicitly
 */
void StateGLColorMaterial(
	state_gl_struct *s,
	GLenum face, GLenum mode
)
{
	if(s != NULL)
	{
	    if((s->color_material_face != face) ||
	       (s->color_material_mode != mode)
	    )
	    {
		s->color_material_face = face;
		s->color_material_mode = mode;
	    }
	    else
	    {
		return;
	    }
	}
	glColorMaterial(face, mode);
}

/*
 *	Sets the depth test function
 *
 *	If s is NULL then values will be set explicitly
 */
void StateGLDepthFunc(
	state_gl_struct *s, GLenum func
)
{
	if(s != NULL)
	{
	    if(s->depth_func_func != func)
	    {
		s->depth_func_func = func;
	    }
	    else
	    {
		return;
	    }
	}
	glDepthFunc(func);
}

/*
 *	Sets the depth buffer writing state
 *
 *	If s is NULL then values will be set explicitly
 */
void StateGLDepthMask(
	state_gl_struct *s, GLboolean flag
)
{
	if(s != NULL)
	{
	    if(s->depth_mask_flag != flag)
	    {
		s->depth_mask_flag = flag;
	    }
	    else
	    {
		return;
	    }
	}
	glDepthMask(flag);
}

/*
 *	Sets the cull face winding for the front face parameter
 *
 *	If s is NULL then values will be set explicitly
 */
void StateGLFrontFace(
	state_gl_struct *s, GLenum mode
)
{
	if(s != NULL)
	{
	    if(s->front_face_mode != mode)
		s->front_face_mode = mode;
	    else
		return;
	}
	glFrontFace(mode);
}

/*
 *	Sets the line width
 *
 *	If s is NULL then values will be set explicitly
 */
void StateGLLineWidth(
	state_gl_struct *s, GLfloat width
)
{
	if(s != NULL)
	{
	    if(s->line_width != width)
		s->line_width = width;
	    else
		return;
	}
	glLineWidth(width);
}

/*
 *	Sets the point size
 *
 *	If s is NULL then values will be set explicitly
 */
void StateGLPointSize(
	state_gl_struct *s, GLfloat size
)
{
	if(s != NULL)
	{
	    if(s->point_size != size)
		s->point_size = size;
	    else
		return;
	}
	glPointSize(size);
}

/*
 *	Sets the polygon offset parameter
 *
 *	If s is NULL then values will be set explicitly
 */
void StateGLPolygonOffset(
	state_gl_struct *s, GLfloat factor, GLfloat units
)
{
	if(s != NULL)
	{
	    if((s->polygon_offset_factor != factor) ||
	       (s->polygon_offset_units != units)
	    )
	    {
		s->polygon_offset_factor = factor;
		s->polygon_offset_units = units;
	    }
	    else
		return;
	}
	glPolygonOffset(factor, units);
}

/*
 *	Sets the scissor test region
 *
 *	Given width and height must be non-negative
 */
void StateGLScissor(
	state_gl_struct *s, GLint x, GLint y, GLsizei width, GLsizei height
)
{
	if(s != NULL)
	{
	    if((s->scissor_test_x != x) ||
	       (s->scissor_test_y != y) ||
	       (s->scissor_test_width != width) ||
	       (s->scissor_test_height != height)
	    )
	    {
		s->scissor_test_x = x;
		s->scissor_test_y = y;
		s->scissor_test_width = width;
		s->scissor_test_height = height;
	    }
	    else
		return;
	}
	glScissor(x, y, width, height);
}

/*
 *	Sets the shade model
 *
 *	If s is NULL then value will be set explicitly
 */
void StateGLShadeModel(  
	state_gl_struct *s, GLenum mode
)
{
	if(s != NULL)
	{
	    if(s->shade_model_mode != mode)
	    {
		s->shade_model_mode = mode;
	    }
	    else
		return;
	}
	glShadeModel(mode);
}

/*
 *	Sets the stencil function
 *
 *	If s is NULL then value will be set explicitly
 */
void StateGLStencilFunc(
	state_gl_struct *s, GLenum func, GLint ref, GLuint mask
)
{
	if(s != NULL)
	{
	    if((s->stencil_func_func != func) ||
	       (s->stencil_func_ref != ref) ||
	       (s->stencil_func_mask != mask)
	    )
	    {
		s->stencil_func_func = func;
		s->stencil_func_ref = ref;
		s->stencil_func_mask = mask;
	    }
	    else
		return;
	}
	glStencilFunc(func, ref, mask);
}

/*
 *	Sets the stencil operation
 *
 *	If s is NULL then value will be set explicitly
 */
void StateGLStencilOp(
	state_gl_struct *s, GLenum fail, GLenum zfail, GLenum zpass 
)
{
	if(s != NULL)
	{
	    if((s->stencil_op_fail != fail) ||
	       (s->stencil_op_zfail != zfail) ||
	       (s->stencil_op_zpass != zpass)
	    )
	    {
		s->stencil_op_fail = fail;
		s->stencil_op_zfail = zfail;
		s->stencil_op_zpass = zpass;
	    }   
	    else
		return;
	}
	glStencilOp(fail, zfail, zpass);
}

/*
 *	Sets the texture enviroment parameters
 *
 *	If s is NULL then values will be set explicitly
 */
void StateGLTexEnvI(
	state_gl_struct *s,
	GLenum target, GLenum pname, GLint param
)
{
	if(s != NULL)
	{
	    if((s->tex_env_target != target) ||
	       (s->tex_env_pname != pname) ||
	       (s->tex_env_param != param)
	    )
	    {
		s->tex_env_target = target;
		s->tex_env_pname = pname;
		s->tex_env_param = param;
	    }
	    else
		return;
	}
	glTexEnvi(target, pname, param);
}


/*
 *	Resets all states (supported in the state_gl_struct)
 *	for the current OpenGL context to be disabled.
 *
 *	If s is NULL then no operation will be performed.
 */
void StateGLResetAll(state_gl_struct *s)
{
#if 0
	if(s == NULL)
	    return;
#endif

	/* Reset states */
	s->alpha_test = False;
	glDisable(GL_ALPHA_TEST);

	s->blend = False;
	glDisable(GL_BLEND);

	s->color_material = False;
	glDisable(GL_COLOR_MATERIAL);

	s->cull_face = False;
	glDisable(GL_CULL_FACE);

	s->depth_test = False;
	glDisable(GL_DEPTH_TEST);

	s->dither = False;
	glDisable(GL_DITHER);

	s->fog = False;
	glDisable(GL_FOG);

	s->lighting = False;
	glDisable(GL_LIGHTING);

	s->light0 = False;
	glDisable(GL_LIGHT0);

	s->light1 = False;
	glDisable(GL_LIGHT1);

	s->light2 = False;
	glDisable(GL_LIGHT2);

	s->light3 = False;
	glDisable(GL_LIGHT3);

	s->light4 = False;
	glDisable(GL_LIGHT4);

	s->light5 = False;
	glDisable(GL_LIGHT5);

	s->light6 = False;
	glDisable(GL_LIGHT6);

	s->light7 = False;
	glDisable(GL_LIGHT7);

	s->line_smooth = False;
	glDisable(GL_LINE_SMOOTH);

	s->point_smooth = False;
	glDisable(GL_POINT_SMOOTH);

	s->polygon_offset_fill = False;
	glDisable(GL_POLYGON_OFFSET_FILL);

	s->polygon_offset_line = False;
	glDisable(GL_POLYGON_OFFSET_LINE);

	s->polygon_offset_point = False;
	glDisable(GL_POLYGON_OFFSET_POINT);

	s->scissor_test = False;
	glDisable(GL_SCISSOR_TEST);

	s->stencil_test = False;
	glDisable(GL_STENCIL_TEST);

	s->texture_1d = False;
	glDisable(GL_TEXTURE_1D);

	s->texture_2d = False;
	glDisable(GL_TEXTURE_2D);

	s->texture_3d = False;
	glDisable(GL_TEXTURE_2D);


	/* Functions and paramters. */
	s->alpha_func_func = GL_ALWAYS;
	s->alpha_func_ref = 0.0f;
	glAlphaFunc(GL_ALWAYS, 0.0f);

	s->blend_func_sfactor = GL_ONE;
	s->blend_func_dfactor = GL_ZERO;
	glBlendFunc(GL_ONE, GL_ZERO);

	s->color_material_face = GL_FRONT_AND_BACK;
	s->color_material_mode = GL_AMBIENT_AND_DIFFUSE;
	glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

	s->depth_func_func = GL_LESS;
	glDepthFunc(GL_LESS);

	s->front_face_mode = GL_CCW;
	glFrontFace(GL_CCW);

	s->depth_mask_flag = GL_TRUE;
	glDepthMask(GL_TRUE);

	s->line_width = 1.0f;
	glLineWidth(1.0f);

	s->point_size = 1.0f;
	glPointSize(1.0f);

	s->polygon_offset_factor = 0.0f;
	s->polygon_offset_units = 0.0f;
	glPolygonOffset(0.0f, 0.0f);

	s->scissor_test_x = 0;
	s->scissor_test_y = 0;
	s->scissor_test_width = 0;
	s->scissor_test_height = 0;
	glScissor(0, 0, 0, 0);

	s->shade_model_mode = GL_SMOOTH;
	glShadeModel(GL_SMOOTH);

	s->stencil_func_func = GL_ALWAYS;
	s->stencil_func_ref = 0;
	s->stencil_func_mask = 0xffffffff;
	glStencilFunc(GL_ALWAYS, 0, 0xffffffff);

	s->stencil_op_fail = GL_KEEP;
	s->stencil_op_zfail = GL_KEEP;
	s->stencil_op_zpass = GL_KEEP;
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

	s->tex_env_target = GL_TEXTURE_ENV;
	s->tex_env_pname = GL_TEXTURE_ENV_MODE;
	s->tex_env_param = GL_MODULATE;
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

}
