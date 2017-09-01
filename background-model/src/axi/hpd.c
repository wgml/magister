#include "hpd.h"

void init_hpd(struct axi_interface *hpd) {
	fdebug("[%s] Configuring...", hpd->id);
	// set hpd low
	axi_write(hpd->virt_addr, HPD_DATA_REGISTER, 0);

	// set hpd as output
	axi_write(hpd->virt_addr, HPD_TRI_REGISTER, 0);

	// set hpd high
	axi_write(hpd->virt_addr, HPD_DATA_REGISTER, 0xffffffff);

	fdebug("[%s] Configuration done.", hpd->id);
}


void debug_hpd(const struct axi_interface hpd) {
	unsigned int status = axi_read(hpd.virt_addr, HPD_DATA_REGISTER);
	fdebug("[%s] HPD_DATA_REGISTER is %08x.", hpd.id, status);
}

void stop_hpd(struct axi_interface *hpd) {
	fdebug("[%s] Stopping hdmi transfer...", hpd->id);
	axi_write(hpd->virt_addr, HPD_DATA_REGISTER, 0);
	fdebug("[%s] Hdmi transfer terminated.", hpd->id);
}
