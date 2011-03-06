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
			       V3D Models

	When adding new members to the structure, make sure you update
	functions ModelCreate(), ModelDup(), and ModelDestroy() to
	ensure all members are handled properly.

 */

#ifndef V3DMODEL_H
#define V3DMODEL_H

#include <sys/types.h>
#include "v3dmp.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/*
 *	Model type codes:
 */
#define V3D_MODEL_TYPE_STANDARD		1
#define V3D_MODEL_TYPE_OTHER_DATA	2	/* Third party data block. */

/*
 *	Model flags:
 */
#define V3D_MODEL_FLAG_HIDE		(1 << 1)


/*
 *	Model structure:
 */
typedef struct {

	int type;		/* One of V3D_MODEL_TYPE_*. */
	unsigned int flags;	/* Any of V3D_MODEL_FLAG_*. */
	char *name;		/* To be displayed on the list widget. */

	/* Only if type is V3D_MODEL_TYPE_STANDARD. */
	void **primitive;
	int total_primitives;

	/* Only if type is V3D_MODEL_TYPE_OTHERDATA. */
	char **other_data_line;
	int total_other_data_lines;

} v3d_model_struct;


extern v3d_model_struct *V3DModelCreate(int type, const char *name);
extern v3d_model_struct *V3DModelDup(const v3d_model_struct *m);
extern void V3DModelDestroy(v3d_model_struct *m);

extern v3d_model_struct *V3DModelListGetPtr(v3d_model_struct **list, int total, int i);
extern v3d_model_struct *V3DModelListInsert(
	v3d_model_struct ***list, int *total, int i,
	int type, const char *name
);
extern void V3DModelListDelete(v3d_model_struct ***list, int *total, int i);
extern void V3DModelListDeleteAll(v3d_model_struct ***list, int *total);


extern int V3DModelGetType(v3d_model_struct *m);


#ifdef __cplusplus
}  
#endif /* __cplusplus */

#endif	/* V3DMODEL_H */
