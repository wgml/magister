`timescale 1ns / 1ps

module background_model_impl #
	(
		parameter integer TDATA_WIDTH	= 24
	) (
        input wire [TDATA_WIDTH-1:0] current_frame,
        input wire [TDATA_WIDTH-1:0] previous_frame,
        input wire [TDATA_WIDTH-1:0] background_frame,
        input wire [7:0] alpha,
        input wire [7:0] bg_th,
        input wire [7:0] fd_th,
        
        input wire clk,
        
        output wire [TDATA_WIDTH-1:0] obackground_frame,
        output wire [TDATA_WIDTH-1:0] oforeground_mask,
        output wire [TDATA_WIDTH-1:0] omovement_mask
    );
    
    wire [8:0] one_minus_alpha;
    
    add_u8xu8_l1 one_minus_alpha_compute
    (
        .A(8'b10000000),
        .B(alpha),
        .S(one_minus_alpha),
        .CLK(clk),
        .ADD(1'b0)
    );
    
    wire [7:0] movement_mask_value;
    wire [7:0] difference_y = (current_frame[23:16] > background_frame[23:16]) ? (current_frame[23:16] - background_frame[23:16]) : (background_frame[23:16] - current_frame[23:16]);
    wire [7:0] difference_cb = (current_frame[15:8] > background_frame[15:8]) ? (current_frame[15:8] - background_frame[15:8]) : (background_frame[15:8] - current_frame[15:8]);
    wire [7:0] difference_cr = (current_frame[7:0] > background_frame[7:0]) ? (current_frame[7:0] - background_frame[7:0]) : (background_frame[7:0] - current_frame[7:0]);
    wire [7:0] difference_cbcr = difference_cb + difference_cr;
    
    // for now, Y_WEIGHT = 1, CBCR_WEIGHT = 2
    wire [7:0] difference_weighted = difference_y + {difference_cbcr[6:0], 1'b0};
    wire [7:0] foreground_mask = (difference_weighted > bg_th) ? 8'd255 : 8'd0;
    
    wire [7:0] movement_mask_v = (current_frame[23:16] > previous_frame[23:16]) ? (current_frame[23:16] - previous_frame[23:16]) : (background_frame[23:16] - previous_frame[23:16]);
    wire [7:0] movement_mask = (movement_mask_v > fd_th) ? 8'd255 : 8'd0;
    
    wire is_in_background = (foreground_mask == 8'd0) && (movement_mask == 8'd0);
    
    wire [7:0] foreground_mask_d;
    wire [7:0] movement_mask_d;
    
    background_model_compute Y (
        .clk(clk),
        .ce(is_in_background),
        .in_image(current_frame[23:16]),
        .in_background(background_frame[23:16]),
        .alpha(alpha),
        .one_minus_alpha(one_minus_alpha),
        .out_background(obackground_frame[23:16])
    );
    background_model_compute Cb (
        .clk(clk),
        .ce(is_in_background),
        .in_image(current_frame[15:8]),
        .in_background(background_frame[15:8]),
        .alpha(alpha),
        .one_minus_alpha(one_minus_alpha),
        .out_background(obackground_frame[15:8])
    );

    background_model_compute Cr (
        .clk(clk),
        .ce(is_in_background),
        .in_image(current_frame[7:0]),
        .in_background(background_frame[7:0]),
        .alpha(alpha),
        .one_minus_alpha(one_minus_alpha),
        .out_background(obackground_frame[7:0])
    );
    
    delay #(
        .N(16),
        .DELAY(4)
    ) delay_masks (
        .clk(clk),
        .ce(ce),
        .d({foreground_mask, movement_mask}),
        .q({foreground_mask_d, movement_mask_d})
    );

    assign oforeground_mask = (foreground_mask_d == 8'd255) ? {8'd255, 8'd255, 8'd255} : 24'd0;
    assign omovement_mask = (movement_mask_d == 8'd255) ? {8'd255, 8'd255, 8'd255} : 24'd0;
endmodule
