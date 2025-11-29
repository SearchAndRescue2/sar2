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

#ifndef SARKEY_H
#define SARKEY_H

#include "gw.h"
#include "sar.h"

extern void SARKey(
	sar_core_struct *core_ptr,
	int c,          /* GW Key value or character. */
	Boolean state,  /* Pressed (True) or released (False). */
	long t          /* Time stamp. */
);

#ifndef SAR_KEY_FUNC_PROTOTYPE
/* Prototype for all SARKey*() functions */
#define SAR_KEY_FUNC_PROTOTYPE					\
	sar_core_struct *core_ptr, gw_display_struct *display,	\
	sar_scene_struct *scene, Boolean state
extern void SARKeyCommand(SAR_KEY_FUNC_PROTOTYPE);
#endif	/* SAR_KEY_FUNC_PROTOTYPE */

#endif	/* SARKEY_H */
