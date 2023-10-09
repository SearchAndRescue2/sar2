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

#define AC3DMAXLEVELS 20
#define AC3DMAXTEXTURES 10
#define AC3DMAXMATERIALS 50
#define AC3DMAXLIGHTS 20

// per object verticies
#define AC3DMAXVERTICIES 10000

int ac3dToV3d(const char *source, const char *dest, UserModifier *userModifier) {
    FILE *fpIn, *fpOut, *fopen();
    char lineBuffer[MAX_LENGTH + 1], buffer[MAXPATHLENGTH + MAX_LENGTH + 7], token[MAX_LENGTH + 1];
    char textureName[MAX_LENGTH + 1];
    char *path = NULL, *baseName = NULL, *extension = NULL; // for output files names
    unsigned int lines = 0, triangle = 0, quads = 0, polygon = 0, textures = 0, lights = 0;
    unsigned int ac3dVersion = 0;
    int level = -1, activeMaterial = -1;
    unsigned int polys = 0, groups = 0, verticies = 0, surfaces = 0, materials = 0;
    char * surfType;
    float Xmin = 99999.99, Ymin = 99999.99, Zmin = 99999.99, Xmax = -99999.99, Ymax = -99999.99, Zmax = -99999.99;

    struct LevelDatas {
        unsigned int kids; // remaining kids for this object
        char type[5+1]; // world, poly, group, light
        struct Modifier modifier; // translate x, y, z, rotate h, p, b.
    };
/*
    struct Vertex {
        double x;
        double y;
        double z;
    };
*/
    struct MatColor {
        float red;
        float green;
        float blue;
    };

    struct Material { // ac3d material (color)
        struct MatColor rgb; //  diffuse, from 0.0 to 1.0
        struct MatColor amb; //  ambient, from 0.0 to 1.0
        struct MatColor emis; // emissive, from 0.0 to 1.0
        struct MatColor spec; // specular, from 0.0 to 1.0
        struct MatColor shi; //  shininess, from 0 to 255 ?
        float transp; //         transparency, from 0.0 to 1.0. A fully transparent object has a transparency value of 100% (100% means 0% in v3d format).
    };
    
    struct Vcoord vertex[AC3DMAXVERTICIES];
    
    struct LevelDatas levelDatas[AC3DMAXLEVELS];
    
    struct Material material[AC3DMAXMATERIALS];
    
    modifier.tx = 0;
    modifier.ty = 0;
    modifier.tz = 0;
    modifier.rh = 0;
    modifier.rp = 0;
    modifier.rb = 0;
    
    if ((fpIn = fopen(source, "r")) == NULL) {
        perror(source);
        return -1;
    }
    fgets(lineBuffer, MAX_LENGTH, fpIn);
    sscanf( lineBuffer, "%4c%x", buffer, &ac3dVersion );
    if ( !strcmp(buffer, "AC3D") ) {
        printf(" *.ac file version: %d ('%s%x')\n", ac3dVersion, buffer, ac3dVersion );
    }
    else {
        fprintf( stderr, "%s is not an ac3d file.\n", source );
        fclose(fpIn);
        return -1;
    }

    // Generate output file name
    scanFileName( dest, &path, &baseName, &extension );
    sprintf( buffer, "%s%s.3d", path, baseName );
    if ((fpOut = fopen(buffer, "w")) == NULL) {
        perror(buffer);
        return(-1);
    }

    fprintf(fpOut, "begin_header\n");
    // look for textures in whole AC3D file because vertex3d needs them in file header
    char * texturesList[AC3DMAXTEXTURES] = { NULL };
    float texPriority = 0.7; // default texture priority
    lineNr = 1; // input file line counter
    while ( fgets(lineBuffer, MAX_LENGTH, fpIn) ) {
        sscanf( lineBuffer, "%s", token );
        if ( !strcmp(token, "texture") ) {
            sscanf( lineBuffer, "%*s \"%[^\"]s", textureName );
	    scanFileName( textureName, &path, &baseName, &extension );
            short registered = 0;
            for ( int counter = 0; counter < textures; counter++) {
                if ( !strcmp( baseName, texturesList[counter] ) ) { // if texture found in textures list
                    registered = 1; // mark it at registered
                    break;
                }
            }
            if ( !registered ) { // it's a new texture, write it in *.3d file
                fprintf(fpOut, "texture_load %s textures/%s.tga %f\n", baseName, baseName, texPriority); // WARNING : extension is hard coded
                texturesList[textures] = strdup(baseName);
                textures++;
            }
        }
        lineNr++;
    }
    rewind(fpIn);
    lineNr = 1; // input file line counter
    
    fprintf(fpOut, "end_header\n\n");
    fprintf(fpOut, "type 1\n");
    fprintf(fpOut, "range 2000\n");
    fprintf(fpOut, "crash_flags 0 1 0 3\n\n");

    while ( fgets(lineBuffer, MAX_LENGTH, fpIn) ) {
        sscanf( lineBuffer, "%s", token );

        if ( !strcmp(token, "OBJECT") ) {
            sscanf( lineBuffer, "%*s %s", token );
            if ( ! ( !strcmp(token, "world") || !strcmp(token, "poly") || !strcmp(token, "group") || !strcmp(token, "light") ) ) { //
                fprintf( stderr, "Input error: unknown OBJECT type (\"%s\") at line #%ld.\n", token, lineNr );
                fclose(fpIn);
                fclose(fpOut);
                return -1;
            }
            
            if ( level < AC3DMAXLEVELS - 1 ) {
                level++;
                strcpy ( levelDatas[level].type, token );
            }
            else {
                fprintf( stderr, "ac3dtov3d error: too much levels in %s file. See #define AC3DMAXLEVELS %d\n", source, AC3DMAXLEVELS);
                return -1;
            }
            
            levelDatas[level].modifier.tx = 0;
            levelDatas[level].modifier.ty = 0;
            levelDatas[level].modifier.tz = 0;
            
            if ( !strcmp(token, "world") ) {
                // we don't care of world datas, read file until world "kids" line
                while ( strcmp(token, "kids") ) { // current token value is "OBJECT"...
                    if ( fgets(lineBuffer, MAX_LENGTH, fpIn) ) {
                        lineNr++; // input file line counter
                        sscanf( lineBuffer, "%s", token );
                    }
                }
                sscanf( lineBuffer, "%*s %u", &levelDatas[level].kids );
            }
            else if ( !strcmp(token, "poly") ) {
                if ( polys == 0) fprintf(fpOut, "begin_model standard\n");
                polys++;
                if ( levelDatas[level-1].kids > 0 ) {
                    levelDatas[level-1].kids--;
                }
            }
            else if ( !strcmp(token, "group") ) {
                groups++;
                if ( levelDatas[level-1].kids > 0 ) {
                    levelDatas[level-1].kids--;
                }
            }
            else if ( !strcmp(token, "light") ) {
                
                
                
                
                
                double xx = 0.0, yy = 0.0, zz = 0.0;
                while ( strcmp(token, "loc") ) { // current token value is "OBJECT"...
                    if ( fgets(lineBuffer, MAX_LENGTH, fpIn) ) {
                        lineNr++; // input file line counter
                        sscanf( lineBuffer, "%s", token );
                    }
                }
                sscanf( lineBuffer, "%*s %lf %lf %lf", &xx, &yy, &zz );
                
                // apply scale
                xx *= userModifier->scale;
                yy *= userModifier->scale;
                zz *= userModifier->scale;

                // add current level offsets
                //xx += levelDatas[level].modifier.tx;
                //yy += levelDatas[level].modifier.ty;
                //zz += levelDatas[level].modifier.tz;
                
                // add lowers levels offsets
                for ( int counter1 = level; counter1 > 0; counter1--) {
//fprintf( stderr, "levelDatas[%d].modifier.translate = %lf %lf %lf\n", counter1 - 1, levelDatas[counter1 - 1].modifier.tx, levelDatas[counter1 - 1].modifier.ty, levelDatas[counter1 - 1].modifier.tz );
                    if ( !strcmp( levelDatas[counter1 - 1].type, "group" ) ) {
                        xx += levelDatas[counter1 - 1].modifier.tx;
                        yy += levelDatas[counter1 - 1].modifier.ty;
                        zz += levelDatas[counter1 - 1].modifier.tz;
                    }
                }
                
                
                
                
                
                fprintf( stdout, "Light #%d detected at line %ld. Position: %lf %lf %lf\n", lights, lineNr, xx, yy, zz );
                
                lights++;
                if ( levelDatas[level-1].kids > 0 ) {
                    levelDatas[level-1].kids--;
                }
            }
            else {
                ;
            }
        }
        else if ( !strcmp(token, "kids") ) {
            sscanf( lineBuffer, "%*s %d", &levelDatas[level].kids );

            while ( levelDatas[level].kids == 0 ) { // clear level offsets values for this level and lower ones
                levelDatas[level].modifier.tx = 0;
                levelDatas[level].modifier.ty = 0;
                levelDatas[level].modifier.tz = 0;
                if ( level >= 0 ) {
                    level--;
                }
                else {
                    break;
                }
            }
        }
        else if ( !strcmp(token, "numvert") ) {
            sscanf( lineBuffer, "%*s %d", &verticies );
            if ( verticies > AC3DMAXVERTICIES - 1 ) {
                fprintf( stderr, "ac3dtov3d error: too much verticies in %s file. See #define AC3DMAXVERTICIES %d\n", source, AC3DMAXVERTICIES);
                return -1;
            }
            
            for ( unsigned int counter = 0; counter < verticies; counter++ ) {
                fgets(lineBuffer, MAX_LENGTH, fpIn);
                lineNr++; // input file line counter
                sscanf( lineBuffer, "%lf %lf %lf", &vertex[counter].x, &vertex[counter].y, &vertex[counter].z );
                
                // apply scale
                vertex[counter].x *= userModifier->scale;
                vertex[counter].y *= userModifier->scale;
                vertex[counter].z *= userModifier->scale;

                // add current level offsets
                vertex[counter].x += levelDatas[level].modifier.tx;
                vertex[counter].y += levelDatas[level].modifier.ty;
                vertex[counter].z += levelDatas[level].modifier.tz;
                
				// add lowers levels offsets
                for ( int counter1 = level; counter1 >= 0; counter1--) {
                    if ( !strcmp( levelDatas[counter1 - 1].type, "group" ) ) {
                        vertex[counter].x += levelDatas[counter1 - 1].modifier.tx;
                        vertex[counter].y += levelDatas[counter1 - 1].modifier.ty;
                        vertex[counter].z += levelDatas[counter1 - 1].modifier.tz;
                    }
                }
                
                // apply rotations (ac3d to v3d orientation conversion)
                double pw = 0;
                RotateX ( &vertex[counter].x, &vertex[counter].y, &vertex[counter].z, &pw, -90 );
                RotateZ ( &vertex[counter].x, &vertex[counter].y, &vertex[counter].z, &pw, 90 );
            }
        }
        else if ( !strcmp(token, "numsurf") ) {
            unsigned int refs = 0; // number of refs
            sscanf( lineBuffer, "%*s %d", &surfaces );
            
            
            while ( surfaces ) {
                while ( 1 ) { // read until 'refs'
                    fgets(lineBuffer, MAX_LENGTH, fpIn);
                    lineNr++; // input file line counter
                    sscanf( lineBuffer, "%s", token );
                    if ( !strcmp(token, "refs") ) {
                        sscanf( lineBuffer, "%*s %d", &refs ); // number of refs
                        break;
                    }
                    else if ( !strcmp(token, "SURF") ) {
                        ;
                    }
                    else if ( !strcmp(token, "mat") ) {
                        sscanf( lineBuffer, "%*s %d", &activeMaterial ); // material number
                    }
                }

                struct Ref {
                    unsigned int index;
                    float u;
                    float v;
                };
                struct Ref ref[refs];
                
                switch (refs) { // refs is the number of surface verticies
                    case 0:
                    case 1:
                        surfType = "";
                        break;
                    case 2:
                        surfType = "line";
                        lines++;
                        break;
                    case 3:
                        surfType = "triangles";
                        triangle++;
                        break;
                    case 4:
                        surfType = "quads";
                        quads++;
                        break;
                    default:
                        surfType = "polygon";
                        polygon++;
                        break;
                }
                
                if ( activeMaterial >= 0 ) {
                    fprintf(fpOut, "color %f %f %f %f\n", material[activeMaterial].rgb.red, material[activeMaterial].rgb.green, material[activeMaterial].rgb.blue, 1.0 - material[activeMaterial].transp );
                }
                    
                if ( strcmp( surfType, "" ) ) {
                    fprintf(fpOut, "begin_%s\n", surfType);

                    long counter = 0;
                    for ( counter = 0; counter < refs; counter++ ) {
                        fgets(lineBuffer, MAX_LENGTH, fpIn);
                        lineNr++; // input file line counter
                        sscanf( lineBuffer, "%d %f %f", &ref[counter].index, &ref[counter].u, &ref[counter].v );
                    }
                    
                    // it it's a surface, then calculate surface normal and write it
		    if ( refs >= 3 )
		    {
			Vnormal vnTemp;
			calculateSurfaceNormal( &vertex[ref[0].index], (int) refs, &vnTemp );
			fprintf( fpOut, " normal %f %f %f\n", vnTemp.i, vnTemp.j, vnTemp.k );
		    }
		    
                    if ( userModifier->invertFaces == 1 ) { // invert faces
                        for ( counter = 0; counter < refs; counter++ ) {
                            fprintf(fpOut, " texcoord %f %f\n", ref[counter].u, ref[counter].v );
                            fprintf(fpOut, " %f %f %f\n", vertex[ref[counter].index].x, vertex[ref[counter].index].y, vertex[ref[counter].index].z );
                            
                            // for object size calculation
                            if ( vertex[ref[counter].index].x < Xmin ) Xmin = vertex[ref[counter].index].x;
                            else if ( vertex[ref[counter].index].x > Xmax ) Xmax = vertex[ref[counter].index].x;
                            if ( vertex[ref[counter].index].y < Ymin ) Ymin = vertex[ref[counter].index].y;
                            else if ( vertex[ref[counter].index].y > Ymax ) Ymax = vertex[ref[counter].index].y;
                            if ( vertex[ref[counter].index].z < Zmin ) Zmin = vertex[ref[counter].index].z;
                            else if ( vertex[ref[counter].index].z > Zmax ) Zmax = vertex[ref[counter].index].z;
                        }
                    }
                    else { // don't invert faces
                        for ( counter = refs - 1; counter >= 0; counter-- ) {
                            fprintf(fpOut, " texcoord %f %f\n", ref[counter].u, ref[counter].v );
                            fprintf(fpOut, " %f %f %f\n", vertex[ref[counter].index].x, vertex[ref[counter].index].y, vertex[ref[counter].index].z );
                            
                            // for object size calculation
                            if ( vertex[ref[counter].index].x < Xmin ) Xmin = vertex[ref[counter].index].x;
                            else if ( vertex[ref[counter].index].x > Xmax ) Xmax = vertex[ref[counter].index].x;
                            if ( vertex[ref[counter].index].y < Ymin ) Ymin = vertex[ref[counter].index].y;
                            else if ( vertex[ref[counter].index].y > Ymax ) Ymax = vertex[ref[counter].index].y;
                            if ( vertex[ref[counter].index].z < Zmin ) Zmin = vertex[ref[counter].index].z;
                            else if ( vertex[ref[counter].index].z > Zmax ) Zmax = vertex[ref[counter].index].z;
                        } 
                    }
                    fprintf(fpOut, "end_%s\n", surfType);
                }
                surfaces--;
		
            }
        }
        else if ( !strcmp(token, "texture") ) {
            sscanf( lineBuffer, "%*s \"%s\"\"", textureName );
	    scanFileName( textureName, &path, &baseName, &extension );
            fprintf(fpOut, "texture_select %s\n", baseName);
        }
        else if ( !strcmp(token, "MATERIAL") ) {
            sscanf( lineBuffer, "MATERIAL %*s rgb %f %f %f amb %*f %*f %*f emis %*f %*f %*f spec %*f %*f %*f shi %*f %*f %*f trans %f", &material[materials].rgb.red, &material[materials].rgb.green, &material[materials].rgb.blue, &material[materials].transp );
            
            if ( material[materials].rgb.red == 0 && material[materials].rgb.green == 0 && material[materials].rgb.blue == 0 ) {
                material[materials].rgb.red = 0.004;
                material[materials].rgb.green = 0.004;
                material[materials].rgb.blue = 0.004;
            }
            
            if ( materials < AC3DMAXMATERIALS - 1 ) {
                materials++;
            }
            else {
                fprintf( stderr, "ac3dtov3d.c error: too much materials in %s file. See #define AC3DMAXMATERIALS %d\n", source, AC3DMAXMATERIALS);
                return -1;
            }
        }
        else if ( !strcmp(token, "loc") ) {
            sscanf( lineBuffer, "%*s %lf %lf %lf", &levelDatas[level].modifier.tx, &levelDatas[level].modifier.ty, &levelDatas[level].modifier.tz );

            // apply scale
            levelDatas[level].modifier.tx *= userModifier->scale;
            levelDatas[level].modifier.ty *= userModifier->scale;
            levelDatas[level].modifier.tz *= userModifier->scale;
            
            // sar2 issue ??? "translate" command seems to have no effet, thus we add "loc" values to verticies (see "numvert")
            //fprintf(fpOut, "translate %f %f %f\n", levelDatas[level].modifier.tx, levelDatas[level].modifier.ty, levelDatas[level].modifier.tz );
        }
        else if ( !strcmp(token, "name") ) {
            sscanf( lineBuffer, "%*s \"%s", token );
            token[strlen(token) - 1] = '\0'; // discard last double quote
            fprintf(fpOut, "# name: %s\n", token);
        }
        else {
            if ( lineBuffer[0] != '\n' ) {
                fprintf(fpOut, "#%ld: %s", lineNr, lineBuffer); // if tag is not recognized, set it as comment and start it with input file line number
            }
            else {
                fprintf(fpOut, "\n");
            }
        }
        
        lineNr++; // input file line counter
    }
    fprintf(fpOut, "end_model standard\n");

    // print some usefull information on console
    fprintf( stdout, "Object X, Y and Z dimensions [m]: %.3lf %.3lf %.3lf\n", Xmax - Xmin, Ymax - Ymin, Zmax - Zmin );
    fprintf( stdout, "Rectangular Xmin Xmax Ymin Ymax Zmin Zmax bounds [m]: %.3lf %.3lf %.3lf %.3lf %.3lf %.3lf\n", Xmin, Xmax, Ymin, Ymax, Zmin, Zmax );
    
    for ( int cnt = 0; cnt < AC3DMAXTEXTURES; cnt++ )
    {
	free( texturesList[cnt] );
    }

    
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
    
    fclose(fpOut);
    fclose(fpIn);
    return 0;
}
