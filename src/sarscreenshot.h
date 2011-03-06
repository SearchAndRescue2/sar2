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
