/*
			Weather Preset Data Structures
 */

#ifndef WEATHER_H
#define WEATHER_H

#include <sys/types.h>
#include "obj.h"

/*
 *	Weather Data:
 */
typedef struct {

	/* Name of this weather setting. */
	char		*name;

	/* Sky and horizon gradient colors. */
	sar_color_struct	sky_nominal_color,
				sky_brighten_color,
				sky_darken_color;

	/* Celestial tint colors. */
	sar_color_struct	star_low_color,
				star_high_color,
				sun_low_color,
				sun_high_color,
				moon_low_color,
				moon_high_color;

	/* Atmoshere minimal distance from camera, a value of 0.5
	 * would be half of the distance and 0.9 would be really close
	 * up to the camera (very foggy)
	 */
	float		atmosphere_dist_coeff;

	/* Atmosphere density, gets more dense as value approaches
	 * 1.0
	 */
	float		atmosphere_density_coeff;

	/* Density of rain, 0.0 for no rain, 0.5 for moderate, 1.0
	 * for densest rain
	 */
	float		rain_density_coeff;

	/* Cloud layers that will be created into the scene, note that
	 * there are no visual models loaded on these cloud layer
	 * structures.
	 */
	sar_cloud_layer_struct **cloud_layer;
	int		total_cloud_layers;

	/* `Billboard' clouds, clouds that appear as `billboard objects'
	 * in a repeating tiled region across the scene.
	 */
	sar_cloud_bb_struct **cloud_bb;
	int		total_cloud_bbs;

} sar_weather_data_entry_struct;

#define SAR_WAATHER_DATA_ENTRY(p)	((sar_weather_data_entry_struct *)(p))


/*
 *	Contains a list of preset weather data structures.
 */
typedef struct {

	void *core_ptr;		/* Pointer back to core. */

	sar_weather_data_entry_struct **preset;
	int total_presets;

} sar_weather_data_struct;

#define SAR_WEATHER_DATA(p)	((sar_weather_data_struct *)(p))


/* In weather.c. */
extern sar_weather_data_struct *SARWeatherPresetsInit(void *core_ptr);
extern void SARWeatherPresetsShutdown(sar_weather_data_struct *w);

extern void SARWeatherEntryDelete(
	sar_weather_data_struct *w, sar_weather_data_entry_struct *entry
);

extern void SARWeatherSetScenePreset(
	sar_weather_data_struct *w, sar_scene_struct *scene,
	const char *name
);

/* In weatherio.c. */
extern int SARWeatherLoadFromFile(
	sar_weather_data_struct *w, const char *filename
);


#endif	/* WEATHER_H */
