#ifndef SRC_VIDEO_TRANSMIT_H_
#define SRC_VIDEO_TRANSMIT_H_

#include <fcntl.h>
#include <stdlib.h>

#include "axi/axi.h"
#include "axi/config.h"
#include "axi/hpd.h"
#include "axi/parameters.h"
#include "axi/vdma.h"
#include "debug.h"
#include "memory.h"
#include "application_config.h"

struct video_transmit {
	struct axi_vdma *vdma_frame_buffer;
	struct axi_vdma *vdma_background_buffer;
	struct axi_vdma *vdma_result_frame;
	struct axi_interface *hpd;
	struct axi_interface *parameters;

	struct video_config *config;

	memory_handle_t memory_handle;
};

struct video_transmit* setup(struct application_config config);
void initialize(struct video_transmit *video_transmit);
void cleanup(struct video_transmit *video_transmit);

void debug_video(const struct video_transmit video_transmit);

#endif /* SRC_VIDEO_TRANSMIT_H_ */
