module mhd_mit(a, b, f);
parameter _bit = 64;
parameter mhd = 16;
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
assign diff[34] = a[34] ^ b[34];
assign diff[35] = a[35] ^ b[35];
assign diff[36] = a[36] ^ b[36];
assign diff[37] = a[37] ^ b[37];
assign diff[38] = a[38] ^ b[38];
assign diff[39] = a[39] ^ b[39];
assign diff[40] = a[40] ^ b[40];
assign diff[41] = a[41] ^ b[41];
assign diff[42] = a[42] ^ b[42];
assign diff[43] = a[43] ^ b[43];
assign diff[44] = a[44] ^ b[44];
assign diff[45] = a[45] ^ b[45];
assign diff[46] = a[46] ^ b[46];
assign diff[47] = a[47] ^ b[47];
assign diff[48] = a[48] ^ b[48];
assign diff[49] = a[49] ^ b[49];
assign diff[50] = a[50] ^ b[50];
assign diff[51] = a[51] ^ b[51];
assign diff[52] = a[52] ^ b[52];
assign diff[53] = a[53] ^ b[53];
assign diff[54] = a[54] ^ b[54];
assign diff[55] = a[55] ^ b[55];
assign diff[56] = a[56] ^ b[56];
assign diff[57] = a[57] ^ b[57];
assign diff[58] = a[58] ^ b[58];
assign diff[59] = a[59] ^ b[59];
assign diff[60] = a[60] ^ b[60];
assign diff[61] = a[61] ^ b[61];
assign diff[62] = a[62] ^ b[62];
assign diff[63] = a[63] ^ b[63];
assign sum = diff[0] + diff[1] + diff[2] + diff[3] + diff[4] + diff[5] + diff[6] + diff[7] + diff[8] + diff[9] + diff[10] + diff[11] + diff[12] + diff[13] + diff[14] + diff[15] + diff[16] + diff[17] + diff[18] + diff[19] + diff[20] + diff[21] + diff[22] + diff[23] + diff[24] + diff[25] + diff[26] + diff[27] + diff[28] + diff[29] + diff[30] + diff[31] + diff[32] + diff[33] + diff[34] + diff[35] + diff[36] + diff[37] + diff[38] + diff[39] + diff[40] + diff[41] + diff[42] + diff[43] + diff[44] + diff[45] + diff[46] + diff[47] + diff[48] + diff[49] + diff[50] + diff[51] + diff[52] + diff[53] + diff[54] + diff[55] + diff[56] + diff[57] + diff[58] + diff[59] + diff[60] + diff[61] + diff[62] + diff[63];
assign f = (sum > mhd);
endmodule
