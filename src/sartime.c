#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <sys/types.h>
#ifdef __MSW__
# include <windows.h>
#else
# include <sys/time.h>
#endif

#include "../include/strexp.h"
#include "../include/string.h"

#include "sartime.h"

time_t SARGetCurMilliTime(void);
unsigned int SARRandom(unsigned int seed_offset);
float SARRandomCoeff(unsigned int seed_offset);

int SARParseTimeOfDay(
	const char *string,
	int *h, int *m, int *s
);
int SARParseLatitudeDMS(const char *string, float *dms);
int SARParseLongitudeDMS(const char *string, float *dms);


#define ATOI(s)         (((s) != NULL) ? atoi(s) : 0)
#define ATOL(s)         (((s) != NULL) ? atol(s) : 0)
#define ATOF(s)         (((s) != NULL) ? atof(s) : 0.0f)
#define STRDUP(s)       (((s) != NULL) ? strdup(s) : NULL)

#define MAX(a,b)        (((a) > (b)) ? (a) : (b))
#define MIN(a,b)        (((a) < (b)) ? (a) : (b))
#define CLIP(a,l,h)     (MIN(MAX((a),(l)),(h)))
#define IS_STRING_EMPTY(s)      (((s) != NULL) ? (*(s) == '\0') : TRUE)

#define RADTODEG(r)     ((r) * 180 / PI)
#define DEGTORAD(d)     ((d) * PI / 180)


/*
 *      Returns the current time since midnight in milliseconds from the
 *      system.
 */
time_t SARGetCurMilliTime(void)
{
#ifdef __MSW__
	SYSTEMTIME t;   /* Current time of day structure. */

	GetSystemTime(&t);
	return(
	    (time_t)(
		(((((t.wHour * 60.0) + t.wMinute) * 60) + t.wSecond) * 1000) +
		t.wMilliseconds
	    )
	);
#else
	struct timeval tv[1];

	if(gettimeofday(tv, NULL) < 0)
	    return(-1);

	return(((tv->tv_sec % 86400) * 1000) + (tv->tv_usec / 1000));
#endif
}

/*
 *      Returns a random number in the range of 0 to 0xffffffff.
 *
 *      If you call this function more than once per cycle then you
 *      should pass seed_offset as a unique value each time to better
 *      obtain a more random number.
 */
unsigned int SARRandom(unsigned int seed_offset)
{
#if defined(RAND_MAX)
/*	srand((unsigned int)cur_millitime + seed_offset); */
	return(rand());
#else
	static u_int32_t gSeed = 1234145;
#ifdef __MSW__
	long result = gSeed * (long)1103515245 + 12345;
#else
	int64_t result = gSeed * (int64_t)1103515245 + 12345;
#endif
	gSeed = (u_int32_t)result;

	return((u_int32_t)(result >> 16));
#endif
}

/*
 *      Returns a random coefficient from 0.0 to 1.0, setting the seed
 *      as the current time in milliseconds plus the given seed_offset.
 *
 *      If you call this function more than once per cycle then you
 *      should pass seed_offset as a unique value each time to better
 *      obtain a more random number.
 */
float SARRandomCoeff(unsigned int seed_offset)
{
#if defined(RAND_MAX)
/*	srand((unsigned int)cur_millitime + seed_offset); */
	return((float)rand() / (float)RAND_MAX);
#else
	return((float)SARRandom(seed_offset) / (float)0xffffffff);
#endif
}


/*
 *      Parses the given string which should be in the format
 *      "h:m:s". Two available formats 24 and 12 hour are supported,
 *      where the 12 hour is checked for by searching for an in
 *      string 'p' character (case insensitive).
 *
 *      String is terminated by a null or blank character.
 *
 *      Returns non-zero if parsing failed.
 */
int SARParseTimeOfDay(
	const char *string,
	int *h, int *m, int *s
)
{
	const char *sp;
	int in_len, is_12hourpm = 0;

	int lstring_len = 80;
	char *sp2, *sp3;
	char lstring[80];

	/* Reset inputs. */
	if(h != NULL)
	    (*h) = 0;
	if(m != NULL)
	    (*m) = 0;
	if(s != NULL)
	    (*s) = 0;

	if((string == NULL) ? 1 : ((*string) == '\0'))
	    return(-1);

	/* Check if this is 12 or 24 hour format. */
	sp = string;
	in_len = 0;
	while(((*sp) != '\0') && !ISBLANK(*sp))
	{
	    if(toupper(*sp) == 'P')
		is_12hourpm = 1;

	    sp++;
	    in_len++;
	}

	/* Copy input string to local string. */
	if(in_len > 0)
	    strncpy(
		lstring, string, MIN(in_len, lstring_len)
	    );
	else
	    (*lstring) = '\0';
	lstring[MIN(in_len, lstring_len - 1)] = '\0';

	/* Fetch hour. */
	sp2 = lstring;
	sp3 = strchr(sp2, ':');
	if(sp3 != NULL)
	{
	    (*sp3) = '\0';
	    if(h != NULL)
		*h = (ATOI(sp2) + ((is_12hourpm) ? + 12 : 0)) % 24;
	    sp2 = sp3 + 1;
	}
	else
	{
	    if(h != NULL)
		*h = (ATOI(sp2) + ((is_12hourpm) ? + 12 : 0)) % 24;
	    sp2 = NULL;
	}
	if(sp2 == NULL)
	    return(0);

	/* Fetch minute. */
	sp3 = strchr(sp2, ':');
	if(sp3 != NULL)
	{
	    *sp3 = '\0';
	    if(m != NULL)
		*m = ATOI(sp2) % 60;
	    sp2 = sp3 + 1;
	}
	else
	{
	    if(m != NULL)
		*m = ATOI(sp2) % 60;
	    sp2 = NULL;
	}
	if(sp2 == NULL)
	    return(0);

	/* Fetch seconds. */
	sp3 = strchr(sp2, ':');
	if(sp3 != NULL)
	{
	    *sp3 = '\0';
	    if(s != NULL)
		*s = ATOI(sp2) % 60;
	}
	else
	{
	    if(s != NULL)
		*s = ATOI(sp2) % 60;
	}

	return(0);
}

/*
 *      Parses the given string which should be in the format
 *      "d'm"s" or "d". Examples (respective) "34'10"59" or "34.1833".
 *
 *      For format "d" a decimal value from .000 to .999 can be used
 *      to represent decimal degrees.
 *
 *      Degrees is bounded by -90.0 and 90.0.
 *
 *      String is terminated by a null or blank character.
 *
 *      Returns non-zero if parsing failed.
 */
int SARParseLatitudeDMS(const char *string, float *dms)
{
	const char *sp;
	int in_len, is_traditional = 0, is_south = 0;

	int lstring_len = 80;
	char *sp2, *sp3;
	char lstring[80];

	/* Reset inputs. */
	if(dms != NULL)
	    *dms = 0.0f;

	if((string != NULL) ? (*string == '\0') : 1)
	    return(-1);

	/* Check if this is in decimal or traditional notation. */
	sp = string;
	in_len = 0;
	while((*sp != '\0') && !ISBLANK(*sp))
	{
	    if(toupper(*sp) == '\'')
		is_traditional = 1;

	    if(toupper(*sp) == 'S')
		is_south = 1;

	    sp++;
	    in_len++;
	}

	/* Copy input string to local string. */
	if(in_len > 0)
	    strncpy(
		lstring, string, MIN(in_len, lstring_len)
	    );
	else
	    *lstring = '\0';
	lstring[MIN(in_len, lstring_len - 1)] = '\0';

	/* Handle by type. */
	if(is_traditional)
	{
	    /* Traditional notation. */
	    int d = 0, m = 0, s = 0;

	    /* Fetch degrees. */
	    sp2 = lstring;
	    if(sp2 != NULL)
	    {
		sp3 = strchr(sp2, '\'');
		if(sp3 != NULL)
		{
		    *sp3 = '\0';
		    d = ATOI(sp2);
		    sp2 = sp3 + 1;
		}
		else
		{
		    d = ATOI(sp2);
		    sp2 = NULL;
		}
	    }

	    /* Fetch minutes. */
	    if(sp2 != NULL)
	    {
		sp3 = strchr(sp2, '"');
		if(sp3 != NULL)
		{
		    *sp3 = '\0';
		    m = ATOI(sp2);
		    sp2 = sp3 + 1;
		}
		else
		{
		    m = ATOI(sp2);
		    sp2 = NULL;
		}
	    }

	    /* Fetch seconds. */
	    if(sp2 != NULL)
	    {
		sp3 = strchr(sp2, '"');
		if(sp3 != NULL)
		{
		    *sp3 = '\0';
		    s = ATOI(sp2);
		    sp2 = sp3 + 1;
		}
		else
		{
		    s = ATOI(sp2);
		    sp2 = NULL;
		}
	    }

	    /* Convert and add to dms return. */
	    if(dms != NULL)
	    {
		/* Negative? */
		if(is_south && (d > 0.0f))
		    *dms = (float)-d -
			((float)m / 60.0f) - ((float)s / 60.0f / 100.0f);
		else if(d < 0)
		    *dms = (float)d -
			((float)m / 60.0f) - ((float)s / 60.0f / 100.0f);
		else
		    *dms = (float)d +
			((float)m / 60.0f) + ((float)s / 60.0f / 100.0f);
	    }
	}
	else
	{
	    /* Decimal notation (this is easy). */
	    sp2 = lstring;
	    if(dms != NULL)
	    {
		float d = (float)ATOF(sp2);
		if(is_south && (d > 0.0f))
		    *dms = -d;
		else
		    *dms = d;
	    }
	}

	return(0);
}

/*
 *      Parses the given string which should be in the format
 *      "d'm"s" or "d". Examples (respective) "34'10"59" or "34.1833".
 *
 *      For format "d" a decimal value from .000 to .999 can be used
 *      to represent decimal degrees.
 *
 *      Degrees is bounded by -140.0 and 140.0.
 *
 *      String is terminated by a null or blank character.
 *
 *      Returns non-zero if parsing failed.
 */
int SARParseLongitudeDMS(const char *string, float *dms)
{
	const char *sp;
	int in_len, is_traditional = 0, is_west = 0;

	int lstring_len = 80;
	char *sp2, *sp3;
	char lstring[80];

	/* Reset inputs. */
	if(dms != NULL)
	    *dms = 0.0f;

	if((string != NULL) ? (*string == '\0') : 1)
	    return(-1);

	/* Check if this is in decimal or traditional notation. */
	sp = string;
	in_len = 0;
	while((*sp != '\0') && !ISBLANK(*sp))
	{
	    if(toupper(*sp) == '\'')
		is_traditional = 1;

	    if(toupper(*sp) == 'W')
		is_west = 1;

	    sp++;
	    in_len++;
	}

	/* Copy input string to local string. */
	if(in_len > 0)
	    strncpy(
		lstring, string, MIN(in_len, lstring_len)
	    );
	else
	    *lstring = '\0';
	lstring[MIN(in_len, lstring_len - 1)] = '\0';

	/* Handle by type. */
	if(is_traditional)
	{
	    /* Traditional notation. */
	    int d = 0, m = 0, s = 0;

	    /* Fetch degrees. */
	    sp2 = lstring;
	    if(sp2 != NULL)
	    {
		sp3 = strchr(sp2, '\'');
		if(sp3 != NULL)
		{
		    *sp3 = '\0';
		    d = ATOI(sp2);
		    sp2 = sp3 + 1;
		}
		else
		{
		    d = ATOI(sp2);
		    sp2 = NULL;
		}
	    }

	    /* Fetch minutes. */
	    if(sp2 != NULL)
	    {
		sp3 = strchr(sp2, '"');
		if(sp3 != NULL)
		{
		    *sp3 = '\0';
		    m = ATOI(sp2);
		    sp2 = sp3 + 1;
		}
		else
		{
		    m = ATOI(sp2);
		    sp2 = NULL;
		}
	    }

	    /* Fetch seconds. */
	    if(sp2 != NULL)
	    {
		sp3 = strchr(sp2, '"');
		if(sp3 != NULL)
		{
		    *sp3 = '\0';
		    s = ATOI(sp2);
		    sp2 = sp3 + 1;
		}
		else
		{
		    s = ATOI(sp2);
		    sp2 = NULL;
		}
	    }

	    /* Convert and add to dms return. */
	    if(dms != NULL)
	    {
		/* Negative? */
		if(is_west && (d > 0))
		    *dms = (float)-d -
			((float)m / 60.0f) - ((float)s / 60.0f / 100.0f);
		else if(d < 0)
		    *dms = (float)d -
			((float)m / 60.0f) - ((float)s / 60.0f / 100.0f);
		else
		    *dms = (float)d +
			((float)m / 60.0f) + ((float)s / 60.0f / 100.0f);
	    }
	}
	else
	{
	    /* Decimal notation (this is easy). */
	    sp2 = lstring;
	    if(dms != NULL)
	    {
		float d = (float)ATOF(sp2);
		if(is_west && (d > 0.0f))
		    *dms = -d;
		else
		    *dms = d;
	    }
	}

	return(0);
}

