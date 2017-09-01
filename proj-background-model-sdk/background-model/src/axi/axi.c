#include "axi.h"
#include "../debug.h"

bool axi_verbose = false;

bool are_bits_set(virt_address virt_addr, int location, unsigned int mask) {
	if (axi_verbose)
		fdebug("are_bits_set addr=%p loc=%d mask=%08x", virt_addr, location, mask);

	return (virt_addr[location >> 2] & mask) == mask;
}

void axi_set(virt_address virt_addr, int location, unsigned int mask) {
	if (axi_verbose)
		fdebug("axi_set addr=%p loc=%d mask=%08x", virt_addr, location, mask);

	virt_addr[location >> 2] |= mask;
}

void axi_unset(virt_address virt_addr, int location, unsigned int mask) {
	if (axi_verbose)
		fdebug("axi_unset addr=%p loc=%d mask=%08x", virt_addr, location, mask);

	virt_addr[location >> 2] &= ~mask;
}

unsigned int axi_read(virt_address virt_addr, int location) {
	if (axi_verbose)
		fdebug("axi_read addr=%p loc=%d", virt_addr, location);

	return virt_addr[location >> 2];
}

void axi_write(virt_address virt_addr, int location, unsigned int value) {
	if (axi_verbose)
		fdebug("axi_write addr=%p loc=%d mask=%08x", virt_addr, location, value);

	virt_addr[location >> 2] = value;
}

void set_verbose(bool value) {
	axi_verbose = value;
}
