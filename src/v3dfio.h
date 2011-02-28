/*
			   V3D Format File IO
 */

#ifndef V3DFIO_H
#define V3DFIO_H

#include <stdio.h>
#include <sys/types.h>

#include "v3dmp.h"
#include "v3dmodel.h"
#include "v3dtex.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#ifndef V3D_COMMENT_CHAR
# define V3D_COMMENT_CHAR	'#'
#endif

#ifndef V3D_SAVE_FILE_OPTIMIZATION_MAX
# define V3D_SAVE_FILE_OPTIMIZATION_MAX		2
#endif

extern int V3DLoadModel(
	const char **buf, FILE *fp,
	void ***mh_item, int *total_mh_items,
	v3d_model_struct ***model, int *total_models,
	void *client_data,
	int (*progress_cb)(void *, int, int)
);
extern int V3DSaveModel(
	char ***buf, FILE *fp,
	void **mh_item, int total_mh_items,
	v3d_model_struct **model, int total_models,
	int optimization_level,		/* 0 to V3D_SAVE_FILE_OPTIMIZATION_MAX. */
	int strip_extras,               /* 1 for true, strips extranous data. */
	void *client_data,
	int (*progress_cb)(void *, int, int)
);


#ifdef __cplusplus
}  
#endif /* __cplusplus */

#endif	/* V3DFIO_H */
