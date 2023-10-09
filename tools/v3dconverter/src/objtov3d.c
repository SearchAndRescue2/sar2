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

/* *.mtl material */
typedef struct Material Material;
struct Material {
    char *name;
    double KaR; double KaG; double KaB;
    double KdR; double KdG; double KdB;
    double KeR; double KeG; double KeB;
    double KsR; double KsG; double KsB;
    double Ns;
    double Ni;
    double d;
    int illum;
    char *texName;
};

#define MAX_MTLLIB_FILENAMES 10 // a mtllib statement can be followed by multiple file names. Here is defined maximum number of file names.

#define FREE_AND_CLOSE_ALL_OBJTOV3D \
if ( v != NULL ) free( v ); \
if ( vn != NULL ) free( vn ); \
if ( vt != NULL ) free( vt ); \
if ( mat != NULL ) { \
    for ( int cnt = 0; cnt < materials; cnt++ ) { \
	if ( mat[ cnt ].name != NULL ) free( mat[ cnt ].name ); \
	if ( mat[ cnt ].texName != NULL ) free( mat[ cnt ].texName ); \
    } \
    free( mat ); \
} \
if ( fpOut != NULL ) fclose( fpOut ); \
if ( fpIn != NULL ) fclose( fpIn );


int readMtlFile( char *mtllibSource, Material **mat, int *mtl ); // read .mtl (material) file

int objToV3d(const char *source, const char *dest, UserModifier *userModifier ) { // special bit nr0 = 1: invert faces
    FILE *fpIn = NULL, *fpOut = NULL, *fopen();
    long lineNr = 1;
    long vertices = 0, textCoords = 0, normals = 0, faces = 0; // counters.
    int materials = 0; // counters.
    double vx = 0, vy = 0, vz = 0, vni = 0, vnj = 0, vnk = 0;
    bool textureOn = false, firstObject = true;
    double Xmin = 999999999, Xmax = -999999999, Ymin = 999999999, Ymax = -999999999, Zmin = 999999999, Zmax = -999999999; // object rectangular bounds
    double XYradius, maxXYradius = 0; // object cylendrical bounds (maxXYradius, Zmin, Zmax)
    double XYZradius, maxXYZradius = 0; // object spherical bound (maxXYZradius)
    char lineBuffer[MAX_LENGTH + 1], objTag[MAX_LENGTH + 1];
    char previousV3dParam[ 40 + 1 ] = "";
    
    #define V3DMODELTYPES 18+1
    #define V3DMODELTYPEMAXLENGTH 14
    char v3dModelType[ V3DMODELTYPES ][ V3DMODELTYPEMAXLENGTH + 1 ] = {
	{ "standard" }, // "by tradition", first model is always named 'standard'. V3dconverter will automatically create it after "texture_load" statements.
	{ "standard_dawn" },
	{ "standard_dusk" },
	{ "standard_far" },
	{ "standard_night" },
	{ "rotor" },
	{ "aileron_left" },
	{ "aileron_right" },
	{ "rudder_top" },
	{ "rudder_bottom" },
	{ "elevator" },
	{ "flap" },
	{ "air_brake" },
	{ "landing_gear" },
	{ "door" },
	{ "fueltank" },
	{ "cockpit" },
	{ "shadow" },
	{ "" }
    };
    char currentV3dModelType[ V3DMODELTYPEMAXLENGTH + 1 ] = "";
    
    /* Input and output files ok ? */
    if ( ( fpIn = fopen(source, "r" ) ) == NULL ) {
	perror(source);
	return -1;
    }
    if ( ( fpOut = fopen( dest, "w+" ) ) == NULL ) {
	fclose( fpIn );
	perror( dest );
	return -1;
    }
    
    /* Prepare structures */
    Vcoord *v = malloc ( sizeof ( Vcoord ) );
    if ( v == NULL )
    {
	fprintf( stderr, "Can't allocate memory for Vcoord.\n");
	return EXIT_FAILURE;
    }
    
    Vnormal *vn = malloc ( sizeof ( Vnormal ) );
    if ( vn == NULL )
    {
	fprintf( stderr, "Can't allocate memory for Vnormal.\n");
	free( v );
	return EXIT_FAILURE;
    }
    
    Tcoord *vt = malloc ( sizeof ( Tcoord ) );
    if ( vt == NULL )
    {
	fprintf( stderr, "Can't allocate memory for Tcoord.\n");
	free( v );
	free( vn );
	return EXIT_FAILURE;
    }

    Material *mat = malloc ( sizeof ( Material ) );
    if ( mat == NULL )
    {
	fprintf( stderr, "Can't allocate memory for Material.\n");
	free( v );
	free( vn );
	free( vt );
	return EXIT_FAILURE;
    }

    struct Color currentColor = { -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0 };
    
    /* Read and process each *.obj file line */
    while ( fgets(lineBuffer, MAX_LENGTH, fpIn) ) {
	if ( lineBuffer[ 0 ] == '\n' || lineBuffer[ 0 ] == '\r' || ( sscanf( lineBuffer, " %s[\n]", objTag ) ) == 0 ) // blank line or objTag == ""
	{
	    lineNr++;
	    continue;
	}
	else if ( !strcmp( objTag, "v" ) ) // vertex
	{
	    if ( previousV3dParam[0] != '\0' )
	    {
		fprintf( fpOut, "end_%s\n", previousV3dParam ); // write end of previous parameter type (if any)
		strcpy( previousV3dParam, "" );
	    }
	    
	    vertices++; // add a vertex coord. v[0] is never used (as in *.obj file format description).
	    
	    Vcoord *tmp = realloc( v, ( vertices + 1 ) * sizeof( Vcoord ) );
	    if (tmp == NULL)
	    {
		FREE_AND_CLOSE_ALL_OBJTOV3D
		fprintf( stderr, "Line #%ld: Can't allocate memory for Vcoord[%ld].\n", lineNr, vertices );
		return EXIT_FAILURE;
	    }
	    v = tmp;

	    /* read vertex coordinates and re-orient them because obj coords system is different from v3d coords system */
	    sscanf( lineBuffer, " v %lf %lf %lf", &vx, &vy, &vz );
	    double foo = 0; // rotateZ waits for a 'w' value
	    RotateX ( &vx, &vy, &vz, &foo, -90 ); // rotate vertex arround X axis. Angle given in degrees.
	    RotateZ ( &vx, &vy, &vz, &foo, 90 ); // rotate vertex arround Z axis. Angle given in degrees.
	    
	    /* apply user modifiers. Apply order is: X rotation, then Y rotation, then Z rotation, then scale, then translations. */
	    if ( userModifier->rx != 0.0 )
	    {
		RotateX ( &vx, &vy, &vz, &foo, userModifier->rx );
	    }
	    if ( userModifier->ry != 0.0 )
	    {
		RotateY ( &vx, &vy, &vz, &foo, userModifier->ry );
	    }
	    if ( userModifier->rz != 0.0 )
	    {
		RotateZ ( &vx, &vy, &vz, &foo, userModifier->rz );
	    }
	    
	    if ( userModifier->scale != 1.0 )
	    {
		vx *= userModifier->scale;
		vy *= userModifier->scale;
		vz *= userModifier->scale;
	    }
	    
	    if ( userModifier->tx != 0.0 )
	    {
		vx += userModifier->tx;
	    }
	    if ( userModifier->ty != 0.0 )
	    {
		vy += userModifier->ty;
	    }
	    if ( userModifier->tz != 0.0 )
	    {
		vz += userModifier->tz;
	    }
	    
	    /* store coords */
	    v[vertices].x = vx;
	    v[vertices].y = vy;
	    v[vertices].z = vz;
	    
	    /* easy way to compute object rectangular bounds */
	    if ( vx <  Xmin )
		Xmin = vx;
	    if ( vx >  Xmax )
		Xmax = vx;
	    if ( vy <  Ymin )
		Ymin = vy;
	    if ( vy >  Ymax )
		Ymax = vy;
	    if ( vz <  Zmin )
		Zmin = vz;
	    if ( vz >  Zmax )
		Zmax = vz;
	    
	    /* cylindrical bounds */
	    XYradius = sqrt( pow( vx, 2 ) + pow( vy, 2 ) );
	    if ( XYradius > maxXYradius )
		maxXYradius = XYradius;

	    /* spherical bound */
	    XYZradius = sqrt( pow( XYradius, 2 ) + pow( vz, 2 ) );
	    if ( XYZradius > maxXYZradius )
		maxXYZradius = XYZradius;
	}
	else if ( !strcmp( objTag, "vn" ) ) // vertex normal
	{
	    if ( previousV3dParam[0] != '\0' )
	    {
		fprintf( fpOut, "end_%s\n", previousV3dParam ); // write end of previous parameter type (if any)
		strcpy( previousV3dParam, "" );
	    }
	    
	    normals++; // add a vertex normal. vn[0] is not used.
	    
	    Vnormal *tmp = realloc( vn, ( normals + 1 ) * sizeof( Vnormal ) );
	    if (tmp == NULL)
	    {
		FREE_AND_CLOSE_ALL_OBJTOV3D
		fprintf( stderr, "Line #%ld: Can't allocate memory for Vnormal[%ld].\n", lineNr, normals );
		return EXIT_FAILURE;
	    }
	    vn = tmp;
	    
	    /* read vertex normal coordinates and re-orient them because obj coords system is different from v3d coords system */
	    sscanf( lineBuffer, " vn %lf %lf %lf", &vni, &vnj, &vnk );
	    double foo = 0; // rotateZ waits for a 'w' value
	    RotateX ( &vni, &vnj, &vnk, &foo, -90 ); // rotate vertex arround X axis. Angle given in degrees.
	    RotateZ ( &vni, &vnj, &vnk, &foo, 90 ); // rotate vertex arround Z axis. Angle given in degrees.
	    
	    /* it's a normal, no need to apply user modifiers. */
	    
	    /* store coords */
	    vn[normals].i = vni;
	    vn[normals].j = vnj;
	    vn[normals].k = vnk;
	}
	else if ( !strcmp( objTag, "vt" ) ) // texture vertex
	{
	    if ( previousV3dParam[0] != '\0' )
	    {
		fprintf( fpOut, "end_%s\n", previousV3dParam ); // write end of previous parameter type (if any)
		strcpy( previousV3dParam, "" );
	    }
	    
	    textCoords++; // add a vertex texture coord. vt[0] is not used.
	    
	    Tcoord *tmp = realloc( vt, ( textCoords + 1 ) * sizeof( Tcoord ) );
	    if (tmp == NULL)
	    {
		FREE_AND_CLOSE_ALL_OBJTOV3D
		fprintf( stderr, "Line #%ld: Can't allocate memory for Tcoord[%ld].\n", lineNr, textCoords );
		return EXIT_FAILURE;
	    }
	    vt = tmp;
	    
	    sscanf( lineBuffer, " vt %lf %lf %lf", &vt[textCoords].u, &vt[textCoords].v, &vt[textCoords].w );
	}
	else if ( !strcmp( objTag, "p" ) ) // point
	{
	    if ( strcmp( previousV3dParam, "points" ) ) // previous type was not points ?
	    {
		if ( previousV3dParam[0] != '\0' )
		    fprintf( fpOut, "end_%s\n", previousV3dParam ); // write end of previous parameter type (if any)
		else
		    fprintf( fpOut, "begin_points\n" );
		
		strcpy( previousV3dParam, "points" );
	    }
	    
	    const char delim[2] = " ";
	    char *token;
	    int vNum = 0;
	    token = strtok(lineBuffer, delim); // first token contains "p" (point)
	    token = strtok(NULL, delim); // second token contains first point vertex
	    while( token != NULL ) {
		sscanf( token, " %d", &vNum ); // read vertex number
		
		/* write point coord
		 * Info: an obj point don't have a normal, but a v3d point seems to have one (which is always 0.0 0.0 1.0 ?).
		 * FIXME: is it necessary to add a normal, and if yes, how to compute this normal? This point is not on a surface!
		 */
		if ( vNum > 0 )
		    fprintf( fpOut, " %f %f %f\n", v[vNum].x, v[vNum].y, v[vNum].z );
		else
		    fprintf( fpOut, " %f %f %f\n", v[vertices + vNum].x, v[vertices + vNum].y, v[vertices + vNum].z );
	    }
	}
	else if ( !strcmp( objTag, "l" ) ) // line
	{
	    // count line vertices
	    int lineVerticies = 0, lastStatementPos = 0, firstVertexNr = 0, lastVertexNr = 0;
	    int vNum = 0, vtNum = 0;
	    const char delim[2] = " ";
	    char *token;
	    
	    for ( int cnt = 0; cnt < strlen( lineBuffer ); cnt++ ) {
		if ( lineBuffer[ cnt ] == ' ' ||  lineBuffer[ cnt ] == '\t' )
		{
		    lineVerticies++;
		    while ( lineBuffer[ cnt ] == ' ' ||  lineBuffer[ cnt ] == '\t' ) cnt++; // if multiple spaces / tabs
		    lastStatementPos = cnt - 1;
		}
	    }
	    
	    char lastToken[MAX_LENGTH + 1];
	    for ( int cnt = lastStatementPos; lineBuffer[ cnt ] != '\0'; cnt++ )
		lastToken[ cnt - lastStatementPos ] = lineBuffer[ cnt + 1 ];
	    sscanf( lastToken, " %d", &lastVertexNr );
	    sscanf( lineBuffer, " l %d", &firstVertexNr );
	    
	    if ( lineVerticies == 2 ) // it's a single line
	    {
		if ( strcmp( previousV3dParam, "lines" ) ) // if previous type was not "lines"
		{
		    fprintf( fpOut, "end_%s\n", previousV3dParam ); // write end of previous parameter type (if any)
		    fprintf( fpOut, "begin_lines\n" );
		    
		    strcpy( previousV3dParam, "lines" );
		}
	    }
	    else if ( firstVertexNr == lastVertexNr ) // there are more than 2 vertices and it's a line loop
	    {
		// always only one loop per v3d "begin_line_loop" statement: no need to check previous type, just "end_" it.
		if ( previousV3dParam[0] != '\0' )
		    fprintf( fpOut, "end_%s\n", previousV3dParam ); // write end of previous parameter type (if any)
		
		fprintf( fpOut, "begin_line_loop\n" );
		strcpy( previousV3dParam, "line_loop" );

		lineVerticies--; // will be used when writing datas to avoid last vertex print in case of line loop
		
	    }
	    else // there are more than 2 vertices, and it's a line strip
	    {
		// always only one strip per v3d "begin_line_strip" statement: no need to check previous type, just "end_" it.
		if ( previousV3dParam[0] != '\0' )
		    fprintf( fpOut, "end_%s\n", previousV3dParam ); // write end of previous parameter type (if any)
		
		fprintf( fpOut, "begin_line_strip\n" );
		strcpy( previousV3dParam, "line_strip" );
	    }
	    
	    // write datas
	    token = strtok( lineBuffer, delim ); // first token contains "l" (line)
	    token = strtok( NULL, delim ); // second token contains first v/vt reference numbers
	    while( token != NULL ) {
		vNum = 0; vtNum = 0;
		sscanf( token, " %d/%d", &vNum, &vtNum );
		
		if ( lineVerticies > 0 ) // check lineVerticies value to avoid last vertex print in case of line loop
		{
		    if ( textureOn == true )
		    {
			// write texture coord only if vtNum is not equal to 0
			if ( vtNum > 0 )
			    fprintf( fpOut, " texcoord %f %f\n", vt[vtNum].u, vt[vtNum].v );
			else if ( vtNum < 0 )
			    fprintf( fpOut, " texcoord %f %f\n", vt[textCoords + vtNum].u, vt[textCoords + vtNum].v );
		    }
		    
		    // write vertex coord
		    if ( vNum > 0 )
			fprintf( fpOut, " %f %f %f\n", v[vNum].x, v[vNum].y, v[vNum].z );
		    else
			fprintf( fpOut, " %f %f %f\n", v[vertices + vNum].x, v[vertices + vNum].y, v[vertices + vNum].z );
		}
		
		lineVerticies--;
		token = strtok( NULL, delim ); // next token
	    }
	}
	else if ( !strcmp( objTag, "f" ) ) // face
	{
	    char *token;
	    long vNum = 0, vtNum = 0, vnNum = 0;
	    int faceVerticies = 0;
	    
	    faces++;

	    /* count face vertices in order to determine face type (triangle, quad, polygon) */
	    int cnt = 0;
	    int bufLength = strlen( lineBuffer );
	    while ( ( cnt < bufLength ) && ( lineBuffer[ cnt ] != 'f' ) ) // look for 'f'
		cnt++;
	    cnt++; // go to next character
	    while ( cnt < bufLength  )
	    {
		while ( ( cnt < bufLength ) && ( isspace( lineBuffer[ cnt ] ) ) ) // look for next non-blank character
		    cnt++;
		while ( ( cnt < bufLength ) && ( !isspace( lineBuffer[ cnt ] ) ) ) // look for next blank character
		    cnt++;
		if ( cnt < bufLength )
		    faceVerticies++;
	    }
	    
	    if ( faceVerticies == 3 ) // triangle
	    {
		if ( strcmp( previousV3dParam, "triangles" ) ) // previous surface type was not triangles ?
		{
		    if ( previousV3dParam[0] != '\0' )
			fprintf( fpOut, "end_%s\n", previousV3dParam ); // write end of previous parameter type (if any)
		    
		    fprintf( fpOut, "begin_triangles\n" );
		    strcpy( previousV3dParam, "triangles" );
		}
	    }
	    else if ( faceVerticies == 4 ) // quad
	    {
		if ( strcmp( previousV3dParam, "quads" ) ) // previous surface type was not quads ?
		{
		    if ( previousV3dParam[0] != '\0' )
			fprintf( fpOut, "end_%s\n", previousV3dParam ); // write end of previous parameter type (if any)
		    
		    fprintf( fpOut, "begin_quads\n" );
		    strcpy( previousV3dParam, "quads" );
		}
	    }
	    else if ( faceVerticies > 4 ) // if faceVerticies is > 4, it is a polygon.
	    {
		// always only one polygon per "begin_polygon" statement: no need to check previous type, just "end_" it.
		if ( previousV3dParam[0] != '\0' )
		    fprintf( fpOut, "end_%s\n", previousV3dParam ); // write end of previous parameter type (if any)
		fprintf( fpOut, "begin_polygon\n" );
		
		strcpy( previousV3dParam, "polygon" );
	    }
	    else  // faceVerticies < 3 : should never happen !!!
	    {
		fprintf( stderr, " Warning in *.obj file, line #%ld: face with only %d vertices detected and removed.\n", lineNr, faceVerticies );
	    }
	    
	    /* alloc memory */
	    Vertex *tmpVertex = calloc ( faceVerticies, sizeof ( Vertex ) );
	    if ( tmpVertex == NULL )
	    {
		fprintf( stderr, "Can't allocate memory for tmpVertex.\n");
		FREE_AND_CLOSE_ALL_OBJTOV3D
		return EXIT_FAILURE;
	    }
	    
	    Vcoord *tmpVertex2 = calloc ( faceVerticies, sizeof ( Vcoord ) ); // for surface normal calculation (when necessary)
	    if ( tmpVertex2 == NULL )
	    {
		fprintf( stderr, "Can't allocate memory for tmpVertex2.\n");
		FREE_AND_CLOSE_ALL_OBJTOV3D
		return EXIT_FAILURE;
	    }
	    
	    /* face type (triangle, quad, polygon) has been determined. Now, read input line until end and write face vertices. */
	    token = strtok( lineBuffer, " " ); // first token contains "f" (face)
	    token = strtok( NULL, " " ); // second token contains first v/vt/vn reference numbers
	    long tabCounter;
	    double first_i, first_j, first_k;
	    bool faceNormal = true;
	    
	    if ( userModifier->invertFaces == 0 ) // default case: faces MUST be re-oriented
	    {
		tabCounter = faceVerticies;
		
		while( token != NULL ) {
		    tabCounter--;
		    
		    if ( !( sscanf( token, " */%ld/*", &tmpVertex[tabCounter].vt ) ) ) // 'v' / no 'vt' / 'vn' case : sscanf will stop reading after 'v'
			sscanf( token, " %ld//%ld", &tmpVertex[tabCounter].v, &tmpVertex[tabCounter].vn );
		    sscanf( token, " %ld/%ld/%ld", &tmpVertex[tabCounter].v, &tmpVertex[tabCounter].vt, &tmpVertex[tabCounter].vn );
		    
		    token = strtok( NULL, " " ); // next token
		}

		for ( int cnt0 = 0; cnt0 < faceVerticies; cnt0++ )
		{
		    tmpVertex2[ cnt0 ].x = v[ cnt0 + 1 ].x; // + 1 because first *.obj vertex number is always equal to 1
		    tmpVertex2[ cnt0 ].y = v[ cnt0 + 1 ].y;
		    tmpVertex2[ cnt0 ].z = v[ cnt0 + 1 ].z;
		}
	    }
	    else // user asked to re-orient faces, i.e. they MUST NOT be re-oriented!
	    {
		tabCounter = 0;
		while( token != NULL ) {
		    if ( !( sscanf( token, " */%ld/*", &tmpVertex[tabCounter].vt ) ) ) // 'v' / no 'vt' / 'vn' case : sscanf will stop reading after 'v'
			sscanf( token, " %ld//%ld", &tmpVertex[tabCounter].v, &tmpVertex[tabCounter].vn ); // read v and vn
		    sscanf( token, " %ld/%ld/%ld", &tmpVertex[tabCounter].v, &tmpVertex[tabCounter].vt, &tmpVertex[tabCounter].vn ); // read v, vt, and vn if any
		    
		    token = strtok( NULL, " " ); // next token
		    
		    tabCounter++;
		}
		for ( int cnt0 = 0; cnt0 < faceVerticies; cnt0++ )
		{
		    tmpVertex2[ cnt0 ].x = v[ faceVerticies - cnt0 ].x; // last [ faceVerticies - cnt0 ] will be equal to 1, which is first *.obj vertex number
		    tmpVertex2[ cnt0 ].y = v[ faceVerticies - cnt0 ].y;
		    tmpVertex2[ cnt0 ].z = v[ faceVerticies - cnt0 ].z;
		}
	    }
	    
	    /* check faces has identical vertices */
	    bool faceHasIdenticalVertexIndicies = false;
	    for ( int cnt0 = 0; cnt0 < faceVerticies - 1; cnt0++ )
	    {
		for ( int cnt1 = cnt0 + 1; cnt1 < faceVerticies; cnt1++ )
		{
		    if ( tmpVertex[cnt1].v == tmpVertex[cnt0].v )
			faceHasIdenticalVertexIndicies = true;
		    if ( faceHasIdenticalVertexIndicies == true )
			break;
		}
		if ( faceHasIdenticalVertexIndicies == true )
		    break;
	    }
	    
	    if ( faceHasIdenticalVertexIndicies == true )
	    {
		fprintf( stderr, " Warning in *.obj file, line #%ld: identical vertex indicies found in the same face. Face ignored.\n", lineNr );
		lineNr++;
		continue; // go to *.obj file next line
	    }

	    // if vertices don't have normals, then calculate face normal and write it
	    if ( tmpVertex[1].vn == 0 ) // if first vertex normal is not defined
	    {
		faceNormal = true;
		Vnormal vnTemp;
		if ( !calculateSurfaceNormal( tmpVertex2, (int) faceVerticies, &vnTemp ) )
		    fprintf( fpOut, " normal %lf %lf %lf\n", vnTemp.i, vnTemp.j, vnTemp.k );
		else
		    ; // DIV BY ZERO ERROR when calculating normal
	    }
	    else
	    { // check if normal is a face normal (i.e. not a vertex one). If all vertices on a face have the same normal, then normal is a face normal. 
		vnNum = tmpVertex[0].vn;
		if ( vnNum > 0 )
		{
		    first_i = vn[vnNum].i;
		    first_j = vn[vnNum].j;
		    first_k = vn[vnNum].k;
		}
		else // vnNum < 0
		{
		    first_i = vn[normals + vnNum].i;
		    first_j = vn[normals + vnNum].j;
		    first_k = vn[normals + vnNum].k;
		}
		
		for ( tabCounter = 1; tabCounter < faceVerticies; tabCounter++ )
		{
		    vnNum = tmpVertex[tabCounter].vn;
		    if ( vnNum > 0 )
		    {
			if ( ( vn[vnNum].i != first_i ) || ( vn[vnNum].j != first_j ) || ( vn[vnNum].k != first_k ) ) 
			{
			    faceNormal = false;
			    break;
			}
		    }
		    else // vnNum is negative (a *.obj vertex number can't be null)
		    {
			if ( ( vn[normals + vnNum].i != first_i ) || ( vn[normals + vnNum].j != first_j ) || ( vn[normals + vnNum].k != first_k ) ) 
			{
			    faceNormal = false;
			    break;
			}
		    }
		}
		if ( faceNormal == true )
		{
		    fprintf( fpOut, " normal %f %f %f\n", first_i, first_j, first_k );
		}
	    }
	    free( tmpVertex2 );
	    
	    for ( tabCounter = 0; tabCounter < faceVerticies; tabCounter++ )
	    {
		vnNum = tmpVertex[tabCounter].vn;
		if ( faceNormal == false )
		{
		    // write vertex normal only if vnNum is not equal to 0
		    if ( vnNum == 0 )
			;
		    else if ( vnNum > 0 )
			fprintf( fpOut, " normal %f %f %f\n", vn[vnNum].i, vn[vnNum].j, vn[vnNum].k );
		    else // vnNum < 0
			fprintf( fpOut, " normal %f %f %f\n", vn[normals + vnNum].i, vn[normals + vnNum].j, vn[normals + vnNum].k );
		}
		
		vtNum = tmpVertex[tabCounter].vt;
		// write texture coord only if vtNum is not equal to 0 and texture is on.
		// "textureOn" test is not mandatory because texcoord statement is automatically ignored by v3d if texture is off,
		// but v3d file will be smaller if we test it...
		if ( vtNum == 0 || textureOn == false )
		    ;
		else if ( vtNum > 0 )
		    fprintf( fpOut, " texcoord %f %f\n", vt[vtNum].u, vt[vtNum].v );
		else // vtNum < 0
		    fprintf( fpOut, " texcoord %f %f\n", vt[textCoords + vtNum].u, vt[textCoords + vtNum].v );
		
		vNum = tmpVertex[tabCounter].v;
		// write vertex coord
		if ( vNum > 0 )
		    fprintf( fpOut, " %f %f %f\n", v[vNum].x, v[vNum].y, v[vNum].z );
		else // vNum < 0
		    fprintf( fpOut, " %f %f %f\n", v[vertices + vNum].x, v[vertices + vNum].y, v[vertices + vNum].z );
	    }
	    
	    free( tmpVertex );
	}
	else if ( !strcmp( objTag, "mtllib" ) ) // material file
	{
	    /*
	     * According to *.obj specifications, mtllib can be followed by multiple *.mtl file names
	    */
	    char mtlSourceTmp[ MAX_LENGTH + 1 ];
	    char mtlSource[ 2 * MAX_LENGTH + 2 ];
	    
	    char *path = NULL;
	    char *textureBaseName = NULL;
	    char *modelBaseName = NULL;
	    char *extension = NULL;
	    scanFileName( source, &path, &modelBaseName, &extension ); // scan source model name to extract path

	    /* remove 'mtllib' then copy (all) file(s) name(s) to tempBuffer */
	    char *ret;
	    int start = 0, end = 0, cnt = 0, originPos = 0, copyPos = 0;
	    char tempBuffer[MAX_LENGTH + 1];
	    strcpy( tempBuffer, lineBuffer );
	    ret = strstr( tempBuffer, "mtllib" );
	    start = ret + strlen( "mtllib" ) - tempBuffer;
	    while ( lineBuffer[ start ] == ' ' || lineBuffer[ start ] == '\t' )
		start++;
	    copyPos = 0;
	    while ( lineBuffer[ end ] == ' ' || lineBuffer[ end ] == '\t' )
		end++;
	    for ( originPos = start; originPos < strlen ( lineBuffer ); originPos++ )
		tempBuffer[ copyPos++ ] = lineBuffer[ originPos ];
	    tempBuffer[ copyPos ] = '\0';
	    
	    int mtllibFileStart[MAX_MTLLIB_FILENAMES], mtllibFileNr = 0;
	    mtllibFileStart[mtllibFileNr++] = 0;

	    for ( cnt = 0; cnt < strlen( tempBuffer ); cnt++ )
	    {
		if ( ( ret = strstr( ( tempBuffer + cnt), ".mtl" ) ) )
		{
		    mtllibFileStart[ mtllibFileNr ] = ret - tempBuffer + strlen( ".mtl" );
		    cnt = mtllibFileStart[ mtllibFileNr ];
		    mtllibFileNr++;
		}
		if ( mtllibFileNr > MAX_MTLLIB_FILENAMES - 1 )
		{
		    fprintf(stderr, "Line #%ld: more than %d file names found in mtllib statement. Exiting.\n", lineNr, MAX_MTLLIB_FILENAMES );
		    FREE_AND_CLOSE_ALL_OBJTOV3D
		    return EXIT_FAILURE;
		}
	    }

	    for ( cnt = 0; cnt < mtllibFileNr - 1; cnt ++ )
	    {
		while ( tempBuffer[ mtllibFileStart[ cnt ] ] == ' ' || tempBuffer[ mtllibFileStart[ cnt ] ] == '\t' )
		    mtllibFileStart[ cnt ] += 1;
		copyPos = 0;
		for ( int cnt1 = mtllibFileStart[ cnt ]; cnt1 < mtllibFileStart[ cnt + 1 ]; cnt1++ )
		    mtlSourceTmp[ copyPos++ ] = tempBuffer[ cnt1 ];
		mtlSourceTmp[ copyPos ] = '\0';
		
		sprintf( mtlSource, "%s%s", path, mtlSourceTmp ); // put *.obj file path before mtl file name
		
		int result = readMtlFile( mtlSource, &mat, &materials );
		if ( result == -1 )
		{
		    fprintf(stderr, "Line #%ld: file '%s' not found. Exiting.\n", lineNr, mtlSource );
		    FREE_AND_CLOSE_ALL_OBJTOV3D
		    return EXIT_FAILURE;
		}
		else if ( result == -2 )
		{
		    fprintf(stderr, "Line #%ld: something wrong has happen in %s file. Exiting.\n", lineNr, mtlSource );
		    FREE_AND_CLOSE_ALL_OBJTOV3D
		    return EXIT_FAILURE;
		}
	    }
	    
	    fprintf( fpOut, "begin_header\n" );
	    fprintf( fpOut, "creator v3dconverter\n" );
	    
	    if ( materials > 0 )
	    {
		if ( path != NULL )
		{
		    free( path );
		    path = NULL;
		}
		if ( textureBaseName != NULL )
		{
		    free( textureBaseName );
		    textureBaseName = NULL;
		}
		if ( modelBaseName != NULL )
		{
		    free( modelBaseName );
		    modelBaseName = NULL;
		}
		if (extension != NULL)
		{
		    free( extension );
		    extension = NULL;
		}
		for ( int cnt = 0; cnt < materials; cnt++ )
		{
		    if ( mat[ cnt ].texName != NULL ) // if material is a texture
		    {
			scanFileName( dest, &path, &modelBaseName, &extension ); // scan destination model name to extract model base name
			for ( int cnt = 0; modelBaseName[ cnt ] != '\0'; cnt++ ) // replace all spaces by underscores before because v3d format don't like spaces in pathes / names
			{
			    if ( modelBaseName[ cnt ] == ' ')
				modelBaseName[ cnt ] = '_';
			}
			
			// Use model base name as texture sub-directory name
			fprintf( fpOut, "texture_load %s textures/%s/", mat[ cnt ].name, modelBaseName );
			
			char *fullFileName = strdup( mat[ cnt ].texName );
			
			scanFileName( fullFileName, &path, &textureBaseName, &extension ); // scan material texName to extract texture name
			if ( fullFileName != NULL )
			{
			    free( fullFileName );
			    fullFileName = NULL;
			}
			
			// Use texture base name as *.tex texture file name
			fprintf( fpOut, "%s.tex 0.8\n", textureBaseName );
		    }
		}
		if ( path != NULL )
		{
		    free( path );
		    path = NULL;
		}
		if ( textureBaseName != NULL )
		{
		    free( textureBaseName );
		    textureBaseName = NULL;
		}
		if ( modelBaseName != NULL )
		{
		    free( modelBaseName );
		    modelBaseName = NULL;
		}
		if (extension != NULL)
		{
		    free( extension );
		    extension = NULL;
		}
	    }
	    
	    fprintf( fpOut, "end_header\n\n" );
	    
	    /* Considering that 'mtllib' is the first statement of an *.obj file, once all texture_load lines are written,
	     * we can write some generic statements.
	     */
	    fprintf( fpOut, "# Generic statements for making this object visible:\n" );
	    //fprintf( fpOut, "type 1\n" ); // deprecated
	    fprintf( fpOut, "range 2000\n\n" );
	    
	    fprintf( fpOut, "# Generic statement to add object smoothing. Generally, buildings do not need to be smoothed:\n" );
	    fprintf( fpOut, "shade_model_smooth\n\n" );
	}
	else if ( !strcmp( objTag, "usemtl" ) )
	{
	    if ( previousV3dParam[0] != '\0' )
	    {
		fprintf( fpOut, "end_%s\n", previousV3dParam ); // write end of previous parameter type (if any)
		strcpy( previousV3dParam, "" );
	    }
	    
	    char mtlName[MAX_LENGTH + 1];
	    int cnt0 = 0, cnt1 = 0;
	    while ( lineBuffer[cnt0++] != ' ' ); // discard "usemtl "
	    while ( cnt0 < strlen( lineBuffer ) ) // copy material name
	    {
		if ( lineBuffer[cnt0] == ' ' )
		    lineBuffer[cnt0] = '_'; // replace ' ' per '_'
		if ( lineBuffer[ cnt0 ] == '\n' || lineBuffer[ cnt0 ] == '\r' )
		    break;
		mtlName[cnt1++] = lineBuffer[cnt0++];
	    }
	    mtlName[cnt1] = '\0'; // close string
	    
	    int mtlNr = 0;
	    while ( mtlNr < materials ) // look for material number
	    {
		if ( !strcmp( mat[ mtlNr ].name, mtlName ) ) // material found
		    break;
		mtlNr++;
	    }

	    if ( mtlNr == materials ) // material not found
	    {
		fprintf(stderr, " Warning in *.obj file, line #%ld: material %s called by *.obj file not found in *.mtl file.\n", lineNr, mtlName );
	    }
	    else
	    {
		double new_r = 0.2, new_g = 0.2, new_b = 0.2, new_a = 1.0, new_amb = 1.0, new_dif = 1.0, new_spe = 0.1, new_shi = 0.0, new_emi = 0.1; // init and default values
		int counter = 0;
		double KTotal = 0;
		
		// *** Remember that readMtlFile() initialized unread values to -1.0 ***
		
		/* Color */
		if ( mat[ mtlNr ].KdR >= 0 )
		    new_r = mat[ mtlNr ].KdR; // use mtl diffuse red color as v3d base color
		else
		    ; // use default new_r value
		if ( mat[ mtlNr ].KdG >= 0 )
		    new_g = mat[ mtlNr ].KdG; // use mtl diffuse green color as v3d base color
		else
		    ; // use default new_g value
		if ( mat[ mtlNr ].KdB >= 0 )
		    new_b = mat[ mtlNr ].KdB; // use mtl diffuse blue color as v3d base color
		else
		    ; // use default new_b value
		
		/* Transparency */
		if ( mat[ mtlNr ].d >= 0 )
		    new_a = mat[ mtlNr ].d; // transparency ( mtl "dissolve" coefficient )
		else
		    ; // use default new_a value
		
		/* Ambient coefficient */
		new_amb = 1.0; // FIXME force ambient coefficient to 1.0 (as Blender seems to do).
		
		/* Diffuse coefficient */
		new_dif = 1.0; // mtl Kd diffuse color was used as v3d base color, then Kd(r,g,b) / new_(r,g,b) = 1.0
		
		/* Specularity coefficent. FIXME Is this calculation right ??? */
		counter = 0;
		KTotal = 0.0;
		if ( mat[ mtlNr ].KsR >= 0 )
		{
		    KTotal = mat[ mtlNr ].KsR;
		    counter++;
		}
		if ( mat[ mtlNr ].KsG >= 0 )
		{
		    KTotal += mat[ mtlNr ].KsG;
		    counter++;
		}
		if ( mat[ mtlNr ].KsB >= 0 )
		{
		    KTotal += mat[ mtlNr ].KsB;
		    counter++;
		}
		if ( counter != 0 )
		    new_spe = KTotal / counter;
		else
		    ; // use default new_spe value
		
		/* Shininess coefficient */
		if ( mat[ mtlNr ].Ns >= 0 )
		    new_shi = mat[ mtlNr ].Ns / 1000; // Ns range is 0 to 1000
		else
		    ; // use default value
		
		/* Emission coefficient. FIXME Is this calculation right ??? */
		counter = 0;
		KTotal = 0.0;
		if ( mat[ mtlNr ].KeR >= 0 )
		{
		    KTotal = mat[ mtlNr ].KeR;
		    counter++;
		}
		if ( mat[ mtlNr ].KeG >= 0 )
		{
		    KTotal += mat[ mtlNr ].KeG;
		    counter++;
		}
		if ( mat[ mtlNr ].KeB >= 0 )
		{
		    KTotal += mat[ mtlNr ].KeB;
		    counter++;
		}
		if ( counter != 0 )
		    new_emi = KTotal / counter;
		else
		    ; // use default new_emi value
		
		if ( mat[ mtlNr ].texName != NULL ) // if material is a texture, force color to white
		{
		    new_r = 1.0;
		    new_g = 1.0;
		    new_b = 1.0;
		}
		
		/* check if current color is same as new one */
		if (	 currentColor.r == new_r
		    &&   currentColor.g == new_g
		    &&   currentColor.b == new_b
		    &&   currentColor.a == new_a
		    && currentColor.amb == new_amb
		    && currentColor.dif == new_dif
		    && currentColor.spe == new_spe
		    && currentColor.shi == new_shi
		    && currentColor.emi == new_emi
		)
		{
		    ; // current color is same as the new one, no need to repeat it
		}
		else
		{
		    // set current color to new value, then print it
		    currentColor.r = new_r;
		    currentColor.g = new_g;
		    currentColor.b = new_b;
		    currentColor.a = new_a;
		    currentColor.amb = new_amb;
		    currentColor.dif = new_dif;
		    currentColor.spe = new_spe;
		    currentColor.shi = new_shi;
		    currentColor.emi = new_emi;
		    
		    if ( mat[ mtlNr ].texName == NULL ) // if material has no texture ...
		    {
			if ( textureOn == true ) // ... and if previous material was a textured one
			{
			    fprintf( fpOut, "texture_off\n" );
			    textureOn = false;
			}
		    }
		    
		    fprintf( fpOut, "\n#OBJFILE material: %s\n", mtlName );
		    fprintf( fpOut, "color %.6f %.6f %.6f", currentColor.r, currentColor.g, currentColor.b ); // v3d base color
		    fprintf( fpOut, " %.6f", currentColor.a  ); // transparency
		    fprintf( fpOut, " %.6f %.6f %.6f %.6f %.6f\n", currentColor.amb, currentColor.dif, currentColor.spe, currentColor.shi, currentColor.emi ); // coefficients
		}
		
		if ( mat[ mtlNr ].texName != NULL ) // if material is a texture, print it
		{
		    fprintf( fpOut, "texture_select %s\n", mat[ mtlNr ].name );
		    textureOn = true;
		}
		
		fprintf( fpOut, "\n" );
	    }
	}
	/* ### See "o" (object) ###
	else if ( !strcmp( objTag, "g" ) ) // group
	{
	    // remove carriage return
	    int cnt = 0;
	    while ( lineBuffer[ cnt ] != '\n' && lineBuffer[ cnt++ ] != '\r' );
	    lineBuffer[ cnt ] = '\0';
	    
	    fprintf( fpOut, "#OBJFILE group: %s\n", lineBuffer );
	}
	*/
	else if ( !strcmp( objTag, "s" ) ) // smoothing group
	{
	    if ( previousV3dParam[0] != '\0' )
	    {
		fprintf( fpOut, "end_%s\n", previousV3dParam ); // write end of previous parameter type (if any)
		strcpy( previousV3dParam, "" );
	    }
	    
	    // Don't convert obj 's' statement into v3d 'shade_model_*' because only one 'shade_model_*' per V3d object.
	    /*
	    int foo = -1;
	    if ( sscanf( lineBuffer, " s %d", &foo ) ) // if foo is an int
	    {
		if ( foo > 0 ) fprintf( fpOut, "shade_model_smooth\n" ); // smooth on
	    }
	    else
	    {
		fprintf( fpOut, "shade_model_flat\n" ); // smooth off
	    }
	    */
	}
	else if ( !strcmp( objTag, "o" ) || !strcmp( objTag, "g" ) ) // new object or group
	{
	    if ( previousV3dParam[0] != '\0' )
	    {
		fprintf( fpOut, "end_%s\n", previousV3dParam ); // write end of previous parameter type (if any)
		strcpy( previousV3dParam, "" );
	    }

	    if ( textureOn )
	    {
		fprintf( fpOut, "texture_off\n" );
		textureOn = false;
	    }
	    
	    /* check if object name is known as a v3d model type */
	    char objectName[MAX_LENGTH + 1];
	    int cnt = 0;
	    sscanf( lineBuffer, " %*s %[^\n]", objectName ); // read object name
	    
	    while ( cnt < V3DMODELTYPES )
	    {
		if ( !strcmp( objectName, v3dModelType[ cnt ] ) ) // if object name found in v3dModelType[] array
		    break;
		cnt++;
	    }
	    
	    if ( cnt != V3DMODELTYPES ) // if object name is a v3d model type
	    {
		if ( currentV3dModelType[0] != '\0' )
		{
		    fprintf( fpOut, "end_model %s\n", currentV3dModelType );
		}
		
		fprintf( fpOut, "begin_model %s\n", objectName );
		strcpy( currentV3dModelType, objectName );
	    }
	    else
	    {
		for ( int cnt = 0; objectName[ cnt ] != '\0'; cnt++ ) // replace all spaces by underscores
		{
		    if ( objectName[ cnt ] == ' ')
			objectName[ cnt ] = '_';
		}
		if ( !strcmp( objTag, "o" ) )
		    fprintf( fpOut, "#OBJFILE object:  %s\n", objectName );
		else
		    fprintf( fpOut, "#OBJFILE group:  %s\n", objectName );
		
		if ( firstObject )
		{
		    strcpy( currentV3dModelType, "standard" );
		    fprintf( fpOut, "begin_model standard\n" );
		    firstObject = false;
		}
	    }
	}
	else if ( objTag[0] == '#' ) // comment
	{
	    // remove carriage return
	    int cnt = 0;
	    while ( lineBuffer[ cnt ] != '\n' && lineBuffer[ cnt++ ] != '\r' );
	    lineBuffer[ cnt ] = '\0';
	    
	    fprintf( fpOut, "#OBJFILE comment: %s\n", lineBuffer );
	}
	else if ( !strcmp( objTag, "curv" ) || !strcmp( objTag, "curv2" ) || !strcmp( objTag, "surf" ) )
	{
	    fprintf( stderr, " Warning in *.obj file, line #%ld: free-form geometry conversion not supported.\n", lineNr );
	}
	else
	{
	    if ( previousV3dParam[0] != '\0' )
	    {
		fprintf( fpOut, "end_%s\n", previousV3dParam ); // write end of previous parameter type (if any)
		strcpy( previousV3dParam, "" );
	    }
	    
	    fprintf( stderr, " Warning in *.obj file, line #%ld: unknown tag \"%s\".\n", lineNr, objTag );
	}

	lineNr++;
	
    } // end of main file read/write loop
	
    if ( previousV3dParam[0] != '\0' )
	fprintf( fpOut, "end_%s\n", previousV3dParam ); // write end of previous parameter type (triangle, quad, line, ...), if any.
    
    if ( currentV3dModelType[0] != '\0' )
	fprintf( fpOut, "end_model %s\n", currentV3dModelType ); // write end of previous model type (if any)
    
    // print some usefull information at start of v3d output file and on console
    FILE* tmp = tmpfile();
    int c;
    rewind ( fpOut );
    while ( ( c = fgetc( fpOut ) ) != EOF )
        fputc( c, tmp );
    rewind (fpOut);
    fprintf( fpOut, "# Object X, Y and Z dimensions [m]: %.3lf %.3lf %.3lf\n", Xmax - Xmin, Ymax - Ymin, Zmax - Zmin );
    fprintf( stdout, "Object X, Y and Z dimensions [m]: %.3lf %.3lf %.3lf\n", Xmax - Xmin, Ymax - Ymin, Zmax - Zmin );
    double volume = (Xmax - Xmin) * (Ymax - Ymin) * (Zmax - Zmin);
    fprintf( fpOut, "# Rectangular Xmin Xmax Ymin Ymax Zmin Zmax bounds [m]: %.3lf %.3lf %.3lf %.3lf %.3lf %.3lf (volume: %.1lf m3)\n", Xmin, Xmax, Ymin, Ymax, Zmin, Zmax, volume );
    fprintf( stdout, "Rectangular Xmin Xmax Ymin Ymax Zmin Zmax bounds [m]: %.3lf %.3lf %.3lf %.3lf %.3lf %.3lf (volume: %.1lf m3)\n", Xmin, Xmax, Ymin, Ymax, Zmin, Zmax, volume );
    volume = M_PI * maxXYradius * maxXYradius * (Zmax - Zmin);
    fprintf( fpOut, "# Cylendrical radius Zmin Zmax bounds [m]: %.3lf %.3lf %.3lf (volume: %.1lf m3)\n", maxXYradius, Zmin, Zmax, volume );
    fprintf( stdout, "Cylendrical radius Zmin Zmax bounds [m]: %.3lf %.3lf %.3lf (volume: %.1lf m3)\n", maxXYradius, Zmin, Zmax, volume );
    volume = 4/3 * M_PI * pow(maxXYZradius, 3.0);
    fprintf( fpOut, "# Spherical bound radius [m]: %.3lf (volume: %.1lf m3)\n\n", maxXYZradius, volume );
    fprintf( stdout, "Spherical bound radius [m]: %.3lf (volume: %.1lf m3)\n", maxXYZradius, volume );
    rewind ( tmp );
    while ( ( c = fgetc( tmp ) ) != EOF )
        fputc( c, fpOut );
    fclose( tmp );

    //fprintf( stderr, "Number of\n  vertices: %ld\n    normals: %ld\n textCoords: %ld\n      faces: %ld\n", vertices, normals, textCoords, faces );
    
    FREE_AND_CLOSE_ALL_OBJTOV3D
     
    return EXIT_SUCCESS;
}


int readMtlFile( char *mtllibSource, Material **mat, int *mtl ) { // read a *.mtl file and store materials in a mat[] array
    FILE *fpIn = NULL, *fopen();
    char lineBuffer[MAX_LENGTH + 1], fileName[MAX_LENGTH + 1] = "", mtlLibTag[MAX_LENGTH + 1];
    long mtlLibLineNr = 1;
    Material *tmpMat = NULL, *localMat = NULL;
    
    if ( ( fpIn = fopen( mtllibSource, "r" ) ) == NULL)
    {
	return -1;
    }
    
    tmpMat = realloc( *mat, (*mtl + 1) * sizeof( Material ) );
    if (tmpMat == NULL)
    {
	fprintf( stderr, "File '%s' , line #%ld: Can't allocate memory for tmpMat.\n", mtllibSource, mtlLibLineNr );
	return -2;
    }
    localMat = tmpMat;

    (*mtl)--; // because mtl will be incremented before first use
    
    // Read each line of .mtl file
    while ( fgets( lineBuffer, MAX_LENGTH, fpIn ) ) {
	if ( lineBuffer[ 0 ] == '\n' || lineBuffer[ 0 ] == '\r' || ( sscanf( lineBuffer, " %s", mtlLibTag ) ) == 0 ) // blank line or objTag == ""
	{
	    mtlLibLineNr++;
	    continue;
	}

	if ( !strcmp( mtlLibTag, "newmtl" ) ) // new material
	{
	    (*mtl)++;
	    
	    tmpMat = realloc( localMat, ( *mtl + 1 ) * sizeof( Material ) );
	    if (tmpMat == NULL)
	    {
		*mat = localMat;
		fprintf( stderr, "File '%s' , line #%ld: Can't allocate memory for more Material.\n", mtllibSource, mtlLibLineNr );
		return -1;
	    }
	    localMat = tmpMat;

	    int cnt0 = 0, cnt1 = 0;
	    while ( lineBuffer[cnt0++] != ' ' ); // discard up to "newmtl "
	    while ( cnt0 < strlen( lineBuffer ) ) // copy material name
	    {
		if ( lineBuffer[cnt0] == ' ' )
		    lineBuffer[cnt0] = '_'; // replace ' ' per '_'
		if ( lineBuffer[ cnt0 ] == '\n' || lineBuffer[ cnt0 ] == '\r' )
		    break;
		fileName[cnt1++] = lineBuffer[cnt0++];
	    }
	    fileName[cnt1] = '\0'; // close string
	    localMat[ *mtl ].name = strdup( fileName );
	    
	    // init
	    localMat[ *mtl ].KaR = -1.0; localMat[ *mtl ].KaG = -1.0; localMat[ *mtl ].KaB = -1.0;
	    localMat[ *mtl ].KdR = -1.0; localMat[ *mtl ].KdG = -1.0; localMat[ *mtl ].KdB = -1.0;
	    localMat[ *mtl ].KeR = -1.0; localMat[ *mtl ].KeG = -1.0; localMat[ *mtl ].KeB = -1.0;
	    localMat[ *mtl ].KsR = -1.0; localMat[ *mtl ].KsG = -1.0; localMat[ *mtl ].KsB = -1.0;
	    localMat[ *mtl ].Ns = -1.0;
	    localMat[ *mtl ].Ni = -1.0;
	    localMat[ *mtl ].d = -1.0;
	    localMat[ *mtl ].illum = -1.0;
	    localMat[ *mtl ].texName = NULL;
	}
	else if ( !strcmp( mtlLibTag, "Ka" ) ) // ambient color (ambient reflectivity of material, from 0.0 to 1.0)
	{
	    sscanf( lineBuffer, " Ka %lf %lf %lf", &localMat[ *mtl ].KaR, &localMat[ *mtl ].KaG, &localMat[ *mtl ].KaB );
	}
	else if ( !strcmp( mtlLibTag, "Kd" ) ) // diffuse color (diffuse reflectivity of material, from 0.0 to 1.0). This is the main object color.
	{
	    sscanf( lineBuffer, " Kd %lf %lf %lf", &localMat[ *mtl ].KdR, &localMat[ *mtl ].KdG, &localMat[ *mtl ].KdB );
	    if ( localMat[ *mtl ].KdR == 0.0 && localMat[ *mtl ].KdG == 0.0 && localMat[ *mtl ].KdB == 0.0 )
	    {
		fprintf( stderr, " Warning in file '%s' line #%ld: pure black (0,0,0) color found and overwritten by (1,1,1).\n", mtllibSource, mtlLibLineNr );
		localMat[ *mtl ].KdR = 1.0 / 255;
		localMat[ *mtl ].KdG = 1.0 / 255;
		localMat[ *mtl ].KdB = 1.0 / 255;
	    }
	}
	else if ( !strcmp( mtlLibTag, "Ke" ) ) // emissive color (from 0.0 to 1.0).
	{
	    sscanf( lineBuffer, " Ke %lf %lf %lf", &localMat[ *mtl ].KeR, &localMat[ *mtl ].KeG, &localMat[ *mtl ].KeB );
	}
	else if ( !strcmp( mtlLibTag, "Ks" ) ) // specular color (specular reflectivity of material, from 0.0 to 1.0)
	{
	    sscanf( lineBuffer, " Ks %lf %lf %lf", &localMat[ *mtl ].KsR, &localMat[ *mtl ].KsG, &localMat[ *mtl ].KsB );
	}
	else if ( !strcmp( mtlLibTag, "Ns" ) ) // specular exponent (from 0 to 1000)
	{
	    sscanf( lineBuffer, " Ns %lf", &localMat[ *mtl ].Ns );
	}
	else if ( !strcmp( mtlLibTag, "Ni" ) ) // optical density (index of refraction, from 0.001 to 10)
	{
	    sscanf( lineBuffer, " Ni %lf", &localMat[ *mtl ].Ni );
	}
	else if ( !strcmp( mtlLibTag, "Tr" ) ) // transparency (from 0.0 to 1.0). Transparency = 1.0 - d
	{
	    // Tr = 0.0 = fully opaque, Tr = 1.0 = fully transparent (opposite as v3d format)
	    double tempTr = 0.0;
	    sscanf( lineBuffer, " Tr %lf", &tempTr );
	    localMat[ *mtl ].d = 1.0 - tempTr; // 'Tr' to 'd' conversion
	    if ( localMat[ *mtl ].d == 0.0 )
		fprintf( stderr, " Warning in file '%s' line #%ld: 'Tr 1.0' makes '%s' invisible.\n", mtllibSource, mtlLibLineNr, localMat[ *mtl ].name );
	}
	else if ( !strcmp( mtlLibTag, "d" ) ) // dissolve (from 0.0 to 1.0). Dissolve = 1.0 - Tr
	{
	    // d = 0.0 = fully dissolved (fully transparent), d = 1.0 = fully opaque (as in v3d format)
	    sscanf( lineBuffer, " d %lf", &localMat[ *mtl ].d );
	    if ( localMat[ *mtl ].d == 0.0 )
		fprintf( stderr, " Warning in file '%s' line #%ld: 'd 0.0' makes '%s' invisible.\n", mtllibSource, mtlLibLineNr, localMat[ *mtl ].name );
	}
	else if ( !strcmp( mtlLibTag, "illum" ) )
	{
	    sscanf( lineBuffer, " illum %d", &localMat[ *mtl ].illum );
	}
	else if ( !strcmp( mtlLibTag, "map_Kd" ) ) // texture
	{
	    int cnt = 0;
	    int bufLength = strlen( lineBuffer );
	    
	    /* Extract file name from buffer string, then replace file name spaces (if any) by underscores */
	    while ( ( cnt < bufLength ) && ( isspace( lineBuffer[ cnt ] ) ) ) // look for next non-blank character
		cnt++;
	    cnt += strlen( "map_Kd" );
	    for ( ; lineBuffer[ cnt ] != '\0'; cnt++ ) // replace all spaces by underscores because v3d format don't like spaces in pathes / names
	    {
		if ( isblank( lineBuffer[ cnt ] ) )
		    lineBuffer[ cnt ] = '_';
	    }
	    fileName[0] = '\0';
	    sscanf( lineBuffer, " map_Kd_%s", fileName );

	    if ( fileName[0] == '\0' ) 
	    {
		fprintf( stderr, " Warning in file '%s' line #%ld: map_Kd has no file name.\n", mtllibSource, mtlLibLineNr );
	    }
	    else if ( fileName[0] != '-' ) 
	    {
		localMat[ *mtl ].texName = strdup( fileName );
	    }
	    else
	    {
		; // If first token after map_Kd begins whith a '-', it's an option. See http://www.paulbourke.net/dataformats/mtl/
		fprintf( stderr, " Warning in file '%s' line #%ld: option '%s' not supported.\n", mtllibSource, mtlLibLineNr, mtlLibTag );
	    }
	}
	else if ( mtlLibTag[0] == '#' ) { // it's a comment
	    ;
	}
	else if ( !strcmp( mtlLibTag, "Tf" ) ) // transmission filter (for transparent materials, from 0.0 to 1.0)
	{
	    ; // See http://www.paulbourke.net/dataformats/mtl/
	    fprintf( stderr, " Warning in file '%s' line #%ld: 'Tf' (Transmission Filter color) not supported.\n", mtllibSource, mtlLibLineNr );
	}
	else if ( !strcmp( mtlLibTag, "map_Ka" ) ) // texture ambient reflectivity
	{
	    ; // See http://www.paulbourke.net/dataformats/mtl/
	    fprintf( stderr, " Warning in file '%s' line #%ld: 'map_Ka' (texture Ambient reflectivity coefficient) not supported.\n", mtllibSource, mtlLibLineNr );
	}
	else if ( !strcmp( mtlLibTag, "map_Ks" ) ) // texture specular reflectivity
	{
	    ; // See http://www.paulbourke.net/dataformats/mtl/
	    fprintf( stderr, " Warning in file '%s' line #%ld: 'map_Ks' (texture Specular reflectivity coefficient) not supported.\n", mtllibSource, mtlLibLineNr );
	}
	else if ( !strcmp( mtlLibTag, "bump" ) ) // bump texture
	{
	    ; // See http://www.paulbourke.net/dataformats/mtl/
	    fprintf( stderr, " Warning in file '%s' line #%ld: 'bump' (surface normal modification texture) not supported.\n", mtllibSource, mtlLibLineNr );
	}
	else if ( !strcmp( mtlLibTag, "map_d" ) ) // scalar texture file or scalar procedural texture file
	{
	    ; // See http://www.paulbourke.net/dataformats/mtl/
	    fprintf( stderr, " Warning in file '%s' line #%ld: 'map_d' (scalar [procedural] texture file) not supported.\n", mtllibSource, mtlLibLineNr );
	}
	else if ( !strcmp( mtlLibTag, "refl" ) ) // reflection map
	{
	    ; // See http://www.paulbourke.net/dataformats/mtl/
	    fprintf( stderr, " Warning in file '%s' line #%ld: 'refl' (reflection map) not supported.\n", mtllibSource, mtlLibLineNr );
	}
	else {
	    fprintf( stderr, " Warning in file '%s' line #%ld: unknown tag '%s'.\n", mtllibSource, mtlLibLineNr, mtlLibTag );
	}

	mtlLibLineNr++; // line counter

	*mat = localMat;
    }
    fclose( fpIn );
    
    (*mtl)++; // because mtl was decremented before entering main reading loop

    return EXIT_SUCCESS;
}
