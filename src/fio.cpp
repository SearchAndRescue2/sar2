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
#include <ctype.h>
#include <limits.h>

#include "../include/xsw_ctype.h"
#include "../include/os.h"
#include "../include/string.h"
#include "../include/fio.h"

#ifdef MEMWATCH
# include "memwatch.h"
#endif


FILE *FOpen(const char *path, const char *mode);
void FClose(FILE *fp);

void FSeekNextLine(FILE *fp);
void FSeekPastSpaces(FILE *fp);
void FSeekPastChar(FILE *fp, char c);

int FSeekToParm(FILE *fp, const char *parm, char comment, char delim);
char *FSeekNextParm(FILE *fp, char *buf, char comment, char delim);

int FGetValuesI(FILE *fp, int *value, int nvalues);
int FGetValuesL(FILE *fp, long *value, int nvalues);
int FGetValuesF(FILE *fp, double *value, int nvalues);
char *FGetString(FILE *fp);
char *FGetStringLined(FILE *fp);
char *FGetStringLiteral(FILE *fp);

char *FReadNextLineAlloc(FILE *fp, char comment);
char *FReadNextLineAllocCount(
	FILE *fp, char comment, int *line_count
);


#define ISCR(c) (((c) == '\n') || ((c) == '\r'))

/*
 *      Allocate memory while reading a line from file in chunk size
 *      of this many bytes:
 */
#define FREAD_ALLOC_CHUNK_SIZE  8


#define ATOI(s)         (((s) != NULL) ? atoi(s) : 0)
#define ATOL(s)         (((s) != NULL) ? atol(s) : 0)
#define ATOF(s)         (((s) != NULL) ? atof(s) : 0.0f)
#define STRDUP(s)       (((s) != NULL) ? strdup(s) : NULL)

#define MAX(a,b)        (((a) > (b)) ? (a) : (b))
#define MIN(a,b)        (((a) < (b)) ? (a) : (b))
#define CLIP(a,l,h)     (MIN(MAX((a),(l)),(h)))
#define STRLEN(s)       (((s) != NULL) ? strlen(s) : 0)
#define STRISEMPTY(s)   (((s) != NULL) ? (*(s) == '\0') : 1)


/*
 *	OS wrapper to open file using UNIX path notation.
 *
 *	The returned FILE pointer can be used with all the ANSI C standard
 *	file IO functions.
 */
FILE *FOpen(const char *path, const char *mode) 
{
	int len;
	char *new_path, *strptr2;
	const char *strptr1;
	FILE *fp;

	if(STRISEMPTY(path) || STRISEMPTY(mode))
	    return(NULL);

	/* Get length of path and allocate new buffer */
	len = STRLEN(path);
	new_path = (char *)malloc((len + 1) * sizeof(char));
	if(new_path == NULL)
	    return(NULL);

	/* Copy path to new_path */
	strptr1 = path;
	strptr2 = new_path;
	while(*strptr1 != '\0')
	{
	    *strptr2 = *strptr1;

#ifdef __MSW__
	    if(*strptr2 == '/')
		*strptr2 = '\\';
#endif	/* __MSW__ */

	    strptr1++; strptr2++;
	}
	*strptr2 = '\0';

	/* Open file */
	fp = fopen(new_path, mode);

	/* Delete coppied path */
	free(new_path);

	return(fp);
}

/*
 *	Closes the file opened by FOpen().
 */
void FClose(FILE *fp)
{
	if(fp != NULL)
	    fclose(fp);
}

/*
 *	Seeks to next line, escape sequences will be parsed.
 */
void FSeekNextLine(FILE *fp)
{
	int c;

	if(fp == NULL)
	    return;

	do
	{
	    c = fgetc(fp);

	    /* Escape sequence? */
	    if(c == '\\')
		c = fgetc(fp);
	    /* New line? */
	    else if(ISCR(c))
		break;

	} while(c != EOF);
}

/*
 *      Seeks fp past any spaces.
 */
void FSeekPastSpaces(FILE *fp)
{
	int c;

	if(fp == NULL)
	    return;

	while(1)
	{
	    c = fgetc(fp);
	    if(c == EOF)
		break;

	    if(ISBLANK(c))
		continue;

	    fseek(fp, -1, SEEK_CUR);
	    break;
	}
}

/*
 *	Seeks fp past the first occurance of c or EOF.
 */
void FSeekPastChar(FILE *fp, char c)
{
	int i;


	if(fp == NULL)
	    return;

	do
	{
	    i = fgetc(fp);
	    if(i == c)
		break;

	} while(i != EOF);
}

/*
 *	Seeks fp to the beginning of the value of the specified
 *	parameter.
 *
 *	The delim specifies the deliminator character between the
 *	parameter and the value. The delim can be '\0' to specify
 *	"any blanks" (in the case of no deliminators between the
 *	parameter and the value).
 *
 *	Parameters are assumed to not contain spaces or escape sequences.
 *
 *	Returns 0 on successful match or non-zero on error.
 */
int FSeekToParm(FILE *fp, const char *parm, char comment, char delim)
{
	int c, parm_len;

	if(fp == NULL)
	    return(-1);

	parm_len = STRLEN(parm);
	if(parm_len <= 0)
	    return(-1);

	do
	{
	    c = fgetc(fp);
	    if(c == EOF)
		return(-1);

	    /* Seek past spaces */
	    if(ISBLANK(c))
		FSeekPastSpaces(fp);

	    /* New line? */
	    if(ISCR(c))
	        continue;

	    /* First non-blank a comment character? */
	    if(c == comment)
	    {
		FSeekNextLine(fp);
		continue;
	    }

	    /* Matches parameter? */
	    if(c == *parm)
	    {
		/* First char matches parm */
		const char *strptr = parm + 1;


		while(*strptr != '\0')
		{
		    c = fgetc(fp);
		    if(c != *strptr)
			break;

		    strptr++;
		}
		if(*strptr == '\0')
		{
		    /* Got match, seek fp past deliminator */
		    if(delim == '\0')
		    {
			FSeekPastSpaces(fp);
		    }
		    else
		    {
			FSeekPastSpaces(fp);

			/* Seek to delim or newline */
			do
			{
			    c = fgetc(fp);
			    if((c == EOF) || (c == delim))
				break;

			    if(ISCR(c))
			    {
				fseek(fp, -1, SEEK_CUR);
				break;
			    }

			} while(1);

			FSeekPastSpaces(fp);
		    }

		    return(0);
		}
		else
		{
		    /* No match, seek to next line */
		    FSeekNextLine(fp);
		}
	    }
	    else
	    {
		/* No match, seek to next line */
		FSeekNextLine(fp);
	    }

	} while(1);

	return(-1);
}

/*
 *	Fetches the next parameter found at the file position fp.
 *
 *	If buf is NULL then a newly allocated string will be returned
 *	containing the fetched parm. If buf is not NULL, then buf will
 *	be realloc()'ed and returned as a new pointer containing
 *	the fetched parm.
 *
 *	If EOF is reached by the given fp position, then NULL will
 *	be returned and the given buf will have been free()ed by this
 *	function.
 */
char *FSeekNextParm(FILE *fp, char *buf, char comment, char delim)
{
	int c, buf_pos = 0, buf_len, buf_inc = FREAD_ALLOC_CHUNK_SIZE;


	if(fp == NULL)
	{
	    free(buf);
	    return(NULL);
	}

	/* Get length of buf (less than actual allocated is okay) */
	buf_len = (buf != NULL) ? strlen(buf) : 0;

	/* Seek past spaces and comments to next parameter */
	while(1)
	{
	    FSeekPastSpaces(fp);
	    c = fgetc(fp);
	    if(c == EOF)
	    {
		free(buf);
		return(NULL);
	    }
	    else if(c == comment)
	    {
		FSeekNextLine(fp);
		continue;
	    }
	    else if(ISCR(c))
	    {
		continue;
	    }
	    else
	    {
		fseek(fp, -1, SEEK_CUR);
		break;
	    }
	}

	/* Begin fetching this parm */
	while(1)
	{
	    /* Get next char */
	    c = fgetc(fp);
	    if(c == EOF)
	    {
		break;
	    }

	    /* Blank character reached? */
	    if(ISBLANK(c))
	    {
		/* Blank char reached, seek past delimiantor and position
		 * fp at beginning of value.
		 */
		if(delim == '\0')
		{
		    FSeekPastSpaces(fp);
		}
		else
		{
		    FSeekPastSpaces(fp);

		    /* Seek to deim or newline */
		    do
		    {
			c = fgetc(fp);
			if((c == EOF) || (c == delim))
			    break;

			if(ISCR(c))
			{
			    fseek(fp, -1, SEEK_CUR);
			    break;
			}

		    } while(1);

		    FSeekPastSpaces(fp);
		}
		break;
	    }

	    /* CR reached? */
	    if(ISCR(c))
	    {
	    	fseek(fp, -1, SEEK_CUR);
		break;
	    }

	    /* Deliminator reached? */
	    if(c == delim)
	    {
		FSeekPastSpaces(fp);
		break;
	    }

	    /* Need to allocate buffer? */
	    if(buf_pos <= buf_len)
	    {
		buf_len += buf_inc;

		buf = (char *)realloc(buf, buf_len * sizeof(char));
		if(buf == NULL)
		{
		    FSeekNextLine(fp);
		    return(NULL);
		}
	    }

	    buf[buf_pos] = (char)c;
	    buf_pos++;
	}

	/* Put null terminating byte on buffer */
	if(c == EOF)
	{
	    free(buf);
	    buf = NULL;
	}
	else
	{
	    if(buf_pos <= buf_len)
	    {
		buf_len = buf_pos + 1;

		buf = (char *)realloc(buf, buf_len * sizeof(char));
		if(buf == NULL)
		    return(NULL);
	    }
	    buf[buf_pos] = '\0';
	}

	return(buf);
}

/*
 *      Loads values as ints from the file starting at the
 *      specified fp. Will not load more than nvalues.
 *
 *      The fp will be positioned at the start of the next line.
 *
 *      Returns non-zero on error. 
 */
int FGetValuesI(FILE *fp, int *value, int nvalues)
{
	int c, i, n, line_done = 0;
#define len	80
	char num_str[len];


	if(fp == NULL)
	    return(-1);

	FSeekPastSpaces(fp);

	/* Begin fetching values */
	for(i = 0; i < nvalues; i++)
	{
	    (*num_str) = '\0';

	    /* Read number */   
	    for(n = 0; n < len; n++)
	    {
		if(line_done)   
		    break;

		c = fgetc(fp);
		if((c == EOF) || ISCR(c))
		{
		    num_str[n] = '\0';
		    line_done = 1;
		    break;
		}
		/* Escape sequence? */
		else if(c == '\\')
		{
		    c = fgetc(fp);
		    if(c == EOF)
		    {
			num_str[n] = '\0';
			line_done = 1;
			break;
		    }
		    if(c != '\\')
		       c = fgetc(fp);

		    if(c == EOF)
		    {
			num_str[n] = '\0';
			line_done = 1;
			break;
		    }
		}
		/* Separator? */
		else if(ISBLANK(c) || (c == ','))
		{
		    num_str[n] = '\0';
		    FSeekPastSpaces(fp);
		    break;
		}

		num_str[n] = (char)c;
	    }
	    num_str[len - 1] = '\0';

	    value[i] = atoi(num_str);
	}
	if(!line_done)
	    FSeekNextLine(fp);
#undef len
	return(0);
}

/*
 *	Loads values as longs from the file starting at the
 *	specified fp. Will not load more than nvalues.
 *
 *	The fp will be positioned at the start of the next line.
 *
 *	Returns non-zero on error.
 */
int FGetValuesL(FILE *fp, long *value, int nvalues)
{
	int c, i, n, line_done = 0;
#define len     80
	char num_str[len];


	if(fp == NULL)
	    return(-1);

	FSeekPastSpaces(fp);

	/* Begin fetching values */
	for(i = 0; i < nvalues; i++)
	{
	    (*num_str) = '\0';

	    /* Read number */
	    for(n = 0; n < len; n++)
	    {
		if(line_done)
		    break;

		c = fgetc(fp);
		if((c == EOF) || ISCR(c))
		{
		    num_str[n] = '\0';
		    line_done = 1;
		    break;
		}
		/* Escape sequence? */
		else if(c == '\\')
		{
		    c = fgetc(fp);
		    if(c == EOF)
		    {
			num_str[n] = '\0';
			line_done = 1;
			break;
		    }
		    if(c != '\\')
		       c = fgetc(fp);

		    if(c == EOF)
		    {
			num_str[n] = '\0';
			line_done = 1;
			break;
		    }
		}
		/* Separator? */
		else if(ISBLANK(c) || (c == ','))
		{
		    num_str[n] = '\0';
		    FSeekPastSpaces(fp);
		    break;
		}

		num_str[n] = (char)c;
	    }
	    num_str[len - 1] = '\0';

	    value[i] = atol(num_str);
	}
	if(!line_done)
	    FSeekNextLine(fp);
#undef len
	return(0);
}

/*
 *      Loads values as doubles from the file starting at the
 *      specified fp. Will not load more than nvalues.
 *
 *      The fp will be positioned at the start of the next line.
 *
 *	Returns non-zero on error.
 */
int FGetValuesF(FILE *fp, double *value, int nvalues)
{
	int c, i, n, line_done = 0;
#define len	80
	char num_str[len];


	if(fp == NULL)
	    return(-1);

	FSeekPastSpaces(fp);

	/* Begin fetching values */
	for(i = 0; i < nvalues; i++)
	{
	    (*num_str) = '\0';

	    /* Read number */
	    for(n = 0; n < len; n++)
	    {
		if(line_done)
		    break;

		c = fgetc(fp);
		if((c == EOF) || ISCR(c))
		{
		    num_str[n] = '\0';
		    line_done = 1; 
		    break;   
		}
		/* Escape sequence? */
		else if(c == '\\')
		{
		    c = fgetc(fp);
		    if(c == EOF)
		    {
			num_str[n] = '\0';
			line_done = 1;
			break;
		    }
		    if(c != '\\')
		       c = fgetc(fp); 

		    if(c == EOF)
		    {
			num_str[n] = '\0';
			line_done = 1;
			break;  
		    }
		}
		/* Separator? */
		else if(ISBLANK(c) || (c == ','))
		{
		    num_str[n] = '\0';
		    FSeekPastSpaces(fp);
		    break;
		}

		num_str[n] = (char)c;
	    }
	    num_str[len - 1] = '\0';

	    value[i] = atof(num_str);
	}
	if(!line_done)
	    FSeekNextLine(fp);

#undef len
	return(0);
}

/*
 *	Returns a dynamically allocated string containing the value as a
 *	string obtained from the file specified by fp. Reads from the
 *	current position of fp to the next new line character or EOF.
 *
 *      Escape sequences will be parsed and spaces will be stripped.
 *
 *      The fp is positioned after the new line or at the EOF.
 */
char *FGetString(FILE *fp)
{
	int c, i = 0, len = 0;
	char *s = NULL, *s2;


	if(fp == NULL)
	    return(s);

	/* Begin reading string from file */

	/* Skip initial spaces */
	c = fgetc(fp);
	while((c != EOF) && ISBLANK(c))
	    c = fgetc(fp);

	if(c == EOF)
	    return(s);

	/* Read string */
	while(1)
	{
	    /* Need to increase allocation? */
	    if(i >= len)
	    {
		len = MAX(len + 128, i + 1);
		s = (char *)realloc(s, len * sizeof(char));
		if(s == NULL)
		{
		    len = i = 0;
		    break;
		}
	    }

	    /* Get pointer to current position in string */
	    s2 = s + i;

	    /* Set new character value */
	    *s2 = c;

	    /* End of file or end of the line? */
	    if((c == EOF) || ISCR(c))
	    {
		*s2 = '\0';  
		break;
	    }
	    /* Escape sequence? */
	    else if(c == '\\')
	    {
		/* Read next character after backslash */
		c = fgetc(fp);
		if((c == EOF) || (c == '\0'))
		{
		    *s2 = '\0';
		}
		else if(ISCR(c))
		{
		    /* New line (do not save this newline char) */
		    i--;
		}
		else if(c == '\\')
		{
		    /* Literal backslash */
		    *s2 = '\\';
		}
		else if(c == '0')
		{
		    /* Null */
		    *s2 = '\0';
		}
		else if(c == 'b')
		{
		    /* Bell */
		    *s2 = '\b';
		}
		else if(c == 'n')
		{
		    /* New line */
		    *s2 = '\n';
		}
		else if(c == 'r')
		{
		    /* Line return */
		    *s2 = '\r';
		}
		else if(c == 't')
		{
		    /* Tab */
		    *s2 = '\t';
		}
		else
		{
		    /* Unsupported escape sequence, store it as is */
		    *s2 = c;
		}

		/* Read next character and increment position */
		c = fgetc(fp);
		i++;
	    }
	    /* Regular character */
	    else
	    {
		/* Read next character and increment position */
		c = fgetc(fp);
		i++;
	    }
	}

	/* Cut off tailing spaces */
	if(s != NULL)
	{
	    s2 = s + i - 1;

	    while(ISBLANK(*s2) && (s2 >= s))
		*s2 = '\0';
	}

	return(s);
}


/*
 *	Works just like FGetString() except the string is loaded
 *	literally and the only escape sequence to be handled will
 *	be the two characters '\\' '\n', when those characters
 *	are encountered the character '\n' will be saved into the return
 *	string.
 *
 *	Spaces will not be striped, the fp will be positioned after the
 *	newline or EOF (whichever is encountered first).
 */
char *FGetStringLined(FILE *fp)
{
	int c, i = 0, len = 0;
	char *s = NULL, *s2;


	if(fp == NULL)
	    return(s);

	/* Begin reading string from file */

	/* Get first character */
	c = fgetc(fp);
	if(c == EOF)
	    return(s);

	/* Read string */
	while(1)
	{
	    /* Need to increase allocation? */
	    if(i >= len)
	    {
		len = MAX(len + 128, i + 1);
		s = (char *)realloc(s, len * sizeof(char));
		if(s == NULL)
		{
		    len = i = 0;
		    break;
		}
	    }

	    /* Get pointer to current position in string */
	    s2 = s + i;

	    /* Set new character value */
	    *s2 = c;

	    /* End of file or end of line? */
	    if((c == EOF) || ISCR(c))
	    {
		*s2 = '\0';
		break;
	    }
	    /* Escape sequence? */
	    else if(c == '\\')
	    {
		/* Read next character after backslash */
		c = fgetc(fp);
		if(c == EOF)
		{
		    i++;
		    continue;
		}
		else if(ISCR(c))
		{
		    /* New line, store it as is */
		    *s2 = c;
		}
		else
		{
		    /* All other escaped characters leave as is
		     * it will be set on the next loop
		     */
		    i++;
		    continue;
		}

		/* Read next character and increment position */
		c = fgetc(fp);
		i++;
	    }
	    /* Regular character */
	    else
	    {
		/* Read next character and increment position */
		c = fgetc(fp);
		i++;
	    }
	}   

	return(s);
}

/*
 *      Works just like FGetString() except the string is loaded
 *	literally and no escape sequences parsed, that would be all
 *	characters from the current given fp position to the first
 *	encountered newline ('\n') character (escaped or not).
 *
 *	Spaces will not be striped, the fp will be positioned after the
 *	newline or EOF (whichever is encountered first).
 */
char *FGetStringLiteral(FILE *fp)  
{
	int c, i = 0, len = 0;
	char *s = NULL, *s2;


	if(fp == NULL)
	    return(s);

	/* Begin reading string from file */

	/* Get first character */
	c = fgetc(fp);
	if(c == EOF)
	    return(s);

	/* Read string */
	while(1)
	{
	    /* Need to increase allocation? */
	    if(i >= len)
	    {
		len = MAX(len + 128, i + 1);
		s = (char *)realloc(s, len * sizeof(char));
		if(s == NULL)
		{
		    len = i = 0;
		    break;
		}
	    }

	    /* Get pointer to current position in string */
	    s2 = s + i;

	    /* Set new character value */
	    *s2 = c;

	    /* End of file or end of line? */
	    if((c == EOF) || ISCR(c))
	    {
		*s2 = '\0';
		break;
	    }
	    else
	    {
		/* Read next character and increment position */
		c = fgetc(fp);   
		i++;
	    }
	}

	return(s);
}


/*
 *	Returns an allocated string containing the entire
 *	line or NULL on error or EOF.
 *
 *	If comment is '\0' then the next line is read regardless
 *	if it is a comment or not.
 *
 *	Calling function must free() the returned pointer.
 */
char *FReadNextLineAlloc(FILE *fp, char comment)
{
	return(FReadNextLineAllocCount(fp, comment, NULL));
}

char *FReadNextLineAllocCount(
	FILE *fp,
	char comment,
	int *line_count
)
{
	int i, m, n;
	char *strptr;


	if(fp == NULL)
	    return(NULL);

	/* Is comment character specified? */
	if(comment != '\0')
	{
	    /* Comment character is specified */

	    /* Read past spaces, newlines, and comments */
	    i = fgetc(fp);
	    if(i == EOF)
		return(NULL);

	    while((i == ' ') || (i == '\t') || (i == '\n') || (i == '\r') ||
		  (i == comment)
	    )
	    {
		if(i == EOF)
		    return(NULL);

		/* If newline, then increment line count */
		if((i == '\n') ||
		   (i == '\r')
		)
		{
		    if(line_count != NULL)
			*line_count += 1;
		}

		/* If comment, then skip to next line */
		if(i == comment)
		{
		    i = fgetc(fp);
		    while((i != '\n') && (i != '\r'))
		    {
			if(i == EOF)
			    return(NULL);
			i = fgetc(fp);
		    }
		    if(line_count != NULL)
			*line_count += 1;
		}

		/* Get next character */
		i = fgetc(fp);
	    }

	    /* Begin adding characters to string */
	    m = 0;	/* mem size */
	    n = 1;	/* chars read */
	    strptr = NULL;

	    while((i != '\n') && (i != '\r') && (i != '\0'))
	    {
		/* Escape character? */
		if(i == '\\')
		{
		    /* Read next character */
		    i = fgetc(fp);

		    /* Skip newlines internally */
		    if((i == '\n') || (i == '\r'))
		    {
			i = fgetc(fp);

			/* Still counts as a line though! */
			if(line_count != NULL)
			    *line_count += 1;
		    }
		}

		if(i == EOF)
		    break;

		/* Allocate more memory as needed */
		if(m < n)
		{
		    /* Allocate FREAD_ALLOC_CHUNK_SIZE more bytes */
		    m += FREAD_ALLOC_CHUNK_SIZE;

		    strptr = (char *)realloc(strptr, m * sizeof(char));
		    if(strptr == NULL)
			return(NULL);
		}

		strptr[n - 1] = (char)i;

		/* Read next character from file */
		i = fgetc(fp);
		n++;	/* Increment characters read */
	    }

	    /* Add newline and null terminate */
	    m += 2;	/* 2 more chars */
	    strptr = (char *)realloc(strptr, m * sizeof(char));
	    if(strptr == NULL)
		return(NULL);
	    strptr[n - 1] = '\n';
	    strptr[n] = '\0';

	    /* Increment line count */
	    if(line_count != NULL)
		*line_count += 1;
	}
	else
	{
	    /* Comment character is not specified */

	    i = fgetc(fp);
	    if(i == EOF)
		return(NULL);

	    /* Begin adding characters to string */
	    m = 0;      /* Memory size */
	    n = 1;      /* Characters read */
	    strptr = NULL;	/* Return string */

	    while((i != '\n') && (i != '\r') && (i != '\0'))
	    {
		/* Escape character? */
		if(i == '\\')
		{
		    /* Read next character */
		    i = fgetc(fp);

		    /* Skip newlines internally */
		    if((i == '\n') || (i == '\r'))
		    {
			i = fgetc(fp);

			/* Still counts as a line though! */
			if(line_count != NULL)
			    *line_count += 1;
		    }
		}

		if(i == EOF)
		    break;

		/* Allocate more memory as needed */
		if(m < n)
		{
		    /* Allocate FREAD_ALLOC_CHUNK_SIZE more bytes */
		    m += FREAD_ALLOC_CHUNK_SIZE;

		    strptr = (char *)realloc(strptr, m * sizeof(char));
		    if(strptr == NULL)
			return(NULL);
		}
	
		strptr[n - 1] = (char)i;

		/* Read next character from file */
		i = fgetc(fp);
		n++;	/* Increment characters read */
	    }

	    /* Add newline and null terminate */
	    m += 2;	/* 2 more chars */
	    strptr = (char *)realloc(strptr, m * sizeof(char));
	    strptr[n - 1] = '\n';
	    strptr[n] = '\0';

	    /* Increment line count */
	    if(line_count != NULL)
		*line_count += 1;
	}


	return(strptr);
}
