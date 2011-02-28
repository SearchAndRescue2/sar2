/*
		    SAR Flight Model - Data Types
 */

#ifndef SFMTYPES_H
#define SFMTYPES_H

#include <sys/types.h>
#ifdef __MSW__
# include "../include/os.h"
#endif

/*
 *	Basic data types:
 */
#define SFMBoolean	u_int8_t
#ifdef __MSW__
# define SFMFlags	unsigned long
#else
# define SFMFlags	u_int64_t
#endif
#define SFMTime		time_t


/*
 *	Boolean states:
 */
#ifndef True
# define True	1
#endif
#ifndef False
# define False	0
#endif

/*
 *	PI constant:
 */
#ifndef PI
# define PI     3.14159265
#endif


/*
 *	Position structure (also the vector structure):
 */
typedef struct {

	double x, y, z;

} SFMPositionStruct;


/*
 *	Direction structure:
 */
typedef struct {

	double heading, pitch, bank;

	double i, j, k;		/* Unit vector of heading and pitch
				 * (no bank).
				 */

} SFMDirectionStruct;





#endif	/* SFMTYPES_H */
