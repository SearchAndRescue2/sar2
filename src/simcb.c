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
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <math.h>

#include "matrixmath.h"
#include "sfm.h"
#include "obj.h"
#include "objutils.h"
#include "objsound.h"
#include "messages.h"
#include "smoke.h"
#include "explosion.h"
#include "sar.h"
#include "simmanage.h"
#include "simcb.h"
#include "simop.h"
#include "simutils.h"
#include "sardrawselect.h"
#include "config.h"


void SARSimInitModelCB(
    void *realm_ptr, SFMModelStruct *model,
    void *client_data
    );
void SARSimDestroyModelCB(
    void *realm_ptr, SFMModelStruct *model,
    void *client_data                     
    );

void SARSimAirborneCB(
    void *realm_ptr, SFMModelStruct *model,
    void *client_data
    );
void SARSimTouchDownCB(
    void *realm_ptr, SFMModelStruct *model,
    void *client_data, double impact_coeff
    );
void SARSimParkedCB(
    SFMModelStruct *model,
    void *client_data
    );
void SARSimOverspeedCB(
    void *realm_ptr, SFMModelStruct *model,
    void *client_data, double cur_speed,
    double overspeed_expected,
    double overspeed
    );
void SARSimCollisionCB(
    void *realm_ptr, SFMModelStruct *model, SFMModelStruct *obstruction,
    void *client_data, double impact_coeff 
    );
void SARSimObjectCollisionCB(
    void *client_data,              /* Core structure */
    sar_object_struct *obj_ptr, sar_object_struct *obstruction_obj_ptr,
    double impact_coeff
    );


#define MAX(a,b)        (((a) > (b)) ? (a) : (b))
#define MIN(a,b)        (((a) < (b)) ? (a) : (b))
#define CLIP(a,l,h)     (MIN(MAX((a),(l)),(h)))

#define RADTODEG(r)     ((r) * 180 / PI)
#define DEGTORAD(d)     ((d) * PI / 180)


/*
 *	FDM Initialize callback
 *
 *	Sets up initial values from the model's corresponding object.
 */
void SARSimInitModelCB(
    void *realm_ptr, SFMModelStruct *model,
    void *client_data
    )
{
    int obj_num;
    sar_object_struct *obj_ptr;
    SFMRealmStruct *realm = SFM_REALM(realm_ptr);
    sar_core_struct *core_ptr = SAR_CORE(client_data);
    if((realm == NULL) || (model == NULL) || (core_ptr == NULL))
        return;

    /* Match object from FDM */
    obj_ptr = SARSimMatchObjectFromFDM(
        core_ptr->object, core_ptr->total_objects,
        model, &obj_num
	);
    if(obj_ptr == NULL)
        return;

    /* Set up initial values, note that position and direction
     * values will probably be reset soon afterwards since
     * creation of FDM's usually happen in the middle of
     * scene file parsings and their positions will be
     * set after creation of the FDM
     */
#if 0
/* No need to set position of model, calling function should do that */
    model->position.x = obj_ptr->pos.x;
    model->position.y = obj_ptr->pos.y;
    model->position.z = obj_ptr->pos.z;

    model->direction.heading = obj_ptr->dir.heading;
    model->direction.pitch = obj_ptr->dir.pitch;
    model->direction.bank = obj_ptr->dir.bank;
#endif

    /* Always assume landed */
    model->landed_state = True;
    model->stopped = True;
}


/*
 *	FDM Destroy callback.
 *
 *	Note that the specified FDM is invalid and should not be
 *	referenced.
 */
void SARSimDestroyModelCB(
    void *realm_ptr, SFMModelStruct *model,
    void *client_data
    )
{
    int i;
    sar_object_struct *obj_ptr;
    sar_object_aircraft_struct *obj_aircraft_ptr;
    SFMRealmStruct *realm = SFM_REALM(realm_ptr);
    sar_core_struct *core_ptr = SAR_CORE(client_data);
    if((realm == NULL) || (model == NULL) || (core_ptr == NULL))
        return;

    /* Iterate through objects and unset FDM references to the
     * specified FDM
     */
    for(i = 0; i < core_ptr->total_objects; i++)
    {
        obj_ptr = core_ptr->object[i];
        if(obj_ptr == NULL)
            continue;

        switch(obj_ptr->type)
        {
            case SAR_OBJ_TYPE_GARBAGE:
            case SAR_OBJ_TYPE_STATIC:
            case SAR_OBJ_TYPE_AUTOMOBILE:
            case SAR_OBJ_TYPE_WATERCRAFT:
		break;

            case SAR_OBJ_TYPE_AIRCRAFT:
		obj_aircraft_ptr = SAR_OBJ_GET_AIRCRAFT(obj_ptr);
		if(obj_aircraft_ptr == NULL)
		    break;
		if(obj_aircraft_ptr->fdm == model)
		    obj_aircraft_ptr->fdm = NULL;
		break;

            case SAR_OBJ_TYPE_GROUND:
            case SAR_OBJ_TYPE_RUNWAY:
            case SAR_OBJ_TYPE_HELIPAD:
            case SAR_OBJ_TYPE_HUMAN:
            case SAR_OBJ_TYPE_SMOKE:
            case SAR_OBJ_TYPE_FIRE:
            case SAR_OBJ_TYPE_EXPLOSION:
            case SAR_OBJ_TYPE_CHEMICAL_SPRAY:
            case SAR_OBJ_TYPE_FUELTANK:
            case SAR_OBJ_TYPE_PREMODELED:
		break;
        }
    }
}


/*
 *	FDM Airborne callback.
 *
 *	Called whenever the FDM just leaves the ground.
 */
void SARSimAirborneCB(
    void *realm_ptr, SFMModelStruct *model,
    void *client_data
    )
{
    SFMRealmStruct *realm = SFM_REALM(realm_ptr);
    sar_core_struct *core_ptr = SAR_CORE(client_data);
    if((realm == NULL) || (model == NULL) || (core_ptr == NULL))
        return;

#if 0
    /* Skip airborne check entirly if in slew mode */
    if(SARSimIsSlew(obj_ptr))
        return;
#endif

}       


/*
 *	FDM Touchdown callback.
 *
 *	Called whenever the FDM just lands (on any surface).
 */
void SARSimTouchDownCB(
    void *realm_ptr, SFMModelStruct *model,
    void *client_data, double impact_coeff
    )
{
    int obj_num, crash_cause = 0;
    Boolean	over_water = False,
        have_floats = False,
        camera_in_cockpit = False;
    float	distance_to_camera,
        contact_radius = 0.0f;
    sar_obj_flags_t crash_flags = 0;
    sar_air_worthy_state air_worthy_state = SAR_AIR_WORTHY_FLYABLE;
    sar_position_struct	*pos,
        *ear_pos,
        *vel = NULL;
    sar_direction_struct *dir;
    sar_contact_bounds_struct *cb;
    sar_object_struct *obj_ptr;
    sar_object_aircraft_struct *obj_aircraft_ptr = NULL;
    sar_scene_struct *scene;   
    SFMRealmStruct *realm = SFM_REALM(realm_ptr);
    sar_core_struct *core_ptr = SAR_CORE(client_data);
    const sar_option_struct *opt;
    if((realm == NULL) || (model == NULL) || (core_ptr == NULL))
        return;

    opt = &core_ptr->option;
    scene = core_ptr->scene;
    if(scene == NULL)
        return;

/* Creates sparks and plays the landed on belly sound */
#define DO_EFFECTS_LAND_BELLY                                           \
    { if((obj_aircraft_ptr != NULL) && (cb != NULL)) {                  \
            const time_t life_span = 5000l;                             \
            sar_position_struct pos_offset;                             \
            pos_offset.x = 0.0f;					\
            pos_offset.y = 0.0f;					\
            pos_offset.z = -obj_aircraft_ptr->belly_height;             \
            /* Create sparks */                                         \
            SmokeCreateSparks(                                          \
                scene, &core_ptr->object, &core_ptr->total_objects,	\
                &obj_ptr->pos, &pos_offset,				\
                cb->contact_radius * 2.0f,				\
                obj_num,						\
                cur_millitime + life_span				\
                );							\
        }                                                               \
        /* Play sound */                                                \
        if(opt->event_sounds)                                           \
            SARSoundSourcePlayFromList(                                 \
                core_ptr->recorder,					\
                obj_ptr->sndsrc, obj_ptr->total_sndsrcs,		\
                camera_in_cockpit ? "land_belly_inside" : "land_belly", \
                pos, dir, ear_pos					\
                );							\
    }

/* Plays landed on skis with minimal scraping sound */
#define DO_EFFECTS_LAND_SKIS                                            \
    { if(opt->event_sounds)                                             \
            SARSoundSourcePlayFromList(                                 \
                core_ptr->recorder,					\
                obj_ptr->sndsrc, obj_ptr->total_sndsrcs,		\
                camera_in_cockpit ? "land_ski_inside" : "land_ski",	\
                pos, dir, ear_pos					\
                );							\
    }

/* Creates sparks and play landed on skis with hard scraping sound */
#define DO_EFFECTS_LAND_SKIS_SKID                                       \
    { if((obj_aircraft_ptr != NULL) && (cb != NULL)) {                  \
            const time_t life_span = 4000l;                             \
            sar_position_struct pos_offset;                             \
            pos_offset.x = 0.0f;					\
            pos_offset.y = 0.0f;					\
            pos_offset.z = (float)-(                                    \
                obj_aircraft_ptr->belly_height +			\
                obj_aircraft_ptr->gear_height                           \
                );							\
            /* Create sparks */                                         \
            SmokeCreateSparks(                                          \
                scene, &core_ptr->object, &core_ptr->total_objects,	\
                &obj_ptr->pos, &pos_offset,				\
                cb->contact_radius * 2.0f,				\
                obj_num,						\
                cur_millitime + life_span				\
                );							\
        }                                                               \
        /* Play Sound */                                                \
        if(opt->event_sounds)                                           \
            SARSoundSourcePlayFromList(                                 \
                core_ptr->recorder,					\
                obj_ptr->sndsrc, obj_ptr->total_sndsrcs,		\
                camera_in_cockpit ? "land_ski_skid_inside" : "land_ski_skid", \
                pos, dir, ear_pos					\
                );							\
    }

/* Creates smoke puffs at each landing gear and plays the wheel skid
 * sound
 */
#define DO_EFFECTS_LAND_WHEEL_SKID                                      \
    { if(obj_aircraft_ptr != NULL) {                                    \
            const time_t life_span = 3500l;                             \
            int i;                                                      \
            sar_obj_part_struct *part;                                  \
            for(i = 0; True; i++)	{				\
                part = SARObjGetPartPtr(				\
                    obj_ptr, SAR_OBJ_PART_TYPE_LANDING_GEAR, i		\
                    );							\
                if(part == NULL)					\
                    break;						\
                /* Create smoke puff at this landing gear */		\
                SmokeCreate(						\
                    scene, &core_ptr->object, &core_ptr->total_objects, \
                    SAR_SMOKE_TYPE_SMOKE,				\
                    &obj_ptr->pos, &part->pos_max,			\
                    0.25f, 1.5f,/* Radius min and max */		\
                    -1.0f,	/* Autocalc growth */			\
                    1,		/* Hide at max? */			\
                    1,		/* Total units */			\
                    life_span,	/* Respawn interval in ms */		\
                    SAR_STD_TEXNAME_SMOKE_LIGHT,			\
                    -1,		/* No reference object */		\
                    cur_millitime + life_span				\
                    );							\
            }                                                           \
        }                                                               \
        /* Play sound */                                                \
        if(opt->event_sounds)                                           \
            SARSoundSourcePlayFromList(                                 \
                core_ptr->recorder,					\
                obj_ptr->sndsrc, obj_ptr->total_sndsrcs,		\
                camera_in_cockpit ? "land_wheel_skid_inside" : "land_wheel_skid", \
                pos, dir, ear_pos					\
                );							\
    }

/* Plays the crash on ground sound */
#define DO_EFFECTS_CRASH_GROUND				\
    { if(opt->event_sounds)                             \
            SARSoundSourcePlayFromList(                 \
                core_ptr->recorder,                     \
                scene->sndsrc, scene->total_sndsrcs,    \
                "crash_ground",                         \
                pos, dir, ear_pos                       \
                );                                      \
    }

/* Plays the splash on water sound */
#define DO_EFFECTS_SPLASH_AIRCRAFT			\
    { if(opt->event_sounds)				\
            SARSoundSourcePlayFromList(                 \
                core_ptr->recorder,                     \
                scene->sndsrc, scene->total_sndsrcs,    \
                "splash_aircraft",                      \
                pos, dir, ear_pos                       \
                );                                      \
    }


    /* Match object from FDM */
    obj_ptr = SARSimMatchObjectFromFDM(
        core_ptr->object, core_ptr->total_objects,
        model, &obj_num
	);
    if(obj_ptr == NULL)
        return;

    /* Skip touch down check entirly if in slew mode */
    if(SARSimIsSlew(obj_ptr))
        return;

    /* Check if this object landed over water */
    SARGetGHCOverWater(
        core_ptr, scene,
        &core_ptr->object, &core_ptr->total_objects,
        obj_num,
        NULL, &over_water
	);

    /* Get up to date object position from the FDM */
    pos = &obj_ptr->pos;
    pos->x = (float)model->position.x;
    pos->y = (float)model->position.y;
    pos->z = (float)model->position.z;

    ear_pos = &scene->ear_pos;

    /* Get up to date object direction from the FDM */
    dir = &obj_ptr->dir;
    dir->heading = (float)model->direction.heading;
    dir->pitch = (float)model->direction.pitch;
    dir->bank = (float)model->direction.bank;

    /* Get contact bounds */
    cb = obj_ptr->contact_bounds;
    if(cb != NULL)
    {
        crash_flags = cb->crash_flags;
        contact_radius = SARSimGetFlatContactRadius(obj_ptr);
    }

    /* Calculate distance between the object and the camera/ear */
    distance_to_camera = (float)SFMHypot3(
        pos->x - ear_pos->x,
        pos->y - ear_pos->y,
        pos->z - ear_pos->z
	);

    /* Check if camera is inside the cockpit */
    switch(scene->camera_ref)
    {
        case SAR_CAMERA_REF_COCKPIT:
	    camera_in_cockpit = True;
	    break;
        case SAR_CAMERA_REF_SPOT:
        case SAR_CAMERA_REF_TOWER:
        case SAR_CAMERA_REF_MAP:
        case SAR_CAMERA_REF_HOIST:
	    break;
    }


    /* Get object type specific values */
    switch(obj_ptr->type)
    {
        case SAR_OBJ_TYPE_GARBAGE:
        case SAR_OBJ_TYPE_STATIC:
        case SAR_OBJ_TYPE_AUTOMOBILE:
        case SAR_OBJ_TYPE_WATERCRAFT:
	    break;

        case SAR_OBJ_TYPE_AIRCRAFT:
	    obj_aircraft_ptr = SAR_OBJ_GET_AIRCRAFT(obj_ptr);
	    if(obj_aircraft_ptr != NULL)
	    {
		int i;
		const sar_obj_part_struct *gear;

		vel = &obj_aircraft_ptr->vel;
		vel->x = (float)model->velocity_vector.x;
		vel->y = (float)model->velocity_vector.y;
		vel->z = (float)model->velocity_vector.z;
		air_worthy_state = obj_aircraft_ptr->air_worthy_state;

		/* Check if this object has floats */
		for(i = 0; True; i++)
		{
		    gear = SARObjGetPartPtr(
			obj_ptr, SAR_OBJ_PART_TYPE_LANDING_GEAR, i
                        );
		    if(gear == NULL)
			break;

		    if(gear->flags & SAR_OBJ_PART_FLAG_LGEAR_FLOATS)
			have_floats = True;
		}
	    }
	    break;

        case SAR_OBJ_TYPE_GROUND:
        case SAR_OBJ_TYPE_RUNWAY:
        case SAR_OBJ_TYPE_HELIPAD:
        case SAR_OBJ_TYPE_HUMAN:
        case SAR_OBJ_TYPE_SMOKE:
        case SAR_OBJ_TYPE_FIRE:
        case SAR_OBJ_TYPE_EXPLOSION:
        case SAR_OBJ_TYPE_CHEMICAL_SPRAY:
        case SAR_OBJ_TYPE_FUELTANK:
        case SAR_OBJ_TYPE_PREMODELED:
	    break;
    }
    /* Substructure data pointer should now be set */

    /* Values for crash_cause are:
     *
     *	5	Hit ground but was not flyable
     *	4	Splash
     *	3	Hit ground but was out of control
     *	2	Landed at bad angle
     *	1	Hit ground too hard
     *	0	No crash
     */

    /* If object was already in flight but was not flyable
     * (ie it hit a building and is falling to the ground),
     * then contacting the ground is a crash.
     */
    if(air_worthy_state == SAR_AIR_WORTHY_NOT_FLYABLE)
    {
        /* Already crashed and is considered a pile of junk */
        crash_cause = 5;
    }
    else if(air_worthy_state == SAR_AIR_WORTHY_OUT_OF_CONTROL)
    {
        /* Collided with building, overspeed damage, or some other
         * prior damage
         */
        crash_cause = 3;
    }
    /* Impact tolorance exceeded? */ 
    else if(impact_coeff > 1.0f)
    {
        /* Aircraft was in flyable condition but now has contacted
         * ground with an impact greater than it's tolorance, which
         * means that it has crashed
         */
        crash_cause = 1;
    }
    /* Landed in water and does not have floats? */
    else if(over_water && !have_floats)
    {
        /* Splash */
        crash_cause = 4;
    }
    /* Contacted ground at bad angle? */
    else
    {
        if(dir->pitch > (float)(1.0 * PI))
        {
            if(dir->pitch < (float)(1.75 * PI))
                crash_cause = 2;
        }
        else
        {
            if(dir->pitch > (float)(0.25 * PI))
                crash_cause = 2;
        }
	    
        if(dir->bank > (float)(1.0 * PI))
        {
            if(dir->bank < (float)(1.75 * PI))
                crash_cause = 2;
        }
        else
        {
            if(dir->bank > (float)(0.25 * PI))
                crash_cause = 2;
        }
    }

#if 0
    printf(
        "Touch down: crash_cause=%i impact_coeff=%.4f\n",
        crash_cause, impact_coeff
        );
#endif

    /* Did the object contact the ground abnormally (did it
     * "crash")?
     */
    if(crash_cause)
    {
        float explosion_radius;
        sar_position_struct explosion_pos;
        char text[1024];

        /* Set object as crashed in accordance with its type */
        /* Aircraft? */
        if(obj_aircraft_ptr != NULL)
        {
            /* If the crash was a splash then mark the object that
             * it is on water
             */
            if(over_water)
                obj_aircraft_ptr->on_water = 1;

            /* Set all appropriate values for the aircraft to show
             * that it is crashed
             */
            SARSimSetAircraftCrashed(
                scene, &core_ptr->object, &core_ptr->total_objects,
                obj_ptr, obj_aircraft_ptr
		);
        }
        else
        {
/* Add support for other types of objects that can crash */

        }

        /* Is the crashed object the player object? */
        if(obj_ptr == scene->player_obj_ptr)
        {
            /* Mark the player object as crashed on scene */
            scene->player_has_crashed = True;

            /* Check if object has not crashed into anything
             * prior, such as if the object's airworth state was
             * not set to SAR_AIR_WORTHY_NOT_FLYABLE or
             * SAR_AIR_WORTHY_OUT_OF_CONTROL
             */
            if((crash_cause != 3) && (crash_cause != 5))
            {
                /* Set banner message to display the cause of the
                 * crash
                 */
                SARBannerMessageAppend(scene, NULL);
                SARBannerMessageAppend(scene, SAR_MESG_CRASH_BANNER);
                switch(crash_cause)
                {
                    case 4:	/* Splash */
			strcpy(text, SAR_MESG_CRASH_SPLASH);
			SARBannerMessageAppend(scene, text);
			break;

                    case 2:	/* Landed at bad angle */
			strcpy(text, SAR_MESG_CRASH_ROTATION_TOO_STEEP);
			SARBannerMessageAppend(scene, text);
			break;

                    case 1:	/* Hit ground too hard */
			/* There should be exactly one occurance of
			 * "%.0f%%" in
			 * SAR_MESG_CRASH_IMPACTED_PAST_TOLORANCE for
			 * the substitution to work
			 */
			sprintf(
			    text,
			    SAR_MESG_CRASH_IMPACTED_PAST_TOLORANCE,
			    (float)(impact_coeff * 100)
                            );
			SARBannerMessageAppend(scene, text);
			break;
                }

                /* Set footer to indicate what player should do
                 * now, press space to continue
                 */
                SARBannerMessageAppend(scene, SAR_MESG_POST_CRASH_BANNER);
            }
        }
        else
        {
            /* An object other than the player object has
             * crashed
             */
            char numstr[80];

            sprintf(numstr, "Object #%i", obj_num);

            *text = '\0';
            strcat(text, "*** ");
            strncat(text, (obj_ptr->name != NULL) ?
		    obj_ptr->name : numstr,
		    80
		);
            strcat(text, " has crashed! ***");

            SARMessageAdd(scene, text);
        }


        /* Get position and size for the creation of the explosion */
        memcpy(&explosion_pos, &obj_ptr->pos, sizeof(sar_position_struct));
        explosion_radius = (float)MAX(contact_radius * 1.8, 10.0);

        /* Begin checking if we should create explosion or splash */

        /* Check if object has not crashed into anything
         * prior, such as if the object's airworth state was
         * not set to SAR_AIR_WORTHY_NOT_FLYABLE or
         * SAR_AIR_WORTHY_OUT_OF_CONTROL
         */
        if((crash_cause != 3) && (crash_cause != 5))
        {
            /* Just crashed into ground and did not crash into
             * anything prior or was not out of control, so we
             * should create an explosion
             */
            int i, explosion_obj_num = -1;
            sar_object_struct *explosion_obj_ptr = NULL;
            sar_external_fueltank_struct *eft_ptr;
            float fuel_remaining = 10.0f;	/* Assume some fuel, in kg */

            /* Get amount of fuel remaining on this object */
            if(obj_aircraft_ptr != NULL)
            {
                fuel_remaining = MAX(obj_aircraft_ptr->fuel, 0.0f);
                for(i = 0; i < obj_aircraft_ptr->total_external_fueltanks; i++)
                {
                    eft_ptr = obj_aircraft_ptr->external_fueltank[i];
                    if((eft_ptr != NULL) ?
                       (eft_ptr->flags & SAR_EXTERNAL_FUELTANK_FLAG_ONBOARD) : False
			)
                        fuel_remaining += (eft_ptr->dry_mass + eft_ptr->fuel);
                }
            }
            /* Has fuel and not over water? */
            if((fuel_remaining > 0.0f) && !over_water)
            {
                /* Create explosion because this object crashed with
                 * fuel
                 */
                sar_position_struct smoke_offset;

                /* Delete all effects objects related to this object
                 * but only stop smoke trails from respawning
                 */
                SARSimDeleteEffects(
                    core_ptr, scene,
                    &core_ptr->object, &core_ptr->total_objects,
                    obj_num,
                    SARSIM_DELETE_EFFECTS_SMOKE_STOP_RESPAWN
		    );
                /* Create explosion object */
                explosion_obj_num = ExplosionCreate(
                    core_ptr, scene,
                    &core_ptr->object, &core_ptr->total_objects,
                    &explosion_pos,     /* Position of explosion */
                    explosion_radius,   /* Radius of size in meters */
                    obj_num,            /* Reference object number */
                    SAR_STD_TEXNAME_EXPLOSION, SAR_STD_TEXNAME_EXPLOSION_IR
		    );
                if(explosion_obj_num > -1)
                {
                    sar_object_explosion_struct *obj_explosion_ptr;

                    explosion_obj_ptr = core_ptr->object[explosion_obj_num];
                    obj_explosion_ptr = SAR_OBJ_GET_EXPLOSION(explosion_obj_ptr);
                    if(obj_explosion_ptr != NULL)
                    {
                        /* Set lifespan for explosion */
                        explosion_obj_ptr->life_span = cur_millitime +
                            opt->crash_explosion_life_span;

                        /* Repeat frames until life span is reached */
                        obj_explosion_ptr->total_frame_repeats = -1;
                    }
                }
                /* Create smoke trails object */
                smoke_offset.x = 0.0f;
                smoke_offset.y = 0.0f;
                smoke_offset.z = explosion_radius;
                SmokeCreate(
                    scene, &core_ptr->object, &core_ptr->total_objects,
                    SAR_SMOKE_TYPE_SMOKE,
                    &explosion_pos, &smoke_offset,
                    explosion_radius * 1.0f,	/* Radius start */
                    explosion_radius * 3.0f,	/* Radius max */
                    -1.0f,				/* Autocalc growth */
                    1,				/* Hide at max */
                    10,				/* Total units */
                    3000l,				/* Respawn interval in ms */
                    SAR_STD_TEXNAME_SMOKE_DARK,
                    obj_num,
                    cur_millitime + opt->crash_explosion_life_span
		    );
            }
            /* Splash? */
            else if(over_water)
            {
                /* Delete all effects objects related to this object
                 * but only stop smoke trails from respawning
                 */
                SARSimDeleteEffects(
                    core_ptr, scene,
                    &core_ptr->object, &core_ptr->total_objects,
                    obj_num,
                    SARSIM_DELETE_EFFECTS_SMOKE_STOP_RESPAWN
		    );
                /* Create splash */
                explosion_obj_num = SplashCreate(
                    core_ptr, scene,
                    &core_ptr->object, &core_ptr->total_objects,
                    &explosion_pos,     /* Position of explosion */
                    explosion_radius,   /* Radius of size in meters */
                    obj_num,            /* Reference object number */
                    SAR_STD_TEXNAME_SPLASH, SAR_STD_TEXNAME_SPLASH
		    );
                if(explosion_obj_num > -1)
                {
                    explosion_obj_ptr = core_ptr->object[explosion_obj_num];

                    /* No need to modify splash values */
                }
            }

            /* Is this the player object? */
            if(scene->player_obj_ptr == obj_ptr)
            {
                /* Set spot camera position */
                scene->camera_ref = SAR_CAMERA_REF_SPOT;
                scene->camera_target = scene->player_obj_num;
            }
        }
        /* Was out of control? */
        else if(crash_cause == 3)
        {
            /* Object was out of control and has now just hit the
             * ground so it needs to crash
             */
            int explosion_obj_num = -1;
            sar_object_struct *explosion_obj_ptr = NULL;

            /* We still need to create a splash if landed on water */
            if(over_water)
            {
                /* Delete all effects objects related to this object
                 * but only stop smoke trails from respawning
                 */
                SARSimDeleteEffects(
                    core_ptr, scene,
                    &core_ptr->object, &core_ptr->total_objects,
                    obj_num,
                    SARSIM_DELETE_EFFECTS_SMOKE_STOP_RESPAWN
		    );
                /* Create splash (explosion) object */
                explosion_obj_num = SplashCreate(
                    core_ptr, scene,
                    &core_ptr->object, &core_ptr->total_objects,
                    &explosion_pos,     /* Position of explosion */
                    explosion_radius,   /* Radius of size in meters */
                    obj_num,            /* Reference object number */
                    SAR_STD_TEXNAME_SPLASH, SAR_STD_TEXNAME_SPLASH
		    );
                if(explosion_obj_num > -1)
                {
                    explosion_obj_ptr = core_ptr->object[explosion_obj_num];

                    /* No need to modify splash values */
                }
            }

            /* Is this the player object? */
            if(scene->player_obj_ptr == obj_ptr)
            {
                /* Set spot camera position */
                scene->camera_ref = SAR_CAMERA_REF_SPOT;
                scene->camera_target = scene->player_obj_num;
            }
        }

        /* Play splash or explosion sound if the object
           was not crashed before (cause 5)*/
        if (crash_cause != 5) {
            if(over_water)
            {
                DO_EFFECTS_SPLASH_AIRCRAFT
                    }
            else
            {
                DO_EFFECTS_CRASH_GROUND
                    }
        }

        /* Call mission destroy notify instead of mission land
         * notify, to let mission know this object has crashed
         */
        SARMissionDestroyNotify(core_ptr, obj_ptr);
    }
    else
    {
        /* Safe landing (no crash) */
        sar_obj_part_struct *lgear_ptr = SARObjGetPartPtr(
            obj_ptr, SAR_OBJ_PART_TYPE_LANDING_GEAR, 0
	    );

        /* Handle safe landing by object type */
        /* Aircraft? */
        if(obj_aircraft_ptr != NULL)
        {
	    double land_speed = SFMHypot2(obj_aircraft_ptr->vel.x, obj_aircraft_ptr->vel.y);

            /* Mark as on water? */
            if(over_water)
                obj_aircraft_ptr->on_water = 1;
            else
                obj_aircraft_ptr->on_water = 0;

            /* First landing gear down? */
            if((lgear_ptr != NULL) ?
               (lgear_ptr->flags & SAR_OBJ_PART_FLAG_STATE) : False
		)
            {
                /* Landed with gear down */

                /* First landing gear is a ski? */
                if((lgear_ptr != NULL) ?
                   (lgear_ptr->flags & SAR_OBJ_PART_FLAG_LGEAR_SKI) : False
		    )
                {
                    /* Landing gear is a ski, check if landed
                     * at velocity great enough to cause scrape
                     */
                    if(land_speed > SFMMPHToMPC(5))
                    {
                        DO_EFFECTS_LAND_SKIS_SKID
                            } 
                    else
                    {
                        DO_EFFECTS_LAND_SKIS
                            }
                }
                else
                {
                    /* Landing gear is a wheel */

                    /* landed with brakes on? */
                    if(obj_aircraft_ptr->wheel_brakes_state > 0)
                    {
                        if(land_speed > SFMMPHToMPC(10))
                        {
                            DO_EFFECTS_LAND_WHEEL_SKID
                                }
                    }
                    else
                    {
                        if(land_speed > SFMMPHToMPC(25))
                        {
                            DO_EFFECTS_LAND_WHEEL_SKID
                                }
                    }
                }
            }
            else
            {
                /* Landed on belly, check if landed at velocity
                 * great enough to cause scrape.
                 */
                if(land_speed > SFMMPHToMPC(5))
                {
                    DO_EFFECTS_LAND_BELLY
                        }
                else
                {
                    DO_EFFECTS_LAND_SKIS
                        }
            }
        }
        /* Other object type */
        else
        {
/* Add support for safe landing of other object types here */

        }
    }

#undef DO_EFFECTS_LAND_BELLY
#undef DO_EFFECTS_LAND_SKIS
#undef DO_EFFECTS_LAND_SKIS_SKID
#undef DO_EFFECTS_LAND_WHEEL_SKID
#undef DO_EFFECTS_CRASH_GROUND
#undef DO_EFFECTS_SPLASH_AIRCRAFT
}

/*
 *	FDM Park callback.
 *
 *	Called whenever the FDM is parked (landed and speed reaches 0).
 *
 *	This notifies the mission to verify if we have reached
 *	an objective. Figuring out where we landed is an operation
 *      quite expensive to run.
 */
void SARSimParkedCB(
    SFMModelStruct *model,
    void *client_data
    )
{
    int obj_num, *gcc_list, gcc_list_total = 0;
    sar_object_struct *obj_ptr;
    sar_scene_struct *scene;
    sar_core_struct *core_ptr = SAR_CORE(client_data);

    scene = core_ptr->scene;
    if(scene == NULL)
        return;

    // No mission success.
    if(scene->player_has_crashed)
	return;

    /* Match object from FDM */
    obj_ptr = SARSimMatchObjectFromFDM(
        core_ptr->object, core_ptr->total_objects,
        model, &obj_num
	);
    if(obj_ptr == NULL)
        return;

    /* Get list of objects at this object's position */
    /* EXPENSIVE */
    gcc_list = SARGetGCCHitList(
        core_ptr, scene,
        &core_ptr->object, &core_ptr->total_objects,
        obj_num,
        &gcc_list_total
	);
    if(gcc_list == NULL)
	return;

     /* Call mission land notify */
    SARMissionLandNotify(core_ptr, obj_ptr, gcc_list, gcc_list_total);
    free(gcc_list);
    gcc_list = NULL;
    gcc_list_total = 0;
}

/*
 *	FDM Overspeed callback.
 *
 *	Called whenever the FDM is exceeding its maximum expected
 *	speed.
 *
 *	This function may be called quite often.
 */
void SARSimOverspeedCB(
    void *realm_ptr, SFMModelStruct *model,
    void *client_data, double cur_speed,
    double overspeed_expected,
    double overspeed
    )
{
    int obj_num;
    sar_object_struct *obj_ptr;
    sar_object_aircraft_struct *obj_aircraft_ptr;
    sar_contact_bounds_struct *cb;
    sar_scene_struct *scene;
    SFMRealmStruct *realm = SFM_REALM(realm_ptr);
    sar_core_struct *core_ptr = SAR_CORE(client_data);
    if((realm == NULL) || (model == NULL) || (core_ptr == NULL))
        return;

    scene = core_ptr->scene;
    if(scene == NULL)
        return;

    /* Match object from FDM */ 
    obj_ptr = SARSimMatchObjectFromFDM(
        core_ptr->object, core_ptr->total_objects,
        model, &obj_num
	);
    if(obj_ptr == NULL)
        return;

    /* Skip overspeed check entirly if in slew mode */
    if(SARSimIsSlew(obj_ptr))
        return;

    /* Handle by object type */
    switch(obj_ptr->type)
    {
        case SAR_OBJ_TYPE_GARBAGE:
        case SAR_OBJ_TYPE_STATIC:
        case SAR_OBJ_TYPE_AUTOMOBILE:
        case SAR_OBJ_TYPE_WATERCRAFT:
	    break;

        case SAR_OBJ_TYPE_AIRCRAFT:
	    obj_aircraft_ptr = SAR_OBJ_GET_AIRCRAFT(obj_ptr);
	    if(obj_aircraft_ptr != NULL)
	    {
		/* Current speed has exceeded actual overspeed? */
		if(cur_speed > overspeed)
		{
		    /* Is aircraft still flyable? */
		    if(obj_aircraft_ptr->air_worthy_state == SAR_AIR_WORTHY_FLYABLE)
		    {
			float contact_radius = SARSimGetFlatContactRadius(obj_ptr);


			/* Set it to be out of control */
			obj_aircraft_ptr->air_worthy_state = SAR_AIR_WORTHY_OUT_OF_CONTROL;

			/* Reduce to 10% hit points (as needed) */
			if(obj_ptr->hit_points > (obj_ptr->hit_points_max * 0.10f))
			    obj_ptr->hit_points = obj_ptr->hit_points_max * 0.10f;

			/* Get pointer to object's contact bounds structure */
			cb = obj_ptr->contact_bounds;
			if(cb != NULL)
			{
			    /* Remove SAR_CRASH_FLAG_CRASH_OTHER flag from
			     * object's crash flags so that this object
			     * won't crash into other objects from now on.
			     *
			     * Note that this will not create an explosion when
			     * it hits the ground.
			     */
			    cb->crash_flags &= ~SAR_CRASH_FLAG_CRASH_OTHER;
			}

			/* Delete all effects objects related to this object */
			SARSimDeleteEffects(
			    core_ptr, scene,
			    &core_ptr->object, &core_ptr->total_objects,
			    obj_num,
			    0
                            );
			/* Create smoke trails object */
			SmokeCreate(
			    scene, &core_ptr->object, &core_ptr->total_objects,
			    SAR_SMOKE_TYPE_SMOKE,
			    &obj_ptr->pos, NULL,
			    contact_radius * 0.25f,	/* Radius start */
			    contact_radius * 2.5f,	/* Radius max */
			    -1.0f,			/* Autocalc growth */
			    1,				/* Hide at max */
			    15,				/* Total units */
			    500,			/* Respawn interval in ms */
			    SAR_STD_TEXNAME_SMOKE_MEDIUM,
			    obj_num,
			    0				/* Life span */
                            );

			/* Is this the player object? */
			if(scene->player_obj_ptr == obj_ptr)
			{
			    /* Mark player object as has crashed */
			    scene->player_has_crashed = True;

			    /* Set overspeed crash banner */
			    SARBannerMessageAppend(scene, NULL);
 			    SARBannerMessageAppend(scene, SAR_MESG_CRASH_OVERSPEED_BANNER);
#if 0
/* Do not post footer, pilot may still want to land */
			    SARBannerMessageAppend(scene, SAR_MESG_POST_CRASH_BANNER);
                            */
#endif
                                /* Set spot camera position */
                                scene->camera_ref = SAR_CAMERA_REF_SPOT;
                                scene->camera_target = scene->player_obj_num;
			}

		    }

#if 0
/* Do not call mission destroy notify, pilot may still want to land */    
		    SARMissionDestroyNotify(core_ptr, obj_ptr);
#endif
		}
	    }
	    break;

        case SAR_OBJ_TYPE_GROUND:
        case SAR_OBJ_TYPE_RUNWAY:
        case SAR_OBJ_TYPE_HELIPAD:
        case SAR_OBJ_TYPE_HUMAN:
        case SAR_OBJ_TYPE_SMOKE:
        case SAR_OBJ_TYPE_FIRE:
        case SAR_OBJ_TYPE_EXPLOSION:
        case SAR_OBJ_TYPE_CHEMICAL_SPRAY:
        case SAR_OBJ_TYPE_FUELTANK:
        case SAR_OBJ_TYPE_PREMODELED:
	    break;
    }
}


/*
 *	FDM midair collision callback. This function won't be called
 *	by the SFM library (since all crash detection values passed
 *	to the SFM are False), instead SAR will do collision detection
 *	outside of the SFM level.
 */
void SARSimCollisionCB(
    void *realm_ptr, SFMModelStruct *model, SFMModelStruct *obstruction,
    void *client_data, double impact_coeff
    )
{
    /* Ignore this callback, SAR uses its own crash detection 
     * callback functions.
     */
}

/*
 *      Same as SARSimCollisionCB except that it takes inputs in
 *	the form of object pointers, this is for collision detection
 *	on our side (not using the SFM collision check callback).
 */
void SARSimObjectCollisionCB(
    void *client_data,		/* Core structure */
    sar_object_struct *obj_ptr, sar_object_struct *obstruction_obj_ptr,
    double impact_coeff
    )
{
    int n, ref_object = -1;
    float r, distance_to_camera;
    float	tower_offset_x = 50.0f,
        tower_offset_y = 50.0f,
        tower_offset_z = 50.0f;
    sar_position_struct *pos, *ear_pos, explosion_pos, smoke_offset;
    sar_direction_struct *dir;
    sar_scene_struct *scene;
    sar_object_aircraft_struct *obj_aircraft_ptr;
    sar_contact_bounds_struct *cb;
    sar_core_struct *core_ptr = SAR_CORE(client_data);
    const sar_option_struct *opt;
    if((core_ptr == NULL) || (obj_ptr == NULL) ||
       (obstruction_obj_ptr == NULL)
	)
        return;

    opt = &core_ptr->option;

    scene = core_ptr->scene;
    if(scene == NULL)
        return;

    /* Skip collision check entirly if in slew mode */
    if(SARSimIsSlew(obj_ptr))
        return;


#define DO_PLAY_CRASH_OBSTRUCTION			\
    { if(opt->event_sounds)				\
            SARSoundSourcePlayFromList(                 \
                core_ptr->recorder,                     \
                scene->sndsrc, scene->total_sndsrcs,    \
                "crash_obstruction",                    \
                pos, dir, ear_pos                       \
                );                                      \
    }

    /* Get pointer to position and direction of victim object */
    pos = &obj_ptr->pos;
    dir = &obj_ptr->dir;

    /* Get pointer to position of ear */
    ear_pos = &scene->ear_pos;

    /* Calculate distance to camera */
    distance_to_camera = (float)SFMHypot3(
        pos->x - ear_pos->x,
        pos->y - ear_pos->y,
        pos->z - ear_pos->z
	);


    /* Check if obstruction object (the `crashed into' object) is
     * valid.
     */
    if(obstruction_obj_ptr != NULL)
    {
        float bearing, bearing_obstruction, bearing_velocity;
        float theta, theta_absolute, tower_offset;
        sar_position_struct *pos2, *vel;


        /* Get pointer to obstruction object's position */
        pos2 = &obstruction_obj_ptr->pos;

        /* Begin setting up victim object's values to mark that it has
         * crashed into the obstruction object
         */

        /* Change some values on victim object by its type, do not
         * set object as fully crashed since it's probably
         * plumitting to the ground.
         */
        switch(obj_ptr->type)
        {
            case SAR_OBJ_TYPE_GARBAGE:
            case SAR_OBJ_TYPE_STATIC:
            case SAR_OBJ_TYPE_AUTOMOBILE:
            case SAR_OBJ_TYPE_WATERCRAFT:
		break;

            case SAR_OBJ_TYPE_AIRCRAFT:
                obj_aircraft_ptr = SAR_OBJ_GET_AIRCRAFT(obj_ptr);
		if(obj_aircraft_ptr == NULL)
		    break;

		vel = &obj_aircraft_ptr->vel;

		/* Calculate bearing of victim object to obstruction
		 * object in world coordinates.
		 */
		bearing = (float)SFMSanitizeRadians(
		    (0.5 * PI) - atan2(pos2->y - pos->y, pos2->x - pos->x)
                    );
		/* Calculate bearing from world coordinates to
		 * the victim object coordinates.
		 */
		bearing_obstruction = (float)SFMSanitizeRadians(
		    bearing - dir->heading
                    );

		/* Get direction of velocity relative to victim object */
		bearing_velocity = (float)SFMSanitizeRadians(
		    (0.5 * PI) - atan2(vel->y, vel->x)
                    );

		/* Calculate angle from bearing_velocity to
		 * bearing_obstruction.
		 */
		theta = (float)SFMDeltaRadians(
		    bearing_velocity, bearing_obstruction
                    );
		theta_absolute = ((theta < 0.0f) ? -theta : theta);

		/* If the absolute angle of velocity to direction of 
		 * obstruction object is less than (0.5 * PI) then that
		 * means victim object has hit the obstruction object
		 * and that its velocity needs to be changed
		 */
		if(theta_absolute < (0.5 * PI))
		{
		    double a[3], r[3];	/* r is overloaded */
		    SFMModelStruct *fdm = obj_aircraft_ptr->fdm;
		    if(fdm != NULL)
		    {
#define TAR_PTR fdm
			a[0] = TAR_PTR->velocity_vector.x;
			a[1] = TAR_PTR->velocity_vector.y;
			a[2] = TAR_PTR->velocity_vector.z;
			MatrixRotateHeading3(
			    a,
			    2.0 * (theta - (0.5 * PI)),
			    r
                            );
			TAR_PTR->velocity_vector.x = r[0] * 0.1;
			TAR_PTR->velocity_vector.y = r[1] * 0.1;
			TAR_PTR->velocity_vector.z = r[2] * 0.1;
			SFMSetAirspeed(scene->realm, TAR_PTR);
#undef TAR_PTR
		    }
		}

		/* Begin calculating tower offset */
		tower_offset = (float)MAX(
		    SARSimGetFlatContactRadius(obj_ptr) * 4.0,
		    10.0
                    );
		tower_offset_x = (float)(-sin(bearing) * tower_offset);
		tower_offset_y = (float)(-cos(bearing) * tower_offset);
		tower_offset_z = tower_offset;

		/* Set air worthy state based on if object is already
		 * landed or not
		 */
		if(obj_aircraft_ptr->landed)
		{
		    /* Call SARSimSetAircraftCrashed() procedure since
		     * aircraft is already on ground and won't crash
		     * again
		     */
		    SARSimSetAircraftCrashed(
			scene, &core_ptr->object, &core_ptr->total_objects,
			obj_ptr, obj_aircraft_ptr
                        );
		}
		else
		{
		    /* Aircraft is still flying, mark it as out of
		     * control
		     */
		    obj_aircraft_ptr->air_worthy_state =
			SAR_AIR_WORTHY_OUT_OF_CONTROL;

		    /* Turn engines off */
		    obj_aircraft_ptr->engine_state = SAR_ENGINE_OFF;
		}
		break;

            case SAR_OBJ_TYPE_GROUND:
            case SAR_OBJ_TYPE_RUNWAY:
            case SAR_OBJ_TYPE_HELIPAD:
            case SAR_OBJ_TYPE_HUMAN:
            case SAR_OBJ_TYPE_SMOKE:
            case SAR_OBJ_TYPE_FIRE:
            case SAR_OBJ_TYPE_EXPLOSION:
            case SAR_OBJ_TYPE_CHEMICAL_SPRAY:
            case SAR_OBJ_TYPE_FUELTANK:
            case SAR_OBJ_TYPE_PREMODELED:
		break;
        }
    }	/* if(obstruction_obj_ptr != NULL) */


	/* Get pointer to victim object's contact bounds structure */
    cb = obj_ptr->contact_bounds;
    if(cb != NULL)
    {
        /* Remove SAR_CRASH_FLAG_CRASH_OTHER flag from victim
         * object's crash flags so that the victim object won't
         * crash into other objects from now on.
         */
        cb->crash_flags &= ~SAR_CRASH_FLAG_CRASH_OTHER;
    }


    /* Begin creating explosion */

    /* Get position and values for creating explosion object */
    memcpy(&explosion_pos, &obj_ptr->pos, sizeof(sar_position_struct));
    ref_object = SARGetObjectNumberFromPointer(
        scene, core_ptr->object, core_ptr->total_objects, obj_ptr
	);
    r = (float)MAX(SARSimGetFlatContactRadius(obj_ptr) * 1.8, 10.0);

    /* Delete all effects objects related to this object but only
     * stop smoke trails from respawning.
     */
    SARSimDeleteEffects(
        core_ptr, scene, &core_ptr->object, &core_ptr->total_objects,
        ref_object,
        SARSIM_DELETE_EFFECTS_SMOKE_STOP_RESPAWN
	);
    /* Create explosion object */
    n = ExplosionCreate(
        core_ptr, scene,
        &core_ptr->object, &core_ptr->total_objects,
        &explosion_pos,	/* Position of explosion */
        r,			/* Radius of size in meters */
        ref_object,		/* Reference object number */
        SAR_STD_TEXNAME_EXPLOSION, SAR_STD_TEXNAME_EXPLOSION_IR
	);
    if(n > -1)  
    {
        sar_object_explosion_struct *obj_explosion_ptr;
        sar_object_struct *explosion_obj_ptr = core_ptr->object[n];

        obj_explosion_ptr = SAR_OBJ_GET_EXPLOSION(explosion_obj_ptr);
        if(obj_explosion_ptr != NULL)
        {
            /* Set lifespan for explosion */
            explosion_obj_ptr->life_span = cur_millitime +
                opt->crash_explosion_life_span;

            /* Repeat frames until life span is reached */
            obj_explosion_ptr->total_frame_repeats = -1;
        }
    }
    /* Create smoke trails object */
    smoke_offset.x = 0.0f;
    smoke_offset.y = 0.0f;
    smoke_offset.z = r;
    SmokeCreate(
        scene, &core_ptr->object, &core_ptr->total_objects,
        SAR_SMOKE_TYPE_SMOKE,
        &explosion_pos, &smoke_offset,
        r * 1.0f,		/* Radius start */
        r * 3.0f,		/* Radius max */
        -1.0f,		/* Autocalc growth */
        1,			/* Hide at max */
        10,			/* Total units */
        3000,		/* Respawn interval in ms */
        SAR_STD_TEXNAME_SMOKE_DARK,
        ref_object,
        cur_millitime + opt->crash_explosion_life_span
	);

    /* Play crash obstruction sound */
    DO_PLAY_CRASH_OBSTRUCTION


	/* Is this the player object? */
	if(scene->player_obj_ptr == obj_ptr)
	{         
	    char text[128];

	    /* Mark player object as has crashed */
	    scene->player_has_crashed = True;

	    /* If camera reference type is not currently tower or spot
	     * then set camera reference type to tower.
	     */
	    if(((scene->camera_ref == SAR_CAMERA_REF_TOWER) ?
		(scene->camera_target != scene->player_obj_num) : 1
                   ) &&
	       ((scene->camera_ref == SAR_CAMERA_REF_SPOT) ?
		(scene->camera_target != scene->player_obj_num) : 1
                   )
                )
	    {
		/* Set tower position */
		scene->camera_tower_pos.x = obj_ptr->pos.x + tower_offset_x;
		scene->camera_tower_pos.y = obj_ptr->pos.y + tower_offset_y;
		scene->camera_tower_pos.z = obj_ptr->pos.z + tower_offset_z;

		scene->camera_ref = SAR_CAMERA_REF_TOWER;
		scene->camera_target = scene->player_obj_num;
	    }

	    /* Set scene banner to indicate collision and type */
	    SARBannerMessageAppend(scene, NULL);
	    SARBannerMessageAppend(scene, SAR_MESG_COLLISION_BANNER);

	    if(obstruction_obj_ptr != NULL)
	    {
		cb = obstruction_obj_ptr->contact_bounds;
		*text = '\0';
		switch((cb != NULL) ? cb->crash_type : SAR_CRASH_TYPE_OBSTRUCTION)
		{
                    case SAR_CRASH_TYPE_OBSTRUCTION:
                        strcpy(text, SAR_MESG_CRASH_OBSTRUCTION);
                        break;
                    case SAR_CRASH_TYPE_GROUND:
                        /* Never should occure for midair collisions */
                        strcpy(text, SAR_MESG_CRASH_GROUND);
                        break;
                    case SAR_CRASH_TYPE_MOUNTAIN:
                        strcpy(text, SAR_MESG_CRASH_MOUNTAIN);
                        break;
                    case SAR_CRASH_TYPE_BUILDING:
                        strcpy(text, SAR_MESG_CRASH_BOULDING);
                        break;
                    case SAR_CRASH_TYPE_AIRCRAFT:
                        strcpy(text, SAR_MESG_CRASH_AIRCRAFT);
                        break;
                    case SAR_CRASH_TYPE_FIRE:
                        strcpy(text, SAR_MESG_CRASH_FIRE);
                        break;
		}
		if(*text == '\0')
		    sprintf(
			text,
			"UNSUPPORTED COLLISION CODE %i",
			(int)((cb != NULL) ? cb->crash_type : SAR_CRASH_TYPE_OBSTRUCTION)
                        );
		SARBannerMessageAppend(scene, text);
	    }

	    /* Set footer to indicate what pilot should do now */
	    SARBannerMessageAppend(scene, SAR_MESG_POST_CRASH_BANNER);
	}

#undef DO_PLAY_CRASH_OBSTRUCTION
}
