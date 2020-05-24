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
#include <sys/types.h>
#include <math.h>

#include <string.h>

#ifdef __MSW__
# include <windows.h>
#endif
#include <GL/gl.h>
#include <GL/glu.h>

#include "../include/string.h"

#include "matrixmath.h"
#include "sfm.h"
#include "gw.h"
#include "stategl.h"
#include "textinput.h"
#include "sarreality.h"
#include "messages.h"
#include "gctl.h"
#include "obj.h"
#include "objutils.h"
#include "objsound.h"
#include "simop.h"
#include "simutils.h"
#include "sar.h"
#include "sardraw.h"
#include "sardrawselect.h"
#include "sardrawpm.h"
#include "sardrawdefs.h"
#include "config.h"


/* Camera Setting */
static void SARSetCameraMapDrawGCC(sar_dc_struct *dc, int obj_num);
static void SARSetCamera(sar_dc_struct *dc);

/* Spot Light Setting & Cast Drawing */
static void SARDrawSetSpotLight(
	sar_dc_struct *dc, sar_object_struct *obj_ptr
);
static void SARDrawSpotLightCast(
	sar_dc_struct *dc, sar_object_struct *obj_ptr
);

/* Object Compoents & Effects */
static void SARDrawPart(
	sar_dc_struct *dc,
	sar_object_struct *obj_ptr, const sar_obj_part_struct *part
);
static void SARDrawRotor(
	sar_dc_struct *dc,
	sar_object_struct *obj_ptr, sar_obj_rotor_struct *rotor,
	float throttle
);
static void SARDrawFuelTank(
	sar_dc_struct *dc,
	sar_object_struct *obj_ptr,
	sar_external_fueltank_struct *eft_ptr
);
static void SARDrawHoistDeployment(
	sar_dc_struct *dc,
	sar_object_struct *obj_ptr, const sar_obj_hoist_struct *hoist
);
static void SARDrawSmoke(
	sar_dc_struct *dc,
	sar_object_struct *obj_ptr, sar_object_smoke_struct *obj_smoke_ptr
);
static void SARDrawExplosion(
	sar_dc_struct *dc,
	sar_object_struct *obj_ptr,
	sar_object_explosion_struct *obj_explosion_ptr
);
static void SARDrawFire(
	sar_dc_struct *dc,
	sar_object_struct *obj_ptr,
	sar_object_fire_struct *obj_fire_ptr
);

/* HUD */
static void SARDrawHUDAttitude(
	sar_dc_struct *dc,
	const sar_direction_struct *dir,
	const sar_position_struct *vel,
	int offset_x, int offset_y,
	float fovx, float fovy
);
static void SARDrawHUDHeading(
	sar_dc_struct *dc,
	const sar_direction_struct *dir,
	const sar_intercept_struct *intercept,
	int offset_x, int offset_y	/* Look offset */
);
static void SARDrawHUDElevatorTrim(
	sar_dc_struct *dc,
	GLfloat xc, GLfloat yc,
	GLfloat length, GLfloat width,
	float elevator_trim 
);
static void SARDrawHUD(
	sar_dc_struct *dc, sar_object_struct *obj_ptr
);
static void SARDrawOutsideAttitude(
	sar_dc_struct *dc, sar_object_struct *obj_ptr
);

/* Map Grids & Markings */
static void SARDrawMapGrids(
	sar_dc_struct *dc, sar_position_struct *pos
);
static void SARDrawMapCrossHairs(sar_dc_struct *dc);

/* Lights */
static void SARDrawLights(
	sar_dc_struct *dc,
	sar_light_struct **ptr, int total
);

/* Cockpit */
static void SARDrawObjectCockpit(
	sar_dc_struct *dc, sar_object_struct *obj_ptr
);

/* Shadow */
static void SARDrawObjectShadow(
	sar_dc_struct *dc, sar_object_struct *obj_ptr
);

/* Scene Foundation (Ground) */
static void SARDrawSceneFoundations(sar_dc_struct *dc);

/* Clouds (Tiled Layer & BillBoards) */
static void SARDrawCloudLayer(
	sar_dc_struct *dc,
	sar_cloud_layer_struct *cloud_layer_ptr, Boolean flip
);
static void SARDrawCloudBBIterate(
	sar_dc_struct *dc,
	const sar_position_struct *pos,
	float hw, float height,
	const GLfloat *c,
	v3d_texture_ref_struct *t,
	float lightening_intensity_coeff,
	const sar_position_struct *lightening_point,
	int lightening_points
);
static void SARDrawCloudBB(
	sar_dc_struct *dc,
	const sar_cloud_bb_struct *cloud_bb_ptr
);

/* Horizon & Celestial Objects */
static void SARDrawSceneHorizon(sar_dc_struct *dc);
static void SARDrawSceneCelestialAdditive(sar_dc_struct *dc);

/* Main  */
void SARDraw(sar_core_struct *core_ptr);
void SARDrawMap(
	sar_core_struct *core_ptr,
	Boolean draw_for_gcc, Boolean draw_for_ghc, int gcc_obj_num
);


/*
 *	Sets up camera for map draw ground contact check (called by
 *	SARDrawMap() with draw_for_gcc set to True).
 *
 *	Works the same as SARSetCamera() except sets camera position to be
 *	at position of specified object obj_num looking straight down.
 *	If obj_num is -1 then it defaults to the scene structure's player
 *	object.
 */
static void SARSetCameraMapDrawGCC(sar_dc_struct *dc, int obj_num)
{
	sar_object_struct *obj_ptr;
	sar_scene_struct *scene = dc->scene;
	sar_position_struct *cam_pos = &dc->camera_pos;
	sar_direction_struct *cam_dir = &dc->camera_dir;

	/* Reset camera rotation matrix */
	scene->camera_rotmatrix_count = 0;

	/* Update camera ref code */
	dc->camera_ref = scene->camera_ref;

	/* Camera is not in cockpit for this situation */
	dc->camera_in_cockpit = False;
	dc->ear_in_cockpit = False;

	/* Get pointer to object (use player object as needed) */
	if((obj_num >= 0) && (obj_num < dc->total_objects))
	    obj_ptr = dc->object[obj_num];
	else
	    obj_ptr = scene->player_obj_ptr;

	/* Got reference object? */
	if(obj_ptr != NULL)
	{
	    int i;
	    const sar_position_struct *pos = &obj_ptr->pos;

	    /* Set up initial normal */
	    glNormal3f(0.0f, 0.0f, 1.0f);

	    glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
	    glTranslatef(-pos->x, -pos->z, pos->y);

	    /* Record camera position */
	    memcpy(cam_pos, pos, sizeof(sar_position_struct));

	    /* Record camera direction */
	    cam_dir->heading = (float)(0.0 * PI);
	    cam_dir->pitch = (float)(0.5 * PI);
	    cam_dir->bank = (float)(0.0 * PI);

	    /* Set camera rotation matrix */
	    i = 0;
	    if(i < SAR_CAMERA_ROTMATRIX_MAX)
	    {
		MatrixGetHeading33(
		    0.0 * PI,
		    scene->camera_rotmatrix[i]
		);
		scene->camera_rotmatrix_count = i + 1;
	    }
	    i = 1;
	    if(i < SAR_CAMERA_ROTMATRIX_MAX)
	    {
		MatrixGetPitch33(
		    -0.5 * PI,
		    scene->camera_rotmatrix[i] 
		);
		scene->camera_rotmatrix_count = i + 1;
	    }
	}
}


/*
 *	Translates camera to proper positions defined on the given
 *	scene structure.
 *
 *	Local globals camera_pos, camera_dir, camera_in_cockpit, and
 *	ear_in_cockpit will be updated.
 *
 *	Scene structure's ear_pos will be updated as needed.
 *
 *	Note: if GET_CAM_DIR is False then camera_dir will be all
 *	zero'ed.
 */
static void SARSetCamera(sar_dc_struct *dc)
{
	int obj_num;
	sar_object_struct *obj_ptr;
	sar_scene_struct *scene = dc->scene;
	sar_position_struct *cam_pos = &dc->camera_pos;
	sar_direction_struct *cam_dir = &dc->camera_dir;


	/* Reset camera rotation matrix */
	scene->camera_rotmatrix_count = 0;

	/* Update camera ref code */
	dc->camera_ref = scene->camera_ref;

	switch(dc->camera_ref)
	{
	  case SAR_CAMERA_REF_HOIST:
	    dc->camera_in_cockpit = False;
	    dc->ear_in_cockpit = False;
	    obj_ptr = scene->player_obj_ptr;
	    if(obj_ptr != NULL)
	    {
		int i;
		float d = scene->camera_hoist_dist;
		float cx, cy, cz;
		float dx, dy, dz;
		float dxu, dyu, dzu;
		float ux, uy, uz;
		float sin_heading, cos_heading;
		float cos_pitch;
		const sar_direction_struct *dir, *dir2;
		const sar_position_struct *pos;
		const sar_obj_hoist_struct *hoist_ptr;


		/* Get first hoist on object and check if the hoist
		 * rope is deployed, if it is then use the rope end as
		 * the target position otherwise use the object as the
		 * target position
		 */
		hoist_ptr = SARObjGetHoistPtr(obj_ptr, 0, NULL);
		if((hoist_ptr != NULL) ? (hoist_ptr->rope_cur_vis > 0.0f) : False)
		{
		    pos = &hoist_ptr->pos;
		    dir = &obj_ptr->dir;
		    dir2 = &scene->camera_hoist_dir;
		}
		else
		{
		    pos = &obj_ptr->pos;
		    dir = &obj_ptr->dir;
		    dir2 = &scene->camera_hoist_dir;
		}

		/* Pre-calculate trig values */
		sin_heading = (float)sin(dir->heading);
		cos_heading = (float)cos(dir->heading);
		cos_pitch = (float)cos(dir2->pitch);	/* Camera spot pitch */

		/* Calculate unit delta from object to camera */
		dxu = (float)sin(dir2->heading) * cos_pitch;
		dyu = (float)cos(dir2->heading) * cos_pitch;
		dzu = (float)-sin(dir2->pitch);

		/* Calculate delta from object to camera */
		dx = dxu * d;
		dy = dyu * d;
		dz = dzu * d;

		/* Calculate camera position */
		cx = pos->x + (cos_heading * dx) +
		    (sin_heading * dy);
		cy = pos->y - (sin_heading * dx) +
		    (cos_heading * dy);
		cz = MAX(pos->z + dz, 1.0f);

		/* Calculate up vector */
		ux = -((sin_heading * dyu) + (cos_heading * dxu)) * dzu;
		uy = -((cos_heading * dyu) - (sin_heading * dxu)) * dzu;
		uz = cos_pitch;

		/* Set camera position */
		gluLookAt(
		    cx, cz, -cy,
		    pos->x, pos->z, -pos->y,
		    ux, uz, -uy
		);

		/* Record camera position */
		cam_pos->x = cx;
		cam_pos->y = cy;
		cam_pos->z = cz;

		/* Set camera rotation matrix */
		i = 0;
		if(i < SAR_CAMERA_ROTMATRIX_MAX)
		{
		    MatrixGetHeading33(
			-(dir2->heading + dir->heading + PI),
			scene->camera_rotmatrix[i]
		    );
		    scene->camera_rotmatrix_count = i + 1;
		}
		i = 1;
		if(i < SAR_CAMERA_ROTMATRIX_MAX)
		{
		    MatrixGetPitch33(
			dir2->pitch,
			scene->camera_rotmatrix[i]
		    );
		    scene->camera_rotmatrix_count = i + 1;
		}
	    }
	    break;

	  case SAR_CAMERA_REF_MAP:
	    dc->camera_in_cockpit = False;
	    dc->ear_in_cockpit = False;
	    obj_ptr = scene->player_obj_ptr;
	    if(obj_ptr != NULL)
	    {
		int i;
		const sar_position_struct *pos = &scene->camera_map_pos;

		/* Set up initial normal before we translate */
		glNormal3f(0.0f, 0.0f, 1.0f);

		glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
		glTranslatef(-pos->x, -pos->z, pos->y);

		/* Record camera position */
		memcpy(cam_pos, pos, sizeof(sar_position_struct));

		/* Set camera rotation matrix */
		i = 0;
		if(i < SAR_CAMERA_ROTMATRIX_MAX)
		{
		    MatrixGetHeading33(
			0.0 * PI,
			scene->camera_rotmatrix[i]
		    );
		    scene->camera_rotmatrix_count = i + 1;
		}
		i = 1;
		if(i < SAR_CAMERA_ROTMATRIX_MAX)
		{
		    MatrixGetPitch33(
			-0.5 * PI,
			scene->camera_rotmatrix[i]
		    );
		    scene->camera_rotmatrix_count = i + 1;
		}
	    }
	    break;

	  case SAR_CAMERA_REF_TOWER:
	    dc->camera_in_cockpit = False;
	    dc->ear_in_cockpit = False;
	    obj_num = scene->camera_target;
	    if(SARObjIsAllocated(
		dc->object, dc->total_objects, obj_num
	    ))
	    {
		obj_ptr = dc->object[obj_num];
	    }
	    else
	    {
		obj_num = scene->player_obj_num;
		obj_ptr = scene->player_obj_ptr;
	    }
	    if(obj_ptr != NULL)
	    {
		int i;
		float x, y, z;
		const sar_position_struct *pos = &obj_ptr->pos;

		/* Get position of tower */
		x = scene->camera_tower_pos.x;
		y = scene->camera_tower_pos.y;
		z = MAX(scene->camera_tower_pos.z, 1.0f);

		gluLookAt(
		    x, z, -y,
		    pos->x, pos->z, -pos->y,
		    0.0, 1.0, 0.0
		);

		/* Record camera position */
		cam_pos->x = x;
		cam_pos->y = y;
		cam_pos->z = z;

		/* Set camera rotation matrix */
		i = 0;
		if(i < SAR_CAMERA_ROTMATRIX_MAX)
		{
		    MatrixGetHeading33(
			-((0.5 * PI) - atan2(pos->y - y, pos->x - x)),
			scene->camera_rotmatrix[i]
		    );
		    scene->camera_rotmatrix_count = i + 1;
		}
		i = 1;
		if(i < SAR_CAMERA_ROTMATRIX_MAX)
		{
		    MatrixGetPitch33(
			atan2(pos->z - z,
			    SFMHypot2(pos->y - y, pos->x - x)
			),
			scene->camera_rotmatrix[i]
		    );
		    scene->camera_rotmatrix_count = i + 1;
		}
	    }
	    else
	    {
		/* No target object, set a default camera position */
		gluLookAt(
		    0.0, 0.0, 0.0,
		    0.0, 0.0, -1.0,
		    0.0, 1.0, 0.0
		);
	    }
	    break;

	  case SAR_CAMERA_REF_SPOT:
	    dc->camera_in_cockpit = False;
	    dc->ear_in_cockpit = False;
	    obj_num = scene->camera_target;
	    if(SARObjIsAllocated(
		dc->object, dc->total_objects, obj_num
	    ))
	    {
		obj_ptr = dc->object[obj_num];
	    }
	    else
	    {
		obj_num = scene->player_obj_num;
		obj_ptr = scene->player_obj_ptr;
	    }
	    if(obj_ptr != NULL)
	    {
		int i;
		float d = scene->camera_spot_dist;
		float cx, cy, cz;
		float dx, dy, dz;
		float dxu, dyu, dzu;
		float ux, uy, uz;
		float sin_heading, cos_heading;
		float cos_pitch;
		const sar_direction_struct	*dir = &obj_ptr->dir,
						*dir2 = &scene->camera_spot_dir;
		const sar_position_struct	*pos = &obj_ptr->pos;

		/* Pre-calculate object heading trig values */
		sin_heading = (float)sin(dir->heading);
		cos_heading = (float)cos(dir->heading);
		cos_pitch = (float)cos(dir2->pitch);	/* Camera spot pitch */

		/* Calculate unit delta from object to camera */
		dxu = (float)sin(dir2->heading) * cos_pitch;
		dyu = (float)cos(dir2->heading) * cos_pitch;
		dzu = (float)-sin(dir2->pitch);

		/* Calculate delta from object to camera */
		dx = dxu * d;
		dy = dyu * d;
		dz = dzu * d;

		/* Calculate camera position */
		cx = pos->x + (cos_heading * dx) +
		    (sin_heading * dy);
		cy = pos->y - (sin_heading * dx) +
		    (cos_heading * dy);
		cz = MAX(pos->z + dz, 1.0f);

		/* Calculate up vector */
		ux = -((sin_heading * dyu) + (cos_heading * dxu)) * dzu;
 		uy = -((cos_heading * dyu) - (sin_heading * dxu)) * dzu;
		uz = cos_pitch;

		/* Set camera position */
		gluLookAt(
		    cx, cz, -cy,
		    pos->x, pos->z, -pos->y,
		    ux, uz, -uy
		);

		/* Record camera position */
		cam_pos->x = cx;
		cam_pos->y = cy;
		cam_pos->z = cz;

		/* Set camera rotation matrix */
		i = 0;
		if(i < SAR_CAMERA_ROTMATRIX_MAX)
		{
		    MatrixGetHeading33(
			-(dir2->heading + dir->heading + PI),
			scene->camera_rotmatrix[i]
		    );
		    scene->camera_rotmatrix_count = i + 1;
		}
		i = 1;
		if(i < SAR_CAMERA_ROTMATRIX_MAX)
		{
		    MatrixGetPitch33(
			dir2->pitch,
			scene->camera_rotmatrix[i]
		    );
		    scene->camera_rotmatrix_count = i + 1;
		}
	    }
	    else
	    {
		/* No target object (set a default camera position) */
		gluLookAt(
		    0.0, 0.0, 0.0,
		    0.0, 0.0, -1.0,
		    0.0, 1.0, 0.0
		);
	    }
	    break;

	  default:	/* SAR_CAMERA_REF_COCKPIT */
	    dc->camera_in_cockpit = True;
	    dc->ear_in_cockpit = True;
	    obj_ptr = scene->player_obj_ptr;
	    if(obj_ptr != NULL)
	    {
		int i;
		const sar_position_struct	*pos = &obj_ptr->pos,
						*pos2;
		const sar_direction_struct	*dir = &obj_ptr->dir,
						*dir2 = &scene->camera_cockpit_dir;
		sar_object_aircraft_struct *aircraft = SAR_OBJ_GET_AIRCRAFT(obj_ptr);
		float ground_pitch_offset;

		/* Get object type specific values */
		if(aircraft != NULL)
		{
		    pos2 = &aircraft->cockpit_offset_pos;
		    ground_pitch_offset = aircraft->ground_pitch_offset;
		}
		else
		{
		    pos2 = NULL;
		    ground_pitch_offset = (float)(0.0 * PI);
		}

		/* Set up initial normal before translate */
		glNormal3f(0.0f, 0.0f, 1.0f);

		if(dir2->pitch != 0)
		    glRotatef(
			(float)SFMRadiansToDegrees(dir2->pitch),
			1.0f, 0.0f, 0.0f
		    );
		if(dir2->heading != 0)
		    glRotatef(
			(float)SFMRadiansToDegrees(dir2->heading),
			0.0f, 1.0f, 0.0f
		    );
		if(pos2 != NULL)
		    glTranslatef(-pos2->x, -pos2->z, pos2->y);

		if(dir->bank != 0)
		    glRotatef(
			(float)SFMRadiansToDegrees(dir->bank),
			0.0f, 0.0f, 1.0f
		    );
		if((dir->pitch + ground_pitch_offset) != 0)
		    glRotatef(
			(float)SFMRadiansToDegrees(dir->pitch + ground_pitch_offset),
			1.0f, 0.0f, 0.0f
		    );
		if(dir->heading != 0)
		    glRotatef(
			(float)SFMRadiansToDegrees(dir->heading),
			0.0f, 1.0f, 0.0f
		    );
		glTranslatef(-pos->x, -pos->z, pos->y);

		/* Record camera position */
		cam_pos->x = pos->x;
		cam_pos->y = pos->y;
		cam_pos->z = pos->z;

		/* Set camera rotation matrix */
		i = 0;
		if(i < SAR_CAMERA_ROTMATRIX_MAX)
		{
		    MatrixGetHeading33(
			-dir->heading,
			scene->camera_rotmatrix[i]
		    );
		    scene->camera_rotmatrix_count = i + 1;
		}
		i = 1;
		if(i < SAR_CAMERA_ROTMATRIX_MAX)
		{
		    MatrixGetPitch33(
			-dir->pitch,
			scene->camera_rotmatrix[i]
		    );
		    scene->camera_rotmatrix_count = i + 1;
		}
		i = 2;
		if(i < SAR_CAMERA_ROTMATRIX_MAX)
		{
		    MatrixGetBank33( 
			dir->bank,	/* Our bank value is already negative
					 * for this case.
					 */
			scene->camera_rotmatrix[i]
		    );
		    scene->camera_rotmatrix_count = i + 1;
		}
		i = 3;
		if(i < SAR_CAMERA_ROTMATRIX_MAX)
		{
		    MatrixGetHeading33(
			-dir2->heading,
			scene->camera_rotmatrix[i]
		    );
		    scene->camera_rotmatrix_count = i + 1;
		}
		i = 4;
		if(i < SAR_CAMERA_ROTMATRIX_MAX)
		{
		    MatrixGetPitch33(
			-dir2->pitch,
			scene->camera_rotmatrix[i]
		    );
		    scene->camera_rotmatrix_count = i + 1;
		}
	    }
	    else
	    {
		/* No target object so set a default camera position */
		gluLookAt(
		    0.0, 0.0, 0.0,
		    0.0, 0.0, -1.0,
		    0.0, 1.0, 0.0
		);
	    }
	    break;
	}

	/* Update camera direction? */
	if(GET_CAM_DIR)
	{
	    GLfloat val[16];

	    /* Get matrix containing results of our camera rotations
	     * and translations.
	     */
	    glGetFloatv(GL_MODELVIEW_MATRIX, val);

	    /* Record camera direction */
	    cam_dir->heading = (float)SFMSanitizeRadians(
	        (0.5 * PI) - atan2(val[10], val[8])
	    );
	    cam_dir->pitch = (float)SFMSanitizeRadians(
	        atan2(val[6], val[5])
	    );
	    cam_dir->bank = (float)SFMSanitizeRadians(asin(-val[4]));
	}
	else
	{
	    memset(cam_dir, 0x00, sizeof(sar_direction_struct));
	}

	/* Update ear position on scene structure based on calculated
	 * camera_pos so that the ear position follows the camera
	 */
	memcpy(&scene->ear_pos, cam_pos, sizeof(sar_position_struct));
}

/*
 *	Sets up the search light (GL_LIGHT1) for the player object, the
 *	given object must be the player object. This function will
 *	always either enable or disable the spot light when called.
 *
 *	Note that this does not draw the spot light cast, see
 *	SARDrawSpotLightCast().
 *
 *	Inputs assumed valid.
 *
 *	Matrix should be placed at the light's position.
 */
static void SARDrawSetSpotLight(
	sar_dc_struct *dc, sar_object_struct *obj_ptr
)
{
	gw_display_struct *display = dc->display;
	state_gl_struct *state = &display->state_gl;
	sar_scene_struct *scene = dc->scene;
	Boolean spot_light_on = False;
	const int light_num = GL_LIGHT1;	/* GL light number to use
						 * as the spot light */
	float g = 1.0f;
	const sar_color_struct *light_color = &scene->light_color;
	const float scene_gamma = MIN(
	    ((light_color->r + light_color->g + light_color->b) / 3.0f) +
	    scene->pri_lightening_coeff,
	    1.0f
	);
	const sar_position_struct *pos = NULL;
	const sar_direction_struct *dir = NULL;
	sar_object_aircraft_struct *aircraft =
	    SAR_OBJ_GET_AIRCRAFT(obj_ptr);

	/* Spot light should not show up in FLIR mode */
	if(dc->flir)
	    return;

	/* If the scene's gamma is less than 0.75 (if it is dark
	 * enough to see the spot light)?
	 */
	if(scene_gamma < 0.75f)
	{
	    /* Check if object has a spot light */
	    int i;
	    const sar_light_struct *light;
	    for(i = 0; i < obj_ptr->total_lights; i++)
	    {
		light = obj_ptr->light[i];
		if(light == NULL)
		    continue;

		if((light->flags & SAR_LIGHT_FLAG_ATTENUATE) &&
		   (light->flags & SAR_LIGHT_FLAG_ON)
		)
		    break;
	    }
	    /* Found spot light on object? */
	    if(i < obj_ptr->total_lights)
	    {
		/* Mark that we got a spot light and get its position &
		 * direction
		 */
		spot_light_on = True;
		pos = (light != NULL) ? &light->pos : NULL;
		dir = (aircraft != NULL) ?
		    &aircraft->spotlight_dir : NULL;

		/* Calculate the gamma for spot light
		 *
		 * As scene's gamma gets darker, the spot light
		 * gets brighter
		 */
		g = (float)CLIP(1.0 - scene_gamma, 0.0, 1.0);
	    }
	}

	/* Is the spot light suppose to be on? */
	if(spot_light_on && (pos != NULL) && (dir != NULL))
	{
	    double a[3 * 1], r[3 * 1];
	    GLfloat v[4];

	    /* Calculate spot light direction unit vector */
	    a[0] = 0.0;
	    a[1] = 1.0;
	    a[2] = 0.0;

	    /* Rotate matrix a into r */
	    MatrixRotateBank3(a, -dir->bank, r);	/* Our bank is negative */
	    MatrixRotatePitch3(r, dir->pitch, a);
	    MatrixRotateHeading3(a, dir->heading, r);

	    /* Enable spot light and set up new values */
	    StateGLEnable(state, light_num);

	    v[0] = pos->x;
	    v[1] = pos->z;
	    v[2] = -pos->y;
	    v[3] = 1.0f;		/* Positional light */
	    glLightfv(light_num, GL_POSITION, &(v[0]));

	    v[0] = 0.0f;
	    v[1] = 0.0f;
	    v[2] = 0.0f;
	    v[3] = 1.0f;
	    glLightfv(light_num, GL_AMBIENT, &(v[0]));

	    if(dc->flir)
	    {
		v[0] = 0.0f;
		v[1] = g;
		v[2] = 0.0f;
		v[3] = 1.0f;
	    }
	    else
	    {
		v[0] = g;
		v[1] = g;
		v[2] = g;
		v[3] = 1.0f;
	    }
	    glLightfv(light_num, GL_DIFFUSE, &(v[0]));

	    v[0] = 1.0f;
	    v[1] = 1.0f;
	    v[2] = 1.0f;
	    v[3] = 1.0f;
	    glLightfv(light_num, GL_SPECULAR, &(v[0]));

	    glLightf(light_num, (GLuint)GL_CONSTANT_ATTENUATION, 1.0f);
	    glLightf(light_num, (GLuint)GL_LINEAR_ATTENUATION, 0.01f);
	    glLightf(light_num, (GLuint)GL_QUADRATIC_ATTENUATION, 0.0f);

	    /* Set up light direction */
	    v[0] = (GLfloat)r[0];	/* X */
	    v[1] = (GLfloat)r[2];	/* Z */
	    v[2] = (GLfloat)-r[1];	/* -Y */
	    v[3] = 0.0f;
	    glLightfv(light_num, GL_SPOT_DIRECTION, &(v[0]));

	    glLightf(light_num, GL_SPOT_CUTOFF, 75.0f);
	    glLightf(light_num, GL_SPOT_EXPONENT, 0.0f);
	}
	else
	{
	    StateGLDisable(state, light_num);
	}
}

/*
 *	Draws the spot light cast based on the spot light of the given
 *	object.
 *
 *      Inputs assumed valid.
 *
 *      Matrix should be placed at the identity.
 */
static void SARDrawSpotLightCast(
	sar_dc_struct *dc, sar_object_struct *obj_ptr
)
{
	gw_display_struct *display = dc->display;
	state_gl_struct *state = &display->state_gl;
	sar_scene_struct *scene = dc->scene;
	const sar_option_struct *opt = dc->option;
	Boolean spot_light_on = False;
	float g = 1.0f;
	const sar_color_struct *light_color = &scene->light_color;
	const float scene_gamma = MIN(
	    ((light_color->r + light_color->g + light_color->b) / 3.0f) +
	    scene->pri_lightening_coeff,
	    1.0f
	);
	const sar_position_struct *pos = NULL;
	const sar_direction_struct *dir = NULL, *sdir = NULL;
	sar_object_aircraft_struct *aircraft = SAR_OBJ_GET_AIRCRAFT(obj_ptr);

	/* Spot light should not show up in FLIR mode */
	if(dc->flir)
	    return;

	/* Is the scene's gamma is less than 0.75 (is it dark enough
	 * to see the spot light cast)?
	 */
	if(scene_gamma < 0.75f)
	{
	    /* Check if object has a spot light */
	    int i;
	    const sar_light_struct *light;
	    for(i = 0; i < obj_ptr->total_lights; i++)
	    {
		light = obj_ptr->light[i];
		if(light == NULL)
		    continue;

		if((light->flags & SAR_LIGHT_FLAG_ATTENUATE) &&
		   (light->flags & SAR_LIGHT_FLAG_ON)
		)
		    break;
	    }
	    /* Found spot light on object? */
	    if(i < obj_ptr->total_lights)
	    {
		/* Mark that we got a spot light and get its position,
		 * direction, and the aircraft's direction
		 */
		spot_light_on = True;
		pos = (light != NULL) ? &light->pos : NULL;
		dir = &obj_ptr->dir;
		sdir = (aircraft != NULL) ?
		    &aircraft->spotlight_dir : NULL;

		/* Calculate the gamma for the spot light from the
		 * scene's gamma
		 *
		 * As the scene's gamma gets darker, the spot light
		 * gets brighter
		 */
		g = (float)CLIP(1.0 - scene_gamma, 0.0, 1.0);
	    }
	}

	/* Got spot light */
	if(spot_light_on && (pos != NULL) && (dir != NULL) && (sdir != NULL))
	{
	    StateGLBoolean	lighting = state->lighting,
				depth_test = state->depth_test,
				depth_mask_flag = state->depth_mask_flag,
				polygon_offset_fill = state->polygon_offset_fill;
	    sar_position_struct	light_pos,
				light_pos2,
				light_vect;
	    float ground_elevation, light_z, radius;
	    double a[3 * 1], r[3 * 1];


	    /* Generate a unit vector describing the final direction
	     * of the spot light, start with a unit vector pointing
	     * forwards
	     */
	    a[0] = 0.0;
	    a[1] = 1.0;
	    a[2] = 0.0;

	    /* Rotate to the spot light's direction */
	    MatrixRotateBank3(a, -sdir->bank, r);	/* Our bank is negative */
	    MatrixRotatePitch3(r, sdir->pitch, a);
	    MatrixRotateHeading3(a, sdir->heading, r);

	    /* Rotate relative to object to obtain the rotated unit 
	     * vector in world direction
	     */
	    MatrixRotateBank3(r, -dir->bank, a);	/* Our bank is negative */
	    MatrixRotatePitch3(a, dir->pitch, r);
	    MatrixRotateHeading3(r, dir->heading, a);

	    /* At this point we have a unit vector that describes the
	     * spot light direction in world direction, now copy it to
	     * light_vect
	     */
	    light_vect.x = (float)a[0];
	    light_vect.y = (float)a[1];
	    light_vect.z = (float)a[2];


	    /* Begin calculating the spot light position in world
	     * coordinates and rotate it about the object to obtain the
	     * final light position in world coordinates
	     */
	    a[0] = pos->x;
	    a[1] = pos->y;
	    a[2] = pos->z;

	    MatrixRotateBank3(a, -dir->bank, r);	/* Our bank is negative */
	    MatrixRotatePitch3(r, dir->pitch, a);
	    MatrixRotateHeading3(a, dir->heading, r);

	    /* At this point we have the position of the spot light
	     * source in world coordinates, now copy it to light_pos
	     */
	    light_pos.x = (float)(obj_ptr->pos.x + r[0]);
	    light_pos.y = (float)(obj_ptr->pos.y + r[1]);
	    light_pos.z = (float)(obj_ptr->pos.z + r[2]);


	    /* Calculate ground elevation and the distance from the
	     * spot light source to the ground
	     */
	    ground_elevation = obj_ptr->pos.z +
		((aircraft != NULL) ? aircraft->center_to_ground_height : 0.0f);
	    light_z = light_pos.z - ground_elevation;

	    /* Calculate spot light cast position as light_pos2 */
	    light_pos2.x = light_pos.x + ((light_vect.z != 0.0f) ?
		-(light_vect.x / light_vect.z) * light_z : light_z
	    );
	    light_pos2.y = light_pos.y + ((light_vect.z != 0.0f) ?
		-(light_vect.y / light_vect.z) * light_z : light_z
	    );
	    /* Need to adjust the final spot light position's z
	     * coordinate incase its height above ground is not the same
	     * as the object's height above ground
	     */
	    light_pos2.z = light_pos.z;
	    light_pos2.z = SARSimFindGround(
		scene, dc->object, dc->total_objects,
		&light_pos2
	    );
	    /* Update the spot light source to the ground distance */
	    light_z = light_pos.z - light_pos2.z;

	    /* Distance between light and ground less than 2000 feet
	     * (600 meters) and light vector is pointing down?
	     */
	    if((light_z >= 0.0f) && (light_z < 600.0f) &&
	       (light_vect.z < 0.0f)
	    )
	    {
		int tex_num = scene->texnum_spotlightcast;
		v3d_texture_ref_struct *t = SARIsTextureAllocated(scene, tex_num) ?
		    scene->texture_ref[tex_num] : NULL;
		float z_coeff = light_z / 600.0f;

		/* Adjust gamma based on distance of light to surface */
		g *= 1.0f - z_coeff;

		/* Calculate the spot light cast radius based on the
		 * distance of light to the surface
		 *
		 * This radius will be largest when the source is
		 * farthest away from the surface
		 */
		radius = 25.0f + (z_coeff * 140.0f);

		/* Set up GL states */
		if(lighting)
		    StateGLDisable(state, GL_LIGHTING);
		StateGLEnable(state, GL_BLEND);
		StateGLBlendFunc(state, GL_ONE, GL_ONE);
		StateGLDisable(state, GL_ALPHA_TEST);
		StateGLEnable(state, GL_DEPTH_TEST);
		StateGLDepthFunc(state, GL_LEQUAL);
		StateGLDepthMask(state, GL_FALSE);
		StateGLEnable(state, GL_POLYGON_OFFSET_FILL);
		StateGLPolygonOffset(
		    state,
		    (GLfloat)opt->gl_polygon_offset_factor, -1.0f
		);

		glColor4f(g, g, g, 1.0f);
		V3DTextureSelect(t);

		glBegin(GL_QUADS);
		{
		    glNormal3f(0.0f, 1.0f, 0.0f);
		    glTexCoord2f(0.0f, 1.0f - 0.0f);
		    glVertex3f(
			light_pos2.x - radius,
			light_pos2.z,
			-(light_pos2.y + radius)
		    );
		    glTexCoord2f(0.0f, 1.0f - 1.0f);
		    glVertex3f(
			light_pos2.x - radius,
			light_pos2.z,
			-(light_pos2.y - radius)
		    );
		    glTexCoord2f(1.0f, 1.0f - 1.0f);
		    glVertex3f(
		        light_pos2.x + radius,
			light_pos2.z,
			-(light_pos2.y - radius)
		    );
		    glTexCoord2f(1.0f, 1.0f - 0.0f);
		    glVertex3f(
			light_pos2.x + radius,
			light_pos2.z,
			-(light_pos2.y + radius)
		    );
		}
		glEnd();

		/* Restore GL states */
		if(lighting)
		    StateGLEnable(state, GL_LIGHTING);
		StateGLDisable(state, GL_BLEND);
		StateGLEnable(state, GL_ALPHA_TEST);
		if(!depth_test)
		    StateGLDisable(state, GL_DEPTH_TEST);
		if(depth_mask_flag)
		    StateGLDepthMask(state, GL_TRUE);
		if(!polygon_offset_fill)
		    StateGLDisable(state, GL_POLYGON_OFFSET_FILL);
	    }
	}
}

/*
 *      Draws the object part relative to the given object.
 *
 *      Inputs assumed valid.
 */
static void SARDrawPart(
	sar_dc_struct *dc,
	sar_object_struct *obj_ptr, const sar_obj_part_struct *part
)
{
	gw_display_struct *display = dc->display;
	state_gl_struct *state = &display->state_gl;
	sar_obj_flags_t flags = part->flags;
	const sar_position_struct	*pos_min = &part->pos_min,
					*pos_cen = &part->pos_cen,
					*pos_max = &part->pos_max;
	const sar_direction_struct	*dir_min = &part->dir_min,
					*dir_cen = &part->dir_cen,
					*dir_max = &part->dir_max;
	float anim_coeff, dx, dy, dz;
	float	theta,
		heading_mod = 0.0f,
		pitch_mod = 0.0f,
		bank_mod = 0.0f;
	sar_visual_model_struct *vmodel; 
	sar_object_aircraft_struct *aircraft = SAR_OBJ_GET_AIRCRAFT(obj_ptr);

	if(dc->flir)
	    vmodel = (part->visual_model_ir != NULL) ?
		part->visual_model_ir : part->visual_model;
	else
	    vmodel = part->visual_model;
	if(vmodel == NULL)
	    return;

	/* Set FLIR temperature color? */
	if(dc->flir && (part->visual_model_ir != NULL))
	{
	    StateGLDisable(state, GL_LIGHTING);
	    SARDrawSetColorFLIRTemperature(part->temperature);
	}

	/* Calculate the part's animation coefficent */
	anim_coeff = SAR_GRAD_ANIM_COEFF(part->anim_pos);

	/* Calculate delta positions from max to min */
	dx = pos_max->x - pos_min->x;
	dy = pos_max->y - pos_min->y;
	dz = pos_max->z - pos_min->z;

	/* Translation should be at object's center position */
	glPushMatrix();
	{
	    /* Begin drawing by part type */
	    switch(part->type)
	    {
#define DO_DRAW_CONTROL_PART {			\
 glTranslatef(					\
  pos_cen->x, pos_cen->z, -pos_cen->y		\
 );						\
 theta = dir_cen->heading + heading_mod;	\
 if(theta != 0.0f)				\
  glRotatef(					\
   (GLfloat)-SFMRadiansToDegrees(theta),	\
   0.0f, 1.0f, 0.0f				\
  );						\
 theta = dir_cen->pitch + pitch_mod;		\
 if(theta != 0.0f)				\
  glRotatef(					\
   (GLfloat)-SFMRadiansToDegrees(theta),	\
   1.0f, 0.0f, 0.0f				\
  );						\
 theta = dir_cen->bank + bank_mod;		\
 if(theta != 0.0f)				\
  glRotatef(					\
   (GLfloat)-SFMRadiansToDegrees(theta),	\
   0.0f, 0.0f, 1.0f				\
  );						\
 SARVisualModelCallList(vmodel);		\
}

	      case SAR_OBJ_PART_TYPE_AILERON_LEFT:
		if(aircraft != NULL)
		    pitch_mod = (float)(
			aircraft->control_bank *
			-(dir_max->pitch - dir_min->pitch)
		    );
		DO_DRAW_CONTROL_PART
		break;

	      case SAR_OBJ_PART_TYPE_AILERON_RIGHT:
		if(aircraft != NULL)
		    pitch_mod = (float)(
			aircraft->control_bank *
			(dir_max->pitch - dir_min->pitch)
		    );
		DO_DRAW_CONTROL_PART
		break;

	      case SAR_OBJ_PART_TYPE_RUDDER_TOP:
		if(aircraft != NULL)
		    heading_mod = (float)(
			aircraft->control_heading *
			-(dir_max->heading - dir_min->heading)
		    );
		DO_DRAW_CONTROL_PART
		break;

	      case SAR_OBJ_PART_TYPE_RUDDER_BOTTOM:
		if(aircraft != NULL)
		    heading_mod = (float)(
			aircraft->control_heading *
			(dir_max->heading - dir_min->heading)
		    );
		DO_DRAW_CONTROL_PART
		break;

	      case SAR_OBJ_PART_TYPE_ELEVATOR:
		if(aircraft != NULL)
		    pitch_mod = (float)(
			(aircraft->control_pitch +
			aircraft->elevator_trim) *
			-(dir_max->pitch - dir_min->pitch)
		    );
		DO_DRAW_CONTROL_PART
		break;

	      case SAR_OBJ_PART_TYPE_CANNARD:
		if(aircraft != NULL)
		    pitch_mod = (float)(
			(aircraft->control_pitch +
			aircraft->elevator_trim) *
			(dir_max->pitch - dir_min->pitch)
		    );
		DO_DRAW_CONTROL_PART
		break;

	      case SAR_OBJ_PART_TYPE_AILERON_ELEVATOR_LEFT:
		if(aircraft != NULL)
		    pitch_mod = (float)(
			((aircraft->control_pitch +
			aircraft->elevator_trim) *
			-((dir_max->pitch - dir_min->pitch) / 2)) +
			(aircraft->control_bank *
			-((dir_max->pitch - dir_min->pitch) / 2))
		    );
		DO_DRAW_CONTROL_PART
		break;

	      case SAR_OBJ_PART_TYPE_AILERON_ELEVATOR_RIGHT:
		if(aircraft != NULL)
		    pitch_mod = (float)(
			((aircraft->control_pitch +
			aircraft->elevator_trim) *
			-((dir_max->pitch - dir_min->pitch) / 2)) +
			(aircraft->control_bank *
			((dir_max->pitch - dir_min->pitch) / 2))
		    );
		DO_DRAW_CONTROL_PART
		break;

	      case SAR_OBJ_PART_TYPE_FLAP:
		if(aircraft != NULL)
		    pitch_mod = (float)(
			aircraft->flaps_position *
			(dir_max->pitch - dir_min->pitch)
		    );
		DO_DRAW_CONTROL_PART
		break;
#undef DO_DRAW_CONTROL_PART

	      case SAR_OBJ_PART_TYPE_AIR_BRAKE:
		if((flags & SAR_OBJ_PART_FLAG_HIDE_MIN) &&
		   (part->anim_pos == 0)
		)
		    break;
		if((flags & SAR_OBJ_PART_FLAG_HIDE_MAX) &&
		   (part->anim_pos == (sar_grad_anim_t)-1)
		)
		    break;

		/* Set offset from center of object */
		glTranslatef(
		    pos_min->x + (dx * anim_coeff),
		    pos_min->z + (dz * anim_coeff),
		    -(pos_min->y + (dy * anim_coeff))
		);

		/* Set deployed direction, the minimum direction
		 * indicates the initial (undeployed) direction, the
		 * maximum direction is a *delta* direction to be
		 * added to the minimum direction
		 */
		theta = dir_min->heading + (dir_max->heading * anim_coeff);
		if(theta != 0.0f)
		    glRotatef(
			(GLfloat)-SFMRadiansToDegrees(theta),
			0.0f, 1.0f, 0.0f
		    );
		theta = dir_min->pitch + (dir_max->pitch * anim_coeff);
		if(theta != 0.0f)
		    glRotatef(
			(GLfloat)-SFMRadiansToDegrees(theta),
			1.0f, 0.0f, 0.0f
		    );
		theta = dir_min->bank + (dir_max->bank * anim_coeff);
		if(theta != 0.0f)
		    glRotatef(
			(GLfloat)-SFMRadiansToDegrees(theta),
			0.0f, 0.0f, 1.0f
		    );

		/* Draw visual model */
		SARVisualModelCallList(vmodel);
		break;

	      case SAR_OBJ_PART_TYPE_DOOR:
	      case SAR_OBJ_PART_TYPE_DOOR_RESCUE:
	      case SAR_OBJ_PART_TYPE_CANOPY:
		if((flags & SAR_OBJ_PART_FLAG_HIDE_MIN) &&
		   (part->anim_pos == 0)
		)
		    break;
		if((flags & SAR_OBJ_PART_FLAG_HIDE_MAX) &&
		   (part->anim_pos == (sar_grad_anim_t)-1)
		)
		    break;

		/* Set offset from center of object */
		glTranslatef(
		    pos_min->x + (dx * anim_coeff),
		    pos_min->z + (dz * anim_coeff),
		    -(pos_min->y + (dy * anim_coeff))
		);

		/* Set deployed direction, the minimum direction
		 * indicates the initial (undeployed) direction, the
		 * maximum direction is a *delta* direction to be
		 * added to the minimum direction
		 */
		theta = dir_min->heading + (dir_max->heading * anim_coeff);
		if(theta != 0.0f)
		    glRotatef(
			(GLfloat)-SFMRadiansToDegrees(theta),
			0.0f, 1.0f, 0.0f
		    );
		theta = dir_min->pitch + (dir_max->pitch * anim_coeff);
		if(theta != 0.0f)
		    glRotatef(
			(GLfloat)-SFMRadiansToDegrees(theta),
			1.0f, 0.0f, 0.0f
		    );
		theta = dir_min->bank + (dir_max->bank * anim_coeff);
		if(theta != 0.0f)
		    glRotatef(
			(GLfloat)-SFMRadiansToDegrees(theta),
			0.0f, 0.0f, 1.0f
		    );

		/* Draw visual model */
		SARVisualModelCallList(vmodel);
		break;

	      case SAR_OBJ_PART_TYPE_LANDING_GEAR:
		if((flags & SAR_OBJ_PART_FLAG_HIDE_MIN) &&
		   (part->anim_pos == 0)
		)
		    break;
		if((flags & SAR_OBJ_PART_FLAG_HIDE_MAX) &&
		   (part->anim_pos == (sar_grad_anim_t)-1)
		)
		    break;

		/* anim_coeff needs to be flipped for landing gears */
		anim_coeff = 1.0f - anim_coeff;

		/* Set offset from center of object */
		glTranslatef(
		    pos_min->x + (dx * anim_coeff),
		    pos_min->z + (dz * anim_coeff),
		    -(pos_min->y + (dy * anim_coeff))
		);

		/* Set deployed direction, the minimum direction
		 * indicates the initial (undeployed) direction, the
		 * maximum direction is a *delta* direction to be
		 * added to the minimum direction
		 */
		theta = dir_min->heading + (dir_max->heading * anim_coeff);
		if(theta != 0.0f)
		    glRotatef(
			(GLfloat)-SFMRadiansToDegrees(theta),
			0.0f, 1.0f, 0.0f
		    );
		theta = dir_min->pitch + (dir_max->pitch * anim_coeff);
		if(theta != 0.0f)
		    glRotatef(
			(GLfloat)-SFMRadiansToDegrees(theta),
			1.0f, 0.0f, 0.0f
		    );
		theta = dir_min->bank + (dir_max->bank * anim_coeff);
		if(theta != 0.0f)
		    glRotatef(
			(GLfloat)-SFMRadiansToDegrees(theta),
			0.0f, 0.0f, 1.0f
		    );

		/* Draw visual model */
		SARVisualModelCallList(vmodel);
		break;

/* Add support for drawing of other part types here */

	    }
	}
	glPopMatrix();

	if(dc->flir && (part->visual_model_ir != NULL))
	    StateGLEnable(state, GL_LIGHTING);
}

/*
 *      Draws the rotor/propellar relative to the given object.
 *
 *      Inputs assumed valid.
 */
static void SARDrawRotor(
	sar_dc_struct *dc,
	sar_object_struct *obj_ptr, sar_obj_rotor_struct *rotor,
	float throttle
)
{
	gw_display_struct *display = dc->display;
	state_gl_struct *state = &display->state_gl;
	sar_scene_struct *scene = dc->scene;
	StateGLBoolean lighting = state->lighting;
	const sar_option_struct *opt = dc->option;
	sar_rotor_flags flags = rotor->flags;
	float pitch_coeff;
	float anim_coeff;
	sar_visual_model_struct *vmodel = (dc->flir && (rotor->visual_model_ir != NULL)) ?
	    rotor->visual_model_ir : rotor->visual_model;
	if(vmodel == NULL)
	    return;

	/* Calculate the rotor pitch coefficient, 0.0 for upwards and
	 * 1.0 for forwards
	 */
	pitch_coeff = (flags & SAR_ROTOR_FLAG_CAN_PITCH) ?
	    SAR_GRAD_ANIM_COEFF(rotor->pitch_anim_pos) : 0.0f;

	/* Calculate the rotor spin animation coefficient */
	anim_coeff = (flags & SAR_ROTOR_FLAG_SPINS) ?
	    SAR_GRAD_ANIM_COEFF(rotor->anim_pos) : 0.0f;

	/* Set FLIR temperature color? */
	if(dc->flir && (rotor->visual_model_ir != NULL))
	{
	    StateGLDisable(state, GL_LIGHTING);
	    SARDrawSetColorFLIRTemperature(obj_ptr->temperature);
	}

	/* Set matrix to the center of the rotor
	 *
	 * Current transformation assumed at object's center
	 */
	glPushMatrix();
	{
	    Boolean draw_blured;
	    sar_position_struct *pos = &rotor->pos;
	    sar_direction_struct *dir = &rotor->dir;

	    /* Set offset from center of object */
	    glTranslatef(pos->x, pos->z, -pos->y);

	    /* Rotate */
	    if(dir->heading != 0)
		glRotatef(
		    (GLfloat)-SFMRadiansToDegrees(dir->heading),
		    0.0f, 1.0f, 0.0f
		);
	    glRotatef(
		(GLfloat)-SFMRadiansToDegrees(
		    dir->pitch +
		    (0.5 * PI * pitch_coeff) +
		    ((flags & SAR_ROTOR_FLAG_FOLLOW_CONTROLS) ?
	      rotor->control_coeff_pitch * (5.0 * PI / 180.0) : 0.0
		    )
		),
		1.0f, 0.0f, 0.0f
	    );
	    glRotatef(
		(GLfloat)-SFMRadiansToDegrees(
		    dir->bank +
		    ((flags & SAR_ROTOR_FLAG_FOLLOW_CONTROLS) ?
	      rotor->control_coeff_bank * (5.0 * PI / 180.0) : 0.0
		    )
		),
		0.0f, 0.0f, 1.0f
	    );

	    /* Always draw blured rotor blades? */
	    if(flags & SAR_ROTOR_FLAG_BLUR_ALWAYS)
	    {
		draw_blured = True;
	    }
	    /* Draw blured rotor blades when fast? */
	    else if(flags & SAR_ROTOR_FLAG_BLUR_WHEN_FAST)
	    {
		/* Throttle above 32%? */
/* Use a global setting for this? */
		if(throttle > 0.32f)
		    draw_blured = True;
		else
		    draw_blured = False;
	    }
	    else
	    {
		draw_blured = False;
	    }

	    /* Check whether to draw from the rotor's visual model or
	     * blured blades
	     */
	    if(draw_blured)
	    {
		/* Draw disc representing blured rotors */
		GLenum shade_model_mode = state->shade_model_mode;
		GLboolean depth_mask_flag = state->depth_mask_flag;
		sar_color_struct *c = &rotor->blades_blur_color;

		/* Set up GL states */
		StateGLShadeModel(state, GL_FLAT);
		StateGLDepthMask(state, GL_FALSE);
		StateGLDisable(state, GL_ALPHA_TEST);
		StateGLEnable(state, GL_BLEND);
		StateGLBlendFunc(
		    state,
		    GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA
		);

		/* Set blured blades color */
		if(dc->flir)
		{
		    float g = CLIP(
			throttle * 0.4f,
			0.0f,
			0.4f 
		    );
		    glColor4f(
			CLIP((g - 0.5f) * 2.0f, 0.0f, 1.0f),
			CLIP(g * 2.0f, 0.0f, 1.0f),
			CLIP((g - 0.5f) * 2.0f, 0.0f, 1.0f),
		        c->a
		    );
		}
		else
		    glColor4f(c->r, c->g, c->b, c->a);

		/* Draw blured blades by style */
		switch(opt->rotor_blur_style)
		{
		  case SAR_ROTOR_BLUR_CLOCK_RADIAL:
		    /* Draw blured blades as indivdual textured blades */
		    if(rotor->total_blades > 0)
		    {
			const int nblades = rotor->total_blades;
			const int tex_num = rotor->blade_blur_tex_num;
			const float	r = rotor->radius,
					blades_offset = rotor->blades_offset,
					dtheta = (float)(2.0 * PI) /
					    (float)nblades,
					spin_theta = (float)(
					    -anim_coeff * (2.0 * PI)
					);
			const StateGLBoolean texture_2d = state->texture_2d;
			v3d_texture_ref_struct *t =
			    SARIsTextureAllocated(scene, tex_num) ?
				scene->texture_ref[tex_num] : NULL;
			int i;
			float theta;

			/* Set up GL states */
			SAR_DRAW_TEXTURE_2D_ON

			/* Select blured blades texture */
			V3DTextureSelect(t);

			/* Draw each blured blade */
			for(i = 0,
			    theta = (float)(0.0 * PI);
			    i < nblades;
			    i++, theta += dtheta
			)
			{
			    glPushMatrix();
			    /* Rotate counter clockwise along Z axis to
			     * to simulate rotor spinning
			     */
			    glRotatef(
				(GLfloat)-SFMRadiansToDegrees(
				    spin_theta + theta
				),
				0.0f, 1.0f, 0.0f
			    );

			    /* Top */
			    glBegin(GL_POLYGON);
			    {
				float tc, theta;

				glNormal3f(0.0f, 1.0f, 0.0f);
				glTexCoord2f(0.0f, 1.0f - 0.0f);
				glVertex3f(
				    (GLfloat)(r * sin(-0.2 * PI)),
				    blades_offset,
				    0.0f
				);
				glTexCoord2f(1.0f, 1.0f - 0.0f);
				glVertex3f(
				    (GLfloat)(r * sin(0.2 * PI)),
				    blades_offset,
				    0.0f
				);
				for(tc = 1.0f, theta = 0.2f;
				    theta >= -0.2f;
				    tc -= 1.0f / 4.0f, theta -= 0.1f
				)
 				{
				    glTexCoord2f(tc, 1.0f - 1.0f);
				    glVertex3f(
					(GLfloat)(r * sin(theta * PI)),
					blades_offset,                 
					(GLfloat)-(r * cos(theta * PI))
				    );
				}
			    }
			    glEnd();

			    /* Bottom */
			    glBegin(GL_POLYGON);
			    {
				float tc, theta;

				glNormal3f(0.0f, -1.0f, 0.0f);
				glTexCoord2f(1.0f, 1.0f - 0.0f);
				glVertex3f(
				    (GLfloat)(r * sin(0.2 * PI)),
				    blades_offset,
				    0.0f
				);
				glTexCoord2f(0.0f, 1.0f - 0.0f);
				glVertex3f(
				    (GLfloat)(r * sin(-0.2 * PI)), 
				    blades_offset, 
				    0.0f
				);
				for(tc = 0.0f, theta = -0.2f;
				    theta <= 0.2f;
				    tc += 1.0f / 4.0f, theta += 0.1f
				)
				{
				    glTexCoord2f(tc, 1.0f - 1.0f);
				    glVertex3f(      
					(GLfloat)(r * sin(theta * PI)),
					blades_offset,                 
					(GLfloat)-(r * cos(theta * PI))
				    );
				}
			    }
			    glEnd();

			    glPopMatrix();
			}

			/* Restore GL texture states */
			V3DTextureSelect(NULL);
			if(!texture_2d)
			    SAR_DRAW_TEXTURE_2D_OFF
		    }
		    break;

		  case SAR_ROTOR_BLUR_SOLID:
		    /* Draw blured blades as a solid disk */
		    /* Draw top face */
		    glBegin(GL_POLYGON);
		    {
			const float	r = rotor->radius,
					blades_offset = rotor->blades_offset;
			float theta;

			glNormal3f(0.0f, 1.0f, 0.0f);
			for(theta = 2.0f; theta > 0.0f; theta -= 0.1f)
			{
#if 0
			    glTexCoord2f(
			(GLfloat)((sin(theta * PI) + 1.0) / 2.0),
			(GLfloat)((cos(theta * PI) + 1.0) / 2.0)
			    );
#endif
			    glVertex3f(
				(GLfloat)(r * sin(theta * PI)),
				blades_offset,
				(GLfloat)-(r * cos(theta * PI))
			    );
			}
		    }
		    glEnd();
		    /* Draw bottom face */
		    glBegin(GL_POLYGON);
		    {
			const float	r = rotor->radius,
					blades_offset = rotor->blades_offset;
			float theta;

			glNormal3f(0.0f, -1.0f, 0.0f);
			for(theta = 0.0f; theta < 2.0f; theta += 0.1f)
			    glVertex3f(
				(GLfloat)(r * sin(theta * PI)),
				blades_offset,
				(GLfloat)-(r * cos(theta * PI))
			    );
		    }
		    glEnd();
		    break;
		}

		/* Restore GL states */
		StateGLShadeModel(state, shade_model_mode);
		StateGLDepthMask(state, depth_mask_flag);
		StateGLEnable(state, GL_ALPHA_TEST);
		StateGLDisable(state, GL_BLEND);
	    }
	    else
	    {
		glPushMatrix();
		{
		    /* Rotate counter clockwise along Z axis to 
		     * simulate rotor spinning
		     */
		    glRotatef(
			(GLfloat)-SFMRadiansToDegrees(
			    -anim_coeff * (2.0 * PI)
			),
			0.0f, 1.0f, 0.0f
		    );  
		    SARVisualModelCallList(vmodel);
		}
		glPopMatrix();
	    }
	}
	glPopMatrix();

	/* Restore GL states */
	if(lighting)
	    StateGLEnable(state, GL_LIGHTING);
}

/*
 *	External reserved fuel tank which is assumed to be associated with
 *	the object.
 *
 *	Do not confuse this with falling fuel tanks which are drawn by
 *	SARDrawObjectFuelTank.
 *
 *	All inputs assumed valid.
 */
static void SARDrawFuelTank(
	sar_dc_struct *dc,
	sar_object_struct *obj_ptr,
	sar_external_fueltank_struct *eft_ptr
)
{
	gw_display_struct *display = dc->display;
	state_gl_struct *state = &display->state_gl;
	sar_position_struct *offset_pos = &eft_ptr->offset_pos;
	sar_visual_model_struct *vmodel;

	if(dc->flir)
	    vmodel = (eft_ptr->visual_model_ir != NULL) ?
		eft_ptr->visual_model_ir : eft_ptr->visual_model;
	else
	    vmodel = eft_ptr->visual_model;
	if(vmodel == NULL)
	    return;

	/* Fuel tank already dropped? */
	if(!(eft_ptr->flags & SAR_EXTERNAL_FUELTANK_FLAG_ONBOARD))
	    return;

	/* Set FLIR temperature color? */
	if(dc->flir && (eft_ptr->visual_model_ir != NULL))
	{
	    StateGLDisable(state, GL_LIGHTING);
	    SARDrawSetColorFLIRTemperature(eft_ptr->temperature);
	}

	glPushMatrix();
	{
	    glTranslatef(
		offset_pos->x, offset_pos->z, -offset_pos->y
	    );
	    SARVisualModelCallList(vmodel);
	}
	glPopMatrix();

	if(dc->flir && (eft_ptr->visual_model_ir != NULL))
	    StateGLEnable(state, GL_LIGHTING);
}

/*
 *	Draws the hoist deployment (rescue basket or diver) and the
 *	tope that leads up to the origin of the hoist.
 *
 *	Translation matrix should already be at location of the hoist
 *	deployment (end of rope).
 *
 *	All inputs assumed valid.
 */
static void SARDrawHoistDeployment(
	sar_dc_struct *dc,
	sar_object_struct *obj_ptr, const sar_obj_hoist_struct *hoist
)
{
	gw_display_struct *display = dc->display;
	sar_scene_struct *scene = dc->scene;
	float l, w, h, hl, hw, hh;
	float rope_len = hoist->rope_cur_vis;


	/* Get length as the diameter of the contact bounds */
	l = hoist->contact_radius * 2.0f;

	/* Get width as half of the length */
	w = l / 2;

	/* Height of the basket is the upper z cylendrical contact
	 * bounds
	 */
	h = hoist->contact_z_max;

	/* Calculate half dimensions */
	hl = l / 2.0f;
	hw = w / 2.0f;
	hh = h / 2.0f;



	/* Draw by hoist deployment */

	/* Hook? */
	if(hoist->cur_deployment == SAR_HOIST_DEPLOYMENT_HOOK)
	{
	    /* Draw rope leading back up to the hoist */
	    V3DTextureSelect(NULL);
	    glColor4f(0.9f, 0.9f, 0.9f, 1.0f);
	    glBegin(GL_LINES);
	    {
		glNormal3f(0.0f, 1.0f, 0.0f);
		glVertex3f(0.0f, 0.3f, 0.0f);
		glVertex3f(0.0f, rope_len, 0.0f);
	    }
	    glEnd();

	    /* Begin drawing hook */
	    glPushMatrix();
	    {
		glTranslatef(0.0f, 0.3f, 0.0f);
		glColor4f(0.5f, 0.5f, 0.5f, 1.0f);
		glBegin(GL_QUADS);
		{
		    SARDrawBoxBaseNS(
			0.3f, 0.3f, 0.3f, False
		    );
		}
		glEnd();
	    }
	    glPopMatrix();

	    glPushMatrix();
	    {
		glColor4f(0.2f, 0.2f, 0.2f, 1.0f);
		glBegin(GL_QUADS);
		{
		    SARDrawBoxBaseNS(
			0.8f, 0.8f, 0.3f, True
		    );
		}
		glEnd();
	    }
	    glPopMatrix();
	}
	/* Diver? */
	else if(hoist->cur_deployment == SAR_HOIST_DEPLOYMENT_DIVER)
	{
	    /* Draw rope leading back up to the hoist */
	    V3DTextureSelect(NULL);
	    glColor4f(0.9f, 0.9f, 0.9f, 1.0f);
	    glBegin(GL_LINES);
	    {
		glNormal3f(0.0f, 1.0f, 0.0f);
		glVertex3f(0.0f, h - 1.0f, 0.0f);
		glVertex3f(0.0f, rope_len, 0.0f);
	    }
	    glEnd();

	    /* Begin drawing diver */
	    glPushMatrix();
	    {
		/* Translate back just a bit so drawn human is behind
		 * the rope (and victim)
		 */
		glTranslatef(0.0f, -1.0f, 0.25f);

		SARDrawHumanIterate(
		    dc,
		    1.9f,		/* Height in meters */
		    54.0f,		/* Mass in kg */
		    SAR_HUMAN_FLAG_ALERT | SAR_HUMAN_FLAG_AWARE |
		    SAR_HUMAN_FLAG_GRIPPED | SAR_HUMAN_FLAG_DIVER_CATCHER,
		    hoist->diver_color,
		    hoist->water_ripple_tex_num,
		    hoist->anim_pos
		);
	    }
	    glPopMatrix();
	}
	/* Basket? */
	else if(hoist->cur_deployment == SAR_HOIST_DEPLOYMENT_BASKET)
	{
	    int tex_num;
	    GLenum shade_model_mode = display->state_gl.shade_model_mode;
	    v3d_texture_ref_struct *side_tex, *end_tex, *bottom_tex;

	    /* Get pointers to textures */
	    tex_num = hoist->side_tex_num;
	    if(SARIsTextureAllocated(scene, tex_num))
		side_tex = scene->texture_ref[tex_num];
	    else
		side_tex = NULL;

	    tex_num = hoist->end_tex_num;
	    if(SARIsTextureAllocated(scene, tex_num))
		end_tex = scene->texture_ref[tex_num];
	    else
		end_tex = NULL;

	    tex_num = hoist->bottom_tex_num;
	    if(SARIsTextureAllocated(scene, tex_num))
		bottom_tex = scene->texture_ref[tex_num];
	    else
		bottom_tex = NULL;

	    /* Set up GL states */
	    StateGLShadeModel(&display->state_gl, GL_SMOOTH);

	    /* Begin drawing rescue basket */

	    /* Draw bottom base */
	    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	    V3DTextureSelect(bottom_tex);
	    glBegin(GL_QUADS);
	    {
		/* Upward facing */
		glNormal3f(-0.33f, 0.88f, 0.33f);
		glTexCoord2f(0.0f, 1.0f - 0.0f);
		glVertex3f(-hl, 0.0f,  hw);

		glNormal3f(0.33f, 0.88f, 0.33f);
		glTexCoord2f(1.0f, 1.0f - 0.0f);
		glVertex3f(hl,  0.0f,  hw);

		glNormal3f(0.33f, 0.88f, -0.33f);
		glTexCoord2f(1.0f, 1.0f - 1.0f);
		glVertex3f(hl,  0.0f, -hw);

		glNormal3f(-0.33f, 0.88f, -0.33f);
		glTexCoord2f(0.0f, 1.0f - 1.0f);
		glVertex3f(-hl, 0.0f, -hw);

		/* Downward facing */
		glNormal3f(-0.33f, -0.88f, -0.33f);
		glTexCoord2f(0.0f, 1.0f - 1.0f);
		glVertex3f(-hl, 0.0f, -hw);

		glNormal3f(0.33f, -0.88f, -0.33f);
		glTexCoord2f(1.0f, 1.0f - 1.0f);
		glVertex3f(hl, 0.0f, -hw);

		glNormal3f(0.33f, -0.88f, 0.33f);
		glTexCoord2f(1.0f, 1.0f - 0.0f);
		glVertex3f(hl, 0.0f,  hw);

		glNormal3f(-0.33f, -0.88f, 0.33f);
		glTexCoord2f(0.0f, 1.0f - 0.0f);
		glVertex3f(-hl, 0.0f,  hw);
	    }
	    glEnd();

	    /* Sides */
	    V3DTextureSelect(side_tex);
	    glBegin(GL_QUADS);
	    {
		/* Front forwards facing side */
		glNormal3f(-0.33f, 0.33f, -0.88f);
		glTexCoord2f(1.0f, 1.0f - 1.0f);
		glVertex3f(-hl, hh,  -hw);

		glNormal3f(0.33f, 0.33f, -0.88f);
		glTexCoord2f(0.0f, 1.0f - 1.0f);
		glVertex3f( hl, hh,  -hw);

		glNormal3f(0.33f, -0.33f, -0.88f);
		glTexCoord2f(0.0f, 1.0f - 0.0f);
		glVertex3f( hl, 0.0f, -hw);

		glNormal3f(-0.33f, -0.33f, -0.88f);
		glTexCoord2f(1.0f, 1.0f - 0.0f);
		glVertex3f(-hl, 0.0f, -hw);


		/* Backwards facing side */
		glNormal3f(-0.33f, -0.33f, 0.88f);
		glTexCoord2f(1.0f, 1.0f - 0.0f);
		glVertex3f(-hl, 0.0f, -hw);

		glNormal3f(0.33f, -0.33f, 0.88f);
		glTexCoord2f(0.0f, 1.0f - 0.0f);
		glVertex3f( hl, 0.0f, -hw);

		glNormal3f(0.33f, 0.33f, 0.88f);
		glTexCoord2f(0.0f, 1.0f - 1.0f);
		glVertex3f( hl, hh, -hw);

		glNormal3f(-0.33f, 0.33f, 0.88f);
	        glTexCoord2f(1.0f, 1.0f - 1.0f);
		glVertex3f(-hl, hh, -hw);


		/* Back forwards facing side */
		glNormal3f(-0.33f, 0.33f, -0.88f);
		glTexCoord2f(0.0f, 1.0f - 1.0f);
		glVertex3f(-hl, hh,  hw);

		glNormal3f(0.33f, 0.33f, -0.88f);
		glTexCoord2f(1.0f, 1.0f - 1.0f);
		glVertex3f( hl, hh,  hw);

		glNormal3f(0.33f, -0.33f, -0.88f); 
		glTexCoord2f(1.0f, 1.0f - 0.0f);
		glVertex3f( hl, 0.0f, hw);

		glNormal3f(-0.33f, -0.33f, -0.88f);
		glTexCoord2f(0.0f, 1.0f - 0.0f);
		glVertex3f(-hl, 0.0f, hw);

		/* Back backwards facing side */
		glNormal3f(-0.33f, -0.33f, 0.88f);
		glTexCoord2f(0.0f, 1.0f - 0.0f);
		glVertex3f(-hl, 0.0f, hw);

		glNormal3f(0.33f, -0.33f, 0.88f);
	        glTexCoord2f(1.0f, 1.0f - 0.0f);
		glVertex3f( hl, 0.0f, hw);

		glNormal3f(0.33f, 0.33f, 0.88f);
		glTexCoord2f(1.0f, 1.0f - 1.0f);
		glVertex3f( hl, hh, hw);

		glNormal3f(-0.33f, 0.33f, 0.88f);
		glTexCoord2f(0.0f, 1.0f - 1.0f);
		glVertex3f(-hl, hh, hw);
	    }
	    glEnd();

	    /* Left and right side ends */
	    V3DTextureSelect(end_tex);
	    glBegin(GL_QUADS);
	    {
		/* Left, left facing side */
		glNormal3f(-0.88f, -0.33f, -0.33f);
		glTexCoord2f(0.0f, 1.0f - 0.0f);
		glVertex3f(-hl, 0.0f, -hw);

		glNormal3f(-0.88f, -0.33f, 0.33f);
		glTexCoord2f(1.0f, 1.0f - 0.0f);
		glVertex3f(-hl, 0.0, hw);

		glNormal3f(-0.88f, 0.33f, 0.33f);
		glTexCoord2f(1.0f, 1.0f - 1.0f);
		glVertex3f(-hl, hh, hw);

		glNormal3f(-0.88f, 0.33f, -0.33f);
		glTexCoord2f(0.0f, 1.0f - 1.0f);
		glVertex3f(-hl, hh, -hw);

		/* Left, right facing side */
		glNormal3f(0.88f, 0.33f, -0.33f);
		glTexCoord2f(0.0f, 1.0f - 1.0f);
		glVertex3f(-hl, hh, -hw);

		glNormal3f(0.88f, 0.33f, 0.33f);
		glTexCoord2f(1.0f, 1.0f - 1.0f);
		glVertex3f(-hl, hh, hw);

		glNormal3f(0.88f, -0.33f, 0.33f);
		glTexCoord2f(1.0f, 1.0f - 0.0f);
		glVertex3f(-hl, 0.0f, hw);

		glNormal3f(0.88f, -0.33f, -0.33f);
		glTexCoord2f(0.0f, 1.0f - 0.0f);
		glVertex3f(-hl, 0.0f, -hw);


		/* Right, left facing side */
		glNormal3f(-0.88f, -0.33f, -0.33f);
		glTexCoord2f(0.0f, 1.0f - 0.0f);
		glVertex3f(hl, 0.0f, -hw);

		glNormal3f(-0.88f, -0.33f, 0.33f);
		glTexCoord2f(1.0f, 1.0f - 0.0f);
		glVertex3f(hl, 0.0f, hw);

		glNormal3f(-0.88f, 0.33f, 0.33f);
		glTexCoord2f(1.0f, 1.0f - 1.0f);
		glVertex3f(hl, hh, hw);

		glNormal3f(-0.88f, 0.33f, -0.33f);
		glTexCoord2f(0.0f, 1.0f - 1.0f);
		glVertex3f(hl, hh, -hw);

		/* Right, right facing side */
		glNormal3f(0.88f, 0.33f, -0.33f);
		glTexCoord2f(0.0f, 1.0f - 1.0f);
		glVertex3f(hl, hh, -hw);

		glNormal3f(0.88f, 0.33f, 0.33f);
		glTexCoord2f(1.0f, 1.0f - 1.0f);
		glVertex3f(hl, hh, hw);

		glNormal3f(0.88f, -0.33f, 0.33f);
		glTexCoord2f(1.0f, 1.0f - 0.0f);
		glVertex3f(hl, 0.0f, hw);

		glNormal3f(0.88f, -0.33f, -0.33f);
		glTexCoord2f(0.0f, 1.0f - 0.0f);
		glVertex3f(hl, 0.0f, -hw);
	    }
	    glEnd();

	    /* Other details, floatation strips */
	    V3DTextureSelect(NULL);
	    glColor4f(0.9f, 0.1f, 0.1f, 1.0f);
	    glBegin(GL_QUADS);
	    {
		glNormal3f(0.0f, 1.0f, 0.0f);
		glVertex3f(-hl,        hh, hw);
		glVertex3f(-hl * 0.9f, hh, hw);
		glVertex3f(-hl * 0.9f, hh, -hw);
		glVertex3f(-hl,        hh, -hw);

		glNormal3f(0.0f, 1.0f, 0.0f);
		glVertex3f(hl,        hh, -hw);
		glVertex3f(hl * 0.9f, hh, -hw);
		glVertex3f(hl * 0.9f, hh,  hw);
		glVertex3f(hl,        hh,  hw);
	    }
	    glEnd();

	    /* Begin drawing cables */
	    glBegin(GL_LINES);
	    {
		float hl = l / 2.0f;
		float hw = w / 2.0f;
		float hh = h / 2.0f;

		glColor4f(0.9f, 0.9f, 0.9f, 1.0f);

		glNormal3f(0.0f, 1.0f, 0.0f);
		glVertex3f(hl, hh, hw);
		glVertex3f(0.0f, h, 0.0f);

		glVertex3f(hl, hh, -hw);
		glVertex3f(0.0f, h, 0.0f);

		glVertex3f(-hl, hh, -hw);
		glVertex3f(0.0f, h, 0.0f);

		glVertex3f(-hl, hh, hw);
		glVertex3f(0.0f, h, 0.0f);

		/* Draw rope leading back up to the hoist */
		glVertex3f(0.0f, h, 0.0f);
		glVertex3f(0.0f, rope_len, 0.0f);
	    }
	    glEnd();

	    /* Restore GL states */
	    StateGLShadeModel(&display->state_gl, shade_model_mode);
	}
}

/*
 *	Draws smoke trail object, all translations and rotations will
 *	be handled within this function.
 */
static void SARDrawSmoke(
	sar_dc_struct *dc,
	sar_object_struct *obj_ptr, sar_object_smoke_struct *obj_smoke_ptr
)
{
	gw_display_struct *display = dc->display;
	state_gl_struct *state = &display->state_gl;
	sar_scene_struct *scene = dc->scene;
	const sar_position_struct *cam_pos = &dc->camera_pos;
	int tex_num = obj_smoke_ptr->tex_num;
	v3d_texture_ref_struct *t;
	GLenum shade_model_mode = state->shade_model_mode;
	StateGLBoolean lighting = state->lighting;

	/* Get texture for this smoke trail */
	if(SARIsTextureAllocated(scene, tex_num))
	    t = scene->texture_ref[tex_num];
	else
	    t = NULL;

	/* Select texture (if any) */
	V3DTextureSelect(t);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

	/* Set up GL states */
	switch(obj_smoke_ptr->type)
	{
	  case SAR_SMOKE_TYPE_SMOKE:
	    /* Do not draw smoke in FLIR mode */
	    if(dc->flir)
		return;
	    StateGLShadeModel(state, GL_FLAT);
	    break;
	  case SAR_SMOKE_TYPE_SPARKS:
	    StateGLShadeModel(state, GL_FLAT);
	    StateGLDisable(state, GL_LIGHTING);
	    break;
	  case SAR_SMOKE_TYPE_DEBRIS:
	    /* TODO */
	    StateGLShadeModel(state, GL_FLAT);
	    break;
	}

	/* This smoke object has individual smoke units? */
	if(obj_smoke_ptr->unit != NULL)
	{
	    int i;
	    float r;
	    const sar_object_smoke_unit_struct *u;
	    const sar_position_struct *pos;
	    sar_direction_struct to_dir;
	    const sar_color_struct *c;

	    /* Iterate through each unit */
	    for(i = 0; i < obj_smoke_ptr->total_units; i++)
	    {
		u = &obj_smoke_ptr->unit[i];

		/* Do not draw units that are not visible */
		if(u->visibility <= 0.0f)
		    continue;

		r = u->radius;
		c = &u->color;
		pos = &u->pos;

		/* Draw by smoke type */
		switch(obj_smoke_ptr->type)
		{
		  case SAR_SMOKE_TYPE_SMOKE:
		    glPushMatrix();
		    {
			/* Translate */
			glTranslatef(pos->x, pos->z, -pos->y);

			/* Adjust heading to camera heading */
			SARDrawGetDirFromPos(
			    cam_pos, pos, &to_dir
			);
			if(to_dir.heading != 0)
			    glRotatef(
				(GLfloat)-SFMRadiansToDegrees(to_dir.heading),
				0.0f, 1.0f, 0.0f
			    );
			if(to_dir.pitch != 0)
			    glRotatef(
				(GLfloat)-SFMRadiansToDegrees(to_dir.pitch),
				1.0f, 0.0f, 0.0f
			    );

		        /* Begin drawing smoke puff */
		        glBegin(GL_QUADS);
		        {
			    glNormal3f(0.0f, 1.0f, 0.0f);
			    glTexCoord2f(0.0f, 1.0f - 0.0f);
			    glVertex3f(-r, -r, 0.0f);
			    glTexCoord2f(1.0f, 1.0f - 0.0f);
			    glVertex3f(r, -r, 0.0f);
			    glTexCoord2f(1.0f, 1.0f - 1.0f);
			    glVertex3f(r, r, 0.0f);
			    glTexCoord2f(0.0f, 1.0f - 1.0f);
			    glVertex3f(-r, r, 0.0f);
		        }
		        glEnd();
		    }
		    glPopMatrix();
		    break;

		  case SAR_SMOKE_TYPE_SPARKS:
		    StateGLEnable(state, GL_POINT_SMOOTH);
		    StateGLPointSize(state, 2.0f);
		    if(dc->flir)
		    {
			float g = MAX(MAX(c->r, c->g), c->b);
			glColor4f(g, g, g, c->a);
		    }
		    else
		    {
			glColor4f(c->r, c->g, c->b, c->a);
		    }

		    glPushMatrix();
		    {
			/* Translate */
			glTranslatef(pos->x, pos->z, -pos->y);

			glBegin(GL_POINTS);
			{
			    glNormal3f(0.0f, 1.0f, 0.0f);
			    glVertex3f(0.0f, 0.0f, 0.0f);
			}
			glEnd();
		    }
		    glPopMatrix();
		    StateGLDisable(state, GL_POINT_SMOOTH);
		    StateGLPointSize(state, 1.0f);
		    break;

		  case SAR_SMOKE_TYPE_DEBRIS:
		    /* TODO */
		    break;
		}	/* Draw by smoke trail type */
	    }	/* Iterate through each unit */
	}

	/* Restore GL states */ 
	StateGLShadeModel(state, shade_model_mode);
	if(lighting)
	    StateGLEnable(state, GL_LIGHTING);
}

/*
 *	Draws explosion (and splash) object.
 */
static void SARDrawExplosion(
	sar_dc_struct *dc,
	sar_object_struct *obj_ptr,
	sar_object_explosion_struct *obj_explosion_ptr
)
{
	gw_display_struct *display = dc->display;
	state_gl_struct *state = &display->state_gl;
	sar_scene_struct *scene = dc->scene;
	int	cur_frame = obj_explosion_ptr->cur_frame,
		tex_num = dc->flir ?
		    obj_explosion_ptr->ir_tex_num : obj_explosion_ptr->tex_num;
	StateGLBoolean lighting = state->lighting;
	GLenum shade_model_mode = state->shade_model_mode;
	v3d_texture_ref_struct *t;
	float r = obj_explosion_ptr->radius;

	/* Get explosion or splash texture */
	if(SARIsTextureAllocated(scene, tex_num))
	    t = scene->texture_ref[tex_num];
	else
	    return;

	/* Sanitize frame number */
	if(cur_frame >= t->total_frames)
	    cur_frame = t->total_frames - 1;
	if(cur_frame < 0)
	    cur_frame = 0;

	/* Select texture */
	V3DTextureSelectFrame(t, cur_frame);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

	/* Set up GL states */
	switch(obj_explosion_ptr->color_emission)
	{
	  case SAR_EXPLOSION_COLOR_EMISSION_EMIT_LIGHT:
	  case SAR_EXPLOSION_COLOR_EMISSION_IS_LIGHT:
	    StateGLShadeModel(state, GL_FLAT);
	    StateGLDisable(state, GL_LIGHTING);
	    break;
	  case SAR_EXPLOSION_COLOR_EMISSION_NONE:
	    StateGLShadeModel(state, GL_FLAT);
	    break;
	}

	/* Begin drawing explosion */
	glBegin(GL_QUADS);
	{
	    glNormal3f(0.0f, 1.0f, 0.0f);

	    glTexCoord2f(0.0f, 1.0f - 0.0f);
	    glVertex3f(-r, -r, 0.0f);
	    glTexCoord2f(1.0f, 1.0f - 0.0f);
	    glVertex3f(r, -r, 0.0f);
	    glTexCoord2f(1.0f, 1.0f - 1.0f);
	    glVertex3f(r, r, 0.0f);
	    glTexCoord2f(0.0f, 1.0f - 1.0f);
	    glVertex3f(-r, r, 0.0f);
	}
	glEnd();

	/* Restore GL states */
	StateGLShadeModel(state, shade_model_mode);
	if(lighting)
	    StateGLEnable(state, GL_LIGHTING);
}

/*
 *      Draws fire object.
 */
static void SARDrawFire(
	sar_dc_struct *dc,
	sar_object_struct *obj_ptr,
	sar_object_fire_struct *obj_fire_ptr
)
{
	gw_display_struct *display = dc->display;
	state_gl_struct *state = &display->state_gl;
	sar_scene_struct *scene = dc->scene;
	int	cur_frame = obj_fire_ptr->cur_frame,
		tex_num = dc->flir ?
			obj_fire_ptr->ir_tex_num : obj_fire_ptr->tex_num;
	float	r = MAX(obj_fire_ptr->radius, 0.0f),
		h = MAX(obj_fire_ptr->height, 0.0f);
	v3d_texture_ref_struct *t;
	StateGLBoolean lighting = state->lighting;
	GLenum shade_model_mode = state->shade_model_mode;


	/* Get fire texture */
	if(SARIsTextureAllocated(scene, tex_num))
	    t = scene->texture_ref[tex_num];
	else
	    return;

	/* Sanitize frame number */
	if(cur_frame >= t->total_frames)
	    cur_frame = t->total_frames - 1;
	if(cur_frame < 0)
	    cur_frame = 0;

	/* Select texture */
	V3DTextureSelectFrame(t, cur_frame);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

	/* Set up GL states */
	StateGLShadeModel(&display->state_gl, GL_FLAT);
	StateGLDisable(&display->state_gl, GL_LIGHTING);

	/* Begin drawing fire */
	glBegin(GL_QUADS);
	{
	    glNormal3f(0.0f, 1.0f, 0.0f);
	    glTexCoord2f(0.0f, 1.0f);
	    glVertex3f(-r, 0.0f, 0.0f);
	    glTexCoord2f(1.0f, 1.0f);
	    glVertex3f(r, 0.0f, 0.0f);
	    glTexCoord2f(1.0f, 0.0f);
	    glVertex3f(r, h, 0.0f);
	    glTexCoord2f(0.0f, 0.0f);
	    glVertex3f(-r, h, 0.0f);
	}
	glEnd();

	/* Restore GL states */
	StateGLShadeModel(&display->state_gl, shade_model_mode);
	if(lighting)
	    StateGLEnable(&display->state_gl, GL_LIGHTING);
}

/*
 *	Draws HUD attitude ticks.
 *
 *      Inputs are assumed valid.
 */
static void SARDrawHUDAttitude(
	sar_dc_struct *dc,
	const sar_direction_struct *dir,
	const sar_position_struct *vel,
	int offset_x, int offset_y,
	float fovx, float fovy
)
{
	gw_display_struct *display = dc->display;
	int width = dc->width, height = dc->height;
	int x, y, x2, y2, degi;
	float pitch_deg;
	int half_width, half_height;
	float sin_bank, cos_bank, r;
	int tick_start_y, end_min, end_max;	/* In pixels */
	int	tick_inc_yi,			/* In pixels */
		tick_inc_degreesi,		/* In degrees */
		tick_x_max,			/* In pixels */
		tick_x_min;
	float	tick_inc_y,			/* In pixels */
		tick_inc_degrees;		/* In degrees */
	int tick_start_pitch;			/* In degrees */
	char text[80];

	/* Glide slope cursor */
	int gs_w = 15, gs_h = 6;
	static u_int8_t gs_bm[] = {
 0x03, 0x80,
 0x04, 0x40,
 0xfc, 0x7e,
 0x04, 0x40,
 0x03, 0x80,
 0x01, 0x00
	};

	/* Coordinates are handled in left hand rule xy plane,
	 * the y axis is inverted at the call to the X draw
	 * function
	 *
	 * All degree calculations are in units of degrees * 100, this
	 * is needed to increase the resolution of the degrees that are
	 * typed as int's
	 */

	/* Calculate half of size */
	half_width = width / 2;
	half_height = height / 2;

	/* Calculate approximate bounds for drawing the ticks pixelwise,
	 * these bounds should be a bit larger since the scissors
	 * bounding box enabled further below will do more immediate
	 * and precise clipping
	 */
	end_min = (int)(-half_height);
	end_max = (int)(half_height);

	/* Calculate increments */
	tick_inc_degrees = 10 * 100;	/* One tick every 10 degrees */

	tick_x_max = (int)(0.06 * width);
	tick_x_min = (int)(0.03 * width);

	sin_bank = (float)sin(dir->bank);
	cos_bank = (float)cos(dir->bank);

	/* Calculate tick_inc_y */
	r = (float)height / fovy;
	tick_inc_y = r * (float)SFMDegreesToRadians(tick_inc_degrees / 100.0);

	/* Split into two cases, positive and negative pitch */
	if(dir->pitch > (1.0 * PI))
	{
	    /* Positive pitch */
	    float deg;

	    /* Calculate pitch in degrees * 100 units */
	    pitch_deg = (float)(
		36000 - (int)(SFMRadiansToDegrees(dir->pitch) * 100)
	    );
	    deg = (float)((int)pitch_deg % (int)tick_inc_degrees);
	    tick_start_y = (int)(-deg * tick_inc_y / tick_inc_degrees);
	    tick_start_pitch = (int)(pitch_deg - deg);
	}
	else
	{
	    /* Negative pitch */
	    float deg;

	    /* Calculate pitch in degrees * 100 units */
	    pitch_deg = (float)(
		SFMRadiansToDegrees(dir->pitch) * 100.0
	    );
	    deg = tick_inc_degrees -
		((int)pitch_deg % (int)tick_inc_degrees);
	    tick_start_y = (int)(-deg * tick_inc_y / tick_inc_degrees);
	    tick_start_pitch = (int)(-pitch_deg - deg);
	}
	tick_inc_degreesi = (int)tick_inc_degrees;
	tick_inc_yi = (int)tick_inc_y;


	/* Set up scissors bounding box to contain the upcomming drawing
	 * of the attitude ticks.
	 */
	if(True)
	{
	    GLsizei	s_width = (GLsizei)(0.40 * width),
			s_height = (GLsizei)(0.60 * height);

	    StateGLScissor(
		&display->state_gl,
		(GLint)(offset_x - (s_width / 2) + half_width),
		(GLint)(offset_y - (s_height / 2) + half_height),
		s_width, s_height
	    );
	    StateGLEnable(&display->state_gl, GL_SCISSOR_TEST);
	}

/* Define procedure to draw each tick for use in the for() loop
 * just below.
 */
#define DO_DRAW_ATTITUDE_TICK	{			\
 x2 = (int)(-sin_bank * y);				\
 y2 = (int)(cos_bank * y);				\
 if(degi == 0) {					\
  glBegin(GL_LINES); {					\
   glVertex2f(						\
(GLfloat)(offset_x + x2 + (tick_x_min * cos_bank) + half_width), \
(GLfloat)(offset_y + y2 + (tick_x_min * sin_bank) + half_height) \
   );							\
   glVertex2f(						\
(GLfloat)(offset_x + x2 + ((8 + tick_x_max) * cos_bank) + half_width), \
(GLfloat)(offset_y + y2 + ((8 + tick_x_max) * sin_bank) + half_height) \
   );							\
							\
   glVertex2f(						\
(GLfloat)(offset_x + x2 - (tick_x_min * cos_bank) + half_width), \
(GLfloat)(offset_y + y2 - (tick_x_min * sin_bank) + half_height) \
   );							\
   glVertex2f(						\
(GLfloat)(offset_x + x2 - ((8 + tick_x_max) * cos_bank) + half_width), \
(GLfloat)(offset_y + y2 - ((8 + tick_x_max) * sin_bank) + half_height) \
   );							\
  }							\
  glEnd();						\
 } else {						\
  glBegin(GL_LINES); {					\
   glVertex2f(						\
(GLfloat)(offset_x + x2 + (tick_x_min * cos_bank) + half_width), \
(GLfloat)(offset_y + y2 + (tick_x_min * sin_bank) + half_height) \
   );							\
   glVertex2f(						\
(GLfloat)(offset_x + x2 + (tick_x_max * cos_bank) + half_width), \
(GLfloat)(offset_y + y2 + (tick_x_max * sin_bank) + half_height) \
   );							\
							\
   glVertex2f(						\
(GLfloat)(offset_x + x2 - (tick_x_min * cos_bank) + half_width), \
(GLfloat)(offset_y + y2 - (tick_x_min * sin_bank) + half_height) \
   );							\
   glVertex2f(						\
(GLfloat)(offset_x + x2 - (tick_x_max * cos_bank) + half_width), \
(GLfloat)(offset_y + y2 - (tick_x_max * sin_bank) + half_height) \
   );							\
 }							\
 glEnd();						\
							\
 sprintf(text, "%i", degi / 100);			\
 /* Right */						\
 GWDrawString(						\
  display,						\
  (int)(offset_x + x2 + (tick_x_max * cos_bank) + half_width \
   - 8 + (12 * cos_bank)),				\
  (int)(height - (offset_y + y2 + (tick_x_max * sin_bank) + \
   half_height + 3)),					\
  text							\
 );							\
 /* Left */						\
 GWDrawString(						\
  display,						\
   (int)(offset_x + x2 - (tick_x_max * cos_bank) + half_width \
    - 8 - (18 * cos_bank)),				\
   (int)(height - (offset_y + y2 - (tick_x_max * sin_bank) + \
    half_height + 3)),					\
   text							\
  );							\
 }							\
 degi += tick_inc_degreesi;				\
}

	degi = tick_start_pitch;

	/* Draw upper (pitch up) ticks */
	for(y = tick_start_y; y < end_max; y += tick_inc_yi)
	{
	    if(degi > (90 * 100))
		break;

	    DO_DRAW_ATTITUDE_TICK
	}

	degi = tick_start_pitch - tick_inc_degreesi;

	/* Draw lower (pitch down) ticks */
	for(y = tick_start_y - tick_inc_yi; y > end_min; y -= tick_inc_yi)
	{
	    if(degi < -(90 * 100))
		break;

	    DO_DRAW_ATTITUDE_TICK
	}
#undef DO_DRAW_ATTITUDE_TICK


	/* Draw nose axis marker */
	x = offset_x + half_width;
	y = offset_y + half_height;
	glBegin(GL_LINES);
	{
	    glVertex2f(
		(GLfloat)(x - (0.05 * width)),
		(GLfloat)y
	    );
	    glVertex2f(
		(GLfloat)(x - (0.02 * width)),
		(GLfloat)y
	    );

	    glVertex2f(
		(GLfloat)(x + (0.05 * width)),
		(GLfloat)y
	    );
	    glVertex2f(
		(GLfloat)(x + (0.02 * width)),
		(GLfloat)y
	    );
	}
	glEnd();

	/* Draw glide slope marker */
	if(vel != NULL)
	{
	    float h, theta;

	    /* Calculate angle to ground relative to velocity */
	    theta = (float)SFMSanitizeRadians(
		(2 * PI) - atan2(vel->z, vel->y)
	    );
	    /* Adjust angle to ground relative to aircraft's pitch */
	    theta = (float)SFMSanitizeRadians(
		theta - dir->pitch
	    );
	    if(theta > (float)(0.5 * PI))
	    {
/* Don't go up beyond nose level */
		h = 0;
		theta = 0;
	    }
	    else
	    {
	        h = (height / fovy) * theta;
	    }

	    if(h < -end_min)
	    {
		x = (int)(offset_x + half_width);
		y = (int)(offset_y + half_height - h);

		glRasterPos2i(x - (gs_w / 2), y - (gs_h / 2));
		glBitmap(
		    gs_w, gs_h,
		    0.0f, 0.0f,
		    (GLfloat)gs_w, 0.0f,
		    gs_bm
		);
	    }
	}

	/* Disable scissors bounding box */
	StateGLDisable(&display->state_gl, GL_SCISSOR_TEST);
}

/*
 *	Draws HUD heading ticks and intercept arrow.
 *
 *	Inputs are assumed valid.
 */
static void SARDrawHUDHeading(
	sar_dc_struct *dc,
	const sar_direction_struct *dir,
	const sar_intercept_struct *intercept,
	int offset_x, int offset_y	/* Look offset */
)
{
	gw_display_struct *display = dc->display;
	sar_scene_struct *scene = dc->scene;
	const sar_option_struct *opt = dc->option;
	int width = dc->width, height = dc->height;
	const sar_position_struct *cam_pos = &dc->camera_pos;
	int x, y, deg, heading_deg, len;
	int half_width, half_height;
	int tick_start_x, end_min, end_max;	/* In pixels */
	int	tick_inc_x,			/* In pixels */
		tick_inc_degrees;		/* In degrees */
	int tick_start_heading;			/* In degrees */
	char text[256];
	int ia_w = 8, ia_h = 4;
	u_int8_t ia_bm[] = {0xc6, 0x6c, 0x38, 0x10};


	/* Coordinates are handled in left hand rule xy plane, with the
	 * y axis already inverted
	 */

	/* Calculate half of size */
	half_width = width / 2;
	half_height = height / 2;

	/* Calculate limits and increments */
	end_min = (int)(half_width - (0.3 * half_width));
	end_max = (int)(half_width + (0.3 * half_width));

	tick_inc_x = (int)(0.09 * width);
	tick_inc_degrees = 10;		/* Always 10 degrees */

	heading_deg = (int)SFMRadiansToDegrees(
	    dir->heading - scene->cant_angle
	);
	deg = heading_deg % tick_inc_degrees;
	tick_start_x = half_width - (deg * tick_inc_x / tick_inc_degrees);
	tick_start_heading = (int)SFMSanitizeDegrees(
	    heading_deg - deg
	);


#define DO_DRAW_HEADING_TICK	{		\
 glBegin(GL_LINES); {				\
  glVertex2f(					\
   (GLfloat)(x + offset_x),			\
   (GLfloat)(height - (y + offset_y - 10))	\
  );						\
  glVertex2f(					\
   (GLfloat)(x + offset_x),			\
   (GLfloat)(height - (y + offset_y - 14))	\
  );						\
 }						\
 glEnd();					\
						\
 sprintf(text, "%i", deg / 10);			\
 len = STRLEN(text);				\
 GWDrawString(					\
  display,					\
  (int)(x + offset_x - ((len - 1) * 4) - 3),	\
  (int)(y + offset_y - 11),			\
  text						\
 );						\
 deg = (int)SFMSanitizeDegrees(			\
  deg + tick_inc_degrees			\
 );						\
}

	y = (int)(half_height - (0.35 * height));
	deg = tick_start_heading;
	for(x = tick_start_x; x < end_max; x += tick_inc_x)
	{
	    DO_DRAW_HEADING_TICK
	}

	deg = (int)SFMSanitizeDegrees(tick_start_heading - tick_inc_degrees);
	for(x = tick_start_x - tick_inc_x; x > end_min; x -= tick_inc_x)
	{
	    DO_DRAW_HEADING_TICK
	}

#undef DO_DRAW_HEADING_TICK

	/* Draw intercept tick */
	if(intercept != NULL)
	{
	    int intercept_tick_dx;
	    float dtheta, icpt_dist;
	    float	icpt_dx = intercept->x - cam_pos->x,
			icpt_dy = intercept->y - cam_pos->y;


	    /* Calculate intercept distance in meters */
	    icpt_dist = (float)SFMHypot2(icpt_dx, icpt_dy);

	    /* Calculate delta theta from current heading to
	     * intercept bearing.
	     */
	    dtheta = (float)SFMDeltaRadians(
		dir->heading,
		SFMSanitizeRadians(
		    (0.5 * PI) - atan2(icpt_dy, icpt_dx)
		)
	    );
	    /* Convert to degrees, no sanitize */
	    dtheta = (float)(dtheta * (180.0 / PI));

	    if(tick_inc_degrees > 0)
	        intercept_tick_dx = (int)(dtheta * ((float)tick_inc_x /
		    (float)tick_inc_degrees));
	    else
		intercept_tick_dx = 0;

	    /* Apply half width */
	    intercept_tick_dx += half_width;

	    if(intercept_tick_dx < end_min)
		intercept_tick_dx = end_min;
	    else if(intercept_tick_dx > end_max)
		intercept_tick_dx = end_max;

	    glRasterPos2i(
		intercept_tick_dx + offset_x + 1 - (ia_w / 2),
		height - (y + offset_y + 5 - (ia_h / 2))
	    );
	    glBitmap(
		ia_w, ia_h,
		0.0f, 0.0f,
		(GLfloat)ia_w, 0.0f,
		ia_bm
	    );

	    /* Draw distance to intercept */
	    if(True)
	    {
		float v1;
		const char *units_str1;

		switch(opt->units)
		{
		  case SAR_UNITS_METRIC:
		  case SAR_UNITS_METRIC_ALT_FEET:
		    v1 = icpt_dist / 1000.0f;
		    units_str1 = "KM";
		    break;
		  default:      /* SAR_UNITS_ENGLISH */
		    v1 = (float)SFMMetersToMiles(icpt_dist);
		    units_str1 = "MI";
		    break;
		}
		if(v1 > 10.0f)
		    sprintf(text, "R: %.0f %s", v1, units_str1);
		else
		    sprintf(text, "R: %.1f %s", v1, units_str1);

		GWDrawString(
		    display,
		    (int)(offset_x + half_width + (0.4 * half_width)),
		    (int)(offset_y + y),
		    text
		);
	    }
	}

	/* Current heading tick */
	glBegin(GL_LINES);
	{
	    glVertex2f(
		(float)((half_width + offset_x)),
		(float)(height - (y + offset_y + 1))
	    );
	    glVertex2f(
		(float)(half_width + offset_x),
		(float)(height - (y + offset_y + 5))
	    );
	}
	glEnd();
}


/*
 *	Draws elevator trim on HUD display.
 *
 *	Given coordinates are in window coordinates at the center of the 
 *	trim gauge.
 *
 *	This can also be used to draw the outside attitude stuff.
 */
static void SARDrawHUDElevatorTrim(
	sar_dc_struct *dc,
	GLfloat xc, GLfloat yc,
	GLfloat length, GLfloat width,
	float elevator_trim
)
{
	GLfloat x[2], y[2], tick_pos, tick_spacing = length / 6;
	GLsizei icon_width = 8, icon_height = 8;
	const GLubyte icon_bm[] = {
		0x00,
		0x10,
		0x30,
		0x70,
		0xf0,
		0x70,
		0x30,
		0x10
	};

	x[0] = xc - (width / 2);
	x[1] = xc + (width / 2);
	y[0] = yc - (length / 2);
	y[1] = yc + (length / 2);

	glBegin(GL_LINE_STRIP);
	{
	    glVertex2d(x[1], y[1]);
	    glVertex2d(x[0], y[1]);
	    glVertex2d(x[0], y[0]);
	    glVertex2d(x[1], y[0]);
	}
	glEnd();

	if(tick_spacing > 2.0)
	{
	    glBegin(GL_LINES);
	    for(tick_pos = 0.0f; tick_pos < length; tick_pos += tick_spacing)
	    {
		glVertex2f(x[0], y[0] + tick_pos);
		glVertex2f(x[0] + (width / 2.0f), y[0] + tick_pos);
	    }
	    glEnd();
	}

	/* Current position */
	tick_pos = (GLfloat)((-elevator_trim * (length / 2)) + (length / 2));
	glRasterPos2f(
	    (GLfloat)x[0] + (width / 2),
	    (GLfloat)(y[0] + tick_pos - (icon_height / 2))
	);
	glBitmap(
	    icon_width, icon_height,
	    0.0f, 0.0f,
	    0.0f, 0.0f,
	    icon_bm
	);


#if 0
	/* Center */
	tick_pos = length / 2.0f;
	glBegin(GL_LINES);
	{
	    glVertex2f(x[0], y[0] + tick_pos);
	    glVertex2f(x[1], y[0] + tick_pos);
	}
	glEnd();
#endif
}


/*
 *	Draw HUD.
 *
 *	The given aspect is the width of the window in pixels
 *	divided by the height of the window in pixels.
 *
 *	All units are in projected units and radians, projected
 *	units are derived from the given width and height.
 */
static void SARDrawHUD(sar_dc_struct *dc, sar_object_struct *obj_ptr)
{
	gw_display_struct *display = dc->display;
	const sar_option_struct *opt = dc->option;
	sar_scene_struct *scene = dc->scene;
	float aspect = dc->view_aspect;
	int width = dc->width, height = dc->height;
	GWFont *font = opt->hud_font;
	sar_object_aircraft_struct *aircraft = SAR_OBJ_GET_AIRCRAFT(obj_ptr);

	int i;
	float	fovz = scene->camera_fovz,
		fovx = fovz * aspect;

	const sar_direction_struct	*dir	  = &obj_ptr->dir;
	const sar_direction_struct	*cam_dir  = &scene->camera_cockpit_dir;
	const sar_position_struct	*pos	  = &obj_ptr->pos;
	const sar_position_struct	*vel	  = NULL;
	const sar_position_struct	*airspeed = NULL;

	const sar_intercept_struct *intercept = NULL;
	sar_engine_state engine_state = SAR_ENGINE_OFF;
	int gear_state = 0;
	float	cockpit_offset_z = 0.0f,
		throttle = 0.0f,
		fuel = 0.0f,		/* In kg */
		elevator_trim = 0.0f;
	char text[80];

	if((aspect <= 0.0f) || (fovz <= 0.0f))
	    return;

	/* Get other values specific to object type */
	if(aircraft != NULL)
	{
	    cockpit_offset_z = aircraft->cockpit_offset_pos.z;
	    vel = &aircraft->vel;
	    airspeed = &aircraft->airspeed;
	    engine_state = aircraft->engine_state;
	    throttle = SARSimThrottleOutputCoeff(
		(aircraft->flight_model_type != SAR_FLIGHT_MODEL_SLEW) ?
		    aircraft->flight_model_type :
		    aircraft->last_flight_model_type
		,
		aircraft->throttle,
		aircraft->collective,
		aircraft->collective_range
	    );
	    fuel = aircraft->fuel;
	    elevator_trim = aircraft->elevator_trim;

	    i = aircraft->cur_intercept;
	    if((i >= 0) && (i < aircraft->total_intercepts))
		intercept = aircraft->intercept[i];
	}
	gear_state = SARObjLandingGearState(obj_ptr);


	/* Check if camera direction is looking forward enough to
	 * have the HUD in its view
	 */
	if(((cam_dir->heading > (1.5 * PI)) ||
	    (cam_dir->heading < (0.5 * PI))) &&
	   ((cam_dir->pitch > (1.5 * PI)) ||
	    (cam_dir->pitch < (0.5 * PI)))
	)
	{
	    float v1, r, heading, pitch;
	    int x, y;			/* Center of hud in pixels */
	    float speed = aircraft->landed ? airspeed->y : SFMHypot2(airspeed->y, airspeed->z);

	    // Airplanes should not display negative airspeeds.
	    if(aircraft->flight_model_type == SAR_FLIGHT_MODEL_AIRPLANE &&
	       speed < 0)
		speed = 0;

	    heading = cam_dir->heading;
	    if(heading > (float)(1.0 * PI))
		heading = (float)(heading - (2.0 * PI));

	    pitch = cam_dir->pitch;
	    if(pitch > (float)(1.0 * PI))
		pitch = (float)(pitch - (2.0 * PI));

	    /* Set HUD color and font */
	    if(dc->flir)
		glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	    else
		SARDrawSetColor(&opt->hud_color);
	    GWSetFont(display, font);

	    /* Calculate center of HUD in window coordinates */
	    r = width / fovx;
	    x = (int)((width / 2) - (r * heading));

	    r = height / fovz;
	    y = (int)((height / 2) - (r * pitch));

	    /* Draw speed in KTS */
	    switch(opt->units)
	    {
		case SAR_UNITS_METRIC:
		case SAR_UNITS_METRIC_ALT_FEET:
		    v1 = (float)SFMMPCToKPH(speed);
		    break;
		default:      /* SAR_UNITS_ENGLISH */
		    v1 = (float)SFMMPHToKTS(SFMMPCToMPH(speed));
		    break;
	    }
	    if(v1 < 100.0f)
		sprintf(text, "%.1f", v1);
	    else
		sprintf(text, "%.0f", v1);

	    GWDrawString(
		display,
		(int)(x - (width * 0.20) - 25),
		(int)(y + 3),
		text
	    );

	    /* G's */
	    v1 = ((aircraft->z_accel /
		((time_compensation > 0.0f) ? time_compensation : 1.0f)) +
		(float)SAR_GRAVITY) / (float)SAR_GRAVITY;
	    sprintf(text, "%.1fG", v1);
	    GWDrawString(
		display,
		(int)(x - (width * 0.20) - 25),
		(int)(y - (height * 0.36)),
		text
	    );

	    /* Elevator trim */
	    SARDrawHUDElevatorTrim(
		dc,
		(GLfloat)(x - (width * 0.23)),
		(GLfloat)(height - (y - (height * 0.22))),
		30, 5,
		elevator_trim
	    );


	    /* MSL altitude (include scene MSL elevation) */
	    if(True)
	    {
		float msl;

		switch(opt->units)
		{
		  case SAR_UNITS_METRIC:
		    msl = pos->z + cockpit_offset_z + scene->msl_elevation;
		    break;
		  case SAR_UNITS_METRIC_ALT_FEET:
		  default:	/* SAR_UNITS_ENGLISH */
		    msl = (float)SFMMetersToFeet(
			pos->z + cockpit_offset_z + scene->msl_elevation
		    );
		    break;
		}
		if(msl < 100.0f)
		    sprintf(text, "%.1f", msl);
		else
		    sprintf(text, "%.0f", msl);
		GWDrawString(
		    display,
		    (int)(x + (width * 0.20)),
		    (int)(y + 3),
		    text
		);
	    }

	    /* Throttle */
	    switch(engine_state)
	    {
	      case SAR_ENGINE_ON:
		sprintf(text, "Thr: %.0f%%", throttle * 100);
		break;

	      case SAR_ENGINE_INIT:
		sprintf(text, "Eng(S): %.0f%%", throttle * 100);
		break;

	      case SAR_ENGINE_OFF:
		sprintf(text, "Eng(O): %.0f%%", throttle * 100);
		break;
	    }
	    GWDrawString(
		display,
		(int)(x - (width * 0.25)),
		(int)(y + (height * 0.29)),
		text
	    );

	    /* Air brakes */
	    if(dc->player_air_brakes)
		GWDrawString(
		    display,
		    (int)(x - (width * 0.25)),
		    (int)(y + (height * 0.29) - 16),
		    "Brakes"
		);

	    /* Gear state */
	    if(gear_state == 1)
		GWDrawString(
		    display,
		    (int)(x - (width * 0.25)),
		    (int)(y + (height * 0.29) + 16),
		    "Gear"
		);

	    /* Fuel */
	    switch(opt->units)
	    {
	      case SAR_UNITS_METRIC:
	      case SAR_UNITS_METRIC_ALT_FEET:
		sprintf(text, "Fuel: %.0f kg", fuel);
		break;
	      default:	/* SAR_UNITS_ENGLISH */
		sprintf(text, "Fuel: %.0f lbs", SFMKGToLBS(fuel));
		break;
	    }
	    GWDrawString(
		display,
		(int)(x + (width * 0.05)),
		(int)(y + (height * 0.29)),
		text  
	    );

	    /* Draw HUD heading ticks */
	    SARDrawHUDHeading(
		dc,
	        dir,
		intercept,
		(int)(r * -heading), (int)(r * -pitch)
	    );

	    /* Draw HUD attitude ticks */
	    SARDrawHUDAttitude(
		dc,
		dir, vel,
		(int)(r * -heading), (int)(r * pitch),
		fovx, fovz
	    );
	}
}

/*
 *	Draws values describing the attitude values of the given
 *	object.
 *
 *	The color needs to be set prior to this call.
 */
static void SARDrawOutsideAttitude(
	sar_dc_struct *dc, sar_object_struct *obj_ptr
)
{
	gw_display_struct *display = dc->display;
	const sar_option_struct *opt = dc->option;
	sar_scene_struct *scene = dc->scene;
	int width = dc->width, height = dc->height;
	int	half_width = width / 2,
		half_height = height / 2;
	const sar_position_struct *cam_pos = &dc->camera_pos;
	GWFont *font = opt->message_font;
	int fw, fh;
	int i, x, y, x2, y2, degi, offset_x, offset_y;
	float pitch_deg;
	float sin_bank, cos_bank;
	int tick_start_y, end_min, end_max;     /* In pixels */
	int     tick_inc_yi,                    /* In pixels */
		tick_inc_degreesi,              /* In degrees */
		tick_x_max,                     /* In pixels */
		tick_x_min;
	float	tick_inc_y,			/* In pixels */
		tick_inc_degrees;		/* In degrees */
	int tick_start_pitch;                   /* In degrees */
	const sar_position_struct *pos;
	const sar_direction_struct *dir;
	sar_object_aircraft_struct *aircraft = NULL;

	int vel_dir_w = 32, vel_dir_h = 32;
	static u_int8_t vel_dir_bm[] = {
0x00, 0x00, 0x00, 0x00,
0x00, 0x01, 0x00, 0x00, 
0x00, 0x01, 0x00, 0x00, 
0x10, 0x00, 0x00, 0x08, 
0x08, 0x00, 0x00, 0x10, 
0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 
0x00, 0x01, 0x00, 0x00, 
0xc0, 0x03, 0x80, 0x03, 
0x00, 0x01, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 
0x08, 0x00, 0x00, 0x10, 
0x10, 0x00, 0x00, 0x08, 
0x00, 0x01, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00
	};


	/* Coordinates are handled in left hand rule xy plane, the y
	 * axis is inverted at the call to the X draw function.
	 */

	if(obj_ptr == NULL)
	    return;

	/* Get object values */
	pos = &obj_ptr->pos;
	dir = &obj_ptr->dir;
	aircraft = SAR_OBJ_GET_AIRCRAFT(obj_ptr);

	/* Set font and get font size */
	GWSetFont(display, font);
	GWGetFontSize(font, NULL, NULL, &fw, &fh);

	/* Set offset */
	offset_x = 0;
	offset_y = -half_height + 20 + 5;

	/* Calculate Y limits and increments */
	end_min = -20;
	end_max = 20;

	tick_inc_degrees = 10;          /* Always 10 degrees */

	tick_x_max = 10;		/* Tick width */
	tick_x_min = 4;			/* Tick spacing from center */

	sin_bank = (float)sin(dir->bank);
	cos_bank = (float)cos(dir->bank);

	/* Set tick increment along Y axis */
	tick_inc_y = 10;

	/* Split into two cases, positive and negative pitch */ 
	if(dir->pitch > PI)
	{
	    /* Positive pitch */
	    float deg;

	    pitch_deg = (float)(
		360 - (int)SFMRadiansToDegrees(dir->pitch)
	    );
	    deg = (float)((int)pitch_deg % (int)tick_inc_degrees);
	    tick_start_y = (int)(-deg * tick_inc_y / tick_inc_degrees);
	    tick_start_pitch = (int)(pitch_deg - deg);
	}
	else
	{       
	    /* Negative pitch */
	    float deg;

	    pitch_deg = (float)SFMRadiansToDegrees(dir->pitch);
	    deg = tick_inc_degrees -
		((int)pitch_deg % (int)tick_inc_degrees);
	    tick_start_y = (int)(-deg * tick_inc_y / tick_inc_degrees);
	    tick_start_pitch = (int)(-pitch_deg - deg);
	}   
	tick_inc_degreesi = (int)tick_inc_degrees;
	tick_inc_yi = (int)tick_inc_y; 


/* Define procedure to draw each tick for use in the for() loop
 * just below.
 */
#define DO_DRAW_ATTITUDE_TICK	{			\
 x2 = (int)(-sin_bank * y);				\
 y2 = (int)(cos_bank * y);				\
 if(degi == 0) {					\
  glBegin(GL_LINES); {					\
   glVertex2f(						\
(GLfloat)(offset_x + x2 + (tick_x_min * cos_bank) + half_width), \
(GLfloat)(offset_y + y2 + (tick_x_min * sin_bank) + half_height) \
   );							\
   glVertex2f(						\
(GLfloat)(offset_x + x2 + ((5 + tick_x_max) * cos_bank) + half_width), \
(GLfloat)(offset_y + y2 + ((5 + tick_x_max) * sin_bank) + half_height) \
   );							\
							\
   glVertex2f(						\
(GLfloat)(offset_x + x2 - (tick_x_min * cos_bank) + half_width), \
(GLfloat)(offset_y + y2 - (tick_x_min * sin_bank) + half_height) \
   );							\
   glVertex2f(						\
(GLfloat)(offset_x + x2 - ((5 + tick_x_max) * cos_bank) + half_width), \
(GLfloat)(offset_y + y2 - ((5 + tick_x_max) * sin_bank) + half_height) \
   );							\
  }							\
  glEnd();						\
 } else {						\
  glBegin(GL_LINES); {					\
   glVertex2f(						\
(GLfloat)(offset_x + x2 + (tick_x_min * cos_bank) + half_width), \
(GLfloat)(offset_y + y2 + (tick_x_min * sin_bank) + half_height) \
   );							\
   glVertex2f(						\
(GLfloat)(offset_x + x2 + (tick_x_max * cos_bank) + half_width), \
(GLfloat)(offset_y + y2 + (tick_x_max * sin_bank) + half_height) \
   );							\
							\
   glVertex2f(						\
(GLfloat)(offset_x + x2 - (tick_x_min * cos_bank) + half_width), \
(GLfloat)(offset_y + y2 - (tick_x_min * sin_bank) + half_height) \
   );							\
   glVertex2f(						\
(GLfloat)(offset_x + x2 - (tick_x_max * cos_bank) + half_width), \
(GLfloat)(offset_y + y2 - (tick_x_max * sin_bank) + half_height) \
   );							\
  }							\
  glEnd();						\
 }							\
 degi += tick_inc_degreesi;				\
}

	degi = tick_start_pitch;

	/* Draw upper (pitch up) ticks */
	for(y = tick_start_y; y < end_max; y += tick_inc_yi)   
	{
	    if(degi > 90) 
		break;

	    DO_DRAW_ATTITUDE_TICK
	}

	degi = tick_start_pitch - tick_inc_degreesi;

	/* Draw lower (pitch down) ticks */
	for(y = tick_start_y - tick_inc_yi; y > end_min; y -= tick_inc_yi)
	{
	    if(degi < -90)
		break;

	    DO_DRAW_ATTITUDE_TICK
	}
#undef DO_DRAW_ATTITUDE_TICK


	/* Draw aircraft axis marker */
	x = offset_x + half_width;
	y = offset_y + half_height;
	glBegin(GL_LINES);
	{
	    glVertex2i(
		x - 6,
		y
	    );
	    glVertex2i(
		x + 6,
		y
	    );
	}
	glEnd();

	/* Draw speed and direction label */
	if(aircraft != NULL)
	{
	    const sar_intercept_struct *intercept = NULL;
	    float	intercept_dtheta = 0.0f,
			intercept_sin_theta,
			intercept_cos_theta,
			intercept_dist = 0.0f,
			abs_speed;
	    char s[256];


	    /* Get pointer to selected intercept */
	    i = aircraft->cur_intercept;
	    if((i >= 0) && (i < aircraft->total_intercepts))
	    {
		intercept = aircraft->intercept[i];
		if(intercept != NULL)
		{
		    float	icpt_dx = intercept->x - pos->x,
				icpt_dy = intercept->y - pos->y;

		    /* Calculate intercept delta theta in radians */
		    intercept_dtheta = (float)SFMDeltaRadians(
			dir->heading,
			SFMSanitizeRadians(
			    (0.5 * PI) - atan2(icpt_dy, icpt_dx)
			)
		    );
		    /* Calculate intercept distance in meters */
		    intercept_dist = (float)SFMHypot2(icpt_dx, icpt_dy);
		}
	    }
	    intercept_sin_theta = (float)sin(intercept_dtheta);
	    intercept_cos_theta = (float)cos(intercept_dtheta);

	    /* Calculate 2d velocity */
	    abs_speed = (float)SFMHypot2(
		aircraft->vel.x,
		aircraft->vel.y
	    );

	    /* Speed and distance to intercept label */
	    if(intercept_dist > 0.0f)
	    {
		float v1, v2;
		const char *units_str1, *units_str2;

		switch(opt->units)
		{
		  case SAR_UNITS_METRIC:
		  case SAR_UNITS_METRIC_ALT_FEET:
		    v1 = (float)SFMMPCToKPH(aircraft->vel.y);
		    v2 = intercept_dist / 1000.0f;
		    units_str1 = "KPH";
		    units_str2 = "KM";
		    break;
		  default:	/* SAR_UNITS_ENGLISH */
		    v1 = (float)SFMMPCToMPH(aircraft->vel.y);
		    v2 = (float)SFMMetersToMiles(intercept_dist);
		    units_str1 = "MPH";
		    units_str2 = "MI";
		    break;
		}

		x = offset_x + half_width - (width / 4) - (7 * 9);
		y = height - (offset_y + half_height - 4);
		if(v1 < 100.0f)
		{
		    if(v2 < 10.0f)
			sprintf(s, "%.1f %s  %.1f %s",
			    v1, units_str1, v2, units_str2
			);
		    else
			sprintf(s, "%.1f %s  %.0f %s",
			    v1, units_str1, v2, units_str2
			);
		}
		else
		{
		    if(v2 < 10.0f)
			sprintf(s, "%.0f %s  %.1f %s",
			     v1, units_str1, v2, units_str2
			);
		    else
			sprintf(s, "%.0f %s  %.0f %s",
			     v1, units_str1, v2, units_str2
			);
		}
	    }
	    else
	    {
		float v1;
		const char *units_str1;

		switch(opt->units)
		{
		  case SAR_UNITS_METRIC:
		  case SAR_UNITS_METRIC_ALT_FEET:
		    v1 = (float)SFMMPCToKPH(aircraft->vel.y);
		    units_str1 = "KPH";
		    break;
		  default:      /* SAR_UNITS_ENGLISH */
		    v1 = (float)SFMMPCToMPH(aircraft->vel.y);
		    units_str1 = "MPH";
		    break;
		}

		x = offset_x + half_width - (width / 4) - (7 * 5);
		y = height - (offset_y + half_height - 4);
		if(v1 < 100.0f)
		    sprintf(s, "%.1f %s", v1, units_str1);
		else
		    sprintf(s, "%.0f %s", v1, units_str1);
	    }
	    GWDrawString(display, x, y, s);

	    /* Draw velocity direction (object relative) */
	    x = offset_x + half_width - (width / 4);
	    y = offset_y + half_height + 12;
	    if(abs_speed > 0.0f)
	    {
		glBegin(GL_LINES);
		{
		    glVertex2f(
			(GLfloat)(
		x + ((aircraft->vel.x / abs_speed) * 12.0f)
			),
			(GLfloat)(
		y + ((aircraft->vel.y / abs_speed) * 12.0f)
			)
		    );
		    glVertex2i(x, y);
		}
		glEnd();
	    }

	    /* Intercept direction */
	    glBegin(GL_LINES);
	    {
		glVertex2f(
		    (GLfloat)(x + (intercept_sin_theta * 16.0f)),
		    (GLfloat)(y + (intercept_cos_theta * 16.0f))
		);
		glVertex2f(
		    (GLfloat)(x + (intercept_sin_theta * 10.0f)),
		    (GLfloat)(y + (intercept_cos_theta * 10.0f))
	        );
	    }
	    glEnd();

	    glRasterPos2i(
		x - (vel_dir_w / 2) + 1,
		y - (vel_dir_h / 2)
	    );
	    glBitmap(
		vel_dir_w, vel_dir_h, 
		0.0f, 0.0f,
		(GLfloat)vel_dir_w, 0.0f,
		vel_dir_bm
	    );
	}

	/* Draw elevator trim, throttle/collective, roc, and altitude */
	if(aircraft != NULL)
	{
	    float v1, v2, throttle;
	    int gear_state = SARObjLandingGearState(obj_ptr);
	    const char *units_str1;
	    char s[256];

	    /* Elevator trim */
	    SARDrawHUDElevatorTrim(
		dc,
		(float)(offset_x + half_width + (width / 4) - 35),
		(float)(offset_y + half_height + 5),
		30, 5,
		aircraft->elevator_trim
	    );

	    /* Set position for drawing throttle/collective, altitude,
	     * and rate of climb
	     */
	    x = offset_x + half_width + (width / 4) - (7 * 3);
	    y = height - (offset_y + half_height + 32);

	    /* Throttle/collective */
	    throttle = SARSimThrottleOutputCoeff(
		(aircraft->flight_model_type != SAR_FLIGHT_MODEL_SLEW) ?
		    aircraft->flight_model_type :
		    aircraft->last_flight_model_type
		,
		aircraft->throttle,
		aircraft->collective,
		aircraft->collective_range
	    );
	    switch(aircraft->engine_state)
	    {
	      case SAR_ENGINE_OFF:
		sprintf(s, "Eng(O): %.0f%%", throttle * 100);
		break;

	      case SAR_ENGINE_INIT:
		sprintf(s, "Eng(S): %.0f%%", throttle * 100);
		break;

	      default:
		sprintf(s, "Thr: %.0f%%", throttle * 100);
		break;
	    }
	    GWDrawString(display, x, y, s);
	    y += 18;

	    /* Altitude AGL */
	    switch(opt->units)
	    {
	      case SAR_UNITS_METRIC:
		if(aircraft->landed)
		  v1 = 0.0f;
		else
		  v1 = (float)MAX(
		    -((((gear_state == 1) ? aircraft->gear_height : 0.0f) +
		    aircraft->belly_height) +
		    aircraft->center_to_ground_height), 0.0f
		  );
		units_str1 = "M";
		break;
	      case SAR_UNITS_METRIC_ALT_FEET:
	      default:
		if(aircraft->landed)
		  v1 = 0.0f;
		else
		  v1 = (float)SFMMetersToFeet(
		   MAX(
		    -((((gear_state == 1) ? aircraft->gear_height : 0.0f) +
		    aircraft->belly_height) +
		    aircraft->center_to_ground_height), 0.0f
		   )
		  );
		units_str1 = "FT";
		break;
	    }
	    if(v1 >= 100.0f)
		sprintf(s, "%.0f %s AGL", v1, units_str1);
	    else
		sprintf(s, "%.1f %s AGL", v1, units_str1);
	    GWDrawString(display, x, y, s);
	    y += 18;

	    /* Rate of climb */
	    switch(opt->units)
	    {
	      case SAR_UNITS_METRIC:
		if(aircraft->landed)
		    v1 = 0.0f;
		else
		    v1 = aircraft->vel.z *
			((float)SFMCycleUnitsUS / 1000000.0f);
		units_str1 = "MPS";
		break;
	      case SAR_UNITS_METRIC_ALT_FEET:
	      default:
		if(aircraft->landed)
		    v1 = 0.0f;
		else
		    v1 = (float)SFMMPCToFPS(aircraft->vel.z);
		units_str1 = "FPS";
		break;
	    }
	    /* G's */
	    v2 = ((aircraft->z_accel /
		((time_compensation > 0.0f) ? time_compensation : 1.0f)) +
		(float)SAR_GRAVITY) / (float)SAR_GRAVITY;
	    if(v1 > 10.0f)
	        sprintf(s, "+%.0f %s (%.1fG)", v1, units_str1, v2);
	    else if(v1 > 0.0f)
		sprintf(s, "+%.1f %s (%.1fG)", v1, units_str1, v2);
	    else if(v1 > -10.0f)
		sprintf(s, "%.1f %s (%.1fG)", v1, units_str1, v2);
	    else
		sprintf(s, "%.0f %s (%.1fG)", v1, units_str1, v2);
	    GWDrawString(display, x, y, s);
	}

	/* Draw heading label (just above attitude ticks) */
	if(aircraft != NULL)
	{
	    float v1 = (float)SFMRadiansToDegrees(dir->heading);
	    char s[80];

	    sprintf(s, "%.0f", v1);
	    x = (int)(
		offset_x + half_width - (STRLEN(s) * fw / 2)
	    );
	    y = (int)(
		height - (offset_y + half_height + 36)
	    );
	    GWDrawString(display, x, y, s);
	}

	/* If airbrakes are on draw label just above heading label */
	if(dc->player_air_brakes)
	{
	    const char *s = "Brakes";
	    x = (int)(
		offset_x + half_width - (STRLEN(s) * fw / 2)
	    );
	    y = (int)(
		height - (offset_y + half_height + 50)
	    );
	    GWDrawString(display, x, y, s);
	}

	/* Draw coordinates if viewing from map but not in slew mode */
	if((scene->camera_ref == SAR_CAMERA_REF_MAP) &&
	   (dc->player_flight_model_type != SAR_FLIGHT_MODEL_SLEW)
	)
	{
	    float dms_x, dms_y;
	    char s[128];

	    SFMMToDMS(
		cam_pos->x, cam_pos->y,
		scene->planet_radius,
		scene->dms_x_offset, scene->dms_y_offset,
		&dms_x, &dms_y
	    );

	    sprintf(
		s,
		"%s %s  Alt: %.0f",
		SFMLongitudeToString(dms_x),
		SFMLatitudeToString(dms_y),
		SFMMetersToFeet(cam_pos->z)
	    );
	    GWDrawString(display, 5, 5, s);
	}
}

/*
 *	Draws map grids, called by SARDrawMap().
 *
 *	Translation should still be in world coordinates (not 
 *	orthogonal'ed).
 *
 *	Given pos should be the camera position. Field of view fov
 *	is in radians about the Y axis.
 *
 *	Inputs assumed valid.
 */
static void SARDrawMapGrids(
	sar_dc_struct *dc, sar_position_struct *pos
)
{
	gw_display_struct *display = dc->display;
	sar_scene_struct *scene = dc->scene;
	float	planet_radius = scene->planet_radius;
	StateGLBoolean	alpha_test = display->state_gl.alpha_test;
	float x_max, x_min, y_max, y_min;
	float x, y, dx, dy, theta;
	float	lat_dmstom_coeff, lon_dmstom_coeff,
		dms_center_x, dms_center_y;


	/* Planet radius must be reasonably big */
	if(planet_radius < 1000.0f)
	    return;

	/* Calculate visible size bounds */
	x_max = dc->map_dxm / 2.0f;
	x_min = pos->x - x_max;
	x_max += pos->x;
	y_max = dc->map_dym / 2.0f;
	y_min = pos->y - y_max;
	y_max += pos->y;

	/* Calculate theta in radians, representing the Y axis theta */
	theta = (float)CLIP(
	    DEGTORAD(scene->dms_y_offset) + (pos->y / planet_radius)
	, -0.5 * PI, 0.5 * PI);

	/* Calculate conversion coefficients, degrees to meters */
	lat_dmstom_coeff = (float)((PI / 180.0) * planet_radius);
	lon_dmstom_coeff = (float)(cos(theta) * (PI / 180.0) * planet_radius);

	/* Calculate deltas, in meters. Delta is 1 degree */
	dx = 1.0f * lon_dmstom_coeff;
	dy = 1.0f * lat_dmstom_coeff;

	/* Calculate dms center in dms (degrees) */
	SFMMToDMS(
	    pos->x, pos->y, scene->planet_radius,
	    scene->dms_x_offset, scene->dms_y_offset,
	    &dms_center_x, &dms_center_y
	);

	/* Note: All coordinate units in dms, convert to meters just
	 * before drawing but do all calculations in dms (degrees).
	 */
#define COLOR_LEVEL0    glColor4f(1.0f, 0.0f, 0.0f, 1.0f);
#define COLOR_LEVEL1    glColor4f(1.0f, 0.0f, 0.0f, 0.2f);
#define COLOR_LEVEL2    glColor4f(1.0f, 0.0f, 0.0f, 0.4f);

	/* Begin drawing line ticks
	 *
	 * Note that we can use floor() instead of modulus when 
	 * calculating the starting position sinec we are rounding to
	 * the nearest 1 degree as an int
	 */
	StateGLDisable(&display->state_gl, GL_ALPHA_TEST);
	StateGLEnable(&display->state_gl, GL_BLEND);
	StateGLBlendFunc(
	    &display->state_gl,
	    GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA
	);

	/* Latitude ticks */
	if(dy > 0.0f)
	{
	    int clevel;

#define DO_DRAW_LINE	{	\
 switch(clevel) {		\
  case 1:			\
   COLOR_LEVEL1			\
   clevel = 2;			\
   break;			\
  case 2:			\
   COLOR_LEVEL2			\
   clevel = 3;			\
   break;			\
  case 3:			\
   COLOR_LEVEL1			\
   clevel = 0;			\
   break;			\
  default:			\
   COLOR_LEVEL0			\
   clevel = 1;			\
   break;			\
 }				\
 glBegin(GL_LINES); {		\
  glVertex3f(x_min, 0.0f, -y);	\
  glVertex3f(x_max, 0.0f, -y);	\
 }				\
 glEnd();			\
}

	    for(
		y = (
		    ((int)floor(dms_center_y) * lat_dmstom_coeff) -
		    (scene->dms_y_offset * lat_dmstom_coeff)
		),
		clevel = 0;
		y < y_max;
		y += (dy / 4)
	    )
	    {
		DO_DRAW_LINE
	    }
	    for(
		y = (
		    ((int)floor(dms_center_y) * lat_dmstom_coeff) -
		    (scene->dms_y_offset * lat_dmstom_coeff)
		) - (dy / 4),
		clevel = 1;
		y > y_min;
		y -= (dy / 4)
	    )
	    {
		DO_DRAW_LINE
	    }
#undef DO_DRAW_LINE
	}

	/* Longitude ticks */
	if(dx > 0.0f)
	{
	    int clevel;

#define DO_DRAW_LINE	{	\
 switch(clevel) {		\
  case 1:			\
   COLOR_LEVEL1			\
   clevel = 2;			\
   break;			\
  case 2:			\
   COLOR_LEVEL2			\
   clevel = 3;			\
   break;			\
  case 3:			\
   COLOR_LEVEL1			\
   clevel = 0;			\
   break;			\
  default:			\
   COLOR_LEVEL0			\
   clevel = 1;			\
   break;			\
 }				\
 glBegin(GL_LINES); {		\
  glVertex3f(x, 0.0f, -y_min);	\
  glVertex3f(x, 0.0f, -y_max);	\
 }				\
 glEnd();			\
}
	    for(
		x = (
		    ((int)floor(dms_center_x) * lon_dmstom_coeff) -
		    (scene->dms_x_offset * lon_dmstom_coeff)
		),
		clevel = 0;
		x < x_max;
		x += (dx / 4)
	    )
	    {
		DO_DRAW_LINE
	    }
	    for(
		x = (
		    ((int)floor(dms_center_x) * lon_dmstom_coeff) -
		    (scene->dms_x_offset * lon_dmstom_coeff)
		) - (dx / 4),
		clevel = 1;
		x > x_min;
		x -= (dx / 4)
	    )
	    {
		DO_DRAW_LINE
	    }
#undef DO_DRAW_LINE
	}

	StateGLDisable(&display->state_gl, GL_BLEND);
	if(alpha_test)
	    StateGLEnable(&display->state_gl, GL_ALPHA_TEST);

#undef COLOR_LEVEL0
#undef COLOR_LEVEL1
#undef COLOR_LEVEL2
}

/*
 *      Draws cross hairs, called by SARDrawMap().
 *
 *      Translation should still be ortho'ed.
 *
 *      Inputs assumed valid.   
 */
static void SARDrawMapCrossHairs(sar_dc_struct *dc)
{
	int width = dc->width, height = dc->height;
	GLfloat x1, x2, y1, y2;
	GLfloat size_coeff = (GLfloat)0.03;


	glColor4f(1.0f, 0.0f, 0.0f, 1.0f);

	x1 = (GLfloat)(width / 2);
	x2 = (GLfloat)ceil(x1 + ((float)width * size_coeff));
	x1 = (GLfloat)(x1 - ((float)width * size_coeff));

	y1 = (GLfloat)height / 2.0f;

	glBegin(GL_LINES);
	{
	    glVertex2i((GLint)x1, (GLint)y1);
	    glVertex2i((GLint)x2, (GLint)y1);
	}
	glEnd();


	x1 = (GLfloat)width / 2.0f;

	y1 = (GLfloat)height / 2.0f;
	y2 = (GLfloat)ceil(y1 + ((float)height * size_coeff));
	y1 = y1 - ((GLfloat)height * size_coeff);

	glBegin(GL_LINES);
	{
	    glVertex2i((GLint)x1, (GLint)y1);
	    glVertex2i((GLint)x1, (GLint)y2);
	}
	glEnd();
}


/*
 *	Draws the lights.
 */
static void SARDrawLights(
	sar_dc_struct *dc,
	sar_light_struct **ptr, int total
)
{
	gw_display_struct *display = dc->display;
	StateGLBoolean lighting = display->state_gl.lighting;
	StateGLBoolean alpha_test = display->state_gl.alpha_test;
	int i;
	float intensity_coeff;
	const sar_color_struct *c;
	const sar_position_struct *pos;
	const sar_light_struct *light;


	/* No lights to draw? */
	if(total <= 0)
	    return;

	/* Set up GL states */
	StateGLDisable(&display->state_gl, GL_LIGHTING);
	StateGLDisable(&display->state_gl, GL_ALPHA_TEST);
	StateGLEnable(&display->state_gl, GL_BLEND);
	StateGLBlendFunc(
	    &display->state_gl, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA
	);
	StateGLEnable(&display->state_gl, GL_POINT_SMOOTH);

	/* Draw each light */
	for(i = 0; i < total; i++)
	{
	    light = ptr[i];
	    if(light == NULL)
		continue;

	    /* Light not on? */
	    if(!(light->flags & SAR_LIGHT_FLAG_ON))
		continue;

	    /* Skip attenuation spot lights */
	    if(light->flags & SAR_LIGHT_FLAG_ATTENUATE)
		continue;

	    /* Strobe light? */
	    if(light->flags & SAR_LIGHT_FLAG_STROBE)
	    {
		/* Currently off and waiting to be turned on? */
		if(light->next_on > 0)
		{
		    intensity_coeff = CLIP(
			(light->int_off > 0) ?
			(float)(light->next_on - cur_millitime) / 
			    (float)light->int_off : 0.0f,
			0.0f, 1.0f
		    );
		    /* Get intensity coefficient from 0.0 to 0.5.
		     * Set low point 0.0 to occure 20% later.
		     */
		    intensity_coeff = (intensity_coeff > 0.3f) ?
			(intensity_coeff - 0.3f) * (0.5f / 0.7f) :
			(0.3f - intensity_coeff) * (0.5f / 0.3f);
		}
		else
		{
		    /* Currently on and waiting to be turned off */
		    intensity_coeff = CLIP(
			(light->int_on > 0) ?
			(float)(light->next_off - cur_millitime) /
			    (float)light->int_on : 1.0f,
			0.0f, 1.0f
		    );
		    /* Get intensity coefficient from 0.5 to 1.0 */
		    intensity_coeff = ((intensity_coeff > 0.5f) ?
			1.0f - intensity_coeff : intensity_coeff) +
			0.5f;
		}
	    }
	    else
	    {
		intensity_coeff = 1.0f;
	    }

	    /* Get position and color or light */
	    pos = &light->pos;
	    c = &light->color;

	    /* Begin drawing center point intense part of light (for
	     * strobes only)
	     */
	    if((light->flags & SAR_LIGHT_FLAG_STROBE) &&
	       (intensity_coeff >= 0.5f)
	    )
	    {
		StateGLPointSize(
		    &display->state_gl,
		    (GLfloat)MAX(light->radius, 1.0)
		);
		if(dc->flir)
		{
		    float g = MAX(MAX(c->r, c->g), c->b);
		    glColor4f(g, g, g, c->a);
		}
		else
		{
		    glColor4f(c->r, c->g, c->b, c->a);
		}
		glBegin(GL_POINTS);
		{
		    glVertex3f(pos->x, pos->z, -pos->y);
		}
		glEnd();
	    }

	    /* Set point size */
	    StateGLPointSize(
		&display->state_gl,
		(GLfloat)MAX(light->radius * 2.0 * intensity_coeff, 1.0)
	    );

	    /* Set color */
	    if(dc->flir)
	    {
		float g = MAX(MAX(c->r, c->g), c->b);
		glColor4f(g, g, g, c->a * intensity_coeff);
	    }
	    else
	    {
		glColor4f(c->r, c->g, c->b, c->a * intensity_coeff);
	    }

	    /* Draw light */
	    glBegin(GL_POINTS);
	    {
		glVertex3f(pos->x, pos->z, -pos->y);
	    }
	    glEnd();
	}

	/* Restore GL states */
	StateGLDisable(&display->state_gl, GL_POINT_SMOOTH);
	StateGLPointSize(&display->state_gl, 1.0f);
	StateGLDisable(&display->state_gl, GL_BLEND);
	if(alpha_test)
	    StateGLEnable(&display->state_gl, GL_ALPHA_TEST);
	if(lighting)
	    StateGLEnable(&display->state_gl, GL_LIGHTING);
}


/*
 *	Draws the cockpit on the given object, which must be an
 *	aircraft.
 *
 *	The rotation and translation should be at the identity before
 *	this call, that means the identity matrix should be called
 *	just before this function. This function will do the rotation
 *	and translation of the primary light and the camera near
 *	the center in world coordinates (instead of translating to the
 *	object's actual location since that would not be needed and
 *	cause jittery movement).
 *
 *	The rotations and translations will be modified by this call,
 *	so you should reload the identity matrix after this call.
 *	In addition, the perspective will be modified and depth testing
 *	will be turned off by this call.
 *
 *	Inputs assumed valid.
 */
static void SARDrawObjectCockpit(
	sar_dc_struct *dc, sar_object_struct *obj_ptr
)
{
	gw_display_struct *display = dc->display;
	sar_scene_struct *scene = dc->scene;
	float view_aspect = dc->view_aspect;
/*	float visibility_max = dc->visibility_max; */
	GLfloat vf[4];
	const sar_position_struct *offset;
	sar_position_struct light_pos;
	const sar_direction_struct	*dir = &obj_ptr->dir,
					*dir2 = &scene->camera_cockpit_dir;
	sar_object_aircraft_struct *aircraft = SAR_OBJ_GET_AIRCRAFT(obj_ptr);
	if(aircraft == NULL)
	    return;

	offset = &aircraft->cockpit_offset_pos;

	/* At this point, the identity matrix is loaded, so we need
	 * to rotate and translate again. Except that we will not
	 * translate to the actual position of the object and instead
	 * stay near the center.
	 */
	glLoadIdentity();

	glNormal3f(0.0f, 1.0f, 0.0f);
	/* Rotate camera inside cockpit */
	if(dir2->pitch != 0)
	    glRotatef(
		(GLfloat)SFMRadiansToDegrees(dir2->pitch),
		1.0f, 0.0f, 0.0f
	    );
	if(dir2->heading != 0)
	    glRotatef(
		(GLfloat)SFMRadiansToDegrees(dir2->heading),
		0.0f, 1.0f, 0.0f
	    );
	/* Offset of cockpit */
	glTranslatef(-offset->x, -offset->z, offset->y);
	/* Object rotation */
	if(dir->bank != 0)
	    glRotatef(
		(GLfloat)SFMRadiansToDegrees(dir->bank),
		0.0f, 0.0f, 1.0f
	    );
	if(dir->pitch != 0)
	    glRotatef(
		(GLfloat)SFMRadiansToDegrees(dir->pitch),
		1.0f, 0.0f, 0.0f
	    );
	if(dir->heading != 0)
	    glRotatef(
		(GLfloat)SFMRadiansToDegrees(dir->heading),
		0.0f, 1.0f, 0.0f
	    );

	/* Calculate light position */
	light_pos.x = (float)(
	    scene->light_pos.x * SAR_PRILIGHT_TO_CAMERA_RAD
	);
	light_pos.y = (float)(
	    scene->light_pos.y * SAR_PRILIGHT_TO_CAMERA_RAD
	);
	light_pos.z = (float)(
	    scene->light_pos.z * SAR_PRILIGHT_TO_CAMERA_RAD
	);

	/* Set new primary light position but do not modify other light
	 * parameters since they should remain set from values set
	 * before this call
	 */
	vf[0] = (GLfloat)light_pos.x;
	vf[1] = (GLfloat)light_pos.z;
	vf[2] = (GLfloat)-light_pos.y;
	vf[3] = 0.0f;		/* Directional lights only */
	glLightfv(GL_LIGHT0, GL_POSITION, vf);

	glPushMatrix();
	{
	    /* Disable depth testing */
	    SAR_DRAW_DEPTH_TEST_OFF

	    /* Object counter rotation */
	    if(dir->heading != 0)
		glRotatef(
		    (GLfloat)-SFMRadiansToDegrees(dir->heading),
		    0.0f, 1.0f, 0.0f
		);
	    if(dir->pitch != 0)
		glRotatef(
		    (GLfloat)-SFMRadiansToDegrees(dir->pitch),
		    1.0f, 0.0f, 0.0f
		);
	    if(dir->bank != 0)
		glRotatef(
		    (GLfloat)-SFMRadiansToDegrees(dir->bank),
		    0.0f, 0.0f, 1.0f
		);

	    /* Reset projection matrix for close up drawing, set the
	     * near and far clip values to close range
	     */
	    glMatrixMode(GL_PROJECTION);
	    glLoadIdentity();
	    gluPerspective(
		SFMRadiansToDegrees(scene->camera_fovz),	/* Field of view */
		view_aspect,
		0.05,		/* Very close near clip */
		1000.0		/* Reasonable far clip */
	    );
	    glMatrixMode(GL_MODELVIEW);

	    SARVisualModelCallList(
		aircraft->visual_model_cockpit
	    );

	    SAR_DRAW_POST_CALLLIST_RESET_STATES

/* Technically we should reset the persepective, but since no
 * calls that require the perspective to be reset after this
 * point is needed so we will not do that
 */
	}
	glPopMatrix();
}

/*
 *	Draws shadow (if available) on the object. Translation
 *	is already assumed to be at the object's center and heading
 *	(except pitch and bank) of object is already set.
 *
 *	Also draws rotor and prop wash if they are relivent for the
 *	given object's values.
 *
 *	Inputs assumed valid.
 */
static void SARDrawObjectShadow(
	sar_dc_struct *dc, sar_object_struct *obj_ptr
)
{
	gw_display_struct *display = dc->display;
	state_gl_struct *state = &display->state_gl;
	const sar_option_struct *opt = dc->option;
	sar_scene_struct *scene = dc->scene;
	sar_object_aircraft_struct *aircraft = NULL;
	sar_object_fueltank_struct *obj_fueltank_ptr = NULL;
	Boolean on_water = False, is_crashed = False;
	float	throttle = 1.0f,
		center_to_ground_height = 0.0f;
	sar_visual_model_struct *vmodel = obj_ptr->visual_model_shadow;

	switch(obj_ptr->type)
	{
	  case SAR_OBJ_TYPE_AIRCRAFT:
	    aircraft = SAR_OBJ_GET_AIRCRAFT(obj_ptr);
	    if(aircraft != NULL)
	    {
		center_to_ground_height =
		    aircraft->center_to_ground_height;
		throttle = aircraft->throttle;
		on_water = aircraft->on_water ? True : False;
		is_crashed = (aircraft->air_worthy_state == SAR_AIR_WORTHY_NOT_FLYABLE) ?
		    True : False;
	    }
	    break;
	  case SAR_OBJ_TYPE_FUELTANK:
	    obj_fueltank_ptr = SAR_OBJ_GET_FUELTANK(obj_ptr);
	    if(obj_fueltank_ptr != NULL)
	    {
/* TODO */
	    }
	    break;
	  default:
	    /* Calculate center to ground height for all other object
	     * types based on it's ground elevation and current
	     * position.
	     */
	    center_to_ground_height = obj_ptr->ground_elevation_msl -
		obj_ptr->pos.z;
	    break;
	}


	/* Draw prop wash if the object is an aircraft and it center
	 * to ground height is with in the shadow visibility
	 */
	if(!dc->flir && (aircraft != NULL) &&
	   (center_to_ground_height >= -SAR_SHADOW_VISIBILITY_HEIGHT) &&
	   (opt->textured_objects && opt->prop_wash)
	)
	{
	    GLenum shade_model_mode = state->shade_model_mode;
	    GLboolean depth_mask_flag = state->depth_mask_flag;
	    float z = center_to_ground_height;
	    int i, tex_num, frame_num;
	    const sar_obj_rotor_struct *rotor;
	    v3d_texture_ref_struct *t;
	    float r;

	    glPushMatrix();
	    {
		glTranslatef(0.0f, z, 0.0f);

		/* Set up GL states */
		StateGLEnable(state, GL_BLEND);
		StateGLDisable(state, GL_ALPHA_TEST);
		StateGLDepthMask(state, GL_FALSE);  
		StateGLEnable(state, GL_POLYGON_OFFSET_FILL);
		StateGLBlendFunc(
		    state, 
		    GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA
		);
		StateGLPolygonOffset(
		    state,
		    (GLfloat)opt->gl_polygon_offset_factor, -1.0f
		);

		/* Iterate through each rotor on the aircraft */
		for(i = 0; i < aircraft->total_rotors; i++)
		{
		    rotor = aircraft->rotor[i];
		    if(rotor == NULL)
			continue;

		    /* If the rotor is pitched forwards then it never
		     * has prop wash.
		     */
		    if((rotor->flags & SAR_ROTOR_FLAG_CAN_PITCH) &&
		       (rotor->flags & SAR_ROTOR_FLAG_PITCH_STATE)
		    )
			continue;

		    /* Get texture of prop wash and check if it is
		     * valid
		     *
		     * If it is not valid then it implies this
		     * rotor does not have prop wash
		     */
		    tex_num = rotor->wash_tex_num;
		    if(SARIsTextureAllocated(scene, tex_num))
			t = scene->texture_ref[tex_num];
		    else
			continue;

		    /* Get frame number */
		    frame_num = (int)CLIP(
			t->total_frames *
			SAR_GRAD_ANIM_COEFF(rotor->rotor_wash_anim_pos),
			0, t->total_frames - 1
		    );

		    /* Select propwash texture */
		    V3DTextureSelectFrame(t, frame_num);

		    /* Set color, alpha based on distance to ground
		     * divided by shadow visibility height and
		     * multiplied by both global rotor wash visibility
		     * coeff and the throttle value
		     */
		    glColor4f(
			1.0f, 1.0f, 1.0f,
			((1.0f + (float)(
			    center_to_ground_height / SAR_SHADOW_VISIBILITY_HEIGHT)
			) * opt->rotor_wash_vis_coeff * throttle)
		    );

		    /* Use flat shading */
		    StateGLShadeModel(state, GL_FLAT);

		    /* Calculate radius to be 4 times the rotor's size */
		    r = rotor->radius * 4.0f;

		    /* Draw prop wash */
		    glBegin(GL_QUADS);
		    {
			glNormal3f(0.0f, 1.0f, 0.0f);
			glTexCoord2f(0.0f, 1.0f - 0.0f);
			glVertex3f(-r, 0.0f, r);
			glTexCoord2f(1.0f, 1.0f - 0.0f);
			glVertex3f(r, 0.0f, r);
			glTexCoord2f(1.0f, 1.0f - 1.0f);
			glVertex3f(r, 0.0f, -r);
			glTexCoord2f(0.0f, 1.0f - 1.0f);
			glVertex3f(-r, 0.0f, -r);
		    }
		    glEnd();
		}

		/* Restore gl states */
		StateGLDisable(state, GL_POLYGON_OFFSET_FILL);
		StateGLDisable(state, GL_BLEND);
		StateGLDepthMask(state, depth_mask_flag);
		StateGLEnable(state, GL_ALPHA_TEST);
		StateGLShadeModel(state, shade_model_mode);
	    }
	    glPopMatrix();
	}

	/* Draw shadow using the visual model? */
	if((vmodel != NULL) &&
	   (center_to_ground_height >= -SAR_SHADOW_VISIBILITY_HEIGHT) &&
	   (on_water ? !is_crashed : True)
	)
	{
	    GLboolean lighting = display->state_gl.lighting;
	    GLboolean depth_mask_flag = display->state_gl.depth_mask_flag;
	    GLenum shade_model_mode = display->state_gl.shade_model_mode;
	    float z = center_to_ground_height;

	    /* Set up GL states */
	    StateGLDisable(state, GL_LIGHTING);
	    StateGLEnable(state, GL_BLEND);
	    StateGLDisable(state, GL_ALPHA_TEST);
	    StateGLDepthMask(state, GL_FALSE);
	    StateGLEnable(state, GL_POLYGON_OFFSET_FILL);
	    StateGLBlendFunc(
		state, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA
	    );
	    StateGLPolygonOffset(
		state, 
		(GLfloat)opt->gl_polygon_offset_factor, -1.0f
	    );
	    StateGLShadeModel(state, GL_FLAT);

	    /* Draw shadow */
	    glPushMatrix();
	    {
		glTranslatef(0.0f, z, 0.0f);
		glColor4f(0.0f, 0.0f, 0.0f, 0.5f);
		SARVisualModelCallList(vmodel);
	    }
	    glPopMatrix();

	    /* Restore GL states */
	    if(lighting)
		StateGLEnable(state, GL_LIGHTING);
	    StateGLDisable(state, GL_POLYGON_OFFSET_FILL);
	    StateGLDisable(state, GL_BLEND);
	    StateGLDepthMask(state, depth_mask_flag);
	    StateGLEnable(state, GL_ALPHA_TEST);
	    StateGLShadeModel(state, shade_model_mode);
	}
}


/*
 *	Draws scene foundations (ground base tiles).
 *
 *	Lighting should be turned off, the color from the global light
 *	color will be used as simulated lighting instead of gl lighting.
 *	Note that this will make the ground base darker than other
 *	ground objects (such as mountains, but this works out since
 *	the flat ground will be totally dark at night while the
 *	mountains will be barely visible).
 *
 *	Inputs assumed valid.
 */
static void SARDrawSceneFoundations(sar_dc_struct *dc)
{
	gw_display_struct *display = dc->display;
	const sar_option_struct *opt = dc->option;
	sar_scene_struct *scene = dc->scene;
	float aspect = dc->view_aspect;
	const sar_position_struct *cam_pos = &dc->camera_pos;
	Boolean in_map_view;
	long mod_offset_x, mod_offset_y;
	float fovx, fovz;	/* Field of view x and z, in radians */
	const sar_cloud_layer_struct *cloud_layer_ptr = NULL;
	sar_visual_model_struct *vmodel = NULL;
	GLfloat flir_c[3] = SAR_DEF_FLIR_GL_COLOR;


	/* Get field of views in radians */
	fovz = scene->camera_fovz;
	if(fovz <= 0.0f)
	    return;
	fovx = fovz * aspect;

	/* In map view? */
	in_map_view = (dc->camera_ref == SAR_CAMERA_REF_MAP) ? True : False;

	/* Get pointer to first (lowest) cloud layer */
	cloud_layer_ptr = dc->lowest_cloud_layer_ptr;

	/* Cloud layer valid and not in map view? */
	if((cloud_layer_ptr != NULL) && !in_map_view)
	{
	    /* If we are at or above the first cloud layer,
	     * then do not draw foundations since the would not
	     * be visable.
	     */
	    if(cam_pos->z >= cloud_layer_ptr->z)
		return;
	}

	/* Calculate module offset */
	mod_offset_x = (long)cam_pos->x %
	    (long)scene->base.tile_width;
	mod_offset_y = (long)cam_pos->y %
	    (long)scene->base.tile_height;

	glPushMatrix();
	{
	    /* Translate to new tiled position */
 	    glTranslatef(
		(GLfloat)((long)cam_pos->x - mod_offset_x),
		0.0f,
		(GLfloat)-((long)cam_pos->y - mod_offset_y)
	    );

	    /* Draw simple (solid color) base visual model */
	    vmodel = scene->base.visual_model_simple;

	    /* Is visual model valid and not in map view? */
	    if((vmodel != NULL) && !in_map_view)
	    {
		const sar_color_struct *base_c = &scene->base.color;
		const GLfloat *light_c = dc->flir ?
		    flir_c : dc->light_color;

		SAR_DRAW_TEXTURE_1D_OFF
		SAR_DRAW_TEXTURE_2D_OFF
		if(in_map_view)
		    glColor4f(base_c->r, base_c->g, base_c->b, 1.0f);
		else
		    glColor4f(
			base_c->r * light_c[0],
			base_c->g * light_c[1],
			base_c->b * light_c[2],
			1.0f
		    );
		SARVisualModelCallList(vmodel);
	    }

	    /* Draw close (textured) tiles */
	    vmodel = scene->base.visual_model_close;
	    if((vmodel != NULL) && opt->textured_ground)
	    {
		const GLfloat *light_c = dc->flir ?
		    flir_c : dc->light_color;

		SAR_DRAW_TEXTURE_2D_ON
		if(in_map_view)
		    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
		else
		    glColor4f(
			light_c[0],
			light_c[1],
			light_c[2],
			1.0f
		    );
		SARVisualModelCallList(vmodel);
	    }
	}
	glPopMatrix();
}

/*
 *	Draws the given cloud layer.
 *
 *	The winding will be flipped if flip is set to True, this is to
 *	make the cloud layer visible depending on if the camera is
 *	above or below it.
 *
 *	Inputs assumed valid.
 */
static void SARDrawCloudLayer(
	sar_dc_struct *dc,
	sar_cloud_layer_struct *cloud_layer, Boolean flip
)
{
	const sar_position_struct *cam_pos = &dc->camera_pos;
	long mod_offset_x, mod_offset_y;
	const GLfloat *c = dc->light_color;
	const GLfloat flir_c[3] = SAR_DEF_FLIR_SKY_GL_COLOR;
	sar_visual_model_struct *vmodel = cloud_layer->visual_model;
	if(vmodel == NULL)
	    return;

	mod_offset_x = (long)cam_pos->x %
	    (long)cloud_layer->tile_width;
	mod_offset_y = (long)cam_pos->y %
	    (long)cloud_layer->tile_height;

	/* Use flir color? */
	if(dc->flir)
	    c = flir_c;

	glPushMatrix();
	{
	    glTranslatef(
		(GLfloat)((long)cam_pos->x - mod_offset_x),
		(GLfloat)cloud_layer->z,
		(GLfloat)-((long)cam_pos->y - mod_offset_y)
	    );
	    if(flip)
		glRotatef(
		    180.0f, 0.0f, 0.0f, 1.0f
		);
	    glColor4f(c[0], c[1], c[2], 1.0f);
	    SARVisualModelCallList(vmodel);
	}
	glPopMatrix();
}

/*
 *	Called by SARDrawCloudBB() to draw one cloud billboard and its
 *	lightening (if any).
 */
static void SARDrawCloudBBIterate(
	sar_dc_struct *dc,
	const sar_position_struct *pos, float hw, float height,
	const GLfloat *c,
	v3d_texture_ref_struct *t,
	float lightening_intensity_coeff,
	const sar_position_struct *lightening_point,
	int lightening_points
)
{
	gw_display_struct *display = dc->display;
	state_gl_struct *state = &display->state_gl;
	const sar_position_struct *cam_pos = &dc->camera_pos;

	glColor4f(c[0], c[1], c[2], 1.0f);
	V3DTextureSelect(t);
	glPushMatrix();
	{
	    sar_direction_struct dir;

	    /* Get direction to face the camera */
	    SARDrawGetDirFromPos(cam_pos, pos, &dir);

	    /* Translate to the base of the cloud billboard and rotate
	     * in order to place the cloud billboard to be facing the
	     * camera
	     */
	    glTranslatef(
		pos->x, pos->z, -pos->y
	    );
	    if(dir.heading != 0)
		glRotatef(
		    (GLfloat)-SFMRadiansToDegrees(dir.heading),
		    0.0f, 1.0f, 0.0f
		);
#if 0
/* Do not pitch the billboard even if camera is pitched, it should be 
 * facing at a right angle with the horizon at all times
 */
	    if(dir.pitch != 0)
		glRotatef(
		    (GLfloat)-SFMRadiansToDegrees(dir.pitch),
		    1.0f, 0.0f, 0.0f
		);
#endif
	    /* Draw the Cloud Billboard */
	    if(!dc->flir)
	    {
		glBegin(GL_QUADS);
		{
		    glTexCoord2f(0.0f, 1.0f - 0.0f);
		    glVertex3f(-hw, 0.0f, 0.0f);
		    glTexCoord2f(1.0f, 1.0f - 0.0f);
		    glVertex3f(hw, 0.0f, 0.0f);
		    glTexCoord2f(1.0f, 1.0f - 1.0f);
		    glVertex3f(hw, height, 0.0f);
		    glTexCoord2f(0.0f, 1.0f - 1.0f);
		    glVertex3f(-hw, height, 0.0f);
		}
		glEnd();
	    }

	    /* Draw the Lightening */
	    if(lightening_intensity_coeff > 0.0f)
	    {
		StateGLBoolean	fog = state->fog,
				alpha_test = state->alpha_test,
				blend = state->blend;

		/* Turn off texturing and set lightening color */
		V3DTextureSelect(NULL);
		if(dc->flir)
		    glColor4f(
			1.0f * lightening_intensity_coeff,
			1.0f * lightening_intensity_coeff,
			1.0f * lightening_intensity_coeff,
			1.0f
		    );
		else
		    glColor4f(
			0.93f * lightening_intensity_coeff,
			0.75f * lightening_intensity_coeff,
			1.00f * lightening_intensity_coeff,
			1.0f
		    );

		/* Set up GL states */
		StateGLDisable(state, GL_FOG);
		StateGLDisable(state, GL_ALPHA_TEST);
		StateGLEnable(state, GL_BLEND);
		StateGLBlendFunc(
		    state,
		    GL_ONE, GL_ONE
		);
		StateGLEnable(state, GL_LINE_SMOOTH);
		StateGLLineWidth(state, 1.0f);

		/* Draw lightening */
		glBegin(GL_LINE_STRIP);
		{
		    int i;
		    const sar_position_struct *pt;
		    for(i = 0; i < lightening_points; i++)
		    {
			pt = &lightening_point[i];
			glVertex3f(pt->x, pt->z, -pt->y);
		    }
		}
		glEnd();

		/* Restore GL states */
		if(fog)
		    StateGLEnable(state, GL_FOG);
		if(alpha_test)
		    StateGLEnable(state, GL_ALPHA_TEST);
		if(!blend)
		    StateGLDisable(state, GL_BLEND);
		StateGLDisable(state, GL_LINE_SMOOTH);
		StateGLLineWidth(state, 1.0f);
	    }
	}
	glPopMatrix();
}

/*
 *	Draws Cloud Billboards.
 *
 *      Inputs assumed valid.
 */
static void SARDrawCloudBB(
	sar_dc_struct *dc,
	const sar_cloud_bb_struct *cloud_bb_ptr
)
{
	gw_display_struct *display = dc->display;
	state_gl_struct *state = &display->state_gl;
	sar_scene_struct *scene = dc->scene;
	const sar_position_struct *cam_pos = &dc->camera_pos;
	int tex_num = cloud_bb_ptr->tex_num;
	StateGLBoolean alpha_test = state->alpha_test;
	sar_position_struct pos;
	long	tile_width = (long)cloud_bb_ptr->tile_width,
		tile_height = (long)cloud_bb_ptr->tile_height;
	float	hw = cloud_bb_ptr->width / 2,
		hh = cloud_bb_ptr->height / 2;
	const sar_position_struct *lightening_point = cloud_bb_ptr->lightening_point;
	int lightening_points = SAR_LIGHTENING_POINTS_MAX;
	float ligntening_intensity_coeff;
	v3d_texture_ref_struct *t;
	const GLfloat *c = dc->light_color;

	/* Get cloud billboard texture */
	if(SARIsTextureAllocated(scene, tex_num))
	    t = scene->texture_ref[tex_num];
	else
	    return;

	if((tile_width <= 0) || (tile_height <= 0) ||
	   (hw < 1.0f) || (hh < 1.0f)
	)
	    return;

	/* Calculate the initial tiling position relative to the
	 * camera's position modulus the tiling dimension
	 */
	if(cam_pos->x < 0.0f)
	    pos.x = cam_pos->x - (float)((long)cam_pos->x % tile_width)
		- (float)tile_width + cloud_bb_ptr->pos.x;
	else
	    pos.x = cam_pos->x - (float)((long)cam_pos->x % tile_width)
		+ cloud_bb_ptr->pos.x;

	if(cam_pos->y < 0.0f)
	    pos.y = cam_pos->y - (float)((long)cam_pos->y % tile_height)
		- (float)tile_height + cloud_bb_ptr->pos.y;
	else
	    pos.y = cam_pos->y - (float)((long)cam_pos->y % tile_height)
		+ cloud_bb_ptr->pos.y;

	/* Z position is just the Z base position of the cloud */
	pos.z = cloud_bb_ptr->pos.z;


	/* Calculate lightening intensity */
	if(cloud_bb_ptr->lightening_next_off > 0l)
	{
	    time_t	dt = cloud_bb_ptr->lightening_next_off -
		cloud_bb_ptr->lightening_started_on,
			t = cur_millitime -
		cloud_bb_ptr->lightening_started_on;
	    const float threshold = 0.3f;
	    float coeff = (dt > 0) ? ((float)t / (float)dt) : 0.0f;
	    ligntening_intensity_coeff = (coeff > threshold) ?
		(1.0f - ((coeff - threshold) * (1.0f / (1.0f - threshold)))) :
		(coeff * (1.0f / threshold));
	}
	else
	{
	    ligntening_intensity_coeff = 0.0f;
	}

	/* Set up GL states */
	StateGLEnable(state, GL_ALPHA_TEST);
	StateGLAlphaFunc(state, GL_GREATER, 0.5);

	/* Begin drawing cloud billboards */

#define DRAW_CLOUD	{		\
 SARDrawCloudBBIterate(			\
  dc, &pos, hw, cloud_bb_ptr->height,	\
  c, t,					\
  ligntening_intensity_coeff,		\
  lightening_point, lightening_points	\
 );					\
}
	/* Draw center oriented tile */
	DRAW_CLOUD

	/* Upper */
	pos.y += tile_height;
	DRAW_CLOUD

	/* Upper right */
	pos.x += tile_width;
	DRAW_CLOUD
	/* Right */
	pos.y -= tile_height;
	DRAW_CLOUD
	/* Lower right */
	pos.y -= tile_height;
	DRAW_CLOUD

	/* Lower */
	pos.x -= tile_width;
	DRAW_CLOUD
	/* Lower left */
	pos.x -= tile_width;
	DRAW_CLOUD
	/* Left */
	pos.y += tile_height;
	DRAW_CLOUD

	/* Upper left */
	pos.y += tile_height;
	DRAW_CLOUD
#undef DRAW_CLOUD

	/* Restore GL states */
	if(alpha_test)
	    StateGLEnable(state, GL_ALPHA_TEST);
	else
	    StateGLDisable(state, GL_ALPHA_TEST);
}

/*
 *	Draws horizon quads, translation needs to be at scene
 *	origin.
 *
 *	Inputs assumed valid and depth test and lighting turned off.
 */
static void SARDrawSceneHorizon(sar_dc_struct *dc)
{
	gw_display_struct *display = dc->display;
	state_gl_struct *state = &display->state_gl;
	sar_scene_struct *scene = dc->scene;
	float visibility_max = dc->visibility_max;
	const sar_position_struct *cam_pos = &dc->camera_pos;
	GLboolean depth_mask_flag = state->depth_mask_flag;
	GLenum shade_model_mode = state->shade_model_mode;
	Boolean afternoon = False;
	int	i, n,
		total_horizon_textures = 7,
		total_horizon_panels = 12;
	float h_min, h_max, r;
	const GLfloat *c = dc->light_color;
	const sar_scene_horizon_struct *horizon = &scene->horizon;
	const sar_cloud_layer_struct *cloud_layer = NULL;
	const GLfloat flir_c[3] = SAR_DEF_FLIR_SKY_GL_COLOR;


	/* Match cloud layer just above camera */
	for(i = 0; i < scene->total_cloud_layers; i++)
	{
	    cloud_layer = scene->cloud_layer[i];
	    if(cloud_layer == NULL)
		continue;

	    if(cloud_layer->z > cam_pos->z)
		break;
	    else
		cloud_layer = NULL;
	}


	/* Check if horizon has all 7 textures allocated */
	if(horizon->total_textures < total_horizon_textures)
	{
	    /* Not enough textures allocated */
	    return;
	}

	/* Calculate horizon radius r, this is the distance from camera
	 * to each horizon panel
	 *
	 * Note that we make this distance a bit shorter than the
	 * maximum visibility to ensure that the horizon panels are
	 * drawn closer than the camera's far clip
	 */
	r = visibility_max * 0.85f;

	/* Calculate height minimum, always 0.0 */
	h_min = 0.0f;

	/* Calculate height maximum, dependant of cloud layer existance.
	 * If a cloud layer exists then it will be 1.05 times the cloud
	 * layer's altitude (we use 1.05 to ensure that it goes up into
	 * the clouds just a bit). Otherwise it will be calculated by
	 * s = r * theta where theta is (0.25 * PI).
	 */
	if(cloud_layer != NULL)
	    h_max = (float)(cloud_layer->z * 1.05);	/* Add a bit of margin */
	else
	    h_max = (float)MAX(r * (0.25 * PI), cam_pos->z);

	/* Make note if it is afternoon or later */
	if(scene->tod > (12.0f * 3600.0f))
	    afternoon = True;

	/* Use ifr color? */
	if(dc->flir)
	    c = flir_c;

	SAR_DRAW_TEXTURE_1D_ON
	SAR_DRAW_TEXTURE_2D_OFF

	/* Set up gl states */
	StateGLShadeModel(state, GL_FLAT);
	StateGLDepthMask(state, GL_FALSE);

	glPushMatrix();
	{
	    /* Angle in radians of the current angle from camera to the
	     * the horizon panel's `previous' point
	     */
	    float theta;

	    /* Angle in radians of normal */
	    float n_theta;

	    /* Deltas of each panel point along xy plane where index 0
	     * is the `previous point' going clockwise from 9 o'clock
	     */
	    float dx[2], dy[2];


	    /* Translate to camera position but at z = 0.0 */
	    glTranslatef(cam_pos->x, 0.0f, -cam_pos->y);

	    /* Set the color of the horizon */
	    glColor4f(c[0], c[1], c[2], 1.0f);

	    /* Initialize theta */
	    theta = (float)SFMSanitizeRadians(
		(1.5 * PI) -
		(((2.0 * PI) / total_horizon_panels) / 2)
	    );

	    /* Initialize `previous point' for the panel delta offset */
	    dx[0] = (float)(r * sin(theta));
	    dy[0] = (float)(r * cos(theta));

	    /* Begin drawing each horizon panel
	     *
	     * Iterate through horizon panels, note that this more than
	     * the actual number of horizon textures so one texture may 
	     * be used more than once
	     */
	    for(i = 0; i < total_horizon_panels; i++)
	    {
		/* Increment theta to `next' angle first */
		theta = (float)SFMSanitizeRadians(
		    theta +
		    ((2.0 * PI) / total_horizon_panels)
		);

		/* Calculate `next' panel point */
		dx[1] = (float)(r * sin(theta));
		dy[1] = (float)(r * cos(theta));


		/* Calculate horizon texture index based on horizon panel
		 * index. Remember that there are more horizon panels than
		 * horizon textures.
		 */
		if(i >= total_horizon_textures)
		    n = total_horizon_textures -
			i + total_horizon_textures - 2;
		else
		    n = i;
		/* Flip texture index n if time is before noon */
		if(!afternoon)
		    n = total_horizon_textures - n - 1;
		/* Clip */
		if((n >= 0) && (n < total_horizon_textures))
		    V3DTextureSelect(horizon->texture[n]);

		/* Calculate normal theta */
		n_theta = (float)SFMSanitizeRadians(
		    theta -
		    (((2.0 * PI) / total_horizon_panels) / 2) +
		    (1.0 * PI)
		);

		/* Begin drawing this horizon panel as a GL_QUADS */
		glBegin(GL_QUADS);
		/* No lighting so no normal needed */

		glTexCoord2f(1.0f, 0.0f);
		glVertex3f(
		    dx[0], h_min, -dy[0]
		);
		glTexCoord2f(1.0f, 0.0f);
		glVertex3f(
		    dx[1], h_min, -dy[1]
		);
		glTexCoord2f(0.0f, 0.0f);
		glVertex3f(
		    dx[1], h_max, -dy[1]
		);
		glTexCoord2f(0.0f, 0.0f);
		glVertex3f(
		    dx[0], h_max, -dy[0]
		);
		glEnd();
		/* Done drawing this horizon panel */

		/* Set next panel point to previous panel point */
		dx[0] = dx[1];
		dy[0] = dy[1];

	    }	/* Begin drawing each horizon panel */
	}
	glPopMatrix();

	/* Restore gl states */
	StateGLDepthMask(state, depth_mask_flag);
	StateGLShadeModel(state, shade_model_mode);
}

/*
 *	Draws additive celestial objects, like the sun and moon.
 *
 *	These are drawn over cloud cover, so clouds and horizon must be
 *	drawn before calling this function.
 */
static void SARDrawSceneCelestialAdditive(sar_dc_struct *dc)
{
	gw_display_struct *display = dc->display;
	state_gl_struct *state = &display->state_gl;
	sar_scene_struct *scene = dc->scene;
	float visibility_max = dc->visibility_max;
	float aspect = dc->view_aspect;
	float fovz_um = dc->fovz_um;
	const sar_position_struct *cam_pos = &dc->camera_pos;
	GLboolean	depth_mask_flag = state->depth_mask_flag,
			alpha_test = state->alpha_test,
			blend = state->blend,
			fog = state->fog;
	GLenum shade_model_mode = state->shade_model_mode;
	int tex_num;
	v3d_texture_ref_struct *t;
	sar_position_struct *pos, sun_pos, moon_pos;

	/* Depth testing and lighting should already be off prior to
	 * calling this function
	 */

	/* Turn on texturing */
	SAR_DRAW_TEXTURE_1D_OFF
	SAR_DRAW_TEXTURE_2D_ON

	/* Set up GL states */
	StateGLDisable(state, GL_FOG);
	StateGLDisable(state, GL_ALPHA_TEST);
	StateGLEnable(state, GL_BLEND);
	StateGLBlendFunc(
	    state,
	    GL_ONE, GL_ONE
	);
	StateGLShadeModel(state, GL_FLAT);
	StateGLDepthMask(state, GL_FALSE);


	/* Begin drawing sun */
	pos = &sun_pos;

	/* Get texture */
	tex_num = scene->texnum_sun;
	t = (SARIsTextureAllocated(scene, tex_num) ?
	    scene->texture_ref[tex_num] : NULL
	);
	if((t != NULL) && (pos != NULL) &&
	   (scene->light_visibility_hint == 1)
	)
	{
	    sar_direction_struct to_dir;
	    float rx, ry, dy, z_coeff;
	    /* Distance must be a bit closer than maximum visibility
	     * so that it does not get clipped
	     */
	    float distance = visibility_max * 0.85f;

	    /* Calculate the radius based on the field of view and
	     * having 10% of the object visible (10% is how big the
	     * object is)
	     */
	    rx = ry = fovz_um * distance * 0.10f;
	    dy = 2.0f * ry;

	    /* Get sun's position (do not use primary_light_pos as it
	     * is offsetted from the camera's z base instead of the
	     * scene's z base, note that pos->z is the base of the
	     * object, not center and is calculated as a scalar with
	     * the camera's z position
	     *
	     * The scene's light_pos is a unit offset so we multiply
	     * each compoent by distance
	     */
	    pos->x = cam_pos->x + (scene->light_pos.x * distance);
	    pos->y = cam_pos->y + (scene->light_pos.y * distance);
	    pos->z = (scene->light_pos.z * (cam_pos->z + distance)) - dy;

	    /* Calculate the z coefficient, how high the object is
	     * relative to its highest point (where 1.0 is highest
	     * and 0.0 is lowest).
	     */
	    z_coeff = (float)CLIP(scene->light_pos.z, 0.0, 1.0);

	    /* Is there at least some of object showing above the
	     * foundation level?
	     */
	    if((pos->z + dy) > 0.0f)
	    {
		float dy_orig = dy;
		float tc_y;

		if(pos->z < 0.0f)
		{
		    dy -= 0.0f - pos->z;
		    pos->z = 0.0f;
		}

		/* Calculate y position of lower bound texture
		 * coordinates
		 */
		tc_y = (float)(
		    1.0f - ((dy_orig > 0.0f) ? (dy / dy_orig) : 1.0f)
		);

		glPushMatrix();
		{
		    V3DTextureSelect(t);	/* Select texture */

		    /* Set brightness (color) of object */
		    if(dc->flir)
		    {
			const sar_color_struct *c = &scene->sun_high_color;
			float g = (float)((c->r + c->g + c->b) / 3.0);
			if(g > 0.5f)
			{
			    g = (g - 0.5f) / 0.5f;
			    glColor4f(g, 1.0f, g, 1.0f);
			}
			else
			{
			    g = g / 0.5f;
			    glColor4f(0.0f, g, 0.0f, 1.0f);
			}
		    }
		    else
		    {
			glColor4f(
			    (GLfloat)MIN(
				scene->sun_high_color.r * z_coeff +
				scene->sun_low_color.r * (1.0 - z_coeff),
				1.0
			    ),
			    (GLfloat)MIN(
				scene->sun_high_color.g * z_coeff +
				scene->sun_low_color.g * (1.0 - z_coeff),
				1.0
			    ),
			    (GLfloat)MIN(
				scene->sun_high_color.b * z_coeff +
				scene->sun_low_color.b * (1.0 - z_coeff),
				1.0
			    ),
			    1.0f
			);
		    }

		    /* Translate */
		    glTranslatef(pos->x, pos->z, -pos->y);

		    /* Adjust heading to face camera */
		    SARDrawGetDirFromPos(cam_pos, pos, &to_dir);
		    if(to_dir.heading != 0)
			glRotatef(
			    (float)-SFMRadiansToDegrees(to_dir.heading),
			    0.0f, 1.0f, 0.0f
			);
		    if(to_dir.pitch != 0)
			glRotatef(
			    (float)-SFMRadiansToDegrees(to_dir.pitch),
			    1.0f, 0.0f, 0.0f
			);

		    /* Draw sun quad */
		    glBegin(GL_QUADS);
		    {
			glTexCoord2f(0.0f, 1.0f - tc_y);
			glVertex3f(-rx, 0.0f, 0.0f);
			glTexCoord2f(1.0f, 1.0f - tc_y);
			glVertex3f(rx, 0.0f, 0.0f);
			glTexCoord2f(1.0f, 1.0f - 1.0f);
			glVertex3f(rx, dy, 0.0f);
			glTexCoord2f(0.0f, 1.0f - 1.0f);
			glVertex3f(-rx, dy, 0.0f);
		    }
		    glEnd();

/*
printf("\r%.2f %.2f %.2f | %.2f %.2f %.2f | %.2f %.2f %.2f",
 pos->x, pos->y, pos->z,
 scene->light_pos.x, scene->light_pos.y, scene->light_pos.z,
 cam_pos->x, cam_pos->y, cam_pos->z
);
fflush(stdout);
 */
		}
		glPopMatrix();
	    }
	}


	/* Begin drawing moon */
	pos = &moon_pos;

	/* Get texture */
	tex_num = scene->texnum_moon;
	t = SARIsTextureAllocated(scene, tex_num) ?
	    scene->texture_ref[tex_num] : NULL;
	if((t != NULL) && (pos != NULL) && scene->moon_visibility_hint)
	{
	    sar_direction_struct to_dir;
	    float rx, ry, dy, z_coeff;
	    /* Distance must be a bit closer than maximum visibility
	     * so that it does not get clipped
	     */
	    float distance = visibility_max * 0.85f;

	    /* Calculate the radius based on the field of view and
	     * having 5% of the object visible (5% is how big the
	     * object is)
	     */
	    rx = ry = fovz_um * distance * 0.05f;
	    dy = 2.0f * ry;

	    /* Get moon's position, the position is a unit offset so
	     * each compoent needs to be multiplied by the distance
	     */
	    pos->x = cam_pos->x + (scene->moon_pos.x * distance);
	    pos->y = cam_pos->y + (scene->moon_pos.y * distance);
	    pos->z = (scene->moon_pos.z * (cam_pos->z + distance)) - dy;

	    /* Calculate the z coefficient, how high the object is
	     * relative to its highest point (where 1.0 is highest and
	     * 0.0 is lowest)
	     */
	    z_coeff = (float)CLIP(scene->moon_pos.z, 0.0, 1.0);

	    /* Is there at least some of object showing above the
	     * ground level?
	     */
	    if((pos->z + dy) > 0.0f)
	    {
		float dy_orig = dy;
		float tc_y;

		if(pos->z < 0.0f)
		{
		    dy -= 0.0f - pos->z;
		    pos->z = 0.0f;
		}

		/* Calculate y position of the lower bound texture
		 * coordinates
		 */
		tc_y = (float)(
		    1.0f - ((dy_orig > 0.0f) ? dy / dy_orig : 1.0f)
		);

		glPushMatrix();
		{
		    V3DTextureSelect(t);	/* Select texture */

		    if(dc->flir)
		    {
			const sar_color_struct *c = &scene->moon_high_color;
			float g = (float)((c->r + c->g + c->b) / 3.0);
			if(g > 0.5f)
			{
			    g = (g - 0.5f) / 0.5f;
			    glColor4f(g, 1.0f, g, 1.0f);
			}
			else
			{
			    g = g / 0.5f;
			    glColor4f(0.0f, g, 0.0f, 1.0f);

			}
		    }
		    else
		    {
			glColor4f(
			    (GLfloat)MIN(
				scene->moon_high_color.r * z_coeff +
				scene->moon_low_color.r * (1.0 - z_coeff),
				1.0
			    ),
			    (GLfloat)MIN(
				scene->moon_high_color.g * z_coeff +
				scene->moon_low_color.g * (1.0 - z_coeff),
				1.0
			    ),
			    (GLfloat)MIN(
				scene->moon_high_color.b * z_coeff +
				scene->moon_low_color.b * (1.0 - z_coeff),
				1.0
			    ),
			    1.0
			);
		    }

		    /* Translate */
		    glTranslatef(pos->x, pos->z, -pos->y);

		    /* Adjust heading to face camera */
		    SARDrawGetDirFromPos(cam_pos, pos, &to_dir);
		    if(to_dir.heading != 0)
			glRotatef(
			    (float)-SFMRadiansToDegrees(to_dir.heading),
			    0.0f, 1.0f, 0.0f
			);
		    if(to_dir.pitch != 0)
			glRotatef(
			    (float)-SFMRadiansToDegrees(to_dir.pitch),
			    1.0f, 0.0f, 0.0f
			);

		    /* Draw moon quad */
		    glBegin(GL_QUADS);
		    {
			glTexCoord2f(0.0f, 1.0f - tc_y);
			glVertex3f(-rx, 0.0f, 0.0f);
			glTexCoord2f(1.0f, 1.0f - tc_y);
			glVertex3f(rx, 0.0f, 0.0f);
			glTexCoord2f(1.0f, 1.0f - 1.0f);
			glVertex3f(rx, dy, 0.0f);
			glTexCoord2f(0.0f, 1.0f - 1.0f);
			glVertex3f(-rx, dy, 0.0f);
		    }
		    glEnd();
		}
		glPopMatrix();
	    }
	}

	/* Calculate sun's light glare position within the camera's
	 * view
	 */
/* Disable this for now, since its rather cpu costly.
	scene->light_xc = -2.0f;
	scene->light_yc = -2.0f;

	pos = &sun_pos;
	if((pos != NULL) && (scene->light_visibility_hint == 1))

 */
	if(False)
	{
	    int i, n_matrix;
	    double a[3 * 1], r[3 * 1];

	    /* Calculate sun's relativity to the orthogonal camera
	     * view, the position must be an offset from the camera
	     */
	    a[0] = pos->x;
	    a[1] = pos->y;
	    a[2] = pos->z;

	    /* Rotate position based on camera's rotation */
	    for(i = 0,
		n_matrix = MIN(scene->camera_rotmatrix_count,
		    SAR_CAMERA_ROTMATRIX_MAX);
		i < n_matrix;
		i++
	    )
	    {
		MatrixMulti3Rotate33(
		    a,				/* 3 * 1 */
		    scene->camera_rotmatrix[i],	/* 3 * 3 */
		    r				/* 3 * 1 */
		);
		memcpy(a, r, (3 * 1) * sizeof(double));
	    }

	    /* In front of camera (y value positive)? */
	    if(a[1] > 0.0f)
	    {
		/* Calculate orthogonal view position coefficient */
		float xc, zc;
		float	half_edgez = (float)(fovz_um * a[1] / 2.0f),
			half_edgex = half_edgez * aspect;

		if(half_edgez > 0.0f)
		    zc = (float)(a[2] / half_edgez);
		else
		    zc = 2.0f;		/* Off screen */

		if(half_edgex > 0.0f)
		    xc = (float)(a[0] / half_edgex);
		else
		    xc = 2.0f;		/* Off screen */

		/* Coefficients xc and zc now represent the sun's
		 * position relative to the camera's eye center, if
		 * values are out of the range of [-1.0, 1.0] it implies
		 * that the sun's position is out of the camera's view
		 */
		if((zc <= 1.0f) && (zc >= -1.0f) &&
		   (xc <= 1.0f) && (xc >= -1.0f)
		)
		{
#if 0
printf("\r Result %.0f %.0f %.0f | %.2f %.2f",
 a[0], a[1], a[2], xc, zc
); fflush(stdout);
#endif

		    scene->light_xc = xc;
		    scene->light_yc = zc;

		}
	    }
	}

	/* Restore GL states */
	if(alpha_test)
	    StateGLEnable(state, GL_ALPHA_TEST);
	if(!blend)
	    StateGLDisable(state, GL_BLEND);
	if(fog)
	    StateGLEnable(state, GL_FOG);
	StateGLDepthMask(state, depth_mask_flag);
	StateGLShadeModel(state, shade_model_mode);
}



/*
 *	Redraws scene.
 */
void SARDraw(sar_core_struct *core_ptr)
{
	int i, n, *total;
	int width, height;
	float fovz_um, view_aspect;
	Boolean need_clear_color = True;
	gw_display_struct *display;
	state_gl_struct *state;
	sar_scene_struct *scene;
	sar_object_struct ***ptr, *obj_ptr;
	gctl_struct *gc;
	const sar_direction_struct *dir;
	const sar_position_struct *pos;
	sar_object_aircraft_struct *aircraft;
	sar_object_helipad_struct *obj_helipad_ptr;
	sar_object_runway_struct *obj_runway_ptr;
	sar_object_human_struct *obj_human_ptr;
	sar_obj_part_struct *part;
	sar_obj_rotor_struct *rotor;
	sar_external_fueltank_struct *eft_ptr;
	sar_obj_hoist_struct *hoist_ptr;
	sar_cloud_layer_struct *cloud_layer_ptr;
	sar_cloud_bb_struct *cloud_bb_ptr;
	float far_model_range, distance, distance3d, visibility_max;
	GLfloat light_val[4], mat_val[4];
	const sar_option_struct *opt = &core_ptr->option;
	sar_dc_struct _dc, *dc;


	/* Reset drawing context */
	dc = &_dc;
	memset(dc, 0x00, sizeof(sar_dc_struct));
	dc->core_ptr = core_ptr;
	dc->option = &core_ptr->option;
	dc->scene = scene = core_ptr->scene;
	dc->object = core_ptr->object;
	dc->total_objects = core_ptr->total_objects;
	ptr = &core_ptr->object;
	total = &core_ptr->total_objects;
	dc->display = display = core_ptr->display;
	dc->gctl = gc = core_ptr->gctl;

	dc->camera_in_cockpit = False;
	dc->ear_in_cockpit = False;
	dc->flir = core_ptr->flir;
	dc->camera_ref = SAR_CAMERA_REF_COCKPIT;
	dc->map_dxm = 0.0f;
	dc->map_dym = 0.0f;

	dc->lowest_cloud_layer_ptr = NULL;
	dc->highest_cloud_layer_ptr = NULL;
	dc->player_obj_cockpit_ptr = NULL;
	dc->player_flight_model_type = -1;
	dc->player_wheel_brakes = 0;
	dc->player_air_brakes = False;
	dc->player_autopilot = False;
	dc->player_stall = False;
	dc->player_overspeed = False;

	if((display == NULL) || (scene == NULL))
	    return;

	state = &display->state_gl;

	GWContextGet(
	    display, GWContextCurrent(display),
	    NULL, NULL,
	    NULL, NULL,
	    &width, &height
	);
	dc->width = width;
	dc->height = height;
	if((width <= 0) || (height <= 0))
	    return;

	/* If the camera reference is map then we need to call a
	 * different function to do the drawing instead
	 */
	if(scene->camera_ref == SAR_CAMERA_REF_MAP)
	{
	    SARDrawMap(core_ptr, False, False, -1);
	    return;
	}

	/* Calculate maximum clipping visibility */
	dc->visibility_max = visibility_max = (float)MAX(
	    SFMMilesToMeters(3 + (opt->visibility_max * 3)),
	    100.0
	);

	/* Reset viewport */
	SARReshapeCB(0, core_ptr, -1, -1, -1, -1);

	/* Calculate view aspect ratio with aspect offset applied, the
	 * width and height should now be up to date from the above
	 * viewport reset
	 */
	dc->view_aspect = view_aspect = ((float)width / (float)height) +
	    display->aspect_offset;

	/* Calculate field of view in unit meters about the z axis, this
	 * is a special coefficient that when used can give the width of
	 * the viewport at any given distance away from it
	 *
	 * Where: width_of_view = distance_away * fovz_um
	 */
	dc->fovz_um = fovz_um = (float)tan(scene->camera_fovz / 2.0) * 2.0f;

	/* Reset projection matrix */
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(
	    SFMRadiansToDegrees(scene->camera_fovz),	/* Field of view */
	    view_aspect,
	    0.8,
	    visibility_max
	);
	/* Reset model view matrix */
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();


	/* Set camera location, this will set camera position variables
	 * for this source file
	 */
	SARSetCamera(dc);


	/* Get pointers to lowest (first) and highest (last) cloud
	 * layers on the scene
	 */
	i = scene->total_cloud_layers;
	if(i > 0)
	{
	    dc->lowest_cloud_layer_ptr = scene->cloud_layer[0];
	    dc->highest_cloud_layer_ptr = scene->cloud_layer[i - 1];
	}

	/* Get scene light color */
	if(dc->flir)
	{
	    /* FLIR is on, so the scene light color as the FLIR color */
	    const GLfloat c[3] = SAR_DEF_FLIR_GL_COLOR;
	    memcpy(dc->light_color, c, 3 * sizeof(GLfloat));
	}
	else
	{
	    /* Set the light color as the Primary Light Source color
	     * plus the Primary Lightening
	     */
	    const sar_color_struct *c = &scene->light_color;
	    const float pri_lightening_coeff = scene->pri_lightening_coeff;
	    dc->light_color[0] = MIN(c->r + pri_lightening_coeff, 1.0f);
	    dc->light_color[1] = MIN(c->g + pri_lightening_coeff, 1.0f);
	    dc->light_color[2] = MIN(c->b + pri_lightening_coeff, 1.0f);
	}

	/* Atmosphere enabled? */
	if(opt->atmosphere)
	{
	    /* Atmosphere is enabled, set up atmosphere and sky color */
	    GLfloat fog_c[4];

	    /* Highest cloud layer exists? */
	    if(dc->highest_cloud_layer_ptr != NULL)
	    {
		/* Camera below highest cloud layer? */
		if(dc->camera_pos.z < dc->highest_cloud_layer_ptr->z)
		{
		    /* Below the highest cloud layer, set the sky color
		     * to the nominal sky color
		     */
		    dc->sky_color[0] = (GLfloat)(
			scene->sky_nominal_color.r *
			POW(dc->light_color[0], 0.5)
		    );
		    dc->sky_color[1] = (GLfloat)(
			scene->sky_nominal_color.g *
			dc->light_color[1]
		    );
		    dc->sky_color[2] = (GLfloat)(
			scene->sky_nominal_color.b *
			dc->light_color[2]
		    );

		    /* If textured clouds is enabled, if so then do not
		     * clear the color buffer since drawing the cloud
		     * layers, horizon, and ground base will ensure that
		     * the frame buffer is effectivly cleared
		     */
		    if(opt->textured_clouds)
			need_clear_color = False;
		}
		else
		{
		    /* Above the highest cloud layer, set the sky
		     * color to be darker than the nominal sky color
		     */
		    dc->sky_color[0] = (GLfloat)(
			scene->sky_nominal_color.r * 0.15 *
			dc->light_color[0]
		    );
		    dc->sky_color[1] = (GLfloat)(
			scene->sky_nominal_color.g * 0.15 *
			dc->light_color[1]
		    );
		    dc->sky_color[2] = (GLfloat)(
			scene->sky_nominal_color.b * 0.15 *
			dc->light_color[2]
		    );
		}
	    }
	    else
	    {
		/* No cloud layers exist, so set the sky color to the
		 * nominal sky color
		 */
		dc->sky_color[0] = (GLfloat)(
		    scene->sky_nominal_color.r *
		    POW(dc->light_color[0], 0.5)
		);
		dc->sky_color[1] = (GLfloat)(
		    scene->sky_nominal_color.g *
		    dc->light_color[1]
		);
		dc->sky_color[2] = (GLfloat)(
		    scene->sky_nominal_color.b *
		    dc->light_color[2]
		);
	    }

	    /* Set the atmosphere color to be slightly brighter than
	     * the sky color calculated above
	     */
	    dc->atmosphere_color[0] = (GLfloat)MIN(
		dc->sky_color[0] * 1.1, 1.0
	    );
	    dc->atmosphere_color[1] = (GLfloat)MIN(
		dc->sky_color[1] * 1.1, 1.0
	    );
	    dc->atmosphere_color[2] = (GLfloat)MIN(
		dc->sky_color[2] * 1.1, 1.0
	    );

	    /* Set the glFog color */
	    if(dc->flir)
	    {
		GLfloat c[3] = SAR_DEF_FLIR_SKY_GL_COLOR;
		fog_c[0] = c[0];
		fog_c[1] = c[1];
		fog_c[2] = c[2];
		fog_c[3] = 1.0f;
	    }
	    else
	    {
		fog_c[0] = dc->atmosphere_color[0];
		fog_c[1] = dc->atmosphere_color[1];
		fog_c[2] = dc->atmosphere_color[2];
		fog_c[3] = 1.0f;
	    }
	    /* Enable glFog */
	    StateGLEnable(state, GL_FOG);
	    glFogi(GL_FOG_MODE, GL_EXP);
	    glFogfv(GL_FOG_COLOR, fog_c);
	    glFogf(
		GL_FOG_DENSITY,
		(GLfloat)(
		    (1.0 / visibility_max) *
		    (8.0 * scene->atmosphere_density_coeff)
		)
	    );
	    glHint(GL_FOG_HINT, GL_DONT_CARE);
	    glFogf(
		GL_FOG_START,
		(GLfloat)(
		    visibility_max *
		    (1.0 - scene->atmosphere_dist_coeff)
		)
	    );
	    glFogf(GL_FOG_END, (GLfloat)visibility_max);

	    /* Set the glClearColor */
	    if(dc->flir)
	    {
		GLfloat c[3] = SAR_DEF_FLIR_SKY_GL_COLOR;
		glClearColor(
		    (GLclampf)c[0],
		    (GLclampf)c[1],
		    (GLclampf)c[2],
		    (GLclampf)0.0f
		);
	    }
	    else
	    {
		glClearColor(
		    (GLclampf)dc->atmosphere_color[0],
		    (GLclampf)dc->atmosphere_color[1],
		    (GLclampf)dc->atmosphere_color[2],
		    (GLclampf)0.0f
		);
	    }
	}
	else
	{
	    /* Atmosphere is disabled */

	    /* Disable fog */
	    StateGLDisable(state, GL_FOG);

	    /* Highest cloud layer exists? */
	    if(dc->highest_cloud_layer_ptr != NULL)
	    {
		/* Camera below the highest cloud layer? */
		if(dc->camera_pos.z < dc->highest_cloud_layer_ptr->z)
		{
		    /* Below the highest cloud layer, set the sky color
		     * to the nominal sky color
		     */
		    dc->sky_color[0] = (GLfloat)(
			scene->sky_nominal_color.r * dc->light_color[0]
		    );
		    dc->sky_color[1] = (GLfloat)(
			scene->sky_nominal_color.g * dc->light_color[1]
		    );
		    dc->sky_color[2] = (GLfloat)(
			scene->sky_nominal_color.b * dc->light_color[2]
		    );

		    /* If textured clouds is enabled, if so then do not
		     * clear the color buffer since drawing the cloud
		     * layers, horizon, and ground base will ensure that
		     * the frame buffer is effectivly cleared
		     */
		    if(opt->textured_clouds)
			need_clear_color = False;
		}
		else
		{
		    /* Above the highest cloud layer, set the sky
		     * color to be darker than the nominal sky color
		     */
		    dc->sky_color[0] = (GLfloat)(
			scene->sky_nominal_color.r * 0.15 *
			dc->light_color[0]
		    );
		    dc->sky_color[1] = (GLfloat)(
			scene->sky_nominal_color.g * 0.15 *
			dc->light_color[1]
		    );
		    dc->sky_color[2] = (GLfloat)(
			scene->sky_nominal_color.b * 0.15 *
			dc->light_color[2]
		    );
		}
	    }
	    else
	    {
		/* No cloud layers exist, so set the sky color to the
		 * nominal sky color
		 */
		dc->sky_color[0] = (GLfloat)(
		    scene->sky_nominal_color.r * dc->light_color[0]
		);
		dc->sky_color[1] = (GLfloat)(
		    scene->sky_nominal_color.g * dc->light_color[1]
		);
		dc->sky_color[2] = (GLfloat)(
		    scene->sky_nominal_color.b * dc->light_color[2]
		);
	    }

	    /* Set the glClearColor */
	    if(dc->flir)
	    {
		GLfloat c[3] = SAR_DEF_FLIR_SKY_GL_COLOR;
		glClearColor(
		    (GLclampf)c[0],
		    (GLclampf)c[1],
		    (GLclampf)c[2],
		    (GLclampf)0.0f
		);
	    }
	    else
	    {
		glClearColor(
		    (GLclampf)dc->sky_color[0],
		    (GLclampf)dc->sky_color[1],
		    (GLclampf)dc->sky_color[2],
		    (GLclampf)0.0f
		);
	    }
 	}

	/* Clear the back buffer */
	glClearDepth((GLclampd)1.0);
	/* Note: glClearColor() has already been called above when
	 * setting up the atmosphere
	 */
	glClear(
	    ((need_clear_color) ? GL_COLOR_BUFFER_BIT : 0) |
	    GL_DEPTH_BUFFER_BIT
	);


	/* Disable lighting, texture, and depth test for drawing the
	 * foundations and clouds
	 */
	StateGLDisable(state, GL_LIGHTING);
	StateGLDisable(state, GL_LIGHT0);
	/* GL_LIGHT1 will be turned on or off later */

	SAR_DRAW_DEPTH_TEST_OFF

	SAR_DRAW_TEXTURE_1D_OFF

	SAR_DRAW_TEXTURE_2D_OFF


	/* Turn off depth mask before drawing ground base */
	StateGLDepthMask(state, GL_FALSE);

	/* Draw ground base */
	SARDrawSceneFoundations(dc);


	/* Turn back on depth mask for drawing clouds */
	StateGLDepthMask(state, GL_TRUE);

	/* Draw cloud layers and textured horizon? */
	if(opt->textured_clouds)
	{
	    /* Horizon */
	    if(dc->highest_cloud_layer_ptr != NULL)
	    {
		if(dc->camera_pos.z < dc->highest_cloud_layer_ptr->z)
		    SARDrawSceneHorizon(dc);
	    }
	    else 
	    {
		SARDrawSceneHorizon(dc);
	    }

	    SAR_DRAW_TEXTURE_1D_OFF
	    SAR_DRAW_TEXTURE_2D_ON

	    /* Note: Cloud layers should be allocated lowest to
	     * highest
	     */

	    /* Go through cloud layers above camera */
	    for(i = 0; i < scene->total_cloud_layers; i++)
	    {
		cloud_layer_ptr = scene->cloud_layer[i];
		if(cloud_layer_ptr == NULL)
		    continue;

		/* Above camera? */
		if(cloud_layer_ptr->z > dc->camera_pos.z)
		{
		    /* Draw only this cloud layer */
		    SARDrawCloudLayer(dc, cloud_layer_ptr, True);
		    break;
		}
	    }
	    /* Go through cloud layers below camera */
	    for(i = scene->total_cloud_layers - 1; i >= 0; i--)
	    {
		cloud_layer_ptr = scene->cloud_layer[i];
		if(cloud_layer_ptr == NULL)
		    continue;

		/* Below camera? */
		if(cloud_layer_ptr->z < dc->camera_pos.z)
		{
		    /* Draw only this cloud layer */
		    SARDrawCloudLayer(dc, cloud_layer_ptr, False);
		    break;
		}
	    }
	}
	else
	{
	    /* Do not draw lower cloud layers or horizon, however we
	     * still must draw the highest cloud layer if it is defined
	     * and the camera is above it
	     */
	    if(dc->highest_cloud_layer_ptr != NULL)
	    {
		if(dc->camera_pos.z > dc->highest_cloud_layer_ptr->z)
		{
		    SAR_DRAW_TEXTURE_2D_ON
		    SARDrawCloudLayer(dc, dc->highest_cloud_layer_ptr, False);
		}
	    }
	}


	/* Calculate and set up primary light position and store the
	 * position in primary_light_pos
	 *
	 * The scene's primary light position is a unit vector offset
	 * and needs to be applied relative to the calculated camera_pos
	 * and multiplyed by a distance
	 */
	dc->primary_light_pos.x = (float)(
	    dc->camera_pos.x +
	    (scene->light_pos.x * SAR_PRILIGHT_TO_CAMERA_RAD)
	);
	dc->primary_light_pos.y = (float)(
	    dc->camera_pos.y +
	    (scene->light_pos.y * SAR_PRILIGHT_TO_CAMERA_RAD)
	);
	dc->primary_light_pos.z = (float)(
	    dc->camera_pos.z +
	    (scene->light_pos.z * SAR_PRILIGHT_TO_CAMERA_RAD)
	);

	/* No depth testing for drawing celestial objects */
	SAR_DRAW_DEPTH_TEST_OFF

	/* Draw celestial objects */
	if(opt->celestial_objects)
	    SARDrawSceneCelestialAdditive(dc);

	/* Turn on depth testing for cloud billboards */
	SAR_DRAW_DEPTH_TEST_ON
	/* Draw cloud billboards */
	if(opt->textured_clouds)
	{
	    for(i = 0; i < scene->total_cloud_bbs; i++)
	    {
		cloud_bb_ptr = scene->cloud_bb[i];
		if(cloud_bb_ptr == NULL)
		    continue;

		SARDrawCloudBB(dc, cloud_bb_ptr);
	    }
	}


	/* Enable lighting and turn on global light for the rest of
	 * the drawing (if celestial objects are displayed)
	 */
	if(opt->celestial_objects)
	{
	    StateGLEnable(state, GL_LIGHTING);
	    StateGLEnable(state, GL_LIGHT0);
	    /* Leave GL_LIGHT1 (the search light) and subsequent lights
	     * alone for now, they will be set later
	     */
	}

	/* Reset material values */
	StateGLDisable(state, GL_COLOR_MATERIAL);
	mat_val[0] = 0.2f;
	mat_val[1] = 0.2f;
	mat_val[2] = 0.2f;
	mat_val[3] = 1.0f;
	glMaterialfv(GL_FRONT, GL_AMBIENT, &(mat_val[0]));
	mat_val[0] = 0.8f;
	mat_val[1] = 0.8f;
	mat_val[2] = 0.8f;
	mat_val[3] = 1.0f;
	glMaterialfv(GL_FRONT, GL_DIFFUSE, &(mat_val[0]));
	mat_val[0] = 0.0f;
	mat_val[1] = 0.0f;  
	mat_val[2] = 0.0f;
	mat_val[3] = 1.0f;
	glMaterialfv(GL_FRONT, GL_SPECULAR, &(mat_val[0]));
	mat_val[0] = 0.0f * 128.0f;
	glMaterialfv(GL_FRONT, GL_SHININESS, &(mat_val[0]));
	mat_val[0] = 0.0f;
	mat_val[1] = 0.0f;
	mat_val[2] = 0.0f;
	mat_val[3] = 1.0f;
	glMaterialfv(GL_FRONT, GL_EMISSION, &(mat_val[0]));

	/* Enable color material (front face only) to follow
	 * ambient and diffuse lighting
	 */
	StateGLColorMaterial(
	    state, GL_FRONT, GL_AMBIENT_AND_DIFFUSE
	);
	StateGLEnable(state, GL_COLOR_MATERIAL);


	/* Reset global lighting */
	if(dc->flir)
	{
	    /* FLIR on, set greenish global lighting */
	    GLfloat vf[] = {0.0f, 0.1f, 0.0f, 1.0f};
	    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, vf);
	}
	else
	{
	    GLfloat vf[] = {0.1f, 0.1f, 0.1f, 1.0f};
	    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, vf);
	}

	/* Set light 0 as the scene's primary light */
	light_val[0] = (GLfloat)dc->primary_light_pos.x;
	light_val[1] = (GLfloat)dc->primary_light_pos.z;
	light_val[2] = (GLfloat)-dc->primary_light_pos.y;
	light_val[3] = 0.0f;		/* Directional light */
	glLightfv(GL_LIGHT0, GL_POSITION, &(light_val[0]));

	light_val[0] = (GLfloat)(0.50 * dc->light_color[0]);
	light_val[1] = (GLfloat)(0.50 * dc->light_color[1]);
	light_val[2] = (GLfloat)(0.50 * dc->light_color[2]);
	light_val[3] = 1.0f;
	glLightfv(GL_LIGHT0, GL_AMBIENT, &(light_val[0]));

	/* Set diffuse as the scene's lighting, increases it just a
	 * bit to make it last brighter around noon
	 */
	light_val[0] = (GLfloat)CLIP(
	    dc->light_color[0] + (dc->light_color[0] * 0.1),
	    0.0, 1.0
	);
	light_val[1] = (GLfloat)CLIP(
	    dc->light_color[1] + (dc->light_color[1] * 0.1),
	    0.0, 1.0
	);
	light_val[2] = (GLfloat)CLIP(
	    dc->light_color[2] + (dc->light_color[2] * 0.1),
	    0.0, 1.0
	);
	light_val[3] = 1.0f;
	glLightfv(GL_LIGHT0, GL_DIFFUSE, &(light_val[0]));

	light_val[0] = 1.0f;
	light_val[1] = 1.0f;
	light_val[2] = 1.0f;
	light_val[3] = 1.0f;
	glLightfv(GL_LIGHT0, GL_SPECULAR, &(light_val[0]));

	light_val[0] = 1.0f;
	glLightfv(GL_LIGHT0, GL_CONSTANT_ATTENUATION, &(light_val[0]));

	light_val[0] = 0.0f;
	glLightfv(GL_LIGHT0, GL_LINEAR_ATTENUATION, &(light_val[0]));

	light_val[0] = 0.0f;
	glLightfv(GL_LIGHT0, GL_QUADRATIC_ATTENUATION, &(light_val[0]));


	/* Turn on/off and set up texturing */
	if(opt->textured_objects)
	{
	    StateGLEnable(state, GL_ALPHA_TEST);
	    StateGLAlphaFunc(state, GL_GREATER, 0.5);

	    SAR_DRAW_TEXTURE_2D_ON
	}
	else
	{
	    SAR_DRAW_TEXTURE_2D_OFF
	}
	/* Don't use 1D texture */
	SAR_DRAW_TEXTURE_1D_OFF


/* Enables polygon offsetting if obj_ptr->flags specifies polygon
 * offsetting
 *
 * Requires that variables display, option, and obj_ptr be valid
 */
#define ENABLE_POLYGON_OFFSET_AS_NEEDED	{			\
 if(obj_ptr->flags & SAR_OBJ_FLAG_POLYGON_OFFSET) {		\
  if(!(obj_ptr->flags & SAR_OBJ_FLAG_POLYGON_OFFSET_WRITE_DEPTH)) \
   StateGLDepthMask(state, GL_FALSE);		\
  StateGLEnable(state, GL_POLYGON_OFFSET_FILL);	\
  StateGLPolygonOffset(						\
   state,						\
   (GLfloat)opt->gl_polygon_offset_factor, -1.0f		\
  );								\
 }								\
 else if(obj_ptr->flags & SAR_OBJ_FLAG_POLYGON_OFFSET_REVERSE)	\
 {								\
  if(!(obj_ptr->flags & SAR_OBJ_FLAG_POLYGON_OFFSET_WRITE_DEPTH)) \
   StateGLDepthMask(state, GL_FALSE);		\
  StateGLEnable(state, GL_POLYGON_OFFSET_FILL);	\
  StateGLPolygonOffset(						\
   state,						\
   -(GLfloat)opt->gl_polygon_offset_factor, 1.0f		\
  );								\
 }								\
}

/* Disables polygon offsetting if obj_ptr->flags specifies polygon
 * offsetting
 *
 * Requires that variables display, option, and obj_ptr be valid
 */
#define DISABLE_POLYGON_OFFSET_AS_NEEDED	{		\
 if(obj_ptr->flags & (SAR_OBJ_FLAG_POLYGON_OFFSET |		\
		      SAR_OBJ_FLAG_POLYGON_OFFSET_REVERSE)	\
 )								\
 {								\
  StateGLDepthMask(state, GL_TRUE);		\
  StateGLDisable(state, GL_POLYGON_OFFSET_FILL);	\
 }								\
}

	/* Iterate through each object, checking if the object is valid
	 * and if it is in bounds to be drawn. If it should be drawn then
	 * appropriate matrix rotations, translations, and GL state
	 * changes will be made and the object will be drawn.
	 */
	for(i = 0; i < core_ptr->total_objects; i++)
	{
	    obj_ptr = core_ptr->object[i];
	    if(obj_ptr == NULL)
		continue;

	    /* Get pointer to position of object */
	    pos = &obj_ptr->pos;


	    /* Begin general GL state changes for this object */

	    /* Check if object is to have depth testing or not */
	    if(obj_ptr->flags & SAR_OBJ_FLAG_NO_DEPTH_TEST)
	    {
		SAR_DRAW_DEPTH_TEST_OFF
		StateGLDepthMask(state, GL_FALSE);
	    }
	    else
	    {
		SAR_DRAW_DEPTH_TEST_ON
		StateGLDepthMask(state, GL_TRUE);
	    }

	    /* Set object's shade model */
	    if(obj_ptr->flags & SAR_OBJ_FLAG_SHADE_MODEL_SMOOTH)
	    {
		StateGLShadeModel(state, GL_SMOOTH);
	    }
	    else
	    {
		StateGLShadeModel(state, GL_FLAT);
	    }

	    /* Handle by object type */
	    switch(obj_ptr->type)
	    {
	      case SAR_OBJ_TYPE_GARBAGE:
		break;

	      case SAR_OBJ_TYPE_AIRCRAFT:
		aircraft = SAR_OBJ_GET_AIRCRAFT(obj_ptr);
		if(aircraft == NULL)
		    break;

		/* If this is the player object then update draw
		 * context information
		 */
		if((obj_ptr == scene->player_obj_ptr) &&
		   (gc != NULL)
		)
		{
		    /* Wheel brakes state */
		    dc->player_wheel_brakes = gc->wheel_brakes_state;
		    if((dc->player_wheel_brakes == 0) &&
		       (gc->wheel_brakes_coeff > 0.0f)
		    )
			dc->player_wheel_brakes = 1;

		    /* Air brakes state */
/*		    dc->player_air_brakes = gc->air_brakes_state; */
		    dc->player_air_brakes = (aircraft->air_brakes_state > 0) ? True : False;

		    /* Flight model */
		    dc->player_flight_model_type =
			aircraft->flight_model_type;

		    /* Auto pilot */
		    dc->player_autopilot = (aircraft->autopilot_state == SAR_AUTOPILOT_ON) ? True : False;

		    /* Get direction */
		    dir = &obj_ptr->dir;

		    /* Overspeed and stall warnings */
		    double rel_speed   = aircraft->landed ?
			aircraft->airspeed.y : SFMHypot2(aircraft->airspeed.y, aircraft->airspeed.z);
		    double stall_speed = SFMCurrentSpeedForStall(
			aircraft->airspeed.y, aircraft->airspeed.z, dir->pitch);

		    if((aircraft->overspeed_expected > 0.0f) &&
		       (rel_speed > aircraft->overspeed_expected))
			dc->player_overspeed = True;

		    /* Update sounds */
		    if(opt->event_sounds)
		    {
			/* Stall warning */
			dc->player_stall = SARSoundStallUpdate(
			    core_ptr->recorder,
			    obj_ptr->sndsrc, obj_ptr->total_sndsrcs,
			    &aircraft->stall_sndplay,
			    aircraft->stall_sndsrc,
			    dc->ear_in_cockpit,
			    aircraft->flight_model_type,
			    aircraft->landed,
			    stall_speed,
			    SARSimStallSpeed(aircraft)
			);

			/* Overspeed */
			SARSoundOverSpeedUpdate(
			    core_ptr->recorder,
			    obj_ptr->sndsrc, obj_ptr->total_sndsrcs,
			    &aircraft->overspeed_sndplay,
			    aircraft->overspeed_sndsrc,
			    dc->ear_in_cockpit,
			    dc->player_overspeed
			);
		    }
		}

		/* Get position and check if in range with camera */
		distance = (float)SFMHypot2(
		    pos->x - dc->camera_pos.x, pos->y - dc->camera_pos.y
		);
		if(distance > obj_ptr->range)
		{
		    /* Out of range with camera */

		    /* Mute sounds */
		    SARSoundEngineMute(
			core_ptr->recorder,
			aircraft->engine_inside_sndplay,
			aircraft->engine_outside_sndplay
		    );
		    break;
		}

		/* Is camera above and object below lowest cloud layer? */
		if(dc->lowest_cloud_layer_ptr != NULL)
		{
		    if((dc->camera_pos.z > dc->lowest_cloud_layer_ptr->z) &&
		       (pos->z < dc->lowest_cloud_layer_ptr->z)
		    )
			break;
		}

		/* Get far model range, if no far visual model available
		 * then set far_model_range as the regular range
		 */
		far_model_range = (obj_ptr->visual_model_far != NULL) ?
		    obj_ptr->range_far : obj_ptr->range;

		/* Calculate 3D distance */
		distance3d = (float)SFMHypot2(
		    distance, pos->z - dc->camera_pos.z
		);

		/* Get pointer to hoist */
		hoist_ptr = SARObjGetHoistPtr(obj_ptr, 0, NULL);

/* Draws hoist, hoist_ptr must be valid. It will be checked if rope is
 * out which implies basket is out. Will modify pos and dir pointer 
 * values
 */
#define DO_DRAW_HOIST_AS_NEEDED	{				\
 if((hoist_ptr != NULL) ? (hoist_ptr->rope_cur_vis > 0.0f) : False) { \
  const sar_position_struct *basket_pos = &hoist_ptr->pos;	\
  const sar_direction_struct *basket_dir = &hoist_ptr->dir;	\
  glPushMatrix(); {						\
   /* Translate and rotate heading */				\
   glTranslatef(basket_pos->x, basket_pos->z, -basket_pos->y);	\
   if(basket_dir->heading != 0)					\
    glRotatef(							\
     (GLfloat)-SFMRadiansToDegrees(basket_dir->heading),	\
     0.0f, 1.0f, 0.0f						\
    );								\
   SARDrawHoistDeployment(dc, obj_ptr, hoist_ptr);		\
  }								\
  glPopMatrix();						\
 }								\
}

		/* Draw far model if current distance is in the far
		 * model's range
		 */
		if(distance > far_model_range)
		{
		    /* Draw only far model */
		    glPushMatrix();
		    {
			/* Translate and rotate heading */
			glTranslatef(
			    pos->x, pos->z, -pos->y
			);
			if(dir->heading != 0)
			    glRotatef(
				(GLfloat)-SFMRadiansToDegrees(dir->heading),
				0.0f, 1.0f, 0.0f
			    );
			if(dir->pitch != 0)
			    glRotatef(
				(GLfloat)-SFMRadiansToDegrees(dir->pitch),
				1.0f, 0.0f, 0.0f
			    );
			if(dir->bank != 0)
			    glRotatef(
				(GLfloat)-SFMRadiansToDegrees(dir->bank),
				0.0f, 0.0f, 1.0f
			    );

			ENABLE_POLYGON_OFFSET_AS_NEEDED

			/* Draw far model during day time only? */
			if((obj_ptr->flags & SAR_OBJ_FLAG_FAR_MODEL_DAY_ONLY) ?
			   (scene->tod_code == SAR_TOD_CODE_DAY) : True
			)
			{
			    /* Draw far visual model */
			    SARVisualModelCallList(obj_ptr->visual_model_far);
			}

			/* Draw lights */
			SARDrawLights(
			    dc, obj_ptr->light, obj_ptr->total_lights
			);

			DISABLE_POLYGON_OFFSET_AS_NEEDED
			SAR_DRAW_POST_CALLLIST_RESET_STATES
		    }
		    glPopMatrix();
		}
		else
		{
		    /* Draw models normally */
		    DO_DRAW_HOIST_AS_NEEDED

		    glPushMatrix();
		    {
			/* Translate and rotate heading */
			glTranslatef(pos->x, pos->z, -pos->y);
			if(dir->heading != 0)
			    glRotatef(
				(GLfloat)-SFMRadiansToDegrees(dir->heading),
				0.0f, 1.0f, 0.0f
			    );

			/* Draw shadow (before applying pitch and bank
			 * matrix rotation), polygon offset will be
			 * applied as needed in this call
			 */
			SARDrawObjectShadow(dc, obj_ptr);

			/* Rotate pitch and bank */
			if(dir->pitch != 0)
			    glRotatef(
				(GLfloat)-SFMRadiansToDegrees(dir->pitch),
				1.0f, 0.0f, 0.0f
			    );
			if(dir->bank != 0)
			    glRotatef(
				(GLfloat)-SFMRadiansToDegrees(dir->bank),
				0.0f, 0.0f, 1.0f
			    );

			ENABLE_POLYGON_OFFSET_AS_NEEDED

			/* Check if this is the player object */
			if((obj_ptr == scene->player_obj_ptr) &&
			   (gc != NULL)
			)
			{
                            SARSoundEngineUpdate(
				core_ptr->recorder,
				obj_ptr->sndsrc, obj_ptr->total_sndsrcs,
				aircraft->engine_inside_sndplay,
				aircraft->engine_inside_sndsrc,
				aircraft->engine_outside_sndplay,
				aircraft->engine_outside_sndsrc,
				dc->ear_in_cockpit,
				aircraft->engine_state,
				aircraft->throttle,
				distance3d
			    );

			    /* Adjust spot light */
			    SARDrawSetSpotLight(dc, obj_ptr);

			    /* Camera reference inside cockpit? */
			    if(dc->camera_in_cockpit)
			    {
				/* Record the pointer to this object
				 * for drawing the cockpit (later).
				 */
				dc->player_obj_cockpit_ptr = obj_ptr;

				DISABLE_POLYGON_OFFSET_AS_NEEDED

				/* Pop one matrix and break (do not
				 * draw the rest of the aircraft).
				 */
				glPopMatrix();
				break;
			    }
			}
			else
			{
			    /* Not player object */

			    /* Update engine sound */
			    SARSoundEngineUpdate(
				core_ptr->recorder,
				obj_ptr->sndsrc, obj_ptr->total_sndsrcs,
				aircraft->engine_inside_sndplay,
				aircraft->engine_inside_sndsrc,
				aircraft->engine_outside_sndplay,
				aircraft->engine_outside_sndsrc,
				False,
				aircraft->engine_state,
				aircraft->throttle,
				distance3d
			    );
			}

			/* Draw standard day time model? */
		        if((obj_ptr->flags & SAR_OBJ_FLAG_HIDE_DAY_MODEL) ?
		           (scene->tod_code == SAR_TOD_CODE_DAY) : 1
		        )
		        {
			    if(dc->flir && (obj_ptr->visual_model_ir != NULL))
			    {
				StateGLDisable(state, GL_LIGHTING);
				SARDrawSetColorFLIRTemperature(obj_ptr->temperature);
				SARVisualModelCallList(obj_ptr->visual_model_ir);
				StateGLEnable(state, GL_LIGHTING);
			    }
			    else
			    {
#if 0
/* Problem with using the color mask is that if the object is drawn
 * over something else the red and blue values may not be modified 
 * causing a purplish color to result
 */
				if(dc->flir)
				    SAR_DRAW_FLIR_COLOR_MASK_SET
#endif
				SARVisualModelCallList(obj_ptr->visual_model);
				if(dc->flir)
				    SAR_DRAW_COLOR_MASK_UNSET
			    }
		        }
			/* Draw standard dawn time model? */
			if((obj_ptr->flags & SAR_OBJ_FLAG_HIDE_DAWN_MODEL) ?
			    (scene->tod_code == SAR_TOD_CODE_DAWN) : 1
			)
			{
			    if(!dc->flir)
			    {
				/* Disable GL_LIGHTING when drawing dawn time model */
				StateGLDisable(state, GL_LIGHTING);
				SARVisualModelCallList(obj_ptr->visual_model_dawn);
				StateGLEnable(state, GL_LIGHTING);
			    }
			}
			/* Draw standard dusk time model? */
			if((obj_ptr->flags & SAR_OBJ_FLAG_HIDE_DUSK_MODEL) ?
			    (scene->tod_code == SAR_TOD_CODE_DUSK) : 1
			)
			{
			    if(!dc->flir)
			    {
				/* Disable GL_LIGHTING when drawing dusk time model */
				StateGLDisable(state, GL_LIGHTING);
				SARVisualModelCallList(obj_ptr->visual_model_dusk);
				StateGLEnable(state, GL_LIGHTING);
			    }
			}
			/* Draw standard night time model? */
			if((obj_ptr->visual_model_night != NULL) &&
			   !dc->flir
			)
			{
			    Boolean draw_night_model = False;
			    if(obj_ptr->flags & SAR_OBJ_FLAG_HIDE_NIGHT_MODEL)
			    {
			        switch(scene->tod_code)
			        {
			          case SAR_TOD_CODE_NIGHT:
				    draw_night_model = True;
				    break;
			          case SAR_TOD_CODE_DAWN:
				    draw_night_model = (obj_ptr->flags &
				        SAR_OBJ_FLAG_NIGHT_MODEL_AT_DAWN) ? True : False;
				    break;
			          case SAR_TOD_CODE_DUSK:
				    draw_night_model = (obj_ptr->flags & 
					SAR_OBJ_FLAG_NIGHT_MODEL_AT_DUSK) ? True : False;
				    break;
				  case SAR_TOD_CODE_DAY:
				  case SAR_TOD_CODE_UNDEFINED:
				    break;
			        }
			    }
			    else
			    {
			        draw_night_model = True;
			    }
			    if(draw_night_model)
			    {
			        /* Disable GL_LIGHTING when drawing night time model */
			        StateGLDisable(state, GL_LIGHTING);
			        SARVisualModelCallList(obj_ptr->visual_model_night);
			        StateGLEnable(state, GL_LIGHTING);
			    }
		        }

			/* Draw object parts */
			for(n = 0; n < aircraft->total_parts; n++)
			{
			    part = aircraft->part[n];
			    if(part == NULL)
				continue;

			    SARDrawPart(dc, obj_ptr, part);
			}

		        /* Draw external reserved fuel tanks */
			for(n = 0; n < aircraft->total_external_fueltanks; n++)
			{
			    eft_ptr = aircraft->external_fueltank[n];
			    if(eft_ptr == NULL)
				continue;

			    SARDrawFuelTank(dc, obj_ptr, eft_ptr);
			}

			/* Draw lights */
			SARDrawLights(  
			    dc, obj_ptr->light, obj_ptr->total_lights  
			);

			/* Draw rotors/propellars */
			for(n = 0; n < aircraft->total_rotors; n++)
			{
			    rotor = aircraft->rotor[n];
			    if(rotor == NULL)
				continue;

			    SARDrawRotor(
				dc, obj_ptr, rotor,
				aircraft->throttle
			    );
			}

			DISABLE_POLYGON_OFFSET_AS_NEEDED
		        SAR_DRAW_POST_CALLLIST_RESET_STATES
		    }
		    glPopMatrix();
		}
#undef DO_DRAW_HOIST_AS_NEEDED
		break;

	      case SAR_OBJ_TYPE_GROUND:
		/* Get position and check if in range with camera */
		distance = (float)SFMHypot2(
		    pos->x - dc->camera_pos.x, pos->y - dc->camera_pos.y
		);
		if(distance > obj_ptr->range)
		    break;

		/* Is camera above and object below lowest cloud layer? */
		if(dc->lowest_cloud_layer_ptr != NULL)
		{
		    if((dc->camera_pos.z > dc->lowest_cloud_layer_ptr->z) &&
		       (pos->z < dc->lowest_cloud_layer_ptr->z)
		    )
			break;
		}

		/* Get direction */
		dir = &obj_ptr->dir;

		glPushMatrix();
		{
		    /* Translate and rotate */
		    glTranslatef(pos->x, pos->z, -pos->y);
		    if(dir->heading != 0)
			glRotatef(
			    (GLfloat)-SFMRadiansToDegrees(dir->heading),
			    0.0f, 1.0f, 0.0f
			);
		    if(dir->pitch != 0)
			glRotatef(
			    (GLfloat)-SFMRadiansToDegrees(dir->pitch),
			    1.0f, 0.0f, 0.0f
			);
		    if(dir->bank != 0)
			glRotatef(
			    (GLfloat)-SFMRadiansToDegrees(dir->bank),
			    0.0f, 0.0f, 1.0f
			);

		    ENABLE_POLYGON_OFFSET_AS_NEEDED

		    /* Draw standard day time model? */
		    if((obj_ptr->flags & SAR_OBJ_FLAG_HIDE_DAY_MODEL) ?
			(scene->tod_code == SAR_TOD_CODE_DAY) : 1
		    )
		    {
			if(dc->flir && (obj_ptr->visual_model_ir != NULL))
			{
			    StateGLDisable(state, GL_LIGHTING);
			    SARDrawSetColorFLIRTemperature(obj_ptr->temperature);
			    SARVisualModelCallList(obj_ptr->visual_model_ir);
			    StateGLEnable(state, GL_LIGHTING);
			}
			else
			{
#if 0
			    if(dc->flir)
				SAR_DRAW_FLIR_COLOR_MASK_SET
#endif
			    SARVisualModelCallList(obj_ptr->visual_model);
			    if(dc->flir)
				SAR_DRAW_COLOR_MASK_UNSET
			}
		    }
		    /* Draw standard dawn time model? */
		    if((obj_ptr->flags & SAR_OBJ_FLAG_HIDE_DAWN_MODEL) ?
			(scene->tod_code == SAR_TOD_CODE_DAWN) : 1
		    )
		    {
			if(!dc->flir)
			{
			    /* Disable GL_LIGHTING when drawing dawn time model */
			    StateGLDisable(state, GL_LIGHTING);
			    SARVisualModelCallList(obj_ptr->visual_model_dawn);
			    StateGLEnable(state, GL_LIGHTING);
			}
		    }
		    /* Draw standard dusk time model? */
		    if((obj_ptr->flags & SAR_OBJ_FLAG_HIDE_DUSK_MODEL) ?
			(scene->tod_code == SAR_TOD_CODE_DUSK) : 1
		    )
		    {
			if(!dc->flir)
			{
			    /* Disable GL_LIGHTING when drawing dusk time model */
			    StateGLDisable(state, GL_LIGHTING);
			    SARVisualModelCallList(obj_ptr->visual_model_dusk);
			    StateGLEnable(state, GL_LIGHTING);
			}
		    }
		    /* Draw standard night time model? */
		    if((obj_ptr->visual_model_night != NULL) &&
		       !dc->flir
		    )
		    {
			Boolean draw_night_model = False;
			if(obj_ptr->flags & SAR_OBJ_FLAG_HIDE_NIGHT_MODEL)
			{
			    switch(scene->tod_code)
			    {
			      case SAR_TOD_CODE_NIGHT:
				draw_night_model = True;
				break;
			      case SAR_TOD_CODE_DAWN:
				draw_night_model = (obj_ptr->flags &
				    SAR_OBJ_FLAG_NIGHT_MODEL_AT_DAWN) ? True : False;
				break;
			      case SAR_TOD_CODE_DUSK:
				draw_night_model = (obj_ptr->flags &
				    SAR_OBJ_FLAG_NIGHT_MODEL_AT_DUSK) ? True : False;
				break;
			      case SAR_TOD_CODE_DAY:
			      case SAR_TOD_CODE_UNDEFINED:
				break;
			    }
			}
			else
			{    
			    draw_night_model = True;
			}
			if(draw_night_model)
			{
			    /* Disable GL_LIGHTING when drawing night time model */
			    StateGLDisable(state, GL_LIGHTING);
			    SARVisualModelCallList(obj_ptr->visual_model_night);   
			    StateGLEnable(state, GL_LIGHTING);
			}
		    }

		    /* Draw lights */
		    SARDrawLights(
			dc, obj_ptr->light, obj_ptr->total_lights
		    );

		    DISABLE_POLYGON_OFFSET_AS_NEEDED
		    SAR_DRAW_POST_CALLLIST_RESET_STATES
		}
		glPopMatrix();
		break;

	      case SAR_OBJ_TYPE_RUNWAY:
		obj_runway_ptr = SAR_OBJ_GET_RUNWAY(obj_ptr);
		if(obj_runway_ptr == NULL)
		    break;

		/* Is camera above and object below lowest cloud layer? */
		if(dc->lowest_cloud_layer_ptr != NULL)
		{
		    if((dc->camera_pos.z > dc->lowest_cloud_layer_ptr->z) &&
		       (pos->z < dc->lowest_cloud_layer_ptr->z)
		    )
			break;
		}

		/* Skip if camera is under runway */
		if(dc->camera_pos.z < pos->z)
		    break;

		/* Get position and check if in range with camera */
		distance = (float)SFMHypot2(
		    pos->x - dc->camera_pos.x, pos->y - dc->camera_pos.y
		);
		if(distance > obj_ptr->range)
		    break;

		/* Calculate 3D distance */
		distance3d = (float)SFMHypot2(
		    distance, pos->z - dc->camera_pos.z
		);
	
		/* Get direction */
		dir = &obj_ptr->dir;

		glPushMatrix();
		{
		    /* Translate and rotate */
		    glTranslatef(pos->x, pos->z, -pos->y);
		    if(dir->heading != 0)
			glRotatef(
			    (GLfloat)-SFMRadiansToDegrees(dir->heading),
			    0.0f, 1.0f, 0.0f
			);
		    if(dir->pitch != 0)
			glRotatef(
			    (GLfloat)-SFMRadiansToDegrees(dir->pitch),
			    1.0f, 0.0f, 0.0f
			);
		    if(dir->bank != 0)
			glRotatef(
			    (GLfloat)-SFMRadiansToDegrees(dir->bank),
			    0.0f, 0.0f, 1.0f
			);

		    ENABLE_POLYGON_OFFSET_AS_NEEDED

		    SARDrawRunway(
			dc, obj_ptr, obj_runway_ptr, distance3d
		    );

		    DISABLE_POLYGON_OFFSET_AS_NEEDED
		    SAR_DRAW_POST_CALLLIST_RESET_STATES
		}
		glPopMatrix();
		break;

	      case SAR_OBJ_TYPE_HELIPAD:
		obj_helipad_ptr = SAR_OBJ_GET_HELIPAD(obj_ptr);
		if(obj_helipad_ptr == NULL)
		    break;

		/* Is camera above and object below lowest cloud layer? */
		if(dc->lowest_cloud_layer_ptr != NULL)
		{
		    if((dc->camera_pos.z > dc->lowest_cloud_layer_ptr->z) &&
		       (pos->z < dc->lowest_cloud_layer_ptr->z)
		    )
			break;
		}

		/* Skip if camera is under helipad */
/* Don't check this, what about recessed helipads? Helipads with additional
   stuff below it.
		if(dc->camera_pos.z < pos->z)
		    break;
 */
		/* Get position and check if in range with camera */
		distance = (float)SFMHypot2(
		    pos->x - dc->camera_pos.x, pos->y - dc->camera_pos.y
		);
		if(distance > obj_ptr->range)
		    break;

		/* Calculate 3D distance */
		distance3d = (float)SFMHypot2(
		    distance, pos->z - dc->camera_pos.z
		);

		/* Get direction */
		dir = &obj_ptr->dir;

		glPushMatrix();
		{
		    /* Translate and rotate */
		    glTranslatef(pos->x, pos->z, -pos->y);
		    if(dir->heading != 0)
			glRotatef(
			    (GLfloat)-SFMRadiansToDegrees(dir->heading),
			    0.0f, 1.0f, 0.0f
			);
		    if(dir->pitch != 0)
			glRotatef(
			    (GLfloat)-SFMRadiansToDegrees(dir->pitch),
			    1.0f, 0.0f, 0.0f
			);
		    if(dir->bank != 0)
			glRotatef(
			    (GLfloat)-SFMRadiansToDegrees(dir->bank),
			    0.0f, 0.0f, 1.0f
			);

		    ENABLE_POLYGON_OFFSET_AS_NEEDED

		    SARDrawHelipad(
			dc, obj_ptr, obj_helipad_ptr, distance3d
		    );

		    DISABLE_POLYGON_OFFSET_AS_NEEDED
		    SAR_DRAW_POST_CALLLIST_RESET_STATES
		}
		glPopMatrix();
		break;

	      case SAR_OBJ_TYPE_HUMAN:
		obj_human_ptr = SAR_OBJ_GET_HUMAN(obj_ptr);
		if(obj_human_ptr == NULL)
		    break;

		/* Get position and check if in range with camera */
		distance = (float)SFMHypot2(
		    pos->x - dc->camera_pos.x, pos->y - dc->camera_pos.y
		);
		if(distance > obj_ptr->range)
		    break;

		/* Is camera above and object below lowest cloud layer? */
		if(dc->lowest_cloud_layer_ptr != NULL)
		{
		    if((dc->camera_pos.z > dc->lowest_cloud_layer_ptr->z) &&
		       (pos->z < dc->lowest_cloud_layer_ptr->z)
		    )
			break;
		}

		/* Get direction */
		dir = &obj_ptr->dir;

		glPushMatrix();
		{
		    /* Translate and rotate heading */
		    glTranslatef(pos->x, pos->z, -pos->y);
		    if(dir->heading != 0)
			glRotatef(
			    (GLfloat)-SFMRadiansToDegrees(dir->heading),
			    0.0f, 1.0f, 0.0f
			);

		    /* Draw shadow (before applying pitch and bank
		     * matrix rotation). Polygon offsetting will be
		     * applied as needed in this call.
		     */
		    SARDrawObjectShadow(dc, obj_ptr);

		    /* Rotate pitch and bank */
		    if(dir->pitch != 0)
			glRotatef(
			    (GLfloat)-SFMRadiansToDegrees(dir->pitch),
			    1.0f, 0.0f, 0.0f
			);
		    if(dir->bank != 0)
			glRotatef(
			    (GLfloat)-SFMRadiansToDegrees(dir->bank),
			    0.0f, 0.0f, 1.0f
			);

		    SARDrawHuman(dc, obj_ptr, obj_human_ptr);

		    SAR_DRAW_POST_CALLLIST_RESET_STATES
		}
		glPopMatrix();
		break;

	      case SAR_OBJ_TYPE_SMOKE:
		/* Get position and check if in range with camera */
		distance = (float)SFMHypot2(
		    pos->x - dc->camera_pos.x, pos->y - dc->camera_pos.y
		);
		if(distance > obj_ptr->range)
		    break;

		/* Get direction */
		dir = &obj_ptr->dir;

		/* Do not push matrix to translate or rotate, the call
		 * to SARDrawSmoke() will do that for us.
		 */
		if((obj_ptr->data != NULL) && opt->smoke_trails)
		    SARDrawSmoke(dc, obj_ptr, SAR_OBJ_GET_SMOKE(obj_ptr));
		SAR_DRAW_POST_CALLLIST_RESET_STATES
		break;

	      case SAR_OBJ_TYPE_FIRE:
		/* Get position and check if in range with camera */
		distance = (float)SFMHypot2(
		    pos->x - dc->camera_pos.x, pos->y - dc->camera_pos.y
		);
		if(distance > obj_ptr->range)
		    break;

		/* Get direction */
		dir = &obj_ptr->dir;

		glPushMatrix();
		{
		    sar_direction_struct fire_dir;

		    /* Translate */
		    glTranslatef(pos->x, pos->z, -pos->y);

		    /* Adjust heading to camera heading */
		    SARDrawGetDirFromPos(
			&dc->camera_pos, pos, &fire_dir
		    );
		    if(fire_dir.heading != 0)
			glRotatef(
			    (GLfloat)-SFMRadiansToDegrees(fire_dir.heading),
			    0.0f, 1.0f, 0.0f
			);
		    if(fire_dir.pitch != 0)
			glRotatef(
			    (GLfloat)-SFMRadiansToDegrees(fire_dir.pitch),
			    1.0f, 0.0f, 0.0f
			);

		    if(obj_ptr->data != NULL)
			SARDrawFire(
			    dc, obj_ptr,
			    SAR_OBJ_GET_FIRE(obj_ptr)
			);

		    SAR_DRAW_POST_CALLLIST_RESET_STATES
		}
		glPopMatrix();
		break;

	      case SAR_OBJ_TYPE_EXPLOSION:
		/* Get position and check if in range with camera */
		distance = (float)SFMHypot2(
		    pos->x - dc->camera_pos.x, pos->y - dc->camera_pos.y
		);
		if(distance > obj_ptr->range)
		    break;

		/* Get direction */
		dir = &obj_ptr->dir;

		glPushMatrix();
		{
		    sar_direction_struct explosion_dir;

		    /* Translate */
		    glTranslatef(pos->x, pos->z, -pos->y);

		    /* Adjust heading to camera heading */
		    SARDrawGetDirFromPos(
			&dc->camera_pos, pos, &explosion_dir
		    );
		    if(explosion_dir.heading != 0)
			glRotatef(
			    (GLfloat)-SFMRadiansToDegrees(explosion_dir.heading),
			    0.0f, 1.0f, 0.0f
			);
		    if(explosion_dir.pitch != 0)
			glRotatef(
			    (GLfloat)-SFMRadiansToDegrees(explosion_dir.pitch),
			    1.0f, 0.0f, 0.0f
			);

		    if(obj_ptr->data != NULL)
			SARDrawExplosion(
			    dc, obj_ptr, SAR_OBJ_GET_EXPLOSION(obj_ptr)
			);

		    SAR_DRAW_POST_CALLLIST_RESET_STATES
		}
		glPopMatrix();
		break;

	      case SAR_OBJ_TYPE_CHEMICAL_SPRAY:
		break;

	      case SAR_OBJ_TYPE_PREMODELED:
		/* Get position and check if in range with camera */
		distance = (float)SFMHypot2(
		    pos->x - dc->camera_pos.x, pos->y - dc->camera_pos.y
		);
		if(distance > obj_ptr->range)
		    break;

		/* Is camera above and object below lowest cloud layer? */
		if(dc->lowest_cloud_layer_ptr != NULL)
		{
		    if((dc->camera_pos.z > dc->lowest_cloud_layer_ptr->z) &&
		       (pos->z < dc->lowest_cloud_layer_ptr->z)
		    )
			break;
		}

		/* Get direction */
		dir = &obj_ptr->dir;

		/* Calculate 3D distance */
		distance3d = (float)SFMHypot2(
		    distance, pos->z - dc->camera_pos.z
		);

		glPushMatrix();
		{
		    /* Translate and rotate */
		    glTranslatef(pos->x, pos->z, -pos->y);
		    if(dir->heading != 0)
			glRotatef(
			    (GLfloat)-SFMRadiansToDegrees(dir->heading),
			    0.0f, 1.0f, 0.0f
			);
		    if(dir->pitch != 0)
			glRotatef(
			    (GLfloat)-SFMRadiansToDegrees(dir->pitch),
			    1.0f, 0.0f, 0.0f
			);
		    if(dir->bank != 0)
			glRotatef(
			    (GLfloat)-SFMRadiansToDegrees(dir->bank),
			    0.0f, 0.0f, 1.0f
			);

		    SARDrawPremodeled(
			dc, obj_ptr,
			SAR_OBJ_GET_PREMODELED(obj_ptr),
			distance3d, False
		    );

		    /* Draw lights */
		    SARDrawLights(
			dc, obj_ptr->light, obj_ptr->total_lights
		    );

		    SAR_DRAW_POST_CALLLIST_RESET_STATES
		}
		glPopMatrix();
		break;

	      case SAR_OBJ_TYPE_WATERCRAFT:
	      case SAR_OBJ_TYPE_AUTOMOBILE:
	      case SAR_OBJ_TYPE_FUELTANK:
	      case SAR_OBJ_TYPE_STATIC:
		/* Get position and check if in range with camera */
		distance = (float)SFMHypot2(
		    pos->x - dc->camera_pos.x, pos->y - dc->camera_pos.y
		);
		if(distance > obj_ptr->range)
		    break;

		/* Is camera above and object below lowest cloud layer? */
		if(dc->lowest_cloud_layer_ptr != NULL)
		{
		    if((dc->camera_pos.z > dc->lowest_cloud_layer_ptr->z) &&
		       (pos->z < dc->lowest_cloud_layer_ptr->z)
		    )
			break;
		}

		/* Get direction */
		dir = &obj_ptr->dir;

		/* Get far model range, if no far visual model available
		 * then set far_model_range as the regular range
		 */
		far_model_range = (obj_ptr->visual_model_far != NULL) ?
		    obj_ptr->range_far : obj_ptr->range;

		/* Draw far model if current distance is in the far
		 * model's range
		 */
		if(distance > far_model_range)
		{
		    /* Draw only far model */
		    glPushMatrix();
		    {
			/* Translate and rotate */
			glTranslatef(pos->x, pos->z, -pos->y);
			if(dir->heading != 0)
			    glRotatef(
				(GLfloat)-SFMRadiansToDegrees(dir->heading),
				0.0f, 1.0f, 0.0f
			    );
			if(dir->pitch != 0)
			    glRotatef(
				(GLfloat)-SFMRadiansToDegrees(dir->pitch),
				1.0f, 0.0f, 0.0f
			    );
			if(dir->bank != 0)
			    glRotatef(
				(GLfloat)-SFMRadiansToDegrees(dir->bank),
				0.0f, 0.0f, 1.0f
			    );

			ENABLE_POLYGON_OFFSET_AS_NEEDED

			/* Draw far model during day time only? */
			if((obj_ptr->flags & SAR_OBJ_FLAG_FAR_MODEL_DAY_ONLY) ?
			   (scene->tod_code == SAR_TOD_CODE_DAY) : 1
			)
			{
			    /* Draw far visual model */
			    SARVisualModelCallList(obj_ptr->visual_model_far);
			}

			/* Draw lights */
			SARDrawLights(
			    dc, obj_ptr->light, obj_ptr->total_lights
			);

			DISABLE_POLYGON_OFFSET_AS_NEEDED
			SAR_DRAW_POST_CALLLIST_RESET_STATES
		    }
		    glPopMatrix();
		}
		else
		{
		    /* Draw closeup models */
		    glPushMatrix();
		    {
			/* Translate and rotate heading */
			glTranslatef(pos->x, pos->z, -pos->y);
			if(dir->heading != 0)
			    glRotatef(
				(GLfloat)-SFMRadiansToDegrees(dir->heading),
				0.0f, 1.0f, 0.0f
			    );

			/* Draw shadow (before applying pitch and bank
			 * matrix rotation). Polygon offsetting will be
			 * applied as needed in this call.
			 */
			SARDrawObjectShadow(dc, obj_ptr);

			/* Rotate pitch and bank */
			if(dir->pitch != 0)
			    glRotatef(
				(GLfloat)-SFMRadiansToDegrees(dir->pitch),
				1.0f, 0.0f, 0.0f
			    );
			if(dir->bank != 0)
			    glRotatef(
				(GLfloat)-SFMRadiansToDegrees(dir->bank),
				0.0f, 0.0f, 1.0f
			    );

			ENABLE_POLYGON_OFFSET_AS_NEEDED

		        /* Draw standard day time model? */
			if((obj_ptr->flags & SAR_OBJ_FLAG_HIDE_DAY_MODEL) ?
			    (scene->tod_code == SAR_TOD_CODE_DAY) : 1
			)
			{
			    if(dc->flir && (obj_ptr->visual_model_ir != NULL))
			    {
				StateGLDisable(state, GL_LIGHTING);
				SARDrawSetColorFLIRTemperature(obj_ptr->temperature);
				SARVisualModelCallList(obj_ptr->visual_model_ir);
				StateGLEnable(state, GL_LIGHTING);
			    }
			    else
			    {
#if 0
				if(dc->flir)
				    SAR_DRAW_FLIR_COLOR_MASK_SET
#endif
				SARVisualModelCallList(obj_ptr->visual_model);
				if(dc->flir)
				    SAR_DRAW_COLOR_MASK_UNSET
			    }
			}
			/* Draw standard dawn time model? */
			if((obj_ptr->flags & SAR_OBJ_FLAG_HIDE_DAWN_MODEL) ?
			    (scene->tod_code == SAR_TOD_CODE_DAWN) : 1
			)
			{
			    if(!dc->flir)
			    {
			        /* Disable GL_LIGHTING when drawing dawn time model */
			        StateGLDisable(state, GL_LIGHTING);
			        SARVisualModelCallList(obj_ptr->visual_model_dawn);
			        StateGLEnable(state, GL_LIGHTING);
			    }
			}
			/* Draw standard dusk time model? */
			if((obj_ptr->flags & SAR_OBJ_FLAG_HIDE_DUSK_MODEL) ?
			    (scene->tod_code == SAR_TOD_CODE_DUSK) : 1
			)
			{
			    if(!dc->flir)
			    {
				/* Disable GL_LIGHTING when drawing dusk time model */
				StateGLDisable(state, GL_LIGHTING);
				SARVisualModelCallList(obj_ptr->visual_model_dusk);
				StateGLEnable(state, GL_LIGHTING);
			    }
			}
			/* Draw standard night time model? */
			if((obj_ptr->visual_model_night != NULL) &&
			   !dc->flir
			)
			{
			    Boolean draw_night_model = False;
			    if(obj_ptr->flags & SAR_OBJ_FLAG_HIDE_NIGHT_MODEL)
			    {
				switch(scene->tod_code)
				{
				  case SAR_TOD_CODE_NIGHT:
				    draw_night_model = True;
				    break;
				  case SAR_TOD_CODE_DAWN:
				    draw_night_model = (obj_ptr->flags &
					SAR_OBJ_FLAG_NIGHT_MODEL_AT_DAWN) ? True : False;
				    break;
				  case SAR_TOD_CODE_DUSK:
				    draw_night_model = (obj_ptr->flags &
					SAR_OBJ_FLAG_NIGHT_MODEL_AT_DUSK) ? True : False;
				    break;
				  case SAR_TOD_CODE_DAY:
				  case SAR_TOD_CODE_UNDEFINED:
				    break;
				}
			    }
			    else
			    {    
				draw_night_model = True;
			    }
			    if(draw_night_model)
			    {
				/* Disable GL_LIGHTING when drawing night time model */
				StateGLDisable(state, GL_LIGHTING);
				SARVisualModelCallList(obj_ptr->visual_model_night);   
				StateGLEnable(state, GL_LIGHTING);
			    }
			}

			/* Draw lights */
			SARDrawLights(
			    dc, obj_ptr->light, obj_ptr->total_lights
			);

			DISABLE_POLYGON_OFFSET_AS_NEEDED
			SAR_DRAW_POST_CALLLIST_RESET_STATES
		    }
		    glPopMatrix();
		}
		break;
	    }
	}	/* Iterate through each object */


	StateGLDisable(state, GL_FOG);

	/* Draw spot light cast for player object */
	if(scene->player_obj_ptr != NULL)
	    SARDrawSpotLightCast(dc, scene->player_obj_ptr);

	/* If the player_obj_cockpit_ptr was recorded, then
	 * it means we need to draw the interior of the cockpit.
	 *
	 * Note that this call will modify the identity matrix for
	 * both the model view and perspective.
	 */
	if(dc->player_obj_cockpit_ptr != NULL)
	    SARDrawObjectCockpit(
		dc, dc->player_obj_cockpit_ptr
	    );


	/* Set up gl states for 2d drawing */
	GWOrtho2D(display);
	StateGLDisable(state, GL_DEPTH_TEST);
	StateGLDepthFunc(state, GL_ALWAYS);
	StateGLDisable(state, GL_COLOR_MATERIAL);
	StateGLDisable(state, GL_LIGHTING);
	StateGLDisable(state, GL_LIGHT0);
	SAR_DRAW_TEXTURE_1D_OFF
	SAR_DRAW_TEXTURE_2D_OFF
	StateGLDisable(state, GL_ALPHA_TEST);

	/* Draw cockpit HUD */
	if(scene->camera_ref == SAR_CAMERA_REF_COCKPIT)
	{
	    if(scene->player_obj_ptr != NULL)
		SARDrawHUD(dc, scene->player_obj_ptr);
	}
	else
	{
	    if(dc->flir)
		glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	    else
		SARDrawSetColor(&opt->message_color);
	    if(scene->player_obj_ptr != NULL)
		SARDrawOutsideAttitude(dc, scene->player_obj_ptr);
	}

	/* Begin drawing text */

	/* Draw control messages */
	SARDrawControlMessages(dc);

	/* Camera title */
	SARDrawCameraRefTitle(dc);

	/* Sticky banner messages */
	SARDrawBanner(dc);

	/* Messages */
	SARDrawMessages(dc);

	/* Text input prompt */
	SARTextInputDraw(core_ptr->text_input);

	/* Help */
	if(core_ptr->display_help > 0)
	    SARDrawHelp(dc);


	/* Put GL buffer to window */
	GWSwapBuffer(display);

	/* Report any errors */
	if(opt->runtime_debug)
	{
	    GLenum error_code = glGetError();
	    if(error_code != GL_NO_ERROR)
		SARReportGLError(core_ptr, error_code);
	}

#undef ENABLE_POLYGON_OFFSET_AS_NEEDED
#undef DISABLE_POLYGON_OFFSET_AS_NEEDED
}


/*
 *      Redraws scene as a map.
 *
 *	If draw_for_gcc is True then drawing will be done in a different
 *	style suitable for `ground contact check'. Also the core's list
 *	of draw map object names will be updated.  It will be reallocated 
 *	to contain the new list of matched objects by index numbers.
 *
 *	If draw_for_ghc is True then drawing will be done in a different
 *	style suitable for `ground hit check'. Also the core's 
 *	drawmap_ghc_result will be set to the alpha pixel read.
 *
 *	gcc_obj_num will be passed to SARSetCameraMapDrawGCC(), it
 *	defines which object is the player object (or -1 to fetch player
 *	object number from scene structure).
 */
void SARDrawMap(
	sar_core_struct *core_ptr,
	Boolean draw_for_gcc, Boolean draw_for_ghc, int gcc_obj_num
)
{
	int i, n, width, height, *total;
	float fovz_um, view_aspect, map_dxm, map_dym, map_radius;
	float distance, distance3d, icon_len;

	gw_display_struct *display;
	state_gl_struct *state;
	sar_scene_struct *scene;
	sar_object_struct ***ptr, *obj_ptr;
	gctl_struct *gc;
	const sar_color_struct *c;
	const sar_direction_struct *dir;
	const sar_position_struct *pos;
	sar_object_aircraft_struct *aircraft;
	sar_object_helipad_struct *obj_helipad_ptr;
	sar_object_runway_struct *obj_runway_ptr;

	GLuint select_name_base;
	GLuint *gl_select_buf = NULL;
	int gl_select_buf_size = 0;
	const sar_option_struct *opt;
	sar_dc_struct _dc, *dc;


	/* Reset drawing context */
	dc = &_dc;
	memset(dc, 0x00, sizeof(sar_dc_struct));
	dc->core_ptr = core_ptr;
	dc->option = &core_ptr->option;
	dc->scene = scene = core_ptr->scene;
	dc->object = core_ptr->object;
	dc->total_objects = core_ptr->total_objects;
	ptr = &core_ptr->object;
	total = &core_ptr->total_objects;
	dc->display = display = core_ptr->display;
	dc->gctl = gc = core_ptr->gctl;

	dc->camera_in_cockpit = False;
	dc->ear_in_cockpit = False;
	dc->flir = False;	/* Always false when drawing map */
	dc->camera_ref = SAR_CAMERA_REF_COCKPIT;
#if 0
/* Set later */
	dc->map_dxm = 0.0f;
	dc->map_dym = 0.0f;
#endif
	dc->lowest_cloud_layer_ptr = NULL;
	dc->highest_cloud_layer_ptr = NULL;
	dc->player_obj_cockpit_ptr = NULL;
	dc->player_flight_model_type = -1;
	dc->player_wheel_brakes = 0;
	dc->player_air_brakes = False;
	dc->player_autopilot = False;
	dc->player_stall = False;
	dc->player_overspeed = False;

	/* Select name base always starts at highest object index + 1
	 * which is the total number of objects (including NULL's).
	 */
	select_name_base = (GLuint)MAX(*total, 0);

	if((display == NULL) || (scene == NULL))
	    return;

	state = &display->state_gl;
	opt = dc->option;

	GWContextGet(
	    display, GWContextCurrent(display),
	    NULL, NULL,
	    NULL, NULL,
	    &width, &height
	);
	dc->width = width;
	dc->height = height;
	if((width <= 0) || (height <= 0))
	    return;

	/* Reset viewport and translations depending on draw style */
	if(draw_for_gcc || draw_for_ghc)
	{
	    /* Set up for ground contact check (gcc) or ground hit check 
	     * (ghc) drawing
	     */

	    /* Adjust the viewport to be a efficient small size (does
	     * not work in Windows)
	     */
	    dc->width = width = 100;
	    dc->height = height = 70;

	    /* Draw for ground contact check? */
	    if(draw_for_gcc)
	    {
		/* Delete the map draw object names list on the core */
		SARDrawMapObjNameListDelete(
		    &core_ptr->drawmap_objname,
		    &core_ptr->total_drawmap_objnames
		);

		/* Initialize GL select buffer and names stack, then
		 * switch to GL_SELECT render mode
		 */
		gl_select_buf_size = ((*total) * 4) + 512;
		gl_select_buf = (GLuint *)realloc(
		    gl_select_buf, gl_select_buf_size * sizeof(GLuint)
		);
		glSelectBuffer(gl_select_buf_size, gl_select_buf);
		glRenderMode(GL_SELECT);
	    }
	    /* Setup specifics for ground hit check? */
	    if(draw_for_ghc)
	    {
		/* Reset drawmap_ghc_result */
		core_ptr->drawmap_ghc_result = 0.0f;

		/* Set stencil buffer clear value to 0x0 */
		glClearStencil(0x0);
		/* Enable stencil buffer drawing and testing */
		StateGLEnable(state, GL_STENCIL_TEST);
		StateGLStencilFunc(
		    state,
		    GL_ALWAYS, 0x1, 0x1
		);
		StateGLStencilOp(
		    state,
		    GL_REPLACE, GL_REPLACE, GL_REPLACE
		);
	    }

	    /* Initialize select names, its safe to initialize this
	     * even if not in GL_SELECT render mode (OpenGL will
	     * ignore this)
	     */
	    glInitNames();
#ifdef GL_MAX_NAME_STACK_DEPTH
	    if(GL_MAX_NAME_STACK_DEPTH > 0)
#else
	    if(True)
#endif
	    {
		glPushName(select_name_base + 0);
	    }


#if 0
	    /* Reset viewport */
	    SARReshapeCB(0, core_ptr, -1, -1, -1, -1);
#endif

	    /* Change the viewport, since using our own width and height
	     * would differ from the actual viewport's size
	     */
	    glViewport(0, 0, width, height);

	    /* Calculate view aspect */
/* Do we need view aspect offset when drawing for contact check or
 * hit check?
 */
	    dc->view_aspect = view_aspect = ((float)width / (float)height) +
		display->aspect_offset;

	    /* Calculate field of view in unit meters about the z axis,
	     * this is a special coefficient that when used can give
	     * the width of the viewport at any given distance away
	     * from it, where:
	     * width_of_view = distance_away * fovz_um
	     */
	    dc->fovz_um = fovz_um = (float)(
		tan(scene->camera_fovz / 2.0) * 2.0
	    );

	    /* Set up projection matrix */
	    glMatrixMode(GL_PROJECTION);
	    glLoadIdentity();
	    /* Set perspective with short far clip but very close near
	     * clip since we are only interested in objects drawn very
	     * close to the camera.
	     */
	    gluPerspective(
		SFMRadiansToDegrees(scene->camera_fovz),        /* Field of viev */
		view_aspect,		/* View aspect */
		0.1,			/* Near clip */
		10000.0			/* Far clip */
/*              SFMFeetToMeters(SAR_MAX_MAP_VIEW_HEIGHT) */
	    );

	    /* Switch back to model view matrix */
	    glMatrixMode(GL_MODELVIEW);
	    glLoadIdentity();

	    /* Set camera location, this will set the camera position
	     * at the position of gcc_obj_num looking straight down
	     */
	    SARSetCameraMapDrawGCC(dc, gcc_obj_num);
	}
	else
	{
	    /* Set up for regular map drawing */

	    /* Reset viewport */
	    SARReshapeCB(0, core_ptr, -1, -1, -1, -1);

	    /* Calculate view aspect */
	    dc->view_aspect = view_aspect = ((float)width / (float)height) +
		display->aspect_offset;

	    /* Calculate field of view in unit meters about the z axis,
	     * this is a special coefficient that when used can give
	     * the width of the viewport at any given distance away
	     * from it, where:
	     *
	     * width_of_view = distance_away * fovz_um
	     */
	    dc->fovz_um = fovz_um = (float)(
		tan(scene->camera_fovz / 2.0) * 2.0
	    );

	    /* Set up projection matrix */
	    glMatrixMode(GL_PROJECTION);
	    glLoadIdentity();
	    /* Set perspective with zfar to be just a bit farther
	     * than the camera altitude from altitude of 0
	     */
	    gluPerspective(
		SFMRadiansToDegrees(scene->camera_fovz),	/* Field of view */
		view_aspect,			/* View aspect */
		1.0,				/* Near clip */
		scene->camera_map_pos.z + 100.0	/* Far clip */
/*		SFMFeetToMeters(SAR_MAX_MAP_VIEW_HEIGHT) */
	    );

	    /* Switch back to model view matrix */
	    glMatrixMode(GL_MODELVIEW);
	    glLoadIdentity();

	    /* Set camera location */
	    SARSetCamera(dc);
	}

	/* Calculate visible size based on camera height in meters */
	pos = &dc->camera_pos;
	dc->map_dym = map_dym = pos->z * fovz_um;
	dc->map_dxm = map_dxm = map_dym * view_aspect;
	map_radius = MAX(map_dxm, map_dym) / 2.0f;

	/* Calculate icon size based on camera height, each icon
	 * should be 0.05 the size of the field of view in meters
	 */
	icon_len = (float)((scene->camera_fovz * dc->camera_pos.z) * 0.05);

	/* Get pointer to ground base color, going to use this to clear
	 * the frame buffer if appropriate
	 */
	c = &scene->base.color;

	/* Clear GL depth buffer and color, match color to be
	 * the color of the ground base
	 */
	glClearDepth((GLclampd)1.0);
	glClearColor(
	    c->r,
	    c->g,
	    c->b,
	    (draw_for_ghc) ? 0.0f : c->a
	);
	glClear(
	    GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT |
	    ((draw_for_ghc) ? GL_STENCIL_BUFFER_BIT : 0)
	);


	/* Disable fog */
	StateGLDisable(state, GL_FOG);

	/* Turn off depth testing and turn on depth writing for
	 * drawing the foundations (the ground base visual models)
	 */
	StateGLDisable(state, GL_LIGHTING);
#if 0
/* Do not disable each individual light, preserve their original
 * states
 */
	StateGLDisable(state, GL_LIGHT0);
	StateGLDisable(state, GL_LIGHT1);
#endif
	StateGLDisable(state, GL_COLOR_MATERIAL);

	StateGLDisable(state, GL_BLEND);

	SAR_DRAW_DEPTH_TEST_OFF
	StateGLDepthMask(state, GL_FALSE);

	StateGLShadeModel(state, GL_FLAT);

	SAR_DRAW_TEXTURE_1D_OFF

	SAR_DRAW_TEXTURE_2D_OFF


	/* Begin drawing */

	/* Draw ground base only when NOT doing ground hit check */
	if(!draw_for_ghc)
	{
	    /* Draw ground base */
	    SARDrawSceneFoundations(dc);
	}

	/* Textured objects enabled? */
	if(opt->textured_objects)
	{
	    /* Enable alpha testing and turn on GL_TEXTURE_2D */
	    StateGLEnable(state, GL_ALPHA_TEST);
	    StateGLAlphaFunc(state, GL_GREATER, 0.5);

	    SAR_DRAW_TEXTURE_2D_ON
	}
	else
	{
	    /* No textured objects, so no alpha testing and no
	     * GL_TEXTURE_2D
	     */
	    SAR_DRAW_TEXTURE_2D_OFF
	}
	/* Don't use 1D texture */
	SAR_DRAW_TEXTURE_1D_OFF

	/* Begin drawing objects */
	for(i = 0; i < *total; i++)
	{
	    obj_ptr = (*ptr)[i];
	    if(obj_ptr == NULL)
		continue;

	    /* Enable depth testing? */
	    if(obj_ptr->flags & SAR_OBJ_FLAG_NO_DEPTH_TEST)
	    {
		SAR_DRAW_DEPTH_TEST_OFF
		StateGLDepthMask(state, GL_FALSE);
	    }
	    else if(!(obj_ptr->flags & SAR_OBJ_FLAG_NO_DEPTH_TEST))
	    {
		SAR_DRAW_DEPTH_TEST_ON
		StateGLDepthMask(state, GL_TRUE);
	    }

	    /* Set object's shade model */
	    if(obj_ptr->flags & SAR_OBJ_FLAG_SHADE_MODEL_SMOOTH)
	    {
		StateGLShadeModel(state, GL_SMOOTH);
	    }
	    else
	    {
		StateGLShadeModel(state, GL_FLAT);
	    }

	    /* Draw by object type */
	    switch(obj_ptr->type)
	    {
	      case SAR_OBJ_TYPE_GARBAGE:
		break;

	      case SAR_OBJ_TYPE_STATIC:
	      case SAR_OBJ_TYPE_AUTOMOBILE:
	      case SAR_OBJ_TYPE_WATERCRAFT:
	      case SAR_OBJ_TYPE_GROUND:
		/* Get position and check if in range with camera */
		pos = &obj_ptr->pos;
		distance = (float)SFMHypot2(
		    pos->x - dc->camera_pos.x, pos->y - dc->camera_pos.y
		);
		if(distance > obj_ptr->range)
		    break;

		/* Set GL name as this object's index number */
		glLoadName((GLuint)i);

		/* Get direction */
		dir = &obj_ptr->dir;

		glPushMatrix();
		{
		    /* Translate and rotate */
		    glTranslatef(pos->x, 0.0f, -pos->y);
		    if(dir->heading != 0)
			glRotatef(
			    (GLfloat)-SFMRadiansToDegrees(dir->heading),
			    0.0f, 1.0f, 0.0f
			);
		    if(dir->pitch != 0)
			glRotatef(
			    (GLfloat)-SFMRadiansToDegrees(dir->pitch),
			    1.0f, 0.0f, 0.0f
			);
		    if(dir->bank != 0)
			glRotatef(
			    (GLfloat)-SFMRadiansToDegrees(dir->bank),
			    0.0f, 0.0f, 1.0f
			);

		    /* Draw standard model if defined */
		    SARVisualModelCallList(obj_ptr->visual_model);
		}
		glPopMatrix();
		SAR_DRAW_POST_CALLLIST_RESET_STATES
		break;

	      case SAR_OBJ_TYPE_AIRCRAFT:
		aircraft = SAR_OBJ_GET_AIRCRAFT(obj_ptr);
		if(aircraft == NULL)
		    break;

		/* Set GL name as this object's index number */
		glLoadName((GLuint)i);

		/* Do not draw this type for ground contact check */
		if(draw_for_gcc || draw_for_ghc)
		    break;

		/* Update draw context information if this is the player
		 * object
		 */
		if((obj_ptr == scene->player_obj_ptr) &&
		   (gc != NULL)
		)
		{
		    /* Wheel brakes state */
		    dc->player_wheel_brakes = gc->wheel_brakes_state;
		    if((dc->player_wheel_brakes == 0) &&
		       (gc->wheel_brakes_coeff > 0.0f)
		    )
			dc->player_wheel_brakes = 1;

		    /* Air brakes state */
/*		    dc->player_air_brakes = gc->air_brakes_state; */
		    dc->player_air_brakes = (aircraft->air_brakes_state > 0) ? True : False;

		    /* Flight model */
		    dc->player_flight_model_type =
			aircraft->flight_model_type;

		    /* Auto pilot */
		    dc->player_autopilot = (aircraft->autopilot_state == SAR_AUTOPILOT_ON) ? True : False;

		    /* Overspeed */
		    double rel_speed = SFMHypot2(aircraft->airspeed.y, aircraft->airspeed.z);
		    if((aircraft->overspeed_expected > 0.0f) &&
		       (rel_speed > aircraft->overspeed_expected)
		    )
			dc->player_overspeed = True;

		    /* Update sounds */
		    if(opt->event_sounds)
		    {
			/* Stall warning */
			dc->player_stall = SARSoundStallUpdate(
			    core_ptr->recorder,
			    obj_ptr->sndsrc, obj_ptr->total_sndsrcs,
			    &aircraft->stall_sndplay,
			    aircraft->stall_sndsrc,
			    dc->ear_in_cockpit,
			    aircraft->flight_model_type,
			    aircraft->landed,
			    rel_speed,
			    SARSimStallSpeed(aircraft)
			);

			/* Overspeed */
			SARSoundOverSpeedUpdate(
			    core_ptr->recorder,
			    obj_ptr->sndsrc, obj_ptr->total_sndsrcs,
			    &aircraft->overspeed_sndplay,
			    aircraft->overspeed_sndsrc,
			    dc->ear_in_cockpit,
			    dc->player_overspeed
			);
		    }
		}

		/* Always mute engine sounds in map view */
		SARSoundEngineMute(
		    core_ptr->recorder,
		    aircraft->engine_inside_sndplay, 
		    aircraft->engine_outside_sndplay 
		);

		/* Get position and direction, we will be drawing the
		 * intercept lines first unconditionally to range
		 * since we want intercept lines always visible
		 */
		pos = &obj_ptr->pos;
		dir = &obj_ptr->dir;

		/* Unselect texture */
		V3DTextureSelect(NULL);

		/* Is this the player object? */
		if((obj_ptr == scene->player_obj_ptr) &&
		   (aircraft->total_intercepts > 0)
		)
		{
		    sar_intercept_struct *intercept_ptr;

		    /* Draw intercept lines and waypoints */
		    glBegin(GL_LINES);
		    {
			glNormal3f(0.0f, 1.0f, 0.0f);

			/* Draw subsequent (not current) intercept
			 * lines first
			 */
			glColor4f(0.8f, 0.0f, 0.0f, 1.0f);
			for(n = MAX(aircraft->cur_intercept, 0);
			    n < (aircraft->total_intercepts - 1);
			    n++
			)
			{
			    intercept_ptr = aircraft->intercept[n];
			    /* Give up if any intercepts are NULL */
			    if(intercept_ptr == NULL)
				break;

			    glVertex3f(
				intercept_ptr->x, 0.0f, -intercept_ptr->y
			    );

			    if((n + 1) < aircraft->total_intercepts)
				intercept_ptr = aircraft->intercept[n + 1];
			    else
				intercept_ptr = NULL;
			    if(intercept_ptr != NULL)
				glVertex3f(
				    intercept_ptr->x, 0.0f, -intercept_ptr->y
				);
			}

			/* Draw intercept from current position to current
			 * intercept waypoint.
			 */
			glColor4f(1.0f, 0.25f, 0.25f, 1.0f);
			n = aircraft->cur_intercept;
			if((n >= 0) && (n < aircraft->total_intercepts))
			{
			    intercept_ptr = aircraft->intercept[n];
			    if(intercept_ptr != NULL)
			    {
				glVertex3f(pos->x, 0.0f, -pos->y);
				glVertex3f(
				    intercept_ptr->x, 0.0f, -intercept_ptr->y
				);
			    }
			}
		    }
		    glEnd();
		}

#if 0
/* Skip range check for aircrafts, aircrafts should always
 * be drawn on map during regular redraws
 */
		distance = (float)SFMHypot2(
		    pos->x - dc->camera_pos.x, pos->y - dc->camera_pos.y
		);
		if(distance > obj_ptr->range)
		    break;
#endif

		/* Begin drawing aircraft `icon' */
		glPushMatrix();
		{  
		    /* Translate and rotate */
		    glTranslatef(pos->x, 0.0f, -pos->y);
		    if(dir->heading != 0)
			glRotatef(
			    (GLfloat)-SFMRadiansToDegrees(dir->heading),
			    0.0f, 1.0f, 0.0f
			);

		    glBegin(GL_LINES);
		    {
			glNormal3f(0.0f, 1.0f, 0.0f);

			if(obj_ptr == scene->player_obj_ptr)
			    glColor4f(1.0f, 1.0f, 0.0f, 1.0f);
			else
			    glColor4f(1.0f, 0.0f, 0.0f, 1.0f);

			glVertex3f(0.0f, 0.0f, 0.0f);
			glVertex3f(0.0f, 0.0f, -(1.0f * icon_len));

			glVertex3f(-0.5f * icon_len, 0.0f, 0.0f);
			glVertex3f(0.5f * icon_len, 0.0f, 0.0f);
		    }
		    glEnd();
		}    
		glPopMatrix();
		SAR_DRAW_POST_CALLLIST_RESET_STATES
		break;

	      case SAR_OBJ_TYPE_HELIPAD:
		obj_helipad_ptr = SAR_OBJ_GET_HELIPAD(obj_ptr);
		if(obj_helipad_ptr == NULL)
		    break;

		/* Set GL name as this object's index number */
		glLoadName((GLuint)i);

		/* Get position and check if in range with camera */
		pos = &obj_ptr->pos;
		distance = (float)SFMHypot2(
		    pos->x - dc->camera_pos.x, pos->y - dc->camera_pos.y
		);
		if(distance > (obj_ptr->range + map_radius))
		    break;

		/* Calculate 3D distance */
		distance3d = (float)SFMHypot2(
		    distance, pos->z - dc->camera_pos.z
		);

		/* Get direction */
		dir = &obj_ptr->dir;

		glPushMatrix();
		{
		  if(draw_for_gcc || draw_for_ghc)
		  {
		    /* Translate and rotate */
		    glTranslatef(pos->x, pos->z, -pos->y);
		    if(dir->heading != 0)
			glRotatef(
			    (GLfloat)-SFMRadiansToDegrees(dir->heading),
			    0.0f, 1.0f, 0.0f
			);
		    if(dir->pitch != 0)
			glRotatef(
			    (GLfloat)-SFMRadiansToDegrees(dir->pitch),
			    1.0f, 0.0f, 0.0f
			);
		    if(dir->bank != 0)
			glRotatef(
			    (GLfloat)-SFMRadiansToDegrees(dir->bank),
			    0.0f, 0.0f, 1.0f
			);

		    /* Draw helipad */
		    SARDrawHelipad(
			dc, obj_ptr, obj_helipad_ptr, distance3d
		    );
		  } 
		  else 
		  {
		    /* Translate and rotate */
		    glTranslatef(pos->x, 0.0f, -pos->y);
		    if(dir->heading != 0)
			glRotatef(
			    (GLfloat)-SFMRadiansToDegrees(dir->heading),
			    0.0f, 1.0f, 0.0f
			);
		    if(dir->pitch != 0)
			glRotatef(
			    (GLfloat)-SFMRadiansToDegrees(dir->pitch),
			    1.0f, 0.0f, 0.0f
			);
		    if(dir->bank != 0)
			glRotatef(
			    (GLfloat)-SFMRadiansToDegrees(dir->bank),
			    0.0f, 0.0f, 1.0f
			);

		    /* Draw helipad for map view */
		    SARDrawHelipadMap(
			dc, obj_ptr, obj_helipad_ptr,
			icon_len
		    );
		  }
		}
		glPopMatrix();
		SAR_DRAW_POST_CALLLIST_RESET_STATES
		break;

	      case SAR_OBJ_TYPE_RUNWAY:
		obj_runway_ptr = SAR_OBJ_GET_RUNWAY(obj_ptr);
		if(obj_runway_ptr == NULL)
		    break;

		/* Set GL name as this object's inedx number */
		glLoadName((GLuint)i);

		/* Get position and check if in range with camera */     
		pos = &obj_ptr->pos;
		distance = (float)SFMHypot2(
		    pos->x - dc->camera_pos.x, pos->y - dc->camera_pos.y
		);
		if(distance > (obj_ptr->range + map_radius))
		    break;

		/* Calculate 3D distance */
		distance3d = (float)SFMHypot2(
		    distance, pos->z - dc->camera_pos.z
		);

		/* Get direction */
		dir = &obj_ptr->dir;

		glPushMatrix();
		{
		  if(draw_for_gcc || draw_for_ghc)
		  {
		    /* Translate and rotate */
		    glTranslatef(pos->x, pos->z, -pos->y);
		    if(dir->heading != 0)
			glRotatef(
			    (GLfloat)-SFMRadiansToDegrees(dir->heading),
			    0.0f, 1.0f, 0.0f
			);
		    if(dir->pitch != 0)
			glRotatef(
			    (GLfloat)-SFMRadiansToDegrees(dir->pitch),
			    1.0f, 0.0f, 0.0f
			);
		    if(dir->bank != 0)
			glRotatef(
			    (GLfloat)-SFMRadiansToDegrees(dir->bank),
			    0.0f, 0.0f, 1.0f
			);
		    /* Draw runway */
		    SARDrawRunway(
			dc, obj_ptr, obj_runway_ptr, distance3d
		    );
		  }
		  else
		  {
		    /* Translate and rotate */
		    glTranslatef(pos->x, 0.0f, -pos->y);
		    if(dir->heading != 0)
			glRotatef(
			    (GLfloat)-SFMRadiansToDegrees(dir->heading),
			    0.0, 1.0, 0.0
			);
		    if(dir->pitch != 0)
			glRotatef(
			    (GLfloat)-SFMRadiansToDegrees(dir->pitch),
			    1.0, 0.0, 0.0
			);
		    if(dir->bank != 0)
			glRotatef(
			    (GLfloat)-SFMRadiansToDegrees(dir->bank),
			    0.0, 0.0, 1.0
			);
		    /* Draw runway for map view */
		    SARDrawRunwayMap(
			dc, obj_ptr, obj_runway_ptr, icon_len
		    );
		  }
		}
		glPopMatrix();
		SAR_DRAW_POST_CALLLIST_RESET_STATES
		break;

	      case SAR_OBJ_TYPE_HUMAN:
	      case SAR_OBJ_TYPE_SMOKE:
	      case SAR_OBJ_TYPE_FIRE:
	      case SAR_OBJ_TYPE_EXPLOSION:
	      case SAR_OBJ_TYPE_CHEMICAL_SPRAY:
	      case SAR_OBJ_TYPE_FUELTANK:
		break;

	      case SAR_OBJ_TYPE_PREMODELED:
		/* Get position and check if in range with camera */
		pos = &obj_ptr->pos;
		distance = (float)SFMHypot2(
		    pos->x - dc->camera_pos.x, pos->y - dc->camera_pos.y
		);
		if(distance > obj_ptr->range)
		    break;

		/* Set GL name as this object's inedx number */
		glLoadName((GLuint)i);

		/* Get direction */
		dir = &obj_ptr->dir;

		/* Calculate 3D distance */
		distance3d = (float)SFMHypot2(
		    distance, pos->z - dc->camera_pos.z
		);

		glPushMatrix();
		{
		    /* Translate and rotate */
		    glTranslatef(pos->x, pos->z, -pos->y);
		    if(dir->heading != 0)
			glRotatef(
			    (GLfloat)-SFMRadiansToDegrees(dir->heading),
			    0.0f, 1.0f, 0.0f
			);
		    if(dir->pitch != 0)
			glRotatef(
			    (GLfloat)-SFMRadiansToDegrees(dir->pitch),
			    1.0f, 0.0f, 0.0f
			);
		    if(dir->bank != 0)
			glRotatef(
			    (GLfloat)-SFMRadiansToDegrees(dir->bank),
			    0.0f, 0.0f, 1.0f
			);

		    /* Draw premodeled object */
		    SARDrawPremodeled(
			dc, obj_ptr,
			SAR_OBJ_GET_PREMODELED(obj_ptr),
			distance3d,
			(Boolean)((draw_for_gcc || draw_for_ghc) ? True : False)
		    );
		}
		glPopMatrix();
		SAR_DRAW_POST_CALLLIST_RESET_STATES
		break;
	    }
	}


	/* Was drawing for ground contact check? */
	if(draw_for_gcc)
	{
	    /* Flush output and switch back to render mode. Tabulate
	     * hits if any.
	     */
	    GLint hits;

	    glFlush();	/* Demos suggest that this is needed */
	    hits = glRenderMode(GL_RENDER);
	    if(hits > 0)
		SARDrawMapProcessHits(core_ptr, hits, gl_select_buf);
	    else if((hits == -1) && opt->runtime_debug)
		fprintf(
		    stderr,
		    "SARDrawMap(): Selection buffer overflowed (allocated to %i values).\n",
		    gl_select_buf_size
		);
		

	    /* Need to restore original viewport size since width
	     * and height have been decreased for this case.
	     */
	    GWContextGet(
		display, GWContextCurrent(display),
		NULL, NULL,
		NULL, NULL,
		&width, &height
	    );
	    dc->width = width;
	    dc->height = height;
	    glViewport(0, 0, width, height);
	}
	/* Was drawing for ground hit check? */
	else if(draw_for_ghc)
	{
	    /* Flush output and switch back to render mode. Read back
	     * frame buffer at center of window coordinates.
	     */
	    GLubyte *bptr, *bend;
#define READ_BACK_BUF_WIDTH	2
#define READ_BACK_BUF_HEIGHT	2
#define READ_BACK_BUF_LEN	(READ_BACK_BUF_WIDTH * READ_BACK_BUF_HEIGHT)
	    GLubyte read_back_buf[READ_BACK_BUF_LEN];

/*	    glFlush();	Don't need to flush everything to read pixels */

	    /* Read back drawn frame buffer's alpha channel */
	    if(display->has_double_buffer)
		glReadBuffer(GL_BACK);
	    else
		glReadBuffer(GL_FRONT);
	    glReadPixels(
		(GLint)(width / 2), (GLint)(height / 2),
		(GLsizei)READ_BACK_BUF_WIDTH, (GLsizei)READ_BACK_BUF_HEIGHT,
		GL_STENCIL_INDEX, GL_UNSIGNED_BYTE,
		read_back_buf
	    );

	    /* Set core_ptr->drawmap_ghc_result (note that it was already
	     * reset at the start of this function.
	     */
	    for(bptr = read_back_buf, 
		bend = read_back_buf + (READ_BACK_BUF_LEN * 1);
		bptr < bend;
		bptr += 1
	    )
	    {
		if(*bptr != 0x0)
		    core_ptr->drawmap_ghc_result = 1.0f;
	    }
#undef READ_BACK_BUF_WIDTH
#undef READ_BACK_BUF_HEIGHT
#undef READ_BACK_BUF_LEN

#if 0
	    if(True)
	    {
 char tmp_text[256];
 sprintf(tmp_text, "Got ghc 0x%.2x %.4f",
  read_back_buf[0], core_ptr->drawmap_ghc_result
 );
 SARMessageAdd(scene, tmp_text);
	    }
#endif

	    /* Disable stencil buffer, don't need it anymore */
	    StateGLDisable(state, GL_STENCIL_TEST);

	    /* Need to restore original viewport size since width
	     * and height have been decreased for this case.
	     */
	    GWContextGet(
		display, GWContextCurrent(display),
		NULL, NULL,
		NULL, NULL,
		&width, &height
	    );
	    dc->width = width;
	    dc->height = height;
	    glViewport(0, 0, width, height);
	}
	/* Was doing regular map drawing? */
	else
	{
	    /* Draw extra super imposed details if not drawing for ground
	     * contact check.
	     */
	    StateGLDisable(state, GL_ALPHA_TEST);
	    StateGLDisable(state, GL_COLOR_MATERIAL);
	    SAR_DRAW_DEPTH_TEST_OFF
	    SAR_DRAW_TEXTURE_1D_OFF
	    SAR_DRAW_TEXTURE_2D_OFF

	    /* Draw map grids (before ortho) */
	    SARDrawMapGrids(dc, &dc->camera_pos);

	    /* Set up gl states for 2d drawing */
	    GWOrtho2D(display);

	    /* Draw map cross hairs */
	    SARDrawMapCrossHairs(dc);

	    /* Draw attitude and stats of player object */
	    if(scene->player_obj_ptr != NULL)
	    {
		glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
		SARDrawOutsideAttitude(dc, scene->player_obj_ptr);
	    }

	    /* Begin drawing text */

	    /* Draw control messages */
	    SARDrawControlMessages(dc);

	    /* Camera title */
/*	    SARDrawCameraRefTitle(dc); */

	    /* Sticky banner messages */
	    SARDrawBanner(dc);

	    /* Messages */
	    SARDrawMessages(dc);

	    /* Draw text input prompt */
	    SARTextInputDraw(core_ptr->text_input);

	    /* Draw help */
	    if(core_ptr->display_help > 0)
		SARDrawHelp(dc);

	    /* Put GL buffer to window */
	    GWSwapBuffer(display);
	}

	/* Delete GL select buffer as needed */
	if(gl_select_buf != NULL)
	{
	    free(gl_select_buf);
	    gl_select_buf = NULL;
	    gl_select_buf_size = 0;
	}

	/* Report any errors */
	if(opt->runtime_debug)
	{
	    GLenum error_code = glGetError();
	    if(error_code != GL_NO_ERROR)
		SARReportGLError(core_ptr, error_code);
	}

#undef SELECT_BUF_SIZE
}           
