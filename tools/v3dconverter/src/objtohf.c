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


#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>


#define FREE_AND_CLOSE_ALL_OBJTOHF \
if ( fpObjIn && (fpObjIn != NULL) ) fclose( fpObjIn ); \
if ( fpHfOut && (fpHfOut != NULL) ) fclose( fpHfOut ); \
if ( fpConversionDataFileIn != NULL ) fclose( fpConversionDataFileIn );


int objToHf(const char *source, const char *dest, UserModifier *userModifier ) {
    FILE *fpObjIn = NULL, *fpHfOut = NULL, *fopen(), *fpConversionDataFileIn = NULL;
    char lineBuffer[MAX_LENGTH + 1], objTag[MAX_LENGTH + 1];
    long lineNr = 1;
    unsigned long verticesNr = 0;
    bool specFileFound = false;
    
    struct TGAImageHeader {
	/*** Fixed length data ***/
	uint8_t idFieldLength; // image ID field length
	uint8_t colorMapType;
	uint8_t imageType;
	/* Color Map Specification */
	uint8_t colorMapIndexL;
	uint8_t colorMapIndexH;
	uint8_t colorMapLengthL;
	uint8_t colorMapLengthH;
	uint8_t colorMapSize;
	/* Image specification */
	uint8_t xOriginL;
	uint8_t xOriginH;
	uint8_t yOriginL;
	uint8_t yOriginH;
	uint8_t widthL;
	uint8_t widthH;
	uint8_t heightL;
	uint8_t heightH;
	uint8_t bitsPerPixel;
	uint8_t imageDescriptor;
    
	/*** Variable length data ***/
	// ...
    };
    
    struct TerrainSpec {
	double sizeX;			// "width", in meters
	double sizeY;			// "height", in meters
	double maxAltitude;		// max altitude, in meters.
	double xPos;			// x translation, in meters
	double yPos;			// y translation, in meters
	double zPos;			// z translation, in meters
	double h;			// heading rotation, in degrees
	double p;			// pitch rotation, in degrees
	double b;			// bank rotation, in degrees
	double scale;			// scale applied by user during *.3d to *.obj conversion
	int bitsWidth;			// image width, in bits
	int bitsHeight;			// image height, in bits
    };
    struct TerrainSpec terrainSpec;
    
    
    /*
     * Input and output files ready ?
     */
    if ( ( fpObjIn = fopen(source, "r" ) ) == NULL ) {
	perror(source);
	
	FREE_AND_CLOSE_ALL_OBJTOHF
	return -1;
    }
    if ( ( fpHfOut = fopen( dest, "w" ) ) == NULL ) {
	perror( dest );
	
	FREE_AND_CLOSE_ALL_OBJTOHF
	return -1;
    }
    
    
    /* 
     * Look for material name in *.obj file in order to retrieve terrain specification file name.
     * This specification file has been automatically generated during *.3d terrain to *.obj conversion
     * and contains some specification data. Then, extract data from this file.
     */
    int valueSet = 0;
    while ( fgets(lineBuffer, MAX_LENGTH, fpObjIn ) )
    {
	if ( lineBuffer[ 0 ] == '\n' || lineBuffer[ 0 ] == '\r' || ( sscanf( lineBuffer, " %s[\n]", objTag ) ) == 0 ) // blank line or objTag == ""
	{
	    lineNr++;
	    continue;
	}
	
	if ( !strcmp( objTag, "usemtl" ) ) // *.obj material name
	{
	    if ( sscanf( lineBuffer, "usemtl %s", objTag ) == 1 )
	    {
		int homeLength = strlen( getenv("HOME") );
		char v3dConverterPath[] = "/.local/share/v3dconverter/";
		unsigned int dirLength = strlen( v3dConverterPath );
		int materialNameLength =  strlen( objTag );
		
		char *name = malloc( homeLength + dirLength + materialNameLength + 1 );
		if ( name == NULL )
		{
			fprintf( stderr, "objtohf.c: can't allocate memory for name.\n" );
			return -1;
		}
		objTag[ 14 ] = '\0'; // limit objTag string length because sometimes Blender adds its own extension to material name.
		sprintf( name, "%s%s%s", getenv("HOME"), v3dConverterPath, objTag );
		
		if ( ( fpConversionDataFileIn = fopen(name, "r" ) ) == NULL )
		{
		    specFileFound = false;
		}
		else
		{
		    specFileFound = true;
		    
		    while ( fgets( lineBuffer, MAX_LENGTH, fpConversionDataFileIn ) )
		    {
			if ( lineBuffer[ 0 ] == '\n' || lineBuffer[ 0 ] == '\r' || ( sscanf( lineBuffer, " %s[\n]", objTag ) ) == 0 ) // blank line or objTag == ""
			    continue;
			
			if ( lineBuffer[ 0 ] == '#' )
				continue;
			
			int cnt = 0;
			while ( lineBuffer[ cnt++ ] != '=' );
			lineBuffer[ cnt - 1 ] = '\0';
			
			if ( !strcmp( lineBuffer, "file" ) )
			{
			    //fprintf( stdout, "file = %s", &lineBuffer[ cnt ] );
			}
			else if ( !strcmp( lineBuffer, "material_name" ) )
			{
			    //fprintf( stdout, "material_name = %s", &lineBuffer[ cnt ] );
			}
			else if ( !strcmp( lineBuffer, "terrain_width" ) )
			{
			    valueSet++;
			    sscanf( &lineBuffer[ cnt ], "%lf", &terrainSpec.sizeX );
			    //fprintf( stdout, "terrain_width = %f\n", terrainSpec.sizeX );
			}
			else if ( !strcmp( lineBuffer, "terrain_height" ) )
			{
			    valueSet++;
			    sscanf( &lineBuffer[ cnt ], "%lf", &terrainSpec.sizeY );
			    //fprintf( stdout, "terrain_height = %f\n", terrainSpec.sizeY );
			}
			else if ( !strcmp( lineBuffer, "terrain_altitude" ) )
			{
			    valueSet++;
			    sscanf( &lineBuffer[ cnt ], "%lf", &terrainSpec.maxAltitude );
			    //fprintf( stdout, "terrain_altitude = %f\n", terrainSpec.maxAltitude );
			}
			else if ( !strcmp( lineBuffer, "terrain_transX" ) )
			{
			    valueSet++;
			    sscanf( &lineBuffer[ cnt ], "%lf", &terrainSpec.xPos );
			    //fprintf( stdout, "terrain_transX = %f\n", terrainSpec.xPos );
			}
			else if ( !strcmp( lineBuffer, "terrain_transY" ) )
			{
			    valueSet++;
			    sscanf( &lineBuffer[ cnt ], "%lf", &terrainSpec.yPos );
			    //fprintf( stdout, "terrain_transY = %f\n", terrainSpec.yPos );
			}
			else if ( !strcmp( lineBuffer, "terrain_transZ" ) )
			{
			    valueSet++;
			    sscanf( &lineBuffer[ cnt ], "%lf", &terrainSpec.zPos );
			    //fprintf( stdout, "terrain_transZ = %f\n", terrainSpec.zPos );
			}
			else if ( !strcmp( lineBuffer, "terrain_scale" ) )
			{
			    valueSet++;
			    sscanf( &lineBuffer[ cnt ], "%lf", &terrainSpec.scale );
			    //fprintf( stdout, "terrain_scale = %f\n", terrainSpec.scale );
			}
			else if ( !strcmp( lineBuffer, "terrain_bitsWidth" ) )
			{
			    valueSet++;
			    sscanf( &lineBuffer[ cnt ], "%d", &terrainSpec.bitsWidth );
			    //fprintf( stdout, "terrain_bitsWidth = %d\n", terrainSpec.bitsWidth );
			}
			else if ( !strcmp( lineBuffer, "terrain_bitsHeight" ) )
			{
			    valueSet++;
			    sscanf( &lineBuffer[ cnt ], "%d", &terrainSpec.bitsHeight );
			    //fprintf( stdout, "terrain_bitsHeight = %d\n", terrainSpec.bitsHeight );
			}
			else
			{
			    fprintf( stdout, "objtohf.c: unexpected data found in file '%s'.\n", name );
			}
		    }
		    free( fpConversionDataFileIn );
		    fpConversionDataFileIn = NULL;
		}
		free( name );
	    }
	    else
	    {
		; // sscanf error
	    }
	}
	lineNr++;
    }
    
    
    /*
     * Check if all terrain specification values are set, and ask them to user if not.
     */
#define TERRAIN_SPEC_VALUES_TO_SET 9

#define READ_AND_CHECK_STDIN \
cnt = 0; \
while( ( cnt < MAX_LENGTH ) && ( c = getchar() ) != '\n' && c != EOF ) \
    lineBuffer[ cnt++ ] = c; \
lineBuffer[ cnt ] = '\0'; \
for ( int cnt1 = cnt - 1; cnt1 >= 0; cnt1-- ) \
{ \
    if ( !( isdigit( lineBuffer[ cnt1 ] ) || ( lineBuffer[ cnt1 ] == '-' ) || ( lineBuffer[ cnt1 ] == '.' ) ) ) \
    { \
	lineBuffer[ 0 ] = '\0'; \
	break; \
    }; \
}
    
    if ( !specFileFound )
    {
	double userAnswer;
	char c, lineBuffer[MAX_LENGTH + 1] = { '\0' };
	int cnt = 0;
	
	fprintf( stderr, "Terrain specification file not found. Please enter data manually:\n" );
	fprintf( stderr, "----------------------------------------------------------------\n" );
	
	while ( true )
	{
	    fprintf( stderr, "Please enter terrain X value (*.hf width) in meters: " );
	    
	    READ_AND_CHECK_STDIN
	    
	    if ( ( sscanf( lineBuffer, "%lf", &userAnswer ) ) == 1 )
	    {
		terrainSpec.sizeX = userAnswer;
		valueSet++;
		break;
	    }
	    else
		fprintf( stderr, "INPUT ERROR\n" );
	}
	
	while ( true )
	{
	    fprintf( stderr, "Please enter terrain Y (*.hf height) value in meters: " );
	    
	    READ_AND_CHECK_STDIN
	    
	    if ( ( sscanf( lineBuffer, "%lf", &userAnswer ) ) == 1 )
	    {
		terrainSpec.sizeY = userAnswer;
		valueSet++;
		break;
	    }
	    else
		fprintf( stderr, "INPUT ERROR\n" );
	}
	
	while ( true )
	{
	    fprintf( stderr, "Please enter terrain Z value (*.hf max altitude) in meters: " );
	    
	    READ_AND_CHECK_STDIN
	    
	    if ( ( sscanf( lineBuffer, "%lf", &userAnswer ) ) == 1 )
	    {
		terrainSpec.maxAltitude = userAnswer;
		valueSet++;
		break;
	    }
	    else
		fprintf( stderr, "INPUT ERROR\n" );
	}
	
	while ( true )
	{
	    fprintf( stderr, "Please enter terrain translation X value in meters: " );
	    
	    READ_AND_CHECK_STDIN
	    
	    if ( ( sscanf( lineBuffer, "%lf", &userAnswer ) ) == 1 )
	    {
		terrainSpec.xPos = userAnswer;
		valueSet++;
		break;
	    }
	    else
		fprintf( stderr, "INPUT ERROR\n" );
	}
	
	while ( true )
	{
	    fprintf( stderr, "Please enter terrain translation Y value in meters: " );
	    
	    READ_AND_CHECK_STDIN
	    
	    if ( ( sscanf( lineBuffer, "%lf", &userAnswer ) ) == 1 )
	    {
		terrainSpec.yPos = userAnswer;
		valueSet++;
		break;
	    }
	    else
		fprintf( stderr, "INPUT ERROR\n" );
	}
	
	while ( true )
	{
	    fprintf( stderr, "Please enter terrain translation Z value in meters: " );
	    
	    READ_AND_CHECK_STDIN
	    
	    if ( ( sscanf( lineBuffer, "%lf", &userAnswer ) ) == 1 )
	    {
		terrainSpec.zPos = userAnswer;
		valueSet++;
		break;
	    }
	    else
		fprintf( stderr, "INPUT ERROR\n" );
	}
	
	while ( true )
	{
	    fprintf( stderr, "Please enter terrain scale value: " );
	    
	    READ_AND_CHECK_STDIN
	    
	    if ( ( sscanf( lineBuffer, "%lf", &userAnswer ) ) == 1 )
	    {
		terrainSpec.scale = userAnswer;
		valueSet++;
		break;
	    }
	    else
		fprintf( stderr, "INPUT ERROR\n" );
	}
	
	while ( true )
	{
	    fprintf( stderr, "Please enter *.hf texture width number of bits: " );
	    
	    READ_AND_CHECK_STDIN
	    
	    if ( ( sscanf( lineBuffer, "%lf", &userAnswer ) ) == 1 )
	    {
		terrainSpec.bitsWidth = userAnswer;
		valueSet++;
		break;
	    }
	    else
		fprintf( stderr, "INPUT ERROR\n" );
	}
	
	while ( true )
	{
	    fprintf( stderr, "Please enter *.hf texture height number of bits: " );
	    
	    READ_AND_CHECK_STDIN
	    
	    if ( ( sscanf( lineBuffer, "%lf", &userAnswer ) ) == 1 )
	    {
		terrainSpec.bitsHeight = userAnswer;
		valueSet++;
		break;
	    }
	    else
		fprintf( stderr, "INPUT ERROR\n" );
	}
	
	if ( valueSet != TERRAIN_SPEC_VALUES_TO_SET )
	    fprintf( stderr, "objtohf.c : something bad has happen. Wrong TERRAIN_SPEC_VALUES_TO_SET ?\n" );
	
	/*
	fprintf( stderr, " terrain width = %lf\n", terrainSpec.sizeX);
	fprintf( stderr, "terrain height = %lf\n", terrainSpec.sizeY);
	fprintf( stderr, "  max altitude = %lf\n", terrainSpec.maxAltitude);
	fprintf( stderr, "terrain transX = %lf\n", terrainSpec.xPos);
	fprintf( stderr, "terrain transY = %lf\n", terrainSpec.yPos);
	fprintf( stderr, "terrain transZ = %lf\n", terrainSpec.zPos);
	fprintf( stderr, " terrain scale = %lf\n", terrainSpec.scale);
	fprintf( stderr, " terrain bitsW = %d\n", terrainSpec.bitsWidth);
	fprintf( stderr, " terrain bitsH = %d\n", terrainSpec.bitsHeight);
	*/
    }
    
    
    /*
     * Prepare tga '.hf' image header
     */
    struct TGAImageHeader hfTexHeader;
    hfTexHeader.idFieldLength = 0;
    hfTexHeader.colorMapType = 0;
    hfTexHeader.imageType = 3; // Uncompressed, "black and white" (grey level) image
    hfTexHeader.colorMapIndexL = 0;
    hfTexHeader.colorMapIndexH = 0;
    hfTexHeader.colorMapLengthL = 0;
    hfTexHeader.colorMapLengthH = 0;
    hfTexHeader.colorMapSize = 0;
    hfTexHeader.xOriginL = 0;
    hfTexHeader.xOriginH = 0;
    hfTexHeader.yOriginL = 0;
    hfTexHeader.yOriginH = 0;
    hfTexHeader.widthL = terrainSpec.bitsWidth << 8;
    hfTexHeader.widthH = terrainSpec.bitsWidth / 256;
    hfTexHeader.heightL = terrainSpec.bitsHeight << 8;
    hfTexHeader.heightH = terrainSpec.bitsHeight / 256;
    hfTexHeader.bitsPerPixel = 8;
    hfTexHeader.imageDescriptor = 32; // Top left origin
    
    
    /*
     * Reserve memory for tga '.hf' image data (i.e. color bytes).
     */
    uint32_t imageDataBytes = ( 256 * hfTexHeader.widthH + hfTexHeader.widthL ) * ( 256 * hfTexHeader.heightH + hfTexHeader.heightL ) * ( hfTexHeader.bitsPerPixel / 8 );
    uint8_t *tgaImageData = malloc( imageDataBytes );
    if ( tgaImageData == NULL )
    {
	fprintf( stderr, "Can't allocate memory for *.hf TGA image data.\n" );
	
	FREE_AND_CLOSE_ALL_OBJTOHF
        return -1;
    }
    
    
    /*
     * Read and process each *.obj file line in order to extract vertices,
     * then convert vertices coordinates to tga ".hf" image column bit number, row bit number, and grey value (i.e. altitude, from 0 to 255).
     */
    rewind( fpObjIn );
    lineNr = 1;
    double perPixelXDistance = terrainSpec.sizeX / (double)terrainSpec.bitsWidth;
    double perPixelYDistance = terrainSpec.sizeY / (double)terrainSpec.bitsHeight;
    double perPixelZDistance = terrainSpec.maxAltitude / 255; // altitude is always an 8 bits value
    double halfXSize = terrainSpec.sizeX / 2 - perPixelXDistance / 2;
    double halfYSize = terrainSpec.sizeY / 2 - perPixelYDistance / 2;
    double x = 0;
    double y = 0;
    double z = 0;
    while ( fgets(lineBuffer, MAX_LENGTH, fpObjIn) ) {
	if ( lineBuffer[ 0 ] == '\n' || lineBuffer[ 0 ] == '\r' || ( sscanf( lineBuffer, " %s[\n]", objTag ) ) == 0 ) // blank line or objTag == ""
	{
	    lineNr++;
	    continue;
	}
	
	if ( !strcmp( objTag, "v" ) ) // vertex
	{
	    verticesNr++;
	    
	    sscanf( lineBuffer, "%*s %lf %lf %lf", &x, &z, &y );
	    y = -y;
	    
	    /* Revert *.3d to *.obj scale (set by user) */
	    x /= terrainSpec.scale ;
	    y /= terrainSpec.scale ;
	    z /= terrainSpec.scale ;
	    
	    /* Revert translations. These translations includes *.3d terrain translations and translations requested by user. */
	    x -= terrainSpec.xPos;
	    y -= terrainSpec.yPos;
	    z -= terrainSpec.zPos;
	    
	    double col = ( x + halfXSize ) / perPixelXDistance;
	    double row = (double)(terrainSpec.bitsWidth) - 1 - ( y + halfYSize ) / perPixelYDistance;
	    double altitude = z / perPixelZDistance;
	    
	    col = round(col);
	    row = round(row);
	    altitude = round(altitude);
	    
	    if ( altitude > 255 )
		altitude = 255;
	    else if ( altitude < 0 )
		altitude = 0;
	    
	    if ( col < 0 || col > terrainSpec.bitsWidth - 1 )
	    {
		fprintf( stderr, "Error : Bad column number detected in *.hf image (col = %lf, row = %lf). Exiting.\n", col, row );
		
		FREE_AND_CLOSE_ALL_OBJTOHF
		return -1;
	    }
	    
	    if ( row < 0 || row > terrainSpec.bitsHeight - 1 )
	    {
		fprintf( stderr, "Error : Bad row number detected in *.hf image (col = %lf, row = %lf). Exiting.\n", col, row );
		
		FREE_AND_CLOSE_ALL_OBJTOHF
		return -1;
	    }
	    
	    unsigned long offset = (unsigned long)col + (unsigned long)row * terrainSpec.bitsWidth;
	    if ( ( offset >= 0 ) && ( offset < imageDataBytes ) )
	    {
		tgaImageData[ offset ] = (uint8_t)altitude;
	    }
	    else
	    {
		fprintf( stderr, "objtohf.c : tgaImageData offset error. Bad terrain specification data ?\n" );
	    }
	}
	else
	    ;

	lineNr++;
    }
    
    
    /*
     * Write tga ".hf" image header
     */
    fwrite( &hfTexHeader, sizeof( hfTexHeader ), 1, fpHfOut );
    
    /*
     * No color map to write
     */
    
    /*
     * Write image data
     */
    for ( unsigned long cnt0 = 0; cnt0 < imageDataBytes / terrainSpec.bitsWidth; cnt0++ )
    {
	fwrite( &tgaImageData[ cnt0 * terrainSpec.bitsWidth ], terrainSpec.bitsWidth, 1, fpHfOut );
    }
    
    FREE_AND_CLOSE_ALL_OBJTOHF
    
    return EXIT_SUCCESS;
}
