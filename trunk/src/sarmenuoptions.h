/*
	            SAR Options Menu IO and Callbacks
 */

#ifndef SARMENUOPTIONS_H
#define SARMENUOPTIONS_H

#include "sar.h"


extern void SARMenuOptionsJoystickTestDrawCB(
	void *dpy,		/* Display */
	void *menu,		/* Menu */
	void *o,		/* This Object */
	int id,			/* ID Code */
	void *client_data,	/* Data */
	int x_min, int y_min, int x_max, int y_max
);

extern void SARMenuOptionsGraphicsInfoRefresh(sar_core_struct *core_ptr);
extern void SARMenuOptionsSoundTest(sar_core_struct *core_ptr);
extern void SARMenuOptionsSoundRefresh(sar_core_struct *core_ptr);
extern void SARMenuOptionsSoundInfoRefresh(sar_core_struct *core_ptr);

extern void SARMenuOptionsApply(sar_core_struct *core_ptr);
extern void SARMenuOptionsFetch(sar_core_struct *core_ptr);

extern void SARMenuOptionsButtonCB(
	void *object, int id, void *client_data
);
extern void SARMenuOptionsSwitchCB(
	void *object, int id, void *client_data,
	Boolean state
);
extern void SARMenuOptionsSpinCB(
	void *object, int id, void *client_data,
	char *value
);
extern void SARMenuOptionsSliderCB(
	void *object, int id, void *client_data,
	float value
);

#endif	/* SARMENUOPTIONS_H */
