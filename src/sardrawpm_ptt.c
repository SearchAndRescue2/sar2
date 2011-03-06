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


void SARDrawPremodeledPowerTransmissionTower(SAR_DRAW_PREMODELED_PROTOTYPE);


/*
 *      Draws a premodeled power transmission tower.
 */
void SARDrawPremodeledPowerTransmissionTower(SAR_DRAW_PREMODELED_PROTOTYPE)
{
	gw_display_struct *display = dc->display;
	state_gl_struct *state = &display->state_gl;
	GLenum shade_model_mode = state->shade_model_mode;
	int i;
	float sx, sy, sz;		/* Scale */


	/* All hard coded coordinates will produce an object of unit
	 * size (height = 1.0)
	 *
	 * So the sx, sy, and sz scale coefficients will simply multiply
	 * each point
	 */

	/* Calculate scales, all based on height */
	sx = sy = sz = obj_premodeled_ptr->height;

	/* Set up gl states */
	StateGLShadeModel(state, GL_FLAT);

	glColor4f(0.45f, 0.45f, 0.45f, 1.0f);

	/* Draw close range model? */
	if(camera_dist < 2000.0f)
	{
	  /* Draw two passes (one for the north side, the other for
	   * the south side.
	   */
	  for(i = 0; i < 2; i++)
	  {
	    glBegin(GL_LINE_STRIP);
	    {
		/* Base */
		glNormal3f(0.0f, 1.0f, 0.0f);

		glVertex3f(-0.12f * sx, 0.0f * sz, -(0.12f * sy));
		glVertex3f(-0.08f * sx, 0.2f * sz, -(0.08f * sy));
		glVertex3f(0.08f * sx, 0.2f * sz, -(0.08f * sy));
		glVertex3f(0.12f * sx, 0.0f * sz, -(0.12f * sy));
		glVertex3f(-0.08f * sx, 0.2f * sz, -(0.08f * sy));
	    }
	    glEnd();
	    glBegin(GL_LINE_STRIP);
	    {
		glVertex3f(-0.12f * sx, 0.0f * sz, -(0.12f * sy));
		glVertex3f(0.08f * sx, 0.2f * sz, -(0.08f * sy));
		glVertex3f(-0.05f * sx, 0.4f * sz, -(0.05f * sy));
		glVertex3f(0.05f * sx, 0.4f * sz, -(0.05f * sy));
		glVertex3f(-0.08f * sx, 0.2f * sz, -(0.08f * sy));
	    }
	    glEnd();
	    glBegin(GL_LINE_STRIP);
	    {
		glVertex3f(-0.08f * sx, 0.2f * sz, -(0.08f * sy));
		glVertex3f(-0.05f * sx, 0.4f * sz, -(0.05f * sy));
		glVertex3f(0.05f * sx, 0.65f * sz, -(0.05f * sy));
		glVertex3f(0.05f * sx, 0.4f * sz, -(0.05f * sy));
	    }
	    glEnd();
	    glBegin(GL_LINE_STRIP);
	    {
		glVertex3f(-0.05f * sx, 0.4f * sz, -(0.05f * sy));
		glVertex3f(-0.05f * sx, 0.65f * sz, -(0.05f * sy));
		glVertex3f(0.05f * sx, 0.4f * sz, -(0.05f * sy));
		glVertex3f(0.08f * sx, 0.2f * sz, -(0.08f * sy));
	    }
	    glEnd();
	    glBegin(GL_LINE_STRIP);
	    {
		glVertex3f(0.05f * sx, 0.65f * sz, -(0.05f * sy));
		glVertex3f(-0.05f * sx, 0.65f * sz, -(0.05f * sy));
		glVertex3f(-0.05f * sx, 0.9f * sz, -(0.05f * sy));
		glVertex3f(0.05f * sx, 0.65f * sz, -(0.05f * sy));
		glVertex3f(0.05f * sx, 0.9f * sz, -(0.05f * sy));
	    }
	    glEnd();
	    glBegin(GL_LINE_STRIP);
	    {
		glVertex3f(-0.05f * sx, 0.65f * sz, -(0.05f * sy));
		glVertex3f(0.05f * sx, 0.9f * sz, -(0.05f * sy));
		glVertex3f(0.0f * sx, 1.0f * sz, -(0.0f * sy));
		glVertex3f(-0.05f * sx, 0.9f * sz, -(0.05f * sy));
		glVertex3f(0.05f * sx, 0.9f * sz, -(0.05f * sy));
	    }
	    glEnd();
	    glBegin(GL_LINE_STRIP);
	    {
		glVertex3f(-0.05f * sx, 0.65f * sz, -(0.05f * sy));
		glVertex3f(-0.25f * sx, 0.65f * sz, -(0.05f * sy));
		glVertex3f(-0.27f * sx, 0.56f * sz, -(0.0f * sy));
		glVertex3f(-0.22f * sx, 0.62f * sz, -(0.05f * sy));
		glVertex3f(-0.05f * sx, 0.59f * sz, -(0.05f * sy));
		glVertex3f(-0.25f * sx, 0.65f * sz, -(0.05f * sy));
		glVertex3f(-0.22f * sx, 0.62f * sz, -(0.05f * sy));
		glVertex3f(-0.05f * sx, 0.65f * sz, -(0.05f * sy));
	    }
	    glEnd();
	    glBegin(GL_LINE_STRIP);
	    {
		glVertex3f(0.05f * sx, 0.65f * sz, -(0.05f * sy));
		glVertex3f(0.25f * sx, 0.65f * sz, -(0.05f * sy));
		glVertex3f(0.27f * sx, 0.56f * sz, -(0.0f * sy));
		glVertex3f(0.22f * sx, 0.62f * sz, -(0.05f * sy));
		glVertex3f(0.05f * sx, 0.59f * sz, -(0.05f * sy));
		glVertex3f(0.25f * sx, 0.65f * sz, -(0.05f * sy));
		glVertex3f(0.22f * sx, 0.62f * sz, -(0.05f * sy));
		glVertex3f(0.05f * sx, 0.65f * sz, -(0.05f * sy));
	    }
	    glEnd();
	    glBegin(GL_LINE_STRIP);
	    {
		glVertex3f(-0.05f * sx, 0.9f * sz, -(0.05f * sy));
		glVertex3f(-0.23f * sx, 0.9f * sz, -(0.05f * sy));
		glVertex3f(-0.25f * sx, 0.81f * sz, -(0.0f * sy));
		glVertex3f(-0.2f * sx, 0.87f * sz, -(0.05f * sy));
		glVertex3f(-0.05f * sx, 0.84f * sz, -(0.05f * sy));
		glVertex3f(-0.23f * sx, 0.9f * sz, -(0.05f * sy));
		glVertex3f(-0.2f * sx, 0.87f * sz, -(0.05f * sy));
		glVertex3f(-0.05f * sx, 0.9f * sz, -(0.05f * sy));
	    }
	    glEnd();
	    glBegin(GL_LINE_STRIP);
	    {
		glVertex3f(0.05f * sx, 0.9f * sz, -(0.05f * sy));
		glVertex3f(0.23f * sx, 0.9f * sz, -(0.05f * sy));
		glVertex3f(0.25f * sx, 0.81f * sz, -(0.0f * sy));
		glVertex3f(0.2f * sx, 0.87f * sz, -(0.05f * sy));
		glVertex3f(0.05f * sx, 0.84f * sz, -(0.05f * sy));
		glVertex3f(0.23f * sx, 0.9f * sz, -(0.05f * sy));
		glVertex3f(0.2f * sx, 0.87f * sz, -(0.05f * sy));
		glVertex3f(0.05f * sx, 0.9f * sz, -(0.05f * sy));
	    }
	    glEnd();

	    /* Flip the y scale value to draw the south side */
	    sy *= -1.0f;
	  }
	}
	else
	{
	    /* Draw far model */
	    glBegin(GL_LINES);
	    {
		glNormal3f(0.0f, 1.0f, 0.0f);

		glVertex3f(0.0f * sx, 0.0f * sz, -(0.0f * sy));
		glVertex3f(0.0f * sx, 1.0f * sz, -(0.0f * sy));

		glVertex3f( 0.27f * sx, 0.62f * sz, -(0.0f * sy));
		glVertex3f(-0.27f * sx, 0.62f * sz, -(0.0f * sy));

		glVertex3f( 0.26f * sx, 0.9f * sz, -(0.0f * sy));
		glVertex3f(-0.26f * sx, 0.9f * sz, -(0.0f * sy));
	    }
	    glEnd();
	}

	/* Reget y scale, since it has been modified in the above loop */
	sy = obj_premodeled_ptr->height;


	/* Restore gl states */
	StateGLShadeModel(state, shade_model_mode);
}
