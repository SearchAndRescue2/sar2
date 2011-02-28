/*
				     Image

	For OpenGL image rendering.
 */

#ifndef IMAGE_H
#define IMAGE_H

#include <sys/types.h>


/*
 *	Image Types:
 */
typedef enum {
	SAR_IMAGE_TYPE_RGBA,
	SAR_IMAGE_TYPE_RGB,
	SAR_IMAGE_TYPE_LUMINANCE
} sar_image_type;


/*
 *	Image structure:
 */
typedef struct {

	sar_image_type	type;	/* One of SAR_IMAGE_TYPE_* */

	int		width,
			height;

	u_int8_t	*data;

} sar_image_struct;
#define SAR_IMAGE(p)	((sar_image_struct *)(p))


/* In image.c */
extern sar_image_struct *SARImageNew(
	sar_image_type type,
	int width, int height,
	u_int8_t *data
);
extern sar_image_struct *SARImageNewFromFile(const char *filename);
extern void SARImageDelete(sar_image_struct *img);

extern void SARImageDraw(
	gw_display_struct *display,
	const sar_image_struct *img,
	int x, int y,
	int width, int height
);


#endif	/* IMAGE_H */
