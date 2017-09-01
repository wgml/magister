`timescale 1 ns / 1 ps

	module frame_difference_v1_0 # (
		parameter integer TDATA_WIDTH	= 24
	)
	(
		input wire  aclk,
		input wire  aresetn,
		input wire ce,
		
		output wire  input_frames_tready,
		input wire [2*TDATA_WIDTH-1 : 0] input_frames_tdata,
		input wire  input_frames_tuser,
		input wire  input_frames_tlast,
		input wire  input_frames_tvalid,

		output wire  output_frame_tvalid,
		output wire [TDATA_WIDTH-1 : 0] output_frame_tdata,
		output wire  output_frame_tuser,
		output wire  output_frame_tlast,
		input wire  output_frame_tready
	);
	
    wire [TDATA_WIDTH-1 : 0] current_frame = input_frames_tdata[TDATA_WIDTH-1 : 0];
    wire [TDATA_WIDTH-1 : 0] previous_frame = input_frames_tdata[2*TDATA_WIDTH-1 : TDATA_WIDTH];

    wire [TDATA_WIDTH-1:0] data_mux[1:0];
    
    assign data_mux[0] = current_frame;
    
    assign data_mux[1][23:16] = (current_frame[23:16] > previous_frame[23:16]) ? (current_frame[23:16] - previous_frame[23:16]) : (previous_frame[23:16] - current_frame[23:16]);
    assign data_mux[1][15:8] = (current_frame[15:8] > previous_frame[15:8]) ? (current_frame[15:8] - previous_frame[15:8]) : (previous_frame[15:8] - current_frame[15:8]);
    assign data_mux[1][7:0] = (current_frame[7:0] > previous_frame[7:0]) ? (current_frame[7:0] - previous_frame[7:0]) : (previous_frame[7:0] - current_frame[7:0]);

    assign output_frame_tvalid = input_frames_tvalid;
    assign output_frame_tdata = data_mux[ce];
    assign output_frame_tuser = input_frames_tuser;
    assign output_frame_tlast = input_frames_tlast;
    assign input_frame_tready = output_frame_tready;

	endmodule
