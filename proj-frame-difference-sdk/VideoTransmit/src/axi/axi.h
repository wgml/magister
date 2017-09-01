#ifndef SRC_AXI_H_
#define SRC_AXI_H_

#include <stdbool.h>

typedef unsigned int* virt_address;

struct axi_interface {
	unsigned int base_addr;
	virt_address virt_addr;
};

bool are_bits_set(virt_address virt_addr, int location, unsigned int mask);

void axi_set(virt_address virt_addr, int location, unsigned int mask);
void axi_unset(virt_address virt_addr, int location, unsigned int mask);

unsigned int axi_read(virt_address virt_addr, int location);
void axi_write(virt_address virt_addr, int location, unsigned int value);

#endif /* SRC_AXI_H_ */
