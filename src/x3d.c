#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef __MSW__
# include <windows.h>
#endif
#include <GL/gl.h>
#include "../include/string.h"
#include "x3d.h"


static const char *X3DGetValues1f(const char *arg, GLfloat *vf);
static const char *X3DGetValues2f(const char *arg, GLfloat *vf);
static const char *X3DGetValues3f(const char *arg, GLfloat *vf);
static const char *X3DGetValues4f(const char *arg, GLfloat *vf);
static GLuint X3DMatchTexture(
	X3DTextureRef **texture, int total_textures,
	const char *name
);

GLuint X3DOpenDataGLList(
	char **data,
	const X3DInterpValues *v,
	X3DTextureRef **texture, int total_textures
);
void X3DOpenDataGLOutput(
	char **data,
	const X3DInterpValues *v,
	X3DTextureRef **texture, int total_textures
);
X3DTextureRef *X3DTextureRefAppend(
	X3DTextureRef ***texture, int *total_textures,
	const char *name,
	GLuint id
);
void X3DTextureDeleteAll(
	X3DTextureRef ***texture, int *total_textures
);


#define ISBLANK(c)	(((c) == ' ') || ((c) == '\t'))

#define ATOI(s)         (((s) != NULL) ? atoi(s) : 0)
#define ATOL(s)         (((s) != NULL) ? atol(s) : 0)
#define ATOF(s)         (((s) != NULL) ? (float)atof(s) : 0.0f)
#define STRDUP(s)       (((s) != NULL) ? strdup(s) : NULL)

#define MAX(a,b)        (((a) > (b)) ? (a) : (b))
#define MIN(a,b)        (((a) < (b)) ? (a) : (b))
#define CLIP(a,l,h)     (MIN(MAX((a),(l)),(h)))


/*
 *      Get 1 value from the argument string arg and seek arg to the
 *      2nd argument (or returns NULL if end of string is reached).
 */
static const char *X3DGetValues1f(const char *arg, GLfloat *vf)
{
	if((arg != NULL) ? (*arg == '\0') : GL_TRUE)
	{
	    vf[0] = 0.0f;
	    return(NULL);
	}

	/* Seek to 1st argument and get argument. */
	while(ISBLANK(*arg))
	    arg++;
	vf[0] = ATOF(arg);

	/* Seek to 2nd argument. */
	while(!ISBLANK(*arg) && (*arg != '\0'))
	    arg++;
	while(ISBLANK(*arg))
	    arg++;

	/* Return new argument position or NULL if end of string. */
	return((*arg != '\0') ? arg : NULL);
}

/*
 *      Get 2 values from the argument string arg and seek arg to the
 *      3rd argument (or returns NULL if end of string is reached).
 */
static const char *X3DGetValues2f(const char *arg, GLfloat *vf)
{
	if((arg != NULL) ? (*arg == '\0') : GL_TRUE)
	{
	    vf[0] = 0.0f;
	    vf[1] = 0.0f;
	    return(NULL);
	}

	/* Seek to 1st argument and get argument. */
	while(ISBLANK(*arg))
	    arg++;
	vf[0] = ATOF(arg);

	/* Seek to 2nd argument and get argument. */
	while(!ISBLANK(*arg) && (*arg != '\0'))
	    arg++;
	while(ISBLANK(*arg))
	    arg++;
	vf[1] = ATOF(arg);

	/* Seek to 3rd argument. */
	while(!ISBLANK(*arg) && (*arg != '\0'))
	    arg++;
	while(ISBLANK(*arg))
	    arg++;

	/* Return new argument position or NULL if end of string. */
	return((*arg != '\0') ? arg : NULL);
}


/*
 *	Get 3 values from the argument string arg and seek arg to the
 *	4th argument (or returns NULL if end of string is reached).
 */
static const char *X3DGetValues3f(const char *arg, GLfloat *vf)
{
	if((arg != NULL) ? (*arg == '\0') : GL_TRUE)
	{
	    vf[0] = 0.0f;
	    vf[1] = 0.0f;
	    vf[2] = 0.0f;
	    return(NULL);
	}

	/* Seek to 1st argument and get argument. */
	while(ISBLANK(*arg))
	    arg++;
	vf[0] = ATOF(arg);

	/* Seek to 2nd argument and get argument. */
	while(!ISBLANK(*arg) && (*arg != '\0'))
	    arg++;
	while(ISBLANK(*arg))
	    arg++;
	vf[1] = ATOF(arg);

	/* Seek to 3rd argument and get argument. */
	while(!ISBLANK(*arg) && (*arg != '\0'))
	    arg++;
	while(ISBLANK(*arg))
	    arg++;
	vf[2] = ATOF(arg);

	/* Seek to 4th argument. */
	while(!ISBLANK(*arg) && (*arg != '\0'))
	    arg++;
	while(ISBLANK(*arg))
	    arg++;

	/* Return new argument position or NULL if end of string. */
	return((*arg != '\0') ? arg : NULL);
}

/*
 *      Get 4 values from the argument string arg and seek arg to the
 *      5th argument (or returns NULL if end of string is reached).
 */
static const char *X3DGetValues4f(const char *arg, GLfloat *vf)
{
	if((arg != NULL) ? (*arg == '\0') : GL_TRUE)
	{
	    vf[0] = 0.0f;
	    vf[1] = 0.0f;
	    vf[2] = 0.0f;
	    vf[3] = 0.0f;
	    return(NULL);
	}

	/* Seek to 1st argument and get argument. */
	while(ISBLANK(*arg))
	    arg++;
	vf[0] = ATOF(arg);

	/* Seek to 2nd argument and get argument. */
	while(!ISBLANK(*arg) && (*arg != '\0'))
	    arg++;
	while(ISBLANK(*arg))
	    arg++;
	vf[1] = ATOF(arg);

	/* Seek to 3rd argument and get argument. */
	while(!ISBLANK(*arg) && (*arg != '\0'))
	    arg++;
	while(ISBLANK(*arg))
	    arg++;
	vf[2] = ATOF(arg);

	/* Seek to 4th argument and get argument. */
	while(!ISBLANK(*arg) && (*arg != '\0'))
	    arg++;
	while(ISBLANK(*arg))
	    arg++;
	vf[3] = ATOF(arg);

	/* Seek to 5th argument. */
	while(!ISBLANK(*arg) && (*arg != '\0'))
	    arg++;
	while(ISBLANK(*arg))
	    arg++;

	/* Return new argument position or NULL if end of string. */
	return((*arg != '\0') ? arg : NULL);
}

/*
 *	Returns a gl texture id that matches the given texture name in
 *	the list or returns 0 on failed match.
 */
static GLuint X3DMatchTexture(
	X3DTextureRef **texture, int total_textures,
	const char *name
)
{
	int i;
	X3DTextureRef *t;

	if(name == NULL)
	    return(0);

	for(i = 0; i < total_textures; i++)
	{
	    t = texture[i];
	    if(t == NULL)
		continue;

	    if(t->name == NULL)
		continue;

	    if(!strcasecmp(t->name, name))
		return(t->id);
	}

	return(0);
}


/*
 *	Open the X3D data into a GL list.
 */
GLuint X3DOpenDataGLList(
	char **data,
	const X3DInterpValues *v,
	X3DTextureRef **texture, int total_textures
)
{
	GLuint gllist = glGenLists(1);
	glNewList(gllist, GL_COMPILE);
	X3DOpenDataGLOutput(data, v, texture, total_textures);
	glEndList();
	return(gllist);
}

/*
 *	Open the X3D data and send it out as GL commands.
 */
void X3DOpenDataGLOutput(
	char **data,
	const X3DInterpValues *v,
	X3DTextureRef **texture, int total_textures
)
{
	GLboolean	texture_on = GL_FALSE;
	unsigned long	flags = X3D_DEFAULT_VALUE_FLAGS;
	int		coordinate_system = X3D_COORDINATE_SYSTEM_XYZ;
	GLfloat		scale_x = 1.0f,
			scale_y = 1.0f,
			scale_z = 1.0f;
	GLfloat		offset_x = 0.0f,
			offset_y = 0.0f,
			offset_z = 0.0f;
	int i;
	char *s, *s2, *s_end;
	const char *arg;
	char parm[X3D_PARM_MAX];


	if(data == NULL)
	    return;

	/* Get interpretation values. */
	if(v != NULL)
	{
	    flags = v->flags;
	    if(flags & X3D_VALUE_FLAG_COORDINATE_SYSTEM)
		coordinate_system = v->coordinate_system;
	    if(flags & X3D_VALUE_FLAG_SCALE)
	    {
		scale_x = v->scale_x;
		scale_y = v->scale_y;
		scale_z = v->scale_z;
	    }
	    if(flags & X3D_VALUE_FLAG_OFFSET)
	    {
		offset_x = v->offset_x;
		offset_y = v->offset_y;
		offset_z = v->offset_z;
	    }
	}

	/* Iterate through each data line. */
	for(i = 0; data[i] != NULL; i++)
	{
	    /* Skip to line #4 (lines #0 to #3 are header and ignored). */
	    if(i < 4)
		continue;

	    /* Get parameter. */
	    s = data[i];
	    s_end = s + X3D_PARM_MAX - 1;
	    s2 = parm;
	    while(!ISBLANK(*s) && (*s != '\0') && (s < s_end))
		*s2++ = *s++;
	    *s2 = '\0';

	    /* Get argument. */
	    arg = s;
	    while(ISBLANK(*arg))
		arg++;


	    /* Enable. */
	    if(!strcasecmp(parm, "enable"))
	    {
		if(!(flags & X3D_VALUE_FLAG_SKIP_STATE_CHANGES))
		{

		}
	    }
	    /* Disable. */
	    else if(!strcasecmp(parm, "disable"))
	    {
		if(!(flags & X3D_VALUE_FLAG_SKIP_STATE_CHANGES))
		{

		}
	    }
	    /* Points. */
	    else if(!strcasecmp(parm, "begin_points"))
	    {
		glBegin(GL_POINTS);
	    }
	    else if(!strcasecmp(parm, "end_points"))
	    {
		glEnd();
	    }
	    /* Lines. */
	    else if(!strcasecmp(parm, "begin_lines"))
	    {
		glBegin(GL_LINES);
	    }
	    else if(!strcasecmp(parm, "end_lines"))
	    {
		glEnd();
	    }
	    /* Line strip. */
	    else if(!strcasecmp(parm, "begin_line_strip"))
	    {
		glBegin(GL_LINE_STRIP);
	    }
	    else if(!strcasecmp(parm, "end_line_strip"))
	    {
		glEnd();
	    }
	    /* Line loop. */
	    if(!strcasecmp(parm, "begin_line_loop"))
	    {
		glBegin(GL_LINE_LOOP);
	    }
	    else if(!strcasecmp(parm, "end_line_loop"))
	    {
		glEnd();
	    }
	    /* Triangles. */
	    else if(!strcasecmp(parm, "begin_triangles"))
	    {
		glBegin(GL_TRIANGLES);
	    }
	    else if(!strcasecmp(parm, "end_triangles"))
	    {
		glEnd();
	    }
	    /* Triangle strip. */
	    else if(!strcasecmp(parm, "begin_triangle_strip"))
	    {
		glBegin(GL_TRIANGLE_STRIP);
	    }
	    else if(!strcasecmp(parm, "end_triangle_strip"))
	    {
		glEnd();
	    }
	    /* Triangle fan. */
	    else if(!strcasecmp(parm, "begin_triangle_fan"))
	    {
		glBegin(GL_TRIANGLE_FAN);
	    }
	    else if(!strcasecmp(parm, "end_triangle_fan"))
	    {
		glEnd();
	    }
	    /* Quads. */
	    else if(!strcasecmp(parm, "begin_quads"))
	    {
		glBegin(GL_QUADS);
	    }
	    else if(!strcasecmp(parm, "end_quads"))
	    {
		glEnd();
	    }
	    /* Quad strip. */
	    else if(!strcasecmp(parm, "begin_quad_strip"))
	    {
		glBegin(GL_QUAD_STRIP);
	    }
	    else if(!strcasecmp(parm, "end_quad_strip"))
	    {
		glEnd();
	    }
	    /* Polygon. */
	    else if(!strcasecmp(parm, "begin_polygon"))
	    {
		glBegin(GL_POLYGON);
	    }
	    else if(!strcasecmp(parm, "end_polygon"))
	    {
		glEnd();
	    }
	    /* Normal. */
	    else if(!strcasecmp(parm, "normal") ||
	            !strcasecmp(parm, "normal3") ||
		    !strcasecmp(parm, "normal3f")
	    )
	    {
		GLfloat vf[3];

		arg = X3DGetValues3f(arg, vf);
		if(!(flags & X3D_VALUE_FLAG_SKIP_NORMALS))
		{
		    if(coordinate_system == X3D_COORDINATE_SYSTEM_GL)
			glNormal3f(vf[0], vf[1], vf[2]);
		    else
			glNormal3f(vf[0], vf[2], -vf[1]);
		}
	    }
	    /* Texcoord. */
	    else if(!strcasecmp(parm, "texcoord1f") ||
		    !strcasecmp(parm, "texcoord1")
	    )
	    {
		GLfloat vf[1];

		arg = X3DGetValues1f(arg, vf);
		if(!(flags & X3D_VALUE_FLAG_SKIP_TEXCOORDS))
		{
		    if(coordinate_system == X3D_COORDINATE_SYSTEM_GL)
			glTexCoord1f(vf[0]);
		    else
			glTexCoord1f(vf[0]);
		}
	    }
	    else if(!strcasecmp(parm, "texcoord2f") ||
		    !strcasecmp(parm, "texcoord2") ||
		    !strcasecmp(parm, "texcoord")
	    )
	    {
		GLfloat vf[2];

		arg = X3DGetValues2f(arg, vf);
		if(!(flags & X3D_VALUE_FLAG_SKIP_TEXCOORDS))
		{
		    if(coordinate_system == X3D_COORDINATE_SYSTEM_GL)
			glTexCoord2f(vf[0], vf[1]);
		    else
			glTexCoord2f(vf[0], 1.0f - vf[1]);
		}
	    }
	    else if(!strcasecmp(parm, "texcoord3f") ||
		    !strcasecmp(parm, "texcoord3")
	    )
	    {
		GLfloat vf[3];

		arg = X3DGetValues3f(arg, vf);
		if(!(flags & X3D_VALUE_FLAG_SKIP_TEXCOORDS))
		{
		    if(coordinate_system == X3D_COORDINATE_SYSTEM_GL)
			glTexCoord3f(vf[0], vf[1], vf[2]);
		    else
			glTexCoord3f(vf[0], 1.0f - vf[1], vf[2]);
		}
	    }
	    /* Vertex. */
	    else if(!strcasecmp(parm, "vertex3f") ||
		    !strcasecmp(parm, "vertex3") ||
		    !strcasecmp(parm, "vertex")
	    )
	    {
		GLfloat vf[3];

		arg = X3DGetValues3f(arg, vf);
		if(!(flags & X3D_VALUE_FLAG_SKIP_VERTICES))
		{
		    if(coordinate_system == X3D_COORDINATE_SYSTEM_GL)
			glVertex3f(
			    (vf[0] * scale_x) + offset_x,
			    (vf[1] * scale_y) + offset_y,
			    (vf[2] * scale_z) + offset_z
			);
		    else
			glVertex3f(
			    (vf[0] * scale_x) + offset_x,
			    (vf[2] * scale_z) + offset_z,
			    -((vf[1] * scale_y) + offset_y)
			);
		}
	    }
	    else if(!strcasecmp(parm, "vertex4f") ||
		    !strcasecmp(parm, "vertex4")
	    )
	    {
		GLfloat vf[4];

		arg = X3DGetValues4f(arg, vf);
		if(!(flags & X3D_VALUE_FLAG_SKIP_VERTICES))
		{
		    if(coordinate_system == X3D_COORDINATE_SYSTEM_GL)
			glVertex4f(
			    (vf[0] * scale_x) + offset_x,
			    (vf[1] * scale_y) + offset_y,
			    (vf[2] * scale_z) + offset_z,
			    vf[3]
			);
		    else
			glVertex4f(
			    (vf[0] * scale_x) + offset_x,
			    (vf[2] * scale_z) + offset_z,
			    -((vf[1] * scale_y) + offset_y),
			    vf[3]
			);
		}
	    }
	    /* Color */
	    else if(!strcasecmp(parm, "color3f") ||
		    !strcasecmp(parm, "color3") ||
		    !strcasecmp(parm, "color")
	    )
	    {
		GLfloat vf[3];

		arg = X3DGetValues3f(arg, vf);
		if(!(flags & X3D_VALUE_FLAG_SKIP_COLORS))
		    glColor3f(vf[0], vf[1], vf[2]);
	    }
	    else if(!strcasecmp(parm, "color4f") ||
		    !strcasecmp(parm, "color4")
	    )
	    {
		GLfloat vf[4];

		arg = X3DGetValues4f(arg, vf);
		if(!(flags & X3D_VALUE_FLAG_SKIP_COLORS))
		    glColor4f(vf[0], vf[1], vf[2], vf[3]);
	    }
	    /* Bind Texture */
	    else if(!strcasecmp(parm, "bind_texture_1d"))
	    {
		GLuint id = X3DMatchTexture(texture, total_textures, arg);
		if(id != 0)
		{
		    glBindTexture(GL_TEXTURE_1D, id);
		    texture_on = GL_TRUE;
		}
		else
		{
		    glBindTexture(GL_TEXTURE_1D, 0);
		    texture_on = GL_FALSE;
		}
	    }
	    else if(!strcasecmp(parm, "bind_texture_2d") ||
		    !strcasecmp(parm, "bind_texture")
	    )
	    {
		GLuint id = X3DMatchTexture(texture, total_textures, arg);
		if(id != 0)
		{
		    glBindTexture(GL_TEXTURE_2D, id);
		    texture_on = GL_TRUE;
		}
		else
		{
		    glBindTexture(GL_TEXTURE_2D, 0);
		    texture_on = GL_FALSE;
		}
	    }
#ifdef GL_TEXTURE_3D
	    else if(!strcasecmp(parm, "bind_texture_3d"))
	    {
		GLuint id = X3DMatchTexture(texture, total_textures, arg);
		if(id != 0)
		{
		    glBindTexture(GL_TEXTURE_3D, id);
		    texture_on = GL_TRUE;
		}
		else
		{
		    glBindTexture(GL_TEXTURE_3D, 0);
		    texture_on = GL_FALSE;
		}
	    }
#endif

	}
}

/*
 *	Appends a texture reference to the list.
 */
X3DTextureRef *X3DTextureRefAppend(
	X3DTextureRef ***texture, int *total_textures,
	const char *name,
	GLuint id
)
{
	int i;
	X3DTextureRef *t = NULL;

	if((texture == NULL) || (total_textures == NULL))
	    return(t);

	i = MAX(*total_textures, 0);
	*total_textures = i + 1;
	*texture = (X3DTextureRef **)realloc(
	    *texture,
	    (*total_textures) * sizeof(X3DTextureRef *)
	);
	if(*texture == NULL)
	{
	    *total_textures = 0;
	    return(t);
	}

	*texture[i] = t = (X3DTextureRef *)calloc(
	    1, sizeof(X3DTextureRef)
	);
	if(t == NULL)
	    return(t);

	t->name = STRDUP(name);
	t->id = id;

	return(t);
}

/*
 *	Delete all texture references.
 */
void X3DTextureDeleteAll(
	X3DTextureRef ***texture, int *total_textures
)
{
	int i;
	X3DTextureRef *t;

	if((texture == NULL) || (total_textures == NULL))
	    return;

	for(i = 0; i < *total_textures; i++)
	{
	    t = (*texture)[i];
	    if(t == NULL)
		continue;

	    free(t->name);
	    free(t);
	}

	free(*texture);
	*texture = NULL;
	*total_textures = 0;
}
