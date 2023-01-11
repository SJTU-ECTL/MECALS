module binsqrd (
	input [7:0] a,
	input [7:0] b,
	output [17:0] o
	
);
	wire [15:0] a_sqrd;
	wire [15:0] b_sqrd;
	wire [16:0] abx2;

	assign a_sqrd = a * a;
	assign b_sqrd = b * b;
	assign abx2 = 2 * a * b;

	assign o = a_sqrd + abx2 + b_sqrd;
endmodule