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

#ifndef FIO_H
#define FIO_H

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

extern FILE *FOpen(const char *path, const char *mode);
extern void FClose(FILE *fp);

extern void FSeekNextLine(FILE *fp);
extern void FSeekPastSpaces(FILE *fp);
extern void FSeekPastChar(FILE *fp, char c);

extern int FSeekToParm(FILE *fp, const char *parm, char comment, char delimiator);
extern char *FSeekNextParm(FILE *fp, char *buf, char comment, char delim);

extern int FGetValuesI(FILE *fp, int *value, int nvalues);
extern int FGetValuesL(FILE *fp, long *value, int nvalues);
extern int FGetValuesF(FILE *fp, double *value, int nvalues);
extern char *FGetString(FILE *fp);
extern char *FGetStringLined(FILE *fp);
extern char *FGetStringLiteral(FILE *fp);

extern char *FReadNextLineAlloc(FILE *fp, char comment);

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" char *FReadNextLineAllocCount(
	FILE *fp,
	char comment,
	int *line_count
);
#else
extern char *FReadNextLineAllocCount(
	FILE *fp,
	char comment,
	int *line_count
);
#endif


#ifdef __cplusplus
}
#endif

#endif	/* FIO_H */

