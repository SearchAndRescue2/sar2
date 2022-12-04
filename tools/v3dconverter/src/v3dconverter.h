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


#define VERSION "0.3"

#include <linux/limits.h> // for PATH_MAX
#define MAXPATHLENGTH PATH_MAX

#define MAX_LENGTH 2048
#define MAX_PARAMETER_LINES 6

#define OBJ_OBJECT_NAME_TERRAIN "Terrain"
#define OBJ_OBJECT_NAME_V3DCONVERTER_DATA "v3dConverter_data_DO_NOT_REMOVE"

typedef struct UserModifier UserModifier;
struct UserModifier {
    double rx;
    double ry;
    double rz;
    double tx;
    double ty;
    double tz;
    double scale;
    unsigned short invertFaces;
};

// vertex geometric coords
typedef struct Vcoord Vcoord;
struct Vcoord {
    double x;
    double y;
    double z;
};

// normal direction
typedef struct Vnormal Vnormal;
struct Vnormal {
    double i;
    double j;
    double k;
};

// texture coords
typedef struct Tcoord Tcoord;
struct Tcoord {
    double u;
    double v;
    double w; // not mandatory
};

// whole vertex datas (vertex coords, normal, texture coords)
typedef struct Vertex Vertex;
struct Vertex {
    long v;
    long vn;
    long vt;
};

// color
typedef struct Color Color;
struct Color {
    double r;   // red
    double g;   // green
    double b;   // blue
    double a;   // alpha (transparency)
    double amb; // ambiant
    double dif; // diffuse
    double spe; // specular
    double shi; // shininess
    double emi; // emission
};
