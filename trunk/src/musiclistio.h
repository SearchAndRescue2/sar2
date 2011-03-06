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
 *                       Music File Reference List IO
 */
#ifndef MUSICLISTIO_H
#define MUSICLISTIO_H

#include <sys/types.h>
#include "sound.h"


/*
 *	Music Reference Flags:
 */
#define SAR_MUSIC_REF_FLAGS_REPEAT	(1 << 1)
#define SAR_MUSIC_REF_FLAGS_FADE_IN	(1 << 2)
#define SAR_MUSIC_REF_FLAGS_FADE_OUT	(1 << 3)

/*
 *	Music Reference:
 */
typedef struct {

	int		id;		/* One of SAR_MUSIC_ID_* */
	char		*filename;
	unsigned int	flags;		/* Any of SAR_MUSIC_REF_FLAGS_* */

} sar_music_ref_struct;
#define SAR_MUSIC_REF(p)	((sar_music_ref_struct *)(p))


extern sar_music_ref_struct *SARMusicMatchPtr(
	sar_music_ref_struct **ml, int total,
	int id, int *music_ref_num
);

extern void SARMusicListDeleteAll(
	sar_music_ref_struct ***ml, int *total
);
extern int SARMusicListLoadFromFile(
	const char *filename,
	sar_music_ref_struct ***ml, int *total
);

#endif	/* MUSICLISTIO_H */
