/*
 *	               SAR Command Processing
 *
 *	Processes high-level commands, all SARCmd*() functions should
 *	be prototyped here.
 *
 *	cmd.c contains all front end and callback functions.
 *
 *	cmd<name>.c contains functions for specific purposes, the <name>
 *	specifies the command's name.  The functions here should be
 *	called by a function in cmd.c.
 *
 *	To add a new command, first add its prototype here and update
 *	the #define for SAR_CMD_FUNC_REF_LIST (command functions list)
 *	below. Then create a new .c module and name it cmd<name>.c where
 *	<name> is the name of the new command.
 */

#ifndef CMD_H
#define CMD_H

#include <stdio.h>


/*
 *	Prototype for all SARCmd*() function input parameters.
 */
#define SAR_CMD_PROTOTYPE	void *data, const char *arg, unsigned long flags

/*
 *	Flags for SARCmd*() flags input.
 */
#define SAR_CMD_FLAG_VERBOSE	(1 << 0)	/* Print responses/warnings/errors */
#define SAR_CMD_FLAG_ISUSER	(1 << 1)	/* Hint that user issued the command */ 

#define SAR_CMD_IS_VERBOSE(x)	((x) & SAR_CMD_FLAG_VERBOSE)


/* cmdclean.c - scene "cleaning" (deletion of effects objects) */
extern void SARCmdClean(SAR_CMD_PROTOTYPE);

/* cmdfire.c - fire creation */
extern void SARCmdFire(SAR_CMD_PROTOTYPE);

/* cmdoption.c - options setting */
extern void SARCmdOption(SAR_CMD_PROTOTYPE);

/* cmdmemory.c - memory management */
extern void SARCmdMemory(SAR_CMD_PROTOTYPE);

/* cmdset.c - object setting */
extern void SARCmdSet(SAR_CMD_PROTOTYPE);

/* cmdsmoke.c - smoke creation */
extern void SARCmdSmoke(SAR_CMD_PROTOTYPE);

/* cmdtime.c - time setting */
extern void SARCmdTime(SAR_CMD_PROTOTYPE);


/* cmd.c - front end and callback functions */
extern void SARCmdTextInputCB(const char *value, void *data);


/* List of SARCmd*() functions. This is used to match a command name
 * string with a command function. This is used primarly in the
 * functions in cmd.c to call other SARCmd*() functions.
 *
 * The format of this list is four pointers per entry. Where the
 * first pointer is a string specifying the name of the command, the
 * second pointer to the SARCmd*() function, the third and fourth
 * pointers are reserved.
 *
 * The last set of pointers should be four NULL pointers.
 */
#define SAR_CMD_FUNC_REF_LIST	{				\
 { (void *)"clean",	(void *)SARCmdClean,	NULL,	NULL },	\
 { (void *)"fire",	(void *)SARCmdFire,	NULL,	NULL },	\
 { (void *)"memory",	(void *)SARCmdMemory,	NULL,	NULL }, \
 { (void *)"mem",	(void *)SARCmdMemory,	NULL,	NULL },	\
 { (void *)"option",	(void *)SARCmdOption,	NULL,	NULL },	\
 { (void *)"opt",	(void *)SARCmdOption,	NULL,	NULL }, \
 { (void *)"set",	(void *)SARCmdSet,	NULL,	NULL },	\
 { (void *)"smoke",	(void *)SARCmdSmoke,	NULL,	NULL },	\
 { (void *)"time",	(void *)SARCmdTime,	NULL,	NULL },	\
 { NULL,		NULL,			NULL,	NULL }	\
}



#endif	/* CMD_H */
