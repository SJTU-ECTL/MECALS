# -*- coding: utf-8 -*-


import os
import subprocess
from optparse import OptionParser


def GetOpt():
    parser = OptionParser()
    parser.add_option('-e', '--exact',
                      action='store',
                      dest='exact',
                      default='./exact.blif',
                      help='path to exact circuit')
    parser.add_option('-l', '--lib',
                      action='store',
                      dest='lib',
                      default='./nangate.lib',
                      help='path to standard cell library')
    parser.add_option('-t', '--trunc',
                      action='store',
                      dest='trunc',
                      default='1',
                      help='number of truncation bits')
    options, _ = parser.parse_args()
    return options


def RunAbc(comm, logFileName=""):
    if logFileName != "":
        subprocess.run(f'abc -c \"{comm}\" >> {logFileName}', shell=True)
    else:
        subprocess.run(f'abc -c \"{comm}\"', shell=True)


opt = GetOpt()
exact, lib, trunc = opt.exact, opt.lib, int(opt.trunc)
assert os.path.exists(exact)
assert os.path.exists(lib)
comm = f'r {exact}; r {lib}; '
for i in range(trunc):
    comm += f'zeropo -N {i}; '
comm += 'resyn2; dch; amap; stime;'
RunAbc(comm)