#ifndef _DRM_NOTIFIER_MI_H_
#define _DRM_NOTIFIER_MI_H_

#include <linux/notifier.h>

#define MI_DRM_EARLY_EVENT_BLANK	0x01
#define MI_DRM_EVENT_BLANK		0x02
#define MI_DRM_PRE_EVENT_BLANK		0x03
#define MI_DRM_BLANK_UNBLANK		0
#define MI_DRM_BLANK_POWERDOWN		1

struct mi_drm_notifier {
	struct notifier_block notifier;
	void *data;
	int id;
};

static inline int mi_drm_register_client(struct mi_drm_notifier *nb) { return 0; }
static inline int mi_drm_unregister_client(struct mi_drm_notifier *nb) { return 0; }

#endif
