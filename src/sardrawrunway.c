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

#include <string.h>

#include "matrixmath.h"
#include "gw.h"
#include "stategl.h"
#include "obj.h"
#include "objutils.h"
#include "sar.h"
#include "sardraw.h"
#include "sardrawdefs.h"
#include "config.h"


void SARDrawRunway(
	sar_dc_struct *dc, sar_object_struct *obj_ptr,
	sar_object_runway_struct *runway,
	float distance		/* 3d distance in meters */
);
void SARDrawRunwayMap(
	sar_dc_struct *dc, sar_object_struct *obj_ptr,
	sar_object_runway_struct *runway,
	float icon_len
);


/*
 *      Draws a runway specified by the object obj_ptr.
 *
 *	3d distance from camera to object is specified by distance in
 *	meters.
 *
 *	The runway substructure runway should be the one on
 *	the object.
 *
 * Need to add support to draw multiple styles.
 *
 *	All inputs are assumed valid.
 */
void SARDrawRunway(
	sar_dc_struct *dc, sar_object_struct *obj_ptr,
	sar_object_runway_struct *runway,
	float distance		/* 3d distance in meters */
) 
{
	gw_display_struct *display = dc->display;
	state_gl_struct *state = &display->state_gl;
	sar_scene_struct *scene = dc->scene;
	const sar_position_struct *cam_pos = &dc->camera_pos;
	GLenum shade_model_mode = state->shade_model_mode;
	const sar_position_struct *pos = &obj_ptr->pos;
	const sar_direction_struct *dir = &obj_ptr->dir;
	sar_runway_flags flags = runway->flags;
	float	length = runway->length,
		width = runway->width,
		threshold_length = (float)MIN(50.0, length * 0.5);
	int	tex_num = runway->tex_num;
	v3d_texture_ref_struct *t;
	sar_visual_model_struct *vmodel;
	float	x_min, x_max,
		y_min, y_max,
		yt_min, yt_max;
	sar_position_struct cam_pos_runway;

	if((length <= 0.0f) || (width <= 0.0f))
	    return;

	/* We will be skipping the drawing of some elements when rendering
	   with GL_SELECT as it triggers ghosting of these elements (they are
	   drawn and not removed anymore on subsequent frames). This is only
	   done for ground contact check and these elements (thresholds,
	   midway markers, touchdown markers, north-south text...) are not
	   relevant objects. Under GL_SELECT an error is thrown sometimes on
	   my intel hardware:

	   "HW GL_SELECT does not support draw mode GL_LINES_ADJACENCY"

	   GL_LINES_ADJACENCY must be internally use by the OpenGL
	   implementation but ghosting also happens without it
	   sometimes.. This appears to affect only runway elements. I am not
	   sure why it doesn't affect Helipad or anything else
	   apparently... *shrug*. This can well be a manifestation of a Mesa
	   OpenGL bug with intel, as I have not found any other cause.
	*/
	GLint render_mode;
	glGetIntegerv(GL_RENDER_MODE, &render_mode);
	render_mode = GL_RENDER;

	/* Get runway background texture */
	if(SARIsTextureAllocated(scene, tex_num))
	    t = scene->texture_ref[tex_num];
	else
	    t = NULL;

	/* Calculate bounds */
	y_max = (float)(length * 0.5);
	y_min = -y_max;
	x_max = (float)(width * 0.5);
	x_min = -x_max;

	/* Calculate threshold bounds */
	yt_max = y_max - runway->north_displaced_threshold;
	yt_min = y_min + runway->south_displaced_threshold;

	/* Calculate camera position relative to runway's position and
	 * direction.
	 */
	if(cam_pos != NULL)
	{
	    double a[3 * 3], r[3 * 3];

	    a[0] = cam_pos->x - pos->x;
	    a[1] = cam_pos->y - pos->y;
	    a[2] = cam_pos->z - pos->z;

	    /* Rotate matrix a into r */
	    MatrixRotateBank3(a, dir->bank, r);	/* Our bank is negative,
						 * so pass as flipped sign.
						 */
	    MatrixRotatePitch3(r, -dir->pitch, a);
	    MatrixRotateHeading3(a, -dir->heading, r);

	    cam_pos_runway.x = (GLfloat)r[0];
	    cam_pos_runway.y = (GLfloat)r[1];
	    cam_pos_runway.z = (GLfloat)r[2];
	}
	else
	{
	    memset(&cam_pos_runway, 0x00, sizeof(sar_position_struct));
	}

	/* Set up gl states */
	StateGLShadeModel(state, GL_FLAT);

	/* Camera farther than 3 miles away? */
	if(distance > (float)SFMMilesToMeters(3.0))
	{
	    /* Draw simple, no texture with greyish background */
	    V3DTextureSelect(NULL);
	    glColor4f(0.4f, 0.4f, 0.4f, 1.0f);

	    glBegin(GL_QUADS);
	    {
		glNormal3f(0.0f, 1.0f, 0.0f);
		glVertex3f(x_min, 0.0f, -y_max);
		glVertex3f(x_min, 0.0f, -y_min);
		glVertex3f(x_max, 0.0f, -y_min);
		glVertex3f(x_max, 0.0f, -y_max);
	    }
	    glEnd();
	}
	else
	{
	    /* Set texture for runway background if available */
	    if(t == NULL)
	    {
		V3DTextureSelect(NULL);
		glColor4f(0.5f, 0.5f, 0.5f, 1.0f);
	    }
	    else
	    {
		V3DTextureSelect(t);
		glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	    }

	    /* Draw background */
	    glBegin(GL_QUADS);
	    {
		glNormal3f(0.0f, 1.0f, 0.0f);

		glTexCoord2f(0.0f, 1.0f - 0.0f);
		glVertex3f(x_min, 0.0f, -y_max);

		glTexCoord2f(0.0f, 1.0f - 1.0f);
		glVertex3f(x_min, 0.0f, -y_min);

		glTexCoord2f(1.0f, 1.0f - 1.0f);
		glVertex3f(x_max, 0.0f, -y_min);

		glTexCoord2f(1.0f, 1.0f - 0.0f);
		glVertex3f(x_max, 0.0f, -y_max);
	    }
	    glEnd();

	    if(t != NULL)
		V3DTextureSelect(NULL);

	    /* Draw north displaced threshold? */
	    vmodel = runway->north_displaced_threshold_vmodel;
	    if(vmodel != NULL && render_mode != GL_SELECT)
	    {
		glPushMatrix();
		{
		    glTranslatef(
			0.0f,
			0.0f,
			-y_max
		    );
		    glRotatef(-180.0f, 0.0f, 1.0f, 0.0f);
		    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
		    SARVisualModelCallList(vmodel);
		}
		glPopMatrix();
	    }
	    /* Draw south displaced threshold? */
	    vmodel = runway->south_displaced_threshold_vmodel;
	    if(vmodel != NULL && render_mode != GL_SELECT)
	    {
		glPushMatrix();
		{
		    glTranslatef(
			0.0f,
			0.0f,
			-y_min
		    );
		    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
		    SARVisualModelCallList(vmodel);
		}
		glPopMatrix();
	    }

	    /* Draw dashes? */
	    if(runway->dashes > 0)
	    {
		int	dashes = runway->dashes;
		float	dash_width_max = (float)(width * 0.025),
			dash_width_min = -dash_width_max,
			dash_spacing = (float)MAX(
			    (yt_max - yt_min) /
			    ((dashes * 2) + 1),
			    1.0
			),
			dash_cur;

		/* Draw each dash */
		glBegin(GL_QUADS);
		{
		    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
		    glNormal3f(0.0f, 1.0f, 0.0f);
		    for(dash_cur = yt_min + dash_spacing;
			(dash_cur + dash_spacing) < yt_max;
			dash_cur += (float)(dash_spacing * 2)
		    )
		    {
			glVertex3f(
			    dash_width_min,
			    0.0f,
			    -(dash_cur + dash_spacing)
			);
			glVertex3f(
			    dash_width_min,
			    0.0f,
			    -dash_cur
			);
			glVertex3f(
			    dash_width_max,
			    0.0f,
			    -dash_cur
			);
			glVertex3f(
			    dash_width_max,
			    0.0f,
			    -(dash_cur + dash_spacing)
			);
		    }
		}
		glEnd();
	    }

	    /* Draw touchdown markers */
	    vmodel = runway->td_marker_vmodel;
	    if(vmodel != NULL && render_mode != GL_SELECT)
	    {
		glPushMatrix();
		{
		    glTranslatef(
			0.0f,
			0.0f,
			(GLfloat)-(yt_max * 0.75)
		    );
		    glRotatef(-180.0f, 0.0f, 1.0f, 0.0f);
		    SARVisualModelCallList(vmodel);
		}
		glPopMatrix();
		glPushMatrix();
		{
		    glTranslatef(
			0.0f,
			0.0f,
			(GLfloat)-(yt_min * 0.75)
		    );
		    SARVisualModelCallList(vmodel);
		}
		glPopMatrix();
	    }

	    /* Draw midway markers */
	    vmodel = runway->midway_marker_vmodel;
	    if(vmodel != NULL && render_mode != GL_SELECT)
	    {
		glPushMatrix();
		{
		    glTranslatef(
			0.0f,
			0.0f,
			(GLfloat)-(yt_max * 0.5)
		    );
		    glRotatef(-180.0f, 0.0f, 1.0f, 0.0f);
		    SARVisualModelCallList(vmodel);
		}
		glPopMatrix();
		glPushMatrix();
		{
		    glTranslatef(
			0.0f,
			0.0f,
			(GLfloat)-(yt_min * 0.5)
		    );
		    SARVisualModelCallList(vmodel);
		}
		glPopMatrix();
	    }

	    /* Draw threshold */
	    vmodel = runway->threshold_vmodel;
	    if(vmodel != NULL && render_mode != GL_SELECT)
	    {
		glPushMatrix();
		{
		    glTranslatef(
			0.0f,
			0.0f,
			-yt_max
		    );
		    glRotatef(-180.0f, 0.0f, 1.0f, 0.0f);
		    SARVisualModelCallList(vmodel);
		}
		glPopMatrix();
		glPushMatrix();
		{
		    glTranslatef(
			0.0f,
			0.0f,
			-yt_min
		    );
		    SARVisualModelCallList(vmodel);
		}
		glPopMatrix();
	    }

	    /* Draw borders */
	    if(flags & SAR_RUNWAY_FLAG_BORDERS)
	    {
		glBegin(GL_QUADS);
		{
		    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
		    glNormal3f(0.0f, 1.0f, 0.0f);
		    glVertex3f(
			x_min,
			0.0f,
			-yt_min
		    );
		    glVertex3f(
			(GLfloat)(x_min * 0.95),
			0.0f,
			-yt_min
		    );
		    glVertex3f(
			(GLfloat)(x_min * 0.95),
			0.0f,
			-yt_max
		    );
		    glVertex3f(
			x_min,
			0.0f,
			-yt_max
		    );

		    glVertex3f(
			(GLfloat)(x_max * 0.95),
			0.0f,
			-yt_min
		    );
		    glVertex3f(
			x_max,
			0.0f,
			-yt_min
		    );
		    glVertex3f(
			x_max,
			0.0f,
			-yt_max
		    );
		    glVertex3f(
			(GLfloat)(x_max * 0.95),
			0.0f,
			-yt_max
		    );
		}
		glEnd();
	    }

	    /* Draw north label? */
	    vmodel = runway->north_label_vmodel;
	    if((vmodel != NULL) &&
	       (runway->north_label_width > 0.0f) &&
	       render_mode != GL_SELECT
	    )
	    {
		float label_width = runway->north_label_width;

		glPushMatrix();
		{
		    glTranslatef(
			(GLfloat)(label_width * 0.5),
			0.0f,
			(GLfloat)-(yt_max - threshold_length - 10)
		    );
		    glRotatef(-180.0f, 0.0f, 1.0f, 0.0f);
		    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
		    SARVisualModelCallList(vmodel);
		}
		glPopMatrix();
	    }
	    /* Draw south label? */
	    vmodel = runway->south_label_vmodel;
	    if((vmodel != NULL) &&
	       (runway->south_label_width > 0.0f) &&
	       render_mode != GL_SELECT
	    )
	    {
		float label_width = runway->south_label_width;

		glPushMatrix();
		{
		    glTranslatef(
			(GLfloat)(label_width * -0.5),
			0.0f,
			(GLfloat)-(yt_min + threshold_length + 10)
		    );
		    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
		    SARVisualModelCallList(vmodel);
		}
		glPopMatrix();
	    }

	}

	/* Draw lighting? */
	if((runway->edge_light_spacing > 0.0f) ||
	   (flags & SAR_RUNWAY_FLAG_NORTH_GS) ||
	   (flags & SAR_RUNWAY_FLAG_SOUTH_GS) ||
	   (runway->north_approach_lighting_flags != 0) ||
	   (runway->south_approach_lighting_flags != 0)
	)
	{
	    sar_runway_approach_lighting_flags	n_app_flags =
		runway->north_approach_lighting_flags,
						s_app_flags =
		runway->south_approach_lighting_flags;
	    StateGLBoolean lighting = state->lighting;

	    /* Set up gl states */
	    if(lighting)
		StateGLDisable(state, GL_LIGHTING);
	    StateGLDisable(state, GL_POINT_SMOOTH);
	    StateGLPointSize(state, 1.0f);

	    /* Draw edge lighting? */
	    if(runway->edge_light_spacing > 0.0f)
	    {
		float lighting_length = yt_max - yt_min;
		float p, spacing;

		/* Calculate spacing of each light, min 100 meters
		 * apart.
		 */
		spacing = (float)MAX(
		    POW(2, floor(LOG(distance) / LOG(2.5))),
		    100.0
		);
		glBegin(GL_POINTS);
		{
		    if(dc->flir)
			glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
		    else
			glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
		    glNormal3f(0.0f, 1.0f, 0.0f);
		    for(p = 0; p < lighting_length; p += spacing)
		    {
			glVertex3f(x_min, 0.0f, -(yt_min + p));
			glVertex3f(x_max, 0.0f, -(yt_min + p));
		    }
		    glVertex3f(x_min, 0.0f, -yt_max);
		    glVertex3f(x_max, 0.0f, -yt_max);
		}
		glEnd();
	    }

	    /* North alignment lighting (closer than 8 miles) */
	    if((n_app_flags & SAR_RUNWAY_APPROACH_LIGHTING_ALIGN) &&
	       (distance <= (float)SFMMilesToMeters(8.0))
	    )
	    {
		int i;
		float	x, y,
			x_spacing = 10.0f,
			y_spacing = 50.0f;

		glBegin(GL_POINTS);
		{
		    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
		    glNormal3f(0.0f, 1.0f, 0.0f);
		    x = 0.0f;
		    for(i = 0, y = y_max; i < 10; i++, y += y_spacing)
			glVertex3f(x, 0.0f, -y);

		    y = (float)(y_max + (y_spacing * 5));
		    for(i = 0, x = (float)-(x_spacing * 5.0 / 2.0);
			i < 6;
			i++, x += x_spacing
		    )
			glVertex3f(x, 0.0f, -y);

		    x_spacing = 20.0f;
		    y = (float)(y_max + (y_spacing * 1));
		    for(i = 0, x = (float)-(x_spacing * 5.0 / 2.0);
			i < 6;
			i++, x += x_spacing
		    )
			glVertex3f(x, 0.0f, -y);
		}
		glEnd();
	    }
	    /* South alignment lighting (closer than 8 miles) */
	    if((s_app_flags & SAR_RUNWAY_APPROACH_LIGHTING_ALIGN) &&
	       (distance <= (float)SFMMilesToMeters(8.0))
	    )
	    {
		int i;
		float   x, y,
			x_spacing = 10.0f,
			y_spacing = 50.0f;

		glBegin(GL_POINTS);
		{
		    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
		    glNormal3f(0.0f, 1.0f, 0.0f);
		    x = 0.0f;
		    for(i = 0, y = y_min; i < 10; i++, y -= y_spacing)
			glVertex3f(x, 0.0f, -y);

		    y = (float)(y_min - (y_spacing * 5));
		    for(i = 0, x = (float)(x_spacing * 5.0 / 2.0);
			i < 6;
			i++, x -= x_spacing
		    )
			glVertex3f(x, 0.0f, -y);

		    x_spacing = 20.0f;
		    y = (float)(y_min - (y_spacing * 1));
		    for(i = 0, x = (float)(x_spacing * 5.0 / 2.0);
			i < 6;
			i++, x -= x_spacing
		    )
			glVertex3f(x, 0.0f, -y);
		}
		glEnd();
	    }

	    /* North ILS lighting (closer than 8 miles) */
	    if((n_app_flags & SAR_RUNWAY_APPROACH_LIGHTING_ILS_GLIDE) &&
	       (distance <= (float)SFMMilesToMeters(8.0))
	    )
	    {
		int i;
		float   x, y[3],
			x_spacing = 10.0f,
			y_spacing = 50.0f;

		glBegin(GL_POINTS);
		{
		    if(dc->flir)
			glColor4f(0.7f, 0.7f, 0.7f, 1.0f);
		    else
			glColor4f(1.0f, 0.0f, 0.0f, 1.0f);
		    glNormal3f(0.0f, 1.0f, 0.0f);
		    y[0] = (float)(y_max + (y_spacing * 2));
		    y[1] = (float)(y_max + (y_spacing * 3));
		    y[2] = (float)(y_max + (y_spacing * 4));
		    for(i = 0, x = (float)-(x_spacing * 5.0 / 2.0);
			i < 6;
			i++, x += x_spacing
		    )
		    {
			glVertex3f(x, 0.0f, -y[0]);
			glVertex3f(x, 0.0f, -y[1]);
			glVertex3f(x, 0.0f, -y[2]);
		    }
		}
		glEnd();
	    }
	    /* South ILS lighting (closer than 8 miles) */
	    if((s_app_flags & SAR_RUNWAY_APPROACH_LIGHTING_ILS_GLIDE) &&
	       (distance <= (float)SFMMilesToMeters(8.0))
	    )
	    {
		int i;
		float   x, y[3],
			x_spacing = 10.0f,
			y_spacing = 50.0f;

		glBegin(GL_POINTS);
		{
		    if(dc->flir)
			glColor4f(0.7f, 0.7f, 0.7f, 1.0f);
		    else
			glColor4f(1.0f, 0.0f, 0.0f, 1.0f);
		    glNormal3f(0.0f, 1.0f, 0.0f);
		    y[0] = (float)(y_min - (y_spacing * 2));
		    y[1] = (float)(y_min - (y_spacing * 3));
		    y[2] = (float)(y_min - (y_spacing * 4));
		    for(i = 0, x = (float)(x_spacing * 5.0 / 2.0);
			i < 6;
			i++, x -= x_spacing
		    )
		    {
			glVertex3f(x, 0.0f, -y[0]);
			glVertex3f(x, 0.0f, -y[1]);
			glVertex3f(x, 0.0f, -y[2]);
		    }
		}
		glEnd();
	    }

	    /* North glide slope (closer than 5 miles)? */
	    if((flags & SAR_RUNWAY_FLAG_NORTH_GS) &&
	       (distance <= (float)SFMMilesToMeters(5.0)) &&
	       (cam_pos_runway.y > 0.0f)
	    )
	    {
		float	c, gsa = 3.0f, gsa_tolor = 1.0f,
			p = (float)(yt_max * 0.75),
			dy = cam_pos_runway.y - p,
			dz = cam_pos_runway.z - pos->z,
			theta = (float)RADTODEG(
			    atan2(dz, dy)
			);

		glBegin(GL_LINES);
		{
		    /* Set color for "closer/lower" marker, calculate
		     * c as the coefficient amount of "whiteness".
		     */
		    c = (float)CLIP(
			(theta - gsa) / gsa_tolor,
			0.0, 1.0
		    );
		    if(dc->flir)
		    {
			float g = (float)((c * 0.5) + 0.5);
			glColor4f(g, g, g, 1.0f);
		    }
		    else
			glColor4f(1.0f, c, c, 1.0f);

		    glNormal3f(0.0f, 1.0f, 0.0f);
		    glVertex3f(
			(GLfloat)(x_min - 23.0),
			0.0f,
			(GLfloat)-(p + 50.0)
		    );
		    glVertex3f(
			(GLfloat)(x_min - 7.0),
			0.0f,
			(GLfloat)-(p + 50.0)
		    );
		    glVertex3f(
			(GLfloat)(x_max + 23.0),
			0.0f,
			(GLfloat)-(p + 50.0)
		    );
		    glVertex3f(
			(GLfloat)(x_max + 7.0),
			0.0f,
			(GLfloat)-(p + 50.0)
		    );

		    /* Set color for "further/upper" marker, calculate
		     * c as the coefficient amount of "whiteness".
		     */
		    c = (float)CLIP(
			(theta - gsa + gsa_tolor) / gsa_tolor,
			0.0, 1.0
		    );
		    if(dc->flir)
			glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
		    else
			glColor4f(1.0f, c, c, 1.0f);

		    glVertex3f(
			(GLfloat)(x_min - 23.0),
			0.0f,
			(GLfloat)-(p - 50.0)
		    );
		    glVertex3f(
			(GLfloat)(x_min - 7.0),
			0.0f,
			(GLfloat)-(p - 50.0)
		    );
		    glVertex3f(
			(GLfloat)(x_max + 23.0),
			0.0f,
			(GLfloat)-(p - 50.0)
		    );
		    glVertex3f(
			(GLfloat)(x_max + 7.0),
			0.0f,
			(GLfloat)-(p - 50.0)
		    );
		}
		glEnd();
	    }
	    /* South glide slope (closer than 5 miles)? */
	    if((flags & SAR_RUNWAY_FLAG_SOUTH_GS) &&
	       (distance <= (float)SFMMilesToMeters(5.0)) &&
	       (cam_pos_runway.y < 0.0f)
	    )
	    {
		float   c, gsa = 3.0f, gsa_tolor = 1.0f,
			p = (float)(yt_min * 0.75),
			dy = p - cam_pos_runway.y,
			dz = cam_pos_runway.z - pos->z,
			theta = (float)RADTODEG(
			    atan2(dz, dy)
			);

		glBegin(GL_LINES);
		{
		    /* Set color for "closer/lower" marker, calculate
		     * c as the coefficient amount of "whiteness".
		     */
		    c = (float)CLIP(
			(theta - gsa) / gsa_tolor,
			0.0, 1.0
		    );
		    if(dc->flir)
		    {
			float g = (float)((c * 0.5) + 0.5);
			glColor4f(g, g, g, 1.0f);
		    }
		    else
			glColor4f(1.0f, c, c, 1.0f);

		    glNormal3f(0.0f, 1.0f, 0.0f);
		    glVertex3f(
			(GLfloat)(x_min - 23.0),
			0.0f,
			(GLfloat)-(p - 50.0)
		    );
		    glVertex3f(
			(GLfloat)(x_min - 7.0),
			0.0f,
			(GLfloat)-(p - 50.0)
		    );
		    glVertex3f(
			(GLfloat)(x_max + 23.0),
			0.0f,
			(GLfloat)-(p - 50.0)
		    );
		    glVertex3f(
			(GLfloat)(x_max + 7.0),
			0.0f,
			(GLfloat)-(p - 50.0)
		    );

		    /* Set color for "further/upper" marker, calculate
		     * c as the coefficient amount of "whiteness".
		     */
		    c = (float)CLIP(
			(theta - gsa + gsa_tolor) / gsa_tolor,
			0.0, 1.0
		    );
		    if(dc->flir)
			glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
		    else
			glColor4f(1.0f, c, c, 1.0f);

		    glVertex3f(
			(GLfloat)(x_min - 23.0),
			0.0f,
			(GLfloat)-(p + 50.0)
		    );
		    glVertex3f(
			(GLfloat)(x_min - 7.0),
			0.0f,
			(GLfloat)-(p + 50.0)
		    );
		    glVertex3f(
			(GLfloat)(x_max + 23.0),
			0.0f,
			(GLfloat)-(p + 50.0)
		    );
		    glVertex3f(
			(GLfloat)(x_max + 7.0),
			0.0f,
			(GLfloat)-(p + 50.0)
		    );
		}
		glEnd();
	    }

	    /* North tracer lighting (closer than 6 miles)? */
	    if((n_app_flags & SAR_RUNWAY_APPROACH_LIGHTING_TRACER) &&
	       (distance <= (float)SFMMilesToMeters(6.0))
	    )
	    {
		int nlights, light_num;
		float spacing, end;

		spacing = 50.0f;
		end = (float)(y_max + (spacing * 10));
		nlights = (int)((end - y_max) / spacing);

		StateGLEnable(state, GL_POINT_SMOOTH);
		StateGLPointSize(state, 2.0f);
		glBegin(GL_POINTS);
		{
		    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
		    glNormal3f(0.0f, 1.0f, 0.0f);
		    light_num = (int)(nlights * (1.0 - SAR_GRAD_ANIM_COEFF(
			runway->tracer_anim_pos
		    )));
		    glVertex3f(
			0.0f,
			0.0f,
			(GLfloat)-(y_max + (light_num * spacing))
		    );
		}
		glEnd();
		StateGLDisable(state, GL_POINT_SMOOTH);
		StateGLPointSize(state, 1.0f);
	    }
	    /* South tracer lighting (closer than 6 miles)? */
	    if((s_app_flags & SAR_RUNWAY_APPROACH_LIGHTING_TRACER) &&
	       (distance <= (float)SFMMilesToMeters(6.0))
	    )
	    {
		int nlights, light_num;
		float spacing, end;

		spacing = 50.0f;
		end = (float)(y_min - (spacing * 10));
		nlights = (int)((y_min - end) / spacing);

		StateGLEnable(state, GL_POINT_SMOOTH);
		StateGLPointSize(state, 2.0f);
		glBegin(GL_POINTS);
		{
		    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
		    glNormal3f(0.0f, 1.0f, 0.0f);
		    light_num = (int)(nlights * (1.0 - SAR_GRAD_ANIM_COEFF(
			runway->tracer_anim_pos
		    )));
		    glVertex3f(
			0.0f,
			0.0f,
			(GLfloat)-(y_min - (light_num * spacing))
		    );
		}
		glEnd();
		StateGLDisable(state, GL_POINT_SMOOTH);
		StateGLPointSize(state, 1.0f);
	    }

	    /* North threshold lighting (closer than 4 miles)? */
	    if((n_app_flags & SAR_RUNWAY_APPROACH_LIGHTING_END) &&
	       (distance <= (float)SFMMilesToMeters(4.0))
	    )
	    {
		float p, spacing;

		spacing = (float)(width / 7);

		/* Draw end lighting */
		glBegin(GL_POINTS);
		{
		    /* North end lights */
		    if(dc->flir)
			glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
		    else if(cam_pos_runway.y > yt_max)
			glColor4f(0.0f, 1.0f, 0.0f, 1.0f);
		    else
			glColor4f(1.0f, 0.0f, 0.0f, 1.0f);
		    glNormal3f(0.0f, 1.0f, 0.0f);
		    for(p = 0.0f; p <= x_max; p += spacing)
			glVertex3f(p, 0.0f, -yt_max);
		    for(p = -spacing; p >= x_min; p -= spacing)
			glVertex3f(p, 0.0f, -yt_max);
		}
		glEnd();
	    }
	    /* South threshold lighting (closer than 4 miles)? */
	    if((s_app_flags & SAR_RUNWAY_APPROACH_LIGHTING_END) &&
	       (distance <= (float)SFMMilesToMeters(4.0))
	    )
	    {
		float p, spacing;

		spacing = (float)(width / 7);

		/* Draw end lighting */
		glBegin(GL_POINTS);
		{
		    /* South end lights */
		    if(dc->flir)
			glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
		    else if(cam_pos_runway.y < yt_min)
			glColor4f(0.0f, 1.0f, 0.0f, 1.0f);
		    else
			glColor4f(1.0f, 0.0f, 0.0f, 1.0f);
		    glNormal3f(0.0f, 1.0f, 0.0f);
		    for(p = 0.0f; p <= x_max; p += spacing)
			glVertex3f(p, 0.0f, -yt_min);
		    for(p = -spacing; p >= x_min; p -= spacing)
			glVertex3f(p, 0.0f, -yt_min);

		}
		glEnd();
	    }

/*
SAR_RUNWAY_APPROACH_LIGHTING_END        (1 << 0)
SAR_RUNWAY_APPROACH_LIGHTING_TRACER     (1 << 1)
SAR_RUNWAY_APPROACH_LIGHTING_ALIGN      (1 << 2)
SAR_RUNWAY_APPROACH_LIGHTING_ILS_GLIDE  (1 << 3)
 */


	    /* Need to animation tracer light position? */
	    if((n_app_flags & SAR_RUNWAY_APPROACH_LIGHTING_TRACER) ||
	       (s_app_flags & SAR_RUNWAY_APPROACH_LIGHTING_TRACER)
	    )
		runway->tracer_anim_pos += (sar_grad_anim_t)(
		    (float)runway->tracer_anim_rate *
		    time_compensation * time_compression
		);

	    /* Restore gl states */
	    if(lighting)
		StateGLEnable(state, GL_LIGHTING);
	}


	/* Restore gl states */
	StateGLShadeModel(state, shade_model_mode);
}

/*
 *      Draws runway for display on map.
 *
 *      All inputs are assumed valid.
 */
void SARDrawRunwayMap(
	sar_dc_struct *dc, sar_object_struct *obj_ptr,
	sar_object_runway_struct *runway,
	float icon_len
)
{
	gw_display_struct *display = dc->display;
	state_gl_struct *state = &display->state_gl;
	GLenum shade_model_mode = state->shade_model_mode;
	float x_min, x_max, y_min, y_max;

	/* Calculate bounds */
	x_max = MAX(runway->width / 2, icon_len / 4);
	x_min = -x_max;
	y_max = MAX(runway->length / 2, icon_len / 2);
	y_min = -y_max;

	/* No texture */
	V3DTextureSelect(NULL);

	/* Set up gl states */
	StateGLShadeModel(state, GL_FLAT);

	glBegin(GL_QUADS);
	{
	    /* Blue background */
	    glColor4f(0.0f, 0.0f, 1.0f, 1.0f);
	    glNormal3f(0.0f, 1.0f, 0.0f);
	    glVertex3f(x_min, 0, -y_max);
	    glVertex3f(x_min, 0, -y_min);
	    glVertex3f(x_max, 0, -y_min);
	    glVertex3f(x_max, 0, -y_max);

	    /* White stripe */
	    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	    glVertex3f(x_min * 0.5f, 0.0f, -(y_max - (x_max * 0.5f)));
	    glVertex3f(x_min * 0.5f, 0.0f, -(y_min + (x_max * 0.5f)));
	    glVertex3f(x_max * 0.5f, 0.0f, -(y_min + (x_max * 0.5f)));
	    glVertex3f(x_max * 0.5f, 0.0f, -(y_max - (x_max * 0.5f)));
	}
	glEnd();

	/* Restore gl states */
	StateGLShadeModel(state, shade_model_mode);
}
