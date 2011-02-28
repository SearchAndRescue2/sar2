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
