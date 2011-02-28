#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sys/types.h>

#ifdef __MSW__
# include <windows.h>
#endif
  
#include <GL/gl.h>
#include <GL/glu.h>
	 
#include "../include/string.h"  
#include "../include/fio.h"

#include "v3dtex.h"
#include "v3dfio.h"
#include "v3dgl.h"
#include "gw.h"
#include "stategl.h"
#include "image.h"
#include "menu.h"
#include "sar.h"
#include "config.h"


int SARMenuObjViewNew(
	sar_menu_struct *m,
	float x, float y,
	float width, float height,
	GWFont *font,
	sar_menu_color_struct *color,
	const sar_image_struct **bg_image,      /* Pointer to 9 images */
	const char *title
);
int SARMenuObjViewManagePointer(
	gw_display_struct *display, sar_menu_struct *m,
	sar_menu_objview_struct *ov_ptr, int ov_num,
	int x, int y, gw_event_type type, int btn_num
);
int SARMenuObjViewManageKey(
	gw_display_struct *display, sar_menu_struct *m,
	sar_menu_objview_struct *ov_ptr, int ov_num,
	int key, Boolean state
);
static int SARMenuObjViewLoadLine(
	gw_display_struct *display, sar_menu_struct *m,
	sar_menu_objview_struct *ov_ptr, int ov_num,
	const char *line
);
int SARMenuObjViewLoad(
	gw_display_struct *display, sar_menu_struct *m,
	sar_menu_objview_struct *ov_ptr, int ov_num,
	const char *filename
);
void SARMenuObjViewRebufferImage(
	gw_display_struct *display, sar_menu_struct *m,
	sar_menu_objview_struct *ov_ptr, int ov_num
);


#define ATOI(s)         (((s) != NULL) ? atoi(s) : 0)
#define ATOL(s)         (((s) != NULL) ? atol(s) : 0)
#define ATOF(s)         (((s) != NULL) ? atof(s) : 0.0f)
#define STRDUP(s)       (((s) != NULL) ? strdup(s) : NULL)

#define MAX(a,b)	(((a) > (b)) ? (a) : (b))
#define MIN(a,b)	(((a) < (b)) ? (a) : (b))
#define CLIP(a,l,h)	(MIN(MAX((a),(l)),(h)))

#define RADTODEG(r)     ((r) * 180 / PI)
#define DEGTORAD(d)     ((d) * PI / 180)

#define ISCOMMENT(c)    ((c) == SAR_COMMENT_CHAR)
#define ISCR(c)         (((c) == '\n') || ((c) == '\r'))


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
 *	Creates a 3d object viewer object on menu m, returns the index
 *	number of the 3d object viewer object or -1 on failure.
 */
int SARMenuObjViewNew(
	sar_menu_struct *m,
	float x, float y,
	float width, float height,
	GWFont *font,
	sar_menu_color_struct *color,
	const sar_image_struct **bg_image,      /* Pointer to 9 images */
	const char *title
)
{
	int i, n;
	sar_menu_color_struct *c;
	sar_menu_objview_struct *objview;


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
        /*
	m->object[n] = (void *)objview = SAR_MENU_OBJVIEW(calloc(
	    1, sizeof(sar_menu_objview_struct) ));
        */
        objview = SAR_MENU_OBJVIEW( calloc(1, sizeof(sar_menu_objview_struct) ) );
        
	if(objview == NULL)
	    return(-1);
        m->object[n] = objview;

	/* Set 3d object viewer values */
	objview->type = SAR_MENU_OBJECT_TYPE_OBJVIEW;
	objview->x = x;
	objview->y = y;
	objview->width = width;
	objview->height = height;
	objview->sensitive = True;
	objview->font = font;
	if(color != NULL)
	    memcpy(&objview->color, color, sizeof(sar_menu_color_struct));
	c = &objview->bg_color;
/*
	c->a = 1.0f;
	c->r = 0.52f;
	c->g = 0.69f;
	c->b = 1.00f;
 */
	c->a = 1.0f;
	c->r = 0.3f;
	c->g = 0.3f;
	c->b = 0.3f;
	c = &objview->bold_color;
	c->a = 1.0f;
	c->r = 1.0f;
	c->g = 1.0f;
	c->b = 0.0f;
	c = &objview->underline_color;
	c->a = 1.0f;
	c->r = 0.5f;
	c->g = 0.5f;
	c->b = 1.0f;
	objview->bg_image = bg_image;
	objview->rotate_drag_state = False;
	objview->bank_drag_state = False;
	objview->prev_motion_x = 0;
	objview->prev_motion_y = 0;
	objview->last_motion_time = 0;
	objview->render_lighting = True;
	objview->render_antialias = False;
	objview->title = STRDUP(title);
	objview->message = NULL;
	objview->show_message = True;
	objview->obj_list = NULL;
	objview->obj_size_radius = 0.0f;
	objview->obj_heading = (float)(1.5 * PI);
	objview->obj_pitch = (float)(0.0 * PI);
	objview->obj_bank = (float)(0.0 * PI);
	objview->camera_distance = 20.0f;
	objview->light_heading = (float)(1.25 * PI);
	objview->light_pitch = (float)(1.75 * PI);
	objview->light_bank = (float)(0.0 * PI);
	objview->objimg = NULL;
	objview->objimg_width = 0;
	objview->objimg_height = 0;

	return(n);
}


/*
 *	Manages the pointer event with respect to the given menu 3d
 *	object viewer object.
 */
int SARMenuObjViewManagePointer(
	gw_display_struct *display, sar_menu_struct *m,
	sar_menu_objview_struct *ov_ptr, int ov_num,
	int x, int y, gw_event_type type, int btn_num
)
{
	int events_handled = 0;
	int x_min, x_max, y_min, y_max, width, height, w, h;

	if((display == NULL) || (m == NULL) || (ov_ptr == NULL))
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
	if((ov_ptr->rotate_drag_state || ov_ptr->bank_drag_state) &&
	   (type == GWEventTypeButtonRelease)
	)
	{
	    ov_ptr->rotate_drag_state = False;
	    ov_ptr->bank_drag_state = False;
	}

	w = (int)(ov_ptr->width * width);
	if(w <= 0)
	    w = (int)(width - (2 * DEF_XMARGIN));
	if(w <= 0)
	    w = DEF_XMARGIN;

	if(ov_ptr->height > 0)
	    h = (int)(ov_ptr->height * height);
	else
	    h = DEF_HEIGHT;
	if(h <= 0)
	    h = DEF_YMARGIN;

	/* Get coordinate bounds */
	x_min = (int)((ov_ptr->x * width) - (w / 2));
	x_max = (int)(x_min + w);
	y_min = (int)((ov_ptr->y * height) - (h / 2));
	y_max = (int)(y_min + h);

	/* Pointer event in bounds? */
	if((x >= x_min) && (x < x_max) &&
	   (y >= y_min) && (y < y_max)
	)
	{
	    /* Handle by event type */
	    switch(type)
	    {
	      case GWEventTypeButtonPress:
		events_handled++;
		/* Select new object as needed and redraw the
		 * previously selected object
		 */
		if(m->selected_object != ov_num)
		{
		    int po = m->selected_object;
		    m->selected_object = ov_num;
		    if(m->always_full_redraw)
			SARMenuDrawAll(display, m);
		    else
			SARMenuDrawObject(display, m, po);
		}
		/* Handle by button number */
		switch(btn_num)
		{
		  case 1:
		    ov_ptr->rotate_drag_state = True;
		    ov_ptr->last_motion_time = cur_millitime;
		    ov_ptr->prev_motion_x = x;
		    ov_ptr->prev_motion_y = y;

		    GWSetPointerCursor(
			display, GWPointerCursorTranslate
		    );
		    DO_REDRAW_OBJECT(display, m, ov_num)
		    break;

		  case 3:
		    ov_ptr->bank_drag_state = True;
		    ov_ptr->last_motion_time = cur_millitime;
		    ov_ptr->prev_motion_x = x;
		    ov_ptr->prev_motion_y = y;

		    GWSetPointerCursor(
			display, GWPointerCursorTranslate
		    );
		    DO_REDRAW_OBJECT(display, m, ov_num)
		    break;
		}
		break;

	      case GWEventTypeButtonRelease:
		GWSetPointerCursor(
		    display, GWPointerCursorStandard
		);

		/* Manipulate last motion time so that the following
	         * case checks it properly
		 */
		ov_ptr->last_motion_time = cur_millitime - 1;

		/* Fall through */
	      case GWEventTypePointerMotion:
		/* Too soon to handle a motion event? */
		if(ov_ptr->last_motion_time == cur_millitime)
		    break;
		else
		    ov_ptr->last_motion_time = cur_millitime;
		events_handled++;

		/* Rotating? */
		if(ov_ptr->rotate_drag_state)
		{
		    int dx = x - ov_ptr->prev_motion_x,
			dy = y - ov_ptr->prev_motion_y;
		    float movement_coeff;


		    if(width > 0)
			movement_coeff = (float)dx / (float)width;
		    else
			movement_coeff = 0.0f;

		    ov_ptr->obj_heading += (float)(movement_coeff * 2.0 * PI);
		    while(ov_ptr->obj_heading >= (2.0 * PI))
			ov_ptr->obj_heading -= (float)(2.0 * PI);
		    while(ov_ptr->obj_heading < (0.0 * PI))
			ov_ptr->obj_heading += (float)(2.0 * PI);


		    if(height > 0)
			movement_coeff = (float)dy / (float)height;
		    else
			movement_coeff = 0.0f;

		    ov_ptr->obj_pitch += (float)(movement_coeff * 2.0 * PI);
		    while(ov_ptr->obj_pitch >= (2.0 * PI))
			ov_ptr->obj_pitch -= (float)(2.0 * PI);
		    while(ov_ptr->obj_pitch < (0.0 * PI))
			ov_ptr->obj_pitch += (float)(2.0 * PI);

		    /* Redraw 3d object and regenerate buffered object
		     * image.
		     */
		    SARMenuObjViewRebufferImage(
			display, m, ov_ptr, ov_num
		    );

		    /* Need to redraw everything, since the regeneration
		     * of the buffered object would have messed with
		     * the back buffer and thus the surrounding graphics.
		     *
		     * So simply redrawing the menu object is not
		     * enough to cover all the modified fragments.
		     */
		    GWPostRedraw(display);
		}
		/* Bank? */
		else if(ov_ptr->bank_drag_state)
		{
		    int dx = x - ov_ptr->prev_motion_x;
		    float movement_coeff;


		    if(width > 0)
			movement_coeff = (float)dx / (float)width;
		    else
			movement_coeff = 0.0f;

		    ov_ptr->obj_bank += (float)(movement_coeff * 2.0 * PI);
		    while(ov_ptr->obj_bank >= (2.0 * PI))
			ov_ptr->obj_bank -= (float)(2.0 * PI);
		    while(ov_ptr->obj_bank < (0.0 * PI))
			ov_ptr->obj_bank += (float)(2.0 * PI);


		    /* Redraw 3d object and regenerate buffered object
		     * image.
		     */
		    SARMenuObjViewRebufferImage(
			display, m, ov_ptr, ov_num
		    );

		    /* Need to redraw everything, since the regeneration
		     * of the buffered object would have messed with
		     * the back buffer and thus the surrounding graphics.
		     *
		     * So simply redrawing the menu object is not
		     * enough to cover all the modified fragments.
		     */
		    GWPostRedraw(display);
		}

		/* Record previous pointer coordinates */
		ov_ptr->prev_motion_x = x;
		ov_ptr->prev_motion_y = y;
		break;

	      case GWEventType2ButtonPress:
	      case GWEventType3ButtonPress:
		break;
	    }	/* Handle by event type */

	}	/* Pointer event in bounds? */

	return(events_handled);
}

/*
 *      Manages the key event with respect to the given menu 3d
 *      object viewer object.
 */
int SARMenuObjViewManageKey(
	gw_display_struct *display, sar_menu_struct *m,
	sar_menu_objview_struct *ov_ptr, int ov_num,
	int key, Boolean state
)
{
	int events_handled = 0;

	if((display == NULL) || (m == NULL) || (ov_ptr == NULL) || !state)
	    return(events_handled);

	if(!ov_ptr->sensitive)
	    return(events_handled);

	if(True)
	{
	    switch(key)
	    {
	      case '+': case '=':
		if(ov_ptr->camera_distance > ov_ptr->obj_size_radius)
		{
		    float d = MAX(
			(ov_ptr->camera_distance - ov_ptr->obj_size_radius) *
			    (display->shift_key_state ? 0.5f : 0.1f),
			0.05f
		    );
		    ov_ptr->camera_distance -= d;
		    if(ov_ptr->camera_distance < ov_ptr->obj_size_radius)
			ov_ptr->camera_distance = ov_ptr->obj_size_radius;
		}
		events_handled++;
		break;

	      case GWKeyPageUp:
		if(ov_ptr->camera_distance > ov_ptr->obj_size_radius)
		{
		    float d = MAX(
			(ov_ptr->camera_distance - ov_ptr->obj_size_radius) * 0.5f,
			0.05f
		    );
		    ov_ptr->camera_distance -= d;
		    if(ov_ptr->camera_distance < ov_ptr->obj_size_radius)
			ov_ptr->camera_distance = ov_ptr->obj_size_radius;
		}
		events_handled++;
		break;

	      case '-': case '_':
		if(True)
		{
		    float d = MAX(
			(ov_ptr->camera_distance - ov_ptr->obj_size_radius) *
			    (display->shift_key_state ? 0.5f : 0.1f),
			0.05f
		    );
		    ov_ptr->camera_distance += d;
		}
		events_handled++;
		break;

	      case GWKeyPageDown:
		if(True)
		{
		    float d = MAX(
			(ov_ptr->camera_distance - ov_ptr->obj_size_radius) * 0.5f,
			0.05f
		    );
		    ov_ptr->camera_distance += d;
		}
		events_handled++;
		break;

	      case GWKeyLeft:
		if(display->ctrl_key_state)
		{
		    ov_ptr->obj_bank -= (float)(
			(2.0 * PI) *
			(display->shift_key_state ? 0.125 : 0.0625)
		    );
		    while(ov_ptr->obj_bank >= (float)(2.0 * PI))
			ov_ptr->obj_bank -= (float)(2.0 * PI);
		    while(ov_ptr->obj_bank < (float)(0.0 * PI))
			ov_ptr->obj_bank += (float)(2.0 * PI);
		}
		else
		{
		    ov_ptr->obj_heading -= (float)(
			(2.0 * PI) *
			(display->shift_key_state ? 0.125 : 0.0625)
		    );
		    while(ov_ptr->obj_heading >= (float)(2.0 * PI))
			ov_ptr->obj_heading -= (float)(2.0 * PI);
		    while(ov_ptr->obj_heading < (float)(0.0 * PI))
			ov_ptr->obj_heading += (float)(2.0 * PI);
		}
		events_handled++;
		break;

	      case GWKeyRight:
		if(display->ctrl_key_state)
		{
		    ov_ptr->obj_bank += (float)(
			(2.0 * PI) *
			(display->shift_key_state ? 0.125 : 0.0625)
		    );
		    while(ov_ptr->obj_bank >= (float)(2.0 * PI))
			ov_ptr->obj_bank -= (float)(2.0 * PI);
		    while(ov_ptr->obj_bank < (float)(0.0 * PI))
			ov_ptr->obj_bank += (float)(2.0 * PI);
		}
		else
		{
		    ov_ptr->obj_heading += (float)(
			(2.0 * PI) *
			(display->shift_key_state ? 0.125 : 0.0625)
		    );
		    while(ov_ptr->obj_heading >= (float)(2.0 * PI))
			ov_ptr->obj_heading -= (float)(2.0 * PI);
		    while(ov_ptr->obj_heading < (float)(0.0 * PI))
			ov_ptr->obj_heading += (float)(2.0 * PI);
		}
		events_handled++;
		break;

	      case ' ': case '\n':
		if(True)
		{
		    ov_ptr->obj_heading -= (float)(
			(2.0 * PI) * 0.25
		    );
		    while(ov_ptr->obj_heading >= (float)(2.0 * PI))
			ov_ptr->obj_heading -= (float)(2.0 * PI);
		    while(ov_ptr->obj_heading < (float)(0.0 * PI))
			ov_ptr->obj_heading += (float)(2.0 * PI);
		}
		events_handled++;
		break;

	      case GWKeyUp:
		if(True)
		{
		    ov_ptr->obj_pitch -= (float)(
			(2.0 * PI) *
			(display->shift_key_state ? 0.125 : 0.0625)
		    );
		    while(ov_ptr->obj_pitch >= (float)(2.0 * PI))
			ov_ptr->obj_pitch -= (float)(2.0 * PI);
		    while(ov_ptr->obj_pitch < (float)(0.0 * PI))
			ov_ptr->obj_pitch += (float)(2.0 * PI);
		}
		events_handled++;
		break;

	      case GWKeyDown:
		if(True)
		{
		    ov_ptr->obj_pitch += (float)(
			(2.0 * PI) *
			(display->shift_key_state ? 0.125 : 0.0625)
		    );
		    while(ov_ptr->obj_pitch >= (float)(2.0 * PI))
			ov_ptr->obj_pitch -= (float)(2.0 * PI);
		    while(ov_ptr->obj_pitch < (float)(0.0 * PI))
			ov_ptr->obj_pitch += (float)(2.0 * PI);
		}
		events_handled++;
		break;

	      case GWKeyHome:
		if(True)
		{
		    ov_ptr->obj_heading = (float)(0.0 * PI);
		    ov_ptr->obj_pitch = (float)(0.0 * PI);
		    ov_ptr->obj_bank = (float)(0.0 * PI);
		}
		events_handled++;
		break;

	      case GWKeyEnd:
		if(True)
		{
		    ov_ptr->obj_heading = (float)(1.0 * PI);
		    ov_ptr->obj_pitch = (float)(0.0 * PI);
		    ov_ptr->obj_bank = (float)(0.0 * PI);
		}
		events_handled++;
		break;

	      case 'b': case 'B':
		if(display->shift_key_state)
		{
		    sar_menu_color_struct *c = &ov_ptr->bg_color;
		    if(c->r >= 1.0f)
		    {
			c->r = 0.52f;
			c->g = 0.69f;
			c->b = 1.00f;
		    }
		    else if(c->r >= 0.5f)
		    {
			c->r = 0.3f;
			c->g = 0.3f;
			c->b = 0.3f;
		    }
		    else if(c->r >= 0.3f)
		    {
			c->r = 0.0f;
			c->g = 0.0f;
			c->b = 0.0f;
		    }
		    else
		    {
			c->r = 1.0f;
			c->g = 1.0f;
			c->b = 1.0f;
		    }
		}
		else
		{
		    sar_menu_color_struct *c = &ov_ptr->bg_color;
		    if(c->r >= 1.0f)
		    {
			c->r = 0.0f;
			c->g = 0.0f;
			c->b = 0.0f;
		    }
		    else if(c->r >= 0.5f)
		    {
			c->r = 1.0f;
			c->g = 1.0f;
			c->b = 1.0f;
		    }
		    else if(c->r >= 0.3f)
		    {
			c->r = 0.52f;
			c->g = 0.69f;
			c->b = 1.00f;
		    }
		    else
		    {
			c->r = 0.3f;
			c->g = 0.3f;
			c->b = 0.3f;
		    }
		}
		events_handled++;
		break;

	      case 'm': case 'M':
		ov_ptr->show_message = !ov_ptr->show_message;
		events_handled++;
		break;

	      case 'a': case 'A':
		ov_ptr->render_antialias = !ov_ptr->render_antialias;
		events_handled++;
		break;

	      case 'l': case 'L':
		ov_ptr->render_lighting = !ov_ptr->render_lighting;
		events_handled++;
		break;


	    }
	}

	/* Any events handled? */
	if(events_handled > 0)
	{
	    /* Redraw 3d object and regenerate buffered object
	     * image when an event is handled
	     */
	    SARMenuObjViewRebufferImage(
		display, m, ov_ptr, ov_num
	    );

	    /* Need to redraw everything, since the regeneration of
	     * the buffered object would have messed with the back
	     * buffer and thus the surrounding graphics.
	     *
	     * So simply redrawing the menu object is not enough to
	     * cover all the modified fragments.
	     */
	    GWPostRedraw(display);
	}

	return(events_handled);
}

/*
 *	Called by SARMenuObjViewLoad() to process each line found in
 *	non v3d models in the loaded model file.
 *
 *	This is to get the size of the object (the contact bounds)
 *	the name and description.
 *
 *	Inputs assumed valid.
 *
 *	Returns non-zero on error.
 */
static int SARMenuObjViewLoadLine(
	gw_display_struct *display, sar_menu_struct *m,
	sar_menu_objview_struct *ov_ptr, int ov_num,
	const char *line
)
{
	const char *arg;
	char *strptr;
	char parm[256];


	if(line == NULL)
	    return(-1);

	/* Seek past spaces */
	while(ISBLANK(*line))
	    line++;

	/* Skip comments */
	if(ISCOMMENT(*line))
	    return(0);

	/* Get parameter */
	strncpy(parm, line, 256);
	parm[256 - 1] = '\0';
	strptr = strchr(parm, ' ');
	if(strptr != NULL)
	    *strptr = '\0';
	strptr = strchr(parm, '\t');
	if(strptr != NULL)
	    *strptr = '\0';

	/* Get pointer to argument */
	arg = line;
	while(!ISBLANK(*arg) && (*arg != '\0'))
	    arg++;
	while(ISBLANK(*arg))
	    arg++;


	/* Version */
	if(!strcasecmp(parm, "version"))
	{
/* Ignore this since we're just loading the object view */
	}
	/* Name */
	else if(!strcasecmp(parm, "name"))
	{
	    free(ov_ptr->title);
	    ov_ptr->title = STRDUP(arg);
	}
	/* Description */
	else if(!strcasecmp(parm, "desc") ||
		!strcasecmp(parm, "description")
	)
	{
	    free(ov_ptr->message);
	    ov_ptr->message = STRDUP(arg);
	}
	/* Contact bounds, spherical shape */
	else if(!strcasecmp(parm, "contact_spherical"))
	{
	    /* Arguments are: <radius>
	     */
	    int sr, sc, st = 1;
	    float tan_value, value[1];
	    sr = sscanf(arg, "%f", &value[0]);
	    for(sc = sr; sc < st; sc++)
		value[sc] = 0.0f;

	    ov_ptr->obj_size_radius = value[0];

	    /* Calculate the distance of the camera to be able to
	     * capture the entire size of the object as indicated by
	     * the radius of the contact bounds plus 10% more.
	     *
	     * The field of view will be assumed to be 40 degrees.
	     */
	    tan_value = (float)tan(DEGTORAD(20.0));
	    ov_ptr->camera_distance = (float)(
		(value[0] * 1.1) /
		((tan_value > 0.0) ? tan_value : 1.0)
	    );
	}
	/* Contact bounds, cylendrical shape */
	else if(!strcasecmp(parm, "contact_cylendrical"))
	{
	    /* Arguments are: <radius> <height_min> <height_max>
	     */
	    int sr, sc, st = 3;
	    float tan_value, value[3];
	    sr = sscanf(arg, "%f %f %f",
		&value[0], &value[1], &value[2]
	    );
	    for(sc = sr; sc < st; sc++)
		value[sc] = 0.0f;

	    ov_ptr->obj_size_radius = value[0];

	    /* Calculate the distance of the camera to be able to
	     * capture the entire size of the object as indicated by
	     * the radius of the contact bounds plus 10% more.
	     *
	     * The field of view will be assumed to be 40 degrees.
	     */
	    tan_value = (float)tan(DEGTORAD(20.0f));
	    ov_ptr->camera_distance = (float)(
		(value[0] * 1.1f) /
		((tan_value > 0.0f) ? tan_value : 1.0f)
	    );
	}

	return(0);
}

/*
 *	Loads a v3d model specified by filename to the 3d object
 *	viewer object's data (deleting the previous model stored on
 *	the object's GL list as needed).
 *
 *	Returns non-zero on error.
 */
int SARMenuObjViewLoad(
	gw_display_struct *display, sar_menu_struct *m,
	sar_menu_objview_struct *ov_ptr, int ov_num,
	const char *filename
)
{
	FILE *fp;
	GLuint list;
	int i, n, status;
	const char *name;
	void **mh_item = NULL;
	int total_mh_items = 0;
	v3d_model_struct **model = NULL, *model_ptr;
	int total_models = 0;
	v3d_glinterprite_struct *gli;
	v3d_glresource_struct *glres;


	if((display == NULL) || (m == NULL) || (ov_ptr == NULL))
	    return(-1);

	if(filename == NULL)
	    return(-1);


	/* Delete previous gl list of 3d object as needed */
	list = (GLuint)ov_ptr->obj_list;
	if(list != 0)
	{
	    glDeleteLists(list, 1);
	    ov_ptr->obj_list = NULL;
	}
	/* Delete previous v3d gl resources for the gl list of
	 * the 3d object as needed
	 */
	V3DGLResourceDelete(
	    (v3d_glresource_struct *)ov_ptr->obj_glres
	);
	ov_ptr->obj_glres = NULL;

	/* Reset camera distance (but leave object and light direction
	 * as they were)
	 */
	ov_ptr->camera_distance = 20.0f;

	/* Open the V3D model file for reading */
	fp = FOpen(filename, "rb");
	if(fp == NULL)
	{
	    fprintf(
		stderr,
		"%s: Unable to open the V3D Model file for reading.\n",
		filename
	    );
	    return(-1);
	}

	/* Load data from V3D model file */
	status = V3DLoadModel(
	    NULL, fp,
	    &mh_item, &total_mh_items,
	    &model, &total_models,
	    NULL, NULL
	);

	/* Close V3D model file (data has already been loaded) */
	FClose(fp);
	fp = NULL;

	/* Error loading v3d model file? */
	if(status)
	    return(-1);

	/* Set up v3d to gl interpritation parameters */
	gli = V3DGLInterpriteNew();
	if(gli != NULL)
	{
	    gli->flags = V3D_GLFLAG_COORDINATE_AXIS |
		V3D_GLFLAG_TEXTURE_KEEP | V3D_GLFLAG_ALLOW_TRANSLATIONS |
		V3D_GLFLAG_ALLOW_ROTATIONS | V3D_GLFLAG_FLIP_WINDING |
		V3D_GLFLAG_PASS_NORMALS | V3D_GLFLAG_UNITLIZE_NORMALS |
		V3D_GLFLAG_PASS_TEXCOORDS |
		V3D_GLFLAG_TEXTURE_NAME_CASE_SENSITIVE |
		V3D_GLFLAG_MATERIAL_PROPERTIES | V3D_GLFLAG_FACES |
		V3D_GLFLAG_ENABLE_BLENDING | V3D_GLFLAG_SET_BLEND_FUNC |
		V3D_GLFLAG_HEIGHTFIELD_BASE_DIR |
#ifdef __MSW__
		V3D_GLFLAG_TEXTURE_BASE_DIR
#else
		0
#endif
		;

	    gli->coordinate_axis = V3D_GLCOORDINATE_AXIS_SCIENTIFIC;
	    gli->texture_keep = V3D_GLTEXTURE_KEEP_ALWAYS;
	    gli->allow_translations = TRUE;
	    gli->allow_rotations = TRUE;
	    gli->flip_winding = TRUE;
	    gli->pass_normals = V3D_GLPASS_NORMALS_AS_NEEDED;
	    gli->unitlize_normals = FALSE;
	    gli->pass_texcoords = V3D_GLPASS_TEXCOORDS_AS_NEEDED;
	    gli->texture_name_case_sensitive = FALSE;
	    gli->material_properties = V3D_GLPASS_MATERIAL_PROPERTIES_WITH_COLOR;
	    gli->faces = V3D_GLFACES_FRONT;
	    gli->enable_blending = V3D_GLENABLE_BLENDING_AS_NEEDED;
	    gli->set_blend_func = TRUE;
	    gli->heightfield_base_dir = NULL;
#ifdef __MSW__
	    gli->texture_base_dir = STRDUP(dname.local_data);
#else
	    gli->texture_base_dir = NULL;
#endif

	}

	/* Create a new GL resources from the loaded model data */
	glres = V3DGLResourceNewFromModelData(
	    gli, mh_item, total_mh_items, model, total_models
	);
	ov_ptr->obj_glres = (void *)glres;



	/* Create a new GL list */
	list = glGenLists(1);
	ov_ptr->obj_list = (void *)list;

	/* Begin recording GL list */
	glNewList(list, GL_COMPILE);

	/* Generate GL commands from the loaded v3d models, these GL
	 * commands will be recorded on the GL list
	 */
	for(i = 0; i < total_models; i++)
	{
	    model_ptr = model[i];
	    if(model_ptr == NULL)
		continue;

	    /* Forward any other data lines found on this model to
	     * our handler.
	     */
	    for(n = 0; n < model_ptr->total_other_data_lines; n++)
		SARMenuObjViewLoadLine(
		    display, m, ov_ptr, ov_num,
		    model_ptr->other_data_line[n]
		);

	    /* Skip all models that are not of type
	     * V3D_MODEL_TYPE_STANDARD since they would not contain any
	     * model data.
	     */
	    if(model_ptr->type != V3D_MODEL_TYPE_STANDARD)
		continue;

	    name = model_ptr->name;
	    if(name == NULL)
		continue;

	    /* Accept only models with the appropriate names */
	    if(strcmp(name, "standard") &&
	       strcmp(name, "landing_gear") &&
	       strcmp(name, "rotor") &&
	       strcmp(name, "door") &&
	       strcmp(name, "flap") &&
	       strcmp(name, "air_brake") &&
	       strcmp(name, "aileron_left") &&
	       strcmp(name, "aileron_right") &&
	       strcmp(name, "rudder_top") &&
	       strcmp(name, "rudder_bottom") &&
	       strcmp(name, "elevator") &&
	       strcmp(name, "cannard") &&
	       strcmp(name, "aileron_elevator_left") &&
	       strcmp(name, "aileron_elevator_right") &&
	       strcmp(name, "fueltank") &&
	       strcmp(name, "fuel_tank")
	    )
		continue;

	    V3DGLProcessModel(glres, model_ptr);
	}

	/* End gl list recording */
	glEndList();


	/* Delete V3D model data */
	V3DMHListDeleteAll(&mh_item, &total_mh_items);
	V3DModelListDeleteAll(&model, &total_models);

	/* Delete the GL interpritation legend structure */
	V3DGLInterpriteDelete(gli);

	return(0);
}

/*
 *	Redraws the object in the GL lists of the 3d object viewer
 *	object to its own obj_img buffer, deleting the previous
 *	obj_img buffer as needed.
 */
void SARMenuObjViewRebufferImage(
	gw_display_struct *display, sar_menu_struct *m,
	sar_menu_objview_struct *ov_ptr, int ov_num
)
{
	int width, height;
	float r, view_aspect;
	void *data;
	GLuint list;
	v3d_glresource_struct *glres;
	GLfloat vf[4];


	if((display == NULL) || (m == NULL) || (ov_ptr == NULL))
	    return;

	/* Deallocate previous 3d object buffered image as needed */
	data = ov_ptr->objimg;
	if(data != NULL)
	{
	    free(data);
	    ov_ptr->objimg = data = NULL;
	    ov_ptr->objimg_width = 0;
	    ov_ptr->objimg_height = 0;
	}

	/* Get the GL display list and the v3d GL resources of the
	 * 3d object
	 */
	list = (GLuint)ov_ptr->obj_list;
	glres = (v3d_glresource_struct *)ov_ptr->obj_glres;
	if((list == 0) || (glres == NULL))
	    return;

	/* Get geometry of rendering context */
	GWContextGet(
	    display, GWContextCurrent(display),
	    NULL, NULL,
	    NULL, NULL,
	    &width, &height
	);

	/* Calculate view aspect ratio */
	if(height > 0)
	    view_aspect = (float)width / (float)height;
	else
	    view_aspect = 800.0f / 600.0f;


	/* Switch to 3d drawing and set up GL states to begin drawing
	 * to the back buffer. When the drawing is done, read back
	 * the back buffer to get the new buffered 3d object image
	 */

	glViewport(0, 0, width, height);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(
	    40.0,		/* Field of view z in degrees */
	    view_aspect,
	    0.1,
	    10000.0
	);
	glMatrixMode(GL_MODELVIEW);

	glLoadIdentity();

	/* Set up camera */
	gluLookAt(
	    0.0, 0.0, ov_ptr->camera_distance,	/* Eye x, z, -y */
	    0.0, 0.0, -0.0,			/* Center x, z, -y */
	    0.0, 1.0, -0.0			/* Up x, z, -y */
	);

	/* Clear frame buffer */
	glClearColor(
	    ov_ptr->bg_color.r,
	    ov_ptr->bg_color.g,
	    ov_ptr->bg_color.b,
	    ov_ptr->bg_color.a
	);
	glClearDepth((GLclampd)1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	/* Reset depth states */
	StateGLEnable(&display->state_gl, GL_DEPTH_TEST);
	StateGLDepthFunc(&display->state_gl, GL_LEQUAL);

	/* Reset texture states */
	StateGLEnable(&display->state_gl, GL_TEXTURE_2D);
	StateGLTexEnvI(
	    &display->state_gl,
	    GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE
	);

	/* Reset surface states */
	StateGLEnable(&display->state_gl, GL_CULL_FACE);
	StateGLFrontFace(&display->state_gl, GL_CCW);
	StateGLShadeModel(&display->state_gl, GL_SMOOTH);

	/* Reset material values (and no color material) */
	StateGLDisable(&display->state_gl, GL_COLOR_MATERIAL);
	vf[0] = 0.2f;
	vf[1] = 0.2f;
	vf[2] = 0.2f;
	vf[3] = 1.0f;
	glMaterialfv(GL_FRONT, GL_AMBIENT, vf);
	vf[0] = 0.8f;
	vf[1] = 0.8f;
	vf[2] = 0.8f;
	vf[3] = 1.0f;
	glMaterialfv(GL_FRONT, GL_DIFFUSE, vf);
	vf[0] = 0.2f;
	vf[1] = 0.2f;
	vf[2] = 0.2f;
	vf[3] = 1.0f;
	glMaterialfv(GL_FRONT, GL_SPECULAR, vf);
	vf[0] = 0.9f * 128.0f;
	glMaterialfv(GL_FRONT, GL_SHININESS, vf);
	vf[0] = 0.0f;
	vf[1] = 0.0f;
	vf[2] = 0.0f;
	vf[3] = 1.0f;
	glMaterialfv(GL_FRONT, GL_EMISSION, vf);

	/* Set up lighting */
	vf[0] = 0.1f;
	vf[1] = 0.1f;
	vf[2] = 0.1f;
	vf[3] = 1.0f;
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, vf);

	r = 1000.0f;
	vf[0] = (GLfloat)(sin(ov_ptr->light_heading) * r);
	vf[1] = (GLfloat)(-sin(ov_ptr->light_pitch) * r);
	vf[2] = (GLfloat)(-cos(ov_ptr->light_heading) * r);
	vf[3] = 0.0f;			/* Directional light */
	glLightfv(GL_LIGHT0, GL_POSITION, vf);
	vf[0] = 0.4f;
	vf[1] = 0.4f;
	vf[2] = 0.4f;
	vf[3] = 1.0f;
	glLightfv(GL_LIGHT0, GL_AMBIENT, vf);
	vf[0] = 1.0f;
	vf[1] = 1.0f;
	vf[2] = 1.0f;
	vf[3] = 1.0f;
	glLightfv(GL_LIGHT0, GL_DIFFUSE, vf);
	vf[0] = 1.0f;
	vf[1] = 1.0f;
	vf[2] = 1.0f;
	vf[3] = 1.0f;
	glLightfv(GL_LIGHT0, GL_SPECULAR, vf);
	vf[0] = 1.0f;
	glLightfv(GL_LIGHT0, GL_CONSTANT_ATTENUATION, vf);
	vf[0] = 0.0f;
	glLightfv(GL_LIGHT0, GL_LINEAR_ATTENUATION, vf);
	vf[0] = 0.0f;
	glLightfv(GL_LIGHT0, GL_QUADRATIC_ATTENUATION, vf);

	if(ov_ptr->render_lighting)
	{
	    StateGLEnable(&display->state_gl, GL_LIGHTING);
	    StateGLEnable(&display->state_gl, GL_LIGHT0);
	}
	else
	{
	    StateGLDisable(&display->state_gl, GL_LIGHTING);
	}


	/* Enable alpha testing */
	StateGLEnable(&display->state_gl, GL_ALPHA_TEST);
	StateGLAlphaFunc(&display->state_gl, GL_GREATER, 0.5);


	/* Reset color and texture */
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	V3DTextureSelect(NULL);

	/* Begin drawing object */
	glPushMatrix();
	{
	    glRotated(
		-RADTODEG(ov_ptr->obj_heading),
		0.0, 1.0, 0.0
	    );
	    glRotated(
		-RADTODEG(ov_ptr->obj_pitch),
		1.0, 0.0, 0.0
	    );
	    glRotated(
		-RADTODEG(ov_ptr->obj_bank),
		0.0, 0.0, 1.0
	    );

	    /* Draw the object */
	    glCallList(list);
	}
	glPopMatrix();


	/* Get the drawn object image from the frame buffer */
	if((width > 0) && (height > 0))
	{
	    /* Read the frame buffer into a tempory allocated buffer and
	     * then copy it (because its flipped) to our final buffer.
	     */
	    void *gl_buf = (void *)malloc(
		width * height * 4 * sizeof(GLubyte)
	    );
	    ov_ptr->objimg = data = (void *)malloc(
		width * height * 4 * sizeof(GLubyte)
	    );
	    ov_ptr->objimg_width = width;
	    ov_ptr->objimg_height = height;

	    /* Read pixels from frame buffer to our tempory buffer */
	    if(gl_buf != NULL)
	    {
		/* Select frame buffer */
		if(display->has_double_buffer)
		    glReadBuffer(GL_BACK);
		else
		    glReadBuffer(GL_FRONT);

		/* Read pixels from frame buffer */
/*		glFlush();	Don't need to flush everything to read pixels */
		glReadPixels(
		    0, 0,
		    width, height,	/* Read same size as frame buffer */
		    GL_RGBA, GL_UNSIGNED_BYTE,
		    (GLvoid *)gl_buf
		);

		/* The read pixels stored in gl_buf are flipped, so we
		 * need to flip them back as we copy gl_buf to the
		 * objview's data.
		 */
		if(data != NULL)
		{
		    int y;
		    GLubyte *src = (GLubyte *)gl_buf, *src_ptr;
		    GLubyte *tar = (GLubyte *)data, *tar_ptr;

		    for(y = 0; y < height; y++)
		    {
			src_ptr = &src[
			    y * width * 4 * sizeof(GLubyte)
			];
			tar_ptr = &tar[
			    (height - 1 - y) * width * 4 * sizeof(GLubyte)
			];
			memcpy(
			    tar_ptr, src_ptr,
			    width * 4 * sizeof(GLubyte)
			);
		    }
		}

		/* No longer need gl_buf */
		free(gl_buf);
	    }
	}

	/* Reset GL states and transform model view matrices for
	 * 2d menu drawing due to the modifications of the gl states
	 * and matrices above
	 */
	SARMenuGLStateReset(display);
}
