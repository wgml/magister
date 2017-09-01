#include "config.h"

void init_config(struct video_config *config, int video_width, int video_height) {
	fdebug("Configuring stream with width = %d and height = %d.", video_width, video_height);
	config->width = video_width;
	config->height = video_height;
	config->pixel_size = 3;

	config->framebuffer_length = config->width * config->height * config->pixel_size;
}
