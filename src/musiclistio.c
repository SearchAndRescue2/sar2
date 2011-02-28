#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "../include/fio.h"
#include "../include/disk.h"
#include "../include/string.h"

#include "sound.h"
#include "musiclistio.h"
#include "sar.h"
#include "config.h"


sar_music_ref_struct *SARMusicMatchPtr(
	sar_music_ref_struct **ml, int total,
	int id, int *music_ref_num
);

void SARMusicListDeleteAll(
	sar_music_ref_struct ***ml, int *total
);
int SARMusicListLoadFromFile(
	const char *filename,
	sar_music_ref_struct ***ml, int *total
);


#define ATOI(s)		(((s) != NULL) ? atoi(s) : 0)
#define ATOL(s)		(((s) != NULL) ? atol(s) : 0)
#define ATOF(s)		(((s) != NULL) ? (float)atof(s) : 0.0f)
#define STRDUP(s)	(((s) != NULL) ? strdup(s) : NULL)

#define MAX(a,b)	(((a) > (b)) ? (a) : (b))
#define MIN(a,b)	(((a) < (b)) ? (a) : (b))
#define CLIP(a,l,h)	(MIN(MAX((a),(l)),(h)))
#define STRLEN(s)	(((s) != NULL) ? (int)strlen(s) : 0)
#define STRISEMPTY(s)	(((s) != NULL) ? (*(s) == '\0') : 1)

#define RADTODEG(r)	((r) * 180.0 / PI)
#define DEGTORAD(d)	((d) * PI / 180.0)


/*
 *	Matches the music reference structure in the given list with
 *	the given id code.
 */
sar_music_ref_struct *SARMusicMatchPtr(
	sar_music_ref_struct **ml, int total,
	int id, int *music_ref_num
)
{
	int i;
	sar_music_ref_struct *mr_ptr;


	if(music_ref_num != NULL)
	    *music_ref_num = -1;

	if((ml == NULL) || (total < 1) || (id < 0))
	    return(NULL);

	for(i = 0; i < total; i++)
	{
	    mr_ptr = ml[i];
	    if(mr_ptr == NULL)
		continue;

	    if(mr_ptr->id == id)
	    {
		if(music_ref_num != NULL)
		    *music_ref_num = i;
		return(mr_ptr);
	    }
	}

	return(NULL);
}


/*
 *	Deletes the Music List.
 */
void SARMusicListDeleteAll(
	sar_music_ref_struct ***ml, int *total
)
{
	int i;
	sar_music_ref_struct *mr_ptr;

	if((ml == NULL) || (total == NULL))
	    return;

	for(i = 0; i < (*total); i++)
	{
	    mr_ptr = (*ml)[i];
	    if(mr_ptr == NULL)
		continue;

	    free(mr_ptr->filename);
	    free(mr_ptr);
	}

	if(*ml != NULL)
	{
	    free(*ml);
	    *ml = NULL;
	}
	*total = 0;
}

/*
 *      Loads the Music List from file.
 */
int SARMusicListLoadFromFile(
	const char *filename,
	sar_music_ref_struct ***ml, int *total
)
{
	int i;
	FILE *fp;
	char *buf = NULL;
	struct stat stat_buf;  
	double value[10];
	sar_music_ref_struct *mr_ptr = NULL;

	if(STRISEMPTY(filename) || (ml == NULL) || (total == NULL))
	    return(-1);

	/* Delete existing Music List */
	SARMusicListDeleteAll(ml, total);

	/* Check if the file exists and get its stats */
	if(stat(filename, &stat_buf))
	{
            char *s = STRDUP(strerror(errno));
            if(s == NULL)
                s = STRDUP("no such file");
            *s = toupper(*s);
            fprintf(
                stderr,
                "%s: %s.\n",
                filename, s
            );
            free(s);
	    return(-1);
	}
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
#endif  /* S_ISDIR */

	/* Open the music list file for reading */
	fp = FOpen(filename, "rb");
	if(fp == NULL)   
	{
	    fprintf(
		stderr,
		"%s: Unable to open the Music List file for reading.\n",
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

	    if(!strcasecmp(buf, "Version"))
	    {
		FGetValuesF(fp, value, 3);
	    }

	    else if(!strcasecmp(buf, "MusicAdd"))
	    {
		char *s = FGetString(fp);

		i = MAX(*total, 0);
		*total = i + 1;
		*ml = (sar_music_ref_struct **)realloc(
		    *ml,
		    (*total) * sizeof(sar_music_ref_struct *)
		);
		if(*ml == NULL)
		{
		    *total = 0;
		    break;
		}

		(*ml)[i] = mr_ptr = SAR_MUSIC_REF(calloc(
		    1, sizeof(sar_music_ref_struct)
		));
		mr_ptr->id = ATOI(s);

		free(s);
	    }
	    else if(!strcasecmp(buf, "MusicFileName"))
	    {
		char *s = FGetString(fp);
		if(mr_ptr != NULL)
		{
		    free(mr_ptr->filename);
		    mr_ptr->filename = STRDUP(s);
		}
		free(s);
	    }
	    else if(!strcasecmp(buf, "MusicFlags"))
	    {
		char *s = FGetString(fp);
		const char *s2 = s;

		/* Iterate through value string, checking each flag */
		while((s2 != NULL) ? (*s2 != '\0') : 0)
		{
		    if(strcasepfx(s2, "repeating"))
			mr_ptr->flags |= SAR_MUSIC_REF_FLAGS_REPEAT;
		    else if(strcasepfx(s2, "fade_in") ||
			    strcasepfx(s2, "fadein")
		    )
			mr_ptr->flags |= SAR_MUSIC_REF_FLAGS_FADE_IN;
		    else if(strcasepfx(s2, "fade_out") || 
			    strcasepfx(s2, "fadeout")
		    )
			mr_ptr->flags |= SAR_MUSIC_REF_FLAGS_FADE_OUT;

		    while(!ISBLANK(*s2) && (*s2 != '\0'))
			s2++;
		    while(ISBLANK(*s2))
			s2++;
		}

		free(s);
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
