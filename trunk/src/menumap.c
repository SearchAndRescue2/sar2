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

#include <string.h>
#include <stdlib.h>
#include <sys/types.h>

#ifdef __MSW__
# include <windows.h>
#endif
  
#include <GL/gl.h>
	 
#include "../include/string.h"  

#include "v3dtex.h"
		
#include "gw.h"
#include "stategl.h"

#include "image.h"
#include "menu.h"
#include "sar.h"	/* For cur_millitime */


static sar_menu_map_marking_struct *SARMenuMapMatchMarkingPosition(
	gw_display_struct *display, sar_menu_map_struct *map,
	int x, int y, int start_marking_num, int *matched_marking_num
);

sar_menu_map_marking_struct *SARMenuMapGetMarkingPtr(
	sar_menu_map_struct *map, int i
);

int SARMenuMapNew(
	sar_menu_struct *m,
	float x, float y,
	float width, float height,
	GWFont *font,
	sar_menu_color_struct *color,
	const sar_image_struct **bg_image,	/* Total 9 images */
	float scroll_x, float scroll_y,
	float m_to_pixels_coeff,
	int show_markings_policy
);
int SARMenuMapAppendMarking(
	sar_menu_map_struct *map,
	int type,			/* One of SAR_MENU_MAP_MARKING_TYPE_* */
	const sar_menu_color_struct *fg_color,
	float x, float y,		/* In meters */
	float x_end, float y_end,	/* In meters (for intercept line) */
	const sar_image_struct *icon,	/* Shared icon image */
	const char *desc
);
void SARMenuMapDeleteAllMarkings(sar_menu_map_struct *map);
void SARMenuMapMarkingSelectNext(
	sar_menu_map_struct *map, Boolean allow_warp
);
void SARMenuMapMarkingSelectPrev(
	sar_menu_map_struct *map, Boolean allow_warp
);
void SARMenuMapDraw(
	gw_display_struct *display, sar_menu_struct *m,
	sar_menu_map_struct *map, int map_num
);
int SARMenuMapManagePointer(
	gw_display_struct *display, sar_menu_struct *m,
	sar_menu_map_struct *map, int menu_obj_num,
	int x, int y, gw_event_type type, int btn_num
);


#define ATOI(s)         (((s) != NULL) ? atoi(s) : 0)
#define ATOL(s)         (((s) != NULL) ? atol(s) : 0)
#define ATOF(s)         (((s) != NULL) ? atof(s) : 0.0f)
#define STRDUP(s)       (((s) != NULL) ? strdup(s) : NULL)

#define MAX(a,b)        (((a) > (b)) ? (a) : (b))
#define MIN(a,b)        (((a) < (b)) ? (a) : (b))
#define CLIP(a,l,h)     (MIN(MAX((a),(l)),(h)))
#define STRLEN(s)       (((s) != NULL) ? ((int)strlen(s)) : 0)


/*
 *      Default spacings (in pixels):
 *
 *	These values should match and coenciding values in menu.c.
 */
#define DEF_XMARGIN             5
#define DEF_YMARGIN             5

#define DEF_WIDTH               100
#define DEF_HEIGHT              100


#define DO_REDRAW_MENU(d,m)	{	\
 SARMenuDrawAll((d), (m));		\
 GWSwapBuffer(d);			\
}

#define DO_REDRAW_OBJECT(d,m,n)	{	\
 if((m)->always_full_redraw)		\
  SARMenuDrawAll((d), (m));		\
 else					\
  SARMenuDrawObject((d),(m),(n));	\
 GWSwapBuffer(d);			\
}


/*
 *	Returns the matched marking pointer at the given position
 *	in window coordinates relative to the upper left origin.
 *
 *	Returns NULL on failed match.
 *
 *	The search will start at the marking index specified by
 *	start_marking_num and returns the first match.
 */
static sar_menu_map_marking_struct *SARMenuMapMatchMarkingPosition(
	gw_display_struct *display, sar_menu_map_struct *map,
	int x, int y, int start_marking_num, int *matched_marking_num
)
{
	int i, width, height;
	int tolor_x, tolor_y;
	int marking_xp, marking_yp;
	int scroll_xp, scroll_yp;
	int	map_wp, map_hp,
		map_xp_cen, map_yp_cen,
		map_xp_min, map_yp_min,
		map_xp_max, map_yp_max;
	float mtop_coeff;
	sar_menu_map_marking_struct *marking_ptr;
	const sar_image_struct *icon;


	if(matched_marking_num != NULL)
	    *matched_marking_num = -1;

	if(map == NULL)
	    return(NULL);

	GWContextGet(
	    display, GWContextCurrent(display),
	    NULL, NULL,
	    NULL, NULL,
	    &width, &height
	);

	/* Bounds of map object on menu in pixels */
	map_xp_cen = (int)(map->x * width);
	map_yp_cen = (int)(map->y * height);

	map_wp = (int)(map->width * width);
	map_hp = (int)(map->height * height);
	if((map_wp <= 0) || (map_hp <= 0))
	    return(NULL);

	map_xp_min = MAX(map_xp_cen - (map_wp / 2), 0);
	map_yp_min = MAX(map_yp_cen - (map_hp / 2), 0);

	map_xp_max = map_xp_min + map_wp;
	map_yp_max = map_yp_min + map_hp;

	/* Flip given y coordinate */
	y = (map_hp - (y - map_yp_min)) + map_yp_min;

	mtop_coeff = map->m_to_pixels_coeff;
 
	scroll_xp = map->scroll_x;
	scroll_yp = map->scroll_y;

	/* Iterate through markings on the map object */
	for(i = start_marking_num; i < map->total_markings; i++)
	{
	    marking_ptr = map->marking[i];
	    if(marking_ptr == NULL)
		continue;

	    /* Calculate marking position in pixels */
	    marking_xp = (int)((marking_ptr->x * mtop_coeff) +
		map_xp_cen + scroll_xp);
	    marking_yp = (int)((marking_ptr->y * mtop_coeff) +
		map_yp_cen + scroll_yp);

	    /* Get icon of marking (if any) */
	    icon = marking_ptr->icon;
	    if(icon == NULL)
	    {
		tolor_x = 5;
		tolor_y = 5;
	    }
	    else
	    {
		tolor_x = icon->width / 2;
		tolor_y = icon->height / 2;
	    }

	    /* Given coordinates in bound with tolorance applied to
	     * marking position?
	     */
	    if(((marking_xp - tolor_x) < x) &&
	       ((marking_xp + tolor_x) > x) &&
	       ((marking_yp - tolor_y) < y) &&
	       ((marking_yp + tolor_y) > y)
	    )
	    {
		if(matched_marking_num != NULL)
		    *matched_marking_num = i;
		return(marking_ptr);
	    }
	}
	return(NULL);
}


/*
 *      Returns the pointer to the marking specified by the index i
 *      on the given map object.
 */
sar_menu_map_marking_struct *SARMenuMapGetMarkingPtr(
	sar_menu_map_struct *map, int i
)
{
	if(map == NULL)
	    return(NULL);

	if((i < 0) || (i >= map->total_markings))
	    return(NULL);
	else
	    return(map->marking[i]);
}

/*
 *	Creates a map object on menu m, returns the index number of
 *	the map object or -1 on failure.
 */
int SARMenuMapNew(
	sar_menu_struct *m,
	float x, float y,
	float width, float height,
	GWFont *font,
	sar_menu_color_struct *color,
	const sar_image_struct **bg_image,	/* Total 9 images */
	float scroll_x, float scroll_y,
	float m_to_pixels_coeff,
	int show_markings_policy
)
{
	int i, n;
	sar_menu_map_struct *map;


	if(m == NULL)
	    return(-1);

	if(m->total_objects < 0)
	    m->total_objects = 0;

	for(i = 0; i < m->total_objects; i++)
	{
	    if(m->object[i] == NULL)
		break;
	}
	if(i < m->total_objects)
	{
	    n = i;
	}
	else
	{
	    n = m->total_objects;
	    m->total_objects = n + 1;
	    m->object = (void **)realloc(
		m->object,
		m->total_objects * sizeof(void *)
	    );
	    if(m->object == NULL)
	    {
		m->total_objects = 0;
		return(-1);
	    }
	}   
	    
	/* Allocate structure */
	m->object[n] = map = SAR_MENU_MAP(calloc(
	    1, sizeof(sar_menu_map_struct)
	));
	if(map == NULL)
	    return(-1);
	 
	/* Set map values */
	map->type = SAR_MENU_OBJECT_TYPE_MAP;
	map->x = x;
	map->y = y;
	map->width = width;
	map->height = height;
	map->sensitive = True;
	map->font = font;
	if(color != NULL)
	    memcpy(&map->color, color, sizeof(sar_menu_color_struct));
	map->show_markings_policy = show_markings_policy;
	map->bg_image = bg_image;
	map->scroll_x = (int)(-scroll_x * m_to_pixels_coeff);
	map->scroll_y = (int)(-scroll_y * m_to_pixels_coeff);
	map->m_to_pixels_coeff = m_to_pixels_coeff;
	map->marking = NULL;
	map->total_markings = 0;
	map->selected_marking = -1;

	return(n);
}


/*
 *	Appends a new marking to the given map object.
 *
 *	Returns the new marking index or -1 on error.
 */
int SARMenuMapAppendMarking(
	sar_menu_map_struct *map,
	int type,			/* One of SAR_MENU_MAP_MARKING_TYPE_* */
	const sar_menu_color_struct *fg_color,
	float x, float y,		/* In meters */
	float x_end, float y_end,	/* In meters (for intercept line) */
	const sar_image_struct *icon,	/* Shared icon image */
	const char *desc
)
{
	int i;
	sar_menu_map_marking_struct *marking_ptr;


	if(map == NULL)
	    return(-1);

	i = MAX(map->total_markings, 0);
	map->total_markings = i + 1;
	map->marking = (sar_menu_map_marking_struct **)realloc(
	    map->marking,
	    map->total_markings * sizeof(sar_menu_map_marking_struct *)
	);
	if(map->marking == NULL)
	{
	    map->total_markings = 0;
	    return(-1);
	}

	map->marking[i] = marking_ptr = SAR_MENU_MAP_MARKING(calloc(
	    1, sizeof(sar_menu_map_marking_struct)
	));
	if(marking_ptr == NULL)
	    return(-1);

	marking_ptr->type = type;
	if(fg_color != NULL)
	    memcpy(&marking_ptr->fg_color, fg_color, sizeof(sar_menu_color_struct));
	marking_ptr->x = x;
	marking_ptr->y = y;
	marking_ptr->x_end = x_end;
	marking_ptr->y_end = y_end;
	marking_ptr->icon = icon;
	marking_ptr->desc = STRDUP(desc);

	return(i);
}

/*
 *	Delete all map markings on map object.
 */
void SARMenuMapDeleteAllMarkings(sar_menu_map_struct *map)
{
	int i;
	sar_menu_map_marking_struct *marking_ptr;


	if(map == NULL)
	    return;

	/* Iterate through markings on map object */
	for(i = 0; i < map->total_markings; i++)
	{
	    marking_ptr = map->marking[i];
	    if(marking_ptr == NULL)
		continue;

	    marking_ptr->icon = NULL;	/* Shared */

	    free(marking_ptr->desc);
	    marking_ptr->desc = NULL;

	    free(marking_ptr);
	    map->marking[i] = marking_ptr = NULL;
	}

	free(map->marking);
	map->marking = NULL;
	map->total_markings = 0;
}


/*
 *	Selects the next marking on the given map object.
 */
void SARMenuMapMarkingSelectNext(
	sar_menu_map_struct *map, Boolean allow_warp
)
{
	int i, passes, got_match = 0;
	sar_menu_map_marking_struct *marking_ptr;


	if(map == NULL)
	    return;

	/* Marking can only be selected if marking show policy is set
	 * to SAR_MENU_MAP_SHOW_MARKING_SELECTED.
	 */
	if(map->show_markings_policy != SAR_MENU_MAP_SHOW_MARKING_SELECTED)
	    return;

	/* No markings? */
	if(map->total_markings <= 0)
	    return;

	i = map->selected_marking;
	for(passes = 0; passes < 2; passes++)
	{
	    while(i < (map->total_markings - 1))
	    {
		i++;
		marking_ptr = SARMenuMapGetMarkingPtr(map, i);
		if(marking_ptr != NULL)
		{
		    if(marking_ptr->desc != NULL)
		    {
			got_match = 1;
			break;
		    }
		}
	    }
	    if(got_match)
		break;

	    /* Start from the beginning - 1 for one more pass */
	    if(allow_warp)
		i = -1;
	    else
		break;
	}
	/* If a marking was matched then update selected marking as index i */
	if(got_match)
	{
	    map->selected_marking = i;

	    /* Scroll to selected marking */
	    marking_ptr = SARMenuMapGetMarkingPtr(
		map, map->selected_marking
	    );
	    if(marking_ptr != NULL)
	    {
		map->scroll_x = (int)(-marking_ptr->x *
		    map->m_to_pixels_coeff);
		map->scroll_y = (int)(-marking_ptr->y *
		    map->m_to_pixels_coeff);
	    }
	}
}

/*
 *      Selects the previous marking on the given map object.
 */
void SARMenuMapMarkingSelectPrev(
	sar_menu_map_struct *map, Boolean allow_warp
)
{
	int i, passes, got_match = 0;
	sar_menu_map_marking_struct *marking_ptr;


	if(map == NULL)
	    return;

	/* Marking can only be selected if marking show policy is set
	 * to SAR_MENU_MAP_SHOW_MARKING_SELECTED.
	 */
	if(map->show_markings_policy != SAR_MENU_MAP_SHOW_MARKING_SELECTED)
	    return;

	/* No markings? */
	if(map->total_markings <= 0)
	    return;

	i = map->selected_marking;
	for(passes = 0; passes < 2; passes++)
	{
	    while(i > 0)
	    {
		i--;
		marking_ptr = SARMenuMapGetMarkingPtr(map, i);
		if(marking_ptr != NULL)
		{
		    if(marking_ptr->desc != NULL)
		    {
			got_match = 1;
			break;
		    }
		}
	    }
	    if(got_match)
		break;

	    /* Start from the end + 1 for one more pass */
	    if(allow_warp)
		i = map->total_markings;
	    else
		break;
	}
	/* If a marking was matched then update selected marking as index i */
	if(got_match)
	{
	    map->selected_marking = i;

	    /* Scroll to selected marking */
	    marking_ptr = SARMenuMapGetMarkingPtr(
		map, map->selected_marking
	    );
	    if(marking_ptr != NULL)
	    {
		map->scroll_x = (int)(-marking_ptr->x *
		    map->m_to_pixels_coeff);
		map->scroll_y = (int)(-marking_ptr->y *
		    map->m_to_pixels_coeff);
	    }
	}
}


/*
 *	Draws the map object.
 */
void SARMenuMapDraw(
	gw_display_struct *display,
	sar_menu_struct *m,
	sar_menu_map_struct *map,
	int map_num
)
{
	int i, width, height;
	int scroll_xp, scroll_yp;
	int	map_wp, map_hp,
		map_xp_cen, map_yp_cen,
		map_xp_min, map_yp_min,
		map_xp_max, map_yp_max;

	int	marking_xp, marking_yp;
	float mtop_coeff;
	const sar_image_struct *image;
	v3d_texture_ref_struct *t;
	sar_menu_map_marking_struct *marking_ptr;


	if((display == NULL) || (m == NULL) || (map == NULL))
	    return;

	GWContextGet(
	    display, GWContextCurrent(display),
	    NULL, NULL,
	    NULL, NULL,
	    &width, &height
	);

	mtop_coeff = map->m_to_pixels_coeff;

	scroll_xp = map->scroll_x;
	scroll_yp = map->scroll_y;

	/* Bounds of map object on menu in pixels */
	map_xp_cen = (int)(map->x * width);
	map_yp_cen = (int)(map->y * height);

	map_wp = (int)(map->width * width);
	map_hp = (int)(map->height * height);
	if((map_wp < 1) || (map_hp < 1))
	    return;

	map_xp_min = MAX(map_xp_cen - (map_wp / 2), 0);
	map_yp_min = MAX(map_yp_cen - (map_hp / 2), 0);

	map_xp_max = map_xp_min + map_wp;
	map_yp_max = map_yp_min + map_hp;

	/* Convert all coordinates to lower-left origin for easy
	 * OpenGL calculations.
	 */
	i = map_yp_max;
	map_yp_cen = MAX(height - map_yp_cen, 0);
	map_yp_max = MAX(height - map_yp_min, 0);
	map_yp_min = MAX(height - i, 0);


	t = map->bg_tex;
	if(t != NULL)
	{
	    int tex_wp = (int)MAX(map->bg_tex_width * mtop_coeff, 10);
	    int tex_hp = (int)MAX(map->bg_tex_height * mtop_coeff, 10);
	    float t_xc_min, t_yc_min, t_xc_max, t_yc_max;

	    /* Calculate texture coordinate bounds */
	    t_xc_min = (float)(-scroll_xp + (tex_wp / 2) - (map_wp / 2))
		/ (float)tex_wp;
	    t_xc_max = (float)(-scroll_xp + (tex_wp / 2) + (map_wp / 2))
		/ (float)tex_wp;

	    t_yc_max = (float)(scroll_yp + (tex_hp / 2) - (map_hp / 2))
		/ (float)tex_hp;
	    t_yc_min = (float)(scroll_yp + (tex_hp / 2) + (map_hp / 2))
		/ (float)tex_hp;

	    StateGLTexEnvI(
		&display->state_gl, GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE,
		GL_MODULATE
	    );
	    StateGLEnable(&display->state_gl, GL_TEXTURE_2D);
	    V3DTextureSelect(t);
	    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

	    glColor4d(0.5, 0.5, 0.5, 1.0);
	    glBegin(GL_QUADS);
	    {
		glTexCoord2d(t_xc_min, t_yc_min);
		glVertex2d(map_xp_min, map_yp_min);

		glTexCoord2d(t_xc_max, t_yc_min);
		glVertex2d(map_xp_max, map_yp_min);

		glTexCoord2d(t_xc_max, t_yc_max);
		glVertex2d(map_xp_max, map_yp_max);

		glTexCoord2d(t_xc_min, t_yc_max);
		glVertex2d(map_xp_min, map_yp_max);
	    }
	    glEnd();

	    StateGLDisable(&display->state_gl, GL_TEXTURE_2D);
	}
	else
	{
	    glColor4d(0.0, 0.0, 0.0, 1.0);
	    glBegin(GL_QUADS);
	    {
		glVertex2d((GLfloat)map_xp_min, (GLfloat)map_yp_min);
		glVertex2d((GLfloat)map_xp_max, (GLfloat)map_yp_min);
		glVertex2d((GLfloat)map_xp_max, (GLfloat)map_yp_max);
		glVertex2d((GLfloat)map_xp_min, (GLfloat)map_yp_max);
	    }
	    glEnd();
	}

	/* Draw title? */
	if(map->title != NULL)
	{
	    const char *s = map->title;
	    GWFont *font = map->font;
	    sar_menu_color_struct *color = &map->color;
	    int fw, fh, len = STRLEN(s);

	    GWGetFontSize(font, NULL, NULL, &fw, &fh);
	    GWSetFont(display, font);
	    glColor4f(
		color->r,
		color->g,
		color->b,
		color->a
	    );
	    GWDrawString(
		display,
		map_xp_cen - (fw * len / 2),
		height - (map_yp_max - 16),
		s
	    );
	}

	/* Begin drawing map markings */
	for(i = 0; i < map->total_markings; i++)
	{
	    marking_ptr = map->marking[i];
	    if(marking_ptr == NULL)
		continue;

	    /* Skip this marking if we are only to display the selected
	     * marking and this is not selected.
	     */
	    if(map->show_markings_policy == SAR_MENU_MAP_SHOW_MARKING_SELECTED)
	    {
/* Skip this check for now */
	    }

	    /* Calculate marking position in pixels */
	    marking_xp = (int)((marking_ptr->x * mtop_coeff) +
		map_xp_cen + scroll_xp);
	    marking_yp = (int)((marking_ptr->y * mtop_coeff) +
		map_yp_cen + scroll_yp);

	    /* Handle by type */
	    switch(marking_ptr->type)
	    {
	      /* ***************************************************** */
	      case SAR_MENU_MAP_MARKING_TYPE_ICON:
		image = marking_ptr->icon;
		if(image != NULL)
		{
		    int	img_hw = image->width / 2,
			img_hh = image->height / 2;

		    /* Out of bounds? */
		    if((marking_xp <= (map_xp_min + img_hw)) ||
		       (marking_yp <= (map_yp_min + img_hh)) ||
		       (marking_xp >= (map_xp_max - img_hw)) ||
		       (marking_yp >= (map_yp_max - img_hh))
		    )
			break;

		    glPixelZoom(1.0, -1.0);
		    StateGLEnable(&display->state_gl, GL_ALPHA_TEST);
		    StateGLAlphaFunc(&display->state_gl, GL_GREATER, 0.5);
		    glRasterPos2i(
		        marking_xp - (image->width / 2),
		        marking_yp + (image->height / 2)
		    );
		    glDrawPixels(
			image->width, image->height,
			GL_RGBA, GL_UNSIGNED_BYTE,
			image->data
		    );
		}
		break;

	      case SAR_MENU_MAP_MARKING_TYPE_INTERCEPT_LINE:
/* Need better bounds checking */
		if(marking_xp <= map_xp_min)
		    marking_xp = map_xp_min + 1;
		else if(marking_xp >= map_xp_max)
		    marking_xp = map_xp_max - 1;

		if(marking_yp <= map_yp_min)
		    marking_yp = map_yp_min + 1;
		else if(marking_yp >= map_yp_max)
		    marking_yp = map_yp_max - 1;


		glColor4d(
		    (GLfloat)marking_ptr->fg_color.r,
		    (GLfloat)marking_ptr->fg_color.g,
		    (GLfloat)marking_ptr->fg_color.b,
		    (GLfloat)marking_ptr->fg_color.a
		);
		glBegin(GL_LINES);
		{
		    int marking_x2p = (int)((marking_ptr->x_end * mtop_coeff) +
			map_xp_cen + scroll_xp),
			marking_y2p = (int)((marking_ptr->y_end * mtop_coeff) +
			map_yp_cen + scroll_yp);

/* Need better bounds checking */
		    if(marking_x2p <= map_xp_min)
			marking_x2p = map_xp_min + 1;
		    else if(marking_x2p >= map_xp_max)
			marking_x2p = map_xp_max - 1;

		    if(marking_y2p <= map_yp_min)
			marking_y2p = map_yp_min + 1;
		    else if(marking_y2p >= map_yp_max)
			marking_y2p = map_yp_max - 1;

		    glVertex2d((GLfloat)marking_xp, (GLfloat)marking_yp);
		    glVertex2d((GLfloat)marking_x2p, (GLfloat)marking_y2p);
		}
		glEnd();
		break;
	    }
	}

	/* Check if a marking is selected */
	i = map->selected_marking;
	if((i >= 0) && (i < map->total_markings))
	{
	    marking_ptr = map->marking[i];
	    if((marking_ptr != NULL) ? (marking_ptr->desc != NULL) : False)
	    {
		GWFont *font = map->font;
		sar_menu_color_struct *color = &map->color;
		const char *desc = marking_ptr->desc;
		int fw, fh, len;

		len = STRLEN(desc);
		GWGetFontSize(font, NULL, NULL, &fw, &fh);
		GWSetFont(display, font);
		glColor4f(
		    color->r,
		    color->g,
		    color->b,
		    color->a
		);
		GWDrawString(
		    display,
		    map_xp_cen - (fw * len / 2),
		    height - (map_yp_min + fh + (5 + 16)),
		    desc
		);
	    }
	}
}


/*
 *	Manages the pointer event with respect to the given menu map
 *	object.
 */
int SARMenuMapManagePointer(
	gw_display_struct *display, sar_menu_struct *m,
	sar_menu_map_struct *map, int menu_obj_num,
	int x, int y, gw_event_type type, int btn_num
)
{
	int events_handled = 0;
	int x_min, x_max, y_min, y_max, width, height, w, h;

	static Boolean map_translate_drag_state = False;
	static Boolean map_zoom_drag_state = False;
	static int prev_motion_x, prev_motion_y;
	static time_t last_motion_time = 0;


	if((display == NULL) || (m == NULL) || (map == NULL))
	    return(events_handled);

	GWContextGet(
	    display, GWContextCurrent(display),
	    NULL, NULL,
	    NULL, NULL,
	    &width, &height
	);

	/* If the button state is down and the event is a release,
	 * then reset all states
	 */
	if((map_translate_drag_state || map_zoom_drag_state ) &&
	   (type == GWEventTypeButtonRelease)
	)
	{
	    map_translate_drag_state = False;
	    map_zoom_drag_state = False;
	}

	w = (int)(map->width * width);
	if(w <= 0)
	    w = (int)(width - (2 * DEF_XMARGIN));
	if(w <= 0)
	    w = DEF_XMARGIN;

	if(map->height > 0)
	    h = (int)(map->height * height);
	else
	    h = DEF_HEIGHT;
	if(h <= 0)
	    h = DEF_YMARGIN;

	/* Get coordinate bounds */
	x_min = (int)((map->x * width) - (w / 2));
	x_max = (int)(x_min + w);
	y_min = (int)((map->y * height) - (h / 2));
	y_max = (int)(y_min + h);

	/* Pointer event in bounds? */
	if((x >= x_min) && (x < x_max) &&
	   (y >= y_min) && (y < y_max)
	)
	{
	    /* Calculate scroll bounds */
	    float mtop_coeff;
	    int tex_w, tex_h;
	    int scroll_x_min, scroll_y_min, scroll_x_max, scroll_y_max;

/* Calculates scroll bounds by first fetching latest mtop_coeff
 * value and updates variables tex_w, tex_h, scroll_x_min, scroll_y_min,
 * scroll_x_max, and scroll_y_max.
 */
#define GET_SCROLL_BOUNDS	\
{ \
 mtop_coeff = map->m_to_pixels_coeff; \
 \
 /* Get texture size in pixels */ \
 tex_w = (int)(map->bg_tex_width * mtop_coeff); \
 tex_h = (int)(map->bg_tex_height * mtop_coeff); \
 \
 /* Calculate scroll bounds in pixels */ \
 scroll_x_min = (w / 2) - (tex_w / 2); \
 scroll_x_max = (tex_w / 2) - (w / 2); \
 \
 scroll_y_min = (h / 2) - (tex_h / 2); \
 scroll_y_max = (tex_h / 2) - (h / 2); \
}
	    GET_SCROLL_BOUNDS

	    /* Handle by event type */
	    switch(type)
	    {
	      case GWEventTypeButtonPress:
		events_handled++;
		/* Select new object as needed and redraw the
		 * previously selected object.
		 */
		if(m->selected_object != menu_obj_num)
		{
		    int po = m->selected_object;
		    m->selected_object = menu_obj_num;
		    if(m->always_full_redraw)
			SARMenuDrawAll(display, m);
		    else
			SARMenuDrawObject(display, m, po);
		}
		/* Handle by button number */
		switch(btn_num)
		{
		  case 1:
		    map_translate_drag_state = True;
		    last_motion_time = cur_millitime;
		    prev_motion_x = x;
		    prev_motion_y = y;

		    GWSetPointerCursor(
			display, GWPointerCursorTranslate
		    );
		    DO_REDRAW_OBJECT(display, m, menu_obj_num)
		    break;

		  case 3:
		    map_zoom_drag_state = True;
		    last_motion_time = cur_millitime;
		    prev_motion_x = x;
		    prev_motion_y = y;

		    GWSetPointerCursor(
			display, GWPointerCursorZoom
		    );
		    DO_REDRAW_OBJECT(display, m, menu_obj_num)
		    break;
		}
		break;

	      case GWEventTypeButtonRelease:
		GWSetPointerCursor(
		    display, GWPointerCursorStandard
		);

		/* Manipulate last motion time so next case
		 * checks okay
		 */
		last_motion_time = cur_millitime - 1;

		/* Fall through */
	      case GWEventTypePointerMotion:
		/* Too soon to handle a motion event? */
		if(last_motion_time == cur_millitime)
		    break;
		else
		    last_motion_time = cur_millitime;
		events_handled++;

		/* Translating? */
		if(map_translate_drag_state)
		{
#if 0
		    map->scroll_x = CLIP(
			map->scroll_x + (x - prev_motion_x),
			scroll_x_min, scroll_x_max
		    );
		    map->scroll_y = CLIP(
			map->scroll_y - (y - prev_motion_y),
			scroll_y_min, scroll_y_max
		    );
#endif
		    map->scroll_x = map->scroll_x +
			(x - prev_motion_x);
		    map->scroll_y = map->scroll_y -
			(y - prev_motion_y);

		    DO_REDRAW_OBJECT(display, m, menu_obj_num)
		}
		/* Zooming? */
		else if(map_zoom_drag_state)
		{
		    float scroll_x_coeff, scroll_y_coeff;
		    float prev_m_to_pixels_coeff =
			map->m_to_pixels_coeff;

		    /* Calculate new zoom */
		    map->m_to_pixels_coeff = (float)CLIP(
			map->m_to_pixels_coeff +
			    ((y - prev_motion_y) *
			    map->m_to_pixels_coeff / 100),
			0.002,		/* Zoomed out max */
			0.050		/* Zoomed in max */
		    );

		    /* Need to calculate last scroll position coeff
		     * relative to the previous zoom dimension, then
		     * set the new scroll position by multiplying
		     * the scroll position coeff just calculated by
		     * the new zoom dimension.
		     */
		    if(map->bg_tex_width > 0)
			scroll_x_coeff = (float)map->scroll_x /
			    (map->bg_tex_width *
				prev_m_to_pixels_coeff);
		    else
			scroll_x_coeff = 0.0;
		    map->scroll_x = (int)(map->bg_tex_width *
			map->m_to_pixels_coeff * scroll_x_coeff);

		    if(map->bg_tex_height > 0)
			scroll_y_coeff = -(float)map->scroll_y /
			    (map->bg_tex_height *
				prev_m_to_pixels_coeff);
		    else
			scroll_y_coeff = 0.0;

		    map->scroll_y = -(int)(map->bg_tex_height *
			map->m_to_pixels_coeff * scroll_y_coeff);

		    /* Need to calculate scroll bounds again */
		    GET_SCROLL_BOUNDS

		    /* Reclip scroll bounds */
/* Its safe to scroll anywhere, plus we can see offmap markings.
		    map->scroll_x = CLIP(
			map->scroll_x,
			scroll_x_min, scroll_x_max
		    );
		    map->scroll_y = CLIP(
			map->scroll_y,
			scroll_y_min, scroll_y_max
		    );
 */
		    DO_REDRAW_OBJECT(display, m, menu_obj_num)
		}
		/* All else assume marking select, we can select a
		 * marking with the pointer only if we are showing
		 * all markings. Cannot select a marking if only showing
		 * the selected marking.
		 */
		else if(map->show_markings_policy == SAR_MENU_MAP_SHOW_MARKING_ALL)
		{
		    /* Do this even if button is in released state */

		    int	prev_marking_num = map->selected_marking,
			matched_marking_num = -1,
			next_marking_num;
		    sar_menu_map_marking_struct *marking_ptr;


		    /* Iterate till we find a marking with a description */
		    for(next_marking_num = 0; 1;)
		    {
			marking_ptr = SARMenuMapMatchMarkingPosition(
			    display, map,
			    x, y,
			    next_marking_num, &matched_marking_num
			);
			if((marking_ptr == NULL) || (matched_marking_num < 0))
			    break;

			/* This one has a description? */
			if(marking_ptr->desc != NULL)
			    break;

			next_marking_num = matched_marking_num + 1;
		    }

		    if(matched_marking_num != prev_marking_num)
		    {
			map->selected_marking = matched_marking_num;

			/* For this we need to redraw the entire menu */
			DO_REDRAW_MENU(display,m)
		    }
		}

		/* Record previous pointer coordinates */
		prev_motion_x = x;
		prev_motion_y = y;
		break;

	      case GWEventType2ButtonPress:
	      case GWEventType3ButtonPress:
		break;

	    }	/* Handle by event type */

	}	/* Pointer event in bounds? */


#undef GET_SCROLL_BOUNDS      

	return(events_handled);
}
