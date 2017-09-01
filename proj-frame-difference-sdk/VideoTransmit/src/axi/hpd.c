#include "hpd.h"

void init_hpd(struct axi_interface *hpd) {
	debug("Configuring hpd...");
	// set hpd low
	axi_write(hpd->virt_addr, HPD_DATA_REGISTER, 0);

	// set hpd as output
	axi_write(hpd->virt_addr, HPD_TRI_REGISTER, 0);

	// set hpd high
	axi_write(hpd->virt_addr, HPD_DATA_REGISTER, 0xffffffff);

	debug("Hpd configuration done.");
}


void debug_hpd(const struct axi_interface hpd) {
//	unsigned int status = axi_read(hpd.virt_addr, HPD_DATA_REGISTER);
//	fdebug("HPD_DATA_REGISTER is %08x.", status);
}

void stop_hpd(struct axi_interface *hpd) {
	debug("Stopping hdmi transfer...");
	axi_write(hpd->virt_addr, HPD_DATA_REGISTER, 0);
	debug("Hdmi transfer terminated.");
}
