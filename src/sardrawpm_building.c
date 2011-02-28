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


void SARDrawPremodeledBuilding(SAR_DRAW_PREMODELED_PROTOTYPE);


/*
 *      Draws a premodeled building.
 */
void SARDrawPremodeledBuilding(SAR_DRAW_PREMODELED_PROTOTYPE)
{
	gw_display_struct *display = dc->display;
	state_gl_struct *state = &display->state_gl;
	sar_scene_struct *scene = dc->scene;
	StateGLBoolean lighting = state->lighting;   
	GLenum shade_model_mode = state->shade_model_mode;
	float x_min, x_max, y_min, y_max, z_max;
	int i, tex_num;


	/* Calculate bounds */
	x_max = obj_premodeled_ptr->length / 2.0f;
	x_min = -x_max;

	y_max = obj_premodeled_ptr->width / 2.0f;
	y_min = -y_max;

	z_max = obj_premodeled_ptr->height;


	/* Set up gl states */
	StateGLShadeModel(state, GL_FLAT);


	/* Walls */
	if(dc->flir)
	{
	    StateGLDisable(state, GL_LIGHTING);
	    V3DTextureSelect(NULL);
	    SARDrawSetColorFLIRTemperature(obj_ptr->temperature);
	}
	else
	{
	    i = 0;
	    if(i < SAR_OBJ_PREMODEL_MAX_TEXTURES)
		tex_num = obj_premodeled_ptr->tex_num[i];
	    else
		tex_num = -1;
	    if(SARIsTextureAllocated(scene, tex_num))
		V3DTextureSelect(scene->texture_ref[tex_num]);
	    else
		V3DTextureSelect(NULL);
	    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	}
	glBegin(GL_QUADS);
	{
	    /* North wall */
	    glNormal3f(0.0f, 0.0f, -1.0f);
	    glTexCoord2f(1.0f, 1.0f - 0.0f);
	    glVertex3f(x_min, 0.0f, -y_max);
	    glTexCoord2f(1.0f, 1.0f - 1.0f);
	    glVertex3f(x_min, z_max, -y_max);
	    glTexCoord2f(0.0f, 1.0f - 1.0f);
	    glVertex3f(x_max, z_max, -y_max);
	    glTexCoord2f(0.0f, 1.0f - 0.0f);
	    glVertex3f(x_max, 0.0f, -y_max);

	    /* West wall */
	    glNormal3f(-1.0f, 0.0f, 0.0f);
	    glTexCoord2f(0.0f, 1.0f - 0.0f);
	    glVertex3f(x_min, 0.0f, -y_min);
	    glTexCoord2f(0.0f, 1.0f - 1.0f);
	    glVertex3f(x_min, z_max, -y_min);
	    glTexCoord2f(1.0f, 1.0f - 1.0f);
	    glVertex3f(x_min, z_max, -y_max);
	    glTexCoord2f(1.0f, 1.0f - 0.0f);
	    glVertex3f(x_min, 0.0f, -y_max);

	    /* South wall */
	    glNormal3f(0.0f, 0.0f, 1.0f);
	    glTexCoord2f(0.0f, 1.0f - 0.0f);
	    glVertex3f(x_max, 0.0f, -y_min);
	    glTexCoord2f(0.0f, 1.0f - 1.0f);
	    glVertex3f(x_max, z_max, -y_min);
	    glTexCoord2f(1.0f, 1.0f - 1.0f);
	    glVertex3f(x_min, z_max, -y_min);
	    glTexCoord2f(1.0f, 1.0f - 0.0f);
	    glVertex3f(x_min, 0.0f, -y_min);

	    /* East wall */
	    glNormal3f(1.0f, 0.0f, 0.0f);
	    glTexCoord2f(0.0f, 1.0f - 0.0f);
	    glVertex3f(x_max, 0.0f, -y_max);
	    glTexCoord2f(0.0f, 1.0f - 1.0f);
	    glVertex3f(x_max, z_max, -y_max);
	    glTexCoord2f(1.0f, 1.0f - 1.0f);
	    glVertex3f(x_max, z_max, -y_min);
	    glTexCoord2f(1.0f, 1.0f - 0.0f);
	    glVertex3f(x_max, 0.0f, -y_min);
	}
	glEnd();

	/* Night Walls Lighting */
	if((scene->tod_code != SAR_TOD_CODE_DAY) && !dc->flir)
	{
	    StateGLDisable(state, GL_LIGHTING);

	    i = 1;
	    if(i < SAR_OBJ_PREMODEL_MAX_TEXTURES)
		tex_num = obj_premodeled_ptr->tex_num[i];
	    else
		tex_num = -1;
	    if(SARIsTextureAllocated(scene, tex_num))
		V3DTextureSelect(scene->texture_ref[tex_num]);
	    else
		V3DTextureSelect(NULL);
	    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

	    glBegin(GL_QUADS);
	    {
		/* North wall */
		glNormal3f(0.0f, 0.0f, -1.0f);
		glTexCoord2f(1.0f, 1.0f - 0.0f);
		glVertex3f(x_min, 0.0f, -y_max);
		glTexCoord2f(1.0f, 1.0f - 1.0f);
		glVertex3f(x_min, z_max, -y_max);
		glTexCoord2f(0.0f, 1.0f - 1.0f);
		glVertex3f(x_max, z_max, -y_max);
		glTexCoord2f(0.0f, 1.0f - 0.0f);
		glVertex3f(x_max, 0.0f, -y_max);

		/* West wall */
		glNormal3f(-1.0f, 0.0f, 0.0f);
		glTexCoord2f(0.0f, 1.0f - 0.0f);
		glVertex3f(x_min, 0.0f, -y_min);
		glTexCoord2f(0.0f, 1.0f - 1.0f);
		glVertex3f(x_min, z_max, -y_min);
		glTexCoord2f(1.0f, 1.0f - 1.0f);
		glVertex3f(x_min, z_max, -y_max);
		glTexCoord2f(1.0f, 1.0f - 0.0f);
		glVertex3f(x_min, 0.0f, -y_max);

		/* South wall */
		glNormal3f(0.0f, 0.0f, 1.0f);
		glTexCoord2f(0.0f, 1.0f - 0.0f);
		glVertex3f(x_max, 0.0f, -y_min);
		glTexCoord2f(0.0f, 1.0f - 1.0f);
		glVertex3f(x_max, z_max, -y_min);
		glTexCoord2f(1.0f, 1.0f - 1.0f);
		glVertex3f(x_min, z_max, -y_min);
		glTexCoord2f(1.0f, 1.0f - 0.0f);
		glVertex3f(x_min, 0.0f, -y_min);

		/* East wall */
		glNormal3f(1.0f, 0.0f, 0.0f);
		glTexCoord2f(0.0f, 1.0f - 0.0f);
		glVertex3f(x_max, 0.0f, -y_max);
		glTexCoord2f(0.0f, 1.0f - 1.0f);
		glVertex3f(x_max, z_max, -y_max);
		glTexCoord2f(1.0f, 1.0f - 1.0f);
		glVertex3f(x_max, z_max, -y_min);
		glTexCoord2f(1.0f, 1.0f - 0.0f);
		glVertex3f(x_max, 0.0f, -y_min);
	    }
	    glEnd();

	    if(lighting)
		StateGLEnable(state, GL_LIGHTING);
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
	    i = 2;
	    if(i < SAR_OBJ_PREMODEL_MAX_TEXTURES)
		tex_num = obj_premodeled_ptr->tex_num[i];
	    else
		tex_num = -1;
	    if(SARIsTextureAllocated(scene, tex_num))
		V3DTextureSelect(scene->texture_ref[tex_num]);
	    else
		V3DTextureSelect(NULL);
	    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	}
	glBegin(GL_QUADS);
	{
	    glNormal3f(0.0f, 1.0f, 0.0f);
	    glTexCoord2f(0.0f, 1.0f - 0.0f);
	    glVertex3f(x_max, z_max, -y_min);
	    glTexCoord2f(0.0f, 1.0f - 1.0f);
	    glVertex3f(x_max, z_max, -y_max);
	    glTexCoord2f(1.0f, 1.0f - 1.0f);
	    glVertex3f(x_min, z_max, -y_max);
	    glTexCoord2f(1.0f, 1.0f - 0.0f);
	    glVertex3f(x_min, z_max, -y_min);
	}
	glEnd();

	/* Restore gl states */
	StateGLShadeModel(state, shade_model_mode);
	if(lighting)
	    StateGLEnable(state, GL_LIGHTING);
}
