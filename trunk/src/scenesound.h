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
	SAR Scene Sound Management

	Updates the continuous playing of sound and music on the scene.
 */

#ifndef SCENESOUND_H
#define SCENESOUND_H

#include "sar.h"

extern void SARSceneSoundUpdate(
	sar_core_struct *core_ptr,
	Boolean engine_sounds,
	Boolean event_sounds,
	Boolean voice_sounds,
	Boolean music
);

#endif	/* SCENESOUND_H */
