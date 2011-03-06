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

#include <math.h>
#include <sys/types.h>

#ifdef __MSW__
# include <windows.h>
#endif
#include <GL/gl.h>
#include <GL/glu.h>

#include "gw.h"
#include "stategl.h"
#include "obj.h"
#include "objutils.h"
#include "sar.h"
#include "sardraw.h"
#include "sardrawdefs.h"
#include "config.h"


static void SARDrawHelipadEdgeLighting(
	sar_dc_struct *dc, sar_object_struct *obj_ptr,
	sar_object_helipad_struct *helipad,
	float x_min, float x_max,
	float y_min, float y_max,
	float z_min, float z_max
);
static void SARDrawHelipadPaved(
	sar_dc_struct *dc, sar_object_struct *obj_ptr,
	sar_object_helipad_struct *helipad,
	v3d_texture_ref_struct *t,
	float x_min, float x_max,
	float y_min, float y_max,
	float z_min, float z_max
);
static void SARDrawHelipadBare(
	sar_dc_struct *dc, sar_object_struct *obj_ptr,
	sar_object_helipad_struct *helipad,
	v3d_texture_ref_struct *t,
	float x_min, float x_max,
	float y_min, float y_max,
	float z_min, float z_max
);
static void SARDrawHelipadBuilding(
	sar_dc_struct *dc, sar_object_struct *obj_ptr,
	sar_object_helipad_struct *helipad,
	v3d_texture_ref_struct *t,
	float x_min, float x_max,
	float y_min, float y_max,
	float z_min, float z_max
);
static void SARDrawHelipadVehicle(
	sar_dc_struct *dc, sar_object_struct *obj_ptr,
	sar_object_helipad_struct *helipad,
	v3d_texture_ref_struct *t,
	float x_min, float x_max,
	float y_min, float y_max,
	float z_min, float z_max
);

void SARDrawHelipad(
	sar_dc_struct *dc, sar_object_struct *obj_ptr,
	sar_object_helipad_struct *helipad,
	float distance
);
void SARDrawHelipadMap(
	sar_dc_struct *dc, sar_object_struct *obj_ptr,
	sar_object_helipad_struct *helipad,
	float icon_len
);


static GLfloat edge_lighting_point_size = 1.0f;


/*
 *	Draws standard square edge lighting, a light in each corner.
 *
 *	Calling function needs to unselect texture.
 */
static void SARDrawHelipadEdgeLighting(
	sar_dc_struct *dc, sar_object_struct *obj_ptr,
	sar_object_helipad_struct *helipad,
	float x_min, float x_max,
	float y_min, float y_max,
	float z_min, float z_max
)
{
	gw_display_struct *display = dc->display;
	state_gl_struct *state = &display->state_gl;
	StateGLBoolean lighting = state->lighting;

	/* Unselect texture */
	V3DTextureSelect(NULL);

	/* Set up gl states */
	StateGLDisable(state, GL_LIGHTING);
	StateGLEnable(state, GL_POINT_SMOOTH);
	StateGLPointSize(state, edge_lighting_point_size);

	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

	glBegin(GL_POINTS);
	{
	    glNormal3f(0.0, 1.0, -0.0);
	    glVertex3f(
		x_min, z_max, -y_min
	    );
	    glVertex3f(
		x_min, z_max, -y_max
	    );
	    glVertex3f(
		x_max, z_max, -y_max
	    );
	    glVertex3f(
		x_max, z_max, -y_min
	    );
	}
	glEnd();

	/* Restore gl states */
	StateGLDisable(state, GL_POINT_SMOOTH);
	if(lighting)
	    StateGLEnable(state, GL_LIGHTING);
}

/*
 *	Draws paved version of helipad.
 */
static void SARDrawHelipadPaved(
	sar_dc_struct *dc, sar_object_struct *obj_ptr,
	sar_object_helipad_struct *helipad,
	v3d_texture_ref_struct *t,
	float x_min, float x_max,
	float y_min, float y_max,
	float z_min, float z_max
)
{
	gw_display_struct *display = dc->display;
	state_gl_struct *state = &display->state_gl;
	GLenum shade_model_mode = state->shade_model_mode;
	const sar_option_struct *opt = dc->option;

	/* Texture for landable area available? */
	if(t == NULL)
	{
	    /* No texture available, unselect texture and set color
	     * greyish.
	     */
	    V3DTextureSelect(NULL);
	    glColor4f(0.6f, 0.6f, 0.6f, 1.0f);
	}
	else
	{
	    V3DTextureSelect(t);
	    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	}

	/* Set up gl states */
	StateGLShadeModel(state, opt->gl_shade_model);

	/* Draw main landable area */
	glBegin(GL_QUADS);
	{
	    glNormal3f(-0.33f, 0.88f, -0.33f);
	    glTexCoord2f(0.0f, 1.0f - 0.0f);
	    glVertex3f(
		x_min, z_max, -y_max
	    );

	    glNormal3f(-0.33f, 0.88f, 0.33f);
	    glTexCoord2f(0.0f, 1.0f - 1.0f);
	    glVertex3f(
		x_min, z_max, -y_min
	    );

	    glNormal3f(0.33f, 0.88f, 0.33f);
	    glTexCoord2f(1.0f, 1.0f - 1.0f);
	    glVertex3f(
		x_max, z_max, -y_min
	    );

	    glNormal3f(0.33f, 0.88f, -0.33f); 
	    glTexCoord2f(1.0f, 1.0f - 0.0f);
	    glVertex3f(
		x_max, z_max, -y_max
	    );
	}
	glEnd();

	/* Unselect texture */
	if(t != NULL)
	    V3DTextureSelect(NULL);

	/* Draw label? */
	if(helipad->label_vmodel != NULL)
	{
	    StateGLBoolean depth_mask_flag = state->depth_mask_flag;
	    StateGLBoolean polygon_offset_fill = state->polygon_offset_fill;
	    sar_visual_model_struct *vmodel = helipad->label_vmodel;
	    float label_width = helipad->label_width;

	    StateGLDepthMask(state, GL_FALSE);
	    StateGLEnable(state, GL_POLYGON_OFFSET_FILL);
	    StateGLPolygonOffset(
		state,
		(GLfloat)opt->gl_polygon_offset_factor, -1.0f
	    );

	    glPushMatrix();
	    glTranslatef(
		(GLfloat)(label_width * -0.5),
		0.0f,
		(GLfloat)-(helipad->length * -0.4)
	    );
	    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	    SARVisualModelCallList(vmodel);
	    glPopMatrix();

	    StateGLDepthMask(state, depth_mask_flag);
	    if(polygon_offset_fill)
		StateGLEnable(state, GL_POLYGON_OFFSET_FILL);
	    else
		StateGLDisable(state, GL_POLYGON_OFFSET_FILL);
	}

	/* Draw edge lighting? */
	if(helipad->flags & SAR_HELIPAD_FLAG_EDGE_LIGHTING)
	    SARDrawHelipadEdgeLighting(
		dc, obj_ptr, helipad,
		x_min, x_max, y_min, y_max, z_min, z_max
	    );

	/* Restore gl states */
	StateGLShadeModel(state, shade_model_mode);
}

/*
 *      Draws bare version of helipad.
 */
static void SARDrawHelipadBare(
	sar_dc_struct *dc, sar_object_struct *obj_ptr,
	sar_object_helipad_struct *helipad,
	v3d_texture_ref_struct *t,
	float x_min, float x_max,
	float y_min, float y_max,
	float z_min, float z_max
)
{
	gw_display_struct *display = dc->display;
	state_gl_struct *state = &display->state_gl;
	GLenum shade_model_mode = state->shade_model_mode;
	const sar_option_struct *opt = dc->option;

	/* Texture for landable area available? */
	if(t == NULL)
	{
	    /* No texture available, unselect texture and set color
	     * light beige.
	     */
	    V3DTextureSelect(NULL);
	    glColor4f(0.75f, 0.72f, 0.62f, 1.0f);
	}
	else
	{
	    V3DTextureSelect(t);
	    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	}

	/* Set up gl states */
	StateGLShadeModel(state, opt->gl_shade_model);

	/* Draw main landable area */
	glBegin(GL_QUADS);
	{
	    glNormal3f(-0.33f, 0.88f, -0.33f);
	    glTexCoord2f(0.0f, 1.0f - 0.0f);
	    glVertex3f(
		x_min, z_max, -y_max
	    );

	    glNormal3f(-0.33f, 0.88f, 0.33f);
	    glTexCoord2f(0.0f, 1.0f - 1.0f);
	    glVertex3f(
		x_min, z_max, -y_min
	    );

	    glNormal3f(0.33f, 0.88f, 0.33f);
	    glTexCoord2f(1.0f, 1.0f - 1.0f);
	    glVertex3f(
		x_max, z_max, -y_min
	    );

	    glNormal3f(0.33f, 0.88f, -0.33f);
	    glTexCoord2f(1.0f, 1.0f - 0.0f);
	    glVertex3f(
		x_max, z_max, -y_max
	    );
	}
	glEnd();

	/* Unselect texture */
	if(t != NULL)
	    V3DTextureSelect(NULL);

	/* Draw label? */
	if(helipad->label_vmodel != NULL)
	{
	    StateGLBoolean depth_mask_flag = state->depth_mask_flag;
	    StateGLBoolean polygon_offset_fill = state->polygon_offset_fill;
	    sar_visual_model_struct *vmodel = helipad->label_vmodel;
	    float label_width = helipad->label_width;

	    StateGLDepthMask(state, GL_FALSE);
	    StateGLEnable(state, GL_POLYGON_OFFSET_FILL);
	    StateGLPolygonOffset(
		state,
		(GLfloat)opt->gl_polygon_offset_factor, -1.0f
	    );

	    glPushMatrix();
	    glTranslatef(
		(GLfloat)(label_width * -0.5),
		0.0f,
		(GLfloat)-(helipad->length * -0.4)
	    );
	    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	    SARVisualModelCallList(vmodel);
	    glPopMatrix();

	    StateGLDepthMask(state, depth_mask_flag);
	    if(polygon_offset_fill)
		StateGLEnable(state, GL_POLYGON_OFFSET_FILL);
	    else
		StateGLDisable(state, GL_POLYGON_OFFSET_FILL);
	}

	/* Draw edge lighting? */
	if(helipad->flags & SAR_HELIPAD_FLAG_EDGE_LIGHTING)
	    SARDrawHelipadEdgeLighting(
		dc, obj_ptr, helipad,
		x_min, x_max, y_min, y_max, z_min, z_max
	    );

	/* Restore gl states */
	StateGLShadeModel(state, shade_model_mode);
}

/*
 *      Draws building version of helipad.
 */
static void SARDrawHelipadBuilding(
	sar_dc_struct *dc, sar_object_struct *obj_ptr,
	sar_object_helipad_struct *helipad,
	v3d_texture_ref_struct *t,
	float x_min, float x_max,
	float y_min, float y_max,
	float z_min, float z_max
)
{
	gw_display_struct *display = dc->display;
	state_gl_struct *state = &display->state_gl;
	GLenum shade_model_mode = state->shade_model_mode;
	const sar_option_struct *opt = dc->option;

	V3DTextureSelect(NULL);

	/* Set up gl states */
	StateGLShadeModel(state, opt->gl_shade_model);

	/* Draw recession struts as needed */
	if(z_min < 0.0f)
	{
	  glColor4f(0.3f, 0.3f, 0.3f, 1.0f);

	  glBegin(GL_QUADS);
	  {
	    float x[2], y[2], z[2], coeff = 0.6f;

	    x[0] = x_min * coeff;
	    x[1] = x_max * coeff;
	    y[0] = y_min * coeff;
	    y[1] = y_max * coeff;
	    z[0] = z_min;
	    z[1] = z_max;

	    /* North */
	    glNormal3f(0.0f, 0.0f, -1.0f);
	    glVertex3f(x[1], z[1], -y[1]);
	    glVertex3f(x[1], z[0], -y[1]);
	    glVertex3f(x[0], z[0], -y[1]);
	    glVertex3f(x[0], z[1], -y[1]);

	    /* East */
	    glNormal3f(1.0f, 0.0f, 0.0f);
	    glVertex3f(x[1], z[1], -y[0]);
	    glVertex3f(x[1], z[0], -y[0]);
	    glVertex3f(x[1], z[0], -y[1]);
	    glVertex3f(x[1], z[1], -y[1]);

	    /* South */
	    glNormal3f(0.0f, 0.0f, 1.0f);
	    glVertex3f(x[0], z[1], -y[0]);
	    glVertex3f(x[0], z[0], -y[0]);
	    glVertex3f(x[1], z[0], -y[0]);  
	    glVertex3f(x[1], z[1], -y[0]);  

	    /* West */
	    glNormal3f(-1.0f, 0.0f, 0.0f);
	    glVertex3f(x[0], z[1], -y[1]);  
	    glVertex3f(x[0], z[0], -y[1]);
	    glVertex3f(x[0], z[0], -y[0]);
	    glVertex3f(x[0], z[1], -y[0]);
	  }
	  glEnd();

	  /* Ramp down to recession on south side (this will protrude
	   * out of the helipad's bounds though.
	   */
	  glColor4f(0.75f, 0.75f, 0.75f, 1.0f);
	  glBegin(GL_QUADS);
	  {
	    float coeff = 0.2f;	/* Of width */

	    glNormal3f(0.0f, 0.7f, 0.7f);
	    glVertex3f(
		x_min * coeff,
		z_max,
		-(y_min * 0.9f)
	    );
	    glVertex3f(
		x_min * coeff,
		z_min,
		-(y_min * 1.2f)
	    );
	    glVertex3f(
		x_max * coeff,
		z_min,
		-(y_min * 1.2f)
	    );
	    glVertex3f(   
		x_max * coeff,
		z_max,
		-(y_min * 0.9f)
	    );

	    glNormal3f(0.0f, 0.7f, -0.7f);
	    glVertex3f(
		x_min * coeff,
		z_min,
		-(y_max * 1.2f)
	    );
	    glVertex3f(
		x_min * coeff,
		z_max,
		-(y_max * 0.9f)
	    );
	    glVertex3f(
		x_max * coeff,
		z_max,
		-(y_max * 0.9f)
	    );
	    glVertex3f(
		x_max * coeff,
		z_min,
		-(y_max * 1.2f)
	    );
	  }
	  glEnd();
	}

	/* Draw underside of landable area if have recession */
	if(z_min < 0.0f)
	{
	    GLboolean depth_mask_flag = state->depth_mask_flag;
	    StateGLBoolean polygon_offset_fill = state->polygon_offset_fill;


	    glColor4f(0.6f, 0.6f, 0.6f, 1.0f);
	    glBegin(GL_POLYGON);
	    {
		float theta;

		glNormal3f(0.0f, -1.0f, 0.0f);

		/* Draw a circular area, starting from 12 o'clock */
		for(theta = 0.0f; theta < 2.0f; theta += 0.1f)
		    glVertex3f(
			(float)(x_max * sin(theta * PI)),
			z_max,
			(float)-(y_max * cos(theta * PI))
		    );
	    }
	    glEnd();

	    /* Shadow of helipad */

	    /* Set up gl states */
	    StateGLDepthMask(state, GL_FALSE);
	    StateGLEnable(state, GL_POLYGON_OFFSET_FILL);
	    StateGLPolygonOffset(
		state,
		(GLfloat)opt->gl_polygon_offset_factor, -1.0f
	    );

	    glColor4f(0.2f, 0.2f, 0.2f, 1.0f);
	    glBegin(GL_POLYGON);
	    {
		float theta;

		glNormal3f(0.0f, 1.0f, 0.0f);

		/* Draw a circular area, starting from 12 o'clock */
		for(theta = 2.0f; theta > 0.0f; theta -= 0.1f)
		    glVertex3f(
			(float)(x_max * sin(theta * PI)),
			z_min,
			(float)-(y_max * cos(theta * PI))
		    );
	    }
	    glEnd();

	    /* Restore gl states */
	    StateGLDepthMask(state, depth_mask_flag);
	    if(polygon_offset_fill)
		StateGLEnable(state, GL_POLYGON_OFFSET_FILL);
	    else
		StateGLDisable(state, GL_POLYGON_OFFSET_FILL);
	}


	/* Texture for main landable area available? */
	if(t == NULL)
	{
	    /* No texture available, unselect texture and set color
	     * light grey.
	     */
	    glColor4f(0.84f, 0.84f, 0.84f, 1.0f);
	}
	else
	{
	    V3DTextureSelect(t);
	    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	}

	/* Draw main landable area */
	glBegin(GL_POLYGON);
	{
	    float theta;

	    /* Draw a circular area, starting from 12 o'clock */
	    for(theta = 2.0f; theta > 0.0f; theta -= 0.1f)
	    {
		glNormal3f(
		    (float)(0.33 * sin(theta * PI)),
		    (float)0.88,
		    (float)-(0.33 * cos(theta * PI))
		);
		glTexCoord2f(
		    (float)((sin(theta * PI) + 1.0) / 2.0),
		    1.0f - (float)((cos(theta * PI) + 1.0) / 2.0)
		);
		glVertex3f(
		    (float)(x_max * sin(theta * PI)),
		    (float)z_max,
		    (float)-(y_max * cos(theta * PI))
		);
	    }
	}
	glEnd();

	/* Unselect texture */
	if(t != NULL)
	    V3DTextureSelect(NULL);

	/* Draw label? */
	if(helipad->label_vmodel != NULL)
	{
	    StateGLBoolean depth_mask_flag = state->depth_mask_flag;
	    StateGLBoolean polygon_offset_fill = state->polygon_offset_fill;
	    sar_visual_model_struct *vmodel = helipad->label_vmodel;
	    float label_width = helipad->label_width;

	    StateGLDepthMask(state, GL_FALSE);
	    StateGLEnable(state, GL_POLYGON_OFFSET_FILL);
	    StateGLPolygonOffset(
		state,
		(GLfloat)opt->gl_polygon_offset_factor, -1.0f
	    );

	    glPushMatrix();
	    glTranslatef(
		(GLfloat)(label_width * -0.5),
		0.0f,
		(GLfloat)-(helipad->length * -0.4)
	    );
	    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	    SARVisualModelCallList(vmodel);
	    glPopMatrix();

	    StateGLDepthMask(state, depth_mask_flag);
	    if(polygon_offset_fill)
		StateGLEnable(state, GL_POLYGON_OFFSET_FILL);
	    else
		StateGLDisable(state, GL_POLYGON_OFFSET_FILL);
	}

	/* Draw edge lighting? */
	if(helipad->flags & SAR_HELIPAD_FLAG_EDGE_LIGHTING)
	{
	    /* Draw four points of lighting at the right angle extremes
	     * instead of at the corners.
	     */
	    StateGLBoolean lighting = state->lighting;

	    /* Set up gl states */
	    StateGLDisable(state, GL_LIGHTING);
	    StateGLEnable(state, GL_POINT_SMOOTH);
	    StateGLPointSize(state, edge_lighting_point_size);

	    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

	    glBegin(GL_POINTS);
	    {
		glNormal3f(0.0f, 1.0f, 0.0f);
		glVertex3f(
		    0.0f, z_max, -y_max
		);
		glVertex3f(
		    x_max, z_max, 0.0f
		);
		glVertex3f(
		    0.0f, z_max, -y_min
		);
		glVertex3f(
		    x_min, z_max, 0.0f
		);
	    }
	    glEnd();

	    /* Restore gl states */
	    StateGLDisable(state, GL_POINT_SMOOTH);
	    if(lighting)
		StateGLEnable(state, GL_LIGHTING);
	}

	/* Restore gl states */
	StateGLShadeModel(state, shade_model_mode);
}

/*
 *      Draws vehicle version of helipad.
 */
static void SARDrawHelipadVehicle(
	sar_dc_struct *dc, sar_object_struct *obj_ptr,
	sar_object_helipad_struct *helipad,
	v3d_texture_ref_struct *t,
	float x_min, float x_max,
	float y_min, float y_max,
	float z_min, float z_max
)
{
	gw_display_struct *display = dc->display;
	state_gl_struct *state = &display->state_gl;
	GLenum shade_model_mode = state->shade_model_mode;
	const sar_option_struct *opt = dc->option;

	V3DTextureSelect(NULL);

	/* Set up gl states */
	StateGLShadeModel(state, opt->gl_shade_model);

	/* Draw recessed side walls */
	glColor4f(0.6f, 0.6f, 0.6f, 1.0f);

	glBegin(GL_QUADS);
	{
	    float x[2], y[2], z[2] /*, coeff = 0.4 */;

	    x[0] = x_min;
	    x[1] = x_max;
	    y[0] = y_min;
	    y[1] = y_max;
	    z[0] = z_min;
	    z[1] = z_max;

	    /* North */
	    glNormal3f(0.0f, 0.0f, -1.0f);
	    glVertex3f(x[1], z[1], -y[1]);
	    glVertex3f(x[1], z[0], -y[1]);
	    glVertex3f(x[0], z[0], -y[1]);
	    glVertex3f(x[0], z[1], -y[1]);

	    /* East */
	    glNormal3f(1.0f, 0.0f, 0.0f);
	    glVertex3f(x[1], z[1], -y[0]);
	    glVertex3f(x[1], z[0], -y[0]);
	    glVertex3f(x[1], z[0], -y[1]);
	    glVertex3f(x[1], z[1], -y[1]);

	    /* South */
	    glNormal3f(0.0f, 0.0f, 1.0f);
	    glVertex3f(x[0], z[1], -y[0]);
	    glVertex3f(x[0], z[0], -y[0]);
	    glVertex3f(x[1], z[0], -y[0]);
	    glVertex3f(x[1], z[1], -y[0]);

	    /* West */
	    glNormal3f(-1.0f, 0.0f, 0.0f);
	    glVertex3f(x[0], z[1], -y[1]);
	    glVertex3f(x[0], z[0], -y[1]);
	    glVertex3f(x[0], z[0], -y[0]);
	    glVertex3f(x[0], z[1], -y[0]);

#if 0
	    /* Lower struts */
	    glColor4f(0.3f, 0.3f, 0.3f, 1.0f);

	    coeff = 0.9f;
	    x[0] = x_min * coeff;
	    x[1] = x_max * coeff;
	    y[0] = y_min * coeff;
	    y[1] = y_max * coeff;
	    z[0] = z_min;
	    z[1] = z_max;

	    /* North */
	    glNormal3f(0.0f, 0.0f, -1.0f);
	    glVertex3f(x[1], z[1], -y[1]);
	    glVertex3f(x[1], z[0], -y[1]);
	    glVertex3f(x[0], z[0], -y[1]);
	    glVertex3f(x[0], z[1], -y[1]);
	 
	    /* East */
	    glNormal3f(1.0f, 0.0f, 0.0f);
	    glVertex3f(x[1], z[1], -y[0]);
	    glVertex3f(x[1], z[0], -y[0]);
	    glVertex3f(x[1], z[0], -y[1]);
	    glVertex3f(x[1], z[1], -y[1]);  

	    /* South */
	    glNormal3f(0.0f, 0.0f, 1.0f);
	    glVertex3f(x[0], z[1], -y[0]);
	    glVertex3f(x[0], z[0], -y[0]);
	    glVertex3f(x[1], z[0], -y[0]);
	    glVertex3f(x[1], z[1], -y[0]);

	    /* West */
	    glNormal3f(-1.0f, 0.0f, 0.0f);
	    glVertex3f(x[0], z[1], -y[1]);
	    glVertex3f(x[0], z[0], -y[1]);
	    glVertex3f(x[0], z[0], -y[0]);
	    glVertex3f(x[0], z[1], -y[0]);
#endif
	}
	glEnd();


#if 0
	/* Ramp down to recession on north and south side (this will
	 * protrude out of the helipad's bounds).
	 */
	glColor4f(0.7f, 0.7f, 0.7f, 1.0f);
	glBegin(GL_QUADS);
	{
	    float coeff = 0.2f;		/* Of width */

	    glNormal3f(0.0f, 0.7f, 0.7f);
	    glVertex3f(
		x_min * coeff,
		z_max,
		-y_min
	    );
	    glVertex3f(
		x_min * coeff,
		z_min,
		-(y_min * 1.1f)
	    );
	    glVertex3f(   
		x_max * coeff,
		z_min,
		-(y_min * 1.1f)
	    );
	    glVertex3f(
		x_max * coeff,
		z_max,
		-(y_min)
	    );

	    glNormal3f(0.0f, 0.7f, -0.7f);
	    glVertex3f(
		x_min * coeff,
		z_min,
		-(y_max * 1.1f)
	    );
	    glVertex3f(
		x_min * coeff,
		z_max,
		-(y_max * 1.0f)
	    );
	    glVertex3f(
		x_max * coeff,
		z_max,
		-(y_max * 1.0f)
	    );
	    glVertex3f(
		x_max * coeff,
		z_min,
		-(y_max * 1.1f)
	    );
	}
	glEnd();
#endif


	/* Texture for landable area available? */
	if(t == NULL)
	{
	    /* No texture available, unselect texture and set color
	     * light grey.
	     */
	    V3DTextureSelect(NULL);
	    glColor4f(0.84f, 0.84f, 0.84f, 1.0f);
	}
	else
	{
	    V3DTextureSelect(t);
	    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	}

	/* Draw main landable area */
	glBegin(GL_QUADS);
	{
	    glNormal3f(-0.33f, 0.88f, -0.33f);
	    glTexCoord2f(0.0f, 1.0f - 0.0f);
	    glVertex3f(
		x_min, z_max, -y_max
	    );

	    glNormal3f(-0.33f, 0.88f, 0.33f);
	    glTexCoord2f(0.0f, 1.0f - 1.0f);
	    glVertex3f(
		x_min, z_max, -y_min
	    );

	    glNormal3f(0.33f, 0.88f, 0.33f);
	    glTexCoord2f(1.0f, 1.0f - 1.0f);
	    glVertex3f(
		x_max, z_max, -y_min
	    );

	    glNormal3f(0.33f, 0.88f, -0.33f);
	    glTexCoord2f(1.0f, 1.0f - 0.0f);
	    glVertex3f(
		x_max, z_max, -y_max
	    );
	}
	glEnd();

	/* Unselect texture */
	if(t != NULL)
	    V3DTextureSelect(NULL);

	/* Draw fence */
	if(1)
	{
	    float coeff = 1.1f, zabove = z_max + 0.8f;

	    glColor4f(0.3f, 0.3f, 0.3f, 1.0f);

	    /* East fence */
	    glBegin(GL_LINE_LOOP);
	    {
		glNormal3f(0.33f, 0.88f, -0.33f);
		glVertex3f(
		    x_max * coeff,
		    zabove,
		    -(y_min * coeff)
		);

		glNormal3f(0.33f, 0.88f, 0.33f);
		glVertex3f(
		    x_max * coeff,
		    zabove,
		    -(y_max * coeff)
		);

		glNormal3f(-0.33f, 0.88f, 0.33f);
		glVertex3f(
		    x_max,
		    z_max,
		    -(y_max)
		);

		glNormal3f(-0.33f, 0.88f, -0.33f);
		glVertex3f(
		    x_max,
		    z_max,
		    -(y_min)
		);
	    }
	    glEnd();

	    /* West fence */
	    glBegin(GL_LINE_LOOP);
	    {
		glNormal3f(-0.33f, 0.88f, -0.33f);
		glVertex3f(
		    x_min * coeff,
		    zabove,
		    -(y_max * coeff)
		);

		glNormal3f(-0.33f, 0.88f, 0.33f);
		glVertex3f(   
		    x_min * coeff,
		    zabove,
		    -(y_min * coeff)
		);

		glNormal3f(0.33f, 0.88f, 0.33f);
		glVertex3f(
		    x_min,
		    z_max,
		    -(y_min)
		);

		glNormal3f(0.33f, 0.88f, -0.33f);
		glVertex3f(
		    x_min,
		    z_max,
		    -(y_max)
		);
	    }
	    glEnd();
	}

	/* Draw label? */
	if(helipad->label_vmodel != NULL)
	{
	    StateGLBoolean depth_mask_flag = state->depth_mask_flag;
	    StateGLBoolean polygon_offset_fill = state->polygon_offset_fill;
	    sar_visual_model_struct *vmodel = helipad->label_vmodel;
	    float label_width = helipad->label_width;

	    StateGLDepthMask(state, GL_FALSE);
	    StateGLEnable(state, GL_POLYGON_OFFSET_FILL);
	    StateGLPolygonOffset(
		state,
		(GLfloat)opt->gl_polygon_offset_factor, -1.0f
	    );

	    glPushMatrix();
	    glTranslatef(
		(GLfloat)(label_width * -0.5),
		0.0f,
		(GLfloat)-(helipad->length * -0.4)
	    );
	    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	    SARVisualModelCallList(vmodel);
	    glPopMatrix();

	    StateGLDepthMask(state, depth_mask_flag);
	    if(polygon_offset_fill)
		StateGLEnable(state, GL_POLYGON_OFFSET_FILL);
	    else
		StateGLDisable(state, GL_POLYGON_OFFSET_FILL);
	}


	/* Draw edge lighting? */
	if(helipad->flags & SAR_HELIPAD_FLAG_EDGE_LIGHTING)
	    SARDrawHelipadEdgeLighting(
		dc, obj_ptr, helipad,
		x_min, x_max, y_min, y_max, z_min, z_max
	    );

	/* Restore gl states */
	StateGLShadeModel(state, shade_model_mode);
}


/*
 *      Draws a helipad specified by the object obj_ptr.
 *
 *	3d distance from camera to object is specified by distance in
 *	meters.
 *
 *	The helipad substructure helipad should be the one on
 *	the object.
 *
 *	All inputs are assumed valid.
 */
void SARDrawHelipad(
	sar_dc_struct *dc, sar_object_struct *obj_ptr,
	sar_object_helipad_struct *helipad,
	float distance         /* 3d distance in meters */
) 
{
	sar_scene_struct *scene = dc->scene;
	int tex_num;
	v3d_texture_ref_struct *t; 
	float x_min, x_max, y_min, y_max, z_min, z_max;


	/* Get pointer to helipad main landable area texture */
	tex_num = helipad->tex_num;
	t = (SARIsTextureAllocated(scene, tex_num) ?
	    scene->texture_ref[tex_num] : NULL
	);

	/* Get x and y bounds of helipad, these specify the landable
	 * area of the helipad
	 */
	y_max = helipad->length / 2.0f;
	y_min = -y_max;
	x_max = helipad->width / 2.0f;
	x_min = -x_max;
	z_min = -helipad->recession;
	z_max = 0.0f;


	/* Draw by style */
	switch(helipad->style)
	{
	  case SAR_HELIPAD_STYLE_GROUND_BARE:
	    SARDrawHelipadBare(
		dc, obj_ptr, helipad,
		t, x_min, x_max, y_min, y_max, z_min, z_max
	    );
	    break;
	  case SAR_HELIPAD_STYLE_BUILDING:
	    SARDrawHelipadBuilding(
		dc, obj_ptr, helipad,
		t, x_min, x_max, y_min, y_max, z_min, z_max
	    );
	    break;
	  case SAR_HELIPAD_STYLE_VEHICLE:
	    SARDrawHelipadVehicle(
		dc, obj_ptr, helipad,
		t, x_min, x_max, y_min, y_max, z_min, z_max
	    );
	    break;
	  case SAR_HELIPAD_STYLE_GROUND_PAVED:
	    SARDrawHelipadPaved(
		dc, obj_ptr, helipad,
		t, x_min, x_max, y_min, y_max, z_min, z_max
	    );
	    break;
	}
}

/*
 *	Draws helipad for display on map.
 *
 *	All inputs are assumed valid.
 */
void SARDrawHelipadMap(
	sar_dc_struct *dc, sar_object_struct *obj_ptr,
	sar_object_helipad_struct *helipad,
	float icon_len
)
{
	gw_display_struct *display = dc->display;
	state_gl_struct *state = &display->state_gl;
	GLenum shade_model_mode = state->shade_model_mode;
	float x_min, x_max, y_min, y_max;

	/* Calculate bounds */
	x_min = -(icon_len / 2.0f);
	x_max = icon_len / 2.0f;
	y_min = -(icon_len / 2.0f);
	y_max = icon_len / 2.0f;

	/* No texture */
	V3DTextureSelect(NULL);

	/* Set up gl states */
	StateGLShadeModel(state, GL_FLAT);

	/* Begin drawing icon */
	glBegin(GL_QUADS);
	{
	    /* Blue background quad */
	    glColor4f(0.0f, 0.0f, 1.0f, 1.0f);
	    glNormal3f(0.0f, 1.0f, 0.0f);

	    glVertex3f(x_min, 0.0f, -y_max);
	    glVertex3f(x_min, 0.0f, -y_min);
	    glVertex3f(x_max, 0.0f, -y_min);
	    glVertex3f(x_max, 0.0f, -y_max);

	    /* White `cross' composed of two quads */
	    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	    glVertex3f((x_min * 0.8f), 0.0f, (-y_max * 0.2f));
	    glVertex3f((x_min * 0.8f), 0.0f, (-y_min * 0.2f));
	    glVertex3f((x_max * 0.8f), 0.0f, (-y_min * 0.2f));
	    glVertex3f((x_max * 0.8f), 0.0f, (-y_max * 0.2f));

	    glVertex3f((x_min * 0.2f), 0.0f, (-y_max * 0.8f));
	    glVertex3f((x_min * 0.2f), 0.0f, (-y_min * 0.8f));
	    glVertex3f((x_max * 0.2f), 0.0f, (-y_min * 0.8f));
	    glVertex3f((x_max * 0.2f), 0.0f, (-y_max * 0.8f));
	}
	glEnd();

	/* Restore gl states */
	StateGLShadeModel(state, shade_model_mode);
}
