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
                          Smoke Puffs & Sparks Creation
 */

#ifndef SMOKE_H
#define SMOKE_H

#include "v3dtex.h"
#include "obj.h"


extern int SmokeCreate(
	sar_scene_struct *scene,
	sar_object_struct ***ptr, int *total,
	sar_smoke_type smoke_type,
	const sar_position_struct *pos,
	const sar_position_struct *respawn_offset,
	float radius_start,            /* In meters. */
	float radius_max,              /* In meters. */
	float radius_rate,             /* In meters per second. */
	int hide_at_max,
	int total_units,                /* Max smoke units in trail. */
	time_t respawn_int,		/* In ms. */
	const char *tex_name,
	int ref_object,                 /* Can be -1 for none. */
	time_t life_span
);

extern int SmokeCreateSparks(
	sar_scene_struct *scene,
	sar_object_struct ***ptr, int *total,
	const sar_position_struct *pos,
	const sar_position_struct *respawn_offset,
	float sparks_distance,		/* How far should sparks fly */
	int ref_object,
	time_t life_span
);


#endif	/* SMOKE_H */
