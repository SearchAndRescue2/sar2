#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/string.h"

#include "gw.h"
#include "sar.h"
#include "sarsplash.h"
#include "config.h"

#include "fonts/menu.fnt"
#include "gwdata/opengl.hbm"


void SARSplash(sar_core_struct *core_ptr);


#define ATOI(s)         (((s) != NULL) ? atoi(s) : 0)
#define ATOL(s)         (((s) != NULL) ? atol(s) : 0)
#define ATOF(s)         (((s) != NULL) ? (float)atof(s) : 0.0f)
#define STRDUP(s)       (((s) != NULL) ? strdup(s) : NULL)

#define MAX(a,b)        (((a) > (b)) ? (a) : (b))
#define MIN(a,b)        (((a) < (b)) ? (a) : (b))
#define CLIP(a,l,h)     (MIN(MAX((a),(l)),(h)))
#define STRLEN(s)       (((s) != NULL) ? ((int)strlen(s)) : 0)
#define IS_STRING_EMPTY(s)      (((s) != NULL) ? ((s) == '\0') : True)

#define RADTODEG(r)     ((r) * 180 / PI)
#define DEGTORAD(d)     ((d) * PI / 180)


/*
 *      Called by SARInit().
 *
 *      Splash routine.
 */
void SARSplash(sar_core_struct *core_ptr)
{
	int	width, height,
		img_bpp, img_width, img_height;
	const unsigned char *img_data;
	gw_display_struct *display = core_ptr->display;
	if(display == NULL)
	    return;

	GWContextGet(
	    display, GWContextCurrent(display),
	    NULL, NULL,
	    NULL, NULL,
	    &width, &height
	);

	/* Set up GL viewport and perspective for drawing */
	glViewport(0, 0, width, height);
	GWOrtho2D(display);

	/* Clear background */
	glClearColor(1.0f, 1.0f, 1.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	/* Draw splash image */
	img_data = opengl_hbm;
	GWImageLoadHeaderFromData(
	    display, &img_width, &img_height, NULL, &img_bpp,
	    img_data
	);
	if(img_bpp == 3)
	    GWImageDrawFromDataRGB(
		display, img_data,
		(width / 2) - (img_width / 2),
		(height / 2) - (img_height / 2),
		img_width, img_height
	    );
	else if(img_bpp == 4)
	    GWImageDrawFromDataRGBA(
		display, img_data,
		(width / 2) - (img_width / 2),
		(height / 2) - (img_height / 2),
		img_width, img_height
	    );

	/* Draw version */
	if(True)
	{
	    int fw, fh, sw;
	    GWFont *font = font_menu;
	    char *s = (char *)malloc(
		(80 + 80 + 80) * sizeof(char)
	    );
	    sprintf(
		s,
		"Version %i.%i",
		display->gl_version_major,
		display->gl_version_minor
	    );
	    GWGetFontSize(font, NULL, NULL, &fw, &fh);
	    sw = STRLEN(s) * fw;

	    glColor3f(0.16f, 0.41f, 0.55f);
	    glBegin(GL_QUADS);
	    {
		int	x_min = (width / 2) - (img_width / 2),
			x_max = x_min + img_width,
			y_min = height - ((height / 2) + (img_height / 2) + 5),
			y_max = y_min - (fh + (2 * 2));
		glVertex2i(x_min, y_min);
		glVertex2i(x_min, y_max);
		glVertex2i(x_max, y_max);
		glVertex2i(x_max, y_min);
	    }
	    glEnd();

	    glColor3f(1.0f, 1.0f, 1.0f);
	    GWSetFont(display, font);
	    GWDrawString(
		display,
		(width / 2) - (sw / 2),
		(height / 2) + (img_height / 2) + 5 + 2,
		s
	    );
	    free(s);
	}

	GWSwapBuffer(display);
	GWFlush(display);
}
