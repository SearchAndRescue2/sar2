#ifndef SIMCB_H
#define SIMCB_H

#include <sys/types.h>
#include "sfm.h"
#include "obj.h"
#include "sar.h"


extern void SARSimInitModelCB(
	void *realm_ptr, SFMModelStruct *model,
	void *client_data
);
extern void SARSimDestroyModelCB(
	void *realm_ptr, SFMModelStruct *model,
	void *client_data                     
);

extern void SARSimAirborneCB(
	void *realm_ptr, SFMModelStruct *model,
	void *client_data
);
extern void SARSimTouchDownCB(
	void *realm_ptr, SFMModelStruct *model,
	void *client_data, double impact_coeff
);
extern void SARSimOverspeedCB(
	void *realm_ptr, SFMModelStruct *model,
	void *client_data, double cur_speed,
	double overspeed_expected,
	double overspeed
);
extern void SARSimCollisionCB(
	void *realm_ptr, SFMModelStruct *model, SFMModelStruct *obstruction,
	void *client_data, double impact_coeff 
);
extern void SARSimObjectCollisionCB(
	void *client_data,              /* Core structure. */
	sar_object_struct *obj_ptr, sar_object_struct *obstruction_obj_ptr,
	double impact_coeff
);


#endif	/* SIMCB_H */
