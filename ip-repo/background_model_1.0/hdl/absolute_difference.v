`timescale 1ns / 1ps

module absolute_difference(

    input clk,
    
    input [7:0] a,
    input [7:0] b,
    output [7:0] result
    );
    
    wire [7:0] max = (a > b) ? a : b;
    wire [7:0] min = (a < b) ? a : b;
    
    wire [8:0] diff;
    
    add_u8xu8_l1 compute
    (
        .A(max),
        .B(min),
        .S(diff),
        .ADD(1'b0),
        .CLK(clk)
    );
    
    assign result = diff[7:0];
    
endmodule
