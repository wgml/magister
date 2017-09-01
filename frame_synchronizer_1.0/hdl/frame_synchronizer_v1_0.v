
`timescale 1 ns / 1 ps

	module frame_synchronizer_v1_0 #
	(
		parameter integer TDATA_WIDTH	= 32
	)
	(
	
	    output wire fsync,
	    
		input wire  aclk,
		input wire  aresetn,
		output wire  input_frame_tready,
		input wire [TDATA_WIDTH-1 : 0] input_frame_tdata,
		input wire  input_frame_tuser,
		input wire  input_frame_tlast,
		input wire  input_frame_tvalid,

		// Ports of Axi Master Bus Interface output_frame
		output wire  output_frame_tvalid,
		output wire [TDATA_WIDTH-1 : 0] output_frame_tdata,
		output wire  output_frame_tuser,
		output wire  output_frame_tlast,
		input wire  output_frame_tready
	);
	
	assign output_frame_tvalid = input_frame_tvalid;
	assign output_frame_tdata = input_frame_tdata;
	assign output_frame_tuser = input_frame_tuser;
	assign output_frame_tlast = input_frame_tlast;
	assign input_frame_tready = output_frame_tready;
	assign fsync = input_frame_tuser;
	endmodule
