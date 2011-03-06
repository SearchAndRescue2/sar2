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
#ifdef __MSW__
# include <windows.h>
#endif
#include "../include/string.h"
#include <GL/gl.h>
#include <GL/glu.h>
#include "gw.h"
#include "v3dtex.h"
#include "cpvalues.h"
#include "cpins.h"
#include "cp.h"


void CPInsSetName(CPIns *ins, const char *name);
void CPInsSetPosition(CPIns *ins, float x, float y);
void CPInsSetSize(CPIns *ins, float width, float height);
void CPInsSetTextureBG(CPIns *ins, const char *path);
void CPInsSetTextureFG(CPIns *ins, const char *path);
void CPInsSetResolution(
        CPIns *ins,
        int res_width, int res_height   /* In pixels. */
);
void CPInsSetUpdateInt(CPIns *ins, time_t update_int);
void CPInsSetFunction(
        CPIns *ins, const char *func_name,
        void *func, void *data
);

void CPInsResetTimmers(CPIns *ins, time_t t);
void CPInsChangeValues(CPIns *ins, ControlPanelValues *v);
void CPInsDraw(CPIns *ins);
void CPInsDrawFullScreen(CPIns *ins, const CPRectangle *rect);
void CPInsManage(CPIns *ins, ControlPanelValues *v);
void CPInsRealizeTexture(CPIns *ins);

CPIns *CPInsNew(size_t s, void *cp, int type, const char *name);
void CPInsDelete(CPIns *ins);


#ifndef PI
# define PI     3.14159265
#endif

#define ATOI(s)         (((s) != NULL) ? atoi(s) : 0)
#define ATOL(s)         (((s) != NULL) ? atol(s) : 0)
#define ATOF(s)         (((s) != NULL) ? (float)atof(s) : 0.0f)
#define STRDUP(s)       (((s) != NULL) ? strdup(s) : NULL)

#define MAX(a,b)        (((a) > (b)) ? (a) : (b))
#define MIN(a,b)        (((a) < (b)) ? (a) : (b))
#define CLIP(a,l,h)     (MIN(MAX((a),(l)),(h)))

#define RADTODEG(r)     ((r) * 180.0 / PI)
#define DEGTORAD(d)     ((d) * PI / 180.0)

#define NORMAL3F(x,y,z) (glNormal3f(    \
                         (GLfloat)(x),  \
                         (GLfloat)(z),  \
                         (GLfloat)-(y)  \
                        ))
#define TEXCOORD2F(x,y) (glTexCoord2f(          \
                         (GLfloat)(x),		\
                         (GLfloat)(y)		\
                        ))
#define VERTEX3F(x,y,z) (glVertex3f(                    \
                         (GLfloat)((x) / 100.0),        \
                         (GLfloat)((z) / 100.0),        \
                         (GLfloat)((y) / -100.0)        \
                        ))
#define ROTATEH(r)      (glRotatef((GLfloat)-RADTODEG(r), \
                         0.0f, 1.0f, 0.0f))
#define ROTATEP(r)      (glRotatef((GLfloat)-RADTODEG(r), \
                         1.0f, 0.0f, 0.0f))
#define ROTATEB(r)      (glRotatef((GLfloat)-RADTODEG(r), \
                         0.0f, 0.0f, 1.0f))
#define TRANSLATE(x,y,z)        (glTranslatef(          \
                         (GLfloat)((x) / 100.0),        \
                         (GLfloat)((z) / 100.0),        \
                         (GLfloat)((y) / -100.0)        \
                        ))


/*
 *	Sets name of instrument.
 */
void CPInsSetName(CPIns *ins, const char *name)
{
        if(ins == NULL)
            return;

	free(ins->name);
	ins->name = STRDUP(name);
}

/*
 *	Set position coefficient of instrument relative to the control
 *	panel's upper left corner.
 */
void CPInsSetPosition(CPIns *ins, float x, float y)
{
	if(ins == NULL)
	    return;

	ins->x = x;
	ins->y = y;
}

/*
 *	Sets the size coefficient of the instrument.
 */
void CPInsSetSize(CPIns *ins, float width, float height)
{
        if(ins == NULL)
            return;

        ins->width = width;
        ins->height = height;
}

/*
 *      Set instrument background texture.
 */
void CPInsSetTextureBG(CPIns *ins, const char *path)
{
        if(ins == NULL)
            return;

        V3DTextureDestroy(ins->tex_bg);
        ins->tex_bg = NULL;

        if((path != NULL) ? (*path == '\0') : GL_TRUE)
            return;

        ins->tex_bg = V3DTextureLoadFromFile2DPreempt(
            path, "instrument_bg_tex",
            V3D_TEX_FORMAT_RGBA
        );
}

/*
 *      Set instrument foreground texture.
 */
void CPInsSetTextureFG(CPIns *ins, const char *path)
{
        if(ins == NULL)
            return;

        V3DTextureDestroy(ins->tex_fg);
        ins->tex_fg = NULL;

        if((path != NULL) ? (*path == '\0') : GL_TRUE)
            return;

        ins->tex_fg = V3DTextureLoadFromFile2DPreempt(
            path, "instrument_fg_tex",
            V3D_TEX_FORMAT_RGBA
        );
}


/*
 *	Sets the texture resolution of the instrument in pixels.
 */
void CPInsSetResolution(
        CPIns *ins,
        int res_width, int res_height   /* In pixels. */
)
{
	if(ins == NULL)
            return;

	/* Update resolution only if there is change. */
	if((ins->res_width != res_width) ||
	   (ins->res_height != res_height)
	)
	{
	    ins->res_width = res_width;
	    ins->res_height = res_height;

	    /* Destroy texture. */
	    V3DTextureDestroy(ins->tex);
	    ins->tex = NULL;

	    /* Recreate texture if resolution is positive. */
	    if((res_width > 0) && (res_height > 0))
	    {
		u_int8_t *data = (u_int8_t *)malloc(
		    res_width * res_height * 4 * sizeof(u_int8_t)
		);
		ins->tex = V3DTextureLoadFromData2D(
		    (const void *)data,
		    NULL,
		    res_width, res_height,
		    32,
		    V3D_TEX_FORMAT_RGBA,
		    NULL, NULL
		);
		free(data);
		V3DTexturePriority(ins->tex, 1.0);
	    }
	}
}

/*
 *	Sets the instrument update interval in milliseconds.
 */
void CPInsSetUpdateInt(CPIns *ins, time_t update_int)
{
        if(ins == NULL)
            return;

        ins->update_int = update_int;
}

/*
 *	Sets the function pointer func to the function specified by
 *	func_name.
 */
void CPInsSetFunction(
        CPIns *ins, const char *func_name,
        void *func, void *data
)
{
	if((ins == NULL) || (func_name == NULL))
	    return;

        if(!strcasecmp(func_name, "values_changed"))
        {
            ins->values_changed_func_data = data;
            ins->values_changed_func = func;
        }
        else if(!strcasecmp(func_name, "manage"))
        {
            ins->manage_func_data = data;
            ins->manage_func = func;
        }
	else if(!strcasecmp(func_name, "reset_timmers"))
	{
	    ins->reset_timmers_func_data = data;
	    ins->reset_timmers_func = func;
	}
	else if(!strcasecmp(func_name, "delete"))
        {
            ins->delete_func_data = data;
            ins->delete_func = func;
        }
	else
	{
	    fprintf(
		stderr,
"CPInsSetFunction(): Warning:\
 Instrument \"%s\": Unsupported function \"%s\".",
		CPINS_NAME(ins), func_name
	    );
	}
}

/*
 *	Reset timmer of instrument.
 */
void CPInsResetTimmers(CPIns *ins, time_t t)
{
        if(ins == NULL)
            return;

	ins->update_next = t;
        if(ins->reset_timmers_func != NULL)
            ins->reset_timmers_func(ins, t, ins->reset_timmers_func_data);
}

/*
 *	Change values of instrument.
 */
void CPInsChangeValues(CPIns *ins, ControlPanelValues *v)
{
        if((ins == NULL) || (v == NULL))
            return;

        if(ins->values_changed_func != NULL)
	{
	    ControlPanel *cp = CONTROL_PANEL(ins->cp);
	    gw_display_struct *display = CONTROL_PANEL_DISPLAY(cp);
	    int	width = ins->res_width,
		height = ins->res_height;
	    if((display == NULL) || (width <= 1) || (height < 1))
		return;

	    /* Set up GL states for 2d drawing before calling the
	     * "values_changed" function. This is for convience so that
	     * the GL state is all ready for drawing of the texture.
	     */
            glViewport(0, 0, width, height);

            glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
            glPixelStorei(GL_PACK_ALIGNMENT, 1);

            StateGLDisable(&display->state_gl, GL_LIGHTING);
            StateGLDisable(&display->state_gl, GL_BLEND);
            StateGLDisable(&display->state_gl, GL_FOG);
            StateGLDisable(&display->state_gl, GL_DEPTH_TEST);
            StateGLDepthFunc(&display->state_gl, GL_ALWAYS);
            StateGLEnable(&display->state_gl, GL_ALPHA_TEST);
            StateGLAlphaFunc(&display->state_gl, GL_GREATER, 0.5);
            StateGLShadeModel(&display->state_gl, GL_FLAT);
            StateGLEnable(&display->state_gl, GL_TEXTURE_2D);
            StateGLTexEnvI(
                &display->state_gl,
                GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE
            );
            StateGLDisable(&display->state_gl, GL_CULL_FACE);
            StateGLDisable(&display->state_gl, GL_COLOR_MATERIAL);

	    GWOrtho2DCoord(
		display,
		/* Left and right coordinate values. */
                0.0f, (float)MAX(width - 1, 1),
		/* Top and bottom coordinate values. */
                (float)MAX(height - 1, 1), 0.0f
            );

	    ins->values_changed_func(
		ins,
		v,
		ins->values_changed_func_data
	    );
	}
}

/*
 *	Redraws the instrument in 3d.
 */
void CPInsDraw(CPIns *ins)
{
	GLfloat x, y, width, height;
	ControlPanel *cp;
        gw_display_struct *display;
        v3d_texture_ref_struct *tex;
        if(ins == NULL)
            return;

	cp = CONTROL_PANEL(ins->cp);
	display = CONTROL_PANEL_DISPLAY(cp);
	if(display == NULL)
	    return;

	if(GL_TRUE)
	{
	    /* Calculate position and size (in centimeters) relative to
	     * the control panel's upper left corner.
	     */
	    x = ins->x * cp->width;
	    y = ins->y * cp->height;
	    width = ins->width * cp->width;
	    height = ins->height * cp->height;

	    /* Begin drawing instrument. */
	    glPushMatrix();

	    TRANSLATE(x, -y, 0.0);

	    tex = ins->tex;
	    if(tex != NULL)
	    {
		glColor3f(1.0f, 1.0f, 1.0f);
		V3DTextureSelect(tex);
	    }
	    else
	    {
		glColor3f(0.0f, 0.0f, 0.0f);
	    }
	    glBegin(GL_QUADS);
	    {
		NORMAL3F(0.0, 0.0, 1.0);
		TEXCOORD2F(0, 0);
		VERTEX3F(0.0, 0.0, 0.0);
		TEXCOORD2F(0, 1);
		VERTEX3F(0.0, -height, 0.0);
		TEXCOORD2F(1, 1);
		VERTEX3F(width, -height, 0.0);
		TEXCOORD2F(1, 0);
		VERTEX3F(width, 0.0, 0.0);
	    }
	    glEnd();

	    glPopMatrix();
	}
}

/*
 *      Redraws the instrument in 2d with respect to the given
 *	rectangle indicating the size and position of the control panel
 *	in window coordinates..
 */
void CPInsDrawFullScreen(CPIns *ins, const CPRectangle *rect)
{
	GLfloat x, y, width, height;
        ControlPanel *cp;
        gw_display_struct *display;
        v3d_texture_ref_struct *tex;
        if((ins == NULL) || (rect == NULL))
            return;

        cp = CONTROL_PANEL(ins->cp);
        display = CONTROL_PANEL_DISPLAY(cp);
        if(display == NULL)
            return;

        if(GL_TRUE)
        {
            /* Calculate position and size (in window coordinates)
	     * relative to the control panel's upper left corner.
             */
            x = rect->x + (ins->x * rect->width);
            y = rect->y + (ins->y * rect->height);
            width = ins->width * rect->width;
            height = ins->height * rect->height;

            tex = ins->tex;
            if(tex != NULL)
            {
                glColor3f(1.0f, 1.0f, 1.0f);
                V3DTextureSelect(tex);
            }
            else
            {
                glColor3f(0.0f, 0.0f, 0.0f);
            }
            glBegin(GL_QUADS);
            {
		glTexCoord2f(0, 0);
		glVertex2f(x, y);
                glTexCoord2f(0, 1);
                glVertex2f(x, y + height);
                glTexCoord2f(1, 1);
                glVertex2f(x + width, y + height);
                glTexCoord2f(1, 0);
                glVertex2f(x + width, y);
            }
            glEnd();
	}
}

/*
 *	Manages the instrument, this should be called once per loop.
 *
 *	Only the members current_time and time_compensation in v are
 *	used.
 */
void CPInsManage(CPIns *ins, ControlPanelValues *v)
{
	if((ins == NULL) || (v == NULL))
	    return;

        if(ins->manage_func != NULL)
            ins->manage_func(ins, v, ins->manage_func_data);
}

/*
 *	Gets the texture from the drawn up back buffer of the GL frame 
 *	buffer.
 */
void CPInsRealizeTexture(CPIns *ins)
{
	int x, y, xoffset, yoffset;
	int res_width, res_height;
        ControlPanel *cp;
        gw_display_struct *display;
        v3d_texture_ref_struct *tex;
        if(ins == NULL)
            return;

        cp = CONTROL_PANEL(ins->cp);
        display = CONTROL_PANEL_DISPLAY(cp);
        if(display == NULL)
            return;

	res_width = ins->res_width;
	res_height = ins->res_height;

	/* Get instrument's texture, recreate it as needed. */
	tex = ins->tex;
	if(tex == NULL)
	{
	    CPInsSetResolution(ins, 0, 0);
	    CPInsSetResolution(ins, res_width, res_height);
	    tex = ins->tex;
	}
	if(tex == NULL)
	    return;

	/* Texture size must match resolution size. */
	if((res_width != tex->width) ||
	   (res_height != tex->height)
	)
	    return;

	/* Select instrument texture. */
        StateGLEnable(&display->state_gl, GL_TEXTURE_2D);
	V3DTextureSelect(tex);

	/* Read back drawn frame buffer's alpha channel. */
	if(display->has_double_buffer)
	    glReadBuffer(GL_BACK);
	else
	    glReadBuffer(GL_FRONT);

	/* Texture position. */
	xoffset = 0;
	yoffset = 0;

	/* Frame buffer position. */
	x = 0;
	y = 0;

	/* Copy frame buffer contents to texture. */
	glCopyTexSubImage2D(
	    GL_TEXTURE_2D,
	    0,		/* level-of-detail. */
	    xoffset, yoffset,
	    x, y,
	    res_width, res_height
	);
}


/*
 *	Creates a new instrument.
 *
 *	The size s must be sizeof(CPIns) or greater.
 */
CPIns *CPInsNew(size_t s, void *cp, int type, const char *name)
{
	CPColor *c;
	CPIns *ins;

	if(s < sizeof(CPIns))
	{
	    fprintf(
		stderr,
"CPInsNew(): Error: Instrument \"%s\" size %i is less than required size %i.\n",
		name, s, sizeof(CPIns)
	    );
	    return(NULL);
	}

	ins = CPINS(calloc(1, s));
	if(ins == NULL)
	    return(NULL);

	ins->type = type;
	ins->cp = cp;
	ins->name = STRDUP(name);

	ins->res_width = 64;
	ins->res_height = 64;
	ins->tex = NULL;

	ins->tex_bg = NULL;
	ins->tex_fg = NULL;

	c = &ins->color_bg[CP_COLOR_STATE_NORMAL];
	c->r = 0.3f;
	c->g = 0.3f;
	c->b = 0.3f;
        c = &ins->color_bg[CP_COLOR_STATE_DIM];
        c->r = 0.15f;
        c->g = 0.15f;
        c->b = 0.15f;
        c = &ins->color_bg[CP_COLOR_STATE_DARK];
        c->r = 0.0f;
        c->g = 0.0f;
        c->b = 0.0f;
        c = &ins->color_bg[CP_COLOR_STATE_LIGHTED];
        c->r = 0.3f;
        c->g = 0.05f;
        c->b = 0.0f;

        c = &ins->color_fg[CP_COLOR_STATE_NORMAL];
        c->r = 0.8f;
        c->g = 0.8f;
        c->b = 0.8f;
        c = &ins->color_fg[CP_COLOR_STATE_DIM];
        c->r = 0.4f;
        c->g = 0.4f;
        c->b = 0.4f;
        c = &ins->color_fg[CP_COLOR_STATE_DARK];
        c->r = 0.1f;
        c->g = 0.1f;
        c->b = 0.1f;
        c = &ins->color_fg[CP_COLOR_STATE_LIGHTED];
        c->r = 0.8f;
        c->g = 0.3f;
        c->b = 0.0f;

        c = &ins->color_text[CP_COLOR_STATE_NORMAL];
        c->r = 0.8f;
        c->g = 0.8f;
        c->b = 0.8f;
        c = &ins->color_text[CP_COLOR_STATE_DIM];
        c->r = 0.4f;
        c->g = 0.4f;
        c->b = 0.4f;
        c = &ins->color_text[CP_COLOR_STATE_DARK];
        c->r = 0.1f;
        c->g = 0.1f;
        c->b = 0.1f;
        c = &ins->color_text[CP_COLOR_STATE_LIGHTED];
        c->r = 0.8f;
        c->g = 0.3f;
        c->b = 0.0f;


	return(ins);
}

/*
 *	Deletes the instrument.
 */
void CPInsDelete(CPIns *ins)
{
        if(ins == NULL)
            return;

	/* Call delete function to delete the instrument's
	 * substructures and other resources not accessable from
	 * this module.
	 */
	if(ins->delete_func != NULL)
	    ins->delete_func(ins, ins->delete_func_data);
	else if(!CPINS_IS_INS(ins))
	    fprintf(
		stderr,
"CPInsDelete(): Warning: Instrument \"%s\": \"delete\" function not set.\n",
		CPINS_NAME(ins)
	    );

	/* Begin deleting instrument. */
        V3DTextureDestroy(ins->tex_bg);
        V3DTextureDestroy(ins->tex_fg);
	V3DTextureDestroy(ins->tex);
	free(ins->name);
	free(ins);
}
