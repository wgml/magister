#include "vdma.h"

#define LINUX_MEM_END 224 * 1024 * 1024
#define FB_ADDRESS(n, l) LINUX_MEM_END + n * l

static unsigned int addresses[3] = {0x0e000000,0x0f000000,0x10000000};
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
	fdebug("Setting read & write buffer index to %d.", vdma.current_framebuffer_index);
    axi_write(vdma.common.virt_addr, VDMA_PARK_PTR_REG,
    		vdma.current_framebuffer_index | (vdma.current_framebuffer_index << 8));
}

static void configure_transfer(struct axi_vdma vdma, const struct video_config config) {
	debug("Configuring transfer with triple buffering...");

	debug("Writing reset...");
    axi_write(vdma.common.virt_addr, VDMA_S2MM_CONTROL_REGISTER, VDMA_CONTROL_REGISTER_RESET | VDMA_CONTROL_REGISTER_START);
    axi_write(vdma.common.virt_addr, VDMA_MM2S_CONTROL_REGISTER, VDMA_CONTROL_REGISTER_RESET | VDMA_CONTROL_REGISTER_START);

	debug("Waiting for reset to finish...");
    while(are_bits_set(vdma.common.virt_addr, VDMA_S2MM_CONTROL_REGISTER, VDMA_CONTROL_REGISTER_RESET)) {
    	debug_vdma(vdma, config);
    	sleep(1);
    }
    while(are_bits_set(vdma.common.virt_addr, VDMA_MM2S_CONTROL_REGISTER, VDMA_CONTROL_REGISTER_RESET)) {
    	debug_vdma(vdma, config);
    	sleep(1);
    }

	debug("Clearing error bits...");
    axi_write(vdma.common.virt_addr, VDMA_S2MM_STATUS_REGISTER, 0);
    axi_write(vdma.common.virt_addr, VDMA_MM2S_STATUS_REGISTER, 0);

	debug("Masking interrupts...");
    axi_write(vdma.common.virt_addr, VDMA_S2MM_IRQ_MASK, 0xffffffff);

	debug("Starting vdma...");
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
    	debug("Vdma not yet started, waiting...");
    	debug_vdma(vdma, config);
    	sleep(1);
    }
	debug("Vdma started.");

	debug("Clearing error bits...");
    axi_write(vdma.common.virt_addr, VDMA_S2MM_STATUS_REGISTER, 0);
    axi_write(vdma.common.virt_addr, VDMA_MM2S_STATUS_REGISTER, 0);

	debug("Setting up VDMA_S2MM_REG_INDEX...");
    axi_write(vdma.common.virt_addr, VDMA_S2MM_REG_INDEX, 0);

	debug("Setting up framebuffer addresses...");
    axi_write(vdma.common.virt_addr, VDMA_S2MM_FRAMEBUFFER1, (unsigned int) vdma.framebuffers[0].physical_addr);
    axi_write(vdma.common.virt_addr, VDMA_S2MM_FRAMEBUFFER2, (unsigned int) vdma.framebuffers[1].physical_addr);
    axi_write(vdma.common.virt_addr, VDMA_S2MM_FRAMEBUFFER3, (unsigned int) vdma.framebuffers[2].physical_addr);
    axi_write(vdma.common.virt_addr, VDMA_MM2S_FRAMEBUFFER1, (unsigned int) vdma.framebuffers[0].physical_addr);
    axi_write(vdma.common.virt_addr, VDMA_MM2S_FRAMEBUFFER2, (unsigned int) vdma.framebuffers[1].physical_addr);
    axi_write(vdma.common.virt_addr, VDMA_MM2S_FRAMEBUFFER3, (unsigned int) vdma.framebuffers[2].physical_addr);

	debug("Setting up VDMA_PARK_PTR_REG...");
	set_current_framebuffer(vdma);

	debug("Setting up stride...");
    axi_write(vdma.common.virt_addr, VDMA_S2MM_FRMDLY_STRIDE, config.width * config.pixel_size | (1 << 24)); //stride | frame_delay
    axi_write(vdma.common.virt_addr, VDMA_MM2S_FRMDLY_STRIDE, config.width * config.pixel_size);

	debug("Setting up width...");
    axi_write(vdma.common.virt_addr, VDMA_S2MM_HSIZE, config.width * config.pixel_size);
    axi_write(vdma.common.virt_addr, VDMA_MM2S_HSIZE, config.width * config.pixel_size);

	debug("Setting up height...");
    axi_write(vdma.common.virt_addr, VDMA_S2MM_VSIZE, config.height);
    axi_write(vdma.common.virt_addr, VDMA_MM2S_VSIZE, config.height);

	debug("Transfer configuration done.");
}

void init_vdma(struct axi_vdma *vdma, const struct video_config *config, const memory_handle_t memory_handle) {
	debug("Configuring vdma...");
	debug("Setting up framebuffers (hardcoded values for now)...");
	vdma->num_framebuffers = 3;
	vdma->current_framebuffer_index = 0;
	vdma->framebuffers = (struct vdma_framebuffer*) malloc(vdma->num_framebuffers * sizeof(struct vdma_framebuffer));

	for (size_t fb = 0; fb < vdma->num_framebuffers; fb++)
		setup_buffer_memory(vdma->framebuffers + fb, config->framebuffer_length, memory_handle, addresses[fb]);

	debug("Filling framebuffers...");
	for (size_t fb = 0; fb < vdma->num_framebuffers; fb++)
		fill_framebuffer(vdma->framebuffers[fb], config->framebuffer_length);

	configure_transfer(*vdma, *config);

	debug("Vdma configuration done.");
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
	fdebug("VDMA_MM2S_STATUS_REGISTER is %08x.", status);
	vdma_status_dump(status);
	status = axi_read(vdma.common.virt_addr, VDMA_S2MM_STATUS_REGISTER);
	fdebug("VDMA_S2MM_STATUS_REGISTER is %08x.", status);
	vdma_status_dump(status);
}

void debug_on_frame(const struct axi_vdma vdma) {
	for (int row = 0; row < 255; row++)
		memset(vdma.framebuffers[vdma.current_framebuffer_index].virtual_addr + 3 * 1280 * row, row, 255 * 3); //todo hardcoded for now...
}

void framebuffer_inc(struct axi_vdma *vdma) {
	vdma->current_framebuffer_index = (vdma->current_framebuffer_index + 1) % vdma->num_framebuffers;
	set_current_framebuffer(*vdma);
}

void framebuffer_dec(struct axi_vdma *vdma) {
	size_t idx = vdma->current_framebuffer_index - 1;
	if (idx < 0)
		idx += vdma->num_framebuffers;

	vdma->current_framebuffer_index = idx % vdma->num_framebuffers;
	set_current_framebuffer(*vdma);
}

void stop_vdma(struct axi_vdma *vdma) {
	debug("Stopping vdma...");
	axi_unset(vdma->common.virt_addr, VDMA_S2MM_CONTROL_REGISTER, VDMA_CONTROL_REGISTER_START);
	axi_unset(vdma->common.virt_addr, VDMA_MM2S_CONTROL_REGISTER, VDMA_CONTROL_REGISTER_START);
	debug("Vdma stopped.");
}
