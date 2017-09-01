#include "vdma.h"

static unsigned int addresses[9] = {0x0e000000, 0x0f000000, 0x10000000,
		0x11000000, 0x12000000, 0x13000000,
		0x14000000, 0x15000000, 0x16000000
};

static void setup_buffer_memory(struct vdma_framebuffer *buffer, size_t length, const memory_handle_t memory_handle, off_t base_addr) {
	fdebug("Setting buffer memory with address %08x and length %d.", base_addr, length);
	buffer->physical_addr = (vdma_buffer_addr) base_addr;
	buffer->virtual_addr =  (vdma_buffer_addr) mmap(NULL, length, PROT_READ | PROT_WRITE, MAP_SHARED, memory_handle, base_addr);
	if (buffer->virtual_addr == MAP_FAILED) {
		perror("Failed to map virtual memory.");
		exit(1);
	}
}

static void fill_framebuffer(struct vdma_framebuffer buffer, size_t length) {
	debug("Filling...");
	memset(buffer.virtual_addr, 123, length);
	debug("...done");
}

static void set_current_framebuffer(struct axi_vdma vdma) {
	fdebug("[%s] Setting read & write buffer index to %d.", vdma.common.id, vdma.current_framebuffer_index);
    axi_write(vdma.common.virt_addr, VDMA_PARK_PTR_REG,
    		vdma.current_framebuffer_index | (vdma.current_framebuffer_index << 8));
}

static void configure_transfer(struct axi_vdma vdma, const struct video_config config) {
	fdebug("[%s] Configuring transfer with triple buffering...", vdma.common.id);

	fdebug("[%s] Writing reset...", vdma.common.id);
    axi_write(vdma.common.virt_addr, VDMA_S2MM_CONTROL_REGISTER, VDMA_CONTROL_REGISTER_RESET | VDMA_CONTROL_REGISTER_START);
    axi_write(vdma.common.virt_addr, VDMA_MM2S_CONTROL_REGISTER, VDMA_CONTROL_REGISTER_RESET | VDMA_CONTROL_REGISTER_START);

	fdebug("[%s] Waiting for reset to finish...", vdma.common.id);
    while(are_bits_set(vdma.common.virt_addr, VDMA_S2MM_CONTROL_REGISTER, VDMA_CONTROL_REGISTER_RESET)) {
    	debug_vdma(vdma, config);
    	sleep(1);
    }
    while(are_bits_set(vdma.common.virt_addr, VDMA_MM2S_CONTROL_REGISTER, VDMA_CONTROL_REGISTER_RESET)) {
    	debug_vdma(vdma, config);
    	sleep(1);
    }

	fdebug("[%s] Clearing error bits...", vdma.common.id);
    axi_write(vdma.common.virt_addr, VDMA_S2MM_STATUS_REGISTER, 0);
    axi_write(vdma.common.virt_addr, VDMA_MM2S_STATUS_REGISTER, 0);

	fdebug("[%s] Masking interrupts...", vdma.common.id);
    axi_write(vdma.common.virt_addr, VDMA_S2MM_IRQ_MASK, 0xffffffff);

	fdebug("[%s] Starting vdma...", vdma.common.id);
    axi_write(vdma.common.virt_addr, VDMA_S2MM_CONTROL_REGISTER,
    		VDMA_CONTROL_REGISTER_START
			| (255 << 16) |
			VDMA_CONTROL_REGISTER_GENLOCK_ENABLE |
			VDMA_CONTROL_REGISTER_INTERNAL_GENLOCK |
			VDMA_CONTROL_REGISTER_CIRCULAR_PARK);
    axi_write(vdma.common.virt_addr, VDMA_MM2S_CONTROL_REGISTER,
    		VDMA_CONTROL_REGISTER_START
			| (255 << 16) |
			VDMA_CONTROL_REGISTER_GENLOCK_ENABLE |
			VDMA_CONTROL_REGISTER_INTERNAL_GENLOCK |
			VDMA_CONTROL_REGISTER_CIRCULAR_PARK);

    while (are_bits_set(vdma.common.virt_addr, VDMA_S2MM_STATUS_REGISTER, VDMA_STATUS_REGISTER_HALTED)
    		|| are_bits_set(vdma.common.virt_addr, VDMA_MM2S_STATUS_REGISTER, VDMA_STATUS_REGISTER_HALTED)) {
    	fdebug("[%s] Vdma not yet started, waiting...", vdma.common.id);
    	debug_vdma(vdma, config);
    	sleep(1);
    }
	fdebug("[%s] Vdma started.", vdma.common.id);

	fdebug("[%s] Clearing error bits...", vdma.common.id);
    axi_write(vdma.common.virt_addr, VDMA_S2MM_STATUS_REGISTER, 0);
    axi_write(vdma.common.virt_addr, VDMA_MM2S_STATUS_REGISTER, 0);

	fdebug("[%s] Setting up VDMA_S2MM_REG_INDEX...", vdma.common.id);
    axi_write(vdma.common.virt_addr, VDMA_S2MM_REG_INDEX, 0);

	fdebug("[%s] Setting up framebuffer addresses...", vdma.common.id);
    axi_write(vdma.common.virt_addr, VDMA_S2MM_FRAMEBUFFER1, (unsigned int) vdma.framebuffers[0].physical_addr);
    axi_write(vdma.common.virt_addr, VDMA_S2MM_FRAMEBUFFER2, (unsigned int) vdma.framebuffers[1].physical_addr);
    axi_write(vdma.common.virt_addr, VDMA_S2MM_FRAMEBUFFER3, (unsigned int) vdma.framebuffers[2].physical_addr);
    axi_write(vdma.common.virt_addr, VDMA_MM2S_FRAMEBUFFER1, (unsigned int) vdma.framebuffers[0].physical_addr);
    axi_write(vdma.common.virt_addr, VDMA_MM2S_FRAMEBUFFER2, (unsigned int) vdma.framebuffers[1].physical_addr);
    axi_write(vdma.common.virt_addr, VDMA_MM2S_FRAMEBUFFER3, (unsigned int) vdma.framebuffers[2].physical_addr);

	fdebug("[%s] Setting up VDMA_PARK_PTR_REG...", vdma.common.id);
	set_current_framebuffer(vdma);

	fdebug("[%s] Setting up stride...", vdma.common.id);
    axi_write(vdma.common.virt_addr, VDMA_S2MM_FRMDLY_STRIDE, config.video.width * config.video.pixel_size | (1 << 24)); //stride | frame_delay
    axi_write(vdma.common.virt_addr, VDMA_MM2S_FRMDLY_STRIDE, config.video.width * config.video.pixel_size);

	fdebug("[%s] Setting up width...", vdma.common.id);
    axi_write(vdma.common.virt_addr, VDMA_S2MM_HSIZE, config.video.width * config.video.pixel_size);
    axi_write(vdma.common.virt_addr, VDMA_MM2S_HSIZE, config.video.width * config.video.pixel_size);

	fdebug("[%s] Setting up height...", vdma.common.id);
    axi_write(vdma.common.virt_addr, VDMA_S2MM_VSIZE, config.video.height);
    axi_write(vdma.common.virt_addr, VDMA_MM2S_VSIZE, config.video.height);

	fdebug("[%s] Transfer configuration done.", vdma.common.id);
}

void init_framebuffers(struct axi_vdma *vdma, const struct video_config *config, const memory_handle_t memory_handle) {
	static size_t framebuffer_id = 0;
	fdebug("[%s] Setting up framebuffers (hardcoded values for now)...", vdma->common.id);
	vdma->num_framebuffers = 3;
	vdma->current_framebuffer_index = 0;
	vdma->framebuffers = (struct vdma_framebuffer*) malloc(vdma->num_framebuffers * sizeof(struct vdma_framebuffer));

	for (size_t fb = 0; fb < vdma->num_framebuffers; fb++) {
		if (framebuffer_id >= sizeof(addresses)) {
			fdebug("[%s] Too few framebuffer addresses specified. Wanted address for fb %d but only %d available.", vdma->common.id,
					framebuffer_id + 1, fb);
			abort();
		}
		setup_buffer_memory(vdma->framebuffers + fb, config->video.framebuffer_length, memory_handle, addresses[framebuffer_id++]);
	}

	fdebug("[%s] Filling framebuffers...", vdma->common.id);
	for (size_t fb = 0; fb < vdma->num_framebuffers; fb++)
		fill_framebuffer(vdma->framebuffers[fb], config->video.framebuffer_length);

}
void init_vdma(struct axi_vdma *vdma, const struct video_config *config, const memory_handle_t memory_handle) {
	fdebug("[%s] Configuring vdma...", vdma->common.id);
	init_framebuffers(vdma, config, memory_handle);
	configure_transfer(*vdma, *config);

	fdebug("[%s] Vdma configuration done.", vdma->common.id);
}

static void vdma_status_dump(unsigned int status) {
	char buffer[160] = "";
    if (status & VDMA_STATUS_REGISTER_HALTED) strcat(buffer, " halted"); else strcat(buffer, "running");
    if (status & VDMA_STATUS_REGISTER_VDMAInternalError) strcat(buffer, " vdma-internal-error");
    if (status & VDMA_STATUS_REGISTER_VDMASlaveError) strcat(buffer, " vdma-slave-error");
    if (status & VDMA_STATUS_REGISTER_VDMADecodeError) strcat(buffer, " vdma-decode-error");
    if (status & VDMA_STATUS_REGISTER_StartOfFrameEarlyError) strcat(buffer, " start-of-frame-early-error");
    if (status & VDMA_STATUS_REGISTER_EndOfLineEarlyError) strcat(buffer, " end-of-line-early-error");
    if (status & VDMA_STATUS_REGISTER_StartOfFrameLateError) strcat(buffer, " start-of-frame-late-error");
    if (status & VDMA_STATUS_REGISTER_FrameCountInterrupt) strcat(buffer, " frame-count-interrupt");
    if (status & VDMA_STATUS_REGISTER_DelayCountInterrupt) strcat(buffer, " delay-count-interrupt");
    if (status & VDMA_STATUS_REGISTER_ErrorInterrupt) strcat(buffer, " error-interrupt");
    if (status & VDMA_STATUS_REGISTER_EndOfLineLateError) strcat(buffer, " end-of-line-late-error");
    strcat(buffer, " frame-count:%d delay-count:%d");
    fdebug(buffer, (status & VDMA_STATUS_REGISTER_FrameCount) >> 16, (status & VDMA_STATUS_REGISTER_DelayCount) >> 24);
}

void debug_vdma(const struct axi_vdma vdma, const struct video_config config) {
	unsigned int status = axi_read(vdma.common.virt_addr, VDMA_MM2S_STATUS_REGISTER);
	fdebug("[%s] VDMA_MM2S_STATUS_REGISTER is %08x.", vdma.common.id, status);
	vdma_status_dump(status);
	status = axi_read(vdma.common.virt_addr, VDMA_S2MM_STATUS_REGISTER);
	fdebug("[%s] VDMA_S2MM_STATUS_REGISTER is %08x.", vdma.common.id, status);
	vdma_status_dump(status);
}

void stop_vdma(struct axi_vdma *vdma) {
	fdebug("[%s] Stopping vdma...", vdma->common.id);
	axi_unset(vdma->common.virt_addr, VDMA_S2MM_CONTROL_REGISTER, VDMA_CONTROL_REGISTER_START);
	axi_unset(vdma->common.virt_addr, VDMA_MM2S_CONTROL_REGISTER, VDMA_CONTROL_REGISTER_START);
	fdebug("[%s] Vdma stopped.", vdma->common.id);
}
