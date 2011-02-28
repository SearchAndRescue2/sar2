/*
			       X3D Format Library
 */

#ifndef X3D_H
#define X3D_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <GL/gl.h>


/*
 *	Mazimum length of a parameter name:
 */
#define X3D_PARM_MAX	80


/*
 *	X3D format interpretation values:
 */
typedef struct {

#define X3D_VALUE_FLAG_COORDINATE_SYSTEM	(1 << 0)
#define X3D_VALUE_FLAG_SCALE			(1 << 1)
#define X3D_VALUE_FLAG_OFFSET			(1 << 2)
#define X3D_VALUE_FLAG_SKIP_COLORS		(1 << 3)
#define X3D_VALUE_FLAG_SKIP_TEXTURES		(1 << 4)
#define X3D_VALUE_FLAG_SKIP_NORMALS		(1 << 5)
#define X3D_VALUE_FLAG_SKIP_TEXCOORDS		(1 << 6)
#define X3D_VALUE_FLAG_SKIP_VERTICES		(1 << 7)
#define X3D_VALUE_FLAG_SKIP_TRANSLATES		(1 << 8)
#define X3D_VALUE_FLAG_SKIP_ROTATES		(1 << 9)
#define X3D_VALUE_FLAG_SKIP_STATE_CHANGES	(1 << 10)
	unsigned long	flags;

#define X3D_COORDINATE_SYSTEM_XYZ	0	/* x, y, z */
#define X3D_COORDINATE_SYSTEM_GL	1	/* x, z, -y */
	int	coordinate_system;

	GLfloat		scale_x,		/* Coefficient. */
			scale_y,
			scale_z;

	GLfloat		offset_x,		/* Meters, applied after scale. */
			offset_y,
			offset_z;

} X3DInterpValues;
#define X3D_INTERP_VALUES(p)	((X3DInterpValues *)(p))
#define X3D_DEFAULT_VALUE_FLAGS	X3D_VALUE_FLAG_COORDINATE_SYSTEM


/*
 *	X3D texture reference:
 */
typedef struct {

	char	*name;		/* Name of this texture. */
	GLuint	id;		/* GL texture id. */

} X3DTextureRef;
#define X3D_TEXTURE_REF(p)	((X3DTextureRef *)(p))


extern GLuint X3DOpenDataGLList(
	char **data,
	const X3DInterpValues *v,
	X3DTextureRef **texture, int total_textures
);
extern void X3DOpenDataGLOutput(
	char **data,
	const X3DInterpValues *v,
	X3DTextureRef **texture, int total_textures
);

extern X3DTextureRef *X3DTextureRefAppend(
	X3DTextureRef ***texture, int *total_textures,
	const char *name,
	GLuint id
);
extern void X3DTextureDeleteAll(
	X3DTextureRef ***texture, int *total_textures
);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif	/* X3D_H */
