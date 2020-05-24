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
#include <errno.h>
#include <ctype.h>
#include <sys/stat.h>
#include <math.h>

#ifdef __MSW__
# include <windows.h>
#endif

#include <GL/gl.h>

#include "../include/fio.h"
#include "../include/string.h"
#include "../include/strexp.h"
#include "../include/disk.h"

#include "gw.h"
#include "cp.h"
#include "sfm.h"
#include "sarreality.h"
#include "obj.h"
#include "objsound.h"
#include "objutils.h"
#include "objio.h"
#include "messages.h"
#include "simmanage.h"
#include "simcb.h"
#include "simutils.h"
#include "weather.h"
#include "sar.h"
#include "sarfio.h"
#include "sceneio.h"
#include "config.h"


void SARSceneDestroy(
	sar_core_struct *core_ptr,
	sar_scene_struct *scene,
	sar_object_struct ***ptr, int *total
);
void SARSceneLoadLocationsToList(
	sar_core_struct *core_ptr, sar_menu_struct *m,
	sar_menu_list_struct *list, int list_num,
	const char *filename
);
int SARSceneAddPlayerObject(
	sar_core_struct *core_ptr, sar_scene_struct *scene,
	const char *model_file,
	sar_position_struct *pos, sar_direction_struct *dir
);
int SARSceneLoadFromFile(
	sar_core_struct *core_ptr,
	sar_scene_struct *scene,
	const char *filename,
	const char *weather_preset_name,
	void *client_data,
	int (*progress_func)(void *, long, long)
);


#define MAX(a,b)        (((a) > (b)) ? (a) : (b))
#define MIN(a,b)        (((a) < (b)) ? (a) : (b))
#define CLIP(a,l,h)     (MIN(MAX((a),(l)),(h)))

#define RADTODEG(r)     ((r) * 180 / PI)
#define DEGTORAD(d)     ((d) * PI / 180)

#define ISCOMMENT(c)	((c) == SAR_COMMENT_CHAR)
#define ISCR(c)		(((c) == '\n') || ((c) == '\r'))

#define STRDUP(s)	(((s) != NULL) ? strdup(s) : NULL)


/*
 *	Deletes all resources on the scene structure and all
 *	objects on the given objects pointer array.
 *
 *	Does not delete the scene structure itself, but the
 *	scene structure will be reset.
 */
void SARSceneDestroy(
	sar_core_struct *core_ptr, sar_scene_struct *scene,
	sar_object_struct ***ptr, int *total
)
{
	int i;
	const sar_option_struct *opt = &core_ptr->option;

	if(opt->runtime_debug)
	    printf("SARSceneDestroy(): Destroying scene...\n");


	/* Check if total objects is not NULL (which implies ptr is not
	 * NULL)
	 */
	if(total != NULL)
	{
	    /* Delete objects pointer array from last to first */
	    for(i = (*total) - 1; i >= 0; i--)
	        SARObjDelete(core_ptr, ptr, total, i);
	    *total = 0;
	}
	if(ptr != NULL)
	{
	    free(*ptr);
	    *ptr = NULL;
	}


	/* Delete scene */
	if(scene != NULL)
	{
	    sar_scene_base_struct *base = &scene->base;
	    sar_scene_horizon_struct *horizon_ptr = &scene->horizon;

	    /* Title */
	    free(scene->title);
	    scene->title = NULL;

	    /* Ground base */
	    /* Visual model simple (far away) */
	    SARVisualModelUnref(scene, base->visual_model_simple);
	    base->visual_model_simple = NULL;
	    /* Visual model complex (close up) */
	    SARVisualModelUnref(scene, base->visual_model_close);
	    base->visual_model_close = NULL;

	    /* Cloud Layers */
	    for(i = 0; i < scene->total_cloud_layers; i++)
		SARCloudLayerDelete(scene, scene->cloud_layer[i]);
	    free(scene->cloud_layer);
	    scene->cloud_layer = NULL;
	    scene->total_cloud_layers = 0;

	    /* Cloud BillBoards */
	    for(i = 0; i < scene->total_cloud_bbs; i++)
		SARCloudBBDelete(scene->cloud_bb[i]);
	    free(scene->cloud_bb);
	    scene->cloud_bb = NULL;
	    scene->total_cloud_bbs = 0;

	    /* Horizon */
	    horizon_ptr = &scene->horizon;
	    for(i = 0; i < horizon_ptr->total_textures; i++)
		V3DTextureDestroy(horizon_ptr->texture[i]);
	    free(horizon_ptr->texture);
	    horizon_ptr->texture = NULL;
	    horizon_ptr->total_textures = 0;


	    /* Ground objects list, delete only the pointer array and
	     * not each object
	     */
	    free(scene->ground_object);
	    scene->ground_object = NULL;
	    scene->total_ground_objects = 0;

	    /* Humans that need rescue list */
	    free(scene->human_need_rescue_object);
	    scene->human_need_rescue_object = NULL;
	    scene->total_human_need_rescue_objects = 0;

	    /* Visual models, all visual models should have been
	     * unref'ed by now. So here we actually delete
	     * and destroy the GL lists
	     */
	    SARVisualModelDeleteAll(scene);

	    /* Delete all textures */
	    for(i = 0; i < scene->total_texture_refs; i++)
		V3DTextureDestroy(scene->texture_ref[i]);
	    free(scene->texture_ref);	/* Free pointer array */
	    scene->texture_ref = NULL;
	    scene->total_texture_refs = 0;

	    /* Reset texture reference indices for special textures */
	    scene->texnum_sun = -1;
	    scene->texnum_moon = -1;
	    scene->texnum_spotlightcast = -1;

	    /* Delete all sound sources */
	    for(i = 0; i < scene->total_sndsrcs; i++)
		SARSoundSourceDelete(scene->sndsrc[i]);
	    free(scene->sndsrc);
	    scene->sndsrc = NULL;
	    scene->total_sndsrcs = 0;

	    /* Control panel */
	    CPDelete(
		(ControlPanel *)scene->player_control_panel
	    );
	    scene->player_control_panel = NULL;


	    /* Messages */
	    strlistfree(scene->message, scene->total_messages);
	    scene->message = NULL;
	    scene->total_messages = 0;

	    /* Sticky banner message */
	    strlistfree(
		scene->sticky_banner_message,
		scene->total_sticky_banner_messages
	    );
	    scene->sticky_banner_message = NULL;
	    scene->total_sticky_banner_messages = 0;

	    /* Camera reference name/description title string */
	    free(scene->camera_ref_title);
	    scene->camera_ref_title = NULL;

	    /* FDM Realm */
	    if(scene->realm != NULL)
		SFMShutdown(scene->realm);

	    /* Reset the rest of the scene structure */
	    memset(scene, 0x00, sizeof(sar_scene_struct));
	}

	if(opt->runtime_debug)
	    printf("SARSceneDestroy(): Scene destroyed.\n");
}

/*
 *	Loads the list of Registered Locations from the specified
 *	scene file to the specified List.
 */
void SARSceneLoadLocationsToList(
	sar_core_struct *core_ptr, sar_menu_struct *m,
	sar_menu_list_struct *list, int list_num,
	const char *filename
)
{
	int i, ptype, total_parms;
	void *p, **parm;

	if(list == NULL)
	    return;

	/* Delete existing items in the List */
	for(i = 0; i < list->total_items; i++)
	    SARDeleteListItemData(list->item[i]);
	SARMenuListDeleteAllItems(m, list_num);

	/* Load all SAR_PARM_REGISTER_LOCATION parameters from the
	 * scene file
	 */
	if(SARParmLoadFromFile(
	    filename, SAR_FILE_FORMAT_SCENE,
	    &parm, &total_parms,
	    SAR_PARM_REGISTER_LOCATION,		/* Filter */
	    NULL, NULL
	))
	    return;

	/* Iterate through loaded parms */
	for(i = 0; i < total_parms; i++)
	{
	    p = parm[i];
	    if(p == NULL)
		continue;

	    ptype = *(int *)p;
	    if(ptype == SAR_PARM_REGISTER_LOCATION)
	    {
		int n;
		sar_menu_list_item_data_struct *d;
		sar_parm_register_location_struct *pv =
		    (sar_parm_register_location_struct *)p;

		/* Allocate a new list item data */
		d = SAR_MENU_LIST_ITEM_DATA(calloc(
		    1, sizeof(sar_menu_list_item_data_struct)
		));
		if(d != NULL)
		{
		    /* Position */
		    memcpy(
			&d->pos,
			&pv->pos,
			sizeof(sar_position_struct)
		    );

		    /* Direction */
		    memcpy(
			&d->dir,
			&pv->dir,
			sizeof(sar_direction_struct)
		    );

		    /* Name */
		    d->name = STRDUP(pv->name);

		    /* Add this location to the menu's list object */
		    n = SARMenuListAppendItem(
			m, list_num,
			d->name, d,
			0
		    );
		    if(n < 0)
		    {
			fprintf(
			    stderr,
 "Error appending list item for starting location `%s'\n",
			    d->name
			);

			free(d->filename);
			free(d->name);
			free(d);
		    }
		    d = NULL;
		}
	    }
	}

	/* Delete loaded parms */
	SARParmDeleteAll(&parm, &total_parms);
}


/*
 *	Adds a player object to the scene using the specified model
 *	file and initial position & direction.
 *
 *	Updates the player object references on the scene.
 *
 *	All inputs must be valid (except for pos and dir).
 *
 *	Returns non-zero on error.
 */
int SARSceneAddPlayerObject(
	sar_core_struct *core_ptr, sar_scene_struct *scene,
	const char *model_file,
	sar_position_struct *pos, sar_direction_struct *dir
)
{
	int obj_num;
	sar_object_struct *obj_ptr;
	sar_object_aircraft_struct *obj_aircraft_ptr;
	sar_position_struct lpos;
	sar_direction_struct ldir;


	if((core_ptr == NULL) || (scene == NULL) || (model_file == NULL))
	    return(-1);

	/* Create player object on to scene */
	obj_num = SARObjNew(
	    scene, &core_ptr->object, &core_ptr->total_objects,
	    SAR_OBJ_TYPE_AIRCRAFT
	);
	obj_ptr = (obj_num > -1) ? core_ptr->object[obj_num] : NULL;
	if(obj_ptr == NULL)
	    return(-1);

	/* Update player object references on scene structure (this
	 * must be done immediately after creating of the player object
	 * so that SARObjLoadFromFile() can tell this is the player
	 * object
	 */
	scene->player_obj_num = obj_num;
	scene->player_obj_ptr = obj_ptr;

	/* Load player object model file */
	SARObjLoadFromFile(core_ptr, obj_num, model_file);

	/* Set object name to "player", this will override the name
	 * of this object specified in the model file.  This needs to
	 * be set so that subsequent configurations can reffer to
	 * this object by the name of "player" in order to match it
	 * (ie in mission files)
	 */
	free(obj_ptr->name);
	obj_ptr->name = STRDUP("player");


	/* Set local position structure */
	if(pos != NULL)
	    memcpy(&lpos, pos, sizeof(sar_position_struct));
	else
	    memset(&lpos, 0x00, sizeof(sar_position_struct));

	/* Adjust ground_elevation_msl so that it's at the position of
	 * the player object, this way the player won't suddenly drop
	 * and be destroyed
	 */
	obj_ptr->ground_elevation_msl = lpos.z;

	/* Move player up from ground level so that it does not added into
	 * the scene while embedded in the ground (which may cause a
	 * crash if the given pos is ontop of a building)
	 */
	obj_aircraft_ptr = SAR_OBJ_GET_AIRCRAFT(obj_ptr);
	if(obj_aircraft_ptr != NULL)
	{
	    lpos.z += (float)MAX(obj_aircraft_ptr->belly_height, 0.0);
	    lpos.z += (float)MAX(obj_aircraft_ptr->gear_height, 0.0);
	}

/* Add other object types that need their position modified here */

	/* Set local direction structure */
	if(dir != NULL)
	    memcpy(&ldir, dir, sizeof(sar_direction_struct));
	else
	    memset(&ldir, 0x00, sizeof(sar_direction_struct));

	/* Move player object to starting location and attitude */
	SARSimWarpObject(scene, obj_ptr, &lpos, &ldir);

	return(0);
}

/*
 *	Opens the Scene from the specified file.
 *
 *	All default values for the Scene will be allocated/created/set
 *	and any existing values will be deleted/reset.
 *
 */
int SARSceneLoadFromFile(
	sar_core_struct *core_ptr,
	sar_scene_struct *scene,
	const char *filename,
	const char *weather_preset_name,
	void *client_data,
	int (*progress_func)(void *, long, long)
)
{
	void *p, **parm;
	int i, status, ptype, total_parms;
	int obj_num = -1, *total;
	gw_display_struct *display;
	sar_direction_struct *dir;
	sar_color_struct *c;
	sar_object_struct *obj_ptr = NULL, ***ptr;
	sar_object_aircraft_struct *obj_aircraft_ptr = NULL;
	sar_object_ground_struct *obj_ground_ptr = NULL;
	sar_object_helipad_struct *obj_helipad_ptr = NULL;
	sar_object_runway_struct *obj_runway_ptr = NULL;
	sar_object_human_struct *obj_human_ptr = NULL;
	sar_object_smoke_struct *obj_smoke_ptr = NULL;
	sar_object_fire_struct *obj_fire_ptr = NULL;
	sar_scene_horizon_struct *horizon_ptr;
	struct stat stat_buf;

	sar_parm_version_struct *p_version;
	sar_parm_name_struct *p_name;
	sar_parm_description_struct *p_description;
	sar_parm_player_model_file_struct *p_player_model_file;
	sar_parm_weather_struct *p_weather;
	sar_parm_wind_struct *p_wind;
	sar_parm_register_location_struct *p_register_location;
	sar_parm_scene_gps_struct *p_scene_gps;
	sar_parm_scene_map_struct *p_scene_map;
	sar_parm_scene_elevation_struct *p_scene_elevation;
	sar_parm_scene_cant_struct *p_scene_cant;
	sar_parm_scene_ground_flags_struct *p_scene_ground_flags;  
	sar_parm_scene_ground_tile_struct *p_scene_ground_tile;
	sar_parm_texture_base_directory_struct *p_texture_base_directory;
	sar_parm_texture_load_struct *p_texture_load;
	sar_parm_new_object_struct *p_new_object;
	sar_parm_new_helipad_struct *p_new_helipad;  
	sar_parm_new_runway_struct *p_new_runway;
	sar_parm_new_human_struct *p_new_human;
	sar_parm_new_smoke_struct *p_new_smoke;
	sar_parm_new_fire_struct *p_new_fire;
	sar_parm_new_premodeled_struct *p_new_premodeled;
	sar_parm_model_file_struct *p_model_file;
	sar_parm_range_struct *p_range;
	sar_parm_range_far_struct *p_range_far;
	sar_parm_translate_struct *p_translate;
	sar_parm_translate_random_struct *p_translate_random;
	sar_parm_rotate_struct *p_rotate;
	sar_parm_no_depth_test_struct *p_no_depth_test;
	sar_parm_polygon_offset_struct *p_polygon_offset;
	sar_parm_contact_bounds_spherical_struct *p_contact_bounds_spherical;
	sar_parm_contact_bounds_cylendrical_struct *p_contact_bounds_cylendrical;
	sar_parm_contact_bounds_rectangular_struct *p_contact_bounds_rectangular;
	sar_parm_ground_elevation_struct *p_ground_elevation;
	sar_parm_object_name_struct *p_object_name;
	sar_parm_object_map_description_struct *p_object_map_description;
	sar_parm_fuel_struct *p_fuel;
	sar_parm_hitpoints_struct *p_hitpoints;
	sar_parm_engine_state_struct *p_engine_state;
	sar_parm_passengers_struct *p_passengers;
	sar_parm_runway_approach_lighting_north_struct *p_runway_applight_n;
	sar_parm_runway_approach_lighting_south_struct *p_runway_applight_s;
	sar_parm_human_message_enter_struct *p_human_message_enter;
	sar_parm_human_reference_struct *p_human_reference;
        sar_parm_welcome_message_struct *p_welcome_message;


/* Resets object substructure pointers to NULL */
#define DO_RESET_SUBSTRUCTURE_PTRS	{	\
 obj_aircraft_ptr = NULL;			\
 obj_ground_ptr = NULL;				\
 obj_helipad_ptr = NULL;			\
 obj_runway_ptr = NULL;				\
 obj_human_ptr = NULL;				\
 obj_smoke_ptr = NULL;				\
 obj_fire_ptr = NULL;				\
}


	if((core_ptr == NULL) || (scene == NULL) || (filename == NULL))
	    return(-1);

	display = core_ptr->display;

	ptr = &core_ptr->object;
	total = &core_ptr->total_objects;

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

	/* Load all parameters from file */
	status = SARParmLoadFromFile(
	    filename, SAR_FILE_FORMAT_SCENE,
	    &parm, &total_parms,
	    -1,			/* No filter */
	    NULL, NULL
	);
	if(status)
	{
	    fprintf(
		stderr,
		"%s: Error loading scene.\n",
		filename 
	    );
	    return(-1);
	}


	/* Delete the specified Scene and all its objects */
	SARSceneDestroy(core_ptr, scene, ptr, total);


	/* Reset Scene values */
	scene->tod = (12 * 3600);		/* Noon */
	scene->tod_code = SAR_TOD_CODE_DAY;
	scene->cant_angle = (float)(0.0 * PI);
	scene->base_flags = 0;
	scene->msl_elevation = 0.0f;

	scene->dms_x_offset = 0;
	scene->dms_y_offset = 0;
	scene->planet_radius = SAR_DEF_PLANET_RADIUS;

	scene->visual_model = NULL;
	scene->total_visual_models = 0;

	c = &scene->sky_nominal_color;
	c->a = 1.0f;
	c->r = 1.0f;
	c->g = 1.0f;
	c->b = 1.0f;
	c = &scene->sky_brighten_color;
	c->a = 1.0f;
	c->r = 1.0f;
	c->g = 1.0f;
	c->b = 1.0f;
	c = &scene->sky_darken_color;
	c->a = 1.0f;
	c->r = 1.0f;
	c->g = 1.0f;
	c->b = 1.0f;

	c = &scene->star_low_color;
	c->a = 1.0f;
	c->r = 1.0f;
	c->g = 1.0f;
	c->b = 1.0f;
	c = &scene->star_high_color;
	c->a = 1.0f;
	c->r = 1.0f;
	c->g = 1.0f;
	c->b = 1.0f;

	c = &scene->sun_low_color;
	c->a = 1.0f;
	c->r = 1.0f;
	c->g = 1.0f;
	c->b = 1.0f;
	c = &scene->sun_high_color;
	c->a = 1.0f;
	c->r = 1.0f;
	c->g = 1.0f;
	c->b = 1.0f;

	c = &scene->moon_low_color;
	c->a = 1.0f;
	c->r = 1.0f;
	c->g = 1.0f;
	c->b = 1.0f;
	c = &scene->moon_high_color;
	c->a = 1.0f;
	c->r = 1.0f;
	c->g = 1.0f;
	c->b = 1.0f;

	scene->moon_visibility_hint = 0;
	scene->rain_density_coeff = 0.0f;

	scene->player_obj_num = -1;
	scene->player_obj_ptr = NULL;
	scene->player_has_crashed = False;

	scene->ground_object = NULL;
	scene->total_ground_objects = 0;

	scene->human_need_rescue_object = NULL;
	scene->total_human_need_rescue_objects = 0;

	scene->camera_ref = SAR_CAMERA_REF_COCKPIT;
	scene->camera_fovz = (float)SFMDegreesToRadians(40.0);
	dir = &scene->camera_cockpit_dir;
	dir->heading = (float)(0.0f * PI);
	dir->pitch = (float)(0.0f * PI);
	dir->bank = (float)(0.0f * PI);
	dir = &scene->camera_spot_dir;
	dir->heading = (float)(1.25f * PI);
	dir->pitch = (float)(1.92f * PI);
	dir->bank = (float)(0.0f * PI);
	scene->camera_spot_dist = 20.0f;
	dir = &scene->camera_hoist_dir;
	dir->heading = (float)(0.75f * PI);
	dir->pitch = (float)(1.92f * PI);
	dir->bank = (float)(0.0f * PI);
	scene->camera_hoist_dist = 20.0f;
	scene->camera_target = -1;

	memset(
	    scene->camera_rotmatrix,
	    0x00,
	    SAR_CAMERA_ROTMATRIX_MAX * (3 * 3) * sizeof(double)
	);
	scene->camera_rotmatrix_count = 0;

	scene->light_visibility_hint = 0;
	scene->light_xc = -2.0f;		/* Not in camera view */
	scene->light_yc = -2.0f;

	scene->cloud_layer = NULL;
	scene->total_cloud_layers = 0;

	scene->cloud_bb = NULL;
	scene->total_cloud_bbs = 0;
	scene->pri_lightening_coeff = 0.0f;

	scene->texture_ref = NULL;
	scene->total_texture_refs = 0;

	scene->texnum_sun = -1;
	scene->texnum_moon = -1;
	scene->texnum_spotlightcast = -1;

	scene->sndsrc = NULL;
	scene->total_sndsrcs = 0;

	scene->player_control_panel = NULL;

	/* Reset initial GL states */
	if(display != NULL)
	{
	    StateGLResetAll(&display->state_gl);
	    StateGLEnable(&display->state_gl, GL_CULL_FACE);
	    StateGLFrontFace(&display->state_gl, GL_CCW);
	    StateGLShadeModel(&display->state_gl, GL_FLAT);
	}
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glPixelStorei(GL_PACK_ALIGNMENT, 1);


	/* Allocate the Messages List */
	scene->total_messages = 6;	/* Maximum of 6 at one time */
	free(scene->message);
	scene->message = (char **)calloc(
	    scene->total_messages, sizeof(char *)
	);
	if(scene->message == NULL)
	    scene->total_messages = 0;

	scene->message_display_until = 0;
	scene->camera_ref_title_display_until = 0;

	/* Allocate the FDM Realm */
	scene->realm = SFMInit(0, NULL);
	if(scene->realm == NULL)
	{
	    fprintf(
		stderr,
"SARSceneLoadFromFile(): Error: Unable to create FDM Realm.\n"
	    );
	}
	else
	{
	    SFMRealmStruct *realm = scene->realm;

	    /* Set up FDM realm values */
	    realm->lapsed_time = 0;
	    realm->time_compensation = 1.0f;
	    realm->time_compression = 1.0f;

	    realm->gravity = SFMDefaultGravity;

	    /* Set up realm callback data and callback function ptrs */
	    realm->init_model_cb_client_data = core_ptr;
	    realm->init_model_cb = SARSimInitModelCB;
	    realm->destroy_model_cb_client_data = core_ptr;
	    realm->destroy_model_cb = SARSimDestroyModelCB;
	    realm->airborne_cb_client_data = core_ptr;
	    realm->airborne_cb = SARSimAirborneCB;
	    realm->touch_down_cb_client_data = core_ptr;
	    realm->touch_down_cb = SARSimTouchDownCB;
	    realm->parked_cb_client_data = core_ptr;
	    realm->parked_cb = SARSimParkedCB;
	    realm->overspeed_cb_client_data = core_ptr;
	    realm->overspeed_cb = SARSimOverspeedCB;
	    realm->collision_cb_client_data = core_ptr;
	    realm->collision_cb = SARSimCollisionCB;
	}

#define APPEND_TEXTURE(_t_)	{				\
 const int n = MAX(scene->total_texture_refs, 0);		\
 scene->total_texture_refs = n + 1;				\
 scene->texture_ref = (v3d_texture_ref_struct **)realloc(	\
  scene->texture_ref,						\
  scene->total_texture_refs * sizeof(v3d_texture_ref_struct *)	\
 );								\
 if(scene->texture_ref == NULL) {				\
  scene->total_texture_refs = 0;				\
  return(-3);							\
 }								\
 scene->texture_ref[n] = (_t_);					\
}

	/* Render built in textures */
	if(True)
	{
#if 0
	    v3d_texture_ref_struct *t = SARCreateBladeBlurTexture(
		SAR_STD_TEXNAME_ROTOR_BLADE_BLUR, 0.98f
	    );
	    APPEND_TEXTURE(t);
#endif
	}


	/* Load all textures specified in the global Textures List */
	if(True)
	{
	    const char *name, *path;
	    char *full_path;
	    const sar_texture_name_struct *tn;
	    const int tex_fmt = V3D_TEX_FORMAT_RGBA;

	    /* Iterate through each Texture Name in the Textures List */
	    for(i = 0; i < core_ptr->total_texture_list; i++)
	    {
		tn = core_ptr->texture_list[i];
		if(tn == NULL)
		    continue;

		name = tn->name;
		path = tn->filename;
		if((name == NULL) || (path == NULL))
		    continue;

		if(ISPATHABSOLUTE(path))
		{
		    full_path = STRDUP(path);
		}
		else
		{
		    full_path = STRDUP(PrefixPaths(
			dname.local_data, path
		    ));
 		    if((full_path != NULL) ? stat(full_path, &stat_buf) : True)
		    {
			free(full_path);
			full_path = STRDUP(PrefixPaths(
			    dname.global_data, path
			));
		    }
		}
		if(full_path != NULL)
		{
		    v3d_texture_ref_struct *t = V3DTextureLoadFromFile2DPreempt(
			full_path, name, tex_fmt
		    );
		    V3DTexturePriority(t, tn->priority);
		    APPEND_TEXTURE(t);
		    free(full_path);
		}
		else
		{
		    fprintf(
			stderr,
"%s: Warning: Unable to complete texture \"%s\" path \"%s\".\n",
			filename, name, path
		    );
		}
	    }
	}

	/* Set references to frequently used global textures */
	if(True)
	{
	    scene->texnum_sun = SARGetTextureRefNumberByName(
		scene, SAR_STD_TEXNAME_SUN
	    );
	    scene->texnum_moon = SARGetTextureRefNumberByName(
		scene, SAR_STD_TEXNAME_MOON
	    );
	    scene->texnum_spotlightcast = SARGetTextureRefNumberByName(
		scene, SAR_STD_TEXNAME_SPOTLIGHTCAST
	    );

	}

#undef APPEND_TEXTURE

	/* Create default global sound sources list */
	if(True)
	{
#define LOAD_SNDSRC(name,filename,filename_far,range,range_far)	\
{								\
 i = MAX(scene->total_sndsrcs, 0);				\
 scene->total_sndsrcs = i + 1;					\
 scene->sndsrc = (sar_sound_source_struct **)realloc(		\
  scene->sndsrc,						\
  scene->total_sndsrcs * sizeof(sar_sound_source_struct *)	\
 );								\
 if(scene->sndsrc == NULL)					\
 {								\
  scene->total_sndsrcs = 0;					\
 }								\
 else								\
 {								\
  char *s, *full_path, *full_path_far;				\
								\
  /* Complete Filenames */					\
  s = STRDUP(PrefixPaths(dname.local_data, filename));		\
  if((s != NULL) ? stat(s, &stat_buf) : True) {			\
   free(s);							\
   s = STRDUP(PrefixPaths(dname.global_data, filename));	\
  }								\
  full_path = s;						\
								\
  s = STRDUP(PrefixPaths(dname.local_data, filename_far));	\
  if((s != NULL) ? stat(s, &stat_buf) : True) {			\
   free(s);							\
   s = STRDUP(PrefixPaths(dname.global_data, filename_far));	\
  }								\
  full_path_far = s;						\
								\
  /* Check if file exists */					\
  if((full_path != NULL) ? stat(full_path, &stat_buf) : True)	\
   fprintf(stderr,						\
"SARSceneLoadFromFile(): Warning: Unable to find sound file \"%s\"\n",\
    filename							\
   );								\
  if((full_path_far != NULL) ? stat(full_path_far, &stat_buf) : True) \
   fprintf(stderr,						\
"SARSceneLoadFromFile(): Warning: Unable to find sound file \"%s\"\n",\
    filename_far						\
   );								\
								\
  /* Create sound source */					\
  scene->sndsrc[i] = SARSoundSourceNew(				\
   name, full_path, full_path_far,				\
   range, range_far,						\
   NULL, 0.0f, NULL,						\
   1.0f			/* Sample rate limit in Hz */		\
  );								\
								\
  free(full_path);						\
  free(full_path_far);						\
 }								\
}

	    /* Gound Contact Sounds */
	    LOAD_SNDSRC(
		"land_wheel_skid",
		SAR_DEF_SOUND_LAND_WHEEL_SKID,
		SAR_DEF_SOUND_LAND_WHEEL_SKID,
		600.0f,
		400.0f
	    );
	    LOAD_SNDSRC(
		"land_ski_skid",
		SAR_DEF_SOUND_LAND_SKI_SKID,
		SAR_DEF_SOUND_LAND_SKI_SKID,
		600.0f,
		400.0f
	    );
	    LOAD_SNDSRC(
		"land_ski",
		SAR_DEF_SOUND_LAND_SKI,
		SAR_DEF_SOUND_LAND_SKI,
		600.0f,
		400.0f
	    );
	    LOAD_SNDSRC(
		"land_belly",
		SAR_DEF_SOUND_LAND_BELLY,
		SAR_DEF_SOUND_LAND_BELLY,
		1200.0f,
		900.0f
	    );

	    /* Thud Sounds */
	    LOAD_SNDSRC(
		"thud_light",
		SAR_DEF_SOUND_THUD_LIGHT,
		SAR_DEF_SOUND_THUD_LIGHT,
		600.0f,
		400.0f
	    );
	    LOAD_SNDSRC(
		"thud_medium",
		SAR_DEF_SOUND_THUD_MEDIUM,
		SAR_DEF_SOUND_THUD_MEDIUM,
		600.0f,
		400.0f
	    );
	    LOAD_SNDSRC(
		"thud_heavy",
		SAR_DEF_SOUND_THUD_HEAVY,
		SAR_DEF_SOUND_THUD_HEAVY,
		600.0f,
		400.0f
	    );

	    /* Crash & Collision Sounds */
	    LOAD_SNDSRC(
		"crash_obstruction",
		SAR_DEF_SOUND_CRASH_OBSTRUCTION,
		SAR_DEF_SOUND_CRASH_OBSTRUCTION,
		3300.0f,	/* About 2 miles */
		2500.0f
	    );
	    LOAD_SNDSRC(
		"crash_ground",
		SAR_DEF_SOUND_CRASH_GROUND,
		SAR_DEF_SOUND_CRASH_GROUND,
		3300.0f,	/* About 2 miles */
		2500.0f
	    );
	    LOAD_SNDSRC(
		"splash_aircraft",
		SAR_DEF_SOUND_SPLASH_AIRCRAFT,
		SAR_DEF_SOUND_SPLASH_AIRCRAFT,
		2500.0f,	/* About 1.5 miles */
		2000.0f
	    );
	    LOAD_SNDSRC(
		"splash_human",
		SAR_DEF_SOUND_SPLASH_HUMAN,
		SAR_DEF_SOUND_SPLASH_HUMAN,
		600.0f,
		400.0f
	    );

#undef LOAD_SNDSRC
	}

	/* Create the Horizon */
	horizon_ptr = &scene->horizon;
	horizon_ptr->last_tod = -1;	/* Reset to -1 so horizon gets updated */
	horizon_ptr->texture = NULL;
	horizon_ptr->total_textures = 0;


	/* Generate weather settings from the specified Weather Data
	 * Entry
	 */
	SARWeatherSetScenePreset(
	    core_ptr->weather_data,
	    scene,
	    weather_preset_name
	);

	/* Iterate through loaded parms */
	for(i = 0; i < total_parms; i++)
	{
	    p = parm[i];
	    if(p == NULL)
		continue;

	    ptype = *(int *)p;

	    if(progress_func != NULL)
	    {
		if(progress_func(client_data, i + 1, total_parms))
		    break;
	    }

	    /* Handle by parm type */
	    switch(ptype)
	    {
	      case SAR_PARM_VERSION:
		p_version = (sar_parm_version_struct *)p;
		if((p_version->major > PROG_VERSION_MAJOR) ||
		   (p_version->minor > PROG_VERSION_MINOR) ||
		   (p_version->release > PROG_VERSION_RELEASE)
		)
		{
		    int need_warn = 0;
		    if(p_version->major > PROG_VERSION_MAJOR)
			need_warn = 1;
		    else if((p_version->major == PROG_VERSION_MAJOR) &&
			    (p_version->minor > PROG_VERSION_MINOR)
		    )
			need_warn = 1;
		    else if((p_version->major == PROG_VERSION_MAJOR) &&
			    (p_version->minor == PROG_VERSION_MINOR) &&
			    (p_version->release == PROG_VERSION_RELEASE)
		    )
			need_warn = 1;
		    if(need_warn)
			fprintf(
			    stderr,
"%s: Warning: File format version %i.%i.%i is newer than program\
 version %i.%i.%i.",
			    filename,
			    p_version->major, p_version->minor,
			    p_version->release, PROG_VERSION_MAJOR,
			    PROG_VERSION_MINOR, PROG_VERSION_RELEASE
			);
		}
		break;

	      case SAR_PARM_NAME:
		p_name = (sar_parm_name_struct *)p;

		break;

	      case SAR_PARM_DESCRIPTION:
		p_description = (sar_parm_description_struct *)p;
		break;

	      case SAR_PARM_PLAYER_MODEL_FILE:
		p_player_model_file = (sar_parm_player_model_file_struct *)p;
		break;

	      case SAR_PARM_WEATHER:
		p_weather = (sar_parm_weather_struct *)p;
		break;

	      case SAR_PARM_REGISTER_LOCATION:
		p_register_location = (sar_parm_register_location_struct *)p;
		break;

	      case SAR_PARM_SCENE_GPS:
		p_scene_gps = (sar_parm_scene_gps_struct *)p;
		scene->dms_x_offset = p_scene_gps->dms_x_offset;
		scene->dms_y_offset = p_scene_gps->dms_y_offset;
		scene->planet_radius = p_scene_gps->planet_radius;
		break;

	      case SAR_PARM_SCENE_MAP:
		p_scene_map = (sar_parm_scene_map_struct *)p;
		break;

	      case SAR_PARM_SCENE_ELEVATION:
		p_scene_elevation = (sar_parm_scene_elevation_struct *)p;
		scene->msl_elevation = p_scene_elevation->elevation;
		break;

	      case SAR_PARM_SCENE_CANT:
		p_scene_cant = (sar_parm_scene_cant_struct *)p;
		scene->cant_angle = p_scene_cant->cant;
		break;

	      case SAR_PARM_SCENE_GROUND_FLAGS:
		p_scene_ground_flags = (sar_parm_scene_ground_flags_struct *)p;
		scene->base_flags = (sar_scene_base_flags)p_scene_ground_flags->flags;
		break;

	      case SAR_PARM_SCENE_GROUND_TILE:
		p_scene_ground_tile = (sar_parm_scene_ground_tile_struct *)p;
		if(True)
		{
		    float min, max;
		    GLuint list;
		    sar_visual_model_struct **vmodel;
		    v3d_texture_ref_struct *t = NULL;

		    /* Close range tiling size and range */
		    scene->base.tile_width = ((p_scene_ground_tile->tile_width > 0) ?
			p_scene_ground_tile->tile_width : SAR_DEF_GROUND_BASE_TILE_WIDTH
		    );
		    scene->base.tile_height = ((p_scene_ground_tile->tile_height > 0) ?
			p_scene_ground_tile->tile_height : SAR_DEF_GROUND_BASE_TILE_HEIGHT
		    );
		    scene->base.close_range = ((p_scene_ground_tile->close_range > 0) ?
			p_scene_ground_tile->close_range : SAR_DEF_GROUND_BASE_TILE_CLOSE_RANGE
		    );
		    /* Far solid color */
		    memcpy(
		        &scene->base.color, &p_scene_ground_tile->color,
		        sizeof(sar_color_struct)
		    );

		    /* Select texture */
		    t = SARGetTextureRefByName(scene, p_scene_ground_tile->texture_name);

		    /* Create the simple/far Ground Base Visual Model
		     * as needed
		     */
		    vmodel = &scene->base.visual_model_simple;
		    if(*vmodel != NULL)
		    {
			fprintf(
			    stderr,
"%s: Warning: Ground base simple/far visual model is already defined.\n",
			    filename
			);
		    }
		    else
		    {
			/* Create new visual model */
			*vmodel = SARVisualModelNew(
			    scene, NULL, NULL
			);

			/* (Re)generate GL list on visual model structure */
			list = (GLuint)SARVisualModelNewList(*vmodel);
			if(list != 0)
			{
			    /* Mark visual model as loading */
			    (*vmodel)->load_state = SAR_VISUAL_MODEL_LOADING;

			    /* Begin recording new list */
			    glNewList(list, GL_COMPILE);  
			    {
				/* Far (big) ground base tiles */
				min = -(SAR_MAX_VISIBILITY_DISTANCE +
				    (0.25 * SAR_MAX_VISIBILITY_DISTANCE));
				max = (SAR_MAX_VISIBILITY_DISTANCE +
				    (0.25 * SAR_MAX_VISIBILITY_DISTANCE));
				SARObjGenerateTilePlane(
				    min, max,               /* Min and max */
				    (float)(max / 4),         /* Tile width and height */
				    (float)(max / 4)
				);
			    }
			    glEndList();

			    /* Mark visual model as done loading */
			    (*vmodel)->load_state = SAR_VISUAL_MODEL_LOADED;
			} 
		    }

		    /* Create the detailed/near Ground Base Visual Model
		     * as needed
		     */
		    vmodel = &scene->base.visual_model_close;
		    if(*vmodel != NULL)
		    {
			fprintf(
			    stderr,
"%s: Warning: Ground base detailed/near visual model is already defined.\n",
			    filename
			);
		    }
		    else
		    {
			/* Create new visual model */
			*vmodel = SARVisualModelNew(
			    scene, NULL, NULL
			);

			/* (Re)generate GL list on visual model structure */
			list = (GLuint)SARVisualModelNewList(*vmodel);
			if(list != 0)
			{
			    /* Mark visual model as loading */
			    (*vmodel)->load_state = SAR_VISUAL_MODEL_LOADING;

			    /* Begin recording new list */
			    glNewList(list, GL_COMPILE);
			    {
				/* Select tiled ground texture if defined, if
				 * no texture then a texture unselect will be
				 * recorded.
				 */
				V3DTextureSelect(t);

				/* Generate close range textured tiles */
				SARObjGenerateTilePlane(
				    -scene->base.close_range,       /* Min */
				    scene->base.close_range,        /* Max */
				    (float)scene->base.tile_width,         /* Tile width */
				    (float)scene->base.tile_height         /* Tile height */ 
				);
			    }
			    glEndList();

			    /* Mark visual model as done loading */
			    (*vmodel)->load_state = SAR_VISUAL_MODEL_LOADED;
			}
		    }
		}
		break;

	      case SAR_PARM_TEXTURE_BASE_DIRECTORY:
		p_texture_base_directory = (sar_parm_texture_base_directory_struct *)p;
		break;

	      case SAR_PARM_TEXTURE_LOAD:
		p_texture_load = (sar_parm_texture_load_struct *)p;
		SARObjLoadTexture(
		    core_ptr, scene, p_texture_load
		);
		break;

	      /* Skip mission parms */

	      case SAR_PARM_NEW_OBJECT:
		p_new_object = (sar_parm_new_object_struct *)p;
		obj_num = SARObjNew(
		    scene, ptr, total,
		    p_new_object->object_type
		);
		obj_ptr = (obj_num > -1) ? (*ptr)[obj_num] : NULL;

		/* Reset all substructure type pointers */
		DO_RESET_SUBSTRUCTURE_PTRS

		if(obj_ptr != NULL)
		{
		    /* Get pointer to substructure */
		    switch(obj_ptr->type)
		    {
		      case SAR_OBJ_TYPE_GARBAGE:
		      case SAR_OBJ_TYPE_STATIC:
		      case SAR_OBJ_TYPE_AUTOMOBILE:
		      case SAR_OBJ_TYPE_WATERCRAFT:
			break;
		      case SAR_OBJ_TYPE_AIRCRAFT:
		        obj_aircraft_ptr = SAR_OBJ_GET_AIRCRAFT(obj_ptr);
		        break;
		      case SAR_OBJ_TYPE_GROUND:
		        obj_ground_ptr = SAR_OBJ_GET_GROUND(obj_ptr);
		        break;
		      case SAR_OBJ_TYPE_RUNWAY:
		      case SAR_OBJ_TYPE_HELIPAD:
		      case SAR_OBJ_TYPE_HUMAN:
		      case SAR_OBJ_TYPE_SMOKE:
		      case SAR_OBJ_TYPE_FIRE:
		      case SAR_OBJ_TYPE_EXPLOSION:
		      case SAR_OBJ_TYPE_CHEMICAL_SPRAY:
		      case SAR_OBJ_TYPE_FUELTANK:
		      case SAR_OBJ_TYPE_PREMODELED:
			break;
		    }
		}
		break;

	      case SAR_PARM_NEW_HELIPAD:
		p_new_helipad = (sar_parm_new_helipad_struct *)p;
		DO_RESET_SUBSTRUCTURE_PTRS
		obj_num = SARObjLoadHelipad(
		    core_ptr, scene, p_new_helipad
		);
		obj_ptr = (obj_num > -1) ? (*ptr)[obj_num] : NULL;
		obj_helipad_ptr = SAR_OBJ_GET_HELIPAD(obj_ptr);
		break;

	      case SAR_PARM_NEW_RUNWAY:
		p_new_runway = (sar_parm_new_runway_struct *)p;
		DO_RESET_SUBSTRUCTURE_PTRS
		obj_num = SARObjLoadRunway(
		    core_ptr, scene, p_new_runway
		);
		obj_ptr = (obj_num > -1) ? (*ptr)[obj_num] : NULL;
		obj_runway_ptr = SAR_OBJ_GET_RUNWAY(obj_ptr);
		break;

	      case SAR_PARM_NEW_HUMAN:
		p_new_human = (sar_parm_new_human_struct *)p;
		DO_RESET_SUBSTRUCTURE_PTRS
		obj_num = SARObjLoadHuman(
		    core_ptr, scene, p_new_human
		);
		obj_ptr = (obj_num > -1) ? (*ptr)[obj_num] : NULL;
		obj_human_ptr = SAR_OBJ_GET_HUMAN(obj_ptr);
		break;

	      case SAR_PARM_NEW_FIRE:
		p_new_fire = (sar_parm_new_fire_struct *)p;
		DO_RESET_SUBSTRUCTURE_PTRS
		obj_num = SARObjLoadFire(
		    core_ptr, scene, p_new_fire
		);
		obj_ptr = (obj_num > -1) ? (*ptr)[obj_num] : NULL;
		obj_fire_ptr = SAR_OBJ_GET_FIRE(obj_ptr);
		break;

	      case SAR_PARM_NEW_SMOKE:
		p_new_smoke = (sar_parm_new_smoke_struct *)p;
		DO_RESET_SUBSTRUCTURE_PTRS
		obj_num = SARObjLoadSmoke(
		    core_ptr, scene, p_new_smoke
		);
		obj_ptr = (obj_num > -1) ? (*ptr)[obj_num] : NULL;
		obj_smoke_ptr = SAR_OBJ_GET_SMOKE(obj_ptr);
		break;

	      case SAR_PARM_NEW_PREMODELED:
		p_new_premodeled = (sar_parm_new_premodeled_struct *)p;
		DO_RESET_SUBSTRUCTURE_PTRS
		obj_num = SARObjPremodeledNew(
		    core_ptr, scene,
		    p_new_premodeled->model_type,
		    p_new_premodeled->argc,
		    p_new_premodeled->argv
		);
		obj_ptr = (obj_num > -1) ? (*ptr)[obj_num] : NULL;
		break;

	      case SAR_PARM_MODEL_FILE:
		p_model_file = (sar_parm_model_file_struct *)p;
		if((p_model_file->file != NULL) && (obj_ptr != NULL))
		    SARObjLoadFromFile(core_ptr, obj_num, p_model_file->file);
		break;

	      case SAR_PARM_RANGE:
		p_range = (sar_parm_range_struct *)p;
		if(obj_ptr != NULL)
		{
		    obj_ptr->range = (float)MAX(p_range->range, 0.0);
		}
		break;

	      case SAR_PARM_RANGE_FAR:
		p_range_far = (sar_parm_range_far_struct *)p;
		if(obj_ptr != NULL)
		{
		    obj_ptr->range_far = (float)MAX(p_range_far->range_far, 0.0);
		}
		break;

	      case SAR_PARM_TRANSLATE:
		p_translate = (sar_parm_translate_struct *)p;
		SARObjLoadTranslate(
		    core_ptr, scene,
		    obj_ptr, p_translate
		);
		break;

	      case SAR_PARM_TRANSLATE_RANDOM:
		p_translate_random = (sar_parm_translate_random_struct *)p;
/* Ignore this, for missions only) */
		break;

	      case SAR_PARM_ROTATE:
		p_rotate = (sar_parm_rotate_struct *)p;
		if(obj_ptr != NULL)
		{
		    SARSimWarpObject(  
			scene, obj_ptr,
			NULL, &p_rotate->rotate
		    );
		}
		break;

	      case SAR_PARM_NO_DEPTH_TEST:
		p_no_depth_test = (sar_parm_no_depth_test_struct *)p;
		if(obj_ptr != NULL)
		{
		    obj_ptr->flags |= SAR_OBJ_FLAG_NO_DEPTH_TEST;
		}
		break;

	      case SAR_PARM_POLYGON_OFFSET:
		p_polygon_offset = (sar_parm_polygon_offset_struct *)p;
		if(obj_ptr != NULL)
		{
		    obj_ptr->flags |= p_polygon_offset->flags;
		}
		break;

	      case SAR_PARM_CONTACT_BOUNDS_SPHERICAL:
		p_contact_bounds_spherical = (sar_parm_contact_bounds_spherical_struct *)p;
		if(obj_ptr != NULL)
		{
		    sar_contact_bounds_struct *cb = obj_ptr->contact_bounds;
		    SARObjAddContactBoundsSpherical(
			obj_ptr,
			(cb != NULL) ? cb->crash_flags : 0,
			(cb != NULL) ? cb->crash_type : 0,
			p_contact_bounds_spherical->radius
		    );
		}
		break;

	      case SAR_PARM_CONTACT_BOUNDS_CYLENDRICAL:
		p_contact_bounds_cylendrical = (sar_parm_contact_bounds_cylendrical_struct *)p;
		if(obj_ptr != NULL)
		{
		    sar_contact_bounds_struct *cb = obj_ptr->contact_bounds;
		    SARObjAddContactBoundsCylendrical(
			obj_ptr,
			(cb != NULL) ? cb->crash_flags : 0,
			(cb != NULL) ? cb->crash_type : 0,
			p_contact_bounds_cylendrical->radius,
			p_contact_bounds_cylendrical->height_min,
			p_contact_bounds_cylendrical->height_max
		    );
		}
		break;

	      case SAR_PARM_CONTACT_BOUNDS_RECTANGULAR:
		p_contact_bounds_rectangular = (sar_parm_contact_bounds_rectangular_struct *)p;
		if(obj_ptr != NULL)
		{
		    sar_contact_bounds_struct *cb = obj_ptr->contact_bounds;
		    SARObjAddContactBoundsRectangular(
			obj_ptr,
			(cb != NULL) ? cb->crash_flags : 0,
			(cb != NULL) ? cb->crash_type : 0,
			p_contact_bounds_rectangular->x_min,
			p_contact_bounds_rectangular->x_max,
			p_contact_bounds_rectangular->y_min, 
			p_contact_bounds_rectangular->y_max,
			p_contact_bounds_rectangular->z_min, 
			p_contact_bounds_rectangular->z_max
		    );
		}
		break;

	      case SAR_PARM_GROUND_ELEVATION:
		p_ground_elevation = (sar_parm_ground_elevation_struct *)p;
		if(obj_ground_ptr != NULL)
		{     
		    obj_ground_ptr->elevation = p_ground_elevation->elevation;
		}
		else
		{
		    fprintf(
			stderr,
"%s: Warning:\
 Unable to set ground elevation for object not of type \"%i\".\n",
 			filename, SAR_OBJ_TYPE_GROUND
		    );
		}
		break;

	      case SAR_PARM_OBJECT_NAME:
		p_object_name = (sar_parm_object_name_struct *)p;
		if(obj_ptr != NULL)
		{
		    free(obj_ptr->name);
		    obj_ptr->name = STRDUP(p_object_name->name);
		}
		break;

	      case SAR_PARM_OBJECT_MAP_DESCRIPTION:
		p_object_map_description = (sar_parm_object_map_description_struct *)p;
		if(obj_ptr != NULL)
		{
/* Ignore this */
		}
		break;

	      case SAR_PARM_FUEL:
		p_fuel = (sar_parm_fuel_struct *)p;
		if(obj_aircraft_ptr != NULL)
		{
		    /* Set current fuel */
		    obj_aircraft_ptr->fuel = p_fuel->fuel;

		    /* Set max only if not negative */
		    if(p_fuel->fuel_max >= 0.0)
			obj_aircraft_ptr->fuel_max = p_fuel->fuel_max;

		    /* Sanitize current */
		    if(obj_aircraft_ptr->fuel > obj_aircraft_ptr->fuel_max)
			obj_aircraft_ptr->fuel = obj_aircraft_ptr->fuel_max;
		}
		break;

	      case SAR_PARM_HITPOINTS:
		p_hitpoints = (sar_parm_hitpoints_struct *)p;
		if(obj_ptr != NULL)
		{
		    /* Set current hit points */
		    obj_ptr->hit_points = p_hitpoints->hitpoints;

		    /* Set max only if not negative */
		    if(p_hitpoints->hitpoints_max >= 0.0)
			obj_ptr->hit_points_max = p_hitpoints->hitpoints_max;

		    /* Sanitize current */
		    if(obj_ptr->hit_points > obj_ptr->hit_points_max)
			obj_ptr->hit_points = obj_ptr->hit_points_max;
		}
		break;

	      case SAR_PARM_ENGINE_STATE:
		p_engine_state = (sar_parm_engine_state_struct *)p;
                if (obj_aircraft_ptr != NULL)
                {
                    obj_aircraft_ptr->engine_state = p_engine_state->state;
                }
/* TODO Work on this later */
		break;

	      case SAR_PARM_PASSENGERS:
		p_passengers = (sar_parm_passengers_struct *)p;
/* TODO Work on this later */
		break;

	      case SAR_PARM_RUNWAY_APPROACH_LIGHTING_NORTH:
		p_runway_applight_n =
		    (sar_parm_runway_approach_lighting_north_struct *)p;
		if((obj_ptr != NULL) && (obj_runway_ptr != NULL))
		{
		    obj_runway_ptr->north_approach_lighting_flags =
			p_runway_applight_n->flags;
		}
		break;

	      case SAR_PARM_RUNWAY_APPROACH_LIGHTING_SOUTH:
		p_runway_applight_s =
		    (sar_parm_runway_approach_lighting_south_struct *)p;
		if((obj_ptr != NULL) && (obj_runway_ptr != NULL))
		{
		    obj_runway_ptr->south_approach_lighting_flags =
			p_runway_applight_s->flags;
		}
		break;

	      case SAR_PARM_HUMAN_MESSAGE_ENTER:
		p_human_message_enter = (sar_parm_human_message_enter_struct *)p;
		if((obj_ptr != NULL) && (obj_human_ptr != NULL))
		{
		    free(obj_human_ptr->mesg_enter);
		    obj_human_ptr->mesg_enter = STRDUP(
			p_human_message_enter->message
		    );
		}
		break;

	      case SAR_PARM_HUMAN_REFERENCE:
		p_human_reference = (sar_parm_human_reference_struct *)p;
		if((obj_ptr != NULL) && (obj_human_ptr != NULL) &&
		   (p_human_reference->reference_name != NULL)
		)
		{
		    int human_ref_obj_num = -1;
		    const char *ref_name = (const char *)p_human_reference->reference_name;

		    /* Run towards? */
		    if(p_human_reference->flags & SAR_HUMAN_FLAG_RUN_TOWARDS)
		    {
			obj_human_ptr->flags |= SAR_HUMAN_FLAG_RUN_TOWARDS;
		    }
		    /* Run away? */
		    else if(p_human_reference->flags & SAR_HUMAN_FLAG_RUN_AWAY)
		    {
			obj_human_ptr->flags |= SAR_HUMAN_FLAG_RUN_AWAY;
		    }

		    /* Handle reference object name */
		    if(!strcasecmp(ref_name, "player"))
		    {
			/* Set special intercept code to intercept the player */
			obj_human_ptr->intercepting_object = -2;
		    }
		    else
		    {
			/* All else match by object name */
			SARObjMatchPointerByName(
			    scene, *ptr, *total,
			    ref_name, &human_ref_obj_num
			);
			obj_human_ptr->intercepting_object = human_ref_obj_num;
		    }
		}
		break;
                
                case SAR_PARM_WELCOME_MESSAGE:
                    p_welcome_message = (sar_parm_welcome_message_struct*) p;
                    free(scene->welcome_message);
                    scene->welcome_message = STRDUP(p_welcome_message->message);
                    break;

	    }	/* Handle by parm type */
	}	/* Iterate through loaded parms */


	/* Delete loaded parms */
	SARParmDeleteAll(&parm, &total_parms);

#undef DO_RESET_SUBSTRUCTURE_PTRS

	return(0);
}
