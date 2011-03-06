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

// xsw_ctype.h
// This is intended as a prototype for files using the global/ctype.cpp file.

#if !defined(__FreeBSD__) && !defined(__NetBSD__)
# if defined(__cplusplus) || defined(c_plusplus)

/* Most systems should have this defined.
#ifndef isblank
extern bool isblank(int c);
#endif
 */

#else

/* Most systems should have this defined.

#ifndef isblank
extern int isblank( int );
#endif
 */

# endif	/* __cplusplus || c_plusplus */
#endif	/* __FreeBSD__ */


extern void ctype_dummy_func();
