#include <stdio.h>
#include <sys/types.h>
#include <math.h>

#ifdef __MSW__
# include <windows.h>
#endif

#include <GL/gl.h>
#include <GL/glu.h>

#include "gw.h"

#include "sarreality.h"
#include "obj.h"
#include "objutils.h"
#include "sfm.h"
#include "sar.h"
#include "sardraw.h"
#include "sardrawpm.h"
#include "sardrawdefs.h"


void SARDrawPremodeled(SAR_DRAW_PREMODELED_PROTOTYPE);


/*
 *	Front end to drawing premodeled objects, called by SARDraw().
 */
void SARDrawPremodeled(SAR_DRAW_PREMODELED_PROTOTYPE)
{
	if((dc == NULL) || (obj_ptr == NULL) || (obj_premodeled_ptr == NULL))
	    return;

#define SAR_DRAW_PREMODELED_INPUT		\
	dc, obj_ptr, obj_premodeled_ptr,	\
	camera_dist, draw_for_gcc

	switch(obj_premodeled_ptr->type)
	{
	  case SAR_OBJ_PREMODELED_BUILDING:
	    SARDrawPremodeledBuilding(SAR_DRAW_PREMODELED_INPUT);
	    break;
	  case SAR_OBJ_PREMODELED_CONTROL_TOWER:
	    SARDrawPremodeledControlTower(SAR_DRAW_PREMODELED_INPUT);
	    break;
	  case SAR_OBJ_PREMODELED_HANGAR:
	    /* TODO */
	    break;
	  case SAR_OBJ_PREMODELED_POWER_TRANSMISSION_TOWER:
	    SARDrawPremodeledPowerTransmissionTower(SAR_DRAW_PREMODELED_INPUT);
	    break;
	  case SAR_OBJ_PREMODELED_TOWER:
	    SARDrawPremodeledTower(SAR_DRAW_PREMODELED_INPUT);
	    break;
	  case SAR_OBJ_PREMODELED_RADIO_TOWER:
	    SARDrawPremodeledRadioTower(SAR_DRAW_PREMODELED_INPUT);
	    break;
	}

#undef SAR_DRAW_PREMODELED_INPUT
}

