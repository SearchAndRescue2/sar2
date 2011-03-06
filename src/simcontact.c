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
#include <math.h>

#include "sarreality.h"

#include "obj.h"
#include "objutils.h"

#include "simcb.h"
#include "simutils.h"
#include "simcontact.h"


int SARSimCrashContactCheck(
	sar_core_struct *core_ptr,
	sar_object_struct *obj_ptr
);
int SARSimHoistDeploymentContactCheck(
	sar_core_struct *core_ptr,
	sar_object_struct *obj_ptr
);


#define MAX(a,b)        (((a) > (b)) ? (a) : (b))
#define MIN(a,b)        (((a) < (b)) ? (a) : (b))
#define CLIP(a,l,h)     (MIN(MAX((a),(l)),(h)))


/*
 *      Checks if object obj_ptr has come in contact with another object
 *      on the given core structure.
 *                  
 *      Returns the object number of the object that has come in
 *      contact with or -1 if there was no contact.
 *
 *      Proper procedure will be performed if the object has crashed.
 */
int SARSimCrashContactCheck(
	sar_core_struct *core_ptr, sar_object_struct *obj_ptr
)
{
	int i;
	float dr, z, d;
	float z1_min, z1_max, z2_min, z2_max;
	sar_object_struct *obj_ptr2;
	const sar_contact_bounds_struct	*cb_src = obj_ptr->contact_bounds,
					*cb_tar;
	const sar_position_struct	*pos_src = &obj_ptr->pos,
					*pos_tar;

	/* If there is no contact bounds on the source object then
	 * there is no need to do contact check
	 */
	if(cb_src == NULL)
	    return(-1);

	/* Handle by source contact shape */
	switch(cb_src->contact_shape)
	{
	  case SAR_CONTACT_SHAPE_SPHERICAL:
	    /* Iterate through objects */
	    for(i = 0; i < core_ptr->total_objects; i++)
	    {
		obj_ptr2 = core_ptr->object[i];
		if(obj_ptr2 == NULL)
		    continue;

		cb_tar = obj_ptr2->contact_bounds;
		if(cb_tar == NULL)
		    continue;

		/* Target not crashable? */
		if(!(cb_tar->crash_flags & SAR_CRASH_FLAG_CRASH_CAUSE))
		    continue;

		/* Skip ourself */
		if(obj_ptr2 == obj_ptr)
		    continue;

		pos_tar = &obj_ptr2->pos;

		/* Handle by target object contact shape */
		switch(cb_tar->contact_shape)
		{
		  /* Spherical to Spherical  */
		  case SAR_CONTACT_SHAPE_SPHERICAL:
		    /* Check 2d distance for contact first */
		    dr = (float)SFMHypot2(
			pos_tar->x - pos_src->x,
			pos_tar->y - pos_src->y
		    );
		    if((dr - cb_src->contact_radius - cb_tar->contact_radius)
			> 0.0f
		    )
			break;

		    /* Calculate spherical radius and then check if
		     * in contact
		     */  
		    z = pos_tar->z - pos_src->z;
		    d = (float)SFMHypot2(dr, z);
		    if((d - cb_src->contact_radius - cb_tar->contact_radius)
			<= 0.0f
		    )
		    {
			SARSimObjectCollisionCB(
			    core_ptr, obj_ptr, obj_ptr2, 1.1f
			);
			return(i);
		    }
		    break;

		  /* Spherical to Cylendrical */
		  case SAR_CONTACT_SHAPE_CYLENDRICAL:
		    /* Check 2d distance for contact first */
		    dr = (float)SFMHypot2(
			pos_tar->x - pos_src->x, 
			pos_tar->y - pos_src->y
		    );
		    if((dr - cb_src->contact_radius - cb_tar->contact_radius)
			> 0.0f
		    )
			break;

		    /* Sloppy check on vertical height */
		    z = pos_tar->z;
		    if((pos_src->z >= (z + cb_tar->contact_h_min)) &&
		       (pos_src->z <= (z + cb_tar->contact_h_max))
		    )
		    {
			SARSimObjectCollisionCB(
			    core_ptr, obj_ptr, obj_ptr2, 1.1
			);
			return(i);
		    }
		    break;

		  /* Spherical to Rectangular */
		  case SAR_CONTACT_SHAPE_RECTANGULAR:
		    /* We'll be treating the source object as a square
		     * object based on its contact radius.
		     */
		    /* First check z bounds. */
		    z1_min = pos_src->z - cb_src->contact_radius;
		    z1_max = pos_src->z + cb_src->contact_radius;
		    z2_min = pos_tar->z + cb_tar->contact_z_min;  
		    z2_max = pos_tar->z + cb_tar->contact_z_max;
		    if((z1_min <= z2_max) &&
		       (z2_min <= z1_max)
		    ) 
		    {
			/* Calculate relative deltas from target object
			 * to source object.
			 */   
			float	dx = pos_src->x - pos_tar->x,
				dy = pos_src->y - pos_tar->y,
				src_radius = cb_src->contact_radius;
			float dx2, dy2, x_min, x_max, y_min, y_max;

			/* Rotate dx and dy about the center of the target
			 * object inversly incase target object has
			 * a rotated heading.
			 */
			dx2 = (cb_tar->cos_heading * dx) +  
			    (cb_tar->sin_heading * dy);
			dy2 = (cb_tar->cos_heading * dy) -
			    (cb_tar->sin_heading * dx);

			/* Calculate x and y bounds, add source object's
			 * contact bounds to increase the bounds.
			 */
			x_min = cb_tar->contact_x_min - src_radius;
			x_max = cb_tar->contact_x_max + src_radius;
			y_min = cb_tar->contact_y_min - src_radius;
			y_max = cb_tar->contact_y_max + src_radius;

			/* Check if rotated dx2 and dy2 are in bounds. */
			if((dx2 >= x_min) && (dx2 <= x_max) &&
			   (dy2 >= y_min) && (dy2 <= y_max)
			)
			{
			    SARSimObjectCollisionCB(
				core_ptr, obj_ptr, obj_ptr2, 1.1f
			    );
			    return(i);
			}
		    }
		    break;
		}
	    }
	    break;

	  case SAR_CONTACT_SHAPE_CYLENDRICAL:
	    /* Iterate through objects */
	    for(i = 0; i < core_ptr->total_objects; i++)
	    {
		obj_ptr2 = core_ptr->object[i];
		if(obj_ptr2 == NULL)
		    continue;

		cb_tar = obj_ptr2->contact_bounds;
		if(cb_tar == NULL)
		    continue;

		/* Target not crashable? */
		if(!(cb_tar->crash_flags & SAR_CRASH_FLAG_CRASH_CAUSE))
		    continue;

		/* Skip ourself */
		if(obj_ptr2 == obj_ptr)
		    continue;

		pos_tar = &obj_ptr2->pos;

		/* Handle by target object contact shape */
		switch(cb_tar->contact_shape)
		{
		  /* Cylendrical to Spherical */
		  case SAR_CONTACT_SHAPE_SPHERICAL:
		    /* Check 2d distance for contact first */
		    dr = (float)SFMHypot2(
			pos_tar->x - pos_src->x,   
			pos_tar->y - pos_src->y
		    );
		    if((dr - cb_src->contact_radius - cb_tar->contact_radius)
			> 0.0f
		    )
			break;

		    /* Sloppy check on vertical height */
                    /* Added contact radius check */
		    z = pos_tar->z;
		    if((z + cb_tar->contact_radius >= (pos_src->z + cb_src->contact_h_min)) &&
                        (z - cb_tar->contact_radius <= (pos_src->z + cb_src->contact_h_max))
                       
		    )
		    {
			SARSimObjectCollisionCB(
			    core_ptr, obj_ptr, obj_ptr2, 1.1f
			);
			return(i);
		    }
		    break;

		  /* Cylendrical to Cylendrical */
		  case SAR_CONTACT_SHAPE_CYLENDRICAL:
		    /* Check 2d distance for contact first */
		    dr = (float)SFMHypot2(
			pos_tar->x - pos_src->x,
			pos_tar->y - pos_src->y
		    );
		    if((dr - cb_src->contact_radius - cb_tar->contact_radius)
			> 0.0f
		    )
			break;

		    /* Get height bounds */ 
		    z1_min = pos_src->z + cb_src->contact_h_min;
		    z1_max = pos_src->z + cb_src->contact_h_max;
		    z2_min = pos_tar->z + cb_tar->contact_h_min;
		    z2_max = pos_tar->z + cb_tar->contact_h_max;
		    /* Check height bounds */
		    if((z1_min <= z2_max) &&
		       (z2_min <= z1_max)
		    )
		    {
			SARSimObjectCollisionCB(
			    core_ptr, obj_ptr, obj_ptr2, 1.1
			);
			return(i);
		    }
		    break;

		  /* Cylendrical to Rectangular */
		  case SAR_CONTACT_SHAPE_RECTANGULAR:
		    /* First check z bounds */
		    z1_min = pos_src->z + cb_src->contact_h_min;
		    z1_max = pos_src->z + cb_src->contact_h_max;
		    z2_min = pos_tar->z + cb_tar->contact_z_min;  
		    z2_max = pos_tar->z + cb_tar->contact_z_max;
		    if((z1_min <= z2_max) &&
		       (z2_min <= z1_max)
		    ) 
		    {
			/* Calculate relative deltas from target object
			 * to source object
			 */
			float	dx = pos_src->x - pos_tar->x,
				dy = pos_src->y - pos_tar->y,
				src_radius = cb_src->contact_radius;
			float	dx2, dy2, x_min, x_max, y_min, y_max;

			/* Rotate dx and dy about the center of the target
			 * object inversly incase target object has
			 * a rotated heading
			 */
			dx2 = (cb_tar->cos_heading * dx) +
			    (cb_tar->sin_heading * dy);
			dy2 = (cb_tar->cos_heading * dy) -
			    (cb_tar->sin_heading * dx);

			/* Check if rotated dx and dy are in bounds
			 * with the target object contact bounds.
			 * Increase the bounds by the cylendrical
			 * radius of the source object.
			 */
			x_min = cb_tar->contact_x_min - src_radius;
			x_max = cb_tar->contact_x_max + src_radius;
			y_min = cb_tar->contact_y_min - src_radius;
			y_max = cb_tar->contact_y_max + src_radius;

			if((dx2 >= x_min) && (dx2 <= x_max) &&
			   (dy2 >= y_min) && (dy2 <= y_max)
			)
			{
			    SARSimObjectCollisionCB(
				core_ptr, obj_ptr, obj_ptr2, 1.1
			    );
			    return(i);
			}
		    }
		    break;
		}
	    }
	    break;

	  case SAR_CONTACT_SHAPE_RECTANGULAR:
	    /* Iterate through objects */
	    for(i = 0; i < core_ptr->total_objects; i++)
	    {
		obj_ptr2 = core_ptr->object[i];
		if(obj_ptr2 == NULL)
		    continue;

		cb_tar = obj_ptr2->contact_bounds;
		if(cb_tar == NULL)
		    continue;

		/* Target not crashable? */
		if(!(cb_tar->crash_flags & SAR_CRASH_FLAG_CRASH_CAUSE))
		    continue;

		/* Skip ourself */
		if(obj_ptr2 == obj_ptr)
		    continue;

		pos_tar = &obj_ptr2->pos;

		/* Handle by target object contact shape */
		switch(cb_tar->contact_shape)
		{
		  /* Rectangular to Spherical  */
		  case SAR_CONTACT_SHAPE_SPHERICAL:
		    /* We'll be treating the source object as a square
		     * object based on its contact radius.
		     */
		    /* First check z bounds. */
		    z1_min = pos_src->z + cb_src->contact_z_min;
		    z1_max = pos_src->z + cb_src->contact_z_max;
		    z2_min = pos_tar->z - cb_tar->contact_radius;
		    z2_max = pos_tar->z + cb_tar->contact_radius;
		    if((z1_min <= z2_max) &&
		       (z2_min <= z1_max)
		    )
		    {
			/* Calculate relative deltas from source object
			 * to target object
			 */
			float   dx = pos_tar->x - pos_src->x,
				dy = pos_tar->y - pos_src->y,
				tar_radius = cb_tar->contact_radius;
			float dx2, dy2, x_min, x_max, y_min, y_max;

			/* Rotate dx and dy about the center of the source
			 * object inversly if source object has a rotated
			 * heading
			 */
			dx2 = (cb_src->cos_heading * dx) +
			    (cb_src->sin_heading * dy);
			dy2 = (cb_src->cos_heading * dy) -
			    (cb_src->sin_heading * dx);

			/* Calculate x and y bounds, add target object's
			 * contact bounds to increase the bounds.
			 */
			x_min = cb_src->contact_x_min - tar_radius;
			x_max = cb_src->contact_x_max + tar_radius;
			y_min = cb_src->contact_y_min - tar_radius;
			y_max = cb_src->contact_y_max + tar_radius;

			/* Check if rotated dx2 and dy2 are in bounds. */
			if((dx2 >= x_min) && (dx2 <= x_max) &&
			   (dy2 >= y_min) && (dy2 <= y_max)
			)
			{
			    SARSimObjectCollisionCB(
				core_ptr, obj_ptr, obj_ptr2, 1.1f
			    );
			    return(i);
			}
		    }
		    break;

		  /* Rectangular to Cylendrical  */
		  case SAR_CONTACT_SHAPE_CYLENDRICAL:
		    /* First check z bounds */
		    z1_min = pos_src->z + cb_src->contact_z_min;
		    z1_max = pos_src->z + cb_src->contact_z_max;
		    z2_min = pos_tar->z + cb_tar->contact_h_min;
		    z2_max = pos_tar->z + cb_tar->contact_h_max;
		    if((z1_min <= z2_max) &&
		       (z2_min <= z1_max)
		    )
		    {
			/* Calculate relative deltas from source object
			 * to target object
			 */
			float   dx = pos_tar->x - pos_src->x,
				dy = pos_tar->y - pos_src->y,
				tar_radius = cb_tar->contact_radius;
			float   dx2, dy2, x_min, x_max, y_min, y_max;

			/* Rotate dx and dy about the center of the source
			 * object inversly if target object has a rotated
			 * heading
			 */
			dx2 = (cb_src->cos_heading * dx) +
			    (cb_src->sin_heading * dy);
			dy2 = (cb_src->cos_heading * dy) -
			    (cb_src->sin_heading * dx);

			/* Check if rotated dx and dy are in bounds
			 * with the target object contact bounds.
			 * Increase the bounds by the cylendrical
			 * radius of the source object.
			 */
			x_min = cb_src->contact_x_min - tar_radius;
			x_max = cb_src->contact_x_max + tar_radius;
			y_min = cb_src->contact_y_min - tar_radius;
			y_max = cb_src->contact_y_max + tar_radius;

			if((dx2 >= x_min) && (dx2 <= x_max) &&
			   (dy2 >= y_min) && (dy2 <= y_max)
			)
			{
			    SARSimObjectCollisionCB(
				core_ptr, obj_ptr, obj_ptr2, 1.1
			    );
			    return(i);
			}
		    }
		    break;

		  /* Rectangular to Rectangular  */
		  case SAR_CONTACT_SHAPE_RECTANGULAR:
/* TODO */
		    break;
		}
	    }
	    break;
	}

	return(-1);
}


/*
 *	Hoist deployment contact check.
 *
 *	Checks if an object that can be picked up is within range of the
 *	hoist's deployment to be picked up by the hoist's deployment.
 *
 *	If an object has come in contact then the proper procedure will
 *	be taken.
 *
 *      Inputs assumed valid.
 *
 *	Returns the object number of the picked up object or -1 on no
 *	match.
 */
int SARSimHoistDeploymentContactCheck(
	sar_core_struct *core_ptr, sar_object_struct *obj_ptr
)
{
	Boolean need_break;
	int tar_obj_num, matched_obj_num = -1;
	float d, contact_radius;
	sar_object_struct *tar_obj_ptr;
	sar_object_human_struct *human_ptr;
	sar_obj_hoist_struct *hoist;
	const sar_position_struct *pos_src, *pos_tar;
	const sar_contact_bounds_struct *cb_tar;
	sar_scene_struct *scene = core_ptr->scene;
	const sar_option_struct *opt = &core_ptr->option;

	/* Get first hoist on object, if the object does not have a
	 * hoist then that there is nothing to check
	 */
	hoist = SARObjGetHoistPtr(obj_ptr, 0, NULL);
	if(hoist == NULL)
	    return(matched_obj_num);

	/* Hoist fully retracted? */
	if(hoist->rope_cur <= 0.0f)
	    return(matched_obj_num);

	/* Get hoist deployment's flat contact radius
	 *
	 * If this is the player object then multiply the contact radius
	 * with the hoist contact expansion coefficient
	 */
	contact_radius = hoist->contact_radius * 2.0f *
	    ((scene->player_obj_ptr == obj_ptr) ?
		MAX(opt->hoist_contact_expansion_coeff, 1.0f) : 1.0f);

	pos_src = &hoist->pos;	/* Hoist deployment's position */


	/* Iterate through objects, looking for one that can be picked
	 * up by the hoist's deployment
	 */
	for(tar_obj_num = 0; tar_obj_num < core_ptr->total_objects; tar_obj_num++)
	{
	    tar_obj_ptr = core_ptr->object[tar_obj_num];
	    if(tar_obj_ptr == NULL)
		continue;

	    /* Get contact bounds for target object */
	    cb_tar = tar_obj_ptr->contact_bounds;
	    if(cb_tar == NULL)
		continue;

#if 0
/* It is safe to bypass this check */
	    /* Contact shapes for target object must be cylendrical or
	     * spherical shaped
	     */
	    if((cb_tar->contact_shape != SAR_CONTACT_SHAPE_SPHERICAL) &&
	       (cb_tar->contact_shape != SAR_CONTACT_SHAPE_CYLENDRICAL)
	    )
		continue;
#endif

	    /* Handle by target object type */
	    switch(tar_obj_ptr->type)
	    {
	      case SAR_OBJ_TYPE_GARBAGE:
	      case SAR_OBJ_TYPE_STATIC:
	      case SAR_OBJ_TYPE_AUTOMOBILE:
	      case SAR_OBJ_TYPE_WATERCRAFT:
	      case SAR_OBJ_TYPE_AIRCRAFT:
	      case SAR_OBJ_TYPE_GROUND:
	      case SAR_OBJ_TYPE_RUNWAY:
	      case SAR_OBJ_TYPE_HELIPAD:
		break;

	      case SAR_OBJ_TYPE_HUMAN:
		human_ptr = SAR_OBJ_GET_HUMAN(tar_obj_ptr);
		if(human_ptr == NULL)
		    break;

		/* Does not want to be picked up? */
		if(!(human_ptr->flags & SAR_HUMAN_FLAG_NEED_RESCUE))
		    break;

		pos_tar = &tar_obj_ptr->pos;

		/* Check contact bounds by target object's contact
		 * shape
		 *
		 * Reset need_break to False, it will be to True if
		 * there was NO contact
		 */
		need_break = False;
		switch(cb_tar->contact_shape)
		{
		  case SAR_CONTACT_SHAPE_SPHERICAL:
		    /* Calculate distance */
		    d = (float)SFMHypot2(
			pos_tar->x - pos_src->x,
			pos_tar->y - pos_src->y
		    );
		    /* Within rescue basket pickup distance? */
		    if((d - contact_radius - cb_tar->contact_radius) >
			0.0f
		    )
		    {
			need_break = True;
			break;
		    }
		    /* Is target object under the rescue basket? */
		    if((pos_tar->z + cb_tar->contact_radius) <
			(pos_src->z + hoist->contact_z_min)
		    )
		    {
			need_break = True;
			break;
		    }
		    break;

		  case SAR_CONTACT_SHAPE_CYLENDRICAL:
		    /* Calculate distance */
		    d = (float)SFMHypot2(
			pos_tar->x - pos_src->x,
			pos_tar->y - pos_src->y
		    );
		    /* Within rescue basket pickup distance? */
		    if((d - contact_radius - cb_tar->contact_radius) >
			0.0f
		    )
		    {
			need_break = True;
			break;
		    }
		    /* Is target object under the rescue basket? */
		    if((pos_tar->z + cb_tar->contact_h_max) <
			(pos_src->z + hoist->contact_z_min) 
		    )
		    {
			need_break = True;
			break;
		    }
		    break;

		  case SAR_CONTACT_SHAPE_RECTANGULAR:
		    /* Calculate distance */
		    d = (float)SFMHypot2(
			pos_tar->x - pos_src->x,
			pos_tar->y - pos_src->y
		    );
		    /* Within rescue basket pickup distance? */
/* Using target object's x bounds for now */
		    if((d - contact_radius -
			(cb_tar->contact_x_max - cb_tar->contact_x_min)) >
			0.0f
		    )
		    {
			need_break = True;
			break;
		    }
		    /* Is target object under the rescue basket? */
		    if((pos_tar->z + cb_tar->contact_z_max) <
			(pos_src->z + hoist->contact_z_min)
		    )
		    {
			need_break = True;
			break;
		    }
		    break;
		}
		if(need_break)
		    break;

		/* Put human object into the hoist deployment (if
		 * possible) and update the human object to mark it
		 * as no longer needing rescue
		 */
		if(SARSimDoPickUpHuman(scene, obj_ptr, human_ptr, tar_obj_num) > 0)
		{
		    matched_obj_num = tar_obj_num;
		    return(matched_obj_num);
		}
		break;

	      case SAR_OBJ_TYPE_SMOKE:
	      case SAR_OBJ_TYPE_FIRE:
	      case SAR_OBJ_TYPE_EXPLOSION:
	      case SAR_OBJ_TYPE_CHEMICAL_SPRAY:
	      case SAR_OBJ_TYPE_FUELTANK:
	      case SAR_OBJ_TYPE_PREMODELED:
		break;
	    }
	}

	return(matched_obj_num);
}

