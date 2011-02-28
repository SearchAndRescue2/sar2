#ifndef SARTIME_H
#define SARTIME_H

#include <sys/types.h>

extern time_t SARGetCurMilliTime(void);

extern unsigned int SARRandom(unsigned int seed_offset);
extern float SARRandomCoeff(unsigned int seed_offset);

extern int SARParseTimeOfDay(
	const char *string,
	int *h, int *m, int *s
);
extern int SARParseLatitudeDMS(const char *string, float *dms);
extern int SARParseLongitudeDMS(const char *string, float *dms);

#endif	/* SARTIME_H */
