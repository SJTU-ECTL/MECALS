module wce_mit(a, b, f);
parameter _bit = 129;
parameter wce = 36893488147419103232;
input [_bit - 1: 0] a;
input [_bit - 1: 0] b;
output f;
wire [_bit - 1: 0] diff;
assign diff = (a > b)? (a - b): (b - a);
assign f = (diff > wce);
endmodule
