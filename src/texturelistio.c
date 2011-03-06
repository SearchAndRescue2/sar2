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
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>
#include <sys/types.h> 
#include <sys/stat.h>

#include "../include/fio.h"
#include "../include/disk.h"
#include "../include/string.h"

#include "texturelistio.h"
#include "sar.h"
#include "config.h"


void SARTextureListDeleteAll(
	sar_texture_name_struct ***tl, int *total
);
int SARTextureListLoadFromFile(
	const char *filename,
	sar_texture_name_struct ***tl, int *total
);


#define ATOI(s)         (((s) != NULL) ? atoi(s) : 0)
#define ATOL(s)         (((s) != NULL) ? atol(s) : 0)
#define ATOF(s)         (((s) != NULL) ? atof(s) : 0.0f)
#define STRDUP(s)       (((s) != NULL) ? strdup(s) : NULL)

#define MAX(a,b)        (((a) > (b)) ? (a) : (b))
#define MIN(a,b)        (((a) < (b)) ? (a) : (b))
#define CLIP(a,l,h)     (MIN(MAX((a),(l)),(h)))
#define STRLEN(s)       (((s) != NULL) ? strlen(s) : 0)
#define STRISEMPTY(s)   (((s) != NULL) ? (*(s) == '\0') : 1)


/*
 *	Deletes the Textures List.
 */
void SARTextureListDeleteAll(
	sar_texture_name_struct ***tl, int *total
)
{
	int i;
	sar_texture_name_struct *tn_ptr;

	if((tl == NULL) || (total == NULL))
	    return;

	for(i = 0; i < (*total); i++)
	{
	    tn_ptr = (*tl)[i];
	    if(tn_ptr == NULL)
		continue;

	    free(tn_ptr->name);
	    free(tn_ptr->filename);
	    free(tn_ptr);
	}

	if(*tl != NULL)
	{
	    free(*tl);
	    *tl = NULL;
	}
	*total = 0;
}


/*
 *	Loads the Texture List from file.
 */
int SARTextureListLoadFromFile(
	const char *filename,
	sar_texture_name_struct ***tl, int *total
)
{
	int i;
        FILE *fp;
        char *buf = NULL;
        struct stat stat_buf;
	double value[10];
	sar_texture_name_struct *tn_ptr = NULL;

	if((filename == NULL) || (tl == NULL) || (total == NULL))
	    return(-1);

	/* Delete any existing Texture List */
	SARTextureListDeleteAll(tl, total);

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
#endif	/* S_ISDIR */

	/* Open texture list file for reading */
        fp = FOpen(filename, "rb");
        if(fp == NULL)
        {
            fprintf(
		stderr,
		"%s: Unable to open the Texture List file for reading.\n",
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
                FGetValuesF(fp, value, 2);

            }

	    else if(!strcasecmp(buf, "TextureAdd"))
            {
                char *s = FGetString(fp);

		i = *total;
		*total = i + 1;
		*tl = (sar_texture_name_struct **)realloc(
		    *tl,
		    (*total) * sizeof(sar_texture_name_struct *)
		);
		if(*tl == NULL)
		{
		    *total = 0;
		    tn_ptr = NULL;
		}
		else
		{
		    (*tl)[i] = tn_ptr = (sar_texture_name_struct *)calloc(
			1, sizeof(sar_texture_name_struct)
		    );
		}

		/* Set reference name */
		if(tn_ptr != NULL)
		{
		    free(tn_ptr->name);
		    tn_ptr->name = STRDUP(s);
		}
		free(s);
	    }
            else if(!strcasecmp(buf, "TextureFileName"))
            {
		char *s = FGetString(fp);
                if(tn_ptr != NULL)
		{
		    free(tn_ptr->filename);
                    tn_ptr->filename = STRDUP(s);
		}
		free(s);
            }
            else if(!strcasecmp(buf, "TextureFlags"))
            {
                FGetValuesF(fp, value, 1);

		/* Reserved */
	    }
            else if(!strcasecmp(buf, "TexturePriority"))
            {
                FGetValuesF(fp, value, 1);
                if(tn_ptr != NULL)
                {
                    tn_ptr->priority = CLIP(value[0], 0.0, 1.0);
                } 
            }

            else
            {
                fprintf(
		    stderr,
                    "%s: Unsupported parameter `%s'.\n",
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
