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
#include <sys/stat.h>

#ifdef __MSW__
# include <windows.h>
#endif

#include <GL/gl.h>

#include "../include/disk.h"
#include "../include/tga.h"

#include "gw.h"
#include "stategl.h"
#include "image.h"
#include "sar.h"


sar_image_struct *SARImageNew(
	sar_image_type type,
	int width, int height,
	u_int8_t *data
);
sar_image_struct *SARImageNewFromFile(const char *filename);
void SARImageDelete(sar_image_struct *img);

void SARImageDraw(
	gw_display_struct *display,
	const sar_image_struct *img,
	int x, int y,
	int width, int height
);


#define ATOI(s)		(((s) != NULL) ? atoi(s) : 0)
#define ATOL(s)		(((s) != NULL) ? atol(s) : 0)
#define ATOF(s)		(((s) != NULL) ? atof(s) : 0.0f)
#define STRDUP(s)	(((s) != NULL) ? strdup(s) : NULL)

#define MAX(a,b)	(((a) > (b)) ? (a) : (b))
#define MIN(a,b)	(((a) < (b)) ? (a) : (b))
#define CLIP(a,l,h)	(MIN(MAX((a),(l)),(h)))
#define STRLEN(s)	(((s) != NULL) ? strlen(s) : 0)
#define STRISEMPTY(s)	(((s) != NULL) ? (*(s) == '\0') : 1)

#define RADTODEG(r)	((r) * 180.0 / PI)
#define DEGTORAD(d)	((d) * PI / 180.0)


/*
 *	Creates a new image from the specified values.
 *
 *	The data will be transfered to the image and should not be
 *	referenced again after this call.
 */
sar_image_struct *SARImageNew(
	sar_image_type type,
	int width, int height,
	u_int8_t *data
)
{
	sar_image_struct *img = SAR_IMAGE(calloc(
	    1, sizeof(sar_image_struct)
	));
	img->type = type;
	img->width = width;
	img->height = height;
	img->data = data;
	return(img);
}

/*
 *	Creates a new image from the specified file.
 *
 *	If filename is not an absolute path then the file will be
 *	checked for in the local data directory and then the global
 *	data directory.
 */
sar_image_struct *SARImageNewFromFile(const char *filename)
{
	char *dpath;
	int width, height;
	u_int8_t *data;
	sar_image_struct *img;
	struct stat stat_buf;


	if(STRISEMPTY(filename))
	    return(NULL);

	if(ISPATHABSOLUTE(filename))
	{
	    dpath = STRDUP(filename);
	}
	else
	{
	    const char *s = PrefixPaths(dname.local_data, filename);
	    if((s != NULL) ? stat(s, &stat_buf) : True)
		s = PrefixPaths(dname.global_data, filename);
	    dpath = STRDUP(s);
	}
	if(dpath == NULL)
	    return(NULL);

	/* File exists? */
	if(stat(dpath, &stat_buf))
	{
	    fprintf(
		stderr,
		"%s: No such file.\n",
		dpath
	    );
	    free(dpath);
	    return(NULL);
	}
#ifdef S_ISREF
	if(!S_ISREG(stat_buf.st_mode))
	{
	    fprintf(
		stderr,
		"%s: Not a file.\n",
		dpath
	    );
	    free(dpath);
	    return(NULL);
	}
#endif

	/* Load data from tga image file */
	data = TgaReadFromFileFastRGBA(
	    dpath, &width, &height, 0x00000000
	);
	if(data == NULL)
	{
	    free(dpath);
	    return(NULL);
	}

	/* Create a new image */
	img = SARImageNew(
	    SAR_IMAGE_TYPE_RGBA,	/* Type */
	    width, height,		/* Size */
	    data			/* Image Data (Transfered) */
	);

	free(dpath);

	return(img);
}

/*
 *	Deletes the image.
 */
void SARImageDelete(sar_image_struct *img)
{
	if(img == NULL)
	    return;

	free(img->data);
	free(img);
}

/*
 *	Draws the image to the current GL context at the specified
 *	geometry.
 */
void SARImageDraw(
	gw_display_struct *display,
	const sar_image_struct *img,
	int x, int y,
	int width, int height
)
{
	StateGLBoolean alpha_test;
	GLint x2, y2;
	GLfloat x_zoom_factor, y_zoom_factor;
	int win_width, win_height;


	if((display == NULL) || (img == NULL))
	    return;

	if((img->width < 1) || (img->height < 1) ||
	   (img->data == NULL)
	)
	    return;

	GWContextGet(
	    display, GWContextCurrent(display),
	    NULL, NULL,
	    NULL, NULL,
	    &win_width, &win_height
	);

	/* Calculate GL converted coordinate positions */
	x2 = (GLint)x;
	y2 = (GLint)(win_height - y - 1);

	/* Sanitize positions */
	if(x2 < 0)
	    x2 = 0;
	if(y2 < 0)
	    y2 = 0;

	/* Calculate zoom factors */
	x_zoom_factor = (GLfloat)width / (GLfloat)img->width;
	y_zoom_factor = -(GLfloat)height / (GLfloat)img->height;
 

	/* Set starting drawing position */
	glRasterPos2i(
	    CLIP(x2, 0, win_width - 1),
	    CLIP(y2, 0, win_height - 1)
	);

	/* Set zoom factor */
	glPixelZoom(x_zoom_factor, y_zoom_factor);

	/* Enable GL alpha testing (some images have transparent pixels) */
	alpha_test = display->state_gl.alpha_test;
	StateGLEnable(&display->state_gl, GL_ALPHA_TEST);
	StateGLAlphaFunc(&display->state_gl, GL_GREATER, 0.5f);

	/* Draw the image */
	glDrawPixels(
	    img->width, img->height,
	    GL_RGBA, GL_UNSIGNED_BYTE,
	    (const GLvoid *)img->data
	);

	/* Restore GL states */
	if(!alpha_test)
	    StateGLDisable(&display->state_gl, GL_ALPHA_TEST);

	/* Restore zoom */
	glPixelZoom(1.0f, 1.0f);
}
