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

#include <locale.h>

#include "sar.h"
#include "cmdscnedit.h"
#include "config.h"
#include "cmd.h"


/*
 * WARNING:
 * For now, this code compiles and works fine with the GTK3(.24) library.
 * With GTK4(.10), on my Linux system, code crashes after a lot of
 * "gdk_gl_context_make_current() failed" statements.
 * Note that as GTK4 code crashes at a very early state, I had to stop its
 * developpment for now thus some stuffs are not yet implemented for this GTK
 * version. Look for "gtk_version =" in the scons script to compile with GTK4
 * if you want to test it.
 *
 * Programming note:
 * All the hereunder code is only used to manage the User Interface: the
 * 'real job' is done by calling the SARCmdSceneEditor() function.
 * It means that for example when user clicks on the 'New' button, then selects
 * the object type as 'Fire', sets the fire radius and height values, then
 * clicks on the 'Ok' button, the result of all these actions is only a string
 * which will be sent to SARCmdSceneEditor() exactly as if user had entered
 * '/create_fire radius height' on the keyboard.
 */



void app_startup_cb(GApplication *app, gpointer user_data);
void app_activate_cb(GApplication *app, gpointer user_data);
int X11GetWindowDecorationHeight(Display *display, Window window);
void GwGetSar2WindowTopRightCornerPos(const gw_display_struct *display,
				      int *x, int *y
);
void GwSetWindowFocusToSar2Window(const gw_display_struct *display);
int gtkAppStart(sar_core_struct *core_ptr, unsigned long flags);
void gtkAppStop(sar_core_struct *core_ptr);
void EditorGtkSetAcceptFocus(GtkWidget *window, gboolean state);
void EditorGtkItemChooserSetHumansNameList(const sar_core_struct *core_ptr);
void EditorGtkItemChooserSetTexturesNameList(const sar_core_struct *core_ptr);
void EditorGtkItemChooserSetRefObjsNameList(const sar_core_struct *core_ptr);
gboolean EditorGtkItemChooserSetItemsList(GtkWidget *widget, const char* const* strings);
char *EditorGtkItemChooserGetSelectedString(void *widget);
int EditorGtkItemChooserSetSelectedItemFromString(void *widget, const char *text);
int EditorGtkItemChooserGetItemIndexFromString(void *widget, const char *text);
void EditorGktEntryFilterText(void *widget);
void EditorGktEntryFilterTextNoSpace(void *widget);
void EditoGktEntryAllowInt(void *widget, gboolean non_negative);
int EditorGktEntryToInt(void *widget);
void EditoGktEntryAllowDouble(void *widget, gboolean non_negative);
double EditorGktEntryToDouble(void *widget);
void EditorGtkEntrySetText(void *widget, const char *text);
void EditorGtkEntrySetTextFromInt(void *widget, double value);
void EditorGtkEntrySetTextFromDouble(void *widget, double value);
void EditorGtkAskToQuitWithoutPrint();
void set_children_sensitive_by_name(
	GtkWidget* parent,
	const gchar* name,
	gboolean sensitive
);


char *GetModelNameFromModelFileName(const char *file_name);
char **GetTextureNameListFromSceneryFileName(const char *file_name);

void on_data_window_destroy(GtkWidget *widget, gpointer user_data);
void on_dialog_window_destroy(GtkWidget *widget, gpointer user_data);
void on_menu_window_destroy(GtkWidget *widget, gpointer user_data);
void on_general_type_changed(void *widget, gpointer user_data);
void on_obj_name_changed(void *widget, gpointer user_data);
void on_pos_x_changed(void *widget, gpointer user_data);
void on_pos_y_changed(void *widget, gpointer user_data);
void on_pos_z_changed(void *widget, gpointer user_data);
void on_dir_heading_changed(void *widget, gpointer user_data);
void on_dir_pitch_changed(void *widget, gpointer user_data);
void on_dir_bank_changed(void *widget, gpointer user_data);
void on_fire_radius_changed(void *widget, gpointer user_data);
void on_height_changed(void *widget, gpointer user_data);
void on_helipad_style_changed(void *widget, gpointer user_data);
void on_length_changed(void *widget, gpointer user_data);
void on_width_changed(void *widget, gpointer user_data);
void on_helipad_recession_changed(void *widget, gpointer user_data);
void on_helipad_label_changed(void *widget, gpointer user_data);
void on_helipad_lighting_toggled(void *widget, gpointer user_data);
void on_helipad_fuel_toggled(void *widget, gpointer user_data);
void on_helipad_repair_toggled(void *widget, gpointer user_data);
void on_helipad_drop_off_toggled(void *widget, gpointer user_data);
void on_helipad_is_restarting_toggled(void *widget, gpointer user_data);
void on_helipad_is_referenced_toggled(void *widget, gpointer user_data);
void on_helipad_ref_obj_changed(void *widget, gpointer user_data);
void on_ref_object_pos_x_changed(void *widget, gpointer user_data);
void on_ref_object_pos_y_changed(void *widget, gpointer user_data);
void on_ref_object_pos_z_changed(void *widget, gpointer user_data);
void on_ref_object_dir_heading_changed(void *widget, gpointer user_data);
void on_ref_object_dir_pitch_changed(void *widget, gpointer user_data);
void on_ref_object_dir_bank_changed(void *widget, gpointer user_data);
void on_offset_pos_x_changed(void *widget, gpointer user_data);
void on_offset_pos_y_changed(void *widget, gpointer user_data);
void on_offset_pos_z_changed(void *widget, gpointer user_data);
void on_offset_dir_heading_changed(void *widget, gpointer user_data);
void on_offset_dir_pitch_changed(void *widget, gpointer user_data);
void on_offset_dir_bank_changed(void *widget, gpointer user_data);
void on_human_type_name_changed(void *widget, gpointer user_data);
void on_human_need_rescue_toggled(void *widget, gpointer user_data);
void on_human_sit_up_toggled(void *widget, gpointer user_data);
void on_human_sit_down_toggled(void *widget, gpointer user_data);
void on_human_sitting_toggled(void *widget, gpointer user_data);
void on_human_lying_toggled(void *widget, gpointer user_data);
void on_human_alert_toggled(void *widget, gpointer user_data);
void on_human_aware_toggled(void *widget, gpointer user_data);
void on_human_human_in_water_toggled(void *widget, gpointer user_data);
void on_human_on_stretcher_toggled(void *widget, gpointer user_data);
void on_human_assistants_changed(void *widget, gpointer user_data);
void on_human_assist_1_name_changed(void *widget, gpointer user_data);
void on_human_assist_2_name_changed(void *widget, gpointer user_data);
void on_human_assist_3_name_changed(void *widget, gpointer user_data);
void on_human_assist_4_name_changed(void *widget, gpointer user_data);
void on_human_has_displacement_toggled(void *widget, gpointer user_data);
void on_human_ref_obj_changed(void *widget, gpointer user_data);
void on_human_displacement_changed(void *widget, gpointer user_data);

#if GTK_MAJOR_VERSION == 3 && GTK_MINOR_VERSION >= 24
void on_model_file_name_changed(void *widget, gpointer user_data);
#endif
#if GTK_MAJOR_VERSION == 4 && GTK_MINOR_VERSION >= 10
void EditorGtk4SetFileWidgetsValues(const char *full_name);
void FileChoosed(
    GObject* source_object, GAsyncResult* res, gpointer user_data
);
void on_model_file_name_clicked(void *widget, gpointer user_data);
#endif
void on_premod_type_changed(void *widget, gpointer user_data);
void on_range_changed(void *widget, gpointer user_data);
void on_premod_hazard_lights_changed(void *widget, gpointer user_data);
void on_premod_walls_tex_changed(void *widget, gpointer user_data);
void on_premod_walls_tex_night_changed(void *widget, gpointer user_data);
void on_premod_roof_tex_changed(void *widget, gpointer user_data);
void on_runway_surface_type_changed(void *widget, gpointer user_data);
void on_runway_dashes_num_changed(void *widget, gpointer user_data);
void on_runway_edge_light_spacing_changed(void *widget, gpointer user_data);
void on_runway_north_label_changed(void *widget, gpointer user_data);
void on_runway_south_label_changed(void *widget, gpointer user_data);
void on_runway_n_disp_thresh_changed(void *widget, gpointer user_data);
void on_runway_s_disp_thresh_changed(void *widget, gpointer user_data);
void on_runway_has_thresholds_toggled(void *widget, gpointer user_data);
void on_runway_has_borders_toggled(void *widget, gpointer user_data);
void on_runway_has_td_markers_toggled(void *widget, gpointer user_data);
void on_runway_has_mid_markers_toggled(void *widget, gpointer user_data);
void on_runway_has_north_gs_toggled(void *widget, gpointer user_data);
void on_runway_has_south_gs_toggled(void *widget, gpointer user_data);
void on_smoke_radius_start_changed(void *widget, gpointer user_data);
void on_smoke_radius_max_changed(void *widget, gpointer user_data);
void on_smoke_radius_rate_changed(void *widget, gpointer user_data);
void on_smoke_hide_at_max_changed(void *widget, gpointer user_data);
void on_smoke_respawn_int_changed(void *widget, gpointer user_data);
void on_smoke_total_units_changed(void *widget, gpointer user_data);
void on_smoke_color_code_changed(void *widget, gpointer user_data);
void on_button_ok_clicked(void *widget, gpointer user_data);
void on_button_apply_clicked(void *widget, gpointer user_data);
void on_button_cancel_clicked(void *widget, gpointer user_data);
void on_button_quit_clicked(void *widget, gpointer user_data);
void on_button_print_clicked(void *widget, gpointer user_data);
void on_button_new_clicked(void *widget, gpointer user_data);
void on_button_set_clicked(void *widget, gpointer user_data);
void on_button_copy_clicked(void *widget, gpointer user_data);
void on_button_info_clicked(void *widget, gpointer user_data);
void on_button_info_next_clicked(void *widget, gpointer user_data);
void on_button_unload_clicked(void *widget, gpointer user_data);
void on_button_modify_clicked(void *widget, gpointer user_data);
void on_button_move_clicked(void *widget, gpointer user_data);
void on_button_remove_clicked(void *widget, gpointer user_data);
void on_button_quit_without_print_yes_clicked(GtkButton *button, gpointer user_data);
void on_button_quit_without_print_no_clicked(GtkButton *button, gpointer user_data);
editor_object_data_struct *EditorObjectDataStructNew(void);
int EditorObjectDataStructReinit(editor_object_data_struct *object_data);
int EditorObjectDataStructFree(editor_object_data_struct *object_data);
int EditorObjectDataStructSetFromGtkUi(
    //sar_core_struct *core_ptr,
    editor_object_data_struct *editor_obj_data
);
int EditorGtkUiSetFromObjectDataStruct(
    //sar_core_struct *core_ptr,
    const editor_object_data_struct *editor_obj_data
);
void EditorGtkUiShowInfoWindow(
    const sar_core_struct *core_ptr,
    int picked_obj_num,
    const editor_object_data_struct *editor_obj_data
);
char *DoCmdLineFromObjectDataStruct(editor_object_data_struct *editor_obj_data);

#define STRDUP(s)       (((s) != NULL) ? strdup(s) : NULL)

/*
 * User Interface definition. Three windows are defined:
 * - One for the "menu" buttons
 * - One for the "object data"
 * - One for the "quit without print" dialog (the Gtk.Dialog class is deprecated since 4.10)
 */

#if GTK_MAJOR_VERSION == 3 && GTK_MINOR_VERSION >= 24
#define EDITORMENUGTK_UI "editormenugtk3.ui"
#define EDITORDATAGTK_UI "editordatagtk3.ui"
#define EDITORUITWITHOUTPRINTGTK_UI "editorquitwithoutprintgtk3.ui"
#endif
#if GTK_MAJOR_VERSION == 4 && GTK_MINOR_VERSION >= 10
#define EDITORMENUGTK_UI "editormenugtk4.ui"
#define EDITORDATAGTK_UI "editordatagtk4.ui"
#define EDITORUITWITHOUTPRINTGTK_UI "editorquitwithoutprintgtk4.ui"
#endif

/* What follows is the list of the widgets contained in EDITORDATAGTK_UI
that will be accessible to the code through "editor_*_widgets.[WidgetID]".
Whatever is present in this list will be present in the structure
"editor_*_widgets" as well. The format of each row is:

    _____([WidgetType],		[WidgetCastFunction],	[WidgetID],		[Signal],	[CallbackFunction],			[ToolTip])
*/

/*
 *							**** GTK3 UI version *****
 */
    /*
     * Menu buttons window. Please make sure that each [WidgetID] is contained in editormenugtk3.ui!
     */
#define editor_menu_widgets_gtk3_LIST(_____)													\
    _____(GtkWidget,		GTK_WIDGET,		window,			"destroy", 	on_menu_window_destroy,			NULL)	\
    _____(GtkGrid,		GTK_GRID,		grid,			NULL,	 	NULL,					NULL)	\
    _____(GtkButton,		GTK_BUTTON,		button_print,		"clicked", 	on_button_print_clicked,		NULL)	\
    _____(GtkButton,		GTK_BUTTON,		button_quit,		"clicked", 	on_button_quit_clicked,			NULL)	\
    _____(GtkButton,		GTK_BUTTON,		button_object_new,	"clicked", 	on_button_new_clicked,			NULL)	\
    _____(GtkButton,		GTK_BUTTON,		button_object_set,	"clicked", 	on_button_set_clicked,			NULL)	\
    _____(GtkButton,		GTK_BUTTON,		button_object_copy,	"clicked", 	on_button_copy_clicked,			NULL)	\
    _____(GtkButton,		GTK_BUTTON,		button_object_info,	"clicked", 	on_button_info_clicked,			NULL)	\
    _____(GtkButton,		GTK_BUTTON,		button_object_info_next,"clicked", 	on_button_info_next_clicked,		NULL)	\
    _____(GtkButton,		GTK_BUTTON,		button_object_unload,	"clicked", 	on_button_unload_clicked,		NULL)	\
    _____(GtkButton,		GTK_BUTTON,		button_object_modify,	"clicked", 	on_button_modify_clicked,		NULL)	\
    _____(GtkButton,		GTK_BUTTON,		button_object_move,	"clicked", 	on_button_move_clicked,			NULL)	\
    _____(GtkButton,		GTK_BUTTON,		button_object_remove,	"clicked", 	on_button_remove_clicked,		NULL)

    /*
     * Object data window. Please make sure that each [WidgetID] is contained in editordatagtk3.ui!
     */
#define editor_data_widgets_gtk3_LIST(_____)													\
/* Please, make sure that each [WidgetID] is contained in editorgtk*.ui! */									\
    _____(GtkWidget,		GTK_WIDGET,		window,	"delete_event", gtk_widget_hide_on_delete,				NULL)	\
    _____(GtkWidget,		GTK_WIDGET,		scrolled_window,	NULL,	 	NULL,					NULL)	\
    _____(GtkBox,		GTK_BOX,		main_box,		NULL,	 	NULL,					NULL)	\
    _____(GtkButton,		GTK_BUTTON,		button_ok,		"clicked", 	on_button_ok_clicked,			NULL)	\
    _____(GtkButton,		GTK_BUTTON,		button_apply,		"clicked", 	on_button_apply_clicked,		NULL)	\
    _____(GtkButton,		GTK_BUTTON,		button_cancel,		"clicked", 	on_button_cancel_clicked,		NULL)	\
    /*																		\
     * Frame widgets. These widgets have to be referenced because they will be shown or hidden as needed, depending of object type.		\
     */																		\
    _____(GtkFrame,		GTK_FRAME,		general_frame,		NULL, 		NULL,					NULL)	\
    _____(GtkGrid,		GTK_GRID,		general_frame_grid,	NULL, 		NULL,					NULL)	\
    _____(GtkFrame,		GTK_FRAME,		fire_frame,		NULL, 		NULL,					NULL)	\
    _____(GtkGrid,		GTK_GRID,		fire_frame_grid,	NULL, 		NULL,					NULL)	\
    _____(GtkFrame,		GTK_FRAME,		helipad_frame,		NULL, 		NULL,					NULL)	\
    _____(GtkGrid,		GTK_GRID,		helipad_frame_grid,	NULL, 		NULL,					NULL)	\
    _____(GtkFrame,		GTK_FRAME,		ref_object_frame,	NULL,	 	NULL,					NULL)	\
    _____(GtkFrame,		GTK_FRAME,		human_frame,		NULL, 		NULL,					NULL)	\
    _____(GtkGrid,		GTK_GRID,		human_frame_grid,	NULL, 		NULL,					NULL)	\
    _____(GtkFrame,		GTK_FRAME,		model_frame,		NULL, 		NULL,					NULL)	\
    _____(GtkGrid,		GTK_GRID,		model_frame_grid,	NULL, 		NULL,					NULL)	\
    _____(GtkFrame,		GTK_FRAME,		premodeled_frame,	NULL, 		NULL,					NULL)	\
    _____(GtkGrid,		GTK_GRID,		premodeled_frame_grid,	NULL, 		NULL,					NULL)	\
    _____(GtkFrame,		GTK_FRAME,		runway_frame,		NULL, 		NULL,					NULL)	\
    _____(GtkGrid,		GTK_GRID,		runway_frame_grid,	NULL, 		NULL,					NULL)	\
    _____(GtkFrame,		GTK_FRAME,		smoke_frame,		NULL, 		NULL,					NULL)	\
    _____(GtkGrid,		GTK_GRID,		smoke_frame_grid,	NULL, 		NULL,					NULL)	\
    /*																		\
     * Most of the "data entry" widgets are connected to an on_* callback function. A same callback can be called by several widgets.		\
     * Label widgets are not connected to callbacks, and only some of them appears in this list.						\
     * As all hereunder widgets are referenced, they can be easily shown/hidden/enabled/disabled/and so on... separatly as needed.		\
     */																		\
    _____(GtkComboBoxText,	GTK_COMBO_BOX_TEXT,	general_type,		"changed", 	on_general_type_changed,		NULL)	\
    _____(GtkLabel,		GTK_LABEL,		general_object_name_label,NULL, 	NULL,					NULL)	\
    _____(GtkEntry,		GTK_ENTRY,		general_object_name,	"changed", 	on_obj_name_changed,			NULL)	\
    _____(GtkLabel,		GTK_LABEL,		general_pos_label,	NULL, 		NULL,					NULL)	\
    _____(GtkEntry,		GTK_ENTRY,		general_pos_x,		"changed", 	on_pos_x_changed,			NULL)	\
    _____(GtkEntry,		GTK_ENTRY,		general_pos_y,		"changed", 	on_pos_y_changed,			NULL)	\
    _____(GtkEntry,		GTK_ENTRY,		general_pos_z,		"changed", 	on_pos_z_changed,			NULL)	\
    _____(GtkLabel,		GTK_LABEL,		general_dir_label,	NULL, 		NULL,					NULL)	\
    _____(GtkEntry,		GTK_ENTRY,		general_dir_heading,	"changed", 	on_dir_heading_changed,			NULL)	\
    _____(GtkEntry,		GTK_ENTRY,		general_dir_pitch,	"changed", 	on_dir_pitch_changed,			NULL)	\
    _____(GtkEntry,		GTK_ENTRY,		general_dir_bank,	"changed", 	on_dir_bank_changed,			NULL)	\
    _____(GtkEntry,		GTK_ENTRY,		fire_radius,		"changed", 	on_fire_radius_changed,			NULL)	\
    _____(GtkEntry,		GTK_ENTRY,		fire_height,		"changed", 	on_height_changed,			NULL)	\
    _____(GtkComboBoxText,	GTK_COMBO_BOX_TEXT,	helipad_style,		"changed", 	on_helipad_style_changed,		NULL)	\
    _____(GtkEntry,		GTK_ENTRY,		helipad_length,		"changed", 	on_length_changed,			NULL)	\
    _____(GtkEntry,		GTK_ENTRY,		helipad_width,		"changed", 	on_width_changed,			NULL)	\
    _____(GtkEntry,		GTK_ENTRY,		helipad_recession,	"changed", 	on_helipad_recession_changed,		NULL)	\
    _____(GtkEntry,		GTK_ENTRY,		helipad_label,		"changed", 	on_helipad_label_changed,		NULL)	\
    _____(GtkCheckButton,	GTK_CHECK_BUTTON,	helipad_has_lighting,	"toggled", 	on_helipad_lighting_toggled,		NULL)	\
    _____(GtkCheckButton,	GTK_CHECK_BUTTON,	helipad_has_fuel,	"toggled", 	on_helipad_fuel_toggled,		NULL)	\
    _____(GtkCheckButton,	GTK_CHECK_BUTTON,	helipad_has_repair,	"toggled", 	on_helipad_repair_toggled,		NULL)	\
    _____(GtkCheckButton,	GTK_CHECK_BUTTON,	helipad_has_drop_off,	"toggled", 	on_helipad_drop_off_toggled,		NULL)	\
    _____(GtkCheckButton,	GTK_CHECK_BUTTON,	helipad_is_restarting,	"toggled", 	on_helipad_is_restarting_toggled,	NULL)	\
    _____(GtkCheckButton,	GTK_CHECK_BUTTON,	helipad_is_referenced,	"toggled", 	on_helipad_is_referenced_toggled,	NULL)	\
    _____(GtkLabel,		GTK_LABEL,		helipad_ref_obj_label,	NULL, 		NULL,					NULL)	\
    _____(GtkComboBoxText,	GTK_COMBO_BOX_TEXT,	helipad_ref_obj_name,	"changed", 	on_helipad_ref_obj_changed,		NULL)	\
    _____(GtkLabel,		GTK_LABEL,		helipad_offset_pos_label,NULL, 		NULL,					NULL)	\
    _____(GtkEntry,		GTK_ENTRY,		helipad_offset_pos_x,	"changed", 	on_offset_pos_x_changed,		NULL)	\
    _____(GtkEntry,		GTK_ENTRY,		helipad_offset_pos_y,	"changed", 	on_offset_pos_y_changed,		NULL)	\
    _____(GtkEntry,		GTK_ENTRY,		helipad_offset_pos_z,	"changed", 	on_offset_pos_z_changed,		NULL)	\
    _____(GtkLabel,		GTK_LABEL,		helipad_offset_dir_label,NULL, 		NULL,					NULL)	\
    _____(GtkEntry,		GTK_ENTRY,		helipad_offset_dir_heading,"changed", 	on_offset_dir_heading_changed,		NULL)	\
    _____(GtkEntry,		GTK_ENTRY,		helipad_offset_dir_pitch,"changed", 	on_offset_dir_pitch_changed,		NULL)	\
    _____(GtkEntry,		GTK_ENTRY,		helipad_offset_dir_bank,"changed", 	on_offset_dir_bank_changed,		NULL)	\
    _____(GtkComboBoxText,	GTK_COMBO_BOX_TEXT,	human_type_name,	"changed", 	on_human_type_name_changed,		NULL)	\
    _____(GtkCheckButton,	GTK_CHECK_BUTTON,	human_need_rescue,	"toggled", 	on_human_need_rescue_toggled,		NULL)	\
    _____(GtkCheckButton,	GTK_CHECK_BUTTON,	human_sit_up,		"toggled", 	on_human_sit_up_toggled,		NULL)	\
    _____(GtkCheckButton,	GTK_CHECK_BUTTON,	human_sit_down,		"toggled", 	on_human_sit_down_toggled,		NULL)	\
    _____(GtkCheckButton,	GTK_CHECK_BUTTON,	human_sitting,		"toggled", 	on_human_sitting_toggled,		NULL)	\
    _____(GtkCheckButton,	GTK_CHECK_BUTTON,	human_lying,		"toggled", 	on_human_lying_toggled,			NULL)	\
    _____(GtkCheckButton,	GTK_CHECK_BUTTON,	human_alert,		"toggled", 	on_human_alert_toggled,			NULL)	\
    _____(GtkCheckButton,	GTK_CHECK_BUTTON,	human_aware,		"toggled", 	on_human_aware_toggled,			NULL)	\
    _____(GtkCheckButton,	GTK_CHECK_BUTTON,	human_in_water,		"toggled", 	on_human_human_in_water_toggled,	NULL)	\
    _____(GtkCheckButton,	GTK_CHECK_BUTTON,	human_on_stretcher,	"toggled", 	on_human_on_stretcher_toggled,		NULL)	\
    _____(GtkSpinButton,	GTK_SPIN_BUTTON,	human_assistants,	"changed", 	on_human_assistants_changed,		NULL)	\
    _____(GtkLabel,		GTK_LABEL,		human_assist_1_name_label,NULL, 	NULL,					NULL)	\
    _____(GtkComboBoxText,	GTK_COMBO_BOX_TEXT,	human_assist_1_name,	"changed", 	on_human_assist_1_name_changed,		NULL)	\
    _____(GtkLabel,		GTK_LABEL,		human_assist_2_name_label,NULL, 	NULL,					NULL)	\
    _____(GtkComboBoxText,	GTK_COMBO_BOX_TEXT,	human_assist_2_name,	"changed", 	on_human_assist_2_name_changed,		NULL)	\
    _____(GtkLabel,		GTK_LABEL,		human_assist_3_name_label,NULL, 	NULL,					NULL)	\
    _____(GtkComboBoxText,	GTK_COMBO_BOX_TEXT,	human_assist_3_name,	"changed", 	on_human_assist_3_name_changed,		NULL)	\
    _____(GtkLabel,		GTK_LABEL,		human_assist_4_name_label,NULL, 	NULL,					NULL)	\
    _____(GtkComboBoxText,	GTK_COMBO_BOX_TEXT,	human_assist_4_name,	"changed", 	on_human_assist_4_name_changed,		NULL)	\
    _____(GtkCheckButton,	GTK_CHECK_BUTTON,	human_has_displacement,	"toggled", 	on_human_has_displacement_toggled,	NULL)	\
    _____(GtkLabel,		GTK_LABEL,		human_ref_obj_name_label,NULL,	 	NULL,					NULL)	\
    _____(GtkComboBoxText,	GTK_COMBO_BOX_TEXT,	human_ref_obj_name,	"changed", 	on_human_ref_obj_changed,		NULL)	\
    _____(GtkLabel,		GTK_LABEL,		human_displacement_label,NULL, 		NULL,					NULL)	\
    _____(GtkComboBoxText,	GTK_COMBO_BOX_TEXT,	human_displacement,	"changed", 	on_human_displacement_changed,		NULL)	\
    _____(GtkFileChooser,	GTK_FILE_CHOOSER,	model_file_name,	"selection-changed", on_model_file_name_changed,	NULL)	\
    _____(GtkEntry,		GTK_ENTRY,		model_name,		NULL, 		NULL,					NULL)	\
    _____(GtkComboBoxText,	GTK_COMBO_BOX_TEXT,	premod_type,		"changed", 	on_premod_type_changed,			NULL)	\
    _____(GtkEntry,		GTK_ENTRY,		premod_range,		"changed", 	on_range_changed,			NULL)	\
    _____(GtkLabel,		GTK_LABEL,		premod_length_label,	NULL, 		NULL,					NULL)	\
    _____(GtkEntry,		GTK_ENTRY,		premod_length,		"changed", 	on_length_changed,			NULL)	\
    _____(GtkLabel,		GTK_LABEL,		premod_width_label,	NULL, 		NULL,					NULL)	\
    _____(GtkEntry,		GTK_ENTRY,		premod_width,		"changed", 	on_width_changed,			NULL)	\
    _____(GtkEntry,		GTK_ENTRY,		premod_height,		"changed", 	on_height_changed,			NULL)	\
    _____(GtkLabel,		GTK_LABEL,		premod_hazard_label,	NULL, 		NULL,					NULL)	\
    _____(GtkEntry,		GTK_ENTRY,		premod_hazard,		"changed", 	on_premod_hazard_lights_changed,	NULL)	\
    _____(GtkLabel,		GTK_LABEL,		premod_walls_tex_label,	NULL, 		NULL,					NULL)	\
    _____(GtkComboBoxText,	GTK_COMBO_BOX_TEXT,	premod_walls_tex,	"changed", 	on_premod_walls_tex_changed,		NULL)	\
    _____(GtkLabel,		GTK_LABEL,		premod_walls_tex_night_label,NULL, 	NULL,					NULL)	\
    _____(GtkComboBoxText,	GTK_COMBO_BOX_TEXT,	premod_walls_tex_night,	"changed", 	on_premod_walls_tex_night_changed,	NULL)	\
    _____(GtkLabel,		GTK_LABEL,		premod_roof_tex_label,	NULL, 		NULL,					NULL)	\
    _____(GtkComboBoxText,	GTK_COMBO_BOX_TEXT,	premod_roof_tex,	"changed", 	on_premod_roof_tex_changed,		NULL)	\
    _____(GtkEntry,		GTK_ENTRY,		runway_range,		"changed", 	on_range_changed,			NULL)	\
    _____(GtkEntry,		GTK_ENTRY,		runway_length,		"changed", 	on_length_changed,			NULL)	\
    _____(GtkEntry,		GTK_ENTRY,		runway_width,		"changed", 	on_width_changed,			NULL)	\
    _____(GtkComboBoxText,	GTK_COMBO_BOX_TEXT,	runway_surface_type,	"changed", 	on_runway_surface_type_changed,		NULL)	\
    _____(GtkEntry,		GTK_ENTRY,		runway_dashes_num,	"changed", 	on_runway_dashes_num_changed,		NULL)	\
    _____(GtkEntry,		GTK_ENTRY,		runway_edge_light_spacing,"changed", 	on_runway_edge_light_spacing_changed,	NULL)	\
    _____(GtkEntry,		GTK_ENTRY,		runway_north_label,	"changed", 	on_runway_north_label_changed,		NULL)	\
    _____(GtkEntry,		GTK_ENTRY,		runway_south_label,	"changed", 	on_runway_south_label_changed,		NULL)	\
    _____(GtkEntry,		GTK_ENTRY,		runway_n_disp_thresh,	"changed", 	on_runway_n_disp_thresh_changed,	NULL)	\
    _____(GtkEntry,		GTK_ENTRY,		runway_s_disp_thresh,	"changed", 	on_runway_s_disp_thresh_changed,	NULL)	\
    _____(GtkCheckButton,	GTK_CHECK_BUTTON,	runway_has_thresholds,	"toggled", 	on_runway_has_thresholds_toggled,	NULL)	\
    _____(GtkCheckButton,	GTK_CHECK_BUTTON,	runway_has_borders,	"toggled", 	on_runway_has_borders_toggled,		NULL)	\
    _____(GtkCheckButton,	GTK_CHECK_BUTTON,	runway_has_td_markers,	"toggled", 	on_runway_has_td_markers_toggled,	NULL)	\
    _____(GtkCheckButton,	GTK_CHECK_BUTTON,	runway_has_mid_markers,	"toggled", 	on_runway_has_mid_markers_toggled,	NULL)	\
    _____(GtkCheckButton,	GTK_CHECK_BUTTON,	runway_has_north_gs,	"toggled", 	on_runway_has_north_gs_toggled,		NULL)	\
    _____(GtkCheckButton,	GTK_CHECK_BUTTON,	runway_has_south_gs,	"toggled", 	on_runway_has_south_gs_toggled,		NULL)	\
    _____(GtkEntry,		GTK_ENTRY,		smoke_offset_pos_x,	"changed", 	on_offset_pos_x_changed,		NULL)	\
    _____(GtkEntry,		GTK_ENTRY,		smoke_offset_pos_y,	"changed", 	on_offset_pos_y_changed,		NULL)	\
    _____(GtkEntry,		GTK_ENTRY,		smoke_offset_pos_z,	"changed", 	on_offset_pos_z_changed,		NULL)	\
    _____(GtkEntry,		GTK_ENTRY,		smoke_radius_start,	"changed", 	on_smoke_radius_start_changed,		NULL)	\
    _____(GtkEntry,		GTK_ENTRY,		smoke_radius_max,	"changed", 	on_smoke_radius_max_changed,		NULL)	\
    _____(GtkEntry,		GTK_ENTRY,		smoke_radius_rate,	"changed", 	on_smoke_radius_rate_changed,		NULL)	\
    _____(GtkEntry,		GTK_ENTRY,		smoke_hide_at_max,	"changed", 	on_smoke_hide_at_max_changed,		NULL)	\
    _____(GtkEntry,		GTK_ENTRY,		smoke_respawn_int,	"changed", 	on_smoke_respawn_int_changed,		NULL)	\
    _____(GtkEntry,		GTK_ENTRY,		smoke_total_units,	"changed", 	on_smoke_total_units_changed,		NULL)	\
    _____(GtkComboBoxText,	GTK_COMBO_BOX_TEXT,	smoke_color_code,	"changed", 	on_smoke_color_code_changed,		NULL)	\
    _____(GtkEntry,		GTK_ENTRY,		ref_object_pos_x,	"changed", 	on_ref_object_pos_x_changed,		NULL)	\
    _____(GtkEntry,		GTK_ENTRY,		ref_object_pos_y,	"changed", 	on_ref_object_pos_y_changed,		NULL)	\
    _____(GtkEntry,		GTK_ENTRY,		ref_object_pos_z,	"changed", 	on_ref_object_pos_z_changed,		NULL)	\
    _____(GtkEntry,		GTK_ENTRY,		ref_object_dir_heading,	"changed", 	on_ref_object_dir_heading_changed,	NULL)	\
    _____(GtkEntry,		GTK_ENTRY,		ref_object_dir_pitch,	"changed", 	on_ref_object_dir_pitch_changed,	NULL)	\
    _____(GtkEntry,		GTK_ENTRY,		ref_object_dir_bank,	"changed", 	on_ref_object_dir_bank_changed,		NULL)

    /*
     * Quit without print window
     */
#define editor_quit_without_print_widgets_gtk3_LIST(_____)											\
/* Please, make sure that each [WidgetID] is contained in editorquitwithoutprintgtk3.ui! */							\
    _____(GtkWidget,		GTK_WIDGET,		window,			"delete_event", gtk_widget_hide_on_delete,		NULL)	\
    _____(GtkLabel,		GTK_LABEL,		label,			NULL,	 	NULL,					NULL)	\
    _____(GtkButton,		GTK_BUTTON,		button_yes,		"clicked", 	on_button_quit_without_print_yes_clicked,NULL)	\
    _____(GtkButton,		GTK_BUTTON,		button_no,		"clicked", 	on_button_quit_without_print_no_clicked,NULL)


/*
 *							**** GTK4 UI version *****
 */
    /*
     * Menu buttons window. Please make sure that each [WidgetID] is contained in editormenugtk4.ui!
     */
#define editor_menu_widgets_gtk4_LIST(_____)													\
    _____(GtkWidget,		GTK_WIDGET,		window,			"destroy", 	on_menu_window_destroy,			NULL)	\
    _____(GtkGrid,		GTK_GRID,		grid,			NULL,	 	NULL,					NULL)	\
    _____(GtkButton,		GTK_BUTTON,		button_print,		"clicked", 	on_button_print_clicked,		NULL)	\
    _____(GtkButton,		GTK_BUTTON,		button_quit,		"clicked", 	on_button_quit_clicked,			NULL)	\
    _____(GtkButton,		GTK_BUTTON,		button_object_new,	"clicked", 	on_button_new_clicked,			NULL)	\
    _____(GtkButton,		GTK_BUTTON,		button_object_set,	"clicked", 	on_button_set_clicked,			NULL)	\
    _____(GtkButton,		GTK_BUTTON,		button_object_copy,	"clicked", 	on_button_copy_clicked,			NULL)	\
    _____(GtkButton,		GTK_BUTTON,		button_object_info,	"clicked", 	on_button_info_clicked,			NULL)	\
    _____(GtkButton,		GTK_BUTTON,		button_object_info_next,"clicked", 	on_button_info_next_clicked,		NULL)	\
    _____(GtkButton,		GTK_BUTTON,		button_object_unload,	"clicked", 	on_button_unload_clicked,		NULL)	\
    _____(GtkButton,		GTK_BUTTON,		button_object_modify,	"clicked", 	on_button_modify_clicked,		NULL)	\
    _____(GtkButton,		GTK_BUTTON,		button_object_move,	"clicked", 	on_button_move_clicked,			NULL)	\
    _____(GtkButton,		GTK_BUTTON,		button_object_remove,	"clicked", 	on_button_remove_clicked,		NULL)

    /*
     * Object data window. Please make sure that each [WidgetID] is contained in editordatagtk4.ui!
     */
#define editor_data_widgets_gtk4_LIST(_____)													\
/* Please, make sure that each [WidgetID] is contained in editorgtk*.ui! */									\
    _____(GtkWidget,		GTK_WIDGET,		window,	"destroy", 	on_data_window_destroy,					NULL)	\
    _____(GtkWidget,		GTK_WIDGET,		scrolled_window,	NULL,	 	NULL,					NULL)	\
    _____(GtkBox,		GTK_BOX,		main_box,		NULL,	 	NULL,					NULL)	\
    _____(GtkButton,		GTK_BUTTON,		button_ok,		"clicked", 	on_button_ok_clicked,			NULL)	\
    _____(GtkButton,		GTK_BUTTON,		button_apply,		"clicked", 	on_button_apply_clicked,		NULL)	\
    _____(GtkButton,		GTK_BUTTON,		button_cancel,		"clicked", 	on_button_cancel_clicked,		NULL)	\
    /*																		\
     * Frame widgets. These widgets have to be referenced because they will be shown or hidden as needed, depending of object type.		\
     */																		\
    _____(GtkFrame,		GTK_FRAME,		general_frame,		NULL, 		NULL,					NULL)	\
    _____(GtkGrid,		GTK_GRID,		general_frame_grid,	NULL, 		NULL,					NULL)	\
    _____(GtkFrame,		GTK_FRAME,		fire_frame,		NULL, 		NULL,					NULL)	\
    _____(GtkGrid,		GTK_GRID,		fire_frame_grid,	NULL, 		NULL,					NULL)	\
    _____(GtkFrame,		GTK_FRAME,		helipad_frame,		NULL, 		NULL,					NULL)	\
    _____(GtkGrid,		GTK_GRID,		helipad_frame_grid,	NULL, 		NULL,					NULL)	\
    _____(GtkFrame,		GTK_FRAME,		ref_object_frame,	NULL,	 	NULL,					NULL)	\
    _____(GtkFrame,		GTK_FRAME,		human_frame,		NULL, 		NULL,					NULL)	\
    _____(GtkGrid,		GTK_GRID,		human_frame_grid,	NULL, 		NULL,					NULL)	\
    _____(GtkFrame,		GTK_FRAME,		model_frame,		NULL, 		NULL,					NULL)	\
    _____(GtkGrid,		GTK_GRID,		model_frame_grid,	NULL, 		NULL,					NULL)	\
    _____(GtkFrame,		GTK_FRAME,		premodeled_frame,	NULL, 		NULL,					NULL)	\
    _____(GtkGrid,		GTK_GRID,		premodeled_frame_grid,	NULL, 		NULL,					NULL)	\
    _____(GtkFrame,		GTK_FRAME,		runway_frame,		NULL, 		NULL,					NULL)	\
    _____(GtkGrid,		GTK_GRID,		runway_frame_grid,	NULL, 		NULL,					NULL)	\
    _____(GtkFrame,		GTK_FRAME,		smoke_frame,		NULL, 		NULL,					NULL)	\
    _____(GtkGrid,		GTK_GRID,		smoke_frame_grid,	NULL, 		NULL,					NULL)	\
    /*																		\
     * Most of the "data entry" widgets are connected to an on_* callback function. A same callback can be called by several widgets.		\
     * Label widgets are not connected to callbacks, and only some of them appears in this list.						\
     * As all hereunder widgets are referenced, they can be easily shown/hidden/enabled/disabled/and so on... separatly as needed.		\
     */																		\
    _____(GtkDropDown,		GTK_DROP_DOWN,		general_type,		"notify::selected-item", on_general_type_changed,	NULL)	\
    _____(GtkLabel,		GTK_LABEL,		general_object_name_label,NULL, 	NULL,					NULL)	\
    _____(GtkEntry,		GTK_ENTRY,		general_object_name,	"changed", 	on_obj_name_changed,			NULL)	\
    _____(GtkLabel,		GTK_LABEL,		general_pos_label,	NULL, 		NULL,					NULL)	\
    _____(GtkEntry,		GTK_ENTRY,		general_pos_x,		"changed", 	on_pos_x_changed,			NULL)	\
    _____(GtkEntry,		GTK_ENTRY,		general_pos_y,		"changed", 	on_pos_y_changed,			NULL)	\
    _____(GtkEntry,		GTK_ENTRY,		general_pos_z,		"changed", 	on_pos_z_changed,			NULL)	\
    _____(GtkLabel,		GTK_LABEL,		general_dir_label,	NULL, 		NULL,					NULL)	\
    _____(GtkEntry,		GTK_ENTRY,		general_dir_heading,	"changed", 	on_dir_heading_changed,			NULL)	\
    _____(GtkEntry,		GTK_ENTRY,		general_dir_pitch,	"changed", 	on_dir_pitch_changed,			NULL)	\
    _____(GtkEntry,		GTK_ENTRY,		general_dir_bank,	"changed", 	on_dir_bank_changed,			NULL)	\
    _____(GtkEntry,		GTK_ENTRY,		fire_radius,		"changed", 	on_fire_radius_changed,			NULL)	\
    _____(GtkEntry,		GTK_ENTRY,		fire_height,		"changed", 	on_height_changed,			NULL)	\
    _____(GtkDropDown,		GTK_DROP_DOWN,		helipad_style,		"notify::selected-item", on_helipad_style_changed,	NULL)	\
    _____(GtkEntry,		GTK_ENTRY,		helipad_length,		"changed", 	on_length_changed,			NULL)	\
    _____(GtkEntry,		GTK_ENTRY,		helipad_width,		"changed", 	on_width_changed,			NULL)	\
    _____(GtkEntry,		GTK_ENTRY,		helipad_recession,	"changed", 	on_helipad_recession_changed,		NULL)	\
    _____(GtkEntry,		GTK_ENTRY,		helipad_label,		"changed", 	on_helipad_label_changed,		NULL)	\
    _____(GtkCheckButton,	GTK_CHECK_BUTTON,	helipad_has_lighting,	"toggled", 	on_helipad_lighting_toggled,		NULL)	\
    _____(GtkCheckButton,	GTK_CHECK_BUTTON,	helipad_has_fuel,	"toggled", 	on_helipad_fuel_toggled,		NULL)	\
    _____(GtkCheckButton,	GTK_CHECK_BUTTON,	helipad_has_repair,	"toggled", 	on_helipad_repair_toggled,		NULL)	\
    _____(GtkCheckButton,	GTK_CHECK_BUTTON,	helipad_has_drop_off,	"toggled", 	on_helipad_drop_off_toggled,		NULL)	\
    _____(GtkCheckButton,	GTK_CHECK_BUTTON,	helipad_is_restarting,	"toggled", 	on_helipad_is_restarting_toggled,	NULL)	\
    _____(GtkCheckButton,	GTK_CHECK_BUTTON,	helipad_is_referenced,	"toggled", 	on_helipad_is_referenced_toggled,	NULL)	\
    _____(GtkLabel,		GTK_LABEL,		helipad_ref_obj_label,	NULL, 		NULL,					NULL)	\
    _____(GtkDropDown,		GTK_DROP_DOWN,		helipad_ref_obj_name,	"notify::selected-item", on_helipad_ref_obj_changed,	NULL)	\
    _____(GtkLabel,		GTK_LABEL,		helipad_offset_pos_label,NULL, 		NULL,					NULL)	\
    _____(GtkEntry,		GTK_ENTRY,		helipad_offset_pos_x,	"changed", 	on_offset_pos_x_changed,		NULL)	\
    _____(GtkEntry,		GTK_ENTRY,		helipad_offset_pos_y,	"changed", 	on_offset_pos_y_changed,		NULL)	\
    _____(GtkEntry,		GTK_ENTRY,		helipad_offset_pos_z,	"changed", 	on_offset_pos_z_changed,		NULL)	\
    _____(GtkLabel,		GTK_LABEL,		helipad_offset_dir_label,NULL, 		NULL,					NULL)	\
    _____(GtkEntry,		GTK_ENTRY,		helipad_offset_dir_heading,"changed", 	on_offset_dir_heading_changed,		NULL)	\
    _____(GtkEntry,		GTK_ENTRY,		helipad_offset_dir_pitch,"changed", 	on_offset_dir_pitch_changed,		NULL)	\
    _____(GtkEntry,		GTK_ENTRY,		helipad_offset_dir_bank,"changed", 	on_offset_dir_bank_changed,		NULL)	\
    _____(GtkDropDown,		GTK_DROP_DOWN,		human_type_name,	"notify::selected-item", on_human_type_name_changed,	NULL)	\
    _____(GtkCheckButton,	GTK_CHECK_BUTTON,	human_need_rescue,	"toggled", 	on_human_need_rescue_toggled,		NULL)	\
    _____(GtkCheckButton,	GTK_CHECK_BUTTON,	human_sit_up,		"toggled", 	on_human_sit_up_toggled,		NULL)	\
    _____(GtkCheckButton,	GTK_CHECK_BUTTON,	human_sit_down,		"toggled", 	on_human_sit_down_toggled,		NULL)	\
    _____(GtkCheckButton,	GTK_CHECK_BUTTON,	human_sitting,		"toggled", 	on_human_sitting_toggled,		NULL)	\
    _____(GtkCheckButton,	GTK_CHECK_BUTTON,	human_lying,		"toggled", 	on_human_lying_toggled,			NULL)	\
    _____(GtkCheckButton,	GTK_CHECK_BUTTON,	human_alert,		"toggled", 	on_human_alert_toggled,			NULL)	\
    _____(GtkCheckButton,	GTK_CHECK_BUTTON,	human_aware,		"toggled", 	on_human_aware_toggled,			NULL)	\
    _____(GtkCheckButton,	GTK_CHECK_BUTTON,	human_in_water,		"toggled", 	on_human_human_in_water_toggled,	NULL)	\
    _____(GtkCheckButton,	GTK_CHECK_BUTTON,	human_on_stretcher,	"toggled", 	on_human_on_stretcher_toggled,		NULL)	\
    _____(GtkSpinButton,	GTK_SPIN_BUTTON,	human_assistants,	"changed", 	on_human_assistants_changed,		NULL)	\
    _____(GtkLabel,		GTK_LABEL,		human_assist_1_name_label,NULL, 	NULL,					NULL)	\
    _____(GtkDropDown,		GTK_DROP_DOWN,		human_assist_1_name,	"notify::selected-item", on_human_assist_1_name_changed,NULL)	\
    _____(GtkLabel,		GTK_LABEL,		human_assist_2_name_label,NULL, 	NULL,					NULL)	\
    _____(GtkDropDown,		GTK_DROP_DOWN,		human_assist_2_name,	"notify::selected-item", on_human_assist_2_name_changed,NULL)	\
    _____(GtkLabel,		GTK_LABEL,		human_assist_3_name_label,NULL, 	NULL,					NULL)	\
    _____(GtkDropDown,		GTK_DROP_DOWN,		human_assist_3_name,	"notify::selected-item", on_human_assist_3_name_changed,NULL)	\
    _____(GtkLabel,		GTK_LABEL,		human_assist_4_name_label,NULL, 	NULL,					NULL)	\
    _____(GtkDropDown,		GTK_DROP_DOWN,		human_assist_4_name,	"notify::selected-item", on_human_assist_4_name_changed,NULL)	\
    _____(GtkCheckButton,	GTK_CHECK_BUTTON,	human_has_displacement,	"toggled", 	on_human_has_displacement_toggled,	NULL)	\
    _____(GtkLabel,		GTK_LABEL,		human_ref_obj_name_label,NULL,	 	NULL,					NULL)	\
    _____(GtkDropDown,		GTK_DROP_DOWN,		human_ref_obj_name,	"notify::selected-item", on_human_ref_obj_changed,	NULL)	\
    _____(GtkLabel,		GTK_LABEL,		human_displacement_label,NULL, 		NULL,					NULL)	\
    _____(GtkDropDown,		GTK_DROP_DOWN,		human_displacement,	"notify::selected-item", on_human_displacement_changed,	NULL)	\
    _____(GtkButton,		GTK_BUTTON,		model_file_name_btn,	"clicked",	on_model_file_name_clicked,		NULL)	\
    _____(GtkLabel,		GTK_LABEL,		model_file_name_hidden,	NULL,	 	NULL,					NULL)	\
    _____(GtkLabel,		GTK_LABEL,		model_file_name_label,	NULL,	 	NULL,					NULL)	\
    _____(GtkEntry,		GTK_ENTRY,		model_name,		NULL, 		NULL,					NULL)	\
    _____(GtkDropDown,		GTK_DROP_DOWN,		premod_type,		"notify::selected-item", on_premod_type_changed,	NULL)	\
    _____(GtkEntry,		GTK_ENTRY,		premod_range,		"changed", 	on_range_changed,			NULL)	\
    _____(GtkLabel,		GTK_LABEL,		premod_length_label,	NULL, 		NULL,					NULL)	\
    _____(GtkEntry,		GTK_ENTRY,		premod_length,		"changed", 	on_length_changed,			NULL)	\
    _____(GtkLabel,		GTK_LABEL,		premod_width_label,	NULL, 		NULL,					NULL)	\
    _____(GtkEntry,		GTK_ENTRY,		premod_width,		"changed", 	on_width_changed,			NULL)	\
    _____(GtkEntry,		GTK_ENTRY,		premod_height,		"changed", 	on_height_changed,			NULL)	\
    _____(GtkLabel,		GTK_LABEL,		premod_hazard_label,	NULL, 		NULL,					NULL)	\
    _____(GtkEntry,		GTK_ENTRY,		premod_hazard,		"changed", 	on_premod_hazard_lights_changed,	NULL)	\
    _____(GtkLabel,		GTK_LABEL,		premod_walls_tex_label,	NULL, 		NULL,					NULL)	\
    _____(GtkDropDown,		GTK_DROP_DOWN,		premod_walls_tex,	"notify::selected-item", on_premod_walls_tex_changed,	NULL)	\
    _____(GtkLabel,		GTK_LABEL,		premod_walls_tex_night_label,NULL, 	NULL,					NULL)	\
    _____(GtkDropDown,		GTK_DROP_DOWN,		premod_walls_tex_night,	"notify::selected-item", on_premod_walls_tex_night_changed,NULL)\
    _____(GtkLabel,		GTK_LABEL,		premod_roof_tex_label,	NULL, 		NULL,					NULL)	\
    _____(GtkDropDown,		GTK_DROP_DOWN,		premod_roof_tex,	"notify::selected-item", on_premod_roof_tex_changed,	NULL)	\
    _____(GtkEntry,		GTK_ENTRY,		runway_range,		"changed", 	on_range_changed,			NULL)	\
    _____(GtkEntry,		GTK_ENTRY,		runway_length,		"changed", 	on_length_changed,			NULL)	\
    _____(GtkEntry,		GTK_ENTRY,		runway_width,		"changed", 	on_width_changed,			NULL)	\
    _____(GtkDropDown,		GTK_DROP_DOWN,		runway_surface_type,	"notify::selected-item", on_runway_surface_type_changed,NULL)	\
    _____(GtkEntry,		GTK_ENTRY,		runway_dashes_num,	"changed", 	on_runway_dashes_num_changed,		NULL)	\
    _____(GtkEntry,		GTK_ENTRY,		runway_edge_light_spacing,"changed", 	on_runway_edge_light_spacing_changed,	NULL)	\
    _____(GtkEntry,		GTK_ENTRY,		runway_north_label,	"changed", 	on_runway_north_label_changed,		NULL)	\
    _____(GtkEntry,		GTK_ENTRY,		runway_south_label,	"changed", 	on_runway_south_label_changed,		NULL)	\
    _____(GtkEntry,		GTK_ENTRY,		runway_n_disp_thresh,	"changed", 	on_runway_n_disp_thresh_changed,	NULL)	\
    _____(GtkEntry,		GTK_ENTRY,		runway_s_disp_thresh,	"changed", 	on_runway_s_disp_thresh_changed,	NULL)	\
    _____(GtkCheckButton,	GTK_CHECK_BUTTON,	runway_has_thresholds,	"toggled", 	on_runway_has_thresholds_toggled,	NULL)	\
    _____(GtkCheckButton,	GTK_CHECK_BUTTON,	runway_has_borders,	"toggled", 	on_runway_has_borders_toggled,		NULL)	\
    _____(GtkCheckButton,	GTK_CHECK_BUTTON,	runway_has_td_markers,	"toggled", 	on_runway_has_td_markers_toggled,	NULL)	\
    _____(GtkCheckButton,	GTK_CHECK_BUTTON,	runway_has_mid_markers,	"toggled", 	on_runway_has_mid_markers_toggled,	NULL)	\
    _____(GtkCheckButton,	GTK_CHECK_BUTTON,	runway_has_north_gs,	"toggled", 	on_runway_has_north_gs_toggled,		NULL)	\
    _____(GtkCheckButton,	GTK_CHECK_BUTTON,	runway_has_south_gs,	"toggled", 	on_runway_has_south_gs_toggled,		NULL)	\
    _____(GtkEntry,		GTK_ENTRY,		smoke_offset_pos_x,	"changed", 	on_offset_pos_x_changed,		NULL)	\
    _____(GtkEntry,		GTK_ENTRY,		smoke_offset_pos_y,	"changed", 	on_offset_pos_y_changed,		NULL)	\
    _____(GtkEntry,		GTK_ENTRY,		smoke_offset_pos_z,	"changed", 	on_offset_pos_z_changed,		NULL)	\
    _____(GtkEntry,		GTK_ENTRY,		smoke_radius_start,	"changed", 	on_smoke_radius_start_changed,		NULL)	\
    _____(GtkEntry,		GTK_ENTRY,		smoke_radius_max,	"changed", 	on_smoke_radius_max_changed,		NULL)	\
    _____(GtkEntry,		GTK_ENTRY,		smoke_radius_rate,	"changed", 	on_smoke_radius_rate_changed,		NULL)	\
    _____(GtkEntry,		GTK_ENTRY,		smoke_hide_at_max,	"changed", 	on_smoke_hide_at_max_changed,		NULL)	\
    _____(GtkEntry,		GTK_ENTRY,		smoke_respawn_int,	"changed", 	on_smoke_respawn_int_changed,		NULL)	\
    _____(GtkEntry,		GTK_ENTRY,		smoke_total_units,	"changed", 	on_smoke_total_units_changed,		NULL)	\
    _____(GtkDropDown,		GTK_DROP_DOWN,		smoke_color_code,	"notify::selected-item", on_smoke_color_code_changed,	NULL)	\
    _____(GtkEntry,		GTK_ENTRY,		ref_object_pos_x,	"changed", 	on_ref_object_pos_x_changed,		NULL)	\
    _____(GtkEntry,		GTK_ENTRY,		ref_object_pos_y,	"changed", 	on_ref_object_pos_y_changed,		NULL)	\
    _____(GtkEntry,		GTK_ENTRY,		ref_object_pos_z,	"changed", 	on_ref_object_pos_z_changed,		NULL)	\
    _____(GtkEntry,		GTK_ENTRY,		ref_object_dir_heading,	"changed", 	on_ref_object_dir_heading_changed,	NULL)	\
    _____(GtkEntry,		GTK_ENTRY,		ref_object_dir_pitch,	"changed", 	on_ref_object_dir_pitch_changed,	NULL)	\
    _____(GtkEntry,		GTK_ENTRY,		ref_object_dir_bank,	"changed", 	on_ref_object_dir_bank_changed,		NULL)

    /*
     * Quit without print window
     */
#define editor_quit_without_print_widgets_gtk4_LIST(_____)											\
/* Please, make sure that each [WidgetID] is contained in editorquitwithoutprintgtk4.ui! */							\
    _____(GtkWidget,		GTK_WIDGET,		window,			"destroy", 	on_dialog_window_destroy,		NULL)	\
    _____(GtkLabel,		GTK_LABEL,		label,			NULL,	 	NULL,					NULL)	\
    _____(GtkButton,		GTK_BUTTON,		button_yes,		"clicked", 	on_button_quit_without_print_yes_clicked,NULL)	\
    _____(GtkButton,		GTK_BUTTON,		button_no,		"clicked", 	on_button_quit_without_print_no_clicked,NULL)


/* Select the widgets lists according to the GTK version */
#if GTK_MAJOR_VERSION == 3 && GTK_MINOR_VERSION >= 24
    #define editor_data_widgets_LIST editor_data_widgets_gtk3_LIST
    #define editor_menu_widgets_LIST editor_menu_widgets_gtk3_LIST
    #define editor_quit_without_print_widgets_LIST editor_quit_without_print_widgets_gtk3_LIST
#endif
#if GTK_MAJOR_VERSION == 4 && GTK_MINOR_VERSION >= 10
    #define editor_data_widgets_LIST editor_data_widgets_gtk4_LIST
    #define editor_menu_widgets_LIST editor_menu_widgets_gtk4_LIST
    #define editor_quit_without_print_widgets_LIST editor_quit_without_print_widgets_gtk4_LIST
#endif

/* Private definitions for `editor_*_widgets` */
#define _AS_DECLARATION_(_W_TYPE_, _W_CAST_, _W_ID_, _W_SIGNAL_, _W_CALLBACKFUNC_, _W_TOOLTIP_) _W_TYPE_ * _W_ID_;
#define EDITOR_GTK_WIDGETS struct editor_data_widgets_t { editor_data_widgets_LIST(_AS_DECLARATION_) }
#define EDITOR_MENU_GTK_WIDGETS struct editor_menu_widgets_t { editor_menu_widgets_LIST(_AS_DECLARATION_) }
#define EDITOR_QUIT_WITHOUT_PRINT_GTK_WIDGETS struct editor_quit_without_print_widgets_t { editor_quit_without_print_widgets_LIST(_AS_DECLARATION_) }

/* The structures `editor_*_widgets` are the containers of all the widgets that we need */
EDITOR_GTK_WIDGETS editor_data_widgets;
EDITOR_MENU_GTK_WIDGETS editor_menu_widgets;
EDITOR_QUIT_WITHOUT_PRINT_GTK_WIDGETS editor_quit_without_print_widgets;

#undef EDITOR_QUIT_WITHOUT_PRINT_GTK_WIDGETS
#undef EDITOR_MENU_GTK_WIDGETS
#undef EDITOR_GTK_WIDGETS
#undef _AS_DECLARATION_


/*
 * Loads UI and fills the editor_menu_widgets structure.
 */
void app_startup_cb(GApplication *app, gpointer user_data)
{
    const sar_core_struct *core_ptr = (sar_core_struct *)user_data;
    GtkBuilder *builder;


    /***
     * Menu window
     ***/

    /* Get UI from file */
    builder = gtk_builder_new ();
    gtk_builder_add_from_resource(builder, "/./"EDITORMENUGTK_UI, NULL);

    /* Populate the editor_menu_widgets struct */
    #define _BUILDER_OBJ_ builder
    #define _AS_ASSIGNEMENT_(_W_TYPE_, _W_CAST_, _W_ID_, _W_SIGNAL_,  _W_CALLBACKFUNC_, _W_TOOLTIP_) \
	editor_menu_widgets._W_ID_ = _W_CAST_(gtk_builder_get_object(_BUILDER_OBJ_, #_W_ID_));\
	if (!editor_menu_widgets._W_ID_) { g_critical ("Widget \""#_W_ID_"\" is missing in file %s.", EDITORMENUGTK_UI); }\
	else {\
	    if ((_W_SIGNAL_) && (_W_CALLBACKFUNC_ != NULL)) { g_signal_connect (editor_menu_widgets._W_ID_, _W_SIGNAL_, G_CALLBACK(_W_CALLBACKFUNC_), (gpointer)core_ptr);}\
	    if(_W_TOOLTIP_ != NULL) { gtk_widget_set_tooltip_markup( GTK_WIDGET(editor_menu_widgets._W_ID_), _W_TOOLTIP_); }\
	}
	editor_menu_widgets_LIST(_AS_ASSIGNEMENT_);
    #undef _AS_ASSIGNEMENT_
    #undef _BUILDER_OBJ_

    /* Clear builder because it will be reused for the data window */
    g_clear_object(&builder);


    /***
     * Object data window
     ***/

    /* Get UI from file */
    builder = gtk_builder_new();
    gtk_builder_add_from_resource(builder, "/./"EDITORDATAGTK_UI, NULL);

    /* Populate the editor_data_widgets struct */
    #define _BUILDER_OBJ_ builder
    #define _AS_ASSIGNEMENT_(_W_TYPE_, _W_CAST_, _W_ID_, _W_SIGNAL_,  _W_CALLBACKFUNC_, _W_TOOLTIP_) \
	editor_data_widgets._W_ID_ = _W_CAST_(gtk_builder_get_object(_BUILDER_OBJ_, #_W_ID_));\
	if (!editor_data_widgets._W_ID_) { g_critical ("Widget \""#_W_ID_"\" is missing in file %s.", EDITORDATAGTK_UI); }\
	else {\
	    if ((_W_SIGNAL_) && (_W_CALLBACKFUNC_ != NULL)) { g_signal_connect (editor_data_widgets._W_ID_, _W_SIGNAL_, G_CALLBACK(_W_CALLBACKFUNC_), (gpointer)core_ptr);}\
	    if(_W_TOOLTIP_ != NULL) { gtk_widget_set_tooltip_markup( GTK_WIDGET(editor_data_widgets._W_ID_), _W_TOOLTIP_); }\
	}
	editor_data_widgets_LIST(_AS_ASSIGNEMENT_);
    #undef _AS_ASSIGNEMENT_
    #undef _BUILDER_OBJ_

    /* Clear builder because it will be reused for the dialog window */
    g_clear_object(&builder);


    /***
     * Dialog window
     ***/

    /* Get UI from file */
    builder = gtk_builder_new();
    gtk_builder_add_from_resource(builder, "/./"EDITORUITWITHOUTPRINTGTK_UI, NULL);

    /* Populate the editor_quit_without_print_widgets struct */
    #define _BUILDER_OBJ_ builder
    #define _AS_ASSIGNEMENT_(_W_TYPE_, _W_CAST_, _W_ID_, _W_SIGNAL_,  _W_CALLBACKFUNC_, _W_TOOLTIP_) \
	editor_quit_without_print_widgets._W_ID_ = _W_CAST_(gtk_builder_get_object(_BUILDER_OBJ_, #_W_ID_));\
	if (!editor_quit_without_print_widgets._W_ID_) { g_critical ("Widget \""#_W_ID_"\" is missing in file %s.", EDITORUITWITHOUTPRINTGTK_UI); }\
	else {\
	    if ((_W_SIGNAL_) && (_W_CALLBACKFUNC_ != NULL)) { g_signal_connect (editor_quit_without_print_widgets._W_ID_, _W_SIGNAL_, G_CALLBACK(_W_CALLBACKFUNC_), (gpointer)core_ptr);}\
	    if(_W_TOOLTIP_ != NULL) { gtk_widget_set_tooltip_markup( GTK_WIDGET(editor_quit_without_print_widgets._W_ID_), _W_TOOLTIP_); }\
	}
	editor_quit_without_print_widgets_LIST(_AS_ASSIGNEMENT_);
    #undef _AS_ASSIGNEMENT_
    #undef _BUILDER_OBJ_

    /* Unreference builder: we don't need it anymore */
    g_object_unref(builder);
}
#undef editor_quit_without_print_widgets_LIST
#undef editor_data_widgets_LIST
#undef editor_menu_widgets_LIST



void app_activate_cb(GApplication *app, gpointer user_data)
{
    const sar_core_struct *core_ptr = (sar_core_struct *)user_data;
    //const sar_scenery_editor_struct *scn_ed = core_ptr->in_game_editor;
    GtkWindow *menu_window, *data_window, *dialog_window;

    menu_window = GTK_WINDOW(editor_menu_widgets.window);
    data_window = GTK_WINDOW(editor_data_widgets.window);
    dialog_window = GTK_WINDOW(editor_quit_without_print_widgets.window);

    /* Get humans name from core then set them to human choice list widgets */
    EditorGtkItemChooserSetHumansNameList(core_ptr);

    /* Get objects name from core then set them to reference objects
     * choice list widgets.
     */
    EditorGtkItemChooserSetRefObjsNameList(core_ptr);

    /* Set human choice list widgets default value to first list item */
#if GTK_MAJOR_VERSION == 3 && GTK_MINOR_VERSION >= 24
    gtk_combo_box_set_active(GTK_COMBO_BOX(editor_data_widgets.human_type_name), 0);
#endif
#if GTK_MAJOR_VERSION == 4 && GTK_MINOR_VERSION >= 10
    gtk_drop_down_set_selected(editor_data_widgets.human_type_name, 0);
#endif

    /* Get textures name from core then set them to texture choice
     * list widgets.
     */
    EditorGtkItemChooserSetTexturesNameList(core_ptr);

    /* As there is no current object at startup, set the related
     * menu buttons insensitive.
     */
    set_children_sensitive_by_name(
	GTK_WIDGET(editor_menu_widgets.grid), "obj_current", FALSE
    );

    /* Hide the data window.
     * Note that it is very important to not destroy the data window during
     * the scenery editing because if it is destroyed the data contained in
     * the Gtk widgets are immediatly destroyed too, which can cause trouble.
     */
#if GTK_MAJOR_VERSION == 3 && GTK_MINOR_VERSION >= 24
    gtk_widget_hide_on_delete(editor_data_widgets.window);
    gtk_widget_hide(editor_data_widgets.window);
#endif
#if GTK_MAJOR_VERSION == 4 && GTK_MINOR_VERSION >= 10
    /////gtk_window_set_child(menu_window, editor_data_widgets.window);
    gtk_window_set_hide_on_close(data_window, TRUE);
    gtk_widget_set_visible(editor_data_widgets.window, FALSE);
#endif

#if GTK_MAJOR_VERSION == 3 && GTK_MINOR_VERSION >= 24
    gtk_widget_hide_on_delete(editor_quit_without_print_widgets.window);
    gtk_widget_hide(editor_quit_without_print_widgets.window);
#endif
#if GTK_MAJOR_VERSION == 4 && GTK_MINOR_VERSION >= 10
    gtk_window_set_hide_on_close(dialog_window, TRUE);
    gtk_widget_set_visible(editor_quit_without_print_widgets.window, FALSE);
#endif

    /* Set menu window to non-modal (non-blocking) */
    gtk_window_set_modal(menu_window, FALSE);

#if GTK_MAJOR_VERSION == 3 && GTK_MINOR_VERSION >= 24
    gtk_widget_hide_on_delete(editor_menu_widgets.window);
    /* Show the menu window */
    gtk_widget_show_all(GTK_WIDGET(menu_window));

    /* Force the menu window to always be on top.
     * Note: it works (at least) on my K Desktop Environment and I hope should
     * work for other X11 systems desktop environments.
     */
    gtk_window_set_keep_above(menu_window, TRUE);

#endif
#if GTK_MAJOR_VERSION == 4 && GTK_MINOR_VERSION >= 10
    gtk_window_set_hide_on_close(menu_window, TRUE);
    /* Show the menu window */
    gtk_window_present(menu_window);

    /* To force the menu window to always be on top, see:
     * https://discourse.gnome.org/t/gtk4-set-window-always-on-top-and-on-center-of-current-monitor/9068
     */
#endif




    /* FIXME For now, hide the data window "Apply" button. Is this button really usefull? */
    gtk_widget_set_sensitive(GTK_WIDGET(editor_data_widgets.button_apply), FALSE);




    /* Remove focus. This will allow user <Home>/<End> and other key presses to
     * be transmitted to the Sar2 main window instead of the menu window.
     */
    EditorGtkSetAcceptFocus(editor_menu_widgets.window, FALSE);

    /* Note that focus will be given back to the toplevel sar2 window
     * by calling the GwSetWindowFocusToSar2Window() function from the
     * SARCmdSceneEditor() function.
     */
}


/*
 * Returns the X11 window decoration (title bar) height.
 *
 * Returns 0 on error.
 */
int X11GetWindowDecorationHeight(Display *display, Window window)
{
    int decoration_height = -1, format, status;
    Atom frame_extents, type;
    long* extents;
    unsigned long nitems, bytes_after;

    frame_extents = XInternAtom(display, "_NET_FRAME_EXTENTS", True);
    status = XGetWindowProperty(display, window,
				frame_extents, 0, 4, False, AnyPropertyType,
				&type, &format, &nitems, &bytes_after,
				(unsigned char**)&extents
			    );

    if (status == Success && nitems == 4) {
	// int left = extents[0];
	// int right = extents[1];
	int top = extents[2];
	int bottom = extents[3];

	decoration_height = top + bottom;
    }
    else
	decoration_height = 0;

    return decoration_height;
}

/*
 * Gets the sar2 X11 window top right corner position
 */
void GwGetSar2WindowTopRightCornerPos(const gw_display_struct *display,
				      int *x, int *y
)
{
    Window sar2_x_window;
    int ctx_num, decoration_height;

    ctx_num = display->gl_context_num;

    /* Get the sar2 window X11 identifier */
    sar2_x_window = display->toplevel[ctx_num];

    /* Get the window decoration height */
    decoration_height = X11GetWindowDecorationHeight(display->display, sar2_x_window);

    *x = display->toplevel_geometry->x + display->toplevel_geometry->width;
    *y = display->toplevel_geometry->y - decoration_height;
}

/*
 * Sets the focus to the sar2 X11 window
 */
void GwSetWindowFocusToSar2Window(const gw_display_struct *display)
{
    Window sar2_x_window;
    int ctx_num;

    ctx_num = display->gl_context_num;

    /* Get the sar2 window X11 identifier */
    sar2_x_window = display->toplevel[ctx_num];

    XSetInputFocus(display->display, sar2_x_window, RevertToNone, CurrentTime);
}


/* TODO comment function */
int gtkAppStart(sar_core_struct *core_ptr, unsigned long flags)
{
#define APPLICATION_ID "com.github.SearchAndRescue2.sar2"
    const gw_display_struct *display = core_ptr->display;
    const sar_scenery_editor_struct *scn_ed = core_ptr->in_game_editor;
    editor_gtk_ui_struct *editor_gtk_ui = scn_ed->editor_gtk_ui;
    GtkApplication *app;
    GMainContext *context;
    //gboolean acquired_context;
    GError *error = NULL;

    /* Record command flags */
    editor_gtk_ui->cmd_flags = flags;

    app = gtk_application_new(APPLICATION_ID, G_APPLICATION_DEFAULT_FLAGS);

    // FIXME: freeze SarII if trying to re-enter editor GTK UI without quitting
    // then restarting SarII.
    context = g_main_context_ref_thread_default();
    //context = g_main_context_default();
    //context = g_main_context_get_thread_default();

    if(!g_main_context_acquire(context))
	return 1;

    /* Note: the ::startup signal will be emitted on success by
     * the g_application_register() function.
     */
    g_signal_connect(app, "startup", G_CALLBACK(app_startup_cb), (gpointer)core_ptr);
    if(!g_application_register(G_APPLICATION(app), NULL, &error))
    {
	g_printerr ("Failed to register: %s\n", error->message);
	g_error_free (error);
	return 1;
    }

    editor_gtk_ui->gtk_application = app;
    editor_gtk_ui->gtk_context = context;

    g_signal_connect(app, "activate", G_CALLBACK(app_activate_cb), (gpointer)core_ptr);
    g_application_activate(G_APPLICATION(app));
    //editor_gtk_ui->gtk_app_running = TRUE;
    core_ptr->gtk_main_loop_on = True;


    /*
     * Set the menu Gtk window top right corner at the same
     * position than the sar2 window top right corner.
     */

    /* Get the menu Gtk window size (from X11) */
    int menu_x_win_width, menu_x_win_height;
    XWindowAttributes attr;
#if GTK_MAJOR_VERSION == 3 && GTK_MINOR_VERSION >= 24
    GdkWindow *gdk_window = gtk_widget_get_window(editor_menu_widgets.window);
    Window menu_x_window = gdk_x11_window_get_xid(gdk_window);
    GdkDisplay *gdk_display = gdk_window_get_display(gdk_window);
    Display *menu_x_display = gdk_x11_display_get_xdisplay(gdk_display);
#endif
#if GTK_MAJOR_VERSION == 4 && GTK_MINOR_VERSION >= 10
#define NATIVEWINDOW GTK_NATIVE(GTK_WINDOW(editor_menu_widgets.window))
// Source: https://discourse.gnome.org/t/set-absolut-window-position-in-gtk4/8552/4
//    GdkSurface *native = gtk_native_get_surface(NATIVEWINDOW);
    Window menu_x_window = gdk_x11_surface_get_xid(GDK_SURFACE(NATIVEWINDOW));
    Display *menu_x_display = gdk_x11_display_get_xdisplay(gtk_widget_get_display(editor_menu_widgets.window));
#undef NATIVEWINDOW
#endif
    if (XGetWindowAttributes(menu_x_display, menu_x_window, &attr)) {
        menu_x_win_width = attr.width;
        menu_x_win_height = attr.height;
    }
    else
    {
        menu_x_win_width = 1;
        menu_x_win_height = 1;
    }

    /* Get the sar2 window top right corner position */
    int sar2_x_win_top_right_x, sar2_x_win_top_right_y;
    GwGetSar2WindowTopRightCornerPos(display,
				     &sar2_x_win_top_right_x,
				     &sar2_x_win_top_right_y
				);

    /* Set the menu Gtk window top left position (using an X11 function) */
    XMoveWindow(menu_x_display, menu_x_window,
		sar2_x_win_top_right_x - menu_x_win_width,
		sar2_x_win_top_right_y
	    );


    /* Set focus to the Sar2 main window. This will allow user
     * <Home>/<End> and other key press to be transmitted to the Sar2 main
     * window instead of to be "eaten" by the menu window.
     */
    GwSetWindowFocusToSar2Window(display);

    return 0;
#undef APPLICATION_ID
}

/*
 * Stops the GTK application then free the editor_gtk_ui structure.
 */
void gtkAppStop(sar_core_struct *core_ptr)
{
    const gw_display_struct *display = core_ptr->display;
    const sar_scenery_editor_struct *scn_ed;
    editor_gtk_ui_struct *editor_gtk_ui;
    GList *windows_list;
    GMainContext *context;

    if(!core_ptr->gtk_main_loop_on)
	return;

    scn_ed = core_ptr->in_game_editor;

    if(scn_ed != NULL)
	editor_gtk_ui = scn_ed->editor_gtk_ui;
    else
	return;

    if(editor_gtk_ui->gtk_application == NULL)
	return;

    /* Ensure that focus in on the Sar2 main window */
    GwSetWindowFocusToSar2Window(display);

    /* Get windows list */
    /* FIXME seems to return more objects than the opened windows!?! */
    windows_list = gtk_window_list_toplevels();

    /* Needed before destroying (see gtk_window_list_toplevels() doc) */
    g_list_foreach(windows_list, (GFunc)g_object_ref, NULL);

    /* Destroy the GTK windows */
    for( ; windows_list != NULL; windows_list = g_list_next(windows_list))
    {
	if(windows_list->data != NULL)
	{
#if GTK_MAJOR_VERSION == 3 && GTK_MINOR_VERSION >= 24
	    gtk_widget_destroy(windows_list->data);
#endif
#if GTK_MAJOR_VERSION == 4 && GTK_MINOR_VERSION >= 10
	    if(GTK_IS_WINDOW(windows_list->data))
		gtk_window_destroy(GTK_WINDOW(windows_list->data));
#endif
	}
    }

    /* Stop the SARManage() "GTK main loop" calls */
    core_ptr->gtk_main_loop_on = False;

    context = editor_gtk_ui->gtk_context;
    if(context != NULL)
    {
	GtkApplication *app = editor_gtk_ui->gtk_application;

	g_settings_sync();

	/* Clear pending events */
	while(g_main_context_iteration(context, FALSE))
	    ;

	g_object_unref(app);
	editor_gtk_ui->gtk_application = NULL;

	g_main_context_release(context);
	g_free(context);
	editor_gtk_ui->gtk_context = NULL;
    }

    free(editor_gtk_ui);

    return;
}



/*
 * GTK widgets utilities
 */

/* For convenience, get the GTK3/GTK4 check button activation state */
#if GTK_MAJOR_VERSION == 3 && GTK_MINOR_VERSION >= 24
#define GTK_CHECK_BUTTON_GET_ACTIVE(widget) gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget))
#endif
#if GTK_MAJOR_VERSION == 4 && GTK_MINOR_VERSION >= 10
#define GTK_CHECK_BUTTON_GET_ACTIVE(widget) gtk_check_button_get_active(widget)
#endif

/* For convenience, set the GTK3/GTK4 check button activation state */
#if GTK_MAJOR_VERSION == 3 && GTK_MINOR_VERSION >= 24
#define GTK_CHECK_BUTTON_SET_ACTIVE(widget, setting) gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget), setting)
#endif
#if GTK_MAJOR_VERSION == 4 && GTK_MINOR_VERSION >= 10
#define GTK_CHECK_BUTTON_SET_ACTIVE(widget, is_active) gtk_check_button_set_active(widget, is_active)
#endif

/* Set or unset the window focus */
void EditorGtkSetAcceptFocus(GtkWidget *window, gboolean state)
{
#if GTK_MAJOR_VERSION == 3 && GTK_MINOR_VERSION >= 24
    GdkWindow *gdk_window = gtk_widget_get_window(window);
    gdk_window_set_accept_focus(gdk_window, state);
#endif
#if GTK_MAJOR_VERSION == 4 && GTK_MINOR_VERSION >= 10
    fprintf(stderr, "\n%s:%d: FIXME: EditorGtkSetAcceptFocus() is not GTK4 compatible!\n", __FILE__, __LINE__);
#endif
}

/*
 * Set the humans name values to all the human choice lists
 */
void EditorGtkItemChooserSetHumansNameList(const sar_core_struct *core_ptr)
{
    sar_human_data_entry_struct *hd_entry;
    sar_human_data_struct *hd = core_ptr->human_data;
    int i, j = 0, total_presets = 0;
    char **human_array;

    total_presets = hd->total_presets;

    /* Add one preset more because last human_array[] pointer must be NULL */
    total_presets++;

    /* Alloc memory and set all human_array[] pointers to NULL */
    human_array = calloc(total_presets, sizeof(char *));

    for(i = 0; i <  hd->total_presets;)
    {
	hd_entry = hd->preset[i++];

	/* Let the "victim_streatcher_assisted" and "diver" presets left */
	if(!strcmp(hd_entry->name, "victim_streatcher_assisted") ||
	    !strcmp(hd_entry->name, "diver")
	)
	    continue;

	human_array[j++] = strdup(hd_entry->name);
    }

    /* Set humans type name in humans choice lists */
    EditorGtkItemChooserSetItemsList(GTK_WIDGET(editor_data_widgets.human_type_name), (const char* const*)human_array);
    EditorGtkItemChooserSetItemsList(GTK_WIDGET(editor_data_widgets.human_assist_1_name), (const char* const*)human_array);
    EditorGtkItemChooserSetItemsList(GTK_WIDGET(editor_data_widgets.human_assist_2_name), (const char* const*)human_array);
    EditorGtkItemChooserSetItemsList(GTK_WIDGET(editor_data_widgets.human_assist_3_name), (const char* const*)human_array);
    EditorGtkItemChooserSetItemsList(GTK_WIDGET(editor_data_widgets.human_assist_4_name), (const char* const*)human_array);

    /* Free human_array */
    for(i = 0; i < total_presets; i++)
	free(human_array[i]);
    free(human_array);
}

/*
 * Set the textures name values to all the texture choice lists
 */
void EditorGtkItemChooserSetTexturesNameList(const sar_core_struct *core_ptr)
{
    char **texture_name_list;
    int i = 0;

    texture_name_list = GetTextureNameListFromSceneryFileName((const char *)core_ptr->cur_scene_file);

if(False)
{
    if(texture_name_list != NULL)
    {
	while(texture_name_list[i] != NULL)
	{
	    g_print("texture_name_list[%d]='%s'\n", i, texture_name_list[i]);
	    i++;
	}
    }
}

    /* Set textures name in texture choice lists */
    EditorGtkItemChooserSetItemsList(GTK_WIDGET(editor_data_widgets.premod_walls_tex), (const char* const*)texture_name_list);
    EditorGtkItemChooserSetItemsList(GTK_WIDGET(editor_data_widgets.premod_walls_tex_night), (const char* const*)texture_name_list);
    EditorGtkItemChooserSetItemsList(GTK_WIDGET(editor_data_widgets.premod_roof_tex), (const char* const*)texture_name_list);

    /* Free textures name array */
    while(texture_name_list[i] != NULL)
	free(texture_name_list[i++]);
    free(texture_name_list);
}

/*
 * Set the reference objects name to all the reference object choice lists
 */
void EditorGtkItemChooserSetRefObjsNameList(const sar_core_struct *core_ptr)
{
    //const sar_scenery_editor_struct *scn_ed = core_ptr->in_game_editor;
    int i, j = 0, total_name = 0;
    sar_object_struct *obj_ptr;
    char **name_array;

    /* Count names by iterating through loaded objects */
    for(i = 0; i < core_ptr->total_objects; i++)
    {
	obj_ptr = core_ptr->object[i];
	if(obj_ptr == NULL)
	    continue;

	if(obj_ptr->name != NULL)
	    total_name++;
    }

    /* Add place for the closing NULL string */
    total_name++;

    /* Alloc memory and set all name_array[] pointers to NULL */
    name_array = calloc(total_name, sizeof(char *));

    /* Get names and fill array by iterating through loaded objects */

    /* Put 'player' as first item of array */
    name_array[j++] = strdup("player");

    /* Fill */
    for(i = 0; i < core_ptr->total_objects; i++)
    {
	obj_ptr = core_ptr->object[i];
	if(obj_ptr == NULL)
	    continue;

	/* Name not NULL nor "player"? */
	if(obj_ptr->name != NULL && strcmp(obj_ptr->name, "player"))
	    name_array[j++] = strdup(obj_ptr->name);
    }

    /* Close array by a NULL pointer */
    name_array[j] = NULL;

    /* Set names in object reference name choice lists */
    EditorGtkItemChooserSetItemsList(GTK_WIDGET(editor_data_widgets.human_ref_obj_name), (const char* const*)name_array);

    /* Helipad can't have player as reference object, skip it */
    EditorGtkItemChooserSetItemsList(GTK_WIDGET(editor_data_widgets.helipad_ref_obj_name), (const char* const*)(name_array + 1));

    /* Free */
    for(i = 0; i < total_name; i++)
	free(name_array[i]);
    free(name_array);
}

/*
 * Populates a choice list widget (GtkComboBox or GtkDropDown, as needed) with
 * the strings contained in the given string array.
 * The last string of the array must be NULL.
 *
 * Returns FALSE if choice list is not a GtkComboBox nor a GtkDropDown.
 */
gboolean EditorGtkItemChooserSetItemsList(GtkWidget *widget, const char* const* string)
{
#if GTK_MAJOR_VERSION == 3 && GTK_MINOR_VERSION >= 24
    if(GTK_IS_COMBO_BOX_TEXT(widget))
    {
	int i = 0;

	gtk_combo_box_text_remove_all(GTK_COMBO_BOX_TEXT(widget));

	while(TRUE)
	{
	    if(string[i] == NULL)
		break;

	    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(widget), string[i++]);
	}
    }
    else
	return FALSE;
#endif
#if GTK_MAJOR_VERSION == 4 && GTK_MINOR_VERSION >= 10
    if(GTK_IS_DROP_DOWN(widget))
    {
	GtkStringList *stringlist = gtk_string_list_new(string);
	gtk_drop_down_set_model(GTK_DROP_DOWN(widget), G_LIST_MODEL(stringlist));
    }
    else
	return FALSE;
#endif

    return TRUE;
}

/*
 * For convenience, returns the value of a
 * Gtk(3)ComboBoxText OR Gtk(4)DropDown, as needed.
 * The value is always a NUL terminated string (never a NULL pointer),
 * and must be freed by the calling function.
 */
char *EditorGtkItemChooserGetSelectedString(void *widget)
{
    char *return_string;

#if GTK_MAJOR_VERSION == 3 && GTK_MINOR_VERSION >= 24
    /* Get combo box selected item string value */
    /* GTK3 note: "The caller of the method takes ownership of the
     * returned data, and is responsible for freeing it."
     */
    char *string = gtk_combo_box_text_get_active_text(widget);
    if(string != NULL)
	return_string = string;
    else
	return_string = strdup("");
#endif
#if GTK_MAJOR_VERSION == 4 && GTK_MINOR_VERSION >= 10
    /* Get drop down selected item string value */
    GObject* g_object = gtk_drop_down_get_selected_item(widget);
    if(g_object != NULL)
	/* GTK4 note: "The returned data is owned by the instance." */
	return_string = strdup(gtk_string_object_get_string(GTK_STRING_OBJECT(g_object)));
    else
	return_string = strdup("");
#endif

    return return_string;
}

/*
 * Sets the selected item of a Gtk(3)ComboBoxText OR Gtk(4)DropDown as needed.
 * Item index is found by comparing the given text value to each value of the
 * Gtk(3)ComboBoxText or Gtk(4)DropDown.
 * If text is NULL, item index will be set to -1 (no active item).
 *
 * Returns a non-zero value on error.
 */
int EditorGtkItemChooserSetSelectedItemFromString(void *widget, const char *text)
{

#if GTK_MAJOR_VERSION == 3 && GTK_MINOR_VERSION >= 24

    GtkTreeModel *tree_model;
    GtkTreeIter iter;
    gchar *str_data;
    gboolean valid, found = FALSE;
    gint index = -1;

    if(text == NULL)
    {
	gtk_combo_box_set_active(GTK_COMBO_BOX(widget), index);
	return 0;
    }

    tree_model = gtk_combo_box_get_model(widget);
    valid = gtk_tree_model_get_iter_first(tree_model, &iter);

    while(valid)
    {
	index++;

	/* Get str_data value.
	 * As our tree has only one level and one column, the str_data value
	 * is at iteration iter (in our case, the line number), column 0.
	 */
	gtk_tree_model_get(tree_model, &iter, 0, &str_data, -1);

	if(!strcasecmp(str_data, text))
	{
	    gtk_combo_box_set_active(GTK_COMBO_BOX(widget), index);
	    found = TRUE;
	    break;
	}

	valid = gtk_tree_model_iter_next(tree_model, &iter);
    }

#endif
#if GTK_MAJOR_VERSION == 4 && GTK_MINOR_VERSION >= 10

    GListModel *list_model;
    GObject *g_object;
    GtkTreeIter iter;
    gchar *str_data;
    gboolean valid, found = FALSE;
    guint list_items;

    if(text == NULL)
    {
	gtk_drop_down_set_selected(widget, GTK_INVALID_LIST_POSITION);
	return 0;
    }

    list_model = gtk_drop_down_get_model(widget);
    list_items = g_list_model_get_n_items(list_model);

    for(int i = 0; i < list_items; i++)
    {
	g_object = g_list_model_get_object(list_model, i);

	if(!strcasecmp(gtk_string_object_get_string(GTK_STRING_OBJECT(g_object)), text))
	{
	    gtk_drop_down_set_selected(widget, i);
	    found = TRUE;
	    break;
	}
    }

#endif

    if(found == TRUE)
	return 0;
    else
	return 1;
}

/*
 * Returns the item index of a Gtk(3)ComboBoxText OR Gtk(4)DropDown as needed.
 * Item index is found by comparing the given text value to each text values
 * of the the Gtk(3)ComboBoxText or Gtk(4)DropDown.
 *
 * Returns -1 if text was not found in the chooser items list.
 */
int EditorGtkItemChooserGetItemIndexFromString(void *widget, const char *text)
{
    gint index = -1;

#if GTK_MAJOR_VERSION == 3 && GTK_MINOR_VERSION >= 24

    GtkTreeModel *tree_model;
    GtkTreeIter iter;
    gchar *str_data;
    gboolean valid, found = FALSE;

    if(text == NULL)
	return -1;

    tree_model = gtk_combo_box_get_model(widget);
    valid = gtk_tree_model_get_iter_first(tree_model, &iter);

    while(valid)
    {
	index++;

	/* Get str_data value.
	 * As our tree has only one level and one column, the str_data value
	 * is at iteration iter (in our case, the line number), column 0.
	 */
	gtk_tree_model_get(tree_model, &iter, 0, &str_data, -1);

	if(!strcasecmp(str_data, text))
	{
	    found = TRUE;
	    break;
	}

	valid = gtk_tree_model_iter_next(tree_model, &iter);
    }

#endif
#if GTK_MAJOR_VERSION == 4 && GTK_MINOR_VERSION >= 10

    GListModel *list_model;
    GObject *g_object;
    GtkTreeIter iter;
    gchar *str_data;
    gboolean valid, found = FALSE;
    guint list_items;

    if(text == NULL)
	return -1;

    list_model = gtk_drop_down_get_model(widget);
    list_items = g_list_model_get_n_items(list_model);

    for(int i = 0; i < list_items; i++)
    {
	g_object = g_list_model_get_object(list_model, i);

	if(!strcasecmp(gtk_string_object_get_string(GTK_STRING_OBJECT(g_object)), text))
	{
	    index = i;
	    found = TRUE;
	    break;
	}
    }

#endif

    if(found == TRUE)
	return index;
    else
	return -1;
}

/*
 * Filters characters in a GtkEntry:
 * Allows only alphabet, digital and underscore characters.
 */
void EditorGktEntryFilterText(void *widget)
{
    int char_num, counter;
    char c;
#if GTK_MAJOR_VERSION == 3 && GTK_MINOR_VERSION >= 24
    const char *text = gtk_entry_get_text(widget);
#endif
#if GTK_MAJOR_VERSION == 4 && GTK_MINOR_VERSION >= 10
    const char *text = gtk_editable_get_text (GTK_EDITABLE (widget));
#endif

    char_num = strlen(text);

    /* Entry empty? */
    if(char_num == 0)
	return;

    /* Remove all non-alphabet nor non-digital nor underscore characters */
    for(counter = 0; counter < char_num; counter++)
    {
	c = text[counter];
	if(!(isalnum(c) || c == '_'))
	    gtk_entry_buffer_delete_text(
		GTK_ENTRY_BUFFER(gtk_entry_get_buffer(widget)),
		counter,
		1
	    );
    }

    return;
}

/*
 * Filters characters in a GtkEntry:
 * Allows all printable characters except space character.
 */
void EditorGktEntryFilterTextNoSpace(void *widget)
{
    int char_num, counter;
    char c;
#if GTK_MAJOR_VERSION == 3 && GTK_MINOR_VERSION >= 24
    const char *text = gtk_entry_get_text(widget);
#endif
#if GTK_MAJOR_VERSION == 4 && GTK_MINOR_VERSION >= 10
    const char *text = gtk_editable_get_text (GTK_EDITABLE (widget));
#endif

    char_num = strlen(text);

    /* Entry empty? */
    if(char_num == 0)
	return;

    /* Remove all non-printable nor space characters */
    for(counter = 0; counter < char_num; counter++)
    {
	c = text[counter];
	if(!isprint(c) || c == ' ')
	    gtk_entry_buffer_delete_text(
		GTK_ENTRY_BUFFER(gtk_entry_get_buffer(widget)),
		counter,
		1
	    );
    }

    return;
}

/*
 * Checks the value entered in a GtkEntry and allow it to be an integer
 * number only. If non_negative is TRUE, only positive or null entries
 * will be allowed.
 */
void EditoGktEntryAllowInt(void *widget, gboolean non_negative)
{
    char c;
    int counter;
#if GTK_MAJOR_VERSION == 3 && GTK_MINOR_VERSION >= 24
    const char *text = gtk_entry_get_text(widget);
#endif
#if GTK_MAJOR_VERSION == 4 && GTK_MINOR_VERSION >= 10
    const char *text = gtk_editable_get_text (GTK_EDITABLE (widget));
#endif

    /* Entry empty? */
    if(strlen(text) == 0)
	return;

    /* Remove first character if non-numeric nor '+' nor '-' as needed */
    c = text[0];
    if(!(isdigit(c) || c == '+' || (non_negative == FALSE && c == '-')))
	gtk_entry_buffer_delete_text(
	    GTK_ENTRY_BUFFER(gtk_entry_get_buffer(widget)),
	    0,
	    1
	);

    /* Remove all other non-numeric characters */
    for(counter = 1; counter < strlen(text); counter++)
    {
	c = text[counter];
	if(!isdigit(c))
	    gtk_entry_buffer_delete_text(
		GTK_ENTRY_BUFFER(gtk_entry_get_buffer(widget)),
		counter,
		1
	    );
    }

    return;
}

/*
 * Converts a GtkEntry text buffer value to an integer. Text buffer
 * value must have been checked by EditoGktEntryAllowInt() while entered.
 * Returns 0 if text buffer is empty.
 */
int EditorGktEntryToInt(void *widget)
{
#if GTK_MAJOR_VERSION == 3 && GTK_MINOR_VERSION >= 24
    const char *text = gtk_entry_get_text(widget);
#endif
#if GTK_MAJOR_VERSION == 4 && GTK_MINOR_VERSION >= 10
    const char *text = gtk_editable_get_text (GTK_EDITABLE (widget));
#endif

    /* Entry empty? */
    if(strlen(text) == 0)
	return 0;

    return atoi(text);
}

/*
 * Checks the value entered in a GtkEntry and allow it to be a double
 * number only. If non_negative is TRUE, only positive or null entries
 * will be allowed.
 */
void EditoGktEntryAllowDouble(void *widget, gboolean non_negative)
{
    char c;
    int counter;
#if GTK_MAJOR_VERSION == 3 && GTK_MINOR_VERSION >= 24
    const char *text = gtk_entry_get_text(widget);
#endif
#if GTK_MAJOR_VERSION == 4 && GTK_MINOR_VERSION >= 10
    const char *text = gtk_editable_get_text(GTK_EDITABLE (widget));
#endif

    /* Entry empty? */
    if(strlen(text) == 0)
	return;

    /* Remove first character if non-numeric nor '+', '-',  nor '.' as needed */
    c = text[0];
    if(!(isdigit(c) || c == '+' || (non_negative == FALSE && c == '-') || c == '.'))
	gtk_entry_buffer_delete_text(
	    GTK_ENTRY_BUFFER(gtk_entry_get_buffer(widget)),
	    0,
	    1
	);

    /* Remove all other non-numeric nor dot decimal point characters */
    for(counter = 1; counter < strlen(text); counter++)
    {
	c = text[counter];
	if(!(isdigit(c) || c == '.'))
	    gtk_entry_buffer_delete_text(
		GTK_ENTRY_BUFFER(gtk_entry_get_buffer(widget)),
		counter,
		1
	    );
    }

    return;
}

/*
 * Converts a GtkEntry text buffer value to a double. Text buffer
 * value must have been checked by EditoGktEntryAllowDouble() while entered.
 * Returns 0 if text buffer is empty.
 */
double EditorGktEntryToDouble(void *widget)
{
    char right_s[11];
    int counter, left = 0, right = 0, denom = 1;
#if GTK_MAJOR_VERSION == 3 && GTK_MINOR_VERSION >= 24
    const char *text = gtk_entry_get_text(widget);
#endif
#if GTK_MAJOR_VERSION == 4 && GTK_MINOR_VERSION >= 10
    const char *text = gtk_editable_get_text(GTK_EDITABLE(widget));
#endif

    /* Entry empty? */
    if(strlen(text) == 0)
	return 0;

    /* Get left and right parts */
    sscanf(text, "%d.%10s", &left, right_s);
    right = atoi(right_s);
    if(left < 0)
	right = -right;

    /* Calculate right denominator */
    for(counter = 0; counter < strlen(right_s); counter++)
	denom *= 10;

    return (double)left + (double)right/denom;
}

/*
 * Sets a text value in a GtkEntry text buffer */
void EditorGtkEntrySetText(void *widget, const char *text)
{
    if(text == NULL)
	return;

#if GTK_MAJOR_VERSION == 3 && GTK_MINOR_VERSION >= 24
    gtk_entry_set_text(widget, text);
#endif
#if GTK_MAJOR_VERSION == 4 && GTK_MINOR_VERSION >= 10
    gtk_editable_set_text(GTK_EDITABLE(widget), text);
#endif
}

/*
 * Sets an integer value in a GtkEntry text buffer */
void EditorGtkEntrySetTextFromInt(void *widget, double value)
{
    char text[16];
    snprintf(text, 15, "%d", (int)value);

#if GTK_MAJOR_VERSION == 3 && GTK_MINOR_VERSION >= 24
    gtk_entry_set_text(widget, text);
#endif
#if GTK_MAJOR_VERSION == 4 && GTK_MINOR_VERSION >= 10
    gtk_editable_set_text(GTK_EDITABLE(widget), text);
#endif
}

/*
 * Sets a double value in a GtkEntry text buffer */
void EditorGtkEntrySetTextFromDouble(void *widget, double value)
{
    char text[16];
    snprintf(text, 15, "%f", value);

    /* Avoid LOCALE issue: set comma as decimal point */
    for(int i = 0; i < strlen(text); i++)
    {
	if(text[i] == '.')
	{
	    break;
	}
	else if(text[i] == ',')
	{
	    text[i] = '.';
	    break;
	}
    }

#if GTK_MAJOR_VERSION == 3 && GTK_MINOR_VERSION >= 24
    gtk_entry_set_text(widget, text);
#endif
#if GTK_MAJOR_VERSION == 4 && GTK_MINOR_VERSION >= 10
    gtk_editable_set_text(GTK_EDITABLE(widget), text);
#endif
}


/*
 * TODO: add comment
 */
void EditorGtkAskToQuitWithoutPrint()
{
    GtkWindow *quit_without_print_window;

    quit_without_print_window = GTK_WINDOW(editor_quit_without_print_widgets.window);

    gtk_window_set_modal(GTK_WINDOW(editor_quit_without_print_widgets.window), TRUE);

#if GTK_MAJOR_VERSION == 3 && GTK_MINOR_VERSION >= 24
    /* Show the window */
    gtk_widget_show_all(GTK_WIDGET(quit_without_print_window));

    /* Force the menu window to always be on top.
     * Note: it works (at least) on my K Desktop Environment and I hope should
     * work for other X11 systems desktop environments.
     */
    gtk_window_set_keep_above(quit_without_print_window, TRUE);
#endif
#if GTK_MAJOR_VERSION == 4 && GTK_MINOR_VERSION >= 10
    /* Show the window */
    gtk_window_present(quit_without_print_window);
#endif
}


#if GTK_MAJOR_VERSION == 3 && GTK_MINOR_VERSION >= 24
void set_children_sensitive_by_name(
	GtkWidget* parent,
	const gchar* name,
	gboolean sensitive
)
{
    GtkWidget *child;

    if(GTK_CONTAINER(parent)) {
	GList *children = gtk_container_get_children(GTK_CONTAINER(parent));
	GList *children_iterate = children;

	while(children_iterate != NULL)
	{
	    child = GTK_WIDGET(children_iterate->data);

	    if(!strcmp(gtk_widget_get_name(child), name))
		gtk_widget_set_sensitive(child, sensitive);

	    children_iterate = children_iterate->next;
	}

	g_list_free(children);
    }
}
#endif
#if GTK_MAJOR_VERSION == 4 && GTK_MINOR_VERSION >= 10
void set_children_sensitive_by_name(
	GtkWidget* parent,
	const gchar* name,
	gboolean sensitive
)
{
    GtkWidget *child;

    child = gtk_widget_get_first_child(parent);

    while(child != NULL)
    {
	if(!strcmp(gtk_widget_get_name(child), name))
		gtk_widget_set_sensitive(child, sensitive);

	child = gtk_widget_get_next_sibling(child);
    }
}
#endif

/* Try to open the V3D (*.3d) file_name file then look for the 'name'
 * parameter. If found, the parameter value is returned as a string
 * which must be freed by calling function.
 *
 * Returns NULL if file can't be open or if the 'name' parameter was not found.
 */
char *GetModelNameFromModelFileName(const char *file_name)
{
    FILE* fp = NULL;
    struct stat sb;
    char *file_contents;
    char cmd[8], model_name[256];
    Boolean found = FALSE;

    if(file_name == NULL)
	return NULL;

    fp = fopen(file_name, "r");
    if(fp != NULL)
    {
	if (stat(file_name, &sb) == -1)
	{
	    perror("stat");
	    exit(EXIT_FAILURE);
	}

	// Note: this allocates a lot of (too much?) memory, but is secure... //
	file_contents = malloc(sb.st_size);

	while (fscanf(fp, "%[^\n] ", file_contents) != EOF)
	{
	    sscanf(file_contents, "%7s %255[^\n]", cmd, model_name);

	    // name parameter found? //
	    if(!strcasecmp(cmd, "name"))
	    {
		found = TRUE;
		break;
	    }
	}

	free(file_contents);
    }
    else
    {
	fprintf(stderr, "%s:%d: can't open file \"%s\": ", __FILE__, __LINE__, file_name);
	perror("");
    }

    if(found == TRUE)
	return strdup(model_name);
    else
	return NULL;
}

/* Tries to open the V3D (*.3d) file_name file then look for 'texture_load'
 * parameters. If found, the texture names are returned as a NULL terminated
 * string array which must be freed by calling function.
 *
 * Returns NULL if file can't be open or if no 'texture_load' parameter was found.
 */
char **GetTextureNameListFromSceneryFileName(const char *file_name)
{
    FILE* fp = NULL;
    struct stat sb;
    char *file_contents;
    char cmd[14], texture_name[256];
    char **texture_list = NULL;
    int total_textures = 0;

    if(file_name == NULL)
	return NULL;

    fp = fopen(file_name, "r");
    if(fp != NULL)
    {
	if (stat(file_name, &sb) == -1)
	{
	    perror("stat");
	    exit(EXIT_FAILURE);
	}

	/* Note: this allocates a lot of (too much?) memory, but is secure... */
	file_contents = malloc(sb.st_size);

	while (fscanf(fp, "%[^\n] ", file_contents) != EOF)
	{
	    sscanf(file_contents, "%12s %255s", cmd, texture_name);

	    /* texture_load parameter found? */
	    if(!strcasecmp(cmd, "texture_load"))
	    {
		texture_list = realloc(texture_list, (total_textures + 1) * sizeof(char *));
		texture_list[total_textures++] = strdup(texture_name);
	    }
	    else if(!strcasecmp(cmd, "create_object"))
		/* Don't waste time and energy: stop reading file at first
		 * create_object parameter. As texture_load parameters are
		 * at top of file, all textures should have been read.
		 */
		break;
	}
	/* Close list with a NULL pointer */
	texture_list = realloc(texture_list, (total_textures + 1) * sizeof(char *));
	texture_list[total_textures] = NULL;

	free(file_contents);
    }
    else
    {
	fprintf(stderr, "%s:%d: can't open file \"%s\": ", __FILE__, __LINE__, file_name);
	perror("");
    }

    return texture_list;
}

/*
 * GTK callbacks.
 * Relevant callback is called when corresponding event is fired (button
 * clicked, value changed, ...).
 * See editor_menu_widgets_gtk3_LIST and/or editor_menu_widgets_gtk4_LIST
 * for widget ID / callback function name pairs.
 *
 * Note: These may not be declared static because signal autoconnection
 * only works with non-static methods
 */

void on_data_window_destroy(GtkWidget *widget, gpointer user_data)
{
    // const sar_core_struct *core_ptr = (sar_core_struct *)user_data;

#if GTK_MAJOR_VERSION == 3 && GTK_MINOR_VERSION >= 24
    gtk_widget_hide(GTK_WIDGET(editor_data_widgets.window));
#endif
#if GTK_MAJOR_VERSION == 4 && GTK_MINOR_VERSION >= 10
    gtk_widget_set_visible(editor_data_widgets.window, FALSE);
#endif
}

void on_dialog_window_destroy(GtkWidget *widget, gpointer user_data)
{
    // const sar_core_struct *core_ptr = (sar_core_struct *)user_data;

#if GTK_MAJOR_VERSION == 3 && GTK_MINOR_VERSION >= 24
    gtk_widget_hide(GTK_WIDGET(editor_quit_without_print_widgets.window));
#endif
#if GTK_MAJOR_VERSION == 4 && GTK_MINOR_VERSION >= 10
    gtk_widget_set_visible(editor_quit_without_print_widgets.window, FALSE);
#endif
}

void on_menu_window_destroy(GtkWidget *widget, gpointer user_data)
{
    on_button_quit_clicked(editor_menu_widgets.button_quit, (gpointer)user_data);
}


void on_general_type_changed(void *widget, gpointer user_data)
{
    /* WARNING FIXME: GTK4.10 seems to returns bad user_data pointer value.
     * // const sar_core_struct *core_ptr = (sar_core_struct *)user_data;
     */
    //const sar_core_struct *core_ptr = (sar_core_struct *)user_data;
    char *selected_s;

    selected_s = EditorGtkItemChooserGetSelectedString(widget);

    /* No object type set? */
    if(selected_s[0] == '\0')
    {
	/* Set the general frame widgets -except the "Type" one- insensitive */
	set_children_sensitive_by_name(
	    GTK_WIDGET(editor_data_widgets.general_frame_grid), "general_data", FALSE);

	/* Hide all other widgets */
	gtk_widget_set_visible(GTK_WIDGET(editor_data_widgets.fire_frame), FALSE);
	gtk_widget_set_visible(GTK_WIDGET(editor_data_widgets.helipad_frame), FALSE);
	gtk_widget_set_visible(GTK_WIDGET(editor_data_widgets.runway_frame), FALSE);
	gtk_widget_set_visible(GTK_WIDGET(editor_data_widgets.human_frame), FALSE);
	gtk_widget_set_visible(GTK_WIDGET(editor_data_widgets.model_frame), FALSE);
	gtk_widget_set_visible(GTK_WIDGET(editor_data_widgets.premodeled_frame), FALSE);
	gtk_widget_set_visible(GTK_WIDGET(editor_data_widgets.smoke_frame), FALSE);
    }
    else if(!strcasecmp(selected_s, SAR_OBJ_TYPE_FIRE_S))
    {
	set_children_sensitive_by_name(
	    GTK_WIDGET(editor_data_widgets.general_frame_grid), "general_data", TRUE);

	gtk_widget_set_visible(GTK_WIDGET(editor_data_widgets.fire_frame), TRUE);
	gtk_widget_set_sensitive(GTK_WIDGET(editor_data_widgets.fire_frame_grid), TRUE);

	gtk_widget_set_visible(GTK_WIDGET(editor_data_widgets.helipad_frame), FALSE);
	gtk_widget_set_visible(GTK_WIDGET(editor_data_widgets.runway_frame), FALSE);
	gtk_widget_set_visible(GTK_WIDGET(editor_data_widgets.human_frame), FALSE);
	gtk_widget_set_visible(GTK_WIDGET(editor_data_widgets.model_frame), FALSE);
	gtk_widget_set_visible(GTK_WIDGET(editor_data_widgets.premodeled_frame), FALSE);
	gtk_widget_set_visible(GTK_WIDGET(editor_data_widgets.smoke_frame), FALSE);
    }
    else if(!strcasecmp(selected_s, SAR_OBJ_TYPE_HELIPAD_S))
    {
	set_children_sensitive_by_name(
	    GTK_WIDGET(editor_data_widgets.general_frame_grid), "general_data", TRUE);

	gtk_widget_set_visible(GTK_WIDGET(editor_data_widgets.fire_frame), FALSE);

	gtk_widget_set_visible(GTK_WIDGET(editor_data_widgets.helipad_frame), TRUE);
	gtk_widget_set_sensitive(GTK_WIDGET(editor_data_widgets.helipad_frame_grid), TRUE);

	gtk_widget_set_visible(GTK_WIDGET(editor_data_widgets.runway_frame), FALSE);
	gtk_widget_set_visible(GTK_WIDGET(editor_data_widgets.human_frame), FALSE);
	gtk_widget_set_visible(GTK_WIDGET(editor_data_widgets.model_frame), FALSE);
	gtk_widget_set_visible(GTK_WIDGET(editor_data_widgets.premodeled_frame), FALSE);
	gtk_widget_set_visible(GTK_WIDGET(editor_data_widgets.smoke_frame), FALSE);
    }
    else if(!strcasecmp(selected_s, SAR_OBJ_TYPE_RUNWAY_S))
    {
	set_children_sensitive_by_name(
	    GTK_WIDGET(editor_data_widgets.general_frame_grid), "general_data", TRUE);

	gtk_widget_set_visible(GTK_WIDGET(editor_data_widgets.fire_frame), FALSE);
	gtk_widget_set_visible(GTK_WIDGET(editor_data_widgets.helipad_frame), FALSE);

	gtk_widget_set_visible(GTK_WIDGET(editor_data_widgets.runway_frame), TRUE);
	gtk_widget_set_sensitive(GTK_WIDGET(editor_data_widgets.runway_frame_grid), TRUE);

	gtk_widget_set_visible(GTK_WIDGET(editor_data_widgets.human_frame), FALSE);
	gtk_widget_set_visible(GTK_WIDGET(editor_data_widgets.model_frame), FALSE);
	gtk_widget_set_visible(GTK_WIDGET(editor_data_widgets.premodeled_frame), FALSE);
	gtk_widget_set_visible(GTK_WIDGET(editor_data_widgets.smoke_frame), FALSE);
    }
    else if(!strcasecmp(selected_s, SAR_OBJ_TYPE_HUMAN_S))
    {
	/* Set assistants name choice list non-sensitive as needed */
	on_human_assistants_changed(editor_data_widgets.human_assistants, (gpointer)user_data);

	set_children_sensitive_by_name(
	    GTK_WIDGET(editor_data_widgets.general_frame_grid), "general_data", TRUE);

	gtk_widget_set_visible(GTK_WIDGET(editor_data_widgets.fire_frame), FALSE);
	gtk_widget_set_visible(GTK_WIDGET(editor_data_widgets.helipad_frame), FALSE);
	gtk_widget_set_visible(GTK_WIDGET(editor_data_widgets.runway_frame), FALSE);

	gtk_widget_set_visible(GTK_WIDGET(editor_data_widgets.human_frame), TRUE);
	gtk_widget_set_sensitive(GTK_WIDGET(editor_data_widgets.human_frame_grid), TRUE);

	gtk_widget_set_visible(GTK_WIDGET(editor_data_widgets.model_frame), FALSE);
	gtk_widget_set_visible(GTK_WIDGET(editor_data_widgets.premodeled_frame), FALSE);
	gtk_widget_set_visible(GTK_WIDGET(editor_data_widgets.smoke_frame), FALSE);
    }
    else if(!strcasecmp(selected_s, SAR_OBJ_TYPE_AUTOMOBILE_S) ||
	!strcasecmp(selected_s, SAR_OBJ_TYPE_STATIC_S) ||
	!strcasecmp(selected_s, SAR_OBJ_TYPE_WATERCRAFT_S)
    )
    {
	char *path;

	/* Clean previous file name and associated model name */
#if GTK_MAJOR_VERSION == 3 && GTK_MINOR_VERSION >= 24
	gtk_file_chooser_set_filename(
		GTK_FILE_CHOOSER(editor_data_widgets.model_file_name),
		""
	    );
	EditorGtkEntrySetText(editor_data_widgets.model_name, "");
#endif
#if GTK_MAJOR_VERSION == 4 && GTK_MINOR_VERSION >= 10
/* TODO
 */
#endif

	/* Define file chooser default path */

	if(!strcasecmp(selected_s, SAR_OBJ_TYPE_AUTOMOBILE_S))
	{
	    path = COMPLETE_PATH((const char *)SAR_DEF_AUTOMOBILES_DIR);
	}
	else if(!strcasecmp(selected_s, SAR_OBJ_TYPE_STATIC_S))
	{
	    path = COMPLETE_PATH((const char *)SAR_DEF_OBJECTS_DIR);
	}
	else if(!strcasecmp(selected_s, SAR_OBJ_TYPE_WATERCRAFT_S))
	{
	    path = COMPLETE_PATH((const char *)SAR_DEF_WATERCRAFTS_DIR);
	}

	/* Set file chooser default path */
#if GTK_MAJOR_VERSION == 3 && GTK_MINOR_VERSION >= 24
	gtk_file_chooser_set_current_folder(
		GTK_FILE_CHOOSER(editor_data_widgets.model_file_name),
		path
	    );
#endif
#if GTK_MAJOR_VERSION == 4 && GTK_MINOR_VERSION >= 10
/* TODO
	GFile *folder = g_file_new_for_path(const char* path);
	gtk_file_dialog_set_initial_folder(
		GTK_FILE_DIALOG(dialog),
		GFile* folder
	);
*/
#endif

	free(path);

	set_children_sensitive_by_name(
	    GTK_WIDGET(editor_data_widgets.general_frame_grid), "general_data", TRUE);

	gtk_widget_set_visible(GTK_WIDGET(editor_data_widgets.fire_frame), FALSE);
	gtk_widget_set_visible(GTK_WIDGET(editor_data_widgets.helipad_frame), FALSE);
	gtk_widget_set_visible(GTK_WIDGET(editor_data_widgets.runway_frame), FALSE);
	gtk_widget_set_visible(GTK_WIDGET(editor_data_widgets.human_frame), FALSE);

	gtk_widget_set_visible(GTK_WIDGET(editor_data_widgets.model_frame), TRUE);
	gtk_widget_set_sensitive(GTK_WIDGET(editor_data_widgets.model_frame_grid), TRUE);

	gtk_widget_set_visible(GTK_WIDGET(editor_data_widgets.premodeled_frame), FALSE);
	gtk_widget_set_visible(GTK_WIDGET(editor_data_widgets.smoke_frame), FALSE);
    }
    else if(!strcasecmp(selected_s, SAR_OBJ_TYPE_PREMODELED_S))
    {
	/* Set selected item as first item */
#if GTK_MAJOR_VERSION == 3 && GTK_MINOR_VERSION >= 24
	gtk_combo_box_set_active(GTK_COMBO_BOX(editor_data_widgets.premod_type), 0);
	gtk_combo_box_set_active(GTK_COMBO_BOX(editor_data_widgets.premod_walls_tex), 0);
	gtk_combo_box_set_active(GTK_COMBO_BOX(editor_data_widgets.premod_walls_tex_night), 0);
	gtk_combo_box_set_active(GTK_COMBO_BOX(editor_data_widgets.premod_roof_tex), 0);
#endif
#if GTK_MAJOR_VERSION == 4 && GTK_MINOR_VERSION >= 10
	gtk_drop_down_set_selected(editor_data_widgets.premod_type, 0);
	gtk_drop_down_set_selected(editor_data_widgets.premod_walls_tex, 0);
	gtk_drop_down_set_selected(editor_data_widgets.premod_walls_tex_night, 0);
	gtk_drop_down_set_selected(editor_data_widgets.premod_roof_tex, 0);
#endif

	set_children_sensitive_by_name(
	    GTK_WIDGET(editor_data_widgets.general_frame_grid), "general_data", TRUE);
	gtk_widget_set_visible(GTK_WIDGET(editor_data_widgets.fire_frame), FALSE);
	gtk_widget_set_visible(GTK_WIDGET(editor_data_widgets.helipad_frame), FALSE);
	gtk_widget_set_visible(GTK_WIDGET(editor_data_widgets.runway_frame), FALSE);
	gtk_widget_set_visible(GTK_WIDGET(editor_data_widgets.human_frame), FALSE);
	gtk_widget_set_visible(GTK_WIDGET(editor_data_widgets.model_frame), FALSE);

	gtk_widget_set_visible(GTK_WIDGET(editor_data_widgets.premodeled_frame), TRUE);
	gtk_widget_set_sensitive(GTK_WIDGET(editor_data_widgets.premodeled_frame_grid), TRUE);

	gtk_widget_set_visible(GTK_WIDGET(editor_data_widgets.smoke_frame), FALSE);
    }
    else if(!strcasecmp(selected_s, SAR_OBJ_TYPE_SMOKE_S))
    {
	set_children_sensitive_by_name(
	    GTK_WIDGET(editor_data_widgets.general_frame_grid), "general_data", TRUE);
	gtk_widget_set_visible(GTK_WIDGET(editor_data_widgets.fire_frame), FALSE);
	gtk_widget_set_visible(GTK_WIDGET(editor_data_widgets.helipad_frame), FALSE);
	gtk_widget_set_visible(GTK_WIDGET(editor_data_widgets.runway_frame), FALSE);
	gtk_widget_set_visible(GTK_WIDGET(editor_data_widgets.human_frame), FALSE);
	gtk_widget_set_visible(GTK_WIDGET(editor_data_widgets.model_frame), FALSE);
	gtk_widget_set_visible(GTK_WIDGET(editor_data_widgets.premodeled_frame), FALSE);

	gtk_widget_set_visible(GTK_WIDGET(editor_data_widgets.smoke_frame), TRUE);
	gtk_widget_set_sensitive(GTK_WIDGET(editor_data_widgets.smoke_frame_grid), TRUE);
    }

    /* Object type set? */
    if(selected_s[0] != '\0')
    {
	/* Reset general_object_name text and placeholder text */
#if GTK_MAJOR_VERSION == 3 && GTK_MINOR_VERSION >= 24
	gtk_entry_set_text(editor_data_widgets.general_object_name, "");
#endif
#if GTK_MAJOR_VERSION == 4 && GTK_MINOR_VERSION >= 10
	gtk_editable_set_text(GTK_EDITABLE(editor_data_widgets.general_object_name), "");
#endif
	gtk_entry_set_placeholder_text(editor_data_widgets.general_object_name, "");

	/* Show object name widgets because user can need them now */
	gtk_widget_set_visible(GTK_WIDGET(editor_data_widgets.general_object_name_label), TRUE);
	gtk_widget_set_visible(GTK_WIDGET(editor_data_widgets.general_object_name), TRUE);
    }

    free(selected_s);

    /* Set scrolled window height.
     * Note that this is the height of the scrollable area inside the window,
     * not the height of the window itself.
     */
    gtk_scrolled_window_set_propagate_natural_height(GTK_SCROLLED_WINDOW(editor_data_widgets.scrolled_window), TRUE);

    /* Resize the window */
    gint natural_height = 0, natural_width = 0;
#if GTK_MAJOR_VERSION == 3 && GTK_MINOR_VERSION >= 24
    gtk_widget_get_preferred_height(GTK_WIDGET(editor_data_widgets.main_box), NULL, &natural_height);
    gtk_widget_get_preferred_width(GTK_WIDGET(editor_data_widgets.main_box), NULL, &natural_width);
    gtk_window_resize(GTK_WINDOW(editor_data_widgets.window), natural_width, natural_height);
#endif
#if GTK_MAJOR_VERSION == 4 && GTK_MINOR_VERSION >= 10
    GtkRequisition minimum_size, natural_size;
    gtk_widget_get_preferred_size(GTK_WIDGET(editor_data_widgets.main_box), &minimum_size, &natural_size);
    natural_width = natural_size.width;
    natural_height = natural_size.height;
    gtk_window_set_default_size(GTK_WINDOW(editor_data_widgets.window), natural_width, natural_height);
#endif


    return;
}

/*
void on_name_changed (void *widget, gpointer user_data) {
    g_print("on_name_changed: text='%s'\n", gtk_entry_get_text(widget));
}
*/

void on_obj_name_changed(void *widget, gpointer user_data)
{
    /* Filter entry text (forbid blank and remove all
     * non-alpha/digit characters).
     */
    EditorGktEntryFilterText(widget);

    return;
}

void on_pos_x_changed(void *widget, gpointer user_data)
{
    /* Allow a positive, zero, or negative double value from entry text */
    EditoGktEntryAllowDouble(widget, FALSE);
    return;
}

void on_pos_y_changed(void *widget, gpointer user_data)
{
    /* Allow a positive, zero, or negative double value from entry text */
    EditoGktEntryAllowDouble(widget, FALSE);
    return;
}

void on_pos_z_changed(void *widget, gpointer user_data)
{
    /* Allow a positive, zero, or negative double value from entry text */
    EditoGktEntryAllowDouble(widget, FALSE);
    return;
}

void on_dir_heading_changed(void *widget, gpointer user_data)
{
    /* Allow a positive, zero, or negative double value from entry text */
    EditoGktEntryAllowDouble(widget, FALSE);
    return;
}

void on_dir_pitch_changed(void *widget, gpointer user_data)
{
    /* Allow a positive, zero, or negative double value from entry text */
    EditoGktEntryAllowDouble(widget, FALSE);
    return;
}

void on_dir_bank_changed(void *widget, gpointer user_data)
{
    /* Allow a positive, zero, or negative double value from entry text */
    EditoGktEntryAllowDouble(widget, FALSE);
    return;
}

void on_fire_radius_changed(void *widget, gpointer user_data)
{
    /* Allow a positive or null double value from entry text */
    EditoGktEntryAllowDouble(widget, TRUE);
    return;
}

void on_height_changed(void *widget, gpointer user_data)
{
    /* Allow a positive or null double value from entry text */
    EditoGktEntryAllowDouble(widget, TRUE);
    return;
}

void on_helipad_style_changed(void *widget, gpointer user_data)
{
    char *selected_s = EditorGtkItemChooserGetSelectedString(widget);

    /* Sets sensitivity of the "Recession" GtkEntry widget in accordance to
     * the helipad style.
     */

    if(!strcmp(selected_s, SAR_HELIPAD_STYLE_BUILDING_S) ||
	 !strcmp(selected_s, SAR_HELIPAD_STYLE_VEHICLE_S)
    )
    {
	gtk_widget_set_sensitive(GTK_WIDGET(editor_data_widgets.helipad_recession), TRUE);
    }
    else
    {
	gtk_widget_set_sensitive(GTK_WIDGET(editor_data_widgets.helipad_recession), FALSE);
    }

#if GTK_MAJOR_VERSION == 3 && GTK_MINOR_VERSION >= 24
/*
    GtkTreeModel *tree_model = gtk_combo_box_get_model(widget);
    fprintf(stderr, "%s:%d: tree_model=%p\n", __FILE__, __LINE__, tree_model);
    GtkTreeIter* iter;
    if(gtk_tree_model_get_iter_first(tree_model, iter))
	g_print("value='%s'\n", gtk_tree_model_get_string_from_iter(tree_model, iter));
*/
//    GType g_type = g_list_model_get_item_type(list_model);
//    g_print("g_type=%d\n", g_type);
//    GTypeQuery *query;
#endif
#if GTK_MAJOR_VERSION == 4 && GTK_MINOR_VERSION >= 10
/*
    guint hash_size;
    GHashTable *hash_table = g_hash_table_new(NULL, NULL);

    g_hash_table_add(hash_table, "hello 1");
    g_hash_table_add(hash_table, "hello 2");
    g_hash_table_add(hash_table, "hello 3");
    hash_size = g_hash_table_size(hash_table);




    gpointer key_array = g_hash_table_get_keys_as_array (hash_table, &hash_size); // Get the keys, which are strings, as an array
    GtkStringList *stringlist = gtk_string_list_new(key_array); // Create a new GtkStringList model from the array of strings.

    for(int i = 0; i < 3; i++)
	g_print("string[%d]='%s'\n", i, gtk_string_list_get_string(stringlist, i));

    if(GTK_IS_DROP_DOWN(widget))
    {
	g_print("Widget...\n");
	gtk_drop_down_set_model(GTK_DROP_DOWN(widget), G_LIST_MODEL(stringlist)); // Set the model as the source for the dropdown menu.
    }

    g_free(hash_table);
*/

#endif

    free(selected_s);

    return;
}

void on_length_changed(void *widget, gpointer user_data)
{
    /* Allow a positive or null double value from entry text */
    EditoGktEntryAllowDouble(widget, TRUE);
    return;
}

void on_width_changed(void *widget, gpointer user_data)
{
    /* Allow a positive or null double value from entry text */
    EditoGktEntryAllowDouble(widget, TRUE);
    return;
}

void on_helipad_recession_changed(void *widget, gpointer user_data)
{
    /* Allow a positive or null double value from entry text */
    EditoGktEntryAllowDouble(widget, TRUE);
    return;
}

void on_helipad_label_changed(void *widget, gpointer user_data)
{
    /* Filter entry text (forbid blank characters) */
    EditorGktEntryFilterTextNoSpace(widget);
    return;
}

void on_helipad_lighting_toggled(void *widget, gpointer user_data)
{
    return;
}

void on_helipad_fuel_toggled(void *widget, gpointer user_data)
{
    return;
}

void on_helipad_repair_toggled(void *widget, gpointer user_data)
{
    return;
}

void on_helipad_drop_off_toggled(void *widget, gpointer user_data)
{
    return;
}

void on_helipad_is_restarting_toggled(void *widget, gpointer user_data)
{
    return;
}

void on_helipad_is_referenced_toggled(void *widget, gpointer user_data)
{
    /* Get check button state */
    gboolean state = GTK_CHECK_BUTTON_GET_ACTIVE(widget);

    /* Copy state to relevant widgets */
    gtk_widget_set_sensitive(GTK_WIDGET(editor_data_widgets.helipad_ref_obj_label), state);
    gtk_widget_set_sensitive(GTK_WIDGET(editor_data_widgets.helipad_ref_obj_name), state);
    gtk_widget_set_sensitive(GTK_WIDGET(editor_data_widgets.helipad_offset_pos_label), state);
    gtk_widget_set_sensitive(GTK_WIDGET(editor_data_widgets.helipad_offset_pos_x), state);
    gtk_widget_set_sensitive(GTK_WIDGET(editor_data_widgets.helipad_offset_pos_y), state);
    gtk_widget_set_sensitive(GTK_WIDGET(editor_data_widgets.helipad_offset_pos_z), state);
    gtk_widget_set_sensitive(GTK_WIDGET(editor_data_widgets.helipad_offset_dir_label), state);
    gtk_widget_set_sensitive(GTK_WIDGET(editor_data_widgets.helipad_offset_dir_heading), state);
    gtk_widget_set_sensitive(GTK_WIDGET(editor_data_widgets.helipad_offset_dir_pitch), state);
    gtk_widget_set_sensitive(GTK_WIDGET(editor_data_widgets.helipad_offset_dir_bank), state);

    return;
}

void on_helipad_ref_obj_changed(void *widget, gpointer user_data)
{
    // const sar_core_struct *core_ptr = (sar_core_struct *)user_data;

    ////////////////////////// TODO set reference object pos and dir values

    return;
}

void on_ref_object_pos_x_changed(void *widget, gpointer user_data)
{
    /* Allow a positive, zero, or negative double value from entry text */
    EditoGktEntryAllowDouble(widget, FALSE);
    return;
}

void on_ref_object_pos_y_changed(void *widget, gpointer user_data)
{
    /* Allow a positive, zero, or negative double value from entry text */
    EditoGktEntryAllowDouble(widget, FALSE);
    return;
}

void on_ref_object_pos_z_changed(void *widget, gpointer user_data)
{
    /* Allow a positive, zero, or negative double value from entry text */
    EditoGktEntryAllowDouble(widget, FALSE);
    return;
}

void on_ref_object_dir_heading_changed(void *widget, gpointer user_data)
{
    /* Allow a positive, zero, or negative double value from entry text */
    EditoGktEntryAllowDouble(widget, FALSE);
    return;
}

void on_ref_object_dir_pitch_changed(void *widget, gpointer user_data)
{
    /* Allow a positive, zero, or negative double value from entry text */
    EditoGktEntryAllowDouble(widget, FALSE);
    return;
}

void on_ref_object_dir_bank_changed(void *widget, gpointer user_data)
{
    /* Allow a positive, zero, or negative double value from entry text */
    EditoGktEntryAllowDouble(widget, FALSE);
    return;
}

void on_offset_pos_x_changed(void *widget, gpointer user_data)
{
    /* Allow a positive, zero, or negative double value from entry text */
    EditoGktEntryAllowDouble(widget, FALSE);
    return;
}

void on_offset_pos_y_changed(void *widget, gpointer user_data)
{
    /* Allow a positive, zero, or negative double value from entry text */
    EditoGktEntryAllowDouble(widget, FALSE);
    return;
}

void on_offset_pos_z_changed(void *widget, gpointer user_data)
{
    /* Allow a positive, zero, or negative double value from entry text */
    EditoGktEntryAllowDouble(widget, FALSE);
    return;
}

void on_offset_dir_heading_changed(void *widget, gpointer user_data)
{
    /* Allow a positive, zero, or negative double value from entry text */
    EditoGktEntryAllowDouble(widget, FALSE);
    return;
}

void on_offset_dir_pitch_changed(void *widget, gpointer user_data)
{
    /* Allow a positive, zero, or negative double value from entry text */
    EditoGktEntryAllowDouble(widget, FALSE);
    return;
}

void on_offset_dir_bank_changed(void *widget, gpointer user_data)
{
    /* Allow a positive, zero, or negative double value from entry text */
    EditoGktEntryAllowDouble(widget, FALSE);
    return;
}

void on_human_type_name_changed(void *widget, gpointer user_data)
{
    return;
}

void on_human_need_rescue_toggled(void *widget, gpointer user_data)
{
    return;
}

void on_human_sit_up_toggled(void *widget, gpointer user_data)
{
    return;
}

void on_human_sit_down_toggled(void *widget, gpointer user_data)
{
    return;
}

void on_human_sitting_toggled(void *widget, gpointer user_data)
{
    return;
}

void on_human_lying_toggled(void *widget, gpointer user_data)
{
    return;
}

void on_human_alert_toggled(void *widget, gpointer user_data)
{
    return;
}

void on_human_aware_toggled(void *widget, gpointer user_data)
{
    return;
}

void on_human_human_in_water_toggled(void *widget, gpointer user_data)
{
    return;
}

void on_human_on_stretcher_toggled(void *widget, gpointer user_data)
{
    return;
}

void on_human_assistants_changed(void *widget, gpointer user_data)
{
    int assistants;

    /* Get value */
    assistants = (int)gtk_spin_button_get_value(widget);

    /* Set non-sensitive as needed */
    switch(assistants)
    {
	case 0:
	    gtk_widget_set_sensitive(GTK_WIDGET(editor_data_widgets.human_assist_1_name_label), FALSE);
	    gtk_widget_set_sensitive(GTK_WIDGET(editor_data_widgets.human_assist_1_name), FALSE);
	case 1:
	    gtk_widget_set_sensitive(GTK_WIDGET(editor_data_widgets.human_assist_2_name_label), FALSE);
	    gtk_widget_set_sensitive(GTK_WIDGET(editor_data_widgets.human_assist_2_name), FALSE);
	case 2:
	    gtk_widget_set_sensitive(GTK_WIDGET(editor_data_widgets.human_assist_3_name_label), FALSE);
	    gtk_widget_set_sensitive(GTK_WIDGET(editor_data_widgets.human_assist_3_name), FALSE);
	case 3:
	    gtk_widget_set_sensitive(GTK_WIDGET(editor_data_widgets.human_assist_4_name_label), FALSE);
	    gtk_widget_set_sensitive(GTK_WIDGET(editor_data_widgets.human_assist_4_name), FALSE);
	default:
	    break;
    }

    /* Set sensitive as needed */
    switch(assistants)
    {
	case 4:
	    gtk_widget_set_sensitive(GTK_WIDGET(editor_data_widgets.human_assist_4_name_label), TRUE);
	    gtk_widget_set_sensitive(GTK_WIDGET(editor_data_widgets.human_assist_4_name), TRUE);
	case 3:
	    gtk_widget_set_sensitive(GTK_WIDGET(editor_data_widgets.human_assist_3_name_label), TRUE);
	    gtk_widget_set_sensitive(GTK_WIDGET(editor_data_widgets.human_assist_3_name), TRUE);
	case 2:
	    gtk_widget_set_sensitive(GTK_WIDGET(editor_data_widgets.human_assist_2_name_label), TRUE);
	    gtk_widget_set_sensitive(GTK_WIDGET(editor_data_widgets.human_assist_2_name), TRUE);
	case 1:
	    gtk_widget_set_sensitive(GTK_WIDGET(editor_data_widgets.human_assist_1_name_label), TRUE);
	    gtk_widget_set_sensitive(GTK_WIDGET(editor_data_widgets.human_assist_1_name), TRUE);
	default:
	    break;
    }

    return;
}

void on_human_assist_1_name_changed(void *widget, gpointer user_data)
{
    return;
}

void on_human_assist_2_name_changed(void *widget, gpointer user_data)
{
    return;
}

void on_human_assist_3_name_changed(void *widget, gpointer user_data)
{
    return;
}

void on_human_assist_4_name_changed(void *widget, gpointer user_data)
{
    return;
}

void on_human_has_displacement_toggled(void *widget, gpointer user_data)
{
    /* Get relative displacement check button state (TRUE if checked, FALSE if not)*/
    gboolean state = GTK_CHECK_BUTTON_GET_ACTIVE(widget);

    /* Set value of relevant widgets */
    gtk_widget_set_sensitive(GTK_WIDGET(editor_data_widgets.human_ref_obj_name_label), state);
    gtk_widget_set_sensitive(GTK_WIDGET(editor_data_widgets.human_ref_obj_name), state);
    gtk_widget_set_sensitive(GTK_WIDGET(editor_data_widgets.human_displacement_label), state);

    /* NOTE:
     * This because the "human displacement direction" can only be
     * "run_towards" for now.
     * See simop.c: else if(human->flags & SAR_HUMAN_FLAG_RUN_AWAY)
     */
    if(state)
    {
	/* Force the selected item to "run_towards" */
	EditorGtkItemChooserSetSelectedItemFromString(
		GTK_WIDGET(editor_data_widgets.human_displacement), "run_towards"
	    );

	/* Set the "human displacement direction" chooser widget UNsensitive */
	gtk_widget_set_sensitive(GTK_WIDGET(editor_data_widgets.human_displacement), FALSE);
    }
    else
    {
	/* Set no selected item */
	EditorGtkItemChooserSetSelectedItemFromString(
		GTK_WIDGET(editor_data_widgets.human_displacement), NULL
	    );

	/* Set the "human displacement direction" chooser widget UNsensitive */
	gtk_widget_set_sensitive(GTK_WIDGET(editor_data_widgets.human_displacement), FALSE);
    }

    return;
}

void on_human_ref_obj_changed(void *widget, gpointer user_data)
{
    return;
}

void on_human_displacement_changed(void *widget, gpointer user_data)
{
    return;
}


/*
 * Model file chooser, GTK3 version.
 */
#if GTK_MAJOR_VERSION == 3 && GTK_MINOR_VERSION >= 24
void on_model_file_name_changed(void *widget, gpointer user_data)
{
    char *file_name, *model_name;

    /* Get file name */
    file_name = STRDUP(gtk_file_chooser_get_filename(widget));

    /* Get model name from model file */
    model_name = STRDUP(GetModelNameFromModelFileName(file_name));
    if(model_name != NULL)
	gtk_entry_set_text(editor_data_widgets.model_name, (const char *)model_name);

    free(model_name);
    free(file_name);

/* TODO: general_object_name must be get from the scenery-loaded object data

    int s_length = sizeof("default: ") + sizeof(name);
    char *s = malloc(s_length);

    snprintf(s, s_length, "default: %s", name);

    /# Set general_object_name placeholder text. #/
    gtk_entry_set_placeholder_text(editor_data_widgets.general_object_name, (const char *)s);

    free(s);


    /# No "name" parameter found in model file? #/
    if(strcasecmp(cmd, "name"))
    {
	/# Reset general_object_name placeholder text. #/
	gtk_entry_set_placeholder_text(editor_data_widgets.general_object_name, "");

	/# Reset model_name. Note that as text value is empty, the
	 # GtkEntry widget placeholder-text will automatically be displayed. #/
	gtk_entry_set_text(editor_data_widgets.model_name, "");
    }
*/

    return;
}
/* GTK3 */
#endif

/*
 * Model file chooser, GTK4 version.
 */
#if GTK_MAJOR_VERSION == 4 && GTK_MINOR_VERSION >= 10
void
EditorGtk4SetFileWidgetsValues(const char *full_name)
{
    char *name, *model_name;

    name = strrchr(full_name, '/');
    name++;

    /* Set the button hidden label text (i.e. the embedded hidden label text) */
    gtk_label_set_text(editor_data_widgets.model_file_name_hidden, (const char *)full_name);

    /* Set the button text (i.e. the embedded label text) */
    gtk_label_set_text(editor_data_widgets.model_file_name_label, (const char *)name);

    /* Get model name from model file */
    model_name = strdup(GetModelNameFromModelFileName(full_name));
    if(model_name != NULL)
	gtk_editable_set_text(GTK_EDITABLE(editor_data_widgets.model_name), (const char *)model_name);

    return;
}

void
FileChoosed(GObject* source_object, GAsyncResult* res, gpointer user_data)
{
    char *full_path;
    GtkFileDialog *dialog = GTK_FILE_DIALOG(source_object);
    GFile *g_file = gtk_file_dialog_open_finish(dialog, res, NULL);

    if(g_file == NULL)
    {
	return;
    }

    full_path = g_file_get_path(g_file);

    EditorGtk4SetFileWidgetsValues(full_path);

    g_free(full_path);

    g_object_unref(g_file);

    return;
}

void on_model_file_name_clicked(void *widget, gpointer user_data)
{
    GtkFileDialog *file_dialog = gtk_file_dialog_new();

    /* Open the file chooser window, then call FileChoosed callback once the
     * chooser window is closed. */
    gtk_file_dialog_open(file_dialog, NULL, NULL, FileChoosed, (gpointer)user_data);

    return;
}
/* GTK4 */
#endif

void on_premod_type_changed(void *widget, gpointer user_data)
{
    char *selected_s;

    /* Get value */
    selected_s = EditorGtkItemChooserGetSelectedString(widget);

    if(!strcmp(selected_s, SAR_PREMODELED_BUILDING_S))
    {
	gtk_widget_set_visible(GTK_WIDGET(editor_data_widgets.premod_length_label), TRUE);
	gtk_widget_set_visible(GTK_WIDGET(editor_data_widgets.premod_length), TRUE);
	gtk_widget_set_visible(GTK_WIDGET(editor_data_widgets.premod_width_label), TRUE);
	gtk_widget_set_visible(GTK_WIDGET(editor_data_widgets.premod_width), TRUE);
	gtk_widget_set_visible(GTK_WIDGET(editor_data_widgets.premod_hazard_label), FALSE);
	gtk_widget_set_visible(GTK_WIDGET(editor_data_widgets.premod_hazard), FALSE);
	gtk_widget_set_visible(GTK_WIDGET(editor_data_widgets.premod_walls_tex_label), TRUE);
	gtk_widget_set_visible(GTK_WIDGET(editor_data_widgets.premod_walls_tex), TRUE);
	gtk_widget_set_visible(GTK_WIDGET(editor_data_widgets.premod_walls_tex_night_label), TRUE);
	gtk_widget_set_visible(GTK_WIDGET(editor_data_widgets.premod_walls_tex_night), TRUE);
	gtk_widget_set_visible(GTK_WIDGET(editor_data_widgets.premod_roof_tex_label), TRUE);
	gtk_widget_set_visible(GTK_WIDGET(editor_data_widgets.premod_roof_tex), TRUE);
    }
    else if(!strcmp(selected_s, SAR_PREMODELED_CONTROL_TOWER_S))
    {
	gtk_widget_set_visible(GTK_WIDGET(editor_data_widgets.premod_length_label), TRUE);
	gtk_widget_set_visible(GTK_WIDGET(editor_data_widgets.premod_length), TRUE);
	gtk_widget_set_visible(GTK_WIDGET(editor_data_widgets.premod_width_label), TRUE);
	gtk_widget_set_visible(GTK_WIDGET(editor_data_widgets.premod_width), TRUE);
	gtk_widget_set_visible(GTK_WIDGET(editor_data_widgets.premod_hazard_label), FALSE);
	gtk_widget_set_visible(GTK_WIDGET(editor_data_widgets.premod_hazard), FALSE);
	gtk_widget_set_visible(GTK_WIDGET(editor_data_widgets.premod_walls_tex_label), TRUE);
	gtk_widget_set_visible(GTK_WIDGET(editor_data_widgets.premod_walls_tex), TRUE);
	gtk_widget_set_visible(GTK_WIDGET(editor_data_widgets.premod_walls_tex_night_label), FALSE);
	gtk_widget_set_visible(GTK_WIDGET(editor_data_widgets.premod_walls_tex_night), FALSE);
	gtk_widget_set_visible(GTK_WIDGET(editor_data_widgets.premod_roof_tex_label), TRUE);
	gtk_widget_set_visible(GTK_WIDGET(editor_data_widgets.premod_roof_tex), TRUE);
    }
    else if(!strcmp(selected_s, SAR_PREMODELED_POWER_TRANSMISSION_TOWER_S) ||
	    !strcmp(selected_s, SAR_PREMODELED_TOWER_S) ||
	    !strcmp(selected_s, SAR_PREMODELED_RADIO_TOWER_S)
    )
    {
	gtk_widget_set_visible(GTK_WIDGET(editor_data_widgets.premod_length_label), FALSE);
	gtk_widget_set_visible(GTK_WIDGET(editor_data_widgets.premod_length), FALSE);
	gtk_widget_set_visible(GTK_WIDGET(editor_data_widgets.premod_width_label), FALSE);
	gtk_widget_set_visible(GTK_WIDGET(editor_data_widgets.premod_width), FALSE);
	gtk_widget_set_visible(GTK_WIDGET(editor_data_widgets.premod_hazard_label), TRUE);
	gtk_widget_set_visible(GTK_WIDGET(editor_data_widgets.premod_hazard), TRUE);
	gtk_widget_set_visible(GTK_WIDGET(editor_data_widgets.premod_walls_tex_label), FALSE);
	gtk_widget_set_visible(GTK_WIDGET(editor_data_widgets.premod_walls_tex), FALSE);
	gtk_widget_set_visible(GTK_WIDGET(editor_data_widgets.premod_walls_tex_night_label), FALSE);
	gtk_widget_set_visible(GTK_WIDGET(editor_data_widgets.premod_walls_tex_night), FALSE);
	gtk_widget_set_visible(GTK_WIDGET(editor_data_widgets.premod_roof_tex_label), FALSE);
	gtk_widget_set_visible(GTK_WIDGET(editor_data_widgets.premod_roof_tex), FALSE);
    }
    else if(!strcmp(selected_s, SAR_PREMODELED_HANGAR_S))
    {
	;
    }
    else if(!strcmp(selected_s, SAR_PREMODELED_UNKNOWN_S))
    {
	;
    }

    free(selected_s);

    return;
}

void on_range_changed(void *widget, gpointer user_data)
{
    /* Allow a positive or null double value from entry text */
    EditoGktEntryAllowDouble(widget, TRUE);
    return;
}

void on_premod_hazard_lights_changed(void *widget, gpointer user_data)
{
    /* Allow a positive or null int value as entry text */
    EditoGktEntryAllowInt(widget, TRUE);
    return;
}

void on_premod_walls_tex_changed(void *widget, gpointer user_data)
{
    return;
}

void on_premod_walls_tex_night_changed(void *widget, gpointer user_data)
{
    return;
}

void on_premod_roof_tex_changed(void *widget, gpointer user_data)
{
    return;
}

void on_runway_surface_type_changed(void *widget, gpointer user_data)
{
    return;
}

void on_runway_dashes_num_changed(void *widget, gpointer user_data)
{
    /* Allow a positive or null int value as entry text */
    EditoGktEntryAllowInt(widget, TRUE);
    return;
}

void on_runway_edge_light_spacing_changed(void *widget, gpointer user_data)
{
    /* Allow a positive or null double value from entry text */
    EditoGktEntryAllowDouble(widget, TRUE);
    return;
}

void on_runway_north_label_changed(void *widget, gpointer user_data)
{
    /* Filter entry text (forbid space characters) */
    EditorGktEntryFilterTextNoSpace(widget);
    return;
}

void on_runway_south_label_changed(void *widget, gpointer user_data)
{
    /* Filter entry text (forbid space characters) */
    EditorGktEntryFilterTextNoSpace(widget);
    return;
}

void on_runway_n_disp_thresh_changed(void *widget, gpointer user_data)
{
    /* Allow a positive or null double value from entry text */
    EditoGktEntryAllowDouble(widget, TRUE);
    return;
}

void on_runway_s_disp_thresh_changed(void *widget, gpointer user_data)
{
    /* Allow a positive or null double value from entry text */
    EditoGktEntryAllowDouble(widget, TRUE);
    return;
}

void on_runway_has_thresholds_toggled(void *widget, gpointer user_data)
{
    return;
}

void on_runway_has_borders_toggled(void *widget, gpointer user_data)
{
    return;
}

void on_runway_has_td_markers_toggled(void *widget, gpointer user_data)
{
    return;
}

void on_runway_has_mid_markers_toggled(void *widget, gpointer user_data)
{
    return;
}

void on_runway_has_north_gs_toggled(void *widget, gpointer user_data)
{
    return;
}

void on_runway_has_south_gs_toggled(void *widget, gpointer user_data)
{
    return;
}

void on_smoke_radius_start_changed(void *widget, gpointer user_data)
{
    /* Allow a positive or null double value from entry text */
    EditoGktEntryAllowDouble(widget, TRUE);
    return;
}

void on_smoke_radius_max_changed(void *widget, gpointer user_data)
{
    /* Allow a positive or null double value from entry text */
    EditoGktEntryAllowDouble(widget, TRUE);
    return;
}

void on_smoke_radius_rate_changed(void *widget, gpointer user_data)
{
    /* Allow a positive or null double value from entry text */
    EditoGktEntryAllowDouble(widget, TRUE);
    return;
}

void on_smoke_hide_at_max_changed(void *widget, gpointer user_data)
{
    /* Allow a positive or null double value from entry text */
    EditoGktEntryAllowDouble(widget, TRUE);
    return;
}

void on_smoke_respawn_int_changed(void *widget, gpointer user_data)
{
    /* Allow a positive or null int value as entry text */
    EditoGktEntryAllowInt(widget, TRUE);
    return;
}

void on_smoke_total_units_changed(void *widget, gpointer user_data)
{
    /* Allow a positive or null int value as entry text */
    EditoGktEntryAllowInt(widget, TRUE);
    return;
}

void on_smoke_color_code_changed(void *widget, gpointer user_data)
{
    return;
}

/* Called when an "Ok" button is clicked.
 * As needed, sends a command to SARCmdSceneEditor() in order to execute it.
 */
void on_button_ok_clicked(void *widget, gpointer user_data)
{
#define S_LENGTH 1024
#define REMAINING(s) (MAX(0, S_LENGTH - strlen(s) - 1))
    ///////////////////////////////////const sar_core_struct *core_ptr = (sar_core_struct *)user_data;
    sar_core_struct *core_ptr = (sar_core_struct *)user_data;
    sar_scenery_editor_struct *scn_ed = core_ptr->in_game_editor;
    editor_gtk_ui_struct *editor_gtk_ui = scn_ed->editor_gtk_ui;
    editor_object_data_struct *editor_obj_data;
    char *cmd_args = NULL;
    char *cmd_line = (char *)malloc((S_LENGTH + 1) * sizeof(char));

    cmd_line[0] = '\0';

    switch(scn_ed->current_action)
    {
	case EDITOR_ACTION_NONE:
	case EDITOR_ACTION_QUIT:
	case EDITOR_ACTION_QUIT_FROM_GTK:
	case EDITOR_ACTION_PRINT:
	    break;

	case EDITOR_ACTION_NEW:
	    /* Create a new temporary object data structure */
	    editor_obj_data = EditorObjectDataStructNew();

	    /* Set temporary structure data by reading the Gtk widgets datas */
	    EditorObjectDataStructSetFromGtkUi(editor_obj_data);

	    /* Set the first command line token as the command name */
	    switch(editor_obj_data->type)
	    {
		case SAR_OBJ_TYPE_GARBAGE:
		    break;

		case SAR_OBJ_TYPE_STATIC:
		case SAR_OBJ_TYPE_AUTOMOBILE:
		case SAR_OBJ_TYPE_WATERCRAFT:
		    strncat(cmd_line, "load_object ", REMAINING(cmd_line));
		    break;

		case SAR_OBJ_TYPE_AIRCRAFT:
		    break;

		case SAR_OBJ_TYPE_GROUND:
		    break;

		case SAR_OBJ_TYPE_RUNWAY:
		    strncat(cmd_line, "create_runway ", REMAINING(cmd_line));
		    break;

		case SAR_OBJ_TYPE_HELIPAD:
		    strncat(cmd_line, "create_helipad ", REMAINING(cmd_line));
		    break;

		case SAR_OBJ_TYPE_HUMAN:
		    strncat(cmd_line, "create_human ", REMAINING(cmd_line));
		    break;

		case SAR_OBJ_TYPE_SMOKE:
		    strncat(cmd_line, "create_smoke ", REMAINING(cmd_line));
		    break;

		case SAR_OBJ_TYPE_FIRE:
		    strncat(cmd_line, "create_fire ", REMAINING(cmd_line));
		    break;

		case SAR_OBJ_TYPE_EXPLOSION:
		case SAR_OBJ_TYPE_CHEMICAL_SPRAY:
		case SAR_OBJ_TYPE_FUELTANK:
		    break;

		case SAR_OBJ_TYPE_PREMODELED:
		    strncat(cmd_line, "create_premodeled ", REMAINING(cmd_line));
		    break;

		default:
		    break;
	    }

	    /* Command name set? */
	    if(cmd_line[0] != '\0')
	    {
		/* Get the SARCmdSceneEditor() command line parameters */
		cmd_args = DoCmdLineFromObjectDataStruct(editor_obj_data);

		/* Add parameter(s) to command name */
		strncat(cmd_line, cmd_args, REMAINING(cmd_line));

		/* Send command line to SARCmdSceneEditor() */
		SARCmdSceneEditor((void *)core_ptr, cmd_line, editor_gtk_ui->cmd_flags);

		free(cmd_args);

		set_children_sensitive_by_name(GTK_WIDGET(editor_menu_widgets.grid), "obj_new", FALSE);
		set_children_sensitive_by_name(GTK_WIDGET(editor_menu_widgets.grid), "obj_current", TRUE);
		set_children_sensitive_by_name(GTK_WIDGET(editor_menu_widgets.grid), "obj_closer", FALSE);
	    }

	    /* Free temporary structure */
	    EditorObjectDataStructFree(editor_obj_data);

	    /* Show general frame pos and dir widgets because they
	     * have been hidden by the on_button_new_clicked() callback.
	     */
	    gtk_widget_set_visible(GTK_WIDGET(editor_data_widgets.general_pos_label), TRUE);
	    gtk_widget_set_visible(GTK_WIDGET(editor_data_widgets.general_pos_x), TRUE);
	    gtk_widget_set_visible(GTK_WIDGET(editor_data_widgets.general_pos_y), TRUE);
	    gtk_widget_set_visible(GTK_WIDGET(editor_data_widgets.general_pos_z), TRUE);
	    gtk_widget_set_visible(GTK_WIDGET(editor_data_widgets.general_dir_label), TRUE);
	    gtk_widget_set_visible(GTK_WIDGET(editor_data_widgets.general_dir_heading), TRUE);
	    gtk_widget_set_visible(GTK_WIDGET(editor_data_widgets.general_dir_pitch), TRUE);
	    gtk_widget_set_visible(GTK_WIDGET(editor_data_widgets.general_dir_bank), TRUE);

	    break;

	case EDITOR_ACTION_SET:
	case EDITOR_ACTION_UNLOAD:
	case EDITOR_ACTION_COPY:
	    break;

	case EDITOR_ACTION_MODIFY:
	    /* Create a new object data temporary structure */
	    editor_obj_data = EditorObjectDataStructNew();

	    /* Set temporary structure data by reading the Gtk widgets datas */
	    EditorObjectDataStructSetFromGtkUi(editor_obj_data);

	    /* Generate the SARCmdSceneEditor() command line */
	    cmd_args = DoCmdLineFromObjectDataStruct(editor_obj_data);

	    /* editor_obj_data struture will be needed by the "modify" command,
	     * save its pointer.
	     */
	    editor_gtk_ui->temp_obj_data = (editor_object_data_struct *)editor_obj_data;

	    /* Send the new parameters to SARCmdSceneEditor().
	     * Note that a "modify" command has been sent earlier
	     * by on_button_modify_clicked().
	     */
	    SARCmdSceneEditor((void *)core_ptr, cmd_args, editor_gtk_ui->cmd_flags);

	    /* Free temporary structure */
	    EditorObjectDataStructFree(editor_obj_data);

	    editor_gtk_ui->temp_obj_data = NULL;

	    set_children_sensitive_by_name(GTK_WIDGET(editor_menu_widgets.grid), "obj_new", TRUE);
	    set_children_sensitive_by_name(GTK_WIDGET(editor_menu_widgets.grid), "obj_current", FALSE);
	    set_children_sensitive_by_name(GTK_WIDGET(editor_menu_widgets.grid), "obj_closer", TRUE);

	    break;

	case EDITOR_ACTION_INFO:
	case EDITOR_ACTION_INFO_NEXT:
	    set_children_sensitive_by_name(GTK_WIDGET(editor_menu_widgets.grid), "obj_new", TRUE);

	    break;

	case EDITOR_ACTION_MOVE:
	    break;

	case EDITOR_ACTION_REMOVE:
	    break;
    }

    free(cmd_line);

#if GTK_MAJOR_VERSION == 3 && GTK_MINOR_VERSION >= 24
    gtk_widget_hide(GTK_WIDGET(editor_data_widgets.window));
#endif
#if GTK_MAJOR_VERSION == 4 && GTK_MINOR_VERSION >= 10
    gtk_widget_set_visible(editor_data_widgets.window, FALSE);
#endif

    /* Unset menu window focus */
    EditorGtkSetAcceptFocus(editor_menu_widgets.window, FALSE);

    /* Note that focus will be given back to the toplevel sar2 window
     * by calling the GwSetWindowFocusToSar2Window() function from the
     * SARCmdSceneEditor() function.
     */

    scn_ed->current_action = EDITOR_ACTION_NONE;

    return;
#undef S_LENGTH
#undef REMAINING
}

void on_button_apply_clicked(void *widget, gpointer user_data)
{
g_print("button_apply_clicked\n");
    const sar_core_struct *core_ptr = (sar_core_struct *)user_data;
    sar_scenery_editor_struct *scn_ed = core_ptr->in_game_editor;


    {
    fprintf(stderr, "%s:%d: Current action: %d\n", __FILE__, __LINE__, scn_ed->current_action);
    switch(scn_ed->current_action)
    {
	case EDITOR_ACTION_NONE:
	    fprintf(stderr, "EDITOR_ACTION_NONE\n");
	    break;
	case EDITOR_ACTION_QUIT:
	    fprintf(stderr, "EDITOR_ACTION_QUIT\n");
	    break;
	case EDITOR_ACTION_QUIT_FROM_GTK:
	    fprintf(stderr, "EDITOR_ACTION_QUIT_FROM_GTK\n");
	    break;
	case EDITOR_ACTION_PRINT:
	    fprintf(stderr, "EDITOR_ACTION_PRINT\n");
	    break;
	case EDITOR_ACTION_NEW:
	    fprintf(stderr, "EDITOR_ACTION_NEW\n");
	    break;
	case EDITOR_ACTION_SET:
	    fprintf(stderr, "EDITOR_ACTION_SET\n");
	    break;
	case EDITOR_ACTION_UNLOAD:
	    fprintf(stderr, "EDITOR_ACTION_UNLOAD\n");
	    break;
	case EDITOR_ACTION_COPY:
	    fprintf(stderr, "EDITOR_ACTION_COPY\n");
	    break;
	case EDITOR_ACTION_MODIFY:
	    fprintf(stderr, "EDITOR_ACTION_MODIFY\n");
	    break;
	case EDITOR_ACTION_INFO:
	    fprintf(stderr, "EDITOR_ACTION_INFO\n");
	    break;
	case EDITOR_ACTION_INFO_NEXT:
	    fprintf(stderr, "EDITOR_ACTION_INFO_NEXT\n");
	    break;
	case EDITOR_ACTION_MOVE:
	    fprintf(stderr, "EDITOR_ACTION_MOVE\n");
	    break;
	case EDITOR_ACTION_REMOVE:
	    fprintf(stderr, "EDITOR_ACTION_REMOVE\n");
	    break;
    }
    }


    // FIXME
    editor_object_data_struct *editor_obj_data = scn_ed->modification_list[4]->obj_data_new;

    switch(scn_ed->current_action)
    {
	case EDITOR_ACTION_NONE:
	case EDITOR_ACTION_QUIT:
	case EDITOR_ACTION_QUIT_FROM_GTK:
	case EDITOR_ACTION_PRINT:
	    break;

	case EDITOR_ACTION_NEW:
	    EditorObjectDataStructSetFromGtkUi(editor_obj_data);
	    DoCmdLineFromObjectDataStruct(editor_obj_data);
	    break;

	case EDITOR_ACTION_SET:
	case EDITOR_ACTION_UNLOAD:
	case EDITOR_ACTION_COPY:
	    break;

	case EDITOR_ACTION_MODIFY:
	    EditorObjectDataStructSetFromGtkUi(editor_obj_data);
	    DoCmdLineFromObjectDataStruct(editor_obj_data);
	    break;

	case EDITOR_ACTION_INFO:
	case EDITOR_ACTION_INFO_NEXT:
	case EDITOR_ACTION_MOVE:
	case EDITOR_ACTION_REMOVE:
	    break;
    }

    /* Unset menu window focus */
    EditorGtkSetAcceptFocus(editor_menu_widgets.window, FALSE);

    /* Note that focus will be given back to the toplevel sar2 window
     * by calling the GwSetWindowFocusToSar2Window() function from the
     * SARCmdSceneEditor() function.
     */

    /* Do not set a new value to scn_ed->current_action */

    return;
}

void on_button_cancel_clicked(void *widget, gpointer user_data)
{
    const sar_core_struct *core_ptr = (sar_core_struct *)user_data;
    sar_scenery_editor_struct *scn_ed = core_ptr->in_game_editor;

#if GTK_MAJOR_VERSION == 3 && GTK_MINOR_VERSION >= 24
    gtk_widget_hide(GTK_WIDGET(editor_data_widgets.window));
#endif
#if GTK_MAJOR_VERSION == 4 && GTK_MINOR_VERSION >= 10
    gtk_widget_set_visible(editor_data_widgets.window, FALSE);
#endif

    switch(scn_ed->current_action)
    {
	case EDITOR_ACTION_NONE:
	case EDITOR_ACTION_QUIT:
	case EDITOR_ACTION_QUIT_FROM_GTK:
	case EDITOR_ACTION_PRINT:
	    break;

	case EDITOR_ACTION_NEW:
	    //EditorObjectDataStructSetFromGtkUi(editor_obj_data);
	    //DoCmdLineFromObjectDataStruct(editor_obj_data);

	    /* Show general frame pos and dir widgets because they
	     * have been hidden by the on_button_new_clicked() callback.
	     */
	    gtk_widget_set_visible(GTK_WIDGET(editor_data_widgets.general_pos_label), TRUE);
	    gtk_widget_set_visible(GTK_WIDGET(editor_data_widgets.general_pos_x), TRUE);
	    gtk_widget_set_visible(GTK_WIDGET(editor_data_widgets.general_pos_y), TRUE);
	    gtk_widget_set_visible(GTK_WIDGET(editor_data_widgets.general_pos_z), TRUE);
	    gtk_widget_set_visible(GTK_WIDGET(editor_data_widgets.general_dir_label), TRUE);
	    gtk_widget_set_visible(GTK_WIDGET(editor_data_widgets.general_dir_heading), TRUE);
	    gtk_widget_set_visible(GTK_WIDGET(editor_data_widgets.general_dir_pitch), TRUE);
	    gtk_widget_set_visible(GTK_WIDGET(editor_data_widgets.general_dir_bank), TRUE);
	    break;

	case EDITOR_ACTION_SET:
	case EDITOR_ACTION_UNLOAD:
	case EDITOR_ACTION_COPY:
	    break;

	case EDITOR_ACTION_MODIFY:
	    set_children_sensitive_by_name(GTK_WIDGET(editor_menu_widgets.grid), "obj_new", TRUE);
	    set_children_sensitive_by_name(GTK_WIDGET(editor_menu_widgets.grid), "obj_current", FALSE);
	    set_children_sensitive_by_name(GTK_WIDGET(editor_menu_widgets.grid), "obj_closer", TRUE);
	    break;

	case EDITOR_ACTION_INFO:
	case EDITOR_ACTION_INFO_NEXT:
	    set_children_sensitive_by_name(GTK_WIDGET(editor_menu_widgets.grid), "obj_new", TRUE);

	    break;

	case EDITOR_ACTION_MOVE:
	    break;

	case EDITOR_ACTION_REMOVE:
	    break;
    }

    /* Unset menu window focus */
    EditorGtkSetAcceptFocus(editor_menu_widgets.window, FALSE);

    /* Note that focus will be given back to the toplevel sar2 window
     * by calling the GwSetWindowFocusToSar2Window() function from the
     * SARCmdSceneEditor() function.
     */

    scn_ed->current_action = EDITOR_ACTION_NONE;

    return;
}


/*
 * Menu window buttons
 */

void on_button_quit_clicked(void *widget, gpointer user_data)
{
    const sar_core_struct *core_ptr = (sar_core_struct *)user_data;
    sar_scenery_editor_struct *scn_ed = core_ptr->in_game_editor;
    //const gw_display_struct *display = core_ptr->display;
    //const editor_gtk_ui_struct *editor_gtk_ui = scn_ed->editor_gtk_ui;

    /* Already quitting? */
    if(scn_ed->current_action == EDITOR_ACTION_QUIT_FROM_GTK)
	return;

    /* Set action as "user asked to quit editor from the GTK UI".
     * Note that this action will be executed from the SARManage() function by
     * calling the SARCmdSceneEditor() "scnedit off" command. If the "scnedit
     * off" command is called from the current on_button_quit_clicked()
     * function, Sar2 will freeze. I suppose that it's because the
     * on_button_quit_clicked() callback tries to return to the GTK application
     * _after_ it had been stopped by the "scnedit off" command.
     */
    scn_ed->current_action = EDITOR_ACTION_QUIT_FROM_GTK;

    return;
}

void on_button_print_clicked(void *widget, gpointer user_data)
{
    const sar_core_struct *core_ptr = (sar_core_struct *)user_data;
    sar_scenery_editor_struct *scn_ed = core_ptr->in_game_editor;
    const editor_gtk_ui_struct *editor_gtk_ui = scn_ed->editor_gtk_ui;

    scn_ed->current_action = EDITOR_ACTION_PRINT;

    /* Send the "print" command to SARCmdSceneEditor() */
    SARCmdSceneEditor((void *)core_ptr, "print", editor_gtk_ui->cmd_flags);

    /* Unset menu window focus */
    EditorGtkSetAcceptFocus(editor_menu_widgets.window, FALSE);

    /* Note that focus will be given back to the toplevel sar2 window
     * by calling the GwSetWindowFocusToSar2Window() function from the
     * SARCmdSceneEditor() function.
     */

    return;
}

/* Shows a clean editor object data Gtk window. Once user has filled it,
 * entered data will be treated by on_button_ok_clicked().
 */
void on_button_new_clicked(void *widget, gpointer user_data)
{
    const sar_core_struct *core_ptr = (sar_core_struct *)user_data;
    sar_scenery_editor_struct *scn_ed = core_ptr->in_game_editor;
    //editor_gtk_ui_struct *editor_gtk_ui = scn_ed->editor_gtk_ui;

    /* Set current action type */
    scn_ed->current_action = EDITOR_ACTION_NEW;

    /* Hide object name widgets because we don't need them for now */
    gtk_widget_set_visible(GTK_WIDGET(editor_data_widgets.general_object_name_label), FALSE);
    gtk_widget_set_visible(GTK_WIDGET(editor_data_widgets.general_object_name), FALSE);

    /* Hide general frame pos and dir widgets because they
     * will be set by the player position and direction.
     */
    gtk_widget_set_visible(GTK_WIDGET(editor_data_widgets.general_pos_label), FALSE);
    gtk_widget_set_visible(GTK_WIDGET(editor_data_widgets.general_pos_x), FALSE);
    gtk_widget_set_visible(GTK_WIDGET(editor_data_widgets.general_pos_y), FALSE);
    gtk_widget_set_visible(GTK_WIDGET(editor_data_widgets.general_pos_z), FALSE);
    gtk_widget_set_visible(GTK_WIDGET(editor_data_widgets.general_dir_label), FALSE);
    gtk_widget_set_visible(GTK_WIDGET(editor_data_widgets.general_dir_heading), FALSE);
    gtk_widget_set_visible(GTK_WIDGET(editor_data_widgets.general_dir_pitch), FALSE);
    gtk_widget_set_visible(GTK_WIDGET(editor_data_widgets.general_dir_bank), FALSE);

    /* Show general frame and set its grid sensitive */
    gtk_widget_set_visible(GTK_WIDGET(editor_data_widgets.general_frame), TRUE);
    gtk_widget_set_sensitive(
	    GTK_WIDGET(editor_data_widgets.general_frame_grid), TRUE);

    /* Ensure to set the type chooser widget as sensitive */
    gtk_widget_set_sensitive(
	    GTK_WIDGET(editor_data_widgets.general_type), TRUE
	);

    /* Set object type choice list to 'no selection' */
#if GTK_MAJOR_VERSION == 3 && GTK_MINOR_VERSION >= 24
    gtk_combo_box_set_active(GTK_COMBO_BOX(editor_data_widgets.general_type), -1);
#endif
#if GTK_MAJOR_VERSION == 4 && GTK_MINOR_VERSION >= 10
    gtk_drop_down_set_selected(editor_data_widgets.general_type, -1);
#endif

    /* Clean preselected folder name */
#if GTK_MAJOR_VERSION == 3 && GTK_MINOR_VERSION >= 24
/*
	gtk_file_chooser_set_filename(
		GTK_FILE_CHOOSER(editor_data_widgets.model_file_name),
		"/foolder/foolder/foo_object.3d"
	    );
*/
#endif
#if GTK_MAJOR_VERSION == 4 && GTK_MINOR_VERSION >= 10
/* TODO
	GFile *folder = g_file_new_for_path(const char* path);
	gtk_file_dialog_set_initial_folder(
		GTK_FILE_DIALOG(dialog),
		GFile* folder
	);
*/
#endif

    /* Hide all object-specific frames.
     * Note that frame visibility and frame grid sensivity will be set to TRUE
     * by the on_general_type_changed() callback.
     */
    gtk_widget_set_visible(GTK_WIDGET(editor_data_widgets.general_frame), TRUE);
    gtk_widget_set_visible(GTK_WIDGET(editor_data_widgets.fire_frame), FALSE);
    gtk_widget_set_visible(GTK_WIDGET(editor_data_widgets.helipad_frame), FALSE);
    gtk_widget_set_visible(GTK_WIDGET(editor_data_widgets.human_frame), FALSE);
    gtk_widget_set_visible(GTK_WIDGET(editor_data_widgets.model_frame), FALSE);
    gtk_widget_set_visible(GTK_WIDGET(editor_data_widgets.premodeled_frame), FALSE);
    gtk_widget_set_visible(GTK_WIDGET(editor_data_widgets.runway_frame), FALSE);
    gtk_widget_set_visible(GTK_WIDGET(editor_data_widgets.smoke_frame), FALSE);

    /* Set human choice list widgets default value to the list first item */
#if GTK_MAJOR_VERSION == 3 && GTK_MINOR_VERSION >= 24
    gtk_combo_box_set_active(GTK_COMBO_BOX(editor_data_widgets.human_type_name), 0);
    gtk_combo_box_set_active(GTK_COMBO_BOX(editor_data_widgets.human_assist_1_name), 0);
    gtk_combo_box_set_active(GTK_COMBO_BOX(editor_data_widgets.human_assist_2_name), 0);
    gtk_combo_box_set_active(GTK_COMBO_BOX(editor_data_widgets.human_assist_3_name), 0);
    gtk_combo_box_set_active(GTK_COMBO_BOX(editor_data_widgets.human_assist_4_name), 0);
#endif
#if GTK_MAJOR_VERSION == 4 && GTK_MINOR_VERSION >= 10
    gtk_drop_down_set_selected(editor_data_widgets.human_type_name, 0);
    gtk_drop_down_set_selected(editor_data_widgets.human_assist_1_name, 0);
    gtk_drop_down_set_selected(editor_data_widgets.human_assist_2_name, 0);
    gtk_drop_down_set_selected(editor_data_widgets.human_assist_3_name, 0);
    gtk_drop_down_set_selected(editor_data_widgets.human_assist_4_name, 0);
#endif

    /* Show the ok, apply, and cancel buttons */
#if GTK_MAJOR_VERSION == 3 && GTK_MINOR_VERSION >= 24
    gtk_widget_show(GTK_WIDGET(editor_data_widgets.button_ok));
    gtk_widget_show(GTK_WIDGET(editor_data_widgets.button_apply));
    gtk_widget_show(GTK_WIDGET(editor_data_widgets.button_cancel));
#endif
#if GTK_MAJOR_VERSION == 4 && GTK_MINOR_VERSION >= 10
    gtk_widget_set_visible(GTK_WIDGET(editor_data_widgets.button_ok), TRUE);
    gtk_widget_set_visible(GTK_WIDGET(editor_data_widgets.button_apply), TRUE);
    gtk_widget_set_visible(GTK_WIDGET(editor_data_widgets.button_cancel), TRUE);
#endif

    /* Show object data window */
#if GTK_MAJOR_VERSION == 3 && GTK_MINOR_VERSION >= 24
    gtk_widget_show(GTK_WIDGET(editor_data_widgets.window));
#endif
#if GTK_MAJOR_VERSION == 4 && GTK_MINOR_VERSION >= 10
    gtk_widget_set_visible(editor_data_widgets.window, TRUE);
#endif

    /* TODO: Update reference object names choice list */
    // if(new object has a name) EditorGtkItemChooserSetRefObjsNameList(core_ptr);

    /* Resize the window */
    gint natural_height = 0, natural_width = 0;
#if GTK_MAJOR_VERSION == 3 && GTK_MINOR_VERSION >= 24
    gtk_widget_get_preferred_height(GTK_WIDGET(editor_data_widgets.main_box), NULL, &natural_height);
    gtk_widget_get_preferred_width(GTK_WIDGET(editor_data_widgets.main_box), NULL, &natural_width);
    gtk_window_resize(GTK_WINDOW(editor_data_widgets.window), natural_width, natural_height);
#endif
#if GTK_MAJOR_VERSION == 4 && GTK_MINOR_VERSION >= 10
    GtkRequisition minimum_size, natural_size;
    gtk_widget_get_preferred_size(GTK_WIDGET(editor_data_widgets.main_box), &minimum_size, &natural_size);
    natural_width = natural_size.width;
    natural_height = natural_size.height;
    gtk_window_set_default_size(GTK_WINDOW(editor_data_widgets.main_box), natural_width, natural_height);
#endif

    /* Unset menu window focus */
    EditorGtkSetAcceptFocus(editor_menu_widgets.window, FALSE);

    /* Note that focus will be given back to the toplevel sar2 window
     * by calling the GwSetWindowFocusToSar2Window() function from the
     * SARCmdSceneEditor() function.
     */

    return;
}

void on_button_set_clicked(void *widget, gpointer user_data)
{
    const sar_core_struct *core_ptr = (sar_core_struct *)user_data;
    sar_scenery_editor_struct *scn_ed = core_ptr->in_game_editor;
    const editor_gtk_ui_struct *editor_gtk_ui = scn_ed->editor_gtk_ui;
    Boolean was_in_move = False;

    scn_ed->current_action = EDITOR_ACTION_SET;

    was_in_move = scn_ed->in_move_state;

    /* Send the "set" command to SARCmdSceneEditor() */
    SARCmdSceneEditor((void *)core_ptr, "set", editor_gtk_ui->cmd_flags);

    if(!was_in_move)
    {
	set_children_sensitive_by_name(GTK_WIDGET(editor_menu_widgets.grid), "obj_new", FALSE);
	set_children_sensitive_by_name(GTK_WIDGET(editor_menu_widgets.grid), "obj_current", TRUE);
	set_children_sensitive_by_name(GTK_WIDGET(editor_menu_widgets.grid), "obj_closer", FALSE);
    }
    else
    {
	set_children_sensitive_by_name(GTK_WIDGET(editor_menu_widgets.grid), "obj_new", TRUE);
	set_children_sensitive_by_name(GTK_WIDGET(editor_menu_widgets.grid), "obj_current", FALSE);
	set_children_sensitive_by_name(GTK_WIDGET(editor_menu_widgets.grid), "obj_closer", TRUE);
    }

    /* Unset menu window focus */
    EditorGtkSetAcceptFocus(editor_menu_widgets.window, FALSE);

    /* Note that focus will be given back to the toplevel sar2 window
     * by calling the GwSetWindowFocusToSar2Window() function from the
     * SARCmdSceneEditor() function.
     */

    return;
}

void on_button_copy_clicked(void *widget, gpointer user_data)
{
    const sar_core_struct *core_ptr = (sar_core_struct *)user_data;
    sar_scenery_editor_struct *scn_ed = core_ptr->in_game_editor;
    const editor_gtk_ui_struct *editor_gtk_ui = scn_ed->editor_gtk_ui;

    scn_ed->current_action = EDITOR_ACTION_COPY;

    /* Send the "copy" command to SARCmdSceneEditor() */
    SARCmdSceneEditor((void *)core_ptr, "copy", editor_gtk_ui->cmd_flags);

    set_children_sensitive_by_name(GTK_WIDGET(editor_menu_widgets.grid), "obj_new", FALSE);
    set_children_sensitive_by_name(GTK_WIDGET(editor_menu_widgets.grid), "obj_current", TRUE);
    set_children_sensitive_by_name(GTK_WIDGET(editor_menu_widgets.grid), "obj_closer", FALSE);

    /* Unset menu window focus */
    EditorGtkSetAcceptFocus(editor_menu_widgets.window, FALSE);

    /* Note that focus will be given back to the toplevel sar2 window
     * by calling the GwSetWindowFocusToSar2Window() function from the
     * SARCmdSceneEditor() function.
     */

    return;
}

void on_button_info_clicked(void *widget, gpointer user_data)
{
    const sar_core_struct *core_ptr = (sar_core_struct *)user_data;
    sar_scenery_editor_struct *scn_ed = core_ptr->in_game_editor;
    const editor_gtk_ui_struct *editor_gtk_ui = scn_ed->editor_gtk_ui;
    int picked_obj_num;
    editor_object_data_struct *editor_obj_data;

    scn_ed->current_action = EDITOR_ACTION_INFO;

    /* Send an "info" command to SARCmdSceneEditor().
     * Note that editor object data structure will be filled by
     * the "info" command.
     */
    SARCmdSceneEditor((void *)core_ptr, "info", editor_gtk_ui->cmd_flags);

    /* Get editor_obj_data structure pointer */
    picked_obj_num = editor_gtk_ui->picked_obj_num;
    editor_modified_object_struct *modification = scn_ed->modification_list[picked_obj_num];
    editor_obj_data = modification->obj_data_new;

    EditorGtkUiShowInfoWindow(core_ptr, picked_obj_num, (const editor_object_data_struct *)editor_obj_data);

    /* Unset menu window focus */
    EditorGtkSetAcceptFocus(editor_menu_widgets.window, FALSE);

    /* Note that focus will be given back to the toplevel sar2 window
     * by calling the GwSetWindowFocusToSar2Window() function from the
     * SARCmdSceneEditor() function.
     */

    return;
}

void on_button_info_next_clicked(void *widget, gpointer user_data)
{
    const sar_core_struct *core_ptr = (sar_core_struct *)user_data;
    sar_scenery_editor_struct *scn_ed = core_ptr->in_game_editor;
    const editor_gtk_ui_struct *editor_gtk_ui = scn_ed->editor_gtk_ui;
    int picked_obj_num;
    editor_object_data_struct *editor_obj_data;

    scn_ed->current_action = EDITOR_ACTION_INFO_NEXT;

    /* Send an "ifn" (info next) command to SARCmdSceneEditor().
     * Note that editor object data structure will be filled by
     * the "ifn" command.
     */
    SARCmdSceneEditor((void *)core_ptr, "ifn", editor_gtk_ui->cmd_flags);

    /* Get editor_obj_data structure pointer */
    picked_obj_num = editor_gtk_ui->picked_obj_num;
    editor_modified_object_struct *modification = scn_ed->modification_list[picked_obj_num];
    editor_obj_data = modification->obj_data_new;

    EditorGtkUiShowInfoWindow(core_ptr, picked_obj_num, (const editor_object_data_struct *)editor_obj_data);

    /* Unset menu window focus */
    EditorGtkSetAcceptFocus(editor_menu_widgets.window, FALSE);

    /* Note that focus will be given back to the toplevel sar2 window
     * by calling the GwSetWindowFocusToSar2Window() function from the
     * SARCmdSceneEditor() function.
     */

    return;
}

void on_button_unload_clicked(void *widget, gpointer user_data)
{
    const sar_core_struct *core_ptr = (sar_core_struct *)user_data;
    sar_scenery_editor_struct *scn_ed = core_ptr->in_game_editor;
    const editor_gtk_ui_struct *editor_gtk_ui = scn_ed->editor_gtk_ui;

    scn_ed->current_action = EDITOR_ACTION_UNLOAD;

    /* Send the "unload" command to SARCmdSceneEditor() */
    SARCmdSceneEditor((void *)core_ptr, "unload", editor_gtk_ui->cmd_flags);

    set_children_sensitive_by_name(GTK_WIDGET(editor_menu_widgets.grid), "obj_new", TRUE);
    set_children_sensitive_by_name(GTK_WIDGET(editor_menu_widgets.grid), "obj_current", FALSE);
    set_children_sensitive_by_name(GTK_WIDGET(editor_menu_widgets.grid), "obj_closer", TRUE);

    /* Unset menu window focus */
    EditorGtkSetAcceptFocus(editor_menu_widgets.window, FALSE);

    /* Note that focus will be given back to the toplevel sar2 window
     * by calling the GwSetWindowFocusToSar2Window() function from the
     * SARCmdSceneEditor() function.
     */

    return;
}

void on_button_modify_clicked(void *widget, gpointer user_data)
{
    const sar_core_struct *core_ptr = (sar_core_struct *)user_data;
    const sar_scene_struct *scene = core_ptr->scene;
    sar_scenery_editor_struct *scn_ed = core_ptr->in_game_editor;
    editor_gtk_ui_struct *editor_gtk_ui = scn_ed->editor_gtk_ui;
    editor_modified_object_struct *modification;
    int picked_obj_num;
    editor_object_data_struct *editor_obj_data;
    float distance;
    char title[31+1];

    scn_ed->current_action = EDITOR_ACTION_MODIFY;

    /* Initiate a "modify" command to SARCmdSceneEditor().
     * Note that:
     *  - editor object data structure will be filled by the "modify" command
     *  - the parameters modified from the GTK window will be sent later by
     *    the on_button_ok_clicked() callback.
     */
    SARCmdSceneEditor((void *)core_ptr, "modify", editor_gtk_ui->cmd_flags);

    /* Get editor_obj_data structure pointer */
    picked_obj_num = editor_gtk_ui->picked_obj_num;
    modification = scn_ed->modification_list[picked_obj_num];
    editor_obj_data = modification->obj_data_new;

    /* Should never happen */
    if(scene == NULL)
	return;

    /* 3D distance between triedron (i.e. player) and #obj_num object */
	distance = (float)SFMHypot3(
	    scene->player_obj_ptr->pos.x - editor_obj_data->pos.x,
	    scene->player_obj_ptr->pos.y - editor_obj_data->pos.y,
	    scene->player_obj_ptr->pos.z - SFMFeetToMeters((double)editor_obj_data->pos.z)
	);

    /* Set window title */
    snprintf(title, 31, "Object #%05d @%.3fm", picked_obj_num, distance);
    gtk_window_set_title(GTK_WINDOW(editor_data_widgets.window), (const gchar*)title);

    /* Set general frame grid as sensitive */
    gtk_widget_set_sensitive(
	    GTK_WIDGET(editor_data_widgets.general_frame_grid), TRUE
	);

    /* Set the type chooser widget as unsensitive */
    gtk_widget_set_sensitive(
	    GTK_WIDGET(editor_data_widgets.general_type), FALSE
	);

    /* Set visibility of all frames as needed */
    on_general_type_changed(editor_data_widgets.general_type, (gpointer)core_ptr);

    EditorGtkUiSetFromObjectDataStruct((const editor_object_data_struct *)editor_obj_data);

    switch(editor_obj_data->type)
    {
	case SAR_OBJ_TYPE_GARBAGE:
	    break;

	case SAR_OBJ_TYPE_STATIC:
	case SAR_OBJ_TYPE_AUTOMOBILE:
	case SAR_OBJ_TYPE_WATERCRAFT:
	case SAR_OBJ_TYPE_AIRCRAFT:
	case SAR_OBJ_TYPE_GROUND:
	    gtk_widget_set_sensitive(
		GTK_WIDGET(editor_data_widgets.model_frame_grid), TRUE);
	    break;

	case SAR_OBJ_TYPE_RUNWAY:
	    gtk_widget_set_sensitive(
		GTK_WIDGET(editor_data_widgets.runway_frame_grid), TRUE);
	    break;

	case SAR_OBJ_TYPE_HELIPAD:
	    gtk_widget_set_sensitive(
		GTK_WIDGET(editor_data_widgets.helipad_frame_grid), TRUE);

		/* Is this object referenced to another one? */
		if(editor_obj_data->ref_obj_name != NULL && editor_obj_data->ref_obj_name[0] != '\0')
		{
		    /* Set position and direction widgets unsensitive */
		    gtk_widget_set_sensitive(GTK_WIDGET(editor_data_widgets.general_pos_x), FALSE);
		    gtk_widget_set_sensitive(GTK_WIDGET(editor_data_widgets.general_pos_y), FALSE);
		    gtk_widget_set_sensitive(GTK_WIDGET(editor_data_widgets.general_pos_z), FALSE);
		    gtk_widget_set_sensitive(GTK_WIDGET(editor_data_widgets.general_dir_heading), FALSE);
		    gtk_widget_set_sensitive(GTK_WIDGET(editor_data_widgets.general_dir_pitch), FALSE);
		    gtk_widget_set_sensitive(GTK_WIDGET(editor_data_widgets.general_dir_bank), FALSE);
		}
	    break;

	case SAR_OBJ_TYPE_HUMAN:
	    gtk_widget_set_sensitive(
		GTK_WIDGET(editor_data_widgets.human_frame_grid), TRUE);
	    break;

	case SAR_OBJ_TYPE_SMOKE:
	    gtk_widget_set_sensitive(
		GTK_WIDGET(editor_data_widgets.smoke_frame_grid), TRUE);
	    break;

	case SAR_OBJ_TYPE_FIRE:
	    gtk_widget_set_sensitive(
		GTK_WIDGET(editor_data_widgets.fire_frame_grid), TRUE);
	    break;

	case SAR_OBJ_TYPE_EXPLOSION:
	case SAR_OBJ_TYPE_CHEMICAL_SPRAY:
	case SAR_OBJ_TYPE_FUELTANK:
	    break;

	case SAR_OBJ_TYPE_PREMODELED:
	    gtk_widget_set_sensitive(
		GTK_WIDGET(editor_data_widgets.premodeled_frame_grid), TRUE);
	    break;
    }

    /* Set "reference object" entry widgets sensitivity as needed */
    on_helipad_is_referenced_toggled(editor_data_widgets.helipad_is_referenced, (gpointer)core_ptr);

    /* Set "recession" entry widget sensitivity as needed */
    on_helipad_style_changed(editor_data_widgets.helipad_style, (gpointer)core_ptr);

    /* Set "human assistants" entry widgets sensitivity as needed */
    on_human_assistants_changed(editor_data_widgets.human_assistants, (gpointer)core_ptr);

    /* Set "human reference" entry widgets sensitivity as needed */
    on_human_has_displacement_toggled(editor_data_widgets.human_has_displacement, (gpointer)core_ptr);

    /* Set object specific frames visibility as needed */
    on_premod_type_changed(editor_data_widgets.premod_type, (gpointer)core_ptr);


    set_children_sensitive_by_name(GTK_WIDGET(editor_menu_widgets.grid), "obj_new", FALSE);
    set_children_sensitive_by_name(GTK_WIDGET(editor_menu_widgets.grid), "obj_current", FALSE);
    set_children_sensitive_by_name(GTK_WIDGET(editor_menu_widgets.grid), "obj_closer", TRUE);

    /* Show the Okay, Apply and Cancel buttons */
#if GTK_MAJOR_VERSION == 3 && GTK_MINOR_VERSION >= 24
    gtk_widget_show(GTK_WIDGET(editor_data_widgets.button_apply));
    gtk_widget_show(GTK_WIDGET(editor_data_widgets.button_cancel));
    gtk_widget_show(GTK_WIDGET(editor_data_widgets.button_cancel));
#endif
#if GTK_MAJOR_VERSION == 4 && GTK_MINOR_VERSION >= 10
    gtk_widget_set_visible(GTK_WIDGET(editor_data_widgets.button_apply), TRUE);
    gtk_widget_set_visible(GTK_WIDGET(editor_data_widgets.button_cancel), TRUE);
    gtk_widget_set_visible(GTK_WIDGET(editor_data_widgets.button_cancel), TRUE);
#endif

    gtk_window_set_default_size(GTK_WINDOW(editor_data_widgets.window), -1,-1);

    /* Show object data window */
#if GTK_MAJOR_VERSION == 3 && GTK_MINOR_VERSION >= 24
    gtk_widget_show(GTK_WIDGET(editor_data_widgets.window));
#endif
#if GTK_MAJOR_VERSION == 4 && GTK_MINOR_VERSION >= 10
    gtk_widget_set_visible(editor_data_widgets.window, TRUE);
#endif

    /* Unset menu window focus */
    EditorGtkSetAcceptFocus(editor_menu_widgets.window, FALSE);

    /* Note that focus will be given back to the toplevel sar2 window
     * by calling the GwSetWindowFocusToSar2Window() function from the
     * SARCmdSceneEditor() function.
     */

    return;
}

void on_button_move_clicked(void *widget, gpointer user_data)
{
    const sar_core_struct *core_ptr = (sar_core_struct *)user_data;
    sar_scenery_editor_struct *scn_ed = core_ptr->in_game_editor;
    const editor_gtk_ui_struct *editor_gtk_ui = scn_ed->editor_gtk_ui;

    scn_ed->current_action = EDITOR_ACTION_MOVE;

    /* Send the "move" command to SARCmdSceneEditor() */
    SARCmdSceneEditor((void *)core_ptr, "move", editor_gtk_ui->cmd_flags);

    set_children_sensitive_by_name(GTK_WIDGET(editor_menu_widgets.grid), "obj_new", FALSE);
    set_children_sensitive_by_name(GTK_WIDGET(editor_menu_widgets.grid), "obj_current", TRUE);
    set_children_sensitive_by_name(GTK_WIDGET(editor_menu_widgets.grid), "obj_closer", FALSE);

    /* Unset menu window focus */
    EditorGtkSetAcceptFocus(editor_menu_widgets.window, FALSE);

    /* Note that focus will be given back to the toplevel sar2 window
     * by calling the GwSetWindowFocusToSar2Window() function from the
     * SARCmdSceneEditor() function.
     */

    return;
}

void on_button_remove_clicked(void *widget, gpointer user_data)
{
    const sar_core_struct *core_ptr = (sar_core_struct *)user_data;
    sar_scenery_editor_struct *scn_ed = core_ptr->in_game_editor;
    const editor_gtk_ui_struct *editor_gtk_ui = scn_ed->editor_gtk_ui;

    scn_ed->current_action = EDITOR_ACTION_REMOVE;

    /* Send the "remove" command to SARCmdSceneEditor() */
    SARCmdSceneEditor((void *)core_ptr, "remove", editor_gtk_ui->cmd_flags);

    set_children_sensitive_by_name(GTK_WIDGET(editor_menu_widgets.grid), "obj_new", TRUE);
    set_children_sensitive_by_name(GTK_WIDGET(editor_menu_widgets.grid), "obj_current", FALSE);
    set_children_sensitive_by_name(GTK_WIDGET(editor_menu_widgets.grid), "obj_closer", TRUE);

    /* Unset menu window focus */
    EditorGtkSetAcceptFocus(editor_menu_widgets.window, FALSE);

    /* Note that focus will be given back to the toplevel sar2 window
     * by calling the GwSetWindowFocusToSar2Window() function from the
     * SARCmdSceneEditor() function.
     */

    return;
}

void on_button_quit_without_print_yes_clicked(GtkButton *button, gpointer user_data)
{
    const sar_core_struct *core_ptr = (sar_core_struct *)user_data;
    const gw_display_struct *display = core_ptr->display;
    sar_scenery_editor_struct *scn_ed = core_ptr->in_game_editor;

    gtk_window_set_modal(GTK_WINDOW(editor_quit_without_print_widgets.window), FALSE);

    /* Set focus to the Sar2 main window before hiding the GTK windows */
    GwSetWindowFocusToSar2Window(display);

    /* Hide the GTK windows */
#if GTK_MAJOR_VERSION == 3 && GTK_MINOR_VERSION >= 24
    gtk_widget_hide(GTK_WIDGET(editor_quit_without_print_widgets.window));
    gtk_widget_hide(GTK_WIDGET(editor_data_widgets.window));
    gtk_widget_hide(GTK_WIDGET(editor_menu_widgets.window));
#endif
#if GTK_MAJOR_VERSION == 4 && GTK_MINOR_VERSION >= 10
    gtk_widget_set_visible(editor_quit_without_print_widgets.window, FALSE);
    gtk_widget_set_visible(editor_data_widgets.window, FALSE);
    gtk_widget_set_visible(editor_menu_widgets.window, FALSE);
#endif

    scn_ed->current_action = EDITOR_ACTION_QUIT_WITHOUT_PRINT;

    /* Set focus to the sar2 top level window */
    //GwSetWindowFocusToSar2Window(display);

    return;
}

void on_button_quit_without_print_no_clicked(GtkButton *button, gpointer user_data)
{
    const sar_core_struct *core_ptr = (sar_core_struct *)user_data;
    const gw_display_struct *display = core_ptr->display;
    sar_scenery_editor_struct *scn_ed = core_ptr->in_game_editor;

    gtk_window_set_modal(GTK_WINDOW(editor_quit_without_print_widgets.window), FALSE);

    /* Set focus to the Sar2 main window before hiding the GTK windows */
    GwSetWindowFocusToSar2Window(display);

    /* Set windows visibility */
#if GTK_MAJOR_VERSION == 3 && GTK_MINOR_VERSION >= 24
    gtk_widget_hide(GTK_WIDGET(editor_quit_without_print_widgets.window));
#endif
#if GTK_MAJOR_VERSION == 4 && GTK_MINOR_VERSION >= 10
    gtk_widget_set_visible(editor_quit_without_print_widgets.window, FALSE);
#endif

    scn_ed->current_action = EDITOR_ACTION_NONE;

    /* Set focus to the sar2 top level window */
    //GwSetWindowFocusToSar2Window(display);

    return;
}







/*
 * Add a new editor_object_data_struct structure
 */
editor_object_data_struct *EditorObjectDataStructNew(void)
{
    editor_object_data_struct *editor_obj_data;

    editor_obj_data = calloc(1, sizeof(editor_object_data_struct));
    if(editor_obj_data == NULL)
	fprintf(stderr, "%s:%d: Memory allocation error.\n", __FILE__, __LINE__);

    return editor_obj_data;
}

/*
 * Gets values from the editor GTK widgets and set the
 * given editor_object_data_struct structure values.
 * Returns a non zero value on error.
 *
 * Any parameter added in this function must be added
 * in the EditorObjectDataStructFree() function.
 * Please check the EditorObjectDataStructReinit() function too.
 */
int EditorObjectDataStructSetFromGtkUi(
    //sar_core_struct *core_ptr,
    editor_object_data_struct *editor_obj_data
)
{
    char *selected_s;
    sar_obj_type type;
    char *type_s = NULL;

    /* Clean structure */
    EditorObjectDataStructReinit(editor_obj_data);

    /* Get type from choice list */
    selected_s = EditorGtkItemChooserGetSelectedString(editor_data_widgets.general_type);

    /* No item selected ? */
    if(selected_s[0] == '\0')
	return 1;

    /* Set object type.
     * Note that if object types are translated in the Gtk *.ui file,
     * tranlated type names have to be added to hereunder test.
     */

    if(!strcasecmp(selected_s, SAR_OBJ_TYPE_FIRE_S))
    {
	type = SAR_OBJ_TYPE_FIRE;
	type_s = strdup(SAR_OBJ_TYPE_FIRE_S);
    }
    else if(!strcasecmp(selected_s, SAR_OBJ_TYPE_HELIPAD_S))
    {
	type = SAR_OBJ_TYPE_HELIPAD;
	type_s = strdup(SAR_OBJ_TYPE_HELIPAD_S);
    }
    else if(!strcasecmp(selected_s, SAR_OBJ_TYPE_RUNWAY_S))
    {
	type = SAR_OBJ_TYPE_RUNWAY;
	type_s = strdup(SAR_OBJ_TYPE_RUNWAY_S);
    }
    else if(!strcasecmp(selected_s, SAR_OBJ_TYPE_HUMAN_S))
    {
	type = SAR_OBJ_TYPE_HUMAN;
	type_s = strdup(SAR_OBJ_TYPE_HUMAN_S);
    }
    else if(!strcasecmp(selected_s, SAR_OBJ_TYPE_AUTOMOBILE_S))
    {
	type = SAR_OBJ_TYPE_AUTOMOBILE;
	type_s = strdup(SAR_OBJ_TYPE_AUTOMOBILE_S);
    }
    else if(!strcasecmp(selected_s, SAR_OBJ_TYPE_STATIC_S))
    {
	type = SAR_OBJ_TYPE_STATIC;
	type_s = strdup(SAR_OBJ_TYPE_STATIC_S);
    }
    else if(!strcasecmp(selected_s, SAR_OBJ_TYPE_WATERCRAFT_S))
    {
	type = SAR_OBJ_TYPE_WATERCRAFT;
	type_s = strdup(SAR_OBJ_TYPE_WATERCRAFT_S);
    }
    else if(!strcasecmp(selected_s, SAR_OBJ_TYPE_PREMODELED_S))
    {
	type = SAR_OBJ_TYPE_PREMODELED;
	type_s = strdup(SAR_OBJ_TYPE_PREMODELED_S);
    }
    else if(!strcasecmp(selected_s, SAR_OBJ_TYPE_SMOKE_S))
    {
	type = SAR_OBJ_TYPE_SMOKE;
	type_s = strdup(SAR_OBJ_TYPE_SMOKE_S);
    }
    /* Should never happen */
    else
    {
	type = SAR_OBJ_TYPE_GARBAGE;
	type_s = strdup(SAR_OBJ_TYPE_GARBAGE_S);
    }

    g_free(selected_s);

    editor_obj_data->type = type;
    editor_obj_data->type_s = type_s;

    if(type == SAR_OBJ_TYPE_GARBAGE)
	return 2;

    /* Object name */
#if GTK_MAJOR_VERSION == 3 && GTK_MINOR_VERSION >= 24
    editor_obj_data->obj_name = STRDUP(gtk_entry_get_text(editor_data_widgets.general_object_name));
#endif
#if GTK_MAJOR_VERSION == 4 && GTK_MINOR_VERSION >= 10
    editor_obj_data->obj_name = STRDUP(gtk_editable_get_text(GTK_EDITABLE(editor_data_widgets.general_object_name)));
#endif

    /* Object position */
    editor_obj_data->pos.x = EditorGktEntryToDouble(editor_data_widgets.general_pos_x);
    editor_obj_data->pos.y = EditorGktEntryToDouble(editor_data_widgets.general_pos_y);
    editor_obj_data->pos.z = EditorGktEntryToDouble(editor_data_widgets.general_pos_z);

    /* Object direction */
    editor_obj_data->dir.heading = EditorGktEntryToDouble(editor_data_widgets.general_dir_heading);
    editor_obj_data->dir.pitch = EditorGktEntryToDouble(editor_data_widgets.general_dir_pitch);
    editor_obj_data->dir.bank = EditorGktEntryToDouble(editor_data_widgets.general_dir_bank);

    switch(type)
    {
	case SAR_OBJ_TYPE_GARBAGE:
	    break;

	case SAR_OBJ_TYPE_STATIC:
	case SAR_OBJ_TYPE_AUTOMOBILE:
	case SAR_OBJ_TYPE_WATERCRAFT:
	    char *full_file_name, *short_path_name;
#if GTK_MAJOR_VERSION == 3 && GTK_MINOR_VERSION >= 24
	    GFile *g_file = gtk_file_chooser_get_file(editor_data_widgets.model_file_name);
	    full_file_name = g_file_get_path(g_file);
	    g_object_unref(g_file);
#endif
#if GTK_MAJOR_VERSION == 4 && GTK_MINOR_VERSION >= 10
	    /* WARNING not tested */
	    full_file_name = STRDUP(gtk_label_get_text(editor_data_widgets.model_file_name_hidden));
#endif

	    if(full_file_name != NULL)
	    {
		/* Extract short_path_name from full_file_name */
		int i, slashes = 2;
		for(i = strlen(full_file_name); i >= 0; i--)
		{
		    if(full_file_name[i] == '/')
			slashes--;
		    if(slashes == 0)
		    break;
		}

		/* short_path_name example: automobiles/cuda.3d */
		short_path_name = full_file_name + i + 1;
		editor_obj_data->file_name = STRDUP(short_path_name);
	    }
	    else
		editor_obj_data->file_name = NULL;

	    /* Free full_file_name (short_path_name don't have to be freed) */
	    g_free(full_file_name);

	    break;

	case SAR_OBJ_TYPE_AIRCRAFT:
	    break;

	case SAR_OBJ_TYPE_GROUND:
	    break;

	case SAR_OBJ_TYPE_RUNWAY:
	    editor_obj_data->range = EditorGktEntryToDouble(editor_data_widgets.runway_range);
	    editor_obj_data->length = EditorGktEntryToDouble(editor_data_widgets.runway_length);
	    editor_obj_data->width = EditorGktEntryToDouble(editor_data_widgets.runway_width);
	    editor_obj_data->surface_type_s = EditorGtkItemChooserGetSelectedString(editor_data_widgets.runway_surface_type);
	    editor_obj_data->dashes = EditorGktEntryToInt(editor_data_widgets.runway_dashes_num);
	    editor_obj_data->edge_light_spacing = EditorGktEntryToDouble(editor_data_widgets.runway_edge_light_spacing);
#if GTK_MAJOR_VERSION == 3 && GTK_MINOR_VERSION >= 24
	    editor_obj_data->north_label = STRDUP(gtk_entry_get_text(editor_data_widgets.runway_north_label));
	    editor_obj_data->south_label = STRDUP(gtk_entry_get_text(editor_data_widgets.runway_south_label));
#endif
#if GTK_MAJOR_VERSION == 4 && GTK_MINOR_VERSION >= 10
	    editor_obj_data->north_label = STRDUP(gtk_editable_get_text(GTK_EDITABLE(editor_data_widgets.runway_north_label)));
	    editor_obj_data->south_label = STRDUP(gtk_editable_get_text(GTK_EDITABLE(editor_data_widgets.runway_south_label)));
#endif
	    editor_obj_data->north_displaced_threshold = EditorGktEntryToDouble(editor_data_widgets.runway_n_disp_thresh);
	    editor_obj_data->south_displaced_threshold = EditorGktEntryToDouble(editor_data_widgets.runway_s_disp_thresh);

	    if(GTK_CHECK_BUTTON_GET_ACTIVE(editor_data_widgets.runway_has_thresholds) == TRUE)
		editor_obj_data->has_thresholds_s = STRDUP("thresholds");
	    else
		editor_obj_data->has_thresholds_s = NULL;

	    if(GTK_CHECK_BUTTON_GET_ACTIVE(editor_data_widgets.runway_has_borders) == TRUE)
		editor_obj_data->has_borders_s = STRDUP("borders");
	    else
		editor_obj_data->has_borders_s = NULL;

	    if(GTK_CHECK_BUTTON_GET_ACTIVE(editor_data_widgets.runway_has_td_markers) == TRUE)
		editor_obj_data->has_td_markers_s = STRDUP("td_markers");
	    else
		editor_obj_data->has_td_markers_s = NULL;

	    if(GTK_CHECK_BUTTON_GET_ACTIVE(editor_data_widgets.runway_has_mid_markers) == TRUE)
		editor_obj_data->has_midway_markers_s = STRDUP("midway_markers");
	    else
		editor_obj_data->has_midway_markers_s = NULL;

	    if(GTK_CHECK_BUTTON_GET_ACTIVE(editor_data_widgets.runway_has_north_gs) == TRUE)
		editor_obj_data->has_north_gs_s = STRDUP("north_gs");
	    else
		editor_obj_data->has_north_gs_s = NULL;

	    if(GTK_CHECK_BUTTON_GET_ACTIVE(editor_data_widgets.runway_has_south_gs) == TRUE)
		editor_obj_data->has_south_gs_s = STRDUP("south_gs");
	    else
		editor_obj_data->has_south_gs_s = NULL;

	    break;

	case SAR_OBJ_TYPE_HELIPAD:
	    editor_obj_data->style_s = EditorGtkItemChooserGetSelectedString(editor_data_widgets.helipad_style);

	    if(!strcasecmp(editor_obj_data->style_s, SAR_HELIPAD_STYLE_GROUND_PAVED_S) ||
		!strcasecmp(editor_obj_data->style_s, SAR_HELIPAD_STYLE_DEFAULT_S) ||
		!strcasecmp(editor_obj_data->style_s, SAR_HELIPAD_STYLE_STANDARD_S)
	    )
	    {
		editor_obj_data->style = SAR_HELIPAD_STYLE_GROUND_PAVED;
	    }
	    else if(!strcasecmp(editor_obj_data->style_s, SAR_HELIPAD_STYLE_GROUND_BARE_S))
	    {
		editor_obj_data->style = SAR_HELIPAD_STYLE_GROUND_BARE;
	    }
	    else if(!strcasecmp(editor_obj_data->style_s, SAR_HELIPAD_STYLE_BUILDING_S))
	    {
		editor_obj_data->style = SAR_HELIPAD_STYLE_BUILDING;
	    }
	    else if(!strcasecmp(editor_obj_data->style_s, SAR_HELIPAD_STYLE_VEHICLE_S))
	    {
		editor_obj_data->style = SAR_HELIPAD_STYLE_VEHICLE;
	    }
	    /* Should never happen */
	    else
	    {
		editor_obj_data->style = SAR_HELIPAD_STYLE_GROUND_PAVED;
		free(editor_obj_data->style_s);
		editor_obj_data->style_s = strdup(SAR_HELIPAD_STYLE_GROUND_PAVED_S);
	    }

	    editor_obj_data->length = EditorGktEntryToDouble(editor_data_widgets.helipad_length);
	    editor_obj_data->width = EditorGktEntryToDouble(editor_data_widgets.helipad_width);
	    editor_obj_data->recession = EditorGktEntryToDouble(editor_data_widgets.helipad_recession);

#if GTK_MAJOR_VERSION == 3 && GTK_MINOR_VERSION >= 24
	    editor_obj_data->label = STRDUP(gtk_entry_get_text(editor_data_widgets.helipad_label));
#endif
#if GTK_MAJOR_VERSION == 4 && GTK_MINOR_VERSION >= 10
	    editor_obj_data->label = STRDUP(gtk_editable_get_text (GTK_EDITABLE(editor_data_widgets.helipad_label)));
#endif


	    if(GTK_CHECK_BUTTON_GET_ACTIVE(editor_data_widgets.helipad_has_lighting))
		editor_obj_data->edge_lighting_c = 'y';
	    else
		editor_obj_data->edge_lighting_c = 'n';

	    if(GTK_CHECK_BUTTON_GET_ACTIVE(editor_data_widgets.helipad_has_fuel))
		editor_obj_data->has_fuel_c = 'y';
	    else
		editor_obj_data->has_fuel_c = 'n';

	    if(GTK_CHECK_BUTTON_GET_ACTIVE(editor_data_widgets.helipad_has_repair))
		editor_obj_data->has_repair_c = 'y';
	    else
		editor_obj_data->has_repair_c = 'n';

	    if(GTK_CHECK_BUTTON_GET_ACTIVE(editor_data_widgets.helipad_has_drop_off))
		editor_obj_data->has_drop_off_c = 'y';
	    else
		editor_obj_data->has_drop_off_c = 'n';

	    if(GTK_CHECK_BUTTON_GET_ACTIVE(editor_data_widgets.helipad_is_restarting))
		editor_obj_data->restarting_point_c = 'y';
	    else
		editor_obj_data->restarting_point_c = 'n';

	    if(GTK_CHECK_BUTTON_GET_ACTIVE(editor_data_widgets.helipad_is_referenced))
	    {
		editor_obj_data->ref_obj_name = EditorGtkItemChooserGetSelectedString(editor_data_widgets.helipad_ref_obj_name);

		/* Helipad pos and dir offsets, relatives to the reference object */
		editor_obj_data->offset_pos.x = EditorGktEntryToDouble(editor_data_widgets.helipad_offset_pos_x);
		editor_obj_data->offset_pos.y = EditorGktEntryToDouble(editor_data_widgets.helipad_offset_pos_y);
		editor_obj_data->offset_pos.z = EditorGktEntryToDouble(editor_data_widgets.helipad_offset_pos_z);
		editor_obj_data->offset_dir.heading = EditorGktEntryToDouble(editor_data_widgets.helipad_offset_dir_heading);
		editor_obj_data->offset_dir.pitch = EditorGktEntryToDouble(editor_data_widgets.helipad_offset_dir_pitch);
		editor_obj_data->offset_dir.bank = EditorGktEntryToDouble(editor_data_widgets.helipad_offset_dir_bank);

		/* Helipad pos and dir of the reference object */
		editor_obj_data->ref_obj_pos.x = EditorGktEntryToDouble(editor_data_widgets.ref_object_pos_x);
		editor_obj_data->ref_obj_pos.y = EditorGktEntryToDouble(editor_data_widgets.ref_object_pos_y);
		editor_obj_data->ref_obj_pos.z = EditorGktEntryToDouble(editor_data_widgets.ref_object_pos_z);
		editor_obj_data->ref_obj_dir.heading = EditorGktEntryToDouble(editor_data_widgets.ref_object_dir_heading);
		editor_obj_data->ref_obj_dir.pitch = EditorGktEntryToDouble(editor_data_widgets.ref_object_dir_pitch);
		editor_obj_data->ref_obj_dir.bank = EditorGktEntryToDouble(editor_data_widgets.ref_object_dir_bank);
	    }
	    break;

	case SAR_OBJ_TYPE_HUMAN:
	    editor_obj_data->type_name = EditorGtkItemChooserGetSelectedString(editor_data_widgets.human_type_name);

	    if(GTK_CHECK_BUTTON_GET_ACTIVE(editor_data_widgets.human_need_rescue))
		editor_obj_data->need_rescue_s = STRDUP("need_rescue");
	    else
		editor_obj_data->need_rescue_s = NULL;

	    if(GTK_CHECK_BUTTON_GET_ACTIVE(editor_data_widgets.human_sit_up))
		editor_obj_data->sit_up_s = STRDUP("sit_up");
	    else
		editor_obj_data->sit_up_s = NULL;

	    if(GTK_CHECK_BUTTON_GET_ACTIVE(editor_data_widgets.human_sit_down))
		editor_obj_data->sit_down_s = STRDUP("sit_down");
	    else
		editor_obj_data->sit_down_s = NULL;

	    if(GTK_CHECK_BUTTON_GET_ACTIVE(editor_data_widgets.human_sitting))
		editor_obj_data->sitting_s = STRDUP("sitting");
	    else
		editor_obj_data->sitting_s = NULL;

	    if(GTK_CHECK_BUTTON_GET_ACTIVE(editor_data_widgets.human_lying))
		editor_obj_data->lying_s = STRDUP("lying");
	    else
		editor_obj_data->lying_s = NULL;

	    if(GTK_CHECK_BUTTON_GET_ACTIVE(editor_data_widgets.human_alert))
		editor_obj_data->alert_s = STRDUP("alert");
	    else
		editor_obj_data->alert_s = NULL;

	    if(GTK_CHECK_BUTTON_GET_ACTIVE(editor_data_widgets.human_aware))
		editor_obj_data->aware_s = STRDUP("aware");
	    else
		editor_obj_data->aware_s = NULL;

	    if(GTK_CHECK_BUTTON_GET_ACTIVE(editor_data_widgets.human_in_water))
		editor_obj_data->in_water_s = STRDUP("in_water");
	    else
		editor_obj_data->in_water_s = NULL;

	    if(GTK_CHECK_BUTTON_GET_ACTIVE(editor_data_widgets.human_on_stretcher))
		editor_obj_data->on_stretcher_s = STRDUP("on_stretcher");
	    else
		editor_obj_data->on_stretcher_s = NULL;

	    editor_obj_data->assistants = (int)gtk_spin_button_get_value(editor_data_widgets.human_assistants);
	    switch(editor_obj_data->assistants)
	    {
		case 4:
		    editor_obj_data->assist_type_name[3] =
			EditorGtkItemChooserGetSelectedString(editor_data_widgets.human_assist_4_name);
		case 3:
		    editor_obj_data->assist_type_name[2] =
			EditorGtkItemChooserGetSelectedString(editor_data_widgets.human_assist_3_name);
		case 2:
		    editor_obj_data->assist_type_name[1] =
			EditorGtkItemChooserGetSelectedString(editor_data_widgets.human_assist_2_name);
		case 1:
		    editor_obj_data->assist_type_name[0] =
			EditorGtkItemChooserGetSelectedString(editor_data_widgets.human_assist_1_name);
		default:
		    break;
	    }

	    if(GTK_CHECK_BUTTON_GET_ACTIVE(editor_data_widgets.human_has_displacement))
	    {
		editor_obj_data->ref_obj_name =
		    EditorGtkItemChooserGetSelectedString(editor_data_widgets.human_ref_obj_name);

		editor_obj_data->human_displacement_dir_s =
		    EditorGtkItemChooserGetSelectedString(editor_data_widgets.human_displacement);
	    }
	    break;

	case SAR_OBJ_TYPE_SMOKE:
	    editor_obj_data->offset_pos.x = EditorGktEntryToDouble(editor_data_widgets.smoke_offset_pos_x);
	    editor_obj_data->offset_pos.y = EditorGktEntryToDouble(editor_data_widgets.smoke_offset_pos_y);
	    editor_obj_data->offset_pos.z = EditorGktEntryToDouble(editor_data_widgets.smoke_offset_pos_z);
	    editor_obj_data->radius_start = EditorGktEntryToDouble(editor_data_widgets.smoke_radius_start);
	    editor_obj_data->radius_max = EditorGktEntryToDouble(editor_data_widgets.smoke_radius_max);
	    editor_obj_data->radius_rate = EditorGktEntryToDouble(editor_data_widgets.smoke_radius_rate);
	    editor_obj_data->hide_at_max = EditorGktEntryToDouble(editor_data_widgets.smoke_hide_at_max);
	    editor_obj_data->respawn_int = (time_t)EditorGktEntryToInt(editor_data_widgets.smoke_respawn_int);
	    editor_obj_data->total_units = EditorGktEntryToInt(editor_data_widgets.smoke_total_units);
	    selected_s = EditorGtkItemChooserGetSelectedString(editor_data_widgets.smoke_color_code);
	    sscanf(selected_s, "%d", &editor_obj_data->color_code);
	    g_free(selected_s);
	    break;

	case SAR_OBJ_TYPE_FIRE:
	    editor_obj_data->radius = EditorGktEntryToDouble(editor_data_widgets.fire_radius);
	    editor_obj_data->height = EditorGktEntryToDouble(editor_data_widgets.fire_height);
	    break;

	case SAR_OBJ_TYPE_EXPLOSION:
	case SAR_OBJ_TYPE_CHEMICAL_SPRAY:
	case SAR_OBJ_TYPE_FUELTANK:
	    break;

	case SAR_OBJ_TYPE_PREMODELED:
	    editor_obj_data->pm_type_s = EditorGtkItemChooserGetSelectedString(editor_data_widgets.premod_type);

	    if(!strcmp(editor_obj_data->pm_type_s, SAR_PREMODELED_BUILDING_S))
		editor_obj_data->pm_type = SAR_OBJ_PREMODELED_BUILDING;
	    else if(!strcmp(editor_obj_data->pm_type_s, SAR_PREMODELED_CONTROL_TOWER_S))
		editor_obj_data->pm_type = SAR_OBJ_PREMODELED_CONTROL_TOWER;
	    else if(!strcmp(editor_obj_data->pm_type_s, SAR_PREMODELED_POWER_TRANSMISSION_TOWER_S))
		editor_obj_data->pm_type = SAR_OBJ_PREMODELED_POWER_TRANSMISSION_TOWER;
	    else if(!strcmp(editor_obj_data->pm_type_s, SAR_PREMODELED_TOWER_S))
		editor_obj_data->pm_type = SAR_OBJ_PREMODELED_TOWER;
	    else if(!strcmp(editor_obj_data->pm_type_s, SAR_PREMODELED_RADIO_TOWER_S))
		editor_obj_data->pm_type = SAR_OBJ_PREMODELED_RADIO_TOWER;
	    else if(!strcmp(editor_obj_data->pm_type_s, SAR_PREMODELED_HANGAR_S))
		editor_obj_data->pm_type = SAR_OBJ_PREMODELED_HANGAR;
	    else
		;

	    editor_obj_data->range = EditorGktEntryToDouble(editor_data_widgets.premod_range);
	    editor_obj_data->length = EditorGktEntryToDouble(editor_data_widgets.premod_length);
	    editor_obj_data->width = EditorGktEntryToDouble(editor_data_widgets.premod_width);
	    editor_obj_data->height = EditorGktEntryToDouble(editor_data_widgets.premod_height);
	    editor_obj_data->hazard_lights = EditorGktEntryToInt(editor_data_widgets.premod_hazard);
	    editor_obj_data->walls_texture_s = EditorGtkItemChooserGetSelectedString(editor_data_widgets.premod_walls_tex);
	    editor_obj_data->walls_texture_night_s = EditorGtkItemChooserGetSelectedString(editor_data_widgets.premod_walls_tex_night);
	    editor_obj_data->roof_texture_s = EditorGtkItemChooserGetSelectedString(editor_data_widgets.premod_roof_tex);
	    break;
    }

    return 0;
}


/*
 * Gets values from the editor_object_data_struct structure and
 * set them to the relevant GTK widgets.
 * Returns a non zero value on error.
 *
 * See EditorObjectDataStructSetFromGtkUi() too.
 */
int EditorGtkUiSetFromObjectDataStruct(
    //sar_core_struct *core_ptr,
    const editor_object_data_struct *editor_obj_data
)
{
    sar_obj_type type;

    if(editor_obj_data == NULL)
	return 1;

    type = editor_obj_data->type;

    /* Object type string */
    EditorGtkItemChooserSetSelectedItemFromString(editor_data_widgets.general_type, editor_obj_data->type_s);

    /* Object name */
    if(editor_obj_data->obj_name != NULL)
	EditorGtkEntrySetText(editor_data_widgets.general_object_name, editor_obj_data->obj_name);
    else
	EditorGtkEntrySetText(editor_data_widgets.general_object_name, "");

    /* Object position */
    EditorGtkEntrySetTextFromDouble(editor_data_widgets.general_pos_x, editor_obj_data->pos.x);
    EditorGtkEntrySetTextFromDouble(editor_data_widgets.general_pos_y, editor_obj_data->pos.y);
    EditorGtkEntrySetTextFromDouble(editor_data_widgets.general_pos_z, editor_obj_data->pos.z);

    /* Object direction */
    EditorGtkEntrySetTextFromDouble(editor_data_widgets.general_dir_heading, editor_obj_data->dir.heading);
    EditorGtkEntrySetTextFromDouble(editor_data_widgets.general_dir_pitch, editor_obj_data->dir.pitch);
    EditorGtkEntrySetTextFromDouble(editor_data_widgets.general_dir_bank, editor_obj_data->dir.bank);

    switch(type)
    {
	case SAR_OBJ_TYPE_GARBAGE:
	    break;

	case SAR_OBJ_TYPE_STATIC:
	case SAR_OBJ_TYPE_AUTOMOBILE:
	case SAR_OBJ_TYPE_WATERCRAFT:
	case SAR_OBJ_TYPE_AIRCRAFT:
	case SAR_OBJ_TYPE_GROUND:
	    char full_file_name[PATH_MAX + 256];

	    snprintf(full_file_name, PATH_MAX + 255, "%s/%s", dname.global_data, editor_obj_data->file_name);

#if GTK_MAJOR_VERSION == 3 && GTK_MINOR_VERSION >= 24
	    /* Set file name */
	    gtk_file_chooser_set_filename(
		GTK_FILE_CHOOSER(editor_data_widgets.model_file_name),
		full_file_name
	    );
#endif
#if GTK_MAJOR_VERSION == 4 && GTK_MINOR_VERSION >= 10
/* TODO */
#endif

	    break;

	case SAR_OBJ_TYPE_RUNWAY:
	    EditorGtkEntrySetTextFromDouble(editor_data_widgets.runway_range, editor_obj_data->range);
	    EditorGtkEntrySetTextFromDouble(editor_data_widgets.runway_length, editor_obj_data->length);
	    EditorGtkEntrySetTextFromDouble(editor_data_widgets.runway_width, editor_obj_data->width);
	    EditorGtkItemChooserSetSelectedItemFromString(editor_data_widgets.runway_surface_type, editor_obj_data->surface_type_s);
	    EditorGtkEntrySetTextFromInt(editor_data_widgets.runway_dashes_num, editor_obj_data->dashes);
	    EditorGtkEntrySetTextFromDouble(editor_data_widgets.runway_edge_light_spacing, editor_obj_data->edge_light_spacing);
	    EditorGtkEntrySetText(editor_data_widgets.runway_north_label, editor_obj_data->north_label);
	    EditorGtkEntrySetText(editor_data_widgets.runway_south_label, editor_obj_data->south_label);
	    EditorGtkEntrySetTextFromDouble(editor_data_widgets.runway_n_disp_thresh, editor_obj_data->north_displaced_threshold);
	    EditorGtkEntrySetTextFromDouble(editor_data_widgets.runway_s_disp_thresh, editor_obj_data->south_displaced_threshold);

	    if(editor_obj_data->has_thresholds_s != NULL)
		GTK_CHECK_BUTTON_SET_ACTIVE(editor_data_widgets.runway_has_thresholds, TRUE);
	    else
		GTK_CHECK_BUTTON_SET_ACTIVE(editor_data_widgets.runway_has_thresholds, FALSE);

	    if(editor_obj_data->has_borders_s != NULL)
		GTK_CHECK_BUTTON_SET_ACTIVE(editor_data_widgets.runway_has_borders, TRUE);
	    else
		GTK_CHECK_BUTTON_SET_ACTIVE(editor_data_widgets.runway_has_borders, FALSE);

	    if(editor_obj_data->has_td_markers_s != NULL)
		GTK_CHECK_BUTTON_SET_ACTIVE(editor_data_widgets.runway_has_td_markers, TRUE);
	    else
		GTK_CHECK_BUTTON_SET_ACTIVE(editor_data_widgets.runway_has_td_markers, FALSE);

	    if(editor_obj_data->has_midway_markers_s != NULL)
		GTK_CHECK_BUTTON_SET_ACTIVE(editor_data_widgets.runway_has_mid_markers, TRUE);
	    else
		GTK_CHECK_BUTTON_SET_ACTIVE(editor_data_widgets.runway_has_mid_markers, FALSE);

	    if(editor_obj_data->has_north_gs_s != NULL)
		GTK_CHECK_BUTTON_SET_ACTIVE(editor_data_widgets.runway_has_north_gs, TRUE);
	    else
		GTK_CHECK_BUTTON_SET_ACTIVE(editor_data_widgets.runway_has_north_gs, FALSE);

	    if(editor_obj_data->has_south_gs_s != NULL)
		GTK_CHECK_BUTTON_SET_ACTIVE(editor_data_widgets.runway_has_south_gs, TRUE);
	    else
		GTK_CHECK_BUTTON_SET_ACTIVE(editor_data_widgets.runway_has_south_gs, FALSE);

	    break;

	case SAR_OBJ_TYPE_HELIPAD:
	    EditorGtkItemChooserSetSelectedItemFromString(editor_data_widgets.helipad_style, editor_obj_data->style_s);
	    EditorGtkEntrySetTextFromDouble(editor_data_widgets.helipad_length, editor_obj_data->length);
	    EditorGtkEntrySetTextFromDouble(editor_data_widgets.helipad_width, editor_obj_data->width);
	    EditorGtkEntrySetTextFromDouble(editor_data_widgets.helipad_recession, editor_obj_data->recession);
	    EditorGtkEntrySetText(editor_data_widgets.helipad_label, editor_obj_data->label);

	    if(editor_obj_data->edge_lighting_c == 'y')
		GTK_CHECK_BUTTON_SET_ACTIVE(editor_data_widgets.helipad_has_lighting, TRUE);
	    else
		GTK_CHECK_BUTTON_SET_ACTIVE(editor_data_widgets.helipad_has_lighting, FALSE);

	    if(editor_obj_data->has_fuel_c == 'y')
		GTK_CHECK_BUTTON_SET_ACTIVE(editor_data_widgets.helipad_has_fuel, TRUE);
	    else
		GTK_CHECK_BUTTON_SET_ACTIVE(editor_data_widgets.helipad_has_fuel, FALSE);

	    if(editor_obj_data->has_repair_c == 'y')
		GTK_CHECK_BUTTON_SET_ACTIVE(editor_data_widgets.helipad_has_repair, TRUE);
	    else
		GTK_CHECK_BUTTON_SET_ACTIVE(editor_data_widgets.helipad_has_repair, FALSE);

	    if(editor_obj_data->has_drop_off_c == 'y')
		GTK_CHECK_BUTTON_SET_ACTIVE(editor_data_widgets.helipad_has_drop_off, TRUE);
	    else
		GTK_CHECK_BUTTON_SET_ACTIVE(editor_data_widgets.helipad_has_drop_off, FALSE);

	    if(editor_obj_data->restarting_point_c == 'y')
		GTK_CHECK_BUTTON_SET_ACTIVE(editor_data_widgets.helipad_is_restarting, TRUE);
	    else
		GTK_CHECK_BUTTON_SET_ACTIVE(editor_data_widgets.helipad_is_restarting, FALSE);

	    /* Is this object referenced to another one? */
	    if(editor_obj_data->ref_obj_name != NULL && editor_obj_data->ref_obj_name[0] != '\0')
	    {
		/* Check the "Is referenced" check button */
		GTK_CHECK_BUTTON_SET_ACTIVE(editor_data_widgets.helipad_is_referenced, TRUE);

		/* Set the referenced object name */
		EditorGtkItemChooserSetSelectedItemFromString(editor_data_widgets.helipad_ref_obj_name, (const char *)editor_obj_data->ref_obj_name);

		/* Helipad pos and dir offsets, relatives to the reference object */
		EditorGtkEntrySetTextFromDouble(editor_data_widgets.helipad_offset_pos_x, editor_obj_data->offset_pos.x);
		EditorGtkEntrySetTextFromDouble(editor_data_widgets.helipad_offset_pos_y, editor_obj_data->offset_pos.y);
		EditorGtkEntrySetTextFromDouble(editor_data_widgets.helipad_offset_pos_z, editor_obj_data->offset_pos.z);
		EditorGtkEntrySetTextFromDouble(editor_data_widgets.helipad_offset_dir_heading, editor_obj_data->offset_dir.heading);
		EditorGtkEntrySetTextFromDouble(editor_data_widgets.helipad_offset_dir_pitch, editor_obj_data->offset_dir.pitch);
		EditorGtkEntrySetTextFromDouble(editor_data_widgets.helipad_offset_dir_bank, editor_obj_data->offset_dir.bank);

		/* Helipad pos and dir of the reference object */
		EditorGtkEntrySetTextFromDouble(editor_data_widgets.ref_object_pos_x, editor_obj_data->ref_obj_pos.x);
		EditorGtkEntrySetTextFromDouble(editor_data_widgets.ref_object_pos_y, editor_obj_data->ref_obj_pos.y);
		EditorGtkEntrySetTextFromDouble(editor_data_widgets.ref_object_pos_z, editor_obj_data->ref_obj_pos.z);
		EditorGtkEntrySetTextFromDouble(editor_data_widgets.ref_object_dir_heading, editor_obj_data->ref_obj_dir.heading);
		EditorGtkEntrySetTextFromDouble(editor_data_widgets.ref_object_dir_pitch, editor_obj_data->ref_obj_dir.pitch);
		EditorGtkEntrySetTextFromDouble(editor_data_widgets.ref_object_dir_bank, editor_obj_data->ref_obj_dir.bank);
	    }
	    else
		/* Uncheck the "Is referenced" check button */
		GTK_CHECK_BUTTON_SET_ACTIVE(editor_data_widgets.helipad_is_referenced, FALSE);

	    break;

	case SAR_OBJ_TYPE_HUMAN:
	    EditorGtkItemChooserSetSelectedItemFromString(editor_data_widgets.human_type_name, editor_obj_data->type_name);

	    if(editor_obj_data->need_rescue_s != NULL)
		GTK_CHECK_BUTTON_SET_ACTIVE(editor_data_widgets.human_need_rescue, TRUE);
	    else
		GTK_CHECK_BUTTON_SET_ACTIVE(editor_data_widgets.human_need_rescue, FALSE);

	    if(editor_obj_data->sit_up_s != NULL)
		GTK_CHECK_BUTTON_SET_ACTIVE(editor_data_widgets.human_sit_up, TRUE);
	    else
		GTK_CHECK_BUTTON_SET_ACTIVE(editor_data_widgets.human_sit_up, FALSE);

	    if(editor_obj_data->sit_down_s != NULL)
		GTK_CHECK_BUTTON_SET_ACTIVE(editor_data_widgets.human_sit_down, TRUE);
	    else
		GTK_CHECK_BUTTON_SET_ACTIVE(editor_data_widgets.human_sit_down, FALSE);

	    if(editor_obj_data->sitting_s != NULL)
		GTK_CHECK_BUTTON_SET_ACTIVE(editor_data_widgets.human_sitting, TRUE);
	    else
		GTK_CHECK_BUTTON_SET_ACTIVE(editor_data_widgets.human_sitting, FALSE);

	    if(editor_obj_data->lying_s != NULL)
		GTK_CHECK_BUTTON_SET_ACTIVE(editor_data_widgets.human_lying, TRUE);
	    else
		GTK_CHECK_BUTTON_SET_ACTIVE(editor_data_widgets.human_lying, FALSE);

	    if(editor_obj_data->alert_s != NULL)
		GTK_CHECK_BUTTON_SET_ACTIVE(editor_data_widgets.human_alert, TRUE);
	    else
		GTK_CHECK_BUTTON_SET_ACTIVE(editor_data_widgets.human_alert, FALSE);

	    if(editor_obj_data->aware_s != NULL)
		GTK_CHECK_BUTTON_SET_ACTIVE(editor_data_widgets.human_aware, TRUE);
	    else
		GTK_CHECK_BUTTON_SET_ACTIVE(editor_data_widgets.human_aware, FALSE);

	    if(editor_obj_data->in_water_s != NULL)
		GTK_CHECK_BUTTON_SET_ACTIVE(editor_data_widgets.human_in_water, TRUE);
	    else
		GTK_CHECK_BUTTON_SET_ACTIVE(editor_data_widgets.human_in_water, FALSE);

	    if(editor_obj_data->on_stretcher_s != NULL)
		GTK_CHECK_BUTTON_SET_ACTIVE(editor_data_widgets.human_on_stretcher, TRUE);
	    else
		GTK_CHECK_BUTTON_SET_ACTIVE(editor_data_widgets.human_on_stretcher, FALSE);

	    gtk_spin_button_set_value(editor_data_widgets.human_assistants, (double)editor_obj_data->assistants);

	    switch(editor_obj_data->assistants)
	    {
		case 1:
		    EditorGtkItemChooserSetSelectedItemFromString(editor_data_widgets.human_assist_1_name, editor_obj_data->assist_type_name[0]);
		    EditorGtkItemChooserSetSelectedItemFromString(editor_data_widgets.human_assist_2_name, NULL);
		    EditorGtkItemChooserSetSelectedItemFromString(editor_data_widgets.human_assist_3_name, NULL);
		    EditorGtkItemChooserSetSelectedItemFromString(editor_data_widgets.human_assist_4_name, NULL);
		    break;

		case 2:
		    EditorGtkItemChooserSetSelectedItemFromString(editor_data_widgets.human_assist_1_name, editor_obj_data->assist_type_name[0]);
		    EditorGtkItemChooserSetSelectedItemFromString(editor_data_widgets.human_assist_2_name, editor_obj_data->assist_type_name[1]);
		    EditorGtkItemChooserSetSelectedItemFromString(editor_data_widgets.human_assist_3_name, NULL);
		    EditorGtkItemChooserSetSelectedItemFromString(editor_data_widgets.human_assist_4_name, NULL);
		    break;

		case 3:
		    EditorGtkItemChooserSetSelectedItemFromString(editor_data_widgets.human_assist_1_name, editor_obj_data->assist_type_name[0]);
		    EditorGtkItemChooserSetSelectedItemFromString(editor_data_widgets.human_assist_2_name, editor_obj_data->assist_type_name[1]);
		    EditorGtkItemChooserSetSelectedItemFromString(editor_data_widgets.human_assist_3_name, editor_obj_data->assist_type_name[2]);
		    EditorGtkItemChooserSetSelectedItemFromString(editor_data_widgets.human_assist_4_name, NULL);
		    break;

		case 4:
		    EditorGtkItemChooserSetSelectedItemFromString(editor_data_widgets.human_assist_1_name, editor_obj_data->assist_type_name[0]);
		    EditorGtkItemChooserSetSelectedItemFromString(editor_data_widgets.human_assist_2_name, editor_obj_data->assist_type_name[1]);
		    EditorGtkItemChooserSetSelectedItemFromString(editor_data_widgets.human_assist_3_name, editor_obj_data->assist_type_name[2]);
		    EditorGtkItemChooserSetSelectedItemFromString(editor_data_widgets.human_assist_4_name, editor_obj_data->assist_type_name[3]);
		    break;

		default:
		    /* Clean all */
		    EditorGtkItemChooserSetSelectedItemFromString(editor_data_widgets.human_assist_1_name, NULL);
		    EditorGtkItemChooserSetSelectedItemFromString(editor_data_widgets.human_assist_2_name, NULL);
		    EditorGtkItemChooserSetSelectedItemFromString(editor_data_widgets.human_assist_3_name, NULL);
		    EditorGtkItemChooserSetSelectedItemFromString(editor_data_widgets.human_assist_4_name, NULL);
		    break;
	    }

	    /* Has human a reference object name set? */
	    if(editor_obj_data->ref_obj_name != NULL && editor_obj_data->ref_obj_name[0] != '\0')
	    {
		/* Check the "Has relative displacement" check button */
		GTK_CHECK_BUTTON_SET_ACTIVE(editor_data_widgets.human_has_displacement, TRUE);

		EditorGtkItemChooserSetSelectedItemFromString(editor_data_widgets.human_ref_obj_name, editor_obj_data->ref_obj_name);
		EditorGtkItemChooserSetSelectedItemFromString(editor_data_widgets.human_displacement, editor_obj_data->human_displacement_dir_s);
	    }
	    else
	    {
		/* Uncheck the "Has relative displacement" check button */
		GTK_CHECK_BUTTON_SET_ACTIVE(editor_data_widgets.human_has_displacement, FALSE);

		EditorGtkItemChooserSetSelectedItemFromString(editor_data_widgets.human_ref_obj_name, NULL);
		EditorGtkItemChooserSetSelectedItemFromString(editor_data_widgets.human_displacement, NULL);
	    }

	    break;

	case SAR_OBJ_TYPE_SMOKE:
	    EditorGtkEntrySetTextFromDouble(editor_data_widgets.smoke_offset_pos_x, editor_obj_data->offset_pos.x);
	    EditorGtkEntrySetTextFromDouble(editor_data_widgets.smoke_offset_pos_y, editor_obj_data->offset_pos.y);
	    EditorGtkEntrySetTextFromDouble(editor_data_widgets.smoke_offset_pos_z, editor_obj_data->offset_pos.z);
	    EditorGtkEntrySetTextFromDouble(editor_data_widgets.smoke_radius_start, editor_obj_data->radius_start);
	    EditorGtkEntrySetTextFromDouble(editor_data_widgets.smoke_radius_max, editor_obj_data->radius_max);
	    EditorGtkEntrySetTextFromDouble(editor_data_widgets.smoke_radius_rate, editor_obj_data->radius_rate);
	    EditorGtkEntrySetTextFromDouble(editor_data_widgets.smoke_hide_at_max, editor_obj_data->hide_at_max);
	    EditorGtkEntrySetTextFromInt(editor_data_widgets.smoke_respawn_int, (int)editor_obj_data->respawn_int);
	    EditorGtkEntrySetTextFromInt(editor_data_widgets.smoke_total_units, editor_obj_data->total_units);
	    char color_s[2];
	    sprintf(color_s, "%1d", editor_obj_data->color_code);
	    EditorGtkItemChooserSetSelectedItemFromString(editor_data_widgets.smoke_color_code, color_s);
	    break;

	case SAR_OBJ_TYPE_FIRE:
	    EditorGtkEntrySetTextFromDouble(editor_data_widgets.fire_radius, editor_obj_data->radius);
	    EditorGtkEntrySetTextFromDouble(editor_data_widgets.fire_height, editor_obj_data->height);
	    break;

	case SAR_OBJ_TYPE_EXPLOSION:
	case SAR_OBJ_TYPE_CHEMICAL_SPRAY:
	case SAR_OBJ_TYPE_FUELTANK:
	    break;

	case SAR_OBJ_TYPE_PREMODELED:
	    EditorGtkItemChooserSetSelectedItemFromString(editor_data_widgets.premod_type, editor_obj_data->pm_type_s);

	    EditorGtkEntrySetTextFromDouble(editor_data_widgets.premod_range, editor_obj_data->range);
	    EditorGtkEntrySetTextFromDouble(editor_data_widgets.premod_length, editor_obj_data->length);
	    EditorGtkEntrySetTextFromDouble(editor_data_widgets.premod_width, editor_obj_data->width);
	    EditorGtkEntrySetTextFromDouble(editor_data_widgets.fire_height, editor_obj_data->height);
	    EditorGtkEntrySetTextFromInt(editor_data_widgets.premod_hazard, editor_obj_data->hazard_lights);

	    EditorGtkItemChooserSetSelectedItemFromString(editor_data_widgets.premod_walls_tex, editor_obj_data->walls_texture_s);
	    EditorGtkItemChooserSetSelectedItemFromString(editor_data_widgets.premod_walls_tex_night, editor_obj_data->walls_texture_night_s);
	    EditorGtkItemChooserSetSelectedItemFromString(editor_data_widgets.premod_roof_tex, editor_obj_data->roof_texture_s);
	    break;
    }

    return 0;
}


void EditorGtkUiShowInfoWindow(
    const sar_core_struct *core_ptr,
    int picked_obj_num,
    const editor_object_data_struct *editor_obj_data
)
{
    const sar_scene_struct *scene = core_ptr->scene;
    float distance;
    char title[31+1];

    /* Should never happen */
    if(scene == NULL)
	return;

    /* 3D distance between triedron (i.e. player) and #obj_num object */
	distance = (float)SFMHypot3(
	    scene->player_obj_ptr->pos.x - editor_obj_data->pos.x,
	    scene->player_obj_ptr->pos.y - editor_obj_data->pos.y,
	    scene->player_obj_ptr->pos.z - SFMFeetToMeters((double)editor_obj_data->pos.z)
	);

    /* Set window title */
    snprintf(title, 31, "Object #%05d @%.3fm", picked_obj_num, distance);
    gtk_window_set_title(GTK_WINDOW(editor_data_widgets.window), (const gchar*)title);

    /* Set widgets value */
    EditorGtkUiSetFromObjectDataStruct(editor_obj_data);

    /* Set the menu buttons sensitivity */
    set_children_sensitive_by_name(GTK_WIDGET(editor_menu_widgets.grid), "obj_new", FALSE);
    set_children_sensitive_by_name(GTK_WIDGET(editor_menu_widgets.grid), "obj_current", FALSE);
    set_children_sensitive_by_name(GTK_WIDGET(editor_menu_widgets.grid), "obj_closer", TRUE);

    /* Set the sensitivity of the wigdets inside the general frame grid */
    set_children_sensitive_by_name(GTK_WIDGET(editor_data_widgets.general_frame_grid), "general_data", FALSE);

    /* Set all frame grids insensitive */
    gtk_widget_set_sensitive(
	    GTK_WIDGET(editor_data_widgets.general_frame_grid), FALSE);
    gtk_widget_set_sensitive(
	    GTK_WIDGET(editor_data_widgets.fire_frame_grid), FALSE);
    gtk_widget_set_sensitive(
	    GTK_WIDGET(editor_data_widgets.helipad_frame_grid), FALSE);
    gtk_widget_set_sensitive(
	    GTK_WIDGET(editor_data_widgets.human_frame_grid), FALSE);
    gtk_widget_set_sensitive(
	    GTK_WIDGET(editor_data_widgets.model_frame_grid), FALSE);
    gtk_widget_set_sensitive(
	    GTK_WIDGET(editor_data_widgets.premodeled_frame_grid), FALSE);
    gtk_widget_set_sensitive(
	    GTK_WIDGET(editor_data_widgets.runway_frame_grid), FALSE);
    gtk_widget_set_sensitive(
	    GTK_WIDGET(editor_data_widgets.smoke_frame_grid), FALSE);

    /* Hide Apply and Cancel buttons, show only the Ok one */
#if GTK_MAJOR_VERSION == 3 && GTK_MINOR_VERSION >= 24
    gtk_widget_hide(GTK_WIDGET(editor_data_widgets.button_apply));
    gtk_widget_hide(GTK_WIDGET(editor_data_widgets.button_cancel));
    gtk_widget_show(GTK_WIDGET(editor_data_widgets.button_cancel));
#endif
#if GTK_MAJOR_VERSION == 4 && GTK_MINOR_VERSION >= 10
    gtk_widget_set_visible(GTK_WIDGET(editor_data_widgets.button_apply), FALSE);
    gtk_widget_set_visible(GTK_WIDGET(editor_data_widgets.button_cancel), FALSE);
    gtk_widget_set_visible(GTK_WIDGET(editor_data_widgets.button_cancel), TRUE);
#endif

    gtk_window_set_default_size(GTK_WINDOW(editor_data_widgets.window), -1,-1);

    /* Show object data window */
#if GTK_MAJOR_VERSION == 3 && GTK_MINOR_VERSION >= 24
    gtk_widget_show(GTK_WIDGET(editor_data_widgets.window));
#endif
#if GTK_MAJOR_VERSION == 4 && GTK_MINOR_VERSION >= 10
    gtk_widget_set_visible(editor_data_widgets.window, TRUE);
#endif

}


/* Generate an arguments list (identical as those which can be found in a *.scn
 * scenery file) from an editor_object_data_struct structure.
 * Returned string must be freed by calling function.
 */
char *DoCmdLineFromObjectDataStruct(editor_object_data_struct *editor_obj_data)
{
#define S_LENGTH 1023
#define REMAINING(s) (MAX(0, S_LENGTH - strlen(s)))
    sar_obj_type type;
    char *cmd_args = (char *)malloc((S_LENGTH + 1) * sizeof(char));
    char *s = (char *)malloc((S_LENGTH + 1) * sizeof(char));

    if(editor_obj_data == NULL)
	return NULL;

    cmd_args[0] = '\0';
    s[0] = '\0';

    type = editor_obj_data->type;

    switch(type)
    {
	case SAR_OBJ_TYPE_GARBAGE:
	    break;

	case SAR_OBJ_TYPE_STATIC:
	case SAR_OBJ_TYPE_AUTOMOBILE:
	case SAR_OBJ_TYPE_WATERCRAFT:
	    snprintf(cmd_args, S_LENGTH, "%s", editor_obj_data->file_name);
	    break;

	case SAR_OBJ_TYPE_AIRCRAFT:
	    break;

	case SAR_OBJ_TYPE_GROUND:
	    break;

	case SAR_OBJ_TYPE_RUNWAY:
	    snprintf(cmd_args, S_LENGTH, "%.3f %.3f %.3f %d %d %f %s %s %.3f %.3f",
		    editor_obj_data->range,
		    editor_obj_data->length,
		    editor_obj_data->width,
		    editor_obj_data->surface_type,
		    editor_obj_data->dashes,
		    editor_obj_data->edge_light_spacing,
		    editor_obj_data->north_label,
		    editor_obj_data->south_label,
		    editor_obj_data->north_displaced_threshold,
		    editor_obj_data->north_displaced_threshold
		);

	    if(editor_obj_data->has_thresholds_s != NULL)
		strcat(s, " thresholds");

	    if(editor_obj_data->has_borders_s != NULL)
		strcat(s, " borders");

	    if(editor_obj_data->has_td_markers_s != NULL)
		strcat(s, " td_markers");

	    if(editor_obj_data->has_midway_markers_s != NULL)
		strcat(s, " midway_markers");

	    if(editor_obj_data->has_north_gs_s != NULL)
		strcat(s, " north_gs");

	    if(editor_obj_data->has_south_gs_s != NULL)
		strcat(s, " south_gs");

	    strncat(cmd_args, s, REMAINING(cmd_args));
	    break;

	case SAR_OBJ_TYPE_HELIPAD:
	    snprintf(cmd_args, S_LENGTH, "%s %.3f %.3f %.3f %s %c %c %c %c %c",
		    editor_obj_data->style_s,
		    editor_obj_data->length,
		    editor_obj_data->width,
		    editor_obj_data->recession,
		    editor_obj_data->label,
		    editor_obj_data->edge_lighting_c,
		    editor_obj_data->has_fuel_c,
		    editor_obj_data->has_repair_c,
		    editor_obj_data->has_drop_off_c,
		    editor_obj_data->restarting_point_c
		);

	    /* Is this object referenced to another one? */
	    if(editor_obj_data->ref_obj_name != NULL && editor_obj_data->ref_obj_name[0] != '\0')
	    {
		snprintf(s, S_LENGTH, " %s %.3f %.3f %.3f %.3f %.3f %.3f",
			editor_obj_data->ref_obj_name,
			editor_obj_data->offset_pos.x,
			editor_obj_data->offset_pos.y,
			editor_obj_data->offset_pos.z,
			editor_obj_data->offset_dir.heading,
			editor_obj_data->offset_dir.pitch,
			editor_obj_data->offset_dir.bank
		    );

		strncat(cmd_args, s, REMAINING(cmd_args));
	    }
	    break;

	case SAR_OBJ_TYPE_HUMAN:
	    snprintf(cmd_args, S_LENGTH, "%s", editor_obj_data->type_name);

	    if(editor_obj_data->need_rescue_s != NULL)
		strcat(s, " need_rescue");

	    if(editor_obj_data->sit_up_s != NULL)
		strcat(s, " sit_up");

	    if(editor_obj_data->sit_down_s != NULL)
		strcat(s, " sit_down");

	    if(editor_obj_data->sitting_s != NULL)
		strcat(s, " sitting");

	    if(editor_obj_data->lying_s != NULL)
		strcat(s, " lying");

	    if(editor_obj_data->alert_s != NULL)
		strcat(s, " alert");

	    if(editor_obj_data->aware_s != NULL)
		strcat(s, " aware");

	    if(editor_obj_data->in_water_s != NULL)
		strcat(s, " in_water");

	    if(editor_obj_data->on_stretcher_s != NULL)
		strcat(s, " on_stretcher");

	    strncat(cmd_args, s, REMAINING(cmd_args));

	    if(editor_obj_data->assistants != 0)
	    {
		snprintf(s, S_LENGTH, " assisted %d", editor_obj_data->assistants);
		strncat(cmd_args, s, REMAINING(cmd_args));

		for(int i = 0; i < editor_obj_data->assistants; i++)
		{
		    sprintf(s, " %s", editor_obj_data->assist_type_name[i]);
		    strncat(cmd_args, s, REMAINING(cmd_args));
		}
	    }
	    break;

	case SAR_OBJ_TYPE_SMOKE:
	    snprintf(cmd_args, S_LENGTH, "%.3f %.3f %.3f %.3f %.3f %.3f %.3f %ld %d %1d",
		    editor_obj_data->offset_pos.x,
		    editor_obj_data->offset_pos.y,
		    editor_obj_data->offset_pos.z,
		    editor_obj_data->radius_start,
		    editor_obj_data->radius_max,
		    editor_obj_data->radius_rate,
		    editor_obj_data->hide_at_max,
		    editor_obj_data->respawn_int,
		    editor_obj_data->total_units,
		    editor_obj_data->color_code
		);

	    break;

	case SAR_OBJ_TYPE_FIRE:
	    snprintf(cmd_args, S_LENGTH, "%.3f %.3f",
		    editor_obj_data->radius,
		    editor_obj_data->height
		);
	    break;

	case SAR_OBJ_TYPE_EXPLOSION:
	case SAR_OBJ_TYPE_CHEMICAL_SPRAY:
	case SAR_OBJ_TYPE_FUELTANK:
	    break;

	case SAR_OBJ_TYPE_PREMODELED:
	    switch(editor_obj_data->pm_type)
	    {
		case SAR_OBJ_PREMODELED_BUILDING:
		    snprintf(cmd_args, S_LENGTH, "%s %f %.3f %.3f %.3f %s %s %s",
			    editor_obj_data->pm_type_s,
			    editor_obj_data->range,
			    editor_obj_data->length,
			    editor_obj_data->width,
			    editor_obj_data->height,
			    editor_obj_data->walls_texture_s,
			    editor_obj_data->walls_texture_night_s,
			    editor_obj_data->roof_texture_s
			);
		    break;

		case SAR_OBJ_PREMODELED_CONTROL_TOWER:
		    snprintf(cmd_args, S_LENGTH, "%s %f %.3f %.3f %.3f %s %s",
			    editor_obj_data->pm_type_s,
			    editor_obj_data->range,
			    editor_obj_data->length,
			    editor_obj_data->width,
			    editor_obj_data->height,
			    editor_obj_data->walls_texture_s,
			    editor_obj_data->roof_texture_s
			);
		    break;

		case SAR_OBJ_PREMODELED_HANGAR:
		    break;

		case SAR_OBJ_PREMODELED_POWER_TRANSMISSION_TOWER:
		case SAR_OBJ_PREMODELED_TOWER:
		case SAR_OBJ_PREMODELED_RADIO_TOWER:
		    snprintf(cmd_args, S_LENGTH, "%s %f %.3f %d",
			    editor_obj_data->pm_type_s,
			    editor_obj_data->range,
			    editor_obj_data->height,
			    editor_obj_data->hazard_lights
			);
		    break;
	    }
	    break;

	default:
	    break;
    }

    free(s);

    return cmd_args;

#undef S_LENGTH
#undef REMAINING
}
