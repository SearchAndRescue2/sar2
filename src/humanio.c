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

#include "obj.h"
#include "human.h"
#include "config.h"


int SARHumanLoadFromFile(sar_human_data_struct *hd, const char *filename);


#define ATOI(s)		(((s) != NULL) ? atoi(s) : 0)
#define ATOL(s)		(((s) != NULL) ? atol(s) : 0)
#define ATOF(s)		(((s) != NULL) ? atof(s) : 0.0f)
#define STRDUP(s)	(((s) != NULL) ? strdup(s) : NULL)

#define MAX(a,b)	(((a) > (b)) ? (a) : (b))
#define MIN(a,b)	(((a) < (b)) ? (a) : (b))
#define CLIP(a,l,h)	(MIN(MAX((a),(l)),(h)))
#define STRLEN(s)	(((s) != NULL) ? strlen(s) : 0)
#define STRISEMPTY(s)	(((s) != NULL) ? (*(s) == '\0') : 1)

#define RADTODEG(r)	((r) * 180.0 / PI)
#define DEGTORAD(d)	((d) * PI / 180.0)


/*
 *	Loads the human presets data from file.
 */
int SARHumanLoadFromFile(sar_human_data_struct *hd, const char *filename)
{
	int i;
	int version_major, version_minor;
	FILE *fp;
	char *buf = NULL;
	struct stat stat_buf;

	sar_human_data_entry_struct *hdp_ptr = NULL;
	sar_color_struct *color_ptr, *palette_ptr = NULL;

	double value[10];


	if(STRISEMPTY(filename) || (hd == NULL))
	    return(-1);

	/* Delete any existing preset human data entries */
	for(i = 0; i < hd->total_presets; i++)
	    SARHumanEntryDelete(hd, hd->preset[i]);
	free(hd->preset);
	hd->preset = NULL;
	hd->total_presets = 0;

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

	/* Open the human presets file */
	fp = FOpen(filename, "rb");
	if(fp == NULL)
	{
	    fprintf(
		stderr,
		"%s: Unable to open the Human Presets file for reading.\n",
		filename
	    );
	    return(-1);
	}

#define DO_SET_COLOR(_p_)	{	\
 if((_p_) != NULL) {			\
  (_p_)->r = (float)value[0];		\
  (_p_)->g = (float)value[1];		\
  (_p_)->b = (float)value[2];		\
  (_p_)->a = (float)value[3];		\
} }

	/* Set version default values.
	 * Historically first human.ini file had no version number:
	 * this version will be named "1.0".
	 */
	version_major = 1;
	version_minor = 0;

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

	    /* Begin handling parameter */

	    /* File version */
	    if(!strcasecmp(buf, "Version"))
	    {
		FGetValuesF(fp, value, 2);
		version_major = (int)value[0];
		version_minor = (int)value[1];
	    }
	    /* PresetAdd */
	    else if(!strcasecmp(buf, "PresetAdd"))
	    {
		char *s = FGetString(fp);
		/* Reset context pointers */
		hdp_ptr = NULL;
		palette_ptr = NULL;

		/* Append a new human data entry */
		i = MAX(hd->total_presets, 0);
		hd->total_presets = i + 1;
		hd->preset = (sar_human_data_entry_struct **)realloc(
		    hd->preset,
		    hd->total_presets * sizeof(sar_human_data_entry_struct *)
		);
		if(hd->preset == NULL)
		{
		    hd->total_presets = 0;
		    break;
		}
		hd->preset[i] = hdp_ptr = SAR_HUMAN_DATA_ENTRY(
		    calloc(1, sizeof(sar_human_data_entry_struct))
		);
		hdp_ptr->name = STRDUP(s);

		/* Set gender to default value (male) for current preset. */
		hdp_ptr->preset_entry_flags &= ~SAR_HUMAN_FLAG_GENDER_FEMALE;

		free(s);
	    }
	    /* Height */
	    else if(!strcasecmp(buf, "Height"))
	    {
		FGetValuesF(fp, value, 1);
		if(hdp_ptr != NULL)
		{
		    hdp_ptr->height = (float)MAX(value[0], 0.0);	/* meters */
		}
	    }
	    /* Mass */
	    else if(!strcasecmp(buf, "Mass"))
	    {
		FGetValuesF(fp, value, 1);

		/* Mass for a 1.90m tall human was set to 54kg in original
		 * "Version 1.0" human.ini file. It was certainly an error, so
		 * let's overwrite it to 90kg.
		 */
		if(version_major == 1 && version_minor == 0)
		    value[0] = 90;

		if(hdp_ptr != NULL)
		{
		    hdp_ptr->mass = (float)MAX(value[0], 0.0);	/* kg */
		}
	    }
	    /* Gender */
	    else if(!strcasecmp(buf, "Gender"))
	    {
		/* Male is default gender.
		* If Female, breasts will be drawn on torso.
		*/

		char *s = FGetString(fp);
		if( (s != NULL) && (hdp_ptr != NULL) )
		{
		    if( !strcasecmp(s, "Female") || !strcasecmp(s, "female") )
			hdp_ptr->preset_entry_flags |= SAR_HUMAN_FLAG_GENDER_FEMALE;
		    else
			/* default */
			hdp_ptr->preset_entry_flags &= ~SAR_HUMAN_FLAG_GENDER_FEMALE;
		}
		free(s);
	    }
	    /* AssistingHumans */
	    else if(!strcasecmp(buf, "AssistingHumans"))
	    {
		FGetValuesF(fp, value, 1);
		if(hdp_ptr != NULL)
		    hdp_ptr->assisting_humans = CLIP((int)value[0], 0, SAR_ASSISTING_HUMANS_MAX);
	    }


	    /* ColorPaletteSelect */
	    else if(!strcasecmp(buf, "ColorPaletteSelect"))
	    {
		char *s = FGetString(fp);
		if(s != NULL)
		{
		    /* Assisting human color palette? */
		    if(!strcasecmp(s, "AssistingHuman") ||
		       !strcasecmp(s, "assisting_human")
		    )
		    {
			palette_ptr = hdp_ptr->assisting_human_color;
		    }
		    /* Standard human color palette? */
		    else if(!strcasecmp(s, "Standard"))
		    {
			palette_ptr = hdp_ptr->color;
		    }
		    else
		    {
			fprintf(
			    stderr,
"%s: ColorPaletteSelect: Warning:\
 Unsupported palette \"%s\".\n",
			    filename, s
			);
		    }

		    free(s);
		}
	    }

	    /* ColorFace */
	    else if(!strcasecmp(buf, "ColorFace"))
	    {
		FGetValuesF(fp, value, 4);
		if((hdp_ptr != NULL) && (palette_ptr != NULL))
		{
		    color_ptr = &palette_ptr[SAR_HUMAN_COLOR_FACE];
		    DO_SET_COLOR(color_ptr)
		}
	    }
	    /* ColorHair */
	    else if(!strcasecmp(buf, "ColorHair"))
	    {
		FGetValuesF(fp, value, 4);
		if((hdp_ptr != NULL) && (palette_ptr != NULL))
		{
		    color_ptr = &palette_ptr[SAR_HUMAN_COLOR_HAIR];
		    DO_SET_COLOR(color_ptr)
		}
	    }
	    /* ColorTorso */
	    else if(!strcasecmp(buf, "ColorTorso"))
	    {
		FGetValuesF(fp, value, 4);
		if((hdp_ptr != NULL) && (palette_ptr != NULL))
		{
		    color_ptr = &palette_ptr[SAR_HUMAN_COLOR_TORSO];
		    DO_SET_COLOR(color_ptr)
		}
	    }
	    /* ColorHips */
	    else if(!strcasecmp(buf, "ColorHips"))
	    {
		FGetValuesF(fp, value, 4);
		if((hdp_ptr != NULL) && (palette_ptr != NULL))
		{
		    color_ptr = &palette_ptr[SAR_HUMAN_COLOR_HIPS];
		    DO_SET_COLOR(color_ptr)
		}
	    }
	    /* ColorLegs */
	    else if(!strcasecmp(buf, "ColorLegs"))
	    {
		FGetValuesF(fp, value, 4);
		if((hdp_ptr != NULL) && (palette_ptr != NULL))
		{
		    color_ptr = &palette_ptr[SAR_HUMAN_COLOR_LEGS];
		    DO_SET_COLOR(color_ptr)
		}
	    }
	    /* ColorFeet */
	    else if(!strcasecmp(buf, "ColorFeet"))
	    {
		FGetValuesF(fp, value, 4);
		if((hdp_ptr != NULL) && (palette_ptr != NULL))
		{
		    color_ptr = &palette_ptr[SAR_HUMAN_COLOR_FEET];
		    DO_SET_COLOR(color_ptr)
		}
	    }
	    /* ColorArms */
	    else if(!strcasecmp(buf, "ColorArms"))
	    {
		FGetValuesF(fp, value, 4);
		if((hdp_ptr != NULL) && (palette_ptr != NULL))
		{
		    color_ptr = &palette_ptr[SAR_HUMAN_COLOR_ARMS];
		    DO_SET_COLOR(color_ptr)
		}
	    }
	    /* ColorHands */
	    else if(!strcasecmp(buf, "ColorHands"))
	    {
		FGetValuesF(fp, value, 4);
		if((hdp_ptr != NULL) && (palette_ptr != NULL))
		{
		    color_ptr = &palette_ptr[SAR_HUMAN_COLOR_HANDS];
		    DO_SET_COLOR(color_ptr)
		}
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

#undef DO_SET_COLOR
	return(0);
}
