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

//
// Compile with:
// gcc -lm -Wall -o v3dconverter v3dconverter.c
//


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <unistd.h> // for getcwd

#include "v3dconverter.h"
#include "utils.c"
#include "rotate.c"
#include "v3dtoobj.c"
#include "ac3dtov3d.c"
#include "objtov3d.c"
#include "v3dhftoobj.c"
#include "objtohf.c"

int main( int argc, char *argv[] )
{
    char * source;
    char * destination;
    char *scaleString;
    char lineBuffer[MAX_LENGTH + 1], objTag[MAX_LENGTH + 1];
    FILE *fp;
    
    struct UserModifier userModifier;
    userModifier.rx = 0;
    userModifier.ry = 0;
    userModifier.rz = 0;
    userModifier.tx = 0;
    userModifier.ty = 0;
    userModifier.tz = 0;
    userModifier.scale = 1.0;
    userModifier.invertFaces = false;
    
    if ( argc == 1 )
    {
	fprintf( stdout, "Try \"%s -h\" for help.\n", argv[0] );
        return 0;
    }
    else if ( ( argc == 2 && !strcmp( argv[1], "-h" ) ) || !strcmp( argv[1], "--help" ) )
    {
	fprintf( stdout, "v3dconverter %s\n", VERSION );
        fprintf( stdout, "Convert a Search And Rescue V3D *.3d model or terrain file to/from Wavefront *.obj or AC3D *.ac format.\n\n" );
        fprintf( stdout, "Usage : %s -i input_file [options] -o output_file | [-h] | [--help]\n", argv[0] );
        fprintf( stdout, "   OPTIONS:\n");
        fprintf( stdout, "     -if\n");
        fprintf( stdout, "        Invert faces visibility (flip winding).\n");
	fprintf( stdout, "     -ro x y z\n");
        fprintf( stdout, "        Rotate object of x, y, and z degrees around respectives axes.\n" );
	fprintf( stdout, "     -sc scale\n");
        fprintf( stdout, "        Scale object. Scale value can be specified as decimal (0.1) or fractional (1/10).\n" );
	fprintf( stdout, "     -tr x y z\n");
        fprintf( stdout, "        Translate object of x, y, and z meters in respectives axes directions.\n" );
	fprintf( stdout, "     -v | --version\n");
        fprintf( stdout, "        Prints v3dconverter version.\n" );
        fprintf( stdout, "\n");
        fprintf( stdout, "   Available conversions:\n");
        fprintf( stdout, "                +--------------------------+\n" );
        fprintf( stdout, "                |        Destination       |\n" );
        fprintf( stdout, "         +------+------+------+------+-----+\n" );
        fprintf( stdout, "    +--. |format| .3d  | .hf  | .obj | .ac |\n" );
        fprintf( stdout, "    | S `+------+------+------+------+-----+\n" );
        fprintf( stdout, "    | o  | .3d  |   .  |   .  |works1|todo?|\n" );
        fprintf( stdout, "    | u  +------+------+------+------+-----+\n" );
        fprintf( stdout, "    | r  | .obj | DONE | DONE |  .   |  .  |\n" );
        fprintf( stdout, "    | c  +------+------+------+------+-----+\n" );
	fprintf( stdout, "    | e  | .ac  |works2| todo?|  .   |  .  |\n" );
	fprintf( stdout, "    +----+------+------+------+------+-----+\n" );
	fprintf( stdout, "     DONE   : *.obj to *.3d conversion functional for *.obj objects with polygonal (non-\"freeform\") faces. *.obj to *.hf conversion functional for *.obj \"terrain\" file converted from a *.3d terrain file.\n" );
	fprintf( stdout, "     works1 : Works fine for *.3d terrain files. For object files, works with a lot of (all of ?) \"original\" Sar2 objects, but code is a proof of concept (not finished, to rewrite, some color issues when opening *.obj files in Blender).\n" );
	fprintf( stdout, "     works2 : Proof of concept: code to rewrite, not finished (no -tr option, ...)\n" );
	fprintf( stdout, "     todo?  : Maybe later...\n" );
	fprintf( stdout, "        .   : Not planned.\n" );
	fprintf( stdout, "     For other formats, try assimp: https://www.assimp.org/\n" );
        fprintf( stdout, "     \n" );
	
        return 0;
    }
    
    /* Read command line arguments */
    for ( unsigned short counter = 1; counter < argc; counter++ )
    {
        if ( !strcmp(argv[counter], "-i") ) // input model file name
	{
            if ( counter < argc )
		source = argv[++counter];
	    if ( source == NULL || source[0] == '-' || source[0] == '\0' )
	    {
		fprintf( stderr, "Error while reading source filename.\n" );
		return 0;
	    }
        }
        else if ( !strcmp(argv[counter], "-o") ) // outpout model file name
	{
	    if ( counter < argc )
		destination = argv[++counter];
	    if ( destination == NULL || destination[0] == '-' || destination[0] == '\0' )
	    {
		fprintf( stderr, "Error while reading destination filename.\n" );
		return 0;
	    }
        }
        else if ( !strcmp(argv[counter], "-if") ) // invert model faces
	{
            userModifier.invertFaces = true;
        }
        else if ( !strcmp(argv[counter], "-sc") ) // scale model
	{
            if ( counter < argc )
		scaleString = argv[++counter];
	    if ( scaleString == NULL )
	    {
		fprintf( stderr, "Error while reading scale.\n" );
		return 0;
	    }
	    
            if ( memchr( scaleString, '/', strlen(scaleString) ) != NULL )
	    {
                long dividend = 1, divider = 1;
                sscanf( scaleString, "%ld/%ld", &dividend, &divider );
                userModifier.scale = (float) dividend / (float) divider;
            }
            else
                sscanf( scaleString, "%lf", &userModifier.scale );
	    
            if ( userModifier.scale < 0 )
	    {
		fprintf( stdout, "\033[7mNegative scale will produce strange results. You've been warned!\033[0m\n" );
	    }
	    else if ( userModifier.scale < 0.001 || userModifier.scale > 1000.0 )
	    {
		fprintf( stdout, "\033[1m%s\033[0m??? What a scale!!! Okay, you're the boss...\n", scaleString );
	    }
        }
        else if ( !strcmp(argv[counter], "-tr") ) // translate model
	{
            if ( counter >= argc - 1 || !sscanf( argv[++counter], "%lf", &userModifier.tx ) )
	    {
		fprintf( stderr, "Error while reading -tr X value.\n" );
		return 0;
	    }
	    if ( counter >= argc - 1 || !sscanf( argv[++counter], "%lf", &userModifier.ty ) )
	    {
		fprintf( stderr, "Error while reading -tr Y value.\n" );
		return 0;
	    }
	    if ( counter >= argc - 1 || !sscanf( argv[++counter], "%lf", &userModifier.tz ) )
	    {
		fprintf( stderr, "Error while reading -tr Z value.\n" );
		return 0;
	    }
        }
        else if ( !strcmp(argv[counter], "-ro") ) // rotate model
	{
	    if ( counter >= argc - 1 || !sscanf( argv[++counter], "%lf", &userModifier.rx ) )
	    {
		fprintf( stderr, "Error while reading -ro X value.\n" );
		return 0;
	    }
	    if ( counter >= argc - 1 || !sscanf( argv[++counter], "%lf", &userModifier.ry ) )
	    {
		fprintf( stderr, "Error while reading -ro Y value.\n" );
		return 0;
	    }
	    if ( counter >= argc - 1 || !sscanf( argv[++counter], "%lf", &userModifier.rz ) )
	    {
		fprintf( stderr, "Error while reading -ro Z value.\n" );
		return 0;
	    }
        }
        else if ( !strcmp(argv[counter], "-v" ) || !strcmp( argv[counter], "--version" ) ) // print version
	{
	    fprintf( stdout, "%s\n", VERSION );
	    return 0;
        }
        else if ( argv[counter][0] == '-' ) // unknown option
	{
	    fprintf( stderr, "\"%s\": unknown option.\n", argv[counter] );
	    return 0;
        }
        else
	{
	    fprintf( stderr, "\"%s\": unknown statement.\n", argv[counter] );
	    return 0;
	}
    }
    
    /* Convert */
    if ( !strcmp( extOf( source ), ".3d") && !strcmp( extOf( destination ), ".obj" ) )
    {
	/* Check if *.3d file is a terrain file, i.e. if it has "heightfield_load" and "texture_load" statements */
	bool hasHeightfield = false, hasTexture = false;
	if ( ( fp = fopen(source, "r" ) ) == NULL ) {
	    perror(source);
	    return -1;
	}
	while ( fgets(lineBuffer, MAX_LENGTH, fp) )
	{
	    if ( lineBuffer[ 0 ] == '\n' || lineBuffer[ 0 ] == '\r' || ( sscanf( lineBuffer, " %s[\n]", objTag ) ) == 0 ) // blank line or objTag == ""
	    {
		lineNr++;
		continue;
	    }
	    else if ( !strcmp( objTag, "heightfield_load" ) )
	    {
		hasHeightfield = true;
		if ( hasTexture )
		    break;
	    }
	    else if ( !strcmp( objTag, "texture_load" ) )
	    {
		hasTexture = true;
		if ( hasHeightfield )
		    break;
	    }
	    else
		;
	}
	fclose( fp );
	
	/* Select the right function, depending of *.3d file type ("object" or "terrain") */
	if ( !hasHeightfield )
	{ // it is *.3d object file
	    fprintf( stdout, "[ INFO ] *** .3d object to .obj conversion is EXPERIMENTAL. Please don't report bugs and use it at your own risk ;-) ***\n" );
	    if ( userModifier.scale != 1.0 )
	    {
		fprintf( stdout, "[ INFO ] -sc (scale) option currently not available in *.3d to *.obj conversion.\n" );
		userModifier.scale = 1.0;
	    }
	    v3dToObj( source, destination, &userModifier );
	}
	else
	{ // it is *.3d terrain file
	    if ( userModifier.rx != 0 || userModifier.ry != 0 || userModifier.rz != 0 )
	    {
		fprintf( stdout, "[ INFO ] -ro (rotate) option not available in *.3d terrain to *.obj conversion.\n" );
		userModifier.rx = 0;
		userModifier.ry = 0;
		userModifier.rz = 0;
	    }
	    if ( userModifier.invertFaces == true )
	    {
		fprintf( stdout, "[ INFO ] -if (invert faces visibility) option not available in *.3d terrain to *.obj conversion.\n" );
		userModifier.invertFaces = false;
	    }
	    v3dHfToObj( source, destination, &userModifier );
	}
    }
    else if ( !strcmp(extOf( source ), ".obj" ) && !strcmp( extOf( destination ), ".3d" )  )
    {
        objToV3d( source, destination, &userModifier );
    }
    else if ( !strcmp(extOf( source ), ".obj" ) && ( !strcmp( extOf( destination ), ".hf" ) || isHf( destination ) ) )
    {
		if ( userModifier.tx != 0 || userModifier.ty != 0 || userModifier.tz != 0 || userModifier.rx != 0 || userModifier.ry != 0 || userModifier.rz != 0 || userModifier.invertFaces == 1 )
	    {
		fprintf( stdout, "[ INFO ] -tr (translate), -ro (rotate), and -if (invert faces) options not available in *.obj terrain to *.hf conversion:\n          translations have been saved during *.3d to *.obj conversion and will be automatically set.\n" );
		userModifier.tx = 0;
		userModifier.ty = 0;
		userModifier.tz = 0;
		userModifier.rx = 0;
		userModifier.ry = 0;
		userModifier.rz = 0;
		userModifier.invertFaces = false;
		}
		if ( userModifier.scale != 1.0 )
	    {
		fprintf( stdout, "[ INFO ] -sc (scale) option not available in *.obj terrain to *.hf conversion:\n          scale has been saved during *.3d to *.obj conversion and will be automatically set.\n" );
		userModifier.scale = 1.0;
	    }
        objToHf( source, destination, &userModifier );
    }
    else if ( !strcmp(extOf( source ), ".ac" ) && !strcmp( extOf( destination ), ".3d" )  )
    {
	fprintf( stdout, "[ INFO ] *** .ac to .3d conversion is EXPERIMENTAL. Please don't report bugs and use it at your own risk ;-) ***\n" );
	if ( userModifier.tx != 0 || userModifier.ty != 0 || userModifier.tz != 0 )
	{
	    fprintf( stdout, "[ INFO ] -tr (translate) option currently not available in *.ac to *.3d conversion.\n" );
		userModifier.tx = 0;
		userModifier.ty = 0;
		userModifier.tz = 0;
	}
        ac3dToV3d( source, destination, &userModifier );
    }
    else
    {
        fprintf( stderr, "%s can't convert \"%s\" format to \"%s\" format. ", argv[0], extOf( source ), extOf( destination ) );
	fprintf( stderr, "Try to convert your \"%s\" file to \".obj\" or \".ac\" with 'assimp': https://www.assimp.org/\n", extOf( source ) );
    }

    return 0;
}
