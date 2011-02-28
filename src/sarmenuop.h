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
