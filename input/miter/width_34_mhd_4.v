module mhd_mit(a, b, f);
parameter _bit = 34;
parameter mhd = 4;
input [_bit - 1: 0] a;
input [_bit - 1: 0] b;
output f;
wire [_bit - 1: 0] diff;
wire [6: 0] sum;
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
assign diff[16] = a[16] ^ b[16];
assign diff[17] = a[17] ^ b[17];
assign diff[18] = a[18] ^ b[18];
assign diff[19] = a[19] ^ b[19];
assign diff[20] = a[20] ^ b[20];
assign diff[21] = a[21] ^ b[21];
assign diff[22] = a[22] ^ b[22];
assign diff[23] = a[23] ^ b[23];
assign diff[24] = a[24] ^ b[24];
assign diff[25] = a[25] ^ b[25];
assign diff[26] = a[26] ^ b[26];
assign diff[27] = a[27] ^ b[27];
assign diff[28] = a[28] ^ b[28];
assign diff[29] = a[29] ^ b[29];
assign diff[30] = a[30] ^ b[30];
assign diff[31] = a[31] ^ b[31];
assign diff[32] = a[32] ^ b[32];
assign diff[33] = a[33] ^ b[33];
assign sum = diff[0] + diff[1] + diff[2] + diff[3] + diff[4] + diff[5] + diff[6] + diff[7] + diff[8] + diff[9] + diff[10] + diff[11] + diff[12] + diff[13] + diff[14] + diff[15] + diff[16] + diff[17] + diff[18] + diff[19] + diff[20] + diff[21] + diff[22] + diff[23] + diff[24] + diff[25] + diff[26] + diff[27] + diff[28] + diff[29] + diff[30] + diff[31] + diff[32] + diff[33];
assign f = (sum > mhd);
endmodule
