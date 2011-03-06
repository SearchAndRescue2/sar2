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

#include "../include/os.h"
#include "../include/fio.h"
#include "../include/tga.h"


int TgaQueryVersion(int *major_rtn, int *minor_rtn);

static void TgaReportError(
	const char *filename, const char *reason, int how_bad
);

int TgaReadHeaderFromFile(const char *filename, tga_data_struct *td);
int TgaReadHeaderFromData(const u_int8_t *data, tga_data_struct *td);

int TgaReadFromFile(
	const char *filename,
	tga_data_struct *td,
	unsigned int depth
);
int TgaReadFromData(
	const u_int8_t *data,
	tga_data_struct *td,
	unsigned int depth
);

int TgaStartReadPartialFromFile(
	const char *filename,
	tga_data_struct *td,
	unsigned int depth
);
int TgaReadPartialFromFile(
	tga_data_struct *td,
	unsigned int depth,
	unsigned int n_pixels
);

u_int8_t *TgaReadFromFileFastRGBA(
	const char *filename,
	int *rtn_width, int *rtn_height,
	u_int32_t transparent_pixel     /* RGBA of transparent pixel */
);

int TgaWriteToFile(
	const char *filename,
	tga_data_struct *td,
	unsigned int depth
);

void TgaDestroyData(tga_data_struct *td);

int TgaTestFile(const char *filename);
 

#ifndef MIN
# define MIN(a,b)	(((a) < (b)) ? (a) : (b))
#endif
#ifndef MAX
# define MAX(a,b)	(((a) > (b)) ? (a) : (b))
#endif


/* Bytes Per Pixel Values */
#ifndef BYTES_PER_PIXEL8
# define BYTES_PER_PIXEL8       1       /* For rrgg gbbb */
#endif

#ifndef BYTES_PER_PIXEL15
# define BYTES_PER_PIXEL15      2       /* For arrr rrgg gggb bbbb */
#endif

#ifndef BYTES_PER_PIXEL16
# define BYTES_PER_PIXEL16      2       /* For rrr rrggg gggb bbbb */
#endif

#ifndef BYTES_PER_PIXEL24
# define BYTES_PER_PIXEL24      4       /* Same as BYTES_PER_PIXEL32 */
#endif

#ifndef BYTES_PER_PIXEL32
# define BYTES_PER_PIXEL32      4
#endif


/* Bit Packing Macros */
#ifndef PACK8TO8
# define PACK8TO8(r,g,b)	(u_int8_t)(		\
 (((r) >> 5) << 5) + (((g) >> 5) << 2) + ((b) >> 6)	\
)
#endif

#ifndef PACK8TO15
# define PACK8TO15(r,g,b)	(u_int16_t)(		\
 (((r) >> 3) << 10) + (((g) >> 3) << 5) + ((b) >> 3)	\
)
#endif

#ifndef PACK8TO16
# define PACK8TO16(r,g,b)	(u_int16_t)(		\
 (((r) >> 3) << 11) + (((g) >> 2) << 5) + ((b) >> 3)	\
)
#endif

#ifndef PACK8TO32
# define PACK8TO32(a,r,g,b)	(u_int32_t)(		\
 ((a) << 24) | ((r) << 16) | ((g) << 8) | (b)		\
)
#endif


/*
 *	`Almost' black pixel values, needed for rounded off
 *	`almost' black pixels on file that decrease in depth.
 *	Depth that need this are 8, 15, and 16.
 */
#define ALMOSTBLACKPIX8		((u_int8_t)(1 << 6) |\
				 (u_int8_t)(1 << 3) |\
				 (u_int8_t)(1 << 0))
#define ALMOSTBLACKPIX15	((u_int16_t)(1 << 10) |\
				 (u_int16_t)(1 << 5)  |\
				 (u_int16_t)(1 << 0))
#define ALMOSTBLACKPIX16	((u_int16_t)(1 << 11) |\
				 (u_int16_t)(1 << 5) |\
				 (u_int16_t)(1 << 0))


/*
 *	Returns the version number.
 */
int TgaQueryVersion(int *major_rtn, int *minor_rtn)
{
	/* Major version number */
	if(major_rtn != NULL)
	    *major_rtn = TgaVersionMajor;

	/* Minor version number */
	if(minor_rtn != NULL)
	    *minor_rtn = TgaVersionMinor;

	return(0);
}

/*
 *	Error reporting function.
 */
static void TgaReportError(
	const char *filename,
	const char *reason,
	int how_bad
)
{
	/* Print severity of error */
	switch(how_bad)
	{
	  case TgaErrorLevelCritical:
	    fprintf(stderr, "Targa Library Critical error:\n");
	    break;

	  case TgaErrorLevelModerate:
	    fprintf(stderr, "Targa Library Moderate error:\n");
	    break;

	  case TgaErrorLevelMinor:
	    fprintf(stderr, "Targa Library Minor error:\n");
	    break;

	  case TgaErrorLevelWarning:
	    fprintf(stderr, "Targa Library Warning:\n");
	    break;
 
	  default:
	    fprintf(stderr, "Targa Library Error:\n");
	    break;
	}

	/* Filename */
	if(filename != NULL)
	    fprintf(stderr, "   Filename: %s\n", filename);

	/* Reason */
	if(reason != NULL)
	    fprintf(stderr, "   Reason: %s\n", reason);
}


/*
 *	Reads the tga image header from the specified file and stores
 *	them in td.
 */
int TgaReadHeaderFromFile(const char *filename, tga_data_struct *td)
{
	int pos, bpp, bpl, colormap_length = 0;
	u_int8_t *buf, *buf_ptr, *buf_end;
	FILE *fp;
	struct stat statbuf;
	char s[1024];

	if((filename == NULL) || (td == NULL))
	    return(TgaNoBuffers);

	/* Clear TGA data structure */
	memset(td, 0x00, sizeof(tga_data_struct));

	/* Check if file exists and get stats */
	if(stat(filename, &statbuf))
	    return(TgaNoFile);

	/* Set file size */
	td->file_size = statbuf.st_size;

	/* Size too small to contain a header? */
	if(td->file_size < TgaHeaderLength)
	    return(TgaBadHeader);

	/* Open file for reading */
	fp = FOpen(filename, "rb");
	if(fp == NULL)
	    return(TgaNoAccess); 

	/* Check if this is a new version by reading the last 26 bytes
	 * of the file and check bytes 8 to 23 to see if it contains
	 * "TRUEVISION-XFILE" (16 bytes long)
	 */
	if(!fseek(fp, (long)td->file_size - 26, SEEK_SET))
	{
	    char buf[16];
	    int bytes_read = fread(buf, sizeof(char), 16, fp);
	    if(bytes_read == 16)
	    {
		if(!memcmp(buf + 8, "TRUEVISION-XFILE", 16))
		    td->version = TgaFormatVersionNew;
	    }
	}
	rewind(fp);

	/* Allocate memory for header */
	td->header_data = buf = (u_int8_t *)malloc(
	    TgaHeaderLength * sizeof(u_int8_t)
	);
	if(buf == NULL)
	{
	    FClose(fp);
	    return(TgaNoBuffers);
	}

	/* Read header */
	if(fread(buf, sizeof(u_int8_t), TgaHeaderLength, fp) < TgaHeaderLength)
	{
	    FClose(fp);
	    return(TgaBadHeader);
	}

	/* Parse header */
	buf_ptr = buf;
	buf_end = buf_ptr + TgaHeaderLength;
	while(buf_ptr < buf_end)
	{
	    pos = buf_ptr - buf;

	    /* Image ID Field Length (in bytes) */
	    if(pos == 0x00)
	    {
		td->id_field_len = *buf_ptr;
		buf_ptr++;
	    }
	    /* Colormap Type */
	    else if(pos == 0x01)
	    {
		td->cmap_type = (tga_colormap_type)*buf_ptr;
		buf_ptr++;
	    }
	    /* Image Data Type */
	    else if(pos == 0x02)
	    {
		td->img_type = (tga_image_data_type)*buf_ptr;
		buf_ptr++;
	    }
	    /* Colormap - First Color Index */
	    else if(pos == 0x03)
	    {
		td->cmap_first_color_index = *(u_int16_t *)buf_ptr;
		buf_ptr += sizeof(u_int16_t);
	    }
	    /* Colormap - Total Number Of Colors */
	    else if(pos == 0x05)
	    {
		td->cmap_total_colors = *(u_int16_t *)buf_ptr;
		buf_ptr += sizeof(u_int16_t);  
	    }
	    /* Colormap - Entry Size (in bits) */
	    else if(pos == 0x07)
	    {
		td->cmap_entry_size = *buf_ptr;
		buf_ptr++;
	    }
	    /* Image Specification - X Origin */
	    else if(pos == 0x08)
	    {
		td->x = *(int16_t *)buf_ptr;
		buf_ptr += sizeof(int16_t);
	    }
	    /* Image Specification - Y Origin */
	    else if(pos == 0x0a)
	    {
		td->y = *(int16_t *)buf_ptr;
		buf_ptr += sizeof(int16_t);
	    }
	    /* Image Specification - Width */
	    else if(pos == 0x0c)
	    {
		td->width = *(u_int16_t *)buf_ptr;
		buf_ptr += sizeof(u_int16_t);
	    }
	    /* Image Specification - Height */
	    else if(pos == 0x0e)
	    {
		td->height = *(u_int16_t *)buf_ptr;
		buf_ptr += sizeof(u_int16_t);
	    }
	    /* Image Specification - Depth */
	    else if(pos == 0x10)
	    {
		td->depth = *buf_ptr;
		buf_ptr++;
	    }
	    /* Image Specification - Descriptor Flags */
	    else if(pos == 0x11)
	    {
		td->flags = (tga_image_flags)*buf_ptr;
		buf_ptr++;
	    }
	    /* All else seek to next value */
	    else
	    {
		buf_ptr++;
	    }
	}

	/* Get Comment from ID Field */
	if(td->id_field_len > 0)
	{
	    int id_field_len = (int)td->id_field_len;
	    char *s = (char *)malloc(id_field_len + 1);
	    int bytes_read = fread(
		s, sizeof(char), id_field_len, fp
	    );

	    if((bytes_read >= 0) && (bytes_read <= id_field_len))
		s[bytes_read] = '\0';
	    else
		*s = '\0';

	    td->comments = s;
	}

	/* Calculate colormap length in bytes */
	if(td->cmap_type == TgaColormapExists)
	{
	    int cmap_bpp;
	    switch(td->cmap_entry_size)
	    {
	      case 1:
	      case 4:
	      case 7:
	      case 8:
		cmap_bpp = 1;
		break;
	      case 15:
	      case 16:
		cmap_bpp = 2;
		break;
	      case 24:
		cmap_bpp = 3;
		break;
	      case 32:
		cmap_bpp = 4;
		break;
	      default:
		cmap_bpp = 1;
		break;
	    }
	    colormap_length = cmap_bpp * (int)td->cmap_total_colors;
	}
	td->cmap_size = colormap_length;

	/* Read colormap */
        /*
	if(colormap_length > 0)
	{


	}
        */

	/* Close file, it is no longer be needed */
	FClose(fp);


	/* Set bits_per_pixel in accordance with the depth obtained from
	 * file
	 */
	td->bits_per_pixel = td->depth;

	/* Get bytes per pixel from depth and bytes per line */
	bpp = MAX((td->depth >> 3), 1);
	bpl = td->width * bpp;

	/* Check and make sure width, height and bit depth are valid */
	if(td->width == 0)
	{
	    TgaReportError(
		filename,
		"Width of image is less than 1 pixel.",
		TgaErrorLevelModerate
	    );
	    return(TgaBadValue);
	}
	if(td->height == 0)
	{
	    TgaReportError(
		filename,
		"Height of image is less than 1 pixel.",
		TgaErrorLevelModerate
	    );
	    return(TgaBadValue);
	}
	if((td->depth != 1) &&
	   (td->depth != 8) &&
	   (td->depth != 16) &&
	   (td->depth != 24) &&
	   (td->depth != 32)
	)
	{
	    TgaReportError(
		filename,
		"Invalid bit depth.",
		TgaErrorLevelWarning
	    );
	    td->bits_per_pixel = 24;		/* Assume 24 bits */
	}

	/* Calculate and verify image data size in bytes */
	td->data_size = td->file_size - (off_t)TgaHeaderLength -
	    (off_t)td->id_field_len - (off_t)td->cmap_size;

	if(td->data_size < ((off_t)bpl * (off_t)td->height))
	{
	    /* Warn about inconsistant size */
	    sprintf(
		s,
 "Image data size %ld less than header indicated size %ld.\n",
		td->data_size,
		(off_t)bpl * (off_t)td->height
	    );
	    TgaReportError(
		filename,
		s,
		TgaErrorLevelWarning
	    );
	}

	return(TgaSuccess);
}


/*
 *	Reads the tga image header from the specified data and stores
 *	them in td.
 */
int TgaReadHeaderFromData(const u_int8_t *data, tga_data_struct *td)
{
	int pos, bpp, bpl, colormap_length = 0;
	const u_int8_t	*data_header, *data_id_field, *data_colormap,
			*buf_ptr, *buf_end;

	if((data == NULL) || (td == NULL))
	    return(TgaNoBuffers);

	/* Clear TGA data structure */
	memset(td, 0, sizeof(tga_data_struct));

	/* Allocate memory for header_data */
	td->header_data = (u_int8_t *)malloc(
	    TgaHeaderLength * sizeof(u_int8_t)
	);
	if(td->header_data == NULL)
	    return(TgaNoBuffers);

	data_header = data;
	memcpy(
	    td->header_data, data_header,
	    TgaHeaderLength * sizeof(u_int8_t)
	);

	/* Parse header */
	buf_ptr = data_header;
	buf_end = buf_ptr + TgaHeaderLength;
	while(buf_ptr < buf_end)
	{
	    pos = buf_ptr - data_header;

	    /* Image ID Field Length (in bytes) */
	    if(pos == 0x00)
	    {  
		td->id_field_len = *buf_ptr;
		buf_ptr++;
	    }
	    /* Colormap Type */
	    else if(pos == 0x01)
	    {
		td->cmap_type = (tga_colormap_type)*buf_ptr;
		buf_ptr++;
	    }
	    /* Image Data Type */
	    else if(pos == 0x02)
	    {
		td->img_type = (tga_image_data_type)*buf_ptr;
		buf_ptr++;
	    }
	    /* Colormap - First Color Index */
	    else if(pos == 0x03)
	    {
		td->cmap_first_color_index = *(u_int16_t *)buf_ptr;
		buf_ptr += sizeof(u_int16_t);
	    }
	    /* Colormap - Total Number Of Colors */
	    else if(pos == 0x05)
	    {
		td->cmap_total_colors = *(u_int16_t *)buf_ptr;
		buf_ptr += sizeof(u_int16_t);
	    }
	    /* Colormap - Entry Size (in bits) */
	    else if(pos == 0x07)
	    {
		td->cmap_entry_size = *buf_ptr;
		buf_ptr++; 
	    }  
	    /* Image Specification - X Origin */
	    else if(pos == 0x08)
	    {
		td->x = *(int16_t *)buf_ptr;
		buf_ptr += sizeof(int16_t);
	    }
	    /* Image Specification - Y Origin */
	    else if(pos == 0x0a)
	    {
		td->y = *(int16_t *)buf_ptr;
		buf_ptr += sizeof(int16_t);
	    }
	    /* Image Specification - Width */
	    else if(pos == 0x0c)
	    {
		td->width = *(u_int16_t *)buf_ptr;
		buf_ptr += sizeof(u_int16_t);
	    }
	    /* Image Specification - Height */
	    else if(pos == 0x0e)
	    {
		td->height = *(u_int16_t *)buf_ptr;
		buf_ptr += sizeof(u_int16_t);
	    }
	    /* Image Specification - Depth */
	    else if(pos == 0x10)
	    {
		td->depth = *buf_ptr;
		buf_ptr++;
	    }
	    /* Image Specification - Descriptor Flags */
	    else if(pos == 0x11)
	    {
		td->flags = (tga_image_flags)*buf_ptr;
		buf_ptr++;
	    }
	    /* All else seek to next value */
	    else
	    {   
		buf_ptr++;
	    }
	}

	/* Get Comment from ID Field */
	data_id_field = data_header + TgaHeaderLength;
	if(td->id_field_len > 0)
	{
	    int id_field_len = (int)td->id_field_len;
	    char *s = (char *)malloc(id_field_len + 1);
	    memcpy(
		s, data_id_field, id_field_len * sizeof(u_int8_t)
	    );
	    s[id_field_len] = '\0';
	    td->comments = s;
	}

	/* Calculate colormap length in bytes */
	if(td->cmap_type == TgaColormapExists)
	{
	    int cmap_bpp;
	    switch(td->cmap_entry_size)
	    {
	      case 1:
	      case 4:
	      case 7:
	      case 8:
		cmap_bpp = 1;
		break;
	      case 15:
	      case 16:
		cmap_bpp = 2;
		break;
	      case 24:
		cmap_bpp = 3;   
		break;
	      case 32:
		cmap_bpp = 4;
		break;
	      default:
		cmap_bpp = 1;
		break;
	    }
	    colormap_length = cmap_bpp * (int)td->cmap_total_colors;
	}
	td->cmap_size = colormap_length;

	/* Read colormap */
	data_colormap = data_id_field + td->id_field_len;
	if(colormap_length > 0)
	{


	}


	/* Set bits_per_pixel in accordance with the depth obtained from
	 * file
	 */
	td->bits_per_pixel = td->depth;

	/* Get bytes per pixel from depth and bytes per line */
	bpp = MAX((td->depth >> 3), 1);
	bpl = td->width * bpp;

	/* Check and make sure width, height and bit depth are valid */
	if(td->width == 0)
	{
	    TgaReportError(
		"Tga data",
		"Width of image is less than 1 pixel.",
		TgaErrorLevelModerate
	    );
	    return(TgaBadValue);
	}
	if(td->height == 0)
	{
	    TgaReportError(
		"Tga data",
		"Height of image is less than 1 pixel.",
		TgaErrorLevelModerate
	    );
	    return(TgaBadValue);
	}
	if((td->depth != 1) &&
	   (td->depth != 8) &&
	   (td->depth != 16) &&
	   (td->depth != 24) &&
	   (td->depth != 32)
	)
	{
	    TgaReportError(
		"Tga data",
		"Invalid bit depth.",
		TgaErrorLevelWarning
	    );
	    td->bits_per_pixel = 24;		/* Assume 24 bits */
	}

	/* Calculate file size and data size */
	td->data_size = (off_t)bpl * (off_t)td->height;
	td->file_size = (off_t)TgaHeaderLength + (off_t)td->id_field_len +
	    (off_t)td->cmap_size + (off_t)td->data_size;

	return(TgaSuccess);
}

/*
 *	Reads the tga image from the specified file.
 *
 *	TgaReadHeaderFromFile() will be automatically called by this
 *	function.   
 *
 *	The depth specifies the bit depth of the loaded image data.
 */
int TgaReadFromFile(
	const char *filename,
	tga_data_struct *td,
	unsigned int depth
)
{
	FILE *fp;
	int status, fpos, data_pos;
	unsigned int bytes_per_pixel;
	char s[256];

	u_int8_t *data_ptr8;
	u_int16_t *data_ptr16;
	u_int32_t *data_ptr32;

	u_int8_t pix[4], r, g, b;

	int pix_total, colum_count, row_count;

	if((filename == NULL) || (td == NULL))
	    return(TgaBadValue);

        /* make sure array is initialised */
        memset(pix, 0, sizeof(u_int8_t) * 4);

	/* Is the specified depth supported? */
	if((depth != 8) &&
	   (depth != 15) &&
	   (depth != 16) &&
	   (depth != 24) &&
	   (depth != 32)
	)
	{
	    sprintf(
		s,
		"Requested destination buffer depth %i is not supported.",
		depth
	    );
	    TgaReportError(filename, s, TgaErrorLevelCritical);
	    return(TgaBadValue);
	}
	/* Sanitize specified depth, if it is 24 then it should be 32 */
	if(depth == 24)
	    depth = 32;

	/* Read header */
	status = TgaReadHeaderFromFile(filename, td);
	if(status != TgaSuccess)
	    return(status);

	/* Do we support the depth obtained from file? */
	if((td->depth != 8) &&
	   (td->depth != 24) &&
	   (td->depth != 32)
	)
	{
	    sprintf(
		s,
		"Image file depth %i is not supported.",
		td->depth
	    );
	    TgaReportError(filename, s, TgaErrorLevelCritical);
	    return(TgaBadValue);
	}

	/* Open file for reading */
	fp = FOpen(filename, "rb");
	if(fp == NULL)
	    return(TgaNoAccess);

	/* Set data_depth to represent the depth of the image data
	 * that we are about to load
	 */
	td->data_depth = depth;		/* In bits per pixel */

	/* Calculate bytes per pixel */
	switch(td->data_depth)
	{
	  case 24:
	    bytes_per_pixel = BYTES_PER_PIXEL24;
	    break;
	  case 15:
	    bytes_per_pixel = BYTES_PER_PIXEL15;
	    break;
	  default:
	    bytes_per_pixel = (td->data_depth >> 3);
	    break;
	}

	/* Allocate image data */
	td->data = (u_int8_t *)malloc(
	    td->width * td->height * bytes_per_pixel
	);
	if(td->data == NULL)
	{
	    FClose(fp);
	    return(TgaNoBuffers);
	}

	/* Seek to start of image data */
	fpos = TgaHeaderLength + (int)td->id_field_len +
	    (int)td->cmap_size;
	if(fseek(fp, (long)fpos, SEEK_SET))
	{
	    FClose(fp);
	    return(TgaBadHeader);
	}


	/* Begin reading the image data from file */

	/* Check which encoding style the image data on file is in */
	if(td->flags & TgaImageFliped)
	{
	    /* READ RIGHTSIDE UP */

	    pix_total = td->width * td->height;
	    colum_count = 0;
	    row_count = 0;
	    data_pos = row_count * td->width * bytes_per_pixel;

	    /* Read by DESTINATION buffer depth */
	    switch(td->data_depth)
	    {
	      /* 32 bits */
	      case 32:
		while((fpos < (int)td->file_size) &&
		      (row_count < (int)td->height)
		)
		{
		    /* Get pointer in destination buffer */
		    data_ptr32 = (u_int32_t *)(&td->data[data_pos]);

		    /* Get pixel color values depending on 
		     * bits_per_pixel ON FILE
		     */
		    switch(td->bits_per_pixel)
		    {
		      case 32:
			pix[0] = (u_int8_t)fgetc(fp);
			pix[1] = (u_int8_t)fgetc(fp);
			pix[2] = (u_int8_t)fgetc(fp);
			pix[3] = (u_int8_t)fgetc(fp);
			*data_ptr32 = PACK8TO32(pix[3], pix[2], pix[1], pix[0]);
			break;

		      case 8:
			pix[0] = (u_int8_t)fgetc(fp);
			*data_ptr32 = PACK8TO32(0xff, pix[0], pix[0], pix[0]);
			break;

		      default:	/* Default to 24-bits */
			pix[0] = (u_int8_t)fgetc(fp);
			pix[1] = (u_int8_t)fgetc(fp);
			pix[2] = (u_int8_t)fgetc(fp);
			*data_ptr32 = PACK8TO32(0xff, pix[2], pix[1], pix[0]);
			break;
		    }

		    /* Increment data position in destination buffer */
		    data_pos += BYTES_PER_PIXEL32;

		    /* Increment colum_count */
		    colum_count++;

		    if(colum_count >= (int)td->width)
		    {
			colum_count = 0;
			row_count++;
			data_pos = row_count * td->width * BYTES_PER_PIXEL32;
		    }
		}
		break;

	      /* 16 bits */
	      case 16:
		while((fpos < (int)td->file_size) &&
		      (row_count < (int)td->height) 
		)
		{
		    /* Get pointer in destination buffer */
		    data_ptr16 = (u_int16_t *)(&td->data[data_pos]);

		    /* Get pixel color values depending on 
		     * bits_per_pixel ON FILE
		     */
		    switch(td->bits_per_pixel)
		    {
		      case 32:
			pix[0] = (u_int8_t)fgetc(fp);
			pix[1] = (u_int8_t)fgetc(fp);
			pix[2] = (u_int8_t)fgetc(fp);
			pix[3] = (u_int8_t)fgetc(fp);
			*data_ptr16 = PACK8TO16(pix[2], pix[1], pix[0]);
			break;

		      case 8:
			pix[0] = (u_int8_t)fgetc(fp);
			*data_ptr16 = PACK8TO16(pix[0], pix[0], pix[0]);
			break;

		      default:  /* Default to 24-bits */
			pix[0] = (u_int8_t)fgetc(fp);
			pix[1] = (u_int8_t)fgetc(fp);
			pix[2] = (u_int8_t)fgetc(fp);
			*data_ptr16 = PACK8TO16(pix[2], pix[1], pix[0]);
			break;
		    }
		    /* `Almost' black pixel roundoff check */
		    if(*data_ptr16 == 0x0000)
		    {
			if(pix[0] || pix[1] || pix[2])
			    *data_ptr16 = ALMOSTBLACKPIX16;
		    }

		    /* Increment data position in destination buffer */
		    data_pos += BYTES_PER_PIXEL16;

		    /* Increment colum_count */
		    colum_count++;

		    if(colum_count >= (int)td->width)
		    {
			colum_count = 0;
			row_count++;
			data_pos = row_count * td->width * BYTES_PER_PIXEL16;
		    }
		}
		break;

	      /* 15 bits */
	      case 15:
		while((fpos < (int)td->file_size) &&
		      (row_count < (int)td->height)
		)
		{
		    /* Get pointer in destination buffer */
		    data_ptr16 = (u_int16_t *)(&td->data[data_pos]);

		    /* Get pixel color values depending on
		     * bits_per_pixel ON FILE
		     */
		    switch(td->bits_per_pixel)
		    {
		      case 32:
			pix[0] = (u_int8_t)fgetc(fp);
			pix[1] = (u_int8_t)fgetc(fp);
			pix[2] = (u_int8_t)fgetc(fp);
			pix[3] = (u_int8_t)fgetc(fp);
			*data_ptr16 = PACK8TO15(pix[2], pix[1], pix[0]);
			break;

		      case 8:
			pix[0] = (u_int8_t)fgetc(fp);
			*data_ptr16 = PACK8TO15(pix[0], pix[0], pix[0]);
			break;

		      default:  /* Default to 24-bits */
			pix[0] = (u_int8_t)fgetc(fp);
			pix[1] = (u_int8_t)fgetc(fp);
			pix[2] = (u_int8_t)fgetc(fp);
			*data_ptr16 = PACK8TO15(pix[2], pix[1], pix[0]);
			break;
		    }
		    /* `Almost' black pixel roundoff check */
		    if(*data_ptr16 == 0x0000)
		    {
			if(pix[0] || pix[1] || pix[2])
			    *data_ptr16 = ALMOSTBLACKPIX15;
		    }

		    /* Increment data position in destination buffer */
		    data_pos += BYTES_PER_PIXEL15;

		    /* Increment colum_count */
		    colum_count++;
		 
		    if(colum_count >= (int)td->width)
		    {
			colum_count = 0;
			row_count++;
			data_pos = row_count * td->width * BYTES_PER_PIXEL15;
		    }
		}
		break;

	      /* 8 bits */
	      case 8:
		while((fpos < (int)td->file_size) &&
		      (row_count < (int)td->height)
		)
		{
		    /* Get pointer in destination buffer */
		    data_ptr8 = (u_int8_t *)(&td->data[data_pos]);

		    /* Get pixel color values depending on
		     * bits_per_pixel ON FILE
		     */
		    switch(td->bits_per_pixel)
		    {
		      case 32:
			r = (u_int8_t)fgetc(fp);
			pix[0] = TgaDitherRedPixel8(r, colum_count, row_count);
			g = (u_int8_t)fgetc(fp);
			pix[1] = TgaDitherGreenPixel8(g, colum_count, row_count);
			b = (u_int8_t)fgetc(fp);
			pix[2] = TgaDitherBluePixel8(b, colum_count, row_count);
			pix[3] = (u_int8_t)fgetc(fp);
			*data_ptr8 = PACK8TO8(pix[2], pix[1], pix[0]);
			break;

		      case 8:
			pix[0] = (u_int8_t)fgetc(fp);
			*data_ptr8 = PACK8TO8(pix[0], pix[0], pix[0]);
			break;

		      default:  /* Default to 24-bits */
			r = (u_int8_t)fgetc(fp);
			pix[0] = TgaDitherRedPixel8(r, colum_count, row_count);
			g = (u_int8_t)fgetc(fp);
			pix[1] = TgaDitherGreenPixel8(g, colum_count, row_count);
			b = (u_int8_t)fgetc(fp);
			pix[2] = TgaDitherBluePixel8(b, colum_count, row_count);
			*data_ptr8 = PACK8TO8(pix[2], pix[1], pix[0]);
			break;
		    }
		    /* `Almost' black pixel roundoff check */
		    if(*data_ptr8 == 0x00)
		    { 
			if(pix[0] || pix[1] || pix[2])
			    *data_ptr8 = ALMOSTBLACKPIX8;
		    }

		    /* Increment data position in destination buffer */
		    data_pos += BYTES_PER_PIXEL8;

		    /* Increment colum_count */
		    colum_count++;
	     
		    if(colum_count >= (int)td->width)
		    {
			colum_count = 0;
			row_count++;
			data_pos = row_count * td->width * BYTES_PER_PIXEL8;
		    }
		}
		break;
	    }
	}
	else
	{
	    /* READ UPSIDE DOWN */

	    pix_total = td->width * td->height;
	    colum_count = 0;
	    row_count = (int)td->height - 1;
	    data_pos = row_count * td->width * bytes_per_pixel;

	    /* Read by DESTINATION buffer depth */
	    switch(td->data_depth)
	    {
	      /* 32 bits */
	      case 32:
	        while((fpos < (int)td->file_size) &&
		      (row_count >= 0)
	        )
	        {
		    /* Get pointer in destination buffer */
		    data_ptr32 = (u_int32_t *)(&td->data[data_pos]);

		    /* Get pixel color values depending on
		     * bits_per_pixel ON FILE
		     */
		    switch(td->bits_per_pixel)
		    { 
		      case 32:
			pix[0] = (u_int8_t)fgetc(fp);
			pix[1] = (u_int8_t)fgetc(fp);
			pix[2] = (u_int8_t)fgetc(fp);
			pix[3] = (u_int8_t)fgetc(fp);
			*data_ptr32 = PACK8TO32(pix[3], pix[2], pix[1], pix[0]);
			break;

		      case 8:
			pix[0] = (u_int8_t)fgetc(fp);
			*data_ptr32 = PACK8TO32(0xff, pix[0], pix[0], pix[0]);
			break;

		      default:  /* Default to 24-bits */
			pix[0] = (u_int8_t)fgetc(fp);
			pix[1] = (u_int8_t)fgetc(fp);
			pix[2] = (u_int8_t)fgetc(fp);
			*data_ptr32 = PACK8TO32(0xff, pix[2], pix[1], pix[0]);
			break;
		    }

		    /* Increment data position in destination buffer */
		    data_pos += BYTES_PER_PIXEL32;

	            /* Increment colum_count */
	            colum_count++;

	            if(colum_count >= (int)td->width)
	            {
		        colum_count = 0;
		        row_count--;
		        data_pos = row_count * td->width * BYTES_PER_PIXEL32;
	            }
		}
		break;

	      /* 16 bits */
	      case 16:
		while((fpos < td->file_size) &&
		      (row_count >= 0)
		)
		{
		    /* Get pointer in destination buffer */
		    data_ptr16 = (u_int16_t *)(&td->data[data_pos]);

		    /* Get pixel color values depending on
		     * bits_per_pixel ON FILE
		     */
		    switch(td->bits_per_pixel)
		    {
		      case 32:
			pix[0] = (u_int8_t)fgetc(fp);
			pix[1] = (u_int8_t)fgetc(fp);
			pix[2] = (u_int8_t)fgetc(fp);
			pix[3] = (u_int8_t)fgetc(fp);
			*data_ptr16 = PACK8TO16(pix[2], pix[1], pix[0]);
			break;

		      case 8:
			pix[0] = (u_int8_t)fgetc(fp);
			*data_ptr16 = PACK8TO16(pix[0], pix[0], pix[0]);
			break;

		      default:  /* Default to 24-bits */
			pix[0] = (u_int8_t)fgetc(fp);
			pix[1] = (u_int8_t)fgetc(fp);
			pix[2] = (u_int8_t)fgetc(fp);
			*data_ptr16 = PACK8TO16(pix[2], pix[1], pix[0]);
			break;
		    }
		    /* `Almost' black pixel roundoff check */
		    if(*data_ptr16 == 0x0000)
		    {
			if(pix[0] || pix[1] || pix[2])
			    *data_ptr16 = ALMOSTBLACKPIX16;
		    }

		    /* Increment data position in destination buffer */
		    data_pos += BYTES_PER_PIXEL16;

		    /* Increment colum_count */
		    colum_count++;

		    if(colum_count >= (int)td->width)
		    {
			colum_count = 0;
			row_count--;
			data_pos = row_count * td->width * BYTES_PER_PIXEL16;
		    }
		}
		break;

	      /* 15 bits */
	      case 15:
		while((fpos < td->file_size) &&
		      (row_count >= 0)
		)
		{
		    /* Get pointer in destination buffer */
		    data_ptr16 = (u_int16_t *)(&td->data[data_pos]);

		    /* Get pixel color values depending on
		     * bits_per_pixel ON FILE
		     */
		    switch(td->bits_per_pixel)
		    {
		      case 32:
			pix[0] = (u_int8_t)fgetc(fp);
			pix[1] = (u_int8_t)fgetc(fp);
			pix[2] = (u_int8_t)fgetc(fp);
			pix[3] = (u_int8_t)fgetc(fp);
			*data_ptr16 = PACK8TO15(pix[2], pix[1], pix[0]);
			break;

		      case 8:
			pix[0] = (u_int8_t)fgetc(fp);
			*data_ptr16 = PACK8TO15(pix[0], pix[0], pix[0]);
			break;

		      default:  /* Default to 24-bits */
			pix[0] = (u_int8_t)fgetc(fp);
			pix[1] = (u_int8_t)fgetc(fp);
			pix[2] = (u_int8_t)fgetc(fp);
			*data_ptr16 = PACK8TO15(pix[2], pix[1], pix[0]);
			break;
		    }
		    /* `Almost' black pixel roundoff check */
		    if(*data_ptr16 == 0x0000) 
		    {
			if(pix[0] || pix[1] || pix[2])
			    *data_ptr16 = ALMOSTBLACKPIX15;
		    }

		    /* Increment data position in destination buffer */
		    data_pos += BYTES_PER_PIXEL15;

		    /* Increment colum_count */
		    colum_count++;

		    if(colum_count >= (int)td->width)
		    {
			colum_count = 0;
			row_count--;
			data_pos = row_count * td->width * BYTES_PER_PIXEL15;
		    }
		}
		break;

	      /* 8 bits */
	      case 8:
		while((fpos < (int)td->file_size) &&
		      (row_count >= 0)
		)
		{
		    /* Get pointer in destination buffer */
		    data_ptr8 = (u_int8_t *)(&td->data[data_pos]);

		    /* Get pixel color values depending on
		     * bits_per_pixel ON FILE
		     */
		    switch(td->bits_per_pixel)
		    {
		      case 32:
			r = (u_int8_t)fgetc(fp);
			pix[0] = TgaDitherRedPixel8(r, colum_count, row_count);
			g = (u_int8_t)fgetc(fp);
			pix[1] = TgaDitherGreenPixel8(g, colum_count, row_count);
			b = (u_int8_t)fgetc(fp);
			pix[2] = TgaDitherBluePixel8(b, colum_count, row_count);
			pix[3] = (u_int8_t)fgetc(fp);
			*data_ptr8 = PACK8TO8(pix[2], pix[1], pix[0]);
			break;

		      case 8:
			pix[0] = (u_int8_t)fgetc(fp);
			*data_ptr8 = PACK8TO8(pix[0], pix[0], pix[0]);
			break;

		      default:  /* Default to 24-bits */
			r = (u_int8_t)fgetc(fp);
			pix[0] = TgaDitherRedPixel8(r, colum_count, row_count);
			g = (u_int8_t)fgetc(fp);
			pix[1] = TgaDitherGreenPixel8(g, colum_count, row_count);
			b = (u_int8_t)fgetc(fp);
			pix[2] = TgaDitherBluePixel8(b, colum_count, row_count);
			*data_ptr8 = PACK8TO8(pix[2], pix[1], pix[0]);
			break;
		    }
		    /* `Almost' black pixel roundoff check */
		    if(*data_ptr8 == 0x00)
		    {
			if(pix[0] || pix[1] || pix[2])
			    *data_ptr8 = ALMOSTBLACKPIX8;
		    }

		    /* Increment data position in destination buffer */
		    data_pos += BYTES_PER_PIXEL8;

		    /* Increment colum_count */
		    colum_count++;

		    if(colum_count >= (int)td->width)
		    {
			colum_count = 0;
			row_count--;
			data_pos = row_count * td->width * BYTES_PER_PIXEL8;
		    }
		}
		break;
	    }
	}

	FClose(fp);

	return(TgaSuccess);
}

/*
 *	Reads the tga image from the specified data.
 *
 *	TgaReadHeaderFromData() will be automatically called by this
 *	function.
 *
 *	The depth specifies the bit depth of the loaded image data.
 */
int TgaReadFromData(
	const u_int8_t *data,
	tga_data_struct *td,
	unsigned int depth
)
{
	int status, fpos, data_pos;
	unsigned int bytes_per_pixel;
	char s[256];

	const u_int8_t *data_ptr;
	u_int8_t *data_ptr8;
	u_int16_t *data_ptr16;
	u_int32_t *data_ptr32;

	u_int8_t pix[4], r, g, b;

	int pix_total, colum_count, row_count;

	if(data == NULL)
	    return(TgaNoBuffers);

        /* make sure array is initialised */
        memset(pix, 0, sizeof(u_int8_t) * 4);

	/* Is the specified depth supported? */
	if((depth != 8) &&
	   (depth != 15) &&
	   (depth != 16) &&
	   (depth != 24) &&
	   (depth != 32)
	)
	{
	    sprintf(
		s,
		"Requested destination buffer bit depth %i not supported.",
		depth
	    );
	    TgaReportError("Tga data", s, TgaErrorLevelCritical);
	    return(TgaBadValue);
	}
	/* Sanitize bit depth, if it is 24 bits then it should be 32 */
	if(depth == 24)
	    depth = 32;

	/* Read header */
	status = TgaReadHeaderFromData(data, td);
	if(status != TgaSuccess)
	    return(status);

	/* Do we support the depth obtained from data? */
	if((td->depth != 24) &&
	   (td->depth != 32)
	)
	{
	    sprintf(
		s,
		"Requested source file bit depth %i not supported.",
		td->depth
	    );
	    TgaReportError("Tga data", s, TgaErrorLevelCritical);
	    return(TgaBadValue);
	}


	/* Get data pointer */
	data_ptr = data;

	/* Set data_depth to represent the depth of the image data
	 * that we are about to load
	 */
	td->data_depth = depth;		/* In bits per pixel */

	/* Calculate bytes per pixel */
	switch(td->data_depth)
	{  
	  case 24:
	    bytes_per_pixel = BYTES_PER_PIXEL24;
	    break;
	  case 15:
	    bytes_per_pixel = BYTES_PER_PIXEL15;
	    break;
	  default:
	    bytes_per_pixel = (td->data_depth >> 3);
	    break;
	}

	/* Allocate image data */
	td->data = (u_int8_t *)malloc(
	    td->width * td->height * bytes_per_pixel
	);
	if(td->data == NULL)
	    return(TgaNoBuffers);

	/* Seek to start of image data */
	fpos = TgaHeaderLength + (int)td->id_field_len +
	    (int)td->cmap_size;
	data_ptr += fpos;


	/* Begin reading the image data from file */

	/* Check which encoding style the image data on file is in */
	if(td->flags & TgaImageFliped)
	{
	    /* READ RIGHTSIDE UP */

	    pix_total = td->width * td->height;
	    colum_count = 0;
	    row_count = 0;
	    data_pos = row_count * td->width * bytes_per_pixel;

	    /* Check which bit padding to read to */
	    switch(td->data_depth)
	    {
	      /* 32 bits */
	      case 32:
		while((fpos < (int)td->file_size) &&
		      (row_count < (int)td->height)
		)   
		{
		    /* Get pointer in destination buffer */
		    data_ptr32 = (u_int32_t *)(td->data + data_pos);

		    /* Get pixel color values depending on 
		     * bits_per_pixel ON FILE.
		     */
		    switch(td->bits_per_pixel)
		    {
		      case 32:
			pix[0] = (u_int8_t)*data_ptr++;
			pix[1] = (u_int8_t)*data_ptr++;
			pix[2] = (u_int8_t)*data_ptr++;
			pix[3] = (u_int8_t)*data_ptr++;
			*data_ptr32 = PACK8TO32(pix[3], pix[2], pix[1], pix[0]);
			break;

		      case 8:
			pix[0] = (u_int8_t)*data_ptr++;
			*data_ptr32 = PACK8TO32(0xff, pix[0], pix[0], pix[0]);
			break;

		      default:  /* Default to 24-bits */
			pix[0] = (u_int8_t)*data_ptr++;
			pix[1] = (u_int8_t)*data_ptr++;
			pix[2] = (u_int8_t)*data_ptr++;
			*data_ptr32 = PACK8TO32(0xff, pix[2], pix[1], pix[0]);
			break;
		    }

		    /* Increment data position in destination buffer */
		    data_pos += BYTES_PER_PIXEL32;

		    /* Increment colum_count */
		    colum_count++;

		    if(colum_count >= (int)td->width)
		    {
			colum_count = 0;
			row_count++;
			data_pos = row_count * (int)td->width * BYTES_PER_PIXEL32;
		    }
		}
		break;

	      /* 16 bits */
	      case 16:
		while((fpos < (int)td->file_size) &&
		      (row_count < (int)td->height) 
		)
		{
		    /* Get pointer in destination buffer */
		    data_ptr16 = (u_int16_t *)(td->data + data_pos);

		    /* Get pixel color values depending on 
		     * bits_per_pixel ON FILE.
		     */
		    switch(td->bits_per_pixel)
		    {
		      case 32:
			pix[0] = (u_int8_t)*data_ptr++;
			pix[1] = (u_int8_t)*data_ptr++;
			pix[2] = (u_int8_t)*data_ptr++;
			pix[3] = (u_int8_t)*data_ptr++;
			*data_ptr16 = PACK8TO16(pix[2], pix[1], pix[0]);
			break;

		      case 8:
			pix[0] = (u_int8_t)*data_ptr++;
			*data_ptr16 = PACK8TO16(pix[0], pix[0], pix[0]);
			break;

		      default:  /* Default to 24-bits */
			pix[0] = (u_int8_t)*data_ptr++;
			pix[1] = (u_int8_t)*data_ptr++;
			pix[2] = (u_int8_t)*data_ptr++;
			*data_ptr16 = PACK8TO16(pix[2], pix[1], pix[0]);
			break;
		    }
		    /* `Almost' black pixel roundoff check */
		    if(*data_ptr16 == 0x0000)
		    {
			if(pix[0] || pix[1] || pix[2])
			    *data_ptr16 = ALMOSTBLACKPIX16;
		    }

		    /* Increment data position in destination buffer */
		    data_pos += BYTES_PER_PIXEL16;


		    /* Increment colum_count */
		    colum_count++;

		    if(colum_count >= (int)td->width)
		    {
			colum_count = 0;
			row_count++;
			data_pos = row_count * (int)td->width * BYTES_PER_PIXEL16;
		    }
		}
		break;

	      /* 15 bits */
	      case 15:
		while((fpos < (int)td->file_size) &&
		      (row_count < (int)td->height)
		)
		{
		    /* Get pointer in destination buffer */
		    data_ptr16 = (u_int16_t *)(td->data + data_pos);

		    /* Get pixel color values depending on
		     * bits_per_pixel ON FILE.
		     */
		    switch(td->bits_per_pixel)
		    {
		      case 32:
			pix[0] = (u_int8_t)*data_ptr++;
			pix[1] = (u_int8_t)*data_ptr++;
			pix[2] = (u_int8_t)*data_ptr++;
			pix[3] = (u_int8_t)*data_ptr++;
			*data_ptr16 = PACK8TO15(pix[2], pix[1], pix[0]);
			break;

		      case 8:
			pix[0] = (u_int8_t)*data_ptr++;
			*data_ptr16 = PACK8TO15(pix[0], pix[0], pix[0]);
			break;

		      default:  /* Default to 24-bits */
			pix[0] = (u_int8_t)*data_ptr++;
			pix[1] = (u_int8_t)*data_ptr++;
			pix[2] = (u_int8_t)*data_ptr++;
			*data_ptr16 = PACK8TO15(pix[2], pix[1], pix[0]);
			break;
		    }
		    /* `Almost' black pixel roundoff check */
		    if(*data_ptr16 == 0x0000)
		    {
			if(pix[0] || pix[1] || pix[2])
			    *data_ptr16 = ALMOSTBLACKPIX15;
		    }

		    /* Increment data position in destination buffer */
		    data_pos += BYTES_PER_PIXEL15;

		    /* Increment colum_count */
		    colum_count++;

		    if(colum_count >= (int)td->width)
		    {
			colum_count = 0;
			row_count++;
			data_pos = row_count * (int)td->width * BYTES_PER_PIXEL15;
		    }
		}
		break;

	      /* 8 bits */
	      case 8:
		while((fpos < (int)td->file_size) &&
		      (row_count < (int)td->height)
		)
		{
		    /* Get pointer in destination buffer */
		    data_ptr8 = (u_int8_t *)(td->data + data_pos);

		    /* Get pixel color values depending on
		     * bits_per_pixel ON FILE.
		     */
		    switch(td->bits_per_pixel)
		    {
		      case 32:
			r = (u_int8_t)*data_ptr++;
			pix[0] = TgaDitherRedPixel8(r, colum_count, row_count);
			g = (u_int8_t)*data_ptr++;
			pix[1] = TgaDitherGreenPixel8(g, colum_count, row_count);
			b = (u_int8_t)*data_ptr++;
			pix[2] = TgaDitherBluePixel8(b, colum_count, row_count);
			pix[3] = (u_int8_t)*data_ptr++;
			*data_ptr8 = PACK8TO8(pix[2], pix[1], pix[0]);
			break;

		      case 8:
			pix[0] = (u_int8_t)*data_ptr++;
			*data_ptr8 = PACK8TO8(pix[0], pix[0], pix[0]);
			break;

		      default:  /* Default to 24-bits */
			r = (u_int8_t)*data_ptr++;
			pix[0] = TgaDitherRedPixel8(r, colum_count, row_count);
			g = (u_int8_t)*data_ptr++;
			pix[1] = TgaDitherGreenPixel8(g, colum_count, row_count);
			b = (u_int8_t)*data_ptr++;
			pix[2] = TgaDitherBluePixel8(b, colum_count, row_count);
			*data_ptr8 = PACK8TO8(pix[2], pix[1], pix[0]);
			break;
		    }
		    /* `Almost' black pixel roundoff check */
		    if(*data_ptr8 == 0x00)
		    {
			if(pix[0] || pix[1] || pix[2])
			    *data_ptr8 = ALMOSTBLACKPIX8;
		    }

		    /* Increment data position in destination buffer */
		    data_pos += BYTES_PER_PIXEL8;

		    /* Increment colum_count */
		    colum_count++;

		    if(colum_count >= (int)td->width)
		    {
			colum_count = 0;
			row_count++;
			data_pos = row_count * (int)td->width * BYTES_PER_PIXEL8;
		    }
		}
		break;

	    }
	}
	else
	{
	    /* READ UPSIDE DOWN */

	    pix_total = td->width * td->height;
	    colum_count = 0;
	    row_count = (int)td->height - 1;
	    data_pos = row_count * td->width * bytes_per_pixel;

	    /* Check which bit padding to read to */
	    switch(td->data_depth)
	    {
	      /* 32 bits */
	      case 32:
		while((fpos < (int)td->file_size) &&
		      (row_count >= 0)
		)
		{
		    /* Get pointer in destination buffer */
		    data_ptr32 = (u_int32_t *)(td->data + data_pos);

		    /* Get pixel color values depending on
		     * bits_per_pixel ON FILE.
		     */
		    switch(td->bits_per_pixel)
		    { 
		      case 32:
			pix[0] = (u_int8_t)*data_ptr++;
			pix[1] = (u_int8_t)*data_ptr++;
			pix[2] = (u_int8_t)*data_ptr++;
			pix[3] = (u_int8_t)*data_ptr++;
			*data_ptr32 = PACK8TO32(pix[3], pix[2], pix[1], pix[0]);
			break;

		      case 8:
			pix[0] = (u_int8_t)*data_ptr++;
			*data_ptr32 = PACK8TO32(0xff, pix[0], pix[0], pix[0]);
			break;

		      default:  /* Default to 24-bits */
			pix[0] = (u_int8_t)*data_ptr++;
			pix[1] = (u_int8_t)*data_ptr++;
			pix[2] = (u_int8_t)*data_ptr++;
			*data_ptr32 = PACK8TO32(0xff, pix[2], pix[1], pix[0]);
			break;
		    }

		    /* Increment data position in destination buffer */
		    data_pos += BYTES_PER_PIXEL32;

		    /* Increment colum_count */
		    colum_count++;

		    if(colum_count >= (int)td->width)
		    {
			colum_count = 0;
			row_count--;
			data_pos = row_count * (int)td->width * BYTES_PER_PIXEL32;
		    }
		}
		break;

	      /* 16 bits */
	      case 16:
		while((fpos < (int)td->file_size) &&
		      (row_count >= 0)
		)
		{       
		    /* Get pointer in destination buffer */
		    data_ptr16 = (u_int16_t *)(td->data + data_pos);

		    /* Get pixel color values depending on
		     * bits_per_pixel ON FILE. 
		     */
		    switch(td->bits_per_pixel)
		    {
		      case 32:
			pix[0] = (u_int8_t)*data_ptr++;
			pix[1] = (u_int8_t)*data_ptr++;
			pix[2] = (u_int8_t)*data_ptr++;
			pix[3] = (u_int8_t)*data_ptr++;
			*data_ptr16 = PACK8TO16(pix[2], pix[1], pix[0]);
			break;

		      case 8:
			pix[0] = (u_int8_t)*data_ptr++;
			*data_ptr16 = PACK8TO16(pix[0], pix[0], pix[0]);
			break;

		      default:  /* Default to 24-bits */
			pix[0] = (u_int8_t)*data_ptr++;
			pix[1] = (u_int8_t)*data_ptr++;
			pix[2] = (u_int8_t)*data_ptr++;
			*data_ptr16 = PACK8TO16(pix[2], pix[1], pix[0]);
			break;
		    }
		    /* `Almost' black pixel roundoff check */
		    if(*data_ptr16 == 0x0000)
		    {
			if(pix[0] || pix[1] || pix[2])
			    *data_ptr16 = ALMOSTBLACKPIX16;
		    }

		    /* Increment data position in destination buffer */
		    data_pos += BYTES_PER_PIXEL16;

		    /* Increment colum_count */
		    colum_count++;
			
		    if(colum_count >= (int)td->width)
		    {
			colum_count = 0;
			row_count--;
			data_pos = row_count * (int)td->width * BYTES_PER_PIXEL16;
		    }
		}   
		break;

	      /* 15 bits */  
	      case 15:
		while((fpos < (int)td->file_size) &&   
		      (row_count >= 0)
		)
		{
		    /* Get pointer in destination buffer */
		    data_ptr16 = (u_int16_t *)(td->data + data_pos);

		    /* Get pixel color values depending on
		     * bits_per_pixel ON FILE.
		     */
		    switch(td->bits_per_pixel)
		    {
		      case 32:
			pix[0] = (u_int8_t)*data_ptr++;
			pix[1] = (u_int8_t)*data_ptr++;
			pix[2] = (u_int8_t)*data_ptr++;
			pix[3] = (u_int8_t)*data_ptr++;
			*data_ptr16 = PACK8TO15(pix[2], pix[1], pix[0]);
			break;

		      case 8:
			pix[0] = (u_int8_t)*data_ptr++;
			*data_ptr16 = PACK8TO15(pix[0], pix[0], pix[0]);
			break;

		      default:  /* Default to 24-bits */
			pix[0] = (u_int8_t)*data_ptr++;
			pix[1] = (u_int8_t)*data_ptr++;
			pix[2] = (u_int8_t)*data_ptr++;
			*data_ptr16 = PACK8TO15(pix[2], pix[1], pix[0]);
			break;
		    }
		    /* `Almost' black pixel roundoff check */
		    if(*data_ptr16 == 0x0000)
		    {
			if(pix[0] || pix[1] || pix[2])
			    *data_ptr16 = ALMOSTBLACKPIX15;
		    }

		    /* Increment data position in destination buffer */
		    data_pos += BYTES_PER_PIXEL15;

		    /* Increment colum_count */
		    colum_count++;

		    if(colum_count >= (int)td->width)
		    {
			colum_count = 0;
			row_count--;
			data_pos = row_count * (int)td->width * BYTES_PER_PIXEL15;
		    }
		}
		break;

	      /* 8 bits */
	      case 8:
		while((fpos < (int)td->file_size) &&
		      (row_count >= 0)
		)
		{
		    /* Get pointer in destination buffer */
		    data_ptr8 = (u_int8_t *)(td->data + data_pos);

		    /*   Get pixel color values depending on
		     *   bits_per_pixel ON FILE.
		     */
		    switch(td->bits_per_pixel)
		    {
		      case 32:
			r = (u_int8_t)*data_ptr++;
			pix[0] = TgaDitherRedPixel8(r, colum_count, row_count);
			g = (u_int8_t)*data_ptr++;
			pix[1] = TgaDitherGreenPixel8(g, colum_count, row_count);
			b = (u_int8_t)*data_ptr++;
			pix[2] = TgaDitherBluePixel8(b, colum_count, row_count);
			pix[3] = (u_int8_t)*data_ptr++;
			*data_ptr8 = PACK8TO8(pix[2], pix[1], pix[0]);
			break;

		      case 8:
			pix[0] = (u_int8_t)*data_ptr++;
			*data_ptr8 = PACK8TO8(pix[0], pix[0], pix[0]);
			break;

		      default:  /* Default to 24-bits */
			r = (u_int8_t)*data_ptr++;
			pix[0] = TgaDitherRedPixel8(r, colum_count, row_count);
			g = (u_int8_t)*data_ptr++;
			pix[1] = TgaDitherGreenPixel8(g, colum_count, row_count);
			b = (u_int8_t)*data_ptr++;
			pix[2] = TgaDitherBluePixel8(b, colum_count, row_count);
			*data_ptr8 = PACK8TO8(pix[2], pix[1], pix[0]);
			break;
		    }
		    /* `Almost' black pixel roundoff check */
		    if(*data_ptr8 == 0x00)
		    {
			if(pix[0] || pix[1] || pix[2])
			    *data_ptr8 = ALMOSTBLACKPIX8;
		    }

		    /* Increment data position in destination buffer */
		    data_pos += BYTES_PER_PIXEL8;


		    /* Increment colum_count */
		    colum_count++;
		       
		    if(colum_count >= (int)td->width)
		    {
			colum_count = 0;
			row_count--;
			data_pos = row_count * (int)td->width * BYTES_PER_PIXEL8;
		    }
		}
		break; 

	    }
	}

	return(TgaSuccess);
}

/*
 *	Sets up for async/partial reading of the tga image from file.
 *
 *	The depth specifies the bit depth of the loaded image data.
 *
 *	Once this call has been made, you may use
 *	TgaReadPartialFromFile() to begin reading the data.
 */
int TgaStartReadPartialFromFile(
	const char *filename,
	tga_data_struct *td,
	unsigned int depth
)
{
	int status;
	unsigned int bytes_per_pixel;
	char s[256];


	if((filename == NULL) || (td == NULL))
	    return(TgaBadValue);

	/* Check if the specified depth is supported */
	if((depth != 8) &&
	   (depth != 15) &&
	   (depth != 16) &&
	   (depth != 24) &&
	   (depth != 32)
	)
	{
	    sprintf(
		s,
		"Requested destination buffer depth %i is not supported.",
		depth
	    );
	    TgaReportError(filename, s, TgaErrorLevelCritical);
	    return(TgaBadValue);
	}
	/* Sanitize depth, if it is 24 bits then it should be 32 */
	if(depth == 24)
	    depth = 32;


	/* Read header */
	status = TgaReadHeaderFromFile(filename, td);
	if(status != TgaSuccess)
	    return(status);

	/* Open the file */
	td->fp = FOpen(filename, "rb");
	if(td->fp == NULL)
	    return(TgaNoAccess);


	/* Check if the depth of the image data on file is supported */
	if((td->depth != 24) &&
	   (td->depth != 32)
	)
	{
	    sprintf(
		s,
		"Image file depth %i is not supported.",
		td->depth
	    );
	    TgaReportError(filename, s, TgaErrorLevelCritical);
	    return(TgaBadValue);
	}

	/* Set data_depth to represent the depth of the image data
	 * that we are about to load
	 */
	td->data_depth = depth;		/* In bits per pixel */

	/* Calculate bytes per pixel */
	switch(td->data_depth)
	{  
	  case 24:
	    bytes_per_pixel = BYTES_PER_PIXEL24;
	    break;
	  case 15:
	    bytes_per_pixel = BYTES_PER_PIXEL15;
	    break;
	  default:
	    bytes_per_pixel = (td->data_depth >> 3);
	    break;
	}

	/* Allocate image data (need to use calloc to have a blank
	 * image at first)
	 */
	td->data = (u_int8_t *)calloc(
	    td->width * td->height * bytes_per_pixel,
	    sizeof(u_int8_t)
	);
	if(td->data == NULL)
	    return(TgaNoBuffers);

	/* Set the loaded pixel bookmark to 0 */
	td->cur_load_pixel = 0;


	return(TgaSuccess);
}

/*
 *	Reads a specified number of pixels from the tga image specified
 *	by td.
 *
 *	The td must be initialized by a prior call to
 *	TgaStartReadPartialFromFile().
 */
int TgaReadPartialFromFile(
	tga_data_struct *td,
	unsigned int depth,
	unsigned int n_pixels
)
{
	char s[256];
	FILE *fp;
	int	f_start = 0,	/* Position on file to start reading at */
		f_end = 0;	/* Position on file to stop reading at
				 * (do not read the byte at f_end) */
	int fpos, data_pos;
	unsigned int bytes_per_pixel;

	u_int8_t *data_ptr8;
	u_int16_t *data_ptr16;
	u_int32_t *data_ptr32;

	u_int8_t pix[4], r, g, b;

	int pix_total;
	int colum_count, row_count;


	if(td == NULL)
	    return(TgaBadValue);

	if((td->width == 0) || (td->height == 0))
	    return(TgaBadValue);

	/* Is td finished loading? */
	if((td->cur_load_pixel < 0) || (td->fp == NULL))
	    return(TgaSuccess);

	/* Nothing to load? */
	if(n_pixels == 0)
	    return(TgaSuccess);

	/* Buffer not allocated on td? */
	if(td->data == NULL)
	    return(TgaNoBuffers);

	/* Make sure the target depth for the target buffer is one that
	 * we support
	 */
	if((depth != 8) &&
	   (depth != 15) &&
	   (depth != 16) &&
	   (depth != 24) &&
	   (depth != 32)
	)
	{
	    sprintf(
		s,
		"Requested destination buffer depth %i is not supported.",
		depth
	    );
	    TgaReportError("loaded segment", s, TgaErrorLevelCritical);
	    return(TgaBadValue);
	}
	/* If the target depth is 24 bits per pixel, then it should
	 * be 32 bits per pixel
	 */
	if(depth == 24)
	    depth = 32;

	/* Calculate bytes per pixel */
	switch(td->data_depth)
	{
	  case 24:	/* Never reached */
	    bytes_per_pixel = BYTES_PER_PIXEL24;
	    break;
	  case 15:
	    bytes_per_pixel = BYTES_PER_PIXEL15;
	    break;
	  default:
	    bytes_per_pixel = (td->data_depth >> 3);
	    break;
	}

	/* Check if the depth of the source image is supported */
	if((td->depth != 24) &&
	   (td->depth != 32)
	)
	{
	    sprintf(
		s,
		"Image file depth %i is not supported.",
		td->depth
	    );
	    TgaReportError("loaded segment", s, TgaErrorLevelCritical);
	    return(TgaBadValue);
	}


	/* Ensure we read at least 2 lines */
	if(n_pixels < (unsigned int)(td->width * 2))
	    n_pixels = (unsigned int)(td->width * 2);

	/* Calculate file positions (we know that the source depth is
	 * valid)
	 */
	if(td->depth == 24)
	{
	    f_start = (TgaHeaderLength + (int)td->id_field_len +
		(int)td->cmap_size) + (td->cur_load_pixel * 3);
	    f_end = f_start + (n_pixels * 3);
	}
	else if(td->depth == 32)
	{
	    f_start = (TgaHeaderLength + (int)td->id_field_len +
		(int)td->cmap_size) + (td->cur_load_pixel * 4);
	    f_end = f_start + (n_pixels * 4);
	}
	fpos = f_start;

	fp = td->fp;

	/* Seek to the position that we want to start reading at */
	if(fseek(fp, f_start, SEEK_SET))
	    return(TgaBadValue);


	/* Begin reading the image data from file */

	/* Check which encoding style the image data on file is in */
	if(td->flags & TgaImageFliped)
	{
	    /* READ RIGHTSIDE UP */

	    pix_total = td->width * td->height;
	    colum_count = (int)td->cur_load_pixel % (int)td->width;
	    row_count = (int)td->cur_load_pixel / (int)td->width;
	    data_pos = ((row_count * (int)td->width) + colum_count) *
		       bytes_per_pixel;

	    /* Read by DESTINATION buffer depth */
	    switch(td->data_depth)
	    {
	      /* 32 bits */
	      case 32:
		while((fpos < (int)td->file_size) &&
		      (row_count < (int)td->height) &&
		      (fpos < f_end)	/* Read limited number of bytes */
		)
		{
		    /* Get pointer in destination buffer */
		    data_ptr32 = (u_int32_t *)(&td->data[data_pos]);

		    /*   Get pixel color values depending on 
		     *   bits_per_pixel ON FILE.
		     */
		    switch(td->bits_per_pixel)
		    {
		      case 32:
			pix[0] = (u_int8_t)fgetc(fp);
			pix[1] = (u_int8_t)fgetc(fp);
			pix[2] = (u_int8_t)fgetc(fp);
			pix[3] = (u_int8_t)fgetc(fp);
			*data_ptr32 = PACK8TO32(pix[3], pix[2], pix[1], pix[0]);
			fpos += 4;
			break;

		      default:  /* Default to 24-bits */
			pix[0] = (u_int8_t)fgetc(fp);
			pix[1] = (u_int8_t)fgetc(fp);
			pix[2] = (u_int8_t)fgetc(fp);
			*data_ptr32 = PACK8TO32(0xff, pix[2], pix[1], pix[0]);
			fpos += 3;
			break;
		    }

		    /* Increment data position in destination buffer */
		    data_pos += BYTES_PER_PIXEL32;

		    /* Increment colum_count */
		    colum_count++;

		    if(colum_count >= (int)td->width)
		    {
			colum_count = 0;
			row_count++;
			data_pos = row_count * td->width * BYTES_PER_PIXEL32;
		    }
		}
		break;

	      /* 16 bits */
	      case 16:
		while((fpos < (int)td->file_size) &&
		      (row_count < (int)td->height) &&
		      (fpos < f_end)    /* Read limited number of bytes */
		)
		{
		    /* Get pointer in destination buffer */
		    data_ptr16 = (u_int16_t *)(&td->data[data_pos]);

		    /*   Get pixel color values depending on 
		     *   bits_per_pixel ON FILE.
		     */
		    switch(td->bits_per_pixel)
		    {
		      case 32:
			pix[0] = (u_int8_t)fgetc(fp);
			pix[1] = (u_int8_t)fgetc(fp);
			pix[2] = (u_int8_t)fgetc(fp);
			pix[3] = (u_int8_t)fgetc(fp);
			*data_ptr16 = PACK8TO16(pix[2], pix[1], pix[0]);
			fpos += 4;
			break;

		      default:  /* Default to 24-bits */
			pix[0] = (u_int8_t)fgetc(fp);
			pix[1] = (u_int8_t)fgetc(fp);
			pix[2] = (u_int8_t)fgetc(fp);
			*data_ptr16 = PACK8TO16(pix[2], pix[1], pix[0]);
			fpos += 3;
			break;
		    }
		    /* `Almost' black pixel roundoff check */
		    if(*data_ptr16 == 0x0000)
		    {
			if(pix[0] || pix[1] || pix[2])
			    *data_ptr16 = ALMOSTBLACKPIX16;
		    }

		    /* Increment data position in destination buffer */
		    data_pos += BYTES_PER_PIXEL16;

		    /* Increment colum_count */
		    colum_count++;

		    if(colum_count >= (int)td->width)
		    {
			colum_count = 0;
			row_count++;
			data_pos = row_count * td->width * BYTES_PER_PIXEL16;
		    }
		}
		break;

	      /* 15 bits */
	      case 15:
		while((fpos < (int)td->file_size) &&
		      (row_count < (int)td->height) &&
		      (fpos < f_end)    /* Read limited number of bytes */
		)       
		{
		    /* Get pointer in destination buffer */
		    data_ptr16 = (u_int16_t *)(&td->data[data_pos]);

		    /*   Get pixel color values depending on
		     *   bits_per_pixel ON FILE.
		     */
		    switch(td->bits_per_pixel)
		    {
		      case 32:
			pix[0] = (u_int8_t)fgetc(fp);
			pix[1] = (u_int8_t)fgetc(fp);
			pix[2] = (u_int8_t)fgetc(fp);
			pix[3] = (u_int8_t)fgetc(fp);
			*data_ptr16 = PACK8TO15(pix[2], pix[1], pix[0]);
			fpos += 4;
			break;

		      default:  /* Default to 24-bits */
			pix[0] = (u_int8_t)fgetc(fp);
			pix[1] = (u_int8_t)fgetc(fp);
			pix[2] = (u_int8_t)fgetc(fp);
			*data_ptr16 = PACK8TO15(pix[2], pix[1], pix[0]);
			fpos += 3;
			break;
		    }
		    /* `Almost' black pixel roundoff check */
		    if(*data_ptr16 == 0x0000)
		    {
			if(pix[0] || pix[1] || pix[2])
			    *data_ptr16 = ALMOSTBLACKPIX15;
		    }

		    /* Increment data position in destination buffer */
		    data_pos += BYTES_PER_PIXEL15;

		    /* Increment colum_count */
		    colum_count++;

		    if(colum_count >= (int)td->width)
		    {
			colum_count = 0;
			row_count++;
			data_pos = row_count * td->width * BYTES_PER_PIXEL15;
		    }
		}
		break;

	      /* 8 bits */
	      case 8:
		while((fpos < (int)td->file_size) &&
		      (row_count < (int)td->height) &&
		      (fpos < f_end)    /* Read limited number of bytes */
		)
		{
		    /* Get pointer in destination buffer */
		    data_ptr8 = (u_int8_t *)(&td->data[data_pos]);

		    /*   Get pixel color values depending on
		     *   bits_per_pixel ON FILE.
		     */
		    switch(td->bits_per_pixel)
		    {
		      case 32:
			r = (u_int8_t)fgetc(fp);
			pix[0] = TgaDitherRedPixel8(r, colum_count, row_count);
			g = (u_int8_t)fgetc(fp);
			pix[1] = TgaDitherGreenPixel8(g, colum_count, row_count);
			b = (u_int8_t)fgetc(fp);
			pix[2] = TgaDitherBluePixel8(b, colum_count, row_count);
			pix[3] = (u_int8_t)fgetc(fp);
			*data_ptr8 = PACK8TO8(pix[2], pix[1], pix[0]);
			fpos += 4;
			break;

		      default:  /* Default to 24-bits */
			r = (u_int8_t)fgetc(fp);
			pix[0] = TgaDitherRedPixel8(r, colum_count, row_count);
			g = (u_int8_t)fgetc(fp);
			pix[1] = TgaDitherGreenPixel8(g, colum_count, row_count);
			b = (u_int8_t)fgetc(fp);
			pix[2] = TgaDitherBluePixel8(b, colum_count, row_count);
			*data_ptr8 = PACK8TO8(pix[2], pix[1], pix[0]);
			fpos += 3;
			break;
		    }
		    /* `Almost' black pixel roundoff check */
		    if(*data_ptr8 == 0x00)
		    {
			if(pix[0] || pix[1] || pix[2])
			    *data_ptr8 = ALMOSTBLACKPIX8;
		    }

		    /* Increment data position in destination buffer */
		    data_pos += BYTES_PER_PIXEL8;

		    /* Increment colum_count */
		    colum_count++;

		    if(colum_count >= (int)td->width)
		    {
			colum_count = 0;
			row_count++;
			data_pos = row_count * td->width * BYTES_PER_PIXEL8;
		    }
		}
		break;
	    }
	}
	else
	{
	    /* READ UPSIDE DOWN */

	    pix_total = td->width * td->height;
	    colum_count = (int)td->cur_load_pixel % (int)td->width;
	    row_count = MAX(
		(int)td->height - 1 -
		((int)td->cur_load_pixel / (int)td->width),
		0
	    );
	    /* Go one line down to make sure line gets loaded */
	    row_count = MIN(row_count, ((int)td->height - 1));

	    data_pos = ((row_count * (int)td->width) + colum_count) *
		       bytes_per_pixel;

	    /* Read by DESTINATION buffer depth */
	    switch(td->data_depth)
	    {
	      /* 32 bits */
	      case 32:
		while((fpos < (int)td->file_size) &&
		      (row_count >= 0) &&
		      (fpos < f_end)    /* Read limited number of bytes */
		)
		{
		    /* Get pointer in destination buffer */
		    data_ptr32 = (u_int32_t *)(&td->data[data_pos]);

		    /*   Get pixel color values depending on
		     *   bits_per_pixel ON FILE.
		     */
		    switch(td->bits_per_pixel)
		    { 
		      case 32:
			pix[0] = (u_int8_t)fgetc(fp);
			pix[1] = (u_int8_t)fgetc(fp);
			pix[2] = (u_int8_t)fgetc(fp);
			pix[3] = (u_int8_t)fgetc(fp);
			*data_ptr32 = PACK8TO32(pix[3], pix[2], pix[1], pix[0]);
			fpos += 4;
			break;

		      default:  /* Default to 24-bits */
			pix[0] = (u_int8_t)fgetc(fp);
			pix[1] = (u_int8_t)fgetc(fp);
			pix[2] = (u_int8_t)fgetc(fp);
			*data_ptr32 = PACK8TO32(0xff, pix[2], pix[1], pix[0]);
			fpos += 3;
			break;
		    }

		    /* Increment data position in destination buffer */
		    data_pos += BYTES_PER_PIXEL32;

		    /* Increment colum_count */
		    colum_count++;

		    if(colum_count >= (int)td->width)
		    {
			colum_count = 0;
			row_count--;
			data_pos = row_count * td->width * BYTES_PER_PIXEL32;
		    }
		}
		break;

	      /* 16 bits */
	      case 16:
		while((fpos < td->file_size) &&
		      (row_count >= 0) &&
		      (fpos < f_end)    /* Read limited number of bytes */
		)
		{
		    /* Get pointer in destination buffer */
		    data_ptr16 = (u_int16_t *)(&td->data[data_pos]);

		    /*   Get pixel color values depending on
		     *   bits_per_pixel ON FILE. 
		     */
		    switch(td->bits_per_pixel)
		    {
		      case 32:
			pix[0] = (u_int8_t)fgetc(fp);
			pix[1] = (u_int8_t)fgetc(fp);
			pix[2] = (u_int8_t)fgetc(fp);
			pix[3] = (u_int8_t)fgetc(fp);
			*data_ptr16 = PACK8TO16(pix[2], pix[1], pix[0]);
			fpos += 4;
			break;
		    
		      default:  /* Default to 24-bits */
			pix[0] = (u_int8_t)fgetc(fp);
			pix[1] = (u_int8_t)fgetc(fp);
			pix[2] = (u_int8_t)fgetc(fp);
			*data_ptr16 = PACK8TO16(pix[2], pix[1], pix[0]);
			fpos += 3;
			break;
		    } 
		    /* `Almost' black pixel roundoff check */
		    if(*data_ptr16 == 0x0000)
		    {
			if(pix[0] || pix[1] || pix[2])
			    *data_ptr16 = ALMOSTBLACKPIX16;
		    }

		    /* Increment data position in destination buffer */
		    data_pos += BYTES_PER_PIXEL16;

		    /* Increment colum_count */
		    colum_count++;

		    if(colum_count >= (int)td->width)
		    {
			colum_count = 0;
			row_count--;
			data_pos = row_count * td->width * BYTES_PER_PIXEL16;
		    }
		}
		break;

	      /* 15 bits */
	      case 15:
		while((fpos < td->file_size) &&
		      (row_count >= 0) &&
		      (fpos < f_end)    /* Read limited number of bytes */
		)
		{
		    /* Get pointer in destination buffer */
		    data_ptr16 = (u_int16_t *)(&td->data[data_pos]);

		    /*   Get pixel color values depending on
		     *   bits_per_pixel ON FILE.
		     */
		    switch(td->bits_per_pixel)
		    {
		      case 32:
			pix[0] = (u_int8_t)fgetc(fp);
			pix[1] = (u_int8_t)fgetc(fp);
			pix[2] = (u_int8_t)fgetc(fp);
			pix[3] = (u_int8_t)fgetc(fp);
			*data_ptr16 = PACK8TO15(pix[2], pix[1], pix[0]);
			fpos += 4;
			break;
			
		      default:  /* Default to 24-bits */
			pix[0] = (u_int8_t)fgetc(fp);
			pix[1] = (u_int8_t)fgetc(fp);
			pix[2] = (u_int8_t)fgetc(fp);
			*data_ptr16 = PACK8TO15(pix[2], pix[1], pix[0]);
			fpos += 3;
			break;
		    }
		    /* `Almost' black pixel roundoff check */
		    if(*data_ptr16 == 0x0000)
		    {
			if(pix[0] || pix[1] || pix[2])
			    *data_ptr16 = ALMOSTBLACKPIX15;
		    }

		    /* Increment data position in destination buffer */
		    data_pos += BYTES_PER_PIXEL15;

		    /* Increment colum_count */
		    colum_count++;

		    if(colum_count >= (int)td->width)
		    {
			colum_count = 0;
			row_count--;
			data_pos = row_count * td->width * BYTES_PER_PIXEL15;
		    }
		}
		break;

	      /* 8 bits */
	      case 8:
		while((fpos < (int)td->file_size) &&
		      (row_count >= 0) &&
		      (fpos < f_end)    /* Read limited number of bytes */
		)
		{
		    /* Get pointer in destination buffer */
		    data_ptr8 = (u_int8_t *)(&td->data[data_pos]);
			
		    /*   Get pixel color values depending on
		     *   bits_per_pixel ON FILE.
		     */
		    switch(td->bits_per_pixel)
		    {
		      case 32:
			r = (u_int8_t)fgetc(fp);
			pix[0] = TgaDitherRedPixel8(r, colum_count, row_count);
			g = (u_int8_t)fgetc(fp);
			pix[1] = TgaDitherGreenPixel8(g, colum_count, row_count);
			b = (u_int8_t)fgetc(fp);
			pix[2] = TgaDitherBluePixel8(b, colum_count, row_count);
			pix[3] = (u_int8_t)fgetc(fp);
			*data_ptr8 = PACK8TO8(pix[2], pix[1], pix[0]);
			fpos += 4;
			break;

		      default:  /* Default to 24-bits */
			r = (u_int8_t)fgetc(fp);
			pix[0] = TgaDitherRedPixel8(r, colum_count, row_count);
			g = (u_int8_t)fgetc(fp);
			pix[1] = TgaDitherGreenPixel8(g, colum_count, row_count);
			b = (u_int8_t)fgetc(fp);
			pix[2] = TgaDitherBluePixel8(b, colum_count, row_count);
			*data_ptr8 = PACK8TO8(pix[2], pix[1], pix[0]);
			fpos += 3;
			break;
		    }
		    /* `Almost' black pixel roundoff check */
		    if(*data_ptr8 == 0x00)
		    {
			if(pix[0] || pix[1] || pix[2])
			    *data_ptr8 = ALMOSTBLACKPIX8;
		    }

		    /* Increment data position in destination buffer */
		    data_pos += BYTES_PER_PIXEL8;

		    /* Increment colum_count */
		    colum_count++;

		    if(colum_count >= (int)td->width)
		    {
			colum_count = 0;
			row_count--;
			data_pos = row_count * td->width * BYTES_PER_PIXEL8;
		    }
		}
		break;
	    }
	}


	/* Update cur_load_pixel */
	td->cur_load_pixel += n_pixels;

	/* Done loading? */
	if((int)(td->cur_load_pixel >= (int)(td->width * td->height)) ||
	   (fpos >= (int)td->file_size)
	)
	{
	    td->cur_load_pixel = -1;

	    /* Close the file */
	    if(td->fp != NULL)
	    {
		FClose(td->fp);
		td->fp = NULL;
	    }
	}

	return(TgaSuccess);
}


/*
 *	Reads the tga image from the specified file using the "fast"
 *	method.
 *
 *	The image data will be in RGBA format.
 */
u_int8_t *TgaReadFromFileFastRGBA(
	const char *filename,
	int *rtn_width, int *rtn_height,
	u_int32_t transparent_pixel	/* RGBA of transparent pixel */
)
{
	FILE *fp;
	unsigned int bytes_per_pixel;
	int src_len, tar_len;
	u_int8_t *src_data, *tar_data;
	tga_data_struct tga_data, *td = &tga_data;
	char s[256];

	if(filename == NULL)
	    return(NULL);

	/* Read header */
	if(TgaReadHeaderFromFile(filename, td) != TgaSuccess)
	{
	    TgaDestroyData(td);
	    return(NULL);
	}

	/* Do we support the depth of the image on file? */
	if((td->depth != 8) &&
	   (td->depth != 24) &&
	   (td->depth != 32)
	)
	{
	    sprintf(
		s,
		"Image file depth %i is not supported.",
		td->depth
	    );
	    TgaReportError(filename, s, TgaErrorLevelCritical);
	    TgaDestroyData(td);
	    return(NULL);
	}

	/* Open file for reading */
	fp = FOpen(filename, "rb");
	if(fp == NULL)
	{
	    TgaDestroyData(td);
	    return(NULL);
	}

	/* Get data_depth since it represents the loaded data in
	 * bytes per pixel in memory
	 *
	 * Note, do not confuse this with bytes_per_pixel which
	 * indicates bytes per pixel on file
	 */
	td->data_depth = 32;	/* In bits per pixel */
	switch(td->data_depth)
	{
	  case 24:
	    bytes_per_pixel = BYTES_PER_PIXEL24;
	    break;
	  case 15:
	    bytes_per_pixel = BYTES_PER_PIXEL15;
	    break;
	  default:
	    bytes_per_pixel = (td->data_depth >> 3);
	    break;
	}

	/* Calculate length for buffer used to read the data from
	 * file unparsed
	 */
	src_len = td->width * td->height * (td->bits_per_pixel >> 3);
	if(src_len <= 0)
	{
	    TgaDestroyData(td);
	    FClose(fp);
	    return(NULL);
	}
	/* Allocate source image data buffer */
	src_data = (u_int8_t *)malloc(src_len);
	if(src_data == NULL)
	{
	    TgaDestroyData(td);
	    FClose(fp);
	    return(NULL);
	}

	/* Seek to start of image data */
	if(fseek(
	    fp,
	    TgaHeaderLength + (long)td->id_field_len + (long)td->cmap_size,
	    SEEK_SET
	))
	{
	    TgaDestroyData(td);
	    free(src_data);
	    FClose(fp);
	    return(NULL);
	}

	/* Read image data */
	if(fread(src_data, sizeof(u_int8_t), src_len, fp) <= 0)
	{
	    TgaDestroyData(td);
	    free(src_data);
	    FClose(fp);
	    return(NULL);
	}

	/* Close file, it is no longer needed */
	FClose(fp);

	/* Calculate length for target image data */
	tar_len = td->width * td->height * bytes_per_pixel;
	if(tar_len <= 0)
	{
	    TgaDestroyData(td);
	    free(src_data);
	    return(NULL);
	}
	/* Allocate target image data buffer */
	tar_data = (u_int8_t *)malloc(tar_len);
	if(tar_data == NULL)
	{
	    TgaDestroyData(td);
	    free(src_data);
	    return(NULL);
	}


	/* Parse/copy source image data to the target image data */

	/* Check which encoding style the image data on file is in */
	if(td->flags & TgaImageFliped)
	{
	    /* Rightside up */
	    int width = td->width, height = td->height;
	    int src_bpp = (td->bits_per_pixel >> 3);
	    int src_bpl = width * src_bpp;
	    int tar_bpp = bytes_per_pixel;
	    int tar_bpl = width * tar_bpp;
	    const u_int8_t *src_line, *src_line_end, *src_ptr, *src_end;
	    u_int8_t *tar_line, *tar_ptr;

	    for(src_line	= src_data,
		src_line_end	= src_line + (src_bpl * height),
		tar_line	= tar_data;
		src_line < src_line_end;
		src_line	+= src_bpl,
		tar_line	+= tar_bpl
	    )
	    {
		src_ptr = src_line;
		src_end = src_ptr + (width * src_bpp);
		tar_ptr = tar_line;

		while(src_ptr < src_end)
		{
		    /* Handle by source bytes per pixel */
		    switch(src_bpp)
		    {
		      case 4:
			*tar_ptr++ = src_ptr[2];
			*tar_ptr++ = src_ptr[1];
			*tar_ptr++ = src_ptr[0];
			*tar_ptr++ = src_ptr[3];
			break;
		      case 3:
			*tar_ptr++ = src_ptr[2];
			*tar_ptr++ = src_ptr[1];
			*tar_ptr++ = src_ptr[0];
			*tar_ptr++ = (
			   (src_ptr[0] != 0x00) ||
			   (src_ptr[1] != 0x00) ||
			   (src_ptr[2] != 0x00)
			) ? 0xff : 0x00;
			break;
		      case 1:
			*tar_ptr++ = *src_ptr;
			*tar_ptr++ = *src_ptr;
			*tar_ptr++ = *src_ptr;
			*tar_ptr++ = (*src_ptr != 0x00) ? 0xff : 0x00;
			break;
		    }
		    src_ptr += src_bpp;
		}
	    }
	}
	else
	{
	    /* Inverted */
	    int width = td->width, height = td->height;
	    int src_bpp = (td->bits_per_pixel >> 3);
	    int src_bpl = width * src_bpp;
	    int tar_bpp = bytes_per_pixel;
	    int tar_bpl = width * tar_bpp;
	    const u_int8_t *src_line, *src_line_end, *src_ptr, *src_end;
	    u_int8_t *tar_line, *tar_ptr;

	    for(src_line	= src_data,   
		src_line_end	= src_line + (src_bpl * height),
		tar_line	= tar_data + (tar_bpl * (height - 1));
		src_line < src_line_end;
		src_line        += src_bpl,
		tar_line        -= tar_bpl
	    )
	    {
		src_ptr = src_line;
		src_end = src_ptr + (width * src_bpp);
		tar_ptr = tar_line;

		while(src_ptr < src_end)
		{
		    /* Handle by source bytes per pixel */   
		    switch(src_bpp)
		    {
		      case 4:
			*tar_ptr++ = src_ptr[2];
			*tar_ptr++ = src_ptr[1];
			*tar_ptr++ = src_ptr[0];
			*tar_ptr++ = src_ptr[3];
			break;
		      case 3:
			*tar_ptr++ = src_ptr[2];
			*tar_ptr++ = src_ptr[1];                             
			*tar_ptr++ = src_ptr[0];
			*tar_ptr++ = (
			   (src_ptr[0] != 0x00) ||
			   (src_ptr[1] != 0x00) ||
			   (src_ptr[2] != 0x00)
			) ? 0xff : 0x00;
			break;
		      case 1:
			*tar_ptr++ = *src_ptr;
			*tar_ptr++ = *src_ptr;
			*tar_ptr++ = *src_ptr;
			*tar_ptr++ = (*src_ptr != 0x00) ? 0xff : 0x00;
			break;
		    }
		    src_ptr += src_bpp;
		}
	    }
	}

	/* Update returns */
	if(rtn_width != NULL)
	    *rtn_width = td->width;
	if(rtn_height != NULL)
	    *rtn_height = td->height;

	TgaDestroyData(td);
	free(src_data);

	return(tar_data);
}


/*
 *	Writes the tga image to file.
 */
int TgaWriteToFile(
	const char *filename,
	tga_data_struct *td,
	unsigned int depth
)
{
	int i, fpos, fend, total_pixels;
	FILE *fp;

	u_int8_t *img_ptr8;
	u_int16_t *img_ptr16;
	u_int32_t *img_ptr32;

	if((filename == NULL) || (td == NULL))
	    return(TgaBadValue);

	if((*filename == '\0') || (td->data == NULL))
	    return(TgaBadValue);

	/* Make sure target depth is supported */
	if((depth != 8) &&
	   (depth != 24) &&
	   (depth != 32)
	)
	    return(TgaBadValue);

	/* Open file for writing */
	fp = FOpen(filename, "wb");
	if(fp == NULL)
	    return(TgaNoAccess);

	/* Begin writing header */
	fend = TgaHeaderLength;
	for(fpos = 0; fpos < fend; fpos++)
	{
	    /* ID Field Length */
	    if(fpos == 0x00)
	    {
		/* Need to update the ID Field Length if there are
		 * comments
		 */
		if(td->comments != NULL)
		    td->id_field_len = MIN(strlen(td->comments), 255);
		else
		    td->id_field_len = 0;
		fputc(td->id_field_len, fp);
	    }
	    /* Colormap Type */
	    else if(fpos == 0x01)
	    {
		fputc(TgaColormapNone, fp);
	    }
	    /* Image Data Type */
	    else if(fpos == 0x02)
	    {
	        fputc(TgaImageDataURGB, fp);
	    }
	    /* Width */
	    else if(fpos == 0x0c)
	    {
		fputc(
		    ((u_int32_t)td->width & 0xff),
		    fp
		);

		fpos++;
		if(fpos < (TgaHeaderLength + td->id_field_len))
		    fputc(
		        (((u_int32_t)td->width & 0xff00) >> 8),
			fp
		    );
	    }
	    /* Height */
	    else if(fpos == 0x0e)
	    {
		fputc(
		    ((u_int32_t)td->height & 0xff),
		    fp
		);

		fpos++;
		if(fpos < (TgaHeaderLength + td->id_field_len))
		    fputc(
			(((u_int32_t)td->height & 0xff00) >> 8),
			fp
		    );
	    }
	    /* Depth */
	    else if(fpos == 0x10)
	    {
		/* Write target depth */
		fputc((u_int8_t)depth, fp);
	    }
	    /* Descriptor Flags */
	    else if(fpos == 0x11)
	    {
		tga_image_flags flags = TgaImageFliped;	/* Rightside up */
		fputc(flags, fp);
	    }         

	    else
	    {
		fputc(0x00, fp);
	    }
	}

	/* Begin writing ID field */
	if(td->id_field_len > 0)
	{
	    const char *s = td->comments;
	    for(fpos = 0,
		fend = td->id_field_len;
		fpos < fend;
		fpos++
	    )
		fputc(*s++, fp);
	}

	/* Begin writing image data */
	total_pixels = td->width * td->height;
	switch(td->data_depth)
	{
	  case 8:
	    for(i = 0, img_ptr8 = (u_int8_t *)td->data;
		i < total_pixels;
		i++, img_ptr8++
	    )
	    {
		/* Check TARGET depth */
		switch(depth)
		{
		  case 32:	/* 8 bits source to 32 bits target */
		    /* Blue */
		    fputc(
			(((*img_ptr8) & 0x03) << 6),
			fp
		    );
		    /* Green */
		    fputc(
			(((*img_ptr8) & 0x1c) << 3),
			fp
		    );
		    /* Red */
		    fputc(
			(((*img_ptr8) & 0xe0)),
			fp
		    );
		    /* Alpha */
		    fputc(0x00, fp);
		    break;

		  case 24:	/* 8 bits source to 24 bits target */
		    /* Blue */
		    fputc(
			(((*img_ptr8) & 0x03) << 6),
			fp
		    );
		    /* Green */
		    fputc(
			(((*img_ptr8) & 0x1c) << 3),
			fp
		    );
		    /* Red */
		    fputc(
			(((*img_ptr8) & 0xe0)),
			fp
		    );
		    break;

		  case 8:	/* 8 bits source to 8 bits target */
		    /* Grey */
		    fputc(
			*img_ptr8,
			fp
		    );
		    break;
		}
	    }
	    break;

	  case 15:
	    for(i = 0, img_ptr16 = (u_int16_t *)td->data;
		i < total_pixels;
		i++, img_ptr16++
	    )
	    {
		/* Check TARGET depth */
		switch(depth)
		{
		  case 32:	/* 15 bits source to 32 bits target */
		    /* Blue */
		    fputc(
			(((*img_ptr16) & 0x001f) << 3),
			fp
		    );
		    /* Green */
		    fputc(
			(((*img_ptr16) & 0x03e0) >> 2),
			fp
		    );
		    /* Red */
		    fputc(
			(((*img_ptr16) & 0x7c00) >> 7),
			fp
		    );
		    /* Alpha */
		    fputc(
			(((*img_ptr16) & 0x8000) >> 15),
			fp
		    );
		    break;

		  case 24:	/* 15 bits source to 24 bits target */
		    /* Blue */
		    fputc(
			 (((*img_ptr16) & 0x001f) << 3),
			fp
		    );
		    /* Green */
		    fputc(
			(((*img_ptr16) & 0x03e0) >> 2),
			fp
		    );
		    /* Red */
		    fputc(
			(((*img_ptr16) & 0x7c00) >> 7),
			fp
		    );
		    break;

		  case 8:	/* 15 bits source to 8 bits target */
		    /* Grey (use source red value) */
		    fputc(
			(((*img_ptr16) & 0x7c00) >> 7),
			fp
		    );
		    break;
		}
	    }
	    break;

	  case 16:
	    for(i = 0, img_ptr16 = (u_int16_t *)td->data;
		i < total_pixels;
		i++, img_ptr16++
	    )
	    {
		/* Check TARGET depth */
		switch(depth)
		{
		  case 32:	/* 16 bits source to 32 bits target */
		    /* Blue */
		    fputc(
			(((*img_ptr16) & 0x001f) << 3),
			fp
		    );
		    /* Green */
		    fputc(
			(((*img_ptr16) & 0x07e0) >> 3),
			fp
		    );
		    /* Red */
		    fputc(
			(((*img_ptr16) & 0xf800) >> 8),
			fp
		    );
		    /* Alpha */
		    fputc(0x00, fp);
		    break;

		  case 24:	/* 16 bits source to 24 bits target */
		    /* Blue */
		    fputc(
			(((*img_ptr16) & 0x001f) << 3),
			fp
		    );
		    /* Green */
		    fputc(
			(((*img_ptr16) & 0x07e0) >> 3),
			fp
		    );
		    /* Red */
		    fputc(
			(((*img_ptr16) & 0xf800) >> 8),
			fp
		    );
		    break;

		  case 8:	/* 16 bits source to 8 bits target */
		    /* Grey (use source red value) */
		    fputc(
			(((*img_ptr16) & 0xf800) >> 8),
			fp
		    );
		    break;
		}
	    }
	    break;

	  case 24:
	    for(i = 0, img_ptr32 = (u_int32_t *)td->data;
		i < total_pixels;
		i++, img_ptr32++
	    )
	    {
		/* Check TARGET depth */
		switch(depth) 
		{
		  case 32:	/* 24 bits source to 32 bits target */
		    /* Blue */
		    fputc(
			(((*img_ptr32) & 0x000000ff)),
			fp
		    );
		    /* Green */
		    fputc(
			(((*img_ptr32) & 0x0000ff00) >> 8),
			fp
		    );
		    /* Red */
		    fputc(
			(((*img_ptr32) & 0x00ff0000) >> 16),
			fp
		    );
		    /* Alpha */
		    fputc(0x00, fp);
		    break;

		  case 24:	/* 24 bits source to 24 bits target */
		    /* Blue */
		    fputc(
			(((*img_ptr32) & 0x000000ff)),
			fp
		    );
		    /* Green */
		    fputc(
			(((*img_ptr32) & 0x0000ff00) >> 8),
			fp
		    );
		    /* Red */
		    fputc(
			(((*img_ptr32) & 0x00ff0000) >> 16),
			fp
		    );
		    break;

		  case 8:	/* 24 bits source to 8 bits target */
		    /* Grey */
		    fputc(
			((((*img_ptr32) & 0x00ff0000) >> 16) +
			 (((*img_ptr32) & 0x0000ff00) >> 8) +
			 (((*img_ptr32) & 0x000000ff))
			) / 3,
			fp
		    );
		    break;

		}
	    }
	    break;  

	  case 32:
	    for(i = 0, img_ptr32 = (u_int32_t *)td->data;
		i < total_pixels;
		i++, img_ptr32++
	    )
	    {
		/* Check TARGET depth */
		switch(depth)   
		{
		  case 32:	/* 32 bits source to 32 bits target */
		    /* Blue */
		    fputc(
			(((*img_ptr32) & 0x000000ff)),
			fp
		    );
		    /* Green */
		    fputc(
			(((*img_ptr32) & 0x0000ff00) >> 8),
			fp
		    );
		    /* Red */
		    fputc(
			(((*img_ptr32) & 0x00ff0000) >> 16),
			fp
		    );
		    /* Alpha */
		    fputc(
			(((*img_ptr32) & 0xff000000) >> 24),
			fp
		    );
		    break;

		  case 24:	/* 32 bits source to 24 bits target */
		    /* Blue */
		    fputc(
			(((*img_ptr32) & 0x000000ff)),
			fp
		    );
		    /* Green */ 
		    fputc(
			(((*img_ptr32) & 0x0000ff00) >> 8),
			fp
		    );
		    /* Red */
		    fputc(
			(((*img_ptr32) & 0x00ff0000) >> 16),
			fp
		    );
		    break;

		  case 8:       /* 32 bits source to 8 bits target */
		    /* Grey */
		    fputc(
			((((*img_ptr32) & 0x00ff0000) >> 16) +
			 (((*img_ptr32) & 0x0000ff00) >> 8) +
			 (((*img_ptr32) & 0x000000ff))
			) / 3,
			fp
		    );
		    break;
		}
	    }
	    break;
	}

	FClose(fp);

	return(TgaSuccess);
}

/*
 *	Deletes all values in the TGA Data.
 */
void TgaDestroyData(tga_data_struct *td)
{
	if(td == NULL)
	    return;

	if(td->fp != NULL)
	    FClose(td->fp);

	free(td->header_data);
	free(td->data);
	free(td->comments);

	/* Clear structure */
	memset(td, 0x00, sizeof(tga_data_struct));

	/* Reset values certain non-zero values */
	td->cur_load_pixel = -1;		/* Do not load */
}


/*
 *	Checks if the file is really a Targa file.
 *
 *	Returns TgaSuccess if it is.
 */
int TgaTestFile(const char *filename)
{
	tga_data_struct td;
	int status = TgaReadHeaderFromFile(filename, &td);
	TgaDestroyData(&td);
	return(status);
}
