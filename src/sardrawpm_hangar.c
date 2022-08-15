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
#include <math.h>

#ifdef __MSW__
# include <windows.h>
#endif

#include <GL/gl.h>
#include <GL/glu.h>

#include "gw.h"

#include "sarreality.h"
#include "obj.h"
#include "objutils.h"
#include "sfm.h"
#include "sar.h"
#include "sardraw.h"
#include "sardrawpm.h"
#include "sardrawdefs.h"
#include "v3dmp.h"


void SARDrawPremodeledHangar(SAR_DRAW_PREMODELED_PROTOTYPE);

#define HANGAR_ROOF_SLOPES obj_premodeled_ptr->multipurpose_int0
#define IS_LIGHTED_AT_NIGHT obj_premodeled_ptr->multipurpose_int1

#define DRAW_FRONT_FACADE \
glBegin(GL_POLYGON);\
{\
    glNormal3f(0.0f, 0.0f, normal);\
    glTexCoord2f(0.5f + day_night_tex_offset, (1.0 - 0.577143f)/2.0);\
    glVertex3f(x1, z_max, y1);\
    if ( HANGAR_ROOF_SLOPES == 4 )\
    {\
	glTexCoord2f(0.375f + day_night_tex_offset, (1.0 - 0.788571f)/2.0);\
	glVertex3f(x1/2.0, z_roof_middle, y1);\
    }\
    glTexCoord2f(0.25f + day_night_tex_offset, (1.0 - 1.0f)/2.0);\
    glVertex3f(0.0f, z_roof_top, y1);\
    if ( HANGAR_ROOF_SLOPES == 4 )\
    {\
	glTexCoord2f(0.125f + day_night_tex_offset, (1.0 - 0.788571f)/2.0);\
	glVertex3f(x2/2.0, z_roof_middle, y1);\
    }\
    glTexCoord2f(0.0f + day_night_tex_offset, (1.0 - 0.577143f)/2.0);\
    glVertex3f(x2, z_max, y1);\
    glTexCoord2f(0.0f + day_night_tex_offset, (1.0 - 0.0f)/2.0);\
    glVertex3f(x2, 0.0f, y1);\
    glTexCoord2f(0.5f + day_night_tex_offset, (1.0 - 0.0f)/2.0);\
    glVertex3f(x1, 0.0f, y1);\
}\
glEnd();

#define DRAW_REAR_FACADE_TOP \
glBegin(GL_POLYGON);\
{\
    glNormal3f(0.0f, 0.0f, normal);\
    glTexCoord2f(0.5f + day_night_tex_offset, (1.0 - 0.577143f)/2.0);\
    glVertex3f(x1, z_max, y1);\
    if ( HANGAR_ROOF_SLOPES == 4 )\
    {\
	glTexCoord2f(0.375f + day_night_tex_offset, (1.0 - 0.788571f)/2.0);\
	glVertex3f(x1/2.0, z_roof_middle, y1);\
    }\
    glTexCoord2f(0.25f + day_night_tex_offset, (1.0 - 1.0f)/2.0);\
    glVertex3f(0.0f, z_roof_top, y1);\
    if ( HANGAR_ROOF_SLOPES == 4 )\
    {\
	glTexCoord2f(0.125f + day_night_tex_offset, (1.0 - 0.788571f)/2.0);\
	glVertex3f(x2/2.0, z_roof_middle, y1);\
    }\
    glTexCoord2f(0.0f + day_night_tex_offset, (1.0 - 0.577143f)/2.0);\
    glVertex3f(x2, z_max, y1);\
}\
glEnd();

#define DRAW_REAR_FACADE_BOTTOM \
glBegin(GL_POLYGON);\
{\
    glNormal3f(0.0f + day_night_tex_offset, 0.0f, normal);\
    glTexCoord2f(0.5f + day_night_tex_offset, 0.5f);\
    glVertex3f(x1, z_max, y1);\
    glTexCoord2f(0.0f + day_night_tex_offset, 0.5f);\
    glVertex3f(x2, z_max, y1);\
    glTexCoord2f(0.0f + day_night_tex_offset, 1.0f);\
    glVertex3f(x2, 0.0f, y1);\
    glTexCoord2f(0.5f + day_night_tex_offset, 1.0f);\
    glVertex3f(x1, 0.0f, y1);\
}\
glEnd();

#define DRAW_SIDE_WALL \
glBegin(GL_QUADS);\
{\
    glNormal3f(normal, 0.0f, 0.0f);\
    glTexCoord2f(0.5f + day_night_tex_offset, 1.0f);\
    glVertex3f(x1, 0.0f, y2);\
    glTexCoord2f(0.0f + day_night_tex_offset, 1.0f);\
    glVertex3f(x1, 0.0f, y1);\
    glTexCoord2f(0.0f + day_night_tex_offset, 0.5f);\
    glVertex3f(x1, z_max, y1);\
    glTexCoord2f(0.5f + day_night_tex_offset, 0.5f);\
    glVertex3f(x1, z_max, y2);\
}\
glEnd();


/*
 *      Draws a premodeled hangar.
 */
void SARDrawPremodeledHangar(SAR_DRAW_PREMODELED_PROTOTYPE)
{
	gw_display_struct *display = dc->display;
	state_gl_struct *state = &display->state_gl;
	sar_scene_struct *scene = dc->scene;
	StateGLBoolean lighting = state->lighting;   
	GLenum shade_model_mode = state->shade_model_mode;
	float x_min, x_max, x1, x2, y_min, y_max, y1, y2, z_max, z_roof_middle, z_roof_top, normal, day_night_tex_offset;
	int i, tex_num;
	
	
	/* Calculate bounds */
	x_max = obj_premodeled_ptr->length / 2.0f;
	x_min = -x_max;

	y_max = obj_premodeled_ptr->width / 2.0f;
	y_min = -y_max;

	z_max = obj_premodeled_ptr->height; // side walls height
	z_roof_middle = z_max + ( x_max / 2.0 ) * 0.268; // 15 degrees slope from side to middle or top
	z_roof_top = z_roof_middle + ( x_max / 2.0 ) * 0.07; // 4 degrees slope from middle to top

	/* Set up gl states */
	StateGLShadeModel(state, GL_FLAT);
	
	obj_ptr->temperature = 0.3; // it's a hangar, so it's not very hot
	
	/* Texture select */
	if(dc->flir)
	{
	    StateGLDisable(state, GL_LIGHTING);
	    V3DTextureSelect(NULL);
	    SARDrawSetColorFLIRTemperature(obj_ptr->temperature);
	}
	else if(IS_LIGHTED_AT_NIGHT && scene->tod_code != SAR_TOD_CODE_DAY)
	{
	    /* Night Walls Lighting */
	    StateGLDisable(state, GL_LIGHTING);

	    i = 0; // walls texture
	    if(i < SAR_OBJ_PREMODEL_MAX_TEXTURES)
		tex_num = obj_premodeled_ptr->tex_num[i];
	    else
		tex_num = -1;
	    if(SARIsTextureAllocated(scene, tex_num))
		V3DTextureSelect(scene->texture_ref[tex_num]);
	    else
		V3DTextureSelect(NULL);
	    
	    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

	    day_night_tex_offset = 0.5f; // set texture to night texture

	}
	else
	{
	    i = 0; // walls texture
	    if(i < SAR_OBJ_PREMODEL_MAX_TEXTURES)
		tex_num = obj_premodeled_ptr->tex_num[i];
	    else
		tex_num = -1;
	    if(SARIsTextureAllocated(scene, tex_num))
		V3DTextureSelect(scene->texture_ref[tex_num]);
	    else
		V3DTextureSelect(NULL);
	    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	    
	    day_night_tex_offset = 0.0f; // set texture to day texture
	}

	/* North facade, outside */
	normal = -1.0f;
	x1 = x_min;
	x2 = x_max;
	y1 = y_min;
	DRAW_FRONT_FACADE
	
	/* West wall, outside */
	normal = -1.0f;
	x1 = x_min;
	y1 = y_max;
	y2 = y_min;
	DRAW_SIDE_WALL
	
	/* East wall, outside */
	normal = 1.0f;
	x1 = x_max;
	y1 = y_min;
	y2 = y_max;
	DRAW_SIDE_WALL
	
	/* South facade top, outside */
	normal = 1.0f;
	x1 = x_max;
	x2 = x_min;
	y1 = y_max;
	DRAW_REAR_FACADE_TOP
	
	/* South facade bottom, outside */
	normal = 1.0f;
	x1 = x_max;
	x2 = x_min;
	y1 = y_max;
	DRAW_REAR_FACADE_BOTTOM
	
	day_night_tex_offset = 0.0f; // set texture to day texture. If not, windows will light when seen from inside.
		
	/* South facade bottom, inside */
	normal = -1.0f;
	x1 = x_min;
	x2 = x_max;
	y1 = y_max;
	DRAW_REAR_FACADE_BOTTOM
	
	/* Draw inside walls only if cam is close enough to see them */
	if(camera_dist < 1500.0f)
	{
	    /* North facade, inside */
	    normal = 1.0f;
	    x1 = x_max;
	    x2 = x_min;
	    y1 = y_min;
	    DRAW_FRONT_FACADE
	    
	    /* South facade top, inside */
	    normal = -1.0f;
	    x1 = x_min;
	    x2 = x_max;
	    y1 = y_max;
	    DRAW_REAR_FACADE_TOP
	    
	    /* West wall, inside */
	    normal = 1.0f;
	    x1 = x_min;
	    y1 = y_min;
	    y2 = y_max;
	    DRAW_SIDE_WALL
	    
	    /* East wall, inside */
	    normal = -1.0f;
	    x1 = x_max;
	    y1 = y_max;
	    y2 = y_min;
	    DRAW_SIDE_WALL
	}
	
	/* Roof */
	if(dc->flir)
	{
	    StateGLDisable(state, GL_LIGHTING);
	    V3DTextureSelect(NULL);
	    SARDrawSetColorFLIRTemperature(obj_ptr->temperature);
	}
	else
	{
	    i = 1; // roof texture
	    if(i < SAR_OBJ_PREMODEL_MAX_TEXTURES)
		tex_num = obj_premodeled_ptr->tex_num[i];
	    else
		tex_num = -1;
	    if(SARIsTextureAllocated(scene, tex_num))
		V3DTextureSelect(scene->texture_ref[tex_num]);
	    else
		V3DTextureSelect(NULL);
	    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	    
	    if(lighting)
		StateGLEnable(state, GL_LIGHTING);
	    day_night_tex_offset = 0.0f; // there is no night texture in roof texture file
	}
	
	if ( HANGAR_ROOF_SLOPES == 4)
	{
	    /* 4 slopes roof, outside */
	    glBegin(GL_QUADS);
	    
	    glNormal3f(-0.2588f, 0.9659f, 0.0f);
	    glTexCoord2f(1.0f, 1.0f);
	    glVertex3f(x_min, z_max, y_min);
	    glTexCoord2f(0.0f, 1.0f);
	    glVertex3f(x_min, z_max, y_max);
	    glTexCoord2f(0.0f, 0.0f);
	    glVertex3f(x_min/2.0, z_roof_middle, y_max);
	    glTexCoord2f(1.0f, 0.0f);
	    glVertex3f(x_min/2.0, z_roof_middle, y_min);
	    
	    glNormal3f(-0.0698f, 0.9976f, 0.0f);
	    glTexCoord2f(1.0f, 1.0f);
	    glVertex3f(x_min/2.0, z_roof_middle, y_min);
	    glTexCoord2f(0.0f, 1.0f);
	    glVertex3f(x_min/2.0, z_roof_middle, y_max);
	    glTexCoord2f(0.0f, 0.0f);
	    glVertex3f(0.0, z_roof_top, y_max);
	    glTexCoord2f(1.0f, 0.0f);
	    glVertex3f(0.0, z_roof_top, y_min);
	    
	    glNormal3f(0.2588f, 0.9659f, 0.0f);
	    glTexCoord2f(1.0f, 1.0f);
	    glVertex3f(0.0, z_roof_top, y_min);
	    glTexCoord2f(0.0f, 1.0f);
	    glVertex3f(0.0, z_roof_top, y_max);
	    glTexCoord2f(0.0f, 0.0f);
	    glVertex3f(x_max/2.0, z_roof_middle, y_max);
	    glTexCoord2f(1.0f, 0.0f);
	    glVertex3f(x_max/2.0, z_roof_middle, y_min);
	    
	    glNormal3f(0.0698f, 0.9976f, 0.0f);
	    glTexCoord2f(1.0f, 1.0f);
	    glVertex3f(x_max/2.0, z_roof_middle, y_min);
	    glTexCoord2f(0.0f, 1.0f);
	    glVertex3f(x_max/2.0, z_roof_middle, y_max);
	    glTexCoord2f(0.0f, 0.0f);
	    glVertex3f(x_max, z_max, y_max);
	    glTexCoord2f(1.0f, 0.0f);
	    glVertex3f(x_max, z_max, y_min);
	    
	    glEnd();
	    
	    /* Draw 4 slopes roof inside only if cam is close enough to see them */
	    if(camera_dist < 100.0f)
	    {
		if(IS_LIGHTED_AT_NIGHT && scene->tod_code != SAR_TOD_CODE_DAY)
		    StateGLDisable(state, GL_LIGHTING);
		else if(lighting)
		    StateGLEnable(state, GL_LIGHTING);
	    
		glBegin(GL_QUADS);
		
		glNormal3f(0.2588f, -0.9659f, 0.0f);
		glTexCoord2f(0.0f, 0.0f);
		glVertex3f(x_min, z_max, y_min);
		glTexCoord2f(0.0f, 1.0f);
		glVertex3f(x_min/2.0, z_roof_middle, y_min);
		glTexCoord2f(1.0f, 1.0f);
		glVertex3f(x_min/2.0, z_roof_middle, y_max);
		glTexCoord2f(1.0f, 0.0f);
		glVertex3f(x_min, z_max, y_max);
		
		glNormal3f(0.0698f, -0.9976f, 0.0f);
		glTexCoord2f(0.0f, 0.0f);
		glVertex3f(x_min/2.0, z_roof_middle, y_min);
		glTexCoord2f(0.0f, 1.0f);
		glVertex3f(0.0, z_roof_top, y_min);
		glTexCoord2f(1.0f, 1.0f);
		glVertex3f(0.0, z_roof_top, y_max);
		glTexCoord2f(1.0f, 0.0f);
		glVertex3f(x_min/2.0, z_roof_middle, y_max);
		
		glNormal3f(-0.2588f, -0.9659f, 0.0f);
		glTexCoord2f(0.0f, 0.0f);
		glVertex3f(0.0, z_roof_top, y_min);
		glTexCoord2f(0.0f, 1.0f);
		glVertex3f(x_max/2.0, z_roof_middle, y_min);
		glTexCoord2f(1.0f, 1.0f);
		glVertex3f(x_max/2.0, z_roof_middle, y_max);
		glTexCoord2f(1.0f, 0.0f);
		glVertex3f(0.0, z_roof_top, y_max);
		
		glNormal3f(-0.0698f, -0.9976f, 0.0f);
		glTexCoord2f(0.0f, 0.0f);
		glVertex3f(x_max/2.0, z_roof_middle, y_min);
		glTexCoord2f(0.0f, 1.0f);
		glVertex3f(x_max, z_max, y_min);
		glTexCoord2f(1.0f, 1.0f);
		glVertex3f(x_max, z_max, y_max);
		glTexCoord2f(1.0f, 0.0f);
		glVertex3f(x_max/2.0, z_roof_middle, y_max);
		    
		glEnd();
		    
		if(lighting)
		    StateGLEnable(state, GL_LIGHTING);
	    }
	}
	else // 2 roof slopes
	{
	    /* 2 slopes roof, outside */
	    glBegin(GL_QUADS);
	    
	    glNormal3f(-0.2588f, 0.9659f, 0.0f);
	    glTexCoord2f(1.0f, 1.0f);
	    glVertex3f(x_min, z_max, y_min);
	    glTexCoord2f(0.0f, 1.0f);
	    glVertex3f(x_min, z_max, y_max);
	    glTexCoord2f(0.0f, 0.0f);
	    glVertex3f(0.0, z_roof_top, y_max);
	    glTexCoord2f(1.0f, 0.0f);
	    glVertex3f(0.0, z_roof_top, y_min);
	    
	    glNormal3f(0.2588f, 0.9659f, 0.0f);
	    glTexCoord2f(1.0f, 1.0f);
	    glVertex3f(0.0, z_roof_top, y_min);
	    glTexCoord2f(0.0f, 1.0f);
	    glVertex3f(0.0, z_roof_top, y_max);
	    glTexCoord2f(0.0f, 0.0f);
	    glVertex3f(x_max, z_max, y_max);
	    glTexCoord2f(1.0f, 0.0f);
	    glVertex3f(x_max, z_max, y_min);
	    
	    glEnd();
	    
	    /* Draw 2 slopes roof inside only if cam if cam is close enough to see them */
	    if(camera_dist < 100.0f)
	    {
		if(IS_LIGHTED_AT_NIGHT && scene->tod_code != SAR_TOD_CODE_DAY)
		    StateGLDisable(state, GL_LIGHTING);
		else if(lighting)
		    StateGLEnable(state, GL_LIGHTING);
		
		glBegin(GL_QUADS);
		    
		glNormal3f(0.2588f, -0.9659f, 0.0f);
		glTexCoord2f(0.0f, 0.0f);
		glVertex3f(x_min, z_max, y_min);
		glTexCoord2f(0.0f, 1.0f);
		glVertex3f(0.0, z_roof_top, y_min);
		glTexCoord2f(1.0f, 1.0f);
		glVertex3f(0.0, z_roof_top, y_max);
		glTexCoord2f(1.0f, 0.0f);
		glVertex3f(x_min, z_max, y_max);
		
		glNormal3f(-0.2588f, -0.9659f, 0.0f);
		glTexCoord2f(0.0f, 0.0f);
		glVertex3f(0.0, z_roof_top, y_min);
		glTexCoord2f(0.0f, 1.0f);
		glVertex3f(x_max, z_max, y_min);
		glTexCoord2f(1.0f, 1.0f);
		glVertex3f(x_max, z_max, y_max);
		glTexCoord2f(1.0f, 0.0f);
		glVertex3f(0.0, z_roof_top, y_max);
		
		glEnd();
		
		if(lighting)
		    StateGLEnable(state, GL_LIGHTING);
	    }
	}
	
	V3DTextureSelect(NULL);
	
	if(!dc->flir)
	{
	    /* Floor */
	    i = 2; // floor texture
	    if(i < SAR_OBJ_PREMODEL_MAX_TEXTURES)
		tex_num = obj_premodeled_ptr->tex_num[i];
	    else
		tex_num = -1;
	    
	    if(SARIsTextureAllocated(scene, tex_num))
	    {
		V3DTextureSelect(scene->texture_ref[tex_num]);
		
		if(IS_LIGHTED_AT_NIGHT && scene->tod_code != SAR_TOD_CODE_DAY)
		    StateGLDisable(state, GL_LIGHTING);
		else if(lighting)
		    StateGLEnable(state, GL_LIGHTING);
	    }
	    else // no floor texture, apply grey color instead
	    {
		V3DTextureSelect(NULL);
		if(IS_LIGHTED_AT_NIGHT && scene->tod_code != SAR_TOD_CODE_DAY) // night
		{
		    StateGLDisable(state, GL_LIGHTING);
		    glColor4f(0.6f, 0.6f, 0.6f, 1.0f);
		}
		else // day
		{
		    if(lighting)
			StateGLEnable(state, GL_LIGHTING);
		    glColor4f(0.4f, 0.4f, 0.4f, 1.0f);
		}
	    }
	    
	    glBegin(GL_QUADS);
	    glNormal3f(0.0f, 1.0f, 0.0f);
	    glTexCoord2f(1.0f, 1.0f);
	    glVertex3f(x_min, 0.02f, y_min);
	    glTexCoord2f(1.0f, 0.0f);
	    glVertex3f(x_min, 0.02f, y_max);
	    glTexCoord2f(0.0f, 0.0f);
	    glVertex3f(x_max, 0.02f, y_max);
	    glTexCoord2f(0.0f, 1.0f);
	    glVertex3f(x_max, 0.02f, y_min);
	    glEnd();
	
	    /* Draw inside roof structure only if cam if cam is close enough to see them */
	    if( (camera_dist < 100.0f) )
	    {
		#define STRIP_WIDTH 0.4f
		
		if(IS_LIGHTED_AT_NIGHT && scene->tod_code != SAR_TOD_CODE_DAY)
		{
		    StateGLDisable(state, GL_LIGHTING);
		    glColor4f(0.839f, 0.800f, 0.631f, 1.0f); // pale yellow
		}
		else
		{
		    if(lighting)
			StateGLEnable(state, GL_LIGHTING);
		    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
		}
		
		int light_strips = floor( obj_premodeled_ptr->length / (3.0f + STRIP_WIDTH) ) + 1;
		float step = obj_premodeled_ptr->length / (float)light_strips;
		float x_start, x_end;
		
		if ( light_strips % 2 ) // odd number of light_strips
		    x_start = -step * (light_strips / 2) - (STRIP_WIDTH / 2.0f);
		else
		    x_start = -step * (light_strips / 2) + (step / 2) - (STRIP_WIDTH / 2.0f);
		x_end = x_start + STRIP_WIDTH;
		
		glBegin(GL_QUADS);
		for ( ; light_strips > 0; light_strips-- )
		{
		    glNormal3f(0.0f, -1.0f, 0.0f);
		    glVertex3f(x_start, z_max, y_min);
		    glVertex3f(x_end, z_max, y_min);
		    glVertex3f(x_end, z_max, y_max);
		    glVertex3f(x_start, z_max, y_max);
		    x_start += step;
		    x_end += step;
		}
		glEnd();
	    }
	}
	
	/* Restore gl states */
	StateGLShadeModel(state, shade_model_mode);
	if(lighting)
	    StateGLEnable(state, GL_LIGHTING);
}
