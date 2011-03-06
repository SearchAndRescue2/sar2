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
This head file contains all the function definitions and misc items
needed to get a campaign underway.
-- Jesse
*/

#ifndef CAMPAIGN_HEADER_FILE__
#define CAMPAIGN_HEADER_FILE__

#ifndef True
#define True 1
#endif
#ifndef False
#define False 0
#endif

#define OBJECTIVE_ARRIVE 0
#define OBJECTIVE_PICKUP 1

#define MAX_OBJECTIVES 5

#define LOCATION_NONE 0
#define LOCATION_LAX 1
#define LOCATION_SMO 2
#define LOCATION_UCLA 3
#define LOCATION_CAT 4
#define LOCATION_USCG 5
#define LOCATION_SEARCH 6

#define RESCUE_DISTANCE 1000


/*
Create an empty campaign file where we can write to it.
Returns an open file on success and NULL on failure.
*/
FILE *Create_Campaign_File();

/*
This function returns a string with one of eight possible weather conditions.
The string should be freed after use. On error, NULL is returned.
*/
char *Create_Campaign_Weather();

/* This function returns a filename for a scenery location. The
string should be freed after use. On error, NULL is returned.
*/
char *Create_Campaign_Scenery();

/*
Picks a place to start at random. The function returns a location number.
*/
int Create_Campaign_Start_Point();

char *Create_Campaign_Description(int first_objective);

/* Creates a string that will contains all aspects of an arrival mission. The returned
string should be freed after use.
The parameters taken in to the function represent where they 
are currently going and where they will go after reaching their current destination.
*/
char *Create_Campaign_Arrive_Objective(int current_destination, int next_destination);

char *Create_Campaign_Pickup_Objective(int next_location);

/* This function returns a string which contains a location internal name. For
example, helipad_lax or helipad_cat. The string should be freed after use.
*/
char *Get_Location_Symbol(int location);

/* This function returns a location's proper name, at it appears to the player.
The returned string should be freed after use.
*/
char *Get_Location_Name(int location);


/*
Accepts a location and returns the map coordinates for that
location. The function returns True if it knows the location and
False if it is an unknown place.
*/
int Get_Coordinates(int location, int *x, int *y);


/*
This function returns a string which gives the location of the
USCG ship and its helipad. If an error occurs, then NULL is returned.
The returned string should be freed after use.
*/
char *Place_Campaign_Ship();

/*
This function returns a long string which contains all the location data
for humans who need to be picked up.
The function takes in a list of destinations and the pilot's start point. It
then tries to place humans near where the pilot will be.
On success, a string is returned which can be written to the mission file. 
The string should be freed after use. On error, NULL is returned.
*/
char *Place_Campaign_Humans(int start_location, int *destinations);


/*
This function selects an appropriate craft for the mission. A string with the craft
name is returned. If an error occures, NULL is returned. The string should be freed
after use.
*/
char *Create_Campaign_Aircraft(int all_arrivals);

/*
Creates a filename, though not a file, which should be created
in the user's home directory. On success, a string is returned,
which should be freed after use. On failure, NULL is returned.
*/
char *Get_Campaign_Filename();

/*
This function is the wrapper for all the above functions. It
creates a new mission file, fills it with missiony goodness
and closes the file. On success it returns True and on error
it returns False and prints a message to standard out.
*/
int Create_Campaign();

#endif

