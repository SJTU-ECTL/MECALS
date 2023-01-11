import os
import time
import psutil


def GenMit(width, wce, baseName):
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


wce = 16
fSASIMI = 1
# circName = 'absdiff'
# nPo = 8
# circName = 'add8u_0FP'
# nPo = 9
circName = 'mac'
nPo = 8

exactPBDPercs = [i * 10 for i in range(11)]

benchmark = 'input/benchmark/muscat/aig/' + circName + '.aig'
for exactPBDPerc in exactPBDPercs:
    mitBase = f'input/miter/width_{nPo}_wce_{wce}'
    GenMit(nPo, wce, mitBase)
    outpPath = f'./result/{circName}/P{exactPBDPerc}'
    os.makedirs(outpPath, exist_ok=True)
    comm = f'./als.out -i {benchmark} -m  {mitBase}.aig -o {outpPath} -p {exactPBDPerc / 100.0} --fSASIMI {fSASIMI} > {outpPath}/log.txt &'
    print(comm)
    os.system(comm)