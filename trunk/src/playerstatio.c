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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "../include/fio.h"
#include "../include/string.h"

#include "sar.h"
#include "playerstatio.h"
#include "config.h"


int SARPlayerStatsLoadFromFile(
	sar_player_stat_struct ***list, int *total,
	const char *filename
);
int SARPlayerStatsSaveToFile(
	sar_player_stat_struct **list, int total,
	const char *filename
);


#define ATOI(s)         (((s) != NULL) ? atoi(s) : 0)
#define ATOL(s)         (((s) != NULL) ? atol(s) : 0)
#define ATOF(s)         (((s) != NULL) ? atof(s) : 0.0f)
#define STRDUP(s)       (((s) != NULL) ? strdup(s) : NULL)

#define MAX(a,b)        (((a) > (b)) ? (a) : (b))
#define MIN(a,b)        (((a) < (b)) ? (a) : (b))
#define CLIP(a,l,h)     (MIN(MAX((a),(l)),(h)))

#ifndef SAR_COMMENT_CHAR
# define SAR_COMMENT_CHAR	'#'
#endif

#ifndef SAR_CFG_DELIM_CHAR
# define SAR_CFG_DELIM_CHAR	'='
#endif


/*
 *	Loads players list from file.
 */
int SARPlayerStatsLoadFromFile(
	sar_player_stat_struct ***list, int *total,
	const char *filename
)
{
	FILE *fp;
	char *buf = NULL;
	struct stat stat_buf;
	sar_player_stat_struct *pstat = NULL;


	if((list == NULL) || (total == NULL) || (filename == NULL))
	    return(-1);

	if(stat(filename, &stat_buf))
	    return(-1);

#ifdef S_ISDIR
	if(S_ISDIR(stat_buf.st_mode))
	{
	    fprintf(
		stderr,
		"%s: Is a directory.\n",
		filename
	    );
	    return(-1);
	}
#endif	/* S_ISDIR */

	/* Open player stats file for reading */
	fp = FOpen(filename, "rb");
	if(fp == NULL)
	{
	    fprintf(
		stderr,
"%s: Unable to open the Player Stats file for reading.\n",
		filename
	    );
	    return(-1);
	}

	do
	{
	    buf = FSeekNextParm(
		fp,
		buf,
		SAR_COMMENT_CHAR,
		SAR_CFG_DELIM_CHAR
	    );
	    if(buf == NULL)
	        break;

	    /* Version */
	    if(!strcasecmp(buf, "Version"))
	    {
		double vf[3];
		FGetValuesF(fp, vf, 3);

	    }

	    /* PilotNew */
	    else if(!strcasecmp(buf, "PilotNew"))
	    {
		char *s = FGetString(fp);

		int i = MAX(*total, 0);
		*total = i + 1;

		*list = (sar_player_stat_struct **)realloc(
		    *list,
		    *total * sizeof(sar_player_stat_struct *)
		);
		if(*list == NULL)
		{
		    *total = 0;
		    pstat = NULL;
		}
		else
		{
		    (*list)[i] = pstat = SARPlayerStatNew(s, 0);
		}

		free(s);
	    }

	    /* Score */
	    else if(!strcasecmp(buf, "Score"))
	    {
		double vf[1];
		FGetValuesF(fp, vf, 1);

		if(pstat != NULL)
		    pstat->score = MAX((int)vf[0], 0);
	    }
	    /* Crashes */
	    else if(!strcasecmp(buf, "Crashes"))
	    {
		double vf[1];
		FGetValuesF(fp, vf, 1);

		if(pstat != NULL)
		    pstat->crashes = MAX((int)vf[0], 0);
	    }
	    /* Rescues */
	    else if(!strcasecmp(buf, "Rescues"))
	    {
		double vf[1];
		FGetValuesF(fp, vf, 1);

		if(pstat != NULL)
		    pstat->rescues = MAX((int)vf[0], 0);
	    }
	    /* MissionsSucceeded */
	    else if(!strcasecmp(buf, "MissionsSucceeded"))
	    {
		double vf[1];
		FGetValuesF(fp, vf, 1);

		if(pstat != NULL)
		    pstat->missions_succeeded = MAX((int)vf[0], 0);
	    }
	    /* MissionsFailed */
	    else if(!strcasecmp(buf, "MissionsFailed"))
	    {
		double vf[1];
		FGetValuesF(fp, vf, 1);

		if(pstat != NULL)
		    pstat->missions_failed = MAX((int)vf[0], 0);
	    }

	    /* TimeAloft */
	    else if(!strcasecmp(buf, "TimeAloft"))
	    {
		double vf[1];
		FGetValuesF(fp, vf, 1);

		if(pstat != NULL)
		    pstat->time_aloft = MAX((time_t)vf[0], 0l);
	    }

	    /* LastMission */
	    else if(!strcasecmp(buf, "LastMission"))
	    {
		double vf[1];
		FGetValuesF(fp, vf, 1);

		if(pstat != NULL)
		    pstat->last_mission = MAX((time_t)vf[0], 0l);
	    }
	    /* LastFreeFlight */
	    else if(!strcasecmp(buf, "LastFreeFlight"))
	    {
		double vf[1];
		FGetValuesF(fp, vf, 1);

		if(pstat != NULL)
		    pstat->last_free_flight = MAX((time_t)vf[0], 0l);
	    }


	    else
	    {
	        fprintf(
		    stderr,
	            "%s: Unsupported parameter \"%s\".\n",
	            filename, buf
	        );
		FSeekNextLine(fp);
	    }

	} while(1);

	free(buf);

	/* Close file */
	FClose(fp);

	return(0);
}

/*
 *	Saves the pilots list to file.
 */
int SARPlayerStatsSaveToFile(
	sar_player_stat_struct **list, int total,
	const char *filename
)
{
	int i;
	FILE *fp;
	struct stat stat_buf;
	const sar_player_stat_struct *pstat;


	if(filename == NULL)
	    return(-1);

	if(!stat(filename, &stat_buf))
	{
#if defined(S_ISDIR)
	    if(S_ISDIR(stat_buf.st_mode))
	    {
		fprintf(
		    stderr,
		    "%s: Is a directory.\n",
		    filename
		);
		return(-1);
	    }
#endif	/* S_ISDIR */
	}

	/* Open player stats file for writing */
	fp = FOpen(filename, "wb");
	if(fp == NULL)
	    return(-1);

	/* Change permissions so that only owner may read and write */
#if defined(__MSW__)

#else
	if(fchmod(fileno(fp), S_IRUSR | S_IWUSR))
	{
	    fprintf(
		stderr,
		"%s: Warning: Unable to set permissions.\n",
		filename
	    );
	}
#endif

#define PUTCR	fputc('\n', fp);

	/* Write header */
	fprintf(fp,
"# %s - Pilots List\n\
#\n\
#       This file is automatically generated, you should only modify\n\
#       these values from the Options menu.\n\
#\n\
\n",
	    PROG_NAME_FULL
	);

	/* Version */
	fprintf(
	    fp,
	    "Version = %i %i %i",
	    PROG_VERSION_MAJOR, PROG_VERSION_MINOR, PROG_VERSION_RELEASE
	);
	PUTCR

	/* Iterate through each player */
	for(i = 0; i < total; i++)
	{
	    pstat = list[i];
	    if(pstat == NULL)
		continue;

	    fprintf(
		fp,
		"PilotNew = %s",
		pstat->name
	    );
	    PUTCR

	    fprintf(
		fp,
		"\tScore = %i",
		pstat->score
	    );
	    PUTCR
	    fprintf(
		fp,
		"\tCrashes = %i",
		pstat->crashes
	    );
	    PUTCR
	    fprintf(
		fp,
		"\tRescues = %i",
		pstat->rescues
	    );
	    PUTCR
	    fprintf(
		fp,
		"\tMissionsSucceeded = %i",
		pstat->missions_succeeded
	    );
	    PUTCR
	    fprintf(
		fp,
		"\tMissionsFailed = %i",
		pstat->missions_failed
	    );
	    PUTCR

	    fprintf(
		fp,
		"\tTimeAloft = %ld",
		pstat->time_aloft
	    );
	    PUTCR

	    fprintf(
		fp,
		"\tLastMission = %ld",
		pstat->last_mission
	    );
	    PUTCR
	    fprintf(
		fp,
		"\tLastFreeFlight = %ld",
		pstat->last_free_flight
	    );
	    PUTCR
	    PUTCR



	}

#undef PUTCR

	/* Close file */
	FClose(fp);

	return(0);
}
