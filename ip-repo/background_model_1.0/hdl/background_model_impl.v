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
    
    wire [7:0] difference_y;
    wire [7:0] difference_cb;
    wire [7:0] difference_cr;
    
    // d = 1
    absolute_difference foreground_y_diff
    (
        .a(current_frame[23:16]),
        .b(background_frame[23:16]),
        .result(difference_y),
        .clk(clk)
    );
    absolute_difference foreground_cb_diff
    (
        .a(current_frame[15:8]),
        .b(background_frame[15:8]),
        .result(difference_cb),
        .clk(clk)
    );
    absolute_difference foreground_cr_diff
    (
        .a(current_frame[7:0]),
        .b(background_frame[7:0]),
        .result(difference_cr),
        .clk(clk)
    );

    wire [8:0] difference_cbcr;
    add_u8xu8_l1 difference_cbcr_compute
    (
        .A(difference_cb),
        .B(difference_cr),
        .S(difference_cbcr),
        .CLK(clk),
        .ADD(1'b1)
    );
    
    wire [7:0] difference_y_d1;
    delay #(
        .N(8),
        .DELAY(1)
    ) delay_y (
        .clk(clk),
        .d(difference_y),
        .q(difference_y_d1)
    );
    wire [8:0] difference_weighted;
    add_u8xu8_l1 difference_weighted_compute
    (
        .A(difference_y_d1),
        .B({difference_cbcr[6:0], 1'b0}),
        .S(difference_weighted),
        .CLK(clk),
        .ADD(1'b1)
    );
    
    wire [1:0] difference_cbcr_d1;
    delay #(
        .N(2),
        .DELAY(1)
    ) delay_cbcr (
        .clk(clk),
        .d(difference_cbcr[8:7]),
        .q(difference_cbcr_d1)
    );

    // entire delay for foreground_mask computation = 3
    wire foreground_mask = ((difference_cbcr_d1 > 2'b0) | (difference_weighted > {1'b0, bg_th})) ? 1'b1 : 1'b0;
    
    wire [7:0] movement_mask_v;
    absolute_difference movement_diff
    (
        .a(current_frame[23:16]),
        .b(previous_frame[23:16]),
        .result(movement_mask_v),
        .clk(clk)
    );
    
    // entire delay for movement_mask computation = 1
    wire movement_mask = (movement_mask_v > fd_th) ? 1'b1 : 1'b0;
    
    wire movement_mask_d2;
    delay #(
        .N(1),
        .DELAY(2)
    ) delay_movement_mask (
        .clk(clk),
        .d(movement_mask),
        .q(movement_mask_d2)
    );    
    
    // entire delay for is_in_background computation = 3
    wire is_in_background = (foreground_mask == 1'b0) && (movement_mask_d2 == 1'b0);

    
    wire [23:0] current_frame_d3;
    wire [23:0] background_frame_d3;
    
    delay #(
        .N(48),
        .DELAY(3)
    ) delay_frames (
        .clk(clk),
        .d({current_frame, background_frame}),
        .q({current_frame_d3, background_frame_d3})
    );    
    
    // d = 4
    background_model_compute Y (
        .clk(clk),
        .ce(is_in_background),
        .in_image(current_frame_d3[23:16]),
        .in_background(background_frame_d3[23:16]),
        .alpha(alpha),
        .one_minus_alpha(one_minus_alpha),
        .out_background(obackground_frame[23:16])
    );
    
    // d = 4
    background_model_compute Cb (
        .clk(clk),
        .ce(is_in_background),
        .in_image(current_frame_d3[15:8]),
        .in_background(background_frame_d3[15:8]),
        .alpha(alpha),
        .one_minus_alpha(one_minus_alpha),
        .out_background(obackground_frame[15:8])
    );

    // d = 4
    background_model_compute Cr (
        .clk(clk),
        .ce(is_in_background),
        .in_image(current_frame_d3[7:0]),
        .in_background(background_frame_d3[7:0]),
        .alpha(alpha),
        .one_minus_alpha(one_minus_alpha),
        .out_background(obackground_frame[7:0])
    );
    
        
    wire foreground_mask_d4;
    wire movement_mask_d4;
    delay #(
        .N(2),
        .DELAY(4)
    ) delay_masks (
        .clk(clk),
        .d({foreground_mask, movement_mask_d2}),
        .q({foreground_mask_d4, movement_mask_d4})
    );

    assign oforeground_mask = (foreground_mask_d4 == 1'b1) ? {8'd255, 8'd255, 8'd255} : 24'd0;
    assign omovement_mask = (movement_mask_d4 == 1'b1) ? {8'd255, 8'd255, 8'd255} : 24'd0;
endmodule
