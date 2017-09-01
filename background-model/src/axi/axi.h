#ifndef SRC_AXI_H_
#define SRC_AXI_H_

#include <stdbool.h>

typedef unsigned int* virt_address;

extern bool axi_verbose;

struct axi_interface {
	unsigned int base_addr;
	virt_address virt_addr;

	char id[32];
};

bool are_bits_set(virt_address virt_addr, int location, unsigned int mask);

void axi_set(virt_address virt_addr, int location, unsigned int mask);
void axi_unset(virt_address virt_addr, int location, unsigned int mask);

unsigned int axi_read(virt_address virt_addr, int location);
void axi_write(virt_address virt_addr, int location, unsigned int value);

void set_verbose(bool value);

#endif /* SRC_AXI_H_ */
