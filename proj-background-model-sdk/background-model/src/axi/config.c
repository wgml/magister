#include "config.h"

void init_config(struct video_config *config, int video_width, int video_height,
		double alpha, unsigned char bg_th, unsigned char fd_th) {
	fdebug("Configuring stream with width = %d and height = %d.", video_width, video_height);
	config->video.width = video_width;
	config->video.height = video_height;
	config->video.pixel_size = 3;

	config->video.framebuffer_length = config->video.width * config->video.height * config->video.pixel_size;

	config->algo.alpha = alpha;
	config->algo.bg_th = bg_th;
	config->algo.fd_th = fd_th;
}
