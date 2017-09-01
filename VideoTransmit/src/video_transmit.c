#include "video_transmit.h"

void initialize(struct video_transmit *video_transmit, int video_width, int video_height) {
	debug("Configuring...");
	init_config(video_transmit->config, video_width, video_height);
	init_hpd(video_transmit->hpd);
	init_vdma(video_transmit->vdma, video_transmit->config, video_transmit->memory_handle);
//	init_processing(video_transmit->processing);
	debug("Configuration done.");
}

void cleanup(struct video_transmit *video_transmit) {
	debug("Cleaning up...");
	free(video_transmit->config);
//	stop_processing(video_transmit->processing);
//	free(video_transmit->processing);
	stop_vdma(video_transmit->vdma);
	free(video_transmit->vdma);
	stop_hpd(video_transmit->hpd);
	free(video_transmit->hpd);
	free(video_transmit);
	debug("Clean up finished.");
}

struct video_transmit* setup(unsigned int vdma_base_addr, unsigned int hpd_base_addr, unsigned int threshold_base_addr) {
	debug("Starting setup...");

	struct video_transmit *video_transmit = (struct video_transmit*) malloc(sizeof(struct video_transmit));
	video_transmit->memory_handle = open("/dev/mem", O_RDWR | O_SYNC);

	debug("Memory handle acquired.");

	video_transmit->config = (struct video_config*) malloc(sizeof(struct video_config));
	if (video_transmit->config == NULL) {
		perror("Config memory allocation failed.");
		exit(1);
	}

	video_transmit->vdma = (struct axi_vdma*) malloc(sizeof(struct axi_vdma));
	if (video_transmit->vdma == NULL) {
		perror("Vdma memory allocation failed.");
		exit(1);
	}
	setup_virt_memory(&video_transmit->vdma->common, 65535, video_transmit->memory_handle, vdma_base_addr);

//	video_transmit->processing = (struct axi_interface*) malloc(sizeof(struct axi_interface));
//	if (video_transmit->processing == NULL) {
//		perror("Processing memory allocation failed.");
//		exit(1);
//	}
//	setup_virt_memory(video_transmit->processing, 65535, video_transmit->memory_handle, threshold_base_addr);

	video_transmit->hpd = (struct axi_interface*) malloc(sizeof(struct axi_interface));
	if (video_transmit->hpd == NULL) {
		perror("Memory allocation failed.");
		exit(1);
	}
	setup_virt_memory(video_transmit->hpd, 65535, video_transmit->memory_handle, hpd_base_addr);

	debug("Configured structures.");

	return video_transmit;
}

void debug_video(const struct video_transmit video_transmit) {
	debug_vdma(*video_transmit.vdma, *video_transmit.config);
//	debug_processing(*video_transmit.processing);
	debug_hpd(*video_transmit.hpd);
}
