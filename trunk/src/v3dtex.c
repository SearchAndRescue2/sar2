#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "../include/os.h"

#ifdef __MSW__
# include <windows.h>
#endif

#include <GL/gl.h>
#include <GL/glu.h>

#include "../include/string.h"
#include "../include/tga.h"

#include "v3dtex.h"

#ifdef MEMWATCH
# include "memwatch.h"
#endif


static int V3DTextureIsPowerOf2(int num);
static void V3DTextureRescaleImage(
	const u_int8_t *in_pixels,
	int bytes_per_pixel, GLenum gl_image_format,
	int in_width, int in_height,
	u_int8_t **out_pixels,
	int *out_width, int *out_height
);
void V3DTextureSelectFrame(v3d_texture_ref_struct *t, int frame_num);
void V3DTextureSelect(v3d_texture_ref_struct *t);
v3d_texture_ref_struct *V3DTextureLoadFromFile2D(
	const char *path, const char *name, v3d_tex_format dest_fmt,
	void *client_data, int (*progress_cb)(void *, int, int)
);
v3d_texture_ref_struct *V3DTextureLoadFromFile2DPreempt(
	const char *path, const char *name, v3d_tex_format dest_fmt
);
v3d_texture_ref_struct *V3DTextureLoadFromData1D(
	const void *data, const char *name,
	int width,
	int bytes_per_pixel, v3d_tex_format dest_fmt,
	void *client_data, int (*progress_cb)(void *, int, int)
);
v3d_texture_ref_struct *V3DTextureLoadFromData2D(
	const void *data, const char *name,
	int width, int height,
	int bytes_per_pixel, v3d_tex_format dest_fmt,
	void *client_data, int (*progress_cb)(void *, int, int)
);
void V3DTexturePriority(v3d_texture_ref_struct *t, float priority);
void V3DTextureDestroy(v3d_texture_ref_struct *t);


#define ATOI(s)         (((s) != NULL) ? atoi(s) : 0)
#define ATOL(s)         (((s) != NULL) ? atol(s) : 0)
#define ATOF(s)         (((s) != NULL) ? atof(s) : 0.0f)
#define STRDUP(s)       (((s) != NULL) ? strdup(s) : NULL)

#define MAX(a,b)        (((a) > (b)) ? (a) : (b))
#define MIN(a,b)        (((a) < (b)) ? (a) : (b))
#define CLIP(a,l,h)     (MIN(MAX((a),(l)),(h)))

#define PACK8TO32(a,r,g,b)	(u_int32_t)(		\
 ((a) << 24) | ((r) << 16) | ((g) << 8) | (b)		\
)

#define TEXTUREIO_TEX_OPTIONS_1D			\
{							\
 glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_REPEAT); \
 glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_T, GL_REPEAT); \
  glTexParameteri(					\
  GL_TEXTURE_1D,					\
  GL_TEXTURE_MAG_FILTER,				\
  GL_NEAREST						\
 );							\
 glTexParameteri(					\
  GL_TEXTURE_1D,					\
  GL_TEXTURE_MIN_FILTER,				\
  GL_NEAREST						\
 );							\
}

#define TEXTUREIO_TEX_OPTIONS_2D			\
{							\
 glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); \
 glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT); \
  glTexParameteri(					\
  GL_TEXTURE_2D,					\
  GL_TEXTURE_MAG_FILTER,				\
  GL_NEAREST						\
 );							\
 glTexParameteri(					\
  GL_TEXTURE_2D,					\
  GL_TEXTURE_MIN_FILTER,				\
  GL_NEAREST						\
 );							\
}



/*
 *	Checks if the the number is a power of 2.
 *
 *	Returns true if it is.
 */
static int V3DTextureIsPowerOf2(int num)
{
	int i;

	for(i = 1; i < num; i = (i << 1))
	{
	    if(num & i)
		return(0);
	}

	return(1);
}

/*
 *	Rescales the texture image data in_pixels as needed to conform
 *	to the required OpenGL texture size.
 *
 *	Returns a pointer that can be:
 *	1) The same pointer as in_pixels, implying no rescaling was
 *	   performed.
 *	2) A pointer to a newly allocated image which must be free'ed
 *	   by the calling function (pointer value will be different from
 *	   in_pixels).
 *	3) NULL on error.
 *
 *	Note that in_pixels must not be NULL and will not be modified
 *	by this function, but this function may return in_pixels as
 *	the pointer, in which case it should be treated as in_pixels
 *	would be treated by the calling function.
 */
static void V3DTextureRescaleImage(
	const u_int8_t *in_pixels,
	int bytes_per_pixel, GLenum gl_image_format,
	int in_width, int in_height,
	u_int8_t **out_pixels,
	int *out_width, int *out_height
)
{
	int i, m, status, longer_in_dim;
	int lout_dim;
	u_int8_t *lout_pixels = NULL;

	if(out_pixels != NULL)
	    *out_pixels = NULL;
	if(out_width != NULL)
	    *out_width = 0;
	if(out_height != NULL)
	    *out_height = 0;

	if((in_pixels == NULL) || (bytes_per_pixel <= 0))
	    return;

	/* Is input image size not conforming to OpenGL standards
	 * for texture?
	 */
	if(V3DTextureIsPowerOf2(in_width))
	{
	    /* Divides clearly for multiple frames? */
	    if((in_height % in_width) == 0)
	    {
		/* Everything ok, return out_pixels as in_pixels */
		if(out_pixels != NULL)
		    *out_pixels = (u_int8_t *)in_pixels;
		if(out_width != NULL)
		    *out_width = in_width;
		if(out_height != NULL)
		    *out_height = in_height;
		return;
	    }
	}

	/* Get longer input dimension */
	longer_in_dim = MAX(in_width, in_height);

	/* Find an equal to or greater than 2^n dimension to be
	 * used
	 */
	for(i = 0, m = 16; i < m; i++)
	{
	    lout_dim = (int)(1 << i);
	    if(lout_dim > longer_in_dim)
		break;
	}
	if(i >= m)
	{
	    /* Could not find a 2^n dimension big enough for the longer
	     * input dimension, so give up (returns will be NULL and 0)
	     */
	    return;
	}

	/* Got new dimension to be used as lout_dim */


	/* Allocate new out image buffer of the size lout_dim * lout_dim
	 * pixels * bytes_per_pixel
	 */
	lout_pixels = (u_int8_t *)malloc(
	    lout_dim * lout_dim * bytes_per_pixel * sizeof(u_int8_t)
	);
	if(lout_pixels == NULL)
	    return;

	status = gluScaleImage(
	    gl_image_format,
	    (GLsizei)in_width, (GLsizei)in_height,
	    GL_UNSIGNED_BYTE,
	    (const void *)in_pixels,
	    (GLsizei)lout_dim, (GLsizei)lout_dim,
	    GL_UNSIGNED_BYTE,
	    (GLvoid *)lout_pixels
	);
	if(status != 0)
	{
	    fprintf(
		stderr,
		"gluScaleImage(): Error: %s\n",
		(const char *)gluErrorString(status)
	    );
	}

	/* Set new return dimensions */
	if(out_pixels != NULL)
	    *out_pixels = lout_pixels;
	if(out_width != NULL)
	    *out_width = lout_dim;
	if(out_height != NULL)
	    *out_height = lout_dim;
}


/*
 *	Sets pointed to texture reference's specified texture frame
 *	number into current context.
 *
 *      If t is NULL or the frame_num is -1, then the texture is
 *	unselected.
 */
void V3DTextureSelectFrame(v3d_texture_ref_struct *t, int frame_num)
{
	GLuint gl_texture_id;


	/* Unselect texture? */
	if((t == NULL) || (frame_num < 0))
	{
	    /* Unselect texture */
	    glBindTexture(GL_TEXTURE_1D, 0);
	    glBindTexture(GL_TEXTURE_2D, 0);
#ifdef GL_TEXTURE_3D
	    glBindTexture(GL_TEXTURE_3D, 0);
#endif
	    return;
	}

	/* Check if frame number is valid */
	if(frame_num >= t->total_frames)
	    return;

	/* Get texture id of specified frame number */
	gl_texture_id = (GLuint)(t->data[frame_num]);

	/* Select the specified texture of the specified frame,
	 * note that gl_texture_id may be 0 in which case
	 * the texture will be unselected
	 */
	switch(t->dimensions)
	{
	  case 3:
#ifdef GL_TEXTURE_3D
	    glBindTexture(GL_TEXTURE_3D, gl_texture_id);
#endif
	    break;

	  case 2:
#ifdef GL_TEXTURE_2D
	    glBindTexture(GL_TEXTURE_2D, gl_texture_id);
#endif
	    break;

	  case 1:
#ifdef GL_TEXTURE_1D
	    glBindTexture(GL_TEXTURE_1D, gl_texture_id);
#endif
	    break;
	}
}

/*
 *	Sets pointed to texture reference's texture frame number 0
 *	into current context.
 *
 *	If t is NULL then the texture is unselected.
 */
void V3DTextureSelect(v3d_texture_ref_struct *t)
{
	V3DTextureSelectFrame(t, 0);
}

/*
 *	Loads a 2D texture from file, returning a dynamically allocated
 *	texture reference structure.
 *
 *      If the height is greater than the width, then it will be treated
 *      as an array of vertical frames where the number of frames is
 *      height / width (this only works when height and width are both
 *	a value of 2^n).
 *
 *	If the progress_cb function is not NULL and returns 0, then
 *	the loading of each frame will stop and the texture will return
 *	the pointer to the texture structure but maybe with fewer frames
 *	loaded.
 */
v3d_texture_ref_struct *V3DTextureLoadFromFile2D(
	const char *path,	/* Filename containing texture data */
	const char *name,	/* Name of texture for referancing */
	v3d_tex_format dest_fmt,
	void *client_data,
	int (*progress_cb)(void *, int, int)
)
{
	int i, status;
#ifndef __MSW__
	struct stat stat_buf;
#endif
	tga_data_struct td;
	v3d_texture_ref_struct *t;
	GLuint gl_texture_id;
	u_int8_t *cur_data_ptr, *loaded_data_ptr;
	int cur_width, cur_height, loaded_width, loaded_height;
	u_int8_t *ptr8, *ptrend8;
	u_int32_t *ptr32, *ptrend32;
	u_int8_t a, r, g, b;


	if(path == NULL)
	    return(NULL);

#ifndef __MSW__
	if(stat(path, &stat_buf))
	{
	    fprintf(stderr,
		"%s: No such file.\n",
		path
	    );
	    return(NULL);
	}
	if(!S_ISREG(stat_buf.st_mode))
	{
	    fprintf(stderr,
		"%s: Not a file.\n",
		path
	    );
	    return(NULL);
	}
#endif
	/* Load data from file, read as 32 bits */
	status = TgaReadFromFile(
	    path,
	    &td,
	    32		/* Read to 32 bits */
	);
	if(status != TgaSuccess)
	{
	    TgaDestroyData(&td);
	    return(NULL);
	}

	/* Check if size is big enough */
	if(td.width < 2)
	    fprintf(stderr,
 "%s: Warning: Image size is too small in width.\n",
		path
	    );
	if(td.height < 2)
	    fprintf(stderr,
 "%s: Warning: Image size is too small in height.\n",
		path
	    );

	/* Pointer to loaded data valid? */
	cur_data_ptr = loaded_data_ptr = td.data;
	if(cur_data_ptr == NULL)
	{
	    TgaDestroyData(&td);
	    return(NULL);
	}
	cur_width = loaded_width = td.width;
	cur_height = loaded_height = td.height;

	/* Allocate a texture reference structure */
	t = (v3d_texture_ref_struct *)calloc(1, sizeof(v3d_texture_ref_struct));
	if(t == NULL)
	{
	    TgaDestroyData(&td);
	    return(NULL);
	}


	/* Switch destination data format type, need to convert the
	 * loaded data
	 */
	switch(dest_fmt)
	{
	  case V3D_TEX_FORMAT_RGB:
	    /* Convert source RGBA data to destination RGB data */
	    ptr32 = (u_int32_t *)cur_data_ptr;
	    ptrend32 = ptr32 + (cur_width * cur_height);
	    ptr8 = (u_int8_t *)cur_data_ptr;
	    ptrend8 = ptr8 + (cur_width * cur_height * 3);
	    while(ptr32 < ptrend32)
	    {
		r = (u_int8_t)(((*ptr32) & 0x00ff0000) >> 16);
		g = (u_int8_t)(((*ptr32) & 0x0000ff00) >> 8);
		b = (u_int8_t)(((*ptr32) & 0x000000ff) >> 0);

		*ptr8++ = r;
		*ptr8++ = g;
		*ptr8++ = b;
		ptr32++;
	    }
	    /* Resize the image as needed (cur_data_ptr may != 
	     * loaded_data_ptr)
	     */
	    V3DTextureRescaleImage(
		loaded_data_ptr,
		3, GL_RGB,
		loaded_width, loaded_height,
		&cur_data_ptr,
		&cur_width, &cur_height
	    );
	    break;

	  case V3D_TEX_FORMAT_RGBA:
	    /* Convert source RGBA data to destination RGBA data */
	    ptr32 = (u_int32_t *)cur_data_ptr;
	    ptrend32 = ptr32 + (cur_width * cur_height);
	    while(ptr32 < ptrend32)
	    {
	        a = (u_int8_t)(((*ptr32) & 0xff000000) >> 24);
	        r = (u_int8_t)(((*ptr32) & 0x00ff0000) >> 16);
		g = (u_int8_t)(((*ptr32) & 0x0000ff00) >> 8);
		b = (u_int8_t)(((*ptr32) & 0x000000ff) >> 0);

		/* Check original data bits per pixel */
		switch(td.bits_per_pixel)
		{
		  case 32:
		    *ptr32++ = PACK8TO32(a, b, g, r);
		    break;

		  default:	/* Assume 24 bit or less */
		    if(*ptr32 == 0x00000000)
			*ptr32++ = PACK8TO32(0x00, b, g, r);
		    else
			*ptr32++ = PACK8TO32(0xff, b, g, r);
		    break;
		}
	    }
	    /* Resize the image as needed (cur_data_ptr may !=
	     * loaded_data_ptr)
	     */
	    V3DTextureRescaleImage(
		loaded_data_ptr,
		4, GL_RGBA,
		loaded_width, loaded_height,
		&cur_data_ptr,
		&cur_width, &cur_height
	    );
	    break;

	  case V3D_TEX_FORMAT_LUMINANCE:
	    /* Convert source RGBA data to destination LUMINANCE data */
	    ptr32 = (u_int32_t *)cur_data_ptr;
	    ptrend32 = ptr32 + (cur_width * cur_height);
	    ptr8 = (u_int8_t *)cur_data_ptr;
	    ptrend8 = ptr8 + (cur_width * cur_height);
	    while(ptr32 < ptrend32)
	    {
		r = (u_int8_t)(((*ptr32) & 0x00ff0000) >> 16);
		g = (u_int8_t)(((*ptr32) & 0x0000ff00) >> 8);
		b = (u_int8_t)(((*ptr32) & 0x000000ff) >> 0);

		*ptr8++ = (u_int8_t)(((int)r + (int)g + (int)b) / 3);
		ptr32++;
	    }
	    /* Resize the image as needed (cur_data_ptr may !=
	     * loaded_data_ptr)
	     */
	    V3DTextureRescaleImage(
		loaded_data_ptr,
		1, GL_LUMINANCE,
		loaded_width, loaded_height,
		&cur_data_ptr,
		&cur_width, &cur_height
	    );
	    break;

	  case V3D_TEX_FORMAT_LUMINANCE_ALPHA:
	    ptr32 = (u_int32_t *)cur_data_ptr;
	    ptrend32 = ptr32 + (cur_width * cur_height);
	    ptr8 = (u_int8_t *)cur_data_ptr;
	    ptrend8 = ptr8 + (cur_width * cur_height);
	    while(ptr32 < ptrend32)
	    {
		a = (u_int8_t)(((*ptr32) & 0xff000000) >> 24);
		r = (u_int8_t)(((*ptr32) & 0x00ff0000) >> 16);
		g = (u_int8_t)(((*ptr32) & 0x0000ff00) >> 8);
		b = (u_int8_t)(((*ptr32) & 0x000000ff) >> 0);

		*ptr8++ = (u_int8_t)(((int)r + (int)g + (int)b) / 3);
		*ptr8++ = a;
		ptr32++;
	    }
	    /* Resize the image as needed (cur_data_ptr may !=
	     * loaded_data_ptr) 
	     */
	    V3DTextureRescaleImage(
		loaded_data_ptr,
		2, GL_LUMINANCE_ALPHA,
		loaded_width, loaded_height,
		&cur_data_ptr, 
		&cur_width, &cur_height
	    );
	    break;

	}
	/* Is cur_data_ptr still valid from above processing? */
	if((cur_data_ptr == NULL) ||
	   (cur_width <= 0) ||
	   (cur_height <= 0)
	)
	{
	    free(t);
	    TgaDestroyData(&td);
	    if(cur_data_ptr != loaded_data_ptr)
		free(cur_data_ptr);
	    return(NULL);
	}


	/* Create each texture frame */
	t->total_frames = (int)cur_height / (int)cur_width;
	if(t->total_frames < 1)
	    t->total_frames = 1;
	t->data = (void **)calloc(
	    t->total_frames,
	    sizeof(void *)
	);
	if(t->data == NULL)
	{
	    free(t);
	    TgaDestroyData(&td);
	    if(cur_data_ptr != loaded_data_ptr)
		free(cur_data_ptr);
	    return(NULL);
	}
	for(i = 0; i < t->total_frames; i++)
	{
	    /* Notify progress callback and see if we should continue
	     * loading
	     */
	    if(progress_cb != NULL)
	    {
		if(!progress_cb(client_data, i, t->total_frames))
		    break;
	    }

	    /* Generate a new texture ID */
	    glGenTextures(1, &gl_texture_id);
	    if(gl_texture_id == 0)
	    {
		fprintf(stderr,
		    "%s: Error generating texture\n",
		    path
		);
		continue;
	    }

	    /* Actually generate texture and make texture active
	     * (selected)
	     */
	    glBindTexture(GL_TEXTURE_2D, gl_texture_id);

	    /* Set parameters of selected texture */
	    TEXTUREIO_TEX_OPTIONS_2D

	    /* Load data we loaded from file into texture by format */
	    switch(dest_fmt)
	    {
	      case V3D_TEX_FORMAT_RGB:
		ptr8 = (u_int8_t *)cur_data_ptr;
		glTexImage2D(
		    GL_TEXTURE_2D,      /* GL_TEXTURE_2D or GL_PROXY_TEXTURE_2D */
		    0,                  /* Level */
		    GL_RGB,		/* Internal format */
		    cur_width,		/* Width */
		    cur_width,		/* Height (same as width) */
		    0,                  /* Border */
		    GL_RGB,		/* Image data format */
		    GL_UNSIGNED_BYTE,   /* Image data type */
		    /* Pointer to image data */
		    (GLubyte *)(ptr8 + (cur_width * cur_width * i * 3))
		);
		break;

	      case V3D_TEX_FORMAT_RGBA:
		ptr32 = (u_int32_t *)cur_data_ptr;
		glTexImage2D(
		    GL_TEXTURE_2D,	/* GL_TEXTURE_2D or GL_PROXY_TEXTURE_2D */
		    0,			/* Level */
		    GL_RGBA,		/* Internal format */
		    cur_width, 		/* Width */
		    cur_width,		/* Height (same as width) */
		    0,			/* Border */
		    GL_RGBA,		/* Image data format */
		    GL_UNSIGNED_BYTE,	/* Image data type */
		    /* Pointer to image data */
		    (GLubyte *)(ptr32 + (cur_width * cur_width * i))
	        );
		break;

	      case V3D_TEX_FORMAT_LUMINANCE:
		ptr8 = (u_int8_t *)cur_data_ptr;
		glTexImage2D(
		    GL_TEXTURE_2D,      /* GL_TEXTURE_2D or GL_PROXY_TEXTURE_2D */
		    0,                  /* Level */
		    GL_LUMINANCE8,	/* Internal format */
		    cur_width,		/* Width */
		    cur_width,		/* Height (same as width) */
		    0,                  /* Border */
		    GL_LUMINANCE,	/* Image data format */
		    GL_UNSIGNED_BYTE,	/* Image data type */
		    /* Pointer to image data */
		    (GLubyte *)(ptr8 + (cur_width * cur_width * i))
		);
		break;

	      case V3D_TEX_FORMAT_LUMINANCE_ALPHA:
		ptr8 = (u_int8_t *)cur_data_ptr;
		glTexImage2D(
		    GL_TEXTURE_2D,	/* GL_TEXTURE_2D or GL_PROXY_TEXTURE_2D */
		    0,			/* Level */
		    GL_LUMINANCE8_ALPHA8,	/* Internal format */
		    cur_width,		/* Width */
		    cur_width,		/* Height (same as width) */
		    0,			/* Border */
		    GL_LUMINANCE_ALPHA,	/* Image data format */
		    GL_UNSIGNED_BYTE,	/* Image data type */
		    /* Pointer to image data */
		    (GLubyte *)(ptr8 + (cur_width * cur_width * i * 2))
		);
		break;
	    }

	    /* Record this gl texture id as data */
	    t->data[i] = (void *)gl_texture_id;
	}

	/* Record other values */
	t->name = STRDUP(name);
	t->filename = STRDUP(path);
	t->width = cur_width;
	t->height = cur_width;	/* Height of tile is width */
	t->dimensions = 2;

	/* Delete the loed data since it is no longer needed, and
	 * also deallocate cur_data_ptr if it does not match
	 * loaded_data_ptr
	 */
	TgaDestroyData(&td);
	if(cur_data_ptr != loaded_data_ptr)
	    free(cur_data_ptr);

	if(progress_cb != NULL)
	    progress_cb(client_data, t->total_frames, t->total_frames);

	return(t);
}

/*
 *	Similar to V3DTextureLoadFromFile2D(), except faster on UNIXes.
 *
 *	This uses the TgaReadFromFileFastRGBA() instead of
 *	TgaReadFromFile().
 */
v3d_texture_ref_struct *V3DTextureLoadFromFile2DPreempt(
	const char *path, const char *name, v3d_tex_format dest_fmt
)
{
	int i;
#ifndef __MSW__
	struct stat stat_buf;
#endif
	v3d_texture_ref_struct *t;
	GLuint gl_texture_id;
	u_int8_t *cur_data_ptr, *loaded_data_ptr;
	int cur_width, cur_height, loaded_width, loaded_height;

	u_int8_t *data;
	int data_width, data_height;

	u_int8_t *ptr8, *ptrend8;
	u_int32_t *ptr32, *ptrend32;
	u_int8_t a, r, g, b;


	if(path == NULL)
	    return(NULL);

#ifndef __MSW__
	if(stat(path, &stat_buf))
	{
	    fprintf(stderr,
		"%s: No such file.\n",
		path
	    );
	    return(NULL);
	}
	if(!S_ISREG(stat_buf.st_mode))
	{
	    fprintf(stderr,
		"%s: Not a file.\n",
		path
	    );
	    return(NULL);
	}
#endif
	/* Load data from file and load to RGBA format */
	data = TgaReadFromFileFastRGBA(
	    path, &data_width, &data_height, 0x00000000
	);
	if(data == NULL)
	    return(NULL);

	/* Check if size is big enough */
	if(data_width < 2)
	    fprintf(stderr,
 "%s: Warning: Image size is too small in width.\n",
		path
	    );
	if(data_height < 2)
	    fprintf(stderr,
 "%s: Warning: Image size is too small in height.\n",
		path
	    );

	/* Get pointer to beginning of loaded data */
	cur_data_ptr = loaded_data_ptr = data;

	cur_width = loaded_width = data_width;
	cur_height = loaded_height = data_height;

	/* Allocate a texture reference structure */
	t = (v3d_texture_ref_struct *)calloc(1, sizeof(v3d_texture_ref_struct));
	if(t == NULL)
	{
	    free(data);
	    return(NULL);
	}


	/* Switch destination data format type, need to convert the
	 * loaded data
	 */
	switch(dest_fmt)
	{
	  case V3D_TEX_FORMAT_RGB:
	    /* Convert source RGBA data to destination RGB data */
	    ptr32 = (u_int32_t *)cur_data_ptr;
	    ptrend32 = ptr32 + (cur_width * cur_height);
	    ptr8 = (u_int8_t *)cur_data_ptr;
	    ptrend8 = ptr8 + (cur_width * cur_height * 3);
	    while(ptr32 < ptrend32)
	    {
		b = (u_int8_t)(((*ptr32) & 0x00ff0000) >> 16);
		g = (u_int8_t)(((*ptr32) & 0x0000ff00) >> 8);
		r = (u_int8_t)(((*ptr32) & 0x000000ff) >> 0);

		*ptr8++ = r;
		*ptr8++ = g;
		*ptr8++ = b;

		ptr32++;
	    }
	    /* Resize the image as needed (cur_data_ptr may !=
	     * loaded_data_ptr).
	     */
	    V3DTextureRescaleImage(
		loaded_data_ptr,
		3, GL_RGB,
		loaded_width, loaded_height,
		&cur_data_ptr,
		&cur_width, &cur_height
	    );
	    break;

	  case V3D_TEX_FORMAT_RGBA:
	    /* Source data is already in RGBA format */

	    /* Resize the image as needed (cur_data_ptr may !=
	     * loaded_data_ptr).
	     */
	    V3DTextureRescaleImage(
		loaded_data_ptr,
		4, GL_RGBA,
		loaded_width, loaded_height,
		&cur_data_ptr,
		&cur_width, &cur_height
	    );
	    break;

	  case V3D_TEX_FORMAT_LUMINANCE:
	    /* Convert source RGBA data to destination LUMINANCE data */
	    ptr32 = (u_int32_t *)cur_data_ptr;
	    ptrend32 = ptr32 + (cur_width * cur_height);
	    ptr8 = (u_int8_t *)cur_data_ptr;
	    ptrend8 = ptr8 + (cur_width * cur_height);
	    while(ptr32 < ptrend32)
	    {
		b = (u_int8_t)(((*ptr32) & 0x00ff0000) >> 16);
		g = (u_int8_t)(((*ptr32) & 0x0000ff00) >> 8);
		r = (u_int8_t)(((*ptr32) & 0x000000ff) >> 0);

		*ptr8++ = (u_int8_t)(((int)r + (int)g + (int)b) / 3);
		ptr32++;
	    }
	    /* Resize the image as needed (cur_data_ptr may !=
	     * loaded_data_ptr)
	     */
	    V3DTextureRescaleImage(
		loaded_data_ptr,
		1, GL_LUMINANCE,
		loaded_width, loaded_height,
		&cur_data_ptr,
		&cur_width, &cur_height
	    );
	    break;

	  case V3D_TEX_FORMAT_LUMINANCE_ALPHA:
	    /* Convert source RGBA data to destination LUMINANCE data */
	    ptr32 = (u_int32_t *)cur_data_ptr;
	    ptrend32 = ptr32 + (cur_width * cur_height);
	    ptr8 = (u_int8_t *)cur_data_ptr;
	    ptrend8 = ptr8 + (cur_width * cur_height);
	    while(ptr32 < ptrend32)
	    {
		a = (u_int8_t)(((*ptr32) & 0xff000000) >> 24);
		b = (u_int8_t)(((*ptr32) & 0x00ff0000) >> 16);
		g = (u_int8_t)(((*ptr32) & 0x0000ff00) >> 8);
		r = (u_int8_t)(((*ptr32) & 0x000000ff) >> 0);

		*ptr8++ = (u_int8_t)(((int)r + (int)g + (int)b) / 3);
		*ptr8++ = a;
		ptr32++;
	    }
	    /* Resize the image as needed (cur_data_ptr may !=
	     * loaded_data_ptr)
	     */
	    V3DTextureRescaleImage(
		loaded_data_ptr,
		2, GL_LUMINANCE_ALPHA,
		loaded_width, loaded_height,
		&cur_data_ptr,
		&cur_width, &cur_height
	    );
	    break;

	}
	/* Is cur_data_ptr still valid from above processing? */
	if((cur_data_ptr == NULL) ||
	   (cur_width <= 0) ||
	   (cur_height <= 0)
	)
	{
	    free(data);
	    if(cur_data_ptr != loaded_data_ptr)
		free(cur_data_ptr);
	    return(NULL);
	}

	/* Create each texture frame */
	t->total_frames = (int)cur_height / (int)cur_width;
	if(t->total_frames < 1)
	    t->total_frames = 1;
	t->data = (void **)calloc(
	    t->total_frames,
	    sizeof(void *)
	);
	if(t->data == NULL)
	{
	    free(data);
	    if(cur_data_ptr != loaded_data_ptr)
		free(cur_data_ptr);
	    return(NULL);
	}
	for(i = 0; i < t->total_frames; i++)
	{
	    /* Generate a new texture ID */
	    glGenTextures(1, &gl_texture_id);
	    if(gl_texture_id == 0)
	    {
		fprintf(stderr,
		    "%s: Error generating texture\n",
		    path
		);
		continue;
	    }

	    /* Actually generate texture and make texture active
	     * (selected).
	     */
	    glBindTexture(GL_TEXTURE_2D, gl_texture_id);

	    /* Set parameters of selected texture */
	    TEXTUREIO_TEX_OPTIONS_2D

	    /* Load data we loaded from file into texture by format */
	    switch(dest_fmt)
	    {
	      case V3D_TEX_FORMAT_RGB:
		ptr8 = (u_int8_t *)cur_data_ptr;
		glTexImage2D(
		    GL_TEXTURE_2D,      /* GL_TEXTURE_2D or GL_PROXY_TEXTURE_2D */
		    0,                  /* Level */
		    GL_RGB,             /* Internal format */
		    cur_width,          /* Width */
		    cur_width,          /* Height (same as width) */
		    0,                  /* Border */
		    GL_RGB,             /* Image data format */
		    GL_UNSIGNED_BYTE,   /* Image data type */
		    /* Pointer to image data */
		    (GLubyte *)(ptr8 + (cur_width * cur_width * i * 3))
		);
		break;

	      case V3D_TEX_FORMAT_RGBA:
		ptr32 = (u_int32_t *)cur_data_ptr;
		glTexImage2D(
		    GL_TEXTURE_2D,      /* GL_TEXTURE_2D or GL_PROXY_TEXTURE_2D */
		    0,                  /* Level */
		    GL_RGBA,            /* Internal format */
		    cur_width,          /* Width */
		    cur_width,          /* Height (same as width) */
		    0,                  /* Border */
		    GL_RGBA,            /* Image data format */
		    GL_UNSIGNED_BYTE,   /* Image data type */
		    /* Pointer to image data */
		    (GLubyte *)(ptr32 + (cur_width * cur_width * i))
		);
		break;

	      case V3D_TEX_FORMAT_LUMINANCE:
		ptr8 = (u_int8_t *)cur_data_ptr;
		glTexImage2D(
		    GL_TEXTURE_2D,      /* GL_TEXTURE_2D or GL_PROXY_TEXTURE_2D */
		    0,                  /* Level */
		    GL_LUMINANCE8,      /* Internal format */
		    cur_width,          /* Width */
		    cur_width,          /* Height (same as width) */
		    0,                  /* Border */
		    GL_LUMINANCE,       /* Image data format */
		    GL_UNSIGNED_BYTE,   /* Image data type */
		    /* Pointer to image data */
		    (GLubyte *)(ptr8 + (cur_width * cur_width * i))
		);
		break;

	      case V3D_TEX_FORMAT_LUMINANCE_ALPHA:
		ptr8 = (u_int8_t *)cur_data_ptr;
		glTexImage2D(
		    GL_TEXTURE_2D,	/* GL_TEXTURE_2D or GL_PROXY_TEXTURE_2D */
		    0,			/* Level */
		    GL_LUMINANCE8_ALPHA8,	/* Internal format */
		    cur_width,		/* Width */
		    cur_width,		/* Height (same as width) */
		    0,			/* Border */
		    GL_LUMINANCE_ALPHA,	/* Image data format */
		    GL_UNSIGNED_BYTE,	/* Image data type */
		    /* Pointer to image data */
		    (GLubyte *)(ptr8 + (cur_width * cur_width * i * 2))
		);
		break;
	    }

	    /* Record this gl texture id as data */
	    t->data[i] = (void *)gl_texture_id;
	}

	/* Record other values */
	t->name = STRDUP(name);
	t->filename = STRDUP(path);
	t->width = cur_width;
	t->height = cur_width;  /* Height of tile is width */
	t->dimensions = 2;

	/* Delete the loaded data since it is no longer needed, and
	 * also deallocate cur_data_ptr if it does not match
	 * loaded_data_ptr
	 */
	free(data);
	if(cur_data_ptr != loaded_data_ptr)
	    free(cur_data_ptr);

	return(t);
}

/*
 *      Creates a 1D texture from memory, returning a dynamically
 *	allocated texture reference structure. The pointer to data
 *	may be deallocated right after calling this function.
 *
 *	There will be no multiple frames allowed.
 *
 *      If the progress_cb function is not NULL and returns 0, then
 *      the loading of each frame will stop and the texture will return
 *      the pointer to the texture structure but maybe with fewer frames
 *      loaded.
 */
v3d_texture_ref_struct *V3DTextureLoadFromData1D(
	const void *data,	/* Texture data */
	const char *name,	/* Name of texture for referancing */
	int width,
	int bytes_per_pixel,	/* Bytes per pixel of data */
	v3d_tex_format dest_fmt,
	void *client_data,
	int (*progress_cb)(void *, int, int)
)
{
	int i;
	v3d_texture_ref_struct *t;
	GLuint gl_texture_id;
	u_int8_t *ptr8;
	u_int32_t *ptr32;

	if(data == NULL)
	    return(NULL);

	/* Check if size is big enough */
	if(width < 2)
	    fprintf(stderr,
 "0x%.8x: Warning: Image size is too small in width.\n",
		(unsigned int)data
	    );

	/* Allocate a texture reference structure */
	t = (v3d_texture_ref_struct *)calloc(1, sizeof(v3d_texture_ref_struct));
	if(t == NULL)
	    return(NULL);

	/* Create one texture frame */
	t->total_frames = 1;
	t->data = (void **)calloc(
	    t->total_frames,
	    sizeof(void *)
	);
	if(t->data == NULL)
	{
	    free(t);
	    return(NULL);
	}
	for(i = 0; i < t->total_frames; i++)
	{
	    /* Notify progress callback and see if we should continue
	     * loading
	     */
	    if(progress_cb != NULL)
	    {
		if(!progress_cb(client_data, i, t->total_frames))
		    break;
	    }

	    /* Generate a new texture ID */
	    glGenTextures(1, &gl_texture_id);
	    if(gl_texture_id == 0)
	    {
		fprintf(stderr,
		    "0x%.8x: Error generating texture\n",
		    (unsigned int)data
		);
		continue;
	    }

	    /* Actually generate texture and make texture active
	     * (selected)
	     */
	    glBindTexture(GL_TEXTURE_1D, gl_texture_id);
		    
	    /* Set parameters of selected texture */
	    TEXTUREIO_TEX_OPTIONS_1D
		
	    /* Load data we loaded from data into texture by format */
	    switch(dest_fmt)
	    {
	      case V3D_TEX_FORMAT_RGB:
		ptr8 = (u_int8_t *)data;
		glTexImage1D(
		    GL_TEXTURE_1D,      /* GL_TEXTURE_1D or GL_PROXY_TEXTURE_1D */
		    0,                  /* Level */
		    GL_RGB,             /* Internal format */
		    width,              /* Width */
		    0,                  /* Border */
		    GL_RGB,             /* Image data format */
		    GL_UNSIGNED_BYTE,   /* Image data type */
		    /* Pointer to image data */
		    (GLubyte *)(ptr8 + (width * i * 3))
		);
		break;

	      case V3D_TEX_FORMAT_RGBA:
		ptr32 = (u_int32_t *)data;
		glTexImage1D(
		    GL_TEXTURE_1D,      /* GL_TEXTURE_1D or GL_PROXY_TEXTURE_1D */
		    0,                  /* Level */
		    GL_RGBA,            /* Internal format */
		    width,              /* Width */
		    0,                  /* Border */
		    GL_RGBA,            /* Image data format */
		    GL_UNSIGNED_BYTE,   /* Image data type */
		    /* Pointer to image data */
		    (GLubyte *)(ptr32 + (width * i))
		);
		break;

	      case V3D_TEX_FORMAT_LUMINANCE:
		ptr8 = (u_int8_t *)data;
		glTexImage1D(
		    GL_TEXTURE_1D,      /* GL_TEXTURE_1D or GL_PROXY_TEXTURE_1D */
		    0,                  /* Level */
		    GL_LUMINANCE8,      /* Internal format */
		    width,              /* Width */
		    0,                  /* Border */
		    GL_LUMINANCE,       /* Image data format */
		    GL_UNSIGNED_BYTE,   /* Image data type */
		    /* Pointer to image data */
		    (GLubyte *)(ptr8 + (width * i))
		);
		break;

	      case V3D_TEX_FORMAT_LUMINANCE_ALPHA:
		ptr8 = (u_int8_t *)data;
		glTexImage1D(
		    GL_TEXTURE_1D,	/* GL_TEXTURE_1D or GL_PROXY_TEXTURE_1D */
		    0,			/* Level */
		    GL_LUMINANCE8_ALPHA8,	/* Internal format */
		    width,		/* Width */
		    0,			/* Border */
		    GL_LUMINANCE_ALPHA,	/* Image data format */
		    GL_UNSIGNED_BYTE,	/* Image data type */
		    /* Pointer to image data */
		    (GLubyte *)(ptr8 + (width * i * 2))
		);
		break;
	    }

	    /* Record this gl texture id as data */
	    t->data[i] = (void *)gl_texture_id;
	}

	/* Record other values */
	t->name = STRDUP(name);
	t->filename = NULL;
	t->width = width;
	t->height = width;   /* Height of tile is width */
	t->dimensions = 1;

	if(progress_cb != NULL)
	    progress_cb(client_data, t->total_frames, t->total_frames);

	return(t);
}

/*
 *	Creates a 2D texture from memory, returning a dynamically
 *	allocated texture reference structure.  The pointer to data
 *      may be deallocated right after calling this function.
 *
 *      If the height is greater than the width, then it will be treated
 *      as an array of vertical frames where the number of frames is
 *      height / width (this only works when height and width are both 
 *      a value of 2^n).
 *
 *      If the progress_cb function is not NULL and returns 0, then
 *      the loading of each frame will stop and the texture will return
 *      the pointer to the texture structure but maybe with fewer frames
 *      loaded.
 */
v3d_texture_ref_struct *V3DTextureLoadFromData2D(
	const void *data,	/* Texture data */
	const char *name,	/* Name of texture for referancing */
	int width, int height,
	int bytes_per_pixel,	/* Bytes per pixel of data */
	v3d_tex_format dest_fmt,
	void *client_data,
	int (*progress_cb)(void *, int, int)
)
{
	int i;
	v3d_texture_ref_struct *t;
	GLuint gl_texture_id;
	u_int8_t *cur_data_ptr, *loaded_data_ptr;
	int cur_width, cur_height, loaded_width, loaded_height;
	u_int8_t *ptr8;
	u_int32_t *ptr32;


	if(data == NULL) 
	    return(NULL);

	/* Check if size is big enough */
	if(width < 2)
	    fprintf(stderr,
 "0x%.8x: Warning: Image size is too small in width.\n",
		(unsigned int)data
	    );
	if(height < 2)
	    fprintf(stderr,
 "0x%.8x: Warning: Image size is too small in height.\n",
		(unsigned int)data
	    );

	/* Get pointers to loaded data */
	cur_data_ptr = loaded_data_ptr = (u_int8_t *)data;
	cur_width = loaded_width = width;
	cur_height = loaded_height = height;

	/* Rescale image data as needed */
	switch(dest_fmt)
	{
	  case V3D_TEX_FORMAT_RGB:
	    V3DTextureRescaleImage(
	        data,
		3, GL_RGB,
		loaded_width, loaded_height,
		&cur_data_ptr,
		&cur_width, &cur_height
	    );
	    break;

	  case V3D_TEX_FORMAT_RGBA:
	    V3DTextureRescaleImage(
		data,
		4, GL_RGBA,
		loaded_width, loaded_height,
		&cur_data_ptr,
		&cur_width, &cur_height
	    );
	    break;

	  case V3D_TEX_FORMAT_LUMINANCE:
	    V3DTextureRescaleImage(
		data,
		1, GL_LUMINANCE,
		loaded_width, loaded_height,
		&cur_data_ptr,
		&cur_width, &cur_height
	    );
	    break;

	  case V3D_TEX_FORMAT_LUMINANCE_ALPHA:
	    V3DTextureRescaleImage(
		data,
		2, GL_LUMINANCE_ALPHA,
		loaded_width, loaded_height,
		&cur_data_ptr,
		&cur_width, &cur_height
	    );
	    break;
	}
	/* Is cur_data_ptr still valid from above processing? */
	if((cur_data_ptr == NULL) ||
	   (cur_width <= 0) ||
	   (cur_height <= 0)
	)
	{
	    if(cur_data_ptr != loaded_data_ptr)
		free(cur_data_ptr);
	    return(NULL);
	}


	/* Allocate a texture reference structure */
	t = (v3d_texture_ref_struct *)calloc(1, sizeof(v3d_texture_ref_struct));
	if(t == NULL)
	{
	    if(cur_data_ptr != loaded_data_ptr)
		free(cur_data_ptr);
	    return(NULL);
	}

	/* Create each texture frame */
	t->total_frames = (int)cur_height / (int)cur_width;
	if(t->total_frames < 1)
	    t->total_frames = 1;
	t->data = (void **)calloc(
	    t->total_frames,
	    sizeof(void *)  
	);
	if(t->data == NULL)
	{
	    free(t);
	    if(cur_data_ptr != loaded_data_ptr)
		free(cur_data_ptr);
	    return(NULL);
	}
	for(i = 0; i < t->total_frames; i++)
	{
	    /* Notify progress callback and see if we should continue
	     * loading.
	     */ 
	    if(progress_cb != NULL)
	    {
		if(!progress_cb(client_data, i, t->total_frames))
		    break;
	    }

	    /* Generate a new texture ID */
	    glGenTextures(1, &gl_texture_id);
	    if(gl_texture_id == 0)
	    {
		fprintf(stderr,
		    "0x%.8x: Error generating texture\n",
		    (unsigned int)cur_data_ptr
		);
		continue;
	    }

	    /* Actually generate texture and make texture active
	     * (selected).
	     */
	    glBindTexture(GL_TEXTURE_2D, gl_texture_id);

	    /* Set parameters of selected texture */
	    TEXTUREIO_TEX_OPTIONS_2D

	    /* Load data we loaded from data into texture by format */
	    switch(dest_fmt)
	    {
	      case V3D_TEX_FORMAT_RGB:
		ptr8 = (u_int8_t *)cur_data_ptr;
		glTexImage2D(
		    GL_TEXTURE_2D,      /* GL_TEXTURE_2D or GL_PROXY_TEXTURE_2D */
		    0,                  /* Level */
		    GL_RGB,             /* Internal format */
		    cur_width,		/* Width */
		    cur_width,		/* Height (same as width) */
		    0,                  /* Border */
		    GL_RGB,             /* Image data format */
		    GL_UNSIGNED_BYTE,   /* Image data type */
		    /* Pointer to image data */
		    (GLubyte *)(ptr8 + (cur_width * cur_width * i * 3))
		);
		break;

	      case V3D_TEX_FORMAT_RGBA:
		ptr32 = (u_int32_t *)cur_data_ptr;
		glTexImage2D(
		    GL_TEXTURE_2D,      /* GL_TEXTURE_2D or GL_PROXY_TEXTURE_2D */
		    0,                  /* Level */
		    GL_RGBA,            /* Internal format */
		    cur_width,		/* Width */
		    cur_width,		/* Height (same as width) */
		    0,                  /* Border */
		    GL_RGBA,            /* Image data format */
		    GL_UNSIGNED_BYTE,   /* Image data type */   
		    /* Pointer to image data */
		    (GLubyte *)(ptr32 + (cur_width * cur_width * i))
		);
		break;

	      case V3D_TEX_FORMAT_LUMINANCE:
		ptr8 = (u_int8_t *)cur_data_ptr;
		glTexImage2D(
		    GL_TEXTURE_2D,      /* GL_TEXTURE_2D or GL_PROXY_TEXTURE_2D */
		    0,                  /* Level */
		    GL_LUMINANCE8,      /* Internal format */
		    cur_width,		/* Width */
		    cur_width,		/* Height (same as width) */
		    0,                  /* Border */
		    GL_LUMINANCE,       /* Image data format */
		    GL_UNSIGNED_BYTE,   /* Image data type */
		    /* Pointer to image data */
		    (GLubyte *)(ptr8 + (cur_width * cur_width * i))
		);
		break;

	      case V3D_TEX_FORMAT_LUMINANCE_ALPHA:
		ptr8 = (u_int8_t *)cur_data_ptr;
		glTexImage2D(
		    GL_TEXTURE_2D,	/* GL_TEXTURE_2D or GL_PROXY_TEXTURE_2D */
		    0,			/* Level */
		    GL_LUMINANCE8_ALPHA8,	/* Internal format */
		    cur_width,		/* Width */
		    cur_width,		/* Height (same as width) */
		    0,			/* Border */
		    GL_LUMINANCE_ALPHA,	/* Image data format */
		    GL_UNSIGNED_BYTE,	/* Image data type */
		    /* Pointer to image data */
		    (GLubyte *)(ptr8 + (cur_width * cur_width * i * 2))
		);
		break;

	    }

	    /* Record this gl texture id as data */
	    t->data[i] = (void *)gl_texture_id;
	}

	/* Record other values */
	t->name = STRDUP(name);
	t->filename = NULL;
	t->width = cur_width;
	t->height = cur_width;	/* Height of tile is width */
	t->dimensions = 2;

	/* Deallocate cur_data_ptr if it does not match
	 * loaded_data_ptr.
	 */
	if(cur_data_ptr != loaded_data_ptr)
	    free(cur_data_ptr);
 
	if(progress_cb != NULL)
	    progress_cb(client_data, t->total_frames, t->total_frames);

	return(t);
}



/*
 *	Sets OpenGL memory resident priority of the given texture.
 *
 *	1.0 is the highest priority (most likly to be in resident texture
 *	memory) and 0.0 is the lowest priority (least likly to be in
 *	resident texture memory).
 *
 *	If texture has multiple frames, all frames will be set with the
 *	given priority.
 */
void V3DTexturePriority(v3d_texture_ref_struct *t, float priority)
{
	int i;
	GLuint tex_id_list[1];
	GLclampf tex_priority_list[1];

	if(t == NULL)
	    return;

	if(priority < 0.0f)
	    priority = 0.0f;
	else if(priority > 1.0f)
	    priority = 1.0f;

	tex_priority_list[0] = (GLclampf)priority;

	/* Update all OpenGL texture names' priority on the specified
	 * texture
	 */
	for(i = 0; i < t->total_frames; i++)
	{
	    tex_id_list[0] = (GLuint)t->data[i];
	    glPrioritizeTextures(1, tex_id_list, tex_priority_list);
	}

	t->priority = priority;
}

/*
 *	Deallocates all resources associated in the texture reference
 *	structure and deallocates the structure itself.
 *
 *	The given structure must be that of one recieved from a call
 *	to V3DTextureLoadFromFile*() or V3DTextureLoadFromData*().
 */
void V3DTextureDestroy(v3d_texture_ref_struct *t)
{
	int i;
	GLuint gl_texture_id;


	if(t == NULL)
	    return;

	/* Free texture data frames */
	for(i = 0; i < t->total_frames; i++)
	{
	    if(t->data[i] == NULL)
		continue;

	    gl_texture_id = (GLuint)(t->data[i]);
	    glDeleteTextures(
		1,
		&gl_texture_id
	    );
	}
	free(t->data);

	/* Free reference name and file name */
	free(t->name);
	free(t->filename);

	/* Free structure itself since we allocated it */
	free(t);
}
