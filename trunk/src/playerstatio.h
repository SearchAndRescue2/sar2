#ifndef PLAYERSTATIO_H
#define PLAYERSTATIO_H

#include "sar.h"

extern int SARPlayerStatsLoadFromFile(
	sar_player_stat_struct ***list, int *total,
	const char *filename
);
extern int SARPlayerStatsSaveToFile(
	sar_player_stat_struct **list, int total,
	const char *filename
);

#endif	/* PLAYERSTATIO_H */
