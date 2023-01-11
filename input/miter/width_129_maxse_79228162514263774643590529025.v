module maxse_mit(a, b, f);
parameter _bit = 129;
parameter maxse = 79228162514263774643590529025;
input [_bit - 1: 0] a;
input [_bit - 1: 0] b;
output f;
wire [_bit - 1: 0] diff;
wire [_bit * 2 - 1: 0] se;
assign diff = (a > b)? (a - b): (b - a);
assign se = diff * diff;
assign f = (se > maxse);
endmodule
