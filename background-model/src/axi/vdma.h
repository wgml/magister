/*
 * vdma.h
 *
 *  Created on: 18 Mar 2017
 *      Author: wgml
 */

#ifndef SRC_VDMA_H_
#define SRC_VDMA_H_

#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "axi.h"
#include "config.h"
#include "../debug.h"
#include "memory.h"

#define VDMA_MM2S_CONTROL_REGISTER       0x00
#define VDMA_MM2S_STATUS_REGISTER        0x04
#define VDMA_MM2S_VSIZE                  0x50
#define VDMA_MM2S_HSIZE                  0x54
#define VDMA_MM2S_FRMDLY_STRIDE          0x58

#define VDMA_PARK_PTR_REG                0x28

#define VDMA_MM2S_FRAMEBUFFER1           0x5c
#define VDMA_MM2S_FRAMEBUFFER2           0x60
#define VDMA_MM2S_FRAMEBUFFER3           0x64

#define VDMA_S2MM_CONTROL_REGISTER       0x30
#define VDMA_S2MM_STATUS_REGISTER        0x34
#define VDMA_S2MM_IRQ_MASK               0x3c
#define VDMA_S2MM_REG_INDEX              0x44
#define VDMA_S2MM_VSIZE                  0xa0
#define VDMA_S2MM_HSIZE                  0xa4
#define VDMA_S2MM_FRMDLY_STRIDE          0xa8

#define VDMA_S2MM_FRAMEBUFFER1           0xac
#define VDMA_S2MM_FRAMEBUFFER2           0xb0
#define VDMA_S2MM_FRAMEBUFFER3           0xb4

/* S2MM and MM2S control register flags */
#define VDMA_CONTROL_REGISTER_START                     0x00000001
#define VDMA_CONTROL_REGISTER_CIRCULAR_PARK             0x00000002
#define VDMA_CONTROL_REGISTER_RESET                     0x00000004
#define VDMA_CONTROL_REGISTER_GENLOCK_ENABLE            0x00000008
#define VDMA_CONTROL_REGISTER_FrameCntEn                0x00000010
#define VDMA_CONTROL_REGISTER_INTERNAL_GENLOCK          0x00000080
#define VDMA_CONTROL_REGISTER_WrPntr                    0x00000f00
#define VDMA_CONTROL_REGISTER_FrmCtn_IrqEn              0x00001000
#define VDMA_CONTROL_REGISTER_DlyCnt_IrqEn              0x00002000
#define VDMA_CONTROL_REGISTER_ERR_IrqEn                 0x00004000
#define VDMA_CONTROL_REGISTER_Repeat_En                 0x00008000
#define VDMA_CONTROL_REGISTER_InterruptFrameCount       0x00ff0000
#define VDMA_CONTROL_REGISTER_IRQDelayCount             0xff000000

/* S2MM status register */
#define VDMA_STATUS_REGISTER_HALTED                     0x00000001  // Read-only
#define VDMA_STATUS_REGISTER_VDMAInternalError          0x00000010  // Read or write-clear
#define VDMA_STATUS_REGISTER_VDMASlaveError             0x00000020  // Read-only
#define VDMA_STATUS_REGISTER_VDMADecodeError            0x00000040  // Read-only
#define VDMA_STATUS_REGISTER_StartOfFrameEarlyError     0x00000080  // Read-only
#define VDMA_STATUS_REGISTER_EndOfLineEarlyError        0x00000100  // Read-only
#define VDMA_STATUS_REGISTER_StartOfFrameLateError      0x00000800  // Read-only
#define VDMA_STATUS_REGISTER_FrameCountInterrupt        0x00001000  // Read-only
#define VDMA_STATUS_REGISTER_DelayCountInterrupt        0x00002000  // Read-only
#define VDMA_STATUS_REGISTER_ErrorInterrupt             0x00004000  // Read-only
#define VDMA_STATUS_REGISTER_EndOfLineLateError         0x00008000  // Read-only
#define VDMA_STATUS_REGISTER_FrameCount                 0x00ff0000  // Read-only
#define VDMA_STATUS_REGISTER_DelayCount                 0xff000000  // Read-only

typedef unsigned char* vdma_buffer_addr;

struct vdma_framebuffer {
	vdma_buffer_addr physical_addr;
	vdma_buffer_addr virtual_addr;
};

struct axi_vdma {
	struct axi_interface common;
	size_t num_framebuffers;
	size_t current_framebuffer_index;
	struct vdma_framebuffer *framebuffers;
};

void init_framebuffers(struct axi_vdma *vdma, const struct video_config *config, const memory_handle_t memory_handle);

void init_vdma(struct axi_vdma *vdma, const struct video_config *config, const memory_handle_t memory_handle);
void debug_vdma(const struct axi_vdma vdma, const struct video_config config);
void stop_vdma(struct axi_vdma *vdma);

#endif /* SRC_VDMA_H_ */
