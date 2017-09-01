#include "processing.h"

void init_processing(struct axi_interface *interface) {
	unsigned char init_value = 0;
	fdebug("Setting initial threshold value to %d...", init_value);
	update_processing(interface, init_value);
	debug("Threshold configuration done.");
}

void update_processing(struct axi_interface *interface, unsigned char threshold) {
	axi_write(interface->virt_addr, PROCESSING_CONTROL_RESISTER, PROCESSING_ENABLED);
	fdebug("Setting threshold value to %d...", threshold);
	axi_write(interface->virt_addr, PROCESSING_VALUE_REGISTER, threshold);
}

void debug_processing(struct axi_interface interface) {
	unsigned char value = (unsigned char) (axi_read(interface.virt_addr, PROCESSING_VALUE_REGISTER) & 0xFF);
	fdebug("THRESHOLD_REGISTER_VALUE is %08x (%d).", value, value);
	value = (value + 1) % 256;
	update_processing(&interface, value);
}

void stop_processing(struct axi_interface *interface) {
	// noop
}
