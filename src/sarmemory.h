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

#ifndef SARMEMORY_H
#define SARMEMORY_H

#include "obj.h"
#include "sar.h"


/*
 *	Memory stats structure:
 */
typedef struct _sar_memory_stat_struct sar_memory_stat_struct;
struct _sar_memory_stat_struct {

	/* All memory units are in bytes unless noted otherwise */
	unsigned long	total,
			texture,
			vmodel,
			scene,
			object;

	int		ntextures,
			nvmodels,
			nobjects;

};

extern void SARMemoryStat(
	sar_core_struct *core_ptr,
	sar_scene_struct *scene,
	sar_object_struct **object, int total_objects,
	sar_memory_stat_struct *stat_buf
);


#endif	/* SARMEMORY_H */
