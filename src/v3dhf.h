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
			   V3D HeightField IO
 */

#ifndef V3DHF_H
#define V3DHF_H

#include <sys/types.h>
#include <GL/gl.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/*
 *	HF options structure:
 */
typedef struct {

#define V3D_HF_OPT_FLAG_WINDING		(1 << 0)
#define V3D_HF_OPT_FLAG_SET_NORMAL	(1 << 1)
#define V3D_HF_OPT_FLAG_SET_TEXCOORD	(1 << 2)
#define V3D_HF_OPT_FLAG_TEX_OFFSET	(1 << 3)
#define V3D_HF_OPT_FLAG_TEX_SIZE	(1 << 4)
	/* Indicates which members are set. */
	unsigned int flags;

#define V3D_HF_WIND_CW	0
#define V3D_HF_WIND_CCW	1
	/* V3D_HF_WIND_CW or V3D_HF_WIND_CCW. */
	int winding;

#define V3D_HF_SET_NORMAL_NEVER		0
#define V3D_HF_SET_NORMAL_AVERAGED	1
#define V3D_HF_SET_NORMAL_STREATCHED	2	/* For smoother shading. */
	/* One of V3D_HF_SET_NORMAL_*. */
	int set_normal;

#define V3D_HF_SET_TEXCOORD_NEVER	0
#define V3D_HF_SET_TEXCOORD_ALWAYS	1
	/* One of V3D_HF_SET_TEXCOORD_*. */
	int set_texcoord;

	/* Texture offset in meters, from upper left corner. */
	double tex_offset_x, tex_offset_y;

	/* Texture size in meters. */
	double tex_width, tex_height;

} v3d_hf_options_struct;


extern int V3DHFLoadFromFile(
	const char *path,	/* Heightfield image file. */
	double x_len, double y_len, double z_len,	/* Size in meters. */
	int *width_rtn, int *height_rtn,	/* Num grids (pixels). */
	double *x_spacing_rtn,	/* Each grid (pixel) is this many meters. */
	double *y_spacing_rtn,
	double **data_rtn,      /* Dynamically allocated z points, each of
				 * type double (can be NULL).
				 */
	GLuint gl_list,		/* GL list (can be NULL). */
	v3d_hf_options_struct *hfopt
);

extern double V3DHFGetHeightFromWorldPosition(
	double x, double y,     /* The world position. */
	double hf_x, double hf_y, double hf_z,  /* HF's world position. */
	double hf_heading,      /* HF's heading in radians. */
	double hf_len_x, double hf_len_y, double hf_len_z,
	int hf_widthp, int hf_heightp,  /* Size of heightfield in points. */
	double *hf_data           /* Heightfield's data. */
);


#ifdef __cplusplus
}  
#endif /* __cplusplus */

#endif	/* V3DHF_H */
