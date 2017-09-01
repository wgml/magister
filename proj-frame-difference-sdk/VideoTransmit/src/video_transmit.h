#ifndef SRC_VIDEO_TRANSMIT_H_
#define SRC_VIDEO_TRANSMIT_H_

#include <fcntl.h>
#include <stdlib.h>

#include "axi/axi.h"
#include "axi/config.h"
#include "axi/hpd.h"
#include "axi/processing.h"
#include "axi/vdma.h"
#include "debug.h"
#include "memory.h"

struct video_transmit {
	struct axi_vdma *vdma;
	struct axi_interface *hpd;
	struct axi_interface *processing;

	struct video_config *config;

	memory_handle_t memory_handle;
};

struct video_transmit* setup(unsigned int vdma_base_addr, unsigned int hpd_base_addr, unsigned int threshold_base_addr);
void initialize(struct video_transmit *video_transmit, int video_width, int video_height);
void cleanup(struct video_transmit *video_transmit);

void debug_video(const struct video_transmit video_transmit);

#endif /* SRC_VIDEO_TRANSMIT_H_ */
