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
	       UNIX Standard Configuration File Format

	For reading and parsing the standard UNIX configuration
	file format.

	The format is:

	    <parameter>=<value>

	Spaces are allowed between the = character.
	All tailing and leading spaces will be removed.

	Lines beginning with a `#' character are consider comments.

 */


#ifndef CFGFMT_H
#define CFGFMT_H


/*
 *	Comment character:
 */
#ifndef UNIXCFG_COMMENT_CHAR
# define UNIXCFG_COMMENT_CHAR		'#'
#endif


/*
 *    Parameter and value delimiter:
 */
#ifndef CFG_PARAMETER_DELIMITER
    #define CFG_PARAMETER_DELIMITER	'='
#endif


/*
 *    Maximum length of Parameter:
 */
#ifndef CFG_PARAMETER_MAX
    #define CFG_PARAMETER_MAX		256
#endif

/*
 *    Maximum length of Value:
 */
#ifndef CFG_VALUE_MAX
    #define CFG_VALUE_MAX		1024
#endif

/*
 *    Maximum length of one line:
 */
#ifndef CFG_STRING_MAX
    #define CFG_STRING_MAX		CFG_PARAMETER_MAX + CFG_VALUE_MAX + 5
#endif




#endif /* CFGFMT_H */
