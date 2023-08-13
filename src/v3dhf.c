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
#include <sys/types.h>
#include <sys/stat.h>
#include <math.h>

#include "../include/os.h"

#ifdef __MSW__
# include <windows.h>
#endif
#include <GL/gl.h>

#include "../include/tga.h"

#include "v3dhf.h"

#ifdef MEMWATCH
# include "memwatch.h"
#endif


static void V3DHFNormalByCrossGridVectors(
	float v1i, float v1k,
	float v2j, float v2k
);
static float V3DHFGetZPixelCoeff(
	u_int32_t *ptr,
	int x, int y,
	int width, int height
);
int V3DHFLoadFromFile(
	const char *path,       /* Heightfield image file. */
	double x_len, double y_len, double z_len,       /* Size. */
	int *width_rtn, int *height_rtn,        /* In grids or pixels. */
	double *x_spacing_rtn,  /* Spacing between grids (points). */
	double *y_spacing_rtn,     
	double **data_rtn,	/* Dynamically allocated z points, each of
				 * type double (can be NULL).
				 */
	GLuint gl_list,          /* GL list (can be NULL). */
	v3d_hf_options_struct *hfopt
);

double V3DHFGetHeightFromWorldPosition(
	double x, double y,	/* The world position. */
	double hf_x, double hf_y, double hf_z,	/* HF's world position. */
	double hf_heading,	/* HF's heading in radians. */
	double hf_len_x, double hf_len_y, double hf_len_z,
	int hf_widthp, int hf_heightp,	/* Size of heightfield in points. */
	double *hf_data		/* Heightfield's data. */
);


#ifndef PI
# define PI     3.14159265359
#endif

#define MAX(a,b)        (((a) > (b)) ? (a) : (b))
#define MIN(a,b)        (((a) < (b)) ? (a) : (b))

#define RADTODEG(r)     ((r) * 180 / PI)


/*
 *	Sets normal given the vectors along the grid (left hand rule).
 *
 *	v1 is the vector running left to right along the grid's
 *	`x axis'.
 *
 *	v2 is the vector running fore to aft along the grid's
 *	`y axis'.
 *
 *	i and j are the value along the 2D grid, and k is the
 *	`height'.
 */
static void V3DHFNormalByCrossGridVectors(
	float v1i, float v1k,
	float v2j, float v2k
)
{
	float r[3];	/* Resultant. */
	float m;	/* Magnitude of resultant. */

	r[0] = (float)(-v1k * v2j);
	r[1] = (float)(-v1i * v2k);
	r[2] = (float)(v1i * v2j);

	m = (float)hypot(hypot(r[0], r[1]), r[2]);
	if(m > 0.0)
	    glNormal3f(
		(r[0] / m),
		(r[2] / m),
		(-r[1] / m)
	    );
}

/*
 *	Returns the z pixel value as a coefficient from 0.0 to 1.0,
 *	taking into account only the first 8 bits of the 32 bit pixel.
 *
 *	The given values are assumed valid.
 */
static float V3DHFGetZPixelCoeff(
	u_int32_t *ptr,
	int x, int y,
	int width, int height
)
{
	return((float)((u_int32_t)(ptr[(y * width) + x]) & 0x000000ff) /
	       (float)0xff
	);
}

/*
 *	Loads a heightfield from an greyscale tga image file specified by
 *	path.
 *
 *	If data_rtn is not NULL then the pointer will be set to
 *	a newly allocated array of z points (each z point is of type double)
 *	and the specified width and height will be updated (in units of
 *	points).
 *
 *	If gl_list is not NULL (not (GLuint)0) then OpenGL operations
 *	to draw the heightfield will be recorded. gl_list should have been
 *	created by the calling function and the calling function is
 *	responsable for ending it.
 *
 *	Center of heightfield will be centered at x_len / 2 and
 *	y_len / 2, and the base of the heightfield will be at z = 0.
 *	and highest point (when a pixel value is 0xff) will be z_len.
 *
 *	Returns non zero on error.
 */
int V3DHFLoadFromFile(
	const char *path,       /* Heightfield image file. */
	double x_len, double y_len, double z_len,       /* Size. */
	int *width_rtn, int *height_rtn,	/* In grids or pixels. */
	double *x_spacing_rtn,	/* Spacing between grids. */
	double *y_spacing_rtn,
	double **data_rtn,	/* Dynamically allocated z points, each of
				 * type double (can be NULL).
				 */
	GLuint gl_list,          /* GL list (can be NULL). */
	v3d_hf_options_struct *hfopt
)
{
	int winding = V3D_HF_WIND_CCW;
	int set_normal = V3D_HF_SET_NORMAL_AVERAGED;
	int set_texcoord = V3D_HF_SET_TEXCOORD_ALWAYS;
	float tex_offset_x = 0.0, tex_offset_y = 0.0;
	float tex_width = (float)x_len, tex_height = (float)y_len;

	int status, total_z_points;
	float x_len_half, y_len_half;
	double *data_ptr = NULL;	/* Local data pointer. */
	u_int32_t *img_data;
	struct stat stat_buf;
	tga_data_struct td;


	/* Reset return sizes if possable. */
	if(width_rtn != NULL)
	    (*width_rtn) = 0;
	if(height_rtn != NULL)
	    (*height_rtn) = 0;
	if(data_rtn != NULL)
	    (*data_rtn) = NULL;
	if(x_spacing_rtn != NULL)
	    (*x_spacing_rtn) = 0.0;
	if(y_spacing_rtn != NULL)
	    (*y_spacing_rtn) = 0.0;

	/* Update heightfield options. */
	if(hfopt != NULL)
	{
	    unsigned int flags = hfopt->flags;

	    if(flags & V3D_HF_OPT_FLAG_WINDING)
		winding = hfopt->winding;
	    if(flags & V3D_HF_OPT_FLAG_SET_NORMAL)
		set_normal = hfopt->set_normal;
	    if(flags & V3D_HF_OPT_FLAG_SET_TEXCOORD)
		set_texcoord = hfopt->set_texcoord;
	    if(flags & V3D_HF_OPT_FLAG_TEX_OFFSET)
	    {
		tex_offset_x = (float)hfopt->tex_offset_x;
		tex_offset_y = (float)hfopt->tex_offset_y;
	    }
	    if(flags & V3D_HF_OPT_FLAG_TEX_SIZE)
	    {
		tex_width = (float)hfopt->tex_width;
		tex_height = (float)hfopt->tex_height;
	    }
	}


	/* Path to heightfield image file must be valid. */
	if(path == NULL)
	    return(-1);

	/* Span size of heightfield must be positive. */
	if((x_len <= 0.0) || (y_len <= 0.0) || (z_len <= 0.0))
	{
	    fprintf(
		stderr,
 "V3DHFLoadFromFile(): Error: Heightfield span is not positive.\n"
	    );
	    return(-3);
	}

	/* Span of texture must be positive. */
	if((tex_width <= 0.0) || (tex_height <= 0.0))
	{
	    fprintf(
		stderr,
 "V3DHFLoadFromFile(): Error: Heightfield texture span is not positive.\n"
	    );
	    return(-3);
	}

	/* Calculate half sizes for x and y axises. */
	x_len_half = (float)(x_len / 2);
	y_len_half = (float)(y_len / 2);

	/* Heightfield image file exists? */
	if(stat(path, &stat_buf))
	{
	    fprintf(
		stderr,
		"%s: No such file.\n",
		path
	    );
	    return(-1);
	}
#ifdef S_ISDIR
	if(S_ISDIR(stat_buf.st_mode))
	{
	    fprintf(stderr,
		"%s: Is a directory.\n",
		path
	    );
	    return(-1);
	}
#endif	/* S_ISDIR */

	/* Load data from file, read as 32 bits. */
	status = TgaReadFromFile(
	    path,
	    &td,
	    32		/* Read to 32 bits. */
	);
	if(status != TgaSuccess)
	{
	    TgaDestroyData(&td);
	    return(-1);
	}

	/* Check if size of heightfield image is big enough. */
	if(td.width < 2)
	    fprintf(stderr,
 "%s: Warning: Heightfield image size is too small in width.\n",
		path
	    );
	if(td.height < 2)
	    fprintf(stderr,
 "%s: Warning: Heightfield image size is too small in height.\n",
		path
	    );

	/* Record size. */
	if(width_rtn != NULL)  
	    (*width_rtn) = MAX((int)td.width, 0);
	if(height_rtn != NULL)
	    (*height_rtn) = MAX((int)td.height, 0);

	/* Calculate the number of z points to be the number of
	 * pixels in the heightfield image.
	 */
	total_z_points = MAX((int)td.width, 0) * MAX((int)td.height, 0);

	/* Set source image pointer to loaded image data. */
	img_data = (u_int32_t *)td.data;
	if(img_data == NULL)
	{   
	    TgaDestroyData(&td);
	    return(-1);  
	}

	/* Allocate z points array if input data_rtn is not NULL
	 * and there are z points is non-empty.
	 */
	if((data_rtn != NULL) && (total_z_points > 0))
	{
	    data_ptr = (double *)malloc(
		total_z_points * sizeof(double)
	    );
	    (*data_rtn) = data_ptr;

	    if(data_ptr == NULL)
		fprintf(stderr,
 "V3DHFLoadFromFile(): Cannot allocate memory to load `%s' which has %i heightfield points.\n",
		    path, total_z_points
		);
	}


	/* Begin setting z points to the return buffer. */
	if(total_z_points > 0)
	{
	    /* Set z point data return if allocated. */
	    if(data_ptr != NULL)
	    {
		/* X and y number of grids should match pixels on image. */
		double *data_cur_ptr = data_ptr;
		u_int32_t	*img_ptr = img_data,
				*img_ptr_end = img_data + (td.width * td.height);

		while(img_ptr < img_ptr_end)
		    *data_cur_ptr++ = 
			(double)((*img_ptr++) & 0x000000ff) /
			(double)(0x000000ff)
			* z_len
		    ;
	    }
	}

	/* Begin issuing gl draw commands to draw the heightfield if
	 * a GL list is given (which implies a GL list is being recorded.
	 */
	if((total_z_points > 0) && (gl_list > 0))
	{
	    int cx, cy;         /* Current position in HF pixels (which is
				 * also grid edge).
				 */
	    float x_sp, y_sp;   /* Each grid (pixel) is spaced this many
				 * meters.
				 */
	    int img_w, img_h;   /* Total number of grids on each HF dimension. */
	    GLfloat vz[4];      /* Four corners z values. */


	    /* Get image size. */
	    img_w = (int)td.width;
	    img_h = (int)td.height;

	    /* Calculate spacing. */
	    if(img_w > 0.0)
		x_sp = (float)x_len / (float)img_w;
	    else
		x_sp = 0.0f;

	    if(img_h > 0.0)
		y_sp = (float)y_len / (float)img_h;
	    else
		y_sp = 0.0f;

	    /* Set grid spacing returns if possable. */
	    if(x_spacing_rtn != NULL)
		(*x_spacing_rtn) = x_sp;
	    if(y_spacing_rtn != NULL)
		(*y_spacing_rtn) = y_sp;


	    /* Begin issuing gl commands for recording into the gl list. */

	    glBegin(GL_TRIANGLES);

	    /* Go through each `top edge' grid. */
	    for(cy = 0; cy < img_h; cy++)
	    {
		/* Go through each `left edge' grid. */
		for(cx = 0; cx < img_w; cx++)
		{
		    /* Get z pizel height of each `corner'.
		     * Vertex numbering layout:
		     *
		     *   v1---v2
		     *   | \   |
		     *   |  \  |
		     *   |   \ |
		     *   v3---v4
		     *
		     * Each grid consists of two triangles, an upper
		     * right one and a lower left one.
		     */

		    /* Upper left (vertex #1 always valid). */
		    vz[0] = (float)(V3DHFGetZPixelCoeff(
			img_data, cx, cy, img_w, img_h
		    ) * z_len);

		    /* Upper right (vertex #2). */
		    if((cx + 1) < img_w)
		    {
			vz[1] = (float)(V3DHFGetZPixelCoeff(
			    img_data, cx + 1, cy, img_w, img_h
			) * z_len);
		    }
		    else
		    {
			/* Data for upper right not available, use upper left. */
			vz[1] = vz[0];
		    }

		    /* Lower left (vertex #3). */
		    if((cy + 1) < img_h)
		    {
			vz[2] = (float)(V3DHFGetZPixelCoeff(
			    img_data, cx, cy + 1, img_w, img_h
			) * z_len);
		    }
		    else
		    {
			/* Data for lower left not available, use upper left. */
			vz[2] = vz[0];
		    }

		    /* Lower right (vertex #4). */
		    if(((cx + 1) < img_w) &&
		       ((cy + 1) < img_h)
		    )
		    {
			vz[3] = (float)(V3DHFGetZPixelCoeff(
			    img_data, cx + 1, cy + 1, img_w, img_h
			) * z_len);
		    }
		    /* Lower left valid? */
		    else if((cy + 1) < img_h)
		    {
			vz[3] = vz[2];
		    }
		    /* Upper right valid? */
		    else if((cx + 1) < img_w)
		    {
			vz[3] = vz[1];
		    }
		    /* Upper left and upper right not available, use upper left. */
		    else
		    {
			vz[3] = vz[0];
		    }

		    /* Begin recording GL drawing. */

		    /* Winding counter clockwise? */
		    if(winding == V3D_HF_WIND_CCW)
		    {
			/* Winding counter clockwise. */

			/* Upper right triangle. */
			/* Set normal. */
			if(set_normal != V3D_HF_SET_NORMAL_NEVER)
			    V3DHFNormalByCrossGridVectors(
				x_sp, vz[1] - vz[0],
				y_sp, vz[1] - vz[3]
			    );

			/* Vertex #1. */
			if(set_texcoord != V3D_HF_SET_TEXCOORD_NEVER)
			    glTexCoord2f(
				((cx + 0) * x_sp / tex_width) - tex_offset_x,
				((cy + 0) * y_sp / tex_height) + tex_offset_y
			    );
			glVertex3f(
			    (((cx + 0) * x_sp) - x_len_half),
			    (vz[0]),   
			    (((cy + 0) * y_sp) - y_len_half)
			);

			/* Vertex #4. */
			if(set_texcoord != V3D_HF_SET_TEXCOORD_NEVER)
			    glTexCoord2f(
				((cx + 1) * x_sp / tex_width) - tex_offset_x,
				((cy + 1) * y_sp / tex_height) + tex_offset_y
			    );
			glVertex3f(
			    (((cx + 1) * x_sp) - x_len_half),
			    (vz[3]),
			    (((cy + 1) * y_sp) - y_len_half)
			);

			/* Vertex #2. */
			if(set_texcoord != V3D_HF_SET_TEXCOORD_NEVER)
			    glTexCoord2f(
				((cx + 1) * x_sp / tex_width) - tex_offset_x,
				((cy + 0) * y_sp / tex_height) + tex_offset_y
			    );
			glVertex3f(
			    (((cx + 1) * x_sp) - x_len_half),
			    (vz[1]),
			    (((cy + 0) * y_sp) - y_len_half)
			);


			/* Lower left triangle. */
			/* Set normal. */
			if(set_normal != V3D_HF_SET_NORMAL_NEVER)
			    V3DHFNormalByCrossGridVectors(
				x_sp, vz[3] - vz[2],
				y_sp, vz[0] - vz[2]
			    );

			/* Vertex #1. */
			if(set_texcoord != V3D_HF_SET_TEXCOORD_NEVER)
			    glTexCoord2f(
				((cx + 0) * x_sp / tex_width) - tex_offset_x,
				((cy + 0) * y_sp / tex_height) + tex_offset_y
			    );
			glVertex3f(
			    (((cx + 0) * x_sp) - x_len_half),
			    (vz[0]),
			    (((cy + 0) * y_sp) - y_len_half)
			);

			/* Vertex #3. */
			if(set_texcoord != V3D_HF_SET_TEXCOORD_NEVER)
			    glTexCoord2f(
				((cx + 0) * x_sp / tex_width) - tex_offset_x,
				((cy + 1) * y_sp / tex_height) + tex_offset_y
			    );
			glVertex3f(
			    (((cx + 0) * x_sp) - x_len_half),
			    (vz[2]),   
			    (((cy + 1) * y_sp) - y_len_half)
			);

			/* Vertex #4. */
			if(set_texcoord != V3D_HF_SET_TEXCOORD_NEVER)
			    glTexCoord2f(
				((cx + 1) * x_sp / tex_width) - tex_offset_x,
				((cy + 1) * y_sp / tex_height) + tex_offset_y
			    );
			glVertex3f(
			    (((cx + 1) * x_sp) - x_len_half),
			    (vz[3]),
			    (((cy + 1) * y_sp) - y_len_half)
			);
		    }
		    else
		    {
			/* Winding clockwise. */

			/* Upper right triangle. */
			/* Set normal. */
			if(set_normal != V3D_HF_SET_NORMAL_NEVER)
			    V3DHFNormalByCrossGridVectors(
				x_sp, vz[1] - vz[0],
				y_sp, vz[1] - vz[3]
			    );

			/* Vertex #1. */
			if(set_texcoord != V3D_HF_SET_TEXCOORD_NEVER)
			    glTexCoord2f(
				((cx + 0) * x_sp / tex_width) - tex_offset_x,
				((cy + 0) * y_sp / tex_height) + tex_offset_y
			    );
			glVertex3f(
			    (((cx + 0) * x_sp) - x_len_half),
			    (vz[0]),
			    (((cy + 0) * y_sp) - y_len_half)
			);

			/* Vertex #2. */
			if(set_texcoord != V3D_HF_SET_TEXCOORD_NEVER)
			    glTexCoord2f(
				((cx + 1) * x_sp / tex_width) - tex_offset_x,
				((cy + 0) * y_sp / tex_height) + tex_offset_y
			    );
			glVertex3f(
			    (((cx + 1) * x_sp) - x_len_half),
			    (vz[1]),
			    (((cy + 0) * y_sp) - y_len_half)
			);

			/* Vertex #4. */
			if(set_texcoord != V3D_HF_SET_TEXCOORD_NEVER)
			    glTexCoord2f(
				((cx + 1) * x_sp / tex_width) - tex_offset_x,
				((cy + 1) * y_sp / tex_height) + tex_offset_y
			    );
			glVertex3f(
			    (((cx + 1) * x_sp) - x_len_half),
			    (vz[3]),
			    (((cy + 1) * y_sp) - y_len_half)
			);


			/* Lower left triangle. */
			/* Set normal. */
			if(set_normal != V3D_HF_SET_NORMAL_NEVER)
			    V3DHFNormalByCrossGridVectors(
				x_sp, vz[3] - vz[2],
				y_sp, vz[0] - vz[2]
			    );

			/* Vertex #1. */
			if(set_texcoord != V3D_HF_SET_TEXCOORD_NEVER)
			    glTexCoord2f(
				((cx + 0) * x_sp / tex_width) - tex_offset_x,
				((cy + 0) * y_sp / tex_height) + tex_offset_y
			    );
			glVertex3f(
			    (((cx + 0) * x_sp) - x_len_half),
			    (vz[0]),   
			    (((cy + 0) * y_sp) - y_len_half)
			);

			/* Vertex #4. */
			if(set_texcoord != V3D_HF_SET_TEXCOORD_NEVER)
			    glTexCoord2f(
				((cx + 1) * x_sp / tex_width) - tex_offset_x,
				((cy + 1) * y_sp / tex_height) + tex_offset_y
			    );
			glVertex3f(
			    (((cx + 1) * x_sp) - x_len_half),
			    (vz[3]),
			    (((cy + 1) * y_sp) - y_len_half)
			);

			/* Vertex #3. */
			if(set_texcoord != V3D_HF_SET_TEXCOORD_NEVER)
			    glTexCoord2f(
				((cx + 0) * x_sp / tex_width) - tex_offset_x,
				((cy + 1) * y_sp / tex_height) + tex_offset_y
			    );
			glVertex3f(
			    (((cx + 0) * x_sp) - x_len_half),
			    (vz[2]),
			    (((cy + 1) * y_sp) - y_len_half)
			);

		    }	/* Begin recording GL drawing, check winding. */


		}
	    }

	    glEnd();
	}


	/* Deallocate loaded image data. */
	TgaDestroyData(&td);

	return(0);
}

/*
 *	Returns the z value of the heightfield point + hf_z that the
 *	given x and y position in world coordinates is over.
 *
 *	Can return 0.0 if x and y is out of the heightfield's bounds.
 */
double V3DHFGetHeightFromWorldPosition(
	double x, double y,     /* The world position. */
	double hf_x, double hf_y, double hf_z,  /* HF's world position. */
	double hf_heading,      /* HF's heading in radians. */
	double hf_len_x, double hf_len_y, double hf_len_z,
	int hf_widthp, int hf_heightp,  /* Size of heightfield in points. */
	double *hf_data           /* Heightfield's data. */
)
{
	double cos_theta, sin_theta;
	double dx, dy, drx, dry;
	double	hf_half_x_len = hf_len_x / 2,
		hf_half_y_len = hf_len_y / 2;
	double hf_grid_x_spacing, hf_grid_y_spacing;
	int hf_xi, hf_yi;


	/* HF has no span? */
	if((hf_len_x <= 0.0) || (hf_len_y <= 0.0))
	    return(0.0);

	/* HF data has no span? */
	if((hf_widthp <= 0) || (hf_heightp <= 0))
	    return(0.0);


	/* Calculate HF grid spacing. */
	hf_grid_x_spacing = hf_len_x / (double)hf_widthp;
	hf_grid_y_spacing = hf_len_y / (double)hf_heightp;
	if((hf_grid_x_spacing <= 0.0) || (hf_grid_y_spacing <= 0.0))
	    return(0.0);

	/* Get dx and dy, the delta distance of the position to
	 * the heightfield's center. Note that dy is flipped so that
	 * dx and dy will be `upper left' oriented.
	 */
	dx = x - hf_x;
	dy = hf_y - y;

	/* Now rotate dx and dy position based on heightfield's heading,
	 * remember that dy is top to bottom.
	 */
	cos_theta = cos(-hf_heading);
	sin_theta = sin(-hf_heading);
	drx = (dx * cos_theta) - (dy * sin_theta);
	dry = (dy * cos_theta) + (dx * sin_theta);


	/* Calculate point index, remember heightfield is centered at
	 * its origin, so need to add x and y point half index values
	 * to index.
	 */
	hf_xi = (int)((drx + hf_half_x_len) / hf_grid_x_spacing);
	hf_yi = (int)((dry + hf_half_y_len) / hf_grid_y_spacing);
/*
printf("\r %i %i | %.3f %.3f | %.3f %.3f | %.3f' %.3f %.3f",
 hf_xi, hf_yi, dx, dy, drx, dry, RADTODEG(hf_heading), cos_theta, 
sin_theta);
fflush(stdout);
*/
	/* Clip hf_xi and hf_yi heightfield index position, if it
	 * is outside of the heightfield's point data buffer then
	 * return 0.0.
	 */
	if((hf_xi < 0) || (hf_xi >= hf_widthp) ||
	   (hf_yi < 0) || (hf_yi >= hf_heightp)
	)
	    return(0.0);

	/* Check if heightfield pointer buffer is given. */
	if(hf_data == NULL)
	{
	    /* In heightfield but not data given, just return
	     * the heightfield's base z position.
	     */
	    return(hf_z);
	}
	else
	{
	    /* Got heightfield pointer buffer. */

	    /* The GL version of the heightfield has two triangles per
	     * grid, one on the `upper right' and one on the `lower left'
	     * (see V3DHFLoadFromFile()).
	     *
	     *                  v0 - v1
	     *                  | \   |
	     *                  |  \  |
	     *                  |   \ |
	     *                  v2 - v3
	     */

	    /* Calculate the target position `in the grid' as a coeff. */
	    double in_grid_x_c =
		((drx + hf_half_x_len) - (hf_xi * hf_grid_x_spacing)) /
		(((hf_xi + 1) * hf_grid_x_spacing) - (hf_xi * hf_grid_x_spacing));
	    double in_grid_y_c =
		((dry + hf_half_y_len) - (hf_yi * hf_grid_y_spacing)) /
		(((hf_yi + 1) * hf_grid_y_spacing) - (hf_yi * hf_grid_y_spacing));
	    double z_value[4];		/* ul, ur, ll, lr. */
	    double z_result;		/* Calculated height point in grid. */
/*
printf("\r %i %i %f %f ",
 hf_xi, hf_yi, in_grid_x_c, in_grid_y_c
); fflush(stdout);
 */
	    /* At this point we only need to fetch three values
	     * from the heightfield since we know we're in one
	     * or the other grid's triangle.
	     */
	    if(in_grid_x_c <= in_grid_y_c)
	    {
		/* In lower left triangle:
		 *
		 * Fetch z values for the lower left three vertices.
		 */

		/* Upper left z value. */
		z_value[0] = hf_data[
		    (hf_yi * hf_widthp) + hf_xi
		];

		/* Lower left z value. */
		if((hf_yi + 1) < hf_heightp)
		{
		    z_value[2] = hf_data[
			((hf_yi + 1) * hf_widthp) + hf_xi
		    ];
		}
		else
		{
		    /* No data for lower left, fall back to using upper left. */
		    z_value[2] = z_value[0];
		}

		/* Lower right z value. */
		if(((hf_xi + 1) < hf_widthp) &&
		   ((hf_yi + 1) < hf_heightp)
		)
		{
		    z_value[3] = hf_data[
			((hf_yi + 1) * hf_widthp) + (hf_xi + 1)
		    ];
		}
		/* No data for lower right, try lower left. */
		else if((hf_yi + 1) < hf_heightp)
		{ 
		    z_value[3] = z_value[2];
		}
		/* Try upper right. */
		else if((hf_xi + 1) < hf_widthp)
		{
		    /* Need to calculate upper right, we don't get it for free. */
		    z_value[3] = hf_data[
			(hf_yi * hf_widthp) + (hf_xi + 1)
		    ];
		}
		/* No UR or LL, use z_value[0]. */
		else
		{
		    z_value[3] = z_value[0];
		}

		/* Perform the height interpolation using the
		 * refactored equations. Basically what this does
		 * is only perform the 'differencing' portion
		 * of the interpolation on x and y axis'.
		 */
		{
		    double height_delta_x = z_value[3] - z_value[2];
		    double height_delta_y = z_value[0] - z_value[2];
		    z_result = z_value[2] +
			(height_delta_x * in_grid_x_c) +
			(height_delta_y * (1.0 - in_grid_y_c));
		}
	    }
	    else
	    {
		/* In upper right triangle:
		 *
		 * Fetch z values for the upper right three vertices.
		 */

		/* Upper left z value. */
		z_value[0] = hf_data[
		    (hf_yi * hf_widthp) + hf_xi
		];

		/* Upper right z value. */
		if((hf_xi + 1) < hf_widthp)
		{
		    z_value[1] = hf_data[
			(hf_yi * hf_widthp) + (hf_xi + 1)
		    ];
		}
		else
		{
		    /* No data for upper right, fall back to using upper left. */
		    z_value[1] = z_value[0];
		}

		/* Lower right z value. */
		if(((hf_xi + 1) < hf_widthp) &&
		   ((hf_yi + 1) < hf_heightp)
		)
		{
		    z_value[3] = hf_data[
			((hf_yi + 1) * hf_widthp) + (hf_xi + 1)
		    ];
		}
		/* No lower right data, try lower left? */
		else if((hf_yi + 1) < hf_heightp)
		{
		    /* Need to calculate lower left, we don't get it for free. */
		    z_value[3] = hf_data[
			((hf_yi + 1) * hf_widthp) + hf_xi
		    ];
		}
		/* Try upper right? */
		else if((hf_xi + 1) < hf_widthp)
		{
		    z_value[3] = z_value[1];
		}
		/* No UR or LL, use z_value[0]. */
		else
		{
		    z_value[3] = z_value[0];
		}

		/* Perform the height interpolation using the
		 * refactored equations. (Same idea as above, just
		 * different points involved.)
		 */
		{
		    double height_delta_x = z_value[0] - z_value[1];
		    double height_delta_y = z_value[3] - z_value[1];
		    z_result = z_value[1] +
			(height_delta_x * (1.0 - in_grid_x_c)) +
			(height_delta_y * in_grid_y_c);
		}
	    }

	    /* Calculation of the new interpirated height z_result
	     * is complete. Return the z_result + hf_z.
	     */
	    return(z_result + hf_z);
	}
}
