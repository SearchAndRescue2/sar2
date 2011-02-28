#include <sys/types.h>
#include <math.h>
#ifdef __MSW__
# include <windows.h>
#endif
#include <GL/gl.h>
#include <GL/glu.h>
#include "gw.h"
#include "sfm.h"
#include "obj.h"
#include "sar.h"                                     
#include "sardraw.h"
#include "sardrawdefs.h"
#include "config.h"


void SARDrawGetDirFromPos(
	const sar_position_struct *pos1, const sar_position_struct *pos2,
	sar_direction_struct *dir_rtn
);
void SARDrawSetColor(const sar_color_struct *c);
void SARDrawSetColorFLIRTemperature(float t);

void SARDrawBoxNS(float x_len, float y_len, float z_len);
void SARDrawBoxBaseNS(
	float x_len, float y_len, float z_len,
	Boolean draw_base
);


#define SWAP_2F(x,y)	{	\
 float t = x;			\
 x = y; y = t;			\
}


/*
 *      Calculates the heading and pitch of position pos1 to
 *      position pos2, storing the results in dir_rtn.
 */
void SARDrawGetDirFromPos(
	const sar_position_struct *pos1, const sar_position_struct *pos2,
	sar_direction_struct *dir_rtn                                    
)
{
	float r;
	sar_position_struct delta;

	if((pos1 == NULL) || (pos2 == NULL) || (dir_rtn == NULL))
	    return;

	delta.x = pos2->x - pos1->x;
	delta.y = pos2->y - pos1->y;
	delta.z = pos2->z - pos1->z;
	r = (float)SFMHypot2(delta.x, delta.y);

	dir_rtn->heading = (float)SFMSanitizeRadians(
	    (0.5 * PI) - atan2(delta.y, delta.x)
	);
	dir_rtn->pitch = (float)SFMSanitizeRadians(
	    -atan2(delta.z, r)
	);                    
	dir_rtn->bank = (float)(0.0 * PI);
}

/*
 *	Set the GL color from the specified SAR color.
 */
void SARDrawSetColor(const sar_color_struct *c)
{ 
	if(c != NULL)
	    glColor4f(c->r, c->g, c->b, c->a);
}  

/*
 *	Sets the GL color from the specified temperature.
 *
 *	The temperature t must be from 0.0 coldest to 1.0 hottest.
 */
void SARDrawSetColorFLIRTemperature(float t)
{
	float	l = CLIP((t - 0.5f) * 2.0f, 0.0f, 1.0f),
		h = CLIP(t * 2.0f, 0.0f, 1.0f);
	glColor4f(l, h, l, 1.0f);
}


/*
 *      Draws a box at the center, normals set for smooth finish.
 */
void SARDrawBoxNS(
	float x_len, float y_len, float z_len
)
{
	float x_min, x_max;
	float y_min, y_max;
	float z_min, z_max;

	x_max = x_len / 2.0f;
	x_min = -x_max;
	if(x_max < x_min)
	    SWAP_2F(x_min, x_max);

	y_max = y_len / 2.0f;
	y_min = -y_max;
	if(y_max < y_min)
	    SWAP_2F(y_min, y_max);

	z_max = z_len / 2.0f;
	z_min = -z_max;
	if(z_max < z_min)
	    SWAP_2F(z_min, z_max);

	/* Front and back */
	glNormal3f(-0.33f, -0.33f, -0.88f);
	glVertex3f(x_min, z_min, -y_max);
	glNormal3f(-0.33f, 0.33f, -0.88f);
	glVertex3f(x_min, z_max, -y_max);
	glNormal3f(0.33f, 0.33f, -0.88f);
	glVertex3f(x_max, z_max, -y_max);
	glNormal3f(0.33f, -0.33f, -0.88f);
	glVertex3f(x_max, z_min, -y_max);

	glNormal3f(-0.33f, 0.33f, 0.88f);
	glVertex3f(x_min, z_max, -y_min);
	glNormal3f(-0.33f, -0.33f, 0.88f);
	glVertex3f(x_min, z_min, -y_min);
	glNormal3f(0.33f, -0.33f, 0.88f);
	glVertex3f(x_max, z_min, -y_min);
	glNormal3f(0.33f, 0.33f, 0.88f);
	glVertex3f(x_max, z_max, -y_min);

	/* Left and right */
	glNormal3f(-0.88f, 0.33f, -0.33f);
	glVertex3f(x_min, z_max, -y_max);
	glNormal3f(-0.88f, -0.33f, -0.33f);
	glVertex3f(x_min, z_min, -y_max);
	glNormal3f(-0.88f, -0.33f, 0.33f);
	glVertex3f(x_min, z_min, -y_min);
	glNormal3f(-0.88f, 0.33f, 0.33f);
	glVertex3f(x_min, z_max, -y_min);

	glNormal3f(0.88f, -0.33f, -0.33f);
	glVertex3f(x_max, z_min, -y_max);
	glNormal3f(0.88f, 0.33f, -0.33f);
	glVertex3f(x_max, z_max, -y_max);
	glNormal3f(0.88f, 0.33f, 0.33f);
	glVertex3f(x_max, z_max, -y_min);
	glNormal3f(0.88f, -0.33f, 0.33f);
	glVertex3f(x_max, z_min, -y_min);

	/* Top and bottom */
	glNormal3f(-0.33f, 0.88f, 0.33f);
	glVertex3f(x_min, z_max, -y_min);
	glNormal3f(0.33f, 0.88f, 0.33f);
	glVertex3f(x_max, z_max, -y_min);
	glNormal3f(0.33f, 0.88f, -0.33f);
	glVertex3f(x_max, z_max, -y_max);
	glNormal3f(-0.33f, 0.88f, -0.33f);
	glVertex3f(x_min, z_max, -y_max);

	glNormal3f(-0.33f, -0.88f, -0.33f);
	glVertex3f(x_min, z_min, -y_max);
	glNormal3f(0.33f, -0.88f, -0.33f);
	glVertex3f(x_max, z_min, -y_max);
	glNormal3f(0.33f, -0.88f, 0.33f);
	glVertex3f(x_max, z_min, -y_min);
	glNormal3f(-0.33f, -0.88f, 0.33f);
	glVertex3f(x_min, z_min, -y_min);
}

/*
 *	Draws a box in the positive octant with respect to the xy
 *	plane, the xy walls are centered, normals are set for smooth
 *	finish.
 *
 *	If draw_base is False then the bottom of the box is not closed.
 */
void SARDrawBoxBaseNS(
	float x_len, float y_len, float z_len,
	Boolean draw_base
)
{
	float x_min, x_max;
	float y_min, y_max;
	float z_min, z_max;

	x_max = x_len / 2;
	x_min = -x_max;
	if(x_max < x_min)
	    SWAP_2F(x_min, x_max);

	y_max = y_len / 2;
	y_min = -y_max;
	if(y_max < y_min)
	    SWAP_2F(y_min, y_max);

	z_min = 0;
	z_max = z_len;
	if(z_max < z_min)
	    SWAP_2F(z_min, z_max);

	/* Front and back */
	glNormal3f(-0.33f, -0.33f, -0.88f);
	glVertex3f(x_min, z_min, -y_max);
	glNormal3f(-0.33f, 0.33f, -0.88f);
	glVertex3f(x_min, z_max, -y_max);
	glNormal3f(0.33f, 0.33f, -0.88f);
	glVertex3f(x_max, z_max, -y_max);
	glNormal3f(0.33f, -0.33f, -0.88f);
	glVertex3f(x_max, z_min, -y_max);

	glNormal3f(-0.33f, 0.33f, 0.88f);
	glVertex3f(x_min, z_max, -y_min);
	glNormal3f(-0.33f, -0.33f, 0.88f);
	glVertex3f(x_min, z_min, -y_min);
	glNormal3f(0.33f, -0.33f, 0.88f);
	glVertex3f(x_max, z_min, -y_min);
	glNormal3f(0.33f, 0.33f, 0.88f);
	glVertex3f(x_max, z_max, -y_min);

	/* Left and right */
	glNormal3f(-0.88f, 0.33f, -0.33f);
	glVertex3f(x_min, z_max, -y_max);
	glNormal3f(-0.88f, -0.33f, -0.33f);
	glVertex3f(x_min, z_min, -y_max);
	glNormal3f(-0.88f, -0.33f, 0.33f);
	glVertex3f(x_min, z_min, -y_min);
	glNormal3f(-0.88f, 0.33f, 0.33f);
	glVertex3f(x_min, z_max, -y_min);

	glNormal3f(0.88f, -0.33f, -0.33f);
	glVertex3f(x_max, z_min, -y_max);
	glNormal3f(0.88f, 0.33f, -0.33f);
	glVertex3f(x_max, z_max, -y_max);
	glNormal3f(0.88f, 0.33f, 0.33f);
	glVertex3f(x_max, z_max, -y_min);
	glNormal3f(0.88f, -0.33f, 0.33f);
	glVertex3f(x_max, z_min, -y_min);

	/* Top */
	glNormal3f(-0.33f, 0.88f, 0.33f);
	glVertex3f(x_min, z_max, -y_min);
	glNormal3f(0.33f, 0.88f, 0.33f);
	glVertex3f(x_max, z_max, -y_min);
	glNormal3f(0.33f, 0.88f, -0.33f);
	glVertex3f(x_max, z_max, -y_max);
	glNormal3f(-0.33f, 0.88f, -0.33f);
	glVertex3f(x_min, z_max, -y_max);

	/* Bottom */
	if(draw_base)
	{
	    glNormal3f(-0.33f, -0.88f, -0.33f);
	    glVertex3f(x_min, z_min, -y_max);
	    glNormal3f(0.33f, -0.88f, -0.33f);
	    glVertex3f(x_max, z_min, -y_max);
	    glNormal3f(0.33f, -0.88f, 0.33f);
	    glVertex3f(x_max, z_min, -y_min);
	    glNormal3f(-0.33f, -0.88f, 0.33f);
	    glVertex3f(x_min, z_min, -y_min);
	}
}
