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

/*
		     Control Panel Instrument
 */

#ifndef CPINS_H
#define CPINS_H

#include <sys/types.h>
#include <GL/gl.h>
#include "v3dtex.h"
#include "cpvalues.h"


/*
 *	Default (this) instrument type:
 */
#define CPINS_TYPE_INS		1


/*
 *	Control Panel Instrument structure:
 *
 *	This is the base structure for all instruments.
 */
typedef struct _CPIns CPIns;
struct _CPIns {

	int	type;		/* One of CPINS_TYPE_. */

	void	*cp;		/* Pointer back to control panel. */

	char	*name;		/* Arbitary name of instrument. */

	/* Position and size (coefficient) relative to the control
	 * panel's upper left corner.
	 */
	float	x, y,
		width, height;

	/* Texture resolution, the size to set the GL back buffer when
	 * drawing for the texture tex.  This will also be the size of
	 * the texture tex.
	 */
	int res_width, res_height;	/* In pixels. */

	/* Texture that will be used when drawing the instrument, this
	 * texture should be created in the "values_changed" function
	 * using the GL back buffer.  The size of this texture is
	 * determined by res_width and res_height, this will also be
	 * the size of the GL back buffer when drawing for this texture.
	 */
	v3d_texture_ref_struct  *tex;

	/* Last time instrument was updated and the update interval. */
	time_t	update_next,		/* In milliseconds. */
		update_int;

	/* Background and foreground textures. */
	v3d_texture_ref_struct	*tex_bg,
				*tex_fg;

	CPColor	color_bg[CP_COLOR_STATES],
		color_fg[CP_COLOR_STATES],
		color_text[CP_COLOR_STATES];

	/* Functions. */
	/* "values_changed" function (CPIns *ins, ControlPanelValues *v, void *data). */
	void *values_changed_func_data;
	void (*values_changed_func)(CPIns *, ControlPanelValues *, void *);

	/* "manage" function (CPIns *ins, ControlPanelValues *v, void *data). */
	void *manage_func_data;
	void (*manage_func)(CPIns *, ControlPanelValues *, void *);

	/* "reset_timmers" function (CPIns *ins, time_t t, void *data). */
	void *reset_timmers_func_data;
	void (*reset_timmers_func)(CPIns *, time_t, void *);

	/* "delete" function (CPIns *ins, void *data). */
	void *delete_func_data;
	void (*delete_func)(CPIns *, void *);
};
#define CPINS(p)	((CPIns *)(p))
#define CPINS_TYPE(p)	(((p) != NULL) ? (*(int *)(p)) : -1)
#define CPINS_NAME(p)	(((p) != NULL) ? CPINS(p)->name : NULL)
#define CPINS_IS_INS(p)	(CPINS_TYPE(p) == CPINS_TYPE_INS)


extern void CPInsSetName(CPIns *ins, const char *name);
extern void CPInsSetPosition(CPIns *ins, float x, float y);
extern void CPInsSetSize(CPIns *ins, float width, float height);
extern void CPInsSetTextureBG(CPIns *ins, const char *path);
extern void CPInsSetTextureFG(CPIns *ins, const char *path);
extern void CPInsSetResolution(
	CPIns *ins,
	int res_width, int res_height	/* In pixels. */
);
extern void CPInsSetUpdateInt(CPIns *ins, time_t update_int);
extern void CPInsSetFunction(
	CPIns *ins, const char *func_name,
	void *func, void *data
);

extern void CPInsResetTimmers(CPIns *ins, time_t t);
extern void CPInsChangeValues(CPIns *ins, ControlPanelValues *v);
extern void CPInsDraw(CPIns *ins);
extern void CPInsDrawFullScreen(CPIns *ins, const CPRectangle *rect);
extern void CPInsManage(CPIns *ins, ControlPanelValues *v);
extern void CPInsRealizeTexture(CPIns *ins);

extern CPIns *CPInsNew(size_t s, void *cp, int type, const char *name);
extern void CPInsDelete(CPIns *ins);


#endif	/* CPINS_H */
