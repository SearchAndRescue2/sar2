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
				 V3D Texture
 */

#ifndef V3DTEX_H
#define V3DTEX_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/*
 *	Texture Formats:
 */
typedef enum {
	V3D_TEX_FORMAT_RGB,
	V3D_TEX_FORMAT_RGBA,
	V3D_TEX_FORMAT_LUMINANCE,
	V3D_TEX_FORMAT_LUMINANCE_ALPHA
} v3d_tex_format;


/*
 *	Texture Reference:
 */
typedef struct {

	char		*name;		/* Reference name (any arbitary name) */
	char		*filename;	/* Full path file name */

	float		priority;	/* Texture memory residency priority
					 * (0.0. to 1.0, 1.0 is highest) */

	void		**data;		/* Array of GLint id's for each texture */
	int		total_frames;	/* Number of textures (size of member data */

	int		width,		/* Size of each frame in pixels */
			height;

	int		dimensions;	/* 1, 2, or 3 */

} v3d_texture_ref_struct;

extern void V3DTextureSelectFrame(v3d_texture_ref_struct *t, int frame_num);
extern void V3DTextureSelect(v3d_texture_ref_struct *t);
extern v3d_texture_ref_struct *V3DTextureLoadFromFile2D(
	const char *path,	/* Filename containing texture data */
	const char *name,	/* Name of texture for referancing */
	v3d_tex_format dest_fmt,
	void *client_data,
	int (*progress_cb)(void *, int, int)
);
extern v3d_texture_ref_struct *V3DTextureLoadFromFile2DPreempt(
	const char *path,	/* Filename containing texture data */
	const char *name,	/* Name of texture for referancing */
	v3d_tex_format dest_fmt
);
extern v3d_texture_ref_struct *V3DTextureLoadFromData1D(
	const void *data,	/* Texture data */
	const char *name,	/* Name of texture for referancing */
	int width,
	int bytes_per_pixel,	/* Bytes per pixel of data */
	v3d_tex_format dest_fmt,
	void *client_data,
	int (*progress_cb)(void *, int, int)
);
extern v3d_texture_ref_struct *V3DTextureLoadFromData2D(
	const void *data,	/* Texture data */
	const char *name,	/* Name of texture for referancing */
	int width, int height,
	int bytes_per_pixel,	/* Bytes per pixel of data */
	v3d_tex_format dest_fmt,
	void *client_data,
	int (*progress_cb)(void *, int, int)
);
extern void V3DTexturePriority(v3d_texture_ref_struct *t, float priority);
extern void V3DTextureDestroy(v3d_texture_ref_struct *t);


#ifdef __cplusplus
}  
#endif /* __cplusplus */

#endif	/* V3DTEX_H */
