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

/*
			  OpenGL State Recording

	Functions to keep track of OpenGL states.
 */

#ifndef STATEGL_H
#define STATEGL_H


#ifdef __MSW__
# include <windows.h>
# include <GL/gl.h>
#endif

#ifndef True
# define True	1
#endif
#ifndef False
# define False	0
#endif

#define StateGLBoolean	unsigned char


/*
 *	GL state record struct:
 *
 *	When adding new states to this structure, be sure to update
 *	function SARGLStateResetAll(), StateGLEnable(), and
 *	StateGLDisable() to handle it
 *
 *	Plus create any additional functions to set specific function
 *	parameters
 */
typedef struct {

	/* Boolean values for each supported states */
	StateGLBoolean	alpha_test,
			blend,
			color_material,
			cull_face,
			depth_test,
			dither,
			fog,
			lighting,
			light0,
			light1,
			light2,
			light3,
			light4,
			light5,
			light6,
			light7,
			line_smooth,
			point_smooth,
			polygon_offset_fill,
			polygon_offset_line,
			polygon_offset_point,
			scissor_test,
			stencil_test,
			texture_1d,
			texture_2d,
			texture_3d,
			write_unbiased_depth;

	/* Alpha function state */
	GLenum		alpha_func_func;
	GLclampf	alpha_func_ref;

	/* Blend function state */
	GLenum		blend_func_sfactor,
			blend_func_dfactor;

	/* Color material parameters */
	GLenum		color_material_face,
			color_material_mode;

	/* Depth test function state */
	GLenum		depth_func_func;

	/* Depth buffer writing (GL_TRUE to enable) */
	GLboolean	depth_mask_flag;

	/* Front face cull winding */
	GLenum		front_face_mode;

	/* Line width */
	GLfloat		line_width;

	/* Point size */
	GLfloat		point_size;

	/* Polygon offset */
	GLfloat		polygon_offset_factor,
			polygon_offset_units;

	/* Scissor test */
	GLint		scissor_test_x,
			scissor_test_y;
	GLsizei		scissor_test_width,
			scissor_test_height;

	/* Shade model */
	GLenum		shade_model_mode;

	/* Stencil function */
	GLenum		stencil_func_func;
	GLint		stencil_func_ref;
	GLuint		stencil_func_mask;

	/* Stencil operation */
	GLenum		stencil_op_fail,
			stencil_op_zfail,
			stencil_op_zpass;

	/* Texture enviroment state */
	GLenum		tex_env_target, tex_env_pname;
	GLint		tex_env_param;


} state_gl_struct;
#define STATE_GL(p)	((state_gl_struct *)(p))


extern void StateGLEnableF(state_gl_struct *s, GLenum cap, GLboolean force);
extern void StateGLEnable(state_gl_struct *s, GLenum cap);
extern void StateGLDisableF(state_gl_struct *s, GLenum cap, GLboolean force);
extern void StateGLDisable(state_gl_struct *s, GLenum cap);

extern void StateGLAlphaFunc(
	state_gl_struct *s,
	GLenum func, GLclampf ref
);
extern void StateGLBlendFunc(
	state_gl_struct *s,
	GLenum sfactor, GLenum dfactor
);
extern void StateGLColorMaterial(
	state_gl_struct *s,
	GLenum face, GLenum mode
);
extern void StateGLDepthFunc(
	state_gl_struct *s, GLenum func
);
extern void StateGLDepthMask(  
	state_gl_struct *s, GLboolean flag
);
extern void StateGLFrontFace(
	state_gl_struct *s, GLenum mode
);
extern void StateGLLineWidth(
	state_gl_struct *s, GLfloat width
);
extern void StateGLPointSize(
	state_gl_struct *s, GLfloat size
);
extern void StateGLPolygonOffset(
	state_gl_struct *s, GLfloat factor, GLfloat units
);
extern void StateGLScissor(
	state_gl_struct *s, GLint x, GLint y, GLsizei width, GLsizei height
);
extern void StateGLShadeModel(
	state_gl_struct *s, GLenum mode
);
extern void StateGLStencilFunc(
	state_gl_struct *s, GLenum func, GLint ref, GLuint mask
);
extern void StateGLStencilOp(
	state_gl_struct *s, GLenum fail, GLenum zfail, GLenum zpass
);                      
extern void StateGLTexEnvI(
	state_gl_struct *s,
	GLenum target, GLenum pname, GLint param
);

extern void StateGLResetAll(state_gl_struct *s);



#endif	/* STATEGL_H */
