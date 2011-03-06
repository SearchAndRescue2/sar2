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
#include <sys/types.h> 

#include "../include/string.h"

#include "sfm.h"

#include "obj.h"
#include "objutils.h"
#include "sarreality.h"
#include "weather.h"
#include "sar.h"
#include "config.h"


sar_weather_data_struct *SARWeatherPresetsInit(void *core_ptr);
void SARWeatherPresetsShutdown(sar_weather_data_struct *w);

void SARWeatherEntryDelete(
	sar_weather_data_struct *w, sar_weather_data_entry_struct *entry
);

void SARWeatherSetScenePreset(
	sar_weather_data_struct *w, sar_scene_struct *scene,
	const char *name
);


#define ATOI(s)         (((s) != NULL) ? atoi(s) : 0)
#define ATOL(s)         (((s) != NULL) ? atol(s) : 0)
#define ATOF(s)         (((s) != NULL) ? atof(s) : 0.0f)
#define STRDUP(s)       (((s) != NULL) ? strdup(s) : NULL)

#define MAX(a,b)        (((a) > (b)) ? (a) : (b))
#define MIN(a,b)        (((a) < (b)) ? (a) : (b))
#define CLIP(a,l,h)     (MIN(MAX((a),(l)),(h)))


/*
 *	Creates a new Weather Data.
 */
sar_weather_data_struct *SARWeatherPresetsInit(void *core_ptr)
{
	sar_weather_data_struct *w = (sar_weather_data_struct *)calloc(
	    1, sizeof(sar_weather_data_struct)
	);
	if(w == NULL)
	    return(w);

	w->core_ptr = core_ptr;

	return(w);
}

/*
 *      Deletes the Weather Data.
 */
void SARWeatherPresetsShutdown(sar_weather_data_struct *w)
{
	int i;

	if(w == NULL)
	    return;

	/* Delete each preset weather entry */
	for(i = 0; i < w->total_presets; i++)
	    SARWeatherEntryDelete(
		w, w->preset[i]
	    );

	free(w->preset);
	w->preset = NULL;
	w->total_presets = 0;

	free(w);
}

/*
 *	Deletes the Weather Entry.
 */
void SARWeatherEntryDelete(
	sar_weather_data_struct *w, sar_weather_data_entry_struct *entry  
)
{
	int i;

	if(entry == NULL)
	    return;

	free(entry->name);

	/* Delete all Cloud Layers */
	for(i = 0; i < entry->total_cloud_layers; i++)
	    SARCloudLayerDelete(NULL, entry->cloud_layer[i]);
	free(entry->cloud_layer);

	/* Delete all Cloud BillBoards */
	for(i = 0; i < entry->total_cloud_bbs; i++)
	    SARCloudBBDelete(entry->cloud_bb[i]);
	free(entry->cloud_bb);

	free(entry);
}


/*
 *	Sets the Scene's Weather.
 */
void SARWeatherSetScenePreset(
	sar_weather_data_struct *w, sar_scene_struct *scene,
	const char *name
)
{
	sar_weather_data_struct *wd = w;
	sar_weather_data_entry_struct *wdp_ptr = NULL;

	int i, cloud_layer_num, cloud_bb_num;
	sar_scene_horizon_struct *horizon_ptr;
	sar_cloud_layer_struct	*cloud_layer_ptr,
				*cloud_layer_src_ptr;
	sar_cloud_bb_struct	*cloud_bb_ptr,
				*cloud_bb_src_ptr;
	sar_color_struct	*c;


	if((wd == NULL) || (scene == NULL))
	    return;

	/* Set default sky shading colors */
	c = &scene->sky_nominal_color;
	c->a = 1.00f;
	c->r = 0.54f;
	c->g = 0.71f;
	c->b = 0.96f;
	c = &scene->sky_brighten_color;
	c->a = 1.00f;
	c->r = 1.00f;
	c->g = 0.95f;
	c->b = 0.60f;
	c = &scene->sky_darken_color;
	c->a = 0.96f;
	c->r = 0.96f;
	c->g = 0.60f;
	c->b = 0.60f;

	/* Set default celestial tint colors */
	c = &scene->star_low_color;
	c->a = 1.00f;
	c->r = 1.00f;
	c->g = 1.00f;
	c->b = 1.00f;
	c = &scene->star_high_color;
	c->a = 1.00f;
	c->r = 1.00f;
	c->g = 1.00f;
	c->b = 1.00f;

	c = &scene->sun_low_color;
	c->a = 1.00f;
	c->r = 1.00f;
	c->g = 1.00f;
	c->b = 1.00f;
	c = &scene->sun_high_color;
	c->a = 1.00f;
	c->r = 1.00f;
	c->g = 1.00f;
	c->b = 1.00f;

	c = &scene->moon_low_color;
	c->a = 1.00f;
	c->r = 1.00f;
	c->g = 1.00f;
	c->b = 1.00f;
	c = &scene->moon_high_color;
	c->a = 1.00f;
	c->r = 1.00f;
	c->g = 1.00f;
	c->b = 1.00f;


	/* Set default atmosphere parameters */
	scene->atmosphere_dist_coeff = 0.90f;
	scene->atmosphere_density_coeff = 0.30f;

	/* No rain */
	scene->rain_density_coeff = 0.0f;

	/* Reset horizon's last update time to -1 so that
	 * SARSimUpdateScene() will update the horizon
	 */
	horizon_ptr = &scene->horizon;
	horizon_ptr->last_tod = -1;

	/* Delete any existing cloud layers on scene structure */
	for(i = 0; i < scene->total_cloud_layers; i++)
	    SARCloudLayerDelete(scene, scene->cloud_layer[i]);
	free(scene->cloud_layer);
	scene->cloud_layer = NULL;
	scene->total_cloud_layers = 0;

	/* Delete any existing cloud billboards */
	for(i = 0; i < scene->total_cloud_bbs; i++)
	    SARCloudBBDelete(scene->cloud_bb[i]);
	free(scene->cloud_bb);
	scene->cloud_bb = NULL;
	scene->total_cloud_bbs = 0;


	/* ******************************************************** */

	/* Match preset weather data with given weather code */
	if(name == NULL)
	{
	    /* No name given, so first preset weather data structure
	     * as default if possable
	     */
	    wdp_ptr = (wd->total_presets > 0) ? wd->preset[0] : NULL;
	}
	else
	{
	    for(i = 0; i < wd->total_presets; i++)
	    {
	        wdp_ptr = wd->preset[i];
	        if(wdp_ptr == NULL)
		    continue;

		if(!strcasecmp(wdp_ptr->name, name))
		    break;
	    }
	    if(i >= wd->total_presets)
	    {
	        /* No match, assume first preset weather data structure
	         * as default if possable.
	         */
		wdp_ptr = (wd->total_presets > 0) ? wd->preset[0] : NULL;

		if(wdp_ptr != NULL)
		    fprintf(
			stderr,
 "SARWeatherSetScenePreset(): No such weather preset named `%s', using default.\n",
			name
		    );
		else
		    fprintf(   
			stderr,
 "SARWeatherSetScenePreset(): No such weather preset named `%s' and no default available.\n",
			name
		    );
	    }
	    else
	    {
		/* Got match */
		wdp_ptr = wd->preset[i];
	    }
	}

	/* No weather data preset available, then fail */
	if(wdp_ptr == NULL)
	    return;

/* Adds a cloud layer */
#define DO_ADD_CLOUD_LAYER	{			\
 cloud_layer_num = MAX(scene->total_cloud_layers, 0);	\
 scene->total_cloud_layers = cloud_layer_num + 1;	\
 scene->cloud_layer = (sar_cloud_layer_struct **)realloc( \
  scene->cloud_layer,					\
  scene->total_cloud_layers * sizeof(sar_cloud_layer_struct *) \
 );							\
 if(scene->cloud_layer == NULL)				\
 {							\
  scene->total_cloud_layers = 0;			\
 }							\
 else							\
 {							\
  scene->cloud_layer[cloud_layer_num] = cloud_layer_ptr; \
 }							\
}

/* Adds a cloud billboard */
#define DO_ADD_CLOUD_BB		{			\
 cloud_bb_num = MAX(scene->total_cloud_bbs, 0);		\
 scene->total_cloud_bbs = cloud_bb_num + 1;		\
 scene->cloud_bb = (sar_cloud_bb_struct **)realloc(	\
  scene->cloud_bb,					\
  scene->total_cloud_bbs * sizeof(sar_cloud_bb_struct *) \
 );							\
 if(scene->cloud_bb == NULL)				\
 {							\
  scene->total_cloud_bbs = 0;				\
 }							\
 else							\
 {							\
  scene->cloud_bb[cloud_bb_num] = cloud_bb_ptr;		\
 }							\
}


	/* Copy weather data sky colors to scene structure */
	memcpy(
	    &scene->sky_nominal_color,
	    &wdp_ptr->sky_nominal_color,
	    sizeof(sar_color_struct)
	);
	memcpy( 
	    &scene->sky_brighten_color,
	    &wdp_ptr->sky_brighten_color,
	    sizeof(sar_color_struct)
	);
	memcpy( 
	    &scene->sky_darken_color,
	    &wdp_ptr->sky_darken_color,
	    sizeof(sar_color_struct)
	);

	/* Copy weather data celestial tint colors to scene structure */
	memcpy(
	    &scene->star_low_color,
	    &wdp_ptr->star_low_color,
	    sizeof(sar_color_struct)
	);
	memcpy(
	    &scene->star_high_color,
	    &wdp_ptr->star_high_color,
	    sizeof(sar_color_struct)
	);

	memcpy(
	    &scene->sun_low_color,
	    &wdp_ptr->sun_low_color,
	    sizeof(sar_color_struct)
	);
	memcpy(
	    &scene->sun_high_color,
	    &wdp_ptr->sun_high_color,
	    sizeof(sar_color_struct)
	);

	memcpy(
	    &scene->moon_low_color,
	    &wdp_ptr->moon_low_color,
	    sizeof(sar_color_struct)
	);
	memcpy(
	    &scene->moon_high_color,
	    &wdp_ptr->moon_high_color,
	    sizeof(sar_color_struct)
	);


	/* Atmosphere */
	scene->atmosphere_dist_coeff = wdp_ptr->atmosphere_dist_coeff;
	scene->atmosphere_density_coeff = wdp_ptr->atmosphere_density_coeff;

	/* Rain */
	scene->rain_density_coeff = wdp_ptr->rain_density_coeff;

	/* Cloud layers */
	for(i = 0; i < wdp_ptr->total_cloud_layers; i++)
	{
	    cloud_layer_src_ptr = wdp_ptr->cloud_layer[i];
	    if(cloud_layer_src_ptr == NULL)
		continue;

	    cloud_layer_ptr = SARCloudLayerNew(
		scene,
		cloud_layer_src_ptr->tile_width,
		cloud_layer_src_ptr->tile_height,
		cloud_layer_src_ptr->range,
		cloud_layer_src_ptr->z,
		cloud_layer_src_ptr->tex_name
	    );
	    DO_ADD_CLOUD_LAYER
	}

	/* Cloud `billboard' objects */
	for(i = 0; i < wdp_ptr->total_cloud_bbs; i++)
	{ 
	    cloud_bb_src_ptr = wdp_ptr->cloud_bb[i];
	    if(cloud_bb_src_ptr == NULL)
		continue;

	    cloud_bb_ptr = SARCloudBBNew(
		scene,
		cloud_bb_src_ptr->tile_width,
		cloud_bb_src_ptr->tile_height,
		&cloud_bb_src_ptr->pos,
		cloud_bb_src_ptr->width,
		cloud_bb_src_ptr->height,
		cloud_bb_src_ptr->tex_name,
		cloud_bb_src_ptr->lightening_min_int,
		cloud_bb_src_ptr->lightening_max_int
	    );
	    DO_ADD_CLOUD_BB
	}

#undef DO_ADD_CLOUD_LAYER
#undef DO_ADD_CLOUD_BB
}
