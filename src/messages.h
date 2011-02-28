#ifndef MESSAGES_H
#define MESSAGES_H

#include "obj.h"

/* Regular messages list handling. */
extern void SARMessageAdd(sar_scene_struct *scene, const char *message);
extern void SARMessageClearAll(sar_scene_struct *scene);

/* Sticky banner message handling. */
extern void SARBannerMessageAppend(sar_scene_struct *scene, const char *message);

/* Camera ref title handling. */
extern void SARCameraRefTitleSet(sar_scene_struct *scene, const char *message);


#endif	/* MESSAGES_H */
