`timescale 1ns / 1ps

module background_model_compute (
        input wire clk,
        input wire ce,
        
        input wire [7:0] in_image,
        input wire [7:0] in_background,
        
        input wire [7:0] alpha,
        input wire [7:0] one_minus_alpha,
        
        output wire [7:0] out_background
    );
    
    wire [15:0] mult_pixel_value;
    wire [15:0] mult_background_value;
    wire [8:0] sum_background_value;
    wire [7:0] in_background_d;
    wire ce_d;
    
    multiply_u8xu8_l3 mult_pixel (
        .A(in_image),
        .B(one_minus_alpha),
        .CLK(clk),
        .P(mult_pixel_value)
    );
    
    multiply_u8xu8_l3 mult_background (
        .A(in_background),
        .B(alpha),
        .CLK(clk),
        .P(mult_background_value)
    );
    
    add_u8xu8_l1 sum_result (
        .A(mult_pixel_value[14:7]),
        .B(mult_background_value[14:7]),
        .CLK(clk),
        .ADD(1'b1),
        .S(sum_background_value)
    );

    delay # (
        .N(9),
        .DELAY(4)
    ) delay_background_ce (
        .d({in_background, ce}),
        .clk(clk),
        .q({in_background_d, ce_d})
    );
    assign out_background = (ce_d == 1'b1) ? ((sum_background_value[8] == 1'b1) ? 8'd255 : sum_background_value[7:0]) : (in_background_d);
    
endmodule
