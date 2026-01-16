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

#ifndef EDITORGTKGUI_H
#define EDITORGTKGUI_H

#include <gtk/gtk.h>
#if GTK_MAJOR_VERSION == 3
#include <gdk/gdk.h>
#endif
#if GTK_MAJOR_VERSION == 4
#include <gdk/x11/gdkx.h>
#endif


/* Editor dialog identifiers */
typedef enum {
	DIALOG_ID_NONE,
	DO_YOU_WANT_TO_PRINT_BEFORE_QUIT
} editor_gtk_dialog_id;

typedef struct {
	/* Gtk specific data variables */
	GdkDisplay		*gdk_display;
	GMainContext		*gtk_context;
	GtkApplication		*gtk_application;

	int			picked_obj_num;
	void			*temp_obj_data; //editor_object_data_struct
	unsigned long		cmd_flags;		/* See SAR_CMD_PROTOTYPE */
} editor_gtk_ui_struct;


/* cmdscnedit.c */
extern int SceneObjectPick(const sar_core_struct *core_ptr,
			   int picker_obj_num,
			   Boolean next
);
void GwSetWindowFocusToSar2Window(const gw_display_struct *display);


/* objio.c */
extern char *COMPLETE_PATH(const char *path);

#endif	/* EDITORGTKGUI_H */
