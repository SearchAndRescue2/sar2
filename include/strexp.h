#ifndef _STREXP_H_
#define _STREXP_H_

#ifdef __cplusplus
extern "C" {
#endif


#include <stdlib.h>
#include <ctype.h>

/* strexp - expand a string into its substrings by whitespace */
extern char **strexp(const char *str, int *n);

/* strchrexp - expand a string into fields (c = designator) */
extern char **strchrexp(const char *str, char c, int *n);


#ifdef __cplusplus
}
#endif

#endif
