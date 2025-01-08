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
#include <sys/types.h>
#include <errno.h>

#include <unistd.h> // usleep()

#include "../include/string.h"
#include "../include/strexp.h"
#include "../include/fio.h"
#include "../include/disk.h"

#include "gw.h"
#include "obj.h"
#include "objutils.h"
#include "messages.h"
#include "simop.h"
#include "cmd.h"
#include "sar.h"
#include "config.h"
#include "objio.h"
#include "simcontact.h"
#include "simutils.h"
#include "smoke.h"
#include "fire.h"
#include "sardraw.h"
#include "simsurface.h"
#include "cmdscnedit.h"

//#include <gtk/gtk.h>

/* For SARScenePostLoading() */
#include "sceneio.h"

/* Prototype for all SARKey*() functions */
#define SAR_KEY_FUNC_PROTOTYPE				\
sar_core_struct *core_ptr, gw_display_struct *display,	\
sar_scene_struct *scene, Boolean state

void EditorOff(sar_core_struct *core_ptr);
/*
static void close_window (GtkWidget *window, sar_core_struct *core_ptr);
static void action_clbk ( GSimpleAction *simple_action, G_GNUC_UNUSED GVariant *parameter, sar_core_struct *core_ptr );
static void activate ( GtkApplication *app, sar_core_struct *core_ptr );
int gtkAppStart(sar_core_struct *core_ptr)
int gtkShowWindow(sar_core_struct *core_ptr, char *title, char* subject)
*/
static void SARKeyCommand(SAR_KEY_FUNC_PROTOTYPE);
FILE *FCopy(const char *source, const char *target);
static int FInsertData(FILE *fp, unsigned long start_pos, const char *data, unsigned long data_length);
/*
static int FRemoveData(FILE *fp, unsigned long start_pos, unsigned long data_length);
*/
static int AddObjNumToSceneryFile(FILE *fp);
char *GetObjectModelFileNameFromSceneryFile(sar_scenery_editor_struct *scn_ed, int obj_num);
static int SceneObjectPick(sar_core_struct *core_ptr, int picker_obj_num, Boolean next);
static int ScnEditLoadObject(sar_core_struct *core_ptr, int obj_num, sar_obj_type type, const char *arg_list);
static char *ScnEditGetTextureNamePtrByRef(sar_scene_struct *scene,int tex_index);
static char *ScnEditGetHumanPresetNameByHumanPtr(sar_core_struct *core_ptr,
						 sar_object_human_struct *human
);
static int EditorObjectDataStructReinit(editor_object_data_struct *object_data);
static int EditorObjectDataStructFree(editor_object_data_struct *object_data);
static const char* SceneObjectGetTypeName(sar_core_struct *core_ptr, int obj_num);
static const char* SceneObjectGetPremodeledTypeName(sar_core_struct *core_ptr, int obj_num);
static char *getObjectFileName(sar_object_struct *obj_ptr);
static editor_object_data_struct *EditorObjectDataStructNew(void);
static int EditorObjectDataStructFill(
    sar_core_struct *core_ptr,
    editor_object_data_struct *object_data,
    int obj_num
);
/*
static editor_object_data_struct *EditorObjectDataStructUpdatePosAndDirOnly(
    sar_core_struct *core_ptr,
    editor_object_data_struct *object_data,
    int obj_num
);
*/
/*
static int EditorObjectDataStructCompare(
    editor_object_data_struct *obj_data_1,
    editor_object_data_struct *obj_data_2
);
*/
static char* EditorObjectDataDoParametersLine(editor_object_data_struct *editor_obj_data);
int ScnEditShowObjectInfoWindow(sar_core_struct *core_ptr, int obj_num);
float ScnEditFindGround(
	sar_scene_struct *scene,
	sar_object_struct **ptr, int total,
	sar_object_struct *src_obj_ptr
);
static int ScnEditPrintModificationsList(sar_scenery_editor_struct *scn_ed, FILE *fp);
void SARCmdSceneEditor(SAR_CMD_PROTOTYPE);



#define ATOI(s)         (((s) != NULL) ? atoi(s) : 0)
#define ATOL(s)         (((s) != NULL) ? atol(s) : 0)
#define ATOF(s)         (((s) != NULL) ? (float)atof(s) : 0.0f)
#define STRDUP(s)       (((s) != NULL) ? strdup(s) : NULL)

#define MAX(a,b)        (((a) > (b)) ? (a) : (b))
#define MIN(a,b)        (((a) < (b)) ? (a) : (b))
#define CLIP(a,l,h)     (MIN(MAX((a),(l)),(h)))
#define STRLEN(s)       (((s) != NULL) ? ((int)strlen(s)) : 0)

#define RADTODEG(r)     ((r) * 180.0 / PI)
#define DEGTORAD(d)     ((d) * PI / 180.0)

/* Editor OBJect number tag and number of characters.
 * Defines a #_EOBJ#nnnnn comment where nnnnn is a 5 characters object number.
 */
#define TAGSTRING "#_EOBJ#"
#define TAGVALLENGTH "%05d"

void EditorOff(sar_core_struct *core_ptr)
{
    gw_display_struct *display = core_ptr->display;
    sar_scene_struct *scene = core_ptr->scene;
    sar_scenery_editor_struct *scn_ed = core_ptr->in_game_editor;
    sar_object_aircraft_struct *aircraft = NULL;
    int player_obj_num, obj_num;
    sar_object_struct *player_obj_ptr;
    int i;

    if(scene == NULL)
	return;

    if(display == NULL)
	return;

    if(core_ptr->editor_mode_on == False)
	return;

    if(scn_ed->cur_obj_num >= 0)
    {
	obj_num = scn_ed->cur_obj_num;

	/* Unlink current object from player object */
	scn_ed->cur_obj_num = -1;

	SARObjDelete(
	    core_ptr,
	    &core_ptr->object,
	    &core_ptr->total_objects,
	    obj_num
	);
    }

    core_ptr->editor_mode_on = False;

    /* Get player object references from scene structure */
    player_obj_num = scene->player_obj_num;
    player_obj_ptr = scene->player_obj_ptr;

    if(scn_ed->scn_file_fp != NULL)
	fclose(scn_ed->scn_file_fp);

    free(scn_ed->scn_file_name);
    scn_ed->scn_file_name = NULL;

    free(scn_ed->cur_obj_arg);
    scn_ed->cur_obj_arg = NULL;

    free(scn_ed->prev_obj_arg);
    scn_ed->prev_obj_arg = NULL;

    if(scn_ed->modification_list != NULL)
    {
	for(i = 0; i < scn_ed->total_objects; i++)
	{
	    /* Should always be True */
	    if(scn_ed->modification_list[i] != NULL)
	    {
		if(scn_ed->modification_list[i]->obj_data_original != NULL)
		    EditorObjectDataStructFree(scn_ed->modification_list[i]->obj_data_original);

		if(scn_ed->modification_list[i]->obj_data_new != NULL)
		    EditorObjectDataStructFree(scn_ed->modification_list[i]->obj_data_new);

		free(scn_ed->modification_list[i]);
	    }
	}
	free(scn_ed->modification_list);
    }

    free(scn_ed);
    scn_ed = NULL;
    core_ptr->in_game_editor = NULL;

    /* FIXME Avoid error at sar2 exit */
    if(core_ptr->cur_player_model_file == NULL)
	core_ptr->cur_player_model_file = STRDUP("###");

    /* Hide pointer cursor */
    GWHideCursor(display);

    if(player_obj_ptr != NULL)
    {
	/* Get aircraft */
	aircraft = SAR_OBJ_GET_AIRCRAFT(player_obj_ptr);
	if(aircraft == NULL)
	    return;

	if(SARSimIsSlew(player_obj_ptr))
	{
	    /* Was in slew mode, now go into previous flight mode */
	    SARSimSetSlew(player_obj_ptr, 0);
	}
    }

    return;
}


/*
static void
close_window (GtkWidget *window, sar_core_struct *core_ptr)
{
fprintf(stderr, "Window closed by user.\n");
    sar_scenery_editor_struct *scn_ed = core_ptr->in_game_editor;

    gtk_window_close (GTK_WINDOW (window));

    scn_ed->gtk_app_running = FALSE;

}

static void action_clbk ( GSimpleAction *simple_action, G_GNUC_UNUSED GVariant *parameter, sar_core_struct *core_ptr )
{
    const char *action = g_action_get_name ( G_ACTION ( simple_action ) );

    if(!strcmp(action, "gtk_off"))
    {
	fprintf(stderr, "The action \"gtk_off\" was clicked.\n");
    }
    else if(!strcmp(action, "editor_off"))
    {
	fprintf(stderr, "The action \"editor_off\" was clicked.\n");
	EditorOff(core_ptr);

//--------------------------------- Il faut fermer la fenêtre Gtk ! ---------------------------------

    }
    else if(!strcmp(action, "object_info"))
    {
	fprintf(stderr, "The action \"object_info\" was clicked.\n");
    }

}

static void activate ( GtkApplication *app, sar_core_struct *core_ptr )
{
fprintf(stderr, "Entering activate()...\n");
    //sar_scenery_editor_struct *scn_ed = core_ptr->in_game_editor;

    GtkWidget *window;

fprintf(stderr, "-0- ");
  //////////window = gtk_window_new ();
  window = gtk_application_window_new (app);
fprintf(stderr, "-1- ");
  //////////gtk_window_set_application (GTK_WINDOW (window), app);
fprintf(stderr, "-2- ");
  // Add destroy call-back so we know when window is closed.
  g_signal_connect (window, "destroy", G_CALLBACK (close_window), NULL);
fprintf(stderr, "-3- ");
  gtk_window_set_title (GTK_WINDOW (window), "Window");
fprintf(stderr, "-4- ");
  gtk_window_set_default_size (GTK_WINDOW (window), 200, 200);
fprintf(stderr, "-5- ");

    // Set up a widget
    GtkWidget *label = gtk_label_new ("Blablabla");

//    gtk_label_set_markup (GTK_LABEL (label),
//                          "<span font_desc=\"100.0\">"
//                              "GTK Layer\nShell example!"
//                          "</span>");

    gtk_window_set_child (GTK_WINDOW(window), label);
fprintf(stderr, "-6- ");
  gtk_window_present (GTK_WINDOW(window));
fprintf(stderr, "-7- ");
fprintf(stderr, "Exting activate()!\n");
}

int gtkAppStart(sar_core_struct *core_ptr)
{
    sar_scenery_editor_struct *scn_ed = core_ptr->in_game_editor;

  GtkApplication *app;
  int status;

  app = gtk_application_new ("org.gtk.example", G_APPLICATION_DEFAULT_FLAGS);
  g_signal_connect (app, "activate", G_CALLBACK (activate), core_ptr);
  scn_ed->gtk_application = app;

  ////////////////////////////////////g_timeout_add_seconds (1, timeout, NULL);

  // **** begin g_application_run alternative ****
  // Setup
  GMainContext *context;
  gboolean acquired_context;

  //context = g_main_context_default ();
  context = g_main_context_get_thread_default ();

  acquired_context = g_main_context_acquire (context);
  g_return_val_if_fail (acquired_context, 0);
  scn_ed->gtk_context = context;


  GError *error = NULL;
  if (!g_application_register (G_APPLICATION (app), NULL, &error))
    {
      g_printerr ("Failed to register: %s\n", error->message);
      g_error_free (error);
      return 1;
    }

  g_application_activate (G_APPLICATION (app));


  // Main event loop
  scn_ed->gtk_app_running = TRUE;

    status = 0;
    return status;
}

// Show the GTK info window (not the X11 one) //
int gtkShowWindow(sar_core_struct *core_ptr, char *title, char* subject)
{
    //sar_scenery_editor_struct *scn_ed = core_ptr->in_game_editor;

    GtkWidget* p_Window, *main_box;
    GtkWidget* p_Label;
	gchar* sUtf8;

    p_Window = gtk_window_new();
    gtk_window_set_title(GTK_WINDOW(p_Window), title);
    gtk_window_set_default_size(GTK_WINDOW(p_Window), 260, 40);
    g_signal_connect(G_OBJECT(p_Window), "destroy", G_CALLBACK(NULL), NULL);

    sUtf8 = g_locale_to_utf8(subject, -1, NULL, NULL, NULL);
    p_Label=gtk_label_new(sUtf8);
    g_free(sUtf8);

    main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_box_set_homogeneous(GTK_BOX(main_box), FALSE);
    gtk_widget_set_margin_start(main_box, 10);
    gtk_widget_set_margin_end(main_box, 10);
    gtk_widget_set_margin_top(main_box, 10);
    gtk_widget_set_margin_bottom(main_box, 10);
    gtk_window_set_child(GTK_WINDOW(p_Window), main_box);

    gtk_box_append (GTK_BOX(main_box), p_Label);

    gtk_window_present(GTK_WINDOW(p_Window));

    return EXIT_SUCCESS;
}
*/


#define NOTIFY(s)			\
{ if(SAR_CMD_IS_VERBOSE(flags) &&	\
     (scene != NULL) && ((s) != NULL)	\
  ) { SARMessageAdd(scene, (s)); }	\
}

#define ISBLANK(c)	(((c) == ' ') || ((c) == '\t'))
#define ISCOMMENT(c)	((c) == SAR_COMMENT_CHAR)
#define STRISEMPTY(s)	(((s) != NULL) ? (*(s) == '\0') : 1)

#define SKIPARGS(cmd_args, n)				\
{							\
    char *val = cmd_args;				\
    for(int i = 0; i < n; i++)				\
    {							\
	while(!ISBLANK(*val) && (*val != '\0'))		\
	    val++;					\
	while(ISBLANK(*val))				\
	    val++;					\
    }							\
    cmd_args = val;					\
}


#define COPYCUROBJDATATOPREVOBJDATA			\
{							\
free(scn_ed->prev_obj_arg);				\
scn_ed->prev_obj_arg = STRDUP(scn_ed->cur_obj_arg);	\
scn_ed->prev_obj_type = scn_ed->cur_obj_type;		\
scn_ed->prev_obj_num = scn_ed->cur_obj_num;		\
}




#define SCENEDITORAIRCRAFT "aircrafts/object_placer.3d"
#define REFOBJECTFILENAME "objects/triedron.3d"

#define PICKSKIPMAX 5

#define NOTIFYSTRINGLENGTH 120


/*
 *	Warning: already defined in sarkey.c
 *
 *	Maps the command prompt, future key events sent to SARKey()
 *	will then be forwarded to SARTextInputHandleKey() until
 *	the command argument is typed in and processed or aborted.
 */
static void SARKeyCommand(SAR_KEY_FUNC_PROTOTYPE)
{
	if(!state)
	    return;

	SARTextInputMap(
	    core_ptr->text_input,
	    "Command", NULL,
	    SARCmdTextInputCB,
	    core_ptr
	);
}

/* Copy source to target and return target FILE pointer or NULL on error.
 */
FILE *FCopy(const char *source, const char *target)
{
    FILE *src, *tar;
    int c;

    src = fopen(source, "r");
    if(src == NULL)
        return NULL;

    tar = fopen(target, "w");
    if(tar == NULL)
    {
	fclose(src);
        return NULL;
    }

    while((c = fgetc(src)) != EOF)
        fputc(c, tar);

    fclose(src);

    return tar;
}


/*
 * Inserts data_length bytes of data in fp file, starting at start_pos offset.
 */
static int FInsertData(FILE *fp, unsigned long start_pos, const char *data, unsigned long data_length)
{
    size_t file_size, buffer_size;
    char *buffer;
    long offset;

    if(fp == NULL)
	return -1;

    /* Get file size */
    fseek(fp, 0L, SEEK_END);
    file_size = ftell(fp);
    rewind(fp);

    buffer_size = file_size - start_pos;
    if(buffer_size < 0)
    {
	fprintf(stderr, "%s:%d: Can't insert %ld bytes in file.\n", __FILE__, __LINE__, buffer_size);
	return -1;
    }
    else if(buffer_size == 0)
	return 0;

    buffer = malloc(buffer_size + 1);
    if(buffer == NULL)
    {
	fprintf(stderr, "%s:%d: Can't allocate %ld bytes for buffer.\n", __FILE__, __LINE__, buffer_size);
	return -1;
    }

    /* Copy file end data to buffer */
    fseek(fp, start_pos, SEEK_SET);
    if(fread(buffer, buffer_size, 1, fp) != 1)
    {
	fprintf(stderr, "%s:%d: Can't copy %ld file bytes to buffer.\n", __FILE__, __LINE__, buffer_size);
	free(buffer);
	return -1;
    }

    /* Write user data */
    fseek(fp, start_pos, SEEK_SET);
    if(fwrite(data, data_length, 1, fp) != 1)
    {
	fprintf(stderr, "%s:%d: Can't write %ld data bytes to file.\n", __FILE__, __LINE__, data_length);
	free(buffer);
	return -1;
    }

    /* Save current cursor position */
    offset = ftell(fp);

    /* Write buffer */
    if(fwrite(buffer, buffer_size, 1, fp) != 1)
    {
	fprintf(stderr, "%s:%d: Can't write %ld buffer bytes at end of file.\n", __FILE__, __LINE__, data_length);
	free(buffer);
	return -1;
    }

    /* Set cursor position */
    fseek(fp, offset, SEEK_SET);
    free(buffer);

    return 0;
}

/*
 * Removes data_length bytes in fp file, starting at start_pos offset.
 */
/*
static int FRemoveData(FILE *fp, unsigned long  start_pos, unsigned long data_length)
{
    size_t file_size, buffer_size;
    char *buffer;

    if(fp == NULL)
	return -1;

    // Get file size //
    fseek(fp, 0L, SEEK_END);
    file_size = ftell(fp);
    rewind(fp);

    // Clear end of file from start_pos? //
    if(data_length >= file_size - start_pos)
    {
	fseek(fp, start_pos, SEEK_SET);
	fputs("", fp);
	return 0;
    }

    buffer_size = file_size - start_pos + data_length;
    buffer = malloc(buffer_size + 1);
    if(buffer == NULL)
    {
	fprintf(stderr, "%s:%d: Can't allocate %ld bytes for buffer.\n", __FILE__, __LINE__, buffer_size);
	return -1;
    }

    // Copy file end data to buffer //
    fseek(fp, start_pos + data_length, SEEK_SET);
    if(fread(buffer, buffer_size, 1, fp) != 1)
    {
	fprintf(stderr, "%s:%d: Can't copy %ld file bytes to buffer.\n", __FILE__, __LINE__, buffer_size);
	free(buffer);
	return -1;
    }

    // Return to start_pos //
    fseek(fp, start_pos, SEEK_SET);

    // Write buffer //
    if(fwrite(buffer, buffer_size, 1, fp) != 1)
    {
	fprintf(stderr, "%s:%d: Can't write %ld buffer bytes at end of file.\n", __FILE__, __LINE__, data_length);
	free(buffer);
	return -1;
    }

    // Set cursor position at line start //
    if(fseek(fp, start_pos, SEEK_SET) != 0)
    {
	free(buffer);
	return -1;
    }

    free(buffer);
    return 0;
}
*/

/*
 * Add object number tags in editor scenery file.
 */
static int AddObjNumToSceneryFile(FILE *fp)
{
    const char *line_parm, *val;
    char *line_buf;
    int obj_num = 0;
    long offset;
    const size_t tag_string_length = strlen(TAGSTRING);

    if(fp == NULL)
	return -1;

    line_buf = NULL;
    while(1)
    {
	/* Delete previous line and load new line
	 * if new line is NULL then that implies end of file is
	 * reached
	 */
	free(line_buf);
	line_buf = NULL;

	/* Save current cursor position */
	offset = ftell(fp);

	/* Read line */
	line_buf = FGetStringLiteral(fp);

	if(line_buf == NULL)
	    break;

	/* Check if this is not the a comment or empty line and get the
	 * pointer to the parameter.
	 */
	line_parm = line_buf;
	while(ISBLANK(*line_parm))
	    line_parm++;
	if(ISCOMMENT(*line_parm))
	{
	    /* Comment don't start whith TAGSTRING? */
	    if(strncmp(line_parm, TAGSTRING, tag_string_length))
		continue;
	}

	/* Set val pointer to start of argument, can be NULL if
	 * there is no argument.
	 */
	val = line_parm;
	while(!ISBLANK(*val) && (*val != '\0'))
	    val++;
	while(ISBLANK(*val))
	    val++;

	/* Begin handling by parameter name */

	/* Tag for next ligne already exists? */
	if(strstr(line_parm, TAGSTRING) != NULL)
	{
	    /* Skip next line */

	    if(line_buf != NULL)
		free(line_buf);
	    line_buf = FGetStringLiteral(fp);
	    if(line_buf == NULL)
		break;
	}
	/* Object creation parameters */
	else if( strcasepfx(line_parm, "add_fire") ||
	    strcasepfx(line_parm, "create_fire") ||
	    strcasepfx(line_parm, "new_fire") ||

	    strcasepfx(line_parm, "add_helipad") ||
	    strcasepfx(line_parm, "create_helipad") ||
	    strcasepfx(line_parm, "new_helipad") ||

	    strcasepfx(line_parm, "add_human") ||
	    strcasepfx(line_parm, "create_human") ||
	    strcasepfx(line_parm, "new_human") ||

	    strcasepfx(line_parm, "add_object") ||
	    strcasepfx(line_parm, "create_object") ||
	    strcasepfx(line_parm, "new_object") ||

	    strcasepfx(line_parm, "add_premodeled") ||
	    strcasepfx(line_parm, "create_premodeled") ||
	    strcasepfx(line_parm, "new_premodeled") ||

	    strcasepfx(line_parm, "add_runway") ||
	    strcasepfx(line_parm, "create_runway") ||
	    strcasepfx(line_parm, "new_runway") ||

	    strcasepfx(line_parm, "add_smoke") ||
	    strcasepfx(line_parm, "create_smoke") ||
	    strcasepfx(line_parm, "new_smoke")
	)
	{
	    /* Set cursor position at line start */
	    if(fseek(fp, offset, SEEK_SET) != 0)
		return -1;

	    /* Prepare tag */
	    char *s = (char *)malloc(16 * sizeof(char));
	    snprintf(s, 15, "%s"TAGVALLENGTH"\n", TAGSTRING, obj_num);

	    /* Write tag */
	    if(FInsertData(fp, (unsigned long)offset, s, strlen(s)) != 0)
	    {
		free(s);
		return -1;
	    }
	    else
		free(s);

	    /* Skip line */
	    if(line_buf != NULL)
		free(line_buf);
	    line_buf = FGetStringLiteral(fp);
	    if(line_buf == NULL)
		break;

	    obj_num++;
	}
    }

    if(line_buf != NULL)
	free(line_buf);

    return 0;
}


/*
 * Get model file name from object number (in editor scenery file).
 * Editor scenery file must have be treated by AddObjNumToSceneryFile() first.
 */
char *GetObjectModelFileNameFromSceneryFile(sar_scenery_editor_struct *scn_ed,
					    int obj_num)
{
    const char *line_parm, *val;
    char *line_buf;
    long offset;
    char *obj_tag_s = (char *)malloc(16 * sizeof(char));
    char *arguments = NULL;
    Boolean obj_num_found = False;
    FILE *fp;

    if(scn_ed == NULL)
    {
	arguments = STRDUP("/");
	return arguments;
    }

    if(scn_ed->scn_file_fp == NULL)
    {
	arguments = STRDUP("/");
	return arguments;
    }

    fp = scn_ed->scn_file_fp;

    /* Go to top of file */
    rewind(fp);

    snprintf(obj_tag_s, 16, "%s"TAGVALLENGTH, TAGSTRING, obj_num);

    line_buf = NULL;
    while(1)
    {
	/* Delete previous line and load new line
	 * if new line is NULL then that implies end of file is
	 * reached
	 */
	if(line_buf != NULL)
	    free(line_buf);
	/* Save current cursor position */
	offset = ftell(fp);

	/* Read line */
	line_buf = FGetStringLiteral(fp);
	if(line_buf == NULL)
	    break;

	/* Check if this is a comment or empty line and get the
	 * pointer to the parameter.
	 */
	line_parm = line_buf;
	while(ISBLANK(*line_parm))
	    line_parm++;
	if(ISCOMMENT(*line_parm))
	    continue;

	/* Set val pointer to start of argument, can be NULL if
	 * there is no argument.
	 */
	val = line_parm;
	while(!ISBLANK(*val) && (*val != '\0'))
	    val++;
	while(ISBLANK(*val))
	    val++;

	/* Object number found? */
	if(strcasepfx(line_parm, obj_tag_s))
	{
	    obj_num_found = True;
	}
	/* Object number found and model_file definition line exists? */
	else if((obj_num_found == True) && strcasepfx(line_parm, "model_file"))
	{
	    arguments = STRDUP(val);
	    break;
	}
    }

    free(obj_tag_s);

    if(line_buf != NULL)
	free(line_buf);

    return arguments;
}

/*
 * Returns the number of the picker_obj_num object closest object.
 *
 * If the 'next' variable is True, next closest object number found in
 * scn_ed->pick_skip_list will be returned. Size of scn_ed->pick_skip_list is
 * hard coded in scn_ed structure.
 *
 * Returns -1 on error.
 */
static int SceneObjectPick(sar_core_struct *core_ptr, int picker_obj_num, Boolean next)
{
    sar_scene_struct *scene = core_ptr->scene;
    sar_object_struct *obj_ptr, *picker_obj_ptr;
    sar_scenery_editor_struct *scn_ed = core_ptr->in_game_editor;
    int picked_obj_num = -1, i;
    double distance, picked_obj_dist = DBL_MAX, temp_dbl;
    int *skip_list, pick_skip_list_index, skip_list_size;
    int total_objects = core_ptr->total_objects, temp_i;
    Boolean one_more_time = True;

    typedef struct {
	int obj_num;
	double obj_dist;
    } object_distance_struct;

    if(scene == NULL)
	return -1;

    picker_obj_ptr = core_ptr->object[picker_obj_num];
    if(picker_obj_ptr == NULL)
	return -1;

    skip_list = scn_ed->pick_skip_list;
    skip_list_size = sizeof(scn_ed->pick_skip_list)/sizeof(int);
    pick_skip_list_index = scn_ed->pick_skip_list_index;

    /* "next" closer object not requested? */
    if(next == False)
    {
	/* Iterate through each scene object, checking wich
	 * scene object is closer to the picker object.
	 */

	for(i = 0; i < total_objects; i++)
	{
	    /* Skip ourself */
	    if(i == picker_obj_num)
		continue;

	    /* Skip currrently edited object, if any */
	    if(i == scn_ed->cur_obj_num)
		continue;

	    obj_ptr = core_ptr->object[i];
	    if(obj_ptr == NULL)
		continue;

	    /* 3D distance */
	    distance = (float)SFMHypot3(
		obj_ptr->pos.x - picker_obj_ptr->pos.x,
		obj_ptr->pos.y - picker_obj_ptr->pos.y,
		obj_ptr->pos.z - picker_obj_ptr->pos.z
	    );
	    if(distance < picked_obj_dist)
	    {
		picked_obj_dist = distance;
		picked_obj_num = i;
	    }
	}

	/* Set skip_list as not initialized */
	pick_skip_list_index = -1;

	skip_list[0] = -1;
    }

    /* "next" closer object requested and skip_list not initialized? */
    if(next == True && pick_skip_list_index == -1)
    {
	/* Iterate through each scene object and generate the sorted
	 * obj_dist_list array which contains a by distance sorted
	 * list of all scene objects, then generate the skip_list which
	 * contains the closests objects.
	 */

	object_distance_struct obj_dist_list[total_objects];

	/* Populate obj_dist array */
	for(i = 0; i < total_objects; i++)
	{
	    /* Set default value */
	    obj_dist_list[i].obj_num = -1;

	    /* Skip ourself */
	    if(i == picker_obj_num)
		continue;

	    /* Skip currrently edited object, if any */
	    if(i == scn_ed->cur_obj_num)
		continue;

	    obj_ptr = core_ptr->object[i];
	    if(obj_ptr == NULL)
		continue;

	    /* 3D distance */
	    distance = SFMHypot3(
		obj_ptr->pos.x - picker_obj_ptr->pos.x,
		obj_ptr->pos.y - picker_obj_ptr->pos.y,
		obj_ptr->pos.z - picker_obj_ptr->pos.z
	    );

	    obj_dist_list[i].obj_num = i;
	    obj_dist_list[i].obj_dist = distance;
	}

	/* Sort obj_dist_list array by distance from picker object */
	while(one_more_time)
	{
	    one_more_time = False;

	    for(i = 1; i < total_objects; i++)
	    {
		if((obj_dist_list[i].obj_num >= 0) && (obj_dist_list[i].obj_dist < obj_dist_list[i - 1].obj_dist) )
		{
		    temp_i = obj_dist_list[i].obj_num;
		    temp_dbl = obj_dist_list[i].obj_dist;

		    obj_dist_list[i].obj_num = obj_dist_list[i - 1].obj_num;
		    obj_dist_list[i].obj_dist = obj_dist_list[i - 1].obj_dist;

		    obj_dist_list[i - 1].obj_num = temp_i;
		    obj_dist_list[i - 1].obj_dist = temp_dbl;

		    one_more_time = True;
		}
	    }
	}

	/*
	 * Fill skip_list
	 */

	for(i = 0; i < skip_list_size; i++)
	    skip_list[i] = obj_dist_list[i].obj_num;

	pick_skip_list_index = 0;
    }

    /* "next" closer object requested and skip_list initialized? */
    if(next == True && skip_list[0] >= 0)
    {
	pick_skip_list_index++;

	if(pick_skip_list_index >= skip_list_size)
	    pick_skip_list_index = 0;

	   picked_obj_num = skip_list[pick_skip_list_index];
    }

    scn_ed->pick_skip_list_index = pick_skip_list_index;

    return picked_obj_num;
}

/*
 * Load (or reload) an object.
 *
 * If new_obj_num is positive or null current new_obj_num object will
 * be deleted before the new object be created.
 *
 * New object will be created with given type and arg_list parameters,
 * then current cur_obj_arg and cur_obj_type values will be set.
 *
 * Returns newly created object number or a negative value on error.
 */
static int ScnEditLoadObject(sar_core_struct *core_ptr,
			     int obj_num,
			     sar_obj_type type,
			     const char *arg_list
)
{
    sar_scene_struct *scene;
    sar_scenery_editor_struct *scn_ed = core_ptr->in_game_editor;
    char **strv;
    int new_obj_num = -1, strc, i, j;
    float height_m;

    if(arg_list == NULL)
	return -1;

    scene = core_ptr->scene;
    if(scene == NULL)
	return -1;

    if(scn_ed == NULL)
	return -1;

     /* Parse argument */
    strv = strexp(arg_list, &strc);

    /* Shall the new object replace the current one? */
    if(obj_num >= 0)
    {
	/* Unlink current object from player object */
	scn_ed->cur_obj_num = -1;

	SARObjDelete(
	    core_ptr,
	    &core_ptr->object,
	    &core_ptr->total_objects,
	    obj_num
	);
    }

    /* Load the new object.
     * Note: new object number is always the first free number found in the
     * object list, so it is not guaranteed that this number will be equal
     * to the last deleted object number, especially if one or more objects
     * have been deleted before.
     */

    switch(type)
    {
	case SAR_OBJ_TYPE_GARBAGE:
	    new_obj_num = -1;
	    break;

	case SAR_OBJ_TYPE_STATIC:
	case SAR_OBJ_TYPE_AUTOMOBILE:
	case SAR_OBJ_TYPE_WATERCRAFT:
	case SAR_OBJ_TYPE_AIRCRAFT:

	    if(type == SAR_OBJ_TYPE_AIRCRAFT)
	    {
		/* TODO: check if it is really an issue.
		 * When I tried, aircraft model was successfully loaded but it
		 * was positionned at 0,0,0 and not linked to the player
		 * position as expected, and its heading follows the player
		 * position dynamically...
		 * Even if it can work, it may be a bad idea to do do that
		 * because it will need a lot of memory for a scenery "static"
		 * item!
		 */
SARMessageAdd(scene, "Sorry: adding a \"flyable\" aircraft as a scenery static item is not permitted.");
		new_obj_num = -1;
		break;
	    }
	    /* Create a new object */
	    new_obj_num = SARObjNew(
		scene, &core_ptr->object, &core_ptr->total_objects,
		type
	    );
	    if(new_obj_num < 0)
		break;

	    /* Load object model and data */
	    if(SARObjLoadFromFile(core_ptr, new_obj_num, arg_list) < 0)
	    {
		/* Object loading fails */

		char *s = (char *)malloc(NOTIFYSTRINGLENGTH * sizeof(char));
		snprintf(s, NOTIFYSTRINGLENGTH, "Can't load '%s' model.",
			arg_list
		    );
		SARMessageAdd(scene, (s));
		free(s);

		SARObjDelete(
		    core_ptr,
		    &core_ptr->object,
		    &core_ptr->total_objects,
		    new_obj_num
		);

		new_obj_num = -1;
	    }
	    else
	    {
		/* Object loading successfull */
	    }
	    break;

	case SAR_OBJ_TYPE_RUNWAY:
	    sar_parm_new_runway_struct *p_new_runway;

	    /* Not enough parameters? */
	    if(strc < 10)
	    {
		new_obj_num = -1;
		SARMessageAdd(scene, "Not enough parameters for runway.");
		break;
	    }

	    p_new_runway = malloc(sizeof(sar_parm_new_runway_struct));
	    if(p_new_runway == NULL)
	    {
		new_obj_num = -1;
		SARMessageAdd(scene, "Can't allocate memory for runway structure.");
		break;
	    }
	    else
	    {
		/* Set structure type */
		p_new_runway->type = SAR_PARM_NEW_RUNWAY;
	    }

	    /* Parse runway values:
	     * <range> <length> <width> <surface> <dashes> <edge_light_spacing>
	     * <north_label> <south_label> <north_displaced_threshold>
	     * <south_displaced_threshold>
	     */

	    p_new_runway->range = (float)MAX(ATOF(strv[0]), 1000); /* Visual range in meters */

	    /* Size (in meters) can't be less that the smallest runway in the world */
	    p_new_runway->length = (float)MAX(ATOF(strv[1]), 122.0);
	    p_new_runway->width = (float)MAX(ATOF(strv[2]), 6.0);

	    p_new_runway->surface_type =
		(sar_runway_surface_type)((ATOI(strv[3]) >= 0 && ATOI(strv[3]) < 4) ?
					    ATOI(strv[3]) : SAR_RUNWAY_SURFACE_CONCRETE);
	    p_new_runway->dashes = ATOI(strv[4]); /* Number of dashes (0 for none) */
	    p_new_runway->edge_light_spacing = ATOF(strv[5]); /* In meters (0.0 for none) */
	    p_new_runway->north_label = strv[6];
	    p_new_runway->south_label = strv[7];
	    p_new_runway->north_displaced_threshold = ATOF(strv[8]);
	    p_new_runway->north_displaced_threshold = ATOF(strv[9]);

	    /* Parse runway optional flags */

	    p_new_runway->flags = 0;
	    if(strc > 10)
	    {
		for(i = 10; i < strc; i++)
		{
		    if(!strcasecmp(strv[i], "thresholds"))
			p_new_runway->flags |= SAR_RUNWAY_FLAG_THRESHOLDS;
		    else if(!strcasecmp(strv[i], "borders"))
			p_new_runway->flags |= SAR_RUNWAY_FLAG_BORDERS;
		    else if(!strcasecmp(strv[i], "td_markers") ||
			    !strcasecmp(strv[i], "tdmarkers"))
			p_new_runway->flags |= SAR_RUNWAY_FLAG_TD_MARKERS;
		    else if(!strcasecmp(strv[i], "midway_markers") ||
			    !strcasecmp(strv[i], "midwaymarkers"))
			p_new_runway->flags |= SAR_RUNWAY_FLAG_MIDWAY_MARKERS;
		    else if(!strcasecmp(strv[i], "north_gs") ||
			    !strcasecmp(strv[i], "northgs"))
			p_new_runway->flags |= SAR_RUNWAY_FLAG_NORTH_GS;
		    else if(!strcasecmp(strv[i], "south_gs") ||
			    !strcasecmp(strv[i], "southgs"))
			p_new_runway->flags |= SAR_RUNWAY_FLAG_SOUTH_GS;
		}
	    }

	    new_obj_num = SARObjLoadRunway(core_ptr, scene, p_new_runway);

	    if(new_obj_num < 0)
		SARMessageAdd(scene, "Can't create runway.");

	    free(p_new_runway);
	    break;

	case SAR_OBJ_TYPE_HELIPAD:
	    sar_parm_new_helipad_struct *p_new_helipad;
	    /* Not enough parameters? */
	    if(strc < 10)
	    {
		new_obj_num = -1;
		SARMessageAdd(scene, "Not enough parameters for helipad.");
		break;
	    }
	    /* Too much parameters? */
	    else if(strc > 17)
	    {
		new_obj_num = -1;
		SARMessageAdd(scene, "Too much parameters for helipad.");
		break;
	    }

	    p_new_helipad = malloc(sizeof(sar_parm_new_helipad_struct));
	    if(p_new_helipad == NULL)
	    {
		new_obj_num = -1;
		SARMessageAdd(scene, "Can't allocate memory for helipad structure.");
		break;
	    }
	    else
	    {
		/* Set structure type */
		p_new_helipad->type = SAR_PARM_NEW_HELIPAD;
	    }

	    /* Parse helipad mandatory values:
	     * <style> <length> <width> <recession> <label>
	     */
	    if(!strcasecmp(strv[0], SAR_HELIPAD_STYLE_DEFAULT_S))
		p_new_helipad->style = STRDUP(SAR_HELIPAD_STYLE_DEFAULT_S);
	    else if(!strcasecmp(strv[0], SAR_HELIPAD_STYLE_STANDARD_S))
		p_new_helipad->style = STRDUP(SAR_HELIPAD_STYLE_STANDARD_S);
	    else if(!strcasecmp(strv[0], SAR_HELIPAD_STYLE_GROUND_PAVED_S))
		p_new_helipad->style = STRDUP(SAR_HELIPAD_STYLE_GROUND_PAVED_S);
	    else if(!strcasecmp(strv[0], SAR_HELIPAD_STYLE_GROUND_BARE_S))
		p_new_helipad->style = STRDUP(SAR_HELIPAD_STYLE_GROUND_BARE_S);
	    else if(!strcasecmp(strv[0], SAR_HELIPAD_STYLE_BUILDING_S))
		p_new_helipad->style = STRDUP(SAR_HELIPAD_STYLE_BUILDING_S);
	    else if(!strcasecmp(strv[0], SAR_HELIPAD_STYLE_VEHICLE_S))
		p_new_helipad->style = STRDUP(SAR_HELIPAD_STYLE_VEHICLE_S);
	    p_new_helipad->length = (float)MAX(ATOF(strv[1]), 2.0); /* Size (in meters) */
	    p_new_helipad->width = (float)MAX(ATOF(strv[2]), 2.0);

	    height_m = (float)SFMFeetToMeters((double)ATOF(strv[3]));
	    p_new_helipad->recession = (height_m >= 0 ? height_m : 0);

	    p_new_helipad->label = strv[4];

	    /* Parse helipad mandatory flags:
	     * <edge_lighting> <has_fuel> <has_repair> <has_drop_off>
	     * <restarting_point>
	     */
	    p_new_helipad->flags = 0;
	    for(i = 5; i < 10; i++)
	    {
		if(!strcasecmp(strv[i], "y") || !strcmp(strv[i], "1"))
		    p_new_helipad->flags |= SAR_HELIPAD_FLAG_EDGE_LIGHTING;
		if(!strcasecmp(strv[i], "y") || !strcmp(strv[i], "1"))
		    p_new_helipad->flags |= SAR_HELIPAD_FLAG_FUEL;
		if(!strcasecmp(strv[i], "y") || !strcmp(strv[i], "1"))
		    p_new_helipad->flags |= SAR_HELIPAD_FLAG_REPAIR;
		if(!strcasecmp(strv[i], "y") || !strcmp(strv[i], "1"))
		    p_new_helipad->flags |= SAR_HELIPAD_FLAG_DROPOFF;
		if(!strcasecmp(strv[i], "y") || !strcmp(strv[i], "1"))
		    p_new_helipad->flags |= SAR_HELIPAD_FLAG_RESTART_POINT;
	    }

	    /* Helipad reference object (if any) */
	    if(strc > 10)
	    {
		/* Parse helipad optional reference object values */
		p_new_helipad->ref_obj_name = strv[10];
		p_new_helipad->ref_offset.x = ATOF(strv[11]);
		p_new_helipad->ref_offset.y = ATOF(strv[12]);
		height_m = (float)SFMFeetToMeters((double)ATOF(strv[13]));
		p_new_helipad->ref_offset.z = height_m;
		p_new_helipad->ref_dir.heading = ATOF(strv[14]);
		p_new_helipad->ref_dir.pitch = ATOF(strv[15]);
		p_new_helipad->ref_dir.bank = ATOF(strv[16]);
	    }
	    else
		p_new_helipad->ref_obj_name = NULL;

	    new_obj_num = SARObjLoadHelipad(core_ptr, scene, p_new_helipad);

	    if(new_obj_num < 0)
		SARMessageAdd(scene, "Can't create helipad.");

	    free(p_new_helipad);
	    break;

	case SAR_OBJ_TYPE_HUMAN:
	    sar_parm_new_human_struct *p_new_human;

	    /* Not enough parameters? */
	    if(strc < 1)
	    {
		new_obj_num = -1;
		SARMessageAdd(scene, "Not enough parameters for human.");
		break;
	    }

	    p_new_human = malloc(sizeof(sar_parm_new_human_struct));
	    if(p_new_human == NULL)
	    {
		new_obj_num = -1;
		SARMessageAdd(scene, "Can't allocate memory for human structure.");
		break;
	    }
	    else
	    {
		/* Set structure type */
		p_new_human->type = SAR_PARM_NEW_HUMAN;
	    }

	    /* Parse human mandatory value: <type_name> */
	    p_new_human->type_name = strv[0];

	    /* Parse human optional flags ans values:
	     * <need_rescue> <sit_up> <sit_down> <sitting> <lying> <alert>
	     * <aware> <in_water> <on_stretcher> <assisted n type_name(s)>
	     */
	    p_new_human->flags = 0;
	    p_new_human->assisting_humans = 0;
	    i = 1;
	    while(i < strc)
	    {
		if(!strcasecmp(strv[i], "need_rescue"))
		    p_new_human->flags |= SAR_HUMAN_FLAG_NEED_RESCUE;
		else if(!strcasecmp(strv[i], "sit_up"))
		    p_new_human->flags |= SAR_HUMAN_FLAG_SIT_UP;
		else if(!strcasecmp(strv[i], "sit_down"))
		    p_new_human->flags |= SAR_HUMAN_FLAG_SIT_DOWN;
		else if(!strcasecmp(strv[i], "sitting"))
		    p_new_human->flags |= SAR_HUMAN_FLAG_SIT;
		else if(!strcasecmp(strv[i], "lying"))
		    p_new_human->flags |= SAR_HUMAN_FLAG_LYING;
		else if(!strcasecmp(strv[i], "alert"))
		    p_new_human->flags |= SAR_HUMAN_FLAG_ALERT;
		else if(!strcasecmp(strv[i], "aware"))
		    p_new_human->flags |= SAR_HUMAN_FLAG_AWARE;
		else if(!strcasecmp(strv[i], "in_water"))
		    p_new_human->flags |= SAR_HUMAN_FLAG_IN_WATER;
		else if(!strcasecmp(strv[i], "on_stretcher"))
		    p_new_human->flags |= SAR_HUMAN_FLAG_ON_STRETCHER;
		else if(!strcasecmp(strv[i], "assisted"))
		{
		    int assistants;

		    i++;
		    assistants = ATOI(strv[i]);
		    p_new_human->assisting_humans = assistants;
		    for(j = 0; j < assistants; j++)
		    {
			i++;

			p_new_human->assisting_human_preset_name[j] = strv[i];
			if(i >= strc)
			{
			    SARMessageAdd(scene, "Missing assistant(s) name(s).");
			    break;
			}
		    }
		}
		i++;
	    }

	    new_obj_num = SARObjLoadHuman(core_ptr, scene, p_new_human);

	    if(new_obj_num < 0)
		SARMessageAdd(scene, "Can't create human.");

	    free(p_new_human);
	    break;

	case SAR_OBJ_TYPE_SMOKE:
	    sar_parm_new_smoke_struct *p_new_smoke;

	    /* Not enough parameters? */
	    if(strc < 10)
	    {
		new_obj_num = -1;
		SARMessageAdd(scene, "Not enough parameters for smoke.");
		break;
	    }
	    /* Too much parameters? */
	    else if(strc > 10)
	    {
		new_obj_num = -1;
		SARMessageAdd(scene, "Too much parameters for smoke.");
		break;
	    }

	    p_new_smoke = malloc(sizeof(sar_parm_new_smoke_struct));
	    if(p_new_smoke == NULL)
	    {
		new_obj_num = -1;
		SARMessageAdd(scene, "Can't allocate memory for smoke structure.");
		break;
	    }
	    else
	    {
		/* Set structure type */
		p_new_smoke->type = SAR_PARM_NEW_SMOKE;
	    }

	    /* Parse smoke values:
	     * <x_offset> <y_offset> <z_offset> <r_st> <r_max> <r_rate>
	     * <hide@max> <respawn_int> <units> <color_code>
	     */
	    p_new_smoke->offset.x = ATOF(strv[0]);
	    p_new_smoke->offset.y = ATOF(strv[1]);
	    p_new_smoke->offset.z = ATOF(strv[2]);
	    p_new_smoke->radius_start = ATOF(strv[3]);
	    p_new_smoke->radius_max = ATOF(strv[4]);
	    p_new_smoke->radius_rate = ATOF(strv[5]);
	    p_new_smoke->hide_at_max = ATOI(strv[6]);
	    p_new_smoke->respawn_int = (time_t)ATOL(strv[7]);
	    p_new_smoke->total_units = ATOI(strv[8]);
	    p_new_smoke->color_code = ATOI(strv[9]);

	    new_obj_num = SARObjLoadSmoke(core_ptr, scene, p_new_smoke);

	    if(new_obj_num < 0)
		SARMessageAdd(scene, "Can't create smoke.");

/*
	    // Set object name /

	    const char *s;
	    switch(strv[0])
	    {
		case 0:
		    s = "Smoke light";
		    break;
		case 1:
		    s = "Smoke medium";
		break;
		case 2:
		    s = "Smoke dark";
		break;ScnEditLoadObject
		case 3:
		    s = "Smoke orange";
		break;
		default:
		    s = "Smoke light";
		break;
	    }

	    obj_ptr = (*&core_ptr->object)[new_obj_num];
	    if(obj_ptr->name != NULL)
		free(obj_ptr->name);
	    obj_ptr->name = STRDUP(s);
*/
	    free(p_new_smoke);
	    break;

	case SAR_OBJ_TYPE_FIRE:
	    sar_parm_new_fire_struct *p_new_fire;

	    /* Not enough parameters? */
	    if(strc < 2)
	    {
		new_obj_num = -1;
		SARMessageAdd(scene, "Not enough parameters for fire.");
		break;
	    }
	    /* Too much parameters? */
	    else if(strc > 2)
	    {
		new_obj_num = -1;
		SARMessageAdd(scene, "Too much parameters for fire.");
		break;
	    }

	    p_new_fire = malloc(sizeof(sar_parm_new_fire_struct));
	    if(p_new_fire == NULL)
	    {
		new_obj_num = -1;
		SARMessageAdd(scene, "Can't allocate memory for fire structure.");
		break;
	    }
	    else
	    {
		/* Set structure type */
		p_new_fire->type = SAR_PARM_NEW_FIRE;
	    }

	    /* Parse fire values: <radius> <height> */
	    p_new_fire->radius = ATOF(strv[0]);
	    height_m = (float)SFMFeetToMeters((double)ATOF(strv[1]));
	    p_new_fire->height = height_m;

	    new_obj_num = SARObjLoadFire(core_ptr, scene, p_new_fire);

	    if(new_obj_num < 0)
		SARMessageAdd(scene, "Can't create fire.");

	    free(p_new_fire);
	    break;

	case SAR_OBJ_TYPE_PREMODELED:
	    sar_parm_new_premodeled_struct *p_new_premodeled;
	    Boolean okay;
	    char s[NOTIFYSTRINGLENGTH + 1];

	    /* Parameters number will be checked later by premodeled type */

	    p_new_premodeled = malloc(sizeof(sar_parm_new_premodeled_struct));
	    if(p_new_premodeled == NULL)
	    {
		new_obj_num = -1;
		SARMessageAdd(scene, "Can't allocate memory for premodeled structure.");
		break;
	    }
	    else
	    {
		/* Set structure type */
		p_new_premodeled->type = SAR_PARM_NEW_PREMODELED;
	    }

	    /* Premodeled type value ok? */
	    if(!strcasecmp(strv[0], SAR_PREMODELED_POWER_TRANSMISSION_TOWER_S) ||
		!strcasecmp(strv[0], SAR_PREMODELED_RADIO_TOWER_S) ||
		!strcasecmp(strv[0], SAR_PREMODELED_TOWER_S) ||
		!strcasecmp(strv[0], SAR_PREMODELED_CONTROL_TOWER_S) ||
		!strcasecmp(strv[0], SAR_PREMODELED_BUILDING_S)
	    )
	    {
		p_new_premodeled->model_type = strv[0];
	    }
	    else
	    {
		new_obj_num = -1;
		SARMessageAdd(scene, "Unknown premodeled type.");
		break;
	    }

	    /* Check parameters number by premodeled type: */
	    okay = False;
	    if((!strcasecmp(strv[0], SAR_PREMODELED_POWER_TRANSMISSION_TOWER_S) ||
		!strcasecmp(strv[0], SAR_PREMODELED_RADIO_TOWER_S) ||
		!strcasecmp(strv[0], SAR_PREMODELED_TOWER_S)) &&
		strc == 4
	    )
		okay = True;
	    else if(!strcasecmp(strv[0], SAR_PREMODELED_CONTROL_TOWER_S) && strc == 7)
		okay = True;
	    else if(!strcasecmp(strv[0], SAR_PREMODELED_BUILDING_S) && strc == 8)
		okay = True;
	    if(okay == False)
	    {
		new_obj_num = -1;
		SARMessageAdd(scene,
		    "Too much or not enough parameters for premodeled."
		    );
		break;
	    }

	    /* Check textures availability */
	    okay = True;
	    if(!strcasecmp(strv[0], SAR_PREMODELED_CONTROL_TOWER_S))
	    {
		for(i = 5; i < 7; i++)
		    if(SARGetTextureRefNumberByName(scene, (const char *)strv[i]) < 0)
		    {
			snprintf(s, NOTIFYSTRINGLENGTH,
				"Texture '%s' not loaded in this scenery.",
				strv[i]
				);
			SARMessageAdd(scene, s);
			okay = False;
		    };
	    }
	    else if(!strcasecmp(strv[0], SAR_PREMODELED_BUILDING_S))
	    {
		for(i = 5; i < 8; i++)
		    if(SARGetTextureRefNumberByName(scene, (const char *)strv[i]) < 0)
		    {
			snprintf(s, NOTIFYSTRINGLENGTH,
				"Texture '%s' not loaded in this scenery.",
				strv[i]
				);
			SARMessageAdd(scene, s);
			okay = False;
		    };
	    }
	    if(okay == False)
	    {
		new_obj_num = -1;
		break;
	    }

	    /* Set premodeled type arguments number and arguments values */

	    p_new_premodeled->argc = strc - 1;
	    p_new_premodeled->argv = malloc(p_new_premodeled->argc * sizeof(char **));

	    for(i = 0; i < p_new_premodeled->argc; i++)
		p_new_premodeled->argv[i] = strv[i + 1];

	    new_obj_num = SARObjPremodeledNew(
		core_ptr, scene,
		strv[0],
		p_new_premodeled->argc,
		p_new_premodeled->argv
	    );

	    if(new_obj_num < 0)
		SARMessageAdd(scene, "Can't create premodeled.");

	    free(p_new_premodeled);
	    break;

	default:
	    SARMessageAdd(scene, "Unknown model type.");
	    break;
    }

    if(new_obj_num < 0)
    {
	scn_ed->cur_obj_type = SAR_OBJ_TYPE_GARBAGE;
	free(scn_ed->cur_obj_arg);
	scn_ed->cur_obj_arg = NULL;

	return new_obj_num;
    }
    else
    {
	scn_ed->cur_obj_type = type;

	char *s = STRDUP(arg_list);
	free(scn_ed->cur_obj_arg);
	scn_ed->cur_obj_arg = STRDUP(s);
	free(s);

	/* As it links current object to player object, let scn_ed->cur_obj_num
	 * be set by calling function.
	 */
    }

    strlistfree(strv, strc);

    editor_modified_object_struct *modification;

    /* Not enough entries in modification_list? */
    if((scn_ed->total_objects - 1) < new_obj_num)
    {
	editor_object_data_struct *editor_obj_data;

	scn_ed->modification_list = realloc(
			scn_ed->modification_list,
			++(scn_ed->total_objects) *
			sizeof(editor_modified_object_struct *)
		    );
	if(scn_ed->modification_list == NULL)
	{
	    fprintf(stderr, "%s:%d: Memory allocation error.\n",
		    __FILE__, __LINE__);
	    return -1;
	}
	modification = malloc(sizeof(editor_modified_object_struct));
	if(modification == NULL)
	{
	    fprintf(stderr, "%s:%d: Memory allocation error.\n",
		    __FILE__, __LINE__);
	    return -1;
	}
	scn_ed->modification_list[new_obj_num] = modification;
	modification->flags = 0;
	modification->obj_data_original = NULL;
	editor_obj_data = EditorObjectDataStructNew();
	modification->obj_data_new = editor_obj_data;
    }
    else
    {
	/* Note: needed when object was deleted then reused later */
	modification = scn_ed->modification_list[new_obj_num];
	if(modification->flags & EDITOR_OBJECT_FLAG_DELETED)
	    modification->flags &= ~EDITOR_OBJECT_FLAG_DELETED;

	modification->flags |= EDITOR_OBJECT_FLAG_MODIFIED;
    }

    return new_obj_num;
}

/*
 *	Returns the pointer to the string wich contains the name of the texture
 *	referenced by texture index.
 * 	Returns NULL if texture index or texture name was not found.
 */
static char *ScnEditGetTextureNamePtrByRef(
	sar_scene_struct *scene, int tex_index
)
{
	int i, total;
	v3d_texture_ref_struct *t, **ptr;

	if((scene == NULL))
	    return NULL;

	ptr = scene->texture_ref;
	total = scene->total_texture_refs;

	for(i = 0; i < total; i++)
	{
	    t = ptr[i];
	    if((t != NULL) ? (t->name == NULL) : 1)
		continue;

	    if(i == tex_index)
	    {
		if(t->name != NULL)
		    return t->name;
		else
		    return NULL;
	    }
	}
	return NULL;
}


/*
 *	Returns a string wich contains the human preset name of the human
 * 	pointed structure. Returns NULL if not found.
 *	String must be freed by calling function.
 */
static char *ScnEditGetHumanPresetNameByHumanPtr(
	sar_core_struct *core_ptr, sar_object_human_struct *human
)
{
	char *human_preset_name = NULL;
	sar_human_data_entry_struct *entry;
	sar_human_data_struct *hd = core_ptr->human_data;
	sar_color_struct *ent_color, *hum_color;
	Boolean colors_matches, gender_matches, found = False;
	int i, j;

	/* Iterate through all human presets */
	for(i = 0; i < hd->total_presets; i++)
	{
	    found = False;

	    entry = hd->preset[i];
	    if(entry == NULL)
		continue;

	    gender_matches = False;
	    if(((entry->preset_entry_flags & SAR_HUMAN_FLAG_GENDER_FEMALE) == SAR_HUMAN_FLAG_GENDER_FEMALE) &&
		((human->flags & SAR_HUMAN_FLAG_GENDER_FEMALE) == SAR_HUMAN_FLAG_GENDER_FEMALE)
	    )
		gender_matches = True;
	    else if(((entry->preset_entry_flags & SAR_HUMAN_FLAG_GENDER_FEMALE) != SAR_HUMAN_FLAG_GENDER_FEMALE) &&
		((human->flags & SAR_HUMAN_FLAG_GENDER_FEMALE) != SAR_HUMAN_FLAG_GENDER_FEMALE)
	    )
		gender_matches = True;

	    if(gender_matches == True &&
		(entry->mass == human->mass) &&
		(entry->height == human->height)
	    )
	    {
		/* Check colors */

		colors_matches = True;
		for(j = 0; j < SAR_HUMAN_COLORS_MAX; j++)
		{
		    ent_color = &entry->color[j];
		    hum_color = &human->color[j];

		    if((ent_color->r != hum_color->r) ||
			(ent_color->g != hum_color->g) ||
			(ent_color->b != hum_color->b) ||
			(ent_color->a != hum_color->a)
		    )
			colors_matches = False;
		}

		if(colors_matches == True)
		{
		    /* Gender, mass, height and color matches. For backward
		    * compatibility, check if human is "default" or
		    * "victim_streatcher_assisted" preset.
		    */

		    /* Is this entry preset the victim_streatcher_assisted one? */
		    if(human->assisting_humans == 1)
		    {
			/* Has this human one assistantt? */
			if(entry->assisting_humans == 1)
			    found = True;
			else
			    found = False;
		    }
		    else
			found = True;
		}

		if(found == True)
		{
		    human_preset_name = STRDUP(entry->name);
		    break;
		}
	    }
	}
	if(found == False)
	    human_preset_name = STRDUP("(unknown)");

	return human_preset_name;
}





/*
 * Re-initialize a data structure created by EditorObjectDataStructNew().
 * Returns non 0 value on error.
 */
static int EditorObjectDataStructReinit(editor_object_data_struct *object_data)
{
    int i;

    if(object_data == NULL)
	return 1;

    free(object_data->type_s);
    object_data->type_s = NULL;
    free(object_data->name);
    object_data->name = NULL;
    free(object_data->obj_name);
    object_data->obj_name = NULL;
    free(object_data->ref_obj_name);
    object_data->ref_obj_name = NULL;
    free(object_data->object_map_description);
    object_data->object_map_description = NULL;
    free(object_data->style_s);
    object_data->style_s = NULL;
    free(object_data->label);
    object_data->label = NULL;
    free(object_data->type_name);
    object_data->type_name = NULL;
    free(object_data->need_rescue_s);
    object_data->need_rescue_s = NULL;
    free(object_data->sit_up_s);
    object_data->sit_up_s = NULL;
    free(object_data->sit_down_s);
    object_data->sit_down_s = NULL;
    free(object_data->sitting_s);
    object_data->sitting_s = NULL;
    free(object_data->lying_s);
    object_data->lying_s = NULL;
    free(object_data->alert_s);
    object_data->alert_s = NULL;
    free(object_data->aware_s);
    object_data->aware_s = NULL;
    free(object_data->in_water_s);
    object_data->in_water_s = NULL;
    free(object_data->on_stretcher_s);
    object_data->on_stretcher_s = NULL;
    free(object_data->assisted_s);
    object_data->assisted_s = NULL;

    for(i = 0; i < SAR_ASSISTING_HUMANS_MAX; i++)
    {
	free(object_data->assist_type_name[i]);
	object_data->assist_type_name[i] = NULL;
    }
    free(object_data->human_displacement_dir_s);
    object_data->human_displacement_dir_s = NULL;
    free(object_data->file_name);
    object_data->file_name = NULL;
    free(object_data->pm_type_s);
    object_data->pm_type_s = NULL;
    free(object_data->walls_texture_s);
    object_data->walls_texture_s = NULL;
    free(object_data->walls_texture_night_s);
    object_data->walls_texture_night_s = NULL;
    free(object_data->roof_texture_s);
    object_data->roof_texture_s = NULL;
    free(object_data->surface_type_s);
    object_data->surface_type_s = NULL;
    free(object_data->north_label);
    object_data->north_label = NULL;
    free(object_data->south_label);
    object_data->south_label = NULL;
    free(object_data->has_thresholds_s);
    object_data->has_thresholds_s = NULL;
    free(object_data->has_borders_s);
    object_data->has_borders_s = NULL;
    free(object_data->has_td_markers_s);
    object_data->has_td_markers_s = NULL;
    free(object_data->has_midway_markers_s);
    object_data->has_midway_markers_s = NULL;
    free(object_data->has_north_gs_s);
    object_data->has_north_gs_s = NULL;
    free(object_data->has_south_gs_s);
    object_data->has_south_gs_s = NULL;

    memset(object_data, 0, sizeof(editor_object_data_struct));

    return 0;
}




/*
 * Frees a data structure created by EditorObjectDataStructNew().
 * Returns non 0 value on error.
 */
static int EditorObjectDataStructFree(editor_object_data_struct *object_data)
{
    if(object_data == NULL)
	return 1;

    /* Free allocated strings */
    EditorObjectDataStructReinit(object_data);

    free(object_data);

    return 0;
}




/*
 * Returns a string containing object type name.
 * Returns NULL on error.
 */
static const char* SceneObjectGetTypeName(sar_core_struct *core_ptr, int obj_num)
{
    const char *obj_type_name;
    sar_object_struct *obj_ptr;

    if(obj_num < 0)
	return NULL;

    obj_ptr = core_ptr->object[obj_num];
    if(obj_ptr != NULL)
    {
	/* Get object type name */
	switch(obj_ptr->type)
	{
	    case SAR_OBJ_TYPE_GARBAGE:
		obj_type_name = SAR_OBJ_TYPE_GARBAGE_S;
		break;
	    case SAR_OBJ_TYPE_STATIC:
		obj_type_name = SAR_OBJ_TYPE_STATIC_S;
		break;

	    case SAR_OBJ_TYPE_AUTOMOBILE:
		obj_type_name = SAR_OBJ_TYPE_AUTOMOBILE_S;
		break;

	    case SAR_OBJ_TYPE_WATERCRAFT:
		obj_type_name = SAR_OBJ_TYPE_WATERCRAFT_S;
		break;

	    case SAR_OBJ_TYPE_AIRCRAFT:
		obj_type_name = SAR_OBJ_TYPE_AIRCRAFT_S;
		break;

	    case SAR_OBJ_TYPE_GROUND:
		obj_type_name = SAR_OBJ_TYPE_GROUND_S;
		break;

	    case SAR_OBJ_TYPE_RUNWAY:
		obj_type_name = SAR_OBJ_TYPE_RUNWAY_S;
		break;

	    case SAR_OBJ_TYPE_HELIPAD:
		obj_type_name = SAR_OBJ_TYPE_HELIPAD_S;
		break;

	    case SAR_OBJ_TYPE_HUMAN:
		obj_type_name = SAR_OBJ_TYPE_HUMAN_S;
		break;

	    case SAR_OBJ_TYPE_SMOKE:
		obj_type_name = SAR_OBJ_TYPE_SMOKE_S;
		break;

	    case SAR_OBJ_TYPE_FIRE:
		obj_type_name = SAR_OBJ_TYPE_FIRE_S;
		break;

	    case SAR_OBJ_TYPE_EXPLOSION:
		obj_type_name = SAR_OBJ_TYPE_EXPLOSION_S;
		break;

	    case SAR_OBJ_TYPE_CHEMICAL_SPRAY:
		obj_type_name = SAR_OBJ_TYPE_CHEMICAL_SPRAY_S;
		break;

	    case SAR_OBJ_TYPE_FUELTANK:
		obj_type_name = SAR_OBJ_TYPE_FUELTANK_S;
		break;

	    case SAR_OBJ_TYPE_PREMODELED:
		obj_type_name = SAR_OBJ_TYPE_PREMODELED_S;
		break;

	    default:
		obj_type_name = "(unknown)";
		break;
	}
	return obj_type_name;
    }
    else
	return NULL;
}


/*
 * Return a string containing premodeled object type name.
 * Return NULL on error.
 */
static const char* SceneObjectGetPremodeledTypeName(sar_core_struct *core_ptr, int obj_num)
{
    const char *premodeled_type_name;
    sar_object_struct *obj_ptr;
    sar_object_premodeled_struct *premodeled;

    if(obj_num < 0)
	return NULL;

    obj_ptr = core_ptr->object[obj_num];
    if(obj_ptr == NULL)
	return NULL;

    premodeled = SAR_OBJ_GET_PREMODELED(obj_ptr);
    if(premodeled == NULL)
	return NULL;

    /* Get object type name */
    switch(premodeled->type)
    {
	case SAR_OBJ_PREMODELED_BUILDING:
	    premodeled_type_name = SAR_PREMODELED_BUILDING_S;
	    break;
	case SAR_OBJ_PREMODELED_CONTROL_TOWER:
	    premodeled_type_name = SAR_PREMODELED_CONTROL_TOWER_S;
	    break;
	case SAR_OBJ_PREMODELED_HANGAR:
	    premodeled_type_name = SAR_PREMODELED_HANGAR_S;
	    break;
	case SAR_OBJ_PREMODELED_POWER_TRANSMISSION_TOWER:
	    premodeled_type_name = SAR_PREMODELED_POWER_TRANSMISSION_TOWER_S;
	    break;
	case SAR_OBJ_PREMODELED_TOWER:
	    premodeled_type_name = SAR_PREMODELED_TOWER_S;
	    break;
	case SAR_OBJ_PREMODELED_RADIO_TOWER:
	    premodeled_type_name = SAR_PREMODELED_RADIO_TOWER_S;
	    break;
	default:
	    premodeled_type_name = SAR_PREMODELED_UNKNOWN_S;
	    break;
    }
    return premodeled_type_name;
}


/*
 * Return a pointer to the object file name (relative to current
 * SAR_DEF_ENV_GLOBAL_DIR), or NULL if file name was not found.
 */
char *getObjectFileName(sar_object_struct *obj_ptr)
{
    sar_visual_model_struct *vmodel = obj_ptr->visual_model;
    char *file_name = NULL;

    if(vmodel != NULL && vmodel->filename != NULL)
    {
	file_name = vmodel->filename + strlen(dname.global_data);
	if(file_name[0] == '/')
	    file_name++;
    }

    return file_name;
}


/*
 * Add a new editor_object_data_struct structure
 */
static editor_object_data_struct *EditorObjectDataStructNew(void)
{
    editor_object_data_struct *editor_obj_data;

    editor_obj_data = calloc(1, sizeof(editor_object_data_struct));
    if(editor_obj_data == NULL)
	fprintf(stderr, "%s:%d: Memory allocation error.\n", __FILE__, __LINE__);

    return editor_obj_data;
}


/*
 * Fill an editor_object_data_struct structure.
 *
 * Any parameter added in this function must be added
 * in the EditorObjectDataStructFree() function.
 */
static int EditorObjectDataStructFill(
    sar_core_struct *core_ptr,
    editor_object_data_struct *editor_obj_data,
    int obj_num
)
{
    sar_scenery_editor_struct *scn_ed;
    sar_object_runway_struct *runway = NULL;
    sar_object_helipad_struct *helipad = NULL;
    sar_object_human_struct *human = NULL;
    sar_object_smoke_struct *smoke = NULL;
    sar_object_fire_struct *fire = NULL;
    sar_object_premodeled_struct *premodeled = NULL;
    sar_object_struct *obj_ptr;
    char *tex_ref_name = NULL;
    int i;

    scn_ed = core_ptr->in_game_editor;
    if(scn_ed == NULL)
	return -1;

    if(editor_obj_data == NULL)
	return -1;

    if(obj_num < 0)
	return obj_num;

    obj_ptr = core_ptr->object[obj_num];
    if(obj_ptr == NULL)
	return -1;

    /* Free allocated strings (if any) */
    EditorObjectDataStructReinit(editor_obj_data);

    /* Set type */
    editor_obj_data->type = obj_ptr->type;

    /* Set object name */
    /* TODO How to get it?
     * editor_obj_data->name = STRDUP(???);
     */

    /* Set object name (the name wich can be used as a reference name) */
    editor_obj_data->obj_name = STRDUP(obj_ptr->name);

    /* Set object map description */
    /* TODO How to get it?
     * editor_obj_data->object_map_description = STRDUP(???);
     */

    /* Set object location */
    memcpy(&editor_obj_data->pos, &obj_ptr->pos, sizeof(sar_position_struct));
    /* Convert height unit */
    editor_obj_data->pos.z =
		(float)SFMMetersToFeet((double)editor_obj_data->pos.z);

    /* Set object attitude */
    memcpy(&editor_obj_data->dir, &obj_ptr->dir, sizeof(sar_direction_struct));
    /* Convert radians to degrees */
    editor_obj_data->dir.heading =
		SFMRadiansToDegrees((double)editor_obj_data->dir.heading);
    editor_obj_data->dir.pitch =
		SFMRadiansToDegrees((double)editor_obj_data->dir.pitch);
    editor_obj_data->dir.bank =
		SFMRadiansToDegrees((double)editor_obj_data->dir.bank);

    switch(editor_obj_data->type)
    {
	case SAR_OBJ_TYPE_GARBAGE:
	    break;

	case SAR_OBJ_TYPE_STATIC:
	case SAR_OBJ_TYPE_AUTOMOBILE:
	case SAR_OBJ_TYPE_WATERCRAFT:
	case SAR_OBJ_TYPE_AIRCRAFT:
	case SAR_OBJ_TYPE_GROUND:
	    /* Set data structure type_s string value */
	    switch(editor_obj_data->type)
	    {
		case SAR_OBJ_TYPE_STATIC:
		    editor_obj_data->type_s = STRDUP(SAR_OBJ_TYPE_STATIC_S);
		    break;
		case SAR_OBJ_TYPE_AUTOMOBILE:
		    editor_obj_data->type_s = STRDUP(SAR_OBJ_TYPE_AUTOMOBILE_S);
		    break;
		case SAR_OBJ_TYPE_WATERCRAFT:
		    editor_obj_data->type_s = STRDUP(SAR_OBJ_TYPE_WATERCRAFT_S);
		    break;
		case SAR_OBJ_TYPE_AIRCRAFT:
		    editor_obj_data->type_s = STRDUP(SAR_OBJ_TYPE_AIRCRAFT_S);
		    break;
		case SAR_OBJ_TYPE_GROUND:
		    editor_obj_data->type_s = STRDUP(SAR_OBJ_TYPE_GROUND_S);
		    break;
		default:
		    editor_obj_data->type_s = NULL;
		    break;
	    }
	    editor_obj_data->file_name = STRDUP(getObjectFileName(obj_ptr));

	    break;

	case SAR_OBJ_TYPE_RUNWAY:
	    runway = SAR_OBJ_GET_RUNWAY(obj_ptr);
	    if(runway == NULL)
		break;

	    editor_obj_data->type_s = STRDUP(SAR_OBJ_TYPE_RUNWAY_S);

	    editor_obj_data->range = obj_ptr->range;
	    editor_obj_data->length = runway->length;
	    editor_obj_data->width = runway->width;

	    editor_obj_data->surface_type = runway->surface_type;
	    switch(editor_obj_data->surface_type)
	    {
		case SAR_RUNWAY_SURFACE_PAVED:
		    editor_obj_data->surface_type_s = STRDUP(SAR_RUNWAY_SURFACE_PAVED_S);
		    break;
		case SAR_RUNWAY_SURFACE_GRAVEL:
		    editor_obj_data->surface_type_s = STRDUP(SAR_RUNWAY_SURFACE_GRAVEL_S);
		    break;
		case SAR_RUNWAY_SURFACE_CONCRETE:
		    editor_obj_data->surface_type_s = STRDUP(SAR_RUNWAY_SURFACE_CONCRETE_S);
		    break;
		case SAR_RUNWAY_SURFACE_GROVED:
		    editor_obj_data->surface_type_s = STRDUP(SAR_RUNWAY_SURFACE_GROVED_S);
		    break;
		default:
		    editor_obj_data->surface_type_s = NULL;
		    break;
	    }

	    editor_obj_data->dashes = runway->dashes;
	    editor_obj_data->edge_light_spacing = runway->edge_light_spacing;

	    editor_obj_data->north_label = STRDUP(runway->north_label);
	    editor_obj_data->south_label = STRDUP(runway->south_label);

	    editor_obj_data->north_displaced_threshold = runway->north_displaced_threshold;
	    editor_obj_data->south_displaced_threshold = runway->south_displaced_threshold;

	    if(runway->flags & SAR_RUNWAY_FLAG_THRESHOLDS)
		editor_obj_data->has_thresholds_s = strdup("thresholds");
	    else
		editor_obj_data->has_thresholds_s = NULL;

	    if(runway->flags & SAR_RUNWAY_FLAG_BORDERS)
		editor_obj_data->has_borders_s = strdup("borders");
	    else
		editor_obj_data->has_borders_s = NULL;

	    if(runway->flags & SAR_RUNWAY_FLAG_TD_MARKERS)
		editor_obj_data->has_td_markers_s = strdup("td_markers");
	    else
		editor_obj_data->has_td_markers_s = NULL;

	    if(runway->flags & SAR_RUNWAY_FLAG_MIDWAY_MARKERS)
		editor_obj_data->has_midway_markers_s = strdup("midway_markers");
	    else
		editor_obj_data->has_midway_markers_s = NULL;

	    if(runway->flags & SAR_RUNWAY_FLAG_NORTH_GS)
		editor_obj_data->has_north_gs_s = strdup("north_gs");
	    else
		editor_obj_data->has_north_gs_s = NULL;

	    if(runway->flags & SAR_RUNWAY_FLAG_SOUTH_GS)
		editor_obj_data->has_south_gs_s = strdup("south_gs");
	    else
		editor_obj_data->has_south_gs_s = NULL;

	    break;

	case SAR_OBJ_TYPE_HELIPAD:
	    helipad = SAR_OBJ_GET_HELIPAD(obj_ptr);
	    if(helipad == NULL)
		break;

	    editor_obj_data->type_s = STRDUP(SAR_OBJ_TYPE_HELIPAD_S);

	    editor_obj_data->style = helipad->style;
	    switch(editor_obj_data->style)
	    {
		case SAR_HELIPAD_STYLE_GROUND_PAVED:
		    editor_obj_data->style_s = STRDUP(SAR_HELIPAD_STYLE_GROUND_PAVED_S);
		    break;
		case SAR_HELIPAD_STYLE_GROUND_BARE:
		    editor_obj_data->style_s = STRDUP(SAR_HELIPAD_STYLE_GROUND_BARE_S);
		    break;
		case SAR_HELIPAD_STYLE_BUILDING:
		    editor_obj_data->style_s = STRDUP(SAR_HELIPAD_STYLE_BUILDING_S);
		    break;
		case SAR_HELIPAD_STYLE_VEHICLE:
		    editor_obj_data->style_s = STRDUP(SAR_HELIPAD_STYLE_VEHICLE_S);
		    break;
		default:
		    editor_obj_data->style_s = STRDUP(SAR_HELIPAD_STYLE_DEFAULT_S);
		    break;
	    }

	    editor_obj_data->length = helipad->length;
	    editor_obj_data->width = helipad->width;
	    editor_obj_data->recession = (float)SFMMetersToFeet((double)helipad->recession);
	    editor_obj_data->label = STRDUP(helipad->label);

	    if(helipad->flags & SAR_HELIPAD_FLAG_EDGE_LIGHTING)
		editor_obj_data->edge_lighting_c = 'y';
	    else
		editor_obj_data->edge_lighting_c = 'n';

	    if(helipad->flags & SAR_HELIPAD_FLAG_FUEL)
		editor_obj_data->has_fuel_c = 'y';
	    else
		editor_obj_data->has_fuel_c = 'n';

	    if(helipad->flags & SAR_HELIPAD_FLAG_REPAIR)
		editor_obj_data->has_repair_c = 'y';
	    else
		editor_obj_data->has_repair_c = 'n';

	    if(helipad->flags & SAR_HELIPAD_FLAG_DROPOFF)
		editor_obj_data->has_drop_off_c = 'y';
	    else
		editor_obj_data->has_drop_off_c = 'n';

	    if(helipad->flags & SAR_HELIPAD_FLAG_RESTART_POINT)
		editor_obj_data->restarting_point_c = 'y';
	    else
		editor_obj_data->restarting_point_c = 'n';

	    if(helipad->ref_object >= 0 )
	    {
		sar_object_struct *ref_obj_ptr;

		/* Get reference object pointer */
		ref_obj_ptr = ((helipad->ref_object < 0) ?
			    NULL : (*&core_ptr->object)[helipad->ref_object]
			);

		editor_obj_data->ref_obj_num = helipad->ref_object;
		editor_obj_data->ref_obj_name = STRDUP(ref_obj_ptr->name);

		/* Copy helipad offset location to editor_obj_data */
		memcpy(&editor_obj_data->offset_pos,
			    &helipad->ref_offset,
			    sizeof(sar_position_struct)
		       );
		/* Convert height unit */
		editor_obj_data->offset_pos.z =
		    (float)SFMMetersToFeet((double)editor_obj_data->offset_pos.z);

		/* Copy helipad offset attitude to editor_obj_data */
		memcpy(&editor_obj_data->offset_dir,
			    &helipad->ref_dir,
			    sizeof(sar_direction_struct)
		       );
		/* Convert radians to degrees */
		editor_obj_data->offset_dir.heading =
		    SFMRadiansToDegrees((double)editor_obj_data->offset_dir.heading);
		editor_obj_data->offset_dir.pitch =
		    SFMRadiansToDegrees((double)editor_obj_data->offset_dir.pitch);
		editor_obj_data->offset_dir.bank =
		    SFMRadiansToDegrees((double)editor_obj_data->offset_dir.bank);

		/* Copy reference object location to editor_obj_data */
		memcpy(&editor_obj_data->ref_obj_pos,
			    &ref_obj_ptr->pos,
			    sizeof(sar_position_struct)
		       );
		/* Convert height unit */
		editor_obj_data->ref_obj_pos.z =
		    (float)SFMMetersToFeet((double)editor_obj_data->ref_obj_pos.z);
		/* Copy reference object attitude to editor_obj_data */
		memcpy(&editor_obj_data->ref_obj_dir,
			    &ref_obj_ptr->dir,
			    sizeof(sar_direction_struct)
		       );
		/* Convert radians to degrees */
		editor_obj_data->ref_obj_dir.heading =
		    SFMRadiansToDegrees((double)editor_obj_data->ref_obj_dir.heading);
		editor_obj_data->ref_obj_dir.pitch =
		    SFMRadiansToDegrees((double)editor_obj_data->ref_obj_dir.pitch);
		editor_obj_data->ref_obj_dir.bank =
		    SFMRadiansToDegrees((double)editor_obj_data->ref_obj_dir.bank);
	    }

	    break;

	case SAR_OBJ_TYPE_HUMAN:
	    human = SAR_OBJ_GET_HUMAN(obj_ptr);
		if(human == NULL)
		    break;

		editor_obj_data->type_s = STRDUP(SAR_OBJ_TYPE_HUMAN_S);

		/* Get human preset name */
		editor_obj_data->type_name =
		    ScnEditGetHumanPresetNameByHumanPtr(core_ptr, human);

		//editor_obj_data->flags = human->flags;

		if(human->flags & SAR_HUMAN_FLAG_NEED_RESCUE)
		    editor_obj_data->need_rescue_s = strdup("need_rescue");
		else
		    editor_obj_data->need_rescue_s = NULL;

		if(human->flags & SAR_HUMAN_FLAG_SIT_UP)
		    editor_obj_data->sit_up_s = strdup("sit_up");
		else
		    editor_obj_data->sit_up_s = NULL;

		if(human->flags & SAR_HUMAN_FLAG_SIT_DOWN)
		    editor_obj_data->sit_down_s = strdup("sit_down");
		else
		    editor_obj_data->sit_down_s = NULL;

		if(human->flags & SAR_HUMAN_FLAG_SIT)
		    editor_obj_data->sitting_s = strdup("sitting");
		else
		    editor_obj_data->sitting_s = NULL;

		if(human->flags & SAR_HUMAN_FLAG_LYING)
		    editor_obj_data->lying_s = strdup("lying");
		else
		    editor_obj_data->lying_s = NULL;

		if(human->flags & SAR_HUMAN_FLAG_ALERT)
		    editor_obj_data->alert_s = strdup("alert");
		else
		    editor_obj_data->alert_s = NULL;

		if(human->flags & SAR_HUMAN_FLAG_AWARE)
		    editor_obj_data->aware_s = strdup("aware");
		else
		    editor_obj_data->aware_s = NULL;

		if(human->flags & SAR_HUMAN_FLAG_IN_WATER)
		    editor_obj_data->in_water_s = strdup("in_water");
		else
		    editor_obj_data->in_water_s = NULL;

		if(human->flags & SAR_HUMAN_FLAG_ON_STRETCHER)
		    editor_obj_data->on_stretcher_s = strdup("on_stretcher");
		else
		    editor_obj_data->on_stretcher_s = NULL;

		editor_obj_data->assistants = human->assisting_humans;

		/* Copy assistant(s) number and preset name(s) only if
		 * main human has assistant(s) and is not the
		 * "victim_streatcher_assisted" human.
		 */
		if(editor_obj_data->assistants > 0 &&
		    strcmp(editor_obj_data->type_name, "victim_streatcher_assisted") &&
		    strcmp(editor_obj_data->type_name, "victim_stretcher_assisted")
		)
		{
		    editor_obj_data->assisted_s = strdup("assisted");

		    i = 0;
		    while(i < editor_obj_data->assistants)
		    {
			editor_obj_data->assist_type_name[i] =
			    STRDUP(human->assisting_human_preset_name[i]);

			i++;
		    }

		    /* Set non-used pointers to NULL */
		    while(i < SAR_ASSISTING_HUMANS_MAX)
			editor_obj_data->assist_type_name[i++] = NULL;
		}

		/* Human displacement */

		/* Get reference/intercepting object number */
		editor_obj_data->ref_obj_num = human->intercepting_object;

		/* Is reference object the player object? */
		if(editor_obj_data->ref_obj_num == -2)
		{
		    editor_obj_data->ref_obj_name = strdup("player");

		    if(human->flags & SAR_HUMAN_FLAG_RUN_TOWARDS)
			editor_obj_data->human_displacement_dir_s = strdup("run_towards");
		    else if(human->flags & SAR_HUMAN_FLAG_RUN_AWAY)
			editor_obj_data->human_displacement_dir_s = strdup("run_away");
		    else
			editor_obj_data->human_displacement_dir_s = strdup("?");

		    /* Don't copy reference object position to editor_obj_data */
		}
		/* Is reference object a scene object? */
		else if(editor_obj_data->ref_obj_num >= 0)
		{
		    obj_num = editor_obj_data->ref_obj_num;
		    obj_ptr = ((obj_num < 0) ? NULL : (*&core_ptr->object)[obj_num]);
		    if(obj_ptr != NULL)
		    {
			/* Get reference object name */
			editor_obj_data->ref_obj_name = STRDUP(obj_ptr->name);

			if(human->flags & SAR_HUMAN_FLAG_RUN_TOWARDS)
			    editor_obj_data->human_displacement_dir_s = strdup("run_towards");
			else if(human->flags & SAR_HUMAN_FLAG_RUN_AWAY)
			    editor_obj_data->human_displacement_dir_s = strdup("run_away");
			else
			    editor_obj_data->human_displacement_dir_s = strdup("?");

			/* Copy reference object location to editor_obj_data */
			memcpy(&editor_obj_data->ref_obj_pos,
			    &obj_ptr->pos,
			    sizeof(sar_position_struct)
			);
			/* Convert height unit */
			editor_obj_data->ref_obj_pos.z =
			(float)SFMMetersToFeet((double)editor_obj_data->ref_obj_pos.z);
			/* Copy reference object attitude to editor_obj_data */
			memcpy(&editor_obj_data->ref_obj_dir,
				    &obj_ptr->dir,
				    sizeof(sar_direction_struct)
			    );
			/* Convert radians to degrees */
			editor_obj_data->ref_obj_dir.heading =
			    SFMRadiansToDegrees((double)editor_obj_data->ref_obj_dir.heading);
			editor_obj_data->ref_obj_dir.pitch =
			    SFMRadiansToDegrees((double)editor_obj_data->ref_obj_dir.pitch);
			editor_obj_data->ref_obj_dir.bank =
			    SFMRadiansToDegrees((double)editor_obj_data->ref_obj_dir.bank);
		    }
		    else
		    {
			/* Force "no reference object" */
			editor_obj_data->ref_obj_num = -1;
		    }
		}
		/* No reference object (should be editor_obj_data->ref_obj_num == -1) */
		else
		{
		    ;
		}

	    break;

	case SAR_OBJ_TYPE_SMOKE:
	    smoke = SAR_OBJ_GET_SMOKE(obj_ptr);
	    if(smoke == NULL)
		break;

	    editor_obj_data->type_s = STRDUP(SAR_OBJ_TYPE_SMOKE_S);

	    /* Copy smoke offset position to editor_obj_data */
	    memcpy(&editor_obj_data->offset_pos, &smoke->respawn_offset, sizeof(sar_position_struct));

	    editor_obj_data->radius_start = smoke->radius_start;
	    editor_obj_data->radius_max = smoke->radius_max;
	    editor_obj_data->radius_rate = smoke->radius_rate;
	    editor_obj_data->hide_at_max = smoke->hide_at_max;
	    editor_obj_data->respawn_int = smoke->respawn_int;
	    editor_obj_data->total_units = smoke->total_units;

	    /* Get smoke texture name */
	    tex_ref_name = ScnEditGetTextureNamePtrByRef(
						core_ptr->scene,
						smoke->tex_num
					    );

	    /* Retrieve smoke color code */
	    if(!strcmp(tex_ref_name, SAR_STD_TEXNAME_SMOKE_LIGHT))
		editor_obj_data->color_code = 0;
	    else if(!strcmp(tex_ref_name, SAR_STD_TEXNAME_SMOKE_MEDIUM))
		editor_obj_data->color_code = 1;
	    else if(!strcmp(tex_ref_name, SAR_STD_TEXNAME_SMOKE_DARK))
		editor_obj_data->color_code = 2;
	    else if(!strcmp(tex_ref_name, SAR_STD_TEXNAME_SMOKE_ORANGE))
		editor_obj_data->color_code = 3;
	    /* Should never happen */
	    else
		;

	    break;

	case SAR_OBJ_TYPE_FIRE:
	    fire = SAR_OBJ_GET_FIRE(obj_ptr);
	    if(fire == NULL)
		break;

	    editor_obj_data->type_s = STRDUP(SAR_OBJ_TYPE_FIRE_S);
	    editor_obj_data->radius = fire->radius;
	    editor_obj_data->height = (float)SFMMetersToFeet((double)fire->height);

	    break;

	case SAR_OBJ_TYPE_EXPLOSION:
	    editor_obj_data->type_s = STRDUP(SAR_OBJ_TYPE_EXPLOSION_S);
	    break;

	case SAR_OBJ_TYPE_CHEMICAL_SPRAY:
	    editor_obj_data->type_s = STRDUP(SAR_OBJ_TYPE_CHEMICAL_SPRAY_S);
	    break;
	case SAR_OBJ_TYPE_FUELTANK:
	    editor_obj_data->type_s = STRDUP(SAR_OBJ_TYPE_FUELTANK_S);
	    break;

	case SAR_OBJ_TYPE_PREMODELED:
	    premodeled = SAR_OBJ_GET_PREMODELED(obj_ptr);
	    if(premodeled == NULL)
		break;

	    editor_obj_data->type_s = STRDUP(SAR_OBJ_TYPE_PREMODELED_S);

	    /* Premodeled type */
	    editor_obj_data->pm_type = premodeled->type;
	    if(premodeled->type == SAR_OBJ_PREMODELED_POWER_TRANSMISSION_TOWER)
		editor_obj_data->pm_type_s = STRDUP(SAR_PREMODELED_POWER_TRANSMISSION_TOWER_S);
	    else if(premodeled->type == SAR_OBJ_PREMODELED_TOWER)
		editor_obj_data->pm_type_s = STRDUP(SAR_PREMODELED_TOWER_S);
	    else if(premodeled->type == SAR_OBJ_PREMODELED_RADIO_TOWER)
		editor_obj_data->pm_type_s = STRDUP(SAR_PREMODELED_RADIO_TOWER_S);
	    else if(premodeled->type == SAR_OBJ_PREMODELED_CONTROL_TOWER)
		editor_obj_data->pm_type_s = STRDUP(SAR_PREMODELED_CONTROL_TOWER_S);
	    else if(premodeled->type == SAR_OBJ_PREMODELED_BUILDING)
		editor_obj_data->pm_type_s = STRDUP(SAR_PREMODELED_BUILDING_S);
	    else
		editor_obj_data->pm_type_s = STRDUP(SAR_PREMODELED_UNKNOWN_S);

	    switch(editor_obj_data->pm_type)
	    {
		case SAR_OBJ_PREMODELED_POWER_TRANSMISSION_TOWER:
		case SAR_OBJ_PREMODELED_TOWER:
		case SAR_OBJ_PREMODELED_RADIO_TOWER:
		    editor_obj_data->range = obj_ptr->range;
		    editor_obj_data->height =
			    (float)SFMMetersToFeet((double)premodeled->height);
		    editor_obj_data->hazard_lights = obj_ptr->total_lights;
		    break;

		case SAR_OBJ_PREMODELED_CONTROL_TOWER:
		    editor_obj_data->range = obj_ptr->range;
		    editor_obj_data->length = premodeled->length;
		    editor_obj_data->width = premodeled->width;
		    editor_obj_data->height =
			    (float)SFMMetersToFeet((double)premodeled->height);

		    /* Get control tower walls texture name */
		    tex_ref_name = ScnEditGetTextureNamePtrByRef(
						core_ptr->scene,
						premodeled->tex_num[0]
					    );
		    editor_obj_data->walls_texture_s = STRDUP(tex_ref_name);

		    /* Get control tower roof texture name */
		    tex_ref_name = ScnEditGetTextureNamePtrByRef(
						core_ptr->scene,
						premodeled->tex_num[1]
					    );
		    editor_obj_data->roof_texture_s = STRDUP(tex_ref_name);
		    break;

		case SAR_OBJ_PREMODELED_BUILDING:
		    editor_obj_data->range = obj_ptr->range;
		    editor_obj_data->length = premodeled->length;
		    editor_obj_data->width = premodeled->width;
		    editor_obj_data->height =
			    (float)SFMMetersToFeet((double)premodeled->height);

		    /* Get building walls texture name */
		    tex_ref_name = ScnEditGetTextureNamePtrByRef(
						core_ptr->scene,
						premodeled->tex_num[0]
					    );
		    editor_obj_data->walls_texture_s = STRDUP(tex_ref_name);

		    /* Get building walls night texture name */
		    tex_ref_name = ScnEditGetTextureNamePtrByRef(
						core_ptr->scene,
						premodeled->tex_num[1]
					    );
		    editor_obj_data->walls_texture_night_s = STRDUP(tex_ref_name);

		    /* Get building roof texture name */
		    tex_ref_name = ScnEditGetTextureNamePtrByRef(
						core_ptr->scene,
						premodeled->tex_num[2]
					    );
		    editor_obj_data->roof_texture_s = STRDUP(tex_ref_name);
		    break;

		case SAR_OBJ_PREMODELED_HANGAR:
		default:

		    break;
	    }

	    break;

	default:
	    break;
    }

    return obj_num;
}



/*
 * Update position and direction in an editor_object_data_struct structure.
 */
/*
static editor_object_data_struct *EditorObjectDataStructUpdatePosAndDirOnly(
    sar_core_struct *core_ptr,
    editor_object_data_struct *editor_obj_data,
    int obj_num
)
{
    sar_scenery_editor_struct *scn_ed;
    sar_object_struct *obj_ptr;

    scn_ed = core_ptr->in_game_editor;
    if(scn_ed == NULL)
	return NULL;

    if(editor_obj_data == NULL)
	return NULL;

    obj_ptr = core_ptr->object[obj_num];
    if(obj_ptr == NULL)
	return NULL;

    // Set object location //
    memcpy(&editor_obj_data->pos, &obj_ptr->pos, sizeof(sar_position_struct));
    // Convert height unit //
    editor_obj_data->pos.z =
		(float)SFMMetersToFeet((double)editor_obj_data->pos.z);

    // Set object attitude //
    memcpy(&editor_obj_data->dir, &obj_ptr->dir, sizeof(sar_direction_struct));
    // Convert radians to degrees //
    editor_obj_data->dir.heading =
		SFMRadiansToDegrees((double)editor_obj_data->dir.heading);
    editor_obj_data->dir.pitch =
		SFMRadiansToDegrees((double)editor_obj_data->dir.pitch);
    editor_obj_data->dir.bank =
		SFMRadiansToDegrees((double)editor_obj_data->dir.bank);

    return editor_obj_data;
}
*/


/*
 * FIXME ------------ Really usefull? Should return a string with modified data ??? ------------
 * Compare 2 editor_object_data structures.
 *
 * Return:
 *  0 is structures are equal,
 * -1 if they are not equal,
 *  1 if obj_data_1 structure pointer is NULL, and
 *  2 if obj_data_2 structure pointer is NULL.
 */
/*
static int EditorObjectDataStructCompare(
    editor_object_data_struct *obj_data_1,
    editor_object_data_struct *obj_data_2
)
{
    int i;
    sar_obj_type type;

    if(obj_data_1 == NULL)
	return 1;
    if(obj_data_2 == NULL)
	return 2;

    if(obj_data_1->type != obj_data_2->type)
	return -1;
    if(strcmp(obj_data_1->type_s, obj_data_2->type_s))
	return -1;
    if(strcmp(obj_data_1->name, obj_data_2->name))
	return -1;
    if(strcmp(obj_data_1->obj_name, obj_data_2->obj_name))
	return -1;
    if(strcmp(obj_data_1->object_map_description, obj_data_2->object_map_description))
	return -1;
    if(obj_data_1->pos.x != obj_data_2->pos.x ||
	obj_data_1->pos.y != obj_data_2->pos.y ||
	obj_data_1->pos.z != obj_data_2->pos.z
    )
	return -1;
    if(obj_data_1->dir.heading != obj_data_2->dir.heading ||
	obj_data_1->dir.pitch != obj_data_2->dir.pitch ||
	obj_data_1->dir.bank != obj_data_2->dir.bank
    )
	return -1;
    if(obj_data_1->range != obj_data_2->range)
	return -1;
    if(obj_data_1->length != obj_data_2->length)
	return -1;
    if(obj_data_1->width != obj_data_2->width)
	return -1;
    if(obj_data_1->height != obj_data_2->height)
	return -1;
    if(obj_data_1->ref_obj_num != obj_data_2->ref_obj_num)
	return -1;
    if(strcmp(obj_data_1->ref_obj_name, obj_data_2->ref_obj_name))
	return -1;
    if(obj_data_1->ref_obj_pos.x != obj_data_2->ref_obj_pos.x ||
	obj_data_1->ref_obj_pos.y != obj_data_2->ref_obj_pos.y ||
	obj_data_1->ref_obj_pos.z != obj_data_2->ref_obj_pos.z
    )
	return -1;
    if(obj_data_1->ref_obj_dir.heading != obj_data_2->ref_obj_dir.heading ||
	obj_data_1->ref_obj_dir.pitch != obj_data_2->ref_obj_dir.pitch ||
	obj_data_1->ref_obj_dir.bank != obj_data_2->ref_obj_dir.bank
    )
	return -1;
    if(obj_data_1->offset_pos.x != obj_data_2->offset_pos.x ||
	obj_data_1->offset_pos.y != obj_data_2->offset_pos.y ||
	obj_data_1->offset_pos.z != obj_data_2->offset_pos.z
    )
	return -1;

    type = obj_data_1->type;
    switch(type)
    {
	case SAR_OBJ_TYPE_FIRE:
	    if(obj_data_1->radius != obj_data_2->radius)
		return -1;
	    break;

	case SAR_OBJ_TYPE_HELIPAD:
	    if(obj_data_1->style != obj_data_2->style)
		return -1;
	    if(strcmp(obj_data_1->style_s, obj_data_2->style_s))
		return -1;
	    if(obj_data_1->recession != obj_data_2->recession)
		return -1;
	    if(strcmp(obj_data_1->label, obj_data_2->label))
		return -1;
	    if(obj_data_1->edge_lighting_c != obj_data_2->edge_lighting_c)
		return -1;
	    if(obj_data_1->has_fuel_c != obj_data_2->has_fuel_c)
		return -1;
	    if(obj_data_1->has_repair_c != obj_data_2->has_repair_c)
		return -1;
	    if(obj_data_1->has_drop_off_c != obj_data_2->has_drop_off_c)
		return -1;
	    if(obj_data_1->restarting_point_c != obj_data_2->restarting_point_c)
		return -1;
	    if(obj_data_1->offset_dir.heading != obj_data_2->offset_dir.heading ||
		obj_data_1->offset_dir.pitch != obj_data_2->offset_dir.pitch ||
		obj_data_1->offset_dir.bank != obj_data_2->offset_dir.bank
	    )
		return -1;
	    break;

	case SAR_OBJ_TYPE_HUMAN:
	    if(strcmp(obj_data_1->type_name, obj_data_2->type_name))
		return -1;
	    if(strcmp(obj_data_1->need_rescue_s, obj_data_2->need_rescue_s))
		return -1;
	    if(strcmp(obj_data_1->sit_up_s, obj_data_2->sit_up_s))
		return -1;
	    if(strcmp(obj_data_1->sit_down_s, obj_data_2->sit_down_s))
		return -1;
	    if(strcmp(obj_data_1->sitting_s, obj_data_2->sitting_s))
		return -1;
	    if(strcmp(obj_data_1->lying_s, obj_data_2->lying_s))
		return -1;
	    if(strcmp(obj_data_1->alert_s, obj_data_2->alert_s))
		return -1;
	    if(strcmp(obj_data_1->aware_s, obj_data_2->aware_s))
		return -1;
	    if(strcmp(obj_data_1->in_water_s, obj_data_2->in_water_s))
		return -1;
	    if(strcmp(obj_data_1->on_stretcher_s, obj_data_2->on_stretcher_s))
		return -1;
	    if(strcmp(obj_data_1->assisted_s, obj_data_2->assisted_s))
		return -1;
	    if(obj_data_1->assistants != obj_data_2->assistants)
		return -1;
	    for(i = 0; i < SAR_ASSISTING_HUMANS_MAX; i++)
		if(strcmp(obj_data_1->assist_type_name[i], obj_data_2->assist_type_name[i]))
		    return -1;
	    if(strcmp(obj_data_1->human_displacement_dir_s, obj_data_2->human_displacement_dir_s))
		return -1;
	    break;

	case SAR_OBJ_TYPE_STATIC:
	case SAR_OBJ_TYPE_AUTOMOBILE:
	case SAR_OBJ_TYPE_WATERCRAFT:
	case SAR_OBJ_TYPE_AIRCRAFT:
	case SAR_OBJ_TYPE_GROUND:
	    if(strcmp(obj_data_1->file_name, obj_data_2->file_name))
		return -1;
	    break;

	case SAR_OBJ_TYPE_PREMODELED:
	    if(obj_data_1->pm_type != obj_data_2->pm_type)
		return -1;
	    if(strcmp(obj_data_1->pm_type_s, obj_data_2->pm_type_s))
		return -1;
	    if(obj_data_1->hazard_lights != obj_data_2->hazard_lights)
		return -1;
	    if(strcmp(obj_data_1->walls_texture_s, obj_data_2->walls_texture_s))
		return -1;
	    if(strcmp(obj_data_1->walls_texture_night_s, obj_data_2->walls_texture_night_s))
		return -1;
	    if(strcmp(obj_data_1->roof_texture_s, obj_data_2->roof_texture_s))
		return -1;
	    break;

	case SAR_OBJ_TYPE_RUNWAY:
	    if(obj_data_1->surface_type != obj_data_2->surface_type)
		return -1;
	    if(strcmp(obj_data_1->surface_type_s, obj_data_2->surface_type_s))
		return -1;
	    if(obj_data_1->dashes != obj_data_2->dashes)
		return -1;
	    if(obj_data_1->edge_light_spacing != obj_data_2->edge_light_spacing)
		return -1;
	    if(strcmp(obj_data_1->north_label, obj_data_2->north_label))
		return -1;
	    if(strcmp(obj_data_1->south_label, obj_data_2->south_label))
		return -1;
	    if(obj_data_1->north_displaced_threshold != obj_data_2->north_displaced_threshold)
		return -1;
	    if(obj_data_1->south_displaced_threshold != obj_data_2->south_displaced_threshold)
		return -1;
	    if(strcmp(obj_data_1->has_thresholds_s, obj_data_2->has_thresholds_s))
		return -1;
	    if(strcmp(obj_data_1->has_borders_s, obj_data_2->has_borders_s))
		return -1;
	    if(strcmp(obj_data_1->has_td_markers_s, obj_data_2->has_td_markers_s))
		return -1;
	    if(strcmp(obj_data_1->has_midway_markers_s, obj_data_2->has_midway_markers_s))
		return -1;
	    if(strcmp(obj_data_1->has_north_gs_s, obj_data_2->has_north_gs_s))
		return -1;
	    if(strcmp(obj_data_1->has_south_gs_s, obj_data_2->has_south_gs_s))
		return -1;
	    break;

	case SAR_OBJ_TYPE_SMOKE:
	    if(obj_data_1->radius_start != obj_data_2->radius_start)
		return -1;
	    if(obj_data_1->radius_max != obj_data_2->radius_max)
		return -1;
	    if(obj_data_1->radius_rate != obj_data_2->radius_rate)
		return -1;
	    if(obj_data_1->hide_at_max != obj_data_2->hide_at_max)
		return -1;
	    if(obj_data_1->respawn_int != obj_data_2->respawn_int)
		return -1;
	    if(obj_data_1->total_units != obj_data_2->total_units)
		return -1;
	    if(obj_data_1->color_code != obj_data_2->color_code)
		return -1;
	    break;

	case SAR_OBJ_TYPE_GARBAGE:
	case SAR_OBJ_TYPE_EXPLOSION:
	case SAR_OBJ_TYPE_CHEMICAL_SPRAY:
	case SAR_OBJ_TYPE_FUELTANK:
	    break;
    }

    return 0;
}
*/


/*
 * Returns a string containing object creation data.
 * Returns NULL on error.
 *
 * String must be freed by calling function.
 */
static char* EditorObjectDataDoParametersLine(editor_object_data_struct *editor_obj_data)
{
#define MAXVALUESTRCHARNUM 64

#define S_LENGTH 1023
#define TITLE_LENGTH 40
#define REMAINING(s) (MAX(0, S_LENGTH - strlen(s)))

    sar_position_struct *pos = NULL;
    sar_direction_struct *dir = NULL;
    char value_str[MAXVALUESTRCHARNUM + 1];
    int i;

    char *s = (char *)malloc((S_LENGTH + 1) * sizeof(char));
    if(s == NULL)
    {
	fprintf(stderr,
		"%s:%d: Memory allocation error.\n",
		__FILE__,
		__LINE__
	    );
	return s;
    }

    /* Init strings */
    s[0] = '\0';
    value_str[0] = '\0';

    if(editor_obj_data == NULL)
	return s;

    switch(editor_obj_data->type)
    {
	case SAR_OBJ_TYPE_GARBAGE:
	    s = NULL;
	    break;

	case SAR_OBJ_TYPE_STATIC:
	case SAR_OBJ_TYPE_AUTOMOBILE:
	case SAR_OBJ_TYPE_WATERCRAFT:
	case SAR_OBJ_TYPE_AIRCRAFT:
	    strncat(s, editor_obj_data->file_name, REMAINING(s));
	    break;

	case SAR_OBJ_TYPE_GROUND:
	    break;

	case SAR_OBJ_TYPE_RUNWAY:
	    sprintf(value_str, "%.3f", editor_obj_data->range);
	    strncat(s, value_str, REMAINING(s));
	    sprintf(value_str, " %.3f", editor_obj_data->length);
	    strncat(s, value_str, REMAINING(s));
	    sprintf(value_str, " %f", editor_obj_data->width);
	    strncat(s, value_str, REMAINING(s));
	    sprintf(value_str, " %d", editor_obj_data->surface_type);
	    strncat(s, value_str, REMAINING(s));
	    sprintf(value_str, " %d", editor_obj_data->dashes);
	    strncat(s, value_str, REMAINING(s));
	    sprintf(value_str, " %.3f", editor_obj_data->edge_light_spacing);
	    strncat(s, value_str, REMAINING(s));
	    snprintf(value_str, MAXVALUESTRCHARNUM, " %s",
(editor_obj_data->north_label != NULL) ? editor_obj_data->north_label : "_");
	    strncat(s, value_str, REMAINING(s));
	    snprintf(value_str, MAXVALUESTRCHARNUM, " %s",
(editor_obj_data->south_label != NULL) ? editor_obj_data->south_label : "_");
	    strncat(s, value_str, REMAINING(s));
	    sprintf(value_str, " %.3f",
		    editor_obj_data->north_displaced_threshold);
	    strncat(s, value_str, REMAINING(s));
	    sprintf(value_str, " %.3f",
		    editor_obj_data->south_displaced_threshold);
	    strncat(s, value_str, REMAINING(s));

	    /* Optional flags */

	    if(editor_obj_data->has_thresholds_s != NULL)
	    {
		strncat(s, " ", REMAINING(s));
		strncat(s, editor_obj_data->has_thresholds_s, REMAINING(s));
	    }
	    if(editor_obj_data->has_borders_s != NULL)
	    {
		strncat(s, " ", REMAINING(s));
		strncat(s, editor_obj_data->has_borders_s, REMAINING(s));
	    }
	    if(editor_obj_data->has_td_markers_s != NULL)
	    {
		strncat(s, " ", REMAINING(s));
		strncat(s, editor_obj_data->has_td_markers_s, REMAINING(s));
	    }
	    if(editor_obj_data->has_midway_markers_s != NULL)
	    {
		strncat(s, " ", REMAINING(s));
		strncat(s, editor_obj_data->has_midway_markers_s, REMAINING(s));
	    }
	    if(editor_obj_data->has_north_gs_s != NULL)
	    {
		strncat(s, " ", REMAINING(s));
		strncat(s, editor_obj_data->has_north_gs_s, REMAINING(s));
	    }
	    if(editor_obj_data->has_south_gs_s != NULL)
	    {
		strncat(s, " ", REMAINING(s));
		strncat(s, editor_obj_data->has_south_gs_s, REMAINING(s));
	    }
	    break;

	case SAR_OBJ_TYPE_HELIPAD:
	    strncat(s, editor_obj_data->style_s, REMAINING(s));
	    sprintf(value_str, " %.3f", editor_obj_data->length);
	    strncat(s, value_str, REMAINING(s));
	    sprintf(value_str, " %.3f", editor_obj_data->width);
	    strncat(s, value_str, REMAINING(s));
	    sprintf(value_str, " %.3f", editor_obj_data->recession);
	    strncat(s, value_str, REMAINING(s));
	    snprintf(value_str, MAXVALUESTRCHARNUM, " %s", editor_obj_data->label);
	    strncat(s, value_str, REMAINING(s));

	    sprintf(value_str, " %c", editor_obj_data->edge_lighting_c);
	    strncat(s, value_str, REMAINING(s));
	    sprintf(value_str, " %c", editor_obj_data->has_fuel_c);
	    strncat(s, value_str, REMAINING(s));
	    sprintf(value_str, " %c", editor_obj_data->has_repair_c);
	    strncat(s, value_str, REMAINING(s));
	    sprintf(value_str, " %c", editor_obj_data->has_drop_off_c);
	    strncat(s, value_str, REMAINING(s));
	    sprintf(value_str, " %c", editor_obj_data->restarting_point_c);
	    strncat(s, value_str, REMAINING(s));

	    /* Has helipad a reference object? */
	    if(editor_obj_data->ref_obj_name != NULL)
	    {
		sprintf(value_str, " %s", editor_obj_data->ref_obj_name);
		strncat(s, value_str, REMAINING(s));

		pos = &(editor_obj_data->offset_pos);
		dir = &(editor_obj_data->offset_dir);

		sprintf(value_str, " %.3f", pos->x);
		strncat(s, value_str, REMAINING(s));
		sprintf(value_str, " %.3f", pos->y);
		strncat(s, value_str, REMAINING(s));
		sprintf(value_str, " %.3f", pos->z);
		strncat(s, value_str, REMAINING(s));
		sprintf(value_str, " %.3f", dir->heading);
		strncat(s, value_str, REMAINING(s));
		sprintf(value_str, " %.3f", dir->pitch);
		strncat(s, value_str, REMAINING(s));
		sprintf(value_str, " %.3f", dir->bank);
		strncat(s, value_str, REMAINING(s));
	    }
	    break;

	case SAR_OBJ_TYPE_HUMAN:
	    strncat(s, editor_obj_data->type_name, REMAINING(s));

	    if(editor_obj_data->need_rescue_s != NULL)
	    {
		strncat(s, " ", REMAINING(s));
		strncat(s, editor_obj_data->need_rescue_s, REMAINING(s));
	    }
	    if(editor_obj_data->sit_up_s != NULL)
	    {
		strncat(s, " ", REMAINING(s));
		strncat(s, editor_obj_data->sit_up_s, REMAINING(s));
	    }
	    if(editor_obj_data->sit_down_s != NULL)
	    {
		strncat(s, " ", REMAINING(s));
		strncat(s, editor_obj_data->sit_down_s, REMAINING(s));
	    }
	    if(editor_obj_data->sitting_s != NULL)
	    {
		strncat(s, " ", REMAINING(s));
		strncat(s, editor_obj_data->sitting_s, REMAINING(s));
	    }
	    if(editor_obj_data->lying_s != NULL)
	    {
		strncat(s, " ", REMAINING(s));
		strncat(s, editor_obj_data->lying_s, REMAINING(s));
	    }
	    if(editor_obj_data->alert_s != NULL)
	    {
		strncat(s, " ", REMAINING(s));
		strncat(s, editor_obj_data->alert_s, REMAINING(s));
	    }
	    if(editor_obj_data->aware_s != NULL)
	    {
		strncat(s, " ", REMAINING(s));
		strncat(s, editor_obj_data->aware_s, REMAINING(s));
	    }
	    if(editor_obj_data->in_water_s != NULL)
	    {
		strncat(s, " ", REMAINING(s));
		strncat(s, editor_obj_data->in_water_s, REMAINING(s));
	    }
	    if(editor_obj_data->on_stretcher_s != NULL)
	    {
		strncat(s, " ", REMAINING(s));
		strncat(s, editor_obj_data->on_stretcher_s, REMAINING(s));
	    }

	    /* Print assistant(s) number and preset name(s) only if
	     * main human has assistant(s) and is not the
	     * "victim_streatcher_assisted" human.
	     */
	    if(editor_obj_data->assistants > 0 &&
		strcmp(editor_obj_data->type_name, "victim_streatcher_assisted") &&
		strcmp(editor_obj_data->type_name,  "victim_stretcher_assisted")
	    )
	    {
		sprintf(value_str, " %s %d",
			    editor_obj_data->assisted_s,
			    editor_obj_data->assistants
			);
		strncat(s, value_str, REMAINING(s));

		for(i = 0; i < editor_obj_data->assistants; i++)
		{
		    sprintf(value_str, " %s", editor_obj_data->assist_type_name[i]);
		    strncat(s, value_str, REMAINING(s));
		}
	    }
	    break;

	case SAR_OBJ_TYPE_SMOKE:
	    pos = &(editor_obj_data->offset_pos);

	    sprintf(value_str, "%.3f", pos->x);
	    strncat(s, value_str, REMAINING(s));
	    sprintf(value_str, " %.3f", pos->y);
	    strncat(s, value_str, REMAINING(s));
	    sprintf(value_str, " %.3f", pos->z);
	    strncat(s, value_str, REMAINING(s));
	    sprintf(value_str, " %.3f", editor_obj_data->radius_start);
	    strncat(s, value_str, REMAINING(s));
	    sprintf(value_str, " %.3f", editor_obj_data->radius_max);
	    strncat(s, value_str, REMAINING(s));
	    sprintf(value_str, " %.3f", editor_obj_data->radius_rate);
	    strncat(s, value_str, REMAINING(s));
	    sprintf(value_str, " %.3f", editor_obj_data->hide_at_max);
	    strncat(s, value_str, REMAINING(s));
	    sprintf(value_str, " %ld", editor_obj_data->respawn_int);
	    strncat(s, value_str, REMAINING(s));
	    sprintf(value_str, " %d", editor_obj_data->total_units);
	    strncat(s, value_str, REMAINING(s));
	    sprintf(value_str, " %d", editor_obj_data->color_code);
	    strncat(s, value_str, REMAINING(s));
	    break;

	case SAR_OBJ_TYPE_FIRE:
	    sprintf(value_str, "%.3f", editor_obj_data->radius);
	    strncat(s, value_str, REMAINING(s));
	    sprintf(value_str, " %.3f", editor_obj_data->height);
	    strncat(s, value_str, REMAINING(s));
	    break;

	case SAR_OBJ_TYPE_EXPLOSION:
	case SAR_OBJ_TYPE_CHEMICAL_SPRAY:
	case SAR_OBJ_TYPE_FUELTANK:
	    strncat(s, "ERROR: Can't get object data.", REMAINING(s));
	    break;

	case SAR_OBJ_TYPE_PREMODELED:
	    strncat(s, editor_obj_data->pm_type_s, REMAINING(s));

	    switch(editor_obj_data->pm_type)
	    {
		case SAR_OBJ_PREMODELED_POWER_TRANSMISSION_TOWER:
		case SAR_OBJ_PREMODELED_TOWER:
		case SAR_OBJ_PREMODELED_RADIO_TOWER:
		    sprintf(value_str, " %.3f", editor_obj_data->range);
		    strncat(s, value_str, REMAINING(s));
		    sprintf(value_str, " %.3f", editor_obj_data->height);
		    strncat(s, value_str, REMAINING(s));
		    sprintf(value_str, " %d", editor_obj_data->hazard_lights);
		    strncat(s, value_str, REMAINING(s));
		    break;

		case SAR_OBJ_PREMODELED_CONTROL_TOWER:
		    sprintf(value_str, " %.3f", editor_obj_data->range);
		    strncat(s, value_str, REMAINING(s));
		    sprintf(value_str, " %.3f", editor_obj_data->length);
		    strncat(s, value_str, REMAINING(s));
		    sprintf(value_str, " %.3f", editor_obj_data->width);
		    strncat(s, value_str, REMAINING(s));
		    sprintf(value_str, " %.3f", editor_obj_data->height);
		    strncat(s, value_str, REMAINING(s));
		    strncat(s, " ", REMAINING(s));
		    strncat(s, editor_obj_data->walls_texture_s, REMAINING(s));
		    strncat(s, " ", REMAINING(s));
		    strncat(s, editor_obj_data->roof_texture_s, REMAINING(s));
		    break;

		case SAR_OBJ_PREMODELED_BUILDING:
		    sprintf(value_str, " %.3f", editor_obj_data->range);
		    strncat(s, value_str, REMAINING(s));
		    sprintf(value_str, " %.3f", editor_obj_data->length);
		    strncat(s, value_str, REMAINING(s));
		    sprintf(value_str, " %.3f", editor_obj_data->width);
		    strncat(s, value_str, REMAINING(s));
		    sprintf(value_str, " %.3f", editor_obj_data->height);
		    strncat(s, value_str, REMAINING(s));
		    strncat(s, " ", REMAINING(s));
		    strncat(s, editor_obj_data->walls_texture_s, REMAINING(s));
		    strncat(s, " ", REMAINING(s));
		    strncat(s, editor_obj_data->walls_texture_night_s, REMAINING(s));
		    strncat(s, " ", REMAINING(s));
		    strncat(s, editor_obj_data->roof_texture_s, REMAINING(s));
		    break;

		default:

		    break;
	    }
	    break;

	default:
	    sprintf(value_str, "ERROR: object #%d type is unknown", editor_obj_data->type);
	    strncat(s, value_str, REMAINING(s));
	    break;
    }

    return s;
#undef MAXVALUESTRCHARNUM
}


int ScnEditShowObjectInfoWindow(sar_core_struct *core_ptr, int obj_num)
{
#define S_LENGTH 1023
#define TITLE_LENGTH 40
#define REMAINING(s) (MAX(0, S_LENGTH - strlen(s)))
    sar_scene_struct *scene = core_ptr->scene;
    gw_display_struct *display = core_ptr->display;
    sar_scenery_editor_struct *scn_ed = core_ptr->in_game_editor;
    editor_object_data_struct *editor_obj_data;
    sar_obj_type type;
    char *s = (char *)malloc((S_LENGTH + 1) * sizeof(char));
    char *s1 = (char *)malloc((S_LENGTH + 1) * sizeof(char));
    char *window_title = (char *)malloc((TITLE_LENGTH + 1) * sizeof(char));
    char *type_str = NULL;
    int i;
    float distance;

    if(scene == NULL ||
	display == NULL ||
	scn_ed == NULL ||
	obj_num < 0
    )
	return -1;

    editor_obj_data = EditorObjectDataStructNew();
    EditorObjectDataStructFill(core_ptr, editor_obj_data, obj_num);

    /* 3D distance between triedron (i.e. player) and #obj_num object */
	distance = (float)SFMHypot3(
	    scene->player_obj_ptr->pos.x - editor_obj_data->pos.x,
	    scene->player_obj_ptr->pos.y - editor_obj_data->pos.y,
	    scene->player_obj_ptr->pos.z - SFMFeetToMeters((double)editor_obj_data->pos.z)
	);

    /* Set window title */
    snprintf(window_title, TITLE_LENGTH, "     Object #%05d @%.3fm    ", obj_num, distance);

    s[0] = '\0';
    strncat(s, "-------- Object general data --------\n", REMAINING(s));

    /* Object type */
    type = editor_obj_data->type;
    type_str = STRDUP(SceneObjectGetTypeName(core_ptr, obj_num));
    snprintf(s1, S_LENGTH, "Type            : %s\n", type_str);
    strncat(s, s1, REMAINING(s));

    /* Object name (the 'name' parameter, wich can contain space characters) */
    strncat(s, "Name           : ", REMAINING(s));
    if(editor_obj_data->name != NULL)
	snprintf(s1, S_LENGTH, "%s\n", editor_obj_data->name);
    else
	snprintf(s1, S_LENGTH, "\n");
    strncat(s, s1, REMAINING(s));

    /* Object name (the 'object_name' parameter, which can be used as
     * a reference name and can't contain any space character).
     */
    strncat(s, "Object name : ", REMAINING(s));
    if(editor_obj_data->obj_name != NULL)
	snprintf(s1, S_LENGTH, "%s\n", editor_obj_data->obj_name);
    else
	snprintf(s1, S_LENGTH, "\n");
    strncat(s, s1, REMAINING(s));

    snprintf(s1, S_LENGTH,
		    "Position (m/m/ft) : %.3f %.3f %.3f\nDirection (deg)  : %.3f %.3f %.3f\n",
		    editor_obj_data->pos.x,
		    editor_obj_data->pos.y,
		    editor_obj_data->pos.z,
		    editor_obj_data->dir.heading,
		    editor_obj_data->dir.pitch,
		    editor_obj_data->dir.bank
		);
    strncat(s, s1, REMAINING(s));

    switch(type)
    {
	case SAR_OBJ_TYPE_GARBAGE:
	    break;

	case SAR_OBJ_TYPE_STATIC:
	case SAR_OBJ_TYPE_AUTOMOBILE:
	case SAR_OBJ_TYPE_WATERCRAFT:
	case SAR_OBJ_TYPE_AIRCRAFT:
	case SAR_OBJ_TYPE_GROUND:
	    snprintf(s1, S_LENGTH,
		     "\nFile name : %s\n",
		    editor_obj_data->file_name
	    );
	    strncat(s, s1, REMAINING(s));

	    break;

	case SAR_OBJ_TYPE_RUNWAY:
	    strncat(s, "\n-------- Runway specific data --------\n", REMAINING(s));

	    snprintf(s1, S_LENGTH,
		     "Visual range                     : %.3f\nLength (m)                        : %.3f\nWidth (m)                          : %.3f\nSurface type                     : %s\nDashes                            : %d\nEdge light spacing             : %.3f\nNorth label                       : %s\nSouth label                       : %s",
		    editor_obj_data->range,
		    editor_obj_data->length,
		    editor_obj_data->width,
		    editor_obj_data->surface_type_s,
		    editor_obj_data->dashes,
		    editor_obj_data->edge_light_spacing,
		    editor_obj_data->north_label,
		    editor_obj_data->south_label
		);
	    strncat(s, s1, REMAINING(s));

	    strncat(s, "\nHas thresholds ?               : ", REMAINING(s));
	    if(editor_obj_data->has_thresholds_s != NULL)
		strncat(s, "y", REMAINING(s));
	    else
		strncat(s, "n", REMAINING(s));

	    strncat(s, "\nHas borders ?                   : ", REMAINING(s));
	    if(editor_obj_data->has_borders_s != NULL)
		strncat(s, "y", REMAINING(s));
	    else
		strncat(s, "n", REMAINING(s));

	    strncat(s, "\nHas touch down markers ? : ", REMAINING(s));
	    if(editor_obj_data->has_td_markers_s != NULL)
		strncat(s, "y", REMAINING(s));
	    else
		strncat(s, "n", REMAINING(s));

	    strncat(s, "\nHas midway markers ?       : ", REMAINING(s));
	    if(editor_obj_data->has_midway_markers_s != NULL)
		strncat(s, "y", REMAINING(s));
	    else
		strncat(s, "n", REMAINING(s));

	    strncat(s, "\nHas north glide slope ?      : ", REMAINING(s));
	    if(editor_obj_data->has_north_gs_s != NULL)
		strncat(s, "y", REMAINING(s));
	    else
		strncat(s, "n", REMAINING(s));

	    strncat(s, "\nHas south glide slope ?     : ", REMAINING(s));
	    if(editor_obj_data->has_south_gs_s != NULL)
		strncat(s, "y", REMAINING(s));
	    else
		strncat(s, "n", REMAINING(s));

	    break;

	case SAR_OBJ_TYPE_HELIPAD:
	    strncat(s, "\n-------- Helipad specific data --------\n", REMAINING(s));

	    snprintf(s1, S_LENGTH,
		     "Style            : %s\nLength (m)    : %.3f\nWidth (m)      : %.3f\nRecession (ft): %.3f\nLabel           : %s\nHas edge lighting ?   : %c\nHas fuel ?                : %c\nHas repair ?             : %c\nHas drop off ?           : %c\nIs a restarting point ? : %c\n",
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
	    strncat(s, s1, REMAINING(s));

	    /* Has helipad a reference object? */
	    if(editor_obj_data->ref_obj_name != NULL)
	    {
		snprintf(s1, S_LENGTH,
		     "Ref. object name : %s\nOffset position (m/m/ft) : %.3f %.3f %.3f\nOffset direction (deg) : %.3f %.3f %.3f\n\n-------- Reference object data --------\nPosition (m/m/ft) : %.3f %.3f %.3f\nDirection (deg)  : %.3f %.3f %.3f",
		    editor_obj_data->ref_obj_name,
		    editor_obj_data->offset_pos.x,
		    editor_obj_data->offset_pos.y,
		    editor_obj_data->offset_pos.z,
		    editor_obj_data->offset_dir.heading,
		    editor_obj_data->offset_dir.pitch,
		    editor_obj_data->offset_dir.bank,
		    editor_obj_data->ref_obj_pos.x,
		    editor_obj_data->ref_obj_pos.y,
		    editor_obj_data->ref_obj_pos.z,
		    editor_obj_data->ref_obj_dir.heading,
		    editor_obj_data->ref_obj_dir.pitch,
		    editor_obj_data->ref_obj_dir.bank
		);
		strncat(s, s1, REMAINING(s));
	    }

	    break;

	case SAR_OBJ_TYPE_HUMAN:
	    strncat(s, "\n-------- Human specific data --------\n", REMAINING(s));

	    snprintf(s1, S_LENGTH,
		     "Type name         : %s",
		    editor_obj_data->type_name
	    );
	    strncat(s, s1, REMAINING(s));

	    strncat(s, "\nNeeds rescue ?  : ", REMAINING(s));
	    if(editor_obj_data->need_rescue_s != NULL)
		strncat(s, "y", REMAINING(s));
	    else
		strncat(s, "n", REMAINING(s));

	    strncat(s, "\nSit up ?             : ", REMAINING(s));
	    if(editor_obj_data->sit_up_s != NULL)
		strncat(s, "y", REMAINING(s));
	    else
		strncat(s, "n", REMAINING(s));

	    strncat(s, "\nSit down ?         : ", REMAINING(s));
	    if(editor_obj_data->sit_down_s != NULL)
		strncat(s, "y", REMAINING(s));
	    else
		strncat(s, "n", REMAINING(s));

	    strncat(s, "\nSitting ?             : ", REMAINING(s));
	    if(editor_obj_data->sitting_s != NULL)
		strncat(s, "y", REMAINING(s));
	    else
		strncat(s, "n", REMAINING(s));

	    strncat(s, "\nLying ?              : ", REMAINING(s));
	    if(editor_obj_data->lying_s != NULL)
		strncat(s, "y", REMAINING(s));
	    else
		strncat(s, "n", REMAINING(s));

	    strncat(s, "\nAlerts ?              : ", REMAINING(s));
	    if(editor_obj_data->alert_s != NULL)
		strncat(s, "y", REMAINING(s));
	    else
		strncat(s, "n", REMAINING(s));

	    strncat(s, "\nIs aware ?          : ", REMAINING(s));
	    if(editor_obj_data->aware_s != NULL)
		strncat(s, "y", REMAINING(s));
	    else
		strncat(s, "n", REMAINING(s));

	    strncat(s, "\nIn water ?           : ", REMAINING(s));
	    if(editor_obj_data->in_water_s != NULL)
		strncat(s, "y", REMAINING(s));
	    else
		strncat(s, "n", REMAINING(s));

	    strncat(s, "\nOn a stretcher ?  : ", REMAINING(s));
	    if(editor_obj_data->on_stretcher_s != NULL)
		strncat(s, "y", REMAINING(s));
	    else
		strncat(s, "n", REMAINING(s));

	    snprintf(s1, S_LENGTH, "\nAssistant(s)         : %d\n",
			editor_obj_data->assistants
		     );
	    strncat(s, s1, REMAINING(s));

	    /* Has this human assistants?
	     * Note that EditorObjectDataStructNew() sets assistants number
	     * to 0 if human type is "victim_stretcher_assisted".
	     */
	    if(editor_obj_data->assistants > 0)
	    {
		for(i = 0; i < editor_obj_data->assistants; i++)
		{
		    snprintf(s1, S_LENGTH, "Assistant #%d type : %s\n",
				i + 1,
				editor_obj_data->assist_type_name[i]
			    );
		    strncat(s, s1, REMAINING(s));
		}
	    }

	    /* Has this human a displacement? */
	    if((editor_obj_data->human_displacement_dir_s != NULL)
	    )
	    {
		strncat(s, "\n-------- Human displacement --------\n", REMAINING(s));
		snprintf(s1, S_LENGTH, "Action type   : %s\nRef. name     : %s\n",
			    editor_obj_data->human_displacement_dir_s,
			    editor_obj_data->ref_obj_name
			);
		strncat(s, s1, REMAINING(s));

		/* Is reference object the player object? */
		if(editor_obj_data->ref_obj_num == -2)
		{
		    strncat(s, "Ref. obj. pos. : (player position)\nRef. obj. dir.  : (player direction)\n", REMAINING(s));
		}
		else if(editor_obj_data->ref_obj_num >= 0)
		{
		    snprintf(s1, S_LENGTH, "Ref. obj. pos. (m/m/ft) : %.3f %.3f %.3f\nRef. obj. dir. (deg)   : %.3f %.3f %.3f\n",
				editor_obj_data->ref_obj_pos.x,
				editor_obj_data->ref_obj_pos.y,
				editor_obj_data->ref_obj_pos.z,
				editor_obj_data->ref_obj_dir.heading,
				editor_obj_data->ref_obj_dir.pitch,
				editor_obj_data->ref_obj_dir.bank
			    );
		    strncat(s, s1, REMAINING(s));
		}
	    }

	    break;

	case SAR_OBJ_TYPE_SMOKE:
	    strncat(s, "\n-------- Smoke specific data --------\n", REMAINING(s));

	    snprintf(s1, S_LENGTH,
		    "X offset (m)                : %.3f\nY offset (m)                : %.3f\nZ offset (m)                : %.3f\nStart radius (m)           : %.3f\nMax radius (m)           : %.3f\nRadius rate (m/s)        : %.3f\nHide at max (m)          : %.3f\nRespawn interval (ms) : %ld\nTotal units                  : %d\nColor code                 : %d\n",
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
	    strncat(s, s1, REMAINING(s));

	    break;

	case SAR_OBJ_TYPE_FIRE:
	    strncat(s, "\n--------- Fire specific data ---------\n", REMAINING(s));

	    snprintf(s1, S_LENGTH,
		    "Radius (m)  : %.3f\nHeight (ft)   : %.3f\n",
		    editor_obj_data->radius,
		    editor_obj_data->height
	    );
	    strncat(s, s1, REMAINING(s));

	    break;

	case SAR_OBJ_TYPE_EXPLOSION:
	case SAR_OBJ_TYPE_CHEMICAL_SPRAY:
	case SAR_OBJ_TYPE_FUELTANK:
	    break;

	case SAR_OBJ_TYPE_PREMODELED:
	    strncat(s, "\n-------- Premodeled specific data --------\n", REMAINING(s));

	    snprintf(s1, S_LENGTH,
		     "Premodeled type  : %s\nVisual range        : %.3f\n",
		    editor_obj_data->pm_type_s,
		    editor_obj_data->range
	    );
	    strncat(s, s1, REMAINING(s));

	    switch(editor_obj_data->pm_type)
	    {
		case SAR_OBJ_PREMODELED_POWER_TRANSMISSION_TOWER:
		case SAR_OBJ_PREMODELED_TOWER:
		case SAR_OBJ_PREMODELED_RADIO_TOWER:
		    snprintf(s1, S_LENGTH,
			"Height                 : %.3f\nHazard lights       : %d\n",
			editor_obj_data->height,
			editor_obj_data->hazard_lights
		);
		strncat(s, s1, REMAINING(s));
		    break;

		case SAR_OBJ_PREMODELED_CONTROL_TOWER:
		    snprintf(s1, S_LENGTH,
			"Length                : %.3f\nWidth                  : %.3f\nHeight                 : %.3f\nWalls texture        : %s\nRoof texture         : %s\n",
			editor_obj_data->length,
			editor_obj_data->width,
			editor_obj_data->height,
			editor_obj_data->walls_texture_s,
			editor_obj_data->roof_texture_s
		);
		strncat(s, s1, REMAINING(s));
		    break;

		case SAR_OBJ_PREMODELED_BUILDING:
		    snprintf(s1, S_LENGTH,
			"Length                : %.3f\nWidth                  : %.3f\nHeight                 : %.3f\nWalls texture        : %s\nRoof texture         : %s\nRoof night texture : %s\n",
			editor_obj_data->length,
			editor_obj_data->width,
			editor_obj_data->height,
			editor_obj_data->walls_texture_s,
			editor_obj_data->walls_texture_night_s,
			editor_obj_data->roof_texture_s
		);
		strncat(s, s1, REMAINING(s));
		    break;

		default:
		    break;
	    }

	    break;

	default:
	    break;
    }


    if(scn_ed->gtk_mode_on == True)
    {
//Display *x11_display = display->display;

//fprintf(stderr, "XPending(x11_display) = %d\n", XPending(x11_display) );
//XSync(x11_display, True);
//fprintf(stderr, "XPending(x11_display) = %d\n", XPending(x11_display) );

	//gtkShowWindow(core_ptr, window_title, s);
    }
    else
    {
	if(True)
	{
	    /* Ugly workaround to avoid immediate window closing: sleep 0.2s .
	     * The [OK] button is set as the default button by
	     * GWOutputMessage() thus when user press the <Enter> key to
	     * get the object information window, [OK] button is validated
	     * therefore the info window is closed immediately.
	     */
	    struct timespec requested, remaining;
	    requested.tv_sec = 0;
	    requested.tv_nsec = 200000000;
	    nanosleep(&requested, &remaining);
	}

	GWOutputMessage(
	    display,
	    GWOutputMessageTypeGeneral,	/* One of GWOutputMessageType* */
	    window_title,		/* Subject string */
	    s,				/* Message string */
	    NULL			/* Help string */
	);
    }

    free(window_title);
    free(s1);
    free(s);
    free(type_str);

    EditorObjectDataStructFree(editor_obj_data);

    return obj_num;
#undef S_LENGTH
}

/*
 *      Near same as SARSimFindGround() except that an object pointer
 *      is passed instead of an object position structure and that this
 *      object is skipped before checking.
 */
float ScnEditFindGround(
    sar_scene_struct *scene,
    sar_object_struct **ptr, int total,
    sar_object_struct *src_obj_ptr
)
{
    int i;
    sar_object_struct *tar_obj_ptr;
    float new_height, cur_height = 0.0f;
    const sar_contact_bounds_struct *cb_tar;
    const sar_position_struct *pos_src;

    if(src_obj_ptr == NULL)
	return(cur_height);
    else
	pos_src = &src_obj_ptr->pos;

    if((scene == NULL) || (pos_src == NULL))
	return(cur_height);

    /* Iterate from last object to first */
    for(i = total - 1; i >= 0; i--)
    {
	tar_obj_ptr = ptr[i];
	if(tar_obj_ptr == NULL)
	    continue;

	/* Skip source object */
	if(tar_obj_ptr == src_obj_ptr)
	    continue;

	/* Check if target object specifies a landable surface */
	cb_tar = tar_obj_ptr->contact_bounds;
	if(cb_tar != NULL)
	{
	    if(cb_tar->crash_flags & SAR_CRASH_FLAG_SUPPORT_SURFACE)
	    {
		new_height = SARSimSupportSurfaceHeight(
		    tar_obj_ptr, cb_tar, pos_src,
		    (float)SAR_DEF_SURFACE_CONTACT_Z_TOLORANCE
		);
		if(new_height > cur_height)
		    cur_height = new_height;
	    }
	}

	/* Get ground object height (function will check
	 * if object really is a ground object), on error
	 * it will return 0.0 so accepting its value unconditionally
	 * is safe.
	 *
	 * Both ground object cylendrical contact and heightfield
	 * contact will be checked.
	 */
	new_height = (float)SARSimHFGetGroundHeight(
	    tar_obj_ptr,    /* Ground object */
	    pos_src
	);

	/* New height higher? */
	if(new_height > cur_height)
	    cur_height = new_height;
    }

    return(cur_height);
}


/*
void enum_windows(Display* display, Window window, int depth) {
  int i;

  XTextProperty text;
  XGetWMName(display, window, &text);
  char* name;
  XFetchName(display, window, &name);
  for (i = 0; i < depth; i++)
    printf("\t");
  printf("id=0x%x, XFetchName=\"%s\", XGetWMName=\"%s\"\n", window, name != NULL ? name : "(no name)", text.value);

  Window root, parent;
  Window* children;
  int n;
  XQueryTree(display, window, &root, &parent, &children, &n);
  if (children != NULL) {
    for (i = 0; i < n; i++) {
      enum_windows(display, children[i], depth + 1);
    }
    XFree(children);
  }
}
*/


int ScnEditPrintModificationsList(sar_scenery_editor_struct *scn_ed, FILE *fp)
{
#define S_LENGTH 1023

#define PRINTCOMMANDNAME(type)			\
switch(type)					\
{						\
    case SAR_OBJ_TYPE_GARBAGE:			\
	break;					\
    case SAR_OBJ_TYPE_STATIC:			\
    case SAR_OBJ_TYPE_AUTOMOBILE:		\
    case SAR_OBJ_TYPE_WATERCRAFT:		\
    case SAR_OBJ_TYPE_AIRCRAFT:			\
    case SAR_OBJ_TYPE_GROUND:			\
	fprintf(fp, "create_object %d\n", type);\
	fprintf(fp, "model_file ");		\
	break;					\
    case SAR_OBJ_TYPE_RUNWAY:			\
	fprintf(fp, "create_runway ");		\
	break;					\
    case SAR_OBJ_TYPE_HELIPAD:			\
	fprintf(fp, "create_helipad ");		\
	break;					\
    case SAR_OBJ_TYPE_HUMAN:			\
	fprintf(fp, "create_human ");		\
	break;					\
    case SAR_OBJ_TYPE_SMOKE:			\
	fprintf(fp, "create_smoke ");		\
	break;					\
    case SAR_OBJ_TYPE_FIRE:			\
	fprintf(fp, "create_fire ");		\
	break;					\
    case SAR_OBJ_TYPE_EXPLOSION:		\
    case SAR_OBJ_TYPE_CHEMICAL_SPRAY:		\
    case SAR_OBJ_TYPE_FUELTANK:			\
	break;					\
    case SAR_OBJ_TYPE_PREMODELED:		\
	fprintf(fp, "create_premodeled ");	\
	break;					\
}

    editor_modified_object_struct *modification;
    editor_object_data_struct *data_new, *data_original;
    int obj_num, total_original_objects, total_objects, i;
    char *s = malloc(S_LENGTH * sizeof(char));

    if(scn_ed == NULL || s == NULL)
	return -1;

    s[0] = '\0';

    total_original_objects = scn_ed->total_original_objects;
    total_objects = scn_ed->total_objects - 1;

    /* Print deleted (and not modified) original objects list */
    for(obj_num = 0; obj_num < total_original_objects; obj_num++)
    {
	modification = scn_ed->modification_list[obj_num];

	if((modification->flags & EDITOR_OBJECT_FLAG_DELETED) &&
	    !(modification->flags & EDITOR_OBJECT_FLAG_MODIFIED)
	)
	{
	    /* Note that "deleted" means "deleted AND not modified" */
	    fprintf(fp, "Object #\033[7m%05d\033[0m has been \033[7mremoved\033[0m.\n", obj_num);
	    continue;
	}
    }
    fprintf(fp, "\n");

    /* Print modified or/and moved original objects list */
    for(obj_num = 0; obj_num < total_original_objects; obj_num++)
    {
	modification = scn_ed->modification_list[obj_num];
	data_original = modification->obj_data_original;
	data_new = modification->obj_data_new;

	if(data_original == NULL && data_new == NULL)
	    continue;

	/* Aurguments modification only? */
	if((modification->flags & EDITOR_OBJECT_FLAG_MODIFIED) &&
	    !(modification->flags & EDITOR_OBJECT_FLAG_MOVED)
	)
	{
	    char *s1;

	    fprintf(fp,
"\033[7m%s"TAGVALLENGTH"\033[0m \033[7mdefinition\033[0m \033[7mchanged\033[0m:\n",
		TAGSTRING,
		obj_num
	    );

	    fprintf(fp, "\033[7mFrom\033[0m:\n");

	    /* Generate then print the 'original object' command line string */
	    PRINTCOMMANDNAME(data_original->type)
	    s1 = EditorObjectDataDoParametersLine(data_original);
	    fprintf(fp, "%s\n", s1);
	    free(s1);

	    fprintf(fp, "\033[7mTo\033[0m:\n");

	    /* Generate then print the 'new object' command line string */
	    PRINTCOMMANDNAME(data_new->type)
	    s1 = EditorObjectDataDoParametersLine(data_new);
	    fprintf(fp, "%s\n", s1);
	    free(s1);

	    fprintf(fp, "\n");
	}
	/* Position modification only? */
	else if((modification->flags & EDITOR_OBJECT_FLAG_MOVED) &&
		!(modification->flags & EDITOR_OBJECT_FLAG_MODIFIED)
	)
	{
	    fprintf(fp,
"\033[7m%s"TAGVALLENGTH"\033[0m \033[7mposition\033[0m \033[7mchanged\033[0m:\n",
		TAGSTRING,
		obj_num
	    );

	    fprintf(fp, "\033[7mFrom\033[0m:\n");
	    fprintf(fp, "translate %.3f %.3f %.3f\n",
		    data_original->pos.x,
		    data_original->pos.y,
		    data_original->pos.z
		);
	    fprintf(fp, "rotate %.3f %.3f %.3f\n",
		    data_original->dir.heading,
		    data_original->dir.pitch,
		    data_original->dir.bank
		);

	    fprintf(fp, "\033[7mTo\033[0m:\n");
	    fprintf(fp, "translate %.3f %.3f %.3f\n",
		    data_new->pos.x,
		    data_new->pos.y,
		    data_new->pos.z
		);
	    if(data_new->type != SAR_OBJ_TYPE_FIRE &&
		data_new->type != SAR_OBJ_TYPE_SMOKE
	    )
	    {
		fprintf(fp, "rotate %.3f %.3f %.3f\n",
			data_new->dir.heading,
			data_new->dir.pitch,
			data_new->dir.bank
		    );
	    }

	    fprintf(fp, "\n");
	}
	/* Arguments and position modification? */
	else if((modification->flags & EDITOR_OBJECT_FLAG_MODIFIED) &&
	    (modification->flags & EDITOR_OBJECT_FLAG_MOVED)
	)
	{
	    char *s1;

	    fprintf(fp,
		    "\033[7m%s"TAGVALLENGTH"\033[0m \033[7mchanged\033[0m:\n",
		    TAGSTRING,
		    obj_num
		);

	    fprintf(fp, "\033[7mFrom\033[0m:\n");

	    /* Generate then print the 'original object' command line string */
	    PRINTCOMMANDNAME(data_original->type)
	    s1 = EditorObjectDataDoParametersLine(data_original);
	    fprintf(fp, "%s\n", s1);
	    free(s1);
	    fprintf(fp, "translate %.3f %.3f %.3f\n",
		    data_original->pos.x,
		    data_original->pos.y,
		    data_original->pos.z
		);
	    fprintf(fp, "rotate %.3f %.3f %.3f\n",
		    data_original->dir.heading,
		    data_original->dir.pitch,
		    data_original->dir.bank
		);

	    fprintf(fp, "\033[7mTo\033[0m:\n");

	    /* Generate then print the 'new object' command line string */
	    PRINTCOMMANDNAME(data_new->type)
	    s1 = EditorObjectDataDoParametersLine(data_new);
	    fprintf(fp, "%s\n", s1);
	    free(s1);
	    fprintf(fp, "translate %.3f %.3f %.3f\n",
		    data_new->pos.x,
		    data_new->pos.y,
		    data_new->pos.z
		);
	    if(data_new->type != SAR_OBJ_TYPE_FIRE &&
		data_new->type != SAR_OBJ_TYPE_SMOKE
	    )
	    {
		fprintf(fp, "rotate %.3f %.3f %.3f\n",
			data_new->dir.heading,
			data_new->dir.pitch,
			data_new->dir.bank
		    );
	    }

	    fprintf(fp, "\n");
	}
    }

    /* Print new objects list */
    for(obj_num = total_original_objects; obj_num < total_objects; obj_num++)
    {
	modification = scn_ed->modification_list[obj_num];

	data_new = modification->obj_data_new;

	if(data_new == NULL)
	    continue;

	if(modification->flags & EDITOR_OBJECT_FLAG_DELETED)
	    continue;

	fprintf(fp, "\033[7m%s"TAGVALLENGTH"\033[0m \033[7madded\033[0m:\n",
		TAGSTRING,
		obj_num
		);

	switch(data_new->type)
	{
	    case SAR_OBJ_TYPE_GARBAGE:
		break;

	    case SAR_OBJ_TYPE_STATIC:
	    case SAR_OBJ_TYPE_AUTOMOBILE:
	    case SAR_OBJ_TYPE_WATERCRAFT:
	    case SAR_OBJ_TYPE_AIRCRAFT:
	    case SAR_OBJ_TYPE_GROUND:
		snprintf(s, S_LENGTH, "create_object %d", data_new->type);
		fprintf(fp, "%s\n", s);
		if(data_new->file_name != NULL)
		{
		    snprintf(s, S_LENGTH, "model_file %s", data_new->file_name);
		    fprintf(fp, "%s\n", s);
		}
		break;

	    case SAR_OBJ_TYPE_RUNWAY:
		fprintf(fp,
			"create_runway %.3f %.3f %.3f %d %d %.3f %s %s %.3f %.3f",
			data_new->range,
			data_new->length,
			data_new->width,
			data_new->surface_type,
			data_new->dashes,
			data_new->edge_light_spacing,
			data_new->north_label,
			data_new->south_label,
			data_new->north_displaced_threshold,
			data_new->south_displaced_threshold
		    );
		    if(data_new->has_thresholds_s != NULL)
			fprintf(fp, " %s", data_new->has_thresholds_s);
		    if(data_new->has_borders_s != NULL)
			fprintf(fp, " %s", data_new->has_borders_s);
		    if(data_new->has_td_markers_s != NULL)
			fprintf(fp, " %s", data_new->has_td_markers_s);
		    if(data_new->has_midway_markers_s != NULL)
			fprintf(fp, " %s", data_new->has_midway_markers_s);
		    if(data_new->has_north_gs_s != NULL)
			fprintf(fp, " %s", data_new->has_north_gs_s);
		    if(data_new->has_south_gs_s != NULL)
			fprintf(fp, " %s", data_new->has_south_gs_s);

		    fprintf(fp, "\n");
		break;

	    case SAR_OBJ_TYPE_HELIPAD:
		fprintf(fp,
			"create_helipad %s %.3f %.3f %.3f %s %c %c %c %c %c",
			data_new->style_s,
			data_new->length,
			data_new->width,
			data_new->recession,
			data_new->label,
			data_new->edge_lighting_c,
			data_new->has_fuel_c,
			data_new->has_repair_c,
			data_new->has_drop_off_c,
			data_new->restarting_point_c
		    );
		if(data_new->ref_obj_name != NULL)
		{
		    fprintf(fp,
			" %s %.3f %.3f %.3f %.3f %.3f %.3f\n",
			data_new->ref_obj_name,
			data_new->offset_pos.x,
			data_new->offset_pos.y,
			data_new->offset_pos.z,
			data_new->offset_dir.heading,
			data_new->offset_dir.pitch,
			data_new->offset_dir.bank
		    );
		}
		else
		    fprintf(fp, "\n");
		break;

	    case SAR_OBJ_TYPE_HUMAN:
		fprintf(fp, "create_human %s", data_new->type_name);
		if(data_new->need_rescue_s != NULL)
		    fprintf(fp, " %s", data_new->need_rescue_s);
		if(data_new->sit_up_s != NULL)
		    fprintf(fp, " %s", data_new->sit_up_s);
		if(data_new->sit_down_s != NULL)
		    fprintf(fp, " %s", data_new->sit_down_s);
		if(data_new->sitting_s != NULL)
		    fprintf(fp, " %s", data_new->sitting_s);
		if(data_new->lying_s != NULL)
		    fprintf(fp, " %s", data_new->lying_s);
		if(data_new->alert_s != NULL)
		    fprintf(fp, " %s", data_new->alert_s);
		if(data_new->aware_s != NULL)
		    fprintf(fp, " %s", data_new->aware_s);
		if(data_new->in_water_s != NULL)
		    fprintf(fp, " %s", data_new->in_water_s);
		if(data_new->on_stretcher_s != NULL)
		    fprintf(fp, " %s", data_new->on_stretcher_s);

		if(data_new->assisted_s != NULL)
		{
		    fprintf(fp, " %s %d",
			     data_new->assisted_s,
			     data_new->assistants
			);
		    for(i = 0; i < data_new->assistants; i++)
			fprintf(fp, " %s", data_new->assist_type_name[i]);
		}

		fprintf(fp, "\n");
		break;

	    case SAR_OBJ_TYPE_SMOKE:
		fprintf(fp,
		    "create_smoke %.3f %.3f %.3f %.3f %.3f %.3f %.3f %ld %d %d\n",
		    data_new->offset_pos.x,
		    data_new->offset_pos.y,
		    data_new->offset_pos.z,
		    data_new->radius_start,
		    data_new->radius_max,
		    data_new->radius_rate,
		    data_new->hide_at_max,
		    data_new->respawn_int,
		    data_new->total_units,
		    data_new->color_code
		);
		break;

	    case SAR_OBJ_TYPE_FIRE:
		fprintf(fp, "create_fire %.3f %.3f\n",
		    data_new->radius,
		    data_new->height
		);
		break;

	    case SAR_OBJ_TYPE_EXPLOSION:
	    case SAR_OBJ_TYPE_CHEMICAL_SPRAY:
	    case SAR_OBJ_TYPE_FUELTANK:
		break;

	    case SAR_OBJ_TYPE_PREMODELED:
		fprintf(fp, "create_premodeled %s", data_new->pm_type_s);
		switch(data_new->pm_type)
		{
		    case SAR_OBJ_PREMODELED_BUILDING:
			fprintf(fp, " %.3f %.3f %.3f %.3f %s %s %s\n",
			    data_new->range,
			    data_new->length,
			    data_new->width,
			    data_new->height,
			    data_new->walls_texture_s,
			    data_new->walls_texture_night_s,
			    data_new->roof_texture_s
			);
		    break;

		    case SAR_OBJ_PREMODELED_CONTROL_TOWER:
			fprintf(fp, " %.3f %.3f %.3f %.3f %s %s\n",
			    data_new->range,
			    data_new->length,
			    data_new->width,
			    data_new->height,
			    data_new->walls_texture_s,
			    data_new->roof_texture_s
			);
		    break;

		    case SAR_OBJ_PREMODELED_HANGAR:
			;
		    break;

		    case SAR_OBJ_PREMODELED_POWER_TRANSMISSION_TOWER:
		    case SAR_OBJ_PREMODELED_TOWER:
		    case SAR_OBJ_PREMODELED_RADIO_TOWER:
			fprintf(fp, " %.3f %.3f %d\n",
			    data_new->range,
			    data_new->height,
			    data_new->hazard_lights
			);
		    break;
		}
		break;
	}

	if(data_new->name != NULL)
	    fprintf(fp, "object_name %s\n", data_new->name);

	/* FIXME compare don't work if limit if is set to > 0.0f instead of > 0.0005f */
	if(data_new->range > 0.0005f)
	    fprintf(fp, "range %.3f\n", data_new->range);

	if(data_new->object_map_description != NULL &&
	    strlen(data_new->object_map_description) != 0
	)
	    fprintf(fp, "object_map_description %s\n", data_new->object_map_description);

	fprintf(fp, "translate %.3f %.3f %.3f\n",
		    data_new->pos.x,
		    data_new->pos.y,
		    data_new->pos.z
		);

	if(data_new->type != SAR_OBJ_TYPE_FIRE &&
	    data_new->type != SAR_OBJ_TYPE_SMOKE
	)
	{
	    fprintf(fp, "rotate %.3f %.3f %.3f\n",
			data_new->dir.heading,
			data_new->dir.pitch,
			data_new->dir.bank
		    );
	}

	fprintf(fp, "\n");
    }

    free(s);
    return total_objects;
#undef PRINTCOMMANDNAME
#undef S_LENGTH
}



/*
 *	Scenery editor utility.
 */
void SARCmdSceneEditor(SAR_CMD_PROTOTYPE)
{
    sar_core_struct *core_ptr = SAR_CORE(data);
    sar_scene_struct *scene = core_ptr->scene;
    gw_display_struct *display = core_ptr->display;
    sar_scenery_editor_struct *scn_ed = core_ptr->in_game_editor;
    int player_obj_num, obj_num = -1, picked_obj_num, new_obj_num;
    int strc, i;
    sar_obj_type type;
    sar_object_struct *obj_ptr, *player_obj_ptr, *picked_obj_ptr;
    sar_object_aircraft_struct *aircraft = NULL;
    sar_object_smoke_struct *smoke = NULL;
    sar_object_explosion_struct *explosion = NULL;
    sar_object_fire_struct *fire = NULL;
    sar_object_fueltank_struct *fueltank = NULL;
    sar_object_helipad_struct *helipad = NULL;
    char **strv, *cmd_args = NULL;
    FILE *editor_scn_file = NULL;
    editor_object_data_struct *editor_obj_data;
    editor_modified_object_struct *modification;

#define EDITOROBJECTSETDATA(obj_num)					\
{									\
    modification = scn_ed->modification_list[obj_num];			\
    editor_obj_data = modification->obj_data_new;			\
    EditorObjectDataStructFill(						\
	core_ptr,							\
	editor_obj_data,						\
	obj_num								\
    );									\
}

#define EDITOROBJECTSETMODIFIED(obj_num)		\
{							\
modification = scn_ed->modification_list[obj_num];	\
modification->flags |= EDITOR_OBJECT_FLAG_MODIFIED;	\
modification->flags &= ~EDITOR_OBJECT_FLAG_MOVED;	\
}

#define EDITOROBJECTSETDELETED(obj_num)					\
{									\
modification = scn_ed->modification_list[obj_num];			\
modification->flags |= EDITOR_OBJECT_FLAG_DELETED;			\
if(modification->flags & EDITOR_OBJECT_FLAG_MODIFIED)			\
    modification->flags &= ~EDITOR_OBJECT_FLAG_MODIFIED;		\
if(modification->flags & EDITOR_OBJECT_FLAG_MOVED)			\
    modification->flags &= ~EDITOR_OBJECT_FLAG_MOVED;			\
}

#define EDITOROBJECTSETMOVED(obj_num)			\
{							\
modification = scn_ed->modification_list[obj_num];	\
modification->flags |= EDITOR_OBJECT_FLAG_MOVED;	\
}

    Boolean in_move, next;

    if(scene == NULL)
	return;

    if(display == NULL)
	return;

    /* Get player object references from scene structure */
    player_obj_num = scene->player_obj_num;
    player_obj_ptr = scene->player_obj_ptr;

    /* Get aircraft */
    aircraft = SAR_OBJ_GET_AIRCRAFT(player_obj_ptr);
    if(aircraft == NULL)
	return;

    /* No argument? */
    if(*arg == '\0')
    {
	NOTIFY(
	    "Usage: scnedit on|off"
	);
	return;
    }

    if(scn_ed == NULL)
    {
	/* Allocate then initialize scenery editor structure */
	scn_ed = malloc(sizeof(sar_scenery_editor_struct));
	if(scn_ed != NULL)
	{
	    scn_ed->scn_file_name = NULL;
	    scn_ed->scn_file_fp = NULL;
	    scn_ed->cur_obj_type = SAR_OBJ_TYPE_GARBAGE;
	    scn_ed->cur_obj_num = -1;
	    scn_ed->cur_obj_arg = NULL;
	    scn_ed->in_move_state = False;
	    scn_ed->in_modif_state = False;
	    scn_ed->gtk_mode_on = False;
	    scn_ed->gtk_application = NULL;
	    scn_ed->gtk_app_running = False;
	    scn_ed->gtk_context = NULL;
	    scn_ed->pick_skip_list_index = -1;
	    for(i = 0; i < sizeof(scn_ed->pick_skip_list)/sizeof(int); i++)
		scn_ed->pick_skip_list[i] = -1;
	    scn_ed->prev_obj_type = SAR_OBJ_TYPE_GARBAGE;
	    scn_ed->prev_obj_num = -1;
	    scn_ed->prev_obj_arg = NULL;
	    scn_ed->mod_obj_num = -1;
	    scn_ed->total_original_objects = core_ptr->total_objects;
	    scn_ed->total_objects = 0;
	    scn_ed->modification_list = NULL;
	}
	else
	    return;

	core_ptr->in_game_editor = (sar_scenery_editor_struct *)scn_ed;
    }

    /* Parse command argument */
    strv = strexp(arg, &strc);

    cmd_args = (char *)arg;

    /* Get editor scene work file pointer.
     * scn_ed->scn_file_fp is NULL when editor is OFF.
     */
    editor_scn_file = scn_ed->scn_file_fp;

    if(scn_ed->in_move_state == True)
	in_move = True;
    else
	in_move = False;

    /* Note:
     * Always check if the 'in_move' flag is set before checking command name
     * because if it is set, only "set" and "scnedit off" commands are allowed.
     */

    /* Set (stick to scene) current object? */
    if(!strcasecmp(strv[0], "set") ||
	!strcasecmp(strv[0], "set_to_ground") ||
	!strcasecmp(strv[0], "stg")
    )
    {
	scn_ed->in_modif_state = False;

	obj_num = scn_ed->cur_obj_num;

	/* Force Z value to ground level? */
	if(!strcasecmp(strv[0], "set_to_ground") ||
	    !strcasecmp(strv[0], "stg")
	)
	{
	    float ground_height;

	    /* Break link between object and player object */
	    scn_ed->cur_obj_num = -1;

	    /* Is there a current object to set? */
	    if(obj_num >= 0)
		obj_ptr = ((obj_num < 0) ? NULL : (*&core_ptr->object)[obj_num]);
	    /* No current object to set, use player object */
	    else
		obj_ptr = player_obj_ptr;

	    /* Get ground height below  */
	    ground_height = ScnEditFindGround(
				scene,
				core_ptr->object, core_ptr->total_objects,
				obj_ptr
			    );

	    /* Move object to ground first */
	    if(obj_ptr != NULL)
	    {
		obj_ptr->pos.z = ground_height;
		/* Realize object position */
		SARSimWarpObject(scene, obj_ptr, &obj_ptr->pos, &obj_ptr->dir);
	    }

	    /* Move player object to ground FIXME Seems to not work!!! */
	    player_obj_ptr->pos.z = ground_height;
	    /* Realize player object position */
	    SARSimWarpObject(scene, player_obj_ptr, &player_obj_ptr->pos, &player_obj_ptr->dir);

	    /* Restore object to player object link */
	    scn_ed->cur_obj_num = obj_num;
	}

	/* In a "move" procedure? */
	if(scn_ed->in_move_state == True)
	{
	    /* Unlink object from player object (stick object to scene) */
	    scn_ed->cur_obj_num = -1;

	    /* Object has been sticked to scene, move procedure is finished */
	    scn_ed->in_move_state = False;

	    char *s = (char *)malloc(NOTIFYSTRINGLENGTH * sizeof(char));
	    obj_ptr = ((obj_num < 0) ? NULL : (*&core_ptr->object)[obj_num]);

	    EDITOROBJECTSETDATA(obj_num)
	    EDITOROBJECTSETMOVED(obj_num)

	    /* Notify user that move has been done */
	    snprintf(s, NOTIFYSTRINGLENGTH, "Object #%d moved to %.3f %.3f %.3f\n",
			obj_num,
			obj_ptr->pos.x,
			obj_ptr->pos.y,
			obj_ptr->pos.z
		    );
	    NOTIFY(s);
	    free(s);
	}
	/* Not in a "move object" procedure */
	else
	{
	    /* No object currently loaded? */
	    if(obj_num < 0)
	    {
/* FIXME don't work properly: printed translate and rotate values are always 0.0f */
		/* Presume that user wants to set a triedron, thus create it */

		new_obj_num = ScnEditLoadObject(core_ptr, -1, SAR_OBJ_TYPE_STATIC, REFOBJECTFILENAME);

		scn_ed->cur_obj_type = SAR_OBJ_TYPE_STATIC;
		free(scn_ed->cur_obj_arg);
		scn_ed->cur_obj_arg = STRDUP(REFOBJECTFILENAME);

		COPYCUROBJDATATOPREVOBJDATA
		EDITOROBJECTSETDATA(new_obj_num)

		/* Unlink object from player object (stick object to scene) */
		scn_ed->cur_obj_num = -1;
	    }
	    else
	    {
		COPYCUROBJDATATOPREVOBJDATA
		EDITOROBJECTSETDATA(obj_num)

		/* Unlink object from player object (stick object to scene) */
		scn_ed->cur_obj_num = -1;

		/* Load and create a new object */
		new_obj_num = ScnEditLoadObject(core_ptr, -1, scn_ed->cur_obj_type, scn_ed->cur_obj_arg);

		/* Link object to player object. Object type and args don't change */
		scn_ed->cur_obj_num = new_obj_num;
	    }

	    obj_ptr = ((new_obj_num < 0) ? NULL : (*&core_ptr->object)[new_obj_num]);
	    if(obj_ptr != NULL)
	    {
		/* Set object position as player position */
		memcpy(&obj_ptr->pos, &player_obj_ptr->pos, sizeof(sar_position_struct));
		memcpy(&obj_ptr->dir, &player_obj_ptr->dir, sizeof(sar_direction_struct));

		/* Realize object position */
		SARSimWarpObject(scene, obj_ptr, &obj_ptr->pos, &obj_ptr->dir);
	    }

	    /* Move a little bit player object to show user that
	     * object has been set.
	     */
	    player_obj_ptr->pos.x += 0.5f * cos(player_obj_ptr->dir.heading);
	    player_obj_ptr->pos.y += 0.5f * sin(player_obj_ptr->dir.heading);

	    /* Realize player new position */
	    SARSimWarpObject(scene, player_obj_ptr, &player_obj_ptr->pos, &player_obj_ptr->dir);

	    /*
	     * Check if object has been moved in order to set the
	     * EDITOR_OBJECT_FLAG_MOVED flag.
	     */

	    editor_object_data_struct *old_data, *new_data;
	    sar_position_struct *old_pos, *new_pos;
	    sar_direction_struct *old_dir, *new_dir;

	    modification = scn_ed->modification_list[new_obj_num];
	    new_data = modification->obj_data_new;
	    new_pos = &(new_data->pos);
	    new_dir = &(new_data->dir);

	    if(obj_num >= 0)
	    {
		modification = scn_ed->modification_list[obj_num];
		old_data = modification->obj_data_original;
		old_pos = &(old_data->pos);
		old_dir = &(old_data->dir);
	    }

	    if(new_data != NULL && old_data!= NULL &&
		(memcmp(old_pos, new_pos, sizeof(sar_position_struct)) ||
		memcmp(old_dir, new_dir, sizeof(sar_direction_struct)))
	    )
	    {
		/* Note that 'modification' points to 'obj_num'
		 * and not to 'new_obj_num'
		 */
		modification->flags |= EDITOR_OBJECT_FLAG_MOVED;
	    }

	    obj_num = new_obj_num;
	}
    }
    /* Exit edit mode? */
    else if((core_ptr->editor_mode_on == True) &&
	    !strcasecmp(strv[0], "scnedit") &&
	    strc > 1 &&
	    !strcasecmp(strv[1], "off")
    )
    {
	EditorOff(core_ptr);
	NOTIFY("Editing mode is now OFF");

	return;
    }
    /* In move sequence running?
     * Note that the commands which are available when a "move" sequence is
     * running must have been tested before this test.
     */
    else if(in_move)
    {
	NOTIFY("You must \"set\" currently moved object before.");
    }
    /* Unload current object? */
    else if(!strcasecmp(strv[0], "ul") || !strcasecmp(strv[0], "unload"))
    {
	scn_ed->in_modif_state = False;

	if(scn_ed->in_move_state == True)
	{
	    NOTIFY(
		"Can't unload this object because it is in \"move\" sequence."
	    );
	}
	else if(scn_ed->cur_obj_num >= 0)
	{
	    COPYCUROBJDATATOPREVOBJDATA

	    /* Unlink object from player object */
	    scn_ed->cur_obj_num = -1;

	    free(scn_ed->cur_obj_arg);
	    scn_ed->cur_obj_arg = NULL;
	    scn_ed->cur_obj_type = SAR_OBJ_TYPE_GARBAGE;

	    SARObjDelete(
		core_ptr,
		&core_ptr->object,
		&core_ptr->total_objects,
		scn_ed->prev_obj_num
	    );
	}
    }
    /* Remove closer object? */
    else if(!strcasecmp(strv[0], "rm") ||
	    !strcasecmp(strv[0], "remove") ||
	    !strcasecmp(strv[0], "del") ||
	    !strcasecmp(strv[0], "delete")
    )
    {
	scn_ed->in_modif_state = False;

	picked_obj_num = SceneObjectPick(core_ptr, player_obj_num, False);

	if(picked_obj_num > -1)
	{
	    int child_obj_num = -1;

	    picked_obj_ptr = core_ptr->object[picked_obj_num];
	    if(picked_obj_ptr == NULL)
		return;

	    /* Is this object named? */
	    if(picked_obj_ptr->name != NULL)
	    {
		/* Check if deleted object is referenced by another object */
		for(i = 0; i < scn_ed->total_objects; i++)
		{
		    modification = scn_ed->modification_list[i];
		    editor_obj_data = modification->obj_data_new;

		    /* Is deleted object referenced by another object? */
		    if(editor_obj_data->ref_obj_name != NULL &&
			!strcmp(editor_obj_data->ref_obj_name, picked_obj_ptr->name)
		    )
		    {
			child_obj_num = i;
		    }
		}
	    }

	    char *s = (char *)malloc((80 + 1) * sizeof(char));
	    if(child_obj_num < 0)
	    {
		snprintf(s, 80, "Object #%d has been removed.", picked_obj_num);

		SARObjDelete(
		    core_ptr,
		    &core_ptr->object,
		    &core_ptr->total_objects,
		    picked_obj_num
		);
		obj_num = picked_obj_num;

		EDITOROBJECTSETDELETED(obj_num)
	    }
	    else
	    {
snprintf(s, 80, "Object #%d and its child #%d have been removed.",
			 picked_obj_num,
			child_obj_num
		    );

		SARObjDelete(
		    core_ptr,
		    &core_ptr->object,
		    &core_ptr->total_objects,
		    child_obj_num
		);
		EDITOROBJECTSETDELETED(child_obj_num)

		SARObjDelete(
		    core_ptr,
		    &core_ptr->object,
		    &core_ptr->total_objects,
		    picked_obj_num
		);
		obj_num = picked_obj_num;
		EDITOROBJECTSETDELETED(obj_num)
	    }
	    NOTIFY(s);
	    free(s);
	}
    }
    /* Give info about closer object? */
    else if(!strcasecmp(strv[0], "info") || !strcasecmp(strv[0], "ifn"))
    {
	scn_ed->in_modif_state = False;

	/* "next" loop requested by user? */
	if((strc >= 2 && !strcasecmp(strv[1], "next")) ||
	    !strcasecmp(strv[0], "ifn")
	)
	    next = True;
	else
	    next = False;

	picked_obj_num = SceneObjectPick(core_ptr, player_obj_num, next);

	if(picked_obj_num > -1)
	    ScnEditShowObjectInfoWindow(core_ptr, picked_obj_num);
	else
	    NOTIFY("No object selected.");

/*
char *display_name;
display_name = strdup(XDisplayString(display->display));
fprintf(stderr, "display_name='%s'\n\n", display_name);
if(display_name != NULL)
    free(display_name);
*/

/*
Window rootWindow = display->root;
char *window_name_return = NULL;
int status = XFetchName(display->display, rootWindow, &window_name_return);
if(status != 0)
{
    fprintf(stderr, "window_name_return='%s'\n", window_name_return);
}
*/
/*
Window rootWindow = XDefaultRootWindow(display->display);
enum_windows(display->display, rootWindow, 0);
*/

    }
    /* Move closer object? */
    else if(!strcasecmp(strv[0], "mv") || !strcasecmp(strv[0], "move"))
    {
	sar_object_struct *picked_obj_ptr;

	scn_ed->in_modif_state = False;

	/* Not already in move state?
	 * User wants to pick and move closer object.
	 */
	if(scn_ed->in_move_state == False)
	{
	    /* Has user given an object number? */
	    if(strc > 1 && strv[1][0] == '#')
	    {
		if(sscanf(strv[1], "#%d", &picked_obj_num) != 1)
		    picked_obj_num = -1;
	    }
	    else
		picked_obj_num = SceneObjectPick(core_ptr, player_obj_num, False);

	    /* Is there already an edited object? */
	    if(scn_ed->cur_obj_num >= 0)
	    {
		/* Delete currently edited object */

		obj_num = scn_ed->cur_obj_num;

		/* Unlink object from player object */
		scn_ed->cur_obj_num = -1;

		SARObjDelete(
		    core_ptr,
		    &core_ptr->object,
		    &core_ptr->total_objects,
		    obj_num
		);
	    }

	    /* Get pick object pointer */
	    picked_obj_ptr = ((picked_obj_num < 0) ? NULL : (*&core_ptr->object)[picked_obj_num]);

	    /* Is picked object a smoke trail? */
	    if((smoke = SAR_OBJ_GET_SMOKE(picked_obj_ptr)) != NULL)
	    {
		;
	    }
	    /* Is picked object an explosion? */
	    else if((explosion = SAR_OBJ_GET_EXPLOSION(picked_obj_ptr)) != NULL)
	    {
		;
	    }
	    /* Is picked object a fire? */
	    else if((fire = SAR_OBJ_GET_FIRE(picked_obj_ptr)) != NULL)
	    {
		;
	    }
	    /* Is picked object a fuel tank? */
	    else if((fueltank = SAR_OBJ_GET_FUELTANK(picked_obj_ptr)) != NULL)
	    {
		;
	    }
	    /* Is picked object a helipad? */
	    else if((helipad = SAR_OBJ_GET_HELIPAD(picked_obj_ptr)) != NULL)
	    {
		// FIXME (helipad->flags & SAR_HELIPAD_FLAG_REF_OBJECT) don't work!
		if(helipad != NULL && helipad->ref_object >= 0)
		{
		    NOTIFY("Can't move a referenced object!");
		    picked_obj_ptr = NULL;
		}
	    }

	    /* Link picked object to player object */

	    if(picked_obj_ptr != NULL)
	    {
		/* Move player object to picked object position */
		player_obj_ptr->pos = picked_obj_ptr->pos;
		player_obj_ptr->dir = picked_obj_ptr->dir;

		/* Realize player new position */
		SARSimWarpObject(scene, player_obj_ptr, &player_obj_ptr->pos, &player_obj_ptr->dir);

		scn_ed->cur_obj_type = picked_obj_ptr->type;

		modification = scn_ed->modification_list[picked_obj_num];
		if(modification->obj_data_new != NULL)
		    editor_obj_data = modification->obj_data_new;
		else
		    editor_obj_data = modification->obj_data_original;

		free(scn_ed->cur_obj_arg);
		/* Generate command line string */
		scn_ed->cur_obj_arg = EditorObjectDataDoParametersLine(editor_obj_data);

		/* Link object to player object */
		scn_ed->cur_obj_num = picked_obj_num;

		scn_ed->in_move_state = True;
	    }
	}
	else
	{
	    /* Already in move state, user must "set" current object */
	}
    }
    /* Enter edit mode?
     * Note that at first run commands are not yet rerouted to editor,
     * thus strv[0] string contains "on" and not "scnedit".
     */
    else if((core_ptr->editor_mode_on != True) && !strcasecmp(strv[0], "on"))
    {
	scn_ed->in_modif_state = False;

	/* Player model not the right one? */
	if(strstr(core_ptr->cur_player_model_file, SCENEDITORAIRCRAFT) == NULL)
	{
	    NOTIFY("Scenery editor must be started with the \"Object placer\" aircraft.");
	    return;
	}
//fprintf(stderr, "%s:%d: dname.global_data='%s'\n", __FILE__, __LINE__, dname.global_data);
	/*
	 * Init modification list
	 */

	editor_modified_object_struct *modification;

	/* Allocate modification_list */
	scn_ed->modification_list = malloc(
		core_ptr->total_objects *
		sizeof(editor_modified_object_struct*)
	    );
	if(scn_ed->modification_list == NULL)
	{
	    fprintf(stderr,
		    "%s:%d: Memory allocation error.\n",
		    __FILE__,
		    __LINE__
		);
	    return;
	}

	/* Fill modification_list */
	for( i = 0; i < core_ptr->total_objects; i++)
	{
	    modification = malloc(sizeof(editor_modified_object_struct));
	    if(modification == NULL)
	    {
		fprintf(stderr,
			"%s:%d: Memory allocation error.\n",
			__FILE__,
			__LINE__
		    );
		return;
	    }
	    scn_ed->modification_list[i] = modification;

	    modification->flags = EDITOR_OBJECT_FLAG_ORIGINAL;
	    editor_obj_data = EditorObjectDataStructNew();
	    EditorObjectDataStructFill(core_ptr, editor_obj_data, i);
	    modification->obj_data_original = editor_obj_data;

	    /*  */
	    editor_obj_data = EditorObjectDataStructNew();
	    EditorObjectDataStructFill(core_ptr, editor_obj_data, i);
	    modification->obj_data_new = editor_obj_data;
	}
	scn_ed->total_original_objects = core_ptr->total_objects;
	scn_ed->total_objects = scn_ed->total_original_objects;

	/* Clean currently edited object values */
	scn_ed->cur_obj_arg = NULL;
	scn_ed->cur_obj_type = SAR_OBJ_TYPE_GARBAGE;

	/* Unlink object from player object */
	scn_ed->cur_obj_num = -1;

	/* Save flight model */
	aircraft->last_flight_model_type = aircraft->flight_model_type;

	/* Enter scenery editing mode */
	core_ptr->editor_mode_on = True;

	/* Show pointer cursor */
	GWShowCursor(display);

	NOTIFY("Editing mode is now ON");

	/* Set spot view direction and distance at startup */
	sar_direction_struct *camera_spot_dir = &scene->camera_spot_dir;
	camera_spot_dir->heading = (float)(-0.95f * PI);
	camera_spot_dir->pitch = (float)(-0.075f * PI);
	camera_spot_dir->bank = (float)(0.0f * PI);
	scene->camera_spot_dist = 25.0f;
	scene->camera_target = scene->player_obj_num;
	scene->camera_ref = SAR_CAMERA_REF_SPOT;

	SARSimSetSlew(player_obj_ptr, 1);
	aircraft->engine_state = SAR_ENGINE_ON;
    }
    /* Attempt to enter edit mode when already editing? */
    else if((core_ptr->editor_mode_on == True) &&
	    !strcasecmp(strv[0], "scnedit") &&
	    strc > 1 &&
	    !strcasecmp(strv[1], "on")
    )
    {
	scn_ed->in_modif_state = False;

	NOTIFY("Editing mode already ON");
    }
    /* Create a fire?
     * create fire <radius> <height>
     * or: lf <radius> <height>
     */
    else if(!strcasecmp(strv[0], "cfi") ||
	    !strcasecmp(strv[0], "create_fire") ||
	    (!strcasecmp(strv[0], "create") &&
	    strc > 1 &&
	    !strcasecmp(strv[1], "fire"))
    )
    {
	scn_ed->in_modif_state = False;

	type = SAR_OBJ_TYPE_FIRE;

	if(!strcasecmp(strv[0], "create"))
	{
	    SKIPARGS(cmd_args, 2);
	}
	else
	{
	    SKIPARGS(cmd_args, 1);
	}

	/* Load and create a new object */
	obj_num = ScnEditLoadObject(core_ptr, -1, type, cmd_args);

	if(obj_num >= 0)
	{
	    //EDITOROBJECTSETDATA(obj_num)

	    /* Link object to player object. */
	    scn_ed->cur_obj_num = obj_num;
	}
    }
    /* Create a helipad?
     * create helipad <style> <length> <width> [ ... ]
     * or: lhe <style> <length> <width> [ ... ]
     */
    else if(!strcasecmp(strv[0], "che") ||
	    !strcasecmp(strv[0], "create_helipad") ||
	    (!strcasecmp(strv[0], "create") &&
	    strc > 1 &&
	    !strcasecmp(strv[1], "helipad"))
    )
    {
	scn_ed->in_modif_state = False;

	type = SAR_OBJ_TYPE_HELIPAD;

	if(!strcasecmp(strv[0], "create"))
	{
	    SKIPARGS(cmd_args, 2);
	}
	else
	{
	    SKIPARGS(cmd_args, 1);
	}

	/* Load and create a new object */
	obj_num = ScnEditLoadObject(core_ptr, -1, type, cmd_args);

	if(obj_num >= 0)
	{
	    //EDITOROBJECTSETDATA(obj_num)

	    /* Link object to player object. */
	    scn_ed->cur_obj_num = obj_num;
	}
    }
    /* Create a human?
     * create human <type_name> <flag> [ ... ]
     * or: lhe <type_name> <flag> [ ... ]
     */
    else if(!strcasecmp(strv[0], "chu") ||
	    !strcasecmp(strv[0], "create_human") ||
	    (!strcasecmp(strv[0], "create") &&
	    strc > 1 &&
	    !strcasecmp(strv[1], "human"))
    )
    {
	scn_ed->in_modif_state = False;

	type = SAR_OBJ_TYPE_HUMAN;

	if(!strcasecmp(strv[0], "create"))
	{
	    SKIPARGS(cmd_args, 2);
	}
	else
	{
	    SKIPARGS(cmd_args, 1);
	}

	/* Load and create a new object */
	obj_num = ScnEditLoadObject(core_ptr, -1, type, cmd_args);

	if(obj_num >= 0)
	{
	    //EDITOROBJECTSETDATA(obj_num)

	    /* Link object to player object. */
	    scn_ed->cur_obj_num = obj_num;
	}
    }
    /* Load an object (an external model)?
     * load object <sub_dir_name/file_name.3d>
     * or: lo <sub_dir_name/file_name.3d>
     */
    else if(!strcasecmp(strv[0], "lob") ||
	    !strcasecmp(strv[0], "load_object") ||
	    (!strcasecmp(strv[0], "load") &&
	    strc > 1 &&
	    !strcasecmp(strv[1], "object"))
    )
    {
	char *sub_dir_name = NULL;

	scn_ed->in_modif_state = False;

	if(!strcasecmp(strv[0], "load") )
	{
	    SKIPARGS(cmd_args, 2);
	}
	else
	{
	    SKIPARGS(cmd_args, 1);
	}

	/* Is an object linked to the player object? */
	if(scn_ed->cur_obj_num >= 0)
	{
	    obj_num = scn_ed->cur_obj_num;

	    //COPYCUROBJDATATOPREVOBJDATA

	    /* Unlink object from player object */
	    scn_ed->cur_obj_num = -1;

	    scn_ed->cur_obj_type = SAR_OBJ_TYPE_GARBAGE;
	    free(scn_ed->cur_obj_arg);
	    scn_ed->cur_obj_arg = NULL;

	    SARObjDelete(
		core_ptr,
		&core_ptr->object,
		&core_ptr->total_objects,
		obj_num
	    );
	}

	/* Extract subdirectory name */

	sub_dir_name = STRDUP(cmd_args);

	/* Remove first slash(s) */
	while(sub_dir_name[0] == '/')
	    sub_dir_name++;

	/* Go to next slash */
	i = 0;
	while(sub_dir_name[i] != '/')
	    i++;

	/* Close sub_dir_name string */
	sub_dir_name[i] = '\0';

	/* Set object type in accordance to subdirectory name
	 * (needed for ScnEditLoadObject())
	 * FIXME is it safe? Maybe better to read object type in the *.3d file.
	 */
	if(!strcmp(sub_dir_name, SAR_DEF_OBJECTS_DIR))
	    type = SAR_OBJ_TYPE_STATIC;
	else if(!strcmp(sub_dir_name, SAR_DEF_AIRCRAFTS_DIR))
	    type = SAR_OBJ_TYPE_AIRCRAFT;
	else if(!strcmp(sub_dir_name, SAR_DEF_AUTOMOBILES_DIR))
	    type = SAR_OBJ_TYPE_AUTOMOBILE;
	else if(!strcmp(sub_dir_name, SAR_DEF_WATERCRAFTS_DIR))
	    type = SAR_OBJ_TYPE_WATERCRAFT;
	else
	{
	    int line_length = 40 + STRLEN(cmd_args);
	    char *s = (char *)malloc((line_length + 1) * sizeof(char));

	    snprintf(s, 80, "Model file must be in a %s subdirectory.", PROG_NAME);
	    NOTIFY(s);
	    free(s);

	    type = SAR_OBJ_TYPE_GARBAGE;
	}
	free(sub_dir_name);

	if(type != SAR_OBJ_TYPE_GARBAGE)
	{
	    COPYCUROBJDATATOPREVOBJDATA

	    /* Load and create a new object */
	    obj_num = ScnEditLoadObject(core_ptr, obj_num, type, cmd_args);

	    if(obj_num >= 0)
	    {
		scn_ed->cur_obj_type = type;
		free(scn_ed->cur_obj_arg);
		scn_ed->cur_obj_arg = STRDUP(cmd_args);
		//EDITOROBJECTSETDATA(obj_num)

		/* Link object to player object. */
		scn_ed->cur_obj_num = obj_num;
	    }
	    else
	    {
		/* Error message has been set by ScnEditLoadObject() */
	    }
	}
	else
	{
	    obj_num = -1;
	    NOTIFY("Model loading aborted");
	}
    }
    /* Create a premodeled object? */
    else if(!strcasecmp(strv[0], "cpr") ||
	    !strcasecmp(strv[0], "create_premodeled") ||
	    (!strcasecmp(strv[0], "create") &&
	    strc > 1 &&
	    !strcasecmp(strv[1], "premodeled"))
    )
    {
	scn_ed->in_modif_state = False;

	type = SAR_OBJ_TYPE_PREMODELED;

	if(!strcasecmp(strv[0], "create") )
	{
	    SKIPARGS(cmd_args, 2);
	}
	else
	{
	    SKIPARGS(cmd_args, 1);
	}

	/* Load object */
	obj_num = ScnEditLoadObject(core_ptr, -1, type, cmd_args);

	if(obj_num >= 0)
	{
	    //EDITOROBJECTSETDATA(obj_num)

	    /* Link object to player object. */
	    scn_ed->cur_obj_num = obj_num;
	}
    }
    /* Create a runway?
     * create runway <length> <width>
     * or: lr runway <length> <width>
     */
    else if(!strcasecmp(strv[0], "cru") ||
	    !strcasecmp(strv[0], "create_runway") ||
	    (!strcasecmp(strv[0], "create") &&
	    strc > 1 &&
	    !strcasecmp(strv[1], "runway"))
    )
    {
	scn_ed->in_modif_state = False;

	type = SAR_OBJ_TYPE_RUNWAY;

	if(!strcasecmp(strv[0], "create") )
	{
	    SKIPARGS(cmd_args, 2);
	}
	else
	{
	    SKIPARGS(cmd_args, 1);
	}

	/* Load and create a new object */
	obj_num = ScnEditLoadObject(core_ptr, -1, type, cmd_args);

	if(obj_num >= 0)
	{
	    //EDITOROBJECTSETDATA(obj_num)

	    /* Link object to player object. */
	    scn_ed->cur_obj_num = obj_num;
	}
    }
    /* Create a smoke?
     * create smoke <color> <start_radius>
     * or: ls smoke <color> <start_radius>
     */
    else if(!strcasecmp(strv[0], "csm") ||
	    !strcasecmp(strv[0], "create_smoke") ||
	    (!strcasecmp(strv[0], "create") &&
	    strc > 1 &&
	    !strcasecmp(strv[1], "smoke"))
    )
    {
	scn_ed->in_modif_state = False;

	type = SAR_OBJ_TYPE_SMOKE;

	if(!strcasecmp(strv[0], "create") )
	{
	    SKIPARGS(cmd_args, 2);
	}
	else
	{
	    SKIPARGS(cmd_args, 1);
	}

	/* Load and create a new object */
	obj_num = ScnEditLoadObject(core_ptr, -1, type, cmd_args);

	if(obj_num >= 0)
	{
	    //EDITOROBJECTSETDATA(obj_num)

	    /* Link object to player object. */
	    scn_ed->cur_obj_num = obj_num;
	}
    }
    /* Copy closer object? */
    else if(!strcasecmp(strv[0], "cp") || !strcasecmp(strv[0], "copy"))
    {
	scn_ed->in_modif_state = False;

	/* Is an object currently loaded? */
	if(scn_ed->cur_obj_num >= 0)
	{
	    /* Unload (delete) current object */

	    COPYCUROBJDATATOPREVOBJDATA

	    /* Unlink object from player object */
	    scn_ed->cur_obj_num = -1;

	    scn_ed->cur_obj_type = SAR_OBJ_TYPE_GARBAGE;
	    free(scn_ed->cur_obj_arg);
	    scn_ed->cur_obj_arg = NULL;

	    SARObjDelete(
		core_ptr,
		&core_ptr->object,
		&core_ptr->total_objects,
		scn_ed->prev_obj_num
	    );
	}

	/* Get number of closest object */
	picked_obj_num = SceneObjectPick(core_ptr, player_obj_num, False);

	if(picked_obj_num >= 0)
	{
	    int child_obj_num = -1;

	    picked_obj_ptr = core_ptr->object[picked_obj_num];
	    if(picked_obj_ptr == NULL)
		return;

	    /* Is this object named? */
	    if(picked_obj_ptr->name != NULL)
	    {
		/* Check if object to copy is referenced by another object */
		for(i = 0; i < scn_ed->total_objects; i++)
		{
		    modification = scn_ed->modification_list[i];
		    editor_obj_data = modification->obj_data_new;

		    /* Is object to copy referenced by another object? */
		    if(editor_obj_data->ref_obj_name != NULL &&
			!strcmp(editor_obj_data->ref_obj_name, picked_obj_ptr->name)
		    )
		    {
			child_obj_num = i;
		    }
		}
	    }

#define S_LENGTH 1023
	    char *s = (char *)malloc((S_LENGTH + 1) * sizeof(char));
	    /* Object don't have a child? */
	    if(child_obj_num < 0)
	    {
		type = picked_obj_ptr->type;

		modification = scn_ed->modification_list[picked_obj_num];
		if(modification->obj_data_new != NULL)
		    editor_obj_data = modification->obj_data_new;
		else
		    editor_obj_data = modification->obj_data_original;

		strncpy(s, EditorObjectDataDoParametersLine(editor_obj_data), S_LENGTH);

		/* Load a new object */
		int obj_new_num = ScnEditLoadObject(
			    core_ptr,
			    -1,
			    type,
			    s
			);

		if(obj_new_num >= 0)
		{
		    obj_ptr = ((obj_new_num < 0) ? NULL : (*&core_ptr->object)[obj_new_num]);

		    /* Copy player position to object position */
		    memcpy(&obj_ptr->pos, &player_obj_ptr->pos, sizeof(sar_position_struct));
		    memcpy(&obj_ptr->dir, &player_obj_ptr->dir, sizeof(sar_direction_struct));

		    /* Realize object position */
		    SARSimWarpObject(scene, obj_ptr, &obj_ptr->pos, &obj_ptr->dir);

		    EDITOROBJECTSETDATA(obj_new_num);

		    free(scn_ed->cur_obj_arg);
		    scn_ed->cur_obj_arg = STRDUP(s);

		    scn_ed->cur_obj_type = type;

		    /* Link object to player object. */
		    scn_ed->cur_obj_num = obj_new_num;

		    /* Move a little bit player object to show user that
		     * object has been copied.
		     */
		    player_obj_ptr->pos.x += 0.5f * cos(player_obj_ptr->dir.heading);
		    player_obj_ptr->pos.y += 0.5f * sin(player_obj_ptr->dir.heading);

		    /* Realize new position */
		    SARSimWarpObject(
			scene,
			player_obj_ptr,
			&player_obj_ptr->pos,
			&player_obj_ptr->dir
		    );
		}

snprintf(s, S_LENGTH, "Object #%d has been copied.", picked_obj_num);
	    }
	    else
	    {
snprintf(s, S_LENGTH, "Can't copy object #%d because it has a child.",
			 picked_obj_num
		    );
	    }
	    NOTIFY(s);
	    free(s);
	}
#undef S_LENGTH
    }
/*
    // Enable or disable GTK User Interface? //
    else if(!strcasecmp(strv[0], "gtk")
    {
	scn_ed->in_modif_state = False;

	if(strc > 1 && !strcasecmp(strv[1], "on"))
	{
	    //gtkStart(core_ptr);

	    if(gtkAppStart(core_ptr) == 0)
	    {
		scn_ed->gtk_mode_on = True;
		NOTIFY("GTK user interface is now ON");
	    }
	    else
		NOTIFY("Can't initialize GTK user interface");
	}
	else
	{
	    if(scn_ed->gtk_mode_on == True)
		NOTIFY("GTK user interface is now OFF");

	    scn_ed->gtk_mode_on = False;
	}
    }
*/
    /* Print modifications to console? */
    else if(!strcasecmp(strv[0], "print"))
    {
#define FILEEXTENSION "_with_obj_num.txt"
	char *s = (char *)malloc((80 + STRLEN(arg)) * sizeof(char));

	scn_ed->in_modif_state = False;

	/* Modifications printed to console? */
	if(ScnEditPrintModificationsList(scn_ed, stdout) > 0)
	{
	    struct stat stat_buf;
	    Boolean no_file, file_ok = False;
	    FILE *fp;

	    NOTIFY("Scenery modifications list printed to console.");

	    /* Copy the scenery file and add the E_OBJ#obj_num tags */

	    char *scn_file_name = strrchr(core_ptr->cur_scene_file, '/');

	    char *editor_scn_filename = (char *)malloc(
				(	strlen(getenv("HOME"))
				    + 1
				    + strlen(scn_file_name)
				    + strlen(FILEEXTENSION)
				) * sizeof(char) + 1);
	    sprintf(editor_scn_filename, "%s%s%s",
		    getenv("HOME"),
		    scn_file_name,
		    FILEEXTENSION
	    );

	    /* Check if file with object numbers exists. */
	    if(stat(editor_scn_filename, &stat_buf))
	    {
		no_file = True;
	    }
	    /* File already exists, try to remove it */
	    else
	    {
		/* File can't be removed? */
		if(remove((const char *)editor_scn_filename) !=0)
		{
		    sprintf(
			s,
			"ERROR: Can't delete file '%s'.",
			editor_scn_filename
		    );
		    NOTIFY(s);
		    no_file = False;
		}
		else
		    no_file = True;
	    }

	    if(no_file)
	    {
		/* Copy file */
		fp = FCopy((const char *)core_ptr->cur_scene_file,
			    (const char *)editor_scn_filename
			);

		/* File copied? */
		if(fp != NULL)
		{
		    /* Change output file access mode to read/write */
		    fp = freopen(editor_scn_filename, "r+", fp );

		    /* No error while adding object number tags in file? */
		    if(fp != NULL && AddObjNumToSceneryFile(fp) == 0)
			file_ok = True;
		    else
		    {
			fclose(fp);
			file_ok = False;
		    }
		}
		else
		    file_ok = False;
	    }

	    if(file_ok)
	    {
		fclose(fp);
		free(scn_ed->scn_file_name);
		scn_ed->scn_file_name = STRDUP(editor_scn_filename);
	    }
	    else
	    {
		free(scn_ed->scn_file_name);
		scn_ed->scn_file_name = NULL;

		sprintf(
		    s,
		    "ERROR: Can't create file '%s'.",
		    editor_scn_filename
		);
		NOTIFY(s);
	    }

	    free(editor_scn_filename);
	}
	else
	{
	    NOTIFY("No scenery modification to print.");
	}

	free(s);

#undef FILEEXTENSION
    }
    /* Modify the closest object parameters? */
    else if(!strcasecmp(strv[0], "mod") || !strcasecmp(strv[0], "modify"))
    {
#define MAXVALUESTRCHARNUM 64
	char *obj_type_name;
	text_input_struct *p;
	char *buf;
	char value_str[MAXVALUESTRCHARNUM + 1];

	scn_ed->in_modif_state = False;

	if(strc == 1)
	{
	    picked_obj_num = SceneObjectPick(core_ptr, player_obj_num, False);
	}
	/* User has given an object number */
	else if(strc == 2)
	{
	    char *endPtr;
	    long value = strtol((const char *)strv[1], &endPtr, 10);

	    /* 'string' to 'long' conversion fails or decimal number given? */
	    if(endPtr == strv[1] || (endPtr != strv[1] + strlen(strv[1])))
	    {
		picked_obj_num = -1;
	    }
	    else
	    {
		/* Object number within limits? */
		if(value >= 0 && value < core_ptr->total_objects)
		    picked_obj_num = (int)value;
		else
		    picked_obj_num = -1;
	    }

	    if(picked_obj_num == -1)
		NOTIFY("Bad object number");
	}
	scn_ed->mod_obj_num = picked_obj_num;

	/* Not already in a modification sequence? */
	if(scn_ed->in_modif_state == False)
	{
	    char *s = NULL;

	    if(picked_obj_num < 0)
		return;

	    COPYCUROBJDATATOPREVOBJDATA

	    obj_num = picked_obj_num;

	    modification = scn_ed->modification_list[obj_num];
	    editor_obj_data = modification->obj_data_new;

	    if(editor_obj_data->type == SAR_OBJ_TYPE_GROUND)
	    {
		NOTIFY("Sorry, ground object modification is not possible.");
		scn_ed->mod_obj_num = -1;
		return;
	    }

	    /* Generate command line string */
	    s = EditorObjectDataDoParametersLine(editor_obj_data);

	    obj_ptr = core_ptr->object[obj_num];

	    if(s != NULL && obj_ptr != NULL)
	    {
		/* Save picked object position */
		memcpy(&scn_ed->cur_obj_pos, &obj_ptr->pos, sizeof(sar_position_struct));
		memcpy(&scn_ed->cur_obj_dir, &obj_ptr->dir, sizeof(sar_direction_struct));

		/* Get object type name */
		obj_type_name = STRDUP(SceneObjectGetTypeName(core_ptr, obj_num));

		/*
		 * Show command line
		 */

		/* Enter command mode */
		SARKeyCommand(core_ptr, display, scene, True);

		/* Prepare command prompt */
		snprintf(value_str, MAXVALUESTRCHARNUM, "#%05d %s",
			    scn_ed->mod_obj_num,
			    obj_type_name
			);
		free(obj_type_name);
		obj_type_name = NULL;

		/* Set command prompt */
		SARTextInputMap(
		    core_ptr->text_input,
		    value_str, NULL,
		    SARCmdTextInputCB,
		    core_ptr
		);

		/* Copy command line string to command text buffer */
		p = core_ptr->text_input;
		p->len = strlen(s);
		p->buf = buf = STRDUP(s);
		if(buf == NULL)
		{
		    p->len = p->pos = 0;
		    free(s);
		    return;
		}

		/* Turn on keyboard autorepeat
		 * FIXME : can cause some trouble in Ubuntu?
		 * 	see Jesse's note in gwx.c / GWKeyboardAutoRepeat()
		 */
		GWKeyboardAutoRepeat(display, True);

		scn_ed->in_modif_state = True;
	    }
	    else
		NOTIFY("Can't get object data.");

	    free(s);
	}

#undef MAXVALUESTRCHARNUM
    }
    /* Object parameters modification validated?
     *
     * Note : this test must be the last one in the 'else if' commands name
     *		tests because if not it will cause some trouble if user pressed
     * 		the <Esc> key instead of <Enter> key while the parameters are
     * 		printed on screen.
     */
    else if(scn_ed->in_modif_state == True)
    {
	char *s = (char *)malloc((NOTIFYSTRINGLENGTH + 1) * sizeof(char));
	obj_num = scn_ed->mod_obj_num;
	s[0] = '\0';

	/* Turn off keyboard autorepeat
	 * FIXME : see Jesse's note in gwx.c / GWKeyboardAutoRepeat()
	 */
	GWKeyboardAutoRepeat(display, False);

	strncat(s, strv[0], NOTIFYSTRINGLENGTH);
	for(i = 1; i < strc; i++)
	{
	    strncat(s, " ", NOTIFYSTRINGLENGTH);
	    strncat(s, strv[i], NOTIFYSTRINGLENGTH);
	}

	/* Get object pointer */
	obj_ptr = core_ptr->object[obj_num];

	/* Delete current object then reload it (easier and faster than
	 * check then update each modified parameter).
	 */
	int obj_new_num = ScnEditLoadObject(core_ptr, obj_num, obj_ptr->type, cmd_args);
	//EDITOROBJECTSETDELETED(obj_num)

	if(obj_new_num >= 0)
	{
	    /* Get new object pointer */
	    obj_ptr = core_ptr->object[obj_new_num];

	    /* Set new object position */
	    memcpy(&obj_ptr->pos, &scn_ed->cur_obj_pos, sizeof(sar_position_struct));
	    memcpy(&obj_ptr->dir, &scn_ed->cur_obj_dir, sizeof(sar_direction_struct));

	    /* Realize new object position */
	    SARSimWarpObject(scene, obj_ptr, &obj_ptr->pos, &obj_ptr->dir);

	    if(obj_ptr->type == SAR_OBJ_TYPE_HELIPAD)
	    {
		helipad = SAR_OBJ_GET_HELIPAD(obj_ptr);

		/* Has helipad a reference object? */
		if(helipad != NULL && helipad->ref_object >= 0)
		{
		    /* Realize new object relative position */
		    SARSimWarpObjectRelative(
			    scene, obj_ptr,
			    core_ptr->object, core_ptr->total_objects,
			    helipad->ref_object,
			    &helipad->ref_offset,
			    &helipad->ref_dir
			);
		}
	    }

	    EDITOROBJECTSETDATA(obj_new_num)

	    /* Object number modified? */
	    if(obj_new_num != obj_num)
	    {
		snprintf(s, NOTIFYSTRINGLENGTH,
"Object #%05d successfully modified and renumbered as #%05d",
			obj_num, obj_new_num
			);

		EDITOROBJECTSETMODIFIED(obj_new_num);
	    }
	    else
	    {
		char *old_parm, *new_parm;
		modification = scn_ed->modification_list[obj_num];

		editor_obj_data = modification->obj_data_original;
		old_parm = STRDUP(EditorObjectDataDoParametersLine(editor_obj_data));

		editor_obj_data = modification->obj_data_new;
		new_parm = STRDUP(EditorObjectDataDoParametersLine(editor_obj_data));

		/* Parameters modified? */
		if(old_parm != NULL && new_parm != NULL &&
		    strcmp(old_parm, new_parm)
		)
		{
		    snprintf(s, NOTIFYSTRINGLENGTH,
			    "Object #%05d successfully modified",
			    obj_num
			    );

		    EDITOROBJECTSETMODIFIED(obj_num);
		}
		else
		{
		    snprintf(s, NOTIFYSTRINGLENGTH,
			    "Object #%05d modification aborted",
			    obj_num
			    );
		}
	    }
	    NOTIFY(s);
	    free(s);
	}

	obj_num = obj_new_num;

	scn_ed->mod_obj_num = -1;
	scn_ed->in_modif_state = False;

    }
    /* Command name error */
    else
    {
	char *s = (char *)malloc((NOTIFYSTRINGLENGTH + 1) * sizeof(char));
	snprintf(s, NOTIFYSTRINGLENGTH, "%s: Invalid editor command.", arg);
	NOTIFY(s);
	free(s);
    }

    if(scn_ed->cur_obj_num >= 0)
    {
	if(scn_ed->cur_obj_type == SAR_OBJ_TYPE_PREMODELED)
	{
	    //obj_num = scn_ed->cur_obj_num;

	    obj_ptr = ((obj_num < 0) ? NULL : (*&core_ptr->object)[obj_num]);
	    if(obj_ptr != NULL)
	    {
		/* Is current object a PTT? */
		if(!strcasecmp(
			SceneObjectGetPremodeledTypeName(core_ptr, obj_num),
			SAR_PREMODELED_POWER_TRANSMISSION_TOWER_S
		    )
		)
		{
/*
		    // Refresh electrical conductors data //
		    SARScenePostLoading(core_ptr, scene);
*/
		}
	    }
	}
    }

    strlistfree(strv, strc);

#undef EDITOROBJECTSETDELETED
#undef EDITOROBJECTSETMODIFIED
#undef EDITOROBJECTSETDATA
}
#undef TAGVALLENGTH
#undef TAGSTRING
