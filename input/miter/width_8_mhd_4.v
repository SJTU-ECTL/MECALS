module mhd_mit(a, b, f);
parameter _bit = 8;
parameter mhd = 4;
input [_bit - 1: 0] a;
input [_bit - 1: 0] b;
output f;
wire [_bit - 1: 0] diff;
wire [3: 0] sum;
assign diff[0] = a[0] ^ b[0];
assign diff[1] = a[1] ^ b[1];
assign diff[2] = a[2] ^ b[2];
assign diff[3] = a[3] ^ b[3];
assign diff[4] = a[4] ^ b[4];
assign diff[5] = a[5] ^ b[5];
assign diff[6] = a[6] ^ b[6];
assign diff[7] = a[7] ^ b[7];
assign sum = diff[0] + diff[1] + diff[2] + diff[3] + diff[4] + diff[5] + diff[6] + diff[7];
assign f = (sum > mhd);
endmodule
