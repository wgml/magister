
#ifndef SRC_HPD_H_
#define SRC_HPD_H_

#include "axi.h"
#include "../debug.h"

#define HPD_DATA_REGISTER 0x0
#define HPD_TRI_REGISTER 0x4

void init_hpd(struct axi_interface *hpd);
void debug_hpd(const struct axi_interface hpd);
void stop_hpd(struct axi_interface *hpd);

#endif /* SRC_HPD_H_ */
