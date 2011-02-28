/*
		Control Panel Instrument - Altimeter
 */

#ifndef CPINSALTIMETER_H
#define CPINSALTIMETER_H

#include <sys/types.h>
#include <GL/gl.h>
#include "v3dtex.h"
#include "cpvalues.h"
#include "cpins.h"

/*
 *	This instrument type code:
 */
#define CPINS_TYPE_ALTIMETER	13

/*
 *      Altimeter Instrument structure:
 */
typedef struct _CPInsAltimeter CPInsAltimeter;
struct _CPInsAltimeter {

	CPIns ins;

};
#define CPINS_ALTIMETER(p)	((CPInsAltimeter *)(p))
#define CPINS_IS_ALTIMETER(p)	(CPINS_TYPE(p) == CPINS_TYPE_ALTIMETER)


extern CPIns *CPInsAltimeterNew(void *cp);


#endif	/* CPINSALTIMETER_H */
