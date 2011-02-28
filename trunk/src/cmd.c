#include <stdlib.h>
#include <string.h>

#include "../include/string.h"

#include "gw.h"
#include "messages.h"
#include "cmd.h"
#include "sar.h"

#include "config.h"


/*
 *	Command Reference:
 */
typedef struct _cmd_ref_struct		cmd_ref_struct;
struct _cmd_ref_struct {
	char		*name;
	void		(*func)(SAR_CMD_PROTOTYPE);
	void		*reserved1;
	void		*reserved2;
};


void SARCmdTextInputCB(const char *value, void *data);


#define ATOI(s)         (((s) != NULL) ? atoi(s) : 0)
#define ATOL(s)         (((s) != NULL) ? atol(s) : 0)
#define ATOF(s)         (((s) != NULL) ? atof(s) : 0.0f)
#define STRDUP(s)       (((s) != NULL) ? strdup(s) : NULL)

#define MAX(a,b)        (((a) > (b)) ? (a) : (b))
#define MIN(a,b)        (((a) < (b)) ? (a) : (b))
#define CLIP(a,l,h)     (MIN(MAX((a),(l)),(h)))
#define STRLEN(s)       (((s) != NULL) ? strlen(s) : 0)
#define STRISEMPTY(s)   (((s) != NULL) ? (*(s) == '\0') : 1)

#define RADTODEG(r)	((r) * 180.0 / PI)
#define DEGTORAD(d)	((d) * PI / 180.0)


/*
 *	Command text input prompt callback.
 *
 *	For executing a command via the text input prompt. This will
 *	parse the command value and call the appropriate SARCmd*()
 *	function.
 */
void SARCmdTextInputCB(const char *value, void *data)
{
	char *s;
	const char *arg;
	Boolean matched_command = False;
	unsigned long flags =	SAR_CMD_FLAG_VERBOSE |
				SAR_CMD_FLAG_ISUSER;
	char cmd[SAR_CMD_MAX];
	const cmd_ref_struct *cmd_list_ptr, cmd_list[] = SAR_CMD_FUNC_REF_LIST;	
	void (*cmd_func)(SAR_CMD_PROTOTYPE);
	sar_core_struct *core_ptr = SAR_CORE(data);
	if((value == NULL) || (core_ptr == NULL))
	    return;


	/* Parse command and value */

	/* Get command */
	strncpy(cmd, value, sizeof(cmd));
	cmd[sizeof(cmd) - 1] = '\0';
	s = strchr(cmd, ' ');
	if(s == NULL)
	    s = strchr(cmd, '\t');
	if(s != NULL)
	    *s = '\0';

	/* Get argument */
	arg = strchr(value, ' ');
	if(arg == NULL)
	{
	    arg = "";
	}
	else
	{
	    while(ISBLANK(*arg))
		arg++;
	}

	/* Iterate through list of SARCmd*() functions to see which
	 * command matches the command tagent from the value string
	 */
	for(cmd_list_ptr = cmd_list;
	    cmd_list_ptr->name != NULL;
	    cmd_list_ptr++
	)
	{
	    const char *cmd_name = cmd_list_ptr->name;
	    cmd_func = cmd_list_ptr->func;

	    if(STRISEMPTY(cmd_name) || (cmd_func == NULL))
		break;

	    /* Commands match? */
	    if(!strcasecmp(cmd_name, cmd))
	    {
		/* Call the function */
		cmd_func(core_ptr, arg, flags);
		matched_command = True;
		break;
	    }
	}

	/* Did not match command? */
	if(!matched_command)
	{
	    if(SAR_CMD_IS_VERBOSE(flags))
	    {
		char *s = (char *)malloc(
		    (80 + STRLEN(cmd)) * sizeof(char)
		);
		sprintf(
		    s,
		    "%s: No such command.",
		    cmd
		);
		SARMessageAdd(core_ptr->scene, s);
		free(s);
	    }
	}
}
