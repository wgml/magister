#ifndef SRC_AXI_PARAMETERS_H_
#define SRC_AXI_PARAMETERS_H_

#include "axi.h"
#include "config.h"
#include "../debug.h"

#define ALPHA_REGISTER 	0
#define BG_REGISTER		4
#define FD_REGISTER		8

void init_parameters(struct axi_interface *interface, struct video_config config);
void update_parameters(struct axi_interface *interface, struct video_config config);
void debug_parameters(struct axi_interface interface);
void stop_parameters(struct axi_interface *interface);

#endif /* SRC_AXI_PARAMETERS_H_ */
