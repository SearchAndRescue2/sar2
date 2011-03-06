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


void SARDrawPremodeledTower(SAR_DRAW_PREMODELED_PROTOTYPE);


/*
 *      Draws a premodeled tower.
 */
void SARDrawPremodeledTower(SAR_DRAW_PREMODELED_PROTOTYPE)
{
	gw_display_struct *display = dc->display;
	GLenum shade_model_mode = display->state_gl.shade_model_mode;
	float sx, sy, sz;		/* Scale. */


	/* All hard coded coordinates will produce an object of 100 unit
	 * size (height = 100.0). So the sx, sy, and sz scale coefficients
	 * will simply multiply each point / 100.0.
	 */

	/* Calculate scales, all based on height. */
	sx = sy = sz = obj_premodeled_ptr->height / 100.0f;

	/* Set up gl states. */
	StateGLShadeModel(&display->state_gl, GL_FLAT);

	glColor4f(0.45f, 0.45f, 0.45f, 1.0f);

	/* Draw close range model? */
	if(camera_dist < 2000.0f)
	{
	    /* Guy wires. */
	    glBegin(GL_LINES);
	    {
		glNormal3f(0.0f, 1.0f, 0.0f);

		glVertex3f(-29.0f * sx,  0.0f * sz, -(-29.0f * sy));
		glVertex3f(  0.0f * sx, 75.0f * sz, -(  0.0f * sy));

		glVertex3f( 29.0f * sx,  0.0f * sz, -(-29.0f * sy));
		glVertex3f(  0.0f * sx, 75.0f * sz, -(  0.0f * sy));

		glVertex3f(  0.0f * sx,  0.0f * sz, -( 40.0f * sy));
		glVertex3f(  0.0f * sx, 75.0f * sz, -(  0.0f * sy));
	    }
	    glEnd();

	    sx = sy = 1.0f;
	    glBegin(GL_LINE_STRIP);
	    {
		glNormal3f(0.0f, 1.0f, 0.0f);

		glVertex3f(  3.0f * sx,   0.0f * sz, -( 0.0f * sy));
		glVertex3f( -3.0f * sx,   0.0f * sz, -( 0.0f * sy));
		glVertex3f( -3.0f * sx, 100.0f * sz, -( 0.0f * sy));
		glVertex3f(  3.0f * sx,  90.0f * sz, -( 0.0f * sy));
		glVertex3f( -3.0f * sx,  80.0f * sz, -( 0.0f * sy));
		glVertex3f(  3.0f * sx,  70.0f * sz, -( 0.0f * sy));
		glVertex3f( -3.0f * sx,  60.0f * sz, -( 0.0f * sy));
		glVertex3f(  3.0f * sx,  50.0f * sz, -( 0.0f * sy));
		glVertex3f( -3.0f * sx,  40.0f * sz, -( 0.0f * sy));
		glVertex3f(  3.0f * sx,  30.0f * sz, -( 0.0f * sy));
		glVertex3f( -3.0f * sx,  20.0f * sz, -( 0.0f * sy));
		glVertex3f(  3.0f * sx,  10.0f * sz, -( 0.0f * sy));
		glVertex3f( -3.0f * sx,   0.0f * sz, -( 0.0f * sy));

		glVertex3f( -3.0f * sx,   0.0f * sz, -(-0.2f * sy));
		glVertex3f(  0.0f * sx,   0.0f * sz, -( 5.0f * sy));
		glVertex3f(  0.0f * sx, 100.0f * sz, -( 5.0f * sy));
		glVertex3f( -3.0f * sx,  90.0f * sz, -(-0.2f * sy));
		glVertex3f(  0.0f * sx,  80.0f * sz, -( 5.0f * sy));
		glVertex3f( -3.0f * sx,  70.0f * sz, -(-0.2f * sy));
		glVertex3f(  0.0f * sx,  60.0f * sz, -( 5.0f * sy));
		glVertex3f( -3.0f * sx,  50.0f * sz, -(-0.2f * sy));
		glVertex3f(  0.0f * sx,  40.0f * sz, -( 5.0f * sy));
		glVertex3f( -3.0f * sx,  30.0f * sz, -(-0.2f * sy));
		glVertex3f(  0.0f * sx,  20.0f * sz, -( 5.0f * sy));
		glVertex3f( -3.0f * sx,  10.0f * sz, -(-0.2f * sy));
		glVertex3f(  0.0f * sx,   0.0f * sz, -( 5.0f * sy));

		glVertex3f(  0.0f * sx,   0.0f * sz, -( 5.2f * sy));
		glVertex3f(  3.0f * sx,   0.0f * sz, -( 0.0f * sy));
		glVertex3f(  3.0f * sx, 100.0f * sz, -( 0.0f * sy));
		glVertex3f(  0.0f * sx,  90.0f * sz, -( 5.2f * sy));
		glVertex3f(  3.0f * sx,  80.0f * sz, -( 0.0f * sy));
		glVertex3f(  0.0f * sx,  70.0f * sz, -( 5.2f * sy));
		glVertex3f(  3.0f * sx,  60.0f * sz, -( 0.0f * sy));
		glVertex3f(  0.0f * sx,  50.0f * sz, -( 5.2f * sy));
		glVertex3f(  3.0f * sx,  40.0f * sz, -( 0.0f * sy));
		glVertex3f(  0.0f * sx,  30.0f * sz, -( 5.2f * sy));
		glVertex3f(  3.0f * sx,  20.0f * sz, -( 0.0f * sy));
		glVertex3f(  0.0f * sx,  10.0f * sz, -( 5.2f * sy));
		glVertex3f(  3.0f * sx,   0.0f * sz, -( 0.0f * sy));
	    }
	    glEnd();
	}
	else
	{
	    /* Draw far model. */
	    glBegin(GL_LINES);
	    {
		glNormal3f(0.0f, 1.0f, 0.0f);

		glVertex3f(0.0f * sx,   0.0f * sz, -(0.0f * sy));
		glVertex3f(0.0f * sx, 100.0f * sz, -(0.0f * sy));
	    }
	    glEnd();
	}

	/* Restore gl states. */
	StateGLShadeModel(&display->state_gl, shade_model_mode);
}
