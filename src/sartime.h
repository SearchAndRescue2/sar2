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

#ifndef SARTIME_H
#define SARTIME_H

#include <sys/types.h>

extern time_t SARGetCurMilliTime(void);

extern unsigned int SARRandom(unsigned int seed_offset);
extern float SARRandomCoeff(unsigned int seed_offset);

extern int SARParseTimeOfDay(
	const char *string,
	int *h, int *m, int *s
);
extern int SARParseLatitudeDMS(const char *string, float *dms);
extern int SARParseLongitudeDMS(const char *string, float *dms);

#endif	/* SARTIME_H */
