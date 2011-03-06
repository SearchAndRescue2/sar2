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
#include "weather.h"
#include "config.h"


int SARWeatherLoadFromFile(
	sar_weather_data_struct *w,
	const char *filename
);

#define ATOI(s)         (((s) != NULL) ? atoi(s) : 0)
#define ATOL(s)         (((s) != NULL) ? atol(s) : 0)
#define ATOF(s)         (((s) != NULL) ? (float)atof(s) : 0.0f)
#define STRDUP(s)       (((s) != NULL) ? strdup(s) : NULL)

#define MAX(a,b)        (((a) > (b)) ? (a) : (b))
#define MIN(a,b)        (((a) < (b)) ? (a) : (b))
#define CLIP(a,l,h)     (MIN(MAX((a),(l)),(h)))


/*
 *	Loads weather presets list from file.
 */
int SARWeatherLoadFromFile(
	sar_weather_data_struct *w,
	const char *filename
)
{
	int i;
	FILE *fp;
	char *buf = NULL;
	struct stat stat_buf;

	sar_weather_data_struct *wd = w;
	sar_weather_data_entry_struct *wdp_ptr = NULL;
	sar_color_struct *color_ptr = NULL;
	sar_cloud_layer_struct *cloud_layer_ptr = NULL;
	sar_cloud_bb_struct *cloud_bb_ptr = NULL;
	double value[10];
	

	if((filename == NULL) || (wd == NULL))
	    return(-1);

	/* Delete any existing preset weather data entries */
	for(i = 0; i < wd->total_presets; i++)
	    SARWeatherEntryDelete(
		wd, wd->preset[i]
	    );

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

	/* Open weather presets file for reading */
	fp = FOpen(filename, "rb");
	if(fp == NULL)
	{
	    fprintf(
		stderr,
		"%s: Unable to open the Weather Presets file for reading.\n",
		filename
	    );
	    return(-1);
	}

#define DO_SET_COLOR	{		\
 if(color_ptr != NULL) {		\
  color_ptr->r = (float)value[0];	\
  color_ptr->g = (float)value[1];	\
  color_ptr->b = (float)value[2];	\
  color_ptr->a = (float)value[3];	\
 }					\
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

	    /* Begin handling parameter */

	    /* Version? */
	    if(!strcasecmp(buf, "Version"))
	    {
		FGetValuesF(fp, value, 2);

		/* Ignore */
	    }
	    /* Add a new preset weather data entry? */
	    else if(!strcasecmp(buf, "PresetAdd"))
	    {
		char *strptr = FGetString(fp);

		wdp_ptr = NULL;

		/* Allocate a new pointer and structure */
		i = wd->total_presets;
		wd->total_presets++;
		wd->preset = (sar_weather_data_entry_struct **)realloc(
		    wd->preset,
		    wd->total_presets * sizeof(sar_weather_data_entry_struct *)
		);
		if(wd->preset == NULL)
		{
		    wd->total_presets = 0;
		}
		else
		{
		    wdp_ptr = (sar_weather_data_entry_struct *)calloc(
			1,
			sizeof(sar_weather_data_entry_struct)
		    );
		    wd->preset[i] = wdp_ptr;
		}

		/* Set perset name */
		if(wdp_ptr != NULL)
		    wdp_ptr->name = strptr;
		else
		    free(strptr);
	    }

	    /* Sky color nominal */
	    else if(!strcasecmp(buf, "ColorSkyNominal"))
	    {
		FGetValuesF(fp, value, 4);
		if(wdp_ptr != NULL)
		{
		    color_ptr = &wdp_ptr->sky_nominal_color;
		    DO_SET_COLOR
		}
	    }
	    /* Sky color brighten */
	    else if(!strcasecmp(buf, "ColorSkyBrighten"))
	    {
		FGetValuesF(fp, value, 4);
		if(wdp_ptr != NULL)
		{
		    color_ptr = &wdp_ptr->sky_brighten_color;
		    DO_SET_COLOR
		}
	    }
	    /* Sky color darken */
	    else if(!strcasecmp(buf, "ColorSkyDarken"))
	    { 
		FGetValuesF(fp, value, 4);
		if(wdp_ptr != NULL)
		{
		    color_ptr = &wdp_ptr->sky_darken_color;
		    DO_SET_COLOR
		}
	    }
	    /* Color of stars */
	    else if(!strcasecmp(buf, "ColorStarLow"))
	    {
		FGetValuesF(fp, value, 4);
		if(wdp_ptr != NULL)
		{
		    color_ptr = &wdp_ptr->star_low_color;
		    DO_SET_COLOR
		}
	    }
	    else if(!strcasecmp(buf, "ColorStarHigh"))
	    {
		FGetValuesF(fp, value, 4);
		if(wdp_ptr != NULL)
		{
		    color_ptr = &wdp_ptr->star_high_color;
		    DO_SET_COLOR
		}
	    }
	    else if(!strcasecmp(buf, "ColorStar"))
	    {
		FGetValuesF(fp, value, 4);
		if(wdp_ptr != NULL)
		{
		    color_ptr = &wdp_ptr->star_low_color;
		    DO_SET_COLOR
		    color_ptr = &wdp_ptr->star_high_color;
		    DO_SET_COLOR
		}
	    }
	    /* Color of sun */
	    else if(!strcasecmp(buf, "ColorSunLow"))
	    {
		FGetValuesF(fp, value, 4);
		if(wdp_ptr != NULL)
		{
		    color_ptr = &wdp_ptr->sun_low_color;
		    DO_SET_COLOR
		}
	    }
	    else if(!strcasecmp(buf, "ColorSunHigh"))
	    {
		FGetValuesF(fp, value, 4);
		if(wdp_ptr != NULL)
		{
		    color_ptr = &wdp_ptr->sun_high_color;
		    DO_SET_COLOR
		}
	    }
	    else if(!strcasecmp(buf, "ColorSun"))
	    {
		FGetValuesF(fp, value, 4);
		if(wdp_ptr != NULL)
		{
		    color_ptr = &wdp_ptr->sun_low_color;
		    DO_SET_COLOR
		    color_ptr = &wdp_ptr->sun_high_color;
		    DO_SET_COLOR
		}
	    }
	    /* Color of moon */
	    else if(!strcasecmp(buf, "ColorMoonLow"))
	    {
		FGetValuesF(fp, value, 4);
		if(wdp_ptr != NULL)
		{
		    color_ptr = &wdp_ptr->moon_low_color;
		    DO_SET_COLOR
		}
	    }
	    else if(!strcasecmp(buf, "ColorMoonHigh"))
	    {
		FGetValuesF(fp, value, 4);
		if(wdp_ptr != NULL)
		{
		    color_ptr = &wdp_ptr->moon_high_color;
		    DO_SET_COLOR
		}
	    }
	    else if(!strcasecmp(buf, "ColorMoon"))
	    {
		FGetValuesF(fp, value, 4);
		if(wdp_ptr != NULL)
		{
		    color_ptr = &wdp_ptr->moon_low_color;
		    DO_SET_COLOR
		    color_ptr = &wdp_ptr->moon_high_color;
		    DO_SET_COLOR
		}
	    }

	    /* Atmosphere distance coefficient (from far to near) */
	    else if(!strcasecmp(buf, "AtmosphereDistanceCoefficient") ||
		    !strcasecmp(buf, "AtmosphereDistanceCoeff")
	    )
	    {
		FGetValuesF(fp, value, 1);
		if(wdp_ptr != NULL)
		    wdp_ptr->atmosphere_dist_coeff = (float)value[0];
	    }
	    /* Atmosphere density coefficient */
	    else if(!strcasecmp(buf, "AtmosphereDensityCoefficient") ||
		    !strcasecmp(buf, "AtmosphereDensityCoeff")      
	    )
	    {
		FGetValuesF(fp, value, 1);
		if(wdp_ptr != NULL)
		    wdp_ptr->atmosphere_density_coeff = (float)value[0];
	    }

	    /* Rain density */
	    else if(!strcasecmp(buf, "RainDensityCoefficient") ||
		    !strcasecmp(buf, "RainDensityCoeff")
	    )
	    {
		FGetValuesF(fp, value, 1);
		if(wdp_ptr != NULL)
		    wdp_ptr->rain_density_coeff = (float)value[0];
	    }

	    /* Add a new cloud layer? */
	    else if(!strcasecmp(buf, "CloudLayerAdd"))
	    {
		FSeekNextLine(fp);	/* No arguments */

		cloud_layer_ptr = NULL;	/* Reset */

		if(wdp_ptr != NULL)
		{
		    /* Allocate a new cloud layer */
		    i = wdp_ptr->total_cloud_layers;
		    wdp_ptr->total_cloud_layers++;
		    wdp_ptr->cloud_layer = (sar_cloud_layer_struct **)realloc(
			wdp_ptr->cloud_layer,
			wdp_ptr->total_cloud_layers * sizeof(sar_cloud_layer_struct *)
		    );
		    if(wdp_ptr->cloud_layer == NULL)
		    {
			wdp_ptr->total_cloud_layers = 0;
		    }
		    else
		    {
			cloud_layer_ptr = (sar_cloud_layer_struct *)calloc(
			    1,
			    sizeof(sar_cloud_layer_struct)
			);
			wdp_ptr->cloud_layer[i] = cloud_layer_ptr;
		    }
		}
	    }
	    else if(!strcasecmp(buf, "CloudLayerSize"))
	    {
		FGetValuesF(fp, value, 2);

		if(cloud_layer_ptr != NULL)
		{
		    cloud_layer_ptr->tile_width = MAX((int)value[0], 100);
		    cloud_layer_ptr->tile_height = MAX((int)value[1], 100);
		}
	    }
	    else if(!strcasecmp(buf, "CloudLayerRange"))
	    {
		FGetValuesF(fp, value, 1);

		if(cloud_layer_ptr != NULL)
		    cloud_layer_ptr->range = (float)MAX(value[0], 100.0);
	    }
	    else if(!strcasecmp(buf, "CloudLayerAltitude"))
	    {
		FGetValuesF(fp, value, 1);

		if(cloud_layer_ptr != NULL)
		    cloud_layer_ptr->z = (float)SFMFeetToMeters(value[0]);
	    }
	    else if(!strcasecmp(buf, "CloudLayerTexture"))
	    {
		char *strptr = FGetString(fp);

		if(strptr != NULL)
		{
		    if(cloud_layer_ptr != NULL)
		    {
			free(cloud_layer_ptr->tex_name);
			cloud_layer_ptr->tex_name = strptr;
		    }
		    else
		    {
			free(strptr);
		    }
		}
	    }

	    /* Add a new cloud `billboard' object? */
	    else if(!strcasecmp(buf, "CloudBBAdd") ||
		    !strcasecmp(buf, "CloudBillboardAdd") ||
		    !strcasecmp(buf, "CloudBillBoardAdd")
	    )
	    {
		FSeekNextLine(fp);      /* No arguments */

		cloud_bb_ptr = NULL; /* Reset */

		if(wdp_ptr != NULL)
		{
		    /* Allocate a new cloud `billboard' object */
		    i = wdp_ptr->total_cloud_bbs;
		    wdp_ptr->total_cloud_bbs++;
		    wdp_ptr->cloud_bb = (sar_cloud_bb_struct **)realloc(
			wdp_ptr->cloud_bb,
			wdp_ptr->total_cloud_bbs * sizeof(sar_cloud_bb_struct *)
		    );
		    if(wdp_ptr->cloud_bb == NULL)
		    {
			wdp_ptr->total_cloud_bbs = 0;
		    }
		    else
		    {
			cloud_bb_ptr = (sar_cloud_bb_struct *)calloc(
			    1,
			    sizeof(sar_cloud_bb_struct)
			);
			wdp_ptr->cloud_bb[i] = cloud_bb_ptr;
		    }
		}
	    }

	    else if(!strcasecmp(buf, "CloudBBSize"))
	    {
		FGetValuesF(fp, value, 4);

		if(cloud_bb_ptr != NULL)
		{
		    cloud_bb_ptr->tile_width = (int)MAX(value[0], 100);
		    cloud_bb_ptr->tile_height = (int)MAX(value[1], 100);
		    cloud_bb_ptr->width = (float)MAX(value[2], 100);
		    cloud_bb_ptr->height = (float)MAX(value[3], 100);
		}
	    }
	    else if(!strcasecmp(buf, "CloudBBOffset"))
	    {
		FGetValuesF(fp, value, 3);
		if(cloud_bb_ptr != NULL)
		{
		    sar_position_struct *pos = &cloud_bb_ptr->pos;
		    pos->x = (float)value[0];
		    pos->y = (float)value[1];
		    pos->z = (float)value[2];
		}
	    }
	    else if(!strcasecmp(buf, "CloudBBTexture"))
	    {
		char *s = FGetString(fp);
		if(s != NULL)
		{
		    if(cloud_bb_ptr != NULL)
		    {
			free(cloud_bb_ptr->tex_name);
			cloud_bb_ptr->tex_name = s;
		    }
		    else
		    {
			free(s);
		    }
		}
	    }
	    else if(!strcasecmp(buf, "CloudBBLighteningInterval") ||
		    !strcasecmp(buf, "CloudBBLighteningInt")
	    )
	    {
		FGetValuesF(fp, value, 2);
		if(cloud_bb_ptr != NULL)
		{
		    cloud_bb_ptr->lightening_min_int = (time_t)value[0];
		    cloud_bb_ptr->lightening_max_int = (time_t)value[1];
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

	/* Close weather file */            
	FClose(fp);

	return(0);
}
