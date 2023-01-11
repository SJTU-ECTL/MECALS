import os
import time
import psutil
import re


def exec(comm):
    result = os.popen(comm).read()
    return result


def GetInfo(_str, key):
    rgx_tmpl = r"{key}\s*=\s*(?P<gVal>\d+(?:\.\d+)?)"
    rgx = re.compile(rgx_tmpl.format(key=key))
    match = re.search(rgx, _str)
    if match:
        ret = float(match.group("gVal"))
    else:
        log.error("Error: no match for {} in {}".format(key, file_stats_orig))
    return ret


def GetP(_str):
    pos0 = _str.find('/P')
    pos1 = _str.find('/log.txt')
    assert(pos0 != -1 and pos1 != -1)
    return int(_str[pos0 + 2: pos1])


# cktName = 'add8u_0FP'
# cktName = 'absdiff'
cktName = 'mac'
folder = f'./result/{cktName}'

targFiles = []
for root, dirs, files in os.walk(folder):
    for file in files:
        if file == 'log.txt':
            logName = os.path.join(root, file)
            targFiles.append(logName)
for targFile in sorted(targFiles):
    # print(targFile)
    P = GetP(targFile)
    # print(P)
    with open(targFile) as f:
        lines = f.readlines();
    satLacs, pbdLacs = [], []
    for line in lines:
        if line.find('#valid LACs detected by SAT') != -1:
            satLacs.append(GetInfo(line, 'by SAT'))
        elif line.find('#valid LACs detected by PBD') != -1:
            pbdLacs.append(GetInfo(line, 'by PBD'))
        elif line.find('time with SAT') != -1:
            satTime = GetInfo(line, 'time with SAT')
        elif line.find('time with PBD') != -1:
            pbdTime = GetInfo(line, 'time with PBD')
    comm = f'cat {targFile} | grep -e \"valid\" -e \"time with\"'
    ret = exec(comm)
    # print(ret)
    assert len(satLacs) == 3 and len(pbdLacs)
    # print(f'{P}\t{pbdLacs[0]}\t{pbdLacs[1]}\t{pbdLacs[2]}\t{pbdTime}\t{satLacs[0]}\t{satLacs[1]}\t{satLacs[2]}\t{satTime}')
    print(f'{P}\t{pbdLacs[0]}\t{satLacs[0]}\t{pbdLacs[0] / satLacs[0]}\t{pbdLacs[1]}\t{satLacs[1]}\t{pbdLacs[1] / satLacs[1]}\t{pbdLacs[2]}\t{satLacs[2]}\t{pbdLacs[2] / satLacs[2]}\t\t{pbdTime}\t{satTime}\t{pbdTime/satTime}')
