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
