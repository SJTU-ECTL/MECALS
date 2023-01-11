module maxse_mit(a, b, f);
parameter _bit = 32;
parameter maxse = 11068046439071772;
input [_bit - 1: 0] a;
input [_bit - 1: 0] b;
output f;
wire [_bit - 1: 0] diff;
wire [_bit * 2 - 1: 0] se;
assign diff = (a > b)? (a - b): (b - a);
assign se = diff * diff;
assign f = (se > maxse);
endmodule
