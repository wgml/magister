#include "memory.h"

void setup_virt_memory(struct axi_interface *interface, size_t length, memory_handle_t memory_handle, off_t base_addr) {
	interface->base_addr = base_addr;
	interface->virt_addr = (unsigned int*)mmap(NULL, length, PROT_READ | PROT_WRITE, MAP_SHARED, memory_handle, base_addr);
	if (interface->virt_addr == MAP_FAILED) {
		perror("Failed to map virtual memory.");
		exit(1);
	}
}
