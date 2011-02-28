#ifndef HORISON_H
#define HORIZON_H

#include "v3dtex.h"
#include "obj.h"


extern v3d_texture_ref_struct *SARCreateHorizonTexture(
        const char *name,
	const sar_color_struct *start_color,	/* Top color */
	const sar_color_struct *end_color,	/* Bottom color */
        int height,
	float midpoint
);


#endif	/* HORIZON_H */
