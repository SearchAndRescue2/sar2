#include <string.h>
#include <stdlib.h>
#include <sys/types.h>

#include "../include/string.h"
#include "../include/strexp.h"

#include "gw.h"
#include "messages.h"
#include "cmd.h"
#include "sar.h"
#include "sarmemory.h"


void SARCmdMemory(SAR_CMD_PROTOTYPE);


#define ATOI(s)         (((s) != NULL) ? atoi(s) : 0)
#define ATOL(s)         (((s) != NULL) ? atol(s) : 0)
#define ATOF(s)         (((s) != NULL) ? (float)atof(s) : 0.0f)
#define STRDUP(s)       (((s) != NULL) ? strdup(s) : NULL)

#define MAX(a,b)        (((a) > (b)) ? (a) : (b))
#define MIN(a,b)        (((a) < (b)) ? (a) : (b))
#define CLIP(a,l,h)     (MIN(MAX((a),(l)),(h)))
#define STRLEN(s)       (((s) != NULL) ? ((int)strlen(s)) : 0)

#define RADTODEG(r)     ((r) * 180.0 / PI)
#define DEGTORAD(d)     ((d) * PI / 180.0)

#define STR_IS_YES(s)	(StringIsYes(s))

#define NOTIFY(s)			\
{ if(SAR_CMD_IS_VERBOSE(flags) &&	\
     (scene != NULL) && ((s) != NULL)	\
  ) { SARMessageAdd(scene, (s)); }	\
}


/*
 *	Memory management.
 */
void SARCmdMemory(SAR_CMD_PROTOTYPE)
{
	int argc;
	char **argv;
	char *parm, *val;
	Boolean got_match = False;
	sar_core_struct *core_ptr = SAR_CORE(data);
	sar_scene_struct *scene = core_ptr->scene;

	/* If no arguments then print memory status */
	if(*arg == '\0')
	{
	    char *s = (char *)malloc(1024 * sizeof(char));
	    sar_memory_stat_struct stat_buf;

	    SARMemoryStat(
		core_ptr, scene,
		core_ptr->object, core_ptr->total_objects,
		&stat_buf
	    );

	    sprintf(s,
"T: %ld  Scn: %ld  Obj: %ld  Tex: %ld  Mdl: %ld",
		stat_buf.total,
		stat_buf.scene,
		stat_buf.object,
		stat_buf.texture,
		stat_buf.vmodel
	    );
	    NOTIFY(s);
	    sprintf(s,
"Objs: %i  Texs: %i  Mdls: %i",
		stat_buf.nobjects,
		stat_buf.ntextures,
		stat_buf.nvmodels
	    );
	    NOTIFY(s);

	    free(s);
	    return;
	}

	/* Parse arguments, format:
	 *
	 * <parameter>=<value>
	 */
	argv = strchrexp(arg, '=', &argc);
	if(argv == NULL)
	    return;

	/* First argument is option/parameter. */
	if(argc < 2)
	{
	    NOTIFY(
		"Usage: mem <parameter>=<value>"
	    );
	    strlistfree(argv, argc);
	    return;
	}

	/* Get links to parm and val in the strv array. */
	if(argc > 0)
	    parm = argv[0];
	else
	    parm = NULL;

	if(argc > 1)
	    val = argv[1];
	else
	    val = NULL;

	if((parm == NULL) || (val == NULL))
	{
	    NOTIFY(
		"Usage: memory <parameter>=<value>"
	    );
	    strlistfree(argv, argc);
	    return;
	} 

	strstrip(parm);
	strstrip(val);


	/* Begin handling parameter */





/* Add other option parameters here */

	if(got_match)
	{
	    char *s = (char *)malloc(
		(80 + STRLEN(parm) + STRLEN(val)) * sizeof(char)
	    );
	    sprintf(
		s,
"Parameter \"%s\" set to \"%s\".",
		parm, val
	    );
	    NOTIFY(s);
	    free(s);
	}
	else
	{
	    char *s = (char *)malloc(
		(80 + STRLEN(parm)) * sizeof(char)
	    );
	    sprintf(
		s,
"%s: No such parameter.",
		parm
	    );
	    NOTIFY(s);
	    free(s);
	}

	/* Delete exploded argument strings */
	strlistfree(argv, argc);
	argv = NULL;
	argc = 0;
	parm = NULL;
	val = NULL;
}
