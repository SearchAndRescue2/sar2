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

#ifndef MESSAGES_H
#define MESSAGES_H

#include "obj.h"

/* Regular messages list handling. */
extern void SARMessageAdd(sar_scene_struct *scene, const char *message);
extern void SARMessageClearAll(sar_scene_struct *scene);

/* Sticky banner message handling. */
extern void SARBannerMessageAppend(sar_scene_struct *scene, const char *message);

/* Camera ref title handling. */
extern void SARCameraRefTitleSet(sar_scene_struct *scene, const char *message);


#endif	/* MESSAGES_H */
