`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date:    15:05:32 03/11/2015 
// Design Name: 
// Module Name:    delay 
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
module delay #
(
	parameter N = 8,
	parameter DELAY = 0
)
(
	input [N - 1:0] d,
	input clk,
	output [N - 1:0] q
);
wire [N - 1:0] tdata [DELAY:0];

assign tdata[0] = d; //zakladam, że jeśli delay=0, ce nie ma znaczenia

genvar i;
generate
	for(i = 0; i < DELAY; i = i + 1)
	begin: regs
	register #
	(
		.N(N)
	)
	reg_i
	(
		.clk(clk),
		.d(tdata[i]),
		.q(tdata[i + 1])
	);
	end
endgenerate
assign q = tdata[DELAY];
endmodule