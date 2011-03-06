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

#ifndef SARMENUOP_H
#define SARMENUOP_H

#include "gw.h"
#include "menu.h"
#include "sar.h"

extern void SARMenuSwitchToMenu(
	sar_core_struct *core_ptr, const char *name
);
extern sar_menu_list_struct *SARMenuGetList(
	sar_menu_struct *m, int skip, int *list_num
);
extern sar_menu_spin_struct *SARMenuGetSpin(
	sar_menu_struct *m, int skip, int *spin_num
);
extern sar_menu_list_item_struct *SARMenuListGetSelectedItem(
	sar_menu_struct *m, int id
);
extern char *SARMenuGetCurrentMenuName(sar_core_struct *core_ptr);

#endif	/* SARMENUOP_H */
