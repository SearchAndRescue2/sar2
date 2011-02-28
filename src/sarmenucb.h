#ifndef SARMENUCB_H
#define SARMENUCB_H

#include "gw.h"
#include "menu.h"
#include "sar.h"

extern void SARMenuButtonCB(void *object, int id_code, void *client_data);
extern void SARMenuListSelectCB(
        void *object, int id_code, void *client_data,
        int item_num, void *item_ptr, void *item_data
);
extern void SARMenuListActivateCB(
        void *object, int id_code, void *client_data,
        int item_num, void *item_ptr, void *item_data
);
extern void SARMenuSwitchCB(
	void *object, int id_code, void *client_data,
	Boolean state
);
extern void SARMenuSpinCB(
	void *object, int id_code, void *client_data,
	char *value
);

#endif	/* SARMENUCB_H */
