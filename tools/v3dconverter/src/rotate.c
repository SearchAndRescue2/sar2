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
#include <math.h>
/*
void RotateX ( double *px, double *py, double *pz, double *pw, double angle ) // Right Handed
{
    double x = 1*(*px) + 0*(*py) + 0*(*pz) + 0*(*pw);
    double y = 0*(*px) + cos(angle*M_PI/180)*(*py) - sin(angle*M_PI/180)*(*pz) + 0*(*pw);
    double z = 0*(*px) + sin(angle*M_PI/180)*(*py) + cos(angle*M_PI/180)*(*pz) + 0*(*pw);
    double w = 0*(*px) + 0*(*py) + 0*(*pz) + 1*(*pw);
    *px = x;
    *py = y;
    *pz = z;
    *pw = w;
}

void RotateY ( double *px, double *py, double *pz, double *pw, double angle ) // Right Handed
{
    double x = cos(angle*M_PI/180)*(*px) + 0*(*py) + sin(angle*M_PI/180)*(*pz) + 0*(*pw);
    double y = 0*(*px) + 1*(*py) + 0*(*pz) + 0*(*pw);
    double z = -sin(angle*M_PI/180)*(*px) + 0*(*py) + cos(angle*M_PI/180)*(*pz) + 0*(*pw);
    double w = 0*(*px) + 0*(*py) + 0*(*pz) + 1*(*pw);
    *px = x;
    *py = y;
    *pz = z;
    *pw = w;
}

void RotateZ ( double *px, double *py, double *pz, double *pw, double angle ) // Right Handed
{
    double x = cos(angle*M_PI/180)*(*px) - sin(angle*M_PI/180)*(*py) + 0*(*pz) + 0*(*pw);
    double y = sin(angle*M_PI/180)*(*px) + cos(angle*M_PI/180)*(*py) - 0*(*pz) + 0*(*pw);
    double z = 0*(*px) + 0*(*py) + 1*(*pz) + 0*(*pw);
    double w = 0*(*px) + 0*(*py) + 0*(*pz) + 1*(*pw);
    *px = x;
    *py = y;
    *pz = z;
    *pw = w;
}
*/

void RotateX ( double *px, double *py, double *pz, double *pw, double angle ) // Left Handed
{
    double x = 1*(*px) + 0*(*py) + 0*(*pz) + 0*(*pw);
    double y = 0*(*px) + cos(angle*M_PI/180)*(*py) + sin(angle*M_PI/180)*(*pz) + 0*(*pw);
    double z = 0*(*px) - sin(angle*M_PI/180)*(*py) + cos(angle*M_PI/180)*(*pz) + 0*(*pw);
    double w = 0*(*px) + 0*(*py) + 0*(*pz) + 1*(*pw);
    *px = x;
    *py = y;
    *pz = z;
    *pw = w;
}

void RotateY ( double *px, double *py, double *pz, double *pw, double angle ) // Left Handed
{
    double x = cos(angle*M_PI/180)*(*px) + 0*(*py) - sin(angle*M_PI/180)*(*pz) + 0*(*pw);
    double y = 0*(*px) + 1*(*py) + 0*(*pz) + 0*(*pw);
    double z = sin(angle*M_PI/180)*(*px) + 0*(*py) + cos(angle*M_PI/180)*(*pz) + 0*(*pw);
    double w = 0*(*px) + 0*(*py) + 0*(*pz) + 1*(*pw);
    *px = x;
    *py = y;
    *pz = z;
    *pw = w;
}

void RotateZ ( double *px, double *py, double *pz, double *pw, double angle ) // Left Handed
{
    double x = cos(angle*M_PI/180)*(*px) + sin(angle*M_PI/180)*(*py) + 0*(*pz) + 0*(*pw);
    double y = -sin(angle*M_PI/180)*(*px) + cos(angle*M_PI/180)*(*py) - 0*(*pz) + 0*(*pw);
    double z = 0*(*px) + 0*(*py) + 1*(*pz) + 0*(*pw);
    double w = 0*(*px) + 0*(*py) + 0*(*pz) + 1*(*pw);
    *px = x;
    *py = y;
    *pz = z;
    *pw = w;
}


// gcc -lm -o rotate rotate.c
/*
void main() {
    double x, y, z, w, angle = 90;
    x = 10;
    y = 20;
    z = 30;
    w = 1;
    
    printf("Before: x=%lf, y=%lf, z=%lf, w=%lf, angle=%lf\n", x, y, z, w, angle );
    RotateX ( &x, &y, &z, &w, 90);
    printf("After: x=%lf, y=%lf, z=%lf, w=%lf, angle=%lf\n", x, y, z, w, angle );
}
*/
