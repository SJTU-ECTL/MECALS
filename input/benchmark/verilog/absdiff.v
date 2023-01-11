module absdiff (
	input [7:0] a,
	input [7:0] b,
	output [7:0] o
	
);
	assign o = (a>b) ? a-b : b-a;
endmodule