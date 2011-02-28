/*
			  Targa Image File Library
 */

#ifndef TGA_H
#define TGA_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <sys/types.h>

/* Catch any undefined bit types (for Solaris and other platforms) */
#include "os.h"



/*
 *	TGA Library Version:
 */
#define TgaLibraryVersion	"1.5"
#define TgaVersionMajor		1
#define TgaVersionMinor		5

/*
 *	Error Codes:
 */
typedef enum {
	TgaSuccess		= 0,
	TgaNoBuffers		= 1,
	TgaBadHeader		= 2,
	TgaBadValue		= 3,
	TgaNoFile		= 4,
	TgaNoAccess		= 5
} tga_error;

/*
 *	Error Severity Codes:
 *
 *	Used in TgaReportError().
 */
typedef enum {
	TgaErrorLevelWarning,
	TgaErrorLevelMinor,
	TgaErrorLevelModerate,
	TgaErrorLevelCritical
} tga_error_severity;


/*
 *	TGA Format Version Codes:
 */
typedef enum {
	TgaFormatVersionOld	= 0,		/* The "old" format */
	TgaFormatVersionNew	= 1		/* The "new" format */
} tga_format_version;


/*
 *	TGA Header Length:
 */
#define TgaHeaderLength		18		/* In bytes */

/*
 *	TGA Colormap Types:
 */
typedef enum {
	TgaColormapNone			= 0,
	TgaColormapExists		= 1
} tga_colormap_type;

/*
 *	TGA Image Data Types:
 *
 *	For byte 2 of the TGA header.
 */
typedef enum {
	TgaImageDataTypeNoImage		= 0x00,
	TgaImageDataUColorMapped	= 0x01,	/* Colormap */
	TgaImageDataURGB		= 0x02,	/* RGB */
	TgaImageDataUBW			= 0x03,	/* Greyscale */

	TgaImageDataREColorMapped	= 0x09,
	TgaImageDataRERGB		= 0x0a,

	TgaImageDataCBW			= 0x0b,

	TgaImageDataCHuffman		= 0x20,
	TgaImageDataCHuffmanQ		= 0x21
} tga_image_data_type;

/*
 *	TGA Flags:
 *
 *	Byte 12 of the header has bits of these values.
 *
 *	NOTE: To comform to AT&T, all bits in byte 0x12 should be 0.
 *	However this is NOT always the case since some programs do
 *	not comform to this.
 *
 *	In most cases, TgaImageFliped is the bit you need to watch out
 *	for.
 */
typedef enum {
#if 0
	TgaImageAttrBit0		= 0x00,	/* Attr bits per pixel */
	TgaImageAttrBit1		= 0x00,	/* Attr bits per pixel */
	TgaImageAttrBit2		= 0x00,	/* Attr bits per pixel */
	TgaImageAttrBit3		= 0x00,	/* Attr bits per pixel */
	TgaImage4			= 0x00,	/* Bit 4 is reserved */
#endif
	TgaImageFliped		= 0x20	/* If set means image is
					 * rightside up */
#if 0
	TgaImageIntLeavFlag0	= 0x00,	/* Data storage interleaving flag */
	TgaImageIntLeavFlag1	= 0x00	/* Data storage interleaving flag */
#endif
} tga_image_flags;


typedef struct _tga_data_struct		tga_data_struct;

/*
 *	TGA Data:
 */
struct _tga_data_struct {

	tga_format_version	version;
	FILE		*fp;		/* For partial/async reading */

	/* Header */
	u_int8_t	id_field_len;
	tga_colormap_type	cmap_type;
	tga_image_data_type	img_type;

	u_int16_t	cmap_first_color_index;
	u_int16_t	cmap_total_colors;
	u_int8_t	cmap_entry_size;	/* 15, 16, 24, or 32 bits */

	int		x, y;
	unsigned int	width, height;
	unsigned int	depth;		/* Depth of data on file; 1, 8,
					 * 16, 24, or 32 */

	tga_image_flags	flags;		/* Image Descriptor */

	/* Statistical information */
	off_t		file_size,	/* Size of file, in bytes */
			/* Size of ID field (see id_field_len) */
			cmap_size,	/* Size of colormap in bytes */
			data_size;	/* Size of image data, in bytes */

	u_int8_t	bits_per_pixel;	/* Original Bits Per Pixel */

	int		cur_load_pixel;	/* Mark on where we left off
					 * during loading, in units of
					 * pixels (for partial/async
					 * reading) */

	/* Loaded Data */
	u_int8_t	*header_data;	/* In raw data (exact as on file) */

	u_int8_t	*data;		/* In ZPixmap data format */
	u_int8_t	data_depth;	/* Bits per pixel of loaded data */

	char		*comments;
};


/*
 *	Returns the version number of this library.
 */
extern int TgaQueryVersion(int *major_rtn, int *minor_rtn);

/*
 *	Reads the tga image header from the specified file and stores
 *	them in td.
 */
extern int TgaReadHeaderFromFile(
	const char *filename,
	tga_data_struct *td
);

/*
 *	Reads the tga image header from the specified data and stores
 *	them in td.
 */
extern int TgaReadHeaderFromData(
	const u_int8_t *data,
	tga_data_struct *td
);

/*
 *	Reads the tga image from the specified file.
 *
 *	TgaReadHeaderFromFile() will be automatically called by this
 *	function.
 *
 *	The depth specifies the bit depth of the loaded image data.
 */
extern int TgaReadFromFile(
	const char *filename,
	tga_data_struct *td,
	unsigned int depth
);

/*
 *	Reads the tga image from the specified data.
 *
 *	TgaReadHeaderFromData() will be automatically called by this
 *	function.
 *
 *	The depth specifies the bit depth of the loaded image data.
 */
extern int TgaReadFromData(
	const u_int8_t *data,
	tga_data_struct *td,
	unsigned int depth
);

/*
 *	Sets up for async/partial reading of the tga image from file.
 *
 *	The depth specifies the bit depth of the loaded image data.
 *
 *	Once this call has been made, you may use
 *	TgaReadPartialFromFile() to begin reading the data.
 */
extern int TgaStartReadPartialFromFile(
	const char *filename,
	tga_data_struct *td,
	unsigned int depth
);


/*
 *   Reads a specific number of pixels from file.
 *
 *   The td must be initialized prior by a call to
 *   TgaStartReadPartialFromFile().
 */
extern int TgaReadPartialFromFile(  
	tga_data_struct *td, 
	unsigned int depth,  
	unsigned int n_pixels
);


/*
 *   Writes the tga image data contained in td to
 *   the file filename at the target depth specified.
 */
extern int TgaWriteToFile(
	const char *filename,
	tga_data_struct *td,
	unsigned int depth
);


/*
 *   Frees all allocated memory in td.
 */
extern void TgaDestroyData(tga_data_struct *td);


/*
 *   Checks if *filename is a valid TGA file, returns
 *   TgaSuccess if it is a valid TGA file or appropriate error
 *   if it is not.
 */
extern int TgaTestFile(const char *filename);


/*
 *	Reads the tga image from the specified file using the "fast"
 *	method.
 *
 *	The image data will be in RGBA format.
 */
extern u_int8_t *TgaReadFromFileFastRGBA(
	const char *filename,
	int *rtn_width, int *rtn_height,
	u_int32_t transparent_pixel     /* RGBA of transparent pixel */
);


/* 
 *      Dithering functions in tgadither.c (used internally):
 */
extern u_int8_t TgaDitherRedPixel8(
	int RedValue,
	int Xp,
	int Yp
);
extern u_int8_t TgaDitherGreenPixel8(
	int GreenValue,
	int Xp,
	int Yp
); 
extern u_int8_t TgaDitherBluePixel8(
	int BlueValue,
	int Xp,
	int Yp
);


#ifdef __cplusplus
}
#endif

#endif /* TGA_H */
