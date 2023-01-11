# -*- coding: utf-8 -*-


import os
import subprocess
import operator
from optparse import OptionParser
from blifparser import blifparser


def GetOpt():
    parser = OptionParser()
    parser.add_option('-e', '--exact',
                      action='store',
                      dest='exact',
                      default='./exact.blif',
                      help='path to exact circuit')
    parser.add_option('-a', '--approx',
                      action='store',
                      dest='approx',
                      default='./approx.blif',
                      help='path to approximate circuit')
    parser.add_option('-l', '--lib',
                      action='store',
                      dest='lib',
                      default='./nangate.lib',
                      help='path to standard cell library')
    parser.add_option('-t', '--type',
                      action='store',
                      dest='errorType',
                      default='wce',
                      help='error type: wce or maxse')
    options, _ = parser.parse_args()
    return options


def RunAbc(comm, logFileName=""):
    if logFileName != "":
        subprocess.run(f'abc -c \"{comm}\" >> {logFileName}', shell=True)
    else:
        subprocess.run(f'abc -c \"{comm}\"', shell=True)


def RunYosys(comm, logFileName=""):
    if logFileName != "":
        subprocess.run(f'echo \"yosys -q -p \\"{comm}\\"\" >> {logFileName}', shell=True)
        subprocess.run(f'yosys -q -p \"{comm}\" >> {logFileName}', shell=True)
    else:
        subprocess.run(f'yosys -p \"{comm}\"', shell=True)


def GenMit(width, errorBound, baseName, errorType, logFileName):
    verName = baseName + '.v'
    blifName = baseName + '.blif'
    if errorType == 'wce':
        with open(verName, 'w') as f:
            f.write('module miter(a, b, f);\n')
            f.write('parameter _bit = ' + str(width) + ';\n')
            f.write('parameter wce = ' + str(errorBound) + ';\n')
            f.write('input [_bit - 1: 0] a;\n')
            f.write('input [_bit - 1: 0] b;\n')
            f.write('output f;\n')
            f.write('wire [_bit - 1: 0] diff;\n')
            f.write('assign diff = (a > b)? (a - b): (b - a);\n')
            f.write('assign f = (diff > wce);\n')
            f.write('endmodule\n')
    elif errorType == 'maxse':
        with open(verName, 'w') as f:
            f.write('module miter(a, b, f);\n')
            f.write('parameter _bit = ' + str(width) + ';\n')
            f.write('parameter maxse = ' + str(errorBound) + ';\n')
            f.write('input [_bit - 1: 0] a;\n')
            f.write('input [_bit - 1: 0] b;\n')
            f.write('output f;\n')
            f.write('wire [_bit - 1: 0] diff;\n')
            f.write('wire [_bit * 2 - 1: 0] se;\n')
            f.write('assign diff = (a > b)? (a - b): (b - a);\n')
            f.write('assign se = diff * diff;\n')
            f.write('assign f = (se > maxse);\n')
            f.write('endmodule\n')
    else:
        assert 0
    RunYosys(f'read_verilog {verName}; synth; write_blif {blifName}', logFileName)


def GetIO(blifName):
    assert os.path.exists(blifName)
    blif = blifparser.BlifParser(blifName).blif
    return blif.inputs.inputs, blif.outputs.outputs


def GenTop(top, exact, approx, miter):
    pis, midOuts = GetIO(exact)
    midIns, _ = GetIO(miter)
    nMidOut = len(midOuts)
    assert len(midIns) == nMidOut * 2
    with open(top, 'w') as f:
        f.write('.model top\n')
        # PIs and POs
        f.write('.inputs')
        for pi in pis:
            f.write(f' {pi}')
        f.write('\n')
        f.write('.outputs f\n')
        # sub-circuit: exact circuit
        f.write('.subckt exact')
        for pi in pis:
            f.write(f' {pi}={pi}')
        for i in range(nMidOut):
            f.write(f' {midOuts[i]}=exact_res[{i}]')
        f.write('\n')
        # sub-circuit: approx circuit
        f.write('.subckt approx')
        for pi in pis:
            f.write(f' {pi}={pi}')
        for i in range(nMidOut):
            f.write(f' {midOuts[i]}=approx_res[{i}]')
        f.write('\n')
        # sub-circuit: miter
        f.write('.subckt miter')
        for i in range(nMidOut):
            f.write(f' {midIns[i]}=exact_res[{i}]')
        for i in range(nMidOut, nMidOut * 2):
            f.write(f' {midIns[i]}=approx_res[{i - nMidOut}]')
        f.write(' f=f\n')
        f.write('.end\n')
    subprocess.run(f'cat {exact} >> {top}', shell=True)
    subprocess.run(f'cat {approx} >> {top}', shell=True)
    subprocess.run(f'cat {miter} >> {top}', shell=True)


def ModifyModelName(blifName, newModelName):
    assert os.path.exists(blifName)
    s = subprocess.run(f'sed -n \"2p\" {blifName}', stdout=subprocess.PIPE, shell=True)
    ret = s.stdout.decode("GBK")
    assert ret.find('.model ') != -1
    subprocess.run(f'sed -i \"2c.model {newModelName}\" {blifName}', shell=True)


def IsMaxErrorNoMoreThan(exact, approx, lib, errorType, errorBound):
    # initialize
    tmpFolder = './tmp'
    os.makedirs(tmpFolder, exist_ok=True)
    logFileName = f'{tmpFolder}/verify.log'
    if os.path.exists(logFileName):
        os.remove(logFileName)

    # pre-process exact circuit
    newExact = f'{tmpFolder}/exact.blif'
    RunAbc(f'r {lib}; r {exact}; st; w {newExact}', logFileName)
    ModifyModelName(f'{newExact}', 'exact')

    # pre-process approximate circuit
    newApprox = f'{tmpFolder}/approx.blif'
    RunAbc(f'r {lib}; r {approx}; st; w {newApprox}', logFileName)
    ModifyModelName(f'{newApprox}', 'approx')

    # check the identity of PIs and POs
    pis, pos = GetIO(newExact)
    approxPis, approxPos = GetIO(newApprox)
    assert operator.eq(pis, approxPis) and operator.eq(pos, approxPos)

    # generate miter
    miterBase = f'{tmpFolder}/miter'
    GenMit(len(pos), errorBound, miterBase, errorType, logFileName)

    # generate top module
    top = f'{tmpFolder}/top.blif'
    GenTop(top, newExact, newApprox, f'{miterBase}.blif')

    # solve
    s = subprocess.run(f'abc -c \"r {top}; st; ifraig; resyn2; dsat\" | grep SATISFIABLE', stdout=subprocess.PIPE, shell=True)
    ret = s.stdout.decode("GBK")
    if ret.find('UNSATISFIABLE') != -1:
        return True
    elif ret.find('SATISFIABLE') != -1:
        return False
    else:
        assert 0
        return False


def GetRefBound(approx):
    pos0 = approx.find('Bound')
    pos1 = approx.find('.blif')
    assert pos0 != -1 and pos1 != -1
    return int(approx[pos0 + 5: pos1])


def GetRealBound(exact, approx, lib, refBound, errorType):
    l = 0
    r = refBound
    while l < r:
        mid = (l + r) // 2
        # print(l, r, mid)
        ret = IsMaxErrorNoMoreThan(exact, approx, lib, errorType, mid)
        print(f'Max error <= {mid}: {ret}')
        if not ret: # bound > mid, SAT
            l = mid + 1
        else: # bound <= mid, UNSAT
            r = mid
    return l


opt = GetOpt()
exact, approx, lib, errorType = opt.exact, opt.approx, opt.lib, opt.errorType
assert os.path.exists(exact)
assert os.path.exists(approx)
assert os.path.exists(lib)
# refBound = GetRefBound(approx)
refBound = 1
print('reference bound =', refBound)
assert(IsMaxErrorNoMoreThan(exact, approx, lib, errorType, refBound))
print(f'Max error <= {refBound}: True')
realBound = GetRealBound(exact, approx, lib, refBound, errorType)
print('real bound =', realBound)
