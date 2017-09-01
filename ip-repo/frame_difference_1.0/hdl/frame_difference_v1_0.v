`timescale 1 ns / 1 ps

	module background_model_v1_0 # (
		parameter integer TDATA_WIDTH	= 24
	)
	(
		input wire  aclk,
		input wire  aresetn,
		input wire  ce,
		
		input wire [7:0] bg_th,
		input wire [7:0] fd_th,
		input wire [7:0] alpha,
		
		output wire  input_frames_tready,
		input wire [3*TDATA_WIDTH-1 : 0] input_frames_tdata,
		input wire  input_frames_tuser,
		input wire  input_frames_tlast,
		input wire  input_frames_tvalid,

		output wire  output_frames_tvalid,
		output wire [3*TDATA_WIDTH-1 : 0] output_frames_tdata,
		output wire  output_frames_tuser,
		output wire  output_frames_tlast,
		input wire  output_frames_tready,
		
		input wire force_ready
	);
    wire  background_model_tvalid;
    wire [TDATA_WIDTH-1 : 0] background_model_tdata;
    wire  background_model_tuser;
    wire  background_model_tlast;
    
    wire  foreground_mask_tvalid;
    wire [TDATA_WIDTH-1 : 0] foreground_mask_tdata;
    wire  foreground_mask_tuser;
    wire  foreground_mask_tlast;

    wire  movement_mask_tvalid;
    wire [TDATA_WIDTH-1 : 0] movement_mask_tdata;
    wire  movement_mask_tuser;
    wire  movement_mask_tlast;

    wire [TDATA_WIDTH-1 : 0] current_frame = input_frames_tdata[TDATA_WIDTH-1 : 0];
    wire [TDATA_WIDTH-1 : 0] previous_frame = input_frames_tdata[2*TDATA_WIDTH-1 : TDATA_WIDTH];
    wire [TDATA_WIDTH-1 : 0] background_data = input_frames_tdata[3*TDATA_WIDTH-1 : 2*TDATA_WIDTH];

    wire [TDATA_WIDTH-1:0] background_data_mux[1:0];
    wire [TDATA_WIDTH-1:0] foreground_data_mux[1:0];
    wire [TDATA_WIDTH-1:0] movement_data_mux[1:0];
    wire tuser_mux[1:0];
    wire tlast_mux[1:0];
    wire tvalid_mux[1:0];
    
    assign background_data_mux[0] = {current_frame};
    assign foreground_data_mux[0] = {current_frame};
    assign movement_data_mux[0] = {current_frame};
    
    assign tuser_mux[0] = input_frames_tuser;
    assign tlast_mux[0] = input_frames_tlast;
    assign tvalid_mux[0] = input_frames_tvalid;
    
    background_model_impl #(
        .TDATA_WIDTH(TDATA_WIDTH)
    ) impl (
        .current_frame(current_frame),
        .previous_frame(previous_frame),
        .background_frame(background_data),
        .alpha(alpha),
        .bg_th(bg_th),
        .fd_th(fd_th),
        .clk(aclk),
        .obackground_frame(background_data_mux[1][TDATA_WIDTH-1:0]),
        .oforeground_mask(foreground_data_mux[1][TDATA_WIDTH-1:0]),
        .omovement_mask(movement_data_mux[1][TDATA_WIDTH-1:0])
    );
    
    delay #(
        .N(3),
        .DELAY(4)
    ) delay_signals (
        .d({input_frames_tuser, input_frames_tlast, input_frames_tvalid}),
        .ce(1'b1),
        .clk(aclk),
        .q({tuser_mux[1], tlast_mux[1], tvalid_mux[1]})
    );
    
    assign output_frames_tvalid = tvalid_mux[ce];
    assign output_frames_tdata = {movement_data_mux[ce], foreground_data_mux[ce], background_data_mux[ce]};
    assign output_frames_tuser = tuser_mux[ce];
    assign output_frames_tlast = tlast_mux[ce];
    assign input_frames_tready = force_ready | output_frames_tready;

	endmodule
