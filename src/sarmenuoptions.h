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

extern void SARMenuOptionsJoystickReinit(sar_core_struct *core_ptr);
extern void SARMenuOptionsJoystickMappingReset(sar_core_struct *core_ptr, int gc_js_nums);
extern void SARMenuOptionsJoystickMappingPrepare(sar_core_struct *core_ptr);
extern void SARMenuOptionsJoystickMappingExit(sar_core_struct *core_ptr);

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
