
`timescale 1 ns / 1 ps

	module algorithm_parameters_v1_0 #
	(
		// Users to add parameters here

		// User parameters ends
		// Do not modify the parameters beyond this line


		// Parameters of Axi Slave Bus Interface ctl
		parameter integer C_ctl_DATA_WIDTH	= 32,
		parameter integer C_ctl_ADDR_WIDTH	= 5
	)
	(
		// Users to add ports here
        output wire [7:0] bg_th,
        output wire [7:0] fd_th,
        output wire [7:0] alpha,
		// User ports ends
		// Do not modify the ports beyond this line


		// Ports of Axi Slave Bus Interface ctl
		input wire  ctl_aclk,
		input wire  ctl_aresetn,
		input wire [C_ctl_ADDR_WIDTH-1 : 0] ctl_awaddr,
		input wire [2 : 0] ctl_awprot,
		input wire  ctl_awvalid,
		output wire  ctl_awready,
		input wire [C_ctl_DATA_WIDTH-1 : 0] ctl_wdata,
		input wire [(C_ctl_DATA_WIDTH/8)-1 : 0] ctl_wstrb,
		input wire  ctl_wvalid,
		output wire  ctl_wready,
		output wire [1 : 0] ctl_bresp,
		output wire  ctl_bvalid,
		input wire  ctl_bready,
		input wire [C_ctl_ADDR_WIDTH-1 : 0] ctl_araddr,
		input wire [2 : 0] ctl_arprot,
		input wire  ctl_arvalid,
		output wire  ctl_arready,
		output wire [C_ctl_DATA_WIDTH-1 : 0] ctl_rdata,
		output wire [1 : 0] ctl_rresp,
		output wire  ctl_rvalid,
		input wire  ctl_rready
	);
// Instantiation of Axi Bus Interface ctl
	algorithm_parameters_v1_0_ctl # ( 
		.C_S_AXI_DATA_WIDTH(C_ctl_DATA_WIDTH),
		.C_S_AXI_ADDR_WIDTH(C_ctl_ADDR_WIDTH)
	) algorithm_parameters_v1_0_ctl_inst (
		.S_AXI_ACLK(ctl_aclk),
		.S_AXI_ARESETN(ctl_aresetn),
		.S_AXI_AWADDR(ctl_awaddr),
		.S_AXI_AWPROT(ctl_awprot),
		.S_AXI_AWVALID(ctl_awvalid),
		.S_AXI_AWREADY(ctl_awready),
		.S_AXI_WDATA(ctl_wdata),
		.S_AXI_WSTRB(ctl_wstrb),
		.S_AXI_WVALID(ctl_wvalid),
		.S_AXI_WREADY(ctl_wready),
		.S_AXI_BRESP(ctl_bresp),
		.S_AXI_BVALID(ctl_bvalid),
		.S_AXI_BREADY(ctl_bready),
		.S_AXI_ARADDR(ctl_araddr),
		.S_AXI_ARPROT(ctl_arprot),
		.S_AXI_ARVALID(ctl_arvalid),
		.S_AXI_ARREADY(ctl_arready),
		.S_AXI_RDATA(ctl_rdata),
		.S_AXI_RRESP(ctl_rresp),
		.S_AXI_RVALID(ctl_rvalid),
		.S_AXI_RREADY(ctl_rready),
		.alpha(alpha),
		.bg_th(bg_th),
		.fd_th(fd_th)
	);
	endmodule
