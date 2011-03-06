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

/*
                             SAR Menu Codes

	Menu Names and Operation codes
 */

#ifndef SARMENUCODES_H
#define SARMENUCODES_H


/*
 *      Menu names:
 */
#define SAR_MENU_NAME_MAIN                      "Main"

#define SAR_MENU_NAME_FREE_FLIGHT               "Free Flight"
#define SAR_MENU_NAME_FREE_FLIGHT_AIRCRAFT      "Free Flight Aircraft"
#define SAR_MENU_NAME_FREE_FLIGHT_WEATHER       "Free Flight Weather"
#define SAR_MENU_NAME_FREE_FLIGHT_AIRCRAFT_INFO "Free Flight Aircraft Info"

#define SAR_MENU_NAME_MISSION                   "Mission"
#define SAR_MENU_NAME_MISSION_BRIEF             "Mission Brief"
#define SAR_MENU_NAME_MISSION_MAP               "Mission Map"
#define SAR_MENU_NAME_MISSION_LOG_MAP           "Mission Log Map"

#define SAR_MENU_NAME_CAMPAIGN                  "Campaign"

#define SAR_MENU_NAME_PLAYER			"Player"

#define SAR_MENU_NAME_OPTIONS                   "Options"
#define SAR_MENU_NAME_OPTIONS_SIMULATION        "Options Simulation"
#define SAR_MENU_NAME_OPTIONS_CONTROLLER        "Options Controler"
#define SAR_MENU_NAME_OPTIONS_CONTROLLER_JS_BTN "Options Controler JS Buttons"
#define SAR_MENU_NAME_OPTIONS_CONTROLLER_TEST	"Options Controler Test"
#define SAR_MENU_NAME_OPTIONS_GRAPHICS          "Options Graphics"
#define SAR_MENU_NAME_OPTIONS_GRAPHICS_INFO     "Options Graphics Info"
#define SAR_MENU_NAME_OPTIONS_SOUND             "Options Sound"
#define SAR_MENU_NAME_OPTIONS_SOUND_INFO        "Options Sound Info"

#define SAR_MENU_NAME_LOADING_SIMULATION        "Loading Simulation"


/*
 *	Menu Object IDs:
 *
 *	Used to identify menu objects and/or operations.
 */
#define SAR_MENU_ID_GOTO_MAIN				0
#define SAR_MENU_ID_GOTO_EXIT				1

#define SAR_MENU_ID_GOTO_MISSION			10
#define SAR_MENU_ID_GOTO_CAMPAIGN			11
#define SAR_MENU_ID_GOTO_FREE_FLIGHT      		12
#define SAR_MENU_ID_GOTO_OPTIONS			13

#define SAR_MENU_ID_MISSION_LIST			20
#define SAR_MENU_ID_MISSION_BRIEF_MESG			21
#define SAR_MENU_ID_GOTO_MISSION_PLAYER			22
#define SAR_MENU_ID_GOTO_MISSION_BRIEF			23
#define SAR_MENU_ID_GOTO_MISSION_BEGIN			24
#define SAR_MENU_ID_GOTO_MISSION_MAP			25
#define SAR_MENU_ID_GOTO_MISSION_LOG_MAP		26	/* View last log */
#define SAR_MENU_ID_MISSION_MAP_LEFT			27
#define SAR_MENU_ID_MISSION_MAP_RIGHT			28
#define SAR_MENU_ID_MISSION_MAP_UP			29
#define SAR_MENU_ID_MISSION_MAP_DOWN			30
#define SAR_MENU_ID_MISSION_MAP_ZOOM_IN			31
#define SAR_MENU_ID_MISSION_MAP_ZOOM_OUT		32
#define SAR_MENU_ID_MISSION_LOG_MAP_LEFT		33
#define SAR_MENU_ID_MISSION_LOG_MAP_RIGHT		34
#define SAR_MENU_ID_MISSION_LOG_MAP_UP			35
#define SAR_MENU_ID_MISSION_LOG_MAP_DOWN		36
#define SAR_MENU_ID_MISSION_LOG_MAP_ZOOM_IN		37
#define SAR_MENU_ID_MISSION_LOG_MAP_ZOOM_OUT		38
#define SAR_MENU_ID_MISSION_LOG_MAP_EVENT_NEXT		39
#define SAR_MENU_ID_MISSION_LOG_MAP_EVENT_PREV		40

#define SAR_MENU_ID_PLAYER_LIST				60
#define SAR_MENU_ID_PLAYER_STATS_MESG			61
#define SAR_MENU_ID_PLAYER_ADD				65
#define SAR_MENU_ID_PLAYER_EDIT				66	/* Change name */
#define SAR_MENU_ID_PLAYER_REMOVE			67

#define SAR_MENU_ID_FREE_FLIGHT_SCENERY_LIST		70
#define SAR_MENU_ID_FREE_FLIGHT_LOCATIONS_LIST		71
#define SAR_MENU_ID_FREE_FLIGHT_AIRCRAFTS_LIST		72
#define SAR_MENU_ID_GOTO_FREE_FLIGHT_AIRCRAFT		73
#define SAR_MENU_ID_GOTO_FREE_FLIGHT_BEGIN		74
#define SAR_MENU_ID_GOTO_FREE_FLIGHT_WEATHER		75
#define SAR_MENU_ID_GOTO_FREE_FLIGHT_AIRCRAFT_INFO	76

#define SAR_MENU_ID_MENU_FREE_FLIGHT_WEATHER_CONDITION	80
#define SAR_MENU_ID_MENU_FREE_FLIGHT_SYSTEM_TIME	81

#define SAR_MENU_ID_GOTO_OPTIONS_SIMULATION		100
#define SAR_MENU_ID_GOTO_OPTIONS_CONTROLLER		110
#define SAR_MENU_ID_GOTO_OPTIONS_CONTROLLER_JS_BTN	111
#define SAR_MENU_ID_GOTO_OPTIONS_CONTROLLER_TEST	112
#define SAR_MENU_ID_GOTO_OPTIONS_GRAPHICS		120
#define SAR_MENU_ID_GOTO_OPTIONS_GRAPHICS_INFO		121
#define SAR_MENU_ID_GRAPHICS_INFO_MESG			122
#define SAR_MENU_ID_GOTO_OPTIONS_SOUND			130
#define SAR_MENU_ID_GOTO_OPTIONS_SOUND_INFO		131
#define SAR_MENU_ID_SOUND_INFO_MESG			132

/*
 *	ID codes for options menu objects, these should not conflict
 *	with any of the SAR_MENU_ID_MENU_* codes)
 */
#define SAR_MENU_ID_OPT_UNITS			510	/* Spin */
#define SAR_MENU_ID_OPT_HOIST_CONTACT		511	/* Spin */
#define SAR_MENU_ID_OPT_DAMAGE_RESISTANCE	512	/* Spin */
#define SAR_MENU_ID_OPT_FLIGHT_PHYSICS		513	/* Spin */

#define SAR_MENU_ID_OPT_JS0_CONNECTION		520	/* Spin */
#define SAR_MENU_ID_OPT_JS1_CONNECTION		521	/* Spin */
#define SAR_MENU_ID_OPT_JS0_AXISES		522	/* Spin */
#define SAR_MENU_ID_OPT_JS1_AXISES		523	/* Spin */
#define SAR_MENU_ID_OPT_JS_PRIORITY		524	/* Spin */
#define SAR_MENU_ID_OPT_JS0_BUTTON_ACTION	530	/* Spin */
#define SAR_MENU_ID_OPT_JS0_BUTTON_NUMBER	531	/* Spin */
#define SAR_MENU_ID_OPT_JS1_BUTTON_ACTION	532	/* Spin */
#define SAR_MENU_ID_OPT_JS1_BUTTON_NUMBER	533	/* Spin */
#define SAR_MENU_ID_OPT_CONTROLLER_REFRESH	534	/* Button */
#define SAR_MENU_ID_OPT_JS_TEST_UPDATE		535	/* MDisplay */

#define SAR_MENU_ID_OPT_GROUND_TEXTURE		550	/* Switch */
#define SAR_MENU_ID_OPT_OBJECT_TEXTURE		551	/* Switch */
#define SAR_MENU_ID_OPT_CLOUDS			552	/* Switch */
#define SAR_MENU_ID_OPT_ATMOSPHERE		553	/* Switch */
#define SAR_MENU_ID_OPT_DUAL_PASS_DEPTH		554	/* Switch */
#define SAR_MENU_ID_OPT_PROP_WASH		555	/* Switch */
#define SAR_MENU_ID_OPT_SMOKE_TRAILS		556	/* Switch */
#define SAR_MENU_ID_OPT_CELESTIAL_OBJECTS	557	/* Switch */
#define SAR_MENU_ID_OPT_VISIBILITY_MAX		558	/* Spin */
#define SAR_MENU_ID_OPT_GRAPHICS_ACCELERATION	559	/* Slider */
#define SAR_MENU_ID_OPT_RESOLUTION		560	/* Switch */
#define SAR_MENU_ID_OPT_FULLSCREEN		561	/* Spin */
#define SAR_MENU_ID_OPT_GRAPHICS_INFO_SAVE	570	/* Button */

#define SAR_MENU_ID_OPT_SOUND_LEVEL		580	/* Spin */
#define SAR_MENU_ID_OPT_SOUND_PRIORITY		581	/* Spin */
#define SAR_MENU_ID_OPT_MUSIC			582	/* Switch */
#define SAR_MENU_ID_OPT_VOLUME_MASTER		585	/* Slider */
#define SAR_MENU_ID_OPT_VOLUME_SOUND		586	/* Slider */
#define SAR_MENU_ID_OPT_VOLUME_MUSIC		587	/* Slider */
#define SAR_MENU_ID_OPT_SOUND_TEST		590	/* Button */
#define SAR_MENU_ID_OPT_SOUND_INFO_SAVE		591	/* Button */

#endif	/* SARMENUCODES_H */
