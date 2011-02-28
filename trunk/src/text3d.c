#include <stdlib.h>
#include <ctype.h>
#ifdef __MSW__
# include <windows.h>
#endif
#include <GL/gl.h>
#include "x3d.h"
#include "text3d.h"

#include "text3d/n0.x3d"
#include "text3d/n1.x3d"
#include "text3d/n2.x3d"
#include "text3d/n3.x3d"
#include "text3d/n4.x3d"
#include "text3d/n5.x3d"
#include "text3d/n6.x3d"
#include "text3d/n7.x3d"
#include "text3d/n8.x3d"
#include "text3d/n9.x3d"

#include "text3d/a.x3d"
#include "text3d/b.x3d"
#include "text3d/c.x3d"
#include "text3d/d.x3d"
#include "text3d/e.x3d"
#include "text3d/f.x3d"
#include "text3d/g.x3d"
#include "text3d/h.x3d"
#include "text3d/i.x3d"
#include "text3d/j.x3d"
#include "text3d/k.x3d"
#include "text3d/l.x3d"
#include "text3d/m.x3d"
#include "text3d/n.x3d"
#include "text3d/o.x3d"
#include "text3d/p.x3d"
#include "text3d/q.x3d"
#include "text3d/r.x3d"
#include "text3d/s.x3d"
#include "text3d/t.x3d"
#include "text3d/u.x3d"
#include "text3d/v.x3d"
#include "text3d/w.x3d"
#include "text3d/x.x3d"
#include "text3d/y.x3d"
#include "text3d/z.x3d"

#include "text3d/colon.x3d"
#include "text3d/comma.x3d"
#include "text3d/exclaimation.x3d"
#include "text3d/minus.x3d"
#include "text3d/period.x3d"
#include "text3d/plus.x3d"
#include "text3d/semicolon.x3d"


static char **Text3DGetCharacterData(char c);
void Text3DStringOutput(
	float font_width, float font_height,	/* In meters. */
	float char_spacing,			/* In meters. */
	const char *s,
	float *string_width                     /* In meters. */
);
GLuint Text3DStringList(
	float font_width, float font_height,    /* In meters. */
	float char_spacing,                     /* In meters. */
	const char *s,
	float *string_width                     /* In meters. */
);


/*
 *	Returns a pointer to a statically allocated character
 *	x3d data or NULL if no data is available.
 */
static char **Text3DGetCharacterData(char c)
{
	switch(toupper(c))
	{
	  case '0':
	    return(n0_x3d);
	  case '1':
	    return(n1_x3d);
	  case '2':
	    return(n2_x3d);
	  case '3':
	    return(n3_x3d);
	  case '4':
	    return(n4_x3d);
	  case '5':
	    return(n5_x3d);
	  case '6':
	    return(n6_x3d);
	  case '7':
	    return(n7_x3d);
	  case '8':
	    return(n8_x3d);
	  case '9':
	    return(n9_x3d);

	  case 'A':
	    return(a_x3d);
	  case 'B':
	    return(b_x3d);
	  case 'C':
	    return(c_x3d);
	  case 'D':
	    return(d_x3d);
	  case 'E':
	    return(e_x3d);
	  case 'F':
	    return(f_x3d);
	  case 'G':
	    return(g_x3d);
	  case 'H':
	    return(h_x3d);
	  case 'I':
	    return(i_x3d);
	  case 'J':
	    return(j_x3d);
	  case 'K':
	    return(k_x3d);
	  case 'L':
	    return(l_x3d);
	  case 'M':
	    return(m_x3d);
	  case 'N':
	    return(n_x3d);
	  case 'O':
	    return(o_x3d);
	  case 'P':
	    return(p_x3d);
	  case 'Q':
	    return(q_x3d);
	  case 'R':
	    return(r_x3d);
	  case 'S':
	    return(s_x3d);
	  case 'T':
	    return(t_x3d);
	  case 'U':
	    return(u_x3d);
	  case 'V':
	    return(v_x3d);
	  case 'W':
	    return(w_x3d);
	  case 'X':
	    return(x_x3d);
	  case 'Y':
	    return(y_x3d);
	  case 'Z':
	    return(z_x3d);

	  case ':':
	    return(colon_x3d);
	  case ',':
	    return(comma_x3d);
	  case '!':
	    return(exclaimation_x3d);
	  case '-':
	    return(minus_x3d);
	  case '.':
	    return(period_x3d);
	  case '+':
	    return(plus_x3d);
	  case ';':
	    return(semicolon_x3d);

	  default:
	    return(NULL);
	}
}

/*
 *	Outputs the given string using GL commands.
 */
void Text3DStringOutput(
	float font_width, float font_height,    /* In meters. */
	float char_spacing,                     /* In meters. */
	const char *s,
	float *string_width			/* In meters. */
)
{
	char **x3d_data;
	X3DInterpValues v;


	if(string_width != NULL)
	    *string_width = 0.0f;

	if((s != NULL) ? (*s == '\0') : GL_TRUE)
	    return;

	/* Set up X3D interpretation values. */
	v.flags =	X3D_VALUE_FLAG_COORDINATE_SYSTEM |
			X3D_VALUE_FLAG_SCALE |
			X3D_VALUE_FLAG_OFFSET |
			X3D_VALUE_FLAG_SKIP_COLORS |
			X3D_VALUE_FLAG_SKIP_TEXTURES |
			X3D_VALUE_FLAG_SKIP_TEXCOORDS |
			X3D_VALUE_FLAG_SKIP_TRANSLATES |
			X3D_VALUE_FLAG_SKIP_ROTATES;
	v.coordinate_system = X3D_COORDINATE_SYSTEM_XYZ;

	v.scale_x = font_width;
	v.scale_y = font_height;
	v.scale_z = 1.0f;

	v.offset_x = 0.0f;
	v.offset_y = 0.0f;
	v.offset_z = 0.0f;

	/* Begin gl commands by iterating through string. */
	while(*s != '\0')
	{
	    x3d_data = Text3DGetCharacterData(*s);
	    if(x3d_data != NULL)
		X3DOpenDataGLOutput(x3d_data, &v, NULL, 0);
	    v.offset_x += (GLfloat)(font_width + char_spacing);
	    s++;
	}

	if(string_width != NULL)
	    *string_width = v.offset_x - char_spacing;
}


/*
 *	Generates a GL list based on the given string.
 */
GLuint Text3DStringList(
	float font_width, float font_height,    /* In meters. */
	float char_spacing,                     /* In meters. */
	const char *s,
	float *string_width                     /* In meters. */
)
{
	GLuint list = glGenLists(1);
	glNewList(list, GL_COMPILE);
	Text3DStringOutput(
	    font_width, font_height, char_spacing, s, string_width
	);
	glEndList();
	return(list);
}
