#ifndef SRC_MEMORY_H_
#define SRC_MEMORY_H_

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>

#include "axi/axi.h"

typedef int memory_handle_t;
void setup_virt_memory(struct axi_interface *interface, size_t length, memory_handle_t memory_handle, off_t base_addr);
#endif /* SRC_MEMORY_H_ */
