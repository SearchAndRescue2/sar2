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

#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#ifdef __MSW__
# include <windows.h>
#endif
#include <GL/gl.h>
#include "matrixmath.h"
#include "gw.h"
#include "v3dtex.h"
#include "cpvalues.h"
#include "cpins.h"
#include "cpinsaltimeter.h"
#include "cp.h"


CPIns *CPInsAltimeterNew(void *cp);
static void CPInsAltimeterValuesChanged(
	CPIns *ins, ControlPanelValues *v, void *data
);
static void CPInsAltimeterManage(
	CPIns *ins, ControlPanelValues *v, void *data
);
static void CPInsAltimeterDelete(CPIns *ins, void *data);


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
			 (GLfloat)(x),          \
			 (GLfloat)(1.0 - (y))   \
			))
#define VERTEX3FC(x,y,z) (glVertex3f(			\
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
 *      Creates a new bearing instrument.
 */
CPIns *CPInsAltimeterNew(void *cp)
{
	CPInsAltimeter *insv;
	CPIns *ins = CPInsNew(
	    sizeof(CPInsAltimeter),
	    cp,
	    CPINS_TYPE_ALTIMETER,
	    "Altimeter"
	);
	if(ins == NULL)
	    return(NULL);

	insv = CPINS_ALTIMETER(ins);

	CPInsSetFunction(
	    ins, "values_changed",
	    CPInsAltimeterValuesChanged, NULL
	);
	CPInsSetFunction(
	    ins, "manage",
	    CPInsAltimeterManage, NULL
	);
	CPInsSetFunction(
	    ins, "delete",
	    CPInsAltimeterDelete, NULL
	);

	return(ins);
}

/*
 *	"values_changed" callback.
 */
static void CPInsAltimeterValuesChanged(
	CPIns *ins, ControlPanelValues *v, void *data
)
{
	ControlPanel *cp;
	gw_display_struct *display;
	int cstate;
	GLfloat width, height;
	v3d_texture_ref_struct *tex_bg, *tex_fg;
	CPInsAltimeter *insv = CPINS_ALTIMETER(ins);
	if((insv == NULL) || (v == NULL))
	    return;

	cp = CONTROL_PANEL(ins->cp);
	display = CONTROL_PANEL_DISPLAY(cp);
	if(display == NULL)
	    return;

	cstate = v->color_state;

	/* At this point the GL perspective matrix should already be
	 * orthoginal and it's size set to match the size of the
	 * frame buffer.  Now get the size of the frame buffer.
	 */
	width = (GLfloat)ins->res_width;
	height = (GLfloat)ins->res_height;

	/* Get instrument background and foreground textures. */
	tex_bg = ins->tex_bg;
	tex_fg = ins->tex_fg;


	/* Begin drawing. */

	/* Draw background. */
	if(tex_bg != NULL)
	{
	    glColor3f(1.0f, 1.0f, 1.0f);
	    StateGLEnable(&display->state_gl, GL_TEXTURE_2D);
	    V3DTextureSelect(tex_bg);

	    glBegin(GL_QUADS);
	    {
		glTexCoord2f(0.0f, 0.0f);
		glVertex2f(0.0f, 0.0f);
		glTexCoord2f(0.0f, 1.0f);
		glVertex2f(0.0f, height);
		glTexCoord2f(1.0f, 1.0f);
		glVertex2f(width, height);
		glTexCoord2f(1.0f, 0.0f);
		glVertex2f(width, 0.0f);
	    }
	    glEnd();
	}

	/* Draw altitude hands. */



	/* Draw foretround. */
	if(tex_fg != NULL)
	{
	    glColor3f(1.0f, 1.0f, 1.0f);
	    StateGLEnable(&display->state_gl, GL_TEXTURE_2D);
	    V3DTextureSelect(tex_fg);

	    glBegin(GL_QUADS);
	    {
		glTexCoord2f(0.0f, 0.0f);
		glVertex2f(0.0f, 0.0f);
		glTexCoord2f(0.0f, 1.0f);
		glVertex2f(0.0f, height);
		glTexCoord2f(1.0f, 1.0f);
		glVertex2f(width, height);
		glTexCoord2f(1.0f, 0.0f);
		glVertex2f(width, 0.0f);
	    }
	    glEnd();
	}

	CPInsRealizeTexture(ins);
}

/*
 *      "manage" callback.
 */
static void CPInsAltimeterManage(
	CPIns *ins, ControlPanelValues *v, void *data
)
{
	CPInsAltimeter *insv = CPINS_ALTIMETER(ins);
	if(insv == NULL)
	    return;
}

/*
 *      Deletes resources of instrument but not instrument itself.
 */
static void CPInsAltimeterDelete(CPIns *ins, void *data)
{
	CPInsAltimeter *insv = CPINS_ALTIMETER(ins);
	if(insv == NULL)
	    return;
}
