#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <sys/stat.h>
#ifdef __MSW__
# include <windows.h>
#else
# include <unistd.h>
#endif
#include "../include/disk.h"
#include "../include/tga.h"
#include "gw.h"
#include "sound.h"
#include "sar.h"
#include "sarscreenshot.h"
#include "config.h"


static char *SARScreenShotFindName(const char *dir);
static int SARScreenShotWriteTGA(
	const char *path,
	int width, int height, int bpl, int bpp,
	const u_int8_t *buf
);
static void SARScreenShotDraw(
        sar_core_struct *core_ptr, int detail_level
);

void SARScreenShotRect(
	sar_core_struct *core_ptr, const char *dir, int detail_level,
        int x, int y, int width, int height
);
void SARScreenShot(
	sar_core_struct *core_ptr, const char *dir, int detail_level
);


#define ATOI(s)         (((s) != NULL) ? atoi(s) : 0)
#define ATOL(s)         (((s) != NULL) ? atol(s) : 0)
#define ATOF(s)         (((s) != NULL) ? (float)atof(s) : 0.0f)
#define STRDUP(s)       (((s) != NULL) ? strdup(s) : NULL)

#define MAX(a,b)        (((a) > (b)) ? (a) : (b))
#define MIN(a,b)        (((a) < (b)) ? (a) : (b))
#define CLIP(a,l,h)     (MIN(MAX((a),(l)),(h)))
#define STRLEN(s)       (((s) != NULL) ? ((int)strlen(s)) : 0)

#define RADTODEG(r)     ((r) * 180.0 / PI)
#define DEGTORAD(d)     ((d) * PI / 180.0)

#define PLAY_SOUND(p)						\
{ if((opt->event_sounds) && (recorder != NULL)) {		\
 char *full_path = (ISPATHABSOLUTE(p)) ?			\
  STRDUP(p) :							\
  STRDUP(PrefixPaths(dname.global_data, (p))			\
 );								\
 SoundStartPlayVoid(						\
  recorder, full_path, 1.0, 1.0, 0, 0				\
 );								\
 free(full_path);						\
} }

/*
 *	Generates a path to a file that does not exist, suitable as
 *	the path to the new screen shot image file.
 *
 *	The returned path must be free'ed by the calling function.
 */
static char *SARScreenShotFindName(const char *dir)
{
	int i;
	char *path = (char *)malloc((PATH_MAX + NAME_MAX + 1) * sizeof(char));
	char cwd[PATH_MAX];
	struct stat stat_buf;

	if(path == NULL)
	    return(NULL);

	/* Use current working directory if no directory was given. */
	if(dir == NULL)
	{
	    if(getcwd(cwd, PATH_MAX) != NULL)
		cwd[PATH_MAX - 1] = '\0';
	    else
		strcpy(cwd, "/");
	    dir = cwd;
	}

/*
	if(STRLEN(dir) >= PATH_MAX)
	    dir[PATH_MAX] = '\0';
 */

	for(i = 1; i < 1000000; i++)
	{
	    sprintf(
		path, "%s%c%s%i%s",
		dir,
		DIR_DELIMINATOR,
		"sar",
		i,
		".tga"
	    );
#ifdef __MSW__
	    if(stat(path, &stat_buf))
#else
	    if(lstat(path, &stat_buf))
#endif
		break;
	}

	return(path);
}

/*
 *	Writes the given buffer to a TGA image.
 *
 *	Returns non-zero on error.
 */
static int SARScreenShotWriteTGA(
        const char *path,
        int width, int height, int bpl, int bpp,
	const u_int8_t *buf
)
{
	int status, tar_depth;
	u_int8_t *tar_buf;
	tga_data_struct td_data, *td = &td_data;

	if((path == NULL) || (buf == NULL))
	    return(-1);

	if((width < 0) || (height < 0) || (bpp < 1))
	    return(-1);

	memset(td, 0x00, sizeof(tga_data_struct));

	/* Calculate bytes per line as needed */
	if(bpl <= 0)
	    bpl = width * bpp;

	/* Get target depth based on given depth */
	switch(bpp)
	{
	  case 4:
	    tar_depth = 32;
	    break;
          case 3:
            tar_depth = 24;
            break;
          default:	/* 1 bpp */
            tar_depth = 8;
            break;
	}

	/* Allocate target buffer to flip and change color */
	tar_buf = (u_int8_t *)malloc(bpl * height);
	if(tar_buf != NULL)
	{
	    int x, y;
	    u_int8_t *tar_ptr;
	    const u_int8_t *src_ptr;

	    for(y = 0; y < height; y++)
	    {
		for(x = 0; x < width; x++)
		{
		    src_ptr = &buf[
			(bpl * y) + (x * bpp)
		    ];
		    tar_ptr = &tar_buf[
			(bpl * (height - y - 1)) +
			(x * bpp)
		    ];
		    switch(bpp)
		    {
		      case 4:	/* 32 bit src RGBA to tar BGRA */
			tar_ptr[0] = src_ptr[2];
			tar_ptr[1] = src_ptr[1];
			tar_ptr[2] = src_ptr[0];
			tar_ptr[3] = src_ptr[3];
			break;
		      case 3:	/* 32 bit src RGB to tar BGR */
                        tar_ptr[0] = src_ptr[2];
                        tar_ptr[1] = src_ptr[1];
                        tar_ptr[2] = src_ptr[0];
			break;
		      default:
			*tar_ptr = *src_ptr;
			break;
		    }
		}
	    }
	}

	/* Set up TGA data values for writing */
	td->fp = NULL;
	td->x = 0;
	td->y = 0;
	td->width = width;
	td->height = height;
	td->depth = bpp * 8;
	td->bits_per_pixel = td->depth;
	td->header_data = NULL;
	td->data = tar_buf;
	td->data_depth = bpp * 8;

	/* Write TGA image */
	status = TgaWriteToFile(path, td, tar_depth);

	/* Delete TGA values and the target buffer */
	TgaDestroyData(td);

	return((status == TgaSuccess) ? 0 : -1);
}


/*
 *	Redraw the scene as needed based on the detail level.
 *
 *	If in the menus then nothing will be redrawn.
 */
static void SARScreenShotDraw(
	sar_core_struct *core_ptr, int detail_level
)
{
	/* In menus? */
	if(core_ptr->cur_menu < 0)
	{	
	    return;
	}







}


/*
 *	Performs a screenshot of a rectangular area of the current GL
 *	frame buffer.
 *
 *	x, y, width, and height geometry values are in window
 *	coordinates with the origin in the upper left corner.
 *
 *	detail_level specifies the amount of detail, where 0 is the
 *	current GL frame buffer as is.  For values 1 and higher, the
 *	scene may be redrawn with additional graphics options enabled.
 */
void SARScreenShotRect(
	sar_core_struct *core_ptr, const char *dir, int detail_level,
        int x, int y, int width, int height
)
{
	char *path;
	int status;
	int bpp = 4, fb_width, fb_height;
	GLubyte *gl_buf;
        gw_display_struct *display = (core_ptr != NULL) ?
            core_ptr->display : NULL;
	snd_recorder_struct *recorder = (core_ptr != NULL) ?
	    core_ptr->recorder : NULL;
	const sar_option_struct *opt;
        if(display == NULL)
            return;

	opt = &core_ptr->option;

	if((width < 1) || (height < 1))
	    return;

	/* Redraw the scene as needed based on the detail level */
	SARScreenShotDraw(core_ptr, detail_level);

	/* At this point the GL frame buffer has been drawn or not
	 * drawn, either way we are now ready to read the GL frame
	 * buffer.
	 */

	/* Allocate the GL read buffer */
	gl_buf = (GLubyte *)malloc(width * height * bpp * sizeof(GLubyte));
	if(gl_buf == NULL)
	    return;

	/* Get actual size of frame buffer */
        GWContextGet(
            display, GWContextCurrent(display),
            NULL, NULL,
            NULL, NULL,
            &fb_width, &fb_height
        );

	/* Play initial click sound just before snapping screen */
	PLAY_SOUND(SAR_DEF_SOUND_SCREENSHOT);
	GWSetInputBusy(display);

	/* If the GL buffer is double buffered then use the back buffer,
	 * otherwise we use the front buffer.
	 */
	if(display->has_double_buffer)
	    glReadBuffer(GL_BACK);
	else
	    glReadBuffer(GL_FRONT);
	glReadPixels(
	    MAX(x, 0),
	    MAX(fb_height - (y + height), 0),
	    width, height,
	    (bpp == 4) ? GL_RGBA : ((bpp == 3) ? GL_RGB : GL_LUMINANCE),
	    GL_UNSIGNED_BYTE,
	    gl_buf
	);

	/* Generate a file name for the new image, the file name will
	 * be that of a file that does not exist.
	 */
	path = SARScreenShotFindName(dir);

	/* Write the read GL buffer to a TGA image file */
	status = SARScreenShotWriteTGA(path, width, height, 0, bpp, gl_buf);
	free(path);

	/* Delete the GL read buffer since it is no longer needed */
	free(gl_buf);

        /* Play finish click sound just after snapping screen */
	PLAY_SOUND(SAR_DEF_SOUND_ERROR);
        GWSetInputReady(display);
}

/*
 *	Performs a screenshot of the current GL frame buffer.
 */
void SARScreenShot(
	sar_core_struct *core_ptr, const char *dir, int detail_level
)
{
	int width, height;
	gw_display_struct *display = (core_ptr != NULL) ?
	    core_ptr->display : NULL;
	if(display == NULL)
	    return;

	GWContextGet(
	    display, GWContextCurrent(display),
	    NULL, NULL,
	    NULL, NULL,
	    &width, &height
	);

	SARScreenShotRect(core_ptr, dir, detail_level, 0, 0, width, height);
}
