/*
                   Texture Reference List Management
 */

#ifndef TEXTURELISTIO_H
#define TEXTURELISTIO_H


/*
 *	Texture reference structure, this contains a reference
 *	to a texture on file and its related values.
 */
typedef struct {

	char *name;		/* Reference name. */
	char *filename;
	double priority;	/* Value from 0.0, to 1.0, where 1.0 is highest. */

} sar_texture_name_struct;

#define SAR_TEXTURE_NAME(p)	((sar_texture_name_struct *)(p))


extern void SARTextureListDeleteAll(
	sar_texture_name_struct ***tl,
	int *total
);
extern int SARTextureListLoadFromFile(
	const char *filename,
	sar_texture_name_struct ***tl,
	int *total
);


#endif	/* TEXTURELISTIO_H */
