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
#include <ctype.h>
#include <stdlib.h>
#include <sys/types.h>

#ifdef __MSW__
# include <windows.h>
#endif

#include <GL/gl.h>

#include "../include/string.h"
#include "v3dgl.h"
#include "v3dtex.h"

#include "gw.h"
#include "stategl.h"
#include "image.h"
#include "menu.h"


/* Utilities */
void SARMenuGLStateReset(gw_display_struct *display);
void *SARMenuGetObject(sar_menu_struct *m, int n);
void *SARMenuGetObjectByID(
	sar_menu_struct *m, int id, int *n
);
Boolean SARMenuObjectIsSensitive(sar_menu_struct *m, int n);

/* Menu */
sar_menu_struct *SARMenuNew(
	sar_menu_type type, const char *name, const char *bg_image
);
void SARMenuLoadBackgroundImage(sar_menu_struct *m);
void SARMenuUnloadBackgroundImage(sar_menu_struct *m);
void SARMenuDelete(sar_menu_struct *m);

/* Label */
int SARMenuLabelNew(
	sar_menu_struct *m,
	float x, float y,
	int width, int height,
	const char *label,
	sar_menu_color_struct *fg_color, GWFont *font,
	const sar_image_struct *image
);

/* Button */
int SARMenuButtonNew(
	sar_menu_struct *m,
	float x, float y,   
	int width, int height,
	const char *label,
	sar_menu_color_struct *fg_color, GWFont *font,
	const sar_image_struct *unarmed_image,
	const sar_image_struct *armed_image,
	const sar_image_struct *highlighted_image,
	void *client_data, int id,
	void (*func_cb)(void *, int, void *)
);

/* Progress Bar */
int SARMenuProgressNew(
	sar_menu_struct *m,
	float x, float y,
	int width, int height,
	const char *label,
	sar_menu_color_struct *fg_color,
	GWFont *font,
	const sar_image_struct *bg_image,
	const sar_image_struct *fg_image,
	float progress
);
void SARMenuProgressSet(
	gw_display_struct *display, sar_menu_struct *m, int n,
	float progress, Boolean redraw
);

/* Message Box */
int SARMenuMessageBoxNew(
	sar_menu_struct *m,
	float x, float y,
	float width, float height,
	sar_menu_color_struct *fg_color, GWFont *font,
	const sar_image_struct **bg_image,	/* Pointer to 9 images */
	void *client_data, int id,
	const char *message
);
void SARMenuMessageBoxSet(
	gw_display_struct *display, sar_menu_struct *m, int n,
	const char *message, Boolean redraw
);

/* List */
int SARMenuListNew(
	sar_menu_struct *m,
	float x, float y,
	float width, float height,
	sar_menu_color_struct *fg_color, GWFont *font,
	const char *label,
	const sar_image_struct **bg_image,
	void *client_data, int id,
	void (*select_cb)(void *, int, void *, int, void *, void *),
	void (*activate_cb)(void *, int, void *, int, void *, void *)
);
int SARMenuListAppendItem(
	sar_menu_struct *m, int n,
	const char *name,
	void *client_data,
	sar_menu_flags_t flags
);
sar_menu_list_item_struct *SARMenuListGetItemByNumber(
	sar_menu_list_struct *list, int i
);
void SARMenuListSelect(
	gw_display_struct *display, sar_menu_struct *m, int n,
	int i,
	Boolean scroll, Boolean redraw
);
void SARMenuListDeleteAllItems(sar_menu_struct *m, int n);

/* Multi-purpose Display */
int SARMenuMDisplayNew(
	sar_menu_struct *m,
	float x, float y,
	float width, float height,
	const sar_image_struct **bg_image,	/* Pointer to 9 images */
	void *client_data, int id,
	void (*draw_cb)(
		void *,         /* Display */
		void *,         /* Menu */
		void *,         /* This Object */
		int,            /* ID Code */
		void *,         /* Data */
		int, int, int, int      /* x_min, y_min, x_max, y_max */
	)
);
void SARMenuMDisplayDraw(
	gw_display_struct *display, sar_menu_struct *m, int n
);

/* Switch */
int SARMenuSwitchNew(
	sar_menu_struct *m,
	float x, float y,
	int width, int height,
	sar_menu_color_struct *fg_color, GWFont *font,
	const char *label,
	const sar_image_struct *bg_image,
	const sar_image_struct *switch_off_image,
	const sar_image_struct *switch_on_image,
	Boolean state,
	void *client_data, int id,
	void (*switch_cb)(void *, int, void *, Boolean)
);
Boolean SARMenuSwitchGetValue(sar_menu_struct *m, int n);
void SARMenuSwitchSetValue(
	gw_display_struct *display, sar_menu_struct *m, int n,
	Boolean state, Boolean redraw
);

/* Spin */
int SARMenuSpinNew(
	sar_menu_struct *m,
	float x, float y,
	float width, float height,
	sar_menu_color_struct *label_color,
	sar_menu_color_struct *value_color,
	GWFont *font,
	GWFont *value_font,
	const char *label,
	const sar_image_struct *label_image,
	const sar_image_struct *value_image,
	const sar_image_struct *dec_armed_image,
	const sar_image_struct *dec_unarmed_image,
	const sar_image_struct *inc_armed_image,
	const sar_image_struct *inc_unarmed_image,
	void *client_data, int id,
	void (*change_cb)(void *, int, void *, char *)
);
void SARMenuSpinSetValueType(
	sar_menu_struct *m, int n, int type
);
int SARMenuSpinAddValue(
	sar_menu_struct *m, int n, const char *value
);
char *SARMenuSpinGetCurrentValue(
	sar_menu_struct *m, int n, int *sel_num
);
void SARMenuSpinSelectValueIndex(
	gw_display_struct *display, sar_menu_struct *m, int n,
	int value_num,
	Boolean redraw
);
void SARMenuSpinSetValueIndex(
	gw_display_struct *display, sar_menu_struct *m, int n,
	int value_num, const char *value,
	Boolean redraw
);
void SARMenuSpinDeleteAllValues(sar_menu_struct *m, int n);
void SARMenuSpinDoInc(
	gw_display_struct *display, sar_menu_struct *m,	int n,
	Boolean redraw
);
void SARMenuSpinDoDec(
	gw_display_struct *display, sar_menu_struct *m, int n,
	Boolean redraw
);

/* Slider */
int SARMenuSliderNew(
	sar_menu_struct *m,
	float x, float y,
	float width, float height,
	sar_menu_color_struct *label_color,
	GWFont *font,
	const char *label,
	const sar_image_struct *label_image,
	const sar_image_struct *trough_image,
	const sar_image_struct *handle_image,
	void *client_data, int id,
	void (*change_cb)(void *, int, void *, float)
);
void SARMenuSliderSetValueBounds(
	gw_display_struct *display, sar_menu_struct *m, int n,
	float lower, float upper,
	Boolean redraw
);
void SARMenuSliderSetValue(
	gw_display_struct *display, sar_menu_struct *m, int n,
	float value,
	Boolean redraw
);
float SARMenuSliderGetValue(sar_menu_struct *m, int n);

/* Object Common */
void SARMenuObjectSetSensitive(
	gw_display_struct *display, sar_menu_struct *m, int n,
	Boolean sensitive, Boolean redraw
);
void SARMenuObjectDelete(sar_menu_struct *m, int n);

/* Drawing */
static const char *SARMenuMessageBoxDrawString(
	gw_display_struct *display,
	int x, int y,
	const char *buf_ptr,
	int visible_columns, int font_width,
	Boolean sensitive,
	Boolean draw_line,
	const sar_menu_color_struct *c_def,
	const sar_menu_color_struct *c_bold,
	const sar_menu_color_struct *c_ul
);
static int SARMenuMessageBoxTotalLines(
	const char *buf_ptr, int visible_columns
);
void SARMenuDrawWindowBG(
	gw_display_struct *display,
	const sar_image_struct **bg_image,	/* Total 9 images */
	const sar_menu_color_struct *base_color,	/* Set NULL to use image as base */
	int x, int y,
	int width, int height,
	Boolean draw_base,
	Boolean draw_shadow,
	Boolean is_selected
);
static void SARMenuDoDrawObject(
	gw_display_struct *display, sar_menu_struct *m,
	int n,			/* Object number on m */
	Boolean draw_shadows
);
void SARMenuDrawObject(
	gw_display_struct *display, sar_menu_struct *m,
	int n			/* Object number on m */
);
void SARMenuDrawAll(
	gw_display_struct *display, sar_menu_struct *m
);

/* Manage Pointer */
int SARMenuManagePointer(
	gw_display_struct *display, sar_menu_struct *m,
	int x, int y, gw_event_type type, int btn_num
);

/* Manage Key */
int SARMenuManageKey(
	gw_display_struct *display,
	sar_menu_struct *m,
	int key, Boolean state
);


#define ATOI(s)         (((s) != NULL) ? atoi(s) : 0)
#define ATOL(s)         (((s) != NULL) ? atol(s) : 0)
#define ATOF(s)         (((s) != NULL) ? atof(s) : 0.0f)
#define STRDUP(s)       (((s) != NULL) ? strdup(s) : NULL)

#define MAX(a,b)        (((a) > (b)) ? (a) : (b))
#define MIN(a,b)        (((a) < (b)) ? (a) : (b))
#define CLIP(a,l,h)     (MIN(MAX((a),(l)),(h)))
#define STRLEN(s)	(((s) != NULL) ? ((int)strlen(s)) : 0)

#define ISCR(c)		(((c) == '\n') || ((c) == '\r'))

#define IS_CHAR_ESC_START(c)	((c) == '<')
#define IS_CHAR_ESC_END(c)	((c) == '>')


/* Default spacings (in pixels) */
#define DEF_XMARGIN			5
#define DEF_YMARGIN			5

/* List object */
#define DEF_LIST_XMARGIN		25
#define DEF_LIST_YMARGIN		20

/* Message box object */
#define DEF_MB_XMARGIN			25
#define DEF_MB_YMARGIN			20

#define DEF_WIDTH			100
#define DEF_HEIGHT			100

#define DEF_PROGRESS_HEIGHT		25

#define DEF_SCROLL_CURSOR_WIDTH		16
#define DEF_SCROLL_CURSOR_HEIGHT	16

#define DEF_SWITCH_WIDTH		48
#define DEF_SWITCH_HEIGHT		48


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
 *	Resets gl states for 2d menu system drawing, this function
 *	should be called at startup and whenever the menu system is
 *	re-entered or needs the gl state to be reset to expected values.
 */
void SARMenuGLStateReset(gw_display_struct *display)
{
	state_gl_struct *s;

	if(display == NULL)
	    return;

	s = &display->state_gl;

	/* Set up GL states for 2d drawing */
	GWOrtho2D(display);

	StateGLDisableF(s, GL_DEPTH_TEST, True);
	StateGLDepthMask(s, GL_TRUE);
	StateGLDepthFunc(s, GL_ALWAYS);

	StateGLDisableF(s, GL_ALPHA_TEST, True);
	StateGLAlphaFunc(s, GL_GREATER, 0.5f);

	StateGLDisableF(s, GL_CULL_FACE, True);
	StateGLShadeModel(s, GL_FLAT);

	StateGLDisableF(s, GL_BLEND, True);
	StateGLDisableF(s, GL_FOG, True);
	StateGLDisableF(s, GL_TEXTURE_1D, True);
	StateGLDisableF(s, GL_TEXTURE_2D, True);
	StateGLDisableF(s, GL_COLOR_MATERIAL, True);
	StateGLDisableF(s, GL_LIGHTING, True);
	StateGLDisableF(s, GL_LIGHT0, True);
	StateGLDisableF(s, GL_LIGHT1, True);
	StateGLDisableF(s, GL_LIGHT2, True);
	StateGLDisableF(s, GL_LIGHT3, True);
	StateGLDisableF(s, GL_LIGHT4, True);
	StateGLDisableF(s, GL_LIGHT5, True);
	StateGLDisableF(s, GL_LIGHT6, True);
	StateGLDisableF(s, GL_LIGHT7, True);
	StateGLDisableF(s, GL_POINT_SMOOTH, True);
	StateGLDisableF(s, GL_LINE_SMOOTH, True);
	StateGLDisableF(s, GL_POLYGON_OFFSET_FILL, True);
	StateGLDisableF(s, GL_POLYGON_OFFSET_LINE, True);
	StateGLDisableF(s, GL_POLYGON_OFFSET_POINT, True);
	StateGLDisableF(s, GL_SCISSOR_TEST, True);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glPixelStorei(GL_PACK_ALIGNMENT, 1);
}


/*
 *	Returns the pointer to the object n on menu m.
 *
 *	Returns NULL on error.
 */
void *SARMenuGetObject(sar_menu_struct *m, int n)
{
	if(m == NULL)
	    return(NULL);
	else if((n < 0) || (n >= m->total_objects))
	    return(NULL);
	else
	    return(m->object[n]);
}

/*
 *      Returns the object who's id matches the specified id.
 */
void *SARMenuGetObjectByID(sar_menu_struct *m, int id, int *n)
{
	int i;
	void *o;

	if(n != NULL)
	    *n = -1;

	if(m == NULL)
	    return(NULL);

	for(i = 0; i < m->total_objects; i++)
	{
	    o = m->object[i];
	    if(o == NULL)
		continue;

	    switch(SAR_MENU_OBJECT_TYPE(o))
	    {
	      case SAR_MENU_OBJECT_TYPE_BUTTON:
		if(SAR_MENU_BUTTON(o)->id == id)
		{
		    if(n != NULL)
			*n = i;
		    return(o);
		}
		break;

	      case SAR_MENU_OBJECT_TYPE_LIST:
		if(SAR_MENU_LIST(o)->id == id)
		{
		    if(n != NULL)
			*n = i;
		    return(o);
		}
		break;

	      case SAR_MENU_OBJECT_TYPE_MESSAGE_BOX:
		if(SAR_MENU_MESSAGE_BOX(o)->id == id)
		{
		    if(n != NULL)
			*n = i;
		    return(o);
		}
		break;

	      case SAR_MENU_OBJECT_TYPE_MDISPLAY:
		if(SAR_MENU_MDISPLAY(o)->id == id)
		{
		    if(n != NULL)
			*n = i;
		    return(o);
		}
		break;

	      case SAR_MENU_OBJECT_TYPE_SWITCH:
		if(SAR_MENU_SWITCH(o)->id == id)
		{
		    if(n != NULL)
			*n = i;
		    return(o);
		}
		break;

	      case SAR_MENU_OBJECT_TYPE_SPIN:
		if(SAR_MENU_SPIN(o)->id == id)
		{
		    if(n != NULL)
			*n = i;
		    return(o);
		}
		break;

	      case SAR_MENU_OBJECT_TYPE_SLIDER:
		if(SAR_MENU_SLIDER(o)->id == id)
		{
		    if(n != NULL)
			*n = i;
		    return(o);
		}
		break;

	      case SAR_MENU_OBJECT_TYPE_MAP:
		if(SAR_MENU_MAP(o)->id == id)
		{
		    if(n != NULL)
			*n = i;
		    return(o);
		}
		break;

	      case SAR_MENU_OBJECT_TYPE_OBJVIEW:
		if(SAR_MENU_OBJVIEW(o)->id == id)
		{
		    if(n != NULL)
			*n = i;
		    return(o);
		}
		break;
	    }
	}

	return(NULL);
}

/*
 *	Returns True if the object n on menu m is sensitive.
 */
Boolean SARMenuObjectIsSensitive(sar_menu_struct *m, int n)
{
	void *o = SARMenuGetObject(m, n);
	if(o == NULL)
	    return(False);

	switch(SAR_MENU_OBJECT_TYPE(o))
	{
	  case SAR_MENU_OBJECT_TYPE_LABEL:
	    return(SAR_MENU_LABEL(o)->sensitive);
	  case SAR_MENU_OBJECT_TYPE_BUTTON:
	    return(SAR_MENU_BUTTON(o)->sensitive);
	  case SAR_MENU_OBJECT_TYPE_PROGRESS:
	    return(SAR_MENU_PROGRESS(o)->sensitive);
	  case SAR_MENU_OBJECT_TYPE_MESSAGE_BOX:
	    return(SAR_MENU_MESSAGE_BOX(o)->sensitive);
	  case SAR_MENU_OBJECT_TYPE_LIST:
	    return(SAR_MENU_LIST(o)->sensitive);
	  case SAR_MENU_OBJECT_TYPE_MDISPLAY:
	    return(SAR_MENU_MDISPLAY(o)->sensitive);
	  case SAR_MENU_OBJECT_TYPE_SWITCH:
	    return(SAR_MENU_SWITCH(o)->sensitive);
	  case SAR_MENU_OBJECT_TYPE_SPIN:
	    return(SAR_MENU_SPIN(o)->sensitive);
	  case SAR_MENU_OBJECT_TYPE_SLIDER:
	    return(SAR_MENU_SLIDER(o)->sensitive);
	  case SAR_MENU_OBJECT_TYPE_MAP:
	    return(SAR_MENU_MAP(o)->sensitive);
	  case SAR_MENU_OBJECT_TYPE_OBJVIEW:
	    return(SAR_MENU_OBJVIEW(o)->sensitive);
	  default:
	    return(False);
	}
}


/*
 *	Creates a new menu.
 */
sar_menu_struct *SARMenuNew(
	sar_menu_type type, const char *name, const char *bg_image
)
{
	sar_menu_struct *m = SAR_MENU(calloc(
	    1, sizeof(sar_menu_struct)
	));
	if(m == NULL)
	    return(NULL);

	m->type = type;
	m->name = STRDUP(name);
	m->always_full_redraw = False;
	m->bg_image_path = STRDUP(bg_image);
	m->bg_image = NULL;

	m->object = NULL;
	m->total_objects = 0;
	m->selected_object = -1;

	return(m);
}

/*
 *	Loads background image as needed on menu m.
 */
void SARMenuLoadBackgroundImage(sar_menu_struct *m)
{
	const char *path;

	if(m == NULL)
	    return;

	/* Get background image file path */
	path = m->bg_image_path;
	if(path == NULL)
	    return;

	/* Load new background image only if the menu does not already
	 * have one
	 */
	if(m->bg_image == NULL)
	    m->bg_image = SARImageNewFromFile(path);
}

/*
 *      Unloads background image as needed on menu m.
 */
void SARMenuUnloadBackgroundImage(sar_menu_struct *m)
{
	if(m == NULL)
	    return;

	SARImageDelete(m->bg_image);
	m->bg_image = NULL;
}

/*
 *	Deletes the Menu.
 */
void SARMenuDelete(sar_menu_struct *m)
{
	int i;

	if(m == NULL)
	    return;

	/* Delete all objects on this menu */
	for(i = 0; i < m->total_objects; i++)
	    SARMenuObjectDelete(m, i);
	free(m->object);
	m->object = NULL;
	m->total_objects = 0;

	free(m->name);
	m->name = NULL;

	free(m->bg_image_path);
	m->bg_image_path = NULL;

	SARImageDelete(m->bg_image);
	m->bg_image = NULL;

	free(m);
}

/*
 *	Allocates a new label on menu m.
 *
 *	Returns its index number or -1 on error.
 */
int SARMenuLabelNew(
	sar_menu_struct *m,
	float x, float y,
	int width, int height,
	const char *label,
	sar_menu_color_struct *fg_color,
	GWFont *font,
	const sar_image_struct *image
)
{
	int i, n;
	sar_menu_label_struct *label_ptr;


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
		return(-3);
	    }
	}

	/* Allocate structure */
	m->object[n] = label_ptr = SAR_MENU_LABEL(calloc(
	    1, sizeof(sar_menu_label_struct)
	));
	if(label_ptr == NULL)
	    return(-3);

	/* Load label values */
	label_ptr->type = SAR_MENU_OBJECT_TYPE_LABEL;
	label_ptr->x = x;
	label_ptr->y = y;
	label_ptr->width = width;
	label_ptr->height = height;
	label_ptr->sensitive = True;
	label_ptr->image = image;
	memcpy(&label_ptr->color, fg_color, sizeof(sar_menu_color_struct));
	label_ptr->font = font;
	label_ptr->label = STRDUP(label);
	label_ptr->align = SAR_MENU_LABEL_ALIGN_CENTER;

	/* Select this object if it is the first one */
	if(m->total_objects == 1)
	    m->selected_object = 0;

	return(n);
}

/*
 *	Allocates a new button on menu m.
 *
 *	Returns its index number or -1 on error.
 */
int SARMenuButtonNew(
	sar_menu_struct *m,
	float x, float y,
	int width, int height,
	const char *label,
	sar_menu_color_struct *fg_color, GWFont *font,
	const sar_image_struct *unarmed_image,
	const sar_image_struct *armed_image,
	const sar_image_struct *highlighted_image,
	void *client_data, int id,
	void (*func_cb)(void *, int, void *)
)
{
	int i, n;
	sar_menu_button_struct *button;


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
		return(-3);
	    }
	}

	/* Allocate structure */
	m->object[n] = button = SAR_MENU_BUTTON(calloc(
	    1, sizeof(sar_menu_button_struct)
	));
	if(button == NULL)
	    return(-3);

	/* Load button values */
	button->type = SAR_MENU_OBJECT_TYPE_BUTTON;
	button->x = x;
	button->y = y;
	button->width = width;
	button->height = height;
	button->sensitive = True;
	button->unarmed_image = unarmed_image;
	button->armed_image = armed_image;
	button->highlighted_image = highlighted_image;
	memcpy(&button->color, fg_color, sizeof(sar_menu_color_struct));
	button->font = font;
	button->label = STRDUP(label);
	button->client_data = client_data;
	button->id = id;
	button->func_cb = func_cb;

	/* Select this object if it is the first one */
	if(m->total_objects == 1)
	    m->selected_object = 0;

	return(n);
}

/*
 *      Allocates a new progress bar on menu m.
 *
 *      Returns its index number or -1 on error.
 */
int SARMenuProgressNew( 
	sar_menu_struct *m,
	float x, float y,
	int width, int height,
	const char *label,
	sar_menu_color_struct *fg_color, GWFont *font,
	const sar_image_struct *bg_image,
	const sar_image_struct *fg_image,
	float progress
)
{
	int i, n;
	sar_menu_progress_struct *pb;


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
		return(-3);
	    }
	}

	/* Allocate structure */
	m->object[n] = pb = SAR_MENU_PROGRESS(calloc(
	    1, sizeof(sar_menu_progress_struct)
	));
	if(pb == NULL)
	    return(-3);

	/* Load progress bar values */
	pb->type = SAR_MENU_OBJECT_TYPE_PROGRESS;
	pb->x = x;
	pb->y = y;
	pb->width = width;
	pb->height = height;
	pb->sensitive = True;
	pb->bg_image = bg_image;
	pb->fg_image = fg_image;
	memcpy(&pb->color, fg_color, sizeof(sar_menu_color_struct));
	pb->font = font;
	pb->label = STRDUP(label);
	pb->progress = progress;

	/* Select this object if it is the first one */
	if(m->total_objects == 1)
	    m->selected_object = 0;

	return(n);
}

/*
 *	Sets the new value for the progress bar.
 */
void SARMenuProgressSet(
	gw_display_struct *display, sar_menu_struct *m, int n,
	float progress, Boolean redraw
)
{
	Boolean changed = False;
	sar_menu_progress_struct *pb = SAR_MENU_PROGRESS(
	    SARMenuGetObject(m, n)
	);
	if(!SAR_MENU_IS_PROGRESS(pb))
	    return;

	if(pb->progress != progress)
	{
	    pb->progress = CLIP(progress, 0.0f, 1.0f);
	    changed = True;
	}

	if(changed && redraw)
	    DO_REDRAW_OBJECT(display, m, n)
}


/*
 *	Create message box on menu m.
 */
int SARMenuMessageBoxNew(
	sar_menu_struct *m,
	float x, float y,
	float width, float height,
	sar_menu_color_struct *fg_color,
	GWFont *font,
	const sar_image_struct **bg_image,	/* Pointer to 9 images */
	void *client_data, int id,
	const char *message
)
{
	int i, n;
	sar_menu_color_struct *c;
	sar_menu_message_box_struct *mesgbox;


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
		return(-3);
	    }
	}

	/* Allocate structure */
	m->object[n] = mesgbox = SAR_MENU_MESSAGE_BOX(calloc(
	    1, sizeof(sar_menu_message_box_struct)
	));
	if(mesgbox == NULL)
	    return(-3);

	/* Load message box values */
	mesgbox->type = SAR_MENU_OBJECT_TYPE_MESSAGE_BOX;
	mesgbox->id = id;
	mesgbox->client_data = client_data;
	mesgbox->x = x;
	mesgbox->y = y;
	mesgbox->width = width;
	mesgbox->height = height;
	mesgbox->sensitive = True;
	mesgbox->bg_image = bg_image;
	memcpy(&mesgbox->color, fg_color, sizeof(sar_menu_color_struct));
	c = &mesgbox->bold_color;
	c->a = 1.0f;
	c->r = 1.0f;
	c->g = 1.0f;
	c->b = 0.0f;
	c = &mesgbox->underline_color;
	c->a = 1.0f;
	c->r = 0.5f;
	c->g = 0.5f;
	c->b = 1.0f;
	mesgbox->font = font;
	mesgbox->message = STRDUP(message);
	mesgbox->scrolled_line = 0;

	/* Select this object if it is the first one */
	if(m->total_objects == 1)
	    m->selected_object = 0;

	return(n);
}

/*
 *	Set the message for the message box.
 */
void SARMenuMessageBoxSet(
	gw_display_struct *display, sar_menu_struct *m, int n,
	const char *message, Boolean redraw
)
{
	sar_menu_message_box_struct *mesgbox = SAR_MENU_MESSAGE_BOX(
	    SARMenuGetObject(m, n)
	);
	if(!SAR_MENU_IS_MESSAGE_BOX(mesgbox))
	    return;

	free(mesgbox->message);
	mesgbox->message = STRDUP(message);

	mesgbox->scrolled_line = 0;

	if(redraw)
	    DO_REDRAW_OBJECT(display, m, n)
}


/*
 *	Create list on menu m.
 *
 *	Returns its index number or -1 on error.
 */
int SARMenuListNew(
	sar_menu_struct *m,
	float x, float y,
	float width, float height,
	sar_menu_color_struct *fg_color, GWFont *font,
	const char *label,
	const sar_image_struct **bg_image,	/* Pointer to 9 images */
	void *client_data, int id,
	void (*select_cb)(void *, int, void *, int, void *, void *),
	void (*activate_cb)(void *, int, void *, int, void *, void *)
)
{
	int i, n;
	sar_menu_list_struct *list;


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
		return(-3);
	    }
	}

	/* Allocate structure */
	m->object[n] = list = SAR_MENU_LIST(calloc(
	    1, sizeof(sar_menu_list_struct)
	));
	if(list == NULL)
	    return(-3);

	/* Set values */
	list->type = SAR_MENU_OBJECT_TYPE_LIST;
	list->id = id;
	list->client_data = client_data;
	list->x = x;
	list->y = y;
	list->width = width;
	list->height = height;
	list->sensitive = True;
	list->bg_image = bg_image;
	memcpy(&list->color, fg_color, sizeof(sar_menu_color_struct));
	list->font = font;
	list->label = STRDUP(label);
	list->item = NULL;
	list->total_items = 0;
	list->items_visable = 0;	/* Calculated when drawn or managed */
	list->scrolled_item = 0;
	list->selected_item = -1;
	list->select_cb = select_cb;
	list->activate_cb = activate_cb;

	/* Select this object if it is the first one */
	if(m->total_objects == 1)
	    m->selected_object = 0;

	return(n);
}

/*
 *	Appends item to the list object n on menu m.
 *
 *	Returns the item number or -1 on error.
 */
int SARMenuListAppendItem(
	sar_menu_struct *m, int n,
	const char *name,
	void *client_data,
	sar_menu_flags_t flags
)
{
	int item_num;
	sar_menu_list_item_struct *item;
	sar_menu_list_struct *list = SAR_MENU_LIST(SARMenuGetObject(m, n));
	if(!SAR_MENU_IS_LIST(list))
	    return(-2);

	item_num = MAX(list->total_items, 0);
	list->total_items = item_num + 1;
	list->item = (sar_menu_list_item_struct **)realloc(
	    list->item,
	    list->total_items * sizeof(sar_menu_list_item_struct *)
	);
	if(list->item == NULL)
	{
	    list->total_items = 0;
	    return(-3);
	}

	list->item[item_num] = item = SAR_MENU_LIST_ITEM(calloc(
	    1, sizeof(sar_menu_list_item_struct)
	));
	if(item == NULL)
	    return(-3);

	/* Set item values */
	item->flags = flags;
	item->name = STRDUP(name);
	item->client_data = client_data;

	return(item_num);
}

/*
 *	Returns the pointer to the item number i or -1 on error.
 */
sar_menu_list_item_struct *SARMenuListGetItemByNumber(
	sar_menu_list_struct *list, int i
)
{
	if(list == NULL)
	    return(NULL);
	else if((i < 0) || (i >= list->total_items))
	    return(NULL);
	else
	    return(list->item[i]);
}

/*
 *	Selects the list item specified by i.
 */
void SARMenuListSelect(
	gw_display_struct *display, sar_menu_struct *m, int n,
	int i,
	Boolean scroll, Boolean redraw
)
{
	Boolean selection_changed = False;
	sar_menu_list_item_struct *item;
	sar_menu_list_struct *list = SAR_MENU_LIST(SARMenuGetObject(m, n));
	if(!SAR_MENU_IS_LIST(list))
	    return;

	if((i < 0) || (i >= list->total_items))
	    return;

	/* Change in selected item? */
	if(i != list->selected_item)
	{
	    list->selected_item = i;
	    selection_changed = True;
	}

	/* Scroll to selected item? */
	if(scroll)
	{
	    list->scrolled_item = i;
	    if((i + list->items_visable) > list->total_items)
		list->scrolled_item = MAX(
		    list->total_items - list->items_visable,
		    0
		);
	    selection_changed = True;
	}

	/* Redraw as needed and requested */
	if(selection_changed && redraw)
	    DO_REDRAW_OBJECT(display, m, n)

	/* Call select callback */
	item = SARMenuListGetItemByNumber(list, i);
	if((item != NULL) && selection_changed &&
	   (list->select_cb != NULL)
	)
	    list->select_cb(
		list,			/* This Object */
		list->id,		/* ID Code */
		list->client_data,	/* Data */
		list->selected_item,	/* Selected Item */
		item,			/* Item */
		item->client_data	/* Item Data */
	    );
}

/*
 *	Deletes all items allocated on list object n on menu m.
 *
 *	Client data on each item should have been taken care of
 *	by the calling function.
 */
void SARMenuListDeleteAllItems(sar_menu_struct *m, int n)
{
	int i;
	sar_menu_list_item_struct *item;
	sar_menu_list_struct *list = SAR_MENU_LIST(SARMenuGetObject(m, n));
	if(!SAR_MENU_IS_LIST(list))
	    return;

	/* Iterate through each item on the list object */
	for(i = 0; i < list->total_items; i++)
	{
	    item = list->item[i];
	    if(item == NULL)
		continue;

	    free(item->name);
	    item->name = NULL;

	    /* Calling function needs to free client_data */
	    if(item->client_data != NULL)
		fprintf(
		    stderr,
"SARMenuListDeleteAllItems(): Client data on item %i on list object %i of menu \"%s\" is not NULL.\n",
		    i, n, m->name
		);

	    free(item);
	    list->item[i] = item = NULL;
	}
	free(list->item);
	list->item = NULL;
	list->total_items = 0;
	list->scrolled_item = 0;
	list->selected_item = -1;
}


/*
 *	Create multi-purpose display object on menu m.
 *
 *	Returns its index number or -1 on error.
 */
int SARMenuMDisplayNew(
	sar_menu_struct *m,
	float x, float y,
	float width, float height,
	const sar_image_struct **bg_image,      /* Pointer to 9 images */
	void *client_data, int id,
	void (*draw_cb)(
		void *,         /* Display */
		void *,         /* Menu */
		void *,         /* This Object */
		int,            /* ID Code */
		void *,         /* Data */
		int, int, int, int      /* x_min, y_min, x_max, y_max */
	)
)
{
	int i, n;
	sar_menu_mdisplay_struct *mdpy;


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
		return(-3);
	    }
	}

	/* Allocate structure */
	m->object[n] = mdpy = SAR_MENU_MDISPLAY(calloc(
	    1, sizeof(sar_menu_mdisplay_struct)
	));
	if(mdpy == NULL)
	   return(-3);

	/* Set mdisplay values */
	mdpy->type = SAR_MENU_OBJECT_TYPE_MDISPLAY;
	mdpy->x = x;
	mdpy->y = y;
	mdpy->width = width;
	mdpy->height = height;
	mdpy->sensitive = True;
	mdpy->bg_image = bg_image;
	mdpy->client_data = client_data;
	mdpy->id = id;
	mdpy->draw_cb = draw_cb;

	/* Select this object if it is the first one */
	if(m->total_objects == 1)
	    m->selected_object = 0;

	return(n);
}

/*
 *	Redraws the multi-purpose display.
 */
void SARMenuMDisplayDraw(
	gw_display_struct *display, sar_menu_struct *m, int n
)
{

}


/*
 *	Create switch object on menu m.
 *
 *	Returns its index number or -1 on error.
 */
int SARMenuSwitchNew(
	sar_menu_struct *m,
	float x, float y,
	int width, int height,
	sar_menu_color_struct *fg_color, GWFont *font,
	const char *label,
	const sar_image_struct *bg_image,
	const sar_image_struct *switch_off_image,
	const sar_image_struct *switch_on_image,
	Boolean state,
	void *client_data, int id,
	void (*switch_cb)(void *, int, void *, Boolean)
)
{
	int i, n;
	sar_menu_switch_struct *sw;


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
		return(-3);
	    }
	}

	/* Allocate structure */ 
	m->object[n] = sw = SAR_MENU_SWITCH(calloc(
	    1, sizeof(sar_menu_switch_struct)
	));
	if(sw == NULL)  
	    return(-3);

	/* Set switch values */
	sw->type = SAR_MENU_OBJECT_TYPE_SWITCH;
	sw->x = x;
	sw->y = y;
	sw->width = width;
	sw->height = height;
	sw->sensitive = True;
	sw->bg_image = bg_image;
	sw->switch_off_image = switch_off_image;
	sw->switch_on_image = switch_on_image;
	memcpy(&sw->color, fg_color, sizeof(sar_menu_color_struct));
	sw->font = font;
	sw->label = STRDUP(label);
	sw->state = state;
	sw->client_data = client_data;
	sw->id = id;
	sw->switch_cb = switch_cb;

	/* Select this object if it is the first one */
	if(m->total_objects == 1)
	    m->selected_object = 0;

	return(n);
}

/*
 *	Get state of the switch object n on menu m.
 */
Boolean SARMenuSwitchGetValue(sar_menu_struct *m, int n)
{
	sar_menu_switch_struct *sw = SAR_MENU_SWITCH(
	    SARMenuGetObject(m, n)
	);
	if(!SAR_MENU_IS_SWITCH(sw))
	    return(False);

	return(sw->state);
}

/*
 *	Set state of switch object n on menu m.
 */
void SARMenuSwitchSetValue(
	gw_display_struct *display, sar_menu_struct *m, int n,
	Boolean state, Boolean redraw
)
{
	Boolean change = False;
	sar_menu_switch_struct *sw = SAR_MENU_SWITCH(
	    SARMenuGetObject(m, n)
	);
	if(!SAR_MENU_IS_SWITCH(sw))
	    return;

	if(sw->state != state)
	{
	    sw->state = state;
	    change = True;
	}

	if(change)
	{
	    if(redraw)
		DO_REDRAW_OBJECT(display, m, n)

	    if(sw->switch_cb != NULL)
		sw->switch_cb(
		    sw,			/* This Object */
		    sw->id,		/* ID Code */
		    sw->client_data,	/* Data */
		    sw->state		/* Value */
		);
	}
}


/*
 *	Create spin object on menu m.
 *
 *	Returns the object's index or -1 on error.
 */
int SARMenuSpinNew(
	sar_menu_struct *m,
	float x, float y,
	float width, float height,
	sar_menu_color_struct *label_color,
	sar_menu_color_struct *value_color,
	GWFont *font, GWFont *value_font,
	const char *label,
	const sar_image_struct *label_image,
	const sar_image_struct *value_image,
	const sar_image_struct *dec_armed_image,
	const sar_image_struct *dec_unarmed_image,
	const sar_image_struct *inc_armed_image,
	const sar_image_struct *inc_unarmed_image,
	void *client_data, int id,
	void (*change_cb)(void *, int, void *, char *)
)
{
	int i, n;
	sar_menu_spin_struct *spin;


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
		return(-3);
	    }
	}

	/* Allocate structure */
	m->object[n] = spin = SAR_MENU_SPIN(calloc(
	    1, sizeof(sar_menu_spin_struct)
	));
	if(spin == NULL)
	    return(-3);

	/* Set spin values */
	spin->type = SAR_MENU_OBJECT_TYPE_SPIN;
	spin->x = x;
	spin->y = y;
	spin->width = width;
	spin->height = height;
	spin->sensitive = True;
	spin->label_image = label_image;
	spin->value_image = value_image;
	spin->dec_armed_image = dec_armed_image;
	spin->dec_unarmed_image = dec_unarmed_image;
	spin->inc_armed_image = inc_armed_image;
	spin->inc_unarmed_image = inc_unarmed_image;
	memcpy(&spin->label_color, label_color, sizeof(sar_menu_color_struct));
	memcpy(&spin->value_color, value_color, sizeof(sar_menu_color_struct));
	spin->font = font;
	spin->value_font = value_font;
	spin->dec_state = SAR_MENU_BUTTON_STATE_UNARMED;
	spin->inc_state = SAR_MENU_BUTTON_STATE_UNARMED;
	spin->label = STRDUP(label);
	spin->value_type = SAR_MENU_SPIN_TYPE_STRING;
	spin->allow_warp = True;
	spin->step = 1.0;
	spin->value = NULL;
	spin->total_values = 0;
	spin->cur_value = -1;
	spin->client_data = client_data;
	spin->id = id;
	spin->change_cb = change_cb;

	/* Select this object if it is the first one */
	if(m->total_objects == 1)
	    m->selected_object = 0;

	return(n);
}

/*
 *	Set value type for spin object n on menu m.
 */
void SARMenuSpinSetValueType(
	sar_menu_struct *m, int n, int type
)
{
	sar_menu_spin_struct *spin = SAR_MENU_SPIN(
	    SARMenuGetObject(m, n)
	);
	if(!SAR_MENU_IS_SPIN(spin))
	    return;

	spin->value_type = type;
}

/*
 *	Adds value to spin object n on menu m.
 *
 *	If the spin object is set to SAR_MENU_SPIN_TYPE_NUMERIC then
 *	this will update its current value, otherwise a new value will
 *	be appended to the spin object's list of values.
 */
int SARMenuSpinAddValue(
	sar_menu_struct *m, int n, const char *value
)
{
	int vn;
	sar_menu_spin_struct *spin = SAR_MENU_SPIN(
	    SARMenuGetObject(m, n)
	);
	if(!SAR_MENU_IS_SPIN(spin))
	    return(-1);

	/* Handle by value type */
	switch(spin->value_type)
	{
	  case SAR_MENU_SPIN_TYPE_STRING:
	    /* Allocate one more pointer */
	    vn = MAX(spin->total_values, 0);
	    spin->total_values = vn + 1;
	    spin->value = (char **)realloc(
		spin->value,
		spin->total_values * sizeof(char *)
	    );
	    if(spin->value == NULL)
	    {
		spin->total_values = 0;
		spin->cur_value = -1;
		return(-3);
	    }
	    /* Now value_num should be a valid pointer, allocate a new
	     * value for it
	     */
	    spin->value[vn] = STRDUP(value);
	    break;

	  case SAR_MENU_SPIN_TYPE_NUMERIC:
	    /* Have at least one value? */
	    if(spin->total_values < 1)
	    {
		/* Need to allocate just one value */
		vn = 0;
		spin->total_values = vn + 1;
		spin->value = (char **)realloc(
		    spin->value,
		    spin->total_values * sizeof(char *)
		);
		if(spin->value == NULL)
		{
		    spin->total_values = 0;
		    spin->cur_value = -1;
		    return(-3);
		}

		spin->value[vn] = NULL;
	    }
	    else
	    {
		vn = 0;
	    }
	    /* Now value_num should be a valid pointer, allocate a new
	     * value string for it.
	     */
	    free(spin->value[vn]);
	    spin->value[vn] = STRDUP(value);
	    break;
	}

	/* Update current value */
	if(spin->total_values > 0)
	{
	    if(spin->cur_value < 0)
		spin->cur_value = 0;
	}

	return(0);
}

/*
 *	Returns the Spin's current value.
 */
char *SARMenuSpinGetCurrentValue(
	sar_menu_struct *m, int n, int *sel_num
)
{
	int vn;
	sar_menu_spin_struct *spin = SAR_MENU_SPIN(
	    SARMenuGetObject(m, n)
	);

	if(sel_num != NULL)
	    *sel_num = -1;

	if(!SAR_MENU_IS_SPIN(spin))
	    return(NULL);

	vn = spin->cur_value;
	if((vn < 0) || (vn >= spin->total_values))
	{
	    return(NULL);
	}
	else
	{
	    if(sel_num != NULL)
		*sel_num = vn;
	    return(spin->value[vn]);
	}
}

/*
 *	Spins the spin to the value specified by value_num.
 */
void SARMenuSpinSelectValueIndex(
	gw_display_struct *display, sar_menu_struct *m, int n,
	int value_num,
	Boolean redraw
)
{
	sar_menu_spin_struct *spin = SAR_MENU_SPIN(
	    SARMenuGetObject(m, n)
	);
	if(!SAR_MENU_IS_SPIN(spin))
	    return;

	if((value_num < 0) || (value_num >= spin->total_values))
	    return;

	if(value_num != spin->cur_value)
	{
	    spin->cur_value = value_num;

	    if(redraw)
		DO_REDRAW_OBJECT(display, m, n)

	    if(spin->change_cb != NULL)
		spin->change_cb(
		    spin,			/* This Object */
		    spin->id,			/* ID Code */
		    spin->client_data,		/* Data */
		    spin->value[value_num]	/* Value */
		);
	}
}

/*
 *	Sets the spin value specified by value_num to the new value
 *	specified by value.
 */
void SARMenuSpinSetValueIndex(
	gw_display_struct *display, sar_menu_struct *m, int n,
	int value_num, const char *value,
	Boolean redraw
)
{
	Boolean need_change = False;
	sar_menu_spin_struct *spin = SAR_MENU_SPIN(
	    SARMenuGetObject(m, n)
	);
	if(!SAR_MENU_IS_SPIN(spin))
	    return;

	if((value_num < 0) || (value_num >= spin->total_values))
	    return;

	if(value == NULL)
	    value = "";

	if(spin->value[value_num] != NULL)
	    need_change = (strcmp(spin->value[value_num], value)) ?
		True : False;
	else
	    need_change = True;
	if(need_change)
	{
	    free(spin->value[value_num]);
	    spin->value[value_num] = STRDUP(value);

	    if(redraw)
		DO_REDRAW_OBJECT(display, m, n)

	    if(spin->change_cb != NULL)
		spin->change_cb(
		    spin,			/* This Object */
		    spin->id,			/* ID Code */
		    spin->client_data,		/* Data */
		    spin->value[value_num]	/* Value */
		);
	}
}

/*
 *	Deletes all values on spin n on menu m.
 */
void SARMenuSpinDeleteAllValues(sar_menu_struct *m, int n)
{
	sar_menu_spin_struct *spin = SAR_MENU_SPIN(
	    SARMenuGetObject(m, n)
	);
	if(!SAR_MENU_IS_SPIN(spin))
	    return;

	strlistfree(spin->value, spin->total_values);
	spin->value = NULL;
	spin->total_values = 0;
	spin->cur_value = -1;
}

/*
 *	Spin increment procedure.
 */
void SARMenuSpinDoInc(
	gw_display_struct *display, sar_menu_struct *m, int n,
	Boolean redraw
)
{
	int prev_value_num;
	char *value_ptr;
	sar_menu_spin_struct *spin = SAR_MENU_SPIN(
	    SARMenuGetObject(m, n)
	);
	if((display == NULL) || (spin == NULL))
	    return;

	if(!SAR_MENU_IS_SPIN(spin))
	    return;

	/* Record previous value number */
	prev_value_num = spin->cur_value;

	/* Increment by value type */
	switch(spin->value_type)
	{
	  case SAR_MENU_SPIN_TYPE_STRING:
	    if(spin->allow_warp ? True :
		(spin->cur_value < (spin->total_values - 1))
	    )
	    {
		spin->cur_value++;
		if(spin->cur_value >= spin->total_values)
		    spin->cur_value = 0;

		if(redraw)
		    DO_REDRAW_OBJECT(display, m, n)

		if((spin->cur_value >= 0) &&
		   (spin->cur_value < spin->total_values)
		)
		    value_ptr = spin->value[spin->cur_value];
		else
		    value_ptr = NULL;

		/* Call change callback? */
		if((spin->change_cb != NULL) &&
		   (prev_value_num != spin->cur_value) &&
		   (value_ptr != NULL)
		)
		    spin->change_cb(
			spin,			/* This Object */
			spin->id,		/* ID Code */
			spin->client_data,	/* Data */
			value_ptr		/* Value */
		    );
	    }
	    break;

	  case SAR_MENU_SPIN_TYPE_NUMERIC:
/* TODO */
	    break;
	}
}

/*
 *	Spin increment procedure.
 */
void SARMenuSpinDoDec(
	gw_display_struct *display, sar_menu_struct *m, int n,
	Boolean redraw
)
{
	int prev_value_num;
	char *value_ptr;
	sar_menu_spin_struct *spin = SAR_MENU_SPIN(
	    SARMenuGetObject(m, n)
	);
	if((display == NULL) || (spin == NULL))
	    return;

	if(!SAR_MENU_IS_SPIN(spin))
	    return;

	/* Record previous value number */
	prev_value_num = spin->cur_value;

	/* Decrement by value type */
	switch(spin->value_type)
	{
	  case SAR_MENU_SPIN_TYPE_STRING:
	    if(spin->allow_warp ? True :
		(spin->cur_value > 0)
	    )
	    {
		spin->cur_value--;
		if(spin->cur_value < 0)
		    spin->cur_value = spin->total_values - 1;

		if(redraw)
		    DO_REDRAW_OBJECT(display, m, n)

		if((spin->cur_value >= 0) &&
		   (spin->cur_value < spin->total_values)
		)
		    value_ptr = spin->value[spin->cur_value];
		else
		    value_ptr = NULL;

		/* Call change callback? */
		if((spin->change_cb != NULL) &&
		   (prev_value_num != spin->cur_value) &&
		   (value_ptr != NULL)
		)
		    spin->change_cb(
			spin,			/* This Object */
			spin->id,		/* ID Code */
			spin->client_data,	/* Data */
			value_ptr		/* Value */
		    );
	    }
	    break;

	  case SAR_MENU_SPIN_TYPE_NUMERIC:
/* TODO */
	    break;
	}
}


/*
 *      Create slider object on menu m.
 *
 *      Returns the object's index or -1 on error.
 */
int SARMenuSliderNew(
	sar_menu_struct *m,
	float x, float y,
	float width, float height,
	sar_menu_color_struct *label_color,
	GWFont *font,
	const char *label,
	const sar_image_struct *label_image,
	const sar_image_struct *trough_image,
	const sar_image_struct *handle_image,
	void *client_data, int id,
	void (*change_cb)(void *, int, void *, float)
)
{
	int i, n;
	sar_menu_slider_struct *slider;


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
		return(-3);
	    }
	}

	/* Allocate structure */
	m->object[n] = slider = SAR_MENU_SLIDER(calloc(
	    1, sizeof(sar_menu_slider_struct)
	));
	if(slider == NULL)
	    return(-3);

	/* Set slider values */
	slider->type = SAR_MENU_OBJECT_TYPE_SLIDER;
	slider->x = x;
	slider->y = y;
	slider->width = width;
	slider->height = height;
	slider->sensitive = True;
	slider->label_image = label_image;
	slider->trough_image = trough_image;
	slider->handle_image = handle_image;
	memcpy(&slider->label_color, label_color, sizeof(sar_menu_color_struct));
	slider->font = font;
	slider->label = STRDUP(label);
	slider->value = 0.0f;
	slider->lower = 0.0f;
	slider->upper = 1.0f;
	slider->drag_button = 0;
	slider->client_data = client_data;
	slider->id = id;
	slider->change_cb = change_cb;

	/* Select this object if it is the first one */
	if(m->total_objects == 1)
	    m->selected_object = 0;

	return(n);
}

/*
 *	Sets the lower and upper value bounds for the slider.
 */
void SARMenuSliderSetValueBounds(
	gw_display_struct *display, sar_menu_struct *m, int n,
	float lower, float upper,
	Boolean redraw
)
{
	Boolean change = False;
	sar_menu_slider_struct *slider = SAR_MENU_SLIDER(
	    SARMenuGetObject(m, n)
	);
	if((display == NULL) || (slider == NULL))
	    return;

	if(!SAR_MENU_IS_SLIDER(slider))
	    return;

	/* Set value only if the new value is different */
	if((slider->lower != lower) || (slider->upper != upper))
	{
	    slider->upper = upper;
	    slider->lower = lower;
	    slider->value = CLIP(slider->value, slider->lower, slider->upper);
	    change = True;
	}

	/* Check if there is a change in the value, if there is then
	 * redraw and call change callback.
	 */
	if(change)
	{
	    if(redraw)
		DO_REDRAW_OBJECT(display, m, n)

	    if(slider->change_cb != NULL)
		slider->change_cb(
		    slider,			/* This Object */
		    slider->id,			/* ID Code */
		    slider->client_data,	/* Data */
		    slider->value		/* Value */
		);
	}
}

/*
 *	Sets the value for the slider.
 */
void SARMenuSliderSetValue(
	gw_display_struct *display, sar_menu_struct *m, int n,
	float value,
	Boolean redraw
)
{
	Boolean change = False;
	sar_menu_slider_struct *slider = SAR_MENU_SLIDER(
	    SARMenuGetObject(m, n)
	);
	if((display == NULL) || (slider == NULL))
	    return;

	if(!SAR_MENU_IS_SLIDER(slider))
	    return;

	/* Set value only if the new value is different */
	if(slider->value != value)
	{
	    slider->value = CLIP(value, slider->lower, slider->upper);
	    change = True;
	}

	/* Check if there is a change in the value, if there is then
	 * redraw and call change callback
	 */
	if(change)
	{
	    if(redraw)
		DO_REDRAW_OBJECT(display, m, n)

	    if(slider->change_cb != NULL)
		slider->change_cb(
		    slider,			/* This Object */
		    slider->id,			/* ID Code */
		    slider->client_data,	/* Data */
		    slider->value		/* Value */
		);
	}
}

/*
 *	Returns the current value of the slider.
 */
float SARMenuSliderGetValue(sar_menu_struct *m, int n)
{
	sar_menu_slider_struct *slider = SAR_MENU_SLIDER(
	    SARMenuGetObject(m, n)
	);
	if(slider == NULL)
	    return(0.0f);

	if(!SAR_MENU_IS_SLIDER(slider))
	    return(0.0f);

	return(slider->value);
}


/*
 *	Sets object n sensitive or insensitive.
 */
void SARMenuObjectSetSensitive(
	gw_display_struct *display, sar_menu_struct *m, int n,
	Boolean sensitive, Boolean redraw
)
{
	sar_menu_label_struct *label_ptr;
	sar_menu_button_struct *button;
	sar_menu_progress_struct *progress;
	sar_menu_message_box_struct *mesgbox;
	sar_menu_list_struct *list;
	sar_menu_mdisplay_struct *mdpy;
	sar_menu_switch_struct *sw;
	sar_menu_spin_struct *spin;
	sar_menu_slider_struct *slider;
	sar_menu_map_struct *map;
	sar_menu_objview_struct *objview;
	Boolean changed = False;
	void *o = SARMenuGetObject(m, n);
	if(o == NULL)
	    return;

	switch(SAR_MENU_OBJECT_TYPE(o))
	{
	  case SAR_MENU_OBJECT_TYPE_LABEL:
	    label_ptr = SAR_MENU_LABEL(o);
	    if(label_ptr->sensitive != sensitive)
	    {
		label_ptr->sensitive = sensitive;
		changed = True;
	    }
	    break;
	  case SAR_MENU_OBJECT_TYPE_BUTTON:
	    button = SAR_MENU_BUTTON(o);
	    if(button->sensitive != sensitive)
	    {
		button->sensitive = sensitive;
		changed = True;
	    }
	    break;
	  case SAR_MENU_OBJECT_TYPE_PROGRESS:
	    progress = SAR_MENU_PROGRESS(o);
	    if(progress->sensitive != sensitive)
	    {
		progress->sensitive = sensitive;
		changed = True;
	    }
	    break;
	  case SAR_MENU_OBJECT_TYPE_MESSAGE_BOX:
	    mesgbox = SAR_MENU_MESSAGE_BOX(o);
	    if(mesgbox->sensitive != sensitive)
	    {
		mesgbox->sensitive = sensitive;
		changed = True;
	    }
	    break;
	  case SAR_MENU_OBJECT_TYPE_LIST:
	    list = SAR_MENU_LIST(o);
	    if(list->sensitive != sensitive)
	    {
		list->sensitive = sensitive;
		changed = True;
	    }
	    break;
	 case SAR_MENU_OBJECT_TYPE_MDISPLAY:
	    mdpy = SAR_MENU_MDISPLAY(o);
	    if(mdpy->sensitive != sensitive)
	    {
		mdpy->sensitive = sensitive;
		changed = True;
	    }
	    break;
	  case SAR_MENU_OBJECT_TYPE_SWITCH:
	    sw = SAR_MENU_SWITCH(o);
	    if(sw->sensitive != sensitive)
	    {
		sw->sensitive = sensitive;
		changed = True;
	    }
	    break;
	  case SAR_MENU_OBJECT_TYPE_SPIN:
	    spin = SAR_MENU_SPIN(o);
	    if(spin->sensitive != sensitive)
	    {
		spin->sensitive = sensitive;
		changed = True;
	    }
	    break;
	  case SAR_MENU_OBJECT_TYPE_SLIDER:
	    slider = SAR_MENU_SLIDER(o);
	    if(slider->sensitive != sensitive)
	    {
		slider->sensitive = sensitive;
		changed = True;
	    }
	    break;
	  case SAR_MENU_OBJECT_TYPE_MAP:
	    map = SAR_MENU_MAP(o);
	    if(map->sensitive != sensitive)
	    {
		map->sensitive = sensitive;
		changed = True;
	    }
	    break;
	  case SAR_MENU_OBJECT_TYPE_OBJVIEW:
	    objview = SAR_MENU_OBJVIEW(o);
	    if(objview->sensitive != sensitive)
	    {
		objview->sensitive = sensitive;
		changed = True;
	    }
	    break;
	}

	if(redraw && changed)
	    DO_REDRAW_OBJECT(display, m, n)
}

/*
 *	Destroys object n on menu m.
 */
void SARMenuObjectDelete(sar_menu_struct *m, int n)
{
	void *o = SARMenuGetObject(m, n);
	if(o != NULL)
	{
	    sar_menu_object_type type;
	    sar_menu_label_struct *label_ptr;
	    sar_menu_button_struct *button;
	    sar_menu_progress_struct *progress_ptr;
	    sar_menu_message_box_struct *mesgbox;
	    sar_menu_list_struct *list;
	    sar_menu_mdisplay_struct *mdpy;
	    sar_menu_switch_struct *sw;
	    sar_menu_spin_struct *spin;
	    sar_menu_slider_struct *slider;
	    sar_menu_map_struct *map;
	    sar_menu_objview_struct *objview;


	    type = SAR_MENU_OBJECT_TYPE(o);
	    switch(type)
	    {
	      case SAR_MENU_OBJECT_TYPE_LABEL:
		label_ptr = SAR_MENU_LABEL(o);
		label_ptr->image = NULL;	/* Shared */
		free(label_ptr->label);
		free(label_ptr);
		break;

	      case SAR_MENU_OBJECT_TYPE_BUTTON:
		button = SAR_MENU_BUTTON(o);
		button->unarmed_image = NULL;	/* Shared */
		button->armed_image = NULL;		/* Shared */
		button->highlighted_image = NULL;	/* Shared */
		free(button->label);
		free(button);
		break;

	      case SAR_MENU_OBJECT_TYPE_PROGRESS:
		progress_ptr = SAR_MENU_PROGRESS(o);
		progress_ptr->bg_image = NULL;	/* Shared */
		progress_ptr->fg_image = NULL;	/* Shared */
		free(progress_ptr->label);
		free(progress_ptr);
		break;

	      case SAR_MENU_OBJECT_TYPE_MESSAGE_BOX:
		mesgbox = SAR_MENU_MESSAGE_BOX(o);
		mesgbox->bg_image = NULL;	/* Shared */
		free(mesgbox->message);
		free(mesgbox);
		break;

	      case SAR_MENU_OBJECT_TYPE_LIST:
		list = SAR_MENU_LIST(o);
		SARMenuListDeleteAllItems(m, n);
		list->bg_image = NULL;	/* Shared */
		free(list->item);
		free(list->label);
		free(list);
		break;

	      case SAR_MENU_OBJECT_TYPE_MDISPLAY:
		mdpy = SAR_MENU_MDISPLAY(o);
		mdpy->bg_image = NULL;	/* Shared */
		free(mdpy);
		break;

	      case SAR_MENU_OBJECT_TYPE_SWITCH:
		sw = SAR_MENU_SWITCH(o);
		sw->bg_image = NULL;		/* Shared */
		sw->switch_off_image = NULL;	/* Shared */
		sw->switch_on_image = NULL;	/* Shared */
		free(sw->label);
		free(sw);
		break;

	      case SAR_MENU_OBJECT_TYPE_SPIN:
		spin = SAR_MENU_SPIN(o);
		SARMenuSpinDeleteAllValues(m, n);
		spin->label_image = NULL;	/* Shared */
		spin->value_image = NULL;	/* Shared */
		spin->dec_armed_image = NULL;	/* Shared */
		spin->dec_unarmed_image = NULL;	/* Shared */
		spin->inc_armed_image = NULL;	/* Shared */
		spin->inc_unarmed_image = NULL;	/* Shared */
		free(spin->label);
		free(spin);
		break;

	      case SAR_MENU_OBJECT_TYPE_SLIDER:
		slider = SAR_MENU_SLIDER(o);
		slider->label_image = NULL;	/* Shared */
		slider->trough_image = NULL;	/* Shared */
		slider->handle_image = NULL;	/* Shared */
		free(slider->label);
		free(slider);
		break;

	      case SAR_MENU_OBJECT_TYPE_MAP:
		map = SAR_MENU_MAP(o);
		SARMenuMapDeleteAllMarkings(map);
		V3DTextureDestroy(map->bg_tex);
		map->bg_tex = NULL;
		free(map->title);
		map->title = NULL;
		free(map);
		break;

	      case SAR_MENU_OBJECT_TYPE_OBJVIEW:
		objview = SAR_MENU_OBJVIEW(o);
		free(objview->title);
		objview->title = NULL;
		free(objview->message);
		objview->message = NULL;
		if(objview->obj_list != NULL)
		{
		    GLuint list = (GLuint)objview->obj_list;
		    glDeleteLists(list, 1);
		    objview->obj_list = NULL;
		}
		V3DGLResourceDelete(
		    (v3d_glresource_struct *)objview->obj_glres
		);
		objview->obj_glres = NULL;
		free(objview->objimg);
		objview->objimg = NULL;
		free(objview);
		break;


	      default:
		fprintf(
		    stderr,
 "SARMenuObjectDelete(): Destroying unknown type `%i'\n",
		    type
		);
		free(o);
		break;
	    }

	    m->object[n] = NULL;
	}
}


/*
 *	Draws one line of string starting at buf_ptr and does not
 *	draw more than the number of whole word characters specified by
 *	visible_columns.
 *
 *	Returns the pointer to the next word that lies within buf_ptr
 *	or NULL if a '\0' character was encountered while drawing the 
 *	string.
 */
static const char *SARMenuMessageBoxDrawString(
	gw_display_struct *display,
	int x, int y,
	const char *buf_ptr,
	int visible_columns, int font_width,
	Boolean sensitive,
	Boolean draw_line,
	const sar_menu_color_struct *c_def,
	const sar_menu_color_struct *c_bold,
	const sar_menu_color_struct *c_ul
)
{
	char c;
	int characters_drawn;
	const char *s, *buf_stop;


	if((display == NULL) || (buf_ptr == NULL) || (visible_columns < 1))
	    return(NULL);

	/* If given buffer points to a end of buffer then return NULL */
	if(*buf_ptr == '\0')
	    return(NULL);

	/* If given string points to a newline then just return the 
	 * pointer to the next character. This will effectivly draw one
	 * empty line.
	 */
	if(ISCR(*buf_ptr))
	    return(buf_ptr + 1);

	/* Seek past initial spaces */
	while(ISBLANK(*buf_ptr))
	    buf_ptr++;

	/* Probe to end of line and mark it by setting buf_stop to point 
	 * to that end.
	 */
	buf_stop = NULL;
	s = buf_ptr;
	for(characters_drawn = 0; characters_drawn < visible_columns;)
	{
	    c = *s;

	    /* End of buffer? */
	    if(c == '\0')
	    {
		buf_stop = s;
		break;
	    }
	    /* Blank character? */
	    else if(ISBLANK(c))
	    {
		/* Mark end of buffer at this blank character and count it
		 * as one character drawn (regardless of how many blank
		 * characters follow).
		 */
		buf_stop = s;
		characters_drawn++;

		/* Seek past this and any following blank characters */
		while(ISBLANK(*s))
		    s++;
	    }
	    /* New line? */
	    else if(ISCR(c))
	    {
		buf_stop = s;
		break;
	    }
	    /* Escape sequence? */
	    else if(IS_CHAR_ESC_START(c))
	    {
		/* Skip this character */
		s++;

		/* Seek to end of escape sequence or end of buffer */
		while((*s != '\0') &&
		      !IS_CHAR_ESC_END(*s)
		)
		    s++;
		if(*s != '\0')
		    s++;
	    }
	    /* Regular character */
	    else
	    {
		/* Mark this as a character to be drawn and increment to
		 * next character.
		 */
		characters_drawn++;
		s++;
	    }
	}

	/* If end of buffer was not found then assume it is at s */
	if(buf_stop == NULL)
	    buf_stop = s;


	/* Draw from buf_ptr to (but not including) buf_stop */
	characters_drawn = 0;
	s = buf_ptr;
	while(s < buf_stop)
	{
	    c = *s;

	    /* End of buffer? */
	    if(c == '\0')
	    {
		break;
	    }
	    /* Blank character? */
	    else if(ISBLANK(c))
	    {
		/* Draw one blank character and seek to next non-blank
		 * character (regardless of how many blank characters
		 * follow).
		 */
		if(draw_line)
		    GWDrawCharacter(
			display,
			x + (characters_drawn * font_width),
			y,
			c
		    );
		characters_drawn++;

		/* Seek past this and any following blank characters */
		while(ISBLANK(*s))
		    s++;
	    }
	    /* Escape sequence? */
	    else if(IS_CHAR_ESC_START(c))
	    {
		int n;
		char tag_name[256];


		/* Skip this character */
		s++;

		/* Seek to end of escape sequence or end of buffer */
		n = 0;
		while((*s != '\0') &&
		      !IS_CHAR_ESC_END(*s)
		)
		{
		    if(n < 254)
		    {
			tag_name[n] = *s;
			n++;
		    }
		    s++;
		}
		if(*s != '\0')
		    s++;

		/* Null terminate tag_name */
		tag_name[n] = '\0';

		/* Handle tag */
		/* Default? */
		if(!strcasecmp(tag_name, "default") ||
		   !strcasecmp(tag_name, "plain") ||
		   !strcasecmp(tag_name, "regular")
		)
		{
		    if(c_def != NULL)
		    {
			if(sensitive)
			    glColor4f(
				c_def->r,
				c_def->g,
				c_def->b,
				c_def->a
			    );
			else
			    glColor4f(
				c_def->r * 0.75f,
				c_def->g * 0.75f,
				c_def->b * 0.75f,
				c_def->a
			    );
		    }
		}
		/* Bold? */
		else if(!strcasecmp(tag_name, "bold"))
		{
		    if(c_bold != NULL)
		    {
			if(sensitive)
			    glColor4f(
				c_bold->r,
				c_bold->g,
				c_bold->b,
				c_bold->a
			    );
			else
			    glColor4f(
				c_bold->r * 0.75f,
				c_bold->g * 0.75f,
				c_bold->b * 0.75f,
				c_bold->a
			    );
		    }
		}
		/* Underline? */
		else if(!strcasecmp(tag_name, "underline") ||
		        !strcasecmp(tag_name, "underlined")
		)
		{
		    if(c_ul != NULL)
		    {
			if(sensitive)
			    glColor4f(
				c_ul->r,
				c_ul->g,
				c_ul->b,
				c_ul->a
			    );
			else
			    glColor4f(
				c_ul->r * 0.75f,
				c_ul->g * 0.75f,
				c_ul->b * 0.75f,
				c_ul->a
			    );
		    }
		}

	    }
	    else
	    {
		/* Actually draw this character? */
		if(draw_line)
		    GWDrawCharacter(
			display,
			x + (characters_drawn * font_width),
			y,
			c
		    );
		characters_drawn++;
		s++;
	    }
	}

	/* Return the pointer to the next character that was not
	 * drawn.
	 */
	return(buf_stop);
}

/*
 *	Returns the total number of lines by seeking through the given
 *	buffer.
 */
static int SARMenuMessageBoxTotalLines(
	const char *buf_ptr, int visible_columns
)
{
	char c;
	int line_count = 0, characters_drawn;
	const char *s, *buf_stop;


	if((buf_ptr == NULL) || (visible_columns < 1))
	    return(line_count);

	/* Iterate until end of buffer */
	while((buf_ptr != NULL) ? (*buf_ptr != '\0') : False)
	{
	    /* Set s to point to the start of current line */
	    s = buf_ptr;

	    /* If s is on a newline then seek to next character and
	     * count this as one line.
	     */
	    if(ISCR(*s))
	    {
		buf_ptr = s + 1;
		line_count++;
		continue;
	    }

	    /* Seek past initial spaces */
	    while(ISBLANK(*s))
		s++;

	    /* Reset stop buffer and seek to end of `line'. Remember that
	     * characters_drawn represents the number of characters that
	     * would be drawn to fit on one line with whole words. Not the
	     * the actual characters drawn since nothing is being drawn in
	     * this function.
	     */
	    buf_stop = NULL;
	    for(characters_drawn = 0; characters_drawn < visible_columns;)
	    {
		c = *s;

		/* End of buffer? */
		if(c == '\0')
		{
		    buf_stop = s;
		    break;
		}
		/* Blank character? */
		else if(ISBLANK(c))
		{
		    /* Count blank character as one character drawn and 
		     * seek to next non-blank character.
		     */
		    buf_stop = s;
		    characters_drawn++;

		    /* Seek past this and any following blank characters */
		    while(ISBLANK(*s))
			s++;
		}
		/* New line? */
		else if(ISCR(c))
		{
		    buf_stop = s;
		    break;
		}
		/* Escape sequence? */
		else if(IS_CHAR_ESC_START(c))
		{
		    /* Skip this character */
		    s++;

		    /* Seek to end of escape sequence or end of buffer */
		    while((*s != '\0') &&
			  !IS_CHAR_ESC_END(*s)
		    )
			s++;
		    if(*s != '\0')
			s++;
		}
		/* Regular character */
		else
		{
		    /* Mark this as a character to be drawn and increment
		     * to next character.
		     */
		    characters_drawn++;
		    s++;
		}
	    }
	    /* If not able to find end of line pointer then assume end of
	     * line is at current position of s.
	     */
	    if(buf_stop == NULL)
		buf_stop = s;

	    /* End of line reached, count this as one line and set buffer
	     * to start of next line.
	     */
	    line_count++;
	    buf_ptr = buf_stop;
	}

	return(line_count);
}

/*
 *	Redraws a `window' comprised of 9 images where the first image
 *	(index 0) is the background and the second image (index 1) through
 *	the 9th image (index 8) are the 8 decoration edges starting from
 *	the upper left corner.
 */
void SARMenuDrawWindowBG(
	gw_display_struct *display,
	const sar_image_struct **bg_image,      /* Total 9 images */
	const sar_menu_color_struct *base_color,	/* Set NULL to use image as base */
	int x, int y,
	int width, int height,
	Boolean draw_base,
	Boolean draw_shadow,
	Boolean is_selected
)
{
	int corner_width = 0, corner_height = 0;
	int vframe_width = 0, hframe_height = 0;
	const sar_image_struct *img;
	int win_width, win_height;


	if((display == NULL) || (bg_image == NULL))
	    return;

	GWContextGet(
	    display, GWContextCurrent(display),
	    NULL, NULL,
	    NULL, NULL,
	    &win_width, &win_height
	);

	/* Get upper left corner image */
	img = bg_image[1];
	if(img != NULL)
	{
	    /* Get size of each corner by assuming they're all the same
	     * and take the size of the upper left corner image.
	     */
	    corner_width = MAX(img->width, 0);
	    corner_height = MAX(img->height, 0);
	}

	/* Get upper horizontal frame image */
	img = bg_image[2];
	if(img != NULL)
	{
	    /* Get height of both horizontal frames by taking the size of
	     * the upper horizontal frame image.
	     */
	    hframe_height = MAX(img->height, 0);
	}

	/* Get right vertical frame image */
	img = bg_image[4];
	if(img != NULL)
	{
	    /* Get width of both vertical frames by taking the size of
	     * the right vertical frame image.
	     */
	    vframe_width = MAX(img->width, 0);
	}

	/* Adjust given geometry to fit `with in' the decorations */
	x += corner_width;
	y += corner_height;
	width = MAX(width - (2 * corner_width), 0);
	height = MAX(height - (2 * corner_height), 0);

	if((width < 1) || (height < 1))
	    return;

	/* Draw shadow */
	if(draw_shadow)
	{
	    StateGLBoolean	alpha_test = display->state_gl.alpha_test,
				blend = display->state_gl.blend;

	    /* Set up GL states */
	    StateGLDisable(&display->state_gl, GL_ALPHA_TEST);
	    StateGLEnable(&display->state_gl, GL_BLEND);
	    StateGLBlendFunc(
		&display->state_gl, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA
	    );
	    glColor4f(0.0f, 0.0f, 0.0f, 0.25f);

	    glBegin(GL_QUADS);
	    {
		glVertex2i(
		    (GLint)(x + width + (0.5 * corner_width)),
		    (GLint)(win_height - (y + height + (1.5 * corner_height)))
		);
		glVertex2i(
		    (GLint)(x + width + (1.5 * corner_width)),
		    (GLint)(win_height - (y + height + (1.5 * corner_height)))
		);
		glVertex2i(
		    (GLint)(x + width + (1.5 * corner_width)),
		    (GLint)(win_height - (y - (0.5 * corner_height)))
		);
		glVertex2i(
		    (GLint)(x + width + (0.5 * corner_width)),
		    (GLint)(win_height - (y - (0.5 * corner_height)))
		);

		glVertex2i(
		    (GLint)(x - (0.5 * corner_width)),
		    (GLint)(win_height - (y + height + (1.5 * corner_height)))
		);
		glVertex2i(
		    (GLint)(x + width + (0.5 * corner_width)),
		    (GLint)(win_height - (y + height + (1.5 * corner_height)))
		);
		glVertex2i(
		    (GLint)(x + width + (0.5 * corner_width)),
		    (GLint)(win_height - (y + height + (0.5 * corner_height)))
		);
		glVertex2i(
		    (GLint)(x - (0.5 * corner_width)),
		    (GLint)(win_height - (y + height + (0.5 * corner_height)))
		);
	    }
	    glEnd();

	    /* Restore GL states */
	    if(alpha_test)
		StateGLEnable(&display->state_gl, GL_ALPHA_TEST);
	    if(!blend)
		StateGLDisable(&display->state_gl, GL_BLEND);
	}

	/* Draw base image at center, scaled to fit the width and height
	 * regardless of the actual image size
	 */
	if(draw_base)
	{
	    if(base_color != NULL)
	    {
		glColor4f(
		    base_color->r,
		    base_color->g,
		    base_color->b,
		    base_color->a
		);
		glBegin(GL_QUADS);
		{
		    glVertex2i(x - 1, win_height - (y - 1));
		    glVertex2i(x - 1, win_height - (y + height + 2));
		    glVertex2i(x + width + 2, win_height - (y + height + 2));
		    glVertex2i(x + width + 2, win_height - (y - 1));
		}
		glEnd();
	    }
	    else
	    {
		img = bg_image[0];
		SARImageDraw(
		    display, img,
		    x - 1, y - 1, width + 2, height + 2
		);
	    }

	    /* Draw selection rectangle? */
	    if(is_selected)
	    {
		glColor4f(0.0f, 0.5f, 0.0f, 1.0f);
		glBegin(GL_LINE_LOOP);
		{
		    int margin = 1;
		    glVertex2i(x + margin, win_height - (y + margin));
		    glVertex2i(x + margin, win_height - (y + height - margin));
		    glVertex2i(x + width - margin, win_height - (y + height - margin));
		    glVertex2i(x + width - margin, win_height - (y + margin));
 		}
		glEnd();
	    }
	}

	/* Draw frame decorations (not including corners) next */
	img = bg_image[2];
	SARImageDraw(
	    display, img,
	    x - 1, y - hframe_height,
	    width + 2, hframe_height
	);

	img = bg_image[4];
	SARImageDraw(
	    display, img,
	    x + width, y - 1,
	    vframe_width, height + 2
	);

	img = bg_image[6];
	SARImageDraw(
	    display, img,
	    x - 1, y + height,
	    width + 2, hframe_height
	);

	img = bg_image[8];
	SARImageDraw(
	    display, img,
	    x - vframe_width, y - 1,
	    vframe_width, height + 2
	);


	/* Draw corners */
	img = bg_image[1];
	SARImageDraw(
	    display, img,
	    x - corner_width, y - corner_height,
	    corner_width, corner_height
	);

	img = bg_image[3];
	SARImageDraw(
	    display, img,
	    x + width, y - corner_height,
	    corner_width, corner_height
	);

	img = bg_image[5];
	SARImageDraw(
	    display, img,
	    x + width, y + height,
	    corner_width, corner_height
	);

	img = bg_image[7];
	SARImageDraw(
	    display, img,
	    x - corner_width, y + height,
	    corner_width, corner_height
	);
}


/*
 *	Draws the specified object n.
 */
static void SARMenuDoDrawObject(
	gw_display_struct *display, sar_menu_struct *m,
	int n,                  /* Object number on m */
	Boolean draw_shadows
)
{
	Boolean draw_selected_rect = True, is_selected;
	int i, x, y, xc, yc, w = 10, h = 10, width, height, len, type;
	int fw, fh;
	int items_visable, chars_per_row;
	const char *cstrptr;
	const sar_menu_color_struct *c;
	void *o;
	sar_menu_label_struct *label_ptr;
	sar_menu_button_struct *button;
	sar_menu_progress_struct *progress_ptr;
	sar_menu_message_box_struct *mesgbox;
	sar_menu_list_struct *list;
	sar_menu_mdisplay_struct *mdpy;
	sar_menu_switch_struct *sw;
	sar_menu_spin_struct *spin;
	sar_menu_slider_struct *slider;
	sar_menu_map_struct *map;
	sar_menu_objview_struct *objview;

	static u_int8_t scroll_cursor_up_bm[] = {
0xff, 0xff, 0x80, 0x01, 0xbf, 0xfd, 0xbf, 0xfd, 0x9f, 0xf9, 0x9f, 0xf9,
0x8f, 0xf1, 0x8f, 0xf1, 0x87, 0xe1, 0x87, 0xe1, 0x83, 0xc1, 0x83, 0xc1,
0x81, 0x81, 0x81, 0x81, 0x80, 0x01, 0xff, 0xff
	}; 

	static u_int8_t scroll_cursor_down_bm[] = {
0xff, 0xff, 0x80, 0x01, 0x81, 0x81, 0x81, 0x81, 0x83, 0xc1, 0x83, 0xc1,
0x87, 0xe1, 0x87, 0xe1, 0x8f, 0xf1, 0x8f, 0xf1, 0x9f, 0xf9, 0x9f, 0xf9,
0xbf, 0xfd, 0xbf, 0xfd, 0x80, 0x01, 0xff, 0xff
	};          
#if 0
	static u_int8_t scroll_cursor_left_bm[] = {
0xff, 0xff, 0x80, 0x01, 0x80, 0x0d, 0x80, 0x3d, 0x80, 0xfd, 0x83, 0xfd,
0x8f, 0xfd, 0xbf, 0xfd, 0xbf, 0xfd, 0x8f, 0xfd, 0x83, 0xfd, 0x80, 0xfd,
0x80, 0x3d, 0x80, 0x0d, 0x80, 0x01, 0xff, 0xff
	};
#endif
#if 0
	static u_int8_t scroll_cursor_right_bm[] = {
0xff, 0xff, 0x80, 0x01, 0xb0, 0x01, 0xbc, 0x01, 0xbf, 0x01, 0xbf, 0xc1,
0xbf, 0xf1, 0xbf, 0xfd, 0xbf, 0xfd, 0xbf, 0xf1, 0xbf, 0xc1, 0xbf, 0x01,
0xbc, 0x01, 0xb0, 0x01, 0x80, 0x01, 0xff, 0xff
	};
#endif
	const sar_image_struct *image;


	if((display == NULL) || (m == NULL))
	    return;

	o = SARMenuGetObject(m, n);
	if(o == NULL)
	    return;

	type = SAR_MENU_OBJECT_TYPE(o);

	is_selected = (m->selected_object == n) ? True : False;

	GWContextGet(
	    display, GWContextCurrent(display),
	    NULL, NULL,
	    NULL, NULL,
	    &width, &height
	);

#define DO_DRAW_IMAGE		\
{ if(image != NULL) {		\
 SARImageDraw(			\
  display, image,		\
  (int)(x - (w / 2)),		\
  (int)(y - (h / 2)),		\
  w, h				\
 );				\
} }

	switch(type)
	{
	  /* ******************************************************** */
	  case SAR_MENU_OBJECT_TYPE_LABEL:
	    label_ptr = SAR_MENU_LABEL(o);
	    w = label_ptr->width;
	    h = label_ptr->height;
	    x = (int)(label_ptr->x * width) + label_ptr->offset_x;
	    y = (int)(label_ptr->y * height) + label_ptr->offset_y;

	    /* Draw label background image */
	    image = label_ptr->image;
	    DO_DRAW_IMAGE	    

	    /* Draw label text */
	    if(label_ptr->label != NULL)
	    {
		const char *s = label_ptr->label;
		int	xp, yp,
			lines = strlines(s),
			longest_line = strlongestline(s);

		GWGetFontSize(label_ptr->font, NULL, NULL, &fw, &fh);

		/* Autocalculate width and height as needed */
		if(w <= 0)
		    w = fw * longest_line;
		if(h <= 0)
		    h = fh * lines;

		/* Set label text font and color */
	        GWSetFont(display, label_ptr->font);
		c = &label_ptr->color;
		if(label_ptr->sensitive)
		    glColor4f(c->r, c->g, c->b, c->a);
		else
		    glColor4f(
			c->r * 0.75f,
			c->g * 0.75f,
			c->b * 0.75f,
			c->a
		    );

		/* Get starting position to draw label text */
		xp = x - (fw * longest_line / 2);
		yp = y - (fh * lines / 2);

		/* Draw each line of the label text*/
		while(*s != '\0')
		{
		    /* New line? */
		    if(ISCR(*s))
		    {
			/* Restore x position and increment y position
			 * to the next line
			 */
			switch(label_ptr->align)
			{
			  case SAR_MENU_LABEL_ALIGN_RIGHT:
			    s++;
			    xp = x + (fw * longest_line / 2) -
				(fw * strlinelen(s));
			    yp += fh;
			    break;
			  case SAR_MENU_LABEL_ALIGN_LEFT:
			    s++;
			    xp = x - (fw * longest_line / 2);
			    yp += fh;
			    break;
			  default:	/* SAR_MENU_LABEL_ALIGN_CENTER */
			    s++;
			    xp = x - (fw * strlinelen(s) / 2);
			    yp += fh;
			    break;
			}
		    }
		    else
		    {
			/* Draw current character */
			GWDrawCharacter(display, xp, yp, *s);
			s++;
			xp += fw;
		    }
		}
	    }
	    break;

	  /* ******************************************************** */
	  case SAR_MENU_OBJECT_TYPE_BUTTON:
	    button = SAR_MENU_BUTTON(o);
	    w = button->width;
	    h = button->height;
	    x = (int)(button->x * width) + button->offset_x;
	    y = (int)(button->y * height) + button->offset_y;
	    /* Select image to use based on button state */
	    switch(button->state)
	    {
	      case SAR_MENU_BUTTON_STATE_HIGHLIGHTED: 
		image = button->highlighted_image;
		break;

	      case SAR_MENU_BUTTON_STATE_ARMED:
		image = button->armed_image;
		break;

	      default:
		image = button->unarmed_image;
		break;
	    }
	    /* If image is not available, then fall back to unarmed
	     * image.
	     */
	    if(image == NULL)
		image = button->unarmed_image;
	    DO_DRAW_IMAGE

	    /* Draw label */
	    cstrptr = button->label;
	    if(cstrptr != NULL)
	    {
		int x_min, x_max, y_min, y_max;

		x_min = x - (w / 2);
		x_max = x_min + w;
		y_min = y - (h / 2);
		y_max = y_min + h;

		len = STRLEN(cstrptr);
		GWGetFontSize(button->font, NULL, NULL, &fw, &fh);
		GWSetFont(display, button->font);
		c = &button->color;
		if(button->sensitive)
		    glColor4f(c->r, c->g, c->b, c->a);
		else
		    glColor4f(
			c->r * 0.75f,
			c->g * 0.75f,
			c->b * 0.75f,
			c->a
		    );
		GWDrawString(
		    display,
		    x - (len * fw / 2),
		    y - (fh / 2),
		    cstrptr
		);

		/* Do not draw outlines for buttons */
	    }
	    break;

	  /* ******************************************************** */
	  case SAR_MENU_OBJECT_TYPE_PROGRESS:
	    progress_ptr = SAR_MENU_PROGRESS(o);

	    x = (int)(progress_ptr->x * width);
	    y = (int)(progress_ptr->y * height);

	    if(progress_ptr->width > 0)
		w = progress_ptr->width;
	    else
/*		w = width - (2 * DEF_XMARGIN); */
		w = width;		/* Butt left to right end */

	    if(progress_ptr->height > 0)
		h = progress_ptr->height;
	    else
		h = DEF_PROGRESS_HEIGHT;

	    xc = MAX(x - (w / 2), 0);
	    yc = MAX(y - (h / 2), 0);
	    if((yc + h) > height)
		yc = height - h;

	    /* Draw progress background */
	    SARImageDraw(display, progress_ptr->bg_image, xc, yc, w, h);
#if 0
	    if(GL_TRUE)
	    {
		StateGLBoolean alpha_test = display->state_gl.alpha_test;
		StateGLBoolean blend = display->state_gl.blend;

		/* Set up GL states */
		StateGLDisable(&display->state_gl, GL_ALPHA_TEST);
		StateGLEnable(&display->state_gl, GL_BLEND);
		StateGLBlendFunc(
		    &display->state_gl, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA
		);

		glColor4f(0.0f, 0.0f, 0.0f, 0.25f);
		glBegin(GL_QUADS);
		{
		    glVertex2i(xc, height - yc);
		    glVertex2i(xc, height - (yc + h));
		    glVertex2i(xc + w, height - (yc + h));
		    glVertex2i(xc + w, height - yc);
		}
		glEnd();

		/* Restore GL states */
		if(alpha_test)
		    StateGLEnable(&display->state_gl, GL_ALPHA_TEST);
		if(!blend)
		    StateGLDisable(&display->state_gl, GL_BLEND);
	    }
#endif
	    /* Draw progress foreground */
	    SARImageDraw(
		display, progress_ptr->fg_image,
		xc, yc,
		(int)(w * progress_ptr->progress), h
	    );
	    break;

	  /* ******************************************************** */
	  case SAR_MENU_OBJECT_TYPE_MESSAGE_BOX:
	    mesgbox = SAR_MENU_MESSAGE_BOX(o);

	    x = (int)(mesgbox->x * width);
	    y = (int)(mesgbox->y * height);

	    w = (int)(mesgbox->width * width);
	    if(w <= 0)
		w = width - (2 * DEF_MB_XMARGIN);
	    if(w <= 0)
		w = DEF_MB_XMARGIN;

	    if(mesgbox->height > 0)
		h = (int)(mesgbox->height * height);
	    else
		h = DEF_HEIGHT;
	    if(h <= 0)
		h = DEF_MB_YMARGIN;

	    xc = MAX(x - (w / 2), 0);
	    yc = MAX(y - (h / 2), 0);

	    SARMenuDrawWindowBG(
		display, mesgbox->bg_image, NULL,
		xc, yc,
		w, h,
		True,		/* Draw base */
		draw_shadows,	/* Draw shadow */
		(Boolean)((draw_selected_rect && is_selected) ? True : False)
	    );

	    GWGetFontSize(
		mesgbox->font,
		NULL, NULL,
		&fw, &fh
	    );

	    GWSetFont(display, mesgbox->font);
	    c = &mesgbox->color;
	    if(mesgbox->sensitive)
		glColor4f(c->r, c->g, c->b, c->a);
	    else
		glColor4f(
		    c->r * 0.75f,
		    c->g * 0.75f,
		    c->b * 0.75f,
		    c->a
		);

	    /* Has message to be drawn? */
	    if(mesgbox->message != NULL)
	    {
		Boolean actually_draw_line;
		const char *buf_ptr;
		int lines_processed = 0, lines_drawn = 0;
		int lines_to_skip = mesgbox->scrolled_line;
		int mesg_x, mesg_y,
		    lines_visable, columns_visable, scroll_width,
		    total_lines;
		const sar_menu_color_struct	*c_b = &mesgbox->bold_color,
						*c_u = &mesgbox->underline_color;

		/* Calculate width (height) of scroll indicator */
		scroll_width = fh + (2 * DEF_MB_YMARGIN);

		/* Calculate lines visable */
		if(fh > 0)
		    lines_visable = (h - (2 * DEF_MB_YMARGIN)) / fh;
		else
		    lines_visable = 0;
		if(lines_visable < 0)
		    lines_visable = 0;

		/* Calculate columns visable (include margin for arrows) */
		if(fw > 0)
		    columns_visable = (w - DEF_SCROLL_CURSOR_WIDTH
			- (2 * DEF_MB_XMARGIN)) / fw;
		else
		    columns_visable = 1;
		/* Must have atleast one column visible */
		if(columns_visable < 1)
		    columns_visable = 1;

		/* Draw scroll up and scroll down arrows */
		glRasterPos2i(
		    xc + w - DEF_SCROLL_CURSOR_WIDTH - DEF_MB_XMARGIN,
		    height - (yc + DEF_SCROLL_CURSOR_HEIGHT + DEF_MB_YMARGIN)
		);
		glBitmap(
		    DEF_SCROLL_CURSOR_WIDTH,
		    DEF_SCROLL_CURSOR_HEIGHT,
		    0.0, 0.0,
		    DEF_SCROLL_CURSOR_WIDTH, 0.0,
		    scroll_cursor_up_bm 
		);
		glRasterPos2i(
		    xc + w - DEF_SCROLL_CURSOR_WIDTH - DEF_MB_XMARGIN,
		    height - (y + (h / 2) - DEF_MB_YMARGIN)
		);
		glBitmap(
		    DEF_SCROLL_CURSOR_WIDTH,
		    DEF_SCROLL_CURSOR_HEIGHT,
		    0.0, 0.0,
		    DEF_SCROLL_CURSOR_WIDTH, 0.0,
		    scroll_cursor_down_bm
		);

		/* Get pointer to start of message buffer and begin
		 * drawing lines.
		 */
		buf_ptr = mesgbox->message;
		lines_processed = 0;
		mesg_x = xc + DEF_MB_XMARGIN;
		mesg_y = yc + DEF_MB_YMARGIN;
		do
		{
		    actually_draw_line = (lines_processed >= lines_to_skip) ?
			True : False;

		    buf_ptr = SARMenuMessageBoxDrawString(
			display,
			mesg_x, mesg_y,
			buf_ptr,
			columns_visable, fw,
			mesgbox->sensitive,
			actually_draw_line,
			c,		/* Foreground color */
			c_b,		/* Bold color */
			c_u		/* Underline color */
		    );
		    lines_processed++;

		    if(actually_draw_line)
		    {
			lines_drawn++;
			mesg_y += fh;
		    }
		}
		while((buf_ptr != NULL) && (lines_drawn < lines_visable));

		/* Calculate total number of lines */
		total_lines = SARMenuMessageBoxTotalLines(
		    mesgbox->message, columns_visable
		);

		/* Draw scroll indicator (not draggable, only a position indicator) */
		int scroll_box_left, scroll_box_right, scroll_box_bottom, scroll_box_top, indicator_top, indicator_bottom;
		float indicator_max_height, indicator_height, indicator_top_blank_space;
		scroll_box_left = xc + w - DEF_MB_XMARGIN - DEF_SCROLL_CURSOR_WIDTH;
		scroll_box_right = xc + w - DEF_MB_XMARGIN - 1;
		scroll_box_bottom = height - (y + (h / 2) - (DEF_MB_YMARGIN + DEF_SCROLL_CURSOR_HEIGHT) + 1);
		scroll_box_top = height - (yc + DEF_SCROLL_CURSOR_HEIGHT + DEF_MB_YMARGIN) - 1;
        
		glBegin(GL_LINES);
		/* Draw left border line */
		glVertex2i(
		    scroll_box_left,
		    scroll_box_top + 1
		);
		glVertex2i(
		    scroll_box_left,
		    scroll_box_bottom
		);
		/* Draw right border line */
		glVertex2i(
		    scroll_box_right,
		    scroll_box_top + 1
		);
		glVertex2i(
		    scroll_box_right,
		    scroll_box_bottom
		);
		glEnd();

		indicator_max_height = scroll_box_top - scroll_box_bottom - 1 - 1; /* one "blank" line at indicator top and bottom */
		indicator_height = indicator_max_height * ((float)lines_visable / (float)total_lines);
		if ( indicator_height > indicator_max_height )
		    indicator_height = indicator_max_height;
		else if ( indicator_height < 1.0 )
		    indicator_height = 1.0; /* at least 1 pixel height */
		indicator_top_blank_space = (float)lines_to_skip * (indicator_height / (float)lines_visable);
		if ( lines_to_skip == 0 )
		    indicator_top_blank_space = 0.0;
        
		indicator_top = scroll_box_top - (int)indicator_top_blank_space;
		if ( (indicator_top > scroll_box_top - 1) || (lines_to_skip == 0) )
		    indicator_top = scroll_box_top - 1;
		indicator_bottom = indicator_top - (int)indicator_height;
		if ( (indicator_bottom <= scroll_box_bottom + 1) || (total_lines == lines_to_skip + lines_visable) )
		indicator_bottom = scroll_box_bottom + 2;

		if ( (indicator_bottom > scroll_box_bottom) && (indicator_top < scroll_box_top) )
		{
		    /* Draw indicator */
		    glRecti(
			scroll_box_right - 2,
			indicator_top,
			scroll_box_left + 1,
			indicator_bottom
		    );
		}
	    }
	    break;

	  /* ******************************************************** */
	  case SAR_MENU_OBJECT_TYPE_LIST:
	    list = SAR_MENU_LIST(o);

	    x = (int)(list->x * width);
	    y = (int)(list->y * height);

	    w = (int)(list->width * width);
	    if(w <= 0)
		w = width - (2 * DEF_LIST_XMARGIN);
	    if(w <= 0)
		w = DEF_LIST_XMARGIN;

	    if(list->height > 0)
		h = (int)(list->height * height);
	    else
		h = DEF_HEIGHT;
	    if(h <= 0)
		h = DEF_LIST_YMARGIN;

	    xc = MAX(x - (w / 2), 0);
	    yc = MAX(y - (h / 2), 0);

	    SARMenuDrawWindowBG(
		display, list->bg_image, NULL,
		xc, yc,
		w, h,
		True,		/* Draw base */
		draw_shadows,	/* Draw shadow */
		(Boolean)((draw_selected_rect && is_selected) ? True : False)
	    );

	    GWGetFontSize(
		list->font,
		NULL, NULL,
		&fw, &fh
	    );
	    /* Calculate items visible, remember to subtract three
	     * items for borders and heading.
	     */
	    if(fh > 0)
		items_visable = MAX(
		    (h / fh) - 3,
		    0
		);
	    else  
		items_visable = 0;
	    /* Update items visable on list */
	    list->items_visable = items_visable;
	    /* Recalculate scroll position if too great */
	    if((list->total_items - list->scrolled_item) <
		items_visable
	    )
	        list->scrolled_item = MAX(
		    list->total_items - items_visable,
		    0
		);

	    /* Calculate len as lines visable + yc */
            /* Substract fh as otherwise one line is 
               drawn on the bottom margin */
	    len = yc + h - fh;

	    /* Calculate characters per row */
	    if(fw > 0)
		chars_per_row = MAX(
		    (w - DEF_SCROLL_CURSOR_WIDTH -
			(3 * DEF_LIST_XMARGIN)) / fw,
		    1
		);
	    else
		chars_per_row = 1;

	    /* Draw heading label */
	    yc += DEF_LIST_YMARGIN;
	    GWSetFont(display, list->font);
	    c = &list->color;
	    if(list->sensitive)
		glColor4f(c->r, c->g, c->b, c->a);
	    else
		glColor4f(
		    c->r * 0.75f,
		    c->g * 0.75f,
		    c->b * 0.75f,
		    c->a
		);
	    GWDrawString(   
		display,  
		xc + DEF_LIST_XMARGIN, yc,
		list->label
	    );
	    yc += fh;

	    /* Draw scroll up and down arrows */
	    glRasterPos2i(
		xc + w - DEF_SCROLL_CURSOR_WIDTH - DEF_LIST_XMARGIN,
		height - (yc + DEF_SCROLL_CURSOR_HEIGHT)
	    );
	    glBitmap(
		DEF_SCROLL_CURSOR_WIDTH,
		DEF_SCROLL_CURSOR_HEIGHT,
		0.0, 0.0,
		DEF_SCROLL_CURSOR_WIDTH, 0.0,
		scroll_cursor_up_bm
	    );
	    glRasterPos2i(
		xc + w - DEF_SCROLL_CURSOR_WIDTH - DEF_LIST_XMARGIN,
		height - (y + (h / 2) - DEF_LIST_YMARGIN)
	    );
	    glBitmap(
		DEF_SCROLL_CURSOR_WIDTH,
		DEF_SCROLL_CURSOR_HEIGHT,
		0.0, 0.0,
		DEF_SCROLL_CURSOR_WIDTH, 0.0,
		scroll_cursor_down_bm
	    );
        
	    /* Sanitize scrolled item position */
	    if(list->scrolled_item < 0)
		list->scrolled_item = 0;
        
	    /* Draw scroll indicator (not draggable, only a position indicator) */
	    int scroll_box_left, scroll_box_right, scroll_box_bottom, scroll_box_top, indicator_top, indicator_bottom;
	    float indicator_max_height, indicator_height, indicator_top_blank_space;
	    scroll_box_left = xc + w - DEF_LIST_XMARGIN - DEF_SCROLL_CURSOR_WIDTH;
	    scroll_box_right = xc + w - DEF_LIST_XMARGIN - 1;
	    scroll_box_bottom = height - (y + (h / 2) - (DEF_LIST_YMARGIN + DEF_SCROLL_CURSOR_HEIGHT) + 1);
	    scroll_box_top = height - (yc + DEF_SCROLL_CURSOR_HEIGHT) - 1;
	    glBegin(GL_LINES);
	    /* Draw left border line */
	    glVertex2i(
		scroll_box_left,
		scroll_box_top + 1
	    );
	    glVertex2i(
		scroll_box_left,
		scroll_box_bottom
	    );
	    /* Draw right border line */
	    glVertex2i(
		scroll_box_right,
		scroll_box_top + 1
	    );
	    glVertex2i(
		scroll_box_right,
		scroll_box_bottom
	    );
	    glEnd();

	    indicator_max_height = scroll_box_top - scroll_box_bottom - 1 - 1; /* one "blank" line at indicator top and bottom */
	    indicator_height = indicator_max_height * ((float)list->items_visable / (float)list->total_items);
	    if ( indicator_height > indicator_max_height )
		indicator_height = indicator_max_height;
	    else if ( indicator_height < 1.0 )
		indicator_height = 1.0; /* at least 1 pixel height */
	    indicator_top_blank_space = (float)list->scrolled_item * (indicator_height / (float)list->items_visable);
	    if ( list->scrolled_item == 0 )
		indicator_top_blank_space = 0.0;
        
	    indicator_top = scroll_box_top - (int)indicator_top_blank_space;
	    if ( (indicator_top > scroll_box_top - 1) || (list->scrolled_item == 0) )
		indicator_top = scroll_box_top - 1;
	    indicator_bottom = indicator_top - (int)indicator_height;
	    if ( (indicator_bottom <= scroll_box_bottom + 1) || (list->total_items == list->scrolled_item + list->items_visable) )
		indicator_bottom = scroll_box_bottom + 2;

	    if ( (indicator_bottom > scroll_box_bottom) && (indicator_top < scroll_box_top) )
	    {
		/* Draw indicator */
		glRecti(
		    scroll_box_right - 2,
		    indicator_top,scroll_box_left + 1,
		    indicator_bottom
		);
	    }

	    /* Draw each item */
	    for(i = list->scrolled_item; i < list->total_items; i++)
	    {
#define item_name_len	256
		char item_name[item_name_len];
		sar_menu_list_item_struct *item = list->item[i];
		if((item != NULL) ? (item->name == NULL) : True)
		    continue;

		if((yc + fh) >= len)
		    break;

		/* Copy item name and shorten it as needed */
		strncpy(item_name, item->name, item_name_len);
		if(chars_per_row < item_name_len)
		    item_name[chars_per_row] = '\0';
		else
		    item_name[item_name_len - 1] = '\0';

		/* Draw item name */
		GWDrawString(
		    display,
		    xc + (1 * fw) + DEF_LIST_XMARGIN,
		    yc,
		    item_name
		);
		/* If this item is selected then draw selection marker
		 * in front of the item's name.
		 */
		if(i == list->selected_item)
		    GWDrawString(
		    display, 
		    xc + DEF_LIST_XMARGIN, yc,
		    ">"
		);

		yc += fh;
#undef item_name_len
	    }
	    break;

	  /* ******************************************************** */
	  case SAR_MENU_OBJECT_TYPE_MDISPLAY:
	    mdpy = SAR_MENU_MDISPLAY(o);

	    x = (int)(mdpy->x * width);
	    y = (int)(mdpy->y * height);

	    w = (int)(mdpy->width * width);
	    if(w <= 0)
		w = width - (2 * DEF_LIST_XMARGIN);
	    if(w <= 0)
		w = DEF_LIST_XMARGIN;

	    if(mdpy->height > 0)
		h = (int)(mdpy->height * height);
	    else
		h = DEF_HEIGHT;
	    if(h <= 0)
		h = DEF_LIST_YMARGIN;

	    xc = MAX(x - (w / 2), 0);
	    yc = MAX(y - (h / 2), 0);

	    SARMenuDrawWindowBG(
		display, mdpy->bg_image, NULL,
		xc, yc,
		w, h,
		True,		/* Draw base */
		draw_shadows,	/* Draw shadow */
		(Boolean)((draw_selected_rect && is_selected) ? True : False)
	    );

	    if(mdpy->draw_cb != NULL)
		mdpy->draw_cb(
		    display,		/* Display */
		    m,			/* Menu */
		    o,			/* This Object */
		    mdpy->id,		/* ID Code */
		    mdpy->client_data,	/* Data */
		    xc, yc, xc + w, yc + h
		);
#if 0
	    /* Draw outline if selected */
	    if(draw_selected_rect && is_selected)
	    {
		int     x_min = x - (w / 2),
			x_max = x_min + w,
			y_min = y - (h / 2),
			y_max = y_min + h;
		glColor4f(0.0f, 0.0f, 0.0f, 1.0f);
		glBegin(GL_LINE_LOOP);
		{
		    glVertex2i(x_min, height - y_min);
		    glVertex2i(x_min, height - y_max);
		    glVertex2i(x_max, height - y_max);
		    glVertex2i(x_max, height - y_min);
		}
		glEnd();
	    }
#endif
	    break;

	  /* ******************************************************** */
	  case SAR_MENU_OBJECT_TYPE_SWITCH:
	    sw = SAR_MENU_SWITCH(o);
	    w = sw->width;
	    h = sw->height;

	    GWGetFontSize(
		sw->font,
		NULL, NULL,
		&fw, &fh
	    );

	    if(w <= 0)
		w = MAX(
		    (2 * DEF_XMARGIN) + DEF_SWITCH_WIDTH,
		    (2 * DEF_XMARGIN) + (STRLEN(sw->label) * fw)
		);
	    if(h <= 0)
		h = (1 * DEF_YMARGIN) + DEF_SWITCH_HEIGHT +
		    ((sw->label != NULL) ? fh : 0);

	    x = (int)(sw->x * width);
	    y = (int)(sw->y * height);

	    image = sw->bg_image;
	    DO_DRAW_IMAGE

	    if(sw->label != NULL)
	    {
		const char *label = sw->label;

		GWSetFont(display, sw->font);
		c = &sw->color;
		if(sw->sensitive)
		    glColor4f(c->r, c->g, c->b, c->a);
		else
		    glColor4f(
			c->r * 0.75f,
			c->g * 0.75f,
			c->b * 0.75f,
			c->a
		    );
		GWDrawString(
		    display,
		    x - (w / 2) + DEF_XMARGIN,
		    y - (h / 2) + DEF_SWITCH_HEIGHT,
		    label
		);
	    }

	    /* Check state of switch and choose the appropriate switch
	     * image to draw.
	     */
	    if(sw->state)
		image = sw->switch_on_image;
	    else
		image = sw->switch_off_image;
	    SARImageDraw(
		display,
		image,
		x - (DEF_SWITCH_WIDTH / 2),
		y - (h / 2) + 2,	/* Move down a little */
		DEF_SWITCH_WIDTH, DEF_SWITCH_HEIGHT
	    );

	    /* Draw outline if selected */
	    if(draw_selected_rect && is_selected)
	    {
		int     x_min = x - (w / 2) + 2,
			x_max = x_min + w - 4,
			y_min = y - (h / 2) + 2,
			y_max = y_min + h - 4;
		glColor4f(0.0f, 0.0f, 0.0f, 1.0f);
		glBegin(GL_LINE_LOOP);
		{
		    glVertex2i(x_min, height - y_min);
		    glVertex2i(x_min, height - y_max);
		    glVertex2i(x_max, height - y_max);
		    glVertex2i(x_max, height - y_min);
		}
		glEnd();
	    }
 	    break;

	  /* ******************************************************** */
	  case SAR_MENU_OBJECT_TYPE_SPIN:
	    spin = SAR_MENU_SPIN(o);

	    GWGetFontSize(
		spin->font,
		NULL, NULL,
		&fw, &fh
	    );

	    w = (int)(width * spin->width);
	    h = (int)MAX(
		height * spin->height,
		(4 * DEF_YMARGIN) + fh
	    );

	    x = (int)(width * spin->x);
	    y = (int)(height * spin->y);

	    if((w > 0) && (h > 0))
	    {
		int	x_start = x - (w / 2),
			y_start = y - (h / 2),
			label_len = STRLEN(spin->label),
			label_width = (2 * DEF_XMARGIN) + (label_len * fw);

		/* Draw label background */
		SARImageDraw(
		    display,   
		    spin->label_image,
		    x_start,
		    y_start,
		    label_width,
		    h
		);
		GWSetFont(display, spin->font);
		c = &spin->label_color;
		if(spin->sensitive)
		    glColor4f(c->r, c->g, c->b, c->a);
		else
		    glColor4f(
			c->r * 0.75f,
			c->g * 0.75f,
			c->b * 0.75f,
			c->a
		    );
		GWDrawString(
		    display,
		    x_start + DEF_XMARGIN,
		    y - (fh / 2),
		    spin->label
		);


		/* Begin drawing value just to the right of the label */
		GWSetFont(display, spin->value_font);
		GWGetFontSize(  
		    spin->value_font,
		    NULL, NULL,
		    &fw, &fh
		);
		/* Draw value background */
		SARImageDraw(
		    display,
		    spin->value_image,
		    x_start + label_width - 1,	/* Overlap one pixel to the left */
		    y_start,
		    MAX(w - label_width, 0),
		    h
		);

		/* Current spin value valid? */
		if((spin->cur_value >= 0) &&
		   (spin->cur_value < spin->total_values)
		)
		{
		    const char *value_ptr = (const char *)spin->value[
			spin->cur_value
		    ];
		    if(value_ptr != NULL)
		    {
			int	value_len = STRLEN(value_ptr),
				value_width = value_len * fw,
				value_twidth = w - label_width;

			/* Draw outline around value if selected */
			if(draw_selected_rect && is_selected)
			{
			    int	x_min = x_start + label_width +
				(value_twidth / 2) - (value_width / 2) - 1,
				x_max = x_min + value_width + 2,
				y_min = y - (fh / 2) - 1,
				y_max = y_min + fh + 2;
			    c = &spin->value_color;
			    glColor4f(
				c->r * 0.5f,
				c->g * 0.5f,
				c->b * 0.5f,
				c->a
			    );
			    glBegin(GL_LINE_LOOP);
			    {
				glVertex2i(x_min, height - y_min);
				glVertex2i(x_min, height - y_max);
				glVertex2i(x_max, height - y_max);
				glVertex2i(x_max, height - y_min);
			    }
			    glEnd();
			}

			/* Draw value */
			c = &spin->value_color;
			if(spin->sensitive)
			    glColor4f(c->r, c->g, c->b, c->a);
			else
			    glColor4f(
				c->r * 0.75f,
				c->g * 0.75f,
				c->b * 0.75f,
				c->a
			    );
			GWDrawString(
			    display,
			    x_start + label_width +
				(value_twidth / 2) - (value_width / 2),
			    y - (fh / 2),
			    value_ptr
			);
		    }
		}

		/* Draw decrement button */
		switch(spin->dec_state)
		{
		  case SAR_MENU_BUTTON_STATE_UNARMED:
		  case SAR_MENU_BUTTON_STATE_HIGHLIGHTED:
		    image = spin->dec_unarmed_image;
		    break;
		  default:
		    image = spin->dec_armed_image;
		    break;
		}
		if(image != NULL)
		{
		    int img_w = MIN(image->width, h),
			img_h = MIN(image->height, h);
		    SARImageDraw(
			display, image,
			x_start + label_width - 1,	/* Overlap one pixel to the left */
			y_start + (h / 2) - (img_h / 2),
			img_w, img_h
		    );
		}

		/* Draw increment button */
		switch(spin->inc_state)
		{
		  case SAR_MENU_BUTTON_STATE_UNARMED:
		  case SAR_MENU_BUTTON_STATE_HIGHLIGHTED:
		    image = spin->inc_unarmed_image;
		    break;
		  default:
		    image = spin->inc_armed_image;
		    break;
		}
		if(image != NULL)
		{
		    int	img_w = MIN(image->width, h),
			img_h = MIN(image->height, h);
		    SARImageDraw(
			display, image,
			x_start + w - h,
			y_start + (h / 2) - (img_h / 2),
			img_w, img_h
		    );
		}
#if 0
			/* Draw spin left arrow? */
			if(spin->allow_warp ? True :
			    (spin->cur_value > 0)
			)
			{
			    glRasterPos2i(
				x_start + label_width + (3 * DEF_XMARGIN),
				height - (y_start + (h / 2) -
				    (DEF_SCROLL_CURSOR_HEIGHT / 2) +
				    DEF_SCROLL_CURSOR_HEIGHT
			        )
			    );
			    glBitmap(
				DEF_SCROLL_CURSOR_WIDTH,
				DEF_SCROLL_CURSOR_HEIGHT,
				0.0, 0.0,
				DEF_SCROLL_CURSOR_WIDTH, 0.0,
				scroll_cursor_left_bm
			    );
			}
			/* Draw spin right arrow? */
			if(spin->allow_warp ? True :
			    (spin->cur_value < (spin->total_values - 1))
			)
			{
			    glRasterPos2i(
				x_start + w - (3 * DEF_XMARGIN) -
				    DEF_SCROLL_CURSOR_WIDTH,
				height - (y_start + (h / 2) -
				    (DEF_SCROLL_CURSOR_HEIGHT / 2) +
				    DEF_SCROLL_CURSOR_HEIGHT
				)
			    );
			    glBitmap(
				DEF_SCROLL_CURSOR_WIDTH,
				DEF_SCROLL_CURSOR_HEIGHT,
				0.0, 0.0,
				DEF_SCROLL_CURSOR_WIDTH, 0.0,
				scroll_cursor_right_bm
			    );
			}
#endif
#if 0
		/* Draw outline if selected */
		if(draw_selected_rect && is_selected)
		{
		    int	x_min = x - (w / 2),
			x_max = x_min + w,
			y_min = y - (h / 2),
			y_max = y_min + h;
		    glColor4f(0.0f, 0.0f, 0.0f, 1.0f);
		    glBegin(GL_LINE_LOOP);
		    {
			glVertex2i(x_min, height - y_min);
			glVertex2i(x_min, height - y_max);
			glVertex2i(x_max, height - y_max);
			glVertex2i(x_max, height - y_min);
		    }
		    glEnd();
		}
#endif
	    }
	    break;

	  /* ******************************************************** */
	  case SAR_MENU_OBJECT_TYPE_SLIDER:
	    slider = SAR_MENU_SLIDER(o);

	    GWGetFontSize(
		slider->font,
		NULL, NULL,
		&fw, &fh
	    );

	    w = (int)(width * slider->width);
	    h = (int)MAX(
		height * slider->height,
		(4 * DEF_YMARGIN) + fh
	    );

	    x = (int)(width * slider->x);
	    y = (int)(height * slider->y);

	    if((w > 0) && (h > 0))
	    {
		int	x_start = x - (w / 2),
			y_start = y - (h / 2),
			label_len = STRLEN(slider->label),
			label_width = (4 * DEF_XMARGIN) + (label_len * fw);

		/* Draw label background */
		SARImageDraw(
		    display,
		    slider->label_image,
		    x_start,
		    y_start,
		    label_width,
		    h
		);
		GWSetFont(display, slider->font);
		c = &slider->label_color;
		if(slider->sensitive)
		    glColor4f(c->r, c->g, c->b, c->a);
		else
		    glColor4f(
			c->r * 0.75f,
			c->g * 0.75f,
			c->b * 0.75f,
			c->a
		    );
		GWDrawString(
		    display,
		    x_start + DEF_XMARGIN,
		    y - (fh / 2),
		    slider->label
		);

		/* Draw trough background */
		SARImageDraw(
		    display,
		    slider->trough_image,
		    x_start + label_width - 1,  /* Overlap one pixel to the left */
		    y_start,
		    MAX(w - label_width, 0),
		    h
		);

		/* Draw handle */
		image = slider->handle_image;
		if(image != NULL)
		{
		    int trough_range = MAX(
			w - label_width - (2 * DEF_XMARGIN),
			0
		    );
		    int handle_x = x_start + label_width - 1 +
			DEF_XMARGIN - (image->width / 2);
		    int handle_y = y_start + (h / 2) -
			(image->height / 2);
		    float vrange = slider->upper - slider->lower;
		    float vc = (vrange > 0.0f) ?
			(slider->value / vrange) : 0.0f;

		    handle_x += (int)(vc * trough_range);
		    if((handle_x + image->width + 1) > (x_start - 1 + w))
			handle_x = (x_start - 1 + w) - (image->width + 1);

		    SARImageDraw(
			display,
			image,
			handle_x, handle_y,
			image->width, image->height
		    );
		}

		/* Draw outline if selected */
		if(draw_selected_rect && is_selected)
		{
		    int	x_min = x - (w / 2) + 2,
			x_max = x_min + w - 4,
			y_min = y - (h / 2) + 2,
			y_max = y_min + h - 4;
		    glColor4f(0.0f, 0.0f, 0.0f, 1.0f);
		    glBegin(GL_LINE_LOOP);
		    {
			glVertex2i(x_min, height - y_min);
			glVertex2i(x_min, height - y_max);
			glVertex2i(x_max, height - y_max);
			glVertex2i(x_max, height - y_min);
		    }
		    glEnd();
		}
	    }
	    break;

	  /* ******************************************************** */
	  case SAR_MENU_OBJECT_TYPE_MAP:
	    map = SAR_MENU_MAP(o);

	    /* Draw map object with its own drawing function first */
	    SARMenuMapDraw(display, m, map, n);

	    /* Calculate map object geometry for window frame drawing */
	    x = (int)(map->x * width);
	    y = (int)(map->y * height);

	    /* Calculate size to be one pixel bigger (to ensure cover) */
	    w = (int)(map->width * width) + 2;
	    h = (int)(map->height * height) + 2;

	    xc = MAX(x - (w / 2) - 1, 0);
	    yc = MAX(y - (h / 2) - 1, 0);

	    SARMenuDrawWindowBG(
		display, map->bg_image, NULL,
		xc, yc,
		w, h,
		False,		/* Do not draw base */
		draw_shadows,	/* Draw shadow */
		(Boolean)((draw_selected_rect && is_selected) ? True : False)
	    );
	    break;

	  /* ******************************************************** */
	  case SAR_MENU_OBJECT_TYPE_OBJVIEW:
	    objview = SAR_MENU_OBJVIEW(o);

	    /* Calculate 3d object viewer object geometry for window
	     * frame drawing.
	     */
	    x = (int)(objview->x * width);
	    y = (int)(objview->y * height);

	    /* Calculate size to be one pixel bigger (to ensure cover) */
	    w = (int)(objview->width * width) + 2;
	    h = (int)(objview->height * height) + 2;

	    xc = MAX(x - (w / 2) - 1, 0);
	    yc = MAX(y - (h / 2) - 1, 0);

	    SARMenuDrawWindowBG(
		display, objview->bg_image, &objview->bg_color,
		xc, yc,
		w, h,
		True,		/* Draw base */
		draw_shadows,	/* Draw shadow */
		(Boolean)((draw_selected_rect && is_selected) ? True : False)
	    );

	    /* Draw buffered 3d object image, this image has an alpha
	     * channel so it should be drawn over the base of the
	     * window background drawn above.
	     */
	    if(objview->objimg != NULL)
	    {
		sar_image_struct tmp_img;
		float aspect = (float)((objview->objimg_width > 0) ?
		    ((float)objview->objimg_height /
			(float)objview->objimg_width) : 0.75
		);
		int	draw_width = w,
			draw_height = (int)(w * aspect);

		if((draw_height > h) && (aspect > 0.0f))
		{
		    draw_width = (int)(h / aspect);
		    draw_height = h;
		}

		tmp_img.width = objview->objimg_width;
		tmp_img.height = objview->objimg_height;
		tmp_img.data = objview->objimg;

		StateGLEnable(&display->state_gl, GL_SCISSOR_TEST);
		StateGLScissor(
		    &display->state_gl,
		    xc + DEF_MB_XMARGIN,
		    height - yc - h + DEF_MB_YMARGIN,
		    MAX(w - (2 * DEF_MB_XMARGIN), DEF_MB_XMARGIN),
		    MAX(h - (2 * DEF_MB_YMARGIN), DEF_MB_YMARGIN)
		);
		if(objview->show_message)
		    SARImageDraw(
			display, &tmp_img,
			MAX(x + (w / 2) - draw_width, 0),
			MAX(y - (draw_height / 2), 0),
			draw_width, draw_height
		    );
		else
		    SARImageDraw(
			display, &tmp_img,
			MAX(x - (draw_width / 2), 0),
			MAX(y - (draw_height / 2), 0),
			draw_width, draw_height
		    );
		StateGLDisable(&display->state_gl, GL_SCISSOR_TEST);
	    }

	    /* Begin drawing message */

	    GWGetFontSize(
		objview->font,
		NULL, NULL,
		&fw, &fh
	    );
	    GWSetFont(display, objview->font);

	    /* Draw message? */
	    if((objview->message != NULL) && objview->show_message)
	    {
		int	draw_x = xc + DEF_MB_XMARGIN,
			draw_y = yc + DEF_MB_YMARGIN;
		const char *buf_ptr = objview->message;
		const sar_menu_color_struct *c_b = &objview->bold_color,
					    *c_u = &objview->underline_color;

		c = &objview->color;
		if(objview->sensitive)
		    glColor4f(c->r, c->g, c->b, c->a);
		else
		    glColor4f(
			c->r * 0.75f,
			c->g * 0.75f,
			c->b * 0.75f,
			c->a
		    );

#define DO_DRAW_LINE_COLORED			\
{						\
 const char *s = buf_ptr;			\
 int sx = draw_x, sy = draw_y;			\
 while((*s != '\n') && (*s != '\0'))		\
 {						\
  if(IS_CHAR_ESC_START(*s))			\
  {						\
   s++;						\
   if(strcasepfx(s, "bold"))			\
    glColor4f(c_b->r, c_b->g, c_b->b, c_b->a);	\
   else if(strcasepfx(s, "underline"))		\
    glColor4f(c_u->r, c_u->g, c_u->b, c_u->a);  \
   else if(strcasepfx(s, "default"))		\
    glColor4f(c->r, c->g, c->b, c->a);		\
   while(!IS_CHAR_ESC_END(*s) && (*s != '\0'))	\
    s++;					\
   if(*s != '\0')				\
    s++;					\
  }						\
  else						\
  {						\
   GWDrawCharacter(display, sx, sy, *s);	\
   s++; sx += fw;				\
  }						\
 }						\
 if(*s == '\n')					\
  s++;						\
 buf_ptr = s;					\
}

		/* Begin drawing each line, draw in a style similar to
		 * the message box.
		 */
		do
		{
		    if(draw_y >= (yc + h - (2 * DEF_MB_YMARGIN)))
			break;

		    DO_DRAW_LINE_COLORED
		    draw_y += fh;

		} while(buf_ptr != NULL);
#undef DO_DRAW_LINE_COLORED
	    }

	    if(objview->objimg != NULL)
	    {
		const char *s =
"(Button1 + DRAG = Heading & Pitch  Button3 + DRAG = Bank)";
		int     draw_x = x - (STRLEN(s) * fw / 2),
			draw_y = yc + h - DEF_MB_YMARGIN - fh;

		c = &objview->color;
		if(objview->sensitive)
		    glColor4f(c->r, c->g, c->b, c->a);
		else
		    glColor4f(
			c->r * 0.75f,
			c->g * 0.75f,
			c->b * 0.75f,
			c->a
		    );

		GWDrawString(
		    display, draw_x, draw_y, s
		);
	    }
	    break;

	}
#undef DO_DRAW_IMAGE
}


/*
 *      Draws the specified object n (but not its shadow).
 */
void SARMenuDrawObject(
	gw_display_struct *display, sar_menu_struct *m,
	int n			/* Object number on m */
)
{
	SARMenuDoDrawObject(display, m, n, False);
}

/*
 *	Draws the specified menu m.
 *
 *	Does not swap the GW buffers.
 */
void SARMenuDrawAll(
	gw_display_struct *display,
	sar_menu_struct *m
)
{
	int i, width, height;

	if((display == NULL) || (m == NULL))
	    return;

	/* Set up gl state for 2d drawing */
	GWOrtho2D(display);

	GWContextGet(
	    display, GWContextCurrent(display),
	    NULL, NULL,
	    NULL, NULL,
	    &width, &height
	);

	/* Draw background image */
	if(m->bg_image != NULL)
	{
	    /* Draw to the size of the frame buffer, thus clearing it */
	    SARImageDraw(
		display,
		m->bg_image,
	        0, 0,
	        width, height
	    );
	}
	else
	{
	    glClearColor(1.0f, 1.0f, 1.0f, 0.0f);
	    glClearDepth(1.0f);
	    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	/* Draw each object (with shadows) */
	for(i = 0; i < m->total_objects; i++)
	    SARMenuDoDrawObject(display, m, i, True);
}


/*
 *	Manages a pointer event assumed for the given menu.
 */
int SARMenuManagePointer(
	gw_display_struct *display, sar_menu_struct *m,
	int x, int y, gw_event_type type, int btn_num
)
{
	int i, events_handled = 0;
	int xp, yp, w, h;
	int x_min, x_max, y_min, y_max, width, height;
	void *o;
	sar_menu_button_struct *button;
	sar_menu_message_box_struct *mesgbox;
	sar_menu_list_struct *list;
	sar_menu_mdisplay_struct *mdpy;
	sar_menu_switch_struct *sw;
	sar_menu_spin_struct *spin;
	sar_menu_slider_struct *slider;
	sar_menu_map_struct *map;
	sar_menu_objview_struct *objview;


	if((display == NULL) || (m == NULL))
	    return(events_handled);

	GWContextGet(
	    display, GWContextCurrent(display),
	    NULL, NULL,
	    NULL, NULL,
	    &width, &height
	);

	/* Iterate through all menu object/widgets */
	for(i = 0; i < m->total_objects; i++)
	{
	    o = m->object[i];
	    if(o == NULL)
		continue;

	    switch(SAR_MENU_OBJECT_TYPE(o))
	    {
	      /* ***************************************************** */
	      case SAR_MENU_OBJECT_TYPE_BUTTON:
		button = SAR_MENU_BUTTON(o);
		if(!button->sensitive)
		    break;

		xp = (int)(button->x * width) + button->offset_x;
		yp = (int)(button->y * height) + button->offset_y;
		w = button->width;
		h = button->height;

		/* Calculate coordinate bounds */
		x_min = (int)MAX(xp - (w / 2), 0);
		x_max = (int)MAX(x_min + w, w);
		y_min = (int)MAX(yp - (h / 2), 0);
		y_max = (int)MAX(y_min + h, h);

		/* Event in bounds? */
		if((events_handled <= 0) &&
		   (x >= x_min) && (x < x_max) &&
		   (y >= y_min) && (y < y_max)
		)
		{
		    /* Handle by event type */
		    switch(type)
		    {
		      case GWEventTypePointerMotion:
			if(button->state == SAR_MENU_BUTTON_STATE_UNARMED)
			{
			    button->state = SAR_MENU_BUTTON_STATE_HIGHLIGHTED;
			    events_handled++;
			    DO_REDRAW_OBJECT(display, m, i)
			}
			break;

		      case GWEventTypeButtonPress:
			events_handled++;
			/* Select new object as needed and redraw the
			 * previously selected object
			 */
			if(m->selected_object != i)
			{
			    int po = m->selected_object;
			    m->selected_object = i;
			    if(m->always_full_redraw)             
				SARMenuDrawAll(display, m);
			    else                                    
				SARMenuDrawObject(display, m, po);
			}
			/* Set button to armed state as appropriate */
			if((button->state != SAR_MENU_BUTTON_STATE_ARMED) &&
			   (btn_num == 1)
			)
			{
			    button->state = SAR_MENU_BUTTON_STATE_ARMED;
			    DO_REDRAW_OBJECT(display, m, i)
			}
			else
			{
			    GWSwapBuffer(display);
			}
			break;

		      case GWEventTypeButtonRelease:
			events_handled++;
			/* Select new object as needed and redraw the
			 * previously selected object
			 */
			if(m->selected_object != i)
			{
			    int po = m->selected_object;
			    m->selected_object = i;
			    if(m->always_full_redraw)             
				SARMenuDrawAll(display, m);
			    else                                    
				SARMenuDrawObject(display, m, po);
			}
			/* Set button back to highlighted state if it
			 * was armed and call the button's click
			 * callback
			 */
			if(button->state == SAR_MENU_BUTTON_STATE_ARMED)
			{
			    button->state = SAR_MENU_BUTTON_STATE_HIGHLIGHTED;
			    if(m->always_full_redraw)             
				SARMenuDrawAll(display, m);
			    else                                    
				SARMenuDrawObject(display, m, i);

			    if(button->func_cb != NULL)
			    {
				int n;
				sar_menu_button_struct *button2;

				/* Set all other buttons on this menu to
				 * unarmed.
				 */
				for(n = 0; n < m->total_objects; n++)
				{
				    o = m->object[n];
				    if(SAR_MENU_IS_BUTTON(o))
					button2 = SAR_MENU_BUTTON(o);
				    else
					continue;
				    if(button == button2)
					continue;

				    if(button2->state != SAR_MENU_BUTTON_STATE_UNARMED)
				    {
					button2->state = SAR_MENU_BUTTON_STATE_UNARMED;
					if(m->always_full_redraw)             
					    SARMenuDrawAll(display, m);
					else
					    SARMenuDrawObject(display, m, n);
				    }
				}

				GWSwapBuffer(display);

				/* Call callback function */
				button->func_cb(
				    button,		/* Object */
				    button->id,		/* ID Code */
				    button->client_data	/* Data */
				);
			    }
			    else
			    {
				GWSwapBuffer(display);
			    }
			}
			break;

		      case GWEventType2ButtonPress:
		      case GWEventType3ButtonPress:
			break;
		    }
		}
		else
		{
		    /* Pointer event out of bounds */
		    switch(type)
		    {
		      case GWEventTypeButtonPress:
			break;
		      case GWEventTypeButtonRelease:
		      case GWEventTypePointerMotion:
			if(button->state != SAR_MENU_BUTTON_STATE_UNARMED)
			{
			    button->state = SAR_MENU_BUTTON_STATE_UNARMED;
			    DO_REDRAW_OBJECT(display, m, i)
			}
			break;
		      case GWEventType2ButtonPress:
		      case GWEventType3ButtonPress:
			break;
		    }
		}
		break;

	      /* ***************************************************** */
	      case SAR_MENU_OBJECT_TYPE_MESSAGE_BOX:
		mesgbox = SAR_MENU_MESSAGE_BOX(o);
		if(!mesgbox->sensitive)
		    break;

		w = (int)(mesgbox->width * width);
		if(w <= 0)
		    w = (int)(width - (2 * DEF_MB_XMARGIN));
		if(w <= 0)
		    w = DEF_MB_XMARGIN;
		
		if(mesgbox->height > 0)
		    h = (int)(mesgbox->height * height);
		else
		    h = DEF_HEIGHT;
		if(h <= 0)
		    h = DEF_MB_YMARGIN;

		/* Get coordinate bounds */
		x_min = (int)((mesgbox->x * width) - (w / 2));
		x_max = (int)(x_min + w);
		y_min = (int)((mesgbox->y * height) - (h / 2));
		y_max = (int)(y_min + h); 
	      
		/* Pointer event in bounds? */
		if((events_handled <= 0) &&
		   (x >= x_min) && (x < x_max) && 
		   (y >= y_min) && (y < y_max) &&
		   (mesgbox->message != NULL) &&
		   (type == GWEventTypeButtonPress)
		)
		{
		    int fh, fw;
		    int lines_visable, columns_visable,
			total_lines, scroll_width;

		    events_handled++;

		    /* Select new object as needed and redraw the
		     * previously selected object.
		     */
		    if(m->selected_object != i)
		    {
			int po = m->selected_object;
			m->selected_object = i;
			if(m->always_full_redraw)             
			    SARMenuDrawAll(display, m);
			else                                    
			    SARMenuDrawObject(display, m, po);
		    }
	      
		    GWGetFontSize(
			mesgbox->font,
			NULL, NULL,
			&fw, &fh
		    );

		    /* Calculate colums visable */
		    if(fw > 0)
			columns_visable = (w - DEF_SCROLL_CURSOR_WIDTH
			    - (2 * DEF_MB_XMARGIN)) / fw;
		    else
			columns_visable = 1;
		    if(columns_visable < 1)
			columns_visable = 1;

		    /* Calculate width (height) of scroll indicator */
		    scroll_width = fh + (2 * DEF_MB_YMARGIN);  

		    /* Calculate lines visable */
		    if(fh > 0)
			lines_visable = (h - (2 * DEF_MB_YMARGIN)) / fh;
		    else
			lines_visable = 0;
		    if(lines_visable < 0)
			lines_visable = 0;

		    /* Calculate total number of lines */
		    total_lines = SARMenuMessageBoxTotalLines(
			mesgbox->message, columns_visable
		    );
		    /* Let's make scrolling simple, check if y is above
		     * or below the midpoint,
		     * or if mouse wheel is rolled (btn_num == 4, btn_num == 5).
		     * Test mouse wheel first because it's the most frequent case.
		     */
		    if ( btn_num == 4 || ( (btn_num == 1 || btn_num == 3) && (y < (mesgbox->y * height)) ) )
		    {
			/* Scroll up */
			mesgbox->scrolled_line--;
		    }
		    else if ( btn_num == 5 || ( (btn_num == 1 || btn_num == 3) && (y >= (mesgbox->y * height)) ) )
		    {
			/* Scroll down */
			if ( mesgbox->scrolled_line + lines_visable < total_lines ) mesgbox->scrolled_line++;
		    }
		    if(mesgbox->scrolled_line >
			(total_lines - (lines_visable / 2))
		    )
			mesgbox->scrolled_line = total_lines -
			    (lines_visable / 2);
		    if(mesgbox->scrolled_line < 0)
			mesgbox->scrolled_line = 0;

		    DO_REDRAW_OBJECT(display, m, i)
		}
		break;

	      /* ***************************************************** */
	      case SAR_MENU_OBJECT_TYPE_LIST:
		list = SAR_MENU_LIST(o);
		if(!list->sensitive)
		    break;

		w = (int)(list->width * width);
		if(w <= 0)
		    w = width - (2 * DEF_LIST_XMARGIN); 
		if(w <= 0)
		    w = DEF_LIST_XMARGIN;

		if(list->height > 0)
		    h = (int)(list->height * height);
		else
		    h = DEF_HEIGHT;
		if(h <= 0)
		    h = DEF_LIST_YMARGIN;

		/* Calculate coordinate bounds */
		x_min = (int)((list->x * width) - (w / 2));  
		x_max = (int)(x_min + w);
		y_min = (int)((list->y * height) - (h / 2));
		y_max = (int)(y_min + h);

		/* Event in bounds? */
		if((events_handled <= 0) &&
		   (x >= x_min) && (x < x_max) &&
		   (y >= y_min) && (y < y_max)
		)
		{
		    int fh, fw, items_visible;
		    int scroll = 0;

		    GWGetFontSize(
			list->font, 
			NULL, NULL,
			&fw, &fh
		    );
		    /* Calculate items visible, remember to subtract
		     * three items for borders and heading
		     */
		    if(fh > 0)
			items_visible = MAX(
			    (h / fh) - 3,
			    0
			);
		    else
			items_visible = 0;

		    /* Update items visable on list */
		    list->items_visable = items_visible;

		    /* Determine if we need to scroll up or down
		     * depending on the location of the event
		     * and the pressed mouse button
		     */
            if ( btn_num == 4 ) /* mouse wheel up */
			    scroll = 1;
            else if ( btn_num == 5 ) /* mouse wheel down */
			    scroll = 2;
            else if ( btn_num == 1 || btn_num == 3 ) /* left or right mouse button */
            {
		    if( (x > (x_max - DEF_SCROLL_CURSOR_WIDTH - DEF_LIST_XMARGIN + 1)) && (x < (x_max - DEF_LIST_XMARGIN)) )
		    {
			if(y < (y_min + DEF_SCROLL_CURSOR_HEIGHT + fh + DEF_LIST_YMARGIN)) /* top arrow click */
			    scroll = 1;
			else if(y >= (y_max - DEF_SCROLL_CURSOR_HEIGHT - DEF_LIST_YMARGIN)) /* bottom arrow click */
			    scroll = 2;
            else
            {
			    /* Left or right click occurred with mouse pointer on scrollbar area */
            }
		    }
		    }

		    /* Handle by event type */
		    switch(type)
		    {
		      case GWEventTypeButtonPress:
			events_handled++;

			/* Select new object as needed and redraw the
			 * previously selected object
			 */
			if(m->selected_object != i)
			{
			    int po = m->selected_object;
			    m->selected_object = i;
			    if(m->always_full_redraw)             
				SARMenuDrawAll(display, m);
			    else
				SARMenuDrawObject(display, m, po);
			}

			/* Scroll up? */
			if(scroll == 1)
			{
			    list->scrolled_item -= MAX(
				items_visible - 1,
				1
			    );
			    if(list->scrolled_item < 0)
				list->scrolled_item = 0;

			    DO_REDRAW_OBJECT(display, m, i)
			}
			/* Scroll down? */
			else if(scroll == 2)
			{
			    list->scrolled_item += MAX(
				items_visible - 1,
				1
			    );
			    if(list->scrolled_item >=
				(list->total_items - (items_visible / 2))
			    )
				list->scrolled_item =
				    list->total_items -
				    (items_visible / 2);
			    if(list->scrolled_item < 0)
				list->scrolled_item = 0;

			    DO_REDRAW_OBJECT(display, m, i)
			}
			else
			{
			    /* Match selection */
			    int sel_num, prev_sel_num;

			    /* Calculate selection position, remember
			     * to - 1 to account for the heading
			     * which should not be selected
			     */
			    if(fh > 0)
				sel_num = list->scrolled_item +
				    ((y - DEF_LIST_YMARGIN - fh - y_min) / fh);
			    else
				sel_num = -1;

			    /* Sanitize selection */
			    if(sel_num < -1)
				sel_num = -1;
			    if(sel_num >= list->total_items)
				sel_num = list->total_items - 1;

			    /* Record previously selected item and set
			     * new selected item
			     */
			    prev_sel_num = list->selected_item;
			    list->selected_item = sel_num;

			    DO_REDRAW_OBJECT(display, m, i)

			    /* Call select callback */
			    if((list->select_cb != NULL) &&
			       (prev_sel_num != sel_num) &&
			       (sel_num >= 0) &&
			       (sel_num < list->total_items)
			    )
			    {
				sar_menu_list_item_struct *item = list->item[sel_num];
				if(item != NULL)
				    list->select_cb(
				    list,		/* This Object */
				    list->id,		/* ID Code */
				    list->client_data,	/* Data */
				    sel_num,		/* Item Number */
				    item,		/* Item Pointer */
				    item->client_data	/* Item Data */
				);
			    }
		        }
			break;

		      case GWEventTypeButtonRelease:
			break;

		      case GWEventTypePointerMotion:
			break;

		      case GWEventType2ButtonPress:
			events_handled++;

			/* No scroll? */
			if(scroll == 0)
			{
			    int sel_num = list->selected_item;

			    /* Call activate callback */
			    if((list->activate_cb != NULL) &&
			       (sel_num >= 0) &&
			       (sel_num < list->total_items)
			    )
			    {
				sar_menu_list_item_struct *item = list->item[sel_num];
				if(item != NULL)
				    list->activate_cb(
				    list,		/* This Object */
				    list->id,		/* ID Code */
				    list->client_data,	/* Data */
				    sel_num,		/* Item Number */
				    item,		/* Item Pointer */
				    item->client_data	/* Item Data */
				);
			    }
			}
			break;

		      case GWEventType3ButtonPress:
			break;
		    }
		}
		break;

	      /* ***************************************************** */
	      case SAR_MENU_OBJECT_TYPE_MDISPLAY:
		mdpy = SAR_MENU_MDISPLAY(o);
		if(!mdpy->sensitive)
		    break;

		w = (int)(mdpy->width * width);
		if(w <= 0)
		    w = width - (2 * DEF_LIST_XMARGIN);
		if(w <= 0)
		    w = DEF_LIST_XMARGIN;

		if(mdpy->height > 0)
		    h = (int)(mdpy->height * height);
		else
		    h = DEF_HEIGHT;
		if(h <= 0)
		    h = DEF_LIST_YMARGIN;

		/* Get coordinate bounds */
		x_min = (int)((mdpy->x * width) - (w / 2));
		x_max = (int)(x_min + w);
		y_min = (int)((mdpy->y * height) - (h / 2));
		y_max = (int)(y_min + h);

		/* Pointer event in bounds? */
		if((events_handled <= 0) &&
		   (x >= x_min) && (x < x_max) &&
		   (y >= y_min) && (y < y_max)
		)
		{
		    switch(type)
		    {
		      case GWEventTypeButtonPress:
			events_handled++;

			/* Select new object as needed and redraw the
			 * previously selected object.
			 */
			if(m->selected_object != i)
			{
			    int po = m->selected_object;
			    m->selected_object = i;
			    if(m->always_full_redraw)             
				SARMenuDrawAll(display, m);
			    else
				SARMenuDrawObject(display, m, po);
			}

			DO_REDRAW_OBJECT(display, m, i)
			break;

		      case GWEventTypeButtonRelease:
		      case GWEventTypePointerMotion:
		      case GWEventType2ButtonPress:
		      case GWEventType3ButtonPress:
			break;
		    }
		}
		break;

	      /* ***************************************************** */
	      case SAR_MENU_OBJECT_TYPE_SWITCH:
		sw = SAR_MENU_SWITCH(o);
		if(sw->sensitive)
		{
		    int fw, fh;
		    const char *label = sw->label;

		    w = sw->width;
		    h = sw->height;

		    GWGetFontSize(
			sw->font,
			NULL, NULL,
			&fw, &fh 
		    );

		    /* Calculate size of switch area */
		    if(w <= 0)
			w = DEF_SWITCH_WIDTH;
		    if(h <= 0)
			h = DEF_YMARGIN + DEF_SWITCH_HEIGHT +
			    ((label != NULL) ? fh : 0);

		    /* Get coordinate bounds for switch area */
		    x_min = (int)((sw->x * width) - (w / 2));
		    x_max = (int)(x_min + DEF_SWITCH_WIDTH);
		    y_min = (int)((sw->y * height) - (h / 2) + 2);
		    y_max = (int)(y_min + DEF_SWITCH_HEIGHT);

		    /* Pointer event in bounds? */
		    if((events_handled <= 0) &&
		       (x >= x_min) && (x < x_max) &&
		       (y >= y_min) && (y < y_max)
		    )
		    {
			switch(type)
			{
			  case GWEventTypeButtonPress:
			    break;

			  case GWEventTypeButtonRelease:
			    switch(btn_num)
			    {
			      case 1:
				events_handled++;

				/* Select new object as needed and
				 * redraw the previously selected object.
				 */
				if(m->selected_object != i)
				{
				    int po = m->selected_object;
				    m->selected_object = i;
				    if(m->always_full_redraw)             
					SARMenuDrawAll(display, m);
				    else
					SARMenuDrawObject(display, m, po);
				}

				sw->state = !sw->state;
				DO_REDRAW_OBJECT(display, m, i)

				if(sw->switch_cb != NULL)
				    sw->switch_cb(
					sw,		/* This Object */
					sw->id,		/* ID Code */
					sw->client_data,/* Data */
					sw->state	/* State */
				);
				break;
			    }
			    break;

			  case GWEventTypePointerMotion:
			  case GWEventType2ButtonPress:
			  case GWEventType3ButtonPress:
			    break;
		        }
		    }
		    /* Event not handled? */
		    if(events_handled <= 0)
		    {
			/* Calculate size of object area */
			w = sw->width;
			h = sw->height;
			if(w <= 0)
			    w = MAX(
				(2 * DEF_XMARGIN) + DEF_SWITCH_WIDTH,
				(2 * DEF_XMARGIN) + (STRLEN(label) * fw)
			    );
			if(h <= 0)
			    h = (1 * DEF_YMARGIN) + DEF_SWITCH_HEIGHT +
				((label != NULL) ? fh : 0);

			/* Get coordinate bounds */
			x_min = (int)((sw->x * width) - (w / 2));
			x_max = (int)(x_min + w);
			y_min = (int)((sw->y * height) - (h / 2));
			y_max = (int)(y_min + h);

			/* Pointer event in bounds? */
			if((x >= x_min) && (x < x_max) &&
			   (y >= y_min) && (y < y_max)
			)
			{
			    switch(type)
			    {
			      case GWEventTypeButtonPress:
				events_handled++;

				/* Select new object as needed and
				 * redraw the previously selected
				 * object
				 */
				if(m->selected_object != i)
				{
				    int po = m->selected_object;
				    m->selected_object = i;
				    if(m->always_full_redraw)             
					SARMenuDrawAll(display, m);
				    else
					SARMenuDrawObject(display, m, po);
				}

				DO_REDRAW_OBJECT(display, m, i)
				break;

			      case GWEventTypeButtonRelease:
			      case GWEventTypePointerMotion:
			      case GWEventType2ButtonPress:
			      case GWEventType3ButtonPress:
				break;
			    }
			}
		    }
		}
		break;

	      /* ***************************************************** */
	      case SAR_MENU_OBJECT_TYPE_SPIN:
		spin = SAR_MENU_SPIN(o);
		if(spin->sensitive)
		{
		    int fw, fh;
		    int label_len = STRLEN(spin->label);
		    int label_width;

		    GWGetFontSize(
			spin->font,
			NULL, NULL,
			&fw, &fh
		    );

		    label_width = (2 * DEF_XMARGIN) + (label_len * fw);

		    w = (int)(width * spin->width);
		    h = (int)MAX(
			height * spin->height,
			(4 * DEF_YMARGIN) + fh
		    );

		    /* Get coordinate bounds for decrement button */
		    x_min = (int)((spin->x * width) - (w / 2) + label_width);
		    x_max = (int)(x_min + h);
		    y_min = (int)((spin->y * height) - (h / 2));
		    y_max = (int)(y_min + h);
		    /* Pointer event in bounds? */
		    if((x >= x_min) && (x < x_max) &&
		       (y >= y_min) && (y < y_max) &&
		       (events_handled <= 0)
		    )
		    {
			switch(type)
			{
			  case GWEventTypeButtonPress:
			    events_handled++;
			    /* Select new object as needed and redraw
			     * the previously selected object
			     */
			    if(m->selected_object != i)
			    {
				int po = m->selected_object;
				m->selected_object = i;
				if(m->always_full_redraw)             
				    SARMenuDrawAll(display, m);
				else
				    SARMenuDrawObject(display, m, po);
			    }
			    spin->dec_state = SAR_MENU_BUTTON_STATE_ARMED;
			    DO_REDRAW_OBJECT(display, m, i)
			    break;
			  case GWEventTypeButtonRelease:
			    events_handled++;
			    switch(spin->dec_state)
			    {
			      case SAR_MENU_BUTTON_STATE_ARMED:
			      case SAR_MENU_BUTTON_STATE_HIGHLIGHTED:
				spin->dec_state = SAR_MENU_BUTTON_STATE_HIGHLIGHTED;
				if(btn_num == 3)
				    SARMenuSpinSelectValueIndex(
					display, m, i,
					0,
					False
				    );
				else
				    SARMenuSpinDoDec(display, m, i, False);
				DO_REDRAW_OBJECT(display, m, i)
				break;
			    }
			    break;
			  case GWEventTypePointerMotion:
			    events_handled++;
			    if(spin->dec_state == SAR_MENU_BUTTON_STATE_UNARMED)
			    {
				spin->dec_state = SAR_MENU_BUTTON_STATE_HIGHLIGHTED;
				DO_REDRAW_OBJECT(display, m, i)
			    }
			    break;
			  case GWEventType2ButtonPress:
			  case GWEventType3ButtonPress:
			    break;
			}
		    }

		    /* Get coordinate bounds for increment button */
		    x_max = (int)((spin->x * width) + (w / 2));
		    x_min = (int)(x_max - h);
		    y_min = (int)((spin->y * height) - (h / 2));
		    y_max = (int)(y_min + h);
		    /* Pointer event in bounds? */
		    if((x >= x_min) && (x < x_max) &&
		       (y >= y_min) && (y < y_max) &&
		       (events_handled <= 0)
		    )
		    {
			switch(type)
			{
			  case GWEventTypeButtonPress:
			    events_handled++;
			    /* Select new object as needed and redraw
			     * the previously selected object
			     */
			    if(m->selected_object != i)
			    {
				int po = m->selected_object;
				m->selected_object = i;
				if(m->always_full_redraw)             
				    SARMenuDrawAll(display, m);
				else
				    SARMenuDrawObject(display, m, po);
			    }
			    spin->inc_state = SAR_MENU_BUTTON_STATE_ARMED;
			    DO_REDRAW_OBJECT(display, m, i)
			    break;
			  case GWEventTypeButtonRelease:
			    events_handled++;
			    switch(spin->inc_state)
			    {
			      case SAR_MENU_BUTTON_STATE_ARMED:
			      case SAR_MENU_BUTTON_STATE_HIGHLIGHTED:
				spin->inc_state = SAR_MENU_BUTTON_STATE_HIGHLIGHTED;
				if(btn_num == 3)
				    SARMenuSpinSelectValueIndex(
					display, m, i,
					MAX(spin->total_values - 1, 0),
					False
				    );
				else
				    SARMenuSpinDoInc(display, m, i, False);
				DO_REDRAW_OBJECT(display, m, i)
				break;
			    }
			    break;
			  case GWEventTypePointerMotion:
			    events_handled++;
			    if(spin->inc_state == SAR_MENU_BUTTON_STATE_UNARMED)
			    {
				spin->inc_state = SAR_MENU_BUTTON_STATE_HIGHLIGHTED;
				DO_REDRAW_OBJECT(display, m, i)
			    }
			    break;
			  case GWEventType2ButtonPress:
			  case GWEventType3ButtonPress:
			    break;
			}
		    }

		    /* Event not handled above? */
		    if(events_handled <= 0)
		    {
			/* Event not handled above, this implies the
			 * event occured outside the event areas of
			 * this object
			 */
			switch(type)
			{
			  case GWEventTypeButtonPress:
			    /* Get coordinate bounds */
			    x_min = (int)((spin->x * width) - (w / 2));
			    x_max = (int)(x_min + w);
			    y_min = (int)((spin->y * height) - (h / 2));
			    y_max = (int)(y_min + h);

			    /* Pointer event in bounds? */
			    if((x >= x_min) && (x < x_max) &&
			       (y >= y_min) && (y < y_max)
			    )
			    {
				events_handled++;
				/* Select new object as needed and
				 * redraw the previously selected object.
				 */
				if(m->selected_object != i)
				{
				    int po = m->selected_object;
				    m->selected_object = i;
				    if(m->always_full_redraw)
					SARMenuDrawAll(display, m);
				    else
					SARMenuDrawObject(display, m, po);
				}
				DO_REDRAW_OBJECT(display, m, i)
			    }
			    break;

			  case GWEventTypeButtonRelease:
			  case GWEventTypePointerMotion:
			    if(spin->dec_state != SAR_MENU_BUTTON_STATE_UNARMED)
			    {
				spin->dec_state = SAR_MENU_BUTTON_STATE_UNARMED;
				DO_REDRAW_OBJECT(display, m, i)
			    }
			    if(spin->inc_state != SAR_MENU_BUTTON_STATE_UNARMED)
			    {
				spin->inc_state = SAR_MENU_BUTTON_STATE_UNARMED;
				DO_REDRAW_OBJECT(display, m, i)
			    }
			    break;

			  case GWEventType2ButtonPress:
			  case GWEventType3ButtonPress:
			    break;
			}
		    }
		}
		break;

	      /* ***************************************************** */
	      case SAR_MENU_OBJECT_TYPE_SLIDER:
		slider = SAR_MENU_SLIDER(o);
		if(slider->sensitive)
		{
		    int fw, fh;
		    int label_len = STRLEN(slider->label);
		    int label_width;

		    GWGetFontSize(
		       slider->font,
		       NULL, NULL,
		       &fw, &fh
		    );

		    w = (int)(width * slider->width);
		    h = (int)MAX(
			height * slider->height,
			(4 * DEF_YMARGIN) + fh
		    );

		    label_width = (4 * DEF_XMARGIN) + (label_len * fw);

		    /* Get coordinate bounds of trough */
		    x_min = (int)(
			(slider->x * width) - (w / 2) + label_width
		    ) + DEF_XMARGIN;
		    x_max = (int)((slider->x * width) + (w / 2)) - DEF_XMARGIN;
		    y_min = (int)((slider->y * height) - (h / 2));
		    y_max = (int)(y_min + h);

		    /* Pointer event in bounds? */
		    if((events_handled <= 0) &&
		       (x >= x_min) && (x < x_max) &&
		       (y >= y_min) && (y < y_max)
		    )
		    {
			float vc, value;
#define DO_CALCULATE_VALUE	{			\
 vc = ((x_max - x_min) > 0) ?				\
  (float)(x - x_min) / (float)(x_max - x_min) : 0.0f;	\
 value = CLIP(slider->lower +				\
  (vc * (slider->upper - slider->lower)),		\
  slider->lower, slider->upper				\
 );							\
}

			switch(type)
			{
			  case GWEventTypeButtonPress:
			    events_handled++;
			    /* Select new object as needed and redraw the
			     * previously selected object
			     */
			    if(m->selected_object != i)
			    {
				int po = m->selected_object;
				m->selected_object = i;
				if(m->always_full_redraw)
				    SARMenuDrawAll(display, m);
				else
				    SARMenuDrawObject(display, m, po);
			    }
			    /* Record drag button number */
			    slider->drag_button = btn_num;
			    if(slider->drag_button == 1)
			    {
				DO_CALCULATE_VALUE
				SARMenuSliderSetValue(
				    display, m, i, value, True
				);
			    }
			    else
			    {
				DO_REDRAW_OBJECT(display, m, i)
			    }
			    break;

			  case GWEventTypeButtonRelease:
			    events_handled++;
			    slider->drag_button = 0;
			    break;

			  case GWEventTypePointerMotion:
			    if(slider->drag_button == 1)
			    {
				DO_CALCULATE_VALUE
				SARMenuSliderSetValue(
				    display, m, i, value, True
				);
				events_handled++;
			    }
			    break;

			  case GWEventType2ButtonPress:
			  case GWEventType3ButtonPress:
			    break;
			}
#undef DO_CALCULATE_VALUE
		    }
		}
		break;

	      /* ***************************************************** */
	      case SAR_MENU_OBJECT_TYPE_MAP:
		map = SAR_MENU_MAP(o);
		if(!map->sensitive)
		    break;
		events_handled += SARMenuMapManagePointer(
		    display, m, map, i,
		    x, y, type, btn_num
		);
		break;

	      /* ***************************************************** */
	      case SAR_MENU_OBJECT_TYPE_OBJVIEW:
		objview = SAR_MENU_OBJVIEW(o);
		if(!objview->sensitive)
		    break;
		events_handled += SARMenuObjViewManagePointer(
		    display, m, objview, i,
		    x, y, type, btn_num
		);
		break;
	    }
	}

	return(events_handled);
}


/*
 *	Manages a key event assumed for the menu.
 */
int SARMenuManageKey(
	gw_display_struct *display, sar_menu_struct *m,
	int key, Boolean state
)
{
	int i, prev_sel_num, events_handled = 0;
	int items_visable = 0;
	int fw, fh, w, h, width, height;
	void *o;
	sar_menu_button_struct *button;
	sar_menu_message_box_struct *mesgbox;
	sar_menu_list_struct *list;
	sar_menu_switch_struct *sw;
	sar_menu_spin_struct *spin;
	sar_menu_slider_struct *slider;
	sar_menu_objview_struct *ov;


	if((display == NULL) || (m == NULL))
	    return(events_handled);

#if 0
	/* Always ignore key releases */
	if(!state)
	    return(events_handled);
#endif

	GWContextGet(
	    display, GWContextCurrent(display),
	    NULL, NULL,
	    NULL, NULL,
	    &width, &height
	);


	/* Menu global key functions */
	switch(key)
	{
	  /* Tab changes selected object */
	  case '\t':
	    if(!state)
	    {
		events_handled++;
		break;
	    }

	    prev_sel_num = m->selected_object;

#define TYPE_SELECTABLE(t)	(		\
 ((t) != SAR_MENU_OBJECT_TYPE_LABEL) &&		\
 ((t) != SAR_MENU_OBJECT_TYPE_PROGRESS)		\
)

	    if(display->shift_key_state)
	    {
		/* Seek backwards until we get a selectable object
		 * type or the list has cycled
		 */
		sar_menu_object_type type;
		int status = 0;
		m->selected_object--;
		while(m->selected_object >= 0)
		{
		    o = SARMenuGetObject(m, m->selected_object);
		    if((o != NULL) &&
		       SARMenuObjectIsSensitive(m, m->selected_object)
		    )
		    {
			type = SAR_MENU_OBJECT_TYPE(o);
			if(TYPE_SELECTABLE(type))
			{
			    status = 1;
			    break;
			}
		    }
		    m->selected_object--;
		}
		/* Did not get a match? */
		if(!status)
		{
		    /* From the end */
		    m->selected_object = m->total_objects - 1;
		    while(m->selected_object > prev_sel_num)
		    {
			o = SARMenuGetObject(m, m->selected_object);
			if((o != NULL) &&
			   SARMenuObjectIsSensitive(m, m->selected_object)
			)
			{
			    type = SAR_MENU_OBJECT_TYPE(o);
			    if(TYPE_SELECTABLE(type))
			    {
				status = 1;
				break;
			    }
			}
			m->selected_object--;
		    }
		}
		/* Still no match? */
		if(!status)
		    m->selected_object = prev_sel_num;
	    }
	    else
	    {
		/* Seek forwards until we get a selectable object
		 * type or the list has cycled
		 */
		sar_menu_object_type type;
		int status = 0;
		m->selected_object++;
		while(m->selected_object < m->total_objects)
		{
		    o = SARMenuGetObject(m, m->selected_object);
		    if((o != NULL) &&
		       SARMenuObjectIsSensitive(m, m->selected_object)
		    )
		    {
			type = SAR_MENU_OBJECT_TYPE(o);
			if(TYPE_SELECTABLE(type))
			{
			    status = 1;
			    break;
			}
		    }
		    m->selected_object++;
		}
		/* Didn't get a match? */
		if(!status)
		{
		    /* From the beginning */
		    m->selected_object = 0;
		    while(m->selected_object < prev_sel_num)
		    {
			o = SARMenuGetObject(m, m->selected_object);
			if((o != NULL) &&
			   SARMenuObjectIsSensitive(m, m->selected_object)
			)
			{
			    type = SAR_MENU_OBJECT_TYPE(o);
			    if(TYPE_SELECTABLE(type))
			    {
				status = 1;
				break;
			    }
			}
			m->selected_object++;
		    }
		}
		/* Still no match? */
		if(!status)
		    m->selected_object = prev_sel_num;
	    }

	    /* Sanitize */
	    if(m->selected_object >= m->total_objects)
		m->selected_object = 0;
	    else if(m->selected_object < 0)
		m->selected_object = m->total_objects - 1;

	    if(m->selected_object < 0)
		m->selected_object = 0;

	    /* If a menu's selected object is this button and this
	     * button is unarmed, it needs to be highlighted
	     */
	    o = SARMenuGetObject(m, prev_sel_num);
	    if(o != NULL)
	    {
		if(SAR_MENU_IS_BUTTON(o))
		{
		    button = SAR_MENU_BUTTON(o);
		    button->state = SAR_MENU_BUTTON_STATE_UNARMED;
		}
	    }
	    o = SARMenuGetObject(m, m->selected_object);
	    if(o != NULL)
	    {
		if(SAR_MENU_IS_BUTTON(o))
		{
		    button = SAR_MENU_BUTTON(o);
		    button->state = SAR_MENU_BUTTON_STATE_HIGHLIGHTED;
		}
	    }

	    DO_REDRAW_MENU(display,m)
	    events_handled++;
	    break;
	}
	if(events_handled > 0)
	    return(events_handled);


	/* Key on selected widget */
	i = m->selected_object;
	o = SARMenuGetObject(m, i);
	if(o != NULL)
	{
	    /* Handle by widget type */
	    switch(SAR_MENU_OBJECT_TYPE(o))
	    {
	      /* **************************************************** */
	      case SAR_MENU_OBJECT_TYPE_BUTTON:
		button = SAR_MENU_BUTTON(o);
		if(!button->sensitive)
		    break;

		switch(key)
		{
		  case '\n': case ' ':
		    if(state)
		    {
			button->state = SAR_MENU_BUTTON_STATE_ARMED;
			DO_REDRAW_OBJECT(display, m, i)
		    }
		    else if(button->state == SAR_MENU_BUTTON_STATE_ARMED)
		    {
			button->state = SAR_MENU_BUTTON_STATE_HIGHLIGHTED;
			DO_REDRAW_OBJECT(display, m, i)
			if(button->func_cb != NULL)
			    button->func_cb(
				button,			/* Object */
				button->id,		/* ID Code */
				button->client_data	/* Data */
			    );
		    }
		    events_handled++;
		    break;
		}
		break;

	      /* **************************************************** */
	      case SAR_MENU_OBJECT_TYPE_MESSAGE_BOX:
		mesgbox = SAR_MENU_MESSAGE_BOX(o);
		if(!mesgbox->sensitive)
		    break;

		if(!state)
		    break;

		w = (int)(mesgbox->width * width);
		if(w <= 0)
		    w = (int)(width - (2 * DEF_MB_XMARGIN));
		if(w <= 0)
		    w = DEF_MB_XMARGIN;

		if(mesgbox->height > 0)
		    h = (int)(mesgbox->height * height);
		else
		    h = DEF_HEIGHT;
		if(h <= 0)
		    h = DEF_MB_YMARGIN;

		if(mesgbox->message != NULL)
		{
		    int	total_lines, columns_visable,
			lines_visable, scroll_width;


		    GWGetFontSize(
			mesgbox->font,
			NULL, NULL,
			&fw, &fh
		    );

		    /* Calculate colums visible */
		    if(fw > 0)
			columns_visable = (w - DEF_SCROLL_CURSOR_WIDTH
			    - (2 * DEF_MB_XMARGIN)) / fw;
		    else
			columns_visable = 1;
		    if(columns_visable < 1)
			columns_visable = 1; 

		    /* Calculate width (height) of scroll indicator */
		    scroll_width = fh + (2 * DEF_MB_YMARGIN);

		    /* Calculate lines visable */
		    if(fh > 0)
			lines_visable = (h - (2 * DEF_MB_YMARGIN)) / fh;
		    else
			lines_visable = 0;
		    if(lines_visable < 0)
			lines_visable = 0;

		    /* Calculate total number of lines */
		    total_lines = SARMenuMessageBoxTotalLines(
			mesgbox->message, columns_visable
		    );

		    switch(key)
		    {
		      case GWKeyHome:
			events_handled++;
			mesgbox->scrolled_line = 0;
			DO_REDRAW_OBJECT(display, m, i)
			break;

		      case GWKeyEnd:
			events_handled++;
			mesgbox->scrolled_line =
			    total_lines - (lines_visable / 2);
			if(mesgbox->scrolled_line < 0)
			    mesgbox->scrolled_line = 0;
			DO_REDRAW_OBJECT(display, m, i)
			break;

		      case GWKeyUp:
			events_handled++;
			mesgbox->scrolled_line--;
			if(mesgbox->scrolled_line < 0)
			    mesgbox->scrolled_line = 0;
			DO_REDRAW_OBJECT(display, m, i)
			break;

		      case GWKeyDown:
			events_handled++;
			mesgbox->scrolled_line++;
			if(mesgbox->scrolled_line >=
			    (total_lines - (lines_visable / 2))
			)
			    mesgbox->scrolled_line = total_lines - (lines_visable / 2);
			if(mesgbox->scrolled_line < 0)
			    mesgbox->scrolled_line = 0;
			DO_REDRAW_OBJECT(display, m, i)
			break;

		      case GWKeyPageUp:
		      case 'b': case '-': case 'p':
			events_handled++;
			mesgbox->scrolled_line -= lines_visable;
			if(mesgbox->scrolled_line < 0)
			    mesgbox->scrolled_line = 0;
			DO_REDRAW_OBJECT(display, m, i)
			break;

		      case GWKeyPageDown:
		      case ' ': case 'n':
			events_handled++;
			mesgbox->scrolled_line += lines_visable;
			if(mesgbox->scrolled_line >=
			    (total_lines - (lines_visable / 2))
			)
			    mesgbox->scrolled_line = total_lines - (lines_visable / 2);
			if(mesgbox->scrolled_line < 0)
			    mesgbox->scrolled_line = 0;
			DO_REDRAW_OBJECT(display, m, i)
			break;
		    }
		}
		break;

	      /* **************************************************** */
	      case SAR_MENU_OBJECT_TYPE_LIST:
		list = SAR_MENU_LIST(o);
		if(!list->sensitive)
		    break;

		if(!state)
		    break;

		w = (int)(list->width * width);
		if(w <= 0)
		    w = width - (2 * DEF_LIST_XMARGIN);
		if(w <= 0)
		    w = DEF_LIST_XMARGIN;
		 
		if(list->height > 0)
		    h = (int)(list->height * height);
		else
		    h = DEF_HEIGHT;
		if(h <= 0)
		    h = DEF_LIST_YMARGIN;

		GWGetFontSize(
		    list->font,
		    NULL, NULL,
		    &fw, &fh
		);
		/* Calculate items visible, remember to subtract 3
		 * items for borders and heading
		 */
		if(fh > 0)
		    items_visable = MAX(
			(h / fh) - 3,
			0
		    );
		else
		    items_visable = 0;

		/* Update items visable on list */
		list->items_visable = items_visable;

		/* Record previously selected item */
		prev_sel_num = list->selected_item;

		switch(key)
		{
		  case GWKeyHome:
		    events_handled++;
		    list->selected_item = 0;
		    list->scrolled_item = 0;
		    DO_REDRAW_OBJECT(display, m, i)
		    break;

		  case GWKeyEnd:
		    events_handled++;
		    list->selected_item = list->total_items - 1;
		    if(list->selected_item < 0)
			list->selected_item = 0;
		    list->scrolled_item = list->total_items -
			(items_visable / 2);
		    if(list->scrolled_item < 0)
			list->scrolled_item = 0;
		    DO_REDRAW_OBJECT(display, m, i)
		    break;

		  case GWKeyUp:
		    events_handled++;
		    list->selected_item--;
		    if(list->selected_item < 0)
			list->selected_item = 0;
		    if(list->selected_item < list->scrolled_item)
		    {
			list->scrolled_item -= (items_visable + 1);
			if(list->scrolled_item < 0)
			    list->scrolled_item = 0;
		    }
		    DO_REDRAW_OBJECT(display, m, i)
		    break;

		  case GWKeyDown:
		    events_handled++;
		    list->selected_item++;
		    if(list->selected_item >= list->total_items)
			list->selected_item = list->total_items - 1;
		    if(list->selected_item < 0)
		       list->selected_item = 0;
		    if(list->selected_item > (list->scrolled_item +
			items_visable)
		    )
		    {
			list->scrolled_item = list->selected_item;
		    }
		    DO_REDRAW_OBJECT(display, m, i)
		    break;

		  case GWKeyPageUp:
		  case 'b': case '-': case 'p':
		    events_handled++;
		    list->scrolled_item -= items_visable;
		    if(list->scrolled_item < 0)
			list->scrolled_item = 0;

		    list->selected_item = list->scrolled_item;

		    DO_REDRAW_OBJECT(display, m, i)
		    break;

		  case GWKeyPageDown:
		  case ' ': case 'n':
		    events_handled++;
		    list->scrolled_item += items_visable;
		    if(list->scrolled_item >=
			(list->total_items - (items_visable / 2))
		    )
			list->scrolled_item = list->total_items -
			    (items_visable / 2);
		    if(list->scrolled_item < 0)
			list->scrolled_item = 0;
		    list->selected_item = list->scrolled_item;
		    DO_REDRAW_OBJECT(display, m, i)
		    break;
		}

		/* Call select notify function */
		if((list->select_cb != NULL) &&
		   (prev_sel_num != list->selected_item) &&
		   (list->selected_item >= 0) &&
		   (list->selected_item < list->total_items)
		)
		{
		    sar_menu_list_item_struct *item = list->item[list->selected_item];
		    if(item != NULL)
			list->select_cb(
			    list,			/* This Object */
			    list->id,			/* ID Code */
			    list->client_data,		/* Data */
			    list->selected_item, 	/* Item Number */
			    item,			/* Item Pointer */
			    item->client_data		/* Item Data */
			);
		}
		break; 

	      /* **************************************************** */
	      case SAR_MENU_OBJECT_TYPE_SWITCH:
		sw = SAR_MENU_SWITCH(o);
		if(!sw->sensitive)
		    break;

		if(!state)
		    break;

		switch(key)
		{
		  case GWKeyUp:
		  case GWKeyDown:
		  case GWKeyLeft:
		  case GWKeyRight:
		  case ' ': case '\n':
		    events_handled++;
		    sw->state = !sw->state;
		    DO_REDRAW_OBJECT(display, m, i)
		    if(sw->switch_cb != NULL)
			sw->switch_cb(
			    sw,			/* This Object */
			    sw->id,		/* ID Code */
			    sw->client_data,	/* Data */
			    sw->state		/* Value */
			);
		    break;
		}
		break;

	      /* ***************************************************** */
	      case SAR_MENU_OBJECT_TYPE_SPIN:
		spin = SAR_MENU_SPIN(o);
		if(!spin->sensitive)
		    break;

		switch(key)
		{
		  case GWKeyRight:
		  case GWKeyDown:
		  case GWKeyPageDown:
		  case ' ': case '\n':
		  case '+': case '=': case 'n':
		    if(state)
		    {
			spin->inc_state = SAR_MENU_BUTTON_STATE_ARMED;
		    }
		    else if(spin->inc_state == SAR_MENU_BUTTON_STATE_ARMED)
		    {
			spin->inc_state = SAR_MENU_BUTTON_STATE_UNARMED;
			SARMenuSpinDoInc(display, m, i, False);
		    }
		    DO_REDRAW_OBJECT(display, m, i)
		    events_handled++;
		    break;
		  case GWKeyLeft:
		  case GWKeyUp:
		  case GWKeyPageUp:
		  case GWKeyBackSpace:
		  case '-': case '_': case 'b':
		    if(state)
		    {
			spin->dec_state = SAR_MENU_BUTTON_STATE_ARMED;
		    }
		    else if(spin->dec_state == SAR_MENU_BUTTON_STATE_ARMED)
		    {
			spin->dec_state = SAR_MENU_BUTTON_STATE_UNARMED;
			SARMenuSpinDoDec(display, m, i, False);
		    }
		    DO_REDRAW_OBJECT(display, m, i)
		    events_handled++;
		    break;
		  case GWKeyHome:
		    if(state)
		    {
			spin->dec_state = SAR_MENU_BUTTON_STATE_ARMED;
		    }
		    else if(spin->dec_state == SAR_MENU_BUTTON_STATE_ARMED)
		    {
			spin->dec_state = SAR_MENU_BUTTON_STATE_UNARMED;
			SARMenuSpinSelectValueIndex(
			    display, m, i,
			    0,
			    False
			);
		    }
		    DO_REDRAW_OBJECT(display, m, i)
		    events_handled++;
		    break;
		  case GWKeyEnd:
		    if(state)
		    {
			spin->inc_state = SAR_MENU_BUTTON_STATE_ARMED;
		    }
		    else if(spin->inc_state == SAR_MENU_BUTTON_STATE_ARMED)
		    {
			spin->inc_state = SAR_MENU_BUTTON_STATE_UNARMED;
			SARMenuSpinSelectValueIndex(
			    display, m, i,
			    MAX(spin->total_values - 1, 0),
			    False
			);
		    }
		    DO_REDRAW_OBJECT(display, m, i)
		    events_handled++;
		    break;
		}
		break;

	      /* ***************************************************** */
	      case SAR_MENU_OBJECT_TYPE_SLIDER:
		slider = SAR_MENU_SLIDER(o);
		if(!slider->sensitive)
		    break;

		if(!state)
		    break;

		if(slider->upper > slider->lower)
		{
		    float inc = (float)(slider->upper - slider->lower) * 0.1f;
		    float page_inc = (float)(slider->upper - slider->lower) * 0.25f;
		    float value;

		    switch(key)
		    {
		      case GWKeyRight:
		      case GWKeyUp:
		      case '+': case '=': case ' ': case '\n':
			value = CLIP(
			    slider->value + inc,
			    slider->lower, slider->upper
			);
			SARMenuSliderSetValue(
			    display, m, i, value, True
			);
			events_handled++;
			break;
		      case GWKeyLeft:
		      case GWKeyDown:
		      case '_': case '-':
			value = CLIP(
			    slider->value - inc,
			    slider->lower, slider->upper
			);
			SARMenuSliderSetValue(
			    display, m, i, value, True
			);
			events_handled++;
			break;
		      case GWKeyPageUp:
			value = CLIP(
			    slider->value + page_inc,
			    slider->lower, slider->upper
			);
			SARMenuSliderSetValue(
			    display, m, i, value, True
			);
			events_handled++;
			break;
		      case GWKeyPageDown:
			value = CLIP(
			    slider->value - page_inc,
			    slider->lower, slider->upper
			);
			SARMenuSliderSetValue(
			    display, m, i, value, True
			);
			events_handled++;
			break;
		      case GWKeyHome:
			value = slider->lower;
			SARMenuSliderSetValue(
			    display, m, i, value, True
			);
			events_handled++;
			break;
		      case GWKeyEnd:
			value = slider->upper;
			SARMenuSliderSetValue(
			    display, m, i, value, True
			);
			events_handled++;
			break;
		    }
		}
		break;

	      /* **************************************************** */
	      case SAR_MENU_OBJECT_TYPE_OBJVIEW:
		ov = SAR_MENU_OBJVIEW(o);
		if(!ov->sensitive)
		    break;
		SARMenuObjViewManageKey(
		    display, m, ov, i,
		    key, state
		);
		break;
	    }
	}

	return(events_handled);
}
