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
			SAR Matrix Math Utilities
 */

#ifndef MATRIXMATH_H
#define MATRIXMATH_H

extern void MatrixMulti3Rotate33(double *a, double *b, double *r);

extern void MatrixRotateHeading3(double *a, double theta, double *r);
extern void MatrixRotatePitch3(double *a, double theta, double *r);
extern void MatrixRotateBank3(double *a, double theta, double *r);

extern void MatrixGetHeading33(double theta, double *r);
extern void MatrixGetPitch33(double theta, double *r);
extern void MatrixGetBank33(double theta, double *r);

#endif	/* MATRIXMATH_H */
