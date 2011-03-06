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
