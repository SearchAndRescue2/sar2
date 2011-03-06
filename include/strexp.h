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

#ifndef _STREXP_H_
#define _STREXP_H_

#ifdef __cplusplus
extern "C" {
#endif


#include <stdlib.h>
#include <ctype.h>

/* strexp - expand a string into its substrings by whitespace */
extern char **strexp(const char *str, int *n);

/* strchrexp - expand a string into fields (c = designator) */
extern char **strchrexp(const char *str, char c, int *n);


#ifdef __cplusplus
}
#endif

#endif
