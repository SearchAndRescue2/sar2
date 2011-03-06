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
These functions create a campaign mission file.
See sarcamp.h for details on how the functions work.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sarcamp.h"




FILE *Create_Campaign_File()
{
    char *filename;
    FILE *camp_file;

   filename = Get_Campaign_Filename();
   if (! filename)
      return NULL;

   camp_file = fopen(filename, "w");
   if (! camp_file)
      printf("Unable to open file %s\n", filename);

   free(filename);
   return camp_file;
}



char *Create_Campaign_Weather()
{
   char *weather_string;
   int random_num;

   weather_string = (char *) calloc(64, sizeof(char));
   if (! weather_string)
      return NULL;

   random_num = rand() % 8;
   switch (random_num)
   {
      case 0: strcpy(weather_string, "weather Clear"); break;
      case 1: strcpy(weather_string, "weather Hazy"); break;
      case 2: strcpy(weather_string, "weather Scattered"); break;
      case 3: strcpy(weather_string, "weather Cloudy"); break;
      case 4: strcpy(weather_string, "weather Overcast"); break;
      case 5: strcpy(weather_string, "weather Foggy"); break;
      case 6: strcpy(weather_string, "weather Stormy Sparse"); break;
      case 7: strcpy(weather_string, "weather Stormy Dense"); break;
   }
   
   return weather_string;
}


char *Create_Campaign_Scenery()
{
    char *scene_file;

    scene_file = (char *) calloc(128, sizeof(char));
    if (! scene_file)
       return NULL;

    strcpy(scene_file, "mission_scene_file scenery/los_angeles.scn");
    return scene_file;
}


int Create_Campaign_Start_Point()
{
     int start_number;

    start_number = (rand() % 4) + 1;
    return start_number;
}


char *Create_Campaign_Description(int first_objective)
{
    char *buffer;
    char *place_name;

    buffer = (char *) calloc(1024, sizeof(char));
    if (! buffer)
       return NULL;

    strcpy(buffer, "name Mission: Patrol\n");
    strcat(buffer, "description ");

    if (first_objective == LOCATION_SEARCH)
         strcat(buffer, "Search the local area for an accident victim.\n");
    else
    {
       place_name = Get_Location_Name(first_objective);
       if (place_name)
       {
           strcat(buffer, "Fly to ");
           strcat(buffer, place_name);
           strcat(buffer, ".\n");
           free(place_name);
       }
    }

    return buffer;
}



int Get_Coordinates(int location, int *x, int *y)
{
    int status = True;

    switch (location)
    {
        case LOCATION_LAX: *x = -5430 ; *y = 35750; break;
        case LOCATION_SMO: *x = -7300; *y = 42580; break;
        case LOCATION_UCLA: *x = -4920; *y = 44800; break;
        case LOCATION_CAT: *x = -4931; *y = -25183; break;
        case LOCATION_USCG: *x = -23758; *y = 34647; break;
        default: status = False; break;
    }

    return status;
}



char *Place_Campaign_Ship()
{
    char *buffer;

    buffer = (char *) calloc(4096, sizeof(char));
    if (! buffer)
      return NULL;

    strcpy(buffer, "create_object 3\n");
    strcat(buffer, "model_file vessels/uscg378.3d\n");
    strcat(buffer, "translation -23758.0 34647.0 0.0\n");
    strcat(buffer, "rotate 300 0 0\n");
    strcat(buffer, "object_name uscg378_01\n");
    strcat(buffer, "create_helipad vehicle 29.0 18.0 3.2 USCG y y y y y uscg378_01 0.0 -58.5 36.0  0.0 0.0 0.0\n");
    strcat(buffer, "object_name helipad_uscg\n");
    strcat(buffer, "object_map_description Coast Guard helipad\n");
    strcat(buffer, "range 2200\n");
   
    return buffer;
}



char *Place_Campaign_Humans(int start_place, int *destinations)
{
    char *buffer;
    char coordinates[256];
    int objective = 0;
    int last_location = start_place;
    int place_x = 0, place_y = 0;
    int human_x, human_y;

    buffer = (char *) calloc(4096, sizeof(char));
    if (! buffer)
       return NULL;

    strcpy(buffer, "# Place humans\n");
    while (objective < MAX_OBJECTIVES)
    {
        if (destinations[objective] == LOCATION_SEARCH)
        {
            if (last_location == LOCATION_USCG)
                strcat(buffer, "create_human default need_rescue alert aware in_water\n");
            else
               strcat(buffer, "create_human default need_rescue alert aware\n");
            strcat(buffer, "translation ");
            Get_Coordinates(last_location, &place_x, &place_y);
            human_x = (rand() % RESCUE_DISTANCE) - (RESCUE_DISTANCE / 2);
            human_x = place_x + human_x;
            human_y = (rand() & RESCUE_DISTANCE) - (RESCUE_DISTANCE / 2);
            human_y = place_y + human_y;
            sprintf(coordinates, "%d %d 0.0", human_x, human_y);
            strcat(buffer, coordinates);
            strcat(buffer, "\nset_human_mesg_enter Thank you for the lift.\n");

            if (last_location == LOCATION_CAT)
            {
                strcat(buffer, "create_fire 10.0 15.0\n");
                strcat(buffer, "translation ");
                sprintf(coordinates, "%d %d 0.0\n", human_x + 200, human_y - 100);
                strcat(buffer, coordinates);
            }
        }
        else if ( (destinations[objective] > LOCATION_NONE) &&
                  (destinations[objective] < LOCATION_SEARCH) )
            last_location = destinations[objective];

        objective++;
    }
 
    return buffer;
}




char *Create_Campaign_Aircraft(int all_arrivals)
{
     char *craft_name;
     int aircraft_number;

     craft_name = (char *) calloc(128, sizeof(char));
     if (! craft_name)
       return NULL;

     strcpy(craft_name, "player_model_file aircrafts/");
     /* we can use any helicoptor for all arrival missions */
     if (all_arrivals)
         aircraft_number = rand() % 8;
     else
         aircraft_number = rand() % 4;

     switch (aircraft_number)
     {
         case 0: strcat(craft_name, "hh60.3d"); break;
         case 1: strcat(craft_name, "hh65.3d"); break;
         case 2: strcat(craft_name, "ka27.3d"); break;
         case 3: strcat(craft_name, "v22.3d"); break;
         /* end of all all mission list. From here it's arrivals only */
         case 4: strcat(craft_name, "as350.3d"); break;
         case 5: strcat(craft_name, "b47.3d"); break;
         case 6: strcat(craft_name, "s64.3d"); break;
         case 7: strcat(craft_name, "uh1d.3d"); break;
     }

     return craft_name;     
}



char *Create_Campaign_Arrive_Objective(int current_location, int next_location)
{
     char *buffer;
     char *location_name;

     buffer = (char *) calloc( 512, sizeof(char));
     if (! buffer)
       return NULL;

     strcpy(buffer, "mission_objective_new arrive\n");
     strcat(buffer, "mission_objective_time_left 0.0\n");
     strcat(buffer, "mission_objective_arrive_at ");
     /* put in current location */
     location_name = Get_Location_Symbol(current_location);
     if (location_name)
     {
         strcat(buffer, location_name);
         free(location_name);
     }
     strcat(buffer, "\n");
     strcat(buffer, "mission_objective_message_success Welcome!");
     location_name = Get_Location_Name(next_location);
     /* put in next location */
     if (location_name)
     {
        strcat(buffer, " Next head for ");
        strcat(buffer, location_name);
        free(location_name);
     }
     strcat(buffer, "\nmission_objective_message_fail You did not arrive safely.\n");
     return buffer;
}



char *Create_Campaign_Pickup_Objective(int next_location)
{
    char *buffer;
    char *location_name;

    buffer = (char *) calloc(1024, sizeof(char));
    if (! buffer)
       return NULL;

    strcpy(buffer, "mission_objective_new pick_up\n");
    strcat(buffer, "mission_objective_time_left 0.0\n");
    strcat(buffer, "mission_objective_humans_tally 1 1\n");
    strcat(buffer, "mission_objective_message_success ");
    /* put next location in here */
    location_name = Get_Location_Name(next_location);
    if (location_name)
    {
       strcat(buffer, "Pick up complete. Head for ");
       strcat(buffer, location_name);
       free(location_name);
    }
    strcat(buffer, "\nmission_objective_message_fail You were unable to complete pick-up.\n");
    
   return buffer;
}




char *Get_Location_Symbol(int location)
{
    char *my_string;

     my_string = (char *) calloc(64, sizeof(char));
     if (! my_string)
         return NULL;

     switch (location)
     {
        case LOCATION_NONE: free(my_string); my_string = NULL; break;
        case LOCATION_LAX: strcpy(my_string, "helipad_lax"); break;
        case LOCATION_SMO: strcpy(my_string, "helipad_smo"); break;
        case LOCATION_UCLA: strcpy(my_string, "helipad_ucla"); break;
        case LOCATION_CAT: strcpy(my_string, "helipad_cat"); break;
        case LOCATION_USCG: strcpy(my_string, "helipad_uscg"); break;
        default: my_string[0] = '\0'; break;
 
     }
     return my_string;
}



char *Get_Location_Name(int location)
{
   char *my_string;
   my_string = (char *) calloc(64, sizeof(char));
   if (! my_string)
     return NULL;

     switch (location)
     {
        case LOCATION_NONE: free(my_string); my_string = NULL; break;
        case LOCATION_LAX: strcpy(my_string, "Los Angeles Airport"); break;
        case LOCATION_SMO: strcpy(my_string, "Santa Monica Airport"); break;
        case LOCATION_UCLA: strcpy(my_string, "UCLA Hospital"); break;
        case LOCATION_CAT: strcpy(my_string, "Catalina Airport"); break;
        case LOCATION_USCG: strcpy(my_string, "US Coast Guard ship"); break;
        case LOCATION_SEARCH: strcpy(my_string, "a nearby victim"); break;

     }
     return my_string;
}



char *Get_Campaign_Filename()
{
    char *full_filename;
    char *home_dir;

    home_dir = getenv("HOME");
    if (! home_dir)
       return NULL;

    full_filename = (char *) calloc( strlen(home_dir) + 64, sizeof(char) );
    if (! full_filename)
       return NULL;

    sprintf(full_filename, "%s/.SearchAndRescue/campaign.mis", home_dir);
    return full_filename;
}




int Create_Campaign()
{
   FILE *camp_file;
   char *objective_string, *temp_string;
   int start_point;
   int objective_type, all_arrivals = True;
   int number_of_objectives;
   int done_description = False;
   int destination[MAX_OBJECTIVES];
   int count;

   camp_file = Create_Campaign_File();
   if (!camp_file)
      return False;

   memset(destination, 0, MAX_OBJECTIVES * sizeof(int));
   start_point = Create_Campaign_Start_Point();
   number_of_objectives = (rand() % 3) + 3;

   /* figure out where we will go */
   count = 0;
   while (count < number_of_objectives)
   {
       if (count < (number_of_objectives - 1) )
           objective_type = rand() % 2;
       else         /* last one is always arrival */
           objective_type = OBJECTIVE_ARRIVE;

       if (objective_type == OBJECTIVE_ARRIVE)
       {
           int done;
           int temp_place;
           do
           {
               temp_place = (rand() % 5) + 1;
               if ( (count == 0) && (temp_place == start_point) )
                  done = False;
               else if ( (count > 0) && (temp_place == destination[count - 1]) )
                  done = False;
               else
                  done = True;
           } while (! done);
           destination[count] = temp_place;
       }
       else
           destination[count] = LOCATION_SEARCH;
       count++;
   }

   count = 0;
   while (count < number_of_objectives)
   {
       if (destination[count] == LOCATION_SEARCH)
       {
          objective_string = Create_Campaign_Pickup_Objective(destination[count + 1]);
          all_arrivals = False;
       }
       else
          objective_string = Create_Campaign_Arrive_Objective(destination[count], destination[count + 1]);

       if (! done_description)
       {
          temp_string = Create_Campaign_Description(destination[0]);
          if (temp_string)
          {
            fprintf(camp_file, "%s\n", temp_string);
            free(temp_string);
          }
          temp_string = Create_Campaign_Weather();
          if (temp_string)
          {
            fprintf(camp_file, "%s\n", temp_string);
            free(temp_string);
          }

          temp_string = Create_Campaign_Scenery();
          if (temp_string)
          {
             fprintf(camp_file, "%s\n", temp_string);
             free(temp_string);
          }

          temp_string = Place_Campaign_Ship();
          if (temp_string)
          {
               fprintf(camp_file, "%s\n", temp_string);
               free(temp_string);
          }

          temp_string = Get_Location_Symbol(start_point);
          if (temp_string)
          {
              fprintf(camp_file, "mission_begin_at %s\n", temp_string);
              free(temp_string);
          }
          done_description = True;
       }
       
       if (objective_string)
       {
           fprintf(camp_file, "%s\n", objective_string);
           free(objective_string);
       }
       count++;
   }

   temp_string = Place_Campaign_Humans(start_point, destination);
   if (temp_string)
   {
       fprintf(camp_file, "%s\n", temp_string);
       free(temp_string);
   }

   temp_string = Create_Campaign_Aircraft(all_arrivals);
   if (temp_string)
   {
        fprintf(camp_file, "%s\n", temp_string);
        free(temp_string);
   }
   fclose(camp_file);

   return True;
}

