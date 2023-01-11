module wce_mit(a, b, f);
parameter _bit = 256;
parameter wce = 340282366920938463463374607431768211456;
input [_bit - 1: 0] a;
input [_bit - 1: 0] b;
output f;
wire [_bit - 1: 0] diff;
assign diff = (a > b)? (a - b): (b - a);
assign f = (diff > wce);
endmodule
