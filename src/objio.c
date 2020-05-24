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

#include "../include/string.h"
#include "../include/fio.h"
#include "../include/disk.h"

#include "gw.h"
#include "x3d.h"
#include "text3d.h"
#include "sfm.h"
#include "v3dhf.h"
#include "v3dtex.h"
#include "v3dmh.h"
#include "v3dmp.h"
#include "v3dmodel.h"
#include "v3dfio.h"

#include "cp.h"
#include "cpfio.h"
#include "sarreality.h"
#include "obj.h"
#include "sar.h"
#include "smoke.h"
#include "fire.h"
#include "weather.h"
#include "sartime.h"
#include "sarfio.h"
#include "simutils.h"
#include "simop.h"
#include "objsound.h"
#include "objutils.h"
#include "objio.h"
#include "config.h"

#include "runway/runway_displaced_threshold.x3d"
#include "runway/runway_midway_markers.x3d"
#include "runway/runway_td_markers.x3d"
#include "runway/runway_threshold.x3d"


/* Utilities */
static const char *NEXT_ARG(const char *s);
static const char *GET_ARG_S(const char *s, char **v);
static const char *GET_ARG_B(const char *s, Boolean *v);
static const char *GET_ARG_I(const char *s, int *v);
static const char *GET_ARG_F(const char *s, float *v);
static const char *GET_ARG_RGBA(const char *s, sar_color_struct *v);
static const char *GET_ARG_POS(const char *s, sar_position_struct *v);
static const char *GET_ARG_DIR(const char *s, sar_direction_struct *v);

/* Object Postload Checking */
static int SARObjLoadPostLoadCheck(
	int obj_num, sar_object_struct *obj_ptr,
	const char *filename
);

/* Visual Model Loading */
sar_visual_model_struct *SARObjLoadX3DDataVisualModel(
	sar_core_struct *core_ptr, sar_scene_struct *scene,
	char **x3d_data,
	const sar_scale_struct *scale
);
sar_visual_model_struct *SARObjLoadTextVisualModel(
	sar_core_struct *core_ptr, sar_scene_struct *scene,
	float font_width, float font_height,	/* In meters */
	float character_spacing,		/* In meters */
	const char *s,
	float *string_width			/* In meters */
);

/* Parameter Loading */
int SARObjLoadTranslate(
	sar_core_struct *core_ptr, sar_scene_struct *scene,
	sar_object_struct *obj_ptr, sar_parm_translate_struct *p_translate
);
int SARObjLoadTranslateRandom(
	sar_core_struct *core_ptr, sar_scene_struct *scene,
	sar_object_struct *obj_ptr,
	sar_parm_translate_random_struct *p_translate_random
);
static int SARObjLoadSoundSource(
	sar_scene_struct *scene,
	sar_sound_source_struct ***list, int *total,
	const char *line, const char *filename, int line_num
);
int SARObjLoadTexture(
	sar_core_struct *core_ptr, sar_scene_struct *scene,
	sar_parm_texture_load_struct *p_texture_load
);
int SARObjLoadHelipad(
	sar_core_struct *core_ptr, sar_scene_struct *scene,
	sar_parm_new_helipad_struct *p_new_helipad
);
int SARObjLoadRunway(
	sar_core_struct *core_ptr, sar_scene_struct *scene,
	sar_parm_new_runway_struct *p_new_runway
);
int SARObjLoadHuman(
	sar_core_struct *core_ptr, sar_scene_struct *scene,
	sar_parm_new_human_struct *p_new_human
);
int SARObjLoadFire(
	sar_core_struct *core_ptr, sar_scene_struct *scene,
	sar_parm_new_fire_struct *p_new_fire
);
int SARObjLoadSmoke(
	sar_core_struct *core_ptr, sar_scene_struct *scene,
	sar_parm_new_smoke_struct *p_new_smoke
);
int SARObjLoadHeightField(
	sar_core_struct *core_ptr,
	int obj_num, sar_object_struct *obj_ptr,
	sar_visual_model_struct *vmodel,
	void *p, const char *filename, int line_num,
	GLuint list
);

static void SARObjLoadProcessVisualPrimitive(
	sar_core_struct *core_ptr,
	int obj_num, sar_object_struct *obj_ptr,
	sar_visual_model_struct *vmodel,
	void *p, const char *filename, int line_num
);
static void SARObjLoadProcessVisualModel(
	sar_core_struct *core_ptr,
	int obj_num, sar_object_struct *obj_ptr,
	sar_visual_model_struct *vmodel,
	v3d_model_struct *v3d_model,
	Boolean process_as_ir,
	const char *filename, int line_num
);

static void SARObjLoadLine(
	sar_core_struct *core_ptr,
	int obj_num, sar_object_struct *obj_ptr,
	const char *line, const char *filename, int line_num
);

int SARObjLoadFromFile(
	sar_core_struct *core_ptr, int obj_num, const char *filename
);


#ifndef SAR_COMMENT_CHAR
# define SAR_COMMENT_CHAR       '#'
#endif


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

#define ISCOMMENT(c)	((c) == SAR_COMMENT_CHAR)
#define ISCR(c)		(((c) == '\n') || ((c) == '\r'))


static int last_begin_primitive_type;


/*
 *	Texture plane orientation codes:
 */
#define TEX_ORIENT_NONE	0
#define TEX_ORIENT_XY	1
#define TEX_ORIENT_YZ	2
#define TEX_ORIENT_XZ	3

static Boolean tex_on;
static int tex_orient;	/* One of TEX_ORIENT_* */

typedef struct {
	float i, j;	/* Position of `upper left' in meters */
	float w, h;	/* Size `to lower right' in meters */
} tex_coord_struct;
static tex_coord_struct tex_coord;


/*
 *	Seeks s past the current argument (if any) then past any blank
 *	characters until the next argument or end of string is reached.
 */
static const char *NEXT_ARG(const char *s)
{
	if(STRISEMPTY(s))
	    return(s);

	while(!ISBLANK(*s) && (*s != '\0'))
	    s++;
	while(ISBLANK(*s))
	    s++;

	return(s);
}

/*
 *	Gets the argument at the current position of the string s, the
 *	return value will be a dynamically allocated string.
 *
 *	Returns the pointer to the next argument or NULL if the end of
 *	the string is reached.
 */
static const char *GET_ARG_S(const char *s, char **v)
{
	if(STRISEMPTY(s))
	    return(s);

	if(*s == '\"')
	{
	    int len;
	    const char *end = s + 1;

	    s++;	/* Skip first quote character */

	    /* Calculate length of word in quotes, seek until next
	     * unescaped quote character or end of string
	     */
	    while((*end != '\"') && (*end != '\0'))
	    {
		if(*end == '\\')
		{
		    end++;
		    if(*end != '\0')
			end++;
		}
		else
		    end++;
	    }
	    len = end - s;

	    if(len > 0)
	    {
		char *s2 = (char *)malloc(len + 1);
		memcpy(s2, s, len);
		s2[len] = '\0';

		free(*v);
		*v = s2;
	    }

	    return(NEXT_ARG(end));
	}
	else
	{
	    int len;
	    const char *end = s;

	    /* Calculate length of word, seek until next blank character
	     * or end of string
	     */
	    while(!ISBLANK(*end) && (*end != '\0'))
		end++;
	    len = end - s;

	    if(len > 0)
	    {
		char *s2 = (char *)malloc(len + 1);
		memcpy(s2, s, len);
		s2[len] = '\0';

		free(*v);
		*v = s2;
	    }

	    return(NEXT_ARG(s));
	}
}

/*
 *      Gets the argument at the current position of the string s, the
 *      return value will a Boolean.
 *
 *      Returns the pointer to the next argument or NULL if the end of
 *      the string is reached.
 */
static const char *GET_ARG_B(const char *s, Boolean *v)
{
	if(STRISEMPTY(s))
	    return(s);

	if(*s == '\"')
	{
	    char c = s[1];
	    const char *end = s + 1;

	    s++;	/* Skip first quote character */

	    /* Calculate length of word in quotes, seek until next
	     * unescaped quote character or end of string
	     */
	    while((*end != '\"') && (*end != '\0'))
	    {
		if(*end == '\\')
		{
		    end++;
		    if(*end != '\0')
			end++;
		}
		else
		    end++;
	    }

	    /* Zero? */
	    if(c == '0')
		*v = False;
	    /* No? */
	    else if(toupper(c) == 'N')
		*v = False;
	    /* On or off? */
	    else if(toupper(c) == 'O')
		*v = (toupper(s[1]) == 'N') ? True : False;
	    /* All else assume true */
	    else
		*v = True;

	    return(NEXT_ARG(end));
	}
	else
	{
	    char c = *s;

	    /* Zero? */
	    if(c == '0')
		*v = False;
	    /* No? */
	    else if(toupper(c) == 'N')
		*v = False;
	    /* On or off? */
	    else if(toupper(c) == 'O')
		*v = (toupper(s[1]) == 'N') ? True : False;
	    /* All else assume true */
	    else
		*v = True;

	    return(NEXT_ARG(s));
	}
}

/*
 *      Gets the argument at the current position of the string s, the
 *      return value will an int.
 *
 *      Returns the pointer to the next argument or NULL if the end of
 *      the string is reached.
 */
static const char *GET_ARG_I(const char *s, int *v)
{
	if(STRISEMPTY(s))
	    return(s);

	if(*s == '\"')
	{
	    const char *end = s + 1;

	    s++;	/* Skip first quote character */

	    /* Calculate length of word in quotes, seek until next
	     * unescaped quote character or end of string
	     */
	    while((*end != '\"') && (*end != '\0'))
	    {
		if(*end == '\\')
		{
		    end++;
		    if(*end != '\0')
			end++;
		}
		else
		    end++;
	    }

	    *v = ATOI(s);
	    return(NEXT_ARG(end));
	}
	else
	{
	    *v = ATOI(s);
	    return(NEXT_ARG(s));
	}
}

/*
 *      Gets the argument at the current position of the string s, the
 *      return value will an float.
 *
 *      Returns the pointer to the next argument or NULL if the end of
 *      the string is reached.
 */
static const char *GET_ARG_F(const char *s, float *v)
{
	if(STRISEMPTY(s))
	    return(s);

	if(*s == '\"')
	{
	    const char *end = s + 1;

	    s++;        /* Skip first quote character */

	    /* Calculate length of word in quotes, seek until next
	     * unescaped quote character or end of string
	     */
	    while((*end != '\"') && (*end != '\0'))
	    {
		if(*end == '\\')
		{
		    end++;
		    if(*end != '\0')
			end++;
		}
		else
		    end++;
	    }

	    *v = ATOF(s);
	    return(NEXT_ARG(end));
	}
	else
	{
	    *v = ATOF(s);
	    return(NEXT_ARG(s));
	}
}

/*
 *      Gets the argument at the current position of the string s, the
 *      return value will a sar_color_struct.
 *
 *      Returns the pointer to the next argument or NULL if the end of
 *      the string is reached.
 */
static const char *GET_ARG_RGBA(const char *s, sar_color_struct *v)
{
	if(STRISEMPTY(s))
	    return(s);

	if(*s == '\"')
	{
	    s++;

	    if(*s == '#')
	    {
		unsigned int r, g, b, a;
		const char *end = s + 1;

		s++;		/* Skip '#' character */

		/* Calculate length of word in quotes, seek until next
		 * unescaped quote character or end of string
		 */
		while((*end != '\"') && (*end != '\0'))
		{
		    if(*end == '\\')
		    {
			end++;
			if(*end != '\0')
			    end++;
		    }
		    else
			end++;
		}

		sscanf(
		    s,
		    "%2x%2x%2x%2x",
		    &a, &r, &g, &b
		);

		v->a = (float)a / (float)0xff;
		v->r = (float)r / (float)0xff;
		v->g = (float)g / (float)0xff;
		v->b = (float)b / (float)0xff;

		return(NEXT_ARG(end));
	    }
	    else
	    {
		s--;

		s = GET_ARG_F(s, &v->r);
		s = GET_ARG_F(s, &v->g);
		s = GET_ARG_F(s, &v->b);
		s = GET_ARG_F(s, &v->a);

		return(s);
	    }
	}
	else
	{
	    s = GET_ARG_F(s, &v->r);
	    s = GET_ARG_F(s, &v->g);
	    s = GET_ARG_F(s, &v->b);
	    s = GET_ARG_F(s, &v->a);

	    return(s);
	}
}

/*
 *      Gets the argument at the current position of the string s, the
 *      return value will a sar_position_struct.
 *
 *      Returns the pointer to the next argument or NULL if the end of
 *      the string is reached.
 */
static const char *GET_ARG_POS(const char *s, sar_position_struct *v)
{
	if(STRISEMPTY(s))
	    return(s);

	s = GET_ARG_F(s, &v->x);
	s = GET_ARG_F(s, &v->y);
	s = GET_ARG_F(s, &v->z);

	return(s);
}

/*
 *      Gets the argument at the current position of the string s, the
 *      return value will a sar_direction_struct.
 *
 *      Returns the pointer to the next argument or NULL if the end of
 *      the string is reached.
 */
static const char *GET_ARG_DIR(const char *s, sar_direction_struct *v)
{
	float d = 0.0f;

	if(STRISEMPTY(s))
	    return(s);

	s = GET_ARG_F(s, &d);
	v->heading = (float)DEGTORAD(d);
	s = GET_ARG_F(s, &d);
	v->pitch = (float)DEGTORAD(d);
	s = GET_ARG_F(s, &d);
	v->bank = (float)DEGTORAD(d);

	return(s);
}

/*
 *	Completes the specified path by checking of the object that
 *	the path refers to exists locally or globally.
 *
 *	Returns the full path to the object, the returned pointer must
 *	be deleted by the calling function.
 */
char *COMPLETE_PATH(const char *path)
{
	char *full_path;

	if(path == NULL)
	    return(NULL);

	if(ISPATHABSOLUTE(path))
	{
	    full_path = STRDUP(path);
	}
	else
	{   
	    const char *s = PrefixPaths(dname.local_data, path);
	    struct stat stat_buf;
	    if((s != NULL) ? stat(s, &stat_buf) : True)
		s = PrefixPaths(dname.global_data, path);
	    full_path = STRDUP(s);
	}

	return(full_path);
}


/*
 *	Object postload checking
 *
 *	Returns number of errors.
 */
static int SARObjLoadPostLoadCheck(
	int obj_num, sar_object_struct *obj_ptr,
	const char *filename
)
{
	int	warnings = 0,
		errors = 0;
	sar_object_aircraft_struct *aircraft;
	sar_object_ground_struct *ground;

	if(obj_ptr == NULL)
	{
	    errors++;
	    return(errors);
	}

	/* Invalid object type? */
	if(obj_ptr->type < 0)
	{
	    warnings++;
	    fprintf(
		stderr,
 "%s: Warning: Invalid object type \"%i\".\n",
		filename, obj_ptr->type
	    );
	}

	/* No type specific data allocated? */
	if(obj_ptr->data == NULL)
	{
	    /* Ignore */
	}

	/* No standard visual model? */
	if(obj_ptr->visual_model == NULL)
	{
	    warnings++;
	    fprintf(
		stderr,
 "%s: Warning: Visual model \"standard\" not defined.\n",
		filename
	    );
	}

	/* Check object type specific values */
	switch(obj_ptr->type)
	{
	  case SAR_OBJ_TYPE_GARBAGE:
	  case SAR_OBJ_TYPE_STATIC:
	  case SAR_OBJ_TYPE_AUTOMOBILE:
	  case SAR_OBJ_TYPE_WATERCRAFT:
	    break;
	  case SAR_OBJ_TYPE_AIRCRAFT:
#define PTR	aircraft
	    PTR = SAR_OBJ_GET_AIRCRAFT(obj_ptr);
	    /* No flight dynamics model? */
	    if(PTR->fdm == NULL)
	    {
		errors++;
		fprintf(
		    stderr,
 "%s: Error: Flight dynamics model was not created.\n",
		    filename
		);
	    }
	    /* No cockpit visual model? */
	    if(PTR->visual_model_cockpit == NULL)
	    {
		warnings++;
		fprintf(
		    stderr,
 "%s: Warning: Visual model \"cockpit\" not defined.\n",
		    filename
		);
	    }
#undef PTR
	    break;

	  case SAR_OBJ_TYPE_GROUND:
#define PTR	ground
	    PTR = SAR_OBJ_GET_GROUND(obj_ptr);

#undef PTR
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


	return(errors);
}

/*
 *	Loads a visual model from the given X3D data.
 */
sar_visual_model_struct *SARObjLoadX3DDataVisualModel(
	sar_core_struct *core_ptr, sar_scene_struct *scene,
	char **x3d_data,
	const sar_scale_struct *scale
)
{
	sar_visual_model_struct *vmodel;
	X3DInterpValues v;

	if(x3d_data == NULL)
	    return(NULL);

	/* Set up X3D data values */
	v.flags =	X3D_VALUE_FLAG_COORDINATE_SYSTEM |
			((scale != NULL) ? X3D_VALUE_FLAG_SCALE : 0) |
			X3D_VALUE_FLAG_SKIP_TRANSLATES |
			X3D_VALUE_FLAG_SKIP_ROTATES;
	v.coordinate_system = X3D_COORDINATE_SYSTEM_XYZ;
	if(scale != NULL)
	{
	    v.scale_x = scale->x;
	    v.scale_y = scale->y;
	    v.scale_z = scale->z;
	}

	/* Create a new SAR visual model */
	vmodel = SARVisualModelNew(
	    scene,
	    NULL, NULL	/* No file or model name (so not shared) */
	);
	/* New visual model must not be shared */
	if(SARVisualModelGetRefCount(vmodel) == 1)
	{
	    GLuint list = (GLuint)SARVisualModelNewList(vmodel);
	    if(list != 0)
	    {
		vmodel->load_state = SAR_VISUAL_MODEL_LOADING;
		glNewList(list, GL_COMPILE);
		X3DOpenDataGLOutput(
		    x3d_data, &v, NULL, 0
		);
		glEndList();
		vmodel->load_state = SAR_VISUAL_MODEL_LOADED;
	    }
	}

	return(vmodel);
}

/*
 *	Loads a SAR visual model displaying the string s in 3d.
 *
 *	Origin is in lower left corner of 3d string.
 */
sar_visual_model_struct *SARObjLoadTextVisualModel(
	sar_core_struct *core_ptr, sar_scene_struct *scene,
	float font_width, float font_height,	/* In meters */
	float character_spacing,		/* In meters */
	const char *s,
	float *string_width			/* In meters */
)
{
	sar_visual_model_struct *vmodel;

	if(string_width != NULL)
	    *string_width = 0.0f;

	if((scene == NULL) || STRISEMPTY(s))
	    return(NULL);

	/* Create a new SAR visual model */
	vmodel = SARVisualModelNew(
	    scene,
	    NULL, NULL	/* No filename or model name, so never shared */
	);
	/* Really a new (not shared) visual model? */
	if(SARVisualModelGetRefCount(vmodel) == 1)
	{
	    GLuint list = (GLuint)SARVisualModelNewList(vmodel);
	    if(list != 0)
	    {
		vmodel->load_state = SAR_VISUAL_MODEL_LOADING;
		glNewList(list, GL_COMPILE);
		Text3DStringOutput(
		    font_width, font_height,
		    character_spacing,
		    s,
		    string_width
		);
		glEndList();
		vmodel->load_state = SAR_VISUAL_MODEL_LOADED;
	    }
	}

	return(vmodel);
}

/*
 *	Handles a translate parameter with respect to the given inputs.
 *
 *	Returns non-zero on error.
 */
int SARObjLoadTranslate(
	sar_core_struct *core_ptr, sar_scene_struct *scene,
	sar_object_struct *obj_ptr, sar_parm_translate_struct *p_translate
)
{
	sar_position_struct new_pos;

	if((obj_ptr == NULL) || (p_translate == NULL))
	    return(-1);

	memcpy(&new_pos, &p_translate->translate, sizeof(sar_position_struct));

	/* Check certain objects that need their position offsetted
	 * internally
	 */
	if(obj_ptr->type == SAR_OBJ_TYPE_HUMAN)
	{
	    /* Human objects fall at a constant rate, so we need to move
	     * it up twice the fall rate cycle to ensure that it
	     * `stays up' any hollow contact surfaces
	     */
	    new_pos.z -= (float)(2.0 * SAR_DEF_HUMAN_FALL_RATE);
	}


	/* Update the ground elevation (for more information on usage,
	 * see SARSimApplyArtificialForce() on how it uses 
	 * SARSimFindGround()
	 */

	/* Check objects of types that need to know about ground
	 * elevation
	 */
	if((obj_ptr->type == SAR_OBJ_TYPE_AIRCRAFT) ||
	   (obj_ptr->type == SAR_OBJ_TYPE_HUMAN) ||
           (obj_ptr->type == SAR_OBJ_TYPE_FIRE) ||
	   (obj_ptr->type == SAR_OBJ_TYPE_FUELTANK)
	)
	{
	    float ground_elevation = 0.0f;

	    ground_elevation += SARSimFindGround(
		scene,
		core_ptr->object, core_ptr->total_objects,
		&new_pos	/* Position of our object */
	    );
           
	    obj_ptr->ground_elevation_msl = ground_elevation;
	}

	/* Realize the new position */
	SARSimWarpObject(
	    scene, obj_ptr,
	    &new_pos, NULL
	);

	return(0);
}

/*
 *	Translates the object to a random position.
 *
 *	Returns non-zero on error.
 */
int SARObjLoadTranslateRandom(
	sar_core_struct *core_ptr, sar_scene_struct *scene,  
	sar_object_struct *obj_ptr,
	sar_parm_translate_random_struct *p_translate_random
)
{
	float rand_coeff = SARRandomCoeff(0);
	float r = p_translate_random->radius_bound;
	float z = p_translate_random->z_bound;
	sar_position_struct *pos;

	/* Calculate a random direction based on the random
	 * coefficient
	 */
	float rand_theta = (float)SFMSanitizeRadians(rand_coeff * (2.0 * PI));

	if((obj_ptr == NULL) || (p_translate_random == NULL))
	    return(-1);

	pos = &obj_ptr->pos;

	pos->x += (float)(sin(rand_theta) * rand_coeff * r);
	pos->y += (float)(cos(rand_theta) * rand_coeff * r);
	pos->z += (float)(rand_coeff * z);

	/* Realize position */
	SARSimWarpObject(scene, obj_ptr, pos, NULL);

	return(0);
}

/*
 *	Loads a sound source.
 *
 *	The string line should point to the argument portion.
 */
static int SARObjLoadSoundSource(
	sar_scene_struct *scene,
	sar_sound_source_struct ***list, int *total,
	const char *line, const char *filename, int line_num
)
{
	int i, sample_rate_limit;
	const char *arg;
	float range, range_far, cutoff;
	char	*s,
		*name = NULL,
		*sndobj_filename = NULL,
		*sndobj_filename_far = NULL;
	sar_position_struct pos;
	sar_direction_struct dir;
	sar_sound_source_struct *sndobj;

#define FREE_ALL	{	\
 free(name);			\
 free(sndobj_filename);		\
 free(sndobj_filename_far);	\
}

	if(STRISEMPTY(line))
	{
	    FREE_ALL
	    return(-1);
	}

	/* Format:
	 *
	 * <name>
	 * <range> <range_far>
	 * <x> <y> <z>
	 * <cutoff>
	 * <heading> <pitch> <bank>
	 * <sample_rate_limit>
	 * <sndobj_filename> <sndobj_filename_far>
	 */

	/* Begin parsing argument */
	arg = line;
	while(ISBLANK(*arg))
	    arg++;

	/* Name */
	arg = GET_ARG_S(arg, &name);

	/* Range */
	arg = GET_ARG_F(arg, &range);

	/* Range Far */
	arg = GET_ARG_F(arg, &range_far);

	/* Position */
	arg = GET_ARG_POS(arg, &pos);


	/* Cutoff */
	arg = GET_ARG_F(arg, &cutoff);
	cutoff = (float)DEGTORAD(cutoff);

	/* Direction */
	arg = GET_ARG_DIR(arg, &dir);

	/* Sample Rate Limit */
	arg = GET_ARG_I(arg, &sample_rate_limit);

	/* SndObj Filename */
	arg = GET_ARG_S(arg, &sndobj_filename);

	/* SndObj Filename Far */
	arg = GET_ARG_S(arg, &sndobj_filename_far);


	/* Warn if sndobj_filename was not given */
	s = sndobj_filename;
	if(STRISEMPTY(s))
	    fprintf(stderr,
 "%s: Line %i: Warning:\
 Value for sound_source_new argument sndobj_filename was not given.\n",
		filename, line_num
	    );
	/* Do not warn for sndobj_filename_far */


	/* Complete file names */
	s = COMPLETE_PATH(sndobj_filename);
	free(sndobj_filename);
	sndobj_filename = s;

	s = COMPLETE_PATH(sndobj_filename_far);
	free(sndobj_filename_far);
	sndobj_filename_far = s;


	/* Check if the sound files exist */
	s = sndobj_filename;
	if(!STRISEMPTY(s))
	{
	    struct stat stat_buf;
	    if(stat(s, &stat_buf))
	    {
		char *s2 = STRDUP(strerror(errno));
		if(s2 == NULL)
		    s2 = STRDUP("no such file");
		*s2 = toupper(*s2);
		fprintf(
		    stderr,
 "%s: Line %i: Warning:\
 Sound source \"%s\" sndobj_filename \"%s\": %s.\n",
		    filename, line_num,
		    name, s, s2
		);
		free(s2);
	    }
	}
	s = sndobj_filename_far;
	if(!STRISEMPTY(s))
	{
	    struct stat stat_buf;
	    if(stat(s, &stat_buf))
	    {
		char *s2 = STRDUP(strerror(errno));
		if(s2 == NULL)
		    s2 = STRDUP("no such file");
		*s2 = toupper(*s2);
		fprintf(
		    stderr,
 "%s: Line %i: Warning:\
 Sound source \"%s\" sndobj_filename_far \"%s\": %s.\n",
		    filename, line_num,
		    name, s, s2
		);
		free(s2);
	    }
	}


	/* Create new sound source */
	i = MAX(*total, 0);
	*total = i + 1;
	*list = (sar_sound_source_struct **)realloc(
	    *list,
	    (*total) * sizeof(sar_sound_source_struct *)
	);
	if(*list == NULL)
	{
	    *total = 0;
	    FREE_ALL
	    return(-1);
	}

        
        float new_sample_rate_limit = (float)(sample_rate_limit) / 11025.0f;
        //11025 is our wav files Hz. So this should be 1 most times.

	(*list)[i] = sndobj = SARSoundSourceNew(
	    name,
	    sndobj_filename,
	    sndobj_filename_far,
	    range,
	    range_far,
	    &pos, cutoff, &dir,
	    new_sample_rate_limit
	);

	FREE_ALL
	return(0);
#undef FREE_ALL
}


/*
 *	Loads a new texture specified by the values in p_texture_load
 *	and stores it on the scene structure.
 *
 *	If the scene structure already has a texture loaded that matches
 *	the texture reference name specified in p_texture_load then 
 *	nothing will be done and 0 is returned.
 *
 *	If the texture reference name specified in p_texture_load is
 *	NULL or an empty string then a new texture will be loaded
 *	implicitly.
 *
 *	Returns non-zero on error.
 */
int SARObjLoadTexture(
	sar_core_struct *core_ptr, sar_scene_struct *scene,
	sar_parm_texture_load_struct *p_texture_load
)
{
	char *name, *path, *full_path;
	v3d_texture_ref_struct *t;

	if((scene == NULL) || (p_texture_load == NULL))
	    return(-1);

	if(STRISEMPTY(p_texture_load->file))
	    return(-1);

	name = STRDUP(p_texture_load->name);
	path = STRDUP(p_texture_load->file);

	/* Check if a texture with he same name is already loaded */
	if(STRISEMPTY(name))
	    t = NULL;
	else
	    t = SARGetTextureRefByName(scene, name);
	/* Is there a texture with the same name already loaded? */
	if(t != NULL)
	    return(0);

	/* Get the full path to the texture file */
	full_path = COMPLETE_PATH(path);

	/* Load texture */
	t = V3DTextureLoadFromFile2DPreempt(
	    full_path, name, V3D_TEX_FORMAT_RGBA
	);
	if(t != NULL)
	{
	    /* Add this texture to the scene's list of textures */
	    int i = MAX(scene->total_texture_refs, 0);
	    scene->total_texture_refs = i + 1;

	    scene->texture_ref = (v3d_texture_ref_struct **)realloc(
		scene->texture_ref,
		scene->total_texture_refs * sizeof(v3d_texture_ref_struct *)
	    );
	    if(scene->texture_ref == NULL)
	    {
		V3DTextureDestroy(t);
		free(full_path);
		free(path);
		free(name);
		return(-3);
	    }
	    scene->texture_ref[i] = t;

	    /* Set texture priority */
	    V3DTexturePriority(t, p_texture_load->priority);

	    free(full_path);
	    free(path);
	    free(name);
	    return(0);
	}
	else
	{
	    /* Could not load texture */
	    free(full_path);
	    free(path);
	    free(name);
	    return(-1);
	}
}

/*
 *      Loads a helipad object from the given argument line string.
 *
 *      Returns the newly created human object number or -1 on error.
 */
int SARObjLoadHelipad(
	sar_core_struct *core_ptr, sar_scene_struct *scene,
	sar_parm_new_helipad_struct *p_new_helipad
)
{
	const char *s;
	int obj_num = -1;
	sar_object_struct *obj_ptr;
	sar_object_helipad_struct *helipad;
	const char *ref_obj_name;
	const char *tex_name = SAR_STD_TEXNAME_HELIPAD_PAVED;
	int ref_obj_num = -1;
	sar_object_struct *ref_obj_ptr = NULL;


	if((scene == NULL) || (p_new_helipad == NULL))
	    return(obj_num);

	/* Create a new Helipad */
	obj_num = SARObjNew(
	    scene, &core_ptr->object, &core_ptr->total_objects,
	    SAR_OBJ_TYPE_HELIPAD
	);
	obj_ptr = (obj_num > -1) ? core_ptr->object[obj_num] : NULL;
	if(obj_ptr == NULL)
	{
	    obj_num = -1;
	    return(obj_num);
	}
	helipad = SAR_OBJ_GET_HELIPAD(obj_ptr);
	if(helipad == NULL)
	    return(obj_num);

	/* Begin setting values */

	/* Helipad flags */
	helipad->flags = p_new_helipad->flags;

	/* Range */
	obj_ptr->range = SAR_HELIPAD_DEF_RANGE;

	/* Style */
	s = p_new_helipad->style;
	if(!STRISEMPTY(s))
	{
	    if(!strcasecmp(s, "ground_paved") ||
	       !strcasecmp(s, "standard") ||
	       !strcasecmp(s, "default")
	    )
	    {
		helipad->style = SAR_HELIPAD_STYLE_GROUND_PAVED;
		tex_name = SAR_STD_TEXNAME_HELIPAD_PAVED;
	    }
	    else if(!strcasecmp(s, "ground_bare"))
	    {
		helipad->style = SAR_HELIPAD_STYLE_GROUND_BARE;
		tex_name = SAR_STD_TEXNAME_HELIPAD_BARE;
	    }
	    else if(!strcasecmp(s, "building"))
	    {
		helipad->style = SAR_HELIPAD_STYLE_BUILDING;
		tex_name = SAR_STD_TEXNAME_HELIPAD_BUILDING;
	    }
	    else if(!strcasecmp(s, "vehicle"))
	    {
		helipad->style = SAR_HELIPAD_STYLE_VEHICLE;
		tex_name = SAR_STD_TEXNAME_HELIPAD_VEHICLE;
	    }
	    else
	    {
		fprintf(
		    stderr,
"Object #%i: Warning:\
 Unsupported helipad style \"%s\".\n",
		    obj_num, s
		);
		helipad->style = SAR_HELIPAD_STYLE_GROUND_PAVED;
		tex_name = SAR_STD_TEXNAME_HELIPAD_PAVED;
	    }
	}
	else
	{
	    /* Set default style */
	    helipad->style = SAR_HELIPAD_STYLE_GROUND_PAVED;
	    tex_name = SAR_STD_TEXNAME_HELIPAD_PAVED;
	}

	/* Size */
	helipad->length = (float)MAX(p_new_helipad->length, 1.0);
	helipad->width = (float)MAX(p_new_helipad->width, 1.0);
	helipad->recession = (float)MAX(p_new_helipad->recession, 0.0);

	/* Specify contact bounds so that the helipad is landable */
	SARObjAddContactBoundsRectangular(
	    obj_ptr, SAR_CRASH_FLAG_SUPPORT_SURFACE, 0,
	    -(helipad->width / 2),
	    (helipad->width / 2),
	    -(helipad->length / 2),
	    (helipad->length / 2),
	    0.0f, 0.0f
	);

	/* Label */
	s = p_new_helipad->label;
	helipad->label = STRDUP(s);
	if(!STRISEMPTY(s))
	{
	    /* Generate visual model for label using 3d text */
	    int len = MAX(STRLEN(s), 1);
	    float font_height = (float)(
		MIN(helipad->width, helipad->length) *
		0.7 / len
	    );
	    float font_width = (float)(font_height * 0.8);
	    helipad->label_vmodel = SARObjLoadTextVisualModel(
		core_ptr, scene,
		font_width, font_height,
		(float)(font_width * 0.05),
		s,
		&helipad->label_width
	    );
	}
	else
	{
	    /* No label */
	    helipad->label_vmodel = NULL;
	    helipad->label_width = 0;
	}

	/* Light spacing */
	helipad->light_spacing = helipad->length / 10;

	/* Texture reference number from scene */
	helipad->tex_num = SARGetTextureRefNumberByName(
	    scene, tex_name
	);

	/* Match reference object? */
	ref_obj_name = p_new_helipad->ref_obj_name;
	if((ref_obj_name != NULL) ? (*ref_obj_name != '\0') : False)
	{
	    ref_obj_ptr = SARObjMatchPointerByName(
		scene, core_ptr->object, core_ptr->total_objects,
		ref_obj_name, &ref_obj_num
	    );
	    if(ref_obj_ptr != NULL)
	    {

	    }
	    else
	    {
		fprintf(
		    stderr,
"Object #%i: Warning:\
 Unable to match reference object \"%s\" for helipad.\n",
		    obj_num, ref_obj_name
		);
	    }
	}
	/* Set reference object */
	helipad->ref_object = ref_obj_num;
	/* Make sure reference object is not this object */
	if(ref_obj_ptr == obj_ptr)
	{
	    ref_obj_num = -1;
	    ref_obj_ptr = NULL;

	    helipad->flags &= ~SAR_HELIPAD_FLAG_REF_OBJECT;
	    helipad->flags &= ~SAR_HELIPAD_FLAG_FOLLOW_REF_OBJECT;

	    helipad->ref_object = -1;
	}

	/* Set reference offset relative to the reference object */
	memcpy(
	    &helipad->ref_offset,
	    &p_new_helipad->ref_offset,
	    sizeof(sar_position_struct)
	);

	/* Set reference direction relative to the reference object */
	memcpy(
	    &helipad->ref_dir,
	    &p_new_helipad->ref_dir, 
	    sizeof(sar_direction_struct)
	);

	/* Realize new position if there is a reference object */
	if((helipad->flags & SAR_HELIPAD_FLAG_REF_OBJECT) &&
	   (helipad->flags & SAR_HELIPAD_FLAG_FOLLOW_REF_OBJECT)
	)
	    SARSimWarpObjectRelative(
		scene, obj_ptr,
		core_ptr->object, core_ptr->total_objects, ref_obj_num,
		&helipad->ref_offset,
		&helipad->ref_dir
	    );

	return(obj_num);
}

/*
 *	Loads a runway object from the given argument line string.
 *
 *      Returns the newly created human object number or -1 on error.
 */
int SARObjLoadRunway(
	sar_core_struct *core_ptr, sar_scene_struct *scene,
	sar_parm_new_runway_struct *p_new_runway
)
{
	const char *s;
	int obj_num = -1;
	sar_object_struct *obj_ptr;
	sar_object_runway_struct *runway;


	if((scene == NULL) || (p_new_runway == NULL))
	    return(obj_num);

	/* Create a new runway object */
	obj_num = SARObjNew(
	    scene, &core_ptr->object, &core_ptr->total_objects,
	    SAR_OBJ_TYPE_RUNWAY
	);
	obj_ptr = (obj_num > -1) ? core_ptr->object[obj_num] : NULL;
	if(obj_ptr == NULL)
	{
	    obj_num = -1;
	    return(obj_num);
	}
	runway = SAR_OBJ_GET_RUNWAY(obj_ptr);
	if(runway == NULL)
	    return(obj_num);

	/* Begin setting values */

	/* Flags */
	runway->flags |= p_new_runway->flags;

	/* Range */
	obj_ptr->range = p_new_runway->range;

	/* Size */
	runway->length = p_new_runway->length;
	runway->width = p_new_runway->width;

	/* Surface type */
	runway->surface_type = p_new_runway->surface_type;

	/* Set contact bounds to ensure that the object is landable */
	SARObjAddContactBoundsRectangular(
	    obj_ptr,
	    SAR_CRASH_FLAG_SUPPORT_SURFACE, 0,
	    -(runway->width / 2),
	    (runway->width / 2),
	    -(runway->length / 2),
	    (runway->length / 2),
	    0.0f, 0.0f
	);

	/* Label */
	runway->north_label = STRDUP(p_new_runway->north_label);
	runway->south_label = STRDUP(p_new_runway->south_label);

	/* Create visual models for labels using 3d text */
	s = p_new_runway->north_label;
	if(!STRISEMPTY(s))
	{
	    int len = MAX(STRLEN(s), 1);
	    float font_height = (float)(runway->width * 0.8 / len);
 	    float font_width = (float)(font_height * 0.8);

	    runway->north_label_vmodel = SARObjLoadTextVisualModel(
		core_ptr, scene,
		font_width, font_height,
		(float)(font_width * 0.15),
		s,
		&runway->north_label_width
	    );
	}
	else
	{
	    runway->north_label_vmodel = NULL;
	    runway->north_label_width = 0;
	}
	s = p_new_runway->south_label;
	if(!STRISEMPTY(s))
	{
	    int len = STRLEN(s);
	    float font_height = (float)(runway->width * 0.8 / len);
	    float font_width = (float)(font_height * 0.8);

	    runway->south_label_vmodel = SARObjLoadTextVisualModel(
		core_ptr, scene,
		font_width, font_height,
		(float)(font_width * 0.15),
		s,
		&runway->south_label_width
	    );
	}
	else
	{
	    runway->south_label_vmodel = NULL;
	    runway->south_label_width = 0;
	}

	/* Dashes */
	runway->dashes = (p_new_runway->dashes > 0) ?
	    p_new_runway->dashes : 10;

	/* Displaced thresholds */
	runway->north_displaced_threshold =
	    p_new_runway->north_displaced_threshold;
	runway->south_displaced_threshold =
	    p_new_runway->south_displaced_threshold;

	/* Edge lighting */
	runway->edge_light_spacing = (float)(
	    (p_new_runway->edge_light_spacing > 0.0f) ?
		p_new_runway->edge_light_spacing :
		(runway->length / 50)
	);

	/* Approach lighting */
	runway->north_approach_lighting_flags = 0;
	runway->south_approach_lighting_flags = 0;

	runway->tracer_anim_pos = 0;
	runway->tracer_anim_rate = (sar_grad_anim_t)(
	    ((sar_grad_anim_t)-1) * 0.5
	);

	/* Get texture number from scene */
	runway->tex_num = SARGetTextureRefNumberByName(
	    scene, SAR_STD_TEXNAME_RUNWAY
	);

	/* Begin loading visual models for runway decorations */
	/* Threshold */
	if(runway->flags & SAR_RUNWAY_FLAG_THRESHOLDS)
	{
	    sar_scale_struct scale;
	    scale.x = runway->width;
	    scale.y = 50.0f;
	    scale.z = 1.0f;
	    runway->threshold_vmodel = SARObjLoadX3DDataVisualModel(
		core_ptr, scene,
		runway_threshold_x3d,
		&scale
	    );
	}
	/* Touchdown markers */
	if(runway->flags & SAR_RUNWAY_FLAG_TD_MARKERS)
	{
	    sar_scale_struct scale;
	    scale.x = runway->width;
	    scale.y = 30.0f;
	    scale.z = 1.0f;
	    runway->td_marker_vmodel = SARObjLoadX3DDataVisualModel(
		core_ptr, scene,
		runway_td_markers_x3d,
		&scale
	    );
	}
	/* Midway markers */
	if(runway->flags & SAR_RUNWAY_FLAG_MIDWAY_MARKERS)
	{
	    sar_scale_struct scale;
	    scale.x = runway->width;
	    scale.y = (float)(runway->length * 0.05);
	    scale.z = 1.0f;
	    runway->midway_marker_vmodel = SARObjLoadX3DDataVisualModel(
		core_ptr, scene,
		runway_midway_markers_x3d,
		&scale
	    );
	}
	/* North displaced threshold */
	if(runway->north_displaced_threshold > 0.0f)
	{
	    sar_scale_struct scale;
	    scale.x = runway->width;
	    scale.y = runway->north_displaced_threshold;
	    scale.z = 1.0f;
	    runway->north_displaced_threshold_vmodel =
		SARObjLoadX3DDataVisualModel(
		    core_ptr, scene,
		    runway_displaced_threshold_x3d,
		    &scale
		);
	}
	/* South displaced threshold */
	if(runway->south_displaced_threshold > 0.0f)
	{
	    sar_scale_struct scale;
	    scale.x = runway->width;
	    scale.y = runway->south_displaced_threshold;
	    scale.z = 1.0f;
	    runway->south_displaced_threshold_vmodel =
		SARObjLoadX3DDataVisualModel(
		    core_ptr, scene,
		    runway_displaced_threshold_x3d,
		    &scale
		);
	}

	return(obj_num);
}

/*
 *	Loads a human object from the given argument line string.
 *
 *	Returns the newly created human object number or -1 on error.
 */
int SARObjLoadHuman(
	sar_core_struct *core_ptr, sar_scene_struct *scene,
	sar_parm_new_human_struct *p_new_human
)
{
	int obj_num = -1;

	if((scene == NULL) || (p_new_human == NULL))
	    return(obj_num);

	/* Create new human object */
	obj_num = SARHumanCreate(
	    core_ptr->human_data,
	    scene, &core_ptr->object, &core_ptr->total_objects,
	    p_new_human->flags,
	    p_new_human->type_name
	);

	return(obj_num);
}

/*
 *      Loads a fire object from the given argument line string.
 *
 *      Returns the newly created fire object number or -1 on error.
 */
int SARObjLoadFire( 
	sar_core_struct *core_ptr, sar_scene_struct *scene,
	sar_parm_new_fire_struct *p_new_fire 
)
{
	int obj_num = -1;
	sar_object_struct *obj_ptr;
	sar_object_fire_struct *fire;
	sar_position_struct pos;


	if((scene == NULL) || (p_new_fire == NULL))
	    return(obj_num);

	memset(&pos, 0x00, sizeof(sar_position_struct));

	/* Create new fire object */
	obj_num = FireCreate(
	    core_ptr, scene,
	    &core_ptr->object, &core_ptr->total_objects,
	    &pos, p_new_fire->radius, p_new_fire->height,
	    -1,
	    SAR_STD_TEXNAME_FIRE, SAR_STD_TEXNAME_FIRE_IR
	);
	obj_ptr = (obj_num > -1) ? core_ptr->object[obj_num] : NULL;
	if(obj_ptr == NULL)
	{
	    obj_num = -1;
	    return(obj_num);
	}
	fire = SAR_OBJ_GET_FIRE(obj_ptr);
	if(fire == NULL)
	    return(obj_num);

	/* Begin setting values */

	/* Animation repeats */
	fire->total_frame_repeats = -1;	/* Infinate */

	obj_ptr->life_span = 0;

	return(obj_num);
}

/*
 *      Loads a smoke object from the given argument line string.
 *
 *      Returns the newly created fire object number or -1 on error.
 */
int SARObjLoadSmoke(
	sar_core_struct *core_ptr, sar_scene_struct *scene,
	sar_parm_new_smoke_struct *p_new_smoke
)
{
	int obj_num = -1;
	sar_object_struct *obj_ptr;
	sar_position_struct pos;
	const char *tex_name = NULL;


	if((scene == NULL) || (p_new_smoke == NULL))
	    return(obj_num);

	memset(&pos, 0x00, sizeof(sar_position_struct));

	/* Get name of smoke texture to use from the color code */
	switch(p_new_smoke->color_code)
	{
	  case 0:	/* Light */
	    tex_name = SAR_STD_TEXNAME_SMOKE_LIGHT;
	    break;
	  case 1:	/* Medium */
	    tex_name = SAR_STD_TEXNAME_SMOKE_MEDIUM;
	    break;
	  case 2:	/* Dark */
	    tex_name = SAR_STD_TEXNAME_SMOKE_DARK;
	    break;
	}

	/* Create new smoke trail object */
	obj_num = SmokeCreate(
	    scene, &core_ptr->object, &core_ptr->total_objects,
	    SAR_SMOKE_TYPE_SMOKE,	/* Smoke type */
	    &pos, &p_new_smoke->offset,	/* Position and unit spawn offset */
	    p_new_smoke->radius_start,
	    p_new_smoke->radius_max,
	    p_new_smoke->radius_rate,	/* May be -1.0 for autocalc */
	    p_new_smoke->hide_at_max,
	    p_new_smoke->total_units,
	    p_new_smoke->respawn_int,
	    tex_name,
	    -1,				/* No reference object */
	    0				/* No limit on life span */
	);
	obj_ptr = (obj_num > -1) ? core_ptr->object[obj_num] : NULL;
	if(obj_ptr == NULL)
	{
	    obj_num = -1;
	    return(obj_num);
	}

	/* Begin setting values */



	return(obj_num);
}

/*
 *	Loads heightfield primitive p for the specified object.
 *
 *	If the GL display list is 0 then no GL commands will be issued
 *	and only the z height points will be recorded on the object.
 */
int SARObjLoadHeightField(
	sar_core_struct *core_ptr,
	int obj_num, sar_object_struct *obj_ptr,
	sar_visual_model_struct *vmodel,
	void *p, const char *filename, int line_num,
	GLuint list			/* Can be 0 */
)
{
	int status, grid_points_total;
	float x_length, y_length, z_length;
	int num_grids_x, num_grids_y;
	double grid_space_x, grid_space_y;
	double *zpoints = NULL;

	char *full_path;
	sar_object_ground_struct *ground = NULL;
	mp_heightfield_load_struct *mp_heightfield_load =
	    (mp_heightfield_load_struct *)p;
	v3d_hf_options_struct hfopt;

	if(STRISEMPTY(mp_heightfield_load->path))
	    return(-1);

	ground = SAR_OBJ_GET_GROUND(obj_ptr);

	/* Get the full path to the heightfield file */
	full_path = COMPLETE_PATH(mp_heightfield_load->path);

	/* Begin loading the heightfield */

	/* Get size of heightfield from heightfield load primitive */
	x_length = (float)mp_heightfield_load->x_length;
	y_length = (float)mp_heightfield_load->y_length;
	z_length = (float)mp_heightfield_load->z_length;
	if((x_length <= 0.0f) ||
	   (y_length <= 0.0f) ||
	   (z_length <= 0.0f)
	)
	{
	    free(full_path);
	    return(-2);
	}

	glPushMatrix();
	{
	    /* Translate */
	    glTranslated(
		mp_heightfield_load->x,
		mp_heightfield_load->z,
		-mp_heightfield_load->y
	    );

	    /* No rotations */

	    /* Set up heightfield options */
	    hfopt.flags = (V3D_HF_OPT_FLAG_WINDING |
		V3D_HF_OPT_FLAG_SET_NORMAL | V3D_HF_OPT_FLAG_SET_TEXCOORD
	    );
	    hfopt.winding = V3D_HF_WIND_CCW;
	    hfopt.set_normal = V3D_HF_SET_NORMAL_STREATCHED;
	    hfopt.set_texcoord = V3D_HF_SET_TEXCOORD_ALWAYS;

	    /* Load heightfield */
	    status = V3DHFLoadFromFile(
		full_path,
		x_length, y_length, z_length,	/* Scaling in meters */
		&num_grids_x, &num_grids_y,	/* Number of grids */
		&grid_space_x, &grid_space_y,	/* Grid spacing in meters */
		&zpoints,			/* Heightfield points return */
		(void *)list,			/* GL display list */
		&hfopt
	    );
	    if(status)
	    {
		/* Error loading heightfield */
		free(zpoints);
		free(full_path);
		return(-1);
	    }
	}
	glPopMatrix();

	/* Calculate total number of points */
	grid_points_total = num_grids_x * num_grids_y;

	/* Calculate statistics */
	if((vmodel != NULL) && (list != 0))
	{
	    vmodel->mem_size += grid_points_total * (
		(2 * 3 * 8 * sizeof(GLfloat)) +
		(3 * sizeof(GLuint))
	    );
	    vmodel->statements += (grid_points_total * 8) + 2;
	    vmodel->primitives += grid_points_total * 2;
	}

	/* Set newly loaded values to object */
	if(ground != NULL)
	{
	    ground->x_trans = (float)mp_heightfield_load->x;
	    ground->y_trans = (float)mp_heightfield_load->y;
	    ground->z_trans = (float)mp_heightfield_load->z;

	    ground->x_len = (float)x_length;
	    ground->y_len = (float)y_length;

	    ground->grid_points_x = num_grids_x;
	    ground->grid_points_y = num_grids_y;
	    ground->grid_points_total = grid_points_total;

	    ground->grid_x_spacing = (float)grid_space_x;
	    ground->grid_y_spacing = (float)grid_space_y;
	    ground->grid_z_spacing = (float)z_length;

	    /* Replace old heightfield z point data on ground object
	     * structure with the newly allocated one
	     */
	    free(ground->z_point_value);
	    ground->z_point_value = zpoints;
	    zpoints = NULL;	/* Reset zpoints to mark it as transfered */
	}

	free(zpoints);
	free(full_path);

	return(0);
}

/*
 *	Called by SARObjLoadProcessVisualModel().
 *
 *	Parses and handles the specified V3D primitive p with the
 *	specified values.
 */
static void SARObjLoadProcessVisualPrimitive(
	sar_core_struct *core_ptr,
	int obj_num, sar_object_struct *obj_ptr,
	sar_visual_model_struct *vmodel,
	void *p, const char *filename, int line_num
)
{
	int i, ptype, v_total = 0;
	Boolean need_end_primitive = False;
	mp_vertex_struct *ns = NULL, **nd = NULL;
	mp_vertex_struct *vs = NULL, **vd = NULL;
	mp_vertex_struct *tcs = NULL, **tcd = NULL;

	mp_point_struct *mp_point;
	mp_line_struct *mp_line;
	mp_line_strip_struct *mp_line_strip;
	mp_line_loop_struct *mp_line_loop;
	mp_triangle_struct *mp_triangle;
	mp_triangle_strip_struct *mp_triangle_strip;
	mp_triangle_fan_struct *mp_triangle_fan;
	mp_quad_struct *mp_quad;
	mp_quad_strip_struct *mp_quad_strip;
	mp_polygon_struct *mp_polygon;
	mp_vertex_struct *first_normal_ptr = NULL, *n_ptr;

	float x = 0.0f, y = 0.0f, z = 0.0f;
	float tx = 0.0f, ty = 0.0f;


	/* Handle by primitive type */
	ptype = V3DMPGetType(p);
	switch(ptype)
	{
	  case V3DMP_TYPE_POINT:
	    mp_point = (mp_point_struct *)p;
	    if(last_begin_primitive_type != ptype)
	    {
		vmodel->statements++;
		vmodel->mem_size += 2 * sizeof(GLuint);
		glBegin(GL_POINTS);
		last_begin_primitive_type = ptype;
	    }
	    ns = &mp_point->n[0];
	    vs = &mp_point->v[0];
	    tcs = &mp_point->tc[0];
	    v_total = V3DMP_POINT_NVERTEX;
	    break;

	  case V3DMP_TYPE_LINE:
	    mp_line = (mp_line_struct *)p;
	    if(last_begin_primitive_type != ptype)
	    {
		vmodel->statements++;
		vmodel->mem_size += 2 * sizeof(GLuint);
		glBegin(GL_LINES);
		last_begin_primitive_type = ptype;
	    }
	    ns = &mp_line->n[0];
	    vs = &mp_line->v[0];
	    tcs = &mp_line->tc[0];
	    v_total = V3DMP_LINE_NVERTEX;
	    break;

	  case V3DMP_TYPE_LINE_STRIP:
	    mp_line_strip = (mp_line_strip_struct *)p;
	    vmodel->statements++;
	    vmodel->mem_size += 2 * sizeof(GLuint);
	    glBegin(GL_LINE_STRIP);
	    need_end_primitive = True;
	    nd = mp_line_strip->n;
	    vd = mp_line_strip->v;
	    tcd = mp_line_strip->tc;
	    v_total = mp_line_strip->total;
	    break;

	  case V3DMP_TYPE_LINE_LOOP:
	    mp_line_loop = (mp_line_loop_struct *)p;
	    vmodel->statements++;
	    vmodel->mem_size += 2 * sizeof(GLuint);
	    glBegin(GL_LINE_LOOP);
	    need_end_primitive = True;
	    nd = mp_line_loop->n;
	    vd = mp_line_loop->v;
	    tcd = mp_line_loop->tc;
	    v_total = mp_line_loop->total;
	    break;

	  case V3DMP_TYPE_TRIANGLE:
	    mp_triangle = (mp_triangle_struct *)p;
	    if(last_begin_primitive_type != ptype)
	    {
		vmodel->statements++;
		vmodel->mem_size += 2 * sizeof(GLuint);
		glBegin(GL_TRIANGLES);
		last_begin_primitive_type = ptype;
	    }
	    ns = &mp_triangle->n[0];
	    vs = &mp_triangle->v[0];
	    tcs = &mp_triangle->tc[0];
	    v_total = V3DMP_TRIANGLE_NVERTEX;
	    break;

	  case V3DMP_TYPE_TRIANGLE_STRIP:
	    mp_triangle_strip = (mp_triangle_strip_struct *)p;
	    vmodel->statements++;
	    vmodel->mem_size += 2 * sizeof(GLuint);
	    glBegin(GL_TRIANGLE_STRIP);
	    need_end_primitive = True;
	    nd = mp_triangle_strip->n;
	    vd = mp_triangle_strip->v;
	    tcd = mp_triangle_strip->tc;
	    v_total = mp_triangle_strip->total;
	    break;

	  case V3DMP_TYPE_TRIANGLE_FAN:
	    mp_triangle_fan = (mp_triangle_fan_struct *)p;
	    vmodel->statements++;
	    vmodel->mem_size += 2 * sizeof(GLuint);
	    glBegin(GL_TRIANGLE_FAN);
	    need_end_primitive = True;
	    nd = mp_triangle_fan->n;
	    vd = mp_triangle_fan->v;
	    tcd = mp_triangle_fan->tc;
	    v_total = mp_triangle_fan->total;
	    break;

	  case V3DMP_TYPE_QUAD:
	    mp_quad = (mp_quad_struct *)p;
	    if(last_begin_primitive_type != ptype)
	    {
		vmodel->statements++;
		vmodel->mem_size += 2 * sizeof(GLuint);
		glBegin(GL_QUADS);
		last_begin_primitive_type = ptype;
	    }
	    ns = &mp_quad->n[0];
	    vs = &mp_quad->v[0];
	    tcs = &mp_quad->tc[0];
	    v_total = V3DMP_QUAD_NVERTEX;
	    break;

	  case V3DMP_TYPE_QUAD_STRIP:
	    mp_quad_strip = (mp_quad_strip_struct *)p;
	    vmodel->statements++;
	    vmodel->mem_size += 2 * sizeof(GLuint);
	    glBegin(GL_QUAD_STRIP);
	    need_end_primitive = True;
	    nd = mp_quad_strip->n;
	    vd = mp_quad_strip->v;   
	    tcd = mp_quad_strip->tc;
	    v_total = mp_quad_strip->total;
	    break;

	  case V3DMP_TYPE_POLYGON:
	    mp_polygon = (mp_polygon_struct *)p;
	    vmodel->statements++;
	    vmodel->mem_size += 2 * sizeof(GLuint);
	    glBegin(GL_POLYGON);
	    need_end_primitive = True;
	    nd = mp_polygon->n;
	    vd = mp_polygon->v;
	    tcd = mp_polygon->tc;
	    v_total = mp_polygon->total;
	    break;
	}


	/* Get pointer to first normal */
	i = 0;
	if(nd != NULL)
	    first_normal_ptr = nd[i];
	else if(ns != NULL)
	    first_normal_ptr = &ns[i];
	else
	    first_normal_ptr = NULL;

	/* Iterate through each vertex start from last to first,
	 * becuase the winding stored on V3D model file is clockwise
	 * and we need to handle it counter-clockwise
	 */
	for(i = v_total - 1; i >= 0; i--)
	{
	    /* Get vertex values but do not set just yet, we only need
	     * the coordinates for now in order to set the texcoord
	     * first (in case the texture is being plane oriented)
	     */
	    if((vd != NULL) ? (vd[i] != NULL) : False)
	    {
		const mp_vertex_struct *v = vd[i];
		x = (float)v->x;
		y = (float)v->y;
		z = (float)v->z;
	    }
	    else if(vs != NULL)
	    {
		const mp_vertex_struct *v = &vs[i];
		x = (float)v->x;
		y = (float)v->y;
		z = (float)v->z;
	    }

	    /* Get normal and make sure it is not a zero vector, if it
	     * is then use the first normal (if any).
	     */
	    if(nd != NULL)
	    {
		n_ptr = nd[i];
		if((n_ptr->x == 0.0) && (n_ptr->y == 0.0) && (n_ptr->z == 0.0))
		    n_ptr = first_normal_ptr;
	    }
	    else if(ns != NULL)
	    {
		n_ptr = &ns[i];
		if((n_ptr->x == 0.0) && (n_ptr->y == 0.0) && (n_ptr->z == 0.0))
		    n_ptr = first_normal_ptr;
	    }
	    else
	    {
		n_ptr = first_normal_ptr;	/* All elese use first normal */
	    }

	    /* Got valid normal? */
	    if(n_ptr != NULL)
	    {
		vmodel->statements++;
		vmodel->mem_size += sizeof(GLuint) + (3 * sizeof(GLfloat));
		glNormal3d(n_ptr->x, n_ptr->z, -n_ptr->y);
	    }

	    /* Texture enabled? */
	    if(tex_on)
	    {
		/* Set texture coordinates */
		switch(tex_orient)
		{
		  case TEX_ORIENT_XY:
		    if(tex_coord.w > 0.0f)
			tx = (x - tex_coord.i) / tex_coord.w;
		    else
			tx = 0.0f;
		    if(tex_coord.h > 0.0f)
			ty = (y - tex_coord.j) / tex_coord.h;
		    else
			ty = 0.0f;
		    vmodel->statements++;
		    vmodel->mem_size += sizeof(GLuint) + (2 * sizeof(GLfloat));
		    glTexCoord2f(tx, 1.0f - ty);
		    break;

		  case TEX_ORIENT_YZ:
		    if(tex_coord.w > 0.0f)
			tx = -(y - tex_coord.i) / tex_coord.w;
		    else
			tx = 0.0f;
		    if(tex_coord.h > 0.0f)
			ty = (z - tex_coord.j) / tex_coord.h;
		    else
			ty = 0.0f;
		    vmodel->statements++;
		    vmodel->mem_size += sizeof(GLuint) + (2 * sizeof(GLfloat));
		    glTexCoord2f(tx, 1.0f - ty);
		    break;

		  case TEX_ORIENT_XZ:
		    if(tex_coord.w > 0.0f)
			tx = (x - tex_coord.i) / tex_coord.w;
		    else
			tx = 0.0f;
		    if(tex_coord.h > 0.0f)
			ty = (z - tex_coord.j) / tex_coord.h;
		    else
			ty = 0.0f;
		    vmodel->statements++;
		    vmodel->mem_size += sizeof(GLuint) + (2 * sizeof(GLfloat));
		    glTexCoord2f(tx, 1.0f - ty);
		    break;

		  default:
		    if(tcd != NULL)
		    {
			if(tcd[i] != NULL)
			{
			    tx = (float)tcd[i]->x;
			    ty = (float)tcd[i]->y;
			}
		    }
		    else if(tcs != NULL)
		    {
			tx = (float)tcs[i].x;
			ty = (float)tcs[i].y;
		    }

		    vmodel->statements++;
		    vmodel->mem_size += sizeof(GLuint) + (2 * sizeof(GLfloat));
		    glTexCoord2f(tx, 1.0f - ty);
		    break;
		}
	    }

	    /* Set vertex */
	    vmodel->statements++;
	    vmodel->mem_size += sizeof(GLuint) + (3 * sizeof(GLfloat));
	    glVertex3d(x, z, -y);
	}

	/* End primitive as needed */
	if(need_end_primitive)
	{
	    vmodel->statements++;
	    vmodel->mem_size += sizeof(GLuint);
	    glEnd();
	    need_end_primitive = False;
	}
} 

/*
 *	Called by SARObjLoadFromFile().
 *
 *	Processes the visual primitives in the V3D Visual Model into GL
 *	commands (suitable for GL list recording).
 *
 *	The vmodel's memory size statistics will be updated.
 */
static void SARObjLoadProcessVisualModel(
	sar_core_struct *core_ptr,
	int obj_num, sar_object_struct *obj_ptr,
	sar_visual_model_struct *vmodel,
	v3d_model_struct *v3d_model,
	Boolean process_as_ir,
	const char *filename, int line_num
)
{
	int pn, ptype;
	void *p;

	StateGLBoolean blend_state = False;

	GLuint list = (GLuint)vmodel->data;
	gw_display_struct *display = core_ptr->display;
	state_gl_struct *state_gl = &display->state_gl;
	sar_scene_struct *scene = core_ptr->scene;


	/* Reset GL states */
	V3DTextureSelect(NULL);
	tex_on = False;
	tex_orient = TEX_ORIENT_NONE;
/* Do not reset color, model file should set it, if not then it means
 * it relys on other code to set it
 *
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
 */

	/* Reset last begin primitive type */
	last_begin_primitive_type = -1;

	/* Set initial GL states for IR processing? */
	if(process_as_ir)
	{

	}

	/* Iterate through each V3D model primitive */
	for(pn = 0; pn < v3d_model->total_primitives; pn++)
	{
	    p = v3d_model->primitive[pn];
	    if(p == NULL)
		continue;

	    /* Get primitive type */
	    ptype = V3DMPGetType(p);

	    /* Check if last begin primitive differs from this one
	     *
	     * If it does then glEnd() needs to be called to end the
	     * first glBegin() that was not ended
	     */
	    if((last_begin_primitive_type != ptype) &&
	       (last_begin_primitive_type > -1)
	    )
	    {
		glEnd();
		vmodel->mem_size += sizeof(GLuint);
		vmodel->statements++;
		last_begin_primitive_type = -1;
	    }

	    switch(ptype)
	    {
	        const mp_comment_struct *mp_comment;
		const mp_color_struct *mp_color;
		const mp_texture_select_struct *mp_texture_select;
		const mp_texture_orient_xy_struct *mp_texture_xy;
		const mp_texture_orient_yz_struct *mp_texture_yz;
		const mp_texture_orient_xz_struct *mp_texture_xz;
		const mp_heightfield_load_struct *mp_heightfield_load;

	      case V3DMP_TYPE_COMMENT:
		mp_comment = (mp_comment_struct *)p;
		/* Ignore comment lines in V3D visual models */
		break;

	      case V3DMP_TYPE_POINT:
	      case V3DMP_TYPE_LINE:
	      case V3DMP_TYPE_LINE_STRIP:
	      case V3DMP_TYPE_LINE_LOOP:
	      case V3DMP_TYPE_TRIANGLE:
	      case V3DMP_TYPE_TRIANGLE_STRIP:
	      case V3DMP_TYPE_TRIANGLE_FAN:
	      case V3DMP_TYPE_QUAD:
	      case V3DMP_TYPE_QUAD_STRIP:
	      case V3DMP_TYPE_POLYGON:
		vmodel->primitives++;
		SARObjLoadProcessVisualPrimitive(
		    core_ptr, obj_num, obj_ptr, vmodel,
		    p, filename, 0
		);
		break;

	      case V3DMP_TYPE_COLOR:
		mp_color = (mp_color_struct *)p;
		if(process_as_ir)
		    break;
		vmodel->mem_size += 4 * sizeof(GLfloat);
		vmodel->statements++;
		glColor4f(
		    (GLfloat)mp_color->r,
		    (GLfloat)mp_color->g,
		    (GLfloat)mp_color->b,
		    (GLfloat)mp_color->a
		);
		/* Enable GL_BLEND if alpha is less than 1.0 */
		if(mp_color->a < 1.0f)
		{
		    if(!blend_state)
		    { 
			vmodel->mem_size += (3 * sizeof(GLuint)) +
			    (2 * sizeof(GLuint));
			vmodel->statements += 3;
			StateGLEnableF(state_gl, GL_BLEND, GL_TRUE);
			glBlendFunc(
			    GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA
			);
			StateGLDisableF(
			    state_gl, GL_ALPHA_TEST, GL_TRUE
			);
			blend_state = True;
		    }
		}
		else
		{
		    if(blend_state)
		    {
			vmodel->mem_size += 2 * sizeof(GLuint);
			vmodel->statements += 2;
			StateGLDisableF(state_gl, GL_BLEND, GL_TRUE);
			StateGLEnableF(
			    state_gl, GL_ALPHA_TEST, GL_TRUE
			);
			blend_state = False;
		    }
		}
		break;

	      case V3DMP_TYPE_TEXTURE_SELECT:
		mp_texture_select = (mp_texture_select_struct *)p;
		if(!process_as_ir)
		{
		    const char *texture_name = mp_texture_select->name;
		    vmodel->mem_size += 2 * sizeof(GLuint);
		    vmodel->statements++;
		    if(STRISEMPTY(texture_name))
		    {
			/* Empty string implies unselect texture */
			V3DTextureSelect(NULL);
			tex_on = False;
			tex_orient = TEX_ORIENT_NONE;
		    }
		    else
		    {   
			/* Select texture */
			v3d_texture_ref_struct *t = SARGetTextureRefByName(
			    scene, texture_name
			);
			if(t == NULL)
			{
			    fprintf(
				stderr,
 "%s: Warning: Texture \"%s\" not defined (on the model or globally).\n",
				filename, texture_name
			    );
			    tex_on = False;
			    tex_orient = TEX_ORIENT_NONE;
			}
			else
			{
			    tex_on = True;
			    V3DTextureSelect(t);
			}
		    }
		}
		break;

	      case V3DMP_TYPE_TEXTURE_ORIENT_XY:
		mp_texture_xy = (mp_texture_orient_xy_struct *)p;
		if(process_as_ir)
		    break;
		tex_orient = TEX_ORIENT_XY;
		tex_coord.i = (float)mp_texture_xy->x;
		tex_coord.j = (float)mp_texture_xy->y;
		tex_coord.w = (float)mp_texture_xy->dx;
		tex_coord.h = (float)mp_texture_xy->dy;
		break;

	      case V3DMP_TYPE_TEXTURE_ORIENT_YZ:
		mp_texture_yz = (mp_texture_orient_yz_struct *)p;
		if(process_as_ir)
		    break;
		tex_orient = TEX_ORIENT_YZ;
		tex_coord.i = (float)mp_texture_yz->y;
		tex_coord.j = (float)mp_texture_yz->z;
		tex_coord.w = (float)mp_texture_yz->dy;
		tex_coord.h = (float)mp_texture_yz->dz;
		break;

	      case V3DMP_TYPE_TEXTURE_ORIENT_XZ:
		mp_texture_xz = (mp_texture_orient_xz_struct *)p;
		if(process_as_ir)
		    break;
		tex_orient = TEX_ORIENT_XZ;
		tex_coord.i = (float)mp_texture_xz->x;
		tex_coord.j = (float)mp_texture_xz->z;
		tex_coord.w = (float)mp_texture_xz->dx;
		tex_coord.h = (float)mp_texture_xz->dz;
		break;

	      case V3DMP_TYPE_TEXTURE_OFF:
		if(process_as_ir)
		    break;
		vmodel->mem_size += 2 * sizeof(GLuint);
		vmodel->statements++;
		V3DTextureSelect(NULL);
		tex_on = False;
		tex_orient = TEX_ORIENT_NONE;
		break;

	      case V3DMP_TYPE_HEIGHTFIELD_LOAD:
		mp_heightfield_load = (mp_heightfield_load_struct *)p;
		SARObjLoadHeightField(
		    core_ptr, obj_num, obj_ptr, vmodel,
		    p, filename, 0,
		    list
		);
		break;
	    }
	}	/* Iterate through each V3D model primitive */

	/* Check if last_begin_primitive_type is valid, if it is then
	 * that means we need to call glEnd() to end a previous
	 * glBegin()
	 */
	if(last_begin_primitive_type > -1)
	{
	    vmodel->mem_size += sizeof(GLuint);
	    vmodel->statements++;
	    glEnd();
	    last_begin_primitive_type = -1;
	}

	/* Check if blending is still enabled */
	if(blend_state)
	{
	    vmodel->mem_size += 2 * sizeof(GLuint);
	    vmodel->statements += 2;
	    StateGLDisableF(state_gl, GL_BLEND, GL_TRUE);
	    StateGLEnableF(state_gl, GL_ALPHA_TEST, GL_TRUE);
	    blend_state = False;
	}

	/* End GL display list */
	glEndList();

	/* Mark this SAR Visual Model as finished loading */
	vmodel->load_state = SAR_VISUAL_MODEL_LOADED;

}


/*
 *	Called by SARObjLoadFromFile().
 *
 *	Handles "other data" lines from V3D Models.
 */
static void SARObjLoadLine(
	sar_core_struct *core_ptr,
	int obj_num, sar_object_struct *obj_ptr,
	const char *line, const char *filename, int line_num
)
{
	char *s;
	char parm[256];
	const char *arg;

	sar_object_aircraft_struct	*aircraft = NULL;
	sar_object_ground_struct	*ground = NULL;
	sar_obj_rotor_struct	*rotor = NULL;
	sar_obj_part_struct	*aileron_left_ptr = NULL,
				*aileron_right_ptr = NULL,
				*rudder_top_ptr = NULL,
				*rudder_bottom_ptr = NULL,
				*elevator_ptr = NULL,
				*cannard_ptr = NULL,
				*aileron_elevator_left_ptr = NULL,
				*aileron_elevator_right_ptr = NULL,
				*flap_ptr = NULL,
				*abrake_ptr = NULL,
				*door_ptr = NULL,
				*lgear_ptr = NULL;
	sar_external_fueltank_struct *eft_ptr = NULL;
	sar_scene_struct *scene = core_ptr->scene;
	Boolean is_player = (scene->player_obj_ptr == obj_ptr) ? True : False;

	if(STRISEMPTY(line))
	    return;

	/* Seek past spaces */
	while(ISBLANK(*line))
	    line++;

	/* Skip comments */
	if(ISCOMMENT(*line))
	    return;

	/* Get object type specific values */
	switch(obj_ptr->type)
	{
	  case SAR_OBJ_TYPE_GARBAGE:
	  case SAR_OBJ_TYPE_STATIC:
	  case SAR_OBJ_TYPE_AUTOMOBILE:
	  case SAR_OBJ_TYPE_WATERCRAFT:
	    break;
	  case SAR_OBJ_TYPE_AIRCRAFT:
	    aircraft = SAR_OBJ_GET_AIRCRAFT(obj_ptr);
	    if(aircraft != NULL)
	    {
		int i = aircraft->total_rotors;
		if(i > 0)
		    rotor = aircraft->rotor[i - 1];
	    }
	    break;
	  case SAR_OBJ_TYPE_GROUND:
	    ground = SAR_OBJ_GET_GROUND(obj_ptr);
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
	/* Get pointers to last (newest) object parts of each type */
	if(True)
	{
	    sar_obj_part_struct *part;

#define GET_PART(_type_)	{		\
 int i = 0;					\
 sar_obj_part_struct *p;			\
 part = NULL;					\
 while(True) {					\
  p = SARObjGetPartPtr(obj_ptr, (_type_), i);	\
  if(p != NULL) {				\
   part = p; i++;				\
  } else {					\
   break;					\
  }						\
 }						\
}
	    GET_PART(SAR_OBJ_PART_TYPE_AILERON_LEFT);
	    aileron_left_ptr = part;

	    GET_PART(SAR_OBJ_PART_TYPE_AILERON_RIGHT);
	    aileron_right_ptr = part;

	    GET_PART(SAR_OBJ_PART_TYPE_RUDDER_TOP);
	    rudder_top_ptr = part;

	    GET_PART(SAR_OBJ_PART_TYPE_RUDDER_BOTTOM);
	    rudder_bottom_ptr = part;

	    GET_PART(SAR_OBJ_PART_TYPE_ELEVATOR);
	    elevator_ptr = part;

	    GET_PART(SAR_OBJ_PART_TYPE_CANNARD);
	    cannard_ptr = part;

	    GET_PART(SAR_OBJ_PART_TYPE_AILERON_ELEVATOR_LEFT);
	    aileron_elevator_left_ptr = part;

	    GET_PART(SAR_OBJ_PART_TYPE_AILERON_ELEVATOR_RIGHT);
	    aileron_elevator_right_ptr = part;

	    GET_PART(SAR_OBJ_PART_TYPE_FLAP);
	    flap_ptr = part;

	    GET_PART(SAR_OBJ_PART_TYPE_LANDING_GEAR);
	    lgear_ptr = part;

	    GET_PART(SAR_OBJ_PART_TYPE_DOOR_RESCUE);
	    door_ptr = part;

#undef GET_PART
	}


	/* Begin parsing line */

	/* Get parameter */
	strncpy(parm, line, 256);
	parm[256 - 1] = '\0';
	s = parm;
	while(!ISBLANK(*s) && (*s != '\0'))
	    s++;
	*s = '\0';

	/* Seek to start of argument(s) in the line */
	arg = line;
	while(!ISBLANK(*arg) && (*arg != '\0'))
	    arg++;
	while(ISBLANK(*arg))
	    arg++;

	/* Begin handling parameter */
	if(True)
	{
	    /* Texture Base Directory */
	    if(!strcasecmp(parm, "texture_base_directory") ||
	       !strcasecmp(parm, "texture_base_dir")
	    )
	    {
		/* Ignore since it is loaded from the V3D header */
	    }
	    /* Texture Load */
	    else if(!strcasecmp(parm, "texture_load"))
	    {
		/* Ignore since it is loaded from the V3D header */
	    }
	    /* Version */
	    else if(!strcasecmp(parm, "version"))
	    {
		/* Arguments:
		 *
		 * <major> <minor> <release>
		 */
		int	major = 0,
			minor = 0,
			release = 0;

		arg = GET_ARG_I(arg, &major);
		arg = GET_ARG_I(arg, &minor);
		arg = GET_ARG_I(arg, &release);

		if((major > PROG_VERSION_MAJOR) ||
		   (minor > PROG_VERSION_MINOR) ||
		   (release > PROG_VERSION_RELEASE)
		)
		{
		    Boolean need_warn = False;
		    if(major > PROG_VERSION_MAJOR)
			need_warn = True;
		    else if((major == PROG_VERSION_MAJOR) &&
			    (minor > PROG_VERSION_MINOR)
		    )
			need_warn = True;
		    else if((major == PROG_VERSION_MAJOR) &&
			    (minor == PROG_VERSION_MINOR) &&
			    (release == PROG_VERSION_RELEASE)
		    )
			need_warn = True;
		    if(need_warn)
			fprintf(
			    stderr,
 "%s: Line %i: Warning:\
 File format version %i.%i.%i is newer than\
 program version %i.%i.%i.\n",
			    filename, line_num,
			    major, minor, release,
			    PROG_VERSION_MAJOR, PROG_VERSION_MINOR, PROG_VERSION_RELEASE
			);
		}
	    }
	    /* Name */
	    else if(!strcasecmp(parm, "name"))
	    {
		/* Arguments:
		 *
		 * <name>
		 */
		char *name = NULL;

		arg = GET_ARG_S(arg, &name);

		free(obj_ptr->name);
		obj_ptr->name = name;
	    }
	    /* Description */
	    else if(!strcasecmp(parm, "desc") ||
		    !strcasecmp(parm, "description")
	    )
	    {
		/* Ignore */
	    }
	    /* Type */
	    else if(!strcasecmp(parm, "type"))
	    {
		/* Ignore since the object type should already be set
		 * prior to calling this function
		 */
	    }
	    /* Range */
	    else if(!strcasecmp(parm, "range"))
	    {
		/* Arguments:
		 *
		 * <range>
		 */
		float range = 0.0f;

		arg = GET_ARG_F(arg, &range);

		if(range < 0.0f)
		    fprintf(stderr,
 "%s: Line %i: Warning:\
 Value for %s argument range=%f should not be negative.\n",
			filename, line_num,
			parm, range
		    );

		obj_ptr->range = (float)MAX(range, 0.0);
	    }
	    /* Range Far */
	    else if(!strcasecmp(parm, "range_far"))
	    {
		/* Arguments:
		 *
		 * <range_far>
		 */
		float range_far = 0.0f;

		arg = GET_ARG_F(arg, &range_far);

		if(range_far < 0.0f)
		    fprintf(stderr,
 "%s: Line %i: Warning:\
 Value for %s argument range_far=%f should not be negative.\n",
			filename, line_num,
			parm, range_far
		    );

		obj_ptr->range_far = (float)MAX(range_far, 0.0);
	    }
	    /* No Depth Test */
	    else if(!strcasecmp(parm, "no_depth_test"))
	    {
		obj_ptr->flags |= SAR_OBJ_FLAG_NO_DEPTH_TEST;
	    }
	    /* Smooth Shading */
	    else if(!strcasecmp(parm, "shade_model_smooth"))
	    {
		obj_ptr->flags |= SAR_OBJ_FLAG_SHADE_MODEL_SMOOTH;
	    }
	    /* Flat Shading */
	    else if(!strcasecmp(parm, "shade_model_flat"))
	    {
		obj_ptr->flags &= ~SAR_OBJ_FLAG_SHADE_MODEL_SMOOTH;
	    }
	    /* Offset Polygons */
	    else if(!strcasecmp(parm, "offset_polygons"))
	    {
		obj_ptr->flags |= SAR_OBJ_FLAG_POLYGON_OFFSET;
	    }
	    /* Show Night Model At Dawn */
	    else if(!strcasecmp(parm, "show_night_model_at_dawn"))
	    {
		obj_ptr->flags |= SAR_OBJ_FLAG_NIGHT_MODEL_AT_DAWN;
	    }
	    /* Show Night Model At Dusk */
	    else if(!strcasecmp(parm, "show_night_model_at_dusk"))
	    {
		obj_ptr->flags |= SAR_OBJ_FLAG_NIGHT_MODEL_AT_DUSK;
	    }
	    /* Show Far Model Day Only */
	    else if(!strcasecmp(parm, "show_far_day_only"))
	    {
		obj_ptr->flags |= SAR_OBJ_FLAG_FAR_MODEL_DAY_ONLY;
	    }
	    /* Crash Flags */
	    else if(!strcasecmp(parm, "crash_flags"))
	    {
		/* Arguments:
		 *
		 * <can_crash?> <causes_crash?> <support_surface?>
		 * <crash_type>
		 */
		sar_contact_bounds_struct *cb = obj_ptr->contact_bounds;
		Boolean	can_crash = False,
			causes_crash = False,
			support_surface = False;
		int	crash_type;

		arg = GET_ARG_B(arg, &can_crash);
		arg = GET_ARG_B(arg, &causes_crash);
		arg = GET_ARG_B(arg, &support_surface);
		arg = GET_ARG_I(arg, &crash_type);

		/* Create contact bounds as needed */
		if(cb == NULL)
		    obj_ptr->contact_bounds = cb = SAR_CONTACT_BOUNDS(
			calloc(1, sizeof(sar_contact_bounds_struct))
		    );

		if(cb != NULL)
		{
		    cb->crash_flags = 0;	/* Must reset flags */

		    /* Crash into other objects? */
		    if(can_crash)
			cb->crash_flags |= SAR_CRASH_FLAG_CRASH_OTHER;

		    /* Causes crash? */
		    if(causes_crash)
			cb->crash_flags |= SAR_CRASH_FLAG_CRASH_CAUSE;

		    /* Support surface? */
		    if(support_surface)
			cb->crash_flags |= SAR_CRASH_FLAG_SUPPORT_SURFACE;

		    /* Crash type code */
		    cb->crash_type = crash_type;
		}
	    }
	    /* Contact Bounds Spherical */
	    else if(!strcasecmp(parm, "contact_spherical"))
	    {
		/* Arguments:
		 *
		 * <radius>
		 */
		sar_contact_bounds_struct *cb = obj_ptr->contact_bounds;
		float radius = 0.0f;

		arg = GET_ARG_F(arg, &radius);

		if(radius < 0.0f)
		    fprintf(stderr,
 "%s: Line %i: Warning:\
 Value for %s argument radius=%f should not be negative.\n",
			filename, line_num,
			parm, radius
		    );

		SARObjAddContactBoundsSpherical(
		    obj_ptr,
		    (cb != NULL) ? cb->crash_flags : 0,
		    (cb != NULL) ? cb->crash_type : 0,
		    radius
		);
	    }
	    /* Contact Bounds Cylendical */
	    else if(!strcasecmp(parm, "contact_cylendrical"))
	    {
		/* Arguments:
		 *
		 * <radius> <height_min> <height_max>
		 */
		sar_contact_bounds_struct *cb = obj_ptr->contact_bounds;
		float	radius = 0.0f,
			height_min = 0.0f,
			height_max = 0.0f;

		arg = GET_ARG_F(arg, &radius);
		arg = GET_ARG_F(arg, &height_min);
		arg = GET_ARG_F(arg, &height_max);

		if(radius < 0.0f)
		    fprintf(stderr,
 "%s: Line %i: Warning:\
 Value for %s argument radius=%f should not be negative.\n",
			filename, line_num,
			parm, radius
		    );
		if(height_min > height_max)
		    fprintf(stderr,
 "%s: Line %i: Warning:\
 Value for %s argument height_min=%f is greater than height_max=%f.\n",
			filename, line_num,
			parm, height_min, height_max
		    );

		SARObjAddContactBoundsCylendrical(
		    obj_ptr,
		    (cb != NULL) ? cb->crash_flags : 0,
		    (cb != NULL) ? cb->crash_type : 0,
		    radius, height_min, height_max
		);
	    }
	    /* Contact Bounds Rectangular */
	    else if(!strcasecmp(parm, "contact_rectangular"))
	    {
		/* Arguments:
		 *
		 * <x_min> <x_max>
		 * <y_min> <y_max>
		 * <z_min> <z_max>
		 */
		sar_contact_bounds_struct *cb = obj_ptr->contact_bounds;
		float	x_min = 0.0f,
			x_max = 0.0f,
			y_min = 0.0f,
			y_max = 0.0f,
			z_min = 0.0f,
			z_max = 0.0f;

		arg = GET_ARG_F(arg, &x_min);
		arg = GET_ARG_F(arg, &x_max);
		arg = GET_ARG_F(arg, &y_min);
		arg = GET_ARG_F(arg, &y_max);
		arg = GET_ARG_F(arg, &z_min);
		arg = GET_ARG_F(arg, &z_max);

		if(x_min > x_max)
		    fprintf(stderr,
 "%s: Line %i: Warning:\
 Value for %s argument x_min=%f is greater than x_max=%f.\n",
			filename, line_num,
			parm, x_min, x_max
		    );
		if(y_min > y_max)
		    fprintf(stderr,
 "%s: Line %i: Warning:\
 Value for %s argument y_min=%f is greater than y_max=%f.\n",
			filename, line_num,
			parm, y_min, y_max
		    );
		if(z_min > z_max)
		    fprintf(stderr,
 "%s: Line %i: Warning:\
 Value for %s argument z_min=%f is greater than z_max=%f.\n",
			filename, line_num,
			parm, z_min, z_max
		    );

		SARObjAddContactBoundsRectangular(
		    obj_ptr,
		    (cb != NULL) ? cb->crash_flags : 0,
		    (cb != NULL) ? cb->crash_type : 0,
		    x_min, x_max,
		    y_min, y_max,
		    z_min, z_max
		);
	    }
	    /* Temperature */
	    else if(!strcasecmp(parm, "temperature"))
	    {
		/* Arguments:
		 *
		 * <temperature_day> <temperature_dusk/dawn>
		 * <temperature_night>
		 */
		float	temperature_day = SAR_DEF_TEMPERATURE,
			temperature_dusk = SAR_DEF_TEMPERATURE,
			temperature_night = SAR_DEF_TEMPERATURE;
		arg = GET_ARG_F(arg, &temperature_day);
		arg = GET_ARG_F(arg, &temperature_dusk);
		arg = GET_ARG_F(arg, &temperature_night);

		if((temperature_day < 0.0f) || (temperature_day > 1.0f))
		    fprintf(stderr,
 "%s: Line %i: Warning:\
 Value for %s argument temperature_day=%f is out of range (0.0 to 1.0).\n",
			filename, line_num,
			parm, temperature_day
		    );
		if((temperature_dusk < 0.0f) || (temperature_dusk > 1.0f))
		    fprintf(stderr,
 "%s: Line %i: Warning:\
 Value for %s argument temperature_dusk=%f is out of range (0.0 to 1.0).\n",
			filename, line_num,
			parm, temperature_dusk
		    );
		if((temperature_night < 0.0f) || (temperature_night > 1.0f))
		    fprintf(stderr,
 "%s: Line %i: Warning:\
 Value for %s argument temperature_night=%f is out of range (0.0 to 1.0).\n",
			filename, line_num,
			parm, temperature_night
		    );

		obj_ptr->temperature = CLIP(temperature_day, 0.0f, 1.0f);
	    }
	    /* Speed */
	    else if(!strcasecmp(parm, "speed"))
	    {
		/* Arguments:
		 *
		 * <speed_stall> <speed_max>
		 * <min_drag>
		 * <overspeed_expected> <overspeed>
		 */
		float	speed_stall = 0.0f,
			speed_max = 0.0f,
			min_drag = 0.0f,
			overspeed_expected = 0.0f,
			overspeed = 0.0f;

		arg = GET_ARG_F(arg, &speed_stall);
		arg = GET_ARG_F(arg, &speed_max);
		arg = GET_ARG_F(arg, &min_drag);
		arg = GET_ARG_F(arg, &overspeed_expected);
		arg = GET_ARG_F(arg, &overspeed);

		if(speed_max < 0.0f)
		    fprintf(stderr,
 "%s: Line %i: Warning:\
 Value for %s argument speed_max=%f should not be negative.\n",
			filename, line_num,
			parm, speed_max
		    );
		if(min_drag < 0.0f)
		    fprintf(stderr,
 "%s: Line %i: Warning:\
 Value for %s argument min_drag=%f should not be negative.\n",
			filename, line_num,
			parm, min_drag
		    );
		if(overspeed_expected > overspeed)
		    fprintf(stderr,
 "%s: Line %i: Warning:\
 Value for %s argument overspeed_expected=%f should not be\
 greater than the value for speed argument overspeed=%f.\n",
			filename, line_num,
			parm, overspeed_expected, overspeed
		    );

		if(aircraft != NULL)
		{
		    aircraft->speed_stall = (float)SFMMPHToMPC(speed_stall);
		    aircraft->speed_max = (float)SFMMPHToMPC(speed_max);
		    aircraft->min_drag = (float)SFMMPHToMPC(min_drag);
		    aircraft->overspeed_expected = (float)SFMMPHToMPC(overspeed_expected);
		    aircraft->overspeed = (float)SFMMPHToMPC(overspeed);
		}
	    }
	    /* Air Brakes */
	    else if(!strcasecmp(parm, "air_brakes"))
	    {
		/* Arguments:
		 *
		 * <drag_rate>
		 */
		float	area = 0.0f;

		arg = GET_ARG_F(arg, &area);

		if(aircraft != NULL)
		{
		    aircraft->air_brakes_area = (float)area;
		    if(aircraft->air_brakes_area <= 0.0f)
			aircraft->air_brakes_state = -1;
		    else
			aircraft->air_brakes_state = 0;
		}
	    }

	    /* Helicopter Acceleration Responsiveness */
	    else if(!strcasecmp(parm, "helicopter_accelresp") ||
		    !strcasecmp(parm, "helicopter_acceleration_responsiveness")
	    )
	    {
		/* Arguments:
		 *
		 * <i> <j> <k>
		 */
		sar_position_struct	a;

		arg = GET_ARG_POS(arg, &a);

		if(a.x <= 0.0f)
		    fprintf(stderr,
 "%s: Line %i: Warning:\
 Value for %s argument i=%f is not positive.\n",
			filename, line_num,
			parm, a.x
		    );
		if(a.y <= 0.0f)
		    fprintf(stderr,
 "%s: Line %i: Warning:\
 Value for %s argument j=%f is not positive.\n",
			filename, line_num,
			parm, a.y
		    );
		if(a.z <= 0.0f)
		    fprintf(stderr,
 "%s: Line %i: Warning:\
 Value for %s argument k=%f is not positive.\n",
			filename, line_num,
			parm, a.z
		    );

		if(aircraft != NULL)
		    memcpy(
			&aircraft->accel_responsiveness,
			&a,
			sizeof(sar_position_struct)
		    );
	    }
	    /* Airplane Acceleration Responsiveness */
	    else if(!strcasecmp(parm, "airplane_accelresp") ||
		    !strcasecmp(parm, "airplane_acceleration_responsiveness")
	    )
	    {
		/* Arguments:
		 *
		 * <i> <j> <k>
		 */
		sar_position_struct	a;

		arg = GET_ARG_POS(arg, &a);

		if(a.x <= 0.0f)
		    fprintf(stderr,
 "%s: Line %i: Warning:\
 Value for %s argument i=%f is not positive.\n",
			filename, line_num,
			parm, a.x
		    );
		if(a.y <= 0.0f)
		    fprintf(stderr,
 "%s: Line %i: Warning:\
 Value for %s argument j=%f is not positive.\n",
			filename, line_num,
			parm, a.y
		    );
		if(a.z <= 0.0f)
		    fprintf(stderr,
 "%s: Line %i: Warning:\
 Value for %s argument k=%f is not positive.\n",
			filename, line_num,
			parm, a.z
		    );

		if(aircraft != NULL)
		    memcpy(
			&aircraft->airplane_accel_responsiveness,
			&a,
			sizeof(sar_position_struct)
		    );
	    }
	    /* Cockpit Offset */
	    else if(!strcasecmp(parm, "cockpit_offset"))
	    {
		/* Arguments:
		 *
		 * <x> <y> <z>
		 */
		sar_position_struct	pos;

		arg = GET_ARG_POS(arg, &pos);

		if(aircraft != NULL)
		    memcpy(
			&aircraft->cockpit_offset_pos,
			&pos,
			sizeof(sar_position_struct)
		    );
	    }
	    /* Control Panel */
	    else if(!strcasecmp(parm, "control_panel") &&
		    is_player
	    )
	    {
		/* Arguments:
		 *
		 * <x> <y> <z>
		 * <heading> <pitch> <bank>
		 * <width> <height>
		 * <path>
		 *
		 * Note that position and size units are in centimeters
		 * The <path> specifies the control panel directory
		 * (not the instruments file)
		 */
		sar_position_struct	pos;
		sar_direction_struct	dir;
		float	width = 0.0f,
			height = 0.0f;
		char	*path = NULL;
		arg = GET_ARG_POS(arg, &pos);
		arg = GET_ARG_DIR(arg, &dir);
		arg = GET_ARG_F(arg, &width);
		arg = GET_ARG_F(arg, &height);
		arg = GET_ARG_S(arg, &path);

		if(!STRISEMPTY(path))
		{
		    ControlPanel *cp = CPNew(core_ptr->display);
		    CPDelete((ControlPanel *)scene->player_control_panel);
		    scene->player_control_panel = cp;
		    if(cp != NULL)
		    {
			CPLoadFromFile(cp, path);
			CPSetPosition(cp, pos.x, pos.y, pos.z);
			CPSetDirection(cp, dir.heading, dir.pitch, dir.bank);
			CPSetSize(cp, width, height);
		    }
		}
		else
		{
		    fprintf(stderr,
 "%s: Line %i: Warning:\
 Value for %s argument path is not given.\n",
			filename, line_num,
			parm
		    );
		}

		free(path);
	    }
	    /* Belly Height */
	    else if(!strcasecmp(parm, "belly_height"))
	    {
		/* Arguments:
		 *
		 * <height>
		 */
		float	height = 0.0f;

		arg = GET_ARG_F(arg, &height);

		if(aircraft != NULL)
		    aircraft->belly_height = height;
	    }
	    /* Length */
	    else if(!strcasecmp(parm, "length"))
	    {
		/* Arguments:
		 *
		 * <length>
		 */
		float	length = 0.0f;

		arg = GET_ARG_F(arg, &length);

		if(aircraft != NULL)
		    aircraft->length = length;
	    }
	    else if(!strcasecmp(parm, "wingspan"))
	    {
		/* Arguments:
		 *
		 * <wingspan>
		 */
		float	wingspan = 0.0f;

		arg = GET_ARG_F(arg, &wingspan);

		if(aircraft != NULL)
		    aircraft->wingspan = wingspan;
	    }
	    /* Landing Gear Height */
	    else if(!strcasecmp(parm, "gear_height"))
	    {
		/* Arguments:
		 *
		 * <height>
		 */
		float	height = 0.0f;

		arg = GET_ARG_F(arg, &height);

		if(height < 0.0f)
		    fprintf(stderr,
 "%s: Line %i: Warning:\
 Value for %s argument height=%f should not be negative.\n",
			filename, line_num,
			parm, height
		    );

		if(aircraft != NULL)
		    aircraft->gear_height = height;
	    }
	    /* Ground Turning */
	    else if(!strcasecmp(parm, "ground_turning"))
	    {
		/* Arguments:
		 *
		 * <turn_rad> <turn_vel_opt> <turn_vel_max>
		 *
		 * All units are in miles per hour
		 */
		float	turn_rad = 0.0f,
			turn_vel_opt = 0.0f,
			turn_vel_max = 0.0f;

		arg = GET_ARG_F(arg, &turn_rad);
		arg = GET_ARG_F(arg, &turn_vel_opt);
		arg = GET_ARG_F(arg, &turn_vel_max);

		if(aircraft != NULL)
		{
		    /* Turn radius in meters, distance from farthest
		     * non-turnable wheel to turnable wheel
		     *
		     * Can be negative
		     *
		     * A value of 0 specifies no turning
		     */
		    aircraft->gturn_radius = turn_rad;

		    /* Optimul ground turning velocity along
		     * aircraft's y axis in meters per cycle
		     */
		    aircraft->gturn_vel_opt =
			(float)SFMMPHToMPC(turn_vel_opt);

		    /* Maximum ground turning velocity along
		     * aircraft's y axis in meters per cycle
		     */
		    aircraft->gturn_vel_max =
			(float)SFMMPHToMPC(turn_vel_max);
		}
	    }
	    /* Dry Mass */
	    else if(!strcasecmp(parm, "dry_mass"))
	    {
		/* Arguments:
		 *
		 * <mass>
		 */
		float	mass = 0.0f;

		arg = GET_ARG_F(arg, &mass);

		if(mass < 0)
		    fprintf(stderr,
 "%s: Line %i: Warning:\
 Value for %s argument mass=%f should not be negative.\n",
			filename, line_num,
			parm, mass
		    );

		if(aircraft != NULL)
		    aircraft->dry_mass = mass;
	    }
	    /* Fuel */
	    else if(!strcasecmp(parm, "fuel"))
	    {
		/* Arguments:
		 *
		 * <consumption_rate>
		 * <initial> <max>
		 *
		 * Units for <consumption_rate> are in kg per second
		 * Units for <fuel_init> and <fuel_max> are in kg.
		 */
		float	consumption_rate = 0.0f,
			initial = 0.0f,
			max = 0.0f;

		arg = GET_ARG_F(arg, &consumption_rate);
		arg = GET_ARG_F(arg, &initial);
		arg = GET_ARG_F(arg, &max);

		if(consumption_rate < 0.0f)
		    fprintf(stderr,
 "%s: Line %i: Warning:\
 Value for %s argument consumption_rate=%f should not be negative.\n",
			filename, line_num,
			parm, consumption_rate
		    );
		if(initial < 0.0f)
		    fprintf(stderr,
 "%s: Line %i: Warning:\
 Value for %s argument initial=%f should not be negative.\n",
			filename, line_num,
			parm, initial
		    );
		if(max < 0.0f)
		    fprintf(stderr,
 "%s: Line %i: Warning:\
 Value for %s argument max=%f should not be negative.\n",
			filename, line_num,
			parm, max
		    );

		if(aircraft != NULL)
		{
		    /* Convert fuel rate from (kg / sec) to (kg / cycle) */
		    aircraft->fuel_rate = (float)MAX(
			consumption_rate * SAR_SEC_TO_CYCLE_COEFF, 0.0
		    );

		    /* Fuel in kg */
		    aircraft->fuel = (float)MAX(initial, 0.0);
		    aircraft->fuel_max = (float)MAX(max, 0.0);
		}
	    }
	    /* Crew */
	    else if(!strcasecmp(parm, "crew"))
	    {
		/* Arguments:
		 *
		 * <crew>
		 * <passengers> <passengers_max>
		 */
		int	crew = 0,
			passengers = 0,
			passengers_max = 0;

		arg = GET_ARG_I(arg, &crew);
		arg = GET_ARG_I(arg, &passengers);
		arg = GET_ARG_I(arg, &passengers_max);

		if(crew < 0)
		    fprintf(stderr,
 "%s: Line %i: Warning:\
 Value for %s argument crew=%i should not be negative.\n",
			filename, line_num,
			parm, crew
		    );
		if(passengers < 0)
		    fprintf(stderr,
 "%s: Line %i: Warning:\
 Value for %s argument passengers=%i should not be negative.\n",
			filename, line_num,
			parm, passengers
		    );
		if(passengers_max < 0)
		    fprintf(stderr,
 "%s: Line %i: Warning:\
 Value for %s argument passengers_max=%i should not be negative.\n",
			filename, line_num,
			parm, passengers_max
		    );

		if(aircraft != NULL)
		{
		    aircraft->crew = crew;
		    aircraft->passengers = passengers;
		    aircraft->passengers_max = passengers_max;
		}
	    }
	    /* Engine */
	    else if(!strcasecmp(parm, "engine"))
	    {
		/* Arguments:
		 *
		 * <can_pitch?> <initial_pitch>
		 * <power>
		 * <collective_range>
		 *
		 * Power from units of kg * m / cycle^2.
		 */
		Boolean	can_pitch = False;
		int	initial_pitch = 0;
		float	power = 0.0f,
			collective_range = 0.0f;

		arg = GET_ARG_B(arg, &can_pitch);
		arg = GET_ARG_I(arg, &initial_pitch);
		arg = GET_ARG_F(arg, &power);
		arg = GET_ARG_F(arg, &collective_range);

		if(power < 0.0f)
		    fprintf(stderr,
 "%s: Line %i: Warning:\
 Value for %s argument power=%f should not be negative.\n",
			filename, line_num,
			parm, power
		    );
		if((collective_range < 0.0f) || (collective_range > 1.0f))
		    fprintf(stderr,
 "%s: Line %i: Warning:\
 Value for %s argument collective_range=%f is out of range [0.0, 1.0].\n",
			filename, line_num,
			parm, collective_range
		    );

		if(aircraft != NULL)
		{
		    /* Can pitch? */
		    aircraft->engine_can_pitch = can_pitch;

		    /* Initial rotor pitch state, this determines the
		     * the value for member flight_model_type
		     */
		    switch(initial_pitch)
		    {
		      case 1:
			aircraft->flight_model_type =
			    SAR_FLIGHT_MODEL_AIRPLANE;
			break;
		      case 0:
			aircraft->flight_model_type =
			    SAR_FLIGHT_MODEL_HELICOPTER;
			break;
		      default:
			fprintf(stderr,
 "%s: Line %i: Warning:\
 Value for %s argument init_pitch=%i is not supported.\n",
			    filename, line_num,
			    parm, initial_pitch
			);
			break;
		    }

		    /* Explicitly set previous flight model type to the
		     * same type as the current one since this is the
		     * first time this is being set for this object
		     */
		    aircraft->last_flight_model_type =
			aircraft->flight_model_type;

		    /* Engine power (in kg * m / cycle^2) */
		    aircraft->engine_power = (float)MAX(
			power, 0.0
		    );

		    /* Collective range coefficient (from 0.0 to 1.0) */
		    aircraft->collective_range = (float)CLIP(
			collective_range, 0.0, 1.0
		    );
		}
	    }
	    /* Service Ceiling */  
	    else if(!strcasecmp(parm, "service_ceiling"))
	    {
		/* Arguments:
		 *
		 * <altitude>
		 */
		float	altitude = 0.0f;

		arg = GET_ARG_F(arg, &altitude);

		if(aircraft != NULL)
		    aircraft->service_ceiling =
			(float)SFMFeetToMeters(altitude);
	    }
	    /* Attitude Change Rates */
	    else if(!strcasecmp(parm, "attitude_change_rate"))
	    {
		/* Arguments:
		 *
		 * <heading> <pitch> <bank>
		 *
		 * Units are in degrees per second
		 */
		sar_direction_struct	a;

		arg = GET_ARG_DIR(arg, &a);

		a.heading *= (float)SAR_SEC_TO_CYCLE_COEFF;
		a.pitch *= (float)SAR_SEC_TO_CYCLE_COEFF;
		a.bank *= (float)SAR_SEC_TO_CYCLE_COEFF;

		if(aircraft != NULL)
		    memcpy(
			&aircraft->attitude_change_rate,
			&a,
			sizeof(sar_direction_struct)
		    );
	    }
	    /* Attitude Leveling */
	    else if(!strcasecmp(parm, "attitude_leveling"))
	    {
		/* Arguments:
		 *
		 * <heading> <pitch> <bank>
		 *
		 * Units are in degrees per second
		 */
		sar_direction_struct	a;

		arg = GET_ARG_DIR(arg, &a);

		a.heading *= (float)SAR_SEC_TO_CYCLE_COEFF;
		a.pitch *= (float)SAR_SEC_TO_CYCLE_COEFF;
		a.bank *= (float)SAR_SEC_TO_CYCLE_COEFF;

		if(aircraft != NULL)
		{
		    aircraft->pitch_leveling = a.pitch;
		    aircraft->bank_leveling = a.bank;
		}
	    }
	    /* Ground Pitch Offset */
	    else if(!strcasecmp(parm, "ground_pitch_offset"))
	    {
		/* Arguments:
		 *
		 * <ground_pitch_offset>
		 */
		float	ground_pitch_offset;

		arg = GET_ARG_F(arg, &ground_pitch_offset);
		ground_pitch_offset = (float)DEGTORAD(ground_pitch_offset);

		if(aircraft != NULL)
		    aircraft->ground_pitch_offset = ground_pitch_offset;
	    }
	    /* New Light */
	    else if(!strcasecmp(parm, "light_new"))
	    {
		/* Arguments:
		 *
		 * <x> <y> <z>
		 * <r> <g> <b> <a>
		 * <radius>
		 * <init_on?> <type> <int_on> <int_off>
		 * <int_on_delay>
		 *
		 * Units for <radius> are in pixels
		 * Units for <int_*> are in milliseconds
		 */
		sar_light_struct	*light;
		sar_position_struct	pos;
		sar_color_struct	c;
		int	radius = 0;
		Boolean init_on = False;
		int	type = 0,
			int_on = 0,
			int_off = 0,
			int_on_delay = 0;

		arg = GET_ARG_POS(arg, &pos);
		arg = GET_ARG_RGBA(arg, &c);
		arg = GET_ARG_I(arg, &radius);
		arg = GET_ARG_B(arg, &init_on);
		arg = GET_ARG_I(arg, &type);
		arg = GET_ARG_I(arg, &int_on);
		arg = GET_ARG_I(arg, &int_off);
		arg = GET_ARG_I(arg, &int_on_delay);

		if(radius < 0)
		    fprintf(stderr,
 "%s: Line %i: Warning:\
 Value for %s argument radius=%i should not be negative (use 0 for default).\n",
			filename, line_num,
			parm, radius
		    );
		if(int_on < 0)
		    fprintf(stderr,
 "%s: Line %i: Warning:\
 Value for %s argument int_on=%i should not be negative (use 0 for none).\n",
			filename, line_num,
			parm, int_on
		    );
		if(int_off < 0)
		    fprintf(stderr,
 "%s: Line %i: Warning:\
 Value for %s argument int_off=%i should not be negative (use 0 for none).\n",
			filename, line_num,
			parm, int_off
		    );
		if(int_on_delay < 0)
		    fprintf(stderr,
 "%s: Line %i: Warning:\
 Value for %s argument int_on_delay=%i should not be negative (use 0 for none).\n",
			filename, line_num,
			parm, int_on_delay
		    );

		/* Create new light */
		light = SARObjLightNew(
		    scene,
		    &obj_ptr->light, &obj_ptr->total_lights
		);
		if(light != NULL)
		{
		    /* Position */
		    memcpy(&light->pos, &pos, sizeof(sar_position_struct));

		    /* Color */
		    memcpy(&light->color, &c, sizeof(sar_color_struct));

		    /* Radius */
		    light->radius = (int)MAX(radius, 0);

		    /* Initially on? */
		    if(init_on)  
			light->flags |= SAR_LIGHT_FLAG_ON;

		    /* Type of light */
		    switch(type)
		    {
		      case 2:	/* Spot light */
			light->flags |= SAR_LIGHT_FLAG_ATTENUATE;
			break;
		      case 1:	/* Strobe */
			light->flags |= SAR_LIGHT_FLAG_STROBE;
			break;
		      case 0:
			break;
		      default:
			fprintf(stderr,
 "%s: Line %i: Warning:\
 Value for %s argument type=%i is not supported [0 | 1 | 2].\n",
			    filename, line_num,
			    parm, type
			);
			break;
		    }

		    /* Strobe on and off intervals */
		    light->int_on = (time_t)int_on;
		    light->int_off = (time_t)int_off;
		    light->int_delay_on = (time_t)int_on_delay;
		}
	    }
	    /* New Sound Source */
	    else if(!strcasecmp(parm, "sound_source_new"))
	    {
		SARObjLoadSoundSource(
		    scene,
		    &obj_ptr->sndsrc, &obj_ptr->total_sndsrcs,
		    arg, filename, line_num
		);
	    }
	    /* New Rotor */
	    else if(!strcasecmp(parm, "rotor_new") ||
		    !strcasecmp(parm, "propellar_new")
	    )
	    {
		/* Arguments:
		 *
		 * <nblades>
		 * <x> <y> <z>
		 * <heading> <pitch> <bank>
		 * <radius> <has_prop_wash?> <follow_controls?>
		 * <blur_criteria>
		 * <can_pitch?> <no_pitch_landed?> <no_rotate?>
		 * <blades_offset>
		 */
		int	nblades = 0;
		sar_position_struct	pos;
		sar_direction_struct	dir;
		float	radius = 0.0f;
		Boolean	has_prop_wash = False,
			follow_controls = False;
		int	blur_criteria = 0;
		Boolean	can_pitch = False,
			no_pitch_landed = False,
			no_rotate = False;
		float	blades_offset = 0.0f;

		arg = GET_ARG_I(arg, &nblades);
		arg = GET_ARG_POS(arg, &pos);
		arg = GET_ARG_DIR(arg, &dir);
		arg = GET_ARG_F(arg, &radius);
		arg = GET_ARG_B(arg, &has_prop_wash);
		arg = GET_ARG_B(arg, &follow_controls);
		arg = GET_ARG_I(arg, &blur_criteria);
		arg = GET_ARG_B(arg, &can_pitch);
		arg = GET_ARG_B(arg, &no_pitch_landed);
		arg = GET_ARG_B(arg, &no_rotate);
		arg = GET_ARG_F(arg, &blades_offset);

		if(radius < 0.0f)
		    fprintf(stderr,
 "%s: Line %i: Warning:\
 Value for %s argument radius=%f should not be negative.\n",
			filename, line_num,
			parm, radius
		    );

		rotor = NULL;
		if(aircraft != NULL)
		{
		    int n = SARObjRotorNew(
			scene,
			&aircraft->rotor,
			&aircraft->total_rotors
		    );
		    rotor = (n > -1) ?
			aircraft->rotor[n] : NULL;
		}
		if(rotor != NULL)
		{
		    /* Total blades */
		    rotor->total_blades = nblades;

		    /* Position */
		    memcpy(&rotor->pos, &pos, sizeof(sar_position_struct));

		    /* Direction */
		    memcpy(&rotor->dir, &dir, sizeof(sar_direction_struct));

		    /* Radius of blades */
		    rotor->radius = (float)MAX(radius, 0.0f);

		    /* Has rotor wash? */
		    if(has_prop_wash)
			rotor->wash_tex_num = SARGetTextureRefNumberByName(
			    scene, SAR_STD_TEXNAME_ROTOR_WASH
			);
		    else
			rotor->wash_tex_num = -1;

		    /* Follow controls for pitch and bank? */
		    if(follow_controls)
			rotor->flags |= SAR_ROTOR_FLAG_FOLLOW_CONTROLS;

		    /* Blur criteria */
		    switch(blur_criteria)
		    {
		      case 2:	/* Blur always */
			rotor->flags |= SAR_ROTOR_FLAG_BLUR_ALWAYS;
			break;
		      case 1:	/* Blur when spinning fast */
			rotor->flags |= SAR_ROTOR_FLAG_BLUR_WHEN_FAST;
			break;
		      case 0:	/* Never blur */
			break;
		      default:
			fprintf(stderr,
 "%s: Line %i: Warning:\
 Value for %s argument blur_criteria=%i is not supported [0 | 1 | 2].\n",
			    filename, line_num,
			    parm, blur_criteria
			);
			break;
		    }

		    /* Can pitch? */
		    if(can_pitch)
			rotor->flags |= SAR_ROTOR_FLAG_CAN_PITCH;

		    /* No pitching of rotors when landed? */
		    if(no_pitch_landed)
			rotor->flags |= SAR_ROTOR_FLAG_NO_PITCH_LANDED;

		    /* No rotate? */
		    if(!no_rotate)
			rotor->flags |= SAR_ROTOR_FLAG_SPINS;

		    /* Blades offset */
		    rotor->blades_offset = blades_offset;

		    /* Rotor blur texture */
		    rotor->blade_blur_tex_num = SARGetTextureRefNumberByName(
			scene, SAR_STD_TEXNAME_ROTOR_BLADE_BLUR
		    );
		}
		else
		{
		    fprintf(stderr,
 "%s: Line %i: Error: Unable to create rotor (check object type).\n",
			filename, line_num
		    );
		}
	    }
	    /* Rotor Blur Color */
	    else if(!strcasecmp(parm, "rotor_blur_color") ||
		    !strcasecmp(parm, "propellar_blur_color")
	    )
	    {
		/* Arguments:
		 *
		 * <r> <b> <g> <a>
		 */
		sar_color_struct	c;

		arg = GET_ARG_RGBA(arg, &c);

		if(rotor != NULL)
		{
		    if((rotor->flags & SAR_ROTOR_FLAG_BLUR_WHEN_FAST) ||
		       (rotor->flags & SAR_ROTOR_FLAG_BLUR_ALWAYS)
		    )
		    {
			memcpy(
			    &rotor->blades_blur_color,
			    &c,
			    sizeof(sar_color_struct)
			);
		    }
		    else
		    {
			fprintf(
			    stderr,
 "%s: Line %i: Warning:\
 Rotor blur_criteria is set to never blur (rotor_blur_color ignored).\n",
			    filename, line_num
			);
		    }
		}
	    }
	    /* Rotor Blade Blur Texture */
	    else if(!strcasecmp(parm, "rotor_blade_blur_texture"))
	    {
		/* Arguments:
		 *
		 * <name>
		 */
		char *name = NULL;

		arg = GET_ARG_S(arg, &name);

		if(rotor != NULL)
		{
		    rotor->blade_blur_tex_num = SARGetTextureRefNumberByName(
			scene, name
		    );
		}

		free(name);
	    }
	    /* New Air Brake */
	    else if(!strcasecmp(parm, "air_brake_new"))
	    {
		/* Arguments:
		 *
		 * <x> <y> <z>
		 * <heading> <pitch> <bank>
		 * <dep_dheading> <dep_dpitch> <dep_dbank>
		 * <deployed?> <anim_rate>
		 * <visible_when_retracted?>
		 */
		sar_position_struct	pos;
		sar_direction_struct	dir,
					dep_dir;
		Boolean	deployed = False;
		int	anim_rate = 0;
		Boolean visible_when_retracted = False;

		arg = GET_ARG_POS(arg, &pos);
		arg = GET_ARG_DIR(arg, &dir);
		arg = GET_ARG_DIR(arg, &dep_dir);
		arg = GET_ARG_B(arg, &deployed);
		arg = GET_ARG_I(arg, &anim_rate);
		arg = GET_ARG_B(arg, &visible_when_retracted);

		abrake_ptr = NULL;
		if(aircraft != NULL)
		    abrake_ptr = SARObjAirBrakeNew(
			scene,
			&aircraft->part,
			&aircraft->total_parts
		    );
		if(abrake_ptr != NULL)
		{
		    abrake_ptr->flags = 0;	/* Must reset flags */

		    /* Position */
		    memcpy(
			&abrake_ptr->pos_min,
			&pos,
			sizeof(sar_position_struct)
		    );
		    memcpy(
			&abrake_ptr->pos_cen,
			&pos,
			sizeof(sar_position_struct)
		    );
		    memcpy(
			&abrake_ptr->pos_max,
			&pos,
			sizeof(sar_position_struct)
		    );

		    /* Direction */
		    memcpy(
			&abrake_ptr->dir_min,
			&dir,
			sizeof(sar_direction_struct)
		    );
		    memcpy(
			&abrake_ptr->dir_cen,
			&dir,
			sizeof(sar_direction_struct)
		    );

		    /* Deployed direction */
		    memcpy(
			&abrake_ptr->dir_max,
			&dep_dir,
			sizeof(sar_direction_struct)
		    );

		    /* Air brake initially deployed? */
		    if(deployed)
		    {
			/* Air brake initially deployed */
			abrake_ptr->anim_pos = (sar_grad_anim_t)-1;
			abrake_ptr->flags |= SAR_OBJ_PART_FLAG_STATE;
		    }
		    else
		    {
			/* Air brake initially retracted */
			abrake_ptr->anim_pos = 0;
			abrake_ptr->flags &= ~SAR_OBJ_PART_FLAG_STATE;
		    }

		    /* Animation rate */
		    abrake_ptr->anim_rate = (sar_grad_anim_t)anim_rate;

		    /* Air brake visible when retracted? */
		    if(!visible_when_retracted)
			abrake_ptr->flags |= SAR_OBJ_PART_FLAG_HIDE_MIN;

		    abrake_ptr->temperature = obj_ptr->temperature * 0.5f;
		}
		else
		{
		    fprintf(stderr,
 "%s: Line %i: Error:\
 Unable to create air brake (check object type).\n",
			filename, line_num
		    );
		}
	    }
	    /* New Landing Gear */
	    else if(!strcasecmp(parm, "landing_gear_new"))
	    {
		/* Arguments:
		 *
		 * <x> <y> <z>
		 * <heading> <pitch> <bank>
		 * <anim_rate> <up?> <fixed?> <skis?> <floats?>
		 */
		sar_position_struct	pos;
		sar_direction_struct	dir;
		int	anim_rate = 0;
		Boolean	up = False,
			fixed = False,
			skis = False,
			floats = False;

		arg = GET_ARG_POS(arg, &pos);
		arg = GET_ARG_DIR(arg, &dir);
		arg = GET_ARG_I(arg, &anim_rate);
		arg = GET_ARG_B(arg, &up);
		arg = GET_ARG_B(arg, &fixed);
		arg = GET_ARG_B(arg, &skis);
		arg = GET_ARG_B(arg, &floats);

		lgear_ptr = NULL;
		if(aircraft != NULL)
		    lgear_ptr = SARObjLandingGearNew(
			scene,
			&aircraft->part,
			&aircraft->total_parts
		    );
		if(lgear_ptr != NULL)
		{
		    lgear_ptr->flags = 0;	/* Must reset flags */

		    memcpy(&lgear_ptr->pos_min, &pos, sizeof(sar_position_struct));
		    memcpy(&lgear_ptr->pos_cen, &pos, sizeof(sar_position_struct));
		    memcpy(&lgear_ptr->pos_max, &pos, sizeof(sar_position_struct));

		    memset(&lgear_ptr->dir_min, 0x00, sizeof(sar_direction_struct));
		    memset(&lgear_ptr->dir_cen, 0x00, sizeof(sar_direction_struct));
		    memcpy(&lgear_ptr->dir_max, &dir, sizeof(sar_direction_struct));

		    lgear_ptr->anim_rate = (sar_grad_anim_t)anim_rate;

		    /* Landing gear up or down? */
		    if(up)
		    {
			/* Landing gear initially up */
			lgear_ptr->anim_pos = (sar_grad_anim_t)-1;
			lgear_ptr->flags &= ~SAR_OBJ_PART_FLAG_STATE;
		    }
		    else
		    {
			/* Landing gear initially down */
			lgear_ptr->anim_pos = 0;
			lgear_ptr->flags |= SAR_OBJ_PART_FLAG_STATE;
		    }

		    /* Landing gear fixed? */
		    if(fixed)
		    {
			/* Fixed */
			lgear_ptr->anim_pos = 0;
			lgear_ptr->flags |= SAR_OBJ_PART_FLAG_STATE;
			lgear_ptr->flags |= SAR_OBJ_PART_FLAG_LGEAR_FIXED;
		    }
		    else
		    {
			/* Retractable */
			lgear_ptr->flags &= ~SAR_OBJ_PART_FLAG_LGEAR_FIXED;
		    }  

		    /* Skis? */
		    if(skis)
			lgear_ptr->flags |= SAR_OBJ_PART_FLAG_LGEAR_SKI;
		    else
			lgear_ptr->flags &= ~SAR_OBJ_PART_FLAG_LGEAR_SKI;

		    /* Floats? */
		    if(floats)
			lgear_ptr->flags |= SAR_OBJ_PART_FLAG_LGEAR_FLOATS;
		    else
			lgear_ptr->flags &= ~SAR_OBJ_PART_FLAG_LGEAR_FLOATS;

		    /* Landing gears are always hidden when retracted
		     * Maximum position is considered retracted
		     */
		    lgear_ptr->flags |= SAR_OBJ_PART_FLAG_HIDE_MAX;

		    /* Need to update landing gear state on aircraft */
		    if(lgear_ptr->flags & SAR_OBJ_PART_FLAG_LGEAR_FIXED)
			aircraft->landing_gear_state = 2;
		    else
			aircraft->landing_gear_state = 1;
		    if(!(lgear_ptr->flags & SAR_OBJ_PART_FLAG_LGEAR_SKI))
			aircraft->wheel_brakes_state = 0;

		    lgear_ptr->temperature = obj_ptr->temperature * 0.5f;
		}
		else
		{
		    fprintf(
			stderr,
 "%s: Line %i: Error:\
 Unable to create landing gear (check object type).\n",
			filename, line_num
		    );
		}
	    }
	    /* Aileron left, aileron right, rudder top, rudder bottom,
	     * elevator, cannard, aileron/elevator left, or
	     * aileron/elevator right
	     */
	    else if(!strcasecmp(parm, "aileron_left_new") ||
	            !strcasecmp(parm, "aileron_right_new") ||
		    !strcasecmp(parm, "rudder_top_new") ||
		    !strcasecmp(parm, "rudder_bottom_new") ||
		    !strcasecmp(parm, "elevator_new") ||
		    !strcasecmp(parm, "cannard_new") ||
		    !strcasecmp(parm, "aileron_elevator_left_new") ||
		    !strcasecmp(parm, "aileron_elevator_right_new") ||
		    !strcasecmp(parm, "flap_new")
	    )
	    {
		sar_obj_part_type part_type = -1;
		sar_obj_part_struct **part_ptr = NULL;

		/* Arguments:
		 *
		 * <x> <y> <z>
		 * <heading> <pitch> <bank>
		 * <t_min> <t_max>
		 */
		sar_position_struct	pos;
		sar_direction_struct	dir;
		float	t_min = 0.0f,
			t_max = 0.0f;

		arg = GET_ARG_POS(arg, &pos);
		arg = GET_ARG_DIR(arg, &dir);
		arg = GET_ARG_F(arg, &t_min);
		arg = GET_ARG_F(arg, &t_max);

		/* Determine the part type and get part pointer */
		if(!strcasecmp(parm, "aileron_left_new"))
		{
		    part_type = SAR_OBJ_PART_TYPE_AILERON_LEFT;
		    part_ptr = &aileron_left_ptr;
		}
		else if(!strcasecmp(parm, "aileron_right_new"))
		{
		    part_type = SAR_OBJ_PART_TYPE_AILERON_RIGHT;
		    part_ptr = &aileron_right_ptr;
		}
		else if(!strcasecmp(parm, "rudder_top_new"))
		{
		    part_type = SAR_OBJ_PART_TYPE_RUDDER_TOP;
		    part_ptr = &rudder_top_ptr;
		}
		else if(!strcasecmp(parm, "rudder_bottom_new"))
		{
		    part_type = SAR_OBJ_PART_TYPE_RUDDER_BOTTOM;
		    part_ptr = &rudder_bottom_ptr;
		}
		else if(!strcasecmp(parm, "elevator_new"))
		{
		    part_type = SAR_OBJ_PART_TYPE_ELEVATOR;
		    part_ptr = &elevator_ptr;
		}
		else if(!strcasecmp(parm, "cannard_new"))
		{
		    part_type = SAR_OBJ_PART_TYPE_CANNARD;
		    part_ptr = &cannard_ptr;
		}
		else if(!strcasecmp(parm, "aileron_elevator_left_new"))
		{
		    part_type = SAR_OBJ_PART_TYPE_AILERON_ELEVATOR_LEFT;
		    part_ptr = &aileron_elevator_left_ptr;
		}
		else if(!strcasecmp(parm, "aileron_elevator_right_new"))
		{
		    part_type = SAR_OBJ_PART_TYPE_AILERON_ELEVATOR_RIGHT;
		    part_ptr = &aileron_elevator_right_ptr;
		}
		else if(!strcasecmp(parm, "flap_new"))
		{
		    part_type = SAR_OBJ_PART_TYPE_FLAP;
		    part_ptr = &flap_ptr;
		}

		/* Got valid part type? */
		if(part_ptr != NULL)
		{
		    /* Create new part */
		    if(aircraft != NULL)
			*part_ptr = SARObjPartNew(
			    scene,
			    &aircraft->part,
			    &aircraft->total_parts,
			    part_type
			);
		    if(*part_ptr != NULL)
		    {
			sar_obj_part_struct	*part = *part_ptr;
			sar_direction_struct	*dir_min = &part->dir_min,
						*dir_cen = &part->dir_cen,
						*dir_max = &part->dir_max;

			/* Minimum position */
			memset(&part->pos_min, 0x00, sizeof(sar_position_struct));

			/* Center position */
			memcpy(&part->pos_cen, &pos, sizeof(sar_position_struct));

			/* Maximum position */
			memset(&part->pos_max, 0x00, sizeof(sar_position_struct));


			/* Minimum direction */
			memset(dir_min, 0x00, sizeof(sar_direction_struct));

			/* Center direction */
			memcpy(dir_cen, &dir, sizeof(sar_direction_struct));

			/* Maximum direction */
			memset(dir_max, 0x00, sizeof(sar_direction_struct));

			/* Set values specific to the part's type */
			switch(part_type)
			{
			  case SAR_OBJ_PART_TYPE_AILERON_LEFT:
			  case SAR_OBJ_PART_TYPE_AILERON_RIGHT:
			    dir_min->pitch += (float)DEGTORAD(t_min);
			    dir_max->pitch += (float)DEGTORAD(t_max);
			    break;
			  case SAR_OBJ_PART_TYPE_RUDDER_TOP:
			  case SAR_OBJ_PART_TYPE_RUDDER_BOTTOM:
			    dir_min->heading += (float)DEGTORAD(t_min);
			    dir_max->heading += (float)DEGTORAD(t_max);
			    break;
			  case SAR_OBJ_PART_TYPE_ELEVATOR:
			  case SAR_OBJ_PART_TYPE_CANNARD:
			    dir_min->pitch += (float)DEGTORAD(t_min);
			    dir_max->pitch += (float)DEGTORAD(t_max);
			    break;
			  case SAR_OBJ_PART_TYPE_AILERON_ELEVATOR_LEFT:
			  case SAR_OBJ_PART_TYPE_AILERON_ELEVATOR_RIGHT:
			    dir_min->pitch += (float)DEGTORAD(t_min);
			    dir_max->pitch += (float)DEGTORAD(t_max);
			    break;
			  case SAR_OBJ_PART_TYPE_FLAP:
			    dir_min->pitch += (float)DEGTORAD(t_min);
			    dir_max->pitch += (float)DEGTORAD(t_max);
			    break;
			  case SAR_OBJ_PART_TYPE_AIR_BRAKE:
			    break;
			  case SAR_OBJ_PART_TYPE_DOOR:
			    break;
			  case SAR_OBJ_PART_TYPE_DOOR_RESCUE:
			    break;
			  case SAR_OBJ_PART_TYPE_CANOPY:
			    break;
			  case SAR_OBJ_PART_TYPE_LANDING_GEAR:
			    break;
			}

			part->temperature = obj_ptr->temperature * 0.5f;

		    }
		    else
		    {
			fprintf(
			    stderr,
 "%s: Line %i: Error:\
 Unable to create part type %i (check object type).\n",
			    filename, line_num,
			    (int)part_type
			);
		    }
		}
	    }

	    /* New External Fuel Tank */
	    else if(!strcasecmp(parm, "fueltank_new") ||
		    !strcasecmp(parm, "fuel_tank_new")
	    )
	    {
		/* Arguments:
		 *
		 * <x> <y> <z>
		 * <radius>
		 * <dry_mass> <fuel> <fuel_max>
		 * <droppable?>
		 */
		sar_position_struct pos;
		float	radius = 0.0f,
			belly_to_center_height = 0.0f,
			dry_mass = 0.0f,
			fuel = 0.0f,
			fuel_max = 0.0f;
		Boolean	droppable = False;

		arg = GET_ARG_POS(arg, &pos);
		arg = GET_ARG_F(arg, &radius);
		arg = GET_ARG_F(arg, &belly_to_center_height);
		arg = GET_ARG_F(arg, &dry_mass);
		arg = GET_ARG_F(arg, &fuel);
		arg = GET_ARG_F(arg, &fuel_max);
		arg = GET_ARG_B(arg, &droppable);

		if(radius < 0.0f)
		    fprintf(stderr,
 "%s: Line %i: Warning:\
 Value for %s argument radius=%f should not be negative.\n",
			filename, line_num,
			parm, radius
		    );
		if(dry_mass < 0.0f)
		    fprintf(stderr,
 "%s: Line %i: Warning:\
 Value for %s argument dry_mass=%f should not be negative.\n",
			filename, line_num,
			parm, dry_mass
		    );
		if(fuel < 0.0f)
		    fprintf(stderr,
 "%s: Line %i: Warning:\
 Value for %s argument fuel=%f should not be negative.\n",
			filename, line_num,
			parm, fuel
		    );
		if(fuel_max < 0.0f)
		    fprintf(stderr,
 "%s: Line %i: Warning:\
 Value for %s argument fuel_max=%f should not be negative.\n",
			filename, line_num,
			parm, fuel_max
		    );
		if(fuel > fuel_max)
		    fprintf(stderr,
 "%s: Line %i: Warning:\
 Value for %s argument fuel=%f is greater than fuel_max=%f.\n",
			filename, line_num,
			parm, fuel, fuel_max
		    );

		eft_ptr = NULL;
		if(aircraft != NULL)
		{
		    int n = SARObjExternalFuelTankNew(
			scene,
			&aircraft->external_fueltank,
			&aircraft->total_external_fueltanks
		    );
		    eft_ptr = (n > -1) ?
			aircraft->external_fueltank[n] : NULL;
		}
		if(eft_ptr != NULL)
		{
		    /* Position */
		    memcpy(&eft_ptr->offset_pos, &pos, sizeof(sar_position_struct));

		    /* Size */
		    eft_ptr->radius = (float)MAX(radius, 0.0f);

		    /* Belly to center height */
		    eft_ptr->belly_to_center_height = belly_to_center_height;

		    /* Mass, fuel, and fuel max */
		    eft_ptr->dry_mass = (float)MAX(dry_mass, 0.0f);
		    eft_ptr->fuel_max = (float)MAX(fuel_max, 0.0f);
		    eft_ptr->fuel = (float)CLIP(fuel, 0.0f, eft_ptr->fuel_max);

		    /* Dropable? */
		    if(droppable)
			eft_ptr->flags |= SAR_EXTERNAL_FUELTANK_FLAG_FIXED;
		    else
			eft_ptr->flags &= ~SAR_EXTERNAL_FUELTANK_FLAG_FIXED;

		    /* Mark as initially on board */
		    eft_ptr->flags |= SAR_EXTERNAL_FUELTANK_FLAG_ONBOARD;

		    eft_ptr->temperature = obj_ptr->temperature * 0.5f;
		}
		else
		{
		    fprintf(
			stderr,
 "%s: Line %i: Error:\
 Unable to create fuel tank (check object type).\n",
			filename, line_num
		    );
		}
	    }
	    /* Rescue Door */
	    else if(!strcasecmp(parm, "rescue_door_new"))
	    {
		/* Arguments:
		 *
		 * <x_closed> <y_closed> <z_closed>
		 * <x_opened> <y_opened> <z_opened>
		 * <h_opened> <p_opened> <b_opened>
		 * <x_thres> <y_thres> <z_thres>
		 * <anim_rate> <opened?>
		 */
		sar_position_struct	pos_closed,
					pos_opened;
		sar_direction_struct	dir_opened;
		sar_position_struct	pos_thres;
		int	anim_rate = 0;
		Boolean	opened = False;

		arg = GET_ARG_POS(arg, &pos_closed);
		arg = GET_ARG_POS(arg, &pos_opened);
		arg = GET_ARG_DIR(arg, &dir_opened);
		arg = GET_ARG_POS(arg, &pos_thres);
		arg = GET_ARG_I(arg, &anim_rate);
		arg = GET_ARG_B(arg, &opened);

		door_ptr = NULL;
		if(aircraft != NULL)
		    door_ptr = SARObjDoorRescueNew(
			scene,
			&aircraft->part,
			&aircraft->total_parts
		    );
		if(door_ptr != NULL)
		{
		    door_ptr->flags = 0;	/* Must reset flags */

		    /* Closed position */
		    memcpy(&door_ptr->pos_min, &pos_closed, sizeof(sar_position_struct));

		    /* Opened position */
		    memcpy(&door_ptr->pos_max, &pos_opened, sizeof(sar_position_struct));

		    /* Opened direction */
		    memcpy(&door_ptr->dir_max, &dir_opened, sizeof(sar_direction_struct));

		    /* Threshold position */
		    memcpy(&door_ptr->pos_cen, &pos_thres, sizeof(sar_position_struct));

		    /* Animation rate */
		    door_ptr->anim_rate = (sar_grad_anim_t)anim_rate;

		    /* Initially opened? */
		    if(opened)
		    {
			/* Door initially open */
			door_ptr->anim_pos = (sar_grad_anim_t)-1;
			door_ptr->flags |= SAR_OBJ_PART_FLAG_STATE;
			door_ptr->flags |= SAR_OBJ_PART_FLAG_DOOR_STAY_OPEN;
		    }
		    else
		    {
			/* Door initially closed */
			door_ptr->anim_pos = 0;
			door_ptr->flags &= ~SAR_OBJ_PART_FLAG_STATE;
		    }

		    door_ptr->temperature = obj_ptr->temperature;
		}
		else
		{
		    fprintf(stderr,
 "%s: Line %i: Error:\
 Unable to create rescue door (check object type).\n",
			filename, line_num
		    );
		}
	    }
	    /* Hoist */
	    else if(!strcasecmp(parm, "hoist"))
	    {
		/* Arguments:
		 *
		 * <x> <y> <z>
		 * <rope_max> <rope_rate> <capacity>
		 * <radius> <z_min> <z_max>
		 * <basket?> <diver?> <hook?>
		 */
		sar_obj_hoist_struct *hoist;
		sar_position_struct	pos;
		float	rope_max = 0.0f,
			rope_rate = 0.0f,
			capacity = 0.0f,
			radius = 0.0f,
			z_min = 0.0f,
			z_max = 0.0f;
		Boolean	basket = False,
			diver = False,
			hook = False;

		arg = GET_ARG_POS(arg, &pos);
		arg = GET_ARG_F(arg, &rope_max);
		arg = GET_ARG_F(arg, &rope_rate);
		arg = GET_ARG_F(arg, &capacity);
		arg = GET_ARG_F(arg, &radius);
		arg = GET_ARG_F(arg, &z_min);
		arg = GET_ARG_F(arg, &z_max);
		arg = GET_ARG_B(arg, &basket);
		arg = GET_ARG_B(arg, &diver);
		arg = GET_ARG_B(arg, &hook);

		if(rope_max < 0.0f)
		    fprintf(stderr,
 "%s: Line %i: Warning:\
 Value for %s argument rope_max=%f should not be negative.\n",
			filename, line_num,
			parm, rope_max
		    );
		if(rope_rate < 0.0f)
		    fprintf(stderr,
 "%s: Line %i: Warning:\
 Value for %s argument rope_rate=%f should not be negative.\n",
			filename, line_num,
			parm, rope_rate
		    );
		if(capacity < 0.0f)
		    fprintf(stderr,
 "%s: Line %i: Warning:\
 Value for %s argument capacity=%f should not be negative.\n",
			filename, line_num,
			parm, capacity
		    );
		if(radius < 0.0f)
		    fprintf(stderr,
 "%s: Line %i: Warning:\
 Value for %s argument radius=%f should not be negative.\n",
			filename, line_num,
			parm, radius
		    );
		if(z_min > z_max)
		    fprintf(stderr,
 "%s: Line %i: Warning:\
 Value for %s argument z_min=%f is greater than z_max=%f.\n",
			filename, line_num,
			parm, z_min, z_max
		    );

		/* Create hoist as needed */
		if(aircraft != NULL)
		{
		    if(aircraft->hoist == NULL)
			aircraft->hoist = SAR_OBJ_HOIST(calloc(
			    1, sizeof(sar_obj_hoist_struct)
			));
		}
		hoist = SARObjGetHoistPtr(obj_ptr, 0, NULL);
		if(hoist != NULL)
		{
		    const sar_human_data_entry_struct *human_data;

		    /* Position */
		    memcpy(&hoist->offset, &pos, sizeof(sar_position_struct));

		    /* Rope length and movement rate */
		    hoist->rope_max = rope_max;
		    hoist->rope_rate = (float)(rope_rate * SAR_SEC_TO_CYCLE_COEFF);

		    /* Capacity, in kg */
		    hoist->capacity = capacity;

		    /* Cylendrical contact size */
		    hoist->contact_radius = radius;
		    hoist->contact_z_min = z_min;
		    hoist->contact_z_max = z_max;

		    /* Deployment type */
		    hoist->deployments = (basket ? SAR_HOIST_DEPLOYMENT_BASKET : 0) |
			(diver ? SAR_HOIST_DEPLOYMENT_DIVER : 0) |
			(hook ? SAR_HOIST_DEPLOYMENT_HOOK : 0);
		    /* Set initial deployment */
		    if(hoist->deployments & SAR_HOIST_DEPLOYMENT_BASKET)
			hoist->cur_deployment = SAR_HOIST_DEPLOYMENT_BASKET;
		    else if(hoist->deployments & SAR_HOIST_DEPLOYMENT_DIVER)
			hoist->cur_deployment = SAR_HOIST_DEPLOYMENT_DIVER;
		    else if(hoist->deployments & SAR_HOIST_DEPLOYMENT_HOOK)
			hoist->cur_deployment = SAR_HOIST_DEPLOYMENT_HOOK;

		    /* Get hoist texture reference numbers from scene */
		    hoist->side_tex_num =
			SARGetTextureRefNumberByName(
			    scene, SAR_STD_TEXNAME_BASKET_SIDE
			);
		    hoist->end_tex_num =
			SARGetTextureRefNumberByName(
			    scene, SAR_STD_TEXNAME_BASKET_END
			);
		    hoist->bottom_tex_num =
			SARGetTextureRefNumberByName(
			    scene, SAR_STD_TEXNAME_BASKET_BOTTOM
			);
		    hoist->water_ripple_tex_num =
			SARGetTextureRefNumberByName(
			    scene, SAR_STD_TEXNAME_WATER_RIPPLE
			);

		    /* Diver color */
		    human_data = SARHumanMatchEntryByName(
			core_ptr->human_data, SAR_HUMAN_PRESET_NAME_DIVER
		    );
		    if(human_data == NULL)
			human_data = SARHumanMatchEntryByName(
			    core_ptr->human_data, SAR_HUMAN_PRESET_NAME_STANDARD
			);
		    if(human_data != NULL)
			memcpy(
			    hoist->diver_color,
			    human_data->color,
			    SAR_HUMAN_COLORS_MAX * sizeof(sar_color_struct)
			);

		    /* Animation position */
		    hoist->anim_pos = 0;
		    hoist->anim_rate = SAR_HUMAN_ANIM_RATE;
/* TODO (using numan animation rate for now) */
		}
		else
		{
		    fprintf(stderr,
 "%s: Line %i: Error:\
 Unable to create hoist (check object type).\n",
			filename, line_num
		    );
		}
	    }
	    /* Ground Elevation */
	    else if(!strcasecmp(parm, "ground_elevation"))
	    {
		/* Arguments:
		 *
		 * <altitude>
		 */
		float	altitude = 0.0f;

		arg = GET_ARG_F(arg, &altitude);

		if(altitude < 0.0f)
		    fprintf(
			stderr,
 "%s: Line %i: Warning:\
 Value for %s argument altitude=%f should not be negative.\n",
			filename, line_num,
			parm, altitude
		    );

		if(ground != NULL)
		{
		    ground->elevation =
			(float)SFMFeetToMeters(altitude);
		}
		else
		{
		    fprintf(
			stderr,
 "%s: Line %i: Warning:\
 Unable to set %s for object type %i (must be type %i).\n",
			filename, line_num,
			parm, obj_ptr->type, SAR_OBJ_TYPE_GROUND
		    );
		}
	    }

	    /* Unsupported Parameter */
	    else
	    {
		fprintf(
		    stderr,
 "%s: Line %i: Warning:\
 Unsupported parameter \"%s\".\n",
		    filename, line_num,
		    parm
		);
	    }
	}
}

/*
 *	Loads a V3D model file specified by filename and applies the
 *	loaded data to the object specified by obj_num.
 */
int SARObjLoadFromFile(
	sar_core_struct *core_ptr, int obj_num, const char *filename
)
{
	int status, line_num, *total;
	char *full_path;
	const char *v3d_model_name;
	FILE *fp;

	void **v3d_h = NULL;
	void *h;
	int hn, htype, total_v3d_h = 0;

	v3d_model_struct *v3d_model_ptr, **v3d_model = NULL;
	int v3d_model_num, total_v3d_models = 0;

	gw_display_struct *display = core_ptr->display;
	sar_scene_struct *scene = core_ptr->scene;
	sar_object_struct ***ptr, *obj_ptr;
	sar_object_aircraft_struct *aircraft = NULL;
	sar_object_ground_struct *ground = NULL;

	int total_rotors = 0;
	sar_obj_rotor_struct *rotor = NULL;

	int total_aileron_lefts = 0;
	sar_obj_part_struct *aileron_left_ptr = NULL;
	int total_aileron_rights = 0;
	sar_obj_part_struct *aileron_right_ptr = NULL;
	int total_rudder_tops = 0;
	sar_obj_part_struct *rudder_top_ptr = NULL;
	int total_rudder_bottoms = 0;
	sar_obj_part_struct *rudder_bottom_ptr = NULL;
	int total_elevators = 0;
	sar_obj_part_struct *elevator_ptr = NULL;
	int total_cannards = 0;
	sar_obj_part_struct *cannard_ptr = NULL;
	int total_aileron_elevator_lefts = 0;
	sar_obj_part_struct *aileron_elevator_left_ptr = NULL;
	int total_aileron_elevator_rights = 0;
	sar_obj_part_struct *aileron_elevator_right_ptr = NULL;
	int total_flaps = 0;
	sar_obj_part_struct *flap_ptr = NULL;
	int total_abrakes = 0;
	sar_obj_part_struct *abrake_ptr = NULL;
	int total_lgears = 0;
	sar_obj_part_struct *lgear_ptr = NULL;
	int total_doors = 0;
	sar_obj_part_struct *door_ptr = NULL;
	int total_external_fueltanks = 0;
	sar_external_fueltank_struct *eft_ptr = NULL;

	sar_direction_struct *dir;
	sar_position_struct *pos;
	char *sar_vmodel_name;
	sar_visual_model_struct **sar_vmodel, **sar_vmodel_ir;

	struct stat stat_buf;


	if(STRISEMPTY(filename) || (display == NULL) || (scene == NULL))
	    return(-1);

	ptr = &core_ptr->object;
	total = &core_ptr->total_objects;

	/* Get pointer to object */
	obj_ptr = (SARObjIsAllocated(*ptr, *total, obj_num)) ?
	    (*ptr)[obj_num] : NULL;
	if(obj_ptr == NULL)
	    return(-1);

	pos = &obj_ptr->pos;
	dir = &obj_ptr->dir;

	/* Get the full path to the object file */
	full_path = COMPLETE_PATH(filename);
	if(full_path == NULL)
	{
	    fprintf(
		stderr,
		"%s: Unable to complete path.\n",
		filename
	    );
	    return(-2);
	}

	/* Set filename as the full path */
	filename = full_path;

	/* Check if file exists and get file stats */
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

	    free(full_path);
	    return(-1);
	}
#ifdef S_ISDIR
	/* File is a directory? */
	if(S_ISDIR(stat_buf.st_mode))
	{
	    fprintf(
		stderr,
		"%s: Is a directory.\n",
		filename
	    );

	    free(full_path);
	    return(-2);
	}
#endif	/* S_ISDIR */

	/* Open the V3D model file for reading */
	fp = FOpen(filename, "rb");
	if(fp == NULL)
	{
	    fprintf(
		stderr,
		"%s: Unable to open the V3D Model file for reading.\n",
		filename
	    );

	    free(full_path);
	    return(-1);
	}


	/* Do preload procedure */

	/* Reset object values */
	if(True)
	{
	    /* Flags */
	    obj_ptr->flags |=	SAR_OBJ_FLAG_HIDE_DAWN_MODEL |
				SAR_OBJ_FLAG_HIDE_DUSK_MODEL |
				SAR_OBJ_FLAG_HIDE_NIGHT_MODEL;


	    /* Begin resetting object type specific values */

	    /* Aircraft */
	    aircraft = SAR_OBJ_GET_AIRCRAFT(obj_ptr);
	    if(aircraft != NULL)
	    {
		sar_direction_struct *dir;

		aircraft->z_accel = 0.0f;

		aircraft->air_worthy_state = SAR_AIR_WORTHY_FLYABLE;
		aircraft->engine_state = SAR_ENGINE_OFF;
		aircraft->next_engine_on = 0;

		aircraft->ground_pitch_offset = (float)(0.0 * PI);

		dir = &aircraft->spotlight_dir;
		dir->heading = (float)SFMDegreesToRadians(
		    SAR_DEF_SPOTLIGHT_HEADING
		);
		dir->pitch = (float)SFMDegreesToRadians(
		    SAR_DEF_SPOTLIGHT_PITCH
		);
		dir->bank = (float)SFMDegreesToRadians(
		    SAR_DEF_SPOTLIGHT_BANK
		);

		aircraft->landed = 1;

		aircraft->landing_gear_state = -1;
		aircraft->air_brakes_state = -1;
		aircraft->wheel_brakes_state = -1;
		aircraft->wheel_brakes_coeff = 0.0;

		obj_ptr->temperature = SAR_DEF_TEMPERATURE_AIRCRAFT;
	    }
	    /* Ground */
	    ground = SAR_OBJ_GET_GROUND(obj_ptr);
	    if(ground != NULL)
	    {


	    }
	}

	/* Begin reading V3D model file */
	status = V3DLoadModel(
	    NULL, fp,
	    &v3d_h, &total_v3d_h,
	    &v3d_model, &total_v3d_models,
	    NULL, NULL
	);

	/* Close the V3D model file, it is no longer needed */
	FClose(fp);
	fp = NULL;

	/* Error loading V3D model file? */
	if(status)
	{
	    /* Error encountered while loading the visual model file,
	     * delete V3D header items, model primitives, and return
	     * indicating error
	     */
	    V3DMHListDeleteAll(&v3d_h, &total_v3d_h);
	    V3DModelListDeleteAll(&v3d_model, &total_v3d_models);
	    free(full_path);
	    return(-1);
	}


	/* Iterate through V3D header items */
	for(hn = 0; hn < total_v3d_h; hn++)
	{
	    h = v3d_h[hn];
	    if(h == NULL)
		continue;

	    /* Handle by header item type */
	    htype = V3DMHGetType(h);
	    switch(htype)
	    {
		const mh_comment_struct *mh_comment;
		const mh_texture_base_directory_struct *mh_tbd;
		const mh_texture_load_struct *mh_texture_load;
		const mh_color_specification_struct *mh_color_spec;
		sar_parm_texture_load_struct *p_texture_load;

	      case V3DMH_TYPE_COMMENT:
		mh_comment = (mh_comment_struct *)h;
		/* Ignore comments */
		break;

	      case V3DMH_TYPE_TEXTURE_BASE_DIRECTORY:
		mh_tbd = (mh_texture_base_directory_struct *)h;
		/* Ignore */
		break;

	      case V3DMH_TYPE_TEXTURE_LOAD:
		mh_texture_load = (mh_texture_load_struct *)h;
		p_texture_load = (sar_parm_texture_load_struct *)SARParmNew(
		    SAR_PARM_TEXTURE_LOAD
		);
		if(p_texture_load != NULL)
		{
		    /* Copy data from model header item to sar parm */
		    free(p_texture_load->name);
		    p_texture_load->name = STRDUP(mh_texture_load->name);
		    free(p_texture_load->file);
		    p_texture_load->file = STRDUP(mh_texture_load->path);
		    p_texture_load->priority = (float)mh_texture_load->priority;
		    /* Load texture using the sar parm */
		    SARObjLoadTexture(
			core_ptr, scene,
			p_texture_load
		    );
		    /* Deallocate sar parm for loading texture, not needed
		     * anymore.
		     */
		    SARParmDelete(p_texture_load);
		}
		break;

	      case V3DMH_TYPE_COLOR_SPECIFICATION:
		mh_color_spec = (mh_color_specification_struct *)h;
		/* Ignore */
		break;
	    }
	}


	/* Iterate through V3D models */
	for(v3d_model_num = 0; v3d_model_num < total_v3d_models; v3d_model_num++)
	{
	    v3d_model_ptr = v3d_model[v3d_model_num];
	    if(v3d_model_ptr == NULL)
		continue;

	    /* Get pointers to the object's parts for use with this
	     * V3D model, the pointers must be obtained for each
	     * V3D model since each V3D model may create a new object
	     * part
	     */
	    if(aircraft != NULL)
	    {
		/* Rotors */
		total_rotors = aircraft->total_rotors;
		rotor = ((total_rotors > 0) ?
		    aircraft->rotor[total_rotors - 1] : NULL
		);

		/* External fuel tanks */
		total_external_fueltanks = aircraft->total_external_fueltanks;
		eft_ptr = ((total_external_fueltanks > 0) ?
		    aircraft->external_fueltank[total_external_fueltanks - 1]
		    : NULL
		);
	    }
/* Add fetching of substructure pointers support for other object
 * types here
 */
	    else
	    {
		/* Rotors */
		total_rotors = 0;
		rotor = NULL;

		/* External fuel tanks */
		total_external_fueltanks = 0;
		eft_ptr = NULL;
	    }

	    /* Get pointers to the object's parts */
	    if(True)
	    {
		int i;
		sar_obj_part_struct *part;

#define GET_PART(_type_) {			\
 sar_obj_part_struct *p;			\
 i = 0;						\
 part = NULL;					\
 while(True) {					\
  p = SARObjGetPartPtr(obj_ptr, (_type_), i);	\
  if(p != NULL) {				\
   part = p; i++;				\
  } else {					\
   break; 					\
  }						\
 }						\
}
		GET_PART(SAR_OBJ_PART_TYPE_AILERON_LEFT);
		total_aileron_lefts = i;
		aileron_left_ptr = part;

		GET_PART(SAR_OBJ_PART_TYPE_AILERON_RIGHT);
		total_aileron_rights = i;
		aileron_right_ptr = part;

		GET_PART(SAR_OBJ_PART_TYPE_RUDDER_TOP);
		total_rudder_tops = i;
		rudder_top_ptr = part;

		GET_PART(SAR_OBJ_PART_TYPE_RUDDER_BOTTOM);
		total_rudder_bottoms = i;
		rudder_bottom_ptr = part;

		GET_PART(SAR_OBJ_PART_TYPE_ELEVATOR);
		total_elevators = i;
		elevator_ptr = part;

		GET_PART(SAR_OBJ_PART_TYPE_CANNARD);
		total_cannards = i;
		cannard_ptr = part;

		GET_PART(SAR_OBJ_PART_TYPE_AILERON_ELEVATOR_LEFT);
		total_aileron_elevator_lefts = i;
		aileron_elevator_left_ptr = part;

		GET_PART(SAR_OBJ_PART_TYPE_AILERON_ELEVATOR_RIGHT);
		total_aileron_elevator_rights = i;
		aileron_elevator_right_ptr = part;

		GET_PART(SAR_OBJ_PART_TYPE_FLAP);
		total_flaps = i;
		flap_ptr = part;

		GET_PART(SAR_OBJ_PART_TYPE_AIR_BRAKE);
		total_abrakes = i;
		abrake_ptr = part;

		GET_PART(SAR_OBJ_PART_TYPE_LANDING_GEAR);
		total_lgears = i;
		lgear_ptr = part;

		GET_PART(SAR_OBJ_PART_TYPE_DOOR_RESCUE);
		total_doors = i;
		door_ptr = part;
#undef GET_PART
	    }


	    /* Iterate through each "other data" line on this V3D
	     * model, these "other data" lines will specify values that
	     * are specific to SAR objects
	     */
 	    for(line_num = 0; line_num < v3d_model_ptr->total_other_data_lines; line_num++)
	    {
		const char *s = v3d_model_ptr->other_data_line[line_num];
		if(s == NULL)
		    continue;

		SARObjLoadLine(
		    core_ptr, obj_num, obj_ptr,
		    s, filename, line_num
		);
	    }

	    /* Do not continue the handling of this V3D model if it
	     * does not contain primitives (if this V3D model is not of
	     * type V3D_MODEL_TYPE_STANDARD)
	     */
	    if(v3d_model_ptr->type != V3D_MODEL_TYPE_STANDARD)
		continue;

	    /* Create model by the V3D model's name, the name must
	     * be defined or else we will not be able to know what
	     * this V3D model is suppose to represent
	     */
	    v3d_model_name = v3d_model_ptr->name;
	    if(STRISEMPTY(v3d_model_name))
		continue;

	    /* Reset the SAR visual model & name pointers */
	    sar_vmodel = NULL;
	    sar_vmodel_ir = NULL;
	    sar_vmodel_name = NULL;

	    /* Get the sar_vmodel & sar_vmodel_name by the V3D model's
	     * name
	     */
	    /* Standard */
	    if(!strcasecmp(v3d_model_name, "standard"))
	    {
		sar_vmodel = &obj_ptr->visual_model;
		sar_vmodel_ir = &obj_ptr->visual_model_ir;
		free(sar_vmodel_name);
		sar_vmodel_name = STRDUP("standard");
	    }
	    /* Standard Far */
	    else if(!strcasecmp(v3d_model_name, "standard_far"))
	    {
		sar_vmodel = &obj_ptr->visual_model_far;
		free(sar_vmodel_name);
		sar_vmodel_name = STRDUP("standard_far");
	    }
	    /* Standard Dusk */ 
	    else if(!strcasecmp(v3d_model_name, "standard_dusk"))
	    {
		sar_vmodel = &obj_ptr->visual_model_dusk;
		free(sar_vmodel_name);
		sar_vmodel_name = STRDUP("standard_dusk");
	    }
	    /* Standard Night */
	    else if(!strcasecmp(v3d_model_name, "standard_night"))
	    {
		sar_vmodel = &obj_ptr->visual_model_night;
		free(sar_vmodel_name);
		sar_vmodel_name = STRDUP("standard_night");
	    }
	    /* Standard Dawn */
	    else if(!strcasecmp(v3d_model_name, "standard_dawn"))
	    {
		sar_vmodel = &obj_ptr->visual_model_dawn;
		free(sar_vmodel_name);
		sar_vmodel_name = STRDUP("standard_dawn");
	    }
	    /* Rotor */
	    else if(!strcasecmp(v3d_model_name, "rotor") ||
		    !strcasecmp(v3d_model_name, "propellar")
	    )
	    {
		if(rotor != NULL)
		{
		    char s[80];
		    sprintf(s, "rotor%i", total_rotors - 1);

		    sar_vmodel = &rotor->visual_model;
		    sar_vmodel_ir = &rotor->visual_model_ir;
		    free(sar_vmodel_name);
		    sar_vmodel_name = STRDUP(s);
		}
	    }
	    /* Aileron Left */
	    else if(!strcasecmp(v3d_model_name, "aileron_left"))
	    {
		if(aileron_left_ptr != NULL)
		{
		    char s[80];
		    sprintf(s, "aileron_left%i", total_aileron_lefts - 1);

		    sar_vmodel = &aileron_left_ptr->visual_model;
		    sar_vmodel_ir = &aileron_left_ptr->visual_model_ir;
		    free(sar_vmodel_name);
		    sar_vmodel_name = STRDUP(s);
		}
	    }
	    /* Aileron Right */
	    else if(!strcasecmp(v3d_model_name, "aileron_right"))
	    {
		if(aileron_right_ptr != NULL)
		{
		    char s[80];
		    sprintf(s, "aileron_right%i", total_aileron_rights - 1);

		    sar_vmodel = &aileron_right_ptr->visual_model;
		    sar_vmodel_ir = &aileron_right_ptr->visual_model_ir;
		    free(sar_vmodel_name);
		    sar_vmodel_name = STRDUP(s);
		}
	    }
	    /* Rudder Top */
	    else if(!strcasecmp(v3d_model_name, "rudder_top"))
	    {
		if(rudder_top_ptr != NULL)
		{
		    char s[80];
		    sprintf(s, "rudder_top%i", total_rudder_tops - 1);

		    sar_vmodel = &rudder_top_ptr->visual_model;
		    sar_vmodel_ir = &rudder_top_ptr->visual_model_ir;
		    free(sar_vmodel_name);
		    sar_vmodel_name = STRDUP(s);
		}
	    }
	    /* Rudder Bottom */
	    else if(!strcasecmp(v3d_model_name, "rudder_bottom"))
	    {
		if(rudder_bottom_ptr != NULL)
		{
		    char s[80];
		    sprintf(s, "rudder_bottom%i", total_rudder_bottoms - 1);

		    sar_vmodel = &rudder_bottom_ptr->visual_model;
		    sar_vmodel_ir = &rudder_bottom_ptr->visual_model_ir;
		    free(sar_vmodel_name);
		    sar_vmodel_name = STRDUP(s);
		}
	    }
	    /* Elevator */
	    else if(!strcasecmp(v3d_model_name, "elevator"))
	    {
		if(elevator_ptr != NULL)
		{
		    char s[80];
		    sprintf(s, "elevator%i", total_elevators - 1);

		    sar_vmodel = &elevator_ptr->visual_model;
		    sar_vmodel_ir = &elevator_ptr->visual_model_ir;
		    free(sar_vmodel_name);
		    sar_vmodel_name = STRDUP(s);
		}
	    }
	    /* Cannard */
	    else if(!strcasecmp(v3d_model_name, "cannard"))
	    {
		if(cannard_ptr != NULL)
		{
		    char s[80];
		    sprintf(s, "cannard%i", total_cannards - 1);

		    sar_vmodel = &cannard_ptr->visual_model;
		    sar_vmodel_ir = &cannard_ptr->visual_model_ir;
		    free(sar_vmodel_name);
		    sar_vmodel_name = STRDUP(s);
		}
	    }
	    /* Aileron Elevator Left */
	    else if(!strcasecmp(v3d_model_name, "aileron_elevator_left"))
	    {
		if(aileron_elevator_left_ptr != NULL)
		{
		    char s[80];
		    sprintf(s, "aileron_elevator_left%i", total_aileron_elevator_lefts - 1);

		    sar_vmodel = &aileron_elevator_left_ptr->visual_model;
		    sar_vmodel_ir = &aileron_elevator_left_ptr->visual_model_ir;
		    free(sar_vmodel_name);
		    sar_vmodel_name = STRDUP(s);
		}
	    }
	    /* Aileron Elevator Right */
	    else if(!strcasecmp(v3d_model_name, "aileron_elevator_right"))
	    {
		if(aileron_elevator_right_ptr != NULL)
		{
		    char s[80];
		    sprintf(s, "aileron_elevator_right%i", total_aileron_elevator_rights - 1);

		    sar_vmodel = &aileron_elevator_right_ptr->visual_model;
		    sar_vmodel_ir = &aileron_elevator_right_ptr->visual_model_ir;
		    free(sar_vmodel_name);
		    sar_vmodel_name = STRDUP(s);
		}
	    }
	    /* Flap */
	    else if(!strcasecmp(v3d_model_name, "flap"))
	    {
		if(flap_ptr != NULL)
		{
		    char s[80];
		    sprintf(s, "flap%i", total_flaps - 1);

		    sar_vmodel = &flap_ptr->visual_model;
		    sar_vmodel_ir = &flap_ptr->visual_model_ir;
		    free(sar_vmodel_name);
		    sar_vmodel_name = STRDUP(s);
		}
	    }
	    /* Air Brake */
	    else if(!strcasecmp(v3d_model_name, "air_brake"))
	    {
		if(abrake_ptr != NULL)
		{
		    char s[80];
		    sprintf(s, "air_brake%i", total_abrakes - 1);

		    sar_vmodel = &abrake_ptr->visual_model;
		    sar_vmodel_ir = &abrake_ptr->visual_model_ir;
		    free(sar_vmodel_name);
		    sar_vmodel_name = STRDUP(s);
		}
	    }
	    /* Landing Gear */
	    else if(!strcasecmp(v3d_model_name, "landing_gear"))
	    {
		if(lgear_ptr != NULL)
		{
		    char s[80];
		    sprintf(s, "landing_gear%i", total_lgears - 1);

		    sar_vmodel = &lgear_ptr->visual_model;
		    sar_vmodel_ir = &lgear_ptr->visual_model_ir;
		    free(sar_vmodel_name);
		    sar_vmodel_name = STRDUP(s);
		}
	    }
	    /* Door */
	    else if(!strcasecmp(v3d_model_name, "door"))
	    {
		if(door_ptr != NULL)
		{
		    char s[80];
		    sprintf(s, "door%i", total_doors - 1);

		    sar_vmodel = &door_ptr->visual_model;
		    sar_vmodel_ir = &door_ptr->visual_model_ir;
		    free(sar_vmodel_name);
		    sar_vmodel_name = STRDUP(s);
		}
	    }
	    /* External Fuel Tank */
	    else if(!strcasecmp(v3d_model_name, "fueltank") ||
		    !strcasecmp(v3d_model_name, "fuel_tank")
	    )
	    {
		if(eft_ptr != NULL)
		{
		    char s[80];
		    sprintf(s, "fueltank%i", total_external_fueltanks - 1);

		    sar_vmodel = &eft_ptr->visual_model;
		    sar_vmodel_ir = &eft_ptr->visual_model_ir;
		    free(sar_vmodel_name);
		    sar_vmodel_name = STRDUP(s);
		}
	    }
	    /* Cockpit */
	    else if(!strcasecmp(v3d_model_name, "cockpit"))
	    {
		if(aircraft != NULL)
		{
		    sar_vmodel = &aircraft->visual_model_cockpit;
		    free(sar_vmodel_name);
		    sar_vmodel_name = STRDUP("cockpit");
		}
	    }
	    /* Shadow */
	    else if(!strcasecmp(v3d_model_name, "shadow"))
	    {
		sar_vmodel = &obj_ptr->visual_model_shadow;
		free(sar_vmodel_name);
		sar_vmodel_name = STRDUP("shadow");
	    }

	    /* If sar_vmodel is NULL then it means that the V3D model's
	     * name did not match a SAR visual model that is supported
	     */
	    if(sar_vmodel == NULL)
	    {
		/* The V3D model's name is not one that we support */
		fprintf(
		    stderr,
 "%s: Warning:\
 V3D model type \"%s\" is not supported on this object (check object type).\n",
		    filename, v3d_model_name
		);

		/* Delete SAR visual model name */
		free(sar_vmodel_name);
		sar_vmodel_name = NULL;

		continue;	/* Do not create this SAR Visual Model */
	    }

	    /* Is this SAR Visual Model already created? */
	    if(*sar_vmodel != NULL)
	    {
		/* Warn about redefining this SAR Visual Model and then
		 * unref it
		 */
		fprintf(
		    stderr,
 "%s: Warning:\
 V3D model type \"%s\" redefined.\n",
		    filename, v3d_model_name
		);
		SARVisualModelUnref(scene, *sar_vmodel);
		*sar_vmodel = NULL;
	    }
	    /* Is this IR SAR Visual Model already created? */
	    if((sar_vmodel_ir != NULL) ? (*sar_vmodel_ir != NULL) : False)
	    {
		/* Unref this IR SAR Visual Model */
		SARVisualModelUnref(scene, *sar_vmodel_ir);
		*sar_vmodel_ir = NULL;
	    }

	    /* Create a new or return existing SAR Visual Model */
	    *sar_vmodel = SARVisualModelNew(
		scene, filename, sar_vmodel_name
	    );
	    /* Is this a "new" Visual Model? */
	    if(SARVisualModelGetRefCount(*sar_vmodel) == 1)
	    {
		/* Create GL display list on the Visual Model */
		sar_visual_model_struct *vmodel = *sar_vmodel;
		GLuint list = (GLuint)SARVisualModelNewList(vmodel);
		if(list != 0)
		{
		    /* Mark SAR Visual Model as loading and begin
		     * recording the GL display list
		     */
		    vmodel->load_state = SAR_VISUAL_MODEL_LOADING;
		    glNewList(list, GL_COMPILE);

		    /* Process this V3D model into GL commands */
		    SARObjLoadProcessVisualModel(
			core_ptr, obj_num, obj_ptr,
			vmodel,
			v3d_model_ptr,
			False,		/* Not not process as IR */
			filename, 0
		    );

		    /* End recording GL display list and mark the SAR
		     * Visual Model as finished loading
		     */
	            glEndList();
		    vmodel->load_state = SAR_VISUAL_MODEL_LOADED;
		}
	    }
	    else
	    {
		/* This SAR Visual Model already has more than one ref
		 * count which implies it is shared
		 *
		 * Check for V3D Model primitives that still need to be
		 * loaded, all of the primitives handled here should
		 * not produce any GL commands
		 */
		int pn, ptype;
		void *p;
		sar_visual_model_struct *vmodel = *sar_vmodel;

		for(pn = 0; pn < v3d_model_ptr->total_primitives; pn++)
		{
		    p = v3d_model_ptr->primitive[pn];
		    if(p == NULL)
			continue;

		    ptype = V3DMPGetType(p);
		    switch(ptype)
		    {
			const mp_heightfield_load_struct *mp_heightfield_load;
		      case V3DMP_TYPE_HEIGHTFIELD_LOAD:
			mp_heightfield_load = (mp_heightfield_load_struct *)p;
			/* Need to load z points for height field but
			 * not issue any GL commands for it (by
			 * passing the GL list as 0)
			 */
			SARObjLoadHeightField(
			    core_ptr, obj_num, obj_ptr, vmodel,
			    p, filename, 0,
			    0		/* No GL display list */
			);
			break;
		    }

		}
	    }

	    /* Create IR SAR Visual Model? */
	    if((sar_vmodel_ir != NULL) &&
	       ((obj_ptr->type == SAR_OBJ_TYPE_STATIC) ||
		(obj_ptr->type == SAR_OBJ_TYPE_AUTOMOBILE) ||
		(obj_ptr->type == SAR_OBJ_TYPE_WATERCRAFT) ||
		(obj_ptr->type == SAR_OBJ_TYPE_AIRCRAFT)
	       )
	    )
	    {
		char *sar_vmodel_name_ir = STRDUP(sar_vmodel_name);

		/* Format the IR SAR Visual Model name */
		sar_vmodel_name_ir = strinsstr(
		    sar_vmodel_name_ir, -1, "_ir"
		);

		/* Create a new or return existing IR SAR Visual Model */
		*sar_vmodel_ir = SARVisualModelNew(
		    scene, filename, sar_vmodel_name_ir
		);
		/* Is this a "new" Visual Model? */
		if(SARVisualModelGetRefCount(*sar_vmodel_ir) == 1)
		{
		    /* Create GL display list on the Visual Model */
		    sar_visual_model_struct *vmodel = *sar_vmodel_ir;
		    GLuint list = (GLuint)SARVisualModelNewList(vmodel);
		    if(list != 0)
		    {
			/* Mark SAR Visual Model as loading and begin
			 * recording the GL display list
			 */
			vmodel->load_state = SAR_VISUAL_MODEL_LOADING;
			glNewList(list, GL_COMPILE);

			/* Process this V3D model into GL commands */
			SARObjLoadProcessVisualModel(
			    core_ptr, obj_num, obj_ptr,
			    vmodel,
			    v3d_model_ptr,
			    True,	/* Process as IR */
			    filename, 0
			);

			/* End recording GL display list and mark the
			 * IR SAR Visual Model as finished loading
			 */
			glEndList();
			vmodel->load_state = SAR_VISUAL_MODEL_LOADED;
		    }
		}

		free(sar_vmodel_name_ir);
	    }    


	    /* Delete the SAR Visual Model name */
	    free(sar_vmodel_name);
	    sar_vmodel_name = NULL;
	}


	/* Delete V3D models and V3D header items (no longer needed) */
	V3DMHListDeleteAll(&v3d_h, &total_v3d_h);
	V3DModelListDeleteAll(&v3d_model, &total_v3d_models);


	/* Create flight dynamics model if object is an aircraft */
	if(aircraft != NULL)
	{
	    if(scene->realm == NULL)
	    {
		fprintf(
		    stderr,
 "%s: Warning: Unable to create SFM for NULL realm on scene\n",
		    filename
		);
	    }
	    else
	    {
		/* Create new flight dynamics model and reset its
		 * values to defaults
		 */
		SFMModelStruct *fdm = SFMModelAllocate();
		if(fdm != NULL)
		{
		    fdm->landed_state = True;
		    fdm->stopped = True;

		    SFMModelAdd(scene->realm, fdm);
		}
		aircraft->fdm = fdm;
	    }
	}


	/* Need to realize translation and rotation values on object */
	SARSimWarpObject(
	    scene, obj_ptr,
	    &obj_ptr->pos, &obj_ptr->dir
	);

	/* Do postload check on object */
	SARObjLoadPostLoadCheck(
	    obj_num, obj_ptr,
	    filename
	);


	free(full_path);

	return(0);
}
