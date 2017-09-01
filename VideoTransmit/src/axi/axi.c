#include "axi.h"

bool are_bits_set(virt_address virt_addr, int location, unsigned int mask) {
	return (virt_addr[location >> 2] & mask) == mask;
}

void axi_set(virt_address virt_addr, int location, unsigned int mask) {
	virt_addr[location >> 2] |= mask;
}

void axi_unset(virt_address virt_addr, int location, unsigned int mask) {
	virt_addr[location >> 2] &= ~mask;
}

unsigned int axi_read(virt_address virt_addr, int location) {
	return virt_addr[location >> 2];
}

void axi_write(virt_address virt_addr, int location, unsigned int value) {
	virt_addr[location >> 2] = value;
}

