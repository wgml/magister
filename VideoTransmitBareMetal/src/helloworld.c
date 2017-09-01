#include "xparameters.h"
#include "platform.h"
#include "xil_printf.h"
#include "xil_io.h"
#include "sleep.h"
#include "xaxivdma.h"
#include "xil_cache.h"
#include "xscugic.h"
#include "xil_exception.h"

// VDMA (write) registers
#define S2MM_VDMACR 0x30
#define S2MM_VDMASR 0x34
#define S2MM_VSIZE 0xA0
#define S2MM_HSIZE 0xA4
#define S2MM_FRMDLY_STRIDE 0xA8
#define S2MM_START_ADDRESS 0xAC

#define PART_PTR_REG 0x28

#define MM2S_VDMACR 0x00
#define MM2S_VDMASR 0x04
#define MM2S_VSIZE 0x50
#define MM2S_HSIZE 0x54
#define MM2S_FRMDLY_STRIDE 0x58
#define MM2S_START_ADDRESS 0x5C

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

// Full (with blanking) input image sizes
#define HSIZE_FULL 1980
#define VSIZE_FULL 750

// Active (only video) image sizes
#define HSIZE_ACTIVE 1280
#define VSIZE_ACTIVE 720

#define PIXEL_SIZE 3
// Buffer for input image
static volatile u8 framebuffer1[VSIZE_ACTIVE*HSIZE_ACTIVE*PIXEL_SIZE] = {0};
static volatile u8 framebuffer2[VSIZE_ACTIVE*HSIZE_ACTIVE*PIXEL_SIZE] = {0};
static volatile u8 framebuffer3[VSIZE_ACTIVE*HSIZE_ACTIVE*PIXEL_SIZE] = {0};
static volatile u8 framebuffer4[VSIZE_ACTIVE*HSIZE_ACTIVE*PIXEL_SIZE] = {0};
static volatile u8 framebuffer5[VSIZE_ACTIVE*HSIZE_ACTIVE*PIXEL_SIZE] = {0};
static volatile u8 framebuffer6[VSIZE_ACTIVE*HSIZE_ACTIVE*PIXEL_SIZE] = {0};
static volatile u8 framebuffer7[VSIZE_ACTIVE*HSIZE_ACTIVE*PIXEL_SIZE] = {0};
static volatile u8 framebuffer8[VSIZE_ACTIVE*HSIZE_ACTIVE*PIXEL_SIZE] = {0};
static volatile u8 framebuffer9[VSIZE_ACTIVE*HSIZE_ACTIVE*PIXEL_SIZE] = {0};

// parameters
#define ALPHA_REGISTER 0
#define BG_TH_REGISTER 4
#define FD_TH_REGISTER 8

XAxiVdma AxiVdmaFrameBuffering;
XAxiVdma AxiVdmaBackgroundBuffering;
XAxiVdma AxiVdmaResultFrame;

void vdma_status_dump(u32 status) {
	char buffer[160] = "";
    if (status & VDMA_STATUS_REGISTER_HALTED) strcat(buffer, " halted"); else strcat(buffer, "running");
    if (status & VDMA_STATUS_REGISTER_VDMAInternalError) strcat(buffer, " vdma-internal-error");
    if (status & VDMA_STATUS_REGISTER_VDMASlaveError) strcat(buffer, " vdma-slave-error");
    if (status & VDMA_STATUS_REGISTER_VDMADecodeError) strcat(buffer, " vdma-decode-error");
    if (status & VDMA_STATUS_REGISTER_StartOfFrameEarlyError) strcat(buffer, " start-of-frame-early-error");
    if (status & VDMA_STATUS_REGISTER_EndOfLineEarlyError) strcat(buffer, " end-of-line-early-error");
    if (status & VDMA_STATUS_REGISTER_StartOfFrameLateError) strcat(buffer, " start-of-frame-late-error");
    if (status & VDMA_STATUS_REGISTER_FrameCountInterrupt) strcat(buffer, " frame-count-interrupt");
    if (status & VDMA_STATUS_REGISTER_DelayCountInterrupt) strcat(buffer, " delay-count-interrupt");
    if (status & VDMA_STATUS_REGISTER_ErrorInterrupt) strcat(buffer, " error-interrupt");
    if (status & VDMA_STATUS_REGISTER_EndOfLineLateError) strcat(buffer, " end-of-line-late-error");
    strcat(buffer, " frame-count:%d delay-count:%d\r\n");
    xil_printf(buffer, (status & VDMA_STATUS_REGISTER_FrameCount) >> 16, (status & VDMA_STATUS_REGISTER_DelayCount) >> 24);
}

void debug_vdma(UINTPTR addr) {
	u32 status = Xil_In32(addr + MM2S_VDMASR);
	xil_printf("VDMA_MM2S_STATUS_REGISTER is %08x.", status);
	vdma_status_dump(status);
	status = Xil_In32(addr + S2MM_VDMASR);
	xil_printf("VDMA_S2MM_STATUS_REGISTER is %08x.", status);
	vdma_status_dump(status);
}

int AxiVDMASelfTestExample(XAxiVdma* ptr, u16 DeviceId)
{
	XAxiVdma_Config *Config;
	int Status = XST_SUCCESS;

	Config = XAxiVdma_LookupConfig(DeviceId);
	if (!Config) {
			return XST_FAILURE;
	}

	/* Initialize DMA engine */
	Status = XAxiVdma_CfgInitialize(ptr, Config, Config->BaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	Status = XAxiVdma_Selftest(ptr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return Status;
}

void init_vdma_buffer(UINTPTR addr, u32 fb1, u32 fb2, u32 fb3)
{
    Xil_Out32(addr + MM2S_VDMACR,
    		(255 << 16) | 0x8 | 0x80 | 0x2 | 0x1);
    Xil_Out32(addr + S2MM_VDMACR,
    		(255 << 16) | 0x8 | 0x80 | 0x2 | 0x1);

    Xil_Out32(addr + MM2S_START_ADDRESS, fb1);
    Xil_Out32(addr + MM2S_START_ADDRESS + 4, fb2);
    Xil_Out32(addr + MM2S_START_ADDRESS + 8, fb3);

    Xil_Out32(addr + S2MM_START_ADDRESS, fb1);
    Xil_Out32(addr + S2MM_START_ADDRESS + 4, fb2);
    Xil_Out32(addr + S2MM_START_ADDRESS + 8, fb3);

    Xil_Out32(addr + MM2S_FRMDLY_STRIDE,HSIZE_ACTIVE*PIXEL_SIZE);
    Xil_Out32(addr + S2MM_FRMDLY_STRIDE,HSIZE_ACTIVE*PIXEL_SIZE | (2 << 24));

    Xil_Out32(addr + MM2S_HSIZE,HSIZE_ACTIVE*PIXEL_SIZE);
    Xil_Out32(addr + S2MM_HSIZE,HSIZE_ACTIVE*PIXEL_SIZE);

    Xil_Out32(addr + S2MM_VSIZE,VSIZE_ACTIVE);
    Xil_Out32(addr + MM2S_VSIZE,VSIZE_ACTIVE);
}

int main()
{
	init_platform();

	xil_printf("VDMA image transfer \n\r");

	// VDMA Self test
	int Status = AxiVDMASelfTestExample(&AxiVdmaFrameBuffering, XPAR_FRAME_BUFFER_VDMA_PREVIOUS_FRAME_DEVICE_ID);
    if (Status != XST_SUCCESS)
    	xil_printf("AxiVDMASelfTestExample PREVIOUS_FRAME: Failed\r\n");
    else
    	xil_printf("AxiVDMASelfTestExample PREVIOUS_FRAME: Passed\r\n");
	Status = AxiVDMASelfTestExample(&AxiVdmaBackgroundBuffering, XPAR_BACKGROUND_BUFFER_VDMA_PREVIOUS_FRAME_DEVICE_ID);
    if (Status != XST_SUCCESS)
    	xil_printf("AxiVDMASelfTestExample BACKGROUND_BUFFER: Failed\r\n");
    else
    	xil_printf("AxiVDMASelfTestExample BACKGROUND_BUFFER: Passed\r\n");

	Status = AxiVDMASelfTestExample(&AxiVdmaResultFrame, XPAR_AXI_VDMA_RESULT_DEVICE_ID);
    if (Status != XST_SUCCESS)
    	xil_printf("AxiVDMASelfTestExample RESULT_FRAME: Failed\r\n");
    else
    	xil_printf("AxiVDMASelfTestExample RESULT_FRAME: Passed\r\n");

    // parameters
//    Xil_Out32(XPAR_ALGORITHM_PARAMETERS_0_CTL_BASEADDR + ALPHA_REGISTER, ~(1 << 7));
//    Xil_Out32(XPAR_ALGORITHM_PARAMETERS_0_CTL_BASEADDR + BG_TH_REGISTER, 25);
//    Xil_Out32(XPAR_ALGORITHM_PARAMETERS_0_CTL_BASEADDR + FD_TH_REGISTER, 30);
    Xil_Out32(XPAR_ALGORITHM_PARAMETERS_0_CTL_BASEADDR + ALPHA_REGISTER, 32);
    Xil_Out32(XPAR_ALGORITHM_PARAMETERS_0_CTL_BASEADDR + BG_TH_REGISTER, 25);
    Xil_Out32(XPAR_ALGORITHM_PARAMETERS_0_CTL_BASEADDR + FD_TH_REGISTER, 30);

    // hpd
    Xil_Out32(XPAR_GPIO_0_BASEADDR, 0);
    Xil_Out32(XPAR_GPIO_0_BASEADDR + 4, 0);
    Xil_Out32(XPAR_GPIO_0_BASEADDR, 0xFFFFFFFF);

    // vdma frame buffering
    init_vdma_buffer(XPAR_FRAME_BUFFER_VDMA_PREVIOUS_FRAME_BASEADDR,
    		(u32)&framebuffer1, (u32)&framebuffer2, (u32)&framebuffer3);

    // vdma background buffering
    init_vdma_buffer(XPAR_BACKGROUND_BUFFER_VDMA_PREVIOUS_FRAME_BASEADDR,
    		(u32)&framebuffer4, (u32)&framebuffer5, (u32)&framebuffer6);

    // vdma background buffering
    init_vdma_buffer(XPAR_AXI_VDMA_RESULT_BASEADDR,
    		(u32)&framebuffer7, (u32)&framebuffer8, (u32)&framebuffer9);

    xil_printf("Config - done\r\n");
	while(1) {
		xil_printf("XPAR_FRAME_BUFFER_VDMA_PREVIOUS_FRAME\r\n");
		debug_vdma(XPAR_FRAME_BUFFER_VDMA_PREVIOUS_FRAME_BASEADDR);
		xil_printf("XPAR_BACKGROUND_BUFFER_VDMA_PREVIOUS_FRAME\r\n");
		debug_vdma(XPAR_BACKGROUND_BUFFER_VDMA_PREVIOUS_FRAME_BASEADDR);
		xil_printf("XPAR_AXI_VDMA_RESULT\r\n");
		debug_vdma(XPAR_AXI_VDMA_RESULT_BASEADDR);
		sleep(1);
	}
	cleanup_platform();
	return 0;
}
