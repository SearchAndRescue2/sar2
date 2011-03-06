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
#include <string.h>

#include <math.h>

#include "matrixmath.h"

#ifdef MEMWATCH
# include "memwatch.h"
#endif


void MatrixMulti3Rotate33(double *a, double *b, double *r);

void MatrixRotateHeading3(double *a, double theta, double *r);
void MatrixRotatePitch3(double *a, double theta, double *r);
void MatrixRotateBank3(double *a, double theta, double *r);

void MatrixGetHeading33(double theta, double *r);
void MatrixGetPitch33(double theta, double *r);
void MatrixGetBank33(double theta, double *r);


/*
 *	Multiplies matrix a (3 by 1) with rotational matrix b (3 by 3)
 *	and stores results into matrix r (3 by 1).
 */
void MatrixMulti3Rotate33(double *a, double *b, double *r)
{
	int i, j;
	int m = 3, n = 3;

	for(j = 0; j < n; j++)
	{
	    r[j] = 0.0;
	    for(i = 0; i < m; i++)
		r[j] += a[i] * b[(i * m) + j];
	}
}

/*
 *	Rotates position matrix a (3 * 1) about z axis clockwise.
 *
 *	Theta is in radians.
 */
void MatrixRotateHeading3(double *a, double theta, double *r)
{
	double b[3 * 3];

	b[0] = cos(theta);
	b[1] = -sin(theta);
	b[2] = 0.0;

	b[3] = -b[1];	/* -sin(theta) */
	b[4] = b[0];	/* cos(theta) */
	b[5] = 0.0;

	b[6] = 0.0;
	b[7] = 0.0;
	b[8] = 1.0;

	MatrixMulti3Rotate33(a, b, r);	
}

/*
 *	Rotates position matrix a (3 * 1) about x axis clockwise.
 *
 *	Theta is in radians.
 */
void MatrixRotatePitch3(double *a, double theta, double *r)
{
	double b[3 * 3];

	b[0] = 1.0;
	b[1] = 0.0;
	b[2] = 0.0;  

	b[3] = 0.0;
	b[4] = cos(theta);
	b[5] = -sin(theta);

	b[6] = 0.0;
	b[7] = -b[5];	/* sin(theta) */
	b[8] = b[4];	/* cos(theta) */

	MatrixMulti3Rotate33(a, b, r);
}       

/*
 *	Rotates position matrix a (3 * 1) about y axis clockwise.
 *
 *	Theta is in radians.
 */
void MatrixRotateBank3(double *a, double theta, double *r)
{
	double b[3 * 3];

	b[0] = cos(theta);
	b[1] = 0.0;
	b[2] = sin(theta);
	
	b[3] = 0.0;
	b[4] = 1.0;
	b[5] = 0.0;

	b[6] = -b[2];	/* -sin(theta) */
	b[7] = 0.0;
	b[8] = b[0];	/* cos(theta) */

	MatrixMulti3Rotate33(a, b, r);
}


/*
 *	Generates a rotation matrix r (3 * 3) about the z axis clockwise.
 *
 *      Theta is in radians.
 */
void MatrixGetHeading33(double theta, double *r)
{
	r[0] = cos(theta);
	r[1] = -sin(theta);
	r[2] = 0.0;

	r[3] = -r[1];   /* -sin(theta) */
	r[4] = r[0];    /* cos(theta) */
	r[5] = 0.0;

	r[6] = 0.0;
	r[7] = 0.0;
	r[8] = 1.0;
}

/*      
 *      Generates a rotation matrix r (3 * 3) about the x axis clockwise.
 *
 *      Theta is in radians.
 */
void MatrixGetPitch33(double theta, double *r)
{
	r[0] = 1.0;
	r[1] = 0.0;
	r[2] = 0.0;

	r[3] = 0.0;
	r[4] = cos(theta);
	r[5] = -sin(theta);

	r[6] = 0.0;
	r[7] = -r[5];   /* sin(theta) */
	r[8] = r[4];    /* cos(theta) */
}

/*
 *      Generates a rotation matrix r (3 * 3) about the y axis clockwise.
 *
 *      Theta is in radians.
 */
void MatrixGetBank33(double theta, double *r)
{
	r[0] = cos(theta);
	r[1] = 0.0;
	r[2] = sin(theta);

	r[3] = 0.0;
	r[4] = 1.0;
	r[5] = 0.0;

	r[6] = -r[2];   /* -sin(theta) */
	r[7] = 0.0;
	r[8] = r[0];    /* cos(theta) */
}
