module mhd_mit(a, b, f);
parameter _bit = 16;
parameter mhd = 5;
input [_bit - 1: 0] a;
input [_bit - 1: 0] b;
output f;
wire [_bit - 1: 0] diff;
wire [4: 0] sum;
assign diff[0] = a[0] ^ b[0];
assign diff[1] = a[1] ^ b[1];
assign diff[2] = a[2] ^ b[2];
assign diff[3] = a[3] ^ b[3];
assign diff[4] = a[4] ^ b[4];
assign diff[5] = a[5] ^ b[5];
assign diff[6] = a[6] ^ b[6];
assign diff[7] = a[7] ^ b[7];
assign diff[8] = a[8] ^ b[8];
assign diff[9] = a[9] ^ b[9];
assign diff[10] = a[10] ^ b[10];
assign diff[11] = a[11] ^ b[11];
assign diff[12] = a[12] ^ b[12];
assign diff[13] = a[13] ^ b[13];
assign diff[14] = a[14] ^ b[14];
assign diff[15] = a[15] ^ b[15];
assign sum = diff[0] + diff[1] + diff[2] + diff[3] + diff[4] + diff[5] + diff[6] + diff[7] + diff[8] + diff[9] + diff[10] + diff[11] + diff[12] + diff[13] + diff[14] + diff[15];
assign f = (sum > mhd);
endmodule
