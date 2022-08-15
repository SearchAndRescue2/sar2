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
	Premodeled Object Drawing
 */

#ifndef SARDRAWPM_H
#define SARDRAWPM_H

#include "gw.h"
#include "obj.h"
#include "sardraw.h"

#define SAR_DRAW_PREMODELED_PROTOTYPE				\
	sar_dc_struct *dc, sar_object_struct *obj_ptr,		\
	sar_object_premodeled_struct *obj_premodeled_ptr,	\
	float camera_dist, Boolean draw_for_gcc

extern void SARDrawPremodeledBuilding(SAR_DRAW_PREMODELED_PROTOTYPE);
extern void SARDrawPremodeledControlTower(SAR_DRAW_PREMODELED_PROTOTYPE);
extern void SARDrawPremodeledHangar(SAR_DRAW_PREMODELED_PROTOTYPE);
extern void SARDrawPremodeledPowerTransmissionTower(SAR_DRAW_PREMODELED_PROTOTYPE);
extern void SARDrawPremodeledTower(SAR_DRAW_PREMODELED_PROTOTYPE);
extern void SARDrawPremodeledRadioTower(SAR_DRAW_PREMODELED_PROTOTYPE);

extern void SARDrawPremodeled(SAR_DRAW_PREMODELED_PROTOTYPE);


#endif	/* SARDRAWPM_H */
