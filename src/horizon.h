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

#ifndef HORISON_H
#define HORIZON_H

#include "v3dtex.h"
#include "obj.h"


extern v3d_texture_ref_struct *SARCreateHorizonTexture(
        const char *name,
	const sar_color_struct *start_color,	/* Top color */
	const sar_color_struct *end_color,	/* Bottom color */
        int height,
	float midpoint
);


#endif	/* HORIZON_H */
