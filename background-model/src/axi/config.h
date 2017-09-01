#ifndef SRC_CONFIG_H_
#define SRC_CONFIG_H_

#include <stddef.h>
#include "../debug.h"

struct video_config {
	struct {
		int width;
		int height;
		size_t pixel_size;
		size_t framebuffer_length;
	} video;

	struct {
		unsigned char bg_th;
		unsigned char fd_th;
		double alpha;
	} algo;
};

void init_config(struct video_config *config, int video_width, int video_height,
		double alpha, unsigned char bg_th, unsigned char fd_th);

#endif /* SRC_CONFIG_H_ */
