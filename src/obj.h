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
			    SAR Objects and Scene
 */

#ifndef OBJ_H
#define OBJ_H

#include <sys/types.h>

#include "sfm.h"
#include "v3dtex.h"
#include "sound.h"


/*
 *	Object Flags Type:
 */
#define sar_obj_flags_t	unsigned long

/*
 *	Gradient Animation Counter Type:
 */
#define sar_grad_anim_t	u_int16_t
#define SAR_GRAD_ANIM_COEFF(x)	((float)(x) / (float)((sar_grad_anim_t)-1))

/*
 *	DMS Units Type:
 */
#define sar_dms_t	float

/*
 *	Time Of Day Codes:
 */
typedef enum {
	SAR_TOD_CODE_UNDEFINED,
	SAR_TOD_CODE_DAY,
	SAR_TOD_CODE_DAWN,
	SAR_TOD_CODE_NIGHT,
	SAR_TOD_CODE_DUSK
} sar_tod_code;

/*
 *	Camera References:
 */
typedef enum {
	SAR_CAMERA_REF_COCKPIT,
	SAR_CAMERA_REF_SPOT,
	SAR_CAMERA_REF_TOWER,
	SAR_CAMERA_REF_MAP,
	SAR_CAMERA_REF_HOIST
} sar_camera_ref;


/*
 *	Air Worthy States:
 */
typedef enum {
	SAR_AIR_WORTHY_NOT_FLYABLE,	/* Pile of junk */
	SAR_AIR_WORTHY_OUT_OF_CONTROL,
	SAR_AIR_WORTHY_FLYABLE
} sar_air_worthy_state;

/*
 *	Engine States:
 */
typedef enum {
	SAR_ENGINE_OFF,
	SAR_ENGINE_INIT,
	SAR_ENGINE_ON
} sar_engine_state;

/*
 *	Autopilot States:
 */
typedef enum {
	SAR_AUTOPILOT_OFF,
	SAR_AUTOPILOT_ON
} sar_autopilot_state;

/*
 *	Human color palette index and max:
 */
#define SAR_HUMAN_COLOR_FACE            0
#define SAR_HUMAN_COLOR_HAIR            1
#define SAR_HUMAN_COLOR_TORSO           2
#define SAR_HUMAN_COLOR_HIPS            3
#define SAR_HUMAN_COLOR_LEGS            4
#define SAR_HUMAN_COLOR_FEET            5
#define SAR_HUMAN_COLOR_ARMS            6
#define SAR_HUMAN_COLOR_HANDS           7

#define SAR_HUMAN_COLORS_MAX            8


/*
 *	Color:
 *
 *	Each member must be of type float and their order is
 *	important, units are in coefficients from 0.0 to 1.0.
 */
typedef struct {
	float		r, g, b, a;
} sar_color_struct;
#define SAR_COLOR(p)	((sar_color_struct *)(p))


/*
 *	Visual Model:
 *
 *	Values for a GL display list with reference count feature that
 *	allows other objects to share this visual model simply by
 *	incrementing the ref_count.  The GL display list pointer data
 *	reffers to a GLuint.
 *
 *	Members filename and name are used to identify if this visual
 *	model should be shared or not by allowing you to check if
 *	another object uses this same visual model.
 */
typedef enum {
	SAR_VISUAL_MODEL_NOT_LOADED,
	SAR_VISUAL_MODEL_LOADING,
	SAR_VISUAL_MODEL_LOADED
} sar_visual_model_load_state;
typedef struct {

	sar_visual_model_load_state	load_state;

	int		ref_count;	/* Reference count */

	char		*filename;	/* Can be NULL */
	char		*name;		/* Can be NULL */

	void		*data;		/* A GLuint referencing a GL list */

	/* Statistics */
	unsigned long	mem_size;	/* Memory size, in bytes */
	int		statements,	/* Total GL statements */
			primitives;	/* Total GL primitives */

} sar_visual_model_struct;
#define SAR_VISUAL_MODEL(p)	((sar_visual_model_struct *)(p))


/*
 *	Position/Velocity:
 *
 *	Each member must be of type float and their order is 
 *	important.
 *
 *	When used as position the units are in meters, when used as
 *	velocity the units are in meters per cycle.
 */
typedef struct {
	float		x, y, z;
} sar_position_struct;
#define SAR_POSITION(p)		((sar_position_struct *)(p))

/*
 *	Direction:
 *
 *	Each member must be of type float and their order is
 *	important, units are in radians.
 */
typedef struct {
	float		heading, pitch, bank;
} sar_direction_struct;
#define SAR_DIRECTION(p)	((sar_direction_struct *)(p))

/*
 *	Scale:
 *
 *	Each member must be of type float and their order is
 *	important, units are in coefficients.
 */
typedef struct {
	float		x, y, z;
} sar_scale_struct;
#define SAR_SCALE(p)		((sar_scale_struct *)(p))


/*
 *	Intercept:
 */
typedef struct {
	sar_obj_flags_t	flags;		/* Reserved */
	float		x, y, z;	/* Center of intercept, in meters */
	float		radius;		/* Cylendrical size of intercept, in meters */
	float		urgency;	/* Specifies the "urgency" of which this
					 * intercept must be reached (used for a
					 * variety of purposes) */
	char		*name;		/* Intercept name/label */
} sar_intercept_struct;
#define SAR_INTERCEPT(p)	((sar_intercept_struct *)(p))

/*
 *	Wind:
 */
# define SAR_WIND_FLAG_GUSTS (1 << 0)


/*
 *	Light:
 */
typedef struct {

#define SAR_LIGHT_FLAG_ON		(1 << 0)
#define SAR_LIGHT_FLAG_STROBE		(1 << 1)
#define SAR_LIGHT_FLAG_ATTENUATE	(1 << 2)
	sar_obj_flags_t	flags;

	sar_position_struct	pos;
	sar_direction_struct	dir;

	int		radius;		/* In pixels */

	sar_color_struct	color,			/* Appearance color */
				attenuate_color;	/* Attenuate color */

	/* On/off timers (if the SAR_LIGHT_FLAG_STROBE flag is set)
	 * in milliseconds
	 */
	time_t		next_on, int_on,
			next_off, int_off;
	time_t		int_delay_on;	/* Additional interval to wait
					 * before turning light on when
					 * timer is reset */
} sar_light_struct;
#define SAR_LIGHT(p)	((sar_light_struct *)(p))


/*
 *	Contact Bounds:
 *
 *	Note that any object can have contact bounds, but rules apply
 *	to different object types.
 *
 *	Objects of type SAR_OBJ_TYPE_HUMAN and SAR_OBJ_TYPE_AIRCRAFT
 *	should only have crash flag SAR_CRASH_FLAG_CRASH_OTHER set and
 *	it's contact shape must be SAR_CONTACT_SHAPE_CYLENDRICAL.
 *
 *	Other objects that you want to set crash flag
 *	SAR_CRASH_FLAG_CRASH_CAUSE can have any contact shape.
 *
 *	Now objects that you want to be landable or walkable must have
 *	crash flag SAR_CRASH_FLAG_SUPPORT_SURFACE set.
 *
 *	If an object's crash flag has set
 *	SAR_CRASH_FLAG_SUPPORT_SURFACE then the object can only have its
 *	heading rotated.
 */
typedef enum {
    SAR_CRASH_TYPE_OBSTRUCTION,
    SAR_CRASH_TYPE_GROUND,
    SAR_CRASH_TYPE_MOUNTAIN,		/* To be more specific than
                                         * just ground */
    SAR_CRASH_TYPE_BUILDING,
    SAR_CRASH_TYPE_AIRCRAFT,		/* Not used */
    SAR_CRASH_TYPE_FIRE
} sar_crash_type;

typedef enum {
	SAR_CONTACT_SHAPE_SPHERICAL,
	SAR_CONTACT_SHAPE_CYLENDRICAL,
	SAR_CONTACT_SHAPE_RECTANGULAR
} sar_contact_shape;

typedef struct {

#define SAR_CRASH_FLAG_CRASH_OTHER      (1 << 0)	/* Crash into or contact
							 * other objects */
#define SAR_CRASH_FLAG_CRASH_CAUSE      (1 << 1)	/* Other objects can
							 * crash into this
							 * object */
#define SAR_CRASH_FLAG_SUPPORT_SURFACE	(1 << 2)	/* Landable/walkable */
	sar_obj_flags_t	crash_flags;

	/* Crash Type (used only if the Crash Flag
	 * SAR_CRASH_FLAG_CRASH_CAUSE is set), this specifies what type
	 * of crash it would be if an object crashed into this object
	 */
	sar_crash_type	crash_type;

	/* Contact Shape */
	sar_contact_shape	contact_shape;

	/* Contact Radius (in meters) for Contact Shape
	 * SAR_CONTACT_SHAPE_SPHERICAL or SAR_CONTACT_SHAPE_CYLENDRICAL
	 */
	float		contact_radius;

	/* Contact Height (in meters) for Contact Shape
	 * SAR_CONTACT_SHAPE_CYLENDRICAL
	 */
	float		contact_h_min,
			contact_h_max;

	/* Contact Geometry (in meters, left hand rule) for Contact
	 * Shape SAR_CONTACT_SHAPE_RECTANGULAR
	 */
	float		contact_x_min, contact_x_max,
			contact_y_min, contact_y_max,
			contact_z_min, contact_z_max;

	/* Value records for the object's inversed heading for the trig
	 * functions, this is used to speed up rotations about heading
	 * for SAR_CONTACT_SHAPE_RECTANGULAR shape contact
	 *
	 * Ie cos_heading = cos(-heading) and sin_heading = sin(-heading)
	 */
	float		cos_heading,
			sin_heading;

} sar_contact_bounds_struct;
#define SAR_CONTACT_BOUNDS(p)	((sar_contact_bounds_struct *)(p))


/*
 *	Sound Source:
 */
typedef struct {

	char		*name;		/* Arbitary name to identify this
					 * in a list of sound sources */

	char		*filename,	/* Full path to sound object */
			*filename_far;

	float		range,		/* Maximum range (in meters) */
			range_far;	/* Play far sound if beyond this
					 * range (in meters) */

	sar_position_struct	pos;	/* Offset from center of object */

	float		cutoff;		/* Cutoff angle (in radians),
					 * can be 0.0 to specify "in all
					 * directions" */
	sar_direction_struct	dir;	/* Direction, used only if cutoff
					 * is positive */

	/* Sample rate limit, the amount that the sample rate can
	 * increase to
	 *
	 * For example if the sound object's sample rate is 11025 hz
	 * and sample_rate_limit is 30000 hz, then the sound object's
	 * sample rate can be increased from 11025 to 30000 hz
	 */
	int		sample_rate_limit;

} sar_sound_source_struct;
#define SAR_SOUND_SOURCE(p)	((sar_sound_source_struct *)(p))


/*
 *	Rotor/Propellar:
 */
typedef enum {
	SAR_ROTOR_FLAG_SPINS		= (1 << 0),
	SAR_ROTOR_FLAG_CAN_PITCH	= (1 << 1),
	SAR_ROTOR_FLAG_PITCH_STATE	= (1 << 2),	/* Set if pitched forward
							 * (does not determine
							 * flight model type) */
	SAR_ROTOR_FLAG_NO_PITCH_LANDED	= (1 << 3),	/* May not pitch when
							 * landed */
	SAR_ROTOR_FLAG_FOLLOW_CONTROLS	= (1 << 5),	/* Pitches & banks in
							 * response to control
							 * positions */
	SAR_ROTOR_FLAG_BLUR_WHEN_FAST	= (1 << 8),	/* Blur when spinning
							 * fast */
	SAR_ROTOR_FLAG_BLUR_ALWAYS	= (1 << 9)	/* Always blur (overrides
							 * SAR_ROTOR_FLAG_BLUR_WHEN_FAST */
} sar_rotor_flags;
typedef struct {

	sar_rotor_flags	flags;

	sar_position_struct	pos;	/* Relative from center of object */
	sar_direction_struct	dir;	/* Note: rotor spins about the Y axis */

	float		radius;		/* Length of longest blade (in meters) */
	float		blades_offset;	/* Blades offset from rotor (in meters) */
	int		total_blades;	/* Number of blades */

	sar_grad_anim_t	anim_pos;	/* Spin animation position */

	sar_visual_model_struct	*visual_model,
				*visual_model_ir;

	/* Rotor pitch rotate position (0 to (sar_grad_anim_t)-1),
	 * where (sar_grad_anim_t)-1 is pitched forwards
	 * (when SAR_ROTOR_FLAG_PITCH_STATE is set)
	 */
	sar_grad_anim_t	pitch_anim_pos;

	/* Rotor wash animation position */
	sar_grad_anim_t	rotor_wash_anim_pos;

	int		blade_blur_tex_num,	/* Blured blade texture */
			wash_tex_num;		/* Prop wash texture */

	/* Control position coefficients (used only if
	 * SAR_ROTOR_FLAG_FOLLOW_PB is set)
	 */
	float		control_coeff_pitch,
			control_coeff_bank;

	/* Blades blur color (used when blades are spinning fast) */
	sar_color_struct	blades_blur_color;

} sar_obj_rotor_struct;
#define SAR_OBJ_ROTOR(p)	((sar_obj_rotor_struct *)(p))


/*
 *	Object Part:
 *
 *	Flaps, ailerons, elevator, rudder, air brakes, landing gears,
 *	canopies, etc
 */
typedef enum {
	SAR_OBJ_PART_TYPE_AILERON_LEFT,
	SAR_OBJ_PART_TYPE_AILERON_RIGHT,
	SAR_OBJ_PART_TYPE_RUDDER_TOP,
	SAR_OBJ_PART_TYPE_RUDDER_BOTTOM,
	SAR_OBJ_PART_TYPE_ELEVATOR,
	SAR_OBJ_PART_TYPE_CANNARD,			/* Forward elevator */
	SAR_OBJ_PART_TYPE_AILERON_ELEVATOR_LEFT,	/* Combo */
	SAR_OBJ_PART_TYPE_AILERON_ELEVATOR_RIGHT,	/* Combo */
	SAR_OBJ_PART_TYPE_FLAP,
	SAR_OBJ_PART_TYPE_AIR_BRAKE,
	SAR_OBJ_PART_TYPE_DOOR,
	SAR_OBJ_PART_TYPE_DOOR_RESCUE,
	SAR_OBJ_PART_TYPE_CANOPY,
	SAR_OBJ_PART_TYPE_LANDING_GEAR
} sar_obj_part_type;

typedef struct {

	sar_obj_part_type	type;

/* Part Flags (different part types may have the same part flag
 * values)
 */
#define SAR_OBJ_PART_FLAG_STATE		(1 << 0)	/* Set if opened/extended */
#define SAR_OBJ_PART_FLAG_HIDE_MIN	(1 << 1)	/* Hide when animation value is at min */
#define SAR_OBJ_PART_FLAG_HIDE_MAX	(1 << 2)	/* Hide when animation value is at max */
/* Doors, Rescue Doors, and Canopies */
#define SAR_OBJ_PART_FLAG_DOOR_FIXED	(1 << 3)	/* True if fixed (always closed) */
#define SAR_OBJ_PART_FLAG_DOOR_LOCKED	(1 << 4)	/* True if locked */
#define SAR_OBJ_PART_FLAG_DOOR_STAY_OPEN	(1 << 5)	/* Do not close door on
								 * next loop check,
								 * ie if door was opened by player explicitly */
/* Landing Gears */
#define SAR_OBJ_PART_FLAG_LGEAR_FIXED	(1 << 3)	/* Always down */
#define SAR_OBJ_PART_FLAG_LGEAR_DAMAGED	(1 << 4)
#define SAR_OBJ_PART_FLAG_LGEAR_MISSING	(1 << 5)	/* Non-existant */
#define SAR_OBJ_PART_FLAG_LGEAR_SKI	(1 << 6)
#define SAR_OBJ_PART_FLAG_LGEAR_FLOATS	(1 << 7)	/* Can land on water */
	sar_obj_flags_t	flags;

	/* Offset relative to center of object, pos_min and pos_max
	 * are deltas relative to pos_cen
	 *
	 * Note that some part types interprite these values differently
	 */
	sar_position_struct	pos_min,
				pos_cen,
				pos_max;

	/* Direction, dir_min and dir_max are deltas relative to 
	 * dir_cen
	 *
	 * Note that some part types interprite these values differently
	 */
	sar_direction_struct	dir_min,
				dir_cen,
				dir_max;

	sar_grad_anim_t anim_pos,       /* 0 to (sar_grad_anim_t)-1 */
			anim_rate;      /* Units per cycle */

	float		temperature;

	sar_visual_model_struct	*visual_model,
				*visual_model_ir;

} sar_obj_part_struct;
#define SAR_OBJ_PART(p)		((sar_obj_part_struct *)(p))


/*
 *	External Fuel Tank:
 *
 *	This is a fuel tank that is on an object.
 *
 *	See sar_object_fueltank_struct (which is a fuel tank object).
 */
typedef struct {

#define SAR_EXTERNAL_FUELTANK_FLAG_FIXED	(1 << 0)	/* Not droppable */
#define SAR_EXTERNAL_FUELTANK_FLAG_ONBOARD	(1 << 1)	/* Not yet jettesoned */
	sar_obj_flags_t	flags;

	/* Offset from center of object, in meters */
	sar_position_struct	offset_pos;

	/* Spherical contact bounds radius, in meters */
	float		radius;

	/* Belly to center height, in meters */
	float		belly_to_center_height;

	/* Mass and fuel, in kg */
	float		dry_mass,
			fuel,
			fuel_max;

	float		temperature;

	sar_visual_model_struct	*visual_model,
				*visual_model_ir;

} sar_external_fueltank_struct;
#define SAR_EXTERNAL_FUELTANK(p)	((sar_external_fueltank_struct *)(p))


/*
 *	Hoist:
 */
typedef enum {
	SAR_HOIST_DEPLOYMENT_BASKET	= (1 << 0),
	SAR_HOIST_DEPLOYMENT_DIVER	= (1 << 1),
	SAR_HOIST_DEPLOYMENT_HOOK	= (1 << 2)
} sar_hoist_deployment_flags;
typedef struct {

	/* Offset from center of object */
	sar_position_struct	offset;

	/* Position (not offset) of basket */
	sar_position_struct	pos;

	/* Direction of basket */
	sar_direction_struct	dir;

	/* Rope extension (if <= 0.0 then implies the rope is fully
	 * retracted)
	 */
	float		rope_cur,	/* In meters */
			rope_max;
	float		rope_rate;	/* In meters per cycle */

	/* Rope visual extension, since rope_cur may be longer than the
	 * distance to the landable ground (in meters) we need to use
	 * this value when drawing
	 */
	float		rope_cur_vis;

	/* Indicates rope end is in contact with ground */
	char		on_ground;

	/* Hoist deployment */
	sar_hoist_deployment_flags	deployments,	/* Available deployments */
					cur_deployment;	/* Selected deployment */

	/* Cylendrical contact area of rescue basket, in meters */
	float		contact_radius,
			contact_z_min,
			contact_z_max;

	/* Load capacity, in kg */
	float		capacity;

	/* Reference to occupant human object index numbers (does not
	 * include the diver)
	 */
	int		*occupant;
	int		total_occupants;

	/* Total mass of occupant object(s) in kg, this is to speed up
	 * calculations. The value is updated when a new occupant object
	 * is added to the hoist and reset to 0 when the hoist is deployed
	 * the next time.
	 */
	float		occupants_mass;

	/* Texture reference numbers on scene structure */
	int		side_tex_num,
			end_tex_num,
			bottom_tex_num,
			water_ripple_tex_num;

	/* Diver color palette */
	sar_color_struct	diver_color[SAR_HUMAN_COLORS_MAX];

	/* Animation position and rate */
	sar_grad_anim_t	anim_pos,
			anim_rate;

} sar_obj_hoist_struct;
#define SAR_OBJ_HOIST(p)	((sar_obj_hoist_struct *)(p))



/*
 *	Object Types:
 */
typedef enum {
	SAR_OBJ_TYPE_GARBAGE		= 0,
	SAR_OBJ_TYPE_STATIC		= 1,
	SAR_OBJ_TYPE_AUTOMOBILE		= 2,
	SAR_OBJ_TYPE_WATERCRAFT		= 3,
	SAR_OBJ_TYPE_AIRCRAFT		= 4,
/* Note type 5 used to be airplane, it's now changed to be part of
 * type 4 (which was helicopter) so both 4 and 5 are all flying things
 * now */
	SAR_OBJ_TYPE_GROUND		= 6,	/* Cylendrical landable object */
	SAR_OBJ_TYPE_RUNWAY		= 7,
	SAR_OBJ_TYPE_HELIPAD		= 8,
	SAR_OBJ_TYPE_HUMAN		= 9,
	SAR_OBJ_TYPE_SMOKE		= 10,
	SAR_OBJ_TYPE_FIRE		= 11,
	SAR_OBJ_TYPE_EXPLOSION		= 12,
	SAR_OBJ_TYPE_CHEMICAL_SPRAY	= 13,	/* Water, fire-retardant, etc */
	SAR_OBJ_TYPE_FUELTANK		= 14,	/* Fuel tank dropped from an
						 * aircraft */
	SAR_OBJ_TYPE_PREMODELED		= 20
} sar_obj_type;


/*
 *	Object Type Check macros:
 */
#define SAR_OBJ_IS_STATIC(p)	\
(((p) != NULL) ? ((*(sar_obj_type *)(p)) == SAR_OBJ_TYPE_STATIC) : 0)

#define SAR_OBJ_IS_AUTOMOBILE(p)	\
(((p) != NULL) ? ((*(sar_obj_type *)(p)) == SAR_OBJ_TYPE_AUTOMOBILE) : 0)

#define SAR_OBJ_IS_WATERCRAFT(p)	\
(((p) != NULL) ? ((*(sar_obj_type *)(p)) == SAR_OBJ_TYPE_WATERCRAFT) : 0)

#define SAR_OBJ_IS_AIRCRAFT(p)	\
(((p) != NULL) ? ((*(sar_obj_type *)(p)) == SAR_OBJ_TYPE_AIRCRAFT) : 0)

#define SAR_OBJ_IS_GROUND(p)	\
(((p) != NULL) ? ((*(sar_obj_type *)(p)) == SAR_OBJ_TYPE_GROUND) : 0)

#define SAR_OBJ_IS_RUNWAY(p)	\
(((p) != NULL) ? ((*(sar_obj_type *)(p)) == SAR_OBJ_TYPE_RUNWAY) : 0)

#define SAR_OBJ_IS_HELIPAD(p)	\
(((p) != NULL) ? ((*(sar_obj_type *)(p)) == SAR_OBJ_TYPE_HELIPAD) : 0)

#define SAR_OBJ_IS_HUMAN(p)	\
(((p) != NULL) ? ((*(sar_obj_type *)(p)) == SAR_OBJ_TYPE_HUMAN) : 0)

#define SAR_OBJ_IS_SMOKE(p)	\
(((p) != NULL) ? ((*(sar_obj_type *)(p)) == SAR_OBJ_TYPE_SMOKE) : 0)

#define SAR_OBJ_IS_FIRE(p)	\
(((p) != NULL) ? ((*(sar_obj_type *)(p)) == SAR_OBJ_TYPE_FIRE) : 0)

#define SAR_OBJ_IS_EXPLOSION(p)	\
(((p) != NULL) ? ((*(sar_obj_type *)(p)) == SAR_OBJ_TYPE_EXPLOSION) : 0)

#define SAR_OBJ_IS_CHEMICAL_SPRAY(p)	\
(((p) != NULL) ? ((*(sar_obj_type *)(p)) == SAR_OBJ_TYPE_CHEMICAL_SPRAY) : 0)

#define SAR_OBJ_IS_FUELTANK(p)	\
(((p) != NULL) ? ((*(sar_obj_type *)(p)) == SAR_OBJ_TYPE_FUELTANK) : 0)

#define SAR_OBJ_IS_PREMODELED(p)	\
(((p) != NULL) ? ((*(sar_obj_type *)(p)) == SAR_OBJ_TYPE_PREMODELED) : 0)


/*
 *	Object Type Substructure Pointer Get macros:
 *
 *	These macros will return a pointer to the substructure (member
 *	data) with cast or NULL if the given object pointer is NULL or
 *	not of the corresponding type.
 */
#define SAR_OBJ_GET_STATIC(p)	\
(sar_object_static_struct *)((SAR_OBJ_IS_STATIC(p)) ? \
 ((sar_object_struct *)(p))->data : NULL \
)

#define SAR_OBJ_GET_AIRCRAFT(p)	\
(sar_object_aircraft_struct *)((SAR_OBJ_IS_AIRCRAFT(p)) ? \
 ((sar_object_struct *)(p))->data : NULL \
)

#define SAR_OBJ_GET_GROUND(p)	\
(sar_object_ground_struct *)((SAR_OBJ_IS_GROUND(p)) ? \
 ((sar_object_struct *)(p))->data : NULL \
)

#define SAR_OBJ_GET_RUNWAY(p)	\
(sar_object_runway_struct *)((SAR_OBJ_IS_RUNWAY(p)) ? \
 ((sar_object_struct *)(p))->data : NULL \
)

#define SAR_OBJ_GET_HELIPAD(p)	\
(sar_object_helipad_struct *)((SAR_OBJ_IS_HELIPAD(p)) ? \
 ((sar_object_struct *)(p))->data : NULL \
)

#define SAR_OBJ_GET_HUMAN(p)	\
(sar_object_human_struct *)((SAR_OBJ_IS_HUMAN(p)) ? \
 ((sar_object_struct *)(p))->data : NULL \
)

#define SAR_OBJ_GET_SMOKE(p)	\
(sar_object_smoke_struct *)((SAR_OBJ_IS_SMOKE(p)) ? \
 ((sar_object_struct *)(p))->data : NULL \
)

#define SAR_OBJ_GET_FIRE(p)	\
(sar_object_fire_struct *)((SAR_OBJ_IS_FIRE(p)) ? \
 ((sar_object_struct *)(p))->data : NULL \
)

#define SAR_OBJ_GET_EXPLOSION(p)	\
(sar_object_explosion_struct *)((SAR_OBJ_IS_EXPLOSION(p)) ? \
 ((sar_object_struct *)(p))->data : NULL \
)

#define SAR_OBJ_GET_CHEMICAL_SPRAY(p)	\
(sar_object_chemical_spray_struct *)((SAR_OBJ_IS_CHEMICAL_SPRAY(p)) ? \
 ((sar_object_struct *)(p))->data : NULL \
)

#define SAR_OBJ_GET_FUELTANK(p)	\
(sar_object_fueltank_struct *)((SAR_OBJ_IS_FUELTANK(p)) ? \
 ((sar_object_struct *)(p))->data : NULL \
)

#define SAR_OBJ_GET_PREMODELED(p)	\
(sar_object_premodeled_struct *)((SAR_OBJ_IS_PREMODELED(p)) ? \
 ((sar_object_struct *)(p))->data : NULL \
)


/*
 *	Aircraft:
 */
typedef enum {
	SAR_FLIGHT_MODEL_HELICOPTER,
	SAR_FLIGHT_MODEL_AIRPLANE,
	SAR_FLIGHT_MODEL_SLEW
} sar_flight_model_type;
typedef struct {

	/* Flight Model Type */
	sar_flight_model_type flight_model_type;

	/* Previously set Flight Model Type (for switching back and
	 * forth between flight model types
	 */
	sar_flight_model_type last_flight_model_type;

	/* Flight Dynamics Model */
	SFMModelStruct	*fdm;

	/* Air Worthy State */
	sar_air_worthy_state air_worthy_state;

	/* Current relative speed, in meters per cycle */
	sar_position_struct airspeed;

	/* Stall speed, the speed at which a controllable stall begins.
	 * Loss of lift can still occure twice beyond this value, in
	 * meters per cycle
	 */
	float		speed_stall,
			stall_coeff;

	/* Maximum speed (this is typically *several* times the rated
	 * maximum speed), in meters per cycle
	 */
	float		speed_max;

	/* overspeed_expected is the overspeed value according to the
	 * specifications of the aircraft, if the speed reaches or
	 * exceeds this value then overspeed warnings and affects are
	 * performed, units are in meters per cycle
	 *
	 * overspeed is the speed at which the aircraft will incure
	 * damage, in meters per cycle
	 */
	float		overspeed_expected,
			overspeed;

	/* Current velocity vector representing velocity in object
	 * relative vector direction, in meters per cycle
	 */
	sar_position_struct vel;

	/* Z acceleration, in meters per cucle^2 */
	float		z_accel;

	/* Minimum drag in meters per cycle, must be non-negative */
	float		min_drag;

	/* Acceleration response for when flight_model_type is set to
	 * SAR_FLIGHT_MODEL_HELICOPTER, higher values produce less
	 * responsiveness)
	 *
	 * Note: This should really be called be accel dampening
	 */
	sar_position_struct accel_responsiveness;

	/* Acceleration response for when flight_model_type is set to
	 * SAR_FLIGHT_MODEL_AIRPLANE, higher values produce less
	 * responsiveness)
	 *
	 * Note: This should really be called be accel dampening
	 */
	sar_position_struct airplane_accel_responsiveness;


	/* Attitude change rates, in radians per cycle */
	sar_direction_struct attitude_change_rate;

	/* Pitch and bank leveling, in radians per cycle */
	float		pitch_leveling,
			bank_leveling;

	/* Ground pitch offset in radians (can be negative) */
	float		ground_pitch_offset;

	/* Cockpit offset relative to object center */
	sar_position_struct cockpit_offset_pos;

	/* Cockpit visual model */
	sar_visual_model_struct *visual_model_cockpit;

	/* Belly to center of object in meters */
	float		belly_height;

	/* Length of aircraft in meters */
	float		length;

	/* Wingspan of aircraft in meters */
	float		wingspan;

	/* Height of landing gear in meters */
	float		gear_height;

	/* Mass */
	float		dry_mass;		/* In kg */
	float		fuel_rate;		/* In kg consumed per cycle */
	float		fuel, fuel_max;		/* In kg */
	int		crew,
			passengers,
			passengers_max;
	float		passengers_mass;	/* Total mass of current
						 * passengers (in kg) */

	/* Passengers pending to leave or drop from this aircraft,
	 * 0 means none and any positive value means this many passengers
	 * need to go when the aircraft's door is fully opened and its
	 * conditions are right (ie landed at the right helipad)
	 */
	int		passengers_leave_pending,
			passengers_drop_pending;

	/* External fuel tanks */
	sar_external_fueltank_struct **external_fueltank;
	int		total_external_fueltanks;

	/* Engine state and control positions */
	sar_engine_state	engine_state;
	time_t		next_engine_on;		/* Time till engine starts up (in ms) */

	/* Throttle control position. In aircraft flight model this is 
	 * the actual throttle control position, in helicopter flight
	 * model this is the speed at which the rotors are spinning.
	 * Range is from 0.0 to 1.0
	 */
	float		throttle;

	/* Current collective value and collective range. The current
	 * collective value is always 1.0 in airplane flight model. In
	 * helicopter flight model, the current collective value matches
	 * the game controller's throttle position
	 *
	 * Both the collective and collective_range are in values from
	 * 0.0 to 1.0
	 */
	float		collective,
			collective_range;

	/* Engine data. Given the simulation model, the power assigned to
	 * different aircrafts engines is not realistic, but rather adjusted
	 * so that they fly correctly along with other options (drag
	 * coefficients etc.).
	 */
	float		engine_power;		/* In kg * m / cycle^2 (Newtons) */
	char		engine_can_pitch;	/* 1 for tilt rotors */

	/* Engine sounds */
	int		engine_inside_sndsrc;
	void		*engine_inside_sndplay;
	int		engine_outside_sndsrc;
	void		*engine_outside_sndplay;

	/* Repeating warning sounds */
	int		stall_sndsrc;
	void		*stall_sndplay;
	int		overspeed_sndsrc;
	void		*overspeed_sndplay;


	/* Visual controller positions, these values only dictate
	 * the visual display of control parts
	 */
	float		control_heading,	/* Rudder, -1.0 to 1.0 */
			control_pitch,		/* Elevator, -1.0 to 1.0 */
			control_bank;		/* Ailerons, -1.0 to 1.0 */

	/* Effective controller positions, these values will be passed
	 * to the FDM and control the actual attitude changing
	 */
	float		control_effective_heading,	/* Rudder, -1.0 to 1.0 */
			control_effective_pitch,	/* Elevator, -1.0 to 1.0 */
			control_effective_bank;		/* Ailerons, -1.0 to 1.0 */

	float		elevator_trim;		/* -1.0 to 1.0 */

	/* Flaps */
	float		flaps_position;		/* 0.0 (retracted) to 1.0
						 * (fully deployed) */
	float		flaps_stall_offset;	/* Moves the speed_stall
						 * ahead by this many
						 * meters per cycle */

	/* Service ceiling, in meters */
	float		service_ceiling;

	/* Distance from object's center to touchable ground in meters
	 * (note that this is usually negative unless the object is
	 * underground)
	 */
	float		center_to_ground_height;


	/* Moveable parts */
	sar_obj_part_struct	**part;
	int		total_parts;

	/* Rotors */
	sar_obj_rotor_struct	**rotor;
	int		total_rotors;

	/* Landed states */
	char		landed,		/* 1 if landed */
			on_water;	/* If landed is 1 and on_water is 1,
					 * then is on water */

	/* Landing gear state:
	 * -1	non-existant
	 * 0    up/retracted/in
	 * 1    down/deployed/out
	 */
	int		landing_gear_state;

	/* Ground turning */
	float		gturn_radius;	/* Distance from farthest non-turning
					 * gear to turning gear in meters, can
					 * be negative (0 implies no turning) */
	/* Ground turning velocity optimul and maximum (meters per
	 * cycle)
	 */
	float		gturn_vel_opt,
			gturn_vel_max;

	/* Air brakes and wheel brakes:
	 * -1   non-existant
	 * 0    off
	 * 1    on
	 * 2    locked (parking brakes on)
	 */
	int		air_brakes_state,
			wheel_brakes_state;

	/* This affects how much reduction speed suffers when brakes are active */
	float		wheel_brakes_coeff;

	/* Air brakes area indicates how much additional surface is exposed
	 * when deployed. This causes extra aerodynamic drag. To disable
	 * brakes, it can be set to 0.
	 */
	float		air_brakes_area;

	/* Spot light direction */
	sar_direction_struct	spotlight_dir;

	/* Rescue hoist */
	sar_obj_hoist_struct	*hoist;

	/* Intercept waypoints */
	sar_intercept_struct	**intercept;
	int		total_intercepts,
			cur_intercept;

	/* Autopilot data */
	sar_autopilot_state autopilot_state;
	float		autopilot_altitude;	/* Target height in meters
						 * above sea level */
} sar_object_aircraft_struct;
#define SAR_OBJECT_AIRCRAFT(p)	((sar_object_aircraft_struct *)(p))


/*
 *	Ground/Heightfield:
 */
typedef struct {

	/* Elevation from (above) local MSL in meters. If MSL was
	 * set to 10 meters and this elevation was set to 3 meters then
	 * the resulting elevation would be 13 meters to any object within
	 * contact of the ground object.
	 *
	 * Note that the ground object itself completely ignores the
	 * scene structure defined ground_elevation_msl value.
	 */
	float		elevation;

	/* Translation of heightfield from center of object */
	float		x_trans,
			y_trans,
			z_trans;

	/* Heightfield data (only used if z_point_value is not NULL) */
	float		x_len,		/* grid_points_x * grid_x_spacing */
			y_len;		/* grid_points_y * grid_y_spacing */
	int		grid_points_x,	/* Number of grid points */
			grid_points_y,
			grid_points_total;	/* grid_points_x * grid_points_y */
	float		grid_x_spacing,	/* Size of each grid in meters */
			grid_y_spacing,
			grid_z_spacing;
	double		*z_point_value;	/* Heightfield z height map, each
					 * point value came from
					 * (image_data_pixel) /
					 * 0xff * grid_z_spacing */

	/* Value records for this ground object's inversed heading for the
	 * trig functions, this is used to speed up rotations of heading
	 * for checking an object over this ground object's heightfield
	 * surface. It also limits ground objects to only have its
	 * heading rotated.
	 *
	 * Ie cos_heading = cos(-heading) and sin_heading = sin(-heading)
	 */
	float		cos_heading,
			sin_heading;

} sar_object_ground_struct;
#define SAR_OBJECT_GROUND(p)	((sar_object_ground_struct *)(p))


/*
 *	Smoke Puffs & Sparks:
 */
typedef enum {
	SAR_SMOKE_TYPE_SMOKE,
	SAR_SMOKE_TYPE_SPARKS,
	SAR_SMOKE_TYPE_DEBRIS
} sar_smoke_type;
/* Smoke Puff & Sparks Units */
typedef struct {

	sar_position_struct pos;	/* In world coordinates */

	sar_position_struct vel;

	/* Color, only used when the smoke type is set to
	 * SAR_SMOKE_TYPE_SPARKS
	 */
	sar_color_struct color;

	/* Current size in meters */
	float		radius;

	/* Current visibility of smoke unit, in range of 0.0 to 1.0.
	 * If 0.0 then it will not be drawn at all and is considered
	 * "available for use"
	 */
	float		visibility;

} sar_object_smoke_unit_struct;
/* Smoke/Sparks */
typedef struct {

	sar_smoke_type	type;

	/* Spawn offset position, this offset is applied to the actual
	 * location of the smoke trail object where each unit will be
	 * spawned at
	 */
	sar_position_struct respawn_offset;

	/* Starting and ending radius values for all units
	 *
	 * If the smoke type is SAR_OBJ_SMOKE_TYPE_SPARKS then
	 * radius_start is ignored and radius_max is how far sparks
	 * should fly
	 */
	float		radius_start,
			radius_max;

	/* Radius increase rate in meters per cycle (must be positive)
	 *
	 * If the smoke type is SAR_OBJ_SMOKE_TYPE_SPARKS then this
	 * value is ignored
	 */
	float		radius_rate;

	/* If this is true then when a smoke unit has reached its 
	 * maximum size its visiblity will be reset to 0.0 (thus marking
	 * it as "available for use")
	 */
	int		hide_at_max;

	/* If this is true then this object will be marked for deletion
	 * (its life span set to 1) when all of its units are no longer
	 * visible (visibility = 0.0)
	 */
	int		delete_when_no_units;

	/* Respawn intervals (in ms)
	 *
	 * If respawn_int is 0 then no respawning will take place
	 *
	 * This is often used to stop respawning but keep the already
	 * visible smoke units around
	 */
	time_t		respawn_int,
			respawn_next;

	/* Texture number on scene structure */
	int		tex_num;

	/* Reference object that this smoke trail is to follow (can be -1
	 * for none or do not follow)
	 */
	int		ref_object;

	/* Each smoke unit forming this smoke trail */
	sar_object_smoke_unit_struct *unit;
	int		total_units;

} sar_object_smoke_struct;
#define SAR_OBJECT_SMOKE_UNIT(p)	((sar_object_smoke_unit_struct *)(p))
#define SAR_OBJECT_SMOKE(p)		((sar_object_smoke_struct *)(p))


/*
 *	Explosion/Splash:
 */
typedef enum {
	SAR_EXPLOSION_COLOR_EMISSION_NONE,	/* Interacts with light (ie splashes) */
	SAR_EXPLOSION_COLOR_EMISSION_IS_LIGHT,	/* Does not interact with light */
	SAR_EXPLOSION_COLOR_EMISSION_EMIT_LIGHT	/* Gives off light */
} sar_explosion_color_emission;
typedef enum {
	SAR_EXPLOSION_CENTER_OFFSET_NONE,
	SAR_EXPLOSION_CENTER_OFFSET_BASE
} sar_explosion_center_offset;
typedef struct {

	/* Spherical size of the explosion (in meters), this is also
	 * used to calculate the displayed explosion billboard
	 */
	float		radius;

	sar_explosion_color_emission	color_emission;
	sar_explosion_center_offset	center_offset;

	time_t		frame_inc_int,	/* Frame increment interval (in ms) */
			next_frame_inc;	/* Time to increment next frame (in ms) */

	int		cur_frame,	/* Current frame */
			frame_repeats;	/* Number of times animation has cycled */

	/* Number of frame repeats, 0 or less to repeat forever (or
	 * when life span has exceeded)
	 */
	int		total_frame_repeats;

	/* Texture number on scene */
	int		tex_num,
			ir_tex_num;

	/* Reference object that this explosion is to follow (can be
	 * -1 for none/do not follow)
	 */
	int		ref_object;

} sar_object_explosion_struct;
#define SAR_OBJECT_EXPLOSION(p)	((sar_object_explosion_struct *)(p))


/* 
 *	Fire:
 */
typedef struct {

	/* Cylendrical size of fire in meters, this is also used to 
	 * calculate the fire billboard
	 *
	 * Center is at the base of the fire
	 */
	float		radius,
			height;

	time_t		frame_inc_int,	/* Frame increment interval, in ms */
			next_frame_inc;	/* Time to increment next frame, in ms */ 
	int		cur_frame,	/* Current frame */
			frame_repeats;	/* Number of times animation has cycled */

	/* Number of frame repeats, 0 or less to repeat forever (or
	 * when life span has exceeded)
	 */     
	int		total_frame_repeats;

	/* Texture number on scene */
	int		tex_num,
			ir_tex_num;

	/* Reference object that this fire is to follow (can be -1 for
	 * none/do not follow)
	 */
	int		ref_object;

} sar_object_fire_struct;
#define SAR_OBJECT_FIRE(p)	((sar_object_fire_struct *)(p))


/*
 *	Chemical Spray:
 *
 *	A single puff of chemical spray (ie water, fire-retardant, etc).
 */
typedef enum {
	SAR_CHEMICAL_WATER,
	SAR_CHEMICAL_FIRE_RETARDANT
} sar_chemical_type;
typedef struct {

	sar_chemical_type	chemical_type;

	int		owner;		/* Object that created this spray or -1 
					 * for none */

	/* Texture number on the scene */
	int		tex_num;

} sar_object_chemical_spray_struct;
#define SAR_OBJECT_CHEMICAL_SPRAY(p)	((sar_object_chemical_spray_struct *)(p))


/*
 *	Fuel Tank:
 *
 *	Note: This is not a fuel tank that is currently on an aircraft,
 *	do not confuse this with the fuel tanks on objects (see
 *	sar_external_fueltank_struct).
 */
typedef enum {
	SAR_FUELTANK_FLAG_ON_GROUND	= (1 << 0)
} sar_fueltank_flags;
typedef struct {

	sar_fueltank_flags	flags;

	/* Current speed in meters per cycle */
	float		speed;

	/* Maximum negative vertical velocity in meters per cycle */
	float		vel_z_max;

	/* Current velocity vector representing velocity in object
	 * relative vector direction, in meters per cycle
	 */
	sar_position_struct	vel;

	/* Belly to center height, in meters */
	float		belly_to_center_height;

	/* Index of object of which this fuel tank fell off of (can be
	 * -1 for unknown)
	 */
	int		ref_object;

	/* Mass and fuel (in kg) */
	float		dry_mass,
			fuel,
			fuel_max;

} sar_object_fueltank_struct;
#define SAR_OBJECT_FUELTANK(p)	((sar_object_fueltank_struct *)(p))


/*
 *	Runway:
 */
typedef enum {
	SAR_RUNWAY_FLAG_THRESHOLDS	= (1 << 0),	/* Thresholds (not
							 * Displaced Thresholds) */
	SAR_RUNWAY_FLAG_BORDERS		= (1 << 1),	/* Side Borders */
	SAR_RUNWAY_FLAG_TD_MARKERS	= (1 << 2),	/* Touch Down Markers */
	SAR_RUNWAY_FLAG_MIDWAY_MARKERS	= (1 << 3),
	SAR_RUNWAY_FLAG_NORTH_GS	= (1 << 4),	/* North Glide Slope */
	SAR_RUNWAY_FLAG_SOUTH_GS	= (1 << 5)	/* South Glide Slope */
} sar_runway_flags;
typedef enum {
	SAR_RUNWAY_SURFACE_PAVED,
	SAR_RUNWAY_SURFACE_GRAVEL,
	SAR_RUNWAY_SURFACE_CONCRETE,
	SAR_RUNWAY_SURFACE_GROVED
} sar_runway_surface_type;
typedef enum {
	SAR_RUNWAY_APPROACH_LIGHTING_END	= (1 << 0),
	SAR_RUNWAY_APPROACH_LIGHTING_TRACER	= (1 << 1),
	SAR_RUNWAY_APPROACH_LIGHTING_ALIGN	= (1 << 2),
	SAR_RUNWAY_APPROACH_LIGHTING_ILS_GLIDE	= (1 << 3)
} sar_runway_approach_lighting_flags;
typedef struct {

	sar_runway_flags	flags;

	/* Size (in meters) */
	float		length,
			width;

	/* Surface Type */
	sar_runway_surface_type	surface_type;

	/* End Labels */
	char		*north_label,
			*south_label;
	sar_visual_model_struct	*north_label_vmodel,
				*south_label_vmodel;
	float		north_label_width,
			south_label_width;

	/* Number of dashes (0 for none) */
	int		dashes;

	/* Edge lighting in meters (0.0 for none) */
	float		edge_light_spacing;

	/* Approach lighting flags */
	sar_runway_approach_lighting_flags	north_approach_lighting_flags,
						south_approach_lighting_flags;
	sar_grad_anim_t	tracer_anim_pos,
			tracer_anim_rate;

	/* Displaced threshold offset from respective edges in meters
	 * (can be 0.0 for none)
	 */
	float		north_displaced_threshold,
			south_displaced_threshold;

	sar_visual_model_struct	*threshold_vmodel,
				*td_marker_vmodel,
				*midway_marker_vmodel,
				*north_displaced_threshold_vmodel,
				*south_displaced_threshold_vmodel;

	/* Runway background texture number (on scene structure) */
	int		tex_num;

} sar_object_runway_struct;
#define SAR_OBJECT_RUNWAY(p)	((sar_object_runway_struct *)(p))


/*
 *	Helipad:
 */
typedef enum {
	SAR_HELIPAD_FLAG_LABEL			= (1 << 1),
	SAR_HELIPAD_FLAG_EDGE_LIGHTING		= (1 << 2),
	SAR_HELIPAD_FLAG_FUEL			= (1 << 3),
	SAR_HELIPAD_FLAG_REPAIR			= (1 << 4),
	SAR_HELIPAD_FLAG_DROPOFF		= (1 << 5),	/* Can drop
								 * off passengers */
	SAR_HELIPAD_FLAG_REF_OBJECT		= (1 << 6),	/* Has ref object */
	SAR_HELIPAD_FLAG_FOLLOW_REF_OBJECT	= (1 << 7),
	SAR_HELIPAD_FLAG_RESTART_POINT		= (1 << 8)	/* Is a restarting
								 * point */
} sar_helipad_flags;
typedef enum {
	SAR_HELIPAD_STYLE_GROUND_PAVED,
	SAR_HELIPAD_STYLE_GROUND_BARE,		/* Unpaved */
	SAR_HELIPAD_STYLE_BUILDING,		/* Roof top */
	SAR_HELIPAD_STYLE_VEHICLE		/* On a vehicle or vessel */
} sar_helipad_style;
typedef struct {

	sar_helipad_flags	flags;
	sar_helipad_style	style;

	/* Size of landable area (in meters) */
	float		length,
			width;

	/* Downward recession from the helipad's landable surface into
	 * the ground (in meters)
	 *
	 * Example, a value of 2 means 2 meters down
	 */
	float		recession;

	/* Label */
	char		*label;
	sar_visual_model_struct	*label_vmodel;
	float		label_width;

	/* Lighting edge spacing, in meters */
	float		light_spacing;

	/* Texture number on scene structure, specifying the texture
	 * for the helipad's main landable area
	 */
	int		tex_num;

	/* If SAR_HELIPAD_FLAG_REF_OBJECT is set then these members
	 * will have affect
	 *
	 * Note: ref_offset is applied before ref_dir
	 */
	int		ref_object;	/* Object that this helipad `follows' */
	sar_position_struct	ref_offset;	/* Relative to ref_object */
	sar_direction_struct	ref_dir;	/* Relative to ref_object */

} sar_object_helipad_struct;
#define SAR_OBJECT_HELIPAD(p)	((sar_object_helipad_struct *)(p))


/*
 *	Human/Actor:
 */
typedef enum {
	SAR_HUMAN_FLAG_NEED_RESCUE	= (1 << 1),
	SAR_HUMAN_FLAG_SIT		= (1 << 2),	/* Base at tush */
	SAR_HUMAN_FLAG_SIT_DOWN		= (1 << 3),	/* Base at tush, feet
							 * out forward */
	SAR_HUMAN_FLAG_SIT_UP		= (1 << 4),	/* Base at feet */
	SAR_HUMAN_FLAG_LYING		= (1 << 5),
	SAR_HUMAN_FLAG_ALERT		= (1 << 6),	/* Is awake */
	SAR_HUMAN_FLAG_AWARE		= (1 << 7),	/* Knows of surroundings */
	SAR_HUMAN_FLAG_IN_WATER		= (1 << 8),
	SAR_HUMAN_FLAG_ON_STREATCHER	= (1 << 9),
	SAR_HUMAN_FLAG_RUN		= (1 << 10),	/* Animate as running */
	SAR_HUMAN_FLAG_RUN_TOWARDS	= (1 << 11),	/* Intends to run towards
							 * (does not imply currently
							 * running */
	SAR_HUMAN_FLAG_RUN_AWAY		= (1 << 12),	/* Intends to run away
							 * (does not imply currently
							 * running */
	SAR_HUMAN_FLAG_PUSHING		= (1 << 13),
	SAR_HUMAN_FLAG_GRIPPED		= (1 << 14),	/* Someone/something is
							 * holding this (ie in
							 * rescue basket */
	SAR_HUMAN_FLAG_DIVER_CATCHER	= (1 << 15)
} sar_human_flags;
typedef struct {

	sar_human_flags	flags;

	float		mass;	/* In kg */

	float		height;	/* In meters */

	sar_grad_anim_t	anim_pos,
			anim_rate;

	/* Colors, see definations for SAR_HUMAN_COLOR_* and
	 * SAR_HUMAN_COLORS_MAX for index positions and maximum colors
	 * (respectivly)
	 */
	sar_color_struct	color[SAR_HUMAN_COLORS_MAX];

	/* Water ripples texture number on the scene */
	int		water_ripple_tex_num;

	/* Reference to object running towards or away from, the following
	 * values have special meaning:
	 *
	 *	-1	No intercepting
	 *	-2	Intercept player
	 *
	 * Works when flags SAR_HUMAN_FLAG_RUN_TOWARDS xor 
	 * SAR_HUMAN_FLAG_RUN_AWAY is set and intercepting_object is 
	 * valid and not the human object itself.
	 */
	int		intercepting_object;

	/* Distance to intercepting object in meters (may be ignored if
	 * intercepting_object is -1.
	 */
	float		intercepting_object_distance2d,
			intercepting_object_distance3d;

	/* Number of assisting humans following this human (0 for none).
	 * these are not other objects but rather groups of humans drawn
	 * with this human object to make it look like multiple
	 * humans.
	 */
	int		assisting_humans;
	sar_color_struct	assisting_human_color[SAR_HUMAN_COLORS_MAX];

	/* Messages */
	char		*mesg_enter;	/* Entering into aircraft or vehicle */

} sar_object_human_struct;
#define SAR_OBJECT_HUMAN(p)	((sar_object_human_struct *)(p))


/*
 *	Premodeled Object:
 */
typedef enum {
	SAR_OBJ_PREMODELED_BUILDING,
	SAR_OBJ_PREMODELED_CONTROL_TOWER,
	SAR_OBJ_PREMODELED_HANGAR,
	SAR_OBJ_PREMODELED_POWER_TRANSMISSION_TOWER,
	SAR_OBJ_PREMODELED_TOWER,
	SAR_OBJ_PREMODELED_RADIO_TOWER
} sar_premodeled_type;
typedef struct {

	sar_premodeled_type	type;

	/* Values, depending on type, not all may be applicateable */
	float		width,
			length,
			height;

	/* Animation */
	sar_grad_anim_t	anim_pos,
			anim_rate;

	/* Reference to a texture on the scene */
#define SAR_OBJ_PREMODEL_MAX_TEXTURES	10
	int		tex_num[SAR_OBJ_PREMODEL_MAX_TEXTURES];

} sar_object_premodeled_struct;
#define SAR_OBJECT_PREMODELED(p)	((sar_object_premodeled_struct *)(p))


/*
 *	SAR Object Core:
 */
typedef struct {

	sar_obj_type	type;

#define SAR_OBJ_FLAG_NO_DEPTH_TEST	(1 << 1)
#define SAR_OBJ_FLAG_HIDE_DAY_MODEL	(1 << 2)	/* Hide day model when not day */
#define SAR_OBJ_FLAG_HIDE_DAWN_MODEL	(1 << 3)	/* Hide dawn model when not dawn */
#define SAR_OBJ_FLAG_HIDE_DUSK_MODEL	(1 << 4)	/* Hide dusk model when not dusk */
#define SAR_OBJ_FLAG_HIDE_NIGHT_MODEL	(1 << 5)	/* Hide night model when not night */

/* These two only work if SAR_OBJ_FLAG_HIDE_NIGHT_MODEL is set */
#define SAR_OBJ_FLAG_NIGHT_MODEL_AT_DAWN	(1 << 6)	/* Show night modem at dawn */
#define SAR_OBJ_FLAG_NIGHT_MODEL_AT_DUSK	(1 << 7)	/* Show night modem at dusk */

#define SAR_OBJ_FLAG_SHADE_MODEL_SMOOTH	(1 << 8)	/* Use smooth shading */

#define SAR_OBJ_FLAG_FAR_MODEL_DAY_ONLY	(1 << 9)	/* Display far model during
							 * time only.
							 */
#define SAR_OBJ_FLAG_POLYGON_OFFSET	(1 << 10)	/* Enable polygon offset */
#define SAR_OBJ_FLAG_POLYGON_OFFSET_REVERSE	(1 << 11)	/* Same as SAR_OBJ_FLAG_POLYGON_OFFSET
								 * cept offsets in
								 * the other direction.
								 */
#define SAR_OBJ_FLAG_POLYGON_OFFSET_WRITE_DEPTH	(1 << 12)	/* Write depth if
								 * polygon offsetting.
								 */

	sar_obj_flags_t flags;

	/* Name of object (can be NULL) */
	char		*name;

	/* Current position and direction */
	sar_position_struct	pos;
	sar_direction_struct	dir;

	/* Visible range of this object, in meters
	 *
	 * Object is not displayed when the camera is farther is beyond
	 * this range
	 */
	float		range;

	/* Visible range for displaying the far visual model when the
	 * camera is beyond this range, in meters
	 */
	float		range_far;


	/* Distance from sea level to touchable ground under object, in
	 * meters (ignored for objects of type SAR_OBJ_TYPE_GROUND)
	 */
	float		ground_elevation_msl;


	/* Contact bounds */
	sar_contact_bounds_struct	*contact_bounds;


	/* Time stamp of when this object was created in milliseconds
	 * and in systime seconds (respectivly). The value in milliseconds 
	 * may not be accurate when timmers are reset
	 */
	time_t		birth_time_ms,
			birth_time_sec;

	/* When this object `dies' (0 if lives forever) */
	time_t		life_span;

	/* Hit points */
	float		hit_points,
			hit_points_max;

	/* Temperature for FLIR (0.0 to 1.0) */
	float		temperature;

	/* Visual models */
	sar_visual_model_struct	*visual_model,		/* Day */
				*visual_model_ir,	/* InfraRed */
				*visual_model_far,	/* Far */
				*visual_model_dawn,	/* Dawn */
				*visual_model_dusk,	/* Dusk */
				*visual_model_night,	/* Night */
				*visual_model_shadow;	/* Shadow */

	/* Lights */
	sar_light_struct	**light;
	int			total_lights;

	/* Sound sources */
	sar_sound_source_struct	**sndsrc;
	int			total_sndsrcs;

	/* Pointer to additional data specific to the object type,
	 * the structure's type is specified by this structure's member
	 * type
	 */
	void *data;

} sar_object_struct;
#define SAR_OBJECT(p)	((sar_object_struct *)(p))



/*
 *	Scene Ground Base:
 *
 *	Contains visual models and tiling values for displaying of the
 *	ground base.
 */
typedef struct {

	/* Note, ground base moves with camera in modulous increments
	 * of the tiled width and height
	 */

	/* Tiling size, in meters */
	int		tile_width,
			tile_height;

	/* Tiling limited to this range from origin, in meters */
	float		close_range;


	/* Simple (far) solid color base plane and visual model */
	sar_color_struct	color;
	sar_visual_model_struct	*visual_model_simple;

	/* Close up texture tiled base plane visual model */
	sar_visual_model_struct	*visual_model_close;

} sar_scene_base_struct;
#define SAR_SCENE_BASE(p)	((sar_scene_base_struct *)(p))

/*
 *	Scene Horizon:
 */
typedef struct {

	/* Records the last time of day since midnight from the scene
	 * in units of 5 minute intervals as a whole number for horizon
	 * texture regeneration, see SARSimUpdateScene() in simmanage.c
	 * for when and how this value is updated
	 */
	int		last_tod;

	/* Gradient textures, 0 is most highlighted and
	 * total_textures - 1 is darkest
	 */
	v3d_texture_ref_struct **texture;
	int		total_textures;

} sar_scene_horizon_struct;
#define SAR_SCENE_HORIZON(p)	((sar_scene_horizon_struct *)(p))

/*
 *	Scene Cloud Layer:
 *
 *	Flat tiled layer of clouds as tiled textured quads.
 */
typedef struct {

	/* Note, cloud layer moves with camera in modulous           
	 * of the tiled width and height on the XY plane
	 */

	/* Tiling size, in meters */
	int		tile_width,
			tile_height;

	/* Tiling limited to this range from origin of tiling, in
	 * meters
	 */
	float		range;

	/* Name of cloud texture on scene structure */
	char		*tex_name;

	sar_visual_model_struct *visual_model;

	/* Height from MSL, in meters */
	float		z;

} sar_cloud_layer_struct;
#define SAR_CLOUD_LAYER(p)	((sar_cloud_layer_struct *)(p))

/*
 *	Scene Cloud Billboard:
 */
typedef struct {

	/* Note, cloud billboard moves with camera in modulous of the
	 * tiled width and height on the XY plane
	 */

	/* Tiling size, in meters */
	int		tile_width,
			tile_height;

	/* Name of texture on scene structure */
	char		*tex_name;
	/* Matched texture on scene structure */
	int		tex_num;

	/* Position of billboard object relative to tiled center */
	sar_position_struct pos;

	/* Size of cloud object, in meters */
	float		width,
			height;

	/* Lightening timers, in ms
	 *
	 * If lightening_min_int and lightening_max_int are 0 then
	 * that means there is no lightening
	 */
	time_t		lightening_min_int,	/* Minimum off interval */
			lightening_max_int,	/* Maximum off interval */
			lightening_next_on,	/* Waiting to go on */
			lightening_next_off,	/* Waiting to go off */
			lightening_started_on;	/* When turned on */

	/* Lightening points relative to cloud billboard's center,
	 * in meters
	 */
#define SAR_LIGHTENING_POINTS_MAX	6
	sar_position_struct lightening_point[SAR_LIGHTENING_POINTS_MAX];

} sar_cloud_bb_struct;
#define SAR_CLOUD_BB(p)		((sar_cloud_bb_struct *)(p))

/*
 *	Scene:
 */
typedef enum {
	SAR_SCENE_BASE_FLAG_IS_WATER	= (1 << 1)	/* All of the base is water */
} sar_scene_base_flags;
typedef struct {

	/* Title of scenery */
	char		*title;

	/* Time of day (in seconds) since midnight */
	float		tod;

	/* Time Of Day Code, updated in SARSimUpdateScene() */
	sar_tod_code	tod_code;

	/* Ground Base Flags */
	sar_scene_base_flags	base_flags;

	/* CANT angle in radians, this value is only applied on readouts
	 * added to the internal angle value in question
	 */
	float		cant_angle;

	/* MSL elevation in meters */
	float		msl_elevation;

	/* GPS information */
	sar_dms_t	dms_x_offset,	/* In degrees, -140.0 to 140.0 */
			dms_y_offset;	/* In degrees, -90.0 to 90.0 */
	float		planet_radius;	/* In meters */

	/* Visual Models List, each object's Visual Models are recorded
	 * here in this list and only these Visual Models may be deleted
	 */
	sar_visual_model_struct	**visual_model;
	int			total_visual_models;

	/* Sky and horizon gradient colors */
	sar_color_struct	sky_nominal_color,
				sky_brighten_color,
				sky_darken_color;
	/* Celestial object colors, these colors define the low and high
	 * tint colors of the celestial object (where low means it is near
	 * the horizon).
	 */
	sar_color_struct	star_low_color,
				star_high_color,
				sun_low_color,
				sun_high_color,
				moon_low_color,
				moon_high_color;

	/* Position of sun is dirived from light_pos */

	/* Moon position, a unit vector to be applied to the camera
	 * position
	 */
	sar_position_struct	moon_pos;
	/* Moon visibility hint, 1 = above horizon, 0 = hidden */
	char			moon_visibility_hint;


	/* Atmosphere (when enabled) */
	float		atmosphere_dist_coeff,	/* Coeff * max visiblity */
			atmosphere_density_coeff;

	/* Rain density coefficient, 0.0 for no rain, 0.5 for moderate,
	 * 1.0 for densest rain
	 */
	float		rain_density_coeff;

	/* Reference to player object */
	int			player_obj_num;
	sar_object_struct	*player_obj_ptr;
	char			player_has_crashed;	/* 1 if player object has crashed */

	/* Pointers to SAR_OBJ_TYPE_GROUND objects, you may free
	 * the pointer array but not the pointer to structures!!
	 */
	sar_object_struct	**ground_object;
	int			total_ground_objects;

	/* Pointers to SAR_OBJ_TYPE_HUMAN objects that *need rescue*.
	 * You may free the pointer array but not the pointers to structures!!
	 */
	sar_object_struct	**human_need_rescue_object;
	int			total_human_need_rescue_objects;


	/* Camera reference */
	sar_camera_ref	camera_ref;

	/* Camera field of view, in radians along Z axis (about the
	 * the X axis)
	 */
	float		camera_fovz;

	/* Direction of camera (when camera_ref=SAR_CAMERA_REF_COCKPIT) */
	sar_direction_struct camera_cockpit_dir;

	/* Direction from object to camera (when
	 * camera_ref=SAR_CAMERA_REF_SPOT)
	 */
	sar_direction_struct camera_spot_dir;
	float		camera_spot_dist;

	/* Distance from hoist to camera (when
	 * camera_ref=SAR_CAMERA_REF_HOIST)
	 */
	sar_direction_struct camera_hoist_dir;
	float	camera_hoist_dist;

	/* Position of camera (when camera_ref=SAR_CAMERA_REF_MAP) */
	sar_position_struct camera_map_pos;

	/* Position of camera (when camera_ref=SAR_CAMERA_REF_TOWER) */
	sar_position_struct camera_tower_pos;

	/* Target object that the camera is tracking (can be -1) */
	int		camera_target;

	/* Stack of camera rotation matrixes (set in sardraw.c) to
	 * rotate any vertex relative to the camera's rotation, each
	 * a 3 * 3 matrix.
	 * Member camera_rotmatrix_count indicates the actual number
	 * of matrixes set, which is equal or less than 
	 * SAR_CAMERA_ROTMATRIX_MAX
	 */
#define SAR_CAMERA_ROTMATRIX_MAX	5
	double		camera_rotmatrix[SAR_CAMERA_ROTMATRIX_MAX][3 * 3];
	int		camera_rotmatrix_count;

	/* Ear position, this is updated when the scene is drawn and
	 * the camera set
	 */
	sar_position_struct ear_pos;


	/* Global Primary Light */

	/* Position offset
	 * This specifies a unit vector that is to be applied to the
	 * camera's position to position the Global Primary Light
	 */
	sar_position_struct light_pos;

	/* Intensity/color
	 * This is not the color of the sun, the sun color is found in
	 * sun_low_color and sun_high_color
	 */
	sar_color_struct light_color;
	char		light_visibility_hint;	/* 1 if above horizon */

	/* Light position coefficient orthogonal to camera's center,
	 * the range is from [-1.0 to 1.0] (values out of this range
	 * mean that it is out of the camera's view
	 */
	float		light_xc,
			light_yc;


	/* Ground Base */
	sar_scene_base_struct base;

	/* Horizon */
	sar_scene_horizon_struct horizon;

	/* Cloud Layers */
	sar_cloud_layer_struct **cloud_layer;
	int		total_cloud_layers;

	/* Cloud Billboards */
	sar_cloud_bb_struct **cloud_bb;
	int		total_cloud_bbs;
	/* Primary Lightening Coeff (0.0 to 1.0, 0.0=off), this is
	 * set to positive when the cloud billboard's lightening is
	 * active, updated in SARSimUpdateScene()
	 */
	float		pri_lightening_coeff;

	/* Loaded Textures List */
	v3d_texture_ref_struct **texture_ref;
	int		total_texture_refs;

	/* Index references to specific textures (a value can be -1 if
	 * the texture in question was not loaded in the texture_ref
	 * list)
	 */
	int		texnum_sun,
			texnum_moon,
			texnum_spotlightcast;

	/* Global Sound Sources */
	sar_sound_source_struct **sndsrc;
	int		total_sndsrcs;


	/* Player control panel (ControlPanel *) */
	void		*player_control_panel;


	/* Messages list */
	char		**message;
	int		total_messages;

	/* Display message until this time runs out (in milliseconds),
	 * a value of 0 milliseconds implies that the message is not to
	 * be shown
	 */
	time_t		message_display_until;

	/* Sticky banner message, for displaying crash reason or
	 * mission failed reason, whenever it is not NULL
	 */
	char		**sticky_banner_message;
	int		total_sticky_banner_messages;

	/* Camera reference title, the name of the target object and/or
	 * description of what the camera is showing
	 */
	char		*camera_ref_title;
	time_t		camera_ref_title_display_until;

	/* Flight dynamics model realm structure, used by the SFM
	 * library as global reference while simulating all flight 
	 * dynamics models
	 */
	SFMRealmStruct	*realm;
    
        /* Welcome message - to be displayed at being of game */
        char           *welcome_message;

} sar_scene_struct;
#define SAR_SCENE(p)	((sar_scene_struct *)(p))
#define SAR_IS_CAMERA_IN_COCKPIT(s)	(((s) != NULL) ? \
 ((s)->camera_ref == SAR_CAMERA_REF_COCKPIT) : 0 \
)
#define SAR_IS_EAR_IN_COCKPIT(s)	(((s) != NULL) ? \
 ((s)->camera_ref == SAR_CAMERA_REF_COCKPIT) : 0 \
)


#endif	/* OBJ_H */
