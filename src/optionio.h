#ifndef OPTIONIO_H
#define OPTIONIO_H

#include "sar.h"

extern int SAROptionsLoadFromFile(
	sar_option_struct *opt, const char *filename
);
extern int SAROptionsSaveToFile(
	const sar_option_struct *opt, const char *filename
);

#endif	/* OPTIONIO_H */
