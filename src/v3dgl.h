/*
			     V3D OpenGL Front End

	Handles V3D to OpenGL IOs/Conversions/Calling/etc

	Depends on v3dhf.c and v3dtex.c

 */

#ifndef V3DGL_H
#define V3DGL_H

#include "v3dhf.h"
#include "v3dmh.h"
#include "v3dmodel.h"
#include "v3dmp.h"
#include "v3dtex.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#ifndef FALSE
# define FALSE	0
#endif

#ifndef TRUE
# define TRUE	1
#endif


/*
 *	gl interpritation structure, contains a legend on how to convert
 *	V3D data to gl data.
 */
typedef struct {

#define V3D_GLFLAG_COORDINATE_AXIS	(1 << 0)
#define V3D_GLFLAG_TEXTURE_KEEP		(1 << 1)
#define V3D_GLFLAG_ALLOW_TRANSLATIONS	(1 << 2)
#define V3D_GLFLAG_ALLOW_ROTATIONS	(1 << 3)
#define V3D_GLFLAG_FLIP_WINDING		(1 << 4)
#define V3D_GLFLAG_PASS_NORMALS		(1 << 5)
#define V3D_GLFLAG_UNITLIZE_NORMALS	(1 << 6)
#define V3D_GLFLAG_PASS_TEXCOORDS	(1 << 7)
#define V3D_GLFLAG_TEXTURE_NAME_CASE_SENSITIVE	(1 << 8)
#define V3D_GLFLAG_MATERIAL_PROPERTIES	(1 << 9)
#define V3D_GLFLAG_FACES		(1 << 10)
#define V3D_GLFLAG_ENABLE_BLENDING	(1 << 11)
#define V3D_GLFLAG_SET_BLEND_FUNC	(1 << 12)
#define V3D_GLFLAG_HEIGHTFIELD_BASE_DIR	(1 << 13)
#define V3D_GLFLAG_TEXTURE_BASE_DIR	(1 << 14)

	/* Determines which members are set in this structure, any
	 * of V3D_GLFLAG_*.
	 */
	unsigned int flags;

#define V3D_GLCOORDINATE_AXIS_SCIENTIFIC	0	/* x, y, z. */
#define V3D_GLCOORDINATE_AXIS_GL		1	/* x, z, -y. */
	int coordinate_axis;

#define V3D_GLTEXTURE_KEEP_ALWAYS		0	/* Keep all textures. */
#define V3D_GLTEXTURE_KEEP_ASNEEDED		1	/* Keep if specified and used. */
	int texture_keep;

	int allow_translations;		/* Ignore translate primitives if FALSE. */

	int allow_rotations;		/* Ignore rotate primitives if FALSE. */

	int flip_winding;		/* Flip winding if TRUE. */

#define V3D_GLPASS_NORMALS_AS_NEEDED	0
#define V3D_GLPASS_NORMALS_ALWAYS	1
#define V3D_GLPASS_NORMALS_NEVER	2
	int pass_normals;

	int unitlize_normals;		/* Convert normals to unit length before
					 * passing to GL.
					 */

#define V3D_GLPASS_TEXCOORDS_AS_NEEDED	0	/* Only when a texture is bounded. */
#define V3D_GLPASS_TEXCOORDS_ALWAYS	1
#define V3D_GLPASS_TEXCOORDS_NEVER	2
	int pass_texcoords;

	int texture_name_case_sensitive;	/* TRUE for case sensitive texture
						 * name matching.
						 */

#define V3D_GLPASS_MATERIAL_PROPERTIES_NEVER		0
#define V3D_GLPASS_MATERIAL_PROPERTIES_INSTEAD_COLOR	1
#define V3D_GLPASS_MATERIAL_PROPERTIES_WITH_COLOR	2
	int material_properties;		/* Material properties from color
						 * primitives will be set if this
						 * is TRUE.
						 */

#define V3D_GLFACES_FRONT		0	/* Account for only the front face. */
#define V3D_GLFACES_BACK		1	/* Account for only the back face. */
#define V3D_GLFACES_FRONT_AND_BACK	2	/* Account for both sides. */
	int faces;

#define V3D_GLENABLE_BLENDING_NEVER		0
#define V3D_GLENABLE_BLENDING_AS_NEEDED		1	/* Enables/disables GL_BLEND as needed. */
	int enable_blending;

	int set_blend_func; /* need to work on this. */

	char *heightfield_base_dir;	/* Directory prefix for relative heightfields. */

	char *texture_base_dir;		/* Directory prefix for relative textures. */

	/* More members may be added in the future. */

} v3d_glinterprite_struct;


/*
 *	gl resources structure, this structure keeps track of all the
 *	loaded gl resources.  Such as the loaded textures and gl
 *	interpritation legend.
 *
 *	This resource needs to be kept around by the calling function
 *	and passed to gl loading routines untill all gl operations for
 *	the V3D models data that it came from are no longer in use
 *	and no longer needed.
 */
typedef struct {

	/* V3D to GL interpritation specifications. */
	v3d_glinterprite_struct *glinterprite;

	/* List of loaded textures. */
	v3d_texture_ref_struct **texture;
	int total_textures;

} v3d_glresource_struct;


extern v3d_glinterprite_struct *V3DGLInterpriteNew(void);
extern void V3DGLInterpriteDelete(v3d_glinterprite_struct *glinterp);

extern v3d_glresource_struct *V3DGLResourceNew(void);
extern v3d_glresource_struct *V3DGLResourceNewFromModelData(
	const v3d_glinterprite_struct *glinterp,
	void **mh_item, int total_mh_items,
	v3d_model_struct **model, int total_models
);
extern int V3DGLResourceSetInterpritation(
	v3d_glresource_struct *glres,
	const v3d_glinterprite_struct *glinterp
);
extern v3d_glinterprite_struct *V3DGLResourceGetInterpritation(
	v3d_glresource_struct *glres
);

extern void V3DGLResourceDelete(v3d_glresource_struct *glres);


/* V3D model to GL interpritation processing. */
extern void V3DGLProcessModelExtra(
	v3d_glresource_struct *glres,
	v3d_model_struct *m,
	void *client_data,
	void (*extra_cb)(v3d_model_struct *, const char *, void *)
);
extern void V3DGLProcessModel(
	v3d_glresource_struct *glres,
	v3d_model_struct *m
);


#ifdef __cplusplus
}  
#endif /* __cplusplus */

#endif	/* V3DGL_H */
