#ifndef SRC_CONFIG_H_
#define SRC_CONFIG_H_

#include <stddef.h>
#include "../debug.h"

struct video_config {
	int width;
	int height;
	size_t pixel_size;
	size_t framebuffer_length;
};

void init_config(struct video_config *config, int video_width, int video_height);

#endif /* SRC_CONFIG_H_ */
