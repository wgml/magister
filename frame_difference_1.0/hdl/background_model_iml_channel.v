`timescale 1ns / 1ps

//latency=4
module background_model_impl_channel (
        input wire [7:0] frame,
        input wire [7:0] background,
        input wire [7:0] alpha,
        input wire [7:0] one_minus_alpha,
        input wire clk,
        output wire [7:0] obackground
    );
    
    wire [15:0] product_frame;
    wire [15:0] product_background;
    wire [8:0] new_background;

    //latency=3
    mult_u8xu8_l3 mult_frame (
        .A(frame),
        .B(alpha),
        .P(product_frame),
        .CLK(clk)
    );
    mult_u8xu8_l3 mult_background (
        .A(background),
        .B(one_minus_alpha),
        .P(product_background),
        .CLK(clk)
    );
    
    //latency=1
    add_u8xu8_l1 sum_background (
        .A(product_frame[14:7]),
        .B(product_background[14:7]),
        .S(new_background),
        .CLK(clk),
        .ADD(1'b1)
    );
    assign obackground[7:0] = (new_background[8] == 1'b1) ? 8'b11111111 : new_background[7:0];
endmodule
