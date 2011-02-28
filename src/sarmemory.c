#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __MSW__
# include <windows.h>
#endif

#include "gw.h"
#include "v3dtex.h"
#include "sfm.h"
#include "obj.h"
#include "sar.h"
#include "sarmemory.h"
#include "config.h"


static unsigned long SARMemoryStatTexture(
	const v3d_texture_ref_struct *t
);
static unsigned long SARMemoryStatSoundSource(
	const sar_sound_source_struct *sndsrc
);

void SARMemoryStat(
	sar_core_struct *core_ptr,
	sar_scene_struct *scene,
	sar_object_struct **object, int total_objects,
	sar_memory_stat_struct *stat_buf
);


#define ATOI(s)         (((s) != NULL) ? atoi(s) : 0)
#define ATOL(s)         (((s) != NULL) ? atol(s) : 0)
#define ATOF(s)         (((s) != NULL) ? (float)atof(s) : 0.0f)
#define STRDUP(s)       (((s) != NULL) ? strdup(s) : NULL)

#define MAX(a,b)        (((a) > (b)) ? (a) : (b))
#define MIN(a,b)        (((a) < (b)) ? (a) : (b))
#define CLIP(a,l,h)     (MIN(MAX((a),(l)),(h)))

#define RADTODEG(r)     ((r) * 180.0 / PI)
#define DEGTORAD(d)     ((d) * PI / 180.0)

#define ISSTREMPTY(s)   (((s) != NULL) ? (*(s) == '\0') : True)
#define STRLEN(s)       (((s) != NULL) ? ((int)strlen(s)) : 0)


/*
 *	Returns memory used by the given texture
 */
static unsigned long SARMemoryStatTexture(
	const v3d_texture_ref_struct *t
)
{
	unsigned long m = 0;

	if(t == NULL)
	    return(m);

	m += STRLEN(t->name) + STRLEN(t->filename);
	switch(t->dimensions)
	{
	  case 1:
	    m += t->width * t->total_frames * 4;
	    break;
	  case 2:
	    m += t->width * t->height + t->total_frames * 4;
	    break;
	}
	m += t->total_frames * sizeof(void *);
	m += sizeof(v3d_texture_ref_struct);

	return(m);
}

/*
 *	Returns memory used by the given sound source
 */
static unsigned long SARMemoryStatSoundSource(
	const sar_sound_source_struct *sndsrc
)
{
	unsigned long m = 0;

	if(sndsrc == NULL)
	    return(m);

	m += STRLEN(sndsrc->name);
	m += STRLEN(sndsrc->filename);
	m += STRLEN(sndsrc->filename_far);
	m += sizeof(sar_sound_source_struct);

	return(m);
}

/*
 *	Fetches memory stats and stores them on the given stat_buf
 */
void SARMemoryStat(
	sar_core_struct *core_ptr,
	sar_scene_struct *scene,
	sar_object_struct **object, int total_objects,
	sar_memory_stat_struct *stat_buf
)
{
	if((core_ptr == NULL) || (stat_buf == NULL))
	    return;

	memset(stat_buf, 0x00, sizeof(sar_memory_stat_struct));

	/* Add up memory used on scene */
	if(scene != NULL)
	{
	    int i;
	    unsigned long m = 0;
	    const sar_visual_model_struct *vmodel;
/*	    sar_scene_base_struct *base = &scene->base; */
	    sar_scene_horizon_struct *horizon = &scene->horizon;

	    m += sizeof(sar_scene_struct);
	    m += STRLEN(scene->title);

	    for(i = 0; i < scene->total_visual_models; i++)
	    {
		vmodel = scene->visual_model[i];
		if(vmodel == NULL)
		    continue;
		stat_buf->vmodel += STRLEN(vmodel->name);
		stat_buf->vmodel += STRLEN(vmodel->filename);
		stat_buf->vmodel += vmodel->mem_size;
		stat_buf->vmodel += sizeof(sar_visual_model_struct);
	    }
	    stat_buf->nvmodels += scene->total_visual_models;
	    m += scene->total_visual_models * sizeof(sar_visual_model_struct *);

	    m += scene->total_ground_objects * sizeof(sar_object_struct *);
	    m += scene->total_human_need_rescue_objects * sizeof(sar_object_struct *);

	    for(i = 0; i < horizon->total_textures; i++)
		stat_buf->texture += SARMemoryStatTexture(
		    horizon->texture[i]
		);
	    stat_buf->ntextures += horizon->total_textures;
	    m += horizon->total_textures * sizeof(v3d_texture_ref_struct *);

	    for(i = 0; i < scene->total_cloud_layers; i++)
	    {
		const sar_cloud_layer_struct *cloud = scene->cloud_layer[i];
		if(cloud == NULL)
		    continue;
		m += STRLEN(cloud->tex_name);
		m += sizeof(sar_cloud_layer_struct *);
	    }
	    m += scene->total_cloud_layers * sizeof(sar_cloud_layer_struct *);

	    for(i = 0; i < scene->total_cloud_bbs; i++)
	    {
		const sar_cloud_bb_struct *cloud = scene->cloud_bb[i];
		if(cloud == NULL)
		    continue;
		m += STRLEN(cloud->tex_name);
		m += sizeof(sar_cloud_bb_struct *);
	    }
	    m += scene->total_cloud_bbs * sizeof(sar_cloud_bb_struct *);

	    for(i = 0; i < scene->total_texture_refs; i++)
		stat_buf->texture += SARMemoryStatTexture(
		    scene->texture_ref[i]
		);
	    stat_buf->ntextures += scene->total_texture_refs;
	    m += scene->total_texture_refs * sizeof(v3d_texture_ref_struct *);

	    for(i = 0; i < scene->total_sndsrcs; i++)
		m += SARMemoryStatSoundSource(scene->sndsrc[i]);
	    m += scene->total_sndsrcs * sizeof(sar_sound_source_struct *);

/* TODO */
/*	    scene->player_control_panel */

	    for(i = 0; i < scene->total_messages; i++)
		m += STRLEN(scene->message[i]);
	    m += scene->total_messages * sizeof(char *);

	    for(i = 0; i < scene->total_sticky_banner_messages; i++)
		m += STRLEN(scene->sticky_banner_message[i]);
	    m += scene->total_sticky_banner_messages * sizeof(char *);

	    m += STRLEN(scene->camera_ref_title);

	    m += sizeof(SFMRealmStruct *);

	    stat_buf->scene += m;
	}

	/* Add up memory used by objects */
	if(object != NULL)
	{
	    int i, n;
	    unsigned long m = 0;

	    for(i = 0; i < total_objects; i++)
	    {
		const sar_object_struct *obj_ptr = object[i];
		const sar_object_aircraft_struct *obj_aircraft;

		if(obj_ptr == NULL)
		    continue;

		obj_aircraft = SAR_OBJ_GET_AIRCRAFT(obj_ptr);
		if(obj_aircraft != NULL)
		{
		    m += (obj_aircraft->fdm != NULL) ?
			sizeof(SFMModelStruct) : 0;

		    m += obj_aircraft->total_external_fueltanks *
			sizeof(sar_external_fueltank_struct);
		    m += obj_aircraft->total_external_fueltanks *
			sizeof(sar_external_fueltank_struct *);

		    m += obj_aircraft->total_parts *
			sizeof(sar_obj_part_struct);
		    m += obj_aircraft->total_parts *
			sizeof(sar_obj_part_struct *);

		    m += obj_aircraft->total_rotors *
			sizeof(sar_obj_rotor_struct);
		    m += obj_aircraft->total_rotors *
			sizeof(sar_obj_rotor_struct *);

		    if(obj_aircraft->hoist != NULL)
		    {
			const sar_obj_hoist_struct *hoist = obj_aircraft->hoist;
			m += hoist->total_occupants * sizeof(int);
			m += sizeof(sar_obj_hoist_struct);
		    }

		    m += obj_aircraft->total_intercepts *
			sizeof(sar_intercept_struct);
		    m += obj_aircraft->total_intercepts *
			sizeof(sar_intercept_struct *);

		    m += sizeof(sar_object_aircraft_struct);
		}

		m += STRLEN(obj_ptr->name);
		m += (obj_ptr->contact_bounds != NULL) ?
		    sizeof(sar_contact_bounds_struct) : 0;
		m += obj_ptr->total_lights * sizeof(sar_light_struct);
		m += obj_ptr->total_lights * sizeof(sar_light_struct *);
		for(n = 0; n < obj_ptr->total_sndsrcs; n++)
		    m += SARMemoryStatSoundSource(obj_ptr->sndsrc[n]);
		m += obj_ptr->total_sndsrcs * sizeof(sar_sound_source_struct *);

		m += sizeof(sar_object_struct);
	    }
	    m += total_objects * sizeof(sar_object_struct *);
	    stat_buf->object += m;

	    stat_buf->nobjects += total_objects;
	}


	/* Add up total */
	stat_buf->total += stat_buf->texture + stat_buf->vmodel +
	    stat_buf->scene + stat_buf->object;
}
