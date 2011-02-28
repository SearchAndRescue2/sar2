#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/types.h>

#include "v3dhf.h"

#include "sfm.h"
#include "obj.h"
#include "simutils.h"
#include "simsurface.h"
#include "sar.h"


float SARSimSupportSurfaceHeight(
	sar_object_struct *obj_ptr,
	const sar_contact_bounds_struct *cb,
	const sar_position_struct *pos,
	float z_tolor
);
float SARSimHFGetGroundHeight(
	sar_object_struct *obj_ptr,
	const sar_position_struct *pos
);


#define MAX(a,b)        (((a) > (b)) ? (a) : (b))
#define MIN(a,b)        (((a) < (b)) ? (a) : (b))
#define CLIP(a,l,h)     (MIN(MAX((a),(l)),(h)))


/*
 *	Checks the contact bounds cb (which should have come from
 *	obj_ptr) against the given pos.
 *
 *	z_tolor must be 0 or a positive value that will be added to
 *	the given position's z compoent for each check.
 *
 *	Returns the z value of the obj_ptr's position plus any contact
 *	bound increased z offsets if and only if the given source pos
 *	is over the obj_ptr. Checks here take into consideration the
 *	hollowness of the target object, if the source position is below
 *	the contact bounds of the target object then the z value of the
 *	target object position is not returned.
 *
 *	Calls to this function should already check if cb's crash_flags
 *	specify SAR_CRASH_FLAG_SUPPORT_SURFACE before calling this
 *	function. So this function assumes the contact bounds specify
 *	the object to have a landable/walkable surface.
 */
float SARSimSupportSurfaceHeight(
	sar_object_struct *obj_ptr,		/* Target */
	const sar_contact_bounds_struct *cb,	/* Target */
	const sar_position_struct *pos,		/* Source */
	float z_tolor				/* In meters */
)
{
	float dx, dy, dx2, dy2, dr, cr;
	float x_min, x_max, y_min, y_max;
	float cur_z_height = 0.0f;
	const sar_contact_bounds_struct *cb_tar = cb;
	const sar_position_struct	*pos_src = pos,
					*pos_tar = &obj_ptr->pos;

	if(z_tolor < 0.0f)
	    z_tolor = 0.0f;

	/* Handle by target object contact bounds shape */
	switch(cb_tar->contact_shape)
	{
	  case SAR_CONTACT_SHAPE_SPHERICAL:
	    dx = pos_src->x - pos_tar->x;
	    dy = pos_src->y - pos_tar->y;
	    cr = cb_tar->contact_radius;
	    dr = (float)SFMHypot2(dx, dy);

	    /* Delta distance apart equal or less than contact radius? */
	    if(dr <= cr)
	    {
		/* Use spherical radius as contact z height offset */
		float check_z = pos_tar->z + cr;

		/* Is source position is under the target surface? */
		if((pos_src->z + z_tolor) < check_z)
		{
		    /* Source object is under, so do not account for
		     * surface
		     *
		     * This will make any source position under
		     * the target surface "hollow"
		     */
		}
		else
		{
		    cur_z_height = check_z;
		}
	    }
	    break;

	  case SAR_CONTACT_SHAPE_CYLENDRICAL:
	    dx = pos_src->x - pos_tar->x;
	    dy = pos_src->y - pos_tar->y;
	    cr = cb_tar->contact_radius;
	    dr = (float)SFMHypot2(dx, dy);

	    /* Delta distance apart equal or less than contact radius? */
	    if(dr <= cr)
	    {
		float check_z = pos_tar->z + cb_tar->contact_h_max;

		/* Is source position is under the target surface? */
		if((pos_src->z + z_tolor) < check_z)
		{
		    /* Source object is under, so do not account for
		     * surface. This will make any source position under
		     * the target surface `hollow'.
		     */
		}
		else
		{
		    cur_z_height = check_z;
		}
	    }
	    break;

	  case SAR_CONTACT_SHAPE_RECTANGULAR:
	    dx = pos_src->x - pos_tar->x; 
	    dy = pos_src->y - pos_tar->y;

	    /* Is source object under the top surface of the target
	     * object?
	     */
	    if((pos_src->z + z_tolor) < (pos_tar->z + cb_tar->contact_z_max))
	    {
		/* Source object is under, so do not account for surface */
	    }
	    else
	    {
		/* Rotate dx and dy about the center of the target
		 * object inversly incase target object has a rotated
		 * heading.
		 */
		dx2 = (cb_tar->cos_heading * dx) +
		    (cb_tar->sin_heading * dy);
		dy2 = (cb_tar->cos_heading * dy) -
		    (cb_tar->sin_heading * dx);

		/* Calculate x and y bounds */
		x_min = cb_tar->contact_x_min;
		x_max = cb_tar->contact_x_max;
		y_min = cb_tar->contact_y_min;
		y_max = cb_tar->contact_y_max;

		/* Check if rotated dx2 and dy2 are in bounds */
		if((dx2 >= x_min) && (dx2 <= x_max) &&
		   (dy2 >= y_min) && (dy2 <= y_max)
		)
		    cur_z_height = pos_tar->z + cb_tar->contact_z_max;
	    }
	    break;
	}

	return(cur_z_height);
}


/*
 *	Returns the elevation of (not to) the object of type
 *	ground given the position pos (z coordinate will be
 *	ignored and msl elevation offset will be ignored).
 *
 *	The object must be of type ground, the return value will
 *	be either the ground object's set elevation or from
 *	its heighfield data (if any), whichever is higher.
 */
float SARSimHFGetGroundHeight(
	sar_object_struct *obj_ptr,
	const sar_position_struct *pos
)
{
	float d, dx, dy;
	float cur_z_height = 0.0, z_result;
	float contact_radius;
	const sar_position_struct *pos_tar = pos, *pos_src;
	sar_object_ground_struct *obj_ground_ptr;

	/* Is this object of type ground? */
	obj_ground_ptr = SAR_OBJ_GET_GROUND(obj_ptr);
	if(obj_ground_ptr == NULL)
	    return(cur_z_height);

	if(obj_ground_ptr == NULL)
	    return(cur_z_height);

	/* Note: Target refers to the specified position and source
	 * refers to the SAR_OBJ_TYPE_GROUND object
	 */

	/* Get flat contact radius of ground object */
	contact_radius = SARSimGetFlatContactRadius(obj_ptr);

	/* All SAR_OBJ_TYPE_GROUND objects have cylendrical solid
	 * surfaces, where the radius is specified by the ground
	 * object's contact bounds (which can be a radius of 0.0)
	 *
	 * Get position of ground object and check distance,
	 * see if outside the ground object's contact radius
	 */
	pos_src = &obj_ptr->pos;
	/* Calculate deltas, target position to source position.
	 * Note dy needs to be `top left' oriented
	 */
	dx = pos_tar->x - (pos_src->x + obj_ground_ptr->x_trans);
	dy = (pos_src->y + obj_ground_ptr->y_trans) - pos_tar->y;
	d = (float)SFMHypot2(dx, dy);
	if(d <= contact_radius)
	{
	    /* Compare flat elevation first, by checking the ground
	     * object's specified elevation from 0. Note that ground
	     * objects completly ignore the scene structure defined
	     * ground_elevation_msl value
	     */
	    if((obj_ground_ptr->elevation + pos_src->z +
		obj_ground_ptr->z_trans) > cur_z_height
	    )
		cur_z_height = obj_ground_ptr->elevation + pos_src->z +
		    obj_ground_ptr->z_trans;
	}

	/* Get heightfield Z position of target position */
	z_result = (float)V3DHFGetHeightFromWorldPosition(
	    pos_tar->x, pos_tar->y,	/* The world position */
	    pos_src->x + obj_ground_ptr->x_trans,
	    pos_src->y + obj_ground_ptr->y_trans,
	    pos_src->z + obj_ground_ptr->z_trans,
	    obj_ptr->dir.heading,
	    obj_ground_ptr->x_len, obj_ground_ptr->y_len, 
	    obj_ground_ptr->grid_z_spacing,
	    obj_ground_ptr->grid_points_x, obj_ground_ptr->grid_points_y,
	    obj_ground_ptr->z_point_value
	);
	if(cur_z_height < z_result)
	    cur_z_height = z_result;

	return(cur_z_height);
}
