#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "../include/string.h"
#include "../include/disk.h"

#include "gw.h"
#include "textinput.h"
#include "menu.h"
#include "obj.h"
#include "mission.h"
#include "simutils.h"
#include "playerstatio.h"
#include "sar.h"
#include "sarcamp.h"
#include "sarmenuop.h"
#include "sarmenucb.h"
#include "sarmenuoptions.h"
#include "missionio.h"
#include "sarsimbegin.h"
#include "sarmenucodes.h"
#include "config.h"


/* Menu Procedures */
static void SARMenuCBBeginMission(sar_core_struct *core_ptr, int campaign_mode);
static void SARMenuCBViewLastLog(sar_core_struct *core_ptr);
static void SARMenuCBMissionMap(sar_core_struct *core_ptr, int campaign_mode);
static void SARMenuCBMissionBrief(sar_core_struct *core_ptr, int campaign_mode);
static void SARMenuCBBeginFreeFlight(sar_core_struct *core_ptr);
static void SARMenuCBFreeFlightAircraftInfo(sar_core_struct *core_ptr);
static void SARMenuCBMapOp(sar_core_struct *core_ptr, int id_code);
static void SARMenuCBPlayerEditCB(const char *v, void *data);
static void SARMenuCBPlayerOp(sar_core_struct *core_ptr, int id_code);

/* Menu Callbacks */
void SARMenuButtonCB(void *object, int id_code, void *client_data);
void SARMenuListSelectCB(
	void *object, int id_code, void *client_data,
	int item_num, void *item_ptr, void *item_data
);
void SARMenuListActivateCB(
	void *object, int id_code, void *client_data,
	int item_num, void *item_ptr, void *item_data
);
void SARMenuSwitchCB(
	void *object, int id_code, void *client_data,
	Boolean state
);
void SARMenuSpinCB(
	void *object, int id_code, void *client_data,
	char *value
);


#define MAX(a,b)        (((a) > (b)) ? (a) : (b))
#define MIN(a,b)        (((a) < (b)) ? (a) : (b))
#define CLIP(a,l,h)     (MIN(MAX((a),(l)),(h)))
#define STRDUP(s)	(((s) != NULL) ? strdup(s) : NULL)

#define STRLEN(s)       (((s) != NULL) ? ((int)strlen(s)) : 0)
#define ISSTREMPTY(s)	(((s) != NULL) ? ((s) == '\0') : True)


/*
 *      Default spacings (in pixels):
 *      
 *      These values should match and coenciding values in menu.c. 
 */
#define DEF_XMARGIN             5
#define DEF_YMARGIN             5

#define DEF_WIDTH               100
#define DEF_HEIGHT              100


/*
 *	Begin Mission.
 */
static void SARMenuCBBeginMission(sar_core_struct *core_ptr, int campaign_mode)
{
	const sar_menu_list_item_struct *item;
	char *filename;
	sar_menu_list_item_data_struct *d;
	sar_menu_struct *m = SARMatchMenuByNamePtr(
	    core_ptr, SAR_MENU_NAME_MISSION
	);
	if(m == NULL)
	    return;

	/* Get the selected mission on the Missions List */
	item = SARMenuListGetSelectedItem(m, SAR_MENU_ID_MISSION_LIST);
	d = SAR_MENU_LIST_ITEM_DATA(
	    (item != NULL) ? item->client_data : NULL
	);
        if (campaign_mode)
           filename = Get_Campaign_Filename();
        else
	   filename = (d != NULL) ? d->filename : NULL;

	/* Update the current mission file */
	free(core_ptr->cur_mission_file);
	core_ptr->cur_mission_file = STRDUP(filename);

        /* clean up */
        if ( (campaign_mode) && (filename) )
           free(filename);

	/* Switch to mission simulation and leave menu system */
	SARSimBeginMission(
	    core_ptr,
	    core_ptr->cur_mission_file
	);
}

/*
 *	Switch to last log map procedure.
 */
static void SARMenuCBViewLastLog(sar_core_struct *core_ptr)
{
	sar_menu_struct *m = SARMatchMenuByNamePtr(
	    core_ptr, SAR_MENU_NAME_MISSION_LOG_MAP
	);

	/* Update the map object's map and markings with the information
	 * on the mission log file
	 */
	SARMissionLoadMissionLogToMenuMap(
	    core_ptr, m,
	    fname.mission_log
	);

	/* Switch to log map menu */
	SARMenuSwitchToMenu(core_ptr, SAR_MENU_NAME_MISSION_LOG_MAP);
}

/*
 *	Switch to mission map procedure.
 */
static void SARMenuCBMissionMap(sar_core_struct *core_ptr, int campaign_mode)
{
        if (campaign_mode)
        {
            printf("Mission map is disabled for campaign mode.\n");
            return;
        }
	sar_menu_struct *m = SARMatchMenuByNamePtr(
	    core_ptr, SAR_MENU_NAME_MISSION
	);
	const char *filename;
	const sar_menu_list_item_struct *item;
	const sar_menu_list_item_data_struct *d;

	/* Get selected mission on missions list */
	item = SARMenuListGetSelectedItem(m, SAR_MENU_ID_MISSION_LIST);
	d = SAR_MENU_LIST_ITEM_DATA(
	    (item != NULL) ? item->client_data : NULL
	);
	filename = (d != NULL) ? d->filename : NULL;

	/* Get pointer to mission map menu */
	m = SARMatchMenuByNamePtr(core_ptr, SAR_MENU_NAME_MISSION_MAP);

	/* Load scene's map and markings to the map object from the given
	 * scene file
	 */
	SARMissionLoadSceneToMenuMap(
	    core_ptr, m, filename
	);

	/* Switch to mission map menu */
	SARMenuSwitchToMenu(core_ptr, SAR_MENU_NAME_MISSION_MAP);
}

/*
 *	Switch to mission briefing procedure.
 */
static void SARMenuCBMissionBrief(sar_core_struct *core_ptr, int campaign_mode)
{
	sar_menu_struct *m = SARMatchMenuByNamePtr(
	    core_ptr, SAR_MENU_NAME_MISSION
	);
	int i;
	char *filename;
	char *mission_desc;
	const sar_menu_list_item_struct *item;
	const sar_menu_list_item_data_struct *d;

	/* Get selected mission on missions list */
	item = SARMenuListGetSelectedItem(m, SAR_MENU_ID_MISSION_LIST);
	d = SAR_MENU_LIST_ITEM_DATA(
	    (item != NULL) ? item->client_data : NULL
	);

        if (campaign_mode)
           filename = Get_Campaign_Filename();
        else
	   filename = (d != NULL) ? d->filename : NULL;


	/* Get pointer to mission brief menu */
	m = SARMatchMenuByNamePtr(core_ptr, SAR_MENU_NAME_MISSION_BRIEF);


	/* Get mission description (briefing) */
	mission_desc = SARMissionLoadDescription(filename);
        if ( (campaign_mode) && (filename) )
           free(filename);

	if((SARMenuGetObjectByID(m, SAR_MENU_ID_MISSION_BRIEF_MESG, &i) != NULL) &&
	   (mission_desc != NULL)
	)
	    SARMenuMessageBoxSet(
		core_ptr->display, m, i, mission_desc, False
	    );
	free(mission_desc);

	/* Switch to mission brief menu */
	SARMenuSwitchToMenu(core_ptr, SAR_MENU_NAME_MISSION_BRIEF);
}

/*
 *	Begin free flight procedure.
 */
static void SARMenuCBBeginFreeFlight(sar_core_struct *core_ptr)
{
	const char *filename;
	sar_menu_struct *m;
	sar_menu_list_item_struct *item;
	sar_menu_list_item_data_struct *d;
	int spin_num, spin_sel_item;
	sar_menu_switch_struct *sw;
	sar_menu_spin_struct *spin_ptr;
	sar_position_struct *start_pos = NULL;
	sar_direction_struct *start_dir = NULL;
	const char *weather_preset_name = NULL;
	Boolean free_flight_system_time = False;


	if(core_ptr == NULL)
	    return;

	/* Begin collecting information from menu objects in order to
	 * pass to the begin free flight simulation function.
	 */

	/* Get pointer to menu Free Flight: Select Scenery */
	m = SARMatchMenuByNamePtr(core_ptr, SAR_MENU_NAME_FREE_FLIGHT);

	/* Get selected scenery from scenery list */
	item = SARMenuListGetSelectedItem(m, SAR_MENU_ID_FREE_FLIGHT_SCENERY_LIST);
	d = SAR_MENU_LIST_ITEM_DATA(
	    (item != NULL) ? item->client_data : NULL
	);
 	filename = (d != NULL) ? d->filename : NULL;
	free(core_ptr->cur_scene_file);
	core_ptr->cur_scene_file = STRDUP(filename);

	/* Get starting location from locations list */
	item = SARMenuListGetSelectedItem(m, SAR_MENU_ID_FREE_FLIGHT_LOCATIONS_LIST);
	d = SAR_MENU_LIST_ITEM_DATA(
	    (item != NULL) ? item->client_data : NULL
	);
 	if(d != NULL)
	{
	    start_pos = &d->pos;
	    start_dir = &d->dir;
	}


	/* Get pointer to menu Free Flight: Select Aircraft */
	m = SARMatchMenuByNamePtr(core_ptr, SAR_MENU_NAME_FREE_FLIGHT_AIRCRAFT);

	/* Get selected aircraft from aircrafts list */
	item = SARMenuListGetSelectedItem(m, SAR_MENU_ID_FREE_FLIGHT_AIRCRAFTS_LIST);
	d = SAR_MENU_LIST_ITEM_DATA(
	    (item != NULL) ? item->client_data : NULL
	);
	filename = (d != NULL) ? d->filename : NULL;
	free(core_ptr->cur_player_model_file);
	core_ptr->cur_player_model_file = STRDUP(filename);


	/* Get pointer to menu Free Flight: Weather */
	m = SARMatchMenuByNamePtr(core_ptr, SAR_MENU_NAME_FREE_FLIGHT_WEATHER);

	/* Weather preset spin */
	spin_ptr = SAR_MENU_SPIN(SARMenuGetObjectByID(
	    m, SAR_MENU_ID_MENU_FREE_FLIGHT_WEATHER_CONDITION,
	    &spin_num
	));
	weather_preset_name = SARMenuSpinGetCurrentValue(
	    m, spin_num, &spin_sel_item
	);

	/* Free flight system time switch */
	sw = SAR_MENU_SWITCH(SARMenuGetObjectByID(
	    m, SAR_MENU_ID_MENU_FREE_FLIGHT_SYSTEM_TIME,
	    NULL
	));
	free_flight_system_time = (sw != NULL) ? sw->state : False;


	/* Switch to free flight simulation and leave menu system */
	SARSimBeginFreeFlight(
	    core_ptr,
	    core_ptr->cur_scene_file,
	    core_ptr->cur_player_model_file,
	    start_pos, start_dir,
	    weather_preset_name, free_flight_system_time
	);
}

/*
 *	Switch to aircraft information menu procedure.
 */
static void SARMenuCBFreeFlightAircraftInfo(sar_core_struct *core_ptr)
{
	int ov_num;
	const char *filename;
	sar_menu_struct *m;
	const sar_menu_list_item_struct *item;
	const sar_menu_list_item_data_struct *d;
	sar_menu_objview_struct *ov_ptr;


	if(core_ptr == NULL)
	    return;

	/* Get pointer to menu Free Flight: Select Aircraft */
	m = SARMatchMenuByNamePtr(core_ptr, SAR_MENU_NAME_FREE_FLIGHT_AIRCRAFT);

	/* Get selected aircraft from aircrafts list */
	item = SARMenuListGetSelectedItem(m, SAR_MENU_ID_FREE_FLIGHT_AIRCRAFTS_LIST);
	d = SAR_MENU_LIST_ITEM_DATA(
	    (item != NULL) ? item->client_data : NULL
	);
	filename = (d != NULL) ? d->filename : NULL;


	/* Get pointer to menu Free Flight: Aircraft information */
	m = SARMatchMenuByNamePtr(core_ptr, SAR_MENU_NAME_FREE_FLIGHT_AIRCRAFT_INFO);

	/* Get pointer to aircraft information 3d object viewer object
	 * on the menu.
	 */
	ov_num = -1;
	ov_ptr = NULL;
	if(m != NULL)
	{
	    ov_num = 0;

	    if((ov_num >= 0) && (ov_num < m->total_objects))
	    {
		void *p = m->object[ov_num];
		if((p != NULL) ?
		    (*(int *)p == SAR_MENU_OBJECT_TYPE_OBJVIEW) : False
		)
		    ov_ptr = SAR_MENU_OBJVIEW(p);
	    }
	}
	if(ov_ptr != NULL)
	{
	    /* Mark input as busy while loading new aircraft information,
	     * note that the input will be set busy again when we
	     * switch menus but that is okay.
	     */
	    GWSetInputBusy(core_ptr->display);

	    /* Set up aircraft information 3d object view to display
	     * new aircraft.
	     */
	    SARMenuObjViewLoad(
		core_ptr->display, m,
		ov_ptr, ov_num, filename
	    );
	    SARMenuObjViewRebufferImage(
		core_ptr->display, m,
		ov_ptr, ov_num
	    );
	}

	/* Switch to aircraft information menu */
	SARMenuSwitchToMenu(core_ptr, SAR_MENU_NAME_FREE_FLIGHT_AIRCRAFT_INFO);
}


/*
 *	Map menu button callback.
 */
static void SARMenuCBMapOp(sar_core_struct *core_ptr, int id_code)
{
	gw_display_struct *display = core_ptr->display;
	int i, ptype, map_num = -1;
	int width, height, w, h;
	void *p;
	sar_menu_struct *m = SARGetCurrentMenuPtr(core_ptr);
	sar_menu_map_struct *map_ptr = NULL;

	float mtop_coeff;
	int	scroll_inc = 20;	/* Scroll increment in pixels */
	int	tex_w, tex_h,	/* All these units in pixels */
		scroll_x_min, scroll_x_max,
		scroll_y_min, scroll_y_max;

	if(m == NULL)
	    return;

	/* Get first map object on the menu */
	for(i = 0; i < m->total_objects; i++)
	{
	    p = m->object[i];
	    if(p == NULL)
		continue;
	    else
		ptype = (*(int *)p);

	    if(ptype == SAR_MENU_OBJECT_TYPE_MAP)
	    {
		map_num = i;
		map_ptr = (sar_menu_map_struct *)p;
		break;
	    }
	}
	if(map_ptr == NULL)
	    return;

/* Calculates scroll bounds by first fetching latest mtop_coeff
 * value and updates variables tex_w, tex_h, scroll_x_min, scroll_y_min,
 * scroll_x_max, and scroll_y_max.
 */
#define GET_SCROLL_BOUNDS	\
{ \
 mtop_coeff = map_ptr->m_to_pixels_coeff; \
 \
 /* Get texture size in pixels */ \
 tex_w = (int)(map_ptr->bg_tex_width * mtop_coeff); \
 tex_h = (int)(map_ptr->bg_tex_height * mtop_coeff); \
 \
 /* Calculate scroll bounds in pixels */ \
 scroll_x_min = (w / 2) - (tex_w / 2); \
 scroll_x_max = (tex_w / 2) - (w / 2); \
 \
 scroll_y_min = (h / 2) - (tex_h / 2); \
 scroll_y_max = (tex_h / 2) - (h / 2); \
}

	/* Get bounds of display */
	GWContextGet(
	    display, GWContextCurrent(display),
	    NULL, NULL,
	    NULL, NULL,
	    &width, &height
	);

	/* Get bounds of map object */
	w = (int)(map_ptr->width * width);
	if(w <= 0)
	    w = (int)(width - (2 * DEF_XMARGIN));
	if(w <= 0)
	    w = DEF_XMARGIN;

	if(map_ptr->height > 0)
	    h = (int)(map_ptr->height * height);
	else
	    h = DEF_HEIGHT;
	if(h <= 0)
	    h = DEF_YMARGIN;


	/* Calculate scroll bounds */
	GET_SCROLL_BOUNDS

	/* Handle by id code */
	switch(id_code)
	{
	  case SAR_MENU_ID_MISSION_MAP_LEFT: case SAR_MENU_ID_MISSION_LOG_MAP_LEFT:
/*
	    map_ptr->scroll_x = CLIP(
		map_ptr->scroll_x + scroll_inc,
		scroll_x_min, scroll_x_max
	    );
 */
	    map_ptr->scroll_x = map_ptr->scroll_x + scroll_inc;
	    break;

	  case SAR_MENU_ID_MISSION_MAP_RIGHT: case SAR_MENU_ID_MISSION_LOG_MAP_RIGHT:
/*
	    map_ptr->scroll_x = CLIP(
		map_ptr->scroll_x - scroll_inc,
		scroll_x_min, scroll_x_max
	    );
 */
	    map_ptr->scroll_x = map_ptr->scroll_x - scroll_inc;
	    break;

	  case SAR_MENU_ID_MISSION_MAP_UP: case SAR_MENU_ID_MISSION_LOG_MAP_UP:
/*
	    map_ptr->scroll_y = CLIP(
		map_ptr->scroll_y - scroll_inc,
		scroll_y_min, scroll_y_max
	    );
 */
	    map_ptr->scroll_y = map_ptr->scroll_y - scroll_inc;
	    break;

	  case SAR_MENU_ID_MISSION_MAP_DOWN: case SAR_MENU_ID_MISSION_LOG_MAP_DOWN:
/*
	    map_ptr->scroll_y = CLIP(
		map_ptr->scroll_y + scroll_inc,
		scroll_y_min, scroll_y_max
	    );
 */
	    map_ptr->scroll_y = map_ptr->scroll_y + scroll_inc;
	    break;

	  case SAR_MENU_ID_MISSION_MAP_ZOOM_IN: case SAR_MENU_ID_MISSION_LOG_MAP_ZOOM_IN:
	    /* This portion of code came from menumap.c's drag zoom
	     * event handling.
	     */
	    {
		    float scroll_x_coeff, scroll_y_coeff;
		    float prev_m_to_pixels_coeff =
			map_ptr->m_to_pixels_coeff;

		    /* Calculate new zoom */
		    map_ptr->m_to_pixels_coeff = (float)CLIP(
			map_ptr->m_to_pixels_coeff +
			    (scroll_inc *
			    map_ptr->m_to_pixels_coeff / 100),
			0.002,          /* Zoomed out max */
			0.050           /* Zoomed in max */
		    );

		    /* Need to calculate last scroll position coeff
		     * relative to the previous zoom dimension, then
		     * set the new scroll position by multiplying
		     * the scroll position coeff just calculated by
		     * the new zoom dimension.
		     */
		    if(map_ptr->bg_tex_width > 0)
			scroll_x_coeff = (float)map_ptr->scroll_x /
			    (map_ptr->bg_tex_width *
				prev_m_to_pixels_coeff);
		    else
			scroll_x_coeff = 0.0;
		    map_ptr->scroll_x = (int)(map_ptr->bg_tex_width *
			map_ptr->m_to_pixels_coeff * scroll_x_coeff);

		    if(map_ptr->bg_tex_height > 0)
			scroll_y_coeff = -(float)map_ptr->scroll_y /
			    (map_ptr->bg_tex_height *
				prev_m_to_pixels_coeff);
		    else
			scroll_y_coeff = 0.0;

		    map_ptr->scroll_y = -(int)(map_ptr->bg_tex_height *
			map_ptr->m_to_pixels_coeff * scroll_y_coeff);

		    /* Need to calculate scroll bounds again */
		    GET_SCROLL_BOUNDS

		    /* Reclip scroll bounds */
/* Its safe to scroll anywhere, plus we can see offmap markings.
		    map_ptr->scroll_x = CLIP(
			map_ptr->scroll_x,
			scroll_x_min, scroll_x_max
		    );
		    map_ptr->scroll_y = CLIP(
			map_ptr->scroll_y,
			scroll_y_min, scroll_y_max
		    );
 */
	    }
	    break;

	  case SAR_MENU_ID_MISSION_MAP_ZOOM_OUT: case SAR_MENU_ID_MISSION_LOG_MAP_ZOOM_OUT:
	    /* This portion of code came from menumap.c's drag zoom  
	     * event handling.
	     */
	    {
		    float scroll_x_coeff, scroll_y_coeff;
		    float prev_m_to_pixels_coeff =
			map_ptr->m_to_pixels_coeff;

		    /* Calculate new zoom */
		    map_ptr->m_to_pixels_coeff = (float)CLIP(
			map_ptr->m_to_pixels_coeff -
			    (scroll_inc *
			    map_ptr->m_to_pixels_coeff / 100),
			0.002,          /* Zoomed out max */
			0.050           /* Zoomed in max */
		    );

		    /* Need to calculate last scroll position coeff
		     * relative to the previous zoom dimension, then
		     * set the new scroll position by multiplying
		     * the scroll position coeff just calculated by
		     * the new zoom dimension.
		     */
		    if(map_ptr->bg_tex_width > 0)
			scroll_x_coeff = (float)map_ptr->scroll_x /
			    (map_ptr->bg_tex_width *
				prev_m_to_pixels_coeff);
		    else
			scroll_x_coeff = 0.0;
		    map_ptr->scroll_x = (int)(map_ptr->bg_tex_width *
			map_ptr->m_to_pixels_coeff * scroll_x_coeff);

		    if(map_ptr->bg_tex_height > 0)
			scroll_y_coeff = -(float)map_ptr->scroll_y /
			    (map_ptr->bg_tex_height * 
				prev_m_to_pixels_coeff);
		    else
			scroll_y_coeff = 0.0;

		    map_ptr->scroll_y = -(int)(map_ptr->bg_tex_height *
			map_ptr->m_to_pixels_coeff * scroll_y_coeff);

		    /* Need to calculate scroll bounds again */   
		    GET_SCROLL_BOUNDS

		    /* Reclip scroll bounds */
/* Its safe to scroll anywhere, plus we can see offmap markings.
		    map_ptr->scroll_x = CLIP(
			map_ptr->scroll_x,
			scroll_x_min, scroll_x_max
		    );
		    map_ptr->scroll_y = CLIP(
			map_ptr->scroll_y,
			scroll_y_min, scroll_y_max
		    );
 */
	    }
	    break;

	  case SAR_MENU_ID_MISSION_LOG_MAP_EVENT_NEXT:
	    SARMenuMapMarkingSelectNext(map_ptr, True);
	    break;

	  case SAR_MENU_ID_MISSION_LOG_MAP_EVENT_PREV:
	    SARMenuMapMarkingSelectPrev(map_ptr, True);
	    break;
	}

	/* Redraw map object */
	SARMenuDrawObject(
	    display, m, map_num
	);
	GWSwapBuffer(display);

#undef GET_SCROLL_BOUNDS
}


/*
 *	Player edit text input prompt callback.
 */
static void SARMenuCBPlayerEditCB(const char *v, void *data)
{
	sar_core_struct *core_ptr = SAR_CORE(data);
	gw_display_struct *display = core_ptr->display;
	sar_menu_struct *m = SARGetCurrentMenuPtr(core_ptr);
	if(m == NULL)
	    return;

	/* Got name? */
	if(!ISSTREMPTY(v))
	{
	    const char *name = v;
	    int list_num;
	    sar_menu_list_struct *list = SAR_MENU_LIST(SARMenuGetObjectByID(
		m, SAR_MENU_ID_PLAYER_LIST, &list_num
	    ));
	    if(list != NULL)
	    {
		int i;
		sar_player_stat_struct *pstat;

		/* Check if name already exists */
		for(i = 0; i < core_ptr->total_player_stats; i++)
		{
		    pstat = core_ptr->player_stat[i];
		    if(pstat == NULL)
			continue;

		    if(pstat->name == NULL)
			continue;

		    if(i == list->selected_item)
			continue;

		    if(!strcmp(pstat->name, name))
			break;
		}
		/* No conflicting name? */
		if(i >= core_ptr->total_player_stats)
		{
		    /* Get selected */
		    pstat = SARGetPlayerStatPtr(
			core_ptr, list->selected_item
		    );
		    if(pstat != NULL)
		    {
			free(pstat->name);
			pstat->name = STRDUP(name);
		    }

		    /* Resort, first delete list contents then resort
		     * and readd the list
		     */
		    SARMenuListDeleteAllItems(m, list_num);
		    SARPlayerStatResort(
			core_ptr->player_stat,
			core_ptr->total_player_stats
		    );
		    for(i = 0; i < core_ptr->total_player_stats; i++)
		    {
			pstat = core_ptr->player_stat[i];
			if(pstat == NULL)
			    continue;

			if(pstat->name == NULL)
			    continue;

			SARMenuListAppendItem(
			    m, list_num,
			    pstat->name,
			    NULL,		/* Data */
			    0			/* Flags */
			);

			/* Is this item the one with the changed name?
			 * If so then select it
			 */
			if(!strcmp(pstat->name, name))
			    SARMenuListSelect(
				display, m, list_num,
				i,
				True,	/* Scroll to */
				False	/* No redraw */
			    );
		    }

		    /* Save to file */
		    SARPlayerStatsSaveToFile(
			core_ptr->player_stat,
			core_ptr->total_player_stats,
			fname.players
		    );
		}
		else
		{
		    /* Conflicting name */
		    GWOutputMessage(
			display, 
			GWOutputMessageTypeWarning,
			"Name Exists",
			"A pilot by that name already exists",
			NULL
		    );
		}
	    }
	}

	/* Redraw the current menu since the prompt has been unmapped */
	SARMenuDrawAll(display, m);
	GWSwapBuffer(display);
}

/*
 *	Player add, edit, and remove callback.
 */
static void SARMenuCBPlayerOp(sar_core_struct *core_ptr, int id_code)
{
	gw_display_struct *display = core_ptr->display;
	sar_menu_struct *m = SARGetCurrentMenuPtr(core_ptr);
	int list_num;
	sar_menu_list_struct *list;
	sar_player_stat_struct *pstat;
	if(m == NULL)
	    return;

	/* Get players list */
	list = SAR_MENU_LIST(SARMenuGetObjectByID(
	    m, SAR_MENU_ID_PLAYER_LIST, &list_num
	));
	if(list == NULL)
	    return;

	/* Add, edit, or remove */
	switch(id_code)
	{
	  case SAR_MENU_ID_PLAYER_ADD:	/* Add player */
	    pstat = SARPlayerStatNew("New Pilot", 0);
	    if(pstat != NULL)
	    {
		int i = MAX(core_ptr->total_player_stats, 0);
		core_ptr->total_player_stats++;

		core_ptr->player_stat = (sar_player_stat_struct **)realloc(
		    core_ptr->player_stat,
		    core_ptr->total_player_stats * sizeof(sar_player_stat_struct *)
		);
		if(core_ptr->player_stat == NULL)
		{
		    core_ptr->total_player_stats = 0;
		    SARPlayerStatDelete(pstat);
		    break;
		}
		else
		{
		    core_ptr->player_stat[i] = pstat;

		    /* Append to list */
		    SARMenuListAppendItem(
			m, list_num,
			pstat->name,
			NULL,		/* Data */
			0		/* Flags */
		    );

		    /* Select */
		    SARMenuListSelect(
			display, m, list_num,
			i,
			True,		/* Scroll to */
			True		/* Redraw */
		    );
		}
	    }
	    /* Fall through to edit */

	  case SAR_MENU_ID_PLAYER_EDIT:	/* Change name */
	    pstat = SARGetPlayerStatPtr(
		core_ptr, list->selected_item
	    );
	    if((pstat != NULL) && (core_ptr->text_input != NULL))
	    {
		text_input_struct *p = core_ptr->text_input;
		if(!SARTextInputIsMapped(p))
		{
		    SARTextInputMap(
			p, "Name", pstat->name,
			SARMenuCBPlayerEditCB, core_ptr
		    );
		    SARTextInputDraw(p);
		    GWSwapBuffer(display);
		}
	    }
	    break;

	  case SAR_MENU_ID_PLAYER_REMOVE:	/* Remove player */
	    pstat = SARGetPlayerStatPtr(
		core_ptr, list->selected_item
	    );
	    if(pstat != NULL)
	    {
		char *s = (char *)malloc(80 + STRLEN(pstat->name));
		sprintf(
		    s,
		    "Remove pilot \"%s\"?",
		    pstat->name
		);
		if(GWConfirmation(
		    display,
		    GWOutputMessageTypeQuestion,
		    "Confirm Remove Pilot",
		    s,
		    NULL,
		    GWConfirmationNo
		) == GWConfirmationYes)
		{
		    int i = list->selected_item;
		    int n;

		    /* Delete player stats and shift pointers in the list */
		    SARPlayerStatDelete(pstat);
		    core_ptr->total_player_stats--;
		    for(n = i; n < core_ptr->total_player_stats; n++)
			core_ptr->player_stat[n] = core_ptr->player_stat[n + 1];
		    if(core_ptr->total_player_stats > 0)
		    {
			core_ptr->player_stat = (sar_player_stat_struct **)realloc(
			    core_ptr->player_stat,
			    core_ptr->total_player_stats * sizeof(sar_player_stat_struct *)
			);
			if(core_ptr->player_stat == NULL)
			    core_ptr->total_player_stats = 0;
		    }
		    else
		    {
			free(core_ptr->player_stat);
			core_ptr->player_stat = NULL;
			core_ptr->total_player_stats = 0;
		    }

		    /* Update list */
		    SARMenuListDeleteAllItems(m, list_num);
		    for(n = 0; n < core_ptr->total_player_stats; n++)
		    {
			pstat = core_ptr->player_stat[n];
			if(pstat == NULL)
			    continue;

			if(pstat->name == NULL)
			    continue;

			SARMenuListAppendItem(
			    m, list_num,
			    pstat->name,
			    NULL,		/* Data */
			    0			/* Flags */
			);
		    }

		    /* Select next item since the current one has been
		     * deleted
		     */
		    if(list->total_items > 0)
			SARMenuListSelect(
			    display, m, list_num,
			    (i >= list->total_items) ?
				MAX(list->total_items - 1, 0) : i,
			    True,	/* Scroll to */
			    True	/* Redraw */
			);
		    else
			SARMenuListSelectCB(
			    list, SAR_MENU_ID_PLAYER_LIST, core_ptr,
			    -1, NULL, NULL
			);
		}
		free(s);
	    }
	    break;
	}
}

/*
 *	Menu button callback.
 */
void SARMenuButtonCB(void *object, int id_code, void *client_data)
{
        static int campaign_mode = False;
	sar_core_struct *core_ptr = SAR_CORE(client_data);
	if((core_ptr == NULL) || (object == NULL))
	    return;

	switch(id_code)
	{
	  case SAR_MENU_ID_GOTO_MAIN:
	    SARMenuSwitchToMenu(core_ptr, SAR_MENU_NAME_MAIN);
	    break;

	  case SAR_MENU_ID_GOTO_EXIT:
	    runlevel = 1;
	    break;

	  case SAR_MENU_ID_GOTO_FREE_FLIGHT:
	    SARMenuSwitchToMenu(core_ptr, SAR_MENU_NAME_FREE_FLIGHT);
	    break;

	  case SAR_MENU_ID_GOTO_FREE_FLIGHT_AIRCRAFT:
	    SARMenuSwitchToMenu(core_ptr, SAR_MENU_NAME_FREE_FLIGHT_AIRCRAFT);
	    break;

	  case SAR_MENU_ID_GOTO_FREE_FLIGHT_WEATHER:
	    SARMenuSwitchToMenu(core_ptr, SAR_MENU_NAME_FREE_FLIGHT_WEATHER);
	    break;

	  case SAR_MENU_ID_GOTO_FREE_FLIGHT_AIRCRAFT_INFO:
	    SARMenuCBFreeFlightAircraftInfo(core_ptr);
	    break;

	  case SAR_MENU_ID_GOTO_FREE_FLIGHT_BEGIN:
	    SARMenuCBBeginFreeFlight(core_ptr);
	    break;

	  case SAR_MENU_ID_GOTO_MISSION:
            campaign_mode = False;
	    SARMenuSwitchToMenu(core_ptr, SAR_MENU_NAME_MISSION);
	    break;

	  case SAR_MENU_ID_GOTO_MISSION_PLAYER:
	    SARMenuSwitchToMenu(core_ptr, SAR_MENU_NAME_PLAYER);
	    break;

	  case SAR_MENU_ID_GOTO_MISSION_LOG_MAP:
	    SARMenuCBViewLastLog(core_ptr);
	    break;

	  case SAR_MENU_ID_GOTO_MISSION_MAP:
	    SARMenuCBMissionMap(core_ptr, campaign_mode);
	    break;

	  case SAR_MENU_ID_GOTO_MISSION_BRIEF:
	    SARMenuCBMissionBrief(core_ptr, False);
	    break;

	  case SAR_MENU_ID_GOTO_MISSION_BEGIN:
	    SARMenuCBBeginMission(core_ptr, campaign_mode);
	    break;

	  case SAR_MENU_ID_MISSION_MAP_LEFT:
	  case SAR_MENU_ID_MISSION_MAP_RIGHT:
	  case SAR_MENU_ID_MISSION_MAP_UP:
	  case SAR_MENU_ID_MISSION_MAP_DOWN:
	  case SAR_MENU_ID_MISSION_MAP_ZOOM_IN:
	  case SAR_MENU_ID_MISSION_MAP_ZOOM_OUT:
	  case SAR_MENU_ID_MISSION_LOG_MAP_LEFT:
	  case SAR_MENU_ID_MISSION_LOG_MAP_RIGHT:
	  case SAR_MENU_ID_MISSION_LOG_MAP_UP:
	  case SAR_MENU_ID_MISSION_LOG_MAP_DOWN:
	  case SAR_MENU_ID_MISSION_LOG_MAP_ZOOM_IN:
	  case SAR_MENU_ID_MISSION_LOG_MAP_ZOOM_OUT:
	  case SAR_MENU_ID_MISSION_LOG_MAP_EVENT_NEXT:
	  case SAR_MENU_ID_MISSION_LOG_MAP_EVENT_PREV:
	    SARMenuCBMapOp(core_ptr, id_code);   
	    break;

	  case SAR_MENU_ID_PLAYER_ADD:
	  case SAR_MENU_ID_PLAYER_EDIT:
	  case SAR_MENU_ID_PLAYER_REMOVE:
	    SARMenuCBPlayerOp(core_ptr, id_code);
	    break;

	  case SAR_MENU_ID_GOTO_CAMPAIGN:
            campaign_mode = True;
            Create_Campaign();
	    SARMenuCBMissionBrief(core_ptr, True);
            /*
	    SARMenuSwitchToMenu(core_ptr, SAR_MENU_NAME_CAMPAIGN);
            */ 
            /*
	    GWOutputMessage(
		core_ptr->display, GWOutputMessageTypeWarning,
		"Campaign Support Not Finished",
 "Sorry, campaign support is still under construction.",
"The coding work for campaign support is still under construction\n\
and not yet playable, please choose to either fly a mission or\n\
free flight."
	    );
            */
	    break;

	  case SAR_MENU_ID_GOTO_OPTIONS:
	    SARMenuOptionsFetch(core_ptr);
	    SARMenuSwitchToMenu(core_ptr, SAR_MENU_NAME_OPTIONS);
	    break;

	  case SAR_MENU_ID_GOTO_OPTIONS_SIMULATION:
	    SARMenuSwitchToMenu(core_ptr, SAR_MENU_NAME_OPTIONS_SIMULATION);
	    break;

	  case SAR_MENU_ID_GOTO_OPTIONS_CONTROLLER:
	    SARMenuSwitchToMenu(core_ptr, SAR_MENU_NAME_OPTIONS_CONTROLLER);
	    break;
	  case SAR_MENU_ID_GOTO_OPTIONS_CONTROLLER_JS_BTN:
	    SARMenuSwitchToMenu(
		core_ptr, SAR_MENU_NAME_OPTIONS_CONTROLLER_JS_BTN
	    );
	    break;
	  case SAR_MENU_ID_GOTO_OPTIONS_CONTROLLER_TEST:
	    SARMenuSwitchToMenu(
		core_ptr, SAR_MENU_NAME_OPTIONS_CONTROLLER_TEST
	    );
	    break;

	  case SAR_MENU_ID_GOTO_OPTIONS_GRAPHICS:
	    SARMenuSwitchToMenu(core_ptr, SAR_MENU_NAME_OPTIONS_GRAPHICS);
	    break;
	  case SAR_MENU_ID_GOTO_OPTIONS_GRAPHICS_INFO:
	    SARMenuOptionsGraphicsInfoRefresh(core_ptr);
	    SARMenuSwitchToMenu(core_ptr, SAR_MENU_NAME_OPTIONS_GRAPHICS_INFO);
	    break;

	  case SAR_MENU_ID_GOTO_OPTIONS_SOUND:
	    SARMenuOptionsSoundRefresh(core_ptr);
	    SARMenuSwitchToMenu(core_ptr, SAR_MENU_NAME_OPTIONS_SOUND);
	    break;
	  case SAR_MENU_ID_GOTO_OPTIONS_SOUND_INFO:
	    SARMenuOptionsSoundInfoRefresh(core_ptr);
	    SARMenuSwitchToMenu(core_ptr, SAR_MENU_NAME_OPTIONS_SOUND_INFO);
	    break;

	  default:
	    fprintf(stderr,
 "SARMenuButtonCB(): Internal error: Unsupported menu operation code `%i'.\n",
		id_code
	    );
	    break;
	}
}


/*
 *	Menu list select callback.
 */
void SARMenuListSelectCB(
	void *object, int id_code, void *client_data,
	int item_num, void *item_ptr, void *item_data
)
{
	gw_display_struct *display;
	sar_core_struct *core_ptr = SAR_CORE(client_data);
	sar_menu_struct *m;
	sar_menu_list_struct *list = SAR_MENU_LIST(object);
	sar_menu_list_item_data_struct *d = SAR_MENU_LIST_ITEM_DATA(item_data);
	sar_option_struct *opt;
	if((list == NULL) || (core_ptr == NULL))
	    return;

	display = core_ptr->display;
	opt = &core_ptr->option;

	switch(id_code)
	{
	  /* Mission: Select Mission */
	  case SAR_MENU_ID_MISSION_LIST:
	    opt->last_selected_mission = item_num;
	    break;

	  /* Player: Select Player */
	  case SAR_MENU_ID_PLAYER_LIST:
	    opt->last_selected_player = item_num;
	    m = SARMatchMenuByNamePtr(
		core_ptr, SAR_MENU_NAME_PLAYER
	    );
	    if(m == NULL)
		break;

	    /* A new player was selected, so the player stats message
	     * needs to be updated
	     */
	    if((item_num >= 0) && (item_num < core_ptr->total_player_stats))
	    {
		const sar_player_stat_struct *pstat = core_ptr->player_stat[item_num];
		int mesgbox_num;
		sar_menu_message_box_struct *mesgbox = SAR_MENU_MESSAGE_BOX(
		    SARMenuGetObjectByID(
			m, SAR_MENU_ID_PLAYER_STATS_MESG, &mesgbox_num
		    )
		);
		if((pstat != NULL) && (mesgbox != NULL))
		{
		    /* Format player stats message */
		    char *mesg = (char *)malloc(
			STRLEN(pstat->name) + (80 * 3)
		    );
		    sprintf(
			mesg,
"Score: <bold>%i<default> Rescues: <bold>%i<default>\n\
Time Aloft: <bold>%ldh %ldm<default> Crashes: <bold>%i<default>\n\
Missions: <bold>%i<default> (<bold>%i<default> Succeeded <bold>%i<default> Failed)",
			pstat->score,
			pstat->rescues,
			pstat->time_aloft / 60 / 60,	/* Hours */
			(pstat->time_aloft / 60) % 60,	/* Minutes */
			pstat->crashes,
			pstat->missions_succeeded + pstat->missions_failed,
			pstat->missions_succeeded,
			pstat->missions_failed
		    );
		    /* Set player stats message */
		    SARMenuMessageBoxSet(
			display, m, mesgbox_num,
			mesg,
			(Boolean)((m == SARGetCurrentMenuPtr(core_ptr)) ? True : False)
		    );
		    free(mesg);
		}
		else if(mesgbox != NULL)
		{
		    /* Clear player stats */
		    SARMenuMessageBoxSet(
			display, m, mesgbox_num,
			NULL,
			(Boolean)((m == SARGetCurrentMenuPtr(core_ptr)) ? True : False)
		    );
		}
	    }
	    else
	    {
		int mesgbox_num;
		sar_menu_message_box_struct *mesgbox = SAR_MENU_MESSAGE_BOX(
		    SARMenuGetObjectByID(
			m, SAR_MENU_ID_PLAYER_STATS_MESG, &mesgbox_num
		    )
		);
		if(mesgbox != NULL)
		{
		    /* Clear player stats */
		    SARMenuMessageBoxSet(
			display, m, mesgbox_num,
			NULL,
			(Boolean)((m == SARGetCurrentMenuPtr(core_ptr)) ? True : False)
		    );
		}
	    }
	    break;

	  /* Free Flight: Select Scenery */
	  case SAR_MENU_ID_FREE_FLIGHT_SCENERY_LIST:
	    opt->last_selected_ffscene = item_num;
	    m = SARMatchMenuByNamePtr(
		core_ptr, SAR_MENU_NAME_FREE_FLIGHT
	    );
	    if(m == NULL)
		break;

	    /* A new scenery was selected on the list, so the scenery's
	     * locations list needs to be updated
	     */
	    if((d != NULL) ? (d->filename != NULL) : False)
	    {
		const char *filename = d->filename;
		int loc_list_num;
		sar_menu_list_struct *loc_list = SAR_MENU_LIST(SARMenuGetObjectByID(
		    m, SAR_MENU_ID_FREE_FLIGHT_LOCATIONS_LIST, &loc_list_num
		));
		if(loc_list != NULL)
		{
		    /* Load locations from the selected scene */
		    SARSceneLoadLocationsToList(
		        core_ptr, m, loc_list, loc_list_num,
		        filename
		    );

		    /* Select first item on Locations List */
		    SARMenuListSelect(
			display, m, loc_list_num,
			0,
			True,		/* Scroll to */
			(Boolean)((m == SARGetCurrentMenuPtr(core_ptr)) ? True : False)
		    );
		}
	    }
	    break;

	  /* Free Flight: Select Scenery Location */
	  case SAR_MENU_ID_FREE_FLIGHT_LOCATIONS_LIST:
	    break;

	  /* Free Flight: Select Aircraft */
	  case SAR_MENU_ID_FREE_FLIGHT_AIRCRAFTS_LIST:
	    opt->last_selected_ffaircraft = item_num;
	    break;
	}
}

/*
 *	Menu List Activate callback.
 */
void SARMenuListActivateCB(
	void *object, int id_code, void *client_data,
	int item_num, void *item_ptr, void *item_data
)
{
/*	int i; */
	gw_display_struct *display;
	sar_core_struct *core_ptr = SAR_CORE(client_data);
/*	sar_menu_struct *m; */
	sar_menu_list_struct *list = SAR_MENU_LIST(object);
/*	sar_menu_list_item_data_struct *d = SAR_MENU_LIST_ITEM_DATA(item_data); */
	sar_option_struct *opt;
	if((list == NULL) || (core_ptr == NULL))
	    return;

	display = core_ptr->display;
	opt = &core_ptr->option;

	switch(id_code)
	{
	  /* Mission: Select Mission */
	  case SAR_MENU_ID_MISSION_LIST:
	    SARMenuCBMissionBrief(core_ptr, False);
	    break;

	  /* Player: Select Player */
	  case SAR_MENU_ID_PLAYER_LIST:
	    break;

	  /* Free Flight: Select Scenery */
	  case SAR_MENU_ID_FREE_FLIGHT_SCENERY_LIST:
	    break;

	  /* Free Flight: Select Scenery Location */
	  case SAR_MENU_ID_FREE_FLIGHT_LOCATIONS_LIST:
	    SARMenuSwitchToMenu(core_ptr, SAR_MENU_NAME_FREE_FLIGHT_AIRCRAFT);
	    break;

	  /* Free Flight: Select Aircraft */
	  case SAR_MENU_ID_FREE_FLIGHT_AIRCRAFTS_LIST:
	    SARMenuCBBeginFreeFlight(core_ptr);
	    break;
	}

}

/*
 *	Menu Standard Switch callback.
 */
void SARMenuSwitchCB(
	void *object, int id_code, void *client_data,
	Boolean state
)
{
	sar_core_struct *core_ptr = SAR_CORE(client_data);
	sar_menu_switch_struct *sw = (sar_menu_switch_struct *)object;
	sar_option_struct *opt;

	if((core_ptr == NULL) || (sw == NULL))
	    return;

	opt = &core_ptr->option;

	/* Handle by menu object id code */
	switch(id_code)
	{
	  case SAR_MENU_ID_MENU_FREE_FLIGHT_SYSTEM_TIME:
	    opt->system_time_free_flight = sw->state;
	    break;

	}
}

/*
 *	Menu Standard Spin callback.
 */
void SARMenuSpinCB(
	void *object, int id_code, void *client_data,
	char *value
)
{
	sar_core_struct *core_ptr = SAR_CORE(client_data);
	sar_menu_spin_struct *spin_ptr = (sar_menu_spin_struct *)object;
	sar_option_struct *opt;

	if((core_ptr == NULL) || (spin_ptr == NULL))
	    return;

	opt = &core_ptr->option;

	/* Handle by menu object id code */
	switch(id_code)
	{
	  case SAR_MENU_ID_MENU_FREE_FLIGHT_WEATHER_CONDITION:
	    opt->last_selected_ffweather = spin_ptr->cur_value;
	    break;

	}
}
