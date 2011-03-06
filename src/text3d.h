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
 *                            OpenGL 3D Text
 */

#ifndef TEXT3D_H
#define TEXT3D_H

#include <GL/gl.h>


extern void Text3DStringOutput(
	float font_width, float font_height,	/* In meters. */
	float char_spacing,			/* In meters. */
	const char *s,
	float *string_width                     /* In meters. */
);
extern GLuint Text3DStringList(
	float font_width, float font_height,    /* In meters. */
	float char_spacing,                     /* In meters. */
	const char *s,
	float *string_width                     /* In meters. */
);


#endif	/* TEXT3D_H */
