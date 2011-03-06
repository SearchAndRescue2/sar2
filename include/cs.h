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
		    CyberSpace 3D Network Protocol

	Network protocol for transfering data about objects in 3D.

	Used by XShipWars.

 */

#ifndef CS_H
#define CS_H


/*
 *	String deliminator character:
 *
 *	For deliminating strings.
 */
#define CS_STRING_DELIMINATOR_CHAR	';'


/*
 *	Data segment maximums (in bytes):
 *
 *	Important: CS_DATA_MAX_LEN must be big enough to contain
 *	the full string sizes of each data segment's arguments!!
 */
#define CS_DATA_MAX_LEN		256
#define CS_DATA_MAX_BACKLOG	100000



/*
 *	Message length maximums:
 *
 *	For messages such as CS_CODE_LIVEMESSAGE.
 *
 *	Length must be LESS than CS_DATA_MAX_LEN.
 */
#define CS_MESG_MAX		128


/*
 *	Standard connection error messages:
 *
 *	If applicateable these should be BSD standard as well.
 */



/*
 *	Network command codes:
 *
 *	These are network command codes, one of these is prefixed to
 *	each CS data sentance.
 *
 *	Argument description provided with each code defination.
 */

/*
 *	To client:
 *	To server: name;password;client_type
 */
#define CS_CODE_LOGIN		11

/*
 *	To client:
 *	To server:
 */
#define CS_CODE_LOGOUT		12

/*
 *	To client: object_num;name
 *	To server:
 */
#define CS_CODE_WHOAMI		13

/*
 *      To client: *not sent*
 *      To server:
 */
#define CS_CODE_REFRESH		14

/*
 *      To client: *not sent*
 *      To server: interval
 */
#define CS_CODE_INTERVAL	15

/*
 *      To client: path
 *      To server:
 */
#define CS_CODE_IMAGESET	16

/*
 *      To client: path
 *      To server:
 */
#define CS_CODE_SOUNDSET        17


/*
 *	To client: url
 *	To server: *not sent*
 */
#define CS_CODE_FORWARD		20


/*
 *	To client: message
 *	To server: *not sent*
 */
#define CS_CODE_LIVEMESSAGE	30

/*
 *      To client: sys_mesg_code message
 *      To server: *not sent*
 *
 *	Note: sys_mesg_code can be one of CS_SYSMESG_*.
 */
#define CS_CODE_SYSMESSAGE	31

/*
 *      To client: *not sent*
 *      To server: command
 */
#define CS_CODE_LITERALCMD	32


/*
 *      To client: sound_code, left_volume, right_volume
 *      To server: *not sent*
 */
#define CS_CODE_PLAYSOUND	37


/*
 *      To client: object_num, type, isref_num, owner, size,
 *                 locked_on, intercepting_object, scanner_range,
 *		   sect_x, sect_y, sect_z,
 *                 x, y, z, heading, pitch, bank,
 *                 velocity, velocity_heading, velocity_pitch,
 *                 velocity_bank, current_frame, anim_int,
 *                 total_frames, cycle_times
 *      To server: *not sent*
 */
#define CS_CODE_CREATEOBJ	40

/* 
 *      To client: object_num
 *      To server: *not sent*
 */
#define CS_CODE_RECYCLEOBJ      41

/*
 *      To client: object_num, type, imageset, size, x, y, z,
 *                 heading, pitch, bank, velocity, velocity_heading,
 *                 velocity_pitch, velocity_bank, current_frame
 *      To server: *not sent*
 */
#define CS_CODE_POSEOBJ		42

/*
 *      To client: object_num, type, imageset, size, x, y, z,
 *                 heading, pitch, bank, velocity, velocity_heading,
 *                 velocity_pitch, velocity_bank, current_frame
 *      To server: *not sent*
 */
#define CS_CODE_FORCEPOSEOBJ	43


/*
 *	To client: propriatery_code [arg...]
 *	To server: propriatery_code [arg...]
 */
#define CS_CODE_EXT		80




/*
 *	Systems Message Codes:
 *
 *	These are prefix codes for each CS_CODE_SYSMESSAGE.
 *	Client and server should NOT send any message with a prefix code
 *	not listed here.
 *
 *	Example for a CS_CODE_SYSMESSAGE using
 *	CS_SYSMESG_LOGINFAIL:
 *
 *	    "31 10 Login failed."
 *
 *	The message (third optional argument) must be less than
 *	CS_MESG_MAX in bytes.
 */
#define CS_SYSMESG_CODE_ERROR		0	/* General error. */
#define CS_SYSMESG_CODE_WARNING		1	/* General warning. */

#define CS_SYSMESG_CODE_LOGINFAIL	10
#define CS_SYSMESG_CODE_LOGINSUCC	11

#define CS_SYSMESG_CODE_ABNDISCON	15	/* Abnormal disconnect. */

#define CS_SYSMESG_CODE_BADVALUE	20
#define CS_SYSMESG_CODE_BADARG		21

/* More to come... */







#endif /* CS_H */
