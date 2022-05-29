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





/**************************************************
*   WARNING: EXPERIMENTAL (BUT FUNCTIONAL)  CODE  *
*                                                 *
*       This whole code must be rewritten !       *
***************************************************/

#define MAX_LENGTH 2048
#define MAX_PARAMETER_LINES 6

struct Modifier {
    double tx;
    double ty;
    double tz;
    double rh; // heading
    double rp; // pitch
    double rb; // bank
};
struct Modifier modifier;


long int lineNr = 1; // input file line counter

long readV3dVerticies ( FILE *fpIn, const char * endBoundString, FILE *fpOut, char* textureOrient, UserModifier *userModifier, struct Modifier modifier ) {
    char lineBuffer[MAX_LENGTH], buffer[MAX_LENGTH], result[MAX_LENGTH];
    int v = 0, vt = 0, vn = 0, primitives = 0;
    double ta = 0, tb = 0, tda = 0, tdb = 0; // texture_orient_ values
    double tu = 0, tv = 0; // texture vertex
    double vx = 0, vy = 0, vz = 0, pw = 0, nx = 0, ny = 0, nz = 0;

    //long position, startPosition, endPosition;
    //startPosition = ftell(fpIn);

    // read and count verticies in *.3d file, then write them to *.obj file
    while ( fgets(lineBuffer, MAX_LENGTH, fpIn) ) {
        sscanf( lineBuffer, "%s", buffer );
        if ( !strcmp(strtoupper( result, buffer), endBoundString) ) { // end of bound
            lineNr++; // input file line counter
            break;
        }
        
        if ( !strcmp(strtoupper( result, buffer), "") ) { // it's an empty line
            break;
        }
        else if ( !strcmp(strtoupper( result, buffer), "NORMAL") ) { // it's a vertex or a face normal
	    //
	    // If it is a face normal, 'vn x y z' must NOT be written because a *.obj surface normal only depends of winding.
	    // Hereunder a very ugly way to detect if it is a surface normal.
	    //
	    fpos_t position;
	    fgetpos(fpIn, &position); // save in-file position
	    int isAVertexNormal = 0;
	    /* Read next three v3d file lines. If "NORMAL" is found in one of these three lines, first "NORMAL" statement was a vertex normal. */
	    for ( int cnt0 = 0; cnt0 < 3; cnt0++ )
	    {
		fgets(lineBuffer, MAX_LENGTH, fpIn);
		sscanf( lineBuffer, "%s", buffer );
		if ( !strcmp(strtoupper( result, buffer), "NORMAL") )
		    isAVertexNormal = 1; // new "NORMAL" statement found, first "NORMAL" statement was a vertex normal
	    }
	    fsetpos(fpIn, &position); // return to original in-file position
	    
	    if ( isAVertexNormal == 1 )
	    {
		vn++;
		sscanf( lineBuffer, "%*s %lf %lf %lf", &nx, &ny, &nz );
		fprintf( fpOut, "vn ");
		
		// apply v3d rotation and position modifiers
		if ( modifier.rh ) RotateZ ( &nx, &ny, &nz, &pw, modifier.rh );
		if ( modifier.rp ) RotateX ( &nx, &ny, &nz, &pw, modifier.rp );
		if ( modifier.rb ) RotateY ( &nx, &ny, &nz, &pw, modifier.rb );
		nx += modifier.tx;
		ny += modifier.ty;
		nz += modifier.tz;
	    
		/* TODO apply user modifiers. Apply order is: X rotation, then Y rotation, then Z rotation, then scale, then translations. */
		/*
		if ( userModifier->rx != 0.0 )
		{
		    RotateX ( &nx, &ny, &nz, &pw, userModifier->rx );
		}
		if ( userModifier->ry != 0.0 )
		{
		    RotateY ( &nx, &ny, &nz, &pw, userModifier->ry );
		}
		if ( userModifier->rz != 0.0 )
		{
		    RotateZ ( &nx, &ny, &nz, &pw, userModifier->rz );
		}
		*/
	    
		/* re-orient normal them because v3d coords system is different from obj coords system */
		RotateZ ( &nx, &ny, &nz, &pw, -90 ); // rotate normal arround Z axis. Angle given in degrees.
		RotateX ( &nx, &ny, &nz, &pw, 90 ); // rotate normal arround X axis. Angle given in degrees.
	    
		// write vertex normal
		fprintf( fpOut, "%lf %lf %lf\n", nx, ny, nz );
	    }
        }
        else if ( !strcmp(strtoupper( result, buffer), "TEXTURE") ) { // it's a texture coordinate
            vt++;
            fprintf( fpOut, "vt ");
	    sscanf( lineBuffer, "%*s %lf %lf", &tu, &tv ); // u, v
	    
            // write texture coords
            fprintf( fpOut, "%lf %lf\n", tu, tv ); // u, v
        }
/*
        else if ( (!strcmp(strtoupper( result, buffer), "TEXTURE") && strlen( result ) == strlen( "TEXTURE" )) || (!strcmp(strtoupper( result, buffer), "TEXCOORD") && strlen( result ) == strlen( "TEXCOORD" )) || (!strcmp(strtoupper( result, buffer), "TEX_COORD") && strlen( result ) == strlen( "TEX_COORD" )) ) { // it's a texture vertex
printf("TEXTURE VERTEX FOUND at line #%ld\n", lineNr);
            vt++;
            fprintf( fpOut, "vt ");
            sscanf( lineBuffer, "%*s %lf %lf %lf", &x, &y, &z ); // u, v, w
        
            //if ( modifier.rh ) RotateZ ( &x, &y, &z, &pw, modifier.rh );
            //if ( modifier.rp ) RotateX ( &x, &y, &z, &pw, modifier.rp );
            //if ( modifier.rb ) RotateY ( &x, &y, &z, &pw, modifier.rb );
        
            //x += modifier.tx;
            //y += modifier.ty;
            //z += modifier.tz;
            fprintf( fpOut, "%lf %lf %lf\n", x, y, z ); // u, v, w
        }
*/
        else { // it's a geometric vertex
            v++;
            fprintf( fpOut, "v ");
            sscanf( lineBuffer, "%lf %lf %lf", &vx, &vy, &vz );
            
            if ( textureOrient[0] != '\0' ) { // if there is a running "texture_orient_" command, then create a texture vertex
                vt++;
                sscanf( textureOrient, "%*s %lf %lf %lf %lf", &ta, &tb, &tda, &tdb ); // "texture_orient_" command parameters
                if ( textureOrient[15] == 'x' || textureOrient[15] == 'X' ) {
                    if ( textureOrient[16] == 'y' || textureOrient[15] == 'Y' ) { // TEXTURE_ORIENT_XY (example: cuda top/bottom)
                        if ( vx > ta ) tu = ( vx - ta ) / tda;
                        else tu = ( ta - vx ) / tda;
                        if ( vy > tb ) tv = ( vy - tb ) / tdb;
                        else tv = ( tb - vy ) / tdb;
                    }
                    else { // TEXTURE_ORIENT_XZ (example: cuda front/rear)
                        if ( vx > ta ) tu = ( vx - ta ) / tda;
                        else tu = ( ta - vx ) / tda;
                        if ( vz > tb ) tv = ( vz - tb ) / tdb;
                        else tv = ( tb - vz ) / tdb;
                    }
                }
                else { // TEXTURE_ORIENT_YZ (example: cuda left/right)
                    if ( vy > ta ) tu = ( vy - ta ) / tda;
                    else tu = ( ta - vy ) / tda;
                    if ( vz > tb ) tv = ( vz - tb ) / tdb;
                    else tv = ( tb - vz ) / tdb;
                }
                
            }
	    
            // apply v3d rotation and position modifiers
            if ( modifier.rh ) RotateZ ( &vx, &vy, &vz, &pw, modifier.rh );
            if ( modifier.rp ) RotateX ( &vx, &vy, &vz, &pw, modifier.rp );
            if ( modifier.rb ) RotateY ( &vx, &vy, &vz, &pw, modifier.rb );
            vx += modifier.tx;
            vy += modifier.ty;
            vz += modifier.tz;
	    
	    /* TODO apply user modifiers. Apply order is: X rotation, then Y rotation, then Z rotation, then scale, then translations. */
	    /*
	    if ( userModifier->rx != 0.0 )
	    {
		RotateX ( &vx, &vy, &vz, &pw, userModifier->rx );
	    }
	    if ( userModifier->ry != 0.0 )
	    {
		RotateY ( &vx, &vy, &vz, &pw, userModifier->ry );
	    }
	    if ( userModifier->rz != 0.0 )
	    {
		RotateZ ( &vx, &vy, &vz, &pw, userModifier->rz );
	    }
	    */
	    
            /* re-orient vertex because v3d coords system is different from obj coords system */
	    RotateZ ( &vx, &vy, &vz, &pw, -90 ); // rotate vertex arround Z axis. Angle given in degrees.
	    RotateX ( &vx, &vy, &vz, &pw, 90 ); // rotate vertex arround X axis. Angle given in degrees.
	    
            fprintf( fpOut, "%lf %lf %lf\n", vx, vy, vz ); // write geometric vertex
            if ( textureOrient[0] != '\0' ) { // write texture vertex
                fprintf( fpOut, "vt %lf %lf\n", tu, tv );
            }
        }
        lineNr++; // input file line counter
    }
    //endPosition = ftell(fpIn);
    endBoundString += 4; // remove firsts 4 characters (i.e. "END_")

    // generate and write primitives (points, lines, faces, ...) to *.obj file
    if ( !strcmp(endBoundString, "POINTS" ) ) {
        primitives = v;
        fprintf(fpOut, "p");
        for ( int cnt0 = -primitives; cnt0 < 0; cnt0++ ) {
            fprintf(fpOut, " %d",  cnt0 );
        }
        fprintf(fpOut, "\n" );
    }
    else if ( !strcmp(endBoundString, "LINES" ) ) {
        primitives = v / 2; // number of lines
        int v1 = 0, v2 = 0;
        for ( int cnt0 = -primitives; cnt0 < 0; cnt0++ ) {
            v1 = cnt0 * 2;
            v2 = v1 + 1;
            fprintf(fpOut, "l");
            if ( v && !vt ) {
                fprintf(fpOut, " %d",  v1 );
                fprintf(fpOut, " %d",  v2 );
            }
            else { // ( v && vt )
                fprintf(fpOut, " %d/%d",  v1, v1 );
                fprintf(fpOut, " %d/%d",  v2, v2 );
            }
            fprintf(fpOut, "\n" );
        }
    }
    else if ( !strcmp(endBoundString, "LINE_STRIP" ) ) {
        primitives = v - 1; // number of lines
        fprintf(fpOut, "l");
        for ( int cnt0 = -primitives - 1; cnt0 < 0; cnt0++ ) {
            if ( v && !vt ) {
                fprintf(fpOut, " %d", cnt0 );
            }
            else { // ( v && vt )
                fprintf(fpOut, " %d/%d", cnt0, cnt0 );
            }
        }
        fprintf(fpOut, "\n");
    }
    else if ( !strcmp(endBoundString, "LINE_LOOP" ) ) {
        primitives = v; // number of lines
        fprintf(fpOut, "l");
        for ( int cnt0 = -primitives; cnt0 < 0; cnt0++ ) {
            if ( v && !vt ) {
                fprintf(fpOut, " %d", cnt0 );
            }
            else { // ( v && vt )
                fprintf(fpOut, " %d/%d", cnt0, cnt0 );
            }
        }
        // close loop
        if ( v && !vt ) {
            fprintf(fpOut, " %d", -primitives );
        }
        else { // ( v && vt )
            fprintf(fpOut, " %d/%d", -primitives, -primitives );
        }
        fprintf(fpOut, "\n");
    }
    else if ( !strcmp(endBoundString, "TRIANGLES" ) ) {
        primitives = v / 3; // number of triangles
        for ( int cnt0 = -primitives; cnt0 < 0; cnt0++ ) {
            int offset = cnt0 * 3;
            fprintf(fpOut, "f");
            if ( v && !vt && !vn ) {
                fprintf(fpOut, " %d", offset + 2 );
                fprintf(fpOut, " %d",  offset + 1 );
                fprintf(fpOut, " %d",  offset );
            }
            else if ( v && !vt && vn ) {
                fprintf(fpOut, " %d//%d", offset + 2, offset + 2 );
                fprintf(fpOut, " %d//%d",  offset + 1, offset + 1 );
                fprintf(fpOut, " %d//%d",  offset, offset );
            }
            else if ( v && vt && !vn ) {
                fprintf(fpOut, " %d/%d", offset + 2, offset + 2 );
                fprintf(fpOut, " %d/%d",  offset + 1, offset + 1 );
                fprintf(fpOut, " %d/%d",  offset, offset );
            }
            else { // ( v && vt && vn )
                fprintf(fpOut, " %d/%d/%d", offset + 2, offset + 2, offset + 2 );
                fprintf(fpOut, " %d/%d/%d",  offset + 1, offset + 1, offset + 1 );
                fprintf(fpOut, " %d/%d/%d",  offset, offset, offset );
            }
            fprintf(fpOut, "\n" );
        }
    }
    else if ( !strcmp(endBoundString, "TRIANGLE_STRIP" ) ) {
        /* Example of triangle strip:
            v -11.1 -35.1 5.3
            v -4.9 -21.5 5.3
            v 0.7 -40.4 5.3
            v 26.1 19 5.3
            v 2.6 -36.1 5.3
            v 17.8 -13.6 5.3
            v 15.1 -41.7 5.3
            v 26 -17.3 5.3
            f -6 -7 -8
            f -5 -7 -6
            f -4 -5 -6
            f -3 -5 -4
            f -2 -3 -4
            f -1 -3 -2
        */
        primitives = v - 2; // number of triangles
        // first triangle
        int v1 = - primitives, v2 = v1 - 1, v3 = v2 - 1;
        fprintf(fpOut, "f");
        if ( v && !vt && !vn ) {
            fprintf(fpOut, " %d", v1 );
            fprintf(fpOut, " %d", v2 );
            fprintf(fpOut, " %d", v3 );
        }
        else if ( v && !vt && vn ) {
            fprintf(fpOut, " %d//%d", v1, v1 );
            fprintf(fpOut, " %d//%d", v2, v2 );
            fprintf(fpOut, " %d//%d", v3, v3 );
        }
        else if ( v && vt && !vn ) {
            fprintf(fpOut, " %d/%d", v1, v1 );
            fprintf(fpOut, " %d/%d", v2, v2 );
            fprintf(fpOut, " %d/%d", v3, v3 );
        }
        else { // ( v && vt && vn )
            fprintf(fpOut, " %d/%d/%d", v1, v1, v1 );
            fprintf(fpOut, " %d/%d/%d", v2, v2, v2 );
            fprintf(fpOut, " %d/%d/%d", v3, v3, v3 );
        }
        fprintf(fpOut, "\n");
            
        // next triangles
        for ( int cnt0 = primitives - 1; cnt0 > 0; cnt0 = cnt0 - 2 ) {
            v1 = v1 + 1;
            v3 = v3 + 2;
            fprintf(fpOut, "f");
            if ( v && !vt && !vn ) {
                fprintf(fpOut, " %d", v1 );
                fprintf(fpOut, " %d", v2 );
                fprintf(fpOut, " %d", v3 );
            }
            else if ( v && !vt && vn ) {
                fprintf(fpOut, " %d//%d", v1, v1 );
                fprintf(fpOut, " %d//%d", v2, v2 );
                fprintf(fpOut, " %d//%d", v3, v3 );
            }
            else if ( v && vt && !vn ) {
                fprintf(fpOut, " %d/%d", v1, v1 );
                fprintf(fpOut, " %d/%d", v2, v2 );
                fprintf(fpOut, " %d/%d", v3, v3 );
            }
            else { // ( v && vt && vn )
                fprintf(fpOut, " %d/%d/%d", v1, v1, v1 );
                fprintf(fpOut, " %d/%d/%d", v2, v2, v2 );
                fprintf(fpOut, " %d/%d/%d", v3, v3, v3 );
            }
            fprintf(fpOut, "\n");
            
            v1 = v1 + 1;
            if ( v1 < 0 ) {
                v2 = v2 + 2;
                fprintf(fpOut, "f");
                if ( v && !vt && !vn ) {
                    fprintf(fpOut, " %d", v1 );
                    fprintf(fpOut, " %d", v2 );
                    fprintf(fpOut, " %d", v3 );
                }
                else if ( v && !vt && vn ) {
                    fprintf(fpOut, " %d//%d", v1, v1 );
                    fprintf(fpOut, " %d//%d", v2, v2 );
                    fprintf(fpOut, " %d//%d", v3, v3 );
                }
                else if ( v && vt && !vn ) {
                    fprintf(fpOut, " %d/%d", v1, v1 );
                    fprintf(fpOut, " %d/%d", v2, v2 );
                    fprintf(fpOut, " %d/%d", v3, v3 );
                }
                else { // v && vt && vn )
                    fprintf(fpOut, " %d/%d/%d", v1, v1, v1 );
                    fprintf(fpOut, " %d/%d/%d", v2, v2, v2 );
                    fprintf(fpOut, " %d/%d/%d", v3, v3, v3 );
                }
                fprintf(fpOut, "\n");
            }
        }
    }
    else if ( !strcmp(endBoundString, "TRIANGLE_FAN" ) ) {
        primitives = v - 2; // number of triangles
        for ( int cnt0 = -primitives; cnt0 < 0; cnt0++ ) {
            fprintf(fpOut, "f");
            if ( v && !vt && !vn ) {
                fprintf(fpOut, " %d", -primitives - 2 );
                fprintf(fpOut, " %d", cnt0 );
                fprintf(fpOut, " %d", cnt0 - 1 );
            }
            else if ( v && !vt && vn ) {
                fprintf(fpOut, " %d//%d", -primitives - 2, -primitives - 2 );
                fprintf(fpOut, " %d//%d", cnt0, cnt0 );
                fprintf(fpOut, " %d//%d", cnt0 - 1, cnt0 - 1 );
            }
            else if ( v && vt && !vn ) {
                fprintf(fpOut, " %d/%d", -primitives - 2, -primitives - 2 );
                fprintf(fpOut, " %d/%d", cnt0, cnt0 );
                fprintf(fpOut, " %d/%d", cnt0 - 1, cnt0 - 1 );
            }
            else { // ( v && vt && vn )
                fprintf(fpOut, " %d/%d/%d", -primitives - 2, -primitives - 2, -primitives - 2 );
                fprintf(fpOut, " %d/%d/%d", cnt0, cnt0, cnt0 );
                fprintf(fpOut, " %d/%d/%d", cnt0 - 1, cnt0 - 1, cnt0 - 1 );
            }
            fprintf(fpOut, "\n");
        }
    }
    else if ( !strcmp(endBoundString, "QUADS" ) ) {
        primitives = v / 4; // number of quads
        for ( int cnt0 = -primitives; cnt0 < 0; cnt0++ ) {
            int offset = cnt0 * 4;
            fprintf(fpOut, "f");
            if ( v && !vt && !vn ) {
                fprintf(fpOut, " %d", offset + 3 );
                fprintf(fpOut, " %d", offset + 2 );
                fprintf(fpOut, " %d", offset + 1 );
                fprintf(fpOut, " %d", offset );
            }
            else if ( v && !vt && vn ) {
                fprintf(fpOut, " %d//%d", offset + 3, offset + 3 );
                fprintf(fpOut, " %d//%d", offset + 2, offset + 2 );
                fprintf(fpOut, " %d//%d", offset + 1, offset + 1 );
                fprintf(fpOut, " %d//%d", offset, offset );
            }
            else if ( v && vt && !vn ) {
                fprintf(fpOut, " %d/%d", offset + 3, offset + 3 );
                fprintf(fpOut, " %d/%d", offset + 2, offset + 2 );
                fprintf(fpOut, " %d/%d", offset + 1, offset + 1 );
                fprintf(fpOut, " %d/%d", offset, offset );
            }
            else { // ( v && vt && vn )
                fprintf(fpOut, " %d/%d/%d", offset + 3, offset + 3, offset + 3 );
                fprintf(fpOut, " %d/%d/%d", offset + 2, offset + 2, offset + 2 );
                fprintf(fpOut, " %d/%d/%d", offset + 1, offset + 1, offset + 1 );
                fprintf(fpOut, " %d/%d/%d", offset, offset, offset );
            }
            fprintf(fpOut, "\n");
        }
    }
    else if ( !strcmp(endBoundString, "QUAD_STRIP" ) ) {
        primitives = ( v / 2 ) - 1; // number of quads
        // first quad
        fprintf(fpOut, "f");
        if ( v && !vt && !vn ) {
            fprintf(fpOut, " %d", -primitives * 2 - 2 );
            fprintf(fpOut, " %d", -primitives * 2 - 1 );
            fprintf(fpOut, " %d", -primitives * 2 + 1 );
            fprintf(fpOut, " %d", -primitives * 2 + 0 );
        }
        else if ( v && !vt && vn ) {
            fprintf(fpOut, " %d//%d", -primitives * 2 - 2, -primitives * 2 - 2 );
            fprintf(fpOut, " %d//%d", -primitives * 2 - 1, -primitives * 2 - 1 );
            fprintf(fpOut, " %d//%d", -primitives * 2 + 0, -primitives * 2 + 0 );
            fprintf(fpOut, " %d//%d", -primitives * 2 + 1, -primitives * 2 + 1 );
        }
        else if ( v && vt && !vn ) {
            fprintf(fpOut, " %d/%d", -primitives * 2 - 2, -primitives * 2 - 2 );
            fprintf(fpOut, " %d/%d", -primitives * 2 - 1, -primitives * 2 - 1 );
            fprintf(fpOut, " %d/%d", -primitives * 2 + 0, -primitives * 2 + 0 );
            fprintf(fpOut, " %d/%d", -primitives * 2 + 1, -primitives * 2 + 1 );
        }
        else { // ( v && vt && vn )
            fprintf(fpOut, " %d/%d/%d", -primitives * 2 - 2, -primitives * 2 - 2, -primitives * 2 - 2 );
            fprintf(fpOut, " %d/%d/%d", -primitives * 2 - 1, -primitives * 2 - 1, -primitives * 2 - 1 );
            fprintf(fpOut, " %d/%d/%d", -primitives * 2 + 0, -primitives * 2 + 0, -primitives * 2 + 0 );
            fprintf(fpOut, " %d/%d/%d", -primitives * 2 + 1, -primitives * 2 + 1, -primitives * 2 + 1 );
        }
        fprintf(fpOut, "\n");
        
        // next quads
        for ( int cnt0 = primitives - 1; cnt0 > 0; cnt0-- ) {
            int offset = -cnt0 * 2;
            fprintf(fpOut, "f");
            if ( v && !vt && !vn ) {
                fprintf(fpOut, " %d", offset - 2 );
                fprintf(fpOut, " %d", offset - 1 );
                fprintf(fpOut, " %d", offset + 1 );
                fprintf(fpOut, " %d", offset + 0 );
            }
            else if ( v && !vt && vn ) {
                fprintf(fpOut, " %d//%d", offset - 2, offset - 2 );
                fprintf(fpOut, " %d//%d", offset - 1, offset - 1 );
                fprintf(fpOut, " %d//%d", offset + 0, offset + 0 );
                fprintf(fpOut, " %d//%d", offset + 1, offset + 1 );
            }
            else if ( v && vt && !vn ) {
                fprintf(fpOut, " %d/%d", offset - 2, offset - 2 );
                fprintf(fpOut, " %d/%d", offset - 1, offset - 1 );
                fprintf(fpOut, " %d/%d", offset + 0, offset + 0 );
                fprintf(fpOut, " %d/%d", offset + 1, offset + 1 );
            }
            else { // ( v && vt && vn )
                fprintf(fpOut, " %d/%d/%d", offset - 2, offset - 2, offset - 2 );
                fprintf(fpOut, " %d/%d/%d", offset - 1, offset - 1, offset - 1 );
                fprintf(fpOut, " %d/%d/%d", offset + 0, offset + 0, offset + 0 );
                fprintf(fpOut, " %d/%d/%d", offset + 1, offset + 1, offset + 1 );
            }
            fprintf(fpOut, "\n");
        }
    }
    else if ( !strcmp(endBoundString, "POLYGON" ) ) {
        primitives = v;
        fprintf(fpOut, "f");
        for ( int cnt0 = 1; cnt0 <= primitives; cnt0++ ) {
            int offset = -cnt0;
            if ( v && vt && vn ) {
                fprintf(fpOut, " %d/%d/%d", offset, offset, offset );
            }
            else if ( v && !vt && vn ) {
                fprintf(fpOut, " %d//%d", offset, offset );
            }
            else if ( v && vt && !vn ) {
                fprintf(fpOut, " %d/%d", offset, offset );
            }
            else { // ( v && !vt && !vn )
                fprintf(fpOut, " %d", offset );
            }
        }
        fprintf(fpOut, "\n");
        primitives = 1; // for printf output
    }
    ////////////////////////////////////////////////////printf("%d %s with %d v, %d vt, %d vn\n", primitives, endBoundString, v, vt, vn);
    
    
    
    
    /*
    fprintf( fpOut, "****************\n");
    fseek(fpIn, startPosition, SEEK_SET);
    while ( ftell(fpIn) <= endPosition ) {
        fgets ( buffer, MAX_LENGTH, fpIn );
        fprintf( fpOut, "%s", buffer );
    }
    fprintf( fpOut, "****************\n");
    */
    
    return v;
}

int v3dToObj( const char *source, const char *dest, UserModifier *userModifier ) {
    FILE *fpIn, *fpOut, *fpMtl, *fopen();
    char lineBuffer[MAX_LENGTH + 1], buffer[MAXPATHLENGTH + MAX_LENGTH + 7], tagName[MAX_LENGTH + 1], parameter1[MAX_LENGTH + 1], textureName[MAX_LENGTH + 1], texturePath[MAX_LENGTH + 1];
    char *path = NULL, *baseName = NULL, *extension = NULL; // for output files names
    char textureOrient[MAX_LENGTH + 1];
    char modelType[22 + 1]; // longest model type: 'aileron_elevator_right'
    long fileSize = 0;
    unsigned int points = 0, lines = 0, line_strip = 0, line_loop = 0, triangle = 0, triangle_strip = 0, triangle_fan = 0, quads = 0, quad_strip = 0, polygon = 0, color = 0, texture = 0, objectNr = 0;
    
    modifier.tx = 0;
    modifier.ty = 0;
    modifier.tz = 0;
    modifier.rh = 0;
    modifier.rp = 0;
    modifier.rb = 0;
    
    textureName[0] = '\0'; // no texture at startup
    textureOrient[0] = '\0'; // no texture at startup
    
    fileSize = file_size(source);
    printf("*.3d file size: %ld bytes.\n", fileSize);
    
    if ((fpIn = fopen(source, "r")) == NULL) {
        perror(source);
        return -1;
    }
    
    // Generate output file name
    scanFileName( dest, &path, &baseName, &extension );
    
    if ( path[0] == '\0' ) // if no path
	sprintf( buffer, "%s.obj", baseName );
    else
	sprintf( buffer, "%s/%s.obj", path, baseName );
    
    if ((fpOut = fopen(buffer, "w")) == NULL) {
        perror(buffer);
        return -1;
    }
    
    if ( path[0] == '\0' ) // if no path
	sprintf( buffer, "%s.mtl", baseName );
    else
	sprintf( buffer, "%s/%s.mtl", path, baseName );
    
    if ((fpMtl = fopen(buffer, "w")) == NULL) {
        perror(buffer);
        return -1;
    }
    else {
        fprintf(fpOut, "mtllib ./%s.mtl\n", baseName); // write material file name in fpOut
    }

    /////////////////////////////fprintf(fpOut, "g %s\n", baseName); // create a group and give it 3d model name FIXME: can be an issue if model name contains one ore more spaces!
    
    /*
    // count file lines, then create as many verticies as lines number (not efficient but easier way to do the job...)
    long counter = 0;
    while ( fgets(lineBuffer, MAX_LENGTH, fpIn) ) counter++;
    printf("Nombre de lignes du fichier: %ld.\n", counter);
    struct Vertex {
        long x;
        long y;
        long z;
    };
    struct Vertex vertex[counter];
    rewind(fpIn);
    */
    
    while ( fgets(lineBuffer, MAX_LENGTH, fpIn) ) {
	strcpy( buffer, ""); // clear buffer
        sscanf( lineBuffer, "%s %s", buffer, parameter1 );
        strtoupper( tagName, buffer );
        strtoupper( parameter1, parameter1 );
        if ( tagName[0] == '#' ) { // it's a comment: just copy it
            fprintf(fpOut, "%s", lineBuffer);
        }
        else if ( !strcmp(tagName, "BEGIN_POINTS") ) {
            points++;
            fprintf(fpOut, "#*.3d begin_points\n");
            readV3dVerticies ( fpIn, "END_POINTS", fpOut, "",  userModifier, modifier );
            fprintf(fpOut, "#*.3d end_points\n");
        }
        else if ( !strcmp(tagName, "BEGIN_LINES") ) {
            lines++;
            fprintf(fpOut, "#*.3d begin_lines\n");
            readV3dVerticies ( fpIn, "END_LINES", fpOut, "",  userModifier, modifier );
            fprintf(fpOut, "#*.3d end_lines\n");
        }
        else if ( !strcmp(tagName, "BEGIN_LINE_STRIP") ) {
            line_strip++;
            fprintf(fpOut, "#*.3d begin_line_strip\n");
            readV3dVerticies ( fpIn, "END_LINE_STRIP", fpOut, "",  userModifier, modifier );
            fprintf(fpOut, "#*.3d end_line_strip\n");
        }
        else if ( !strcmp(tagName, "BEGIN_LINE_LOOP") ) {
            line_loop++;
            fprintf(fpOut, "#*.3d begin_line_loop\n");
            readV3dVerticies ( fpIn, "END_LINE_LOOP", fpOut, "",  userModifier, modifier );
            fprintf(fpOut, "#*.3d end_line_loop\n");
        }
        else if ( !strcmp(tagName, "BEGIN_TRIANGLES") ) {
            triangle++;
            fprintf(fpOut, "#*.3d begin_triangles\n");
            readV3dVerticies ( fpIn, "END_TRIANGLES", fpOut, textureOrient,  userModifier, modifier );
            fprintf(fpOut, "#*.3d end_triangles\n");
        }
        else if ( !strcmp(tagName, "BEGIN_TRIANGLE_STRIP") ) {
            triangle_strip++;
            fprintf(fpOut, "#*.3d begin_triangle_strip\n");
            readV3dVerticies ( fpIn, "END_TRIANGLE_STRIP", fpOut, textureOrient,  userModifier, modifier );
            fprintf(fpOut, "#*.3d end_triangle_strip\n");
        }
        else if ( !strcmp(tagName, "BEGIN_TRIANGLE_FAN") ) {
            triangle_fan++;
            fprintf(fpOut, "#*.3d begin_triangle_fan\n");
            readV3dVerticies ( fpIn, "END_TRIANGLE_FAN", fpOut, textureOrient,  userModifier, modifier );
            fprintf(fpOut, "#*.3d end_triangle_fan\n");
        }
        else if ( !strcmp(tagName, "BEGIN_QUADS") ) {
            quads++;
            fprintf(fpOut, "#*.3d begin_quads\n");
            readV3dVerticies ( fpIn, "END_QUADS", fpOut, textureOrient,  userModifier, modifier );
            fprintf(fpOut, "#*.3d end_quads\n");
        }
        else if ( !strcmp(tagName, "BEGIN_QUAD_STRIP") ) {
            quad_strip++;
            fprintf(fpOut, "#*.3d begin_quad_strip\n");
            readV3dVerticies ( fpIn, "END_QUAD_STRIP", fpOut, textureOrient,  userModifier, modifier );
            fprintf(fpOut, "#*.3d end_quad_strip\n");
        }
        else if ( !strcmp(tagName, "BEGIN_POLYGON") ) {
            polygon++;
            fprintf(fpOut, "#*.3d begin_polygon\n");
            readV3dVerticies ( fpIn, "END_POLYGON", fpOut, textureOrient,  userModifier, modifier );
            fprintf(fpOut, "#*.3d end_polygon\n");
        }
        else if ( !strcmp(tagName, "COLOR") ) {
            // create material in *.mtl file
            //char red[20], green[20], blue[20], transparency[20], ambient[20], diffuse[20], specular[20], shininess[20], emission[20];
            double red = 0, green = 0, blue = 0, transparency = 0, ambient = 1, diffuse = 1, specular = 1, shininess = 1, emission = 1;
            
	    // TODO FIXME check if material already exists before creating a new one !!! If not, a VERY big quantity of materials can be generated and Blender don't like it!!!
            fprintf(fpMtl, "newmtl color_%d\n", color);
	    
            if ( sscanf( lineBuffer, " %*s %lf %lf %lf %lf %lf %lf %lf %lf %lf", &red, &green, &blue, &transparency, &ambient, &diffuse, &specular, &shininess, &emission ) ) {
		/*
                fprintf(fpMtl, "Ka %lf %lf %lf\n", red * ambient, green * ambient, blue * ambient);
                fprintf(fpMtl, "Ks %lf %lf %lf\n", red * specular, green * specular, blue * specular);
                */
		fprintf(fpMtl, "Ka 0.200000 0.200000 0.200000\n");
                fprintf(fpMtl, "Ks 1.000000 1.000000 1.000000\n");
		fprintf(fpMtl, "Kd %lf %lf %lf\n", red * diffuse, green * diffuse, blue * diffuse);
                
                // Ka r g b : The Ka statement specifies the ambient reflectivity using RGB values.
                // Kd r g b : The Kd statement specifies the diffuse reflectivity using RGB values.
                // Ks r g b : The Ks statement specifies the specular reflectivity using RGB values.
                // Illumination    Properties that are turned on in the 
                // model           Property Editor
                //  0		Color on and Ambient off
                //  1		Color on and Ambient on
                //  2		Highlight on
                //  3		Reflection on and Ray trace on
                //  4		Transparency: Glass on
                // 		    Reflection: Ray trace on
                //  5		Reflection: Fresnel on and Ray trace on
                //  6		Transparency: Refraction on
                // 		    Reflection: Fresnel off and Ray trace on
                //  7		Transparency: Refraction on
                // 		    Reflection: Fresnel on and Ray trace on
                //  8		Reflection on and Ray trace off
                //  9		Transparency: Glass on
                // 		    Reflection: Ray trace off
                // 10		Casts shadows onto invisible surfaces

                // TODO ? : implement transparency, ambient,  diffuse, specular, shininess and emission in *.mtl file
            }
            else { // create a default color
                fprintf(fpMtl, "Kd 0.5 0.5 0.5\n"); // grey
            }
            fprintf(fpMtl, "illum 1\n");
            
            // specify material in *.obj file
            fprintf(fpOut, "usemtl color_%d\n", color);
            textureName[0] = '\0';
            textureOrient[0] = '\0';
            color++;
        }
        else if ( !strcmp(tagName, "BEGIN_MODEL") && !strcmp(parameter1, "SHADOW") ) {
            // create material in *.mtl file
            fprintf(fpMtl, "newmtl color_%d\n", color);
            // create a default color
            fprintf(fpMtl, "Kd 0.000000 0.000000 0.000000\n");
            fprintf(fpMtl, "illum 0\n");
            // specify material in *.obj file
            fprintf(fpOut, "#*.3d %s", lineBuffer); // print "begin_model shadow" as a comment
            sscanf( lineBuffer, "%*s %s", modelType );
            // We can't use only the model type as object name because a *.3d file can contain more than one "begin_model" statement with the same model type (example: begin_model rotor)
            fprintf( fpOut, "o obj%d_%s\n", objectNr++, modelType );
            fprintf(fpOut, "usemtl color_%d\n", color);
            textureName[0] = '\0';
            textureOrient[0] = '\0';
            color++;
        }
        else if ( !strcmp(tagName, "BEGIN_MODEL") ) { // create an object and give it model type as name
            sscanf( lineBuffer, "%*s %s", modelType );
            // We can't use only the model type as object name because a *.3d file can contain more than one "begin_model" statement with the same model type (example: begin_model rotor)
            fprintf( fpOut, "o obj%d_%s\n", objectNr++, modelType );
        }
        else if ( !strcmp(tagName, "END_MODEL") ) {
            modifier.tx = 0;
            modifier.ty = 0;
            modifier.tz = 0;
            modifier.rh = 0;
            modifier.rp = 0;
            modifier.rb = 0;
            textureName[0] = '\0';
            textureOrient[0] = '\0';
        }
        else if ( !strcmp(tagName, "TRANSLATE") ) {
            sscanf( lineBuffer, "%*s %lf %lf %lf", &modifier.tx, &modifier.ty, &modifier.tz );
        }
        else if ( !strcmp(tagName, "UNTRANSLATE") || !strcmp(tagName, "END_MODEL") ) {
            modifier.tx = 0;
            modifier.ty = 0;
            modifier.tz = 0;
        }
        else if ( !strcmp(tagName, "ROTATE") ) {
            sscanf( lineBuffer, "%*s %lf %lf %lf", &modifier.rh, &modifier.rp, &modifier.rb );
        }
        else if ( !strcmp(tagName, "UNROTATE") || !strcmp(tagName, "END_MODEL") ) {
            modifier.rh = 0;
            modifier.rp = 0;
            modifier.rb = 0;
        }
        else if ( !strcmp(tagName, "TEXTURE_LOAD") ) {
            char priority[20];
            sscanf( lineBuffer, "%*s %s %s %s", textureName, texturePath, priority );
            fprintf(fpMtl, "newmtl %s\n", textureName);
            fprintf(fpMtl, "Kd 1.000000 1.000000 1.000000\n");
	    
	    /* fast but dirty way to modify extension */
	    int extensionPosition = strlen( texturePath ) - 3;
	    texturePath[ extensionPosition++ ] = 't';
	    texturePath[ extensionPosition++ ] = 'g';
	    texturePath[ extensionPosition ] = 'a';
	    
            fprintf(fpMtl, "map_Kd %s\n", texturePath);
            textureName[0] = '\0';
            textureOrient[0] = '\0';
            texture++;
        }
        else if ( !strcmp(tagName, "TEXTURE_SELECT") ) {
            sscanf( lineBuffer, "%*s %s", textureName );
            fprintf( fpOut, "usemtl %s\n", textureName );
            
            textureOrient[0] = '\0';
        }
        else if ( !strcmp(tagName, "TEXTURE_ORIENT_XY") ||  !strcmp(tagName, "TEXTURE_ORIENT_XZ") || !strcmp(tagName, "TEXTURE_ORIENT_YZ")) {
            strcpy( textureOrient, lineBuffer ); // save texture_orient_ type
        }
        else if ( !strcmp(tagName, "TEXTURE_OFF") ) {
            textureName[0] = '\0';
            textureOrient[0] = '\0';
        }
        else {
//printf("lineBuffer='%s'\n", lineBuffer);
            if ( lineBuffer[0] != '\n' ) {
                fprintf(fpOut, "#%ld: %s", lineNr, lineBuffer); // if tag is not recognized, set it as comment and start it with input file line number
            }
            else {
                fprintf(fpOut, "\n");
            }
            //while ( (c = fgetc(fpIn)) != '\n') fprintf(fpOut, "%c", c);
            
        }
        
        lineNr++; // input file line counter
    }
    printf("%s contains:\n %d points, %d lines, %d line_strip, %d line_loop, %d triangle, %d triangle_strip, %d triangle_fan, %d quads, %d quad_strip, %d polygon, and %d color statements.\n", source, points, lines, line_strip, line_loop, triangle, triangle_strip, triangle_fan, quads, quad_strip, polygon, color);
    
    printf("Files %s.obj and %s.mtl written to: %s directory.\n", baseName, baseName, path);
    
    if ( path != NULL )
    {
	free( path );
	path = NULL;
    }
    if ( baseName != NULL )
    {
	free( baseName );
	baseName = NULL;
    }
    if ( extension != NULL )
    {
	free( extension );
	extension = NULL;
    }
    
    fclose(fpMtl);
    fclose(fpOut);
    fclose(fpIn);
    return 0;
}
