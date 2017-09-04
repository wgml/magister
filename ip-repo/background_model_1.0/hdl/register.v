`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date:    17:09:22 03/11/2015 
// Design Name: 
// Module Name:    register 
// Project Name: 
// Target Devices: 
// Tool versions: 
// Description: 
//
// Dependencies: 
//
// Revision: 
// Revision 0.01 - File Created
// Additional Comments: 
//
//////////////////////////////////////////////////////////////////////////////////
module register #
(
	parameter N = 1
)
(
	input clk,
	input [N - 1:0] d,
	output [N - 1:0] q
);
reg [N - 1:0] val = 0;

always @(posedge clk)
begin
	val <= d;
end

assign q = val;

endmodule