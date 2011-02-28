// include/graphics.h

/*
		 Archtecture Graphics Size Definations

	Low-level definations for blitting functions and other
	things related to graphics.

	Do not adjust these values unless you are specifically
	told to or know what you are doing.


*/

#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <sys/types.h>
#include "../include/os.h"


/*
 *	Bytes per pixel sizes:
 */
#ifndef BYTES_PER_PIXEL8
# define BYTES_PER_PIXEL8	1
#endif

#ifndef BYTES_PER_PIXEL15
# define BYTES_PER_PIXEL15	2
#endif

#ifndef BYTES_PER_PIXEL16
# define BYTES_PER_PIXEL16	2
#endif

#ifndef BYTES_PER_PIXEL24  
# define BYTES_PER_PIXEL24	4       /* Same as BYTES_PER_PIXEL32 */
#endif

#ifndef BYTES_PER_PIXEL32
# define BYTES_PER_PIXEL32	4
#endif  


/*
 *      Lookup table system.
 */
extern u_int16_t gRLookup[256];
extern u_int16_t gGLookup[256];
extern u_int16_t gBLookup[256];



/*
 *	Bit packing macros:
 */
#ifndef PACK8TO8
//-KB-//    #define PACK8TO8(r,g,b)	(u_int8_t)((r>>5)<<5)+((g>>5)<<2)+(b>>6)
# define PACK8TO8(r,g,b)	(u_int8_t)(((r)&0xE0)+(((g)&0xE0)>>3)+((b)>>6))
#endif

#ifndef PACK8TO15
//-KB-//    #define PACK8TO15(r,g,b)    (u_int16_t)((r>>3)<<10)+((g>>3)<<5)+(b>>3)
# define PACK8TO15(r,g,b)	(u_int16_t)(gRLookup[(r)]+gGLookup[(g)]+gBLookup[(b)])
#endif

#ifndef PACK8TO16
//-KB-//    #define PACK8TO16(r,g,b)    (u_int16_t)((r>>3)<<11)+((g>>2)<<5)+(b>>3)
# define PACK8TO16(r,g,b)	(u_int16_t)(gRLookup[(r)]+gGLookup[(g)]+gBLookup[(b)])
#endif

#ifndef PACK8TO32
//-KB-//    #define PACK8TO32(a,r,g,b)	(u_int32_t)((a<<24)|(r<<16)|(g<<8)|(b))
# define PACK8TO32(a,r,g,b)	(u_int32_t)(((a)<<24)|((r)<<16)|((g)<<8)|(b))
#endif






#endif /* GRAPHICS_H */
