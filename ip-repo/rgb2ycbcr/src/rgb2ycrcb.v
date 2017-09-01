`timescale 1ns / 1ps

module rgb2ycbcr (
	input clk,
	input ce,
	
	input [23:0] in_data,
	input in_hsync,          // hsync data
	input in_vsync,          // vsync data
	input in_de,
	
	output [23:0] out_data,
	output out_hsync,          // hsync data
	output out_vsync,          // vsync data
	output out_de
);
wire [7:0] R;
wire [7:0] G;
wire [7:0] B;

wire [7:0] Y;
wire [7:0] Cb;
wire [7:0] Cr;

assign R = in_data[23:16];
assign G = in_data[7:0];
assign B = in_data[15:8];

reg signed [17:0] matrix_coef1_1 = 18'b001001100100010110;
reg signed [17:0] matrix_coef1_2 = 18'b010010110010001011;
reg signed [17:0] matrix_coef1_3 = 18'b000011101001011110;
reg signed [17:0] matrix_coef2_1 = 18'b111010100110011011;
reg signed [17:0] matrix_coef2_2 = 18'b110101011001100100;
reg signed [17:0] matrix_coef2_3 = 18'b010000000000000000;
reg signed [17:0] matrix_coef3_1 = 18'b010000000000000000;
reg signed [17:0] matrix_coef3_2 = 18'b110010100110100001;
reg signed [17:0] matrix_coef3_3 = 18'b111101011001011110;

reg signed [8:0] vector_coef1 = 9'b000000000;
reg signed [8:0] vector_coef2 = 9'b010000000;
reg signed [8:0] vector_coef3 = 9'b010000000;

wire signed [35:0] mul1_1;
wire signed [35:0] mul1_2;
wire signed [35:0] mul1_3;
wire signed [35:0] mul2_1;
wire signed [35:0] mul2_2;
wire signed [35:0] mul2_3;
wire signed [35:0] mul3_1;
wire signed [35:0] mul3_2;
wire signed [35:0] mul3_3;

wire signed [8:0] sum1_1;
wire signed [8:0] sum1_2;
wire signed [8:0] sum1_3;
wire signed [8:0] sum2_1;
wire signed [8:0] sum2_2;
wire signed [8:0] sum2_3;
wire signed [8:0] sum3_1;
wire signed [8:0] sum3_2;
wire signed [8:0] sum3_3;

//Y
multiply mult1_1 (
  .CLK(clk), // input clk
  .A(matrix_coef1_1), // input [17 : 0] a
  .B({10'b0, R}), // input [17 : 0] b
  .P(mul1_1) // output [35 : 0] p
);

multiply mult1_2 (
  .CLK(clk), // input clk
  .A(matrix_coef1_2), // input [17 : 0] a
  .B({10'b0, G}), // input [17 : 0] b
  .P(mul1_2) // output [35 : 0] p
);

multiply mult1_3 (
  .CLK(clk), // input clk
  .A(matrix_coef1_3), // input [17 : 0] a
  .B({10'b0, B}), // input [17 : 0] b
  .P(mul1_3) // output [35 : 0] p
);

sum summ1_1 (
  .A(mul1_1[25:17]), // input [8 : 0] a
  .B(mul1_2[25:17]), // input [8 : 0] b
  .CLK(clk), // input clk
  .S(sum1_1) // output [8 : 0] s
);

sum summ1_2 (
  .A(mul1_3[25:17]), // input [8 : 0] a
  .B(vector_coef1), // input [8 : 0] b
  .CLK(clk), // input clk
  .S(sum1_2) // output [8 : 0] s
);


sum summ1_3 (
  .A(sum1_1), // input [8 : 0] a
  .B(sum1_2), // input [8 : 0] b
  .CLK(clk), // input clk
  .S(sum1_3) // output [8 : 0] s
);

// Cb
multiply mult2_1 (
  .CLK(clk), // input clk
  .A(matrix_coef2_1), // input [17 : 0] a
  .B({10'b0, R}), // input [17 : 0] b
  .P(mul2_1) // output [35 : 0] p
);

multiply mult2_2 (
  .CLK(clk), // input clk
  .A(matrix_coef2_2), // input [17 : 0] a
  .B({10'b0, G}), // input [17 : 0] b
  .P(mul2_2) // output [35 : 0] p
);

multiply mult2_3 (
  .CLK(clk), // input clk
  .A(matrix_coef2_3), // input [17 : 0] a
  .B({10'b0, B}), // input [17 : 0] b
  .P(mul2_3) // output [35 : 0] p
);

sum summ2_1 (
  .A(mul2_1[25:17]), // input [8 : 0] a
  .B(mul2_2[25:17]), // input [8 : 0] b
  .CLK(clk), // input clk
  .S(sum2_1) // output [8 : 0] s
);

sum summ2_2 (
  .A(mul2_3[25:17]), // input [8 : 0] a
  .B(vector_coef2), // input [8 : 0] b
  .CLK(clk), // input clk
  .S(sum2_2) // output [8 : 0] s
);


sum summ2_3 (
  .A(sum2_1), // input [8 : 0] a
  .B(sum2_2), // input [8 : 0] b
  .CLK(clk), // input clk
  .S(sum2_3) // output [8 : 0] s
);

//Cr
multiply mult3_1 (
  .CLK(clk), // input clk
  .A(matrix_coef3_1), // input [17 : 0] a
  .B({10'b0, R}), // input [17 : 0] b
  .P(mul3_1) // output [35 : 0] p
);

multiply mult3_2 (
  .CLK(clk), // input clk
  .A(matrix_coef3_2), // input [17 : 0] a
  .B({10'b0, G}), // input [17 : 0] b
  .P(mul3_2) // output [35 : 0] p
);

multiply mult3_3 (
  .CLK(clk), // input clk
  .A(matrix_coef3_3), // input [17 : 0] a
  .B({10'b0, B}), // input [17 : 0] b
  .P(mul3_3) // output [35 : 0] p
);

sum summ3_1 (
  .A(mul3_1[25:17]), // input [8 : 0] a
  .B(mul3_2[25:17]), // input [8 : 0] b
  .CLK(clk), // input clk
  .S(sum3_1) // output [8 : 0] s
);

sum summ3_2 (
  .A(mul3_3[25:17]), // input [8 : 0] a
  .B(vector_coef3), // input [8 : 0] b
  .CLK(clk), // input clk
  .S(sum3_2) // output [8 : 0] s
);


sum summ3_3 (
  .A(sum3_1), // input [8 : 0] a
  .B(sum3_2), // input [8 : 0] b
  .CLK(clk), // input clk
  .S(sum3_3) // output [8 : 0] s
);


//moduly opozniajace
wire [2:0] syncs;
delay #
(
	.N(3),
	.DELAY(5)
)
delay_sync
(
	.d({in_vsync, in_hsync, in_de}),
	.ce(ce),
	.clk(clk),
	.q(syncs)
);

//przepisanie wynikow
assign Y = 	sum1_3[7:0];
assign Cb = sum2_3[7:0];
assign Cr = sum3_3[7:0];
assign out_data = (ce == 1'b1) ? {Y,Cb,Cr} : in_data;

assign out_de = (ce == 1'b1) ? syncs[0] : in_de;
assign out_hsync = (ce == 1'b1) ? syncs[1] : in_hsync;
assign out_vsync = (ce == 1'b1) ? syncs[2] : in_vsync;

endmodule