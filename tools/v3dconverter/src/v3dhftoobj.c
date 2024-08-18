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

#include <stdint.h>

#define MAX_LENGTH 2048

#define FREE_AND_CLOSE_ALL_V3DHFTOOBJ \
if ( header != NULL ) free( header ); \
if ( path != NULL ) free( path ); \
if ( baseName != NULL ) free( baseName ); \
if ( extension != NULL ) free( extension ); \
if ( fp3dFileIn != NULL ) fclose( fp3dFileIn ); \
if ( fpHfIn != NULL ) fclose( fpHfIn ); \
if ( fpTexIn != NULL ) fclose( fpTexIn ); \
if ( fpObjOut && (fpObjOut != NULL) ) fclose( fpObjOut ); \
if ( fpMtlOut != NULL ) fclose( fpMtlOut ); \
if ( timeStampString != NULL ) free( timeStampString ); \
if ( fpTerrainDataOut != NULL ) fclose( fpTerrainDataOut );


int v3dHfToObj( const char *source, const char *dest, UserModifier *userModifier ) {
    FILE *fp3dFileIn = NULL, *fpHfIn = NULL, *fpTexIn = NULL, *fpObjOut = NULL, *fpMtlOut = NULL, *fpTerrainDataOut = NULL, *fopen();
    char buffer[MAXPATHLENGTH + MAX_LENGTH + 7], lineBuffer[MAX_LENGTH + 1], objTag[MAX_LENGTH + 1];
    char *path = NULL, *baseName = NULL, *extension = NULL; // for output files names
    long lineNr = 1;
    unsigned int triangles = 0;
    bool hasHeightfield = false, hasTexture = false;

    struct Terrain {
	char path[ MAX_LENGTH + 1 ];	// texture path
	double sizeX;			// "width", in meters
	double sizeY;			// "height", in meters
	double maxAltitude;		// max altitude, in meters.
	double xPos;			// x translation, in meters
	double yPos;			// y translation, in meters
	double zPos;			// z translation, in meters
	double h;			// heading rotation, in degrees
	double p;			// pitch rotation, in degrees
	double b;			// bank rotation, in degrees
    };

    struct TexLoad {
	char name[MAX_LENGTH + 1]; // texture name
	char path[MAX_LENGTH + 1]; // texture path
	double priority;
    };

    struct TgaMinimalHeader {
	uint8_t idFieldLength;		// Byte 0: image ID field length
	uint8_t colorMapType;
	uint8_t imageType;

	/* Color Map Specification */
	uint16_t colorMapIndex;		// Byte 3 (LSB), byte 4 (MSB)
	uint16_t colorMapLength;
	uint8_t colorMapSize;

	/* Image specification */
	uint16_t xOrigin;		// Byte 8 (LSB), byte 9 (MSB)
	uint16_t yOrigin;
	uint16_t width;
	uint16_t height;
	uint8_t pixelDepth;		// Byte 16
	uint8_t imageDescriptor;
    };

    struct Terrain terrain;

    struct TexLoad texLoad;
    strcpy( texLoad.name, "" );
    strcpy( texLoad.path, "" );

    /*
     * Generate a time stamp string. This string will be used to name material and to name
     * the file which will be used to store terrain data for later *.obj to *.hf conversion.
     */
    char *timeStampString = timeStamp();


    /*
     * Create and open "timeStampString" file to store terrain data
     * for later *.obj to *.hf conversion.
     */
    int homeLength = strlen( getenv("HOME") );
    char v3dConverterPath[] = "/.local/share/v3dconverter/";
    int dirLength = strlen( v3dConverterPath );
    int fileNameLength = strlen( timeStampString );

    char *name = calloc( homeLength + dirLength + fileNameLength + 1, sizeof(char) );
    if ( name == NULL )
    {
	fprintf( stderr, "v3dhftoobj.c: can't allocate memory for name.\n" );
        return -1;
    }

    strcat( name, getenv("HOME") );
    strcat( name, v3dConverterPath );

    errno = 0;
    if ( mkdir(name, S_IRWXU) == -1 && errno != EEXIST )
	fprintf( stderr, "v3dhftoobj.c: %s : %s\n", name, strerror(errno));

    strcat( name, timeStampString );

    if ( ( fpTerrainDataOut = fopen( name, "w" ) ) == NULL ) {
	perror( name );
	return -1;
    }
    else
    {
	free( name );
    }


    /*
     * Open *.3d file, then look for heightfield and texture names.
     */
    if ( ( fp3dFileIn = fopen( source, "r" ) ) == NULL ) {
	perror( source );
	return -1;
    }
    while ( fgets( lineBuffer, MAX_LENGTH, fp3dFileIn ) )
    {
	if ( lineBuffer[ 0 ] == '\n' || lineBuffer[ 0 ] == '\r' || ( sscanf( lineBuffer, " %s[\n]", objTag ) ) == 0 ) // blank line or objTag == ""
	{
	    lineNr++;
	    continue;
	}
	else if ( !strcmp( objTag, "heightfield_load" ) )
	{
	    hasHeightfield = true;
	    int sscanfResult = 0;
	    char additionalParameter[MAX_LENGTH];
	    sscanfResult = sscanf( lineBuffer, "%*s %s %lf %lf %lf %lf %lf %lf %lf %lf %lf %s",
		    terrain.path,
		    &terrain.sizeX, &terrain.sizeY, &terrain.maxAltitude,
		    &terrain.xPos, &terrain.yPos, &terrain.zPos,
		    &terrain.h, &terrain.p, &terrain.b,
		    additionalParameter
  		);
	    if ( sscanfResult != 10 )
	    {
		int cnt = 0;
		while ( lineBuffer[ cnt++ ] != '\n' );
		lineBuffer[ --cnt ] = '\0';
		if ( sscanfResult < 10 )
		    fprintf( stderr, "v3dhftoobj.c: Error while scanning line #%ld \"%s\": %d parameter(s) missing.\n", lineNr, lineBuffer, 10 - sscanfResult );
		else
		    fprintf( stderr, "v3dhftoobj.c: Warning while scanning line #%ld \"%s\": too much parameters.\n", lineNr, lineBuffer);
	    }

	    /* Wants user to not apply *.3d terrain translations? */
	    if ( userModifier->doNotTransformModels == true )
	    {
		terrain.xPos = 0;
		terrain.yPos = 0;
		terrain.zPos = 0;
		//printf( "Info: -do-not-transform option activated.\n);
	    }
	}
	else if ( !strcmp( objTag, "texture_load" ) )
	{
	    hasTexture = true;
	    int sscanfResult = 0;
	    char additionalParameter[MAX_LENGTH];
	    sscanfResult = sscanf( lineBuffer, "%*s %s %s %lf %s",
		    texLoad.name,
		    texLoad.path,
		    &texLoad.priority,
		    additionalParameter
  		);
	    if ( sscanfResult != 3 )
	    {
		int cnt = 0;
		while ( lineBuffer[ cnt++ ] != '\n' );
		lineBuffer[ --cnt ] = '\0';
		if ( sscanfResult < 3 )
		    fprintf( stderr, "v3dhftoobj.c: Error while scanning line #%ld \"%s\": %d parameter(s) missing.\n", lineNr, lineBuffer, 3 - sscanfResult );
		else
		    fprintf( stderr, "v3dhftoobj.c: Warning while scanning line #%ld \"%s\": too much parameters.\n", lineNr, lineBuffer);
	    }
	}
	else
	    ;

	if ( hasHeightfield && hasTexture )
	    break;

	lineNr++;
    }
    fclose( fp3dFileIn );
    fp3dFileIn = NULL;


    /*
     * Read *.hf image header.
     */
    uint8_t imageOrigin;
    uint32_t colorMapBytes;
    struct TgaMinimalHeader hfHeader;
    uint8_t *header = malloc( sizeof( hfHeader ) );
    if ( header == NULL )
    {
	fprintf( stderr, "Can't allocate memory for TGA image header.\n" );

	FREE_AND_CLOSE_ALL_V3DHFTOOBJ
        return -1;
    }

    /* Try to extract texture full path */
    char * str_tmp_0 = strdup( source );
    char * found = strstr( str_tmp_0, "/data/objects/" );
    /* String found? */
    if ( found != NULL ) {
	char * str_tmp_1 = strdup( source );

	str_tmp_0[strlen(source) - strlen(found) + 1] = '\0';
	sprintf( str_tmp_1, "%sdata/%s", str_tmp_0, terrain.path );

	strcpy( terrain.path, str_tmp_1 );

	free(str_tmp_1);
    }
    free(str_tmp_0);

    if ( (fpHfIn = fopen( terrain.path, "rb") ) == NULL ) {
        perror( terrain.path );

	FREE_AND_CLOSE_ALL_V3DHFTOOBJ
        return -1;
    }

    /* Warning: sizeof( hfHeader ) don't work for image reading because of memory alignment. */
    int tgaImageHeaderLength = 18;
    fread( header, 1, tgaImageHeaderLength, fpHfIn );
    hfHeader.idFieldLength = header[0];
    hfHeader.colorMapType = header[1];
    hfHeader.imageType = header[2];
    hfHeader.colorMapIndex = header[3] + header[4] * 256; // LSB, MSB
    hfHeader.colorMapLength = header[5] + header[6] * 256;
    hfHeader.colorMapSize = header[7];
    hfHeader.xOrigin = header[8] + header[9] * 256;
    hfHeader.yOrigin = header[10] + header[11] * 256;
    hfHeader.width = header[12] + header[13] * 256;
    hfHeader.height = header[14] + header[15] * 256;
    hfHeader.pixelDepth = header[16];
    hfHeader.imageDescriptor= header[17];
/*
    long fileSize = 0;
    fileSize = file_size( terrain.path );
    fprintf( stderr, "------ *.hf image: %s, %ld bytes ------\n", terrain.path, fileSize );
    fprintf( stderr, "  idFieldLength = %d\n", hfHeader.idFieldLength );
    fprintf( stderr, "   colorMapType = %d\n", hfHeader.colorMapType );
    fprintf( stderr, "      imageType = %d\n", hfHeader.imageType );
    fprintf( stderr, "  colorMapIndex = %d ( %d + %d * 256 )\n", hfHeader.colorMapIndex, header[3], header[4] );
    fprintf( stderr, " colorMapLength = %d ( %d + %d * 256 )\n", hfHeader.colorMapLength, header[5], header[6] );
    fprintf( stderr, "   colorMapSize = %d\n", hfHeader.colorMapSize );
    fprintf( stderr, "        xOrigin = %d ( %d + %d * 256 )\n", hfHeader.xOrigin, header[8], header[9] );
    fprintf( stderr, "        yOrigin = %d ( %d + %d * 256 )\n", hfHeader.yOrigin, header[10], header[11] );
    fprintf( stderr, "          width = %d ( %d + %d * 256 )\n", hfHeader.width, header[12], header[13] );
    fprintf( stderr, "         height = %d ( %d + %d * 256 )\n", hfHeader.height, header[14], header[15] );
    fprintf( stderr, "     pixelDepth = %d\n", hfHeader.pixelDepth );
    fprintf( stderr, "imageDescriptor = %08b\n\n", hfHeader.imageDescriptor );
*/
    if ( hfHeader.imageType != 3 )
    {
	fprintf( stderr, "Warning: *.hf TGA image should preferably be of type 3 (ie an uncompressed grayscale image).\n" );
    }
    if ( hfHeader.pixelDepth != 8 )
    {
	fprintf( stderr, "Warning: *.hf TGA image should preferably have 8 bits per pixel (%d bits found).\n", hfHeader.pixelDepth );
    }

    imageOrigin = hfHeader.imageDescriptor & 0b00110000;
    if ( imageOrigin == TGA_TOP_LEFT_MASK )
	; // top left origin, okay.
    else if ( imageOrigin == TGA_BOTTOM_LEFT_MASK )
    {
	fprintf( stderr, "Warning: *.hf TGA image should preferably be top left origin (bottom left found and accepted).\n" );
    }
    else if ( imageOrigin == TGA_BOTTOM_RIGHT_MASK )
    {
	fprintf( stderr, "Error: *.hf TGA image should preferably be top left origin (bottom right found).\n" );

	FREE_AND_CLOSE_ALL_V3DHFTOOBJ
	return -1;
    }
    else // imageOrigin == TGA_TOP_RIGHT_MASK
    {
	fprintf( stderr, "Error: *.hf TGA image should preferably be top left origin (top right found).\n" );

	FREE_AND_CLOSE_ALL_V3DHFTOOBJ
	return -1;
    }


    /*
     * Read ID field (if any). We don't need it, but this will increment file data pointer.
     */
    if ( hfHeader.idFieldLength != 0 )
    {
	uint8_t *imageID = malloc( hfHeader.idFieldLength );
	fread( imageID, 1, hfHeader.idFieldLength, fpHfIn );
	free( imageID );
    }


    /*
     * Read color map field (if any). We don't need it, but this will increment file data pointer.
     */
    colorMapBytes = hfHeader.colorMapLength * hfHeader.colorMapSize;
    if ( colorMapBytes != 0 )
    {
	uint8_t *colorMap = malloc( colorMapBytes );
	fread( colorMap, 1, colorMapBytes, fpHfIn );
	free( colorMap );
	fprintf( stderr, "Error: *.hf TGA image shouldn't have any color map (%d length color map found).\n", hfHeader.colorMapLength);

	FREE_AND_CLOSE_ALL_V3DHFTOOBJ
	return -1;
    }
    free( header );
    header = NULL;


    /*
     * Generate output (*.obj and *.mtl) file names.
     */
    scanFileName( dest, &path, &baseName, &extension );
    if ( path[0] == '\0' ) // if no path
	sprintf( buffer, "%s.obj", baseName );
    else
	sprintf( buffer, "%s/%s.obj", path, baseName );

    if ( ( fpObjOut = fopen(buffer, "w" ) ) == NULL) {
        perror(buffer);

	FREE_AND_CLOSE_ALL_V3DHFTOOBJ
        return -1;
    }

    if ( path[0] == '\0' ) // if no path
	sprintf( buffer, "%s.mtl", baseName );
    else
	sprintf( buffer, "%s/%s.mtl", path, baseName );

    if ( ( fpMtlOut = fopen( buffer, "w") ) == NULL ) {
        perror( buffer );

	FREE_AND_CLOSE_ALL_V3DHFTOOBJ
        return -1;
    }


    /*
     * Write *.mtl file name in *.obj file and write material in *.mtl file.
     */
    if ( fpMtlOut != NULL ) {
	/* Write terrain object name in *obj file */
        //fprintf( fpObjOut, "o Terrain\n\n" );

	/* Write material file name in *obj file, and add texture file name (if any) */
        fprintf( fpObjOut, "mtllib ./%s.mtl\n\n", baseName );

	/* Create material in *.mtl file */
	fprintf( fpMtlOut, "newmtl %s\n", timeStampString );
	fprintf( fpMtlOut, "Ns 127.999985\n" );
	fprintf( fpMtlOut, "Ka 1.000000 1.000000 1.000000\n" );
	fprintf( fpMtlOut, "Kd 0.800000 0.800000 0.800000\n" );
	fprintf( fpMtlOut, "Ks 0.000000 0.000000 0.000000\n" );
	fprintf( fpMtlOut, "Ke 0.000000 0.000000 0.000000\n" );
	fprintf( fpMtlOut, "Ni 1.450000\n" );
	fprintf( fpMtlOut, "d 1.000000\n" );
	fprintf( fpMtlOut, "illum 2\n" );
	if ( strlen(texLoad.path) != 0 )
	    fprintf( fpMtlOut, "map_Kd %s\n", texLoad.path );

	fclose( fpMtlOut );
	fpMtlOut = NULL;
    }


    /*
     * Read *.hf altitudes.
     */
    uint8_t bytesPerPixel = hfHeader.pixelDepth / 8;
    uint8_t *altitudeData = malloc( hfHeader.width * hfHeader.height * bytesPerPixel);
    if ( altitudeData == NULL )
    {
	fprintf( stderr, "Can't allocate memory for altitudeData.\n" );

	FREE_AND_CLOSE_ALL_V3DHFTOOBJ
        return -1;
    }

    if ( imageOrigin == TGA_BOTTOM_LEFT_MASK )
    {
	/* Image is bottom left origin: rows order must be inverted in order to obtain a top left origin image. */
	unsigned long bytesPerRow = hfHeader.width * bytesPerPixel;
	uint8_t *tempRow = malloc( bytesPerRow );
	if ( tempRow == NULL )
	{
	    fprintf( stderr, "Can't allocate memory for tempRow.\n" );

	    FREE_AND_CLOSE_ALL_V3DHFTOOBJ
	    return -1;
	}
	for ( int cnt = hfHeader.height - 1; cnt >= 0; cnt-- )
	{
	    fread( tempRow, 1, bytesPerRow, fpHfIn );
	    memcpy( altitudeData + cnt * bytesPerRow, tempRow, bytesPerRow );
	}
	free( tempRow );
    }
    else if ( imageOrigin == TGA_TOP_LEFT_MASK)
    {
	/* Image is top left origin: we just have to read data. */
	fread( altitudeData, 1, hfHeader.width * hfHeader.height * bytesPerPixel, fpHfIn );
    }
    else
    {
	; /* Other cases (imageOrigin == 1 or imageOrigin == 2) have been detected and have generated an error earlier */
    }


    /*
     * Write terrain vertices.
     */
    long offset = 0;
    double perPixelXDistance = terrain.sizeX / (double)( hfHeader.width - 1 );
    double perPixelYDistance = terrain.sizeY / (double)( hfHeader.height - 1 );
    double perPixelZDistance = terrain.maxAltitude / 255; // altitude is always an 8 bits value
    double halfXSize = terrain.sizeX / 2;
    double halfYSize = terrain.sizeY / 2;
    for ( int line = hfHeader.height - 1; line >= 0; line-- )
    {
	for ( int col = 0; col < hfHeader.width; col++ )
	{
	    double x = (double)col * perPixelXDistance - halfXSize;
	    double y = (double)line * perPixelYDistance - halfYSize;
	    double z = (double)altitudeData[ offset ] * perPixelZDistance;
	    offset += bytesPerPixel;

	    /* Apply *.3d terrain translations */
	    x += terrain.xPos;
	    y += terrain.yPos;
	    z += terrain.zPos;

	    /* Apply user translations */
	    x += userModifier->tx;
	    y += userModifier->ty;
	    z += userModifier->tz;

	    /* Apply user scale */
	    x *= userModifier->scale;
	    y *= userModifier->scale;
	    z *= userModifier->scale;

	    /* Write vertex to *.obj file */
	    fprintf( fpObjOut, "v %.6f %.6f %.6f\n", x, z, -y );
	}
    }
    free( altitudeData );


    /*
     * Write texture vertices, if any.
     */
    bool isTextured = true;
    if ( strlen(texLoad.path) == 0 )
    {
	isTextured = false;
    }
    else /* There is a texture to apply */
    {
	/* Write texture vertices in *.obj file */
	for ( int line = hfHeader.height - 1; line >= 0; line-- )
	{
	    for ( int col = 0; col < hfHeader.width; col++ )
	    {
		fprintf( fpObjOut, "vt %.6f %.6f\n", (double)col / (double)(hfHeader.width - 1), (double)line / (double)(hfHeader.height - 1) );
	    }
	}
    }

    /*
     * Write material name.
     */
	fprintf( fpObjOut, "usemtl %s\n", timeStampString );

    /*
     * Set smooth rendering.
     */
    fprintf( fpObjOut, "s 1\n" );


    /*
     * Write terrain faces.
     */
    int hOffset = 0;
    int vOffset = 0;
    for ( int y = 0; y < hfHeader.height - 1; y++ )
    {
	vOffset = y * hfHeader.width;
	for ( int x = 1; x < hfHeader.width; x++ )
	{
	    hOffset = vOffset + x;
	    if ( isTextured ) // print v/vt
	    {
		/* first triangle */
		fprintf(fpObjOut, "f %d/%d %d/%d %d/%d\n",
			hOffset + 1 + hfHeader.width, hOffset + 1 + hfHeader.width,
			hOffset + 1, hOffset + 1,
			hOffset, hOffset
		);

		/* second triangle */
		fprintf(fpObjOut, "f %d/%d %d/%d %d/%d\n", hOffset, hOffset,
			hOffset + hfHeader.width, hOffset + hfHeader.width,
			hOffset + 1 + hfHeader.width, hOffset + 1 + hfHeader.width
		);
	    }
	    else // no texture, don't print vt, print v only
	    {
		/* first triangle */
		fprintf(fpObjOut, "f %d %d %d\n", hOffset + 1 + hfHeader.width, hOffset + 1, hOffset );

		/* second triangle */
		fprintf(fpObjOut, "f %d %d %d\n", hOffset, hOffset + hfHeader.width, hOffset + 1 + hfHeader.width );
	    }
	    triangles += 2;
	}
    }


    /*
     * Store *.3d terrain data for later *.obj to *.hf conversion.
     */
    fprintf( fpTerrainDataOut, "# File automatically generated. Do not edit manually.\n" );
    fprintf( fpTerrainDataOut,   "file=%s.obj\n", baseName );
    fprintf( fpTerrainDataOut,   "material_name=%s\n", timeStampString );
    fprintf( fpTerrainDataOut,   "terrain_width=%.6f\n", terrain.sizeX );
    fprintf( fpTerrainDataOut,  "terrain_height=%.6f\n", terrain.sizeY );
    fprintf( fpTerrainDataOut,"terrain_altitude=%.6f\n", terrain.maxAltitude );
    fprintf( fpTerrainDataOut,  "terrain_transX=%.6f\n", terrain.xPos + userModifier->tx );
    fprintf( fpTerrainDataOut,  "terrain_transY=%.6f\n", terrain.yPos + userModifier->ty );
    fprintf( fpTerrainDataOut,  "terrain_transZ=%.6f\n", terrain.zPos + userModifier->tz );
    fprintf( fpTerrainDataOut,   "terrain_scale=%.6f\n", userModifier->scale );
    fprintf( fpTerrainDataOut, "terrain_bitsWidth=%d\n", hfHeader.width );
    fprintf( fpTerrainDataOut,"terrain_bitsHeight=%d\n", hfHeader.height );
    fclose( fpTerrainDataOut );
    fpTerrainDataOut = NULL;


    /*
     * Print some info to console.
     */
    //printf("%s contains:\n %d triangles.\n", terrain.path, triangles);
    printf("Files %s.obj and %s.mtl written to: ", baseName, baseName);
    if ( path[0] == '\0' ) // if no path
	printf( "./ directory.\n" );
    else
	printf( "%s directory.\n", path );


    FREE_AND_CLOSE_ALL_V3DHFTOOBJ
    return 0;
}
