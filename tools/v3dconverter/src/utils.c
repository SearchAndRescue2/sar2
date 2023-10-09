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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <sys/stat.h>
#include <math.h>
#include <time.h>


char* extOf( char *bname) { // returns extension of base name
    char *ext;
    ext = strrchr (bname, '.');
    if (ext == NULL) ext = ""; // fast method, could also use &(path[strlen(path)]).
    return ext;
}

char * strtoupper( char * dest, const char * src ) {
    char * result = dest;
    while( (*dest++ = toupper( *src++ )) );
    return result;
}

char * strtolower( char * dest, const char * src ) {
    char * result = dest;
    while( (*dest++ = tolower( *src++ )) );
    return result;
}

long file_size(const char *filename)
{
    struct stat s;
    if  (stat(filename, &s) != 0) return 0;
    else return s.st_size;
}


/* Parse a full file and fill path, baseName, and extention strings.
 * 
 * Examples:
 * 
 * fullFileName = "./a/such/full/name.txt" returns:
 * path = "./a/such/full/"
 * baseName = "name"
 * extension = ".txt"
 * 
 * fullFileName = "a/such/path/" returns:
 * path = "a/such/path/"
 * baseName = ""
 * extension = ""
 * 
 */
void scanFileName( const char *fullFileName, char **path, char **baseName, char **extension )
{
    int cnt0 = 0, fullFileNameLength = 0, lastSlashPos = 0, fullPathLength = 0, lastDotPos = 0;
    
    if ( *path != NULL )
    {
	free( *path );
	*path = NULL;
    }
    if ( *baseName != NULL )
    {
	free( *baseName );
	*baseName = NULL;
    }
    if ( *extension != NULL )
    {
	free( *extension );
	*extension = NULL;
    }
    
    /* full file name length */
    fullFileNameLength = strlen( fullFileName );

    /* last (back)slash position */
    for ( lastSlashPos = fullFileNameLength; lastSlashPos >= 0; lastSlashPos-- )
    {
	if ( ( *( fullFileName + lastSlashPos ) == '/' ) || ( *( fullFileName + lastSlashPos ) == '\\' ) )
	    break;
    }
    
    /*  path length */
    fullPathLength = lastSlashPos + 1;
    
    /*  last dot position */
    for ( lastDotPos = fullFileNameLength; lastDotPos > fullPathLength; lastDotPos-- )
    {
	if ( *( fullFileName + lastDotPos ) == '.' )
	    break;
    }
    
    /* extract base name, path, and extension */
    if ( lastSlashPos < 0 && lastDotPos == 0) // full file name has NO path and NO extension
    {
	/* extract baseName */
	*baseName = malloc ( ( fullFileNameLength + 1 ) * sizeof (char) );
	if ( *baseName == NULL )
	{
	    fprintf(stderr, "Memory allocation error.\n");
	    exit (EXIT_FAILURE);
	}
	for ( cnt0 = 0; cnt0 < fullFileNameLength; cnt0++ )
	    *( *baseName + cnt0 ) = *( fullFileName + cnt0 );
	*( *baseName + cnt0 ) = '\0'; // close string
	
	/* no path */
	*path = malloc ( sizeof (char) );
	if ( *path == NULL )
	{
	    fprintf(stderr, "Memory allocation error.\n");
	    exit (EXIT_FAILURE);
	}
	**path = '\0'; // close string
	
	/* no extension */
	*extension = malloc ( sizeof (char) );
	if ( *extension == NULL )
	{
	    fprintf(stderr, "Memory allocation error.\n");
	    exit (EXIT_FAILURE);
	}
	**extension = '\0'; // close string
	
    }
    else if ( lastDotPos == fullPathLength ) // full file name has NO extension
    {
	/* extract baseName */
	*baseName = malloc ( ( fullFileNameLength - fullPathLength + 1 ) * sizeof (char) );
	if ( *baseName == NULL )
	{
	    fprintf(stderr, "Memory allocation error.\n");
	    exit (EXIT_FAILURE);
	}
	
	for ( cnt0 = fullPathLength; cnt0 < fullFileNameLength; cnt0++ )
	{
	    *( *baseName - fullPathLength + cnt0 ) = *( fullFileName + cnt0 );
	}
	*( *baseName - fullPathLength + cnt0 ) = '\0'; // close string
	
	/* extract path */
	*path = malloc ( ( lastSlashPos + 2 ) * sizeof (char) );
	if ( *path == NULL )
	{
	    fprintf(stderr, "Memory allocation error.\n");
	    exit (EXIT_FAILURE);
	}
	for ( cnt0 = 0; cnt0 <= lastSlashPos; cnt0++ )
	{
	    *( *path + cnt0 ) = *( fullFileName + cnt0 );
	}
	*( *path + cnt0 ) = '\0'; // close string
	
	/* no extension */
	*extension = malloc ( sizeof (char) );
	if ( *extension == NULL )
	{
	    fprintf(stderr, "Memory allocation error.\n");
	    exit (EXIT_FAILURE);
	}
	**extension = '\0'; // close string
    }
    else // full file name has an extension
    {
	/* extract baseName */
	*baseName = malloc ( ( lastDotPos - fullPathLength + 1 ) * sizeof (char) );
	if ( *baseName == NULL )
	{
	    fprintf(stderr, "Memory allocation error.\n");
	    exit (EXIT_FAILURE);
	}
	for ( cnt0 = fullPathLength; cnt0 < lastDotPos; cnt0++ )
	    *( *baseName - fullPathLength + cnt0 ) = *( fullFileName + cnt0 );
	*( *baseName - fullPathLength + cnt0 ) = '\0'; // close string
	
	/* extract path */
	*path = malloc ( ( lastSlashPos + 2 ) * sizeof (char) );
	if ( *path == NULL )
	{
	    fprintf(stderr, "Memory allocation error.\n");
	    exit (EXIT_FAILURE);
	}
	for ( cnt0 = 0; cnt0 <= lastSlashPos; cnt0++ )
	{
	    *( *path + cnt0 ) = *( fullFileName + cnt0 );
	}
	*( *path + cnt0 ) = '\0'; // close string
	
	/* extract extension */
	*extension = malloc ( ( fullFileNameLength - lastDotPos + 1 ) * sizeof (char) );
	if ( *extension == NULL )
	{
	    fprintf(stderr, "Memory allocation error.\n");
	    exit (EXIT_FAILURE);
	}
	for ( cnt0 = lastDotPos; cnt0 < fullFileNameLength; cnt0++ )
	{
	    *( *extension - lastDotPos + cnt0 ) = *( fullFileName + cnt0 );
	}
	*( *extension - lastDotPos + cnt0 ) = '\0'; // close string
    }
}


int calculateSurfaceNormal ( Vcoord *v, int verticies, Vnormal *vn )
{
    // Newell's method, from: https://www.khronos.org/opengl/wiki/Calculating_a_Surface_Normal pseudo-code
    // Returns EXIT_FAILURE (0) in case of div by zero error
    
    Vcoord vCurrent, vNext;
    int cnt1 = 0;

    // init normal
    vn->i = 0;
    vn->j = 0;
    vn->k = 0;
    
    for ( int cnt0 = 0; cnt0 < verticies; cnt0++ )
    {
	vCurrent.x = v[ cnt0 ].x;
	vCurrent.y = v[ cnt0 ].y;
	vCurrent.z = v[ cnt0 ].z;
	
	if ( ( cnt0 + 1 ) >= verticies )
	    cnt1 = 0;
	else
	    cnt1 = cnt0 + 1;
	
	vNext.x = v[ cnt1 ].x;
	vNext.y = v[ cnt1 ].y;
	vNext.z = v[ cnt1 ].z;
	
	vn->i += ( vCurrent.y + vNext.y ) * ( vCurrent.z - vNext.z );
	vn->j += ( vCurrent.z + vNext.z ) * ( vCurrent.x - vNext.x );
	vn->k += ( vCurrent.x + vNext.x ) * ( vCurrent.y - vNext.y );
    }
    
    // normalize
    double vecLength = sqrt( vn->i * vn->i + vn->j * vn->j + vn->k * vn->k );
    if ( vecLength == 0 )
	return EXIT_FAILURE; // div by zero error
    else
    {
	vn->i = vn->i / vecLength;
	vn->j = vn->j / vecLength;
	vn->k = vn->k / vecLength;
	return EXIT_SUCCESS;
    }
}

#include <time.h>
char *timeStamp() // returns a YYYYMMDDHHMMSS timestamp
{
  int h, min, s, day, month, year;
  time_t now;
  char *timeStampString;

  // Current time
  time(&now);
  // Convert to local time
  struct tm *local = localtime(&now);
  h = local->tm_hour;        
  min = local->tm_min;       
  s = local->tm_sec;       
  day = local->tm_mday;          
  month = local->tm_mon + 1;     
  year = local->tm_year + 1900;  

  timeStampString = strdup("YYYYMMDDHHMMSS");
  sprintf(timeStampString, "%02d%02d%d%02d%02d%02d", year, month, day, h, min, s);

  return(timeStampString);
}

#include <stdbool.h>
bool isTex ( const char *fullFileName ) // returns 'true' if file baseName contains '.tex'
{
    char *path = NULL, *baseName = NULL, *extension = NULL;
    
    scanFileName( fullFileName, &path, &baseName, &extension );
    
    if ( strstr( baseName, ".tex" ) )
	return true;
    else
	return false;
}

#include <stdbool.h>
bool isHf ( const char *fullFileName ) // returns 'true' if file baseName contains '.hf'
{
    char *path = NULL, *baseName = NULL, *extension = NULL;
    
    scanFileName( fullFileName, &path, &baseName, &extension );
    
    if ( strstr( baseName, ".hf" ) )
	return true;
    else
	return false;
}
