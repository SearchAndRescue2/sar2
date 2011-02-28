#include <string.h>
#include "../include/string.h"
#include "../include/strexp.h"

#ifdef MEMWATCH
# include "memwatch.h"
#endif


char **strexp(const char *str, int *n);
char **strchrexp(const char *str, char c, int *n);


/*
 *	Returns a dynamically allocated array of strings exploded from
 *	the delimiting periods of white space characters in the given
 *	string str. Value of n will be updated to the number of strings
 *	returned.
 *
 *	Calling function must deallocate the returned strings and the
 *	array.
 *
 *	Example:
 *
 *		"ball rock  hood"
 *
 *	Will be exploded into:
 *
 *		"ball" "rock" "hood"
 */
char **strexp(const char *str, int *n)
{
	char **ret;
	const char *head, *tail;
	int num;
	int len;


	/* return NULL if given an empty string */
	if(!str)
		return(NULL);

	num = 0;
	head = tail = str;
	ret = NULL;

	/* iterate while we're inside the string */
	while(*head)
	{
		/* skip to next non-blank character */
		while(ISBLANK(*head))
			head++;

		/* find next blank character */
		tail = head;
		while(*tail && !ISBLANK(*tail))
			tail++;

		/* determine length of substring */
		len = (tail - head);

		/* add string to list */
		num++;
		ret = (char **)realloc(ret, num * sizeof(char *));
		ret[num - 1] = (char *)malloc(len + 1);

		/* copy string over and null-terminate */
		strncpy(ret[num - 1], head, len);
		ret[num - 1][len] = '\0';

		/* get ready for next substring */
		head = tail;
	}

	/* we're done... */
	(*n) = num;
	return ret;
}

/*
 *      Returns a dynamically allocated array of strings exploded from
 *      each delimiating character c in the given string str. Value of
 *      n will be updated to the number of strings returned.
 *
 *      Calling function must deallocate the returned strings and the
 *      array.
 *
 *	Example (if c == ':'):
 *
 *              "ball:rock :hood::cat"
 *
 *      Will be exploded into:
 * 
 *              "ball" "rock " "hood" "" "cat"
 */
char **strchrexp(const char *str, char c, int *n)
{
	char **ret;
	const char *head, *tail;
	int num;
	int len;


	/* return NULL if given an empty string */
	if(!str)
		return(NULL);

	num = 0;
	head = tail = str;
	ret = NULL;

	/* Iterate while we're inside the string */
	while((*head) && (*tail))
	{
		/* Seek tail from head to next occurance of c or end. */
		tail = head;
		while((*tail) && ((*tail) != c))
			tail++;

		/* Calculate length of substring. */
		len = (tail - head);

		/* Add string to list. */
		num++;
		ret = (char **)realloc(ret, num * sizeof(char *));
		ret[num - 1] = (char *)malloc((len + 1) * sizeof(char));

		/* copy string over and null-terminate */
		strncpy(ret[num - 1], head, len);
		ret[num - 1][len] = '\0';

		/* Seek head past tail or to end of string. */
		head = tail + ((*tail) ? 1 : 0);
	}

	/* we're done... */
	(*n) = num;
	return ret;
}




