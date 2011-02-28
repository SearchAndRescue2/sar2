#ifndef SARSCREENSHOT_H
#define SARSCREENSHOT_H

#include "sar.h"

extern void SARScreenShotRect(
        sar_core_struct *core_ptr, const char *dir, int detail_level,
	int x, int y, int width, int height
);
extern void SARScreenShot(
	sar_core_struct *core_ptr, const char *dir, int detail_level
);



#endif	/* SARSCREENSHOT_H */
