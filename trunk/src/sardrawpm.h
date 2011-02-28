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
extern void SARDrawPremodeledPowerTransmissionTower(SAR_DRAW_PREMODELED_PROTOTYPE);
extern void SARDrawPremodeledTower(SAR_DRAW_PREMODELED_PROTOTYPE);
extern void SARDrawPremodeledRadioTower(SAR_DRAW_PREMODELED_PROTOTYPE);

extern void SARDrawPremodeled(SAR_DRAW_PREMODELED_PROTOTYPE);


#endif	/* SARDRAWPM_H */
