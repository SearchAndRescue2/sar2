/*
			    V3D Model Primitives

	Data types for basic V3D model primitives used to represent
	data and used for OpenGL style drawing.

	When adding new types, make sure you update V3DMPCreate() and
	V3DMPDup() to allocate the correct size of the data type structure
	and update V3DMPDestroy() to deallocate and members that may point
	to allocated resources. Externally, you may need to update other
	functions.

 */

#ifndef V3DMP_H
#define V3DMP_H

#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/* General purpose vertex structure. */
typedef struct {

	double x, y, z, m;

} mp_vertex_struct;


/* Model primitives structure type codes. */
#define V3DMP_TYPE_COMMENT		1

#define V3DMP_TYPE_TRANSLATE		10
#define V3DMP_TYPE_UNTRANSLATE		11
#define V3DMP_TYPE_ROTATE		12
#define V3DMP_TYPE_UNROTATE		13

#define V3DMP_TYPE_POINT		20
#define V3DMP_TYPE_LINE			21
#define V3DMP_TYPE_LINE_STRIP		22
#define V3DMP_TYPE_LINE_LOOP		23
#define V3DMP_TYPE_TRIANGLE		24
#define V3DMP_TYPE_TRIANGLE_STRIP	25
#define V3DMP_TYPE_TRIANGLE_FAN		26
#define V3DMP_TYPE_QUAD			27
#define V3DMP_TYPE_QUAD_STRIP		28
#define V3DMP_TYPE_POLYGON		29

#define V3DMP_TYPE_COLOR		50
#define V3DMP_TYPE_TEXTURE_SELECT	51
#define V3DMP_TYPE_TEXTURE_ORIENT_XY	52
#define V3DMP_TYPE_TEXTURE_ORIENT_YZ	53
#define V3DMP_TYPE_TEXTURE_ORIENT_XZ	54
#define V3DMP_TYPE_TEXTURE_OFF		55
#define V3DMP_TYPE_HEIGHTFIELD_LOAD	56


/*
 *	Model primitive structures:
 */

/* Comment. */
typedef struct {
	int type;       /* Must be V3DMP_TYPE_COMMENT. */
	char **line;
	int total_lines;
} mp_comment_struct;


/* Translate. */
typedef struct {
	int type;	/* Must be V3DMP_TYPE_TRANSLATE. */
	double x, y, z;	/* In meters. */
} mp_translate_struct;
/* Untranslate. */
typedef struct {
	int type;	/* Must be V3DMP_TYPE_UNTRANSLATE. */
} mp_untranslate_struct;

/* Rotate. */
typedef struct {
	int type;       /* Must be V3DMP_TYPE_ROTATE. */
	double heading, bank, pitch;	/* In radians. */
} mp_rotate_struct;
/* Unrotate. */
typedef struct {
	int type;       /* Must be V3DMP_TYPE_UNROTATE. */
} mp_unrotate_struct;


/* Point. */
typedef struct {
	int type;       /* Must be V3DMP_TYPE_POINT. */
#define V3DMP_POINT_NVERTEX	1
	mp_vertex_struct v[V3DMP_POINT_NVERTEX];
	mp_vertex_struct n[V3DMP_POINT_NVERTEX];
	mp_vertex_struct tc[V3DMP_POINT_NVERTEX];
	double r;
} mp_point_struct;

/* Line. */
typedef struct {
	int type;	/* Must be V3DMP_TYPE_LINE. */
#define V3DMP_LINE_NVERTEX	2
	mp_vertex_struct v[V3DMP_LINE_NVERTEX];
	mp_vertex_struct n[V3DMP_LINE_NVERTEX];
	mp_vertex_struct tc[V3DMP_LINE_NVERTEX];
} mp_line_struct;

/* Line strip. */
typedef struct {
	int type;	/* Must be V3DMP_TYPE_LINE_STRIP. */
	mp_vertex_struct **v;
	mp_vertex_struct **n;
	mp_vertex_struct **tc;
	int total;
} mp_line_strip_struct;

/* Line loop. */
typedef struct {
	int type;       /* Must be V3DMP_TYPE_LINE_LOOP. */
	mp_vertex_struct **v;
	mp_vertex_struct **n;
	mp_vertex_struct **tc;
	int total;
} mp_line_loop_struct;

/* Triangle. */
typedef struct {
	int type;       /* Must be V3DMP_TYPE_TRIANGLE. */
#define V3DMP_TRIANGLE_NVERTEX	3
	mp_vertex_struct v[V3DMP_TRIANGLE_NVERTEX];
	mp_vertex_struct n[V3DMP_TRIANGLE_NVERTEX];
	mp_vertex_struct tc[V3DMP_TRIANGLE_NVERTEX];
} mp_triangle_struct;

/* Triangle strip. */
typedef struct {
	int type;       /* Must be V3DMP_TYPE_TRIANGLE_STRIP. */
	mp_vertex_struct **v;
	mp_vertex_struct **n;
	mp_vertex_struct **tc;
	int total;
} mp_triangle_strip_struct;

/* Triangle fan. */
typedef struct {
	int type;       /* Must be V3DMP_TYPE_TRIANGLE_FAN. */
	mp_vertex_struct **v;
	mp_vertex_struct **n;
	mp_vertex_struct **tc;
	int total;
} mp_triangle_fan_struct;

/* Quad. */
typedef struct {
	int type;	/* Must be V3DMP_TYPE_QUAD. */
#define V3DMP_QUAD_NVERTEX	4
	mp_vertex_struct v[V3DMP_QUAD_NVERTEX];
	mp_vertex_struct n[V3DMP_QUAD_NVERTEX];
	mp_vertex_struct tc[V3DMP_QUAD_NVERTEX];
} mp_quad_struct;

/* Quad strip. */
typedef struct {
	int type;       /* Must be V3DMP_TYPE_QUAD_STRIP. */
	mp_vertex_struct **v;
	mp_vertex_struct **n;
	mp_vertex_struct **tc;
	int total;
} mp_quad_strip_struct;

/* Polygon. */
typedef struct {
	int type;	/* Must be V3DMP_TYPE_POLYGON. */
	mp_vertex_struct **v;
	mp_vertex_struct **n;
	mp_vertex_struct **tc;
	int total;
} mp_polygon_struct;


/* Color. */
typedef struct {
	int type;	/* Must be V3DMP_TYPE_COLOR. */
	double a, r, g, b;
	double ambient, diffuse, specular, shininess, emission;
} mp_color_struct;

/* Texture select by reference name. */
typedef struct {
	int type;       /* Must be V3DMP_TYPE_TEXTURE_SELECT. */
	char *name;		/* Texture reference name. */
	void *client_data;	/* Client data (can be a number too). */
} mp_texture_select_struct;

/* Texture orient about xy plane. */
typedef struct {
	int type;	/* Must be V3DMP_TYPE_TEXTURE_ORIENT_XY. */
	double x, y;
	double dx, dy;	/* Goes -x to +x and -y to +y. */
} mp_texture_orient_xy_struct;

/* Texture orient about yz plane. */
typedef struct {
	int type;       /* Must be V3DMP_TYPE_TEXTURE_ORIENT_YZ. */
	double y, z;
	double dy, dz;	/* Goes +y to -y and -z to +z. */
} mp_texture_orient_yz_struct;

/* Texture orient about xz plane. */
typedef struct {
	int type;       /* Must be V3DMP_TYPE_TEXTURE_ORIENT_XZ. */
	double x, z;
	double dx, dz;  /* Goes -x to +x and -z to +z. */
} mp_texture_orient_xz_struct;

/* Texture off (unselect). */
typedef struct {
	int type;	/* Must be V3DMP_TYPE_TEXTURE_OFF. */
} mp_texture_off_struct;

/* Heightfield load. */
typedef struct {
	int type;	/* Must be V3DMP_TYPE_HEIGHTFIELD_LOAD. */
	char *path;
	void *gl_list;	/* GL list (can be NULL). */
	double *data;	/* Array of heightfield z points (can be NULL). */
	int x_points, y_points, total_points;
	double x_length, y_length, z_length;	/* Span. */
	double x, y, z;	/* Translation. */
	double heading, pitch, bank;	/* Rotation in radians. */
} mp_heightfield_load_struct;


extern void *V3DMPCreate(int type);
extern void *V3DMPDup(const void *p);
extern int V3DMPInsertVertex(
	void *p, int i,
	mp_vertex_struct **v_rtn, mp_vertex_struct **n_rtn,
	mp_vertex_struct **tc_rtn
);
extern void V3DMPDestroy(void *p);

#define V3DMPGetType(p)		(((p) != NULL) ? (*(int *)(p)) : 0)
extern void *V3DMPListGetPtr(void **list, int total, int i);
extern void *V3DMPListInsert(
	void ***list, int *total, int i,
	int type
);
extern void V3DMPListDelete(void ***list, int *total, int i);
extern void V3DMPListDeleteAll(void ***list, int *total);

extern mp_vertex_struct *V3DMPGetNormal(void *p, int i);
extern mp_vertex_struct *V3DMPGetVertex(void *p, int i);
extern mp_vertex_struct *V3DMPGetTexCoord(void *p, int i);
extern int V3DMPGetTotal(void *p);

extern int V3DMPUnitlizeNormal(void *p);
extern int V3DMPFlipWinding(
	void *p, int flip_normals, int flip_texcoords
);


#ifdef __cplusplus
}  
#endif /* __cplusplus */

#endif	/* V3DMP_H */
