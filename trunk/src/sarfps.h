/*
                    SAR Frames Per Second Tallying
 */

#ifndef SARFPS_H
#define SARFPS_H

#include <sys/stat.h>

/*
 *	Frame counter structure for the sar_fps_struct:
 */
typedef struct {

	/* Number of frames drawn. */
	unsigned int	selection,		/* Selection buffer. */
			frame,			/* Frame buffer. */
			total;

} sar_fps_frames_struct;

/*
 *	Frames per second tallying structure:
 */
typedef struct {

	void *core_ptr;		/* Pointer back to core. */
	void *gw_ptr;		/* Pointer back to graphics wrapper structure
				 * so we know which window we're taking
				 * fps stats for.
				 */

	/* Frames results from last tally. */
	sar_fps_frames_struct ft;

	/* Frames currently being counted. */
	sar_fps_frames_struct fc;

	time_t last_frames_reset;	/* Last time frames reset to 0 in
					 * milliseconds.
					 */
	time_t next_tally;		/* Always last_frames_reset + 1000
					 * milliseconds.
					 */

} sar_fps_struct;

#endif	/* SARFPS_H */
