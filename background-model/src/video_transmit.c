#include "video_transmit.h"

#include "interfaces.h"

void initialize(struct video_transmit *video_transmit) {
	debug("Configuring...");
	init_hpd(video_transmit->hpd);

	init_parameters(video_transmit->parameters, *video_transmit->config);

	init_vdma(video_transmit->vdma_frame_buffer, video_transmit->config, video_transmit->memory_handle);

	init_vdma(video_transmit->vdma_background_buffer, video_transmit->config, video_transmit->memory_handle);

	init_vdma(video_transmit->vdma_result_frame, video_transmit->config, video_transmit->memory_handle);

	debug("Configuration done.");
}

void cleanup(struct video_transmit *video_transmit) {
	debug("Cleaning up...");
	free(video_transmit->config);

	stop_vdma(video_transmit->vdma_frame_buffer);
	free(video_transmit->vdma_frame_buffer);

	stop_vdma(video_transmit->vdma_background_buffer);
	free(video_transmit->vdma_background_buffer);

	stop_vdma(video_transmit->vdma_result_frame);
	free(video_transmit->vdma_result_frame);

	stop_hpd(video_transmit->hpd);
	free(video_transmit->hpd);

	stop_parameters(video_transmit->parameters);
	free(video_transmit->parameters);

	free(video_transmit);
	debug("Clean up finished.");
}

struct video_transmit* setup(struct application_config config) {
	debug("Starting setup...");

	struct video_transmit *video_transmit = (struct video_transmit*) malloc(sizeof(struct video_transmit));
	video_transmit->memory_handle = open("/dev/mem", O_RDWR | O_SYNC);

	debug("Memory handle acquired.");

	video_transmit->config = (struct video_config*) malloc(sizeof(struct video_config));
	if (video_transmit->config == NULL) {
		perror("config memory allocation failed.");
		exit(1);
	}
	init_config(video_transmit->config, config.image_width, config.image_height,
			config.alpha, config.bg_th, config.fd_th);

	video_transmit->vdma_frame_buffer = (struct axi_vdma*) malloc(sizeof(struct axi_vdma));
	if (video_transmit->vdma_frame_buffer == NULL) {
		perror("vdma_frame_buffer memory allocation failed.");
		exit(1);
	}
	setup_virt_memory(&video_transmit->vdma_frame_buffer->common, 65535, video_transmit->memory_handle, vdma_frame_buffering_addr);
	strcpy(video_transmit->vdma_frame_buffer->common.id, "vdma_frame_buffer");
	init_framebuffers(video_transmit->vdma_frame_buffer, video_transmit->config, video_transmit->memory_handle);

	video_transmit->vdma_background_buffer = (struct axi_vdma*) malloc(sizeof(struct axi_vdma));
	if (video_transmit->vdma_background_buffer == NULL) {
		perror("vdma_background_buffer memory allocation failed.");
		exit(1);
	}
	setup_virt_memory(&video_transmit->vdma_background_buffer->common, 65535, video_transmit->memory_handle, vdma_background_buffer_addr);
	strcpy(video_transmit->vdma_background_buffer->common.id, "vdma_background_buffer");
	init_framebuffers(video_transmit->vdma_background_buffer, video_transmit->config, video_transmit->memory_handle);

	video_transmit->vdma_result_frame = (struct axi_vdma*) malloc(sizeof(struct axi_vdma));
	if (video_transmit->vdma_result_frame == NULL) {
		perror("vdma_result_frame memory allocation failed.");
		exit(1);
	}
	setup_virt_memory(&video_transmit->vdma_result_frame->common, 65535, video_transmit->memory_handle, vdma_result_frame_addr);
	strcpy(video_transmit->vdma_result_frame->common.id, "vdma_result_frame");
	init_framebuffers(video_transmit->vdma_result_frame, video_transmit->config, video_transmit->memory_handle);

	video_transmit->parameters = (struct axi_interface*) malloc(sizeof(struct axi_interface));
	if (video_transmit->parameters == NULL) {
		perror("parameters memory allocation failed.");
		exit(1);
	}
	setup_virt_memory(video_transmit->parameters, 65535, video_transmit->memory_handle, algorithm_parameters_addr);
	strcpy(video_transmit->parameters->id, "parameters");

	video_transmit->hpd = (struct axi_interface*) malloc(sizeof(struct axi_interface));
	if (video_transmit->hpd == NULL) {
		perror("hpd memory allocation failed.");
		exit(1);
	}
	setup_virt_memory(video_transmit->hpd, 65535, video_transmit->memory_handle, axi_gpio_hpd_addr);
	strcpy(video_transmit->hpd->id, "hpd");

	debug("Configured structures.");

	return video_transmit;
}

void debug_video(const struct video_transmit video_transmit) {
	debug_vdma(*video_transmit.vdma_frame_buffer, *video_transmit.config);
	debug_vdma(*video_transmit.vdma_background_buffer, *video_transmit.config);
	debug_vdma(*video_transmit.vdma_result_frame, *video_transmit.config);
	debug_parameters(*video_transmit.parameters);
	debug_hpd(*video_transmit.hpd);
}
