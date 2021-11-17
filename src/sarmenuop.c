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

#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "menu.h"
#include "sar.h"
#include "sarmenuop.h"

void SARMenuSwitchToMenu(sar_core_struct *core_ptr, const char *name);
sar_menu_list_struct *SARMenuGetList(
	sar_menu_struct *m, int skip, int *list_num
);
sar_menu_spin_struct *SARMenuGetSpin(
	sar_menu_struct *m, int skip, int *spin_num
);
sar_menu_list_item_struct *SARMenuListGetSelectedItem(
	sar_menu_struct *m, int id
);
char *SARMenuGetCurrentMenuName(sar_core_struct *core_ptr);


#define MAX(a,b)        (((a) > (b)) ? (a) : (b))
#define MIN(a,b)        (((a) < (b)) ? (a) : (b))
#define CLIP(a,l,h)     (MIN(MAX((a),(l)),(h)))
#define STRDUP(s)       (((s) != NULL) ? strdup(s) : NULL)


/*
 *	Switches to the specified menu.
 *
 *	If no menu is specified (name=NULL) then simulation will
 *	begin.
 */
void SARMenuSwitchToMenu(sar_core_struct *core_ptr, const char *name)
{
	int new_menu_num, old_menu_num;
	gw_display_struct *display;
	const sar_option_struct *opt;

	if(core_ptr == NULL)
	    return;

	display = core_ptr->display;
	opt = &core_ptr->option;

	/* Record previous menu number */
	old_menu_num = core_ptr->cur_menu;

	/* No specified menu to switch to? */
	if(name == NULL)
	{
	    /* This implies we are going out of the menu system and
	     * entering simulation
	     */
	    sar_menu_struct *old_menu = SARIsMenuAllocated(core_ptr, old_menu_num) ?
		core_ptr->menu[old_menu_num] : NULL;

	    /* Unload image on old menu */
/*	    if(opt->prioritize_memory && (old_menu != NULL)) */
	    if(old_menu != NULL)
		SARMenuUnloadBackgroundImage(old_menu);

	    /* Unselect current menu on core structure, indicating that
	     * no menu is selected and that we are not in the menus
	     */
	    core_ptr->cur_menu = -1;

	    /* Skip redrawing, since redrawing would be done rather
	     * frequently during the simulation
	     */
/*	    GWPostRedraw(display); */

	    return;
	}

	/* Get the number of the menu that we want to switch to */
	new_menu_num = SARMatchMenuByName(core_ptr, name);
	/* Got match and matched menu number is different from the one
	 * currently selected?
	 */
	if((new_menu_num > -1) && (new_menu_num != core_ptr->cur_menu))
	{
	    sar_menu_struct *old_menu = SARIsMenuAllocated(core_ptr, old_menu_num) ?
		core_ptr->menu[old_menu_num] : NULL;
	    sar_menu_struct *new_menu = SARIsMenuAllocated(core_ptr, new_menu_num) ?
		core_ptr->menu[new_menu_num] : NULL;
	    const char	*old_bg_image_path = (old_menu != NULL) ?
		old_menu->bg_image_path : NULL,
			*new_bg_image_path = (new_menu != NULL) ?
		new_menu->bg_image_path : NULL;

	    GWSetInputBusy(display);

	    /* Reset values on the previous menu */
            /*
	    if(old_menu != NULL)
	    {





	    }
            */

	    /* Check if background images are the same on both old and
	     * new menus by comparing the background image paths (if
	     * defined)
	     */
	    if(((old_bg_image_path != NULL) &&
		(new_bg_image_path != NULL)) ?
	         ((old_menu->bg_image != NULL) ?
		  !strcmp(old_bg_image_path, new_bg_image_path) : False)
		: False
	    )
	    {
		/* Image paths are the same, so unload the image on the
		 * new menu and transfer the image from the old menu to
		 * the new menu
		 *
		 * Mark the old menu's image NULL after transfer so
		 * that it dosen't get referenced again
		 *
		 * Note that both new_menu and old_menu are known to be
		 * not NULL from the above check
		 */
		SARMenuUnloadBackgroundImage(new_menu);
		if(new_menu->bg_image == NULL)
		{
		    new_menu->bg_image = old_menu->bg_image;
		    old_menu->bg_image = NULL;
		}
	    }
	    else
	    {
		/* Image paths are different, so unload background image
		 * on old menu and load new background image on new menu
		 */
/*		if(opt->prioritize_memory && (old_menu != NULL)) */
		if(old_menu != NULL)
		    SARMenuUnloadBackgroundImage(old_menu);

		if(opt->menu_backgrounds &&
		   ((new_menu != NULL) ? (new_menu->bg_image == NULL) : False)
		)
		    SARMenuLoadBackgroundImage(new_menu);
	    }

            glClearColor(0, 0, 0, 0);
            glClear(GL_COLOR_BUFFER_BIT);
            GWSwapBuffer(core_ptr->display);
            GWFlush(core_ptr->display);

	    /* Is the new menu valid? */
	    if(new_menu != NULL)
	    {
		/* New menu is valid, so switch to it */
		int n;
		void *o;
		sar_menu_button_struct *btn;
		sar_menu_struct *m = new_menu;

		/* Begin resetting object values on the new menu */
		for(n = 0; n < m->total_objects; n++)
		{
		    o = m->object[n];
		    if(o == NULL)
			continue;

		    switch(SAR_MENU_OBJECT_TYPE(o))
		    {
		      case SAR_MENU_OBJECT_TYPE_BUTTON:
			btn = SAR_MENU_BUTTON(o);
			btn->state = SAR_MENU_BUTTON_STATE_UNARMED;
			break;

		      case SAR_MENU_OBJECT_TYPE_PROGRESS:
			SARMenuProgressSet(
			    display, m, n,
			    0.0f, False
			);
			break;
		    }
		}

		/* Select first object on menu */
		m->selected_object = 0;

		/* Update index number of the current menu */
		core_ptr->cur_menu = new_menu_num;

		/* Redraw current menu */
		SARMenuDrawAll(display, m);
		GWSwapBuffer(display);
	    }

	    /* Redraw */
	    GWPostRedraw(display);


	    /* Mark input as ready */
	    GWSetInputReady(display);
	}
}

/*
 *	Returns the pointer to the list object on the menu,
 *	skipping the indicated number of lists.
 *
 *	Can return NULL for no match.
 */
sar_menu_list_struct *SARMenuGetList(
	sar_menu_struct *m, int skip, int *list_num
)     
{
	int i;
	void *ptr;
	sar_menu_list_struct *list_ptr;


	if(m == NULL)
	    return(NULL);

	if(list_num != NULL)
	    *list_num = -1;
 
	for(i = 0; i < m->total_objects; i++)
	{       
	    ptr = m->object[i];
	    if(ptr == NULL)
		continue;

	    if(*(int *)ptr != SAR_MENU_OBJECT_TYPE_LIST)
		continue;

	    /* Get pointer to list object. */
	    list_ptr = ptr;

	    /* Skip this list object? */
	    if(skip > 0)
	    {
		skip--;
		continue;
	    }
	    else
	    {
		if(list_num != NULL)
		    *list_num = i;
		return(list_ptr);
	    }
	}

	return(NULL);
}

/*
 *      Returns the pointer to the spin object on the menu,
 *      skipping the indicated number of spin objects.
 *
 *	Can return NULL for no match.
 */
sar_menu_spin_struct *SARMenuGetSpin(
	sar_menu_struct *m, int skip, int *spin_num
)
{
	int i;
	void *ptr;
	sar_menu_spin_struct *spin_ptr;


	if(m == NULL)
	    return(NULL);

	if(spin_num != NULL)
	    *spin_num = -1;

	for(i = 0; i < m->total_objects; i++)
	{
	    ptr = m->object[i];
	    if(ptr == NULL)
		continue;
 
	    if(*(int *)ptr != SAR_MENU_OBJECT_TYPE_SPIN)
		continue;

	    /* Get pointer to spin object. */
	    spin_ptr = ptr;

	    /* Skip this spin object? */
	    if(skip > 0)
	    {
		skip--;
		continue;
	    }
	    else
	    {
		if(spin_num != NULL)
		    *spin_num = i;

		return(spin_ptr);
	    }
	}

	return(NULL);
}

/*
 *	Returns the pointer to the selected list item on the list object
 *	on the menu, skipping the indicated number of list objects.
 */
sar_menu_list_item_struct *SARMenuListGetSelectedItem(
	sar_menu_struct *m, int id
)
{
	int i;
	sar_menu_list_struct *list = SAR_MENU_LIST(
	    SARMenuGetObjectByID(m, id, NULL)
	);
	if(list == NULL)
	    return(NULL);

	i = list->selected_item;
	if((i >= 0) && (i < list->total_items))
	    return(list->item[i]);
	else
	    return(NULL);
}

/*
 *	Returns the pointer to the name of the currently selected
 *	menu or NULL on failure.
 */
char *SARMenuGetCurrentMenuName(sar_core_struct *core_ptr)
{
	int cur_menu;
	sar_menu_struct *m;


	if(core_ptr == NULL)
	    return(NULL);

	cur_menu = core_ptr->cur_menu;
	if(SARIsMenuAllocated(core_ptr, cur_menu))
	    m = core_ptr->menu[cur_menu];
	else
	    return(NULL);

	return(m->name);
}
