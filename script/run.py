import os
import time
import psutil
import math


def GenWCEMit(width, wce, baseName):
    verName = baseName + '.v'
    blifName = baseName + '.blif'
    aigName = baseName + '.aig'
    print(verName)
    with open(verName, 'w') as f:
        f.write('module wce_mit(a, b, f);\n')
        f.write('parameter _bit = ' + str(width) + ';\n')
        f.write('parameter wce = ' + str(wce) + ';\n')
        f.write('input [_bit - 1: 0] a;\n')
        f.write('input [_bit - 1: 0] b;\n')
        f.write('output f;\n')
        f.write('wire [_bit - 1: 0] diff;\n')
        f.write('assign diff = (a > b)? (a - b): (b - a);\n')
        f.write('assign f = (diff > wce);\n')
        f.write('endmodule\n')
    comm = 'yosys -p \"read_verilog ' + verName + '; synth; write_blif ' + blifName + '\"'
    print(comm)
    os.system(comm)
    synth = 'st; resyn2; ifraig; '
    comm = 'abc -c \"source input/abc.rc; r ' + blifName + '; ' + synth + synth + synth + ' w ' + aigName + '\"'
    print(comm)
    os.system(comm)


def GenMHDMit(width, mhd, baseName):
    verName = baseName + '.v'
    blifName = baseName + '.blif'
    aigName = baseName + '.aig'
    print(verName)
    with open(verName, 'w') as f:
        f.write('module mhd_mit(a, b, f);\n')
        f.write('parameter _bit = ' + str(width) + ';\n')
        f.write('parameter mhd = ' + str(mhd) + ';\n')
        f.write('input [_bit - 1: 0] a;\n')
        f.write('input [_bit - 1: 0] b;\n')
        f.write('output f;\n')
        f.write('wire [_bit - 1: 0] diff;\n')
        f.write(f'wire [{math.ceil(math.log2(width))}: 0] sum;\n')
        for i in range(width):
            f.write(f'assign diff[{i}] = a[{i}] ^ b[{i}];\n')
        assert width > 0
        f.write(f'assign sum = diff[0]')
        for i in range(1, width):
            f.write(f' + diff[{i}]')
        f.write(';\n')
        f.write('assign f = (sum > mhd);\n')
        f.write('endmodule\n')
    comm = 'yosys -p \"read_verilog ' + verName + '; synth; write_blif ' + blifName + '\"'
    print(comm)
    os.system(comm)
    synth = 'st; resyn2; ifraig; '
    comm = 'abc -c \"source input/abc.rc; r ' + blifName + '; ' + synth + synth + synth + ' w ' + aigName + '\"'
    print(comm)
    os.system(comm)


def GenMaxSEMit(width, maxse, baseName):
    verName = baseName + '.v'
    blifName = baseName + '.blif'
    aigName = baseName + '.aig'
    print(verName)
    with open(verName, 'w') as f:
        f.write('module maxse_mit(a, b, f);\n')
        f.write('parameter _bit = ' + str(width) + ';\n')
        f.write('parameter maxse = ' + str(maxse) + ';\n')
        f.write('input [_bit - 1: 0] a;\n')
        f.write('input [_bit - 1: 0] b;\n')
        f.write('output f;\n')
        f.write('wire [_bit - 1: 0] diff;\n')
        f.write('wire [_bit * 2 - 1: 0] se;\n')
        f.write('assign diff = (a > b)? (a - b): (b - a);\n')
        f.write('assign se = diff * diff;\n')
        f.write('assign f = (se > maxse);\n')
        f.write('endmodule\n')
    comm = 'yosys -p \"read_verilog ' + verName + '; synth; write_blif ' + blifName + '\"'
    print(comm)
    os.system(comm)
    synth = 'st; resyn2; ifraig; '
    comm = 'abc -c \"source input/abc.rc; r ' + blifName + '; ' + synth + synth + synth + ' w ' + aigName + '\"'
    print(comm)
    os.system(comm)


circName = 'rca32'
# wces = [76, 593, 1112, 1650, 2164, 2684, 3221, 3800, 4267, 4793, 5448, 5974, 6494]
mhds = [4, 8, 12, 16]
nPo = 33
# circName = 'absdiff'
# # wces = [1, 4, 7, 10, 13, 16, 19, 22, 25, 28, 31, 34, 37, 40, 43, 46, 50, 53, 56, 59, 62, 65, 69, 73, 76]
# mhds = [1, 2, 3, 4]
# nPo = 8
# circName = 'binsqrd'
# wces = [198, 439, 771, 1153, 1394, 1641, 1910, 2158, 2377, 2625, 2866, 3113, 3630, 3870, 4139, 4380, 4606, 4875, 5116]
# mhds = [2, 4, 6, 9]
# nPo = 18
# circName = 'am8'
# # wces = [179, 346, 524, 703, 882, 1049, 1239, 1406, 1585, 1763, 2383, 3098, 3718, 4337, 4957, 5576, 6196, 6816, 7435, 8055, 8686, 9306, 9926, 10545, 11165, 11784, 12404, 13107]
# mhds = [2, 4, 6, 8]
# nPo = 16
# circName = 'add8u_0FP'
# # wces = [1, 3, 5, 7, 9, 11, 14, 16, 23, 29, 36, 44, 50, 57, 63, 69, 76, 85, 91, 97, 103]
# mhds = [1, 2, 3, 4]
# nPo = 9
# circName = 'mac'
# wces = [2**1, 2**2, 2**3, 2**4]
# mhds = [1, 2, 3, 4]
# nPo = 8
# circName = 'buttfly'
# wces = [2**4, 2**8, 2**12, 2**17]
# mhds = [4, 8, 12, 17]
# nPo = 34
# circName = 'mult16'
# # wces = [2**4, 2**8, 2**12, 2**16]
# mhds = [4, 8, 12, 16]
# nPo = 32
# circName = 'fft'
# wces = [2**32, 2**64, 2**96, 2**128]
# nPo = 256
# circName = 'add128'
# wces = [2**16, 2**32, 2**48, 2**65]
# mhds = [16, 32, 48, 65]
# nPo = 129
# circName = 'mult32'
# # wces = [2**8, 2**16, 2**24, 2**32]
# mhds = [8, 16, 24, 32]
# nPo = 64

# wces = [(2**math.floor((i + 1) / 8 * nPo) - 1) for i in range(4)]
# wces = [(2**math.floor((i + 1) / 8 * nPo) - 1) for i in range(4, 6)]
# maxses = [int(round(0.0001 * (i + 1) * (2**nPo - 1)**2)) for i in range(10)]
maxses = [(2**math.floor((i + 1) / 8 * nPo) - 1)**2 for i in range(4, 6)]
print(maxses)

exactPBDPerc = 0.40
# exactPBDPerc = 0.04
# exactPBDPerc = 0.004
fSASIMI = 1

benchmark = 'input/benchmark/aig/' + circName + '.aig'
# for wce in wces:
#     mitBase = f'input/miter/width_{nPo}_wce_{wce}'
#     GenWCEMit(nPo, wce, mitBase)
#     outpPath = f'./result/{circName}/wce{wce}'
#     os.makedirs(outpPath, exist_ok=True)
#     comm = f'./als.out -i {benchmark} -m {mitBase}.aig -o {outpPath} -p {exactPBDPerc} --fSASIMI {fSASIMI} > {outpPath}/log.txt &'
#     print(comm)
#     os.system(comm)

# for mhd in mhds:
#     mitBase = f'input/miter/width_{nPo}_mhd_{mhd}'
#     GenMHDMit(nPo, mhd, mitBase)
#     outpPath = f'./result/{circName}/mhd{mhd}'
#     os.makedirs(outpPath, exist_ok=True)
#     comm = f'./als.out -i {benchmark} -m {mitBase}.aig -o {outpPath} -p {exactPBDPerc} --fSASIMI {fSASIMI} > {outpPath}/log.txt &'
#     print(comm)
#     os.system(comm)

for maxse in maxses:
    mitBase = f'input/miter/width_{nPo}_maxse_{maxse}'
    GenMaxSEMit(nPo, maxse, mitBase)
    outpPath = f'./result/{circName}/maxse{maxse}'
    os.makedirs(outpPath, exist_ok=True)
    comm = f'./als.out -i {benchmark} -m {mitBase}.aig -o {outpPath} -p {exactPBDPerc} --fSASIMI {fSASIMI} > {outpPath}/log.txt &'
    print(comm)
    os.system(comm)