/*
		Control Panel Instrument - Artificial Horizon
 */

#ifndef CPINSHORIZON_H
#define CPINSHORIZON_H

#include <sys/types.h>
#include <GL/gl.h>
#include "v3dtex.h"
#include "cpvalues.h"
#include "cpins.h"

/*
 *	This instrument type code:
 */
#define CPINS_TYPE_HORIZON	12

/*
 *      Artificial Horizon Instrument structure:
 */
typedef struct _CPInsHorizon CPInsHorizon;
struct _CPInsHorizon {

	CPIns ins;

	int	deg_visible;		/* Pitch degrees visible. */

	float	bank_notch_radius;	/* Coefficient from center (0.0)
					 * to edge (1.0).
					 */

};
#define CPINS_HORIZON(p)	((CPInsHorizon *)(p))
#define CPINS_IS_HORIZON(p)	(CPINS_TYPE(p) == CPINS_TYPE_HORIZON)


extern CPIns *CPInsHorizonNew(void *cp);


#endif	/* CPINSHORIZON_H */
