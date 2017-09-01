#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>

#include "axi/axi.h"

#define PARAMETER_A_REGISTER 0
#define PARAMETER_B_REGISTER 4
#define PARAMETER_C_REGISTER 8
#define PARAMETER_D_REGISTER 12

#define BASEADDR 0x43000000

typedef int memory_handle_t;

void setup_virtual_memory(struct axi_interface *interface, size_t length, memory_handle_t memory_handle, off_t base_addr) {
	interface->base_addr = base_addr;
	interface->virt_addr = (unsigned int*)mmap(NULL, length, PROT_READ | PROT_WRITE, MAP_SHARED, memory_handle, base_addr);
	if (interface->virt_addr == MAP_FAILED) {
		perror("Failed to map virtual memory.");
		exit(1);
	}
}

int main() {
	memory_handle_t memory_handle = open("/dev/mem", O_RDWR | O_SYNC);

	struct axi_interface* parameters = (struct axi_interface*) malloc(sizeof(struct axi_interface));
	if (parameters == NULL) {
		perror("Memory allocation failed.");
		exit(1);
	}
	setup_virtual_memory(parameters, 65535, memory_handle, BASEADDR);

    axi_write(parameters->virt_addr, PARAMETER_A_REGISTER, 1);
    axi_write(parameters->virt_addr, PARAMETER_B_REGISTER, 1);
    axi_write(parameters->virt_addr, PARAMETER_C_REGISTER, 1);
    axi_write(parameters->virt_addr, PARAMETER_D_REGISTER, 1);

    while(1);
}
