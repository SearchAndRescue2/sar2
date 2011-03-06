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
			  V3D Model Header Items

	Data types for basic 3D model header items.

	When adding new types, make sure you update V3DMHCreate() to
	allocate the correct size of the data type structure and
	update MV3DHDestroy() to deallocate and members that may point to
	allocated resources. Externally, you may need to update 
	other functions.

 */

#ifndef V3DMH_H
#define V3DMH_H

#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/*
 *	Model header item types:
 */
#define V3DMH_TYPE_COMMENT			1

#define V3DMH_TYPE_VERSION			10
#define V3DMH_TYPE_CREATOR			11
#define V3DMH_TYPE_AUTHOR			12

#define V3DMH_TYPE_HEIGHTFIELD_BASE_DIRECTORY	20
#define V3DMH_TYPE_TEXTURE_BASE_DIRECTORY	21
#define V3DMH_TYPE_TEXTURE_LOAD			22

#define V3DMH_TYPE_COLOR_SPECIFICATION		30


/*
 *	Model header item structures:
 */

/* Comment. */
typedef struct {
	int type;       /* Must be V3DMH_TYPE_COMMENT. */
	char **line;
	int total_lines;
} mh_comment_struct;

/* Version. */
typedef struct {
	int type;	/* Must be V3DMH_TYPE_VERSION. */
	int major, minor;
} mh_version_struct;
/* Creator. */
typedef struct {
	int type;	/* Must be V3DMH_TYPE_CREATOR. */
	char *creator;
} mh_creator_struct;
/* Author. */
typedef struct {
	int type;	/* Must be V3DMH_TYPE_AUTHOR. */
	char *author;
} mh_author_struct;


/* Heightfield base directory. */
typedef struct {
	int type;	/* Must be V3DMH_TYPE_HEIGHTFIELD_BASE_DIRECTORY. */
	char *path;
} mh_heightfield_base_directory_struct;

/* Texture base directory. */
typedef struct {
	int type;       /* Must be V3DMH_TYPE_TEXTURE_BASE_DIRECTORY. */
	char *path;
} mh_texture_base_directory_struct;

/* Texture load. */
typedef struct {
	int type;       /* Must be V3DMH_TYPE_TEXTURE_LOAD. */
	char *name, *path;
	double priority;		/* 0.0 to 1.0. */
} mh_texture_load_struct;

/* Color specification. */
typedef struct {
	int type;       /* Must be V3DMH_TYPE_COLOR_SPECIFICATION. */
	char *name;
	double a, r, g, b;
	double ambient, diffuse, specular, shininess, emission;
} mh_color_specification_struct;



extern void *V3DMHCreate(int type);
extern void V3DMHDestroy(void *p);  

#define V3DMHGetType(p)		(*(int *)p)
extern void *V3DMHListGetPtr(void **list, int total, int i);
extern void *V3DMHListInsert(
	void ***list, int *total, int i,
	int type
);
extern void V3DMHListDelete(void ***list, int *total, int i);
extern void V3DMHListDeleteAll(void ***list, int *total);

extern int V3DMHTextureBaseDirectorySet(
	void ***list, int *total, const char *path
);
extern char *V3DMHTextureBaseDirectoryGet(void **list, int total);
 
extern int V3DMHHeightfieldBaseDirectorySet(
	void ***list, int *total, const char *path
); 
extern char *V3DMHHeightfieldBaseDirectoryGet(void **list, int total);


#ifdef __cplusplus
}  
#endif /* __cplusplus */

#endif	/* V3DMH_H */
