#include <stdio.h>
#include <sys/types.h>
#include "../include/tga.h"

#ifdef MEMWATCH
# include "memwatch.h"
#endif


u_int8_t TgaDitherRedPixel8(int RedValue, int Xp, int Yp);
u_int8_t TgaDitherGreenPixel8(int GreenValue, int Xp, int Yp);
u_int8_t TgaDitherBluePixel8(int BlueValue, int Xp, int Yp);


u_int8_t TgaDitherRedPixel8(
	int RedValue,
	int Xp,
	int Yp
)
{
	static short dither_red[2][16] = {
    {-16,  4, -1, 11,-14,  6, -3,  9,-15,  5, -2, 10,-13,  7, -4,  8},
    { 15, -5,  0,-12, 13, -7,  2,-10, 14, -6,  1,-11, 12, -8,  3, -9}
	};

	int red;
	u_int8_t pixel_red;
	int x_dither_table, y_dither_table;

	/* Return 0 if value is 0. */
	if(RedValue == 0)
	    return(0);

	/* Determine the dither table entries to use based on the pixel address */
	x_dither_table = Xp % 16;   /* X Pixel Address MOD 16 */
	y_dither_table = Yp % 2;    /* Y Pixel Address MOD 2 */

	/* Start with the initial values as supplied by the calling routine */
	red = RedValue;

	/* Generate the red dither value */
	red += dither_red[y_dither_table][x_dither_table];
	/* Check for overflow or underflow on red value */
	if(red > 0xff)
	    red = 0xff;
	else if(red < 0x00)
	    red = 0x00;

	/* Generate the pixel red value */
	/*pixel_red = (red & 0xE0);*/
	pixel_red = red;
	return(pixel_red);
}

u_int8_t TgaDitherGreenPixel8(
	int GreenValue,
	int Xp,
	int Yp
)
{
	static short dither_green[2][16] = {
    { 11,-15,  7, -3,  8,-14,  4, -2, 10,-16,  6, -4,  9,-13,  5, -1},
    {-12, 14, -8,  2, -9, 13, -5,  1,-11, 15, -7,  3,-10, 12, -6,  0}
	};
	int green;
	u_int8_t pixel_green;
	int x_dither_table, y_dither_table;


	/* Return 0 if value is 0. */
	if(GreenValue == 0)
	    return(0);

	/* Determine the dither table entries to use based on the pixel address */
	x_dither_table = Xp % 16;   /* X Pixel Address MOD 16 */
	y_dither_table = Yp % 2;    /* Y Pixel Address MOD 2 */

	/* Start with the initial values as supplied by the calling routine */
	green = GreenValue;

	/* Generate the green dither value */
	green += dither_green[y_dither_table][x_dither_table];
	/* Check for overflow or underflow on green value */
	if(green > 0xff)
	    green = 0xff;
	else if(green < 0x00)
	    green = 0x00;

	/* Generate the pixel red value */
	/*pixel_green = ((green & 0xE0) >> 3);*/
	pixel_green = green;
	return(pixel_green);
}

u_int8_t TgaDitherBluePixel8(
	int BlueValue,
	int Xp,
	int Yp
)
{
	static short dither_blue[2][16] = {
    { -3,  9,-13,  7, -1, 11,-15,  5, -4,  8,-14,  6, -2, 10,-16,  4},
    {  2,-10, 12, -8,  0,-12, 14, -6,  3, -9, 13, -7,  1,-11, 15, -5}
	};
	int blue;
	u_int8_t pixel_blue;
	int x_dither_table, y_dither_table;


	/* Return 0 if value is 0. */
	if(BlueValue == 0)
	    return(0);

	/* Determine the dither table entries to use based on the pixel address */
	x_dither_table = Xp % 16;   /* X Pixel Address MOD 16 */
	y_dither_table = Yp % 2;    /* Y Pixel Address MOD 2 */

	/* Start with the initial values as supplied by the calling routine */
	blue = BlueValue;

	/* Generate the blue dither value */
	blue += (dither_blue[y_dither_table][x_dither_table]<<1);
	/* Check for overflow or underflow on blue value */
	if(blue > 0xff)
	    blue = 0xff;
	else if(blue < 0x00)
	    blue = 0x00;

	/* Generate the pixel value by "or"ing the values together */
	/*pixel_blue = ((blue & 0xc0) >> 6);*/
	pixel_blue = blue;
	return(pixel_blue);
}




