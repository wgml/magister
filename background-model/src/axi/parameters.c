#include "parameters.h"

static unsigned char convert_alpha(double alpha) {
	if (alpha >= 1)
		return 0b10000000;
	if (alpha <= 0)
		return 0;

	unsigned char result = 0;
	for (ssize_t bit = 6; bit >= 0; bit--)
		if ((char) (alpha * 128) >= (result | (1 << bit)))
			result |= 1 << bit;
	return result;
}

void init_parameters(struct axi_interface *interface, struct video_config config) {
	fdebug("[%s] Configuring parameters...", interface->id);
	update_parameters(interface, config);
	fdebug("[%s] Parameters configuration done.", interface->id);
}

void update_parameters(struct axi_interface *interface, struct video_config config) {
	fdebug("[%s] Updating parameters...", interface->id);

	unsigned char alpha_c = convert_alpha(config.algo.alpha);
	fdebug("[%s] Alpha char representation for input %f is %d.", interface->id, config.algo.alpha, alpha_c);

	axi_write(interface->virt_addr, ALPHA_REGISTER, alpha_c);
	axi_write(interface->virt_addr, BG_REGISTER, config.algo.bg_th);
	axi_write(interface->virt_addr, FD_REGISTER, config.algo.fd_th);

	fdebug("[%s] Parameters update done.", interface->id);
}

void debug_parameters(struct axi_interface interface) {
	unsigned int status = axi_read(interface.virt_addr, ALPHA_REGISTER);
	fdebug("[%s] ALPHA_REGISTER is %08x.", interface.id, status);
	status = axi_read(interface.virt_addr, BG_REGISTER);
	fdebug("[%s] BG_REGISTER is %08x.", interface.id, status);
	status = axi_read(interface.virt_addr, FD_REGISTER);
	fdebug("[%s] FD_REGISTER is %08x.", interface.id, status);
}

void stop_parameters(struct axi_interface *interface) {
	// noop
}
