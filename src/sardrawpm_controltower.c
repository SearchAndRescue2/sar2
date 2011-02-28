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
#include "sardrawpm.h"
#include "sardrawdefs.h"


void SARDrawPremodeledControlTower(SAR_DRAW_PREMODELED_PROTOTYPE);


/*
 *      Draws a premodeled control tower.
 */
void SARDrawPremodeledControlTower(SAR_DRAW_PREMODELED_PROTOTYPE)
{
	gw_display_struct *display = dc->display;
	state_gl_struct *state = &display->state_gl;
	sar_scene_struct *scene = dc->scene;
	StateGLBoolean lighting = state->lighting;
	GLenum shade_model_mode = state->shade_model_mode;
	float x_min, x_max, y_min, y_max, z_max, z_roof;
	int i, tex_num;


	/* Calculate bounds */
	x_max = obj_premodeled_ptr->length / 2.0f;
	x_min = -x_max;

	y_max = obj_premodeled_ptr->width / 2.0f;
	y_min = -y_max;

	z_max = (float)MAX(
	    obj_premodeled_ptr->height - 4.0,
	    1.0
	);
	z_roof = obj_premodeled_ptr->height;


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
	    glTexCoord2f(1, 1 - 0);
	    glVertex3f(x_min, 0, -y_max);
	    glTexCoord2f(1, 1 - 1);
	    glVertex3f(x_min, z_max, -y_max);
	    glTexCoord2f(0, 1 - 1);
	    glVertex3f(x_max, z_max, -y_max);
	    glTexCoord2f(0, 1 - 0);
	    glVertex3f(x_max, 0, -y_max);

	    /* West wall */
	    glNormal3f(-1.0f, 0.0f, 0.0f);
	    glTexCoord2f(0, 1 - 0);
	    glVertex3f(x_min, 0, -y_min);
	    glTexCoord2f(0, 1 - 1);
	    glVertex3f(x_min, z_max, -y_min);
	    glTexCoord2f(1, 1 - 1);
	    glVertex3f(x_min, z_max, -y_max);
	    glTexCoord2f(1, 1 - 0);
	    glVertex3f(x_min, 0, -y_max);

	    /* South wall */
	    glNormal3f(0.0f, 0.0f, 1.0f);
	    glTexCoord2f(0, 1 - 0);
	    glVertex3f(x_max, 0, -y_min);
	    glTexCoord2f(0, 1 - 1);
	    glVertex3f(x_max, z_max, -y_min);
	    glTexCoord2f(1, 1 - 1);
	    glVertex3f(x_min, z_max, -y_min);
	    glTexCoord2f(1, 1 - 0);
	    glVertex3f(x_min, 0, -y_min);

	    /* East wall */
	    glNormal3f(1.0f, 0.0f, 0.0f);
	    glTexCoord2f(0, 1 - 0);
	    glVertex3f(x_max, 0, -y_max);
	    glTexCoord2f(0, 1 - 1);
	    glVertex3f(x_max, z_max, -y_max);
	    glTexCoord2f(1, 1 - 1);
	    glVertex3f(x_max, z_max, -y_min);
	    glTexCoord2f(1, 1 - 0);
	    glVertex3f(x_max, 0, -y_min);
	}
	glEnd();


	/* Control tower windows */
	if(dc->flir)
	{

	}
	else if(scene->tod_code != SAR_TOD_CODE_DAY)
	{
	    StateGLDisable(state, GL_LIGHTING);
	    V3DTextureSelect(NULL);
	    glColor4f(0.25f, 0.20f, 0.05f, 1.0f);
	}
	else
	{
	    V3DTextureSelect(NULL);
	    glColor4f(0.05f, 0.05f, 0.0f, 1.0f);
	}
	glBegin(GL_QUADS);
	{
	    /* North */
	    glNormal3f(0.0f, -0.2f, -0.8f);
	    glVertex3f(x_max, z_max, -y_max);
	    glVertex3f(x_min, z_max, -y_max);
	    glVertex3f(x_min, z_roof, (-y_max * 1.1f));
	    glVertex3f(x_max, z_roof, (-y_max * 1.1f));

	    glNormal3f(0.6f, -0.2f, -0.6f);
	    glVertex3f(x_max, z_max, -y_max);
	    glVertex3f(x_max, z_max, -y_max);
	    glVertex3f(x_max, z_roof, (-y_max * 1.1f));
	    glVertex3f((x_max * 1.1f), z_roof, -y_max);

	    /* East */
	    glNormal3f(0.8f, -0.2f, 0.0f);
	    glVertex3f(x_max * 1.1f, z_roof, -y_max);
	    glVertex3f(x_max * 1.1f, z_roof, -y_min);
	    glVertex3f(x_max, z_max, -y_min);
	    glVertex3f(x_max, z_max, -y_max);

	    glNormal3f(0.6f, -0.2f, 0.6f);
	    glVertex3f(x_max, z_max, -y_min);
	    glVertex3f(x_max, z_max, -y_min);
	    glVertex3f(x_max * 1.1f, z_roof, -y_min);
	    glVertex3f(x_max, z_roof, -y_min * 1.1f);

	    /* South */
	    glNormal3f(0.0f, -0.2f, 0.8f);
	    glVertex3f(x_min, z_max, -y_min);
	    glVertex3f(x_max, z_max, -y_min);
	    glVertex3f(x_max, z_roof, -y_min * 1.1f);
	    glVertex3f(x_min, z_roof, -y_min * 1.1f);

	    glNormal3f(-0.6f, -0.2f, 0.6f);
	    glVertex3f(x_min, z_max, -y_min);
	    glVertex3f(x_min, z_max, -y_min);
	    glVertex3f(x_min, z_roof, -y_min * 1.1f);
	    glVertex3f(x_min * 1.1f, z_roof, -y_min);

	    /* West */
	    glNormal3f(-0.8f, -0.2f, 0.0f);
	    glVertex3f(x_min, z_max, -y_max);
	    glVertex3f(x_min, z_max, -y_min);
	    glVertex3f(x_min * 1.1f, z_roof, -y_min);
	    glVertex3f(x_min * 1.1f, z_roof, -y_max);

	    glNormal3f(-0.6f, -0.2f, -0.6f);
	    glVertex3f(x_min, z_max, -y_max);
	    glVertex3f(x_min, z_max, -y_max);
	    glVertex3f(x_min * 1.1f, z_roof, -y_max);
	    glVertex3f(x_min, z_roof, -y_max * 1.1f);
	}
	glEnd();


	/* Roof */
	if(!dc->flir)
	{
	    if(lighting)
		StateGLEnable(state, GL_LIGHTING);

	    /* Select roof texture */
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
	}
	glBegin(GL_QUADS);
	{
	    /* Roof */
	    glNormal3f(0.0f, 1.0f, 0.0f);

	    glTexCoord2f(1, 1 - 0);
	    glVertex3f(x_max * 1.1f, z_roof, -y_max);
	    glTexCoord2f(1, 1 - 1);
	    glVertex3f(x_max, z_roof, -y_max * 1.1f);
	    glTexCoord2f(0, 1 - 1);
	    glVertex3f(x_min, z_roof, -y_max * 1.1f);
	    glTexCoord2f(0, 1 - 0);
	    glVertex3f(x_min * 1.1f, z_roof, -y_max);


	    glTexCoord2f(0, 1 - 0);
	    glVertex3f(x_min * 1.1f, z_roof, -y_min);
	    glTexCoord2f(0, 1 - 1);
	    glVertex3f(x_min, z_roof, -y_min * 1.1f);
	    glTexCoord2f(1, 1 - 1);
	    glVertex3f(x_max, z_roof, -y_min * 1.1f);
	    glTexCoord2f(1, 1 - 0);
	    glVertex3f(x_max * 1.1f, z_roof, -y_min);

	    glTexCoord2f(1, 1 - 0);
	    glVertex3f(x_min * 1.1f, z_roof, -y_min);
	    glTexCoord2f(1, 1 - 1);
	    glVertex3f(x_max * 1.1f, z_roof, -y_min);
	    glTexCoord2f(0, 1 - 1);
	    glVertex3f(x_max * 1.1f, z_roof, -y_max);
	    glTexCoord2f(0, 1 - 0);
	    glVertex3f(x_min * 1.1f, z_roof, -y_max);
	}
	glEnd();

	/* Restore gl states */
	StateGLShadeModel(state, shade_model_mode);
	if(lighting)
	    StateGLEnable(state, GL_LIGHTING);
}
