#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "../include/string.h"
#include "../include/strexp.h"
#include "../include/fio.h"

#include "v3dtex.h"
#include "v3dmh.h"
#include "v3dmp.h"
#include "v3dmodel.h"
#include "v3dfio.h"

#ifdef MEMWATCH
# include "memwatch.h"
#endif


static char *STRSEEKBLANK(char *s);

static char *V3DLoadEscapeNewLines(const char *line);
static void V3DAddLine(char ***line, int *total, const char *new_line);
static char *V3DLoadGetParmF(FILE *fp);
static char *V3DLoadGetParmD(const char *line);

static int V3DHandleComment(
	const char *s,
	void *hi, int hi_num,			/* Header item. */
	v3d_model_struct *model, int model_num,	/* Model. */
	void *p, int p_num			/* Model primitive. */
);

int V3DLoadModel(
	const char **buf, FILE *fp,
	void ***mh_item, int *total_mh_items,   
	v3d_model_struct ***model, int *total_models,
	void *client_data,
	int (*progress_cb)(void *, int, int)
);
int V3DSaveModel(
	char ***buf, FILE *fp,
	void **mh_item, int total_mh_items,
	v3d_model_struct **model, int total_models,
	int optimization_level,         /* 0 to V3D_SAVE_FILE_OPTIMIZATION_MAX. */
	int strip_extras,		/* 1 for true, strips extranous data. */
	void *client_data,
	int (*progress_cb)(void *, int, int)
);


#ifndef PI
# define PI	3.14159265359
#endif

#define ATOI(s)         (((s) != NULL) ? atoi(s) : 0)
#define ATOL(s)         (((s) != NULL) ? atol(s) : 0)
#define ATOF(s)         (((s) != NULL) ? atof(s) : 0.0f)
#define STRDUP(s)       (((s) != NULL) ? strdup(s) : NULL)

#define MAX(a,b)        (((a) > (b)) ? (a) : (b))
#define MIN(a,b)        (((a) < (b)) ? (a) : (b))
#define CLIP(a,l,h)     (MIN(MAX((a),(l)),(h)))
#define STRLEN(s)       (((s) != NULL) ? ((int)strlen(s)) : 0)

#define DEGTORAD(d)     ((d) * PI / 180)   
#define RADTODEG(r)     ((r) * 180 / PI)

#define ISCR(c)         (((c) == '\n') || ((c) == '\r'))
#define ISCOMMENT(c)    ((c) == (V3D_COMMENT_CHAR))


/*
 *	Seeks s to the next blank character.
 */
static char *STRSEEKBLANK(char *s)
{
	if(s == NULL)
	    return(s);

	while((*s != '\0') && !ISBLANK(*s))
	    s++;

	return(s);
}


/*
 *	Returns a dynamically allocated string containing the
 *	same input line except with all occurances of '\n' changed
 *	into '\\' '\n'.
 */
static char *V3DLoadEscapeNewLines(const char *line)
{
	int pos_i = 0, pos_o = 0;
	int line_o_len = 0;
	char *line_o = NULL;


	if(line == NULL)
	    return(line_o);

	while(line[pos_i] != '\0')
	{
	    /* Allocate more of line_o as needed. */
	    if(pos_o >= (line_o_len - 2))
	    {
		line_o_len = (pos_o + 82);
		line_o = (char *)realloc(
		    line_o,
		    line_o_len * sizeof(char)
		);
		if(line_o == NULL)
		{
		    line_o_len = 0;
		    break;
		}
	    }

	    if(line[pos_i] == '\n')
	    {
		line_o[pos_o] = '\\';
		line_o[pos_o + 1] = '\n';
		pos_o += 2;
		pos_i += 1;
	    }
	    else if(line[pos_i] == '\r')
	    {
		line_o[pos_o] = '\\';
		line_o[pos_o + 1] = '\r'; 
		pos_o += 2;
		pos_i += 1;
	    }
	    else
	    {
		line_o[pos_o] = line[pos_i];
		pos_o += 1;
		pos_i += 1;
	    }
	}

	/* Null terminate line_o. */
	if(pos_o >= line_o_len)
	{
	    line_o_len = pos_o + 1;
	    line_o = (char *)realloc(
		line_o,
		line_o_len * sizeof(char)
	    );
	}
	if(line_o != NULL)
	{
	    line_o[pos_o] = '\0';
	}

	return(line_o);
}

/*
 *	Adds a new line to the given pointer array.
 */
static void V3DAddLine(char ***line, int *total, const char *new_line)
{
	int n;

	if((line == NULL) ||
	   (total == NULL) ||
	   (new_line == NULL)
	)
	    return;

	if((*total) < 0)
	    (*total) = 0;

	n = (*total);
	(*total) = n + 1;

	(*line) = (char **)realloc(
	    *line,
	    (*total) * sizeof(char *)
	);
	if((*line) == NULL)
	{
	    (*total) = 0;
	    return;
	}
	else
	{
	    (*line)[n] = STRDUP(new_line);
	}

	return;
}


/*
 *      Return a statically allocated string indicating the
 *      operation fetched from the pointed to fp.  Return can be
 *      an empty string if there was no op string to be found or
 *      error.
 *
 *      fp is positioned at the end of the op string which the next
 *      fetch can be used to get its argument. If a new line character
 *      is detected at the end of string on file, then the fp will be
 *      repositioned at that new line character. The next reading of fp
 *      will read that new line character.
 */
static char *V3DLoadGetParmF(FILE *fp)
{
	int c, i;
#define len     80
	static char rtn_str[len];


	*rtn_str = '\0';  
	if(fp == NULL)
	    return(rtn_str);

	/* Seek past spaces. */      
	FSeekPastSpaces(fp);   

	for(i = 0; i < len; i++)
	{
	    c = fgetc(fp);
	    if((c == EOF) ||
	       ISBLANK(c)
	    )
	    {
		rtn_str[i] = '\0';
		break;
	    }
	    /* Escape sequence? */
	    else if(c == '\\')
	    {
		c = fgetc(fp);
		if(c == EOF)
		{
		    rtn_str[i] = '\0';
		    break;
		}

		if(c != '\\')
		    c = fgetc(fp);

		if(c == EOF)
		{
		    rtn_str[i] = '\0';
		    break;
		}
	    }
	    /* New line? */
	    else if(ISCR(c))
	    {
		/* New line right at the end of the op string, seek
		 * back one character.
		 */
		fseek(fp, -1, SEEK_CUR);

		rtn_str[i] = '\0';
		break;
	    }

	    rtn_str[i] = (char)c;
	}

#undef len
	return(rtn_str);
}


/*
 *	Same as V3DLoadGetParmF() except it reads from the given line.
 */
static char *V3DLoadGetParmD(const char *line)
{
	int c, i;
#define len     80
	static char rtn_str[len];


	*rtn_str = '\0';  
	if(line == NULL)
	    return(rtn_str);

	while(ISBLANK(*line))
	    line++;

	for(i = 0; i < len; i++)
	{
	    c = (*line); line++;
	    if((c == '\0') ||
	       ISBLANK(c)
	    )
	    {
		rtn_str[i] = '\0';
		break;
	    }
	    /* Escape sequence? */
	    else if(c == '\\')
	    {
		c = (*line); line++;
		if(c == '\0')
		{
		    rtn_str[i] = '\0';
		    break;
		}

		if(c != '\\')
		{
		    c = (*line); line++;
		}
		if(c == '\0')
		{
		    rtn_str[i] = '\0';
		    break;
		}
	    }
	    /* New line? */
	    else if(ISCR(c))
	    {
		rtn_str[i] = '\0';
		break;
	    }

	    rtn_str[i] = (char)c;
	}

#undef len
	return(rtn_str);
}


/*
 *      Handles comment string s, with respect to the given inputs.
 *      This is to handle embeded special token parameters within comments
 *	that only V3DSaveModel() writes.
 *
 *      If the comment is not handled (ie it is a regular comment)
 *      then 0 will be returned, otherwise non-zero.
 *
 *      Input s assumed valid.
 */
static int V3DHandleComment(
	const char *s,
	void *hi, int hi_num,			/* Header item. */
	v3d_model_struct *model, int model_num,	/* Model. */
	void *p, int p_num			/* Model primitive. */
)
{
	char *strptr, **strv;
	int i, strc;

	if(s == NULL)
	    return(0);

	/* Seek past spaces. */
	while(ISBLANK(*s))
	    s++;

	/* First non-blank a comment? */
	if(ISCOMMENT(*s))
	    s++;
	else
	    return(0);

	/* Its a comment, now seek past more spaces if any. */
	while(ISBLANK(*s))
	    s++;

	/* Is a special token? */
	if((*s) == '$')
	    s++;
	else
	    return(0);

	/* Pointer s now positioned to parse special commented token
	 * parameter.
	 */
	/* Model flags. */
	if(strcasepfx(s, "model_flags"))
	{
	    /* Is a model given? */
	    if(model != NULL)
	    {
		/* Seek to start of first argument. */
		while(!ISBLANK(*s) &&
		      !((*s) == '\0')
		)
		    s++;
		while(ISBLANK(*s))
		    s++;

		/* Parse arguments. */
		strv = strexp(s, &strc);

		/* Go through each argument. */
		for(i = 0; i < strc; i++)
		{
		    strptr = strv[i];
		    if(strptr == NULL)
			continue;

		    if(!strcasecmp(strptr, "hide"))
			model->flags |= V3D_MODEL_FLAG_HIDE;
		}

		/* Free parsed argument strings. */
		strlistfree(strv, strc);
	    }
	}
	else 
	{
	    /* Some other special token this program doesn't support,
	     * so just skip it and return 0 that way it looks like we
	     * never parsed it.
	     */
	    return(0);
	}

	return(1);
}


/*
 *	Loads the contents from either the buffer array or file pointer
 *	into the given model and texture lists.
 *
 *	The given model lists should be NULL and 0 before
 *	passing to this function otherwise items will be appended.
 *
 *	The returned loaded models and model primitives will not be
 *	`realized', it is up to the calling function to do that.
 *
 *	If fp is not NULL then data will be read from the file at whatever
 *	position fp is at when passed.
 *	If buf is not NULL then each line from buf will be
 *	read and handled until a NULL line is reached.
 */
int V3DLoadModel(
	const char **buf, FILE *fp,
	void ***mh_item, int *total_mh_items,
	v3d_model_struct ***model, int *total_models,
	void *client_data,
	int (*progress_cb)(void *, int, int)
)
{
	int c, i;

	const char *buf_line;
	int lines_read = 0, file_size = 0;
	char *strptr;
	int reading_header = 0;
	v3d_model_struct *model_std = NULL, *model_oth = NULL;
	int models_created = 0;

	int vertex_count = 0;
	mp_vertex_struct *v;

	mp_comment_struct *mp_comment = NULL;
	mp_translate_struct *mp_translate = NULL;
	mp_untranslate_struct *mp_untranslate = NULL;
	mp_rotate_struct *mp_rotate = NULL;
	mp_unrotate_struct *mp_unrotate = NULL;
	mp_point_struct *mp_point = NULL;
	mp_line_struct *mp_line = NULL;
	mp_line_strip_struct *mp_line_strip = NULL;
	mp_line_loop_struct *mp_line_loop = NULL;
	mp_triangle_struct *mp_triangle = NULL;
	mp_triangle_strip_struct *mp_triangle_strip = NULL;
	mp_triangle_fan_struct *mp_triangle_fan = NULL;
	mp_quad_struct *mp_quad = NULL;
	mp_quad_strip_struct *mp_quad_strip = NULL;
	mp_polygon_struct *mp_polygon = NULL;
	mp_color_struct *mp_color = NULL;
	mp_texture_select_struct *mp_texture_select = NULL;
	mp_texture_orient_xy_struct *mp_texture_xy = NULL;
	mp_texture_orient_yz_struct *mp_texture_yz = NULL;
	mp_texture_orient_xz_struct *mp_texture_xz = NULL;
	mp_texture_off_struct *mp_texture_off = NULL;
	mp_heightfield_load_struct *mp_heightfield_load = NULL;

/*
 *	Resets all primitive pointes to NULL and resets
 *	vertex_count to 0.
 */
#define DO_END_PRIMITIVES	\
{ \
 vertex_count = 0; \
\
 mp_comment = NULL; \
\
 mp_translate = NULL; \
 mp_untranslate = NULL; \
 mp_rotate = NULL; \
 mp_unrotate = NULL; \
\
 mp_point = NULL; \
 mp_line = NULL; \
 mp_line_strip = NULL; \
 mp_line_loop = NULL; \
 mp_triangle = NULL; \
 mp_triangle_strip = NULL; \
 mp_triangle_fan = NULL; \
 mp_quad = NULL; \
 mp_quad_strip = NULL; \
 mp_polygon = NULL; \
\
 mp_color = NULL; \
 mp_texture_select = NULL; \
 mp_texture_xy = NULL; \
 mp_texture_yz = NULL; \
 mp_texture_xz = NULL; \
 mp_texture_off = NULL; \
 mp_heightfield_load = NULL; \
}

/* Returns true if no primitives with vertex are in context. */
#define IS_NO_VERTEX_PRIMITIVES_ACTIVE		\
((mp_point == NULL) && \
 (mp_line == NULL) && \
 (mp_line_strip == NULL) && \
 (mp_line_loop == NULL) && \
 (mp_triangle == NULL) && \
 (mp_triangle_strip == NULL) && \
 (mp_triangle_fan == NULL) && \
 (mp_quad == NULL) && \
 (mp_quad_strip == NULL) && \
 (mp_polygon == NULL) \
)

/* Returns true if no infinate vertex primitives are in context. */
#define IS_NO_INFINATE_PRIMITIVES_ACTIVE	\
((mp_line_strip == NULL) && \
 (mp_line_loop == NULL) && \
 (mp_triangle_strip == NULL) && \
 (mp_triangle_fan == NULL) && \
 (mp_quad_strip == NULL) && \
 (mp_polygon == NULL) \
)

/* Seeks fp or buf_line to the next line. */
#define DO_SEEK_NEXT_LINE	\
{ \
 if(fp != NULL) \
 { \
  FSeekNextLine(fp); \
 } \
 else if(buf_line != NULL) \
 { \
  buf_line = (*buf); buf++; \
 } \
}


	if((fp == NULL) && (buf == NULL))
	    return(-1);

	if((fp != NULL) &&
	   (buf != NULL)
	)
	{
	    fprintf(
		stderr,
		"V3DLoadModel(): Internal error, both fp and buf not NULL.\n"
	    );
	    return(-1);
	}

	/* Pointers to header items and models must be valid. */
	if((mh_item == NULL) || (total_mh_items == NULL) ||
	   (model == NULL) || (total_models == NULL)
	)
	    return(-1);

	/* Get file size if reading from file. */
	if(fp != NULL)
	{
	    struct stat stat_buf;
	    if(!fstat(fileno(fp), &stat_buf))
		file_size = (int)stat_buf.st_size;
	}

	/* If reading from buffered lines, set up buf_line to
	 * point to first line and increment buf.
	 */
	if(buf != NULL)
	{
	    /* Note, if buf_line is not NULL, then *(buf - 1) is
	     * assumed to be valid.
	     */
	    buf_line = (*buf); buf++;
	}
	else
	{
	    buf_line = NULL;
	}

	/* Being loading. */
	while(1)
	{
	    /* Update progress? */
	    if(progress_cb != NULL)
	    {
		if(fp != NULL)
		    progress_cb(
			client_data,
			(int)ftell(fp),
			file_size
		    );
		else
		    progress_cb(
			client_data,
			0,
			file_size
		    );
	    }

	    /* Loading standard model? */
	    if(model_std != NULL)
	    {
		/* Loading a standard model. */

		if(fp != NULL)
		{
		    c = fgetc(fp);
		}
		else if((buf != NULL) && (buf_line != NULL))
		{
		    c = (*buf_line); buf_line++;
		    if(c == '\0')
		    {
			c = '\n';
			buf_line = (*buf); buf++;
		    }
		}
		else
		{
		    c = EOF;
		}
		if(c == EOF)
		    break;

		/* Read past leading spaces. */
		if(ISBLANK(c))
		{
		    if(fp != NULL)
		    {
			FSeekPastSpaces(fp);
		    }
		    else if(buf_line != NULL)
		    {
			while(ISBLANK(*buf_line))
			    buf_line++;
		    }

		    /* Get first non blank character. */
		    if(fp != NULL)
		    {
			c = fgetc(fp);
		    }
		    else if((buf != NULL) && (buf_line != NULL))
		    {
			c = (*buf_line); buf_line++;
			if(c == '\0')
			{
			    c = '\n';
			    buf_line = (*buf); buf++;
			}
		    }
		    else
		    {
			c = EOF;
		    }
		    if(c == EOF)
			break;
		}
		lines_read++;

		/* Newline? */
		if(ISCR(c))
	        {
		    /* Record this as a comment with just a newline.
		     * Note: This means that empty lines may not occure
		     * between vertexes except at vertex limits for the
		     * primitive in question.
		     *
		     * Empty lines and comments for infinate vertex primitives
		     * will not be saved since doing so will disrupt their
		     * continuity.
		     */
		    if(IS_NO_VERTEX_PRIMITIVES_ACTIVE)
		    {
			if(mp_comment == NULL)
			{
			    /* Previously no comment. */
			    DO_END_PRIMITIVES
			    mp_comment = V3DMPListInsert(
			        &model_std->primitive,
				&model_std->total_primitives,
				-1,			/* Append. */
				V3DMP_TYPE_COMMENT
			    );
			}
			else
			{
			    /* Previously was comment, leave it alone
			     * so comments get concatinated.
			     */
			}

			if(mp_comment != NULL)
		        {
			    V3DAddLine(
				&(mp_comment->line),
				&(mp_comment->total_lines),
				""
			    );
		        }
		    }
		    continue;
		}	/* New line? */

		/* Rest of loading standard model goes on farther
		 * below.
		 */
	    }
	    else if(reading_header)
	    {
		/* Loading header. */

		/* Get string pointer to operation string. */
		if(fp != NULL)
		{
		    strptr = V3DLoadGetParmF(fp);
		}
		else if(buf_line != NULL)
		{
		    char *strptr2;

		    while(ISBLANK(*buf_line))
			buf_line++;

		    strptr = V3DLoadGetParmD(buf_line);
		    strptr2 = STRSEEKBLANK((char *)buf_line);
		    if(strptr2 != NULL)
		    {
			while(ISBLANK(*strptr2))
			    strptr2++;
			buf_line = strptr2;
		    }
		    else
		    {
			buf_line = (*buf);
		    }
		}
		else
		{
		    strptr = NULL;
		}
		if(strptr == NULL)     
		    break;


		/* Version. */
		if(!strcasecmp(strptr, "version"))
		{
		    double value[2];

		    mh_version_struct *mh_version = V3DMHListInsert(
			mh_item, total_mh_items, -1,
			V3DMH_TYPE_VERSION
		    );
		    if(mh_version != NULL)
		    {
			if(fp != NULL) 
			{       
			    fseek(fp, -1, SEEK_CUR);
			    FGetValuesF(fp, value, 2);
			}
			else if(buf_line != NULL)
			{
			    int sr, sc, st = 2;

			    sr = sscanf(buf_line, "%lf %lf",
				&value[0], &value[1]
			    );
			    for(sc = sr; sc < st; sc++)
				value[sc] = 0.0;

			    buf_line = (*buf); buf++;
			}

			mh_version->major = (int)value[0];
			mh_version->minor = (int)value[1];
		    }
		    else
		    {
			DO_SEEK_NEXT_LINE
		    }
		}
		/* Creator. */
		else if(!strcasecmp(strptr, "creator"))
		{
		    char *strptr2 = NULL;
		    
		    mh_creator_struct *mh_creator = V3DMHListInsert(
			mh_item, total_mh_items, -1,
			V3DMH_TYPE_CREATOR
		    );
		    if(mh_creator != NULL)
		    {
			if(fp != NULL)
			{
			    strptr2 = FGetStringLined(fp);
			}
			else if(buf_line != NULL)
			{
			    strptr2 = STRDUP(buf_line);
			    buf_line = (*buf); buf++;  
			}

			mh_creator->creator = strptr2;
			strptr2 = NULL;
		    }
		    free(strptr2);
		}
		/* Author. */
		else if(!strcasecmp(strptr, "author"))
		{
		    char *strptr2 = NULL;

		    mh_author_struct *mh_author = V3DMHListInsert(
			mh_item, total_mh_items, -1,
			V3DMH_TYPE_AUTHOR
		    );  
		    if(mh_author != NULL)
		    {
			if(fp != NULL)
			{
			    strptr2 = FGetStringLined(fp);
			}
			else if(buf_line != NULL)
			{
			    strptr2 = STRDUP(buf_line);
			    buf_line = (*buf); buf++;
			}

			mh_author->author = strptr2;
			strptr2 = NULL;
		    } 
		    free(strptr2);
		}

		/* Heightfield base directory. */
		else if(!strcasecmp(strptr, "heightfield_base_directory") ||
			!strcasecmp(strptr, "heightfield_base_dir")
		)
		{
		    char *strptr2 = NULL;
		    char **strv; int strc; 

		    mh_heightfield_base_directory_struct *mh_hbd = V3DMHListInsert(
			mh_item, total_mh_items, -1,
			V3DMH_TYPE_HEIGHTFIELD_BASE_DIRECTORY
		    );
		    if(mh_hbd != NULL)
		    {
			if(fp != NULL)
			{
			    strptr2 = FGetStringLined(fp);
			}
			else if(buf_line != NULL)
			{
			    strptr2 = STRDUP(buf_line);
			    buf_line = (*buf); buf++;
			}

			if(strptr2 != NULL)
			{
			    strv = strexp(strptr2, &strc);
		      
			    if(strc > 0)
				mh_hbd->path = STRDUP(strv[0]);

			    strlistfree(strv, strc);
			}
		    }
		    free(strptr2);
		}
		/* Texture base directory. */
		else if(!strcasecmp(strptr, "texture_base_directory") ||
			!strcasecmp(strptr, "texture_base_dir")
		)
		{
		    char *strptr2 = NULL;
		    char **strv; int strc;

		    mh_texture_base_directory_struct *mh_tbd = V3DMHListInsert(
			mh_item, total_mh_items, -1,
			V3DMH_TYPE_TEXTURE_BASE_DIRECTORY
		    );
		    if(mh_tbd != NULL)
		    {
			if(fp != NULL)
			{
			    strptr2 = FGetStringLined(fp);
			}
			else if(buf_line != NULL)
			{
			    strptr2 = STRDUP(buf_line);
			    buf_line = (*buf); buf++;
			}

			if(strptr2 != NULL)
			{
			    strv = strexp(strptr2, &strc);

			    if(strc > 0)
				mh_tbd->path = STRDUP(strv[0]);

			    strlistfree(strv, strc);
			}
		    }
		    free(strptr2);
		}
		/* Texture load. */   
		else if(!strcasecmp(strptr, "texture_load"))
		{
		    char *strptr2 = NULL;
		    char **strv; int strc;

		    mh_texture_load_struct *mh_texture_load = V3DMHListInsert(
			mh_item, total_mh_items, -1,
			V3DMH_TYPE_TEXTURE_LOAD
		    );
		    if(mh_texture_load != NULL)
		    {
			if(fp != NULL)
			{
			    strptr2 = FGetStringLined(fp);
			}
			else if(buf_line != NULL)
			{
			    strptr2 = STRDUP(buf_line);
			    buf_line = (*buf); buf++;
			}
			if(strptr2 != NULL)
			{
			    strv = strexp(strptr2, &strc);

			    if(strc > 0)
				mh_texture_load->name = STRDUP(strv[0]);
			    if(strc > 1)
				mh_texture_load->path = STRDUP(strv[1]);

			    if(strc > 2)
				mh_texture_load->priority = ATOF(strv[2]);
			    else
				mh_texture_load->priority = 0.5f;

			    strlistfree(strv, strc);
			}
		    }
		    free(strptr2);
		}
		/* Color specification. */
		else if(!strcasecmp(strptr, "color"))
		{
		    char *strptr2 = NULL;
		    char **strv; int strc;

		    mh_color_specification_struct *mh_color_spec = V3DMHListInsert(
			mh_item, total_mh_items, -1,
			V3DMH_TYPE_COLOR_SPECIFICATION
		    );
		    if(mh_color_spec != NULL)
		    {
			if(fp != NULL)
			{
			    strptr2 = FGetStringLined(fp);
			}
			else if(buf_line != NULL)
			{
			    strptr2 = STRDUP(buf_line);
			    buf_line = (*buf); buf++;
			}
			if(strptr2 != NULL)
			{
			    strv = strexp(strptr2, &strc);

			    if(strc > 0)
			    {
				free(mh_color_spec->name);
				mh_color_spec->name = STRDUP(strv[0]);
			    }

			    if(strc > 1)
				mh_color_spec->r = ATOF(strv[1]);
			    else
				mh_color_spec->r = 1.0f;

			    if(strc > 2)
				mh_color_spec->g = ATOF(strv[2]);
			    else
				mh_color_spec->g = 1.0f;

			    if(strc > 3)
				mh_color_spec->b = ATOF(strv[3]);
			    else
				mh_color_spec->b = 1.0f;

			    if(strc > 4)
				mh_color_spec->a = ATOF(strv[4]);
			    else
				mh_color_spec->a = 1.0f;

			    if(strc > 5)
				mh_color_spec->ambient = ATOF(strv[5]);
			    else
				mh_color_spec->ambient = 0.0f;

			    if(strc > 6)
				mh_color_spec->diffuse = ATOF(strv[6]);
			    else
				mh_color_spec->diffuse = 0.0f;

			    if(strc > 7)
				mh_color_spec->specular = ATOF(strv[7]);
			    else
				mh_color_spec->specular = 0.0f;

			    if(strc > 8)
				mh_color_spec->shininess = ATOF(strv[8]);
			    else
				mh_color_spec->shininess = 0.0f;

			    if(strc > 9)
				mh_color_spec->emission = ATOF(strv[9]);
			    else
				mh_color_spec->emission = 0.0f;

			    strlistfree(strv, strc);
			}
		    }
		    free(strptr2);
		}


		/* End header. */
		else if(!strcasecmp(strptr, "end_header"))
		{
		    DO_SEEK_NEXT_LINE
		    reading_header = 0;
		}
		else
		{
		    /* Skip unknown. */
		    DO_SEEK_NEXT_LINE
		}

		/* Do not continue with parsing of data while loading of
		 * header items.
		 */
		continue;
	    }
	    else
	    {
		/* Loading other data block. */
		char *strptr2 = NULL;

		/* Get strptr2 as the new line (coppied). */
		if(fp != NULL)
		{
		    strptr2 = FGetStringLined(fp);
		}
		else if(buf_line != NULL)
		{
		    strptr2 = STRDUP(buf_line);
		    buf_line = (*buf); buf++;
		}

		if(strptr2 == NULL)
		{
		    /* End of file. */
		    break;
		}
		else
		{
		    /* Check if this line is prefixed with begin_model. */
		    char *strptr3 = strptr2;

		    /* Position strptr3 for parm checking. */
		    while(ISBLANK(*strptr3))
			strptr3++;

		    /* Header? */
		    if(strcasepfx(strptr3, "begin_header"))
		    {
			/* This line starts the loading of the header,
			 * so end loading other model types.
			 */
			model_oth = NULL;
			model_std = NULL;

			/* Mark that we are loading header. */
			reading_header = 1;
		    }
		    /* Begin standard model? */
		    else if(strcasepfx(strptr3, "begin_model"))
		    {
			/* This line starts the loading of a standard
			 * model, so end loading header and other model
			 * types.
			 */
			reading_header = 0;
			model_oth = NULL;


			/* Seek strptr3 past "begin_model" parm. */
			while(!ISBLANK(*strptr3))
			    strptr3++;

			/* Seek past any spaces between parm and arg. */
			while(ISBLANK(*strptr3))
			    strptr3++;

			/* Reset primitive poitners and create new model
			 * of type V3D_MODEL_TYPE_STANDARD.
			 */
			DO_END_PRIMITIVES
			model_std = V3DModelListInsert(
			    model, total_models,
			    -1,			/* Append. */ 
			    V3D_MODEL_TYPE_STANDARD, strptr3
			);
			models_created++;
		    }
		    /* Assume other data type model. */
		    else
		    {
			/* Add this line to the other data block, allocate
			 * a new model of type V3D_MODEL_TYPE_OTHER_DATA as
			 * needed.
			 */
			if(model_oth == NULL)
			{
			    model_oth = V3DModelListInsert(
				model, total_models,
				-1,		/* Append. */
				V3D_MODEL_TYPE_OTHER_DATA, "other"
			    );
			    models_created++;
			}
			if(model_oth != NULL)
			{
			    if(model_oth->total_other_data_lines < 0)
				model_oth->total_other_data_lines = 0;

			    /* Allocate a new line. */
			    i = model_oth->total_other_data_lines;
			    model_oth->total_other_data_lines++;
			    model_oth->other_data_line = (char **)realloc(
				model_oth->other_data_line,
				model_oth->total_other_data_lines * sizeof(char *)
			    );
			    if(model_oth->other_data_line == NULL)
			    {
				model_oth->total_other_data_lines = 0;
			    }
			    else
			    {
				model_oth->other_data_line[i] = strptr2;
				strptr2 = NULL;	/* Set to NULL so it don't get freed. */
			    }
			}
		    }
		    free(strptr2);
		}

		/* Do not continue with parsing of data while loading of
		 * other data blocks.
		 */
		continue;
	    }



	    /* Continue parsing for standard model. */

	    /* Comment? */
	    if(ISCOMMENT(c))
	    {
		/* Record this comment. */
		char *strptr2 = NULL;

		/* Get strptr2 as the comment line, coppied. */
		if(fp != NULL)
		{
		    fseek(fp, -1, SEEK_CUR);
		    strptr2 = FGetStringLined(fp);
		}
		else if(buf_line != NULL)
		{
		    buf_line--;
		    if(buf_line < (*(buf - 1)))
			buf_line = (*(buf - 1));

		    strptr2 = STRDUP(buf_line);
		    buf_line = (*buf); buf++;
		}

		if((model_std != NULL) && (strptr2 != NULL))
		{
		    /* Check if a standard model is loaded and that there
		     * are no infinate vertex primitives in context
		     * right now. Because we don't want to save comments
		     * or garbage in the middle of an infinate primitive.
		     */
		    if(mp_comment == NULL)
		    {
/*
Do not end primitives when comment encountered, otherwise fininte
vertex primitives may end prematurly if a comment is embedded within.
			DO_END_PRIMITIVES
 */
			mp_comment = V3DMPListInsert(
			    &model_std->primitive,
			    &model_std->total_primitives,
			    -1,                     /* Append. */
			    V3DMP_TYPE_COMMENT
			);
		    }
		    else
		    {

		    }
		    if(mp_comment != NULL)
		    {
			V3DAddLine(
			    &(mp_comment->line),
			    &(mp_comment->total_lines),
			    strptr2
			);
		    }
		}

		free(strptr2);
		strptr2 = NULL;

		continue;
	    }

	    /* ******************************************************* */
	    /* Is a digit (implies a vertex)? */
	    if(isdigit(c) ||
	       (c == '+') ||
	       (c == '-')
	    )
	    {
		/* Handle this as a vertex. */
		double value[3];

		if(fp != NULL)
		{
		    fseek(fp, -1, SEEK_CUR);
		    FGetValuesF(fp, value, 3);
		}
		else if(buf_line != NULL)
		{
		    int sr, sc, st = 3;

		    buf_line--;
		    if(buf_line < (*(buf - 1)))
			buf_line = (*(buf - 1));

		    sr = sscanf(buf_line, "%lf %lf %lf",
			&value[0], &value[1], &value[2]
		    );
		    for(sc = sr; sc < st; sc++)
			value[sc] = 0.0;

		    buf_line = (*buf); buf++;
		}

		if(model_std != NULL)
		{
		    /* Begin setting vertex by the current primitive
		     * in context if any.
		     */

		    /* Point? */
		    if(mp_point != NULL)
		    {
			if(vertex_count >= V3DMP_POINT_NVERTEX)
			{
			    vertex_count = 0;
			    mp_point = V3DMPListInsert(   
				&model_std->primitive,
				&model_std->total_primitives,
				-1,             /* Append. */
				V3DMP_TYPE_POINT
			    );
			}
			if(mp_point != NULL)
			{
			    v = &mp_point->v[vertex_count];
			    v->x = value[0];
			    v->y = value[1];
			    v->z = value[2];
			}
			vertex_count++;
		    }
		    /* Line? */
		    else if(mp_line != NULL)
		    {
			if(vertex_count >= V3DMP_LINE_NVERTEX)
			{
			    vertex_count = 0;
			    mp_line = V3DMPListInsert(
				&model_std->primitive,
				&model_std->total_primitives,
				-1,             /* Append. */
				V3DMP_TYPE_LINE
			    );
			}
			if(mp_line != NULL)
			{
			    v = &mp_line->v[vertex_count];
			    v->x = value[0];
			    v->y = value[1];
			    v->z = value[2];
			}
			vertex_count++;
		    }
		    /* Line strip. */
		    else if(mp_line_strip != NULL)
		    {
			if(vertex_count != (mp_line_strip->total - 1))
			{
			    /* Did not get texture coordinates before
			     * vertex, append a new texture coordinate
			     * and vertex pair.
			     */
			    vertex_count = mp_line_strip->total;
			    V3DMPInsertVertex(
				mp_line_strip, -1,
				&v, NULL, NULL
			    );
			    if(v != NULL)
			    {
				v->x = value[0];
				v->y = value[1];
				v->z = value[2];
			    }
			    else
			    {
				vertex_count = 0;
			    }
			}
			else
			{
			    /* Got texture coordinates before vertex, so
			     * just set vertex.
			     */
			    v = V3DMPGetVertex(mp_line_strip, vertex_count);
			    if(v != NULL)
			    {
				v->x = value[0];
				v->y = value[1]; 
				v->z = value[2];
			    }
			}
			vertex_count++;
		    }
		    /* Line loop? */
		    else if(mp_line_loop != NULL)
		    {
			if(vertex_count != (mp_line_loop->total - 1))
			{
			    /* Did not get texture coordinates before
			     * vertex, append a new texture coordinate
			     * and vertex pair.
			     */
			    vertex_count = mp_line_loop->total;
			    V3DMPInsertVertex(
				mp_line_loop, -1,
				&v, NULL, NULL
			    );
			    if(v != NULL)
			    {  
				v->x = value[0];
				v->y = value[1];
				v->z = value[2];
			    }
			    else
			    {
				vertex_count = 0;
			    }
			}
			else
			{
			    /* Got texture coordinates before vertex, so
			     * just set vertex.
			     */
			    v = V3DMPGetVertex(mp_line_loop, vertex_count);
			    if(v != NULL)
			    {
				v->x = value[0];
				v->y = value[1];
				v->z = value[2];
			    }
			}
			vertex_count++;
		    }
		    /* Triangle? */
		    else if(mp_triangle != NULL)
		    {
		        if(vertex_count >= V3DMP_TRIANGLE_NVERTEX)
		        {
			    /* Vertexes exceeded, insert new primitive. */
			    vertex_count = 0;
			    mp_triangle = V3DMPListInsert(
				&model_std->primitive,
				&model_std->total_primitives,
				-1,		/* Append. */
				V3DMP_TYPE_TRIANGLE
			    );
			}
			if(mp_triangle != NULL)
			{
			    v = &mp_triangle->v[vertex_count];
			    v->x = value[0];
			    v->y = value[1];
			    v->z = value[2];
			}
			vertex_count++;
		    }
		    /* Triangle strip. */
		    else if(mp_triangle_strip != NULL)
		    {
			if(vertex_count != (mp_triangle_strip->total - 1))
			{
			    /* Did not get texture coordinates before   
			     * vertex, append a new texture coordinate
			     * and vertex pair.
			     */
			    vertex_count = mp_triangle_strip->total;
			    V3DMPInsertVertex(
				mp_triangle_strip, -1,
				&v, NULL, NULL
			    );
			    if(v != NULL)
			    {
				v->x = value[0];
				v->y = value[1];
				v->z = value[2];
			    }
			    else
			    {
				vertex_count = 0;
			    }
			}
			else
			{
			    /* Got texture coordinates before vertex, so
			     * just set vertex.
			     */
			    v = V3DMPGetVertex(mp_triangle_strip, vertex_count);
			    if(v != NULL)
			    {
				v->x = value[0];
				v->y = value[1];
				v->z = value[2];
			    }
			}
			vertex_count++;
		    }
		    /* Triangle fan? */
		    else if(mp_triangle_fan != NULL)
		    {
			if(vertex_count != (mp_triangle_fan->total - 1))
			{
			    /* Did not get texture coordinates before
			     * vertex, append a new texture coordinate
			     * and vertex pair.
			     */
			    vertex_count = mp_triangle_fan->total;
			    V3DMPInsertVertex(
				mp_triangle_fan, -1,
				&v, NULL, NULL
			    );
			    if(v != NULL)
			    {
				v->x = value[0];
				v->y = value[1];
				v->z = value[2];
			    }
			    else
			    {
				vertex_count = 0;
			    }
			}
			else
			{
			    /* Got texture coordinates before vertex, so
			     * just set vertex.
			     */
			    v = V3DMPGetVertex(mp_triangle_fan, vertex_count);
			    if(v != NULL)
			    {
				v->x = value[0];
				v->y = value[1];
				v->z = value[2];
			    }
			}
			vertex_count++;
		    }
		    /* Quad? */
		    else if(mp_quad != NULL)
		    {
			if(vertex_count >= V3DMP_QUAD_NVERTEX)
			{
			    /* Vertexes exceeded, insert new primitive. */
			    vertex_count = 0;
			    mp_quad = V3DMPListInsert(
				&model_std->primitive,
				&model_std->total_primitives,
				-1,             /* Append. */
				V3DMP_TYPE_QUAD
			    );
			}
			if(mp_quad != NULL)
			{
			    v = &mp_quad->v[vertex_count];
			    v->x = value[0];
			    v->y = value[1];
			    v->z = value[2];
			}
			vertex_count++;
		    }
		    /* Quad strip? */
		    else if(mp_quad_strip != NULL)
		    {
			if(vertex_count != (mp_quad_strip->total - 1))
			{
			    /* Did not get texture coordinates before
			     * vertex, append a new texture coordinate
			     * and vertex pair.
			     */
			    vertex_count = mp_quad_strip->total;
			    V3DMPInsertVertex(
				mp_quad_strip, -1,  
				&v, NULL, NULL
			    );
			    if(v != NULL)
			    {
				v->x = value[0];
				v->y = value[1];
				v->z = value[2];
			    }
			    else
			    {
				vertex_count = 0;
			    }
			}
			else
			{
			    /* Got texture coordinates before vertex, so
			     * just set vertex. 
			     */
			    v = V3DMPGetVertex(mp_quad_strip, vertex_count);
			    if(v != NULL)
			    {
				v->x = value[0];
				v->y = value[1];
				v->z = value[2];
			    }
			}
			vertex_count++;
		    }
		    /* Polygon? */
		    else if(mp_polygon != NULL)
		    {
			if(vertex_count != (mp_polygon->total - 1))
			{
			    /* Did not get texture coordinates before
			     * vertex, append a new texture coordinate
			     * and vertex pair.
			     */
			    vertex_count = mp_polygon->total;
			    V3DMPInsertVertex(
				mp_polygon, -1,
				&v, NULL, NULL
			    );
			    if(v != NULL)
			    {
				v->x = value[0];  
				v->y = value[1];
				v->z = value[2];
			    }
			    else
			    {
				vertex_count = 0;
			    }
			}
			else
			{
			    /* Got texture coordinates before vertex, so
			     * just set vertex.
			     */
			    v = V3DMPGetVertex(mp_polygon, vertex_count);
			    if(v != NULL)
			    {
				v->x = value[0];
				v->y = value[1];
				v->z = value[2];
			    }  
			}
			vertex_count++;
		    }
		}
	    }
	    /* ******************************************************* */
	    else
	    {
		/* Not a digit, must be an operation. */

		/* Get string pointer to operation string. */
		if(fp != NULL)
		{
		    fseek(fp, -1, SEEK_CUR);
		    strptr = V3DLoadGetParmF(fp);
		}
		else if(buf_line != NULL)
		{
		    char *strptr2;

		    buf_line--;
		    if(buf_line < (*(buf - 1)))
			buf_line = (*(buf - 1));

		    while(ISBLANK(*buf_line))
			buf_line++;

		    strptr = V3DLoadGetParmD(buf_line);
		    strptr2 = STRSEEKBLANK((char *)buf_line);
		    if(strptr2 != NULL)
		    {
			while(ISBLANK(*strptr2))
			    strptr2++;
			buf_line = strptr2;
		    }
		    else
		    {
			buf_line = (*buf);
		    }
		}
		else
		{
		    strptr = NULL;
		}
		if(strptr == NULL)
		    continue;

		/* Note, do not check for parameter begin_model, since
		 * it would have been checked while loading a model
		 * other data block.
		 */
		/* End model data. */
		if(!strcasecmp(strptr, "end_model"))
		{
		    DO_SEEK_NEXT_LINE
		    DO_END_PRIMITIVES
		    model_std = NULL;
		}

		/* Translate. */
		else if(!strcasecmp(strptr, "translate"))
		{
		    double value[3];

		    if(fp != NULL)
		    {
			fseek(fp, -1, SEEK_CUR);
			FGetValuesF(fp, value, 3);
		    }
		    else if(buf_line != NULL)
		    {
			int sr, sc, st = 3;

			sr = sscanf(buf_line, "%lf %lf %lf",
			    &value[0], &value[1], &value[2]
			);
			for(sc = sr; sc < st; sc++)
			    value[sc] = 0.0;

			buf_line = (*buf); buf++;
		    }

		    DO_END_PRIMITIVES

		    if(model_std != NULL)
		    {
			mp_translate = V3DMPListInsert(
			    &model_std->primitive,
			    &model_std->total_primitives,
			    -1,                 /* Append. */
			    V3DMP_TYPE_TRANSLATE
			);
			if(mp_translate != NULL)
			{ 
			    mp_translate->x = value[0];
			    mp_translate->y = value[1];
			    mp_translate->z = value[2];

			    mp_translate = NULL;
			}
		    }
		}
		/* Untranslate. */
		else if(!strcasecmp(strptr, "untranslate"))
		{
		    DO_SEEK_NEXT_LINE
		    DO_END_PRIMITIVES

		    if(model_std != NULL)
		    {
			mp_untranslate = V3DMPListInsert(
			    &model_std->primitive,
			    &model_std->total_primitives,   
			    -1,                 /* Append. */
			    V3DMP_TYPE_UNTRANSLATE
			);

			mp_untranslate = NULL;
		    }
		}

		/* Rotate. */
		else if(!strcasecmp(strptr, "rotate"))
		{
		    double value[3];

		    if(fp != NULL)
		    {
			fseek(fp, -1, SEEK_CUR);
			FGetValuesF(fp, value, 3);
		    }
		    else if(buf_line != NULL)   
		    {
			int sr, sc, st = 3;

			sr = sscanf(buf_line, "%lf %lf %lf",
			    &value[0], &value[1], &value[2]
			);
			for(sc = sr; sc < st; sc++)
			    value[sc] = 0.0;

			buf_line = (*buf); buf++;
		    }

		    DO_END_PRIMITIVES

		    if(model_std != NULL)
		    {
			mp_rotate = V3DMPListInsert(
			    &model_std->primitive,
			    &model_std->total_primitives,
			    -1,                 /* Append. */
			    V3DMP_TYPE_ROTATE
			);
			if(mp_rotate != NULL)
			{
			    mp_rotate->heading = DEGTORAD(value[0]);
			    mp_rotate->pitch = DEGTORAD(value[1]);
			    mp_rotate->bank = DEGTORAD(value[2]);

			    mp_translate = NULL;
			}
		    }
		}
		/* Unrotate. */
		else if(!strcasecmp(strptr, "unrotate"))
		{
		    DO_SEEK_NEXT_LINE
		    DO_END_PRIMITIVES

		    if(model_std != NULL)
		    {
			mp_unrotate = V3DMPListInsert(
			    &model_std->primitive,
			    &model_std->total_primitives,
			    -1,                 /* Append. */
			    V3DMP_TYPE_UNROTATE
			);

			mp_unrotate = NULL;
		    }
		}



		/* Begin points. */
		else if(!strcasecmp(strptr, "begin_points"))
		{
		    DO_SEEK_NEXT_LINE
		    DO_END_PRIMITIVES
		    if(model_std != NULL)
		    {
			mp_point = V3DMPListInsert(
			    &model_std->primitive,
			    &model_std->total_primitives,
			    -1,             /* Append. */
			    V3DMP_TYPE_POINT
			);
		    }
		}
		/* End points. */
		else if(!strcasecmp(strptr, "end_points"))
		{
		    DO_SEEK_NEXT_LINE
		    DO_END_PRIMITIVES
		}

		/* Begin lines. */
		else if(!strcasecmp(strptr, "begin_lines"))
		{
		    DO_SEEK_NEXT_LINE
		    DO_END_PRIMITIVES
		    if(model_std != NULL)
		    {
			mp_line = V3DMPListInsert(
			    &model_std->primitive,
			    &model_std->total_primitives,
			    -1,             /* Append. */
			    V3DMP_TYPE_LINE
			);
		    }
		}
		/* End lines. */
		else if(!strcasecmp(strptr, "end_lines"))
		{
		    DO_SEEK_NEXT_LINE
		    DO_END_PRIMITIVES
		}

		/* Begin line strip. */
		else if(!strcasecmp(strptr, "begin_line_strip"))
		{
		    DO_SEEK_NEXT_LINE
		    DO_END_PRIMITIVES
		    if(model_std != NULL)
		    {
			mp_line_strip = V3DMPListInsert(
			    &model_std->primitive,
			    &model_std->total_primitives,
			    -1,		/* Append. */
			    V3DMP_TYPE_LINE_STRIP
			);
		    }
		}
		/* End line strip. */
		else if(!strcasecmp(strptr, "end_line_strip"))
		{
		    DO_SEEK_NEXT_LINE
		    DO_END_PRIMITIVES
		}

		/* Begin line loop. */
		else if(!strcasecmp(strptr, "begin_line_loop"))
		{
		    DO_SEEK_NEXT_LINE
		    DO_END_PRIMITIVES
		    if(model_std != NULL)
		    {
			mp_line_loop = V3DMPListInsert(
			    &model_std->primitive,
			    &model_std->total_primitives,
			    -1,		/* Append. */
			    V3DMP_TYPE_LINE_LOOP
			);
		    }
		}
		/* End line loop. */
		else if(!strcasecmp(strptr, "end_line_loop"))
		{
		    DO_SEEK_NEXT_LINE
		    DO_END_PRIMITIVES
		}

		/* Begin triangles. */
		else if(!strcasecmp(strptr, "begin_triangles"))
		{
		    DO_SEEK_NEXT_LINE
		    DO_END_PRIMITIVES
		    if(model_std != NULL)
		    {
			mp_triangle = V3DMPListInsert(
			    &model_std->primitive,
			    &model_std->total_primitives,
			    -1,             /* Append. */
			    V3DMP_TYPE_TRIANGLE
			);
		    }
		}
		/* End triangles. */
		else if(!strcasecmp(strptr, "end_triangles"))
		{
		    DO_SEEK_NEXT_LINE
		    DO_END_PRIMITIVES
		}

		/* Begin triangle strip. */
		else if(!strcasecmp(strptr, "begin_triangle_strip"))
		{
		    DO_SEEK_NEXT_LINE
		    DO_END_PRIMITIVES
		    if(model_std != NULL)
		    {
			mp_triangle_strip = V3DMPListInsert(
			    &model_std->primitive,
			    &model_std->total_primitives,
			    -1,		/* Append. */
			    V3DMP_TYPE_TRIANGLE_STRIP
			);
		    }
		}
		/* End triangle strip. */
		else if(!strcasecmp(strptr, "end_triangle_strip"))
		{
		    DO_SEEK_NEXT_LINE
		    DO_END_PRIMITIVES
		}

		/* Begin triangle fan. */
		else if(!strcasecmp(strptr, "begin_triangle_fan"))
		{
		    DO_SEEK_NEXT_LINE
		    DO_END_PRIMITIVES
		    if(model_std != NULL)
		    {
			mp_triangle_fan = V3DMPListInsert(
			    &model_std->primitive,
			    &model_std->total_primitives,
			    -1,             /* Append. */
			    V3DMP_TYPE_TRIANGLE_FAN
			);
		    }
		}
		/* End triangle fan. */
		else if(!strcasecmp(strptr, "end_triangle_fan"))
		{
		    DO_SEEK_NEXT_LINE
		    DO_END_PRIMITIVES
		}

		/* Begin quads. */
		else if(!strcasecmp(strptr, "begin_quads"))
		{
		    DO_SEEK_NEXT_LINE
		    DO_END_PRIMITIVES
		    if(model_std != NULL)
		    {
			mp_quad = V3DMPListInsert(
			    &model_std->primitive,
			    &model_std->total_primitives,
			    -1,             /* Append. */
			    V3DMP_TYPE_QUAD
			);
		    }
		}
		/* End quads. */
		else if(!strcasecmp(strptr, "end_quads"))
		{
		    DO_SEEK_NEXT_LINE
		    DO_END_PRIMITIVES
		}

		/* Begin quad strip. */
		else if(!strcasecmp(strptr, "begin_quad_strip"))
		{
		    DO_SEEK_NEXT_LINE
		    DO_END_PRIMITIVES
		    if(model_std != NULL)
		    {
			mp_quad_strip = V3DMPListInsert(
			    &model_std->primitive,
			    &model_std->total_primitives,
			    -1,             /* Append. */
			    V3DMP_TYPE_QUAD_STRIP
			);
		    }
		}
		/* End quad strip. */
		else if(!strcasecmp(strptr, "end_quad_strip"))
		{
		    DO_SEEK_NEXT_LINE
		    DO_END_PRIMITIVES
		}

		/* Begin polygon. */
		else if(!strcasecmp(strptr, "begin_polygon"))
		{
		    DO_SEEK_NEXT_LINE
		    DO_END_PRIMITIVES
		    if(model_std != NULL)
		    {
			mp_polygon = V3DMPListInsert(
			    &model_std->primitive,
			    &model_std->total_primitives,
			    -1,		/* Append. */
			    V3DMP_TYPE_POLYGON
			);
		    }
		}
		/* End polygon. */
		else if(!strcasecmp(strptr, "end_polygon"))
		{
		    DO_SEEK_NEXT_LINE
		    DO_END_PRIMITIVES
		}


		/* Normal. */
		else if(!strcasecmp(strptr, "normal"))
		{
		    double value[3];

		    if(fp != NULL)
		    {
			fseek(fp, -1, SEEK_CUR);
			FGetValuesF(fp, value, 3);
		    }
		    else if(buf_line != NULL)
		    {
			int sr, sc, st = 3;

			sr = sscanf(buf_line, "%lf %lf %lf",
			    &value[0], &value[1], &value[2]
			);
			for(sc = sr; sc < st; sc++)
			    value[sc] = 0.0;

			buf_line = (*buf); buf++;
		    }

		    if(model_std != NULL)
		    {
			mp_vertex_struct *norm = NULL;

			/* Set normal by current primitive in context. */
			/* Point? */
			if(mp_point != NULL)
			{
			    if(vertex_count >= V3DMP_POINT_NVERTEX)
			    {
				vertex_count = 0;
				mp_point = V3DMPListInsert(
				    &model_std->primitive,
				    &model_std->total_primitives,
				    -1,                 /* Append. */
				    V3DMP_TYPE_POINT
				);
			    }
			    if((vertex_count >= 0) &&
			       (vertex_count < V3DMP_POINT_NVERTEX)
			    )
			    {
				norm = &mp_point->n[vertex_count];
				norm->x = value[0];
				norm->y = value[1];
				norm->z = value[2];
			    }
			}
			/* Line? */
			else if(mp_line != NULL)
			{
			    if(vertex_count >= V3DMP_LINE_NVERTEX)
			    {
				vertex_count = 0;
				mp_line = V3DMPListInsert(
				    &model_std->primitive,
				    &model_std->total_primitives,
				    -1,                 /* Append. */
				    V3DMP_TYPE_LINE
				);
			    }
			    if((vertex_count >= 0) &&
			       (vertex_count < V3DMP_LINE_NVERTEX)
			    )
			    {
				norm = &mp_line->n[vertex_count];
				norm->x = value[0];
				norm->y = value[1];
				norm->z = value[2];
			    }
			}
			/* Line strip? */
			else if(mp_line_strip != NULL)
			{
#define PTR_NAME	mp_line_strip
			    if(vertex_count >= PTR_NAME->total)
			    {
				vertex_count = PTR_NAME->total;
				V3DMPInsertVertex(
				    PTR_NAME, -1,
				    NULL, &norm, NULL
				);
			    }
			    else if(vertex_count >= 0)
			    {
				norm = PTR_NAME->n[vertex_count];
			    }
			    if(norm != NULL)
			    {
				norm->x = value[0];
				norm->y = value[1];
				norm->z = value[2];
			    }
			}
			/* Line loop? */
			else if(mp_line_loop != NULL)
			{
#undef PTR_NAME
#define PTR_NAME	mp_line_loop
			    if(vertex_count >= PTR_NAME->total)
			    {
				vertex_count = PTR_NAME->total;
				V3DMPInsertVertex(
				    PTR_NAME, -1,
				    NULL, &norm, NULL
				);
			    }
			    else if(vertex_count >= 0)
			    {
				norm = PTR_NAME->n[vertex_count];
			    }
			    if(norm != NULL)
			    { 
				norm->x = value[0];
				norm->y = value[1];
				norm->z = value[2];
			    }
			}
			/* Triangle? */
			else if(mp_triangle != NULL)
			{
			    if(vertex_count >= V3DMP_TRIANGLE_NVERTEX)
			    {
				vertex_count = 0;
				mp_triangle = V3DMPListInsert(
				    &model_std->primitive,
				    &model_std->total_primitives,
				    -1,                 /* Append. */
				    V3DMP_TYPE_TRIANGLE
				);
			    }
			    if((vertex_count >= 0) &&
			       (vertex_count < V3DMP_TRIANGLE_NVERTEX)
			    )
			    {
				norm = &mp_triangle->n[vertex_count];
				norm->x = value[0];
				norm->y = value[1];
				norm->z = value[2];
			    }
			}
			/* Triangle strip? */
			else if(mp_triangle_strip != NULL)
			{
#undef PTR_NAME
#define PTR_NAME	mp_triangle_strip
			    if(vertex_count >= PTR_NAME->total)
			    {
				vertex_count = PTR_NAME->total;
				V3DMPInsertVertex(
				    PTR_NAME, -1,
				    NULL, &norm, NULL
				);
			    }
			    else if(vertex_count >= 0)
			    {
				norm = PTR_NAME->n[vertex_count];
			    }
			    if(norm != NULL)
			    {
				norm->x = value[0];
				norm->y = value[1];
				norm->z = value[2];
			    }
			}
			/* Triangle fan? */
			else if(mp_triangle_fan != NULL)
			{
#undef PTR_NAME
#define PTR_NAME	mp_triangle_fan
			    if(vertex_count >= PTR_NAME->total)
			    {
				vertex_count = PTR_NAME->total;
				V3DMPInsertVertex(
				    PTR_NAME, -1,
				    NULL, &norm, NULL
				);
			    }
			    else if(vertex_count >= 0)
			    {
				norm = PTR_NAME->n[vertex_count];
			    }
			    if(norm != NULL)
			    {
				norm->x = value[0];
				norm->y = value[1];
				norm->z = value[2];
			    }
			}
			/* Quad? */
			else if(mp_quad != NULL)
			{
			    if(vertex_count >= V3DMP_QUAD_NVERTEX)
			    {
				vertex_count = 0;
				mp_quad = V3DMPListInsert(
				    &model_std->primitive,
				    &model_std->total_primitives,
				    -1,                 /* Append. */
				    V3DMP_TYPE_QUAD
				);
			    }
			    if((vertex_count >= 0) &&
			       (vertex_count < V3DMP_QUAD_NVERTEX)
			    )
			    {
				norm = &mp_quad->n[vertex_count];
				norm->x = value[0];
				norm->y = value[1];
				norm->z = value[2];
			    }
			}
			/* Quad strip? */
			else if(mp_quad_strip != NULL)
			{
#undef PTR_NAME
#define PTR_NAME	mp_quad_strip
			    if(vertex_count >= PTR_NAME->total)
			    {
				vertex_count = PTR_NAME->total;
				V3DMPInsertVertex(
				    PTR_NAME, -1,
				    NULL, &norm, NULL
				);
			    }
			    else if(vertex_count >= 0)
			    {
				norm = PTR_NAME->n[vertex_count];
			    }
			    if(norm != NULL)
			    {
				norm->x = value[0];   
				norm->y = value[1];
				norm->z = value[2];
			    }
			}
			/* Polygon? */
			else if(mp_polygon != NULL)
			{
#undef PTR_NAME
#define PTR_NAME	mp_polygon
			    if(vertex_count >= PTR_NAME->total)
			    {
				vertex_count = PTR_NAME->total;
				V3DMPInsertVertex(
				    PTR_NAME, -1,
				    NULL, &norm, NULL
				);
			    }
			    else if(vertex_count >= 0)
			    {
				norm = PTR_NAME->n[vertex_count];
			    }
			    if(norm != NULL)
			    {
				norm->x = value[0];
				norm->y = value[1];
				norm->z = value[2];
			    }
			}
		    }
#undef PTR_NAME
		}

		/* Color. */
		else if(!strcasecmp(strptr, "color"))
		{
		    double value[9];

		    /* Need to stop comment primitives if color 
		     * encountered.
		     */
		    mp_comment = NULL;

		    if(fp != NULL)
		    {
			fseek(fp, -1, SEEK_CUR);
			FGetValuesF(fp, value, 9);
		    }
		    else if(buf_line != NULL)
		    {
			int sr, sc, st = 9;

			sr = sscanf(buf_line,
			    "%lf %lf %lf %lf %lf %lf %lf %lf %lf",
			    &value[0], &value[1], &value[2], &value[3],
			    &value[4], &value[5], &value[6], &value[7],
			    &value[8]
			);
			for(sc = sr; sc < st; sc++)
			    value[sc] = 0.0;

			buf_line = (*buf); buf++;
		    }

		    /* Do not end primitive, color can be embedded. */

		    if(model_std != NULL)
		    {
			mp_color = V3DMPListInsert(
			    &model_std->primitive,
			    &model_std->total_primitives, 
			    -1,			/* Append. */
			    V3DMP_TYPE_COLOR
			);
			if(mp_color != NULL)
			{
			    mp_color->r = CLIP(value[0], 0.0, 1.0);
			    mp_color->g = CLIP(value[1], 0.0, 1.0);
			    mp_color->b = CLIP(value[2], 0.0, 1.0);
			    mp_color->a = CLIP(value[3], 0.0, 1.0);
			    mp_color->ambient = CLIP(value[4], 0.0, 1.0);
			    mp_color->diffuse = CLIP(value[5], 0.0, 1.0);
			    mp_color->specular = CLIP(value[6], 0.0, 1.0);
			    mp_color->shininess = CLIP(value[7], 0.0, 1.0);
			    mp_color->emission = CLIP(value[8], 0.0, 1.0);
			}
			mp_color = NULL;
		    }
		}

		/* Texture coordinates. */
		else if(!strcasecmp(strptr, "texcoord") ||
			!strcasecmp(strptr, "tex_coord") ||
			!strcasecmp(strptr, "texture")
		)
		{
		    double value[3];

		    if(fp != NULL)
		    {
			fseek(fp, -1, SEEK_CUR);
			FGetValuesF(fp, value, 3);
		    }
		    else if(buf_line != NULL)
		    {
			int sr, sc, st = 3;

			sr = sscanf(buf_line, "%lf %lf %lf",
			    &value[0], &value[1], &value[2]
			);
			for(sc = sr; sc < st; sc++)
			    value[sc] = 0.0;

			buf_line = (*buf); buf++;
		    }

		    if(model_std != NULL)
		    {
			/* Set texture coordinates by current
			 * primitive in context.
			 */

			/* Point? */
			if(mp_point != NULL)
			{
			    if(vertex_count >= V3DMP_POINT_NVERTEX)
			    {
				vertex_count = 0;
				mp_point = V3DMPListInsert(
				    &model_std->primitive,
				    &model_std->total_primitives,
				    -1,			/* Append. */
				    V3DMP_TYPE_POINT
				);
			    }
			    if((vertex_count >= 0) &&
			       (vertex_count < V3DMP_POINT_NVERTEX)
			    )
			    {
				v = &mp_point->tc[vertex_count];
				v->x = value[0];
				v->y = value[1];
				v->z = value[2];
			    }
			}
			/* Line? */
			else if(mp_line != NULL)
			{
			    if(vertex_count >= V3DMP_LINE_NVERTEX)
			    {
				vertex_count = 0;
				mp_line = V3DMPListInsert(
				    &model_std->primitive,
				    &model_std->total_primitives,  
				    -1,                 /* Append. */
				    V3DMP_TYPE_LINE 
				);
			    }
			    if((vertex_count >= 0) &&
			       (vertex_count < V3DMP_LINE_NVERTEX)
			    )
			    {
				v = &mp_line->tc[vertex_count];
				v->x = value[0];
				v->y = value[1];
				v->z = value[2];
			    }
			}
			/* Line strip? */
			else if(mp_line_strip != NULL)
			{
#define PTR_NAME	mp_line_strip
			    v = NULL;
			    if(vertex_count >= PTR_NAME->total)
			    {
				vertex_count = PTR_NAME->total;
				V3DMPInsertVertex(
				    PTR_NAME, -1,
				    NULL, NULL, &v
				);
			    }
			    else if(vertex_count >= 0)
			    {
				v = PTR_NAME->tc[vertex_count];
			    }
			    if(v != NULL)
			    {
				v->x = value[0];
				v->y = value[1];
				v->z = value[2];
			    }
			}
			/* Line loop? */
			else if(mp_line_loop != NULL)
			{
#undef PTR_NAME
#define PTR_NAME	mp_line_loop
			    v = NULL;
			    if(vertex_count >= PTR_NAME->total)
			    {
				vertex_count = PTR_NAME->total;
				V3DMPInsertVertex(
				    PTR_NAME, -1,
				    NULL, NULL, &v
				);
			    }
			    else if(vertex_count >= 0)
			    {
				v = PTR_NAME->tc[vertex_count];
			    }
			    if(v != NULL)
			    {
				v->x = value[0];
				v->y = value[1];
				v->z = value[2];
			    }
			}
			/* Triangle? */
			else if(mp_triangle != NULL)
			{
			    if(vertex_count >= V3DMP_TRIANGLE_NVERTEX)
			    {
				vertex_count = 0;
				mp_triangle = V3DMPListInsert(
				    &model_std->primitive,
				    &model_std->total_primitives,
				    -1,			/* Append. */
				    V3DMP_TYPE_TRIANGLE
				); 
			    }
			    if((vertex_count >= 0) &&
			       (vertex_count < V3DMP_TRIANGLE_NVERTEX)
			    )
			    {
				v = &mp_triangle->tc[vertex_count];
				v->x = value[0];
				v->y = value[1];
				v->z = value[2];
			    }
			}
			/* Triangle strip? */
			else if(mp_triangle_strip != NULL)
			{
#undef PTR_NAME
#define PTR_NAME	mp_triangle_strip
			    v = NULL;
			    if(vertex_count >= PTR_NAME->total)
			    {
				vertex_count = PTR_NAME->total;
				V3DMPInsertVertex(
				    PTR_NAME, -1, 
				    NULL, NULL, &v
				);
			    }
			    else if(vertex_count >= 0)
			    {
				v = PTR_NAME->tc[vertex_count];
			    }
			    if(v != NULL)
			    {
				v->x = value[0];
				v->y = value[1];
				v->z = value[2];
			    }
			}
			/* Triangle fan? */
			else if(mp_triangle_fan != NULL)
			{
#undef PTR_NAME
#define PTR_NAME	mp_triangle_fan
			    v = NULL;
			    if(vertex_count >= PTR_NAME->total)
			    {
				vertex_count = PTR_NAME->total;
				V3DMPInsertVertex(
				    PTR_NAME, -1,
				    NULL, NULL, &v
				); 
			    }
			    else if(vertex_count >= 0)
			    {
				v = PTR_NAME->tc[vertex_count];
			    }
			    if(v != NULL)
			    {
				v->x = value[0];
				v->y = value[1];
				v->z = value[2];
			    }
			}
			/* Quad? */
			else if(mp_quad != NULL)
			{
			    if(vertex_count >= V3DMP_QUAD_NVERTEX)
			    {
				vertex_count = 0;
				mp_quad = V3DMPListInsert(  
				    &model_std->primitive,
				    &model_std->total_primitives,
				    -1,			/* Append. */
				    V3DMP_TYPE_QUAD
				);
			    }
			    if((vertex_count >= 0) &&
			       (vertex_count < V3DMP_QUAD_NVERTEX)
			    )
			    {
				v = &mp_quad->tc[vertex_count];
				v->x = value[0];
				v->y = value[1];
				v->z = value[2];
			    }
			}
			/* Quad strip? */
			else if(mp_quad_strip != NULL)
			{
#undef PTR_NAME
#define PTR_NAME	mp_quad_strip
			    v = NULL;
			    if(vertex_count >= PTR_NAME->total)
			    {
				vertex_count = PTR_NAME->total;
				V3DMPInsertVertex(
				    PTR_NAME, -1,
				    NULL, NULL, &v
				);
			    }
			    else if(vertex_count >= 0)
			    {
				v = PTR_NAME->tc[vertex_count];
			    }
			    if(v != NULL)
			    {
				v->x = value[0];
				v->y = value[1];
				v->z = value[2];
			    }
			}
			/* Polygon? */
			else if(mp_polygon != NULL)
			{
#undef PTR_NAME
#define PTR_NAME	mp_polygon
			    v = NULL;
			    if(vertex_count >= PTR_NAME->total)
			    {
				vertex_count = PTR_NAME->total;
				V3DMPInsertVertex(
				    PTR_NAME, -1,
				    NULL, NULL, &v
				);
			    }
			    else if(vertex_count >= 0)
			    {
				v = PTR_NAME->tc[vertex_count];
			    }
			    if(v != NULL)
			    {
				v->x = value[0];   
				v->y = value[1];
				v->z = value[2];
			    }
			}
		    }
#undef PTR_NAME
		}
		/* Texture select. */
		else if(!strcasecmp(strptr, "texture_select"))
		{
		    char *strptr2 = NULL;

		    /* Get strptr2 as the new line (coppied). */
		    if(fp != NULL)
		    {
			strptr2 = FGetStringLined(fp);
		    }
		    else if(buf_line != NULL)
		    {
			strptr2 = STRDUP(buf_line);
			buf_line = (*buf); buf++;
		    }

		    DO_END_PRIMITIVES
		    if(model_std != NULL)
		    {
			mp_texture_select = V3DMPListInsert(
			    &model_std->primitive, 
			    &model_std->total_primitives,
			    -1,				/* Append. */
			    V3DMP_TYPE_TEXTURE_SELECT
			);
			if(mp_texture_select != NULL)
			{
			    mp_texture_select->name = strptr2;
			    strptr2 = NULL;
			}
		    }
		    free(strptr2);
		}
		/* Texture orient xy. */
		else if(!strcasecmp(strptr, "texture_orient_xy"))
		{
		    double value[4];

		    if(fp != NULL)
		    {
			fseek(fp, -1, SEEK_CUR);
			FGetValuesF(fp, value, 4);
		    }
		    else if(buf_line != NULL)   
		    {
			int sr, sc, st = 4;

			sr = sscanf(buf_line, "%lf %lf %lf %lf",
			    &value[0], &value[1], &value[2], &value[3]
			);
			for(sc = sr; sc < st; sc++)
			    value[sc] = 0.0;

			buf_line = (*buf); buf++;
		    }

		    DO_END_PRIMITIVES
		    if(model_std != NULL)
		    {
			mp_texture_xy = V3DMPListInsert(
			    &model_std->primitive,
			    &model_std->total_primitives,
			    -1,                 /* Append. */
			    V3DMP_TYPE_TEXTURE_ORIENT_XY
			);
			if(mp_texture_xy != NULL)
			{
			    mp_texture_xy->x = value[0];
			    mp_texture_xy->y = value[1];
			    mp_texture_xy->dx = value[2];
			    mp_texture_xy->dy = value[3];
			}
		    }
		}
		/* Texture orient yz. */
		else if(!strcasecmp(strptr, "texture_orient_yz"))
		{
		    double value[4];

		    if(fp != NULL)
		    {
			fseek(fp, -1, SEEK_CUR);
			FGetValuesF(fp, value, 4);
		    }
		    else if(buf_line != NULL)
		    {
			int sr, sc, st = 4;

			sr = sscanf(buf_line, "%lf %lf %lf %lf",
			    &value[0], &value[1], &value[2], &value[3]
			);
			for(sc = sr; sc < st; sc++)
			    value[sc] = 0.0;

			buf_line = (*buf); buf++;
		    }

		    DO_END_PRIMITIVES
		    if(model_std != NULL)
		    {
			mp_texture_yz = V3DMPListInsert(
			    &model_std->primitive,
			    &model_std->total_primitives,
			    -1,			/* Append. */
			    V3DMP_TYPE_TEXTURE_ORIENT_YZ
			);
			if(mp_texture_yz != NULL)  
			{
			    mp_texture_yz->y = value[0];
			    mp_texture_yz->z = value[1];
			    mp_texture_yz->dy = value[2];
			    mp_texture_yz->dz = value[3];
			}
		    }
		}
		/* Texture orient xz. */
		else if(!strcasecmp(strptr, "texture_orient_xz"))
		{
		    double value[4];

		    if(fp != NULL)
		    {
			fseek(fp, -1, SEEK_CUR);
			FGetValuesF(fp, value, 4);
		    }
		    else if(buf_line != NULL)
		    {
			int sr, sc, st = 4;

			sr = sscanf(buf_line, "%lf %lf %lf %lf",
			    &value[0], &value[1], &value[2], &value[3]
			);
			for(sc = sr; sc < st; sc++)
			    value[sc] = 0.0;

			buf_line = (*buf); buf++;
		    }

		    DO_END_PRIMITIVES
		    if(model_std != NULL)
		    {
			mp_texture_xz = V3DMPListInsert(
			    &model_std->primitive,
			    &model_std->total_primitives,
			    -1,			/* Append. */
			    V3DMP_TYPE_TEXTURE_ORIENT_XZ
			);
			if(mp_texture_xz != NULL)
			{
			    mp_texture_xz->x = value[0];
			    mp_texture_xz->z = value[1]; 
			    mp_texture_xz->dx = value[2];
			    mp_texture_xz->dz = value[3];
			}
		    }
		}
		/* Texture unselect. */
		else if(!strcasecmp(strptr, "texture_off") ||
		        !strcasecmp(strptr, "texture_unselect")
		)
		{
		    DO_SEEK_NEXT_LINE
		    DO_END_PRIMITIVES
		    if(model_std != NULL)
		    {
			mp_texture_off = V3DMPListInsert(
			    &model_std->primitive,
			    &model_std->total_primitives,
			    -1,                 /* Append. */
			    V3DMP_TYPE_TEXTURE_OFF
			);
		    }
		}

		/* Heightfield load. */
		else if(!strcasecmp(strptr, "heightfield_load"))
		{
		    char *strptr2 = NULL;

		    /* Get strptr2 as the new line (coppied). */
		    if(fp != NULL)
		    {
			strptr2 = FGetStringLined(fp);
		    }
		    else if(buf_line != NULL)
		    {
			strptr2 = STRDUP(buf_line);
			buf_line = (*buf); buf++;
		    }

		    DO_END_PRIMITIVES
		    if((model_std != NULL) && (strptr2 != NULL))
		    {
			mp_heightfield_load = V3DMPListInsert(
			    &model_std->primitive,
			    &model_std->total_primitives,
			    -1,                 /* Append. */
			    V3DMP_TYPE_HEIGHTFIELD_LOAD
			);
			if(mp_heightfield_load != NULL)
			{
			    char **strv; int strc;

			    strv = strexp(strptr2, &strc);

			    mp_heightfield_load->path = ((strc > 0) ?
				STRDUP(strv[0]) : NULL
			    );

			    mp_heightfield_load->x_length = (strc > 1) ?
				ATOF(strv[1]) : 0.0f;
			    mp_heightfield_load->y_length = (strc > 2) ?
				ATOF(strv[2]) : 0.0f;
			    mp_heightfield_load->z_length = (strc > 3) ?
				ATOF(strv[3]) : 0.0f;

			    mp_heightfield_load->x = (strc > 4) ?
				ATOF(strv[4]) : 0.0f;
			    mp_heightfield_load->y = (strc > 5) ?
				ATOF(strv[5]) : 0.0f;
			    mp_heightfield_load->z = (strc > 6) ?
				ATOF(strv[6]) : 0.0f;

			    mp_heightfield_load->heading = DEGTORAD((strc > 7) ?
				ATOF(strv[7]) : 0.0f
			    );
			    mp_heightfield_load->pitch = DEGTORAD((strc > 8) ?
				ATOF(strv[8]) : 0.0f
			    );
			    mp_heightfield_load->bank = DEGTORAD((strc > 9) ?
				ATOF(strv[9]) : 0.0f
			    );

			    strlistfree(strv, strc);
			}
		    }
		    free(strptr2);
		}



		/* Unknown operation. */
		else
		{
		    /* Record this as a comment. */
		    char *strptr2 = NULL, *strptr3;
		    int len;

		    /* Get strptr2 as the new line (coppied). */
		    if(fp != NULL)
		    {
			strptr2 = FGetStringLined(fp); 
		    }
		    else if(buf_line != NULL)   
		    {
			strptr2 = STRDUP(buf_line);
			buf_line = (*buf); buf++;
		    }

		    if((model_std != NULL) && (strptr2 != NULL))
		    {
		        len = STRLEN(strptr) + STRLEN(strptr2) + 8;
			strptr3 = (char *)malloc(len * sizeof(char));
			if(strptr3 != NULL)
			    sprintf(strptr3, "%s %s",
				strptr, strptr2
			    );

/*
Do not end primitives for unknown statement being a comment, in case
the statement occured within a primitive

			DO_END_PRIMITIVES
 */
			if(mp_comment == NULL)
			{
			    mp_comment = V3DMPListInsert(
			        &model_std->primitive,
				&model_std->total_primitives,
			        -1,			/* Append. */
			        V3DMP_TYPE_COMMENT
			    );
			}
			else
			{

			}
			if(mp_comment != NULL)
			{
			    V3DAddLine(
				&(mp_comment->line),
				&(mp_comment->total_lines),
				strptr3
			    );
			}

			free(strptr3);
			strptr3 = NULL;
		    }

		    free(strptr2);
		    strptr2 = NULL;
		}
	    }
	}


	/* Variables fp, buf_line, and buf should be considered
	 * invalid from this point on.
	 */

#undef DO_SEEK_NEXT_LINE
#undef DO_END_PRIMITIVES
#undef IS_NO_VERTEX_PRIMITIVES_ACTIVE
#undef IS_NO_INFINATE_PRIMITIVES_ACTIVE


	/* Parse through each model's other data line and comment
	 * primitives for special commented parameters that only would
	 * have been written by V3DSaveModel().
	 */
	for(i = MAX((*total_models) - models_created, 0);
	    i < (*total_models); i++)
	{
	    int n, j;
	    v3d_model_struct *model_ptr = (*model)[i];
	    if(model_ptr == NULL)
		continue;

	    /* Go through each primitive. */
	    for(n = 0; n < model_ptr->total_primitives; n++)
	    {
		void *p = model_ptr->primitive[n];
		if(p == NULL)
		    continue;  

		/* Is it a primitive of type comment? */
		if((*(int *)p) == V3DMP_TYPE_COMMENT)
		{
		    int line_num;
		    mp_comment = (mp_comment_struct *)p;

		    for(line_num = 0; line_num < mp_comment->total_lines; line_num++)
		    {
			if(V3DHandleComment(
			    mp_comment->line[line_num],
			    NULL, -1,
			    model_ptr, i,
			    p, n
			))
			{
			    /* Was parsed, so remove line from comment
			     * primitive.
			     */
			    mp_comment->total_lines--;
			    free(mp_comment->line[line_num]);
			    for(j = line_num; j < mp_comment->total_lines; j++)
				mp_comment->line[j] = mp_comment->line[j + 1];

			    /* Deincrement line count so current line gets
			     * parsed again since it was shifted.
			     */
			    line_num--;
			}
		    }

		    /* If all lines gone from this comment primitive then
		     * destroy this comment primitive.
		     */
		    if(mp_comment->total_lines <= 0)
		    {
			V3DMPDestroy(p);
			p = NULL;
			model_ptr->total_primitives--;
			for(j = n; j < model_ptr->total_primitives; j++)
			    model_ptr->primitive[j] = model_ptr->primitive[j + 1];

			if(model_ptr->total_primitives > 0)
			{
			    model_ptr->primitive = (void **)realloc(
				model_ptr->primitive,
				model_ptr->total_primitives * sizeof(void *)
			    );
			    if(model_ptr->primitive == NULL)
			    {
				model_ptr->total_primitives = 0;
			    }  
			}
			else
			{
			    free(model_ptr->primitive);
			    model_ptr->primitive = NULL;
			    model_ptr->total_primitives = 0;
			}

			/* Deincrement primitive count so current
			 * primitive gets parsed again since it was shifted.
			 */
			n--;
			continue;
		    }
		}       /* Is it a primitive of type comment? */
		if(p == NULL)
		    continue;

	    }	/* Go through each primitive. */

	    /* Go through each other data line. */
	    for(n = 0; n < model_ptr->total_other_data_lines; n++)
	    {
		strptr = model_ptr->other_data_line[n];
		if(strptr == NULL)
		    continue;

		if(V3DHandleComment(
		    strptr,
		    NULL, -1,
		    model_ptr, i,
		    NULL, -1
		))
		{
		    free(strptr);
		    model_ptr->total_other_data_lines--;

		    for(j = n; j < model_ptr->total_other_data_lines; j++)
			model_ptr->other_data_line[j] =
			    model_ptr->other_data_line[j + 1];
		      
		    if(model_ptr->total_other_data_lines > 0)
		    {
			model_ptr->other_data_line = (char **)realloc(
			    model_ptr->other_data_line,
			    model_ptr->total_other_data_lines * sizeof(char *)
			);
		    }
		    else
		    {
			free(model_ptr->other_data_line);
			model_ptr->other_data_line = NULL;
		    }
		    if(model_ptr->other_data_line == NULL)
			model_ptr->total_other_data_lines = 0;

		    /* Deincrement line count so current line gets
		     * parsed again since it was shifted.
		     */
		    n--;
		    continue;
		}
	    }
	}

	/* Update progress to complete. */
	if(progress_cb != NULL)
	    progress_cb(client_data, file_size, file_size);

	return(0);
}


/*
 *      Saves the model to either the file pointed to by fp or the
 *	lines array pointed to by buf. If saving to buf then buf
 *	will be dynamically allocated (boththe array and each line
 *	string), the last line in buf will be set to NULL.
 *
 *      The given model list will be used as the data to be saved.
 *
 *	Returns -1 on error or the number of lines written.
 */
int V3DSaveModel(
	char ***buf, FILE *fp,
	void **mh_item, int total_mh_items,
	v3d_model_struct **model, int total_models,
	int optimization_level,		/* 0 to V3D_SAVE_FILE_OPTIMIZATION_MAX. */
	int strip_extras,		/* 1 for true, strips extranous data. */
	void *client_data,
	int (*progress_cb)(void *, int, int)
)
{
	int mh_num, model_num, p_num, line_num, v_num;
	int lines_written = 0;
	int texture_state;
	char *strptr;
	v3d_model_struct *model_ptr;

	int last_primitive_type, need_end_primitive;
	void *p;

	mh_comment_struct *mh_comment = NULL;
	mh_version_struct *mh_version = NULL;
	mh_creator_struct *mh_creator = NULL;
	mh_author_struct *mh_author = NULL;
	mh_heightfield_base_directory_struct *mh_hbd = NULL;
	mh_texture_base_directory_struct *mh_tbd = NULL;
	mh_texture_load_struct *mh_texture_load = NULL;
	mh_color_specification_struct *mh_color_spec = NULL;

	mp_vertex_struct *v;

	mp_comment_struct *mp_comment = NULL;
	mp_translate_struct *mp_translate = NULL;  
	mp_untranslate_struct *mp_untranslate = NULL;
	mp_rotate_struct *mp_rotate = NULL;
	mp_unrotate_struct *mp_unrotate = NULL;
	mp_point_struct *mp_point = NULL;
	mp_line_struct *mp_line = NULL;
	mp_line_strip_struct *mp_line_strip = NULL;
	mp_line_loop_struct *mp_line_loop = NULL;
	mp_triangle_struct *mp_triangle = NULL;
	mp_triangle_strip_struct *mp_triangle_strip = NULL;
	mp_triangle_fan_struct *mp_triangle_fan = NULL;
	mp_quad_struct *mp_quad = NULL;
	mp_quad_strip_struct *mp_quad_strip = NULL;
	mp_polygon_struct *mp_polygon = NULL;
	mp_color_struct *mp_color = NULL;
	mp_texture_select_struct *mp_texture_select = NULL;
	mp_texture_orient_xy_struct *mp_texture_xy = NULL;  
	mp_texture_orient_yz_struct *mp_texture_yz = NULL;
	mp_texture_orient_xz_struct *mp_texture_xz = NULL;
	mp_texture_off_struct *mp_texture_off = NULL;
	mp_heightfield_load_struct *mp_hfl = NULL;

	/* Buffer to parse data about to be written, make sure
	 * that out_text_max is big enough to hold any type
	 * of parameter statement and its arguments.
	 */
	char out_text[2048];
	const int out_text_max = 2048;


	if((fp == NULL) &&
	   (buf == NULL)
	)
	    return(-1);

	if((fp != NULL) &&   
	   (buf != NULL)
	)
	{
	    fprintf(
		stderr,
		"V3DSaveModel(): Internal error, both fp and buf not NULL.\n"
	    );
	    return(-1);
	}

/* Writes data to either fp or saves to buf, adding a new line.
 * Variables used are lines_written, out_text, buf, and fp.
 */
#define DO_WRITE_OUT_TEXT	\
{ \
 lines_written++; \
 if(fp != NULL) \
 { \
  fputs(out_text, fp); \
  fputc('\n', fp); \
 } \
 else if(buf != NULL) \
 { \
  (*buf) = (char **)realloc( \
   *buf, \
   lines_written * sizeof(char *) \
  ); \
  if((*buf) == NULL) \
  { \
   lines_written = 0; \
  } \
  else \
  { \
   int tmp_len = STRLEN(out_text); \
   int tmp_line_num = lines_written - 1; \
   char *tmp_line_ptr; \
   tmp_line_ptr = (char *)malloc( \
    (tmp_len + 1) * sizeof(char) \
   ); \
   (*buf)[tmp_line_num] = tmp_line_ptr; \
   if(tmp_line_ptr != NULL) \
   { \
    (*tmp_line_ptr) = '\0'; \
    strcat(tmp_line_ptr, out_text); \
   } \
   else \
   { \
    lines_written--; \
   } \
  } \
 } \
}

/* Similar to DO_WRITE_OUT_TEXT except that it takes a string pointer
 * variable cline_ptr as input.
 */
#define DO_WRITE_CLINE_PTR	\
if(cline_ptr != NULL) \
{ \
 lines_written++; \
 /* Write to file? */ \
 if(fp != NULL) \
 { \
  fputs(cline_ptr, fp); \
  fputc('\n', fp); \
 } \
 /* Write to buffer? */ \
 else if(buf != NULL) \
 { \
  (*buf) = (char **)realloc( \
   *buf, \
   lines_written * sizeof(char *) \
  ); \
  if((*buf) == NULL) \
  { \
   lines_written = 0; \
  } \
  else \
  { \
   int tmp_len = STRLEN(cline_ptr); \
   int tmp_line_num = lines_written - 1; \
   char *tmp_line_ptr; \
   tmp_line_ptr = (char *)malloc( \
    (tmp_len + 1) * sizeof(char) \
   ); \
   (*buf)[tmp_line_num] = tmp_line_ptr; \
   if(tmp_line_ptr != NULL) \
   { \
    (*tmp_line_ptr) = '\0'; \
    strcat(tmp_line_ptr, cline_ptr); \
   } \
   else \
   { \
    lines_written--; \
   } \
  } \
 } \
}

/* Writes the end statement (as needed) for the primitive type specified
 * by variable last_primitive_type.
 */
#define DO_WRITE_END_LAST_PRIMITIVE	\
{ \
 (*out_text) = '\0'; \
 switch(last_primitive_type) \
 { \
  case V3DMP_TYPE_POINT: \
   strcat(out_text, "end_points"); \
   break; \
  case V3DMP_TYPE_LINE: \
   strcat(out_text, "end_lines"); \
   break; \
  case V3DMP_TYPE_LINE_STRIP: \
   strcat(out_text, "end_line_strip"); \
   break; \
  case V3DMP_TYPE_LINE_LOOP: \
   strcat(out_text, "end_line_loop"); \
   break; \
  case V3DMP_TYPE_TRIANGLE: \
   strcat(out_text, "end_triangles"); \
   break; \
  case V3DMP_TYPE_TRIANGLE_STRIP: \
   strcat(out_text, "end_triangle_strip"); \
   break; \
  case V3DMP_TYPE_TRIANGLE_FAN: \
   strcat(out_text, "end_triangle_fan"); \
   break; \
  case V3DMP_TYPE_QUAD: \
   strcat(out_text, "end_quads"); \
   break; \
  case V3DMP_TYPE_QUAD_STRIP: \
   strcat(out_text, "end_quad_strip"); \
   break; \
  case V3DMP_TYPE_POLYGON: \
   strcat(out_text, "end_polygon"); \
   break; \
 } \
 if((*out_text) != '\0') \
 { \
  DO_WRITE_OUT_TEXT \
 } \
}

	/* Reset buffer array if saving to buffer. */
	if(buf != NULL)
	    (*buf) = NULL;

	/* Save header first. */
	if(total_mh_items > 0)
	{
	    strcpy(out_text, "begin_header");
	    DO_WRITE_OUT_TEXT
	}
	for(mh_num = 0; mh_num < total_mh_items; mh_num++)
	{
	    p = mh_item[mh_num];
	    if(p == NULL)
		continue;

	    /* Update progress (from 0.0 to 0.25). */
	    if(progress_cb != NULL)
		progress_cb(client_data, 
		    mh_num, total_mh_items * 4
		);

	    switch(*(int *)p)
	    {
	      case V3DMH_TYPE_COMMENT:
		mh_comment = p;
		if(mh_comment->total_lines > 0)
		{
		    int cline_num;
		    char *cline_ptr;

		    for(cline_num = 0; cline_num < mh_comment->total_lines; cline_num++)
		    {
			cline_ptr = V3DLoadEscapeNewLines(
			    mh_comment->line[cline_num]
			);
			if(cline_ptr == NULL)
			    continue;

			DO_WRITE_CLINE_PTR
			free(cline_ptr);
		    }
		}
		break;

	      case V3DMH_TYPE_VERSION:
		mh_version = p;
		if(1)
		{
		    char num_str[128];

		    strcpy(out_text, "version ");
		    sprintf(num_str, "%i %i",
		    	mh_version->major, mh_version->minor
		    );
		    strcat(out_text, num_str);
		    DO_WRITE_OUT_TEXT
		}
		break;

	      case V3DMH_TYPE_CREATOR:
		mh_creator = p;
		if(mh_creator->creator != NULL)
		{
		    strcpy(out_text, "creator ");
		    strncat(
			out_text,
			mh_creator->creator,
			MAX(out_text_max - 80, 80)
		    );
		    DO_WRITE_OUT_TEXT
		}
		break;

	      case V3DMH_TYPE_AUTHOR:
		mh_author = p;
		if(mh_author->author != NULL)
		{
		    strcpy(out_text, "author ");
		    strncat(
			out_text,
			mh_author->author,
			MAX(out_text_max - 80, 80)
		    );
		    DO_WRITE_OUT_TEXT
		}
		break;

	      case V3DMH_TYPE_HEIGHTFIELD_BASE_DIRECTORY:
		mh_hbd = p;
		if(mh_hbd->path != NULL)
		{
		    strcpy(out_text, "heightfield_base_directory ");
		    strncat(
			out_text,
			mh_hbd->path,
			MAX(out_text_max - 80, 80)
		    );
		    DO_WRITE_OUT_TEXT
		}
		break;

	      case V3DMH_TYPE_TEXTURE_BASE_DIRECTORY:
		mh_tbd = p;
		if(mh_tbd->path != NULL)
		{
		    strcpy(out_text, "texture_base_directory ");
		    strncat(
			out_text,
			mh_tbd->path,
			MAX(out_text_max - 80, 80)
		    );
		    DO_WRITE_OUT_TEXT
		}
		break;

	      case V3DMH_TYPE_TEXTURE_LOAD:
		mh_texture_load = p;
		if(1)
		{
		    char num_str[256];

		    strcpy(out_text, "texture_load ");
		    strncat(
			out_text,
			mh_texture_load->name,
			MAX((out_text_max - 256) / 2, 1)
		    );
		    strcat(out_text, " ");
		    strncat(
			out_text,
			mh_texture_load->path,
			MAX((out_text_max - 256) / 2, 1) 
		    );
		    strcat(out_text, " ");

		    sprintf(num_str, "%f",
			mh_texture_load->priority
		    );
		    strcat(out_text, num_str);

		    DO_WRITE_OUT_TEXT
		}
		break;

	      case V3DMH_TYPE_COLOR_SPECIFICATION:
		mh_color_spec = p;

		if(mh_color_spec->name != NULL)
		{
		    char num_str[256];

		    strcpy(out_text, "color ");
		    strncat(
			out_text,
			mh_color_spec->name,
			MAX(out_text_max - 128, 80)
		    );
		    strcat(out_text, " ");

		    sprintf(num_str, "%f %f %f %f %f %f %f %f %f",
			mh_color_spec->r, mh_color_spec->g,
			mh_color_spec->b, mh_color_spec->a,
			mh_color_spec->ambient, mh_color_spec->diffuse,
			mh_color_spec->specular, mh_color_spec->shininess,
			mh_color_spec->emission
		    );
		    strcat(out_text, num_str);

		    DO_WRITE_OUT_TEXT
		}
		break;
	    }
	}
	if(total_mh_items > 0)
	{
	    strcpy(out_text, "end_header");
	    DO_WRITE_OUT_TEXT
	}


	/* Go through each model. */
	for(model_num = 0; model_num < total_models; model_num++)
	{
	    model_ptr = model[model_num];
	    if(model_ptr == NULL)
		continue;

	    /* Update progress (from 0.25 to 1.0). */
	    if(progress_cb != NULL)
		progress_cb(client_data,
		    (int)((model_num * 0.75) +
			(total_models * 0.25)),
		    total_models
		);

	    /* Handle by model type. */
	    switch(model_ptr->type)
	    {
	      /* **************************************************** */
	      case V3D_MODEL_TYPE_STANDARD:
		/* Begin model statement. */
		sprintf(out_text,
		    "begin_model %s",
		    ((model_ptr->name == NULL) ?
			"untitled" : model_ptr->name
		    )
		);
		DO_WRITE_OUT_TEXT

		/* Reset texture_state; 0 = off, 1 = on, 2 = on and
		 * oriented,
		 */
		texture_state = 0;

		/* Special commented out token parameters for model. */
		sprintf(out_text,
		    "#$model_flags %s",
		    ((model_ptr->flags & V3D_MODEL_FLAG_HIDE) ?
			"hide" : ""
		    )
		);
		DO_WRITE_OUT_TEXT


		/* Reset last primitive type. */
		last_primitive_type = -1;

	        /* Go through each primitive. */
		for(p_num = 0; p_num < model_ptr->total_primitives; p_num++)
		{
		    p = model_ptr->primitive[p_num];
		    if(p == NULL)
			continue;

		    /* Update need_end_primitive mark, checking if the
		     * current primitive type is different from the last
		     * one.
		     */
		    if(last_primitive_type == (*(int *)p))
		    {
			/* Same primitive type. */

			/* Check optimization, if optimization level 1
			 * or higher then we don't need a end primitive
			 * statement. Otherwise, mandatory end primitive
			 * statement needed.
			 */
			if(optimization_level > 0)
			    need_end_primitive = 0;
			else
			    need_end_primitive = 1;
		    }
		    else
		    {
			/* Different primitive types. */
			need_end_primitive = 1;
		    }

		    /* Need to write end last primitive statement? */
		    if(need_end_primitive)
		    {
			DO_WRITE_END_LAST_PRIMITIVE
		    }


		    /* Handle by primitive type. */
		    switch(*(int *)p)
		    {
		      case V3DMP_TYPE_COMMENT:
			mp_comment = p;
			if(mp_comment->total_lines > 0)
			{
			    int cline_num;
			    char *cline_ptr;

			    for(cline_num = 0; cline_num < mp_comment->total_lines; cline_num++)
			    {
				cline_ptr = V3DLoadEscapeNewLines(
				    mp_comment->line[cline_num]
				);
				if(cline_ptr == NULL)
				    continue;

				DO_WRITE_CLINE_PTR
				free(cline_ptr);
			    }
			}
			break;

		      case V3DMP_TYPE_TRANSLATE:
			mp_translate = p;
			sprintf(out_text,
			    "translate %f %f %f",
			    mp_translate->x,
			    mp_translate->y,
			    mp_translate->z
			);
			DO_WRITE_OUT_TEXT
			break;

		      case V3DMP_TYPE_UNTRANSLATE:
			mp_untranslate = p;
			strcpy(out_text, "untranslate");
			DO_WRITE_OUT_TEXT
			break;

		      case V3DMP_TYPE_ROTATE:
			mp_rotate = p;
			sprintf(out_text,
			    "rotate %f %f %f",
			    RADTODEG(mp_rotate->heading),
			    RADTODEG(mp_rotate->pitch),
			    RADTODEG(mp_rotate->bank)
			);
			DO_WRITE_OUT_TEXT
			break;

		      case V3DMP_TYPE_UNROTATE:
			mp_unrotate = p;
			strcpy(out_text, "unrotate");
			DO_WRITE_OUT_TEXT
			break;


		      case V3DMP_TYPE_POINT:
			mp_point = p;
			/* If we wrote an end then we need a start. */
			if(need_end_primitive)
			{
			    strcpy(out_text, "begin_points");
			    DO_WRITE_OUT_TEXT
			}
			for(v_num = 0; v_num < V3DMP_POINT_NVERTEX; v_num++)
			{
			    v = &mp_point->n[v_num];
			    sprintf(out_text, " normal %f %f %f",
				v->x, v->y, v->z
			    );
			    if(optimization_level >= 1)
			    {
			        if((v->x != 0.0) || (v->y != 0.0) || (v->z != 0.0))
				{ DO_WRITE_OUT_TEXT }
			    } else {
				DO_WRITE_OUT_TEXT
			    }

			    if((texture_state == 1) || (optimization_level == 0))
			    {
				v = &mp_point->tc[v_num];
				sprintf(out_text, " texcoord %f %f %f",
				    v->x, v->y, v->z
				);
			        DO_WRITE_OUT_TEXT
			    }
			    v = &mp_point->v[v_num];
			    sprintf(out_text, " %f %f %f",
				v->x, v->y, v->z
			    );
			    DO_WRITE_OUT_TEXT
			}
			break;

		      case V3DMP_TYPE_LINE:
			mp_line = p;
			/* If we wrote an end then we need a start. */
			if(need_end_primitive)
			{
			    strcpy(out_text, "begin_lines");
			    DO_WRITE_OUT_TEXT
			}
			for(v_num = 0; v_num < V3DMP_LINE_NVERTEX; v_num++)
			{
			    v = &mp_line->n[v_num];
			    sprintf(out_text, " normal %f %f %f",
				v->x, v->y, v->z
			    );
			    if(optimization_level >= 1)  
			    {
				if((v->x != 0.0) || (v->y != 0.0) || (v->z != 0.0))
				{ DO_WRITE_OUT_TEXT }
			    } else {
				DO_WRITE_OUT_TEXT
			    }

			    if((texture_state == 1) || (optimization_level == 0))
			    {
				v = &mp_line->tc[v_num];
				sprintf(out_text, " texcoord %f %f %f",
				    v->x, v->y, v->z
				);
				DO_WRITE_OUT_TEXT
			    }
			    v = &mp_line->v[v_num];
			    sprintf(out_text, " %f %f %f",
				v->x, v->y, v->z
			    );
			    DO_WRITE_OUT_TEXT   
			}
			break;

		      case V3DMP_TYPE_LINE_STRIP:
			mp_line_strip = p;
			if(!need_end_primitive)
			{
			    /* Need to write end last primitive
			     * if it's the same primitive type.
			     */
			    DO_WRITE_END_LAST_PRIMITIVE
			}
			strcpy(out_text, "begin_line_strip");
			DO_WRITE_OUT_TEXT

			for(v_num = 0; v_num < mp_line_strip->total; v_num++)
			{
			    v = mp_line_strip->n[v_num];
			    if(v != NULL)
			    {
				sprintf(out_text, " normal %f %f %f",
				    v->x, v->y, v->z
				);
				if(optimization_level >= 1)  
				{
				    if((v->x != 0.0) || (v->y != 0.0) || (v->z != 0.0))
				    { DO_WRITE_OUT_TEXT }
				} else {
				    DO_WRITE_OUT_TEXT
				}
			    }
			    v = mp_line_strip->tc[v_num];
			    if((v != NULL) &&
			       ((texture_state == 1) || (optimization_level == 0))
			    )
			    {
				sprintf(out_text, " texcoord %f %f %f",
				    v->x, v->y, v->z
				);
				DO_WRITE_OUT_TEXT
			    }
			    v = mp_line_strip->v[v_num];
			    if(v != NULL)
			    {
				sprintf(out_text, " %f %f %f",
				    v->x, v->y, v->z
				);
				DO_WRITE_OUT_TEXT
			    }
			}
			break;

		     case V3DMP_TYPE_LINE_LOOP:
			mp_line_loop = p;
			if(!need_end_primitive)
			{
			    /* Need to write end last primitive
			     * if it's the same primitive type.
			     */
			    DO_WRITE_END_LAST_PRIMITIVE
			}
			strcpy(out_text, "begin_line_loop");
			DO_WRITE_OUT_TEXT

			for(v_num = 0; v_num < mp_line_loop->total; v_num++)
			{
			    v = mp_line_loop->n[v_num];
			    if(v != NULL)
			    {
				sprintf(out_text, " normal %f %f %f",
				    v->x, v->y, v->z
				);
				if(optimization_level >= 1)
				{
				    if((v->x != 0.0) || (v->y != 0.0) || (v->z != 0.0))
				    { DO_WRITE_OUT_TEXT }
				} else {
				    DO_WRITE_OUT_TEXT
				}
			    }
			    v = mp_line_loop->tc[v_num];
			    if((v != NULL) &&
			       ((texture_state == 1) || (optimization_level == 0))
			    )
			    {
				sprintf(out_text, " texcoord %f %f %f",
				    v->x, v->y, v->z
				);
				DO_WRITE_OUT_TEXT
			    }
			    v = mp_line_loop->v[v_num];  
			    if(v != NULL)
			    {
				sprintf(out_text, " %f %f %f",
				    v->x, v->y, v->z
				);
				DO_WRITE_OUT_TEXT
			    }
			}
			break;

		     case V3DMP_TYPE_TRIANGLE:
			mp_triangle = p;
			/* If we wrote an end then we need a start. */
			if(need_end_primitive)
			{
			    strcpy(out_text, "begin_triangles");
			    DO_WRITE_OUT_TEXT
			}
			for(v_num = 0; v_num < V3DMP_TRIANGLE_NVERTEX; v_num++)
			{
			    v = &mp_triangle->n[v_num];
			    sprintf(out_text, " normal %f %f %f",
				v->x, v->y, v->z
			    );
			    if(optimization_level >= 1)
			    { 
				if((v->x != 0.0) || (v->y != 0.0) || (v->z != 0.0))
				{ DO_WRITE_OUT_TEXT }
			    } else {
				DO_WRITE_OUT_TEXT
			    }

			    if((texture_state == 1) || (optimization_level == 0))
			    {
				v = &mp_triangle->tc[v_num];
				sprintf(out_text, " texcoord %f %f %f",
				    v->x, v->y, v->z
				);
				DO_WRITE_OUT_TEXT
			    }
			    v = &mp_triangle->v[v_num];
			    sprintf(out_text, " %f %f %f",
				v->x, v->y, v->z
			    );   
			    DO_WRITE_OUT_TEXT
			}
			break;

		     case V3DMP_TYPE_TRIANGLE_STRIP:
			mp_triangle_strip = p;
			if(!need_end_primitive)
			{
			    /* Need to write end last primitive
			     * if it's the same primitive type.
			     */
			    DO_WRITE_END_LAST_PRIMITIVE
			}
			strcpy(out_text, "begin_triangle_strip");
			DO_WRITE_OUT_TEXT

			for(v_num = 0; v_num < mp_triangle_strip->total; v_num++)
			{
			    v = mp_triangle_strip->n[v_num];
			    if(v != NULL)
			    {
				sprintf(out_text, " normal %f %f %f",
				    v->x, v->y, v->z
				);
				if(optimization_level >= 1)
				{
				    if((v->x != 0.0) || (v->y != 0.0) || (v->z != 0.0))
				    { DO_WRITE_OUT_TEXT }
				} else {
				    DO_WRITE_OUT_TEXT
				}
			    }
			    v = mp_triangle_strip->tc[v_num];
			    if((v != NULL) &&
			       ((texture_state == 1) || (optimization_level == 0))
			    ) 
			    {
				sprintf(out_text, " texcoord %f %f %f",
				    v->x, v->y, v->z
				);
				DO_WRITE_OUT_TEXT
			    }
			    v = mp_triangle_strip->v[v_num];
			    if(v != NULL)    
			    {
				sprintf(out_text, " %f %f %f",
				    v->x, v->y, v->z
				);
				DO_WRITE_OUT_TEXT
			    }
			}
			break;

		     case V3DMP_TYPE_TRIANGLE_FAN:
			mp_triangle_fan = p;
			if(!need_end_primitive)
			{
			    /* Need to write end last primitive
			     * if it's the same primitive type.
			     */
			    DO_WRITE_END_LAST_PRIMITIVE
			}
			strcpy(out_text, "begin_triangle_fan");
			DO_WRITE_OUT_TEXT

			for(v_num = 0; v_num < mp_triangle_fan->total; v_num++)
			{
			    v = mp_triangle_fan->n[v_num];
			    if(v != NULL)
			    {
				sprintf(out_text, " normal %f %f %f",
				    v->x, v->y, v->z
				);
				if(optimization_level >= 1)
				{
				    if((v->x != 0.0) || (v->y != 0.0) || (v->z != 0.0))
				    { DO_WRITE_OUT_TEXT }
				} else {
				    DO_WRITE_OUT_TEXT
				}
			    }
			    v = mp_triangle_fan->tc[v_num];
			    if((v != NULL) &&
			       ((texture_state == 1) || (optimization_level == 0))
			    ) 
			    {
				sprintf(out_text, " texcoord %f %f %f",
				    v->x, v->y, v->z
				);
				DO_WRITE_OUT_TEXT
			    }
			    v = mp_triangle_fan->v[v_num];
			    if(v != NULL)
			    {
				sprintf(out_text, " %f %f %f",
				    v->x, v->y, v->z
				);   
				DO_WRITE_OUT_TEXT
			    }
			}
			break;

		     case V3DMP_TYPE_QUAD:
			mp_quad = p;
			/* If we wrote an end then we need a start. */
			if(need_end_primitive)
			{ 
			    strcpy(out_text, "begin_quads");
			    DO_WRITE_OUT_TEXT
			}
			for(v_num = 0; v_num < V3DMP_QUAD_NVERTEX; v_num++)
			{
			    v = &mp_quad->n[v_num];
			    sprintf(out_text, " normal %f %f %f",
				v->x, v->y, v->z
			    );
			    if(optimization_level >= 1)
			    { 
				if((v->x != 0.0) || (v->y != 0.0) || (v->z != 0.0))
				{ DO_WRITE_OUT_TEXT }
			    } else {
				DO_WRITE_OUT_TEXT
			    }

			    if((texture_state == 1) || (optimization_level == 0))
			    {
				v = &mp_quad->tc[v_num];
				sprintf(out_text, " texcoord %f %f %f",
				    v->x, v->y, v->z
				);
				DO_WRITE_OUT_TEXT
			    }
			    v = &mp_quad->v[v_num];
			    sprintf(out_text, " %f %f %f",
				v->x, v->y, v->z
			    );
			    DO_WRITE_OUT_TEXT
			}
			break;

		      case V3DMP_TYPE_QUAD_STRIP:
			mp_quad_strip = p;
			if(!need_end_primitive)
			{
			    /* Need to write end last primitive
			     * if it's the same primitive type.
			     */
			    DO_WRITE_END_LAST_PRIMITIVE
			}
			strcpy(out_text, "begin_quad_strip");
			DO_WRITE_OUT_TEXT

			for(v_num = 0; v_num < mp_quad_strip->total; v_num++)
			{
			    v = mp_quad_strip->n[v_num];
			    if(v != NULL)
			    {
				sprintf(out_text, " normal %f %f %f",
				    v->x, v->y, v->z
				);
				if(optimization_level >= 1)
				{
				    if((v->x != 0.0) || (v->y != 0.0) || (v->z != 0.0))
				    { DO_WRITE_OUT_TEXT }
				} else {
				    DO_WRITE_OUT_TEXT
				}
			    }
			    v = mp_quad_strip->tc[v_num];
			    if((v != NULL) &&
			       ((texture_state == 1) || (optimization_level == 0))
			    ) 
			    {
				sprintf(out_text, " texcoord %f %f %f",
				    v->x, v->y, v->z
				);
				DO_WRITE_OUT_TEXT
			    }
			    v = mp_quad_strip->v[v_num];   
			    if(v != NULL)   
			    {
				sprintf(out_text, " %f %f %f",
				    v->x, v->y, v->z
				);
				DO_WRITE_OUT_TEXT
			    }
			}
			break;

		      case V3DMP_TYPE_POLYGON:
			mp_polygon = p;
			if(!need_end_primitive)
			{
			    /* Need to write end last primitive
			     * if it's the same primitive type.
			     */
			    DO_WRITE_END_LAST_PRIMITIVE
			}
			strcpy(out_text, "begin_polygon");
			DO_WRITE_OUT_TEXT

			for(v_num = 0; v_num < mp_polygon->total; v_num++)
			{
			    v = mp_polygon->n[v_num];
			    if(v != NULL)
			    {
				sprintf(out_text, " normal %f %f %f",
				    v->x, v->y, v->z
				);
				if(optimization_level >= 1)
				{
				    if((v->x != 0.0) || (v->y != 0.0) || (v->z != 0.0))
				    { DO_WRITE_OUT_TEXT }
				} else {
				    DO_WRITE_OUT_TEXT
				}
			    }
			    v = mp_polygon->tc[v_num];
			    if((v != NULL) &&
			       ((texture_state == 1) || (optimization_level == 0))
			    )
			    {
				sprintf(out_text, " texcoord %f %f %f",
				    v->x, v->y, v->z
				);
				DO_WRITE_OUT_TEXT
			    }
			    v = mp_polygon->v[v_num];
			    if(v != NULL)
			    {
				sprintf(out_text, " %f %f %f",
				    v->x, v->y, v->z
				);   
				DO_WRITE_OUT_TEXT
			    }
			}
			break;


		      case V3DMP_TYPE_COLOR:
			mp_color = p;
			sprintf(out_text, "color %f %f %f %f %f %f %f %f %f",
			    mp_color->r, mp_color->g, mp_color->b, mp_color->a,
			    mp_color->ambient, mp_color->diffuse,
			    mp_color->specular, mp_color->shininess,
			    mp_color->emission
			);
			DO_WRITE_OUT_TEXT
			break;

		      case V3DMP_TYPE_TEXTURE_SELECT:
			mp_texture_select = p;
			strptr = mp_texture_select->name;
			if(mp_texture_select->name != NULL)
			{
			    strcpy(out_text, "texture_select ");
			    strncat(out_text, strptr, 256);
			    out_text[out_text_max - 1] = '\0';
			    DO_WRITE_OUT_TEXT

			    texture_state = 1;
			}
			break;

		      case V3DMP_TYPE_TEXTURE_ORIENT_XY:
			mp_texture_xy = p;
			sprintf(out_text, "texture_orient_xy %f %f %f %f",
			    mp_texture_xy->x, mp_texture_xy->y,
			    mp_texture_xy->dx, mp_texture_xy->dy
			);
			DO_WRITE_OUT_TEXT

			texture_state = 2;
			break;

		      case V3DMP_TYPE_TEXTURE_ORIENT_YZ:
			mp_texture_yz = p;
			sprintf(out_text, "texture_orient_yz %f %f %f %f",
			    mp_texture_yz->y, mp_texture_yz->z,
			    mp_texture_yz->dy, mp_texture_yz->dz
			);
			DO_WRITE_OUT_TEXT

			texture_state = 2;
			break;

		      case V3DMP_TYPE_TEXTURE_ORIENT_XZ:
			mp_texture_xz = p;
			sprintf(out_text, "texture_orient_xz %f %f %f %f",
			    mp_texture_xz->x, mp_texture_xz->z,
			    mp_texture_xz->dx, mp_texture_xz->dz
			);
			DO_WRITE_OUT_TEXT

			texture_state = 2;
			break;

		      case V3DMP_TYPE_TEXTURE_OFF:
			mp_texture_off = p;
			strcpy(out_text, "texture_off");
			DO_WRITE_OUT_TEXT

			texture_state = 0;
			break;

		      case V3DMP_TYPE_HEIGHTFIELD_LOAD:
			mp_hfl = p;
			if(mp_hfl->path != NULL)
			{
			    char num_str[256];

			    strcpy(out_text, "heightfield_load ");
			    strncat(
				out_text,
				mp_hfl->path,
				MAX(out_text_max - 256, 80)
			    );
			    sprintf(num_str, " %f %f %f %f %f %f %f %f %f",
				mp_hfl->x_length, mp_hfl->y_length, mp_hfl->z_length,
				mp_hfl->x, mp_hfl->y, mp_hfl->z,
				RADTODEG(mp_hfl->heading),
				RADTODEG(mp_hfl->pitch),
				RADTODEG(mp_hfl->bank)
			    );
			    strcat(out_text, num_str);
			    DO_WRITE_OUT_TEXT
			}
			break;
		    }

		    /* Update last primitive type. */
		    last_primitive_type = (*(int *)p);
		}	/* Go through each primitive. */

		/* Write end primitive statement (as needed). */
		DO_WRITE_END_LAST_PRIMITIVE


		/* End model statement. */
		sprintf(out_text,
		    "end_model %s",
		    ((model_ptr->name == NULL) ?
			"untitled" : model_ptr->name
		    )
		);
		DO_WRITE_OUT_TEXT

		break;

	      /* **************************************************** */
	      case V3D_MODEL_TYPE_OTHER_DATA:
		/* Go through each line. */
		for(line_num = 0; line_num < model_ptr->total_other_data_lines; line_num++)
		{
		    char *cline_ptr = V3DLoadEscapeNewLines(
			model_ptr->other_data_line[line_num]
		    );
		    if(cline_ptr == NULL)
			continue;

		    DO_WRITE_CLINE_PTR
		    free(cline_ptr);
		}
		break;

	      /* **************************************************** */
	      default:
		fprintf(stderr,
 "V3DSaveModel(): Cannot save model of unknown type `%i'\n",
		    model_ptr->type
		);
		break;

	    }	/* Handle by model type. */

	}	/* Go through each model. */



	/* If saving to buffer, add one more line that is NULL. */
	if(buf != NULL)
	{
	    (*buf) = (char **)realloc(
		*buf,
		(lines_written + 1) * sizeof(char *)
	    );
	    if((*buf) == NULL)
	    {
		lines_written = 0;
	    }
	    else
	    {
		(*buf)[lines_written] = NULL;
	    }
	}

	/* Update progress to completion. */
	if(progress_cb != NULL)
	    progress_cb(client_data, 1, 1);

#undef DO_WRITE_OUT_TEXT
#undef DO_WRITE_END_LAST_PRIMITIVE


	return(lines_written);
}
