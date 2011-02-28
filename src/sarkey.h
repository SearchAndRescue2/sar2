#ifndef SARKEY_H
#define SARKEY_H

#include "gw.h"
#include "sar.h"

extern void SARKey(
	sar_core_struct *core_ptr,
	int c,          /* GW Key value or character. */
	Boolean state,  /* Pressed (True) or released (False). */
	long t          /* Time stamp. */
);

#endif	/* SARKEY_H */
