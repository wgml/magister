#ifndef SRC_AXI_PROCESSING_H_
#define SRC_AXI_PROCESSING_H_

#include "axi.h"
#include "../debug.h"

#define PROCESSING_CONTROL_RESISTER 0x0
#define PROCESSING_ENABLED 1
#define PROCESSING_VALUE_REGISTER 0x4

void init_processing(struct axi_interface *interface);
void update_processing(struct axi_interface *interface, unsigned char threshold);
void debug_processing(struct axi_interface interface);
void stop_processing(struct axi_interface *interface);

#endif /* SRC_AXI_PROCESSING_H_ */
