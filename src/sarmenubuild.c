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
#include <limits.h>
#include <ctype.h>
#include <sys/stat.h>

#include "../include/fio.h"
#include "../include/string.h"
#include "../include/disk.h"

#include "menu.h"
#include "weather.h"
#include "sar.h"
#include "sarmenuop.h"
#include "sarmenucb.h"
#include "sarmenuoptions.h"
#include "sarmenucodes.h"
#include "config.h"


static char *SARGetFileObjectName(const char *path);

static int SARMenuBuildStandardButton(
	sar_core_struct *core_ptr, sar_menu_struct *menu,
	float x, float y,
	int w, int h,
	Boolean sensitive,
	const char *label,
	int id
);
static int SARMenuBuildOptionsButton(
	sar_core_struct *core_ptr, sar_menu_struct *menu,
	float x, float y,
	int w, int h,
	Boolean sensitive,
	const char *label,
	int id
);
static int SARMenuBuildButtonImage(
	sar_core_struct *core_ptr, sar_menu_struct *menu,
	float x, float y,
	int w, int h,
	const char *label,
	int id,
	sar_image_struct *unarmed_img,
	sar_image_struct *armed_img,
	sar_image_struct *highlighted_img
);
static int SARMenuBuildStandardSwitch(
	sar_core_struct *core_ptr, sar_menu_struct *menu,
	float x, float y,
	int w, int h,
	const char *label,
	Boolean state,
	int id,
	void (*change_cb)(void *, int, void *, Boolean)
);
static int SARMenuBuildStandardSpin(
	sar_core_struct *core_ptr, sar_menu_struct *menu,
	float x, float y,
	float w, float h,
	const char *label,
	int id,
	void (*change_cb)(void *, int, void *, char *)
);
static int SARMenuBuildStandardSlider(
	sar_core_struct *core_ptr, sar_menu_struct *menu,
	float x, float y,
	float w, float h,
	const char *label,
	int id,
	void (*change_cb)(void *, int, void *, float)
);

static char *SARMenuBuildGetFullPath(const char *name);

static void SARBuildMenusAddToList(
	sar_core_struct *core_ptr, sar_menu_struct *menu
);

static void SARBuildMenusAddDirectory(
	sar_core_struct *core_ptr,
	sar_menu_struct *menu, int list_num,
	const char *subdir
);

int SARBuildMenus(sar_core_struct *core_ptr);


#ifndef SAR_COMMENT_CHAR
# define SAR_COMMENT_CHAR       '#'
#endif

#define ATOI(s)         (((s) != NULL) ? atoi(s) : 0)
#define ATOL(s)         (((s) != NULL) ? atol(s) : 0)
#define ATOF(s)         (((s) != NULL) ? (float)atof(s) : 0.0f)
#define STRDUP(s)       (((s) != NULL) ? strdup(s) : NULL)

#define ISCOMMENT(c)    ((c) == SAR_COMMENT_CHAR)
#define ISCR(c)         (((c) == '\n') || ((c) == '\r'))
#define ISSTREMPTY(s)	(((s) != NULL) ? (*(s) == '\0') : 1)

#define MAX(a,b)        (((a) > (b)) ? (a) : (b))
#define MIN(a,b)        (((a) < (b)) ? (a) : (b))
#define CLIP(a,l,h)     (MIN(MAX((a),(l)),(h)))


/*
 *	Returns a statically allocated string containing the object's
 *	title (if any) or returns just the contents of path on failure.
 */
static char *SARGetFileObjectName(const char *path)
{
	FILE *fp;
	static char name[SAR_OBJ_NAME_MAX];


	if(ISSTREMPTY(path))
	    return("(null)");

	*name = '\0';	/* Reset name */

#define SET_NAME_AS_PATH					\
{								\
 const char *s = (path != NULL) ? strrchr(path, '/') : NULL;	\
 s = (s != NULL) ? (s + 1) : path;				\
 strncpy(name, s, SAR_OBJ_NAME_MAX);				\
 name[SAR_OBJ_NAME_MAX - 1] = '\0';				\
}

	/* Open object file for reading */
	fp = FOpen(path, "rb");
	if(fp == NULL)
	{
	    SET_NAME_AS_PATH
	    return(name);
	}

	/* Name */
	if(!FSeekToParm(fp, "name", SAR_COMMENT_CHAR, '\0'))
	{
	    char *s = FGetString(fp);
	    if(s != NULL)
	    {
		strncpy(name, s, SAR_OBJ_NAME_MAX);
		name[SAR_OBJ_NAME_MAX - 1] = '\0';
		free(s);
	    }
	}

	/* Close object file */
	FClose(fp);

	/* Did not get a name? */
	if(*name == '\0')
	    SET_NAME_AS_PATH

	return(name);
#undef SET_NAME_AS_PATH
}


/*
 *	Procedure to create a standard button.
 *
 *	Returns whatever SARMenuButtonNew() returns.
 */
static int SARMenuBuildStandardButton(
	sar_core_struct *core_ptr,
	sar_menu_struct *menu,
	float x, float y,
	int w, int h,		/* Entire size of button + label */
	Boolean sensitive,
	const char *label,
	int id
)
{
	int n, bw, w2, h2;
	sar_menu_color_struct c;
	sar_menu_label_struct *label_ptr;
	sar_menu_button_struct *button_ptr;
	const sar_option_struct *opt = &core_ptr->option;

	/* Button text color */
	c.a = 1.0f;
	c.r = 0.9f;
	c.g = 0.9f;
	c.b = 0.9f;

	/* Calculate button width (normal size of button is 42x50) */
	bw = (int)((float)h * 42.0 / 50.0);

	/* Calculate label size */
	w2 = MAX(w - bw, 0);
	h2 = h;
	/* Create label */
	n = SARMenuLabelNew(
	    menu, x, y,
	    w2, h2,
	    label,
	    &c, opt->menu_font,
	    core_ptr->menu_button_label_img
	);
	if(n > -1)
	{
	    label_ptr = SAR_MENU_LABEL(menu->object[n]);
	    label_ptr->sensitive = sensitive;
	    label_ptr->align = SAR_MENU_LABEL_ALIGN_CENTER;
	    label_ptr->offset_x = bw / 2;
	}

	/* Create button */
	n = SARMenuButtonNew(
	    menu, x, y,
	    bw, h,
	    NULL,		/* No label */
	    &c, opt->menu_font,
	    core_ptr->menu_button_unarmed_img,
	    core_ptr->menu_button_armed_img,
	    core_ptr->menu_button_highlighted_img,
	    core_ptr, id,
	    SARMenuButtonCB
	);
	if(n > -1)
	{
	    button_ptr = (sar_menu_button_struct *)menu->object[n];
	    button_ptr->sensitive = sensitive;
	    button_ptr->offset_x = (int)((bw / 2.0) - (w / 2.0));
	}

	return(n);
}

/*
 *      Procedure to create a standard button.
 *
 *      Returns whatever SARMenuButtonNew() returns.
 *
 *	This is the same as SARMenuBuildStandardButton() except that
 *	the callback is set to SARMenuOptionsButtonCB().
 */
static int SARMenuBuildOptionsButton(
	sar_core_struct *core_ptr,
	sar_menu_struct *menu,
	float x, float y,
	int w, int h,           /* Entire size of button + label */
	Boolean sensitive,
	const char *label,
	int id
)
{
	int n, bw, w2, h2;
	sar_menu_color_struct c;
	sar_menu_label_struct *label_ptr;
	sar_menu_button_struct *button_ptr;
	const sar_option_struct *opt = &core_ptr->option;

	/* Button text color */
	c.a = 1.0f;
	c.r = 0.9f;
	c.g = 0.9f;
	c.b = 0.9f;

	/* Calculate button width (normal size of button is 42x50) */
	bw = (int)((float)h * 42.0 / 50.0);

	/* Calculate label size */
	w2 = MAX(w - bw, 0);
	h2 = h;
	/* Create label */
	n = SARMenuLabelNew(
	    menu, x, y,
	    w2, h2,
	    label,
	    &c, opt->menu_font,
	    core_ptr->menu_button_label_img
	);
	if(n > -1)
	{
	    label_ptr = SAR_MENU_LABEL(menu->object[n]);
	    label_ptr->sensitive = sensitive;
	    label_ptr->align = SAR_MENU_LABEL_ALIGN_CENTER;
	    label_ptr->offset_x = bw / 2;
	}

	/* Create button */
	n = SARMenuButtonNew(
	    menu, x, y,
	    bw, h,
	    NULL,               /* No label */
	    &c, opt->menu_font,
	    core_ptr->menu_button_unarmed_img,
	    core_ptr->menu_button_armed_img,
	    core_ptr->menu_button_highlighted_img,
	    core_ptr, id,
	    SARMenuOptionsButtonCB
	);
	if(n > -1)
	{
	    button_ptr = (sar_menu_button_struct *)menu->object[n];
	    button_ptr->sensitive = sensitive;
	    button_ptr->offset_x = (int)((bw / 2.0) - (w / 2.0));
	}

	return(n);
}

/*
 *	Similar to SARMenuBuildStandardButton() but you get to specify
 *	the images.
 */
static int SARMenuBuildButtonImage(
	sar_core_struct *core_ptr, sar_menu_struct *menu,
	float x, float y,
	int w, int h,
	const char *label,
	int id,
	sar_image_struct *unarmed_img,
	sar_image_struct *armed_img,
	sar_image_struct *highlighted_img
)
{
	sar_menu_color_struct c;
	const sar_option_struct *opt = &core_ptr->option;

	/* Button text color */
	c.a = 1.0f;
	c.r = 0.9f;
	c.g = 0.9f;
	c.b = 0.9f;

	/* Create button */
	return(
	    SARMenuButtonNew(
		menu, x, y, w, h,
		label,
		&c, opt->menu_font,
		unarmed_img,
		armed_img,
		highlighted_img,
		core_ptr, id,
		SARMenuButtonCB
	    )
	);
}

/*
 *	Procedure to create a standard switch.
 *
 *      Returns whatever SARMenuSwitchNew() returns.
 */
static int SARMenuBuildStandardSwitch(
	sar_core_struct *core_ptr,
	sar_menu_struct *menu,
	float x, float y,
	int w, int h,
	const char *label,
	Boolean state,
	int id,
	void (*change_cb)(void *, int, void *, Boolean)
)
{
	sar_menu_color_struct c;
	const sar_option_struct *opt = &core_ptr->option;

	/* Switch color */
	c.a = 1.0f;
	c.r = 0.9f;
	c.g = 0.9f;
	c.b = 0.9f;

	/* Create switch */
	return(
	    SARMenuSwitchNew(
		menu, x, y, w, h,
		&c, opt->menu_font,
		label,
		core_ptr->menu_switch_bg_img,
		core_ptr->menu_switch_off_img,
		core_ptr->menu_switch_on_img,
		state,
		core_ptr, id,
		change_cb
	    )
	);
}


/*
 *      Procedure to create a standard spin.
 *
 *	Returns whatever SARMenuSpinNew() returns.
 */
static int SARMenuBuildStandardSpin(
	sar_core_struct *core_ptr,
	sar_menu_struct *menu,
	float x, float y,
	float w, float h,
	const char *label,
	int id,
	void (*change_cb)(void *, int, void *, char *)
)
{
	sar_menu_color_struct label_color, value_color, *c;
	const sar_option_struct *opt = &core_ptr->option;

	/* Spin label color */
	c = &label_color;
	c->a = 1.0f;
	c->r = 0.9f;
	c->g = 0.9f;
	c->b = 0.9f;

	/* Spin value color */
	c = &value_color;
	c->a = 1.0f;
	c->r = 0.0f;
	c->g = 0.9f;
	c->b = 0.0f;

	/* Create spin */
	return(
	    SARMenuSpinNew(
		menu, x, y, w, h,
		&label_color, &value_color,
		opt->menu_font, opt->message_font, label,
		core_ptr->menu_spin_label_img,
		core_ptr->menu_spin_value_img,
		core_ptr->menu_spin_dec_armed_img,
		core_ptr->menu_spin_dec_unarmed_img,
		core_ptr->menu_spin_inc_armed_img,
		core_ptr->menu_spin_inc_unarmed_img,
		core_ptr, id,
		change_cb
	    )
	);
}

/*
 *      Procedure to create a standard slider.
 *
 *      Returns whatever SARMenuSliderNew() returns.
 */
static int SARMenuBuildStandardSlider(
	sar_core_struct *core_ptr, sar_menu_struct *menu,
	float x, float y,
	float w, float h,
	const char *label,
	int id,
	void (*change_cb)(void *, int, void *, float)
)
{
	sar_menu_color_struct c;
	const sar_option_struct *opt = &core_ptr->option;

	/* Slider label color */
	c.a = 1.0f;
	c.r = 0.9f;
	c.g = 0.9f;
	c.b = 0.9f;

	/* Create slider */
	return(
	    SARMenuSliderNew(
		menu, x, y, w, h,
		&c, opt->menu_font, label,
		core_ptr->menu_slider_label_img,
		core_ptr->menu_slider_trough_img,
		core_ptr->menu_slider_handle_img,
		core_ptr, id,
		change_cb
	    )
	);
}


/*
 *	Prefixes local data dir to name and checks if file exists,
 *	if not then prefixes global data dir regardless if it exists
 *	or not.
 *
 *	Return is statically allocated.
 */
static char *SARMenuBuildGetFullPath(const char *name)
{
	const char *s;
	struct stat stat_buf;
	static char path[PATH_MAX + NAME_MAX];

	if(name == NULL)
	    return("");

	s = PrefixPaths(dname.local_data, name);
	if(s == NULL)
	    return("");

	strncpy(path, s, PATH_MAX + NAME_MAX);
	path[PATH_MAX + NAME_MAX - 1] = '\0';

	if(stat(path, &stat_buf))
	{
	    /* If does not exist locally, then prefix global */
	    s = PrefixPaths(dname.global_data, name);
	    if(s == NULL)
		return("");

	    strncpy(path, s, PATH_MAX + NAME_MAX);
	    path[PATH_MAX + NAME_MAX - 1] = '\0';
	}

	return(path);
}

/*
 *	Adds the specified menu to the list of menus on the core
 *	structure.
 */
static void SARBuildMenusAddToList(
	sar_core_struct *core_ptr,
	sar_menu_struct *menu
)
{
	int n;

	if(core_ptr->total_menus < 0)
	    core_ptr->total_menus = 0;

	n = core_ptr->total_menus;

	core_ptr->total_menus++;
	core_ptr->menu = (sar_menu_struct **)realloc(
	    core_ptr->menu,
	    core_ptr->total_menus * sizeof(sar_menu_struct *)
	);
	if(core_ptr->menu == NULL)
	{
	    core_ptr->total_menus = 0;
	    return;
	}

	core_ptr->menu[n] = menu;
}

/*
 *	Adds directory listing to the specified list.
 *
 *	Local data directory listing will be added first, then global.
 */
static void SARBuildMenusAddDirectory(
	sar_core_struct *core_ptr,
	sar_menu_struct *menu, int list_num,
	const char *subdir	/* Subdir to local and global data dirs */
)
{
	int strc;
	const char *s, *parent;
	char **strv;
	char tmp_path[PATH_MAX + NAME_MAX];
	struct stat stat_buf;


	if((core_ptr == NULL) || (menu == NULL) || (subdir == NULL))
	    return;

	/* Complete path to local directory */
	parent = dname.local_data;

	s = PrefixPaths(parent, subdir);
	if(s != NULL)
	{
	    strncpy(tmp_path, s, PATH_MAX + NAME_MAX);
	    tmp_path[PATH_MAX + NAME_MAX - 1] = '\0';

	    /* Get directory listing */
	    strv = GetDirEntNames(tmp_path);
	    if(strv != NULL)
	    {
		int i;
		char *name;
		const char *full_path, *item_name;
		sar_menu_list_item_data_struct *data;

		/* Get total number of strings */
		for(strc = 0; strv[strc] != NULL; strc++);

		/* Sort array */
		strv = StringQSort(strv, strc);

		/* Add each file name to the listing */
		for(i = 0; i < strc; i++)
		{
		    name = strv[i];

		    /* Skip directories */
		    if(!strcasecmp(name, ".") ||
		       !strcasecmp(name, "..")
		    )
		    {
			free(name);
			continue;
		    }

		    /* Get complete file name */
		    full_path = PrefixPaths(tmp_path, name);
		    if(stat(full_path, &stat_buf))
		    {
			free(name);
			continue;
		    }
#ifdef S_ISDIR
		    if(S_ISDIR(stat_buf.st_mode))
		    {
			free(name);
			continue;
		    }
#endif	/* S_ISDIR */

		    /* Allocate list item data */
		    data = SAR_MENU_LIST_ITEM_DATA(calloc(
			1, sizeof(sar_menu_list_item_data_struct)
		    ));
		    if(data != NULL)
		    {
			data->core_ptr = core_ptr;
			data->filename = STRDUP(full_path);
			item_name = SARGetFileObjectName(data->filename);
		    }
		    else
		    {
			item_name = name;
		    }
		    /* Add item to list */
		    SARMenuListAppendItem(
			menu, list_num,
			item_name,	/* Item name */
			data,		/* Item data */
			0		/* Flags */
		    );

		    free(name);
		}

		/* Delete string array, each string has already been
		 * deleted
		 */
		free(strv);
	    }
	}

/* No global data for __MSW__ */
#ifndef __MSW__
	/* Complete path to global data directory */
	parent = dname.global_data;

	s = PrefixPaths(parent, subdir);
	if(s != NULL)
	{
	    strncpy(tmp_path, s, PATH_MAX + NAME_MAX);
	    tmp_path[PATH_MAX + NAME_MAX - 1] = '\0';

	    /* Get directory listing */
	    strv = GetDirEntNames(tmp_path);
	    if(strv != NULL)
	    {
		int i;
		char *name;
		const char *full_path, *item_name;
		sar_menu_list_item_data_struct *data;

		/* Get total number of strings */
		for(strc = 0; strv[strc] != NULL; strc++);

		/* Sort array */
		strv = StringQSort(strv, strc);

		/* Add each listing */
		for(i = 0; i < strc; i++)
		{
		    name = strv[i];

		    /* Skip directories */
		    if(!strcasecmp(name, ".") ||
		       !strcasecmp(name, "..")
		    )
		    {
			free(name);
			continue;
		    }

		    /* Get complete file name */
		    full_path = PrefixPaths(tmp_path, name);
		    if(stat(full_path, &stat_buf))
		    {
			free(name);
			continue;
		    }
#ifdef S_ISDIR
		    if(S_ISDIR(stat_buf.st_mode))
		    {
			free(name);
			continue;
		    }
#endif	/* S_ISDIR */

		    /* Allocate list item data */
		    data = SAR_MENU_LIST_ITEM_DATA(calloc(
			1, sizeof(sar_menu_list_item_data_struct)
		    ));
		    if(data != NULL)
		    {
			data->core_ptr = core_ptr;
			data->filename = STRDUP(full_path);
			item_name = SARGetFileObjectName(data->filename);
		    }
		    else
		    {
			item_name = name;
		    }
		    /* Add item to list */
		    SARMenuListAppendItem(
			menu, list_num,
			item_name,	/* Item name */
			data,		/* Item data */
			0		/* Flags */
		    );

		    free(name);
		}

		/* Delete string array, each string has already been
		 * deleted
		 */
		free(strv);
	    }
	}
#endif	/* Not __MSW__ */
}


/*
 *	Builds menus.
 */
int SARBuildMenus(sar_core_struct *core_ptr)
{
	int	label_num, btn_num, list_num, switch_num, spin_num,
		slider_num, map_num;
	const char *img_path;
	gw_display_struct *display = core_ptr->display;
	sar_menu_struct *menu;
	sar_menu_list_struct *list;
	sar_menu_spin_struct *spin;
	sar_menu_color_struct	label_color, list_color, mesgbox_color,
				switch_color, progress_color, *c;
	GWFont *font, *value_font;
	const sar_option_struct *opt = &core_ptr->option;

/* Standard button size is 42x50 and button label is 208x50 */
int btn_width_std = 180, btn_height_std = 40;
int btn_width_main = 180, btn_height_main = 40;
int btn_width_opt = 250, btn_height_opt = 40;
int btn_width_map = 32, btn_height_map = 32;


	c = &label_color;
	c->a = 1.0f;
	c->r = 0.9f;
	c->g = 0.9f;
	c->b = 0.9f;

	c = &list_color;
	c->a = 1.0f;
	c->r = 0.0f;
	c->g = 0.9f;
	c->b = 0.0f;

	c = &mesgbox_color;
	c->a = 1.0f;
	c->r = 0.0f;
	c->g = 0.9f;
	c->b = 0.0f;

	c = &switch_color;
	c->a = 1.0f;
	c->r = 0.9f;
	c->g = 0.9f;
	c->b = 0.9f;

	c = &progress_color;
	c->a = 1.0f;
	c->r = 0.9f;
	c->g = 0.9f;
	c->b = 0.9f;

	font = opt->menu_font;
	value_font = opt->message_font;


	/* Load images for menus (these will be shared amoung the
	 * menu object/widgets
	 */
#define DO_LOAD_IMG(_i_,_p_)	{	\
 if(*(_i_) != NULL)			\
  SARImageDelete(*(_i_));		\
 *(_i_) = SARImageNewFromFile(_p_);	\
}
	/* Allocate pointers for 9 images for the list menu object
	 * background and load each image
	 */
	core_ptr->menu_list_bg_img = (sar_image_struct **)realloc(
	    core_ptr->menu_list_bg_img,
	    9 * sizeof(sar_image_struct *)
	);
	if(core_ptr->menu_list_bg_img != NULL)
	{
	    int i;
	    char tmp_path[PATH_MAX + NAME_MAX];

	    for(i = 0; i < 9; i++)
	    {
		sprintf(tmp_path, "images/list%i.tga", i);

		core_ptr->menu_list_bg_img[i] = NULL;
		DO_LOAD_IMG(
		    &core_ptr->menu_list_bg_img[i],
		    tmp_path
		);
	    }
	}

	/* Button images */
	DO_LOAD_IMG(
	    &core_ptr->menu_button_armed_img,
	    "images/button_armed.tga"
	);
	DO_LOAD_IMG(
	    &core_ptr->menu_button_unarmed_img,
	    "images/button_unarmed.tga"
	);
	DO_LOAD_IMG(
	    &core_ptr->menu_button_highlighted_img,
	    "images/button_highlighted.tga"
	);
	DO_LOAD_IMG(
	    &core_ptr->menu_button_label_img,
	    "images/button_label.tga"
	);

	/* Label images */
	DO_LOAD_IMG(
	    &core_ptr->menu_label_bg_img,
	    "images/label_bg.tga"
	);

	/* Switch images */
	DO_LOAD_IMG(
	    &core_ptr->menu_switch_bg_img,
	    "images/label_bg.tga"	/* Use label for now */
	);
	DO_LOAD_IMG(
	    &core_ptr->menu_switch_off_img,
	    "images/switch_off.tga"
	);
	DO_LOAD_IMG(
	    &core_ptr->menu_switch_on_img,
	    "images/switch_on.tga"
	);

	/* Progress bar images */
	DO_LOAD_IMG(
	    &core_ptr->menu_progress_bg_img,
	    "images/progress_bar_bg.tga"
	);
	DO_LOAD_IMG(
	    &core_ptr->menu_progress_fg_img,
	    "images/progress_bar_fg.tga"
	);

	/* Map panning buttons */
	DO_LOAD_IMG(
	    &core_ptr->menu_button_pan_up_armed_img,
	    "images/button_pan_up_armed.tga"
	);
	DO_LOAD_IMG(
	    &core_ptr->menu_button_pan_up_unarmed_img,
	    "images/button_pan_up_unarmed.tga"
	);

	DO_LOAD_IMG(
	    &core_ptr->menu_button_pan_down_armed_img,
	    "images/button_pan_down_armed.tga"
	);
	DO_LOAD_IMG(
	    &core_ptr->menu_button_pan_down_unarmed_img,
	    "images/button_pan_down_unarmed.tga"
	);

	DO_LOAD_IMG(
	    &core_ptr->menu_button_pan_left_armed_img,
	    "images/button_pan_left_armed.tga"
	);
	DO_LOAD_IMG(
	    &core_ptr->menu_button_pan_left_unarmed_img,
	    "images/button_pan_left_unarmed.tga"
	);

	DO_LOAD_IMG(
	    &core_ptr->menu_button_pan_right_armed_img,
	    "images/button_pan_right_armed.tga"
	);
	DO_LOAD_IMG(
	    &core_ptr->menu_button_pan_right_unarmed_img,
	    "images/button_pan_right_unarmed.tga"
	);

	/* Map zoom buttons */
	DO_LOAD_IMG(
	    &core_ptr->menu_button_zoom_in_armed_img,
	    "images/button_zoom_in_armed.tga"
	);
	DO_LOAD_IMG(
	    &core_ptr->menu_button_zoom_in_unarmed_img,
	    "images/button_zoom_in_unarmed.tga"
	);

	DO_LOAD_IMG(
	    &core_ptr->menu_button_zoom_out_armed_img,
	    "images/button_zoom_out_armed.tga"
	);
	DO_LOAD_IMG(
	    &core_ptr->menu_button_zoom_out_unarmed_img,
	    "images/button_zoom_out_unarmed.tga"
	);

	/* Spin images */
	DO_LOAD_IMG(
	    &core_ptr->menu_spin_label_img,
	    "images/spin_label.tga"
	);
	DO_LOAD_IMG(
	    &core_ptr->menu_spin_value_img,
	    "images/spin_value.tga"
	);
	DO_LOAD_IMG(
	    &core_ptr->menu_spin_dec_armed_img,
	    "images/spin_dec_btn_armed.tga"
	);
	DO_LOAD_IMG(
	    &core_ptr->menu_spin_dec_unarmed_img,
	    "images/spin_dec_btn_unarmed.tga"
	);
	DO_LOAD_IMG(
	    &core_ptr->menu_spin_inc_armed_img,
	    "images/spin_inc_btn_armed.tga"
	);
	DO_LOAD_IMG(
	    &core_ptr->menu_spin_inc_unarmed_img,
	    "images/spin_inc_btn_unarmed.tga"
	);

	/* Slider images */
	DO_LOAD_IMG(
	    &core_ptr->menu_slider_label_img,
	    "images/slider_label.tga"
	);
	DO_LOAD_IMG(
	    &core_ptr->menu_slider_trough_img,
	    "images/slider_trough.tga"
	);
	DO_LOAD_IMG(
	    &core_ptr->menu_slider_handle_img,
	    "images/slider_handle.tga"
	);

	/* Map icon images */
	DO_LOAD_IMG(
	    &core_ptr->menumap_helipad_img,
	    SAR_DEF_MISSION_MAPICON_HELIPAD_FILE
	);
	DO_LOAD_IMG(
	    &core_ptr->menumap_intercept_img,
	    SAR_DEF_MISSION_MAPICON_INTERCEPT_FILE
	);
	DO_LOAD_IMG(
	    &core_ptr->menumap_helicopter_img,
	    SAR_DEF_MISSION_MAPICON_HELICOPTER_FILE
	);
	DO_LOAD_IMG(
	    &core_ptr->menumap_victim_img,
	    SAR_DEF_MISSION_MAPICON_VICTIM_FILE
	);
	DO_LOAD_IMG(
	    &core_ptr->menumap_vessel_img,
	    SAR_DEF_MISSION_MAPICON_VESSEL_FILE
	);
	DO_LOAD_IMG(
	    &core_ptr->menumap_crash_img,
	    SAR_DEF_MISSION_MAPICON_CRASH_FILE
	);

#undef DO_LOAD_IMG

	/* ****************************************************** */
	/* Menu: Main */
	img_path = SARMenuBuildGetFullPath(SAR_DEF_MENU_BGIMG_MAIN_FILE);
	menu = SARMenuNew(
	    SAR_MENU_TYPE_STANDARD,
	    SAR_MENU_NAME_MAIN,
	    img_path
	);
	if(menu == NULL)
	    return(-1);
	else
	    SARBuildMenusAddToList(core_ptr, menu);
	menu->always_full_redraw = opt->menu_always_full_redraw;

	/* List of buttons */
	/* Mission */
	SARMenuBuildStandardButton(
	    core_ptr, menu, 0.18f, 0.23f, btn_width_main, btn_height_main,
	    True,
#if defined(PROG_LANGUAGE_SPANISH)
"Misión"
#elif defined(PROG_LANGUAGE_FRENCH)
"Mission"
#elif defined(PROG_LANGUAGE_GERMAN)
"Mission"
#elif defined(PROG_LANGUAGE_ITALIAN)
"Missione"
#elif defined(PROG_LANGUAGE_DUTCH)
"Missie"
#elif defined(PROG_LANGUAGE_PORTUGUESE)
"Missão"
#elif defined(PROG_LANGUAGE_NORWEGIAN)
"Misjon"
#else
"Mission"
#endif
	    ,
	    SAR_MENU_ID_GOTO_MISSION
	);

	/* Campaign */
	SARMenuBuildStandardButton(
	    core_ptr, menu, 0.18f, 0.37f, btn_width_main, btn_height_main,
	    True,
#if defined(PROG_LANGUAGE_SPANISH)
"Campaña"
#elif defined(PROG_LANGUAGE_FRENCH)
"Campagne"
#elif defined(PROG_LANGUAGE_GERMAN)
"Wahlkampf"
#elif defined(PROG_LANGUAGE_ITALIAN)
"Campagna"
#elif defined(PROG_LANGUAGE_DUTCH)
"Campagne"
#elif defined(PROG_LANGUAGE_PORTUGUESE)
"Campanha"
#elif defined(PROG_LANGUAGE_NORWEGIAN)
"Campaign"
#else
"Campaign"
#endif
	    ,
	    SAR_MENU_ID_GOTO_CAMPAIGN
	);

	/* Free Flight */
	SARMenuBuildStandardButton(
	    core_ptr, menu, 0.18f, 0.51f, btn_width_main, btn_height_main,
	    True,
#if defined(PROG_LANGUAGE_SPANISH)
"Liberta Vuelo"
#elif defined(PROG_LANGUAGE_FRENCH)
"Libérer Vol"
#elif defined(PROG_LANGUAGE_GERMAN)
"Flug"
#elif defined(PROG_LANGUAGE_ITALIAN)
"Libera Volo"
#elif defined(PROG_LANGUAGE_DUTCH)
"Vlucht"
#elif defined(PROG_LANGUAGE_PORTUGUESE)
"Libertam Vôo"
#elif defined(PROG_LANGUAGE_NORWEGIAN)
"Free Flight"
#else
"Free Flight"
#endif
	    ,
	    SAR_MENU_ID_GOTO_FREE_FLIGHT
	);

	/* Options */
	SARMenuBuildStandardButton(
	    core_ptr, menu, 0.18f, 0.65f, btn_width_main, btn_height_main,
	    True,
#if defined(PROG_LANGUAGE_SPANISH)
"Opciones"
#elif defined(PROG_LANGUAGE_FRENCH)
"Options"
#elif defined(PROG_LANGUAGE_GERMAN)
"Option"
#elif defined(PROG_LANGUAGE_ITALIAN)
"Opzioni"
#elif defined(PROG_LANGUAGE_DUTCH)
"Opties"
#elif defined(PROG_LANGUAGE_PORTUGUESE)
"Opções"
#elif defined(PROG_LANGUAGE_NORWEGIAN)
"Options"
#else
"Options"
#endif
	    ,
	    SAR_MENU_ID_GOTO_OPTIONS
	);

	/* Exit */
	SARMenuBuildStandardButton(
	    core_ptr, menu, 0.18f, 0.79f, btn_width_main, btn_height_main,
	    True,
#if defined(PROG_LANGUAGE_SPANISH)
"Salida"
#elif defined(PROG_LANGUAGE_FRENCH)
"Sortie"
#elif defined(PROG_LANGUAGE_GERMAN)
"Ausgang"
#elif defined(PROG_LANGUAGE_ITALIAN)
"L'Uscita"
#elif defined(PROG_LANGUAGE_DUTCH)
"Uitgang"
#elif defined(PROG_LANGUAGE_PORTUGUESE)
"Saída"
#elif defined(PROG_LANGUAGE_NORWEGIAN)
"Exit"
#else
"Exit"
#endif
	    ,
	    SAR_MENU_ID_GOTO_EXIT
	);


	/* ****************************************************** */
	/* Menu: Mission */
	img_path = SARMenuBuildGetFullPath(SAR_DEF_MENU_BGIMG_STANDARD_FILE);
	menu = SARMenuNew(
	    SAR_MENU_TYPE_STANDARD,
	    SAR_MENU_NAME_MISSION,
	    img_path
	);
	if(menu == NULL)
	    return(-1);
	else
	    SARBuildMenusAddToList(core_ptr, menu);
	menu->always_full_redraw = opt->menu_always_full_redraw;

	/* Missions List */
	list_num = SARMenuListNew(
	    menu, 0.50f, 0.36f, 0.95f, 0.60f,
	    &list_color,
	    value_font,
	    "Select Mission:",
	    (const sar_image_struct **)core_ptr->menu_list_bg_img,
	    core_ptr, SAR_MENU_ID_MISSION_LIST,
	    SARMenuListSelectCB,
	    SARMenuListActivateCB
	);
	SARBuildMenusAddDirectory(
	    core_ptr, menu, list_num,
	    SAR_DEF_MISSIONS_DIR
	);
	if(list_num > -1)
	{
	    /* Restore previously selected mission */
	    SARMenuListSelect(
		display, menu, list_num,
		opt->last_selected_mission,
		True,		/* Scroll to */
		False		/* No redraw */
	    );
	}

	/* Change Pilot Button */
	SARMenuBuildStandardButton(
	    core_ptr, menu, 0.25f, 0.74f, btn_width_std, btn_height_std,
	    True, "Pilots...", SAR_MENU_ID_GOTO_MISSION_PLAYER
	);

	/* Last Mission Results Button */
	SARMenuBuildStandardButton(
	    core_ptr, menu, 0.75f, 0.74f, btn_width_std, btn_height_std,
	    True, "Last Log...", SAR_MENU_ID_GOTO_MISSION_LOG_MAP
	);

	/* Back Button */
	SARMenuBuildStandardButton(
	    core_ptr, menu, 0.25f, 0.92f, btn_width_std, btn_height_std,
	    True,
#if defined(PROG_LANGUAGE_SPANISH)
"Espalda"
#elif defined(PROG_LANGUAGE_FRENCH)
"Dos"
#elif defined(PROG_LANGUAGE_GERMAN)
"Zurück"
#elif defined(PROG_LANGUAGE_ITALIAN)
"Dorso"
#elif defined(PROG_LANGUAGE_DUTCH)
"Rug"
#elif defined(PROG_LANGUAGE_PORTUGUESE)
"Costas"
#elif defined(PROG_LANGUAGE_NORWEGIAN)
"Frem"
#else
"Back"
#endif
	    ,
	    SAR_MENU_ID_GOTO_MAIN
	);

	/* Mission Briefing Button */
	SARMenuBuildStandardButton(
	    core_ptr, menu, 0.75f, 0.92f, btn_width_std, btn_height_std,
	    True, "Briefing", SAR_MENU_ID_GOTO_MISSION_BRIEF
	);


	/* ****************************************************** */
	/* Menu: Mission->Briefing */
	img_path = SARMenuBuildGetFullPath(SAR_DEF_MENU_BGIMG_STANDARD_FILE);
	menu = SARMenuNew(
	    SAR_MENU_TYPE_STANDARD,
	    SAR_MENU_NAME_MISSION_BRIEF,
	    img_path
	);
	if(menu == NULL)
	    return(-1);
	else
	    SARBuildMenusAddToList(core_ptr, menu);
	menu->always_full_redraw = opt->menu_always_full_redraw;

	/* Mission Briefing Message Box */
	SARMenuMessageBoxNew(
	    menu, 0.50f, 0.36f, 0.95f, 0.60f,
	    &mesgbox_color,
	    value_font,
	    (const sar_image_struct **)core_ptr->menu_list_bg_img,
	    core_ptr, SAR_MENU_ID_MISSION_BRIEF_MESG,
	    NULL	/* No initial message */
	);

	/* Back Button */
	SARMenuBuildStandardButton(
	    core_ptr, menu, 0.25f, 0.92f, btn_width_std, btn_height_std,
	    True,
#if defined(PROG_LANGUAGE_SPANISH)
"Espalda"
#elif defined(PROG_LANGUAGE_FRENCH)
"Dos"
#elif defined(PROG_LANGUAGE_GERMAN)
"Zurück"
#elif defined(PROG_LANGUAGE_ITALIAN)
"Dorso"
#elif defined(PROG_LANGUAGE_DUTCH)
"Rug"
#elif defined(PROG_LANGUAGE_PORTUGUESE)
"Costas"
#elif defined(PROG_LANGUAGE_NORWEGIAN)
"Frem"
#else
"Back"
#endif
	    , SAR_MENU_ID_GOTO_MISSION
	);

	/* Mission Map Button */
	SARMenuBuildStandardButton(
	    core_ptr, menu, 0.25f, 0.74f, btn_width_std, btn_height_std,
	    True, "Map...", SAR_MENU_ID_GOTO_MISSION_MAP
	);

	/* Begin Mission Button */
	SARMenuBuildStandardButton(
	    core_ptr, menu, 0.75f, 0.92f, btn_width_std, btn_height_std,
	    True, "Begin", SAR_MENU_ID_GOTO_MISSION_BEGIN
	);


	/* ****************************************************** */
	/* Menu: Mission->Briefing->Map */
	img_path = SARMenuBuildGetFullPath(SAR_DEF_MENU_BGIMG_STANDARD_FILE);
	menu = SARMenuNew(
	    SAR_MENU_TYPE_STANDARD,
	    SAR_MENU_NAME_MISSION_MAP,
	    img_path
	);
	if(menu == NULL)
	    return(-1);
	else
	    SARBuildMenusAddToList(core_ptr, menu);
	menu->always_full_redraw = opt->menu_always_full_redraw;

	/* Mission Map */
	map_num = SARMenuMapNew(
	    menu, 0.60f, 0.41f, 0.74f, 0.70f,
	    value_font, &mesgbox_color,
	    (const sar_image_struct **)core_ptr->menu_list_bg_img,
	    0.0f, 0.0f,
	    (float)SAR_MAP_DEF_MTOP_COEFF,
	    SAR_MENU_MAP_SHOW_MARKING_ALL
	);

	/* Map Pan Buttons */
	SARMenuBuildButtonImage(
	    core_ptr, menu, 0.12f, 0.34f, btn_width_map, btn_height_map,
	    NULL,
	    SAR_MENU_ID_MISSION_MAP_UP,
	    core_ptr->menu_button_pan_up_unarmed_img,
	    core_ptr->menu_button_pan_up_armed_img,
	    core_ptr->menu_button_pan_up_unarmed_img
	);
	SARMenuBuildButtonImage(
	    core_ptr, menu, 0.12f, 0.46f, btn_width_map, btn_height_map,
	    NULL,
	    SAR_MENU_ID_MISSION_MAP_DOWN,
	    core_ptr->menu_button_pan_down_unarmed_img,
	    core_ptr->menu_button_pan_down_armed_img,
	    core_ptr->menu_button_pan_down_unarmed_img
	);
	SARMenuBuildButtonImage(
	    core_ptr, menu, 0.06f, 0.40f, btn_width_map, btn_height_map,
	    NULL,
	    SAR_MENU_ID_MISSION_MAP_LEFT,
	    core_ptr->menu_button_pan_left_unarmed_img,
	    core_ptr->menu_button_pan_left_armed_img,
	    core_ptr->menu_button_pan_left_unarmed_img
	);
	SARMenuBuildButtonImage(
	    core_ptr, menu, 0.18f, 0.40f, btn_width_map, btn_height_map,
	    NULL,
	    SAR_MENU_ID_MISSION_MAP_RIGHT,
	    core_ptr->menu_button_pan_right_unarmed_img,
	    core_ptr->menu_button_pan_right_armed_img,
	    core_ptr->menu_button_pan_right_unarmed_img
	);
	/* Map Zoom Buttons */
	SARMenuBuildButtonImage(
	    core_ptr, menu, 0.07f, 0.18f, btn_width_map, btn_height_map,
	    NULL,
	    SAR_MENU_ID_MISSION_MAP_ZOOM_OUT,
	    core_ptr->menu_button_zoom_out_unarmed_img,
	    core_ptr->menu_button_zoom_out_armed_img,
	    core_ptr->menu_button_zoom_out_unarmed_img
	);
	SARMenuBuildButtonImage(
	    core_ptr, menu, 0.17f, 0.18f, btn_width_map, btn_height_map,
	    NULL,
	    SAR_MENU_ID_MISSION_MAP_ZOOM_IN,
	    core_ptr->menu_button_zoom_in_unarmed_img,
	    core_ptr->menu_button_zoom_in_armed_img,
	    core_ptr->menu_button_zoom_in_unarmed_img
	);

	/* Back Button */
	SARMenuBuildStandardButton(
	    core_ptr, menu, 0.25f, 0.92f, btn_width_std, btn_height_std,
	    True,
#if defined(PROG_LANGUAGE_SPANISH)
"Espalda"
#elif defined(PROG_LANGUAGE_FRENCH)
"Dos"
#elif defined(PROG_LANGUAGE_GERMAN)
"Zurück"
#elif defined(PROG_LANGUAGE_ITALIAN)
"Dorso"
#elif defined(PROG_LANGUAGE_DUTCH)
"Rug"
#elif defined(PROG_LANGUAGE_PORTUGUESE)
"Costas"
#elif defined(PROG_LANGUAGE_NORWEGIAN)
"Frem"
#else
"Back"
#endif
	    , SAR_MENU_ID_GOTO_MISSION_BRIEF
	);


	/* ****************************************************** */
	/* Menu: Mission->Log Map */
	img_path = SARMenuBuildGetFullPath(SAR_DEF_MENU_BGIMG_STANDARD_FILE);
	menu = SARMenuNew(
	    SAR_MENU_TYPE_STANDARD,
	    SAR_MENU_NAME_MISSION_LOG_MAP,
	    img_path
	);
	if(menu == NULL)
	    return(-1);
	else
	    SARBuildMenusAddToList(core_ptr, menu);
	menu->always_full_redraw = opt->menu_always_full_redraw;

	/* Log Map */
	map_num = SARMenuMapNew(
	    menu, 0.60f, 0.41f, 0.74f, 0.70f,
	    value_font, &mesgbox_color,
	    (const sar_image_struct **)core_ptr->menu_list_bg_img,
	    0.0f, 0.0f,
	    (float)SAR_MAP_DEF_MTOP_COEFF,
	    SAR_MENU_MAP_SHOW_MARKING_SELECTED
	);

	/* Map Pan Buttons */
	SARMenuBuildButtonImage(
	    core_ptr, menu, 0.12f, 0.34f, btn_width_map, btn_height_map,
	    NULL,
	    SAR_MENU_ID_MISSION_LOG_MAP_UP,
	    core_ptr->menu_button_pan_up_unarmed_img,
	    core_ptr->menu_button_pan_up_armed_img,
	    core_ptr->menu_button_pan_up_unarmed_img
	);
	SARMenuBuildButtonImage(
	    core_ptr, menu, 0.12f, 0.46f, btn_width_map, btn_height_map,
	    NULL,
	    SAR_MENU_ID_MISSION_LOG_MAP_DOWN,
	    core_ptr->menu_button_pan_down_unarmed_img,
	    core_ptr->menu_button_pan_down_armed_img,
	    core_ptr->menu_button_pan_down_unarmed_img
	);
	SARMenuBuildButtonImage(
	    core_ptr, menu, 0.06f, 0.40f, btn_width_map, btn_height_map,
	    NULL,
	    SAR_MENU_ID_MISSION_LOG_MAP_LEFT,
	    core_ptr->menu_button_pan_left_unarmed_img,
	    core_ptr->menu_button_pan_left_armed_img,
	    core_ptr->menu_button_pan_left_unarmed_img
	);
	SARMenuBuildButtonImage(
	    core_ptr, menu, 0.18f, 0.40f, btn_width_map, btn_height_map,
	    NULL,
	    SAR_MENU_ID_MISSION_LOG_MAP_RIGHT,
	    core_ptr->menu_button_pan_right_unarmed_img,
	    core_ptr->menu_button_pan_right_armed_img,
	    core_ptr->menu_button_pan_right_unarmed_img
	);
	/* Map Zoom Buttons */
	SARMenuBuildButtonImage(
	    core_ptr, menu, 0.07f, 0.18f, btn_width_map, btn_height_map,
	    NULL,
	    SAR_MENU_ID_MISSION_LOG_MAP_ZOOM_OUT,
	    core_ptr->menu_button_zoom_out_unarmed_img,
	    core_ptr->menu_button_zoom_out_armed_img,
	    core_ptr->menu_button_zoom_out_unarmed_img
	);
	SARMenuBuildButtonImage(
	    core_ptr, menu, 0.17f, 0.18f, btn_width_map, btn_height_map,
	    NULL,
	    SAR_MENU_ID_MISSION_LOG_MAP_ZOOM_IN,
	    core_ptr->menu_button_zoom_in_unarmed_img,
	    core_ptr->menu_button_zoom_in_armed_img,
	    core_ptr->menu_button_zoom_in_unarmed_img
	);

	/* Log Event Previous & Next Buttons */
	SARMenuBuildStandardButton(
	    core_ptr, menu, 0.12f, 0.57f, 100, btn_height_std,
	    True, "Next", SAR_MENU_ID_MISSION_LOG_MAP_EVENT_NEXT
	);
	SARMenuBuildStandardButton(
	    core_ptr, menu, 0.12f, 0.67f, 100, btn_height_std,
	    True, "Prev", SAR_MENU_ID_MISSION_LOG_MAP_EVENT_PREV
	);

	/* Back Button */
	SARMenuBuildStandardButton(
	    core_ptr, menu, 0.25f, 0.92f, btn_width_std, btn_height_std,
	    True, "Continue", SAR_MENU_ID_GOTO_MISSION
	);


	/* ****************************************************** */
	/* Menu: Mission->Player */
	img_path = SARMenuBuildGetFullPath(SAR_DEF_MENU_BGIMG_STANDARD_FILE);
	menu = SARMenuNew(
	    SAR_MENU_TYPE_STANDARD,
	    SAR_MENU_NAME_PLAYER,
	    img_path
	);
	if(menu == NULL)
	    return(-1);
	else
	    SARBuildMenusAddToList(core_ptr, menu);
	menu->always_full_redraw = opt->menu_always_full_redraw;

	/* Players List */
	list_num = SARMenuListNew(
	    menu, 0.34f, 0.23f, 0.60f, 0.35f,
	    &list_color,
	    value_font,
	    "Pilots:",
	    (const sar_image_struct **)core_ptr->menu_list_bg_img,
	    core_ptr, SAR_MENU_ID_PLAYER_LIST,
	    SARMenuListSelectCB,
	    SARMenuListActivateCB
	);

	/* Player Stats Message Box */
	SARMenuMessageBoxNew(
	    menu, 0.34f, 0.60f, 0.60f, 0.30f,
	    &mesgbox_color,
	    value_font,
	    (const sar_image_struct **)core_ptr->menu_list_bg_img,
	    core_ptr, SAR_MENU_ID_PLAYER_STATS_MESG,
	    NULL        /* No initial message */
	);


	/* Add Button */
	SARMenuBuildStandardButton(
	    core_ptr, menu, 0.82f, 0.20f, btn_width_std, btn_height_std,
	    True,
#if defined(PROG_LANGUAGE_SPANISH)
"Add"
#elif defined(PROG_LANGUAGE_FRENCH)
"Add"
#elif defined(PROG_LANGUAGE_GERMAN)
"Add"
#elif defined(PROG_LANGUAGE_ITALIAN)
"Add"
#elif defined(PROG_LANGUAGE_DUTCH)
"Add"
#elif defined(PROG_LANGUAGE_PORTUGUESE)
"Add"
#elif defined(PROG_LANGUAGE_NORWEGIAN)
"Add"
#else
"Add"
#endif
	    , SAR_MENU_ID_PLAYER_ADD
	);

	/* Edit Button */
	SARMenuBuildStandardButton(
	    core_ptr, menu, 0.82f, 0.42f, btn_width_std, btn_height_std,
	    True,
#if defined(PROG_LANGUAGE_SPANISH)
"Edit"
#elif defined(PROG_LANGUAGE_FRENCH)
"Edit"
#elif defined(PROG_LANGUAGE_GERMAN)
"Edit"
#elif defined(PROG_LANGUAGE_ITALIAN)
"Edit"
#elif defined(PROG_LANGUAGE_DUTCH)
"Edit"
#elif defined(PROG_LANGUAGE_PORTUGUESE)
"Edit"
#elif defined(PROG_LANGUAGE_NORWEGIAN)
"Edit"
#else
"Edit"
#endif
	    , SAR_MENU_ID_PLAYER_EDIT
	);

	/* Remove Button */
	SARMenuBuildStandardButton(
	    core_ptr, menu, 0.82f, 0.64f, btn_width_std, btn_height_std,
	    True,
#if defined(PROG_LANGUAGE_SPANISH)
"Remove"
#elif defined(PROG_LANGUAGE_FRENCH)
"Remove"
#elif defined(PROG_LANGUAGE_GERMAN)
"Remove"
#elif defined(PROG_LANGUAGE_ITALIAN)
"Remove"
#elif defined(PROG_LANGUAGE_DUTCH)
"Remove"
#elif defined(PROG_LANGUAGE_PORTUGUESE)
"Remove"
#elif defined(PROG_LANGUAGE_NORWEGIAN)
"Remove"
#else
"Remove"
#endif
	    , SAR_MENU_ID_PLAYER_REMOVE
	);


	/* Back Button */
	SARMenuBuildStandardButton(
	    core_ptr, menu, 0.25f, 0.92f, btn_width_std, btn_height_std,
	    True,
#if defined(PROG_LANGUAGE_SPANISH)
"Espalda"
#elif defined(PROG_LANGUAGE_FRENCH)
"Dos"
#elif defined(PROG_LANGUAGE_GERMAN)
"Zurück"
#elif defined(PROG_LANGUAGE_ITALIAN)
"Dorso"
#elif defined(PROG_LANGUAGE_DUTCH)
"Rug"
#elif defined(PROG_LANGUAGE_PORTUGUESE)
"Costas"
#elif defined(PROG_LANGUAGE_NORWEGIAN)
"Frem"
#else
"Back"
#endif
	    , SAR_MENU_ID_GOTO_MISSION
	);


	/* Restore previously selected player */
	list = SAR_MENU_LIST(SARMenuGetObjectByID(
	    menu, SAR_MENU_ID_PLAYER_LIST, &list_num
	));
	if(list != NULL)
	{
	    int i;
	    const sar_player_stat_struct *pstat;

	    /* Add player stats to list */
	    for(i = 0; i < core_ptr->total_player_stats; i++)
	    {
		pstat = core_ptr->player_stat[i];
		if(pstat == NULL)
		    continue;

		SARMenuListAppendItem(
		    menu, list_num,
		    pstat->name,	/* Name */
		    NULL,               /* Data */
		    0                   /* Flags */
		);
	    }

	    /* Restore previously selected player */
	    SARMenuListSelect(
		display, menu, list_num,
		opt->last_selected_player,
		True,			/* Scroll to */
		False			/* No redraw */
	    );
	}


	/* ****************************************************** */
	/* Menu: Free Flight */
	img_path = SARMenuBuildGetFullPath(SAR_DEF_MENU_BGIMG_STANDARD_FILE);
	menu = SARMenuNew(
	    SAR_MENU_TYPE_STANDARD,
	    SAR_MENU_NAME_FREE_FLIGHT,
	    img_path
	);
	if(menu == NULL)
	    return(-1);
	else
	    SARBuildMenusAddToList(core_ptr, menu);
	menu->always_full_redraw = opt->menu_always_full_redraw;

	/* Scenery List */
	list_num = SARMenuListNew(
	    menu, 0.50f, 0.23f, 0.95f, 0.35f,
	    &list_color,
	    value_font,
	    "Select Theater:",
	    (const sar_image_struct **)core_ptr->menu_list_bg_img,
	    core_ptr, SAR_MENU_ID_FREE_FLIGHT_SCENERY_LIST,
	    SARMenuListSelectCB,
	    SARMenuListActivateCB
	);
	SARBuildMenusAddDirectory(
	    core_ptr, menu, list_num,
	    SAR_DEF_SCENERY_DIR
	);

	/* Locations List */
	list_num = SARMenuListNew(
	    menu, 0.50f, 0.60f, 0.95f, 0.30f,
	    &list_color,
	    value_font,
	    "Select Location:",
	    (const sar_image_struct **)core_ptr->menu_list_bg_img,
	    core_ptr, SAR_MENU_ID_FREE_FLIGHT_LOCATIONS_LIST,
	    SARMenuListSelectCB,
	    SARMenuListActivateCB
	);

	/* Back Button */
	SARMenuBuildStandardButton(
	    core_ptr, menu, 0.25f, 0.92f, btn_width_std, btn_height_std,
	    True,
#if defined(PROG_LANGUAGE_SPANISH)
"Espalda"
#elif defined(PROG_LANGUAGE_FRENCH)
"Dos"
#elif defined(PROG_LANGUAGE_GERMAN)
"Zurück"
#elif defined(PROG_LANGUAGE_ITALIAN)
"Dorso"
#elif defined(PROG_LANGUAGE_DUTCH)
"Rug"
#elif defined(PROG_LANGUAGE_PORTUGUESE)
"Costas"
#elif defined(PROG_LANGUAGE_NORWEGIAN)
"Frem"
#else
"Back"
#endif
	    , SAR_MENU_ID_GOTO_MAIN
	);

	/* Aircraft Button */
	SARMenuBuildStandardButton(
	    core_ptr, menu, 0.75f, 0.92f, btn_width_std, btn_height_std,
	    True, "Aircraft", SAR_MENU_ID_GOTO_FREE_FLIGHT_AIRCRAFT
	);

	/* Restore the previously selected scene on Scenery List and
	 * update the Locations List
	 */
	list = SAR_MENU_LIST(SARMenuGetObjectByID(
	    menu, SAR_MENU_ID_FREE_FLIGHT_SCENERY_LIST, &list_num
	));
	if(list != NULL)
	{
	    SARMenuListSelect(
		display, menu, list_num,
		opt->last_selected_ffscene,
		True,		/* Scroll to */
		False		/* No redraw */
	    );
	}


	/* ****************************************************** */
	/* Menu: Free Flight->Aircraft */
	img_path = SARMenuBuildGetFullPath(SAR_DEF_MENU_BGIMG_STANDARD_FILE);
	menu = SARMenuNew(
	    SAR_MENU_TYPE_STANDARD,
	    SAR_MENU_NAME_FREE_FLIGHT_AIRCRAFT,
	    img_path
	);
	if(menu == NULL)
	    return(-1);
	else
	    SARBuildMenusAddToList(core_ptr, menu);
	menu->always_full_redraw = opt->menu_always_full_redraw;

	/* Aircrafts List */
	list_num = SARMenuListNew(
	    menu, 0.50f, 0.36f, 0.95f, 0.60f,
	    &list_color,
	    value_font,
	    "Select Aircraft:",
	    (const sar_image_struct **)core_ptr->menu_list_bg_img,
	    core_ptr, SAR_MENU_ID_FREE_FLIGHT_AIRCRAFTS_LIST,
	    SARMenuListSelectCB,
	    SARMenuListActivateCB
	);
	SARBuildMenusAddDirectory(
	    core_ptr, menu, list_num,
	    SAR_DEF_AIRCRAFTS_DIR
	);
	if(list_num > -1)
	{
	    /* Restore previously selected aircraft */
	    SARMenuListSelect(
		display, menu, list_num,
		opt->last_selected_ffaircraft,
		True,		/* Scroll to */
		False		/* No redraw */
	    );
	}

	/* Aircraft Details Button */
	SARMenuBuildStandardButton(
	    core_ptr, menu, 0.25f, 0.74f, btn_width_std, btn_height_std,
	    True, "Details...", SAR_MENU_ID_GOTO_FREE_FLIGHT_AIRCRAFT_INFO
	);

	/* Weather Button */
	SARMenuBuildStandardButton(
	    core_ptr, menu, 0.75f, 0.74f, btn_width_std, btn_height_std,
	    True, "Weather...", SAR_MENU_ID_GOTO_FREE_FLIGHT_WEATHER
	);

	/* Back Button */
	SARMenuBuildStandardButton(
	    core_ptr, menu, 0.25f, 0.92f, btn_width_std, btn_height_std,
	    True,
#if defined(PROG_LANGUAGE_SPANISH)
"Espalda"
#elif defined(PROG_LANGUAGE_FRENCH)
"Dos"
#elif defined(PROG_LANGUAGE_GERMAN)
"Zurück"
#elif defined(PROG_LANGUAGE_ITALIAN)
"Dorso"
#elif defined(PROG_LANGUAGE_DUTCH)
"Rug"
#elif defined(PROG_LANGUAGE_PORTUGUESE)
"Costas"
#elif defined(PROG_LANGUAGE_NORWEGIAN)
"Frem"
#else
"Back"
#endif
	    , SAR_MENU_ID_GOTO_FREE_FLIGHT
	);

	/* Begin Free Flight Button */
	SARMenuBuildStandardButton(
	    core_ptr, menu, 0.75f, 0.92f, btn_width_std, btn_height_std,
	    True, "Begin", SAR_MENU_ID_GOTO_FREE_FLIGHT_BEGIN
	);


	/* ****************************************************** */
	/* Menu: Free Flight->Aircraft->Weather */
	img_path = SARMenuBuildGetFullPath(SAR_DEF_MENU_BGIMG_STANDARD_FILE);
	menu = SARMenuNew(
	    SAR_MENU_TYPE_STANDARD,
	    SAR_MENU_NAME_FREE_FLIGHT_WEATHER,
	    img_path
	);
	if(menu == NULL)
	    return(-1);
	else
	    SARBuildMenusAddToList(core_ptr, menu);
	menu->always_full_redraw = opt->menu_always_full_redraw;

	/* Weather Presets Spin */
	spin_num = SARMenuBuildStandardSpin(
	    core_ptr, menu, 0.50f, 0.18f, 0.9f, 0.0f,
	    "Condition",
	    SAR_MENU_ID_MENU_FREE_FLIGHT_WEATHER_CONDITION,
	    SARMenuSpinCB
	);
	if(spin_num > -1)
	{
	    int i;
	    sar_weather_data_struct *weather_data = core_ptr->weather_data;
	    sar_weather_data_entry_struct *wdp_ptr;

	    spin = SAR_MENU_SPIN(menu->object[spin_num]);
	    spin->allow_warp = False;
	    if(weather_data != NULL)
	    {
		for(i = 0; i < weather_data->total_presets; i++)
		{
		    wdp_ptr = weather_data->preset[i];
		    if(wdp_ptr == NULL)
			continue;

		    if(wdp_ptr->name == NULL)
			continue;

		    SARMenuSpinAddValue(
			menu, spin_num,
			wdp_ptr->name
		    );
		}
	    }

	    /* Restore previously selected weather item */
	    spin->cur_value = opt->last_selected_ffweather;
	}

	/* Use System Time Switch */
	SARMenuBuildStandardSwitch(
	    core_ptr, menu,
	    (float)((1.0 / 8.0) + (0.0 * 1.0 / 4.0)), 0.4f,
	    0, 0,
	    "System Time",
	    opt->system_time_free_flight,
	    SAR_MENU_ID_MENU_FREE_FLIGHT_SYSTEM_TIME,
	    SARMenuSwitchCB
	);

	/* Back Button */
	SARMenuBuildStandardButton(
	    core_ptr, menu, 0.25f, 0.92f, btn_width_std, btn_height_std,
	    True,
#if defined(PROG_LANGUAGE_SPANISH)
"Espalda"
#elif defined(PROG_LANGUAGE_FRENCH)
"Dos"
#elif defined(PROG_LANGUAGE_GERMAN)
"Zurück"
#elif defined(PROG_LANGUAGE_ITALIAN)
"Dorso"
#elif defined(PROG_LANGUAGE_DUTCH)
"Rug"
#elif defined(PROG_LANGUAGE_PORTUGUESE)
"Costas"
#elif defined(PROG_LANGUAGE_NORWEGIAN)
"Frem"
#else
"Back"
#endif
	    , SAR_MENU_ID_GOTO_FREE_FLIGHT_AIRCRAFT
	);

	/* Begin Free Flight Button */
	SARMenuBuildStandardButton(
	    core_ptr, menu, 0.75f, 0.92f, btn_width_std, btn_height_std,
	    True, "Begin", SAR_MENU_ID_GOTO_FREE_FLIGHT_BEGIN
	);


	/* ****************************************************** */
	/* Menu: Free Flight->Aircraft->Aircraft Info */
	img_path = SARMenuBuildGetFullPath(SAR_DEF_MENU_BGIMG_STANDARD_FILE);
	menu = SARMenuNew(
	    SAR_MENU_TYPE_STANDARD,
	    SAR_MENU_NAME_FREE_FLIGHT_AIRCRAFT_INFO,
	    img_path
	);
	if(menu == NULL)
	    return(-1);
	else
	    SARBuildMenusAddToList(core_ptr, menu);
	menu->always_full_redraw = opt->menu_always_full_redraw;

	/* Aircraft Details 3D ObjView */
	if(True)
	{
	    sar_menu_color_struct color;

	    color.a = 1.0f;
	    color.r = 0.0f;
	    color.g = 0.9f;
	    color.b = 0.0f;

	    SARMenuObjViewNew(
		menu,
		0.50f, 0.41f, 0.95f, 0.70f,
		opt->message_font,
		&color,
		(const sar_image_struct **)core_ptr->menu_list_bg_img,
		"Untitled"
	    );
	}

	/* Back Button */
	SARMenuBuildStandardButton(
	    core_ptr, menu, 0.25f, 0.92f, btn_width_std, btn_height_std,
	    True,
#if defined(PROG_LANGUAGE_SPANISH)
"Espalda"
#elif defined(PROG_LANGUAGE_FRENCH)
"Dos"
#elif defined(PROG_LANGUAGE_GERMAN)
"Zurück"
#elif defined(PROG_LANGUAGE_ITALIAN)
"Dorso"
#elif defined(PROG_LANGUAGE_DUTCH)
"Rug"
#elif defined(PROG_LANGUAGE_PORTUGUESE)
"Costas"
#elif defined(PROG_LANGUAGE_NORWEGIAN)
"Frem"
#else
"Back"
#endif
	    , SAR_MENU_ID_GOTO_FREE_FLIGHT_AIRCRAFT
	);

	/* Begin Free Flight Button */
	SARMenuBuildStandardButton(
	    core_ptr, menu, 0.75f, 0.92f, btn_width_std, btn_height_std,
	    True, "Begin", SAR_MENU_ID_GOTO_FREE_FLIGHT_BEGIN
	);


	/* ****************************************************** */
	/* Menu: Options */
	img_path = SARMenuBuildGetFullPath(SAR_DEF_MENU_BGIMG_STANDARD_FILE);
	menu = SARMenuNew(
	    SAR_MENU_TYPE_STANDARD,
	    SAR_MENU_NAME_OPTIONS,
	    img_path
	);
	if(menu == NULL)
	    return(-1);
	else
	    SARBuildMenusAddToList(core_ptr, menu);
	menu->always_full_redraw = opt->menu_always_full_redraw;

	/* Simulation Options Button */
	SARMenuBuildStandardButton(
	    core_ptr, menu, 0.28f, 0.13f, btn_width_opt, btn_height_opt,
	    True,
#if defined(PROG_LANGUAGE_SPANISH)
"Simulación..."
elif defined(PROG_LANGUAGE_FRENCH)
"Simulation..."
#elif defined(PROG_LANGUAGE_GERMAN)
"Simulation..."
#elif defined(PROG_LANGUAGE_ITALIAN)
"Simulazione..."
#elif defined(PROG_LANGUAGE_DUTCH)
"Simulatie..."
#elif defined(PROG_LANGUAGE_PORTUGUESE)
"Simulation..."
#elif defined(PROG_LANGUAGE_NORWEGIAN)
"Simulering..."
#else
"Simulation..."
#endif
	    , SAR_MENU_ID_GOTO_OPTIONS_SIMULATION
	);
	/* Controller Options Button */
	SARMenuBuildStandardButton(
	    core_ptr, menu, 0.28f, 0.32f, btn_width_opt, btn_height_opt,
	    True,
#if defined(PROG_LANGUAGE_SPANISH)
"Director..."
#elif defined(PROG_LANGUAGE_FRENCH)
"Contrôleur..."
#elif defined(PROG_LANGUAGE_GERMAN)
"Steuergerät..."
#elif defined(PROG_LANGUAGE_ITALIAN)
"Controllore..."
#elif defined(PROG_LANGUAGE_DUTCH)
"Controleur..."
#elif defined(PROG_LANGUAGE_PORTUGUESE)
"Censor..."
#elif defined(PROG_LANGUAGE_NORWEGIAN)
"Regulator..."
#else
"Controller..."
#endif
	    , SAR_MENU_ID_GOTO_OPTIONS_CONTROLLER
	);
	/* Graphics Options Button */
	SARMenuBuildStandardButton(
	    core_ptr, menu, 0.28f, 0.51f, btn_width_opt, btn_height_opt,
	    True,
#if defined(PROG_LANGUAGE_SPANISH)
"Gráfica..."
#elif defined(PROG_LANGUAGE_FRENCH)
"Graphiques..."
#elif defined(PROG_LANGUAGE_GERMAN)
"Grafik..."
#elif defined(PROG_LANGUAGE_ITALIAN)
"Grafico..."
#elif defined(PROG_LANGUAGE_DUTCH)
"Grafiek..."
#elif defined(PROG_LANGUAGE_PORTUGUESE)
"Gráfico..."
#elif defined(PROG_LANGUAGE_NORWEGIAN)
"Grafikk..."
#else
"Graphics..."
#endif
	    , SAR_MENU_ID_GOTO_OPTIONS_GRAPHICS
	);
	/* Sound Options Button */
	SARMenuBuildStandardButton(
	    core_ptr, menu, 0.28f, 0.70f, btn_width_opt, btn_height_opt,
	    True,
#if defined(PROG_LANGUAGE_SPANISH)
"Sonido...."
#elif defined(PROG_LANGUAGE_FRENCH)
"Son..."
#elif defined(PROG_LANGUAGE_GERMAN)
"Ton..."
#elif defined(PROG_LANGUAGE_ITALIAN)
"Suono..."
#elif defined(PROG_LANGUAGE_DUTCH)
"Geluid..."
#elif defined(PROG_LANGUAGE_PORTUGUESE)
"Som..."
#elif defined(PROG_LANGUAGE_NORWEGIAN)
"Lyd..."
#else
"Sound..."
#endif
	    , SAR_MENU_ID_GOTO_OPTIONS_SOUND
	);

	/* Back Button */
	SARMenuBuildStandardButton(
	    core_ptr, menu, 0.25f, 0.92f, btn_width_std, btn_height_std,
	    True,
#if defined(PROG_LANGUAGE_SPANISH)
"Espalda"
#elif defined(PROG_LANGUAGE_FRENCH)
"Dos"
#elif defined(PROG_LANGUAGE_GERMAN)
"Zurück"
#elif defined(PROG_LANGUAGE_ITALIAN)
"Dorso"
#elif defined(PROG_LANGUAGE_DUTCH)
"Rug"
#elif defined(PROG_LANGUAGE_PORTUGUESE)
"Costas"
#elif defined(PROG_LANGUAGE_NORWEGIAN)
"Frem"
#else
"Back"
#endif
	    , SAR_MENU_ID_GOTO_MAIN
	);

	/* Version Label */
	label_num = SARMenuLabelNew(
	    menu, 0.75f, 0.74f, 0, 0,
	    PROG_NAME_FULL "\nVersion " PROG_VERSION,
	    &label_color, font,
	    NULL	/* No background image */
	);
	if(label_num > -1)
	{
	    sar_menu_label_struct *label = SAR_MENU_LABEL(
		menu->object[label_num]
	    );
	    label->align = SAR_MENU_LABEL_ALIGN_RIGHT;
	}


	/* ****************************************************** */
	/* Menu: Options->Simulation */
	img_path = SARMenuBuildGetFullPath(SAR_DEF_MENU_BGIMG_STANDARD_FILE);
	menu = SARMenuNew(
	    SAR_MENU_TYPE_STANDARD,
	    SAR_MENU_NAME_OPTIONS_SIMULATION,
	    img_path
	);
	if(menu == NULL)
	    return(-1);
	else
	    SARBuildMenusAddToList(core_ptr, menu);
	menu->always_full_redraw = opt->menu_always_full_redraw;

	/* Units Spin */
	spin_num = SARMenuBuildStandardSpin(
	    core_ptr, menu, 0.5f, 0.14f, 0.9f, 0.0f,
#if defined(PROG_LANGUAGE_SPANISH)
"Unidades"
#elif defined(PROG_LANGUAGE_FRENCH)
"Unités"
#elif defined(PROG_LANGUAGE_GERMAN)
"Einheiten"
#elif defined(PROG_LANGUAGE_ITALIAN)
"Unitì"
#elif defined(PROG_LANGUAGE_DUTCH)
"Eenheden"
#elif defined(PROG_LANGUAGE_PORTUGUESE)
"Unidades"
#elif defined(PROG_LANGUAGE_NORWEGIAN)
"Enheter"
#else
"Units"
#endif
	    , SAR_MENU_ID_OPT_UNITS,
	    SARMenuOptionsSpinCB
	);
	if(spin_num > -1)
	{
	    spin = SAR_MENU_SPIN(menu->object[spin_num]);
	    spin->allow_warp = False;
	    SARMenuSpinAddValue(menu, spin_num, "English");
	    SARMenuSpinAddValue(menu, spin_num, "Metric");
	    SARMenuSpinAddValue(menu, spin_num, "Metric (Altitude In Feet)");
	}

	/* Hoist Contact Spin */
	spin_num = SARMenuBuildStandardSpin(
	    core_ptr, menu, 0.5f, 0.28f, 0.9f, 0.0f,
#if defined(PROG_LANGUAGE_SPANISH)
"Levante El Contacto"
#elif defined(PROG_LANGUAGE_FRENCH)
"Hisser Le Contact"
#elif defined(PROG_LANGUAGE_GERMAN)
"Ziehen Sie Kontakt Hoch"
#elif defined(PROG_LANGUAGE_ITALIAN)
"Sollevare Il Contatto"
#elif defined(PROG_LANGUAGE_DUTCH)
"Hijs Contact"
#elif defined(PROG_LANGUAGE_PORTUGUESE)
"Erga Contato"
#elif defined(PROG_LANGUAGE_NORWEGIAN)
"Heise Contact"
#else
"Hoist Contact"
#endif
	    , SAR_MENU_ID_OPT_HOIST_CONTACT,
	    SARMenuOptionsSpinCB
	);
	if(spin_num > -1)
	{
	    spin = SAR_MENU_SPIN(menu->object[spin_num]);
	    spin->allow_warp = False;
	    SARMenuSpinAddValue(menu, spin_num, "Easy");
	    SARMenuSpinAddValue(menu, spin_num, "Moderate");
	    SARMenuSpinAddValue(menu, spin_num, "Authentic");
	}

	/* Damage Resistance Spin */
	spin_num = SARMenuBuildStandardSpin(
	    core_ptr, menu, 0.5f, 0.39f, 0.9f, 0.0f,
#if defined(PROG_LANGUAGE_SPANISH)
"Dañe La Resistencia"
#elif defined(PROG_LANGUAGE_FRENCH)
"Endommager La Résistance"
#elif defined(PROG_LANGUAGE_GERMAN)
"Beschädigen Sie Widerstand"
#elif defined(PROG_LANGUAGE_ITALIAN)
"Danneggiare La Resistenza"
#elif defined(PROG_LANGUAGE_DUTCH)
"Beschadiig Weerstand"
#elif defined(PROG_LANGUAGE_PORTUGUESE)
"Estrague Resistência"
#elif defined(PROG_LANGUAGE_NORWEGIAN)
"Skad Resistance"
#else
"Damage Resistance"
#endif
	    , SAR_MENU_ID_OPT_DAMAGE_RESISTANCE,
	    SARMenuOptionsSpinCB
	);
	if(spin_num > -1)
	{
	    spin = SAR_MENU_SPIN(menu->object[spin_num]);
	    spin->allow_warp = False;
	    SARMenuSpinAddValue(menu, spin_num, "Strong");
	    SARMenuSpinAddValue(menu, spin_num, "Nominal");
	    SARMenuSpinAddValue(menu, spin_num, "Authentic");
	}

	/* Flight Physics Spin */
	spin_num = SARMenuBuildStandardSpin(
	    core_ptr, menu, 0.5f, 0.50f, 0.9f, 0.0f,
#if defined(PROG_LANGUAGE_SPANISH)
"La Física Del Vuelo"
#elif defined(PROG_LANGUAGE_FRENCH)
"Physique De Vol"
#elif defined(PROG_LANGUAGE_GERMAN)
"Flug Physik"
#elif defined(PROG_LANGUAGE_ITALIAN)
"La Fisica Di Volo"
#elif defined(PROG_LANGUAGE_DUTCH)
"Vlucht Fysica"
#elif defined(PROG_LANGUAGE_PORTUGUESE)
"A Física De Vôo"
#elif defined(PROG_LANGUAGE_NORWEGIAN)
"Flykt Physics"
#else
"Flight Physics"
#endif
	    , SAR_MENU_ID_OPT_FLIGHT_PHYSICS,
	    SARMenuOptionsSpinCB
	);
	if(spin_num > -1)
	{
	    spin = SAR_MENU_SPIN(menu->object[spin_num]);
	    spin->allow_warp = False;
	    SARMenuSpinAddValue(menu, spin_num, "Easy");
	    SARMenuSpinAddValue(menu, spin_num, "Moderate");
	    SARMenuSpinAddValue(menu, spin_num, "Realistic");
	}

	/* Reserved 1 Switch */
	SARMenuBuildStandardSwitch(
	    core_ptr, menu,
	    0.22f, 0.67f,
	    0, 0,
	    "Reserved 1",
	    False, -1,
	    SARMenuOptionsSwitchCB
	);

	/* Reserved 2 Switch */
	SARMenuBuildStandardSwitch(
	    core_ptr, menu,
	    0.50f, 0.67f,
	    0, 0,
	    "Reserved 2",
	    False, -1,
	    SARMenuOptionsSwitchCB
	);

	/* Reserved 3 Switch */
	SARMenuBuildStandardSwitch(
	    core_ptr, menu,
	    0.78f, 0.67f,
	    0, 0,
	    "Reserved 3",
	    False, -1,
	    SARMenuOptionsSwitchCB
	);

	/* Back Button */
	SARMenuBuildStandardButton(
	    core_ptr, menu, 0.25f, 0.92f, btn_width_std, btn_height_std,
	    True,
#if defined(PROG_LANGUAGE_SPANISH)
"Espalda"
#elif defined(PROG_LANGUAGE_FRENCH)
"Dos"
#elif defined(PROG_LANGUAGE_GERMAN)
"Zurück"
#elif defined(PROG_LANGUAGE_ITALIAN)
"Dorso"
#elif defined(PROG_LANGUAGE_DUTCH)
"Rug"
#elif defined(PROG_LANGUAGE_PORTUGUESE)
"Costas"
#elif defined(PROG_LANGUAGE_NORWEGIAN)
"Frem"
#else
"Back"
#endif
	    , SAR_MENU_ID_GOTO_OPTIONS
	);


	/* ****************************************************** */
	/* Menu: Options->Controller */
	img_path = SARMenuBuildGetFullPath(SAR_DEF_MENU_BGIMG_STANDARD_FILE);
	menu = SARMenuNew(
	    SAR_MENU_TYPE_STANDARD,
	    SAR_MENU_NAME_OPTIONS_CONTROLLER,
	    img_path
	);
	if(menu == NULL)
	    return(-1);
	else
	    SARBuildMenusAddToList(core_ptr, menu);
	menu->always_full_redraw = opt->menu_always_full_redraw;

	/* First joystick (js0) axis roles */
	spin_num = SARMenuBuildStandardSpin(
	    core_ptr, menu, 0.50f, 0.26f, 0.9f, 0.0f,
	    "Joystick #1 Axises", SAR_MENU_ID_OPT_JS0_AXISES,
	    SARMenuOptionsSpinCB
	);
	if(spin_num > -1)
	{
	    spin = SAR_MENU_SPIN(menu->object[spin_num]);
	    spin->allow_warp = False;
	    SARMenuSpinAddValue(menu, spin_num, "Off");
	    SARMenuSpinAddValue(menu, spin_num, "2D");
	    SARMenuSpinAddValue(menu, spin_num, "2D with throttle");
	    SARMenuSpinAddValue(menu, spin_num, "2D with hat");
	    SARMenuSpinAddValue(menu, spin_num, "2D with throttle & hat");
	    SARMenuSpinAddValue(menu, spin_num, "3D");
	    SARMenuSpinAddValue(menu, spin_num, "3D with throttle");
	    SARMenuSpinAddValue(menu, spin_num, "3D with hat");
	    SARMenuSpinAddValue(menu, spin_num, "3D with throttle & hat");
	    SARMenuSpinAddValue(menu, spin_num, "As throttle & rudder");
	    SARMenuSpinAddValue(menu, spin_num, "As rudder & wheel brakes");
	}

	/* Second joystick (js1) axis roles */
	spin_num = SARMenuBuildStandardSpin(
	    core_ptr, menu, 0.50f, 0.55f, 0.9f, 0.0f,
	    "Joystick #2 Axises", SAR_MENU_ID_OPT_JS1_AXISES,
	    SARMenuOptionsSpinCB
	);
	if(spin_num > -1)
	{
	    spin = SAR_MENU_SPIN(menu->object[spin_num]);
	    spin->allow_warp = False;
	    SARMenuSpinAddValue(menu, spin_num, "Off");
	    SARMenuSpinAddValue(menu, spin_num, "2D");
	    SARMenuSpinAddValue(menu, spin_num, "2D with throttle");
	    SARMenuSpinAddValue(menu, spin_num, "2D with hat");
	    SARMenuSpinAddValue(menu, spin_num, "2D with throttle & hat");
	    SARMenuSpinAddValue(menu, spin_num, "3D");
	    SARMenuSpinAddValue(menu, spin_num, "3D with throttle");
	    SARMenuSpinAddValue(menu, spin_num, "3D with hat");
	    SARMenuSpinAddValue(menu, spin_num, "3D with throttle & hat");
	    SARMenuSpinAddValue(menu, spin_num, "As throttle & rudder");
	    SARMenuSpinAddValue(menu, spin_num, "As rudder & wheel brakes");

	}

	/* Test Joystick Button */
	btn_num = SARMenuBuildStandardButton(
	    core_ptr, menu, 0.75f, 0.74f, btn_width_std, btn_height_std,
	    True,
	    "Test...", SAR_MENU_ID_GOTO_OPTIONS_CONTROLLER_TEST
	);

	/* Back Button */
	SARMenuBuildStandardButton(
	    core_ptr, menu, 0.25f, 0.92f, btn_width_std, btn_height_std,
	    True,
#if defined(PROG_LANGUAGE_SPANISH)
"Espalda"
#elif defined(PROG_LANGUAGE_FRENCH)
"Dos"
#elif defined(PROG_LANGUAGE_GERMAN)
"Zurück"
#elif defined(PROG_LANGUAGE_ITALIAN)
"Dorso"
#elif defined(PROG_LANGUAGE_DUTCH)
"Rug"
#elif defined(PROG_LANGUAGE_PORTUGUESE)
"Costas"
#elif defined(PROG_LANGUAGE_NORWEGIAN)
"Frem"
#else
"Back"
#endif
	    , SAR_MENU_ID_GOTO_OPTIONS
	);

	/* Joystick Button Mapping Button */
	btn_num = SARMenuBuildStandardButton(
	    core_ptr, menu, 0.75f, 0.92f, btn_width_std, btn_height_std,
	    True,
	    "Buttons", SAR_MENU_ID_GOTO_OPTIONS_CONTROLLER_JS_BTN
	);


	/* ****************************************************** */
	/* Menu: Options->Controller->Buttons */
	img_path = SARMenuBuildGetFullPath(SAR_DEF_MENU_BGIMG_STANDARD_FILE);
	menu = SARMenuNew(
	    SAR_MENU_TYPE_STANDARD,
	    SAR_MENU_NAME_OPTIONS_CONTROLLER_JS_BTN,
	    img_path
	);
	if(menu == NULL)
	    return(-1);
	else
	    SARBuildMenusAddToList(core_ptr, menu);
	menu->always_full_redraw = opt->menu_always_full_redraw;

	/* Joystick 1 (js0) button action spin */
	spin_num = SARMenuBuildStandardSpin(
	    core_ptr, menu, 0.5f, 0.14f, 0.9f, 0.0f,
	    "Joystick #1 Action", SAR_MENU_ID_OPT_JS0_BUTTON_ACTION,
	    SARMenuOptionsSpinCB
	);
	if(spin_num > -1)
	{
	    spin = SAR_MENU_SPIN(menu->object[spin_num]);
	    spin->allow_warp = False;
	    SARMenuSpinAddValue(menu, spin_num, "Rotate Modifier");
	    SARMenuSpinAddValue(menu, spin_num, "Air Brakes");
	    SARMenuSpinAddValue(menu, spin_num, "Wheel Brakes");
	    SARMenuSpinAddValue(menu, spin_num, "Zoom In");
	    SARMenuSpinAddValue(menu, spin_num, "Zoom Out");
	    SARMenuSpinAddValue(menu, spin_num, "Hoist Up");
	    SARMenuSpinAddValue(menu, spin_num, "Hoist Down");
	}

	/* Joystick 1 (js0) button number spin */
	spin_num = SARMenuBuildStandardSpin(
	    core_ptr, menu, 0.5f, 0.26f, 0.9f, 0.0f,
	    "Button Number", SAR_MENU_ID_OPT_JS0_BUTTON_NUMBER,
	    SARMenuOptionsSpinCB
	);
	if(spin_num > -1)
	{
	    spin = SAR_MENU_SPIN(menu->object[spin_num]);
	    spin->allow_warp = False;
	    /* Button values start from spin value - 1, since spin
	     * value 0 really means button -1 (none)
	     */
	    SARMenuSpinAddValue(menu, spin_num, "None");
	    SARMenuSpinAddValue(menu, spin_num, "1");
	    SARMenuSpinAddValue(menu, spin_num, "2");
	    SARMenuSpinAddValue(menu, spin_num, "3");
	    SARMenuSpinAddValue(menu, spin_num, "4");
	    SARMenuSpinAddValue(menu, spin_num, "5");
	    SARMenuSpinAddValue(menu, spin_num, "6");
	    SARMenuSpinAddValue(menu, spin_num, "7");
	    SARMenuSpinAddValue(menu, spin_num, "8");
	    SARMenuSpinAddValue(menu, spin_num, "9");
	    SARMenuSpinAddValue(menu, spin_num, "10");
	    SARMenuSpinAddValue(menu, spin_num, "11");
	    SARMenuSpinAddValue(menu, spin_num, "12");
	    SARMenuSpinAddValue(menu, spin_num, "13");
	    SARMenuSpinAddValue(menu, spin_num, "14");
	    SARMenuSpinAddValue(menu, spin_num, "15");
	    SARMenuSpinAddValue(menu, spin_num, "16");
	}


	/* Joystick 2 (js1) button action spin */
	spin_num = SARMenuBuildStandardSpin(
	    core_ptr, menu, 0.5f, 0.43f, 0.9f, 0.0f,
	    "Joystick #2 Action", SAR_MENU_ID_OPT_JS1_BUTTON_ACTION,
	    SARMenuOptionsSpinCB
	);
	if(spin_num > -1)
	{
	    spin = SAR_MENU_SPIN(menu->object[spin_num]);
	    spin->allow_warp = False;
	    SARMenuSpinAddValue(menu, spin_num, "Rotate Modifier");
	    SARMenuSpinAddValue(menu, spin_num, "Air Brakes");
	    SARMenuSpinAddValue(menu, spin_num, "Wheel Brakes");
	    SARMenuSpinAddValue(menu, spin_num, "Zoom In");
	    SARMenuSpinAddValue(menu, spin_num, "Zoom Out");
	    SARMenuSpinAddValue(menu, spin_num, "Hoist Up");
	    SARMenuSpinAddValue(menu, spin_num, "Hoist Down");
	}

	/* Joystick 2 (js1) button number spin */
	spin_num = SARMenuBuildStandardSpin(
	    core_ptr, menu, 0.5f, 0.55f, 0.9f, 0.0f,
	    "Button Number", SAR_MENU_ID_OPT_JS1_BUTTON_NUMBER,
	    SARMenuOptionsSpinCB
	);
	/* Button values start from spin value - 1, since spin
	 * value 0 really means button -1 (none)
	 */
	if(spin_num > -1)
	{
	    spin = SAR_MENU_SPIN(menu->object[spin_num]);
	    spin->allow_warp = False;
	    SARMenuSpinAddValue(menu, spin_num, "None");
	    SARMenuSpinAddValue(menu, spin_num, "1");
	    SARMenuSpinAddValue(menu, spin_num, "2");
	    SARMenuSpinAddValue(menu, spin_num, "3");
	    SARMenuSpinAddValue(menu, spin_num, "4");
	    SARMenuSpinAddValue(menu, spin_num, "5");
	    SARMenuSpinAddValue(menu, spin_num, "6");
	    SARMenuSpinAddValue(menu, spin_num, "7");
	    SARMenuSpinAddValue(menu, spin_num, "8");
	    SARMenuSpinAddValue(menu, spin_num, "9");
	    SARMenuSpinAddValue(menu, spin_num, "10");
	    SARMenuSpinAddValue(menu, spin_num, "11");
	    SARMenuSpinAddValue(menu, spin_num, "12");
	    SARMenuSpinAddValue(menu, spin_num, "13");
	    SARMenuSpinAddValue(menu, spin_num, "14");
	    SARMenuSpinAddValue(menu, spin_num, "15");
	    SARMenuSpinAddValue(menu, spin_num, "16");
	}

	/* Label */
	label_num = SARMenuLabelNew(
	    menu, 0.25f, 0.74f, 0, 0,
#if defined(PROG_LANGUAGE_SPANISH)
"El botón de palanca de\n\
mando de prensa para\n\
trazar el número del\n\
botón"
#elif defined(PROG_LANGUAGE_FRENCH)
"Appuie sur manche à\n\
balai le bouton pour\n\
faire la carte du\n\
numéro de bouton"
#elif defined(PROG_LANGUAGE_GERMAN)
"Presse Steuerknüppel\n\
Knopf, Knopf Zahl\n\
aufzuzeichnen"
#elif defined(PROG_LANGUAGE_ITALIAN)
"Premere il bottone\n\
di leva di comando al\n\
numero di bottone di\n\
mappa"
#elif defined(PROG_LANGUAGE_DUTCH)
"Pers knuppel knoop\n\
in kaart knoop nummer\n\
te brengen"
#elif defined(PROG_LANGUAGE_PORTUGUESE)
"Apertam botão de\n\
joystick a número de\n\
botão de mapa"
#elif defined(PROG_LANGUAGE_NORWEGIAN)
"Press joystick button\n\
to set button number"
#else
"Press joystick button\n\
to set button number"
#endif
	    , &label_color, font,
	    NULL        /* No background image */
	);
	if(label_num > -1)
	{
	    sar_menu_label_struct *label = SAR_MENU_LABEL(
		menu->object[label_num]
	    );
	    label->align = SAR_MENU_LABEL_ALIGN_LEFT;
	}

	/* Test Joystick Button */
	btn_num = SARMenuBuildStandardButton(
	    core_ptr, menu, 0.75f, 0.74f, btn_width_std, btn_height_std,
	    True, "Test...", SAR_MENU_ID_GOTO_OPTIONS_CONTROLLER_TEST
	);

	/* Back Button */
	SARMenuBuildStandardButton(
	    core_ptr, menu, 0.25f, 0.92f, btn_width_std, btn_height_std,
	    True,
#if defined(PROG_LANGUAGE_SPANISH)
"Espalda"
#elif defined(PROG_LANGUAGE_FRENCH)
"Dos"
#elif defined(PROG_LANGUAGE_GERMAN)
"Zurück"
#elif defined(PROG_LANGUAGE_ITALIAN)
"Dorso"
#elif defined(PROG_LANGUAGE_DUTCH)
"Rug"
#elif defined(PROG_LANGUAGE_PORTUGUESE)
"Costas"
#elif defined(PROG_LANGUAGE_NORWEGIAN)
"Frem"
#else
"Back"
#endif
	    , SAR_MENU_ID_GOTO_OPTIONS_CONTROLLER
	);


	/* ****************************************************** */
	/* Menu: Options->Controller->Test */
	img_path = SARMenuBuildGetFullPath(SAR_DEF_MENU_BGIMG_STANDARD_FILE);
	menu = SARMenuNew(
	    SAR_MENU_TYPE_STANDARD,
	    SAR_MENU_NAME_OPTIONS_CONTROLLER_TEST,
	    img_path
	);
	if(menu == NULL)
	    return(-1);
	else
	    SARBuildMenusAddToList(core_ptr, menu);
	menu->always_full_redraw = opt->menu_always_full_redraw;

	/* Joystick Output Display */
	SARMenuMDisplayNew(
	    menu, 0.50f, 0.36f, 0.95f, 0.60f,
	    (const sar_image_struct **)core_ptr->menu_list_bg_img,
	    core_ptr, SAR_MENU_ID_OPT_JS_TEST_UPDATE,
	    SARMenuOptionsJoystickTestDrawCB
	);

	/* Refresh Controllers Button */
	SARMenuBuildOptionsButton(
	    core_ptr, menu, 0.25f, 0.74f, btn_width_std, btn_height_std,
	    True, "Refresh", SAR_MENU_ID_OPT_CONTROLLER_REFRESH
	);

	/* Back Button */
	SARMenuBuildStandardButton(
	    core_ptr, menu, 0.25f, 0.92f, btn_width_std, btn_height_std,
	    True,
#if defined(PROG_LANGUAGE_SPANISH)
"Espalda"
#elif defined(PROG_LANGUAGE_FRENCH)
"Dos"
#elif defined(PROG_LANGUAGE_GERMAN)
"Zurück"
#elif defined(PROG_LANGUAGE_ITALIAN)
"Dorso"
#elif defined(PROG_LANGUAGE_DUTCH)
"Rug"
#elif defined(PROG_LANGUAGE_PORTUGUESE)
"Costas"
#elif defined(PROG_LANGUAGE_NORWEGIAN)
"Frem"
#else
"Back"
#endif
	    , SAR_MENU_ID_GOTO_OPTIONS_CONTROLLER
	);


	/* ****************************************************** */
	/* Menu: Options->Graphics */
	img_path = SARMenuBuildGetFullPath(SAR_DEF_MENU_BGIMG_STANDARD_FILE);
	menu = SARMenuNew(
	    SAR_MENU_TYPE_STANDARD,
	    SAR_MENU_NAME_OPTIONS_GRAPHICS,
	    img_path
	);
	if(menu == NULL)
	    return(-1);
	else
	    SARBuildMenusAddToList(core_ptr, menu);
	menu->always_full_redraw = opt->menu_always_full_redraw;

	/* Ground Texture Switch */
	SARMenuBuildStandardSwitch(
	    core_ptr, menu,
	    (float)((1.0 / 8.0) + (0 * 1.0 / 4.0)), 0.13f,
	    0, 0,
	    "Ground Texture",
	    False, SAR_MENU_ID_OPT_GROUND_TEXTURE,
	    SARMenuOptionsSwitchCB
	);
	/* Object Texture Switch */
	SARMenuBuildStandardSwitch(
	    core_ptr, menu,
	    (float)((1.0 / 8.0) + (1 * 1.0 / 4.0)), 0.13f,
	    0, 0,
	    "Object Texture",
	    False, SAR_MENU_ID_OPT_OBJECT_TEXTURE,
	    SARMenuOptionsSwitchCB
	);
	/* Clouds Switch */
	SARMenuBuildStandardSwitch(
	    core_ptr, menu,
	    (float)((1.0 / 8.0) + (2 * 1.0 / 4.0)), 0.13f,
	    0, 0,
	    "Clouds",
	    False, SAR_MENU_ID_OPT_CLOUDS,
	    SARMenuOptionsSwitchCB
	);
	/* Atmosphere Switch */
	SARMenuBuildStandardSwitch(
	    core_ptr, menu,
	    (float)((1.0 / 8.0) + (3 * 1.0 / 4.0)), 0.13f,
	    0, 0,
	    "Atmosphere",
	    False, SAR_MENU_ID_OPT_ATMOSPHERE,
	    SARMenuOptionsSwitchCB
	);
	/* Dual Pass Depth Switch */
	SARMenuBuildStandardSwitch(
	    core_ptr, menu,
	    (float)((1.0 / 8.0) + (0 * 1.0 / 4.0)), 0.35f,
	    0, 0,
	    "Dual Pass Depth",
	    False, SAR_MENU_ID_OPT_DUAL_PASS_DEPTH,
	    SARMenuOptionsSwitchCB
	);
	/* Prop Wash Switch */
	SARMenuBuildStandardSwitch(
	    core_ptr, menu,
	    (float)((1.0 / 8.0) + (1 * 1.0 / 4.0)), 0.35f,
	    0, 0,
	    "Prop Wash",
	    False, SAR_MENU_ID_OPT_PROP_WASH,
	    SARMenuOptionsSwitchCB
	);
	/* Smoke Trails Switch*/
	SARMenuBuildStandardSwitch(
	    core_ptr, menu,
	    (float)((1.0 / 8.0) + (2 * 1.0 / 4.0)), 0.35f,
	    0, 0,
	    "Smoke Trails",
	    False, SAR_MENU_ID_OPT_SMOKE_TRAILS,
	    SARMenuOptionsSwitchCB
	);
	/* Celestial Objects Switch */
	SARMenuBuildStandardSwitch(
	    core_ptr, menu,
	    (float)((1.0 / 8.0) + (3 * 1.0 / 4.0)), 0.35f,
	    0, 0,
	    "Celestial",
	    False, SAR_MENU_ID_OPT_CELESTIAL_OBJECTS,
	    SARMenuOptionsSwitchCB
	);

	/* Visibility Spin */
	spin_num = SARMenuBuildStandardSpin(
	    core_ptr, menu, 0.27f, 0.5f, 0.44f, 0.0f,
	    "Visibility", SAR_MENU_ID_OPT_VISIBILITY_MAX,
	    SARMenuOptionsSpinCB
	);
	if(spin_num >= 0)
	{
	    spin = SAR_MENU_SPIN(menu->object[spin_num]);
	    spin->allow_warp = False;
	    /* Values updated later */
	}

	/* Graphics Acceleration Slider */
	slider_num = SARMenuBuildStandardSlider(
	    core_ptr, menu,
	    0.27f, 0.6f, 0.44f, 0.0f,
	    "Graphics Accel",
	    SAR_MENU_ID_OPT_GRAPHICS_ACCELERATION,
	    SARMenuOptionsSliderCB
	);
	if(slider_num > -1)
	{
	    SARMenuSliderSetValueBounds(
		display, menu, slider_num,
		0.0f, 1.0f, False
	    );
	    SARMenuSliderSetValue(
		display, menu, slider_num,
		0.0f, False
	    );
	}

	/* Resolution Spin */
	spin_num = SARMenuBuildStandardSpin(
	    core_ptr, menu, 0.73f, 0.5f, 0.44f, 0.0f,
	    "Resolution", SAR_MENU_ID_OPT_RESOLUTION,
	    SARMenuOptionsSpinCB
	);
	if(spin_num > -1)
	{
	    spin = SAR_MENU_SPIN(menu->object[spin_num]);
	    spin->allow_warp = False;
	}

	int i;
	const int resols[] = RESOLUTIONS;
	char res_str[12];
	int total = sizeof(resols) / sizeof(int);
	for (i = 0; i<total; i+=2)
	{
	    snprintf(res_str, 12, "%dx%d", resols[i], resols[i+1]);
	    SARMenuSpinAddValue(menu, spin_num, res_str);
	}

	/* Fullscreen Switch */
	SARMenuBuildStandardSwitch(
	    core_ptr, menu,
	    0.73f, 0.62f,
	    0, 0,
	    "Fullscreen",
	    False, SAR_MENU_ID_OPT_FULLSCREEN,
	    SARMenuOptionsSwitchCB
	);

	/* Graphics Info Button */
	SARMenuBuildStandardButton(
	    core_ptr, menu, 0.75f, 0.74f, btn_width_std, btn_height_std,
	    True, "Info...", SAR_MENU_ID_GOTO_OPTIONS_GRAPHICS_INFO
	);

	/* Back Button */
	SARMenuBuildStandardButton(
	    core_ptr, menu, 0.25f, 0.92f, btn_width_std, btn_height_std,
	    True,
#if defined(PROG_LANGUAGE_SPANISH)
"Espalda"
#elif defined(PROG_LANGUAGE_FRENCH)
"Dos"
#elif defined(PROG_LANGUAGE_GERMAN)
"Zurück"
#elif defined(PROG_LANGUAGE_ITALIAN)
"Dorso"
#elif defined(PROG_LANGUAGE_DUTCH)
"Rug"
#elif defined(PROG_LANGUAGE_PORTUGUESE)
"Costas"
#elif defined(PROG_LANGUAGE_NORWEGIAN)
"Frem"
#else
"Back"
#endif
	    , SAR_MENU_ID_GOTO_OPTIONS
	);


	/* ****************************************************** */
	/* Menu: Options->Graphics->Info */
	img_path = SARMenuBuildGetFullPath(SAR_DEF_MENU_BGIMG_STANDARD_FILE);
	menu = SARMenuNew(
	    SAR_MENU_TYPE_STANDARD,
	    SAR_MENU_NAME_OPTIONS_GRAPHICS_INFO,
	    img_path
	);
	if(menu == NULL)
	    return(-1);
	else
	    SARBuildMenusAddToList(core_ptr, menu);
	menu->always_full_redraw = opt->menu_always_full_redraw;

	/* Graphics Info Message Box */
	SARMenuMessageBoxNew(
	    menu, 0.50f, 0.36f, 0.95f, 0.60f,
	    &mesgbox_color,
	    value_font,
	    (const sar_image_struct **)core_ptr->menu_list_bg_img,
	    core_ptr, SAR_MENU_ID_GRAPHICS_INFO_MESG,
	    NULL        /* No message */
	);

	/* Save Button */
	SARMenuBuildOptionsButton(
	    core_ptr, menu, 0.25f, 0.74f, btn_width_std, btn_height_std,
	    True, "Save", SAR_MENU_ID_OPT_GRAPHICS_INFO_SAVE
	);

	/* Back Button */
	SARMenuBuildStandardButton(
	    core_ptr, menu, 0.25f, 0.92f, btn_width_std, btn_height_std,
	    True,
#if defined(PROG_LANGUAGE_SPANISH)
"Espalda"
#elif defined(PROG_LANGUAGE_FRENCH)
"Dos"
#elif defined(PROG_LANGUAGE_GERMAN)
"Zurück"
#elif defined(PROG_LANGUAGE_ITALIAN)
"Dorso"
#elif defined(PROG_LANGUAGE_DUTCH)
"Rug"
#elif defined(PROG_LANGUAGE_PORTUGUESE)
"Costas"
#elif defined(PROG_LANGUAGE_NORWEGIAN)
"Frem"
#else
"Back"
#endif
	    , SAR_MENU_ID_GOTO_OPTIONS_GRAPHICS
	);


	/* ****************************************************** */
	/* Menu: Options->Sound */
	img_path = SARMenuBuildGetFullPath(SAR_DEF_MENU_BGIMG_STANDARD_FILE);
	menu = SARMenuNew(
	    SAR_MENU_TYPE_STANDARD,
	    SAR_MENU_NAME_OPTIONS_SOUND,
	    img_path
	);
	if(menu == NULL)
	    return(-1);
	else
	    SARBuildMenusAddToList(core_ptr, menu);
	menu->always_full_redraw = opt->menu_always_full_redraw;

	/* Sound Level Spin */
	spin_num = SARMenuBuildStandardSpin(
	    core_ptr, menu, 0.5f, 0.17f, 0.9f, 0.0f,
	    "Sound Level", SAR_MENU_ID_OPT_SOUND_LEVEL,
	    SARMenuOptionsSpinCB
	);
	if(spin_num > -1)
	{
	    spin = SAR_MENU_SPIN(menu->object[spin_num]);
	    spin->allow_warp = False;
	    SARMenuSpinAddValue(menu, spin_num, "Off");
	    SARMenuSpinAddValue(menu, spin_num, "Events");
	    SARMenuSpinAddValue(menu, spin_num, "Events and Engine");
	    SARMenuSpinAddValue(menu, spin_num, "Events, Engine, and Voice");
	}

	/* Sound Priority Spin */
	spin_num = SARMenuBuildStandardSpin(
	    core_ptr, menu, 0.27f, 0.33f, 0.44f, 0.0f,
	    "Priority", SAR_MENU_ID_OPT_SOUND_PRIORITY,
	    SARMenuOptionsSpinCB
	);
	if(spin_num > -1)
	{
	    spin = SAR_MENU_SPIN(menu->object[spin_num]);
	    spin->allow_warp = False;
	    SARMenuSpinAddValue(menu, spin_num, "Background");
	    SARMenuSpinAddValue(menu, spin_num, "Foreground");
	    SARMenuSpinAddValue(menu, spin_num, "Preempt");
	}

	/* Music Switch */
	switch_num = SARMenuBuildStandardSwitch(
	    core_ptr, menu,
	    0.27f, 0.46f,
	    0, 0,
	    "Music",
	    False, SAR_MENU_ID_OPT_MUSIC,
	    SARMenuOptionsSwitchCB
	);

	/* Volume Sliders */
#ifdef __MSW__
	/* Windows only has sound and music volume */
	/* Sound Volume Slider */
	slider_num = SARMenuBuildStandardSlider(
	    core_ptr, menu,
	    0.73f, 0.33f, 0.44f, 0.0f,
	    "Sound Volume ",
	    SAR_MENU_ID_OPT_VOLUME_SOUND,
	    SARMenuOptionsSliderCB
	);
	if(slider_num > -1)
	{
	    SARMenuSliderSetValueBounds(
		display, menu, slider_num,
		0.0f, 1.0f, False
	    );
	    SARMenuSliderSetValue(
		display, menu, slider_num,
		0.0f, False
	    );
	}

	/* Music Volume Slider */
	slider_num = SARMenuBuildStandardSlider(
	    core_ptr, menu,
	    0.73f, 0.46f, 0.44f, 0.0f,
	    "Music Volume ",
	    SAR_MENU_ID_OPT_VOLUME_MUSIC,
	    SARMenuOptionsSliderCB
	);
	if(slider_num > -1)
	{
	    SARMenuSliderSetValueBounds(
		display, menu, slider_num,
		0.0f, 1.0f, False
	    );
	}
#else
	/* UNIX Volume Sliders */
	/* Master Volume Slider */
	slider_num = SARMenuBuildStandardSlider(
	    core_ptr, menu,
	    0.73f, 0.33f, 0.44f, 0.0f,
	    "Master Volume",
	    SAR_MENU_ID_OPT_VOLUME_MASTER,
	    SARMenuOptionsSliderCB
	);
	if(slider_num > -1)
	{
	    SARMenuSliderSetValueBounds(
		display, menu, slider_num,
		0.0f, 1.0f, False
	    );
	    SARMenuSliderSetValue(
		display, menu, slider_num,
		0.0f, False
	    );
	}

	/* Sound Volume Slider */
	slider_num = SARMenuBuildStandardSlider(
	    core_ptr, menu,
	    0.73f, 0.46f, 0.44f, 0.0f,
	    "Sound Volume ",
	    SAR_MENU_ID_OPT_VOLUME_SOUND,
	    SARMenuOptionsSliderCB
	);
	if(slider_num > -1)
	{
	    SARMenuSliderSetValueBounds(
		display, menu, slider_num,
		0.0f, 1.0f, False
	    );
	    SARMenuSliderSetValue(
		display, menu, slider_num,
		0.0f, False
	    );
	}

	/* Music Volume Slider */
	slider_num = SARMenuBuildStandardSlider(
	    core_ptr, menu,
	    0.73f, 0.58f, 0.44f, 0.0f,
	    "Music Volume ",
	    SAR_MENU_ID_OPT_VOLUME_MUSIC,
	    SARMenuOptionsSliderCB
	);
	if(slider_num > -1)
	{
	    SARMenuSliderSetValueBounds(
		display, menu, slider_num,
		0.0f, 1.0f, False
	    );
	}
#endif	/* __MSW__ */

	/* Sound Test Button */
	SARMenuBuildOptionsButton(
	    core_ptr, menu, 0.25f, 0.74f, btn_width_std, btn_height_std,
	    True, "Test", SAR_MENU_ID_OPT_SOUND_TEST
	);

	/* Sound Info Button */
	SARMenuBuildStandardButton(
	    core_ptr, menu, 0.75f, 0.74f, btn_width_std, btn_height_std,
	    True, "Info...", SAR_MENU_ID_GOTO_OPTIONS_SOUND_INFO
	);

	/* Back Button */
	SARMenuBuildStandardButton(
	    core_ptr, menu, 0.25f, 0.92f, btn_width_std, btn_height_std,
	    True,
#if defined(PROG_LANGUAGE_SPANISH)
"Espalda"
#elif defined(PROG_LANGUAGE_FRENCH)
"Dos"
#elif defined(PROG_LANGUAGE_GERMAN)
"Zurück"
#elif defined(PROG_LANGUAGE_ITALIAN)
"Dorso"
#elif defined(PROG_LANGUAGE_DUTCH)
"Rug"
#elif defined(PROG_LANGUAGE_PORTUGUESE)
"Costas"
#elif defined(PROG_LANGUAGE_NORWEGIAN)
"Frem"
#else
"Back"
#endif
	    , SAR_MENU_ID_GOTO_OPTIONS
	);


	/* ****************************************************** */
	/* Menu: Options->Sound->Info */
	img_path = SARMenuBuildGetFullPath(SAR_DEF_MENU_BGIMG_STANDARD_FILE);
	menu = SARMenuNew(
	    SAR_MENU_TYPE_STANDARD,
	    SAR_MENU_NAME_OPTIONS_SOUND_INFO,
	    img_path
	);
	if(menu == NULL)
	    return(-1);
	else
	    SARBuildMenusAddToList(core_ptr, menu);
	menu->always_full_redraw = opt->menu_always_full_redraw;

	/* Sound Info Message Box */
	SARMenuMessageBoxNew(
	    menu, 0.50f, 0.36f, 0.95f, 0.60f,
	    &mesgbox_color,
	    value_font,
	    (const sar_image_struct **)core_ptr->menu_list_bg_img,
	    core_ptr, SAR_MENU_ID_SOUND_INFO_MESG,
	    NULL        /* No message */
	);

	/* Save Button */
	SARMenuBuildOptionsButton(
	    core_ptr, menu, 0.25f, 0.74f, btn_width_std, btn_height_std,
	    True, "Save", SAR_MENU_ID_OPT_SOUND_INFO_SAVE
	);

	/* Back Button */
	SARMenuBuildStandardButton(
	    core_ptr, menu, 0.25f, 0.92f, btn_width_std, btn_height_std,
	    True,
#if defined(PROG_LANGUAGE_SPANISH)
"Espalda"
#elif defined(PROG_LANGUAGE_FRENCH)
"Dos"
#elif defined(PROG_LANGUAGE_GERMAN)
"Zurück"
#elif defined(PROG_LANGUAGE_ITALIAN)
"Dorso"
#elif defined(PROG_LANGUAGE_DUTCH)
"Rug"
#elif defined(PROG_LANGUAGE_PORTUGUESE)
"Costas"
#elif defined(PROG_LANGUAGE_NORWEGIAN)
"Frem"
#else
"Back"
#endif
	    , SAR_MENU_ID_GOTO_OPTIONS_SOUND
	);


	/* ****************************************************** */
	/* Menu: Loading Simulation */
	img_path = SARMenuBuildGetFullPath(SAR_DEF_MENU_BGIMG_PROGRESS_FILE);
	menu = SARMenuNew(
	    SAR_MENU_TYPE_STANDARD,
	    SAR_MENU_NAME_LOADING_SIMULATION,
	    img_path
	);
	if(menu == NULL)
	    return(-1);
	else
	    SARBuildMenusAddToList(core_ptr, menu);
	menu->always_full_redraw = opt->menu_always_full_redraw;

	/* Progress Bar */
	SARMenuProgressNew(
	    menu,
	    0.5, 1.0,
	    0, 0,
	    NULL,
	    &progress_color,
	    value_font,
	    core_ptr->menu_progress_bg_img,
	    core_ptr->menu_progress_fg_img,
	    0
	);


	return(0);
}
