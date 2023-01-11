import os
import time
import psutil
import re


def exec(comm):
    result = os.popen(comm).read()
    return result


def GetWCEBound(_str):
    pos0 = _str.find('wce')
    if pos0 == -1:
        return -1
    pos1 = _str.find('/final.blif')
    assert(pos0 != -1 and pos1 != -1)
    wce = int(_str[pos0 + 3: pos1])
    return wce


def GetMHDBound(_str):
    pos0 = _str.find('mhd')
    if pos0 == -1:
        return -1
    pos1 = _str.find('/final.blif')
    assert(pos0 != -1 and pos1 != -1)
    mhd = int(_str[pos0 + 3: pos1])
    return mhd


def GetMaxSEBound(_str):
    pos0 = _str.find('maxse')
    if pos0 == -1:
        return -1
    pos1 = _str.find('/final.blif')
    assert(pos0 != -1 and pos1 != -1)
    mhd = int(_str[pos0 + 5: pos1])
    return mhd


def GetInfo(_str, key):
    rgx_tmpl = r"{key}\s*=\s*(?P<gVal>\d+(?:\.\d+)?)"
    rgx = re.compile(rgx_tmpl.format(key=key))
    match = re.search(rgx, _str)
    if match:
        ret = float(match.group("gVal"))
    else:
        assert 0
    return ret


# cktName = 'rca32'
# cktName = 'absdiff'
# cktName = 'add8u_0FP'
# cktName = 'am8'
# cktName = 'binsqrd'
# cktName = 'mac'
# cktName = 'buttfly'
cktName = 'mult16'
# cktName = 'add128'
# cktName = 'mult32'
folder = f'./result/{cktName}'

targFiles = []
for root, dirs, files in os.walk(folder):
    for file in files:
        if file == 'final.blif':
            blifName = os.path.join(root, file)
            targFiles.append(blifName)

for targFile in sorted(targFiles):
    wceBound = GetWCEBound(targFile)
    # print(targFile, wceBound)
    if wceBound == -1:
        continue
    blifName = targFile.replace('final.blif', f'{cktName}_wceBound{wceBound}.blif')
    comm = f'abc -c \"source input/abc.rc; r {targFile}; r input/standard-cell/nangate_45nm_typ.lib; st; dch; amap; stime; w {blifName}\" | grep -e \"Area\"'
    ret = exec(comm)
    area = GetInfo(ret, 'Area')
    delay = GetInfo(ret, 'Delay')
    logFile = targFile.replace('final.blif', 'log.txt')
    comm = f'cat {logFile} | tail -2'
    ret = exec(comm)
    _time = GetInfo(ret, 'runtime')
    print(f'{wceBound}\t{area}\t{delay}\t{_time}')


# for targFile in sorted(targFiles):
#     mhdBound = GetMHDBound(targFile)
#     if mhdBound == -1:
#         continue
#     blifName = targFile.replace('final.blif', f'{cktName}_mhdBound{mhdBound}.blif')
#     blifName1 = f'./mhd-output/{cktName}_mhdBound{mhdBound}.blif'
#     comm = f'abc -c \"source input/abc.rc; r {targFile}; r input/standard-cell/nangate_45nm_typ.lib; st; dch; amap; stime; w {blifName}; w {blifName1}\" | grep -e \"Area\"'
#     ret = exec(comm)
#     area = GetInfo(ret, 'Area')
#     delay = GetInfo(ret, 'Delay')
#     logFile = targFile.replace('final.blif', 'log.txt')
#     comm = f'cat {logFile} | tail -2'
#     ret = exec(comm)
#     _time = GetInfo(ret, 'runtime')
#     print(f'{mhdBound}\t{area}\t{delay}\t{_time}')


# for targFile in sorted(targFiles):
#     maxseBound = GetMaxSEBound(targFile)
#     if maxseBound == -1:
#         continue
#     blifName = targFile.replace('final.blif', f'{cktName}_maxseBound{maxseBound}.blif')
#     blifName1 = f'./output/{cktName}_maxseBound{maxseBound}.blif'
#     comm = f'abc -c \"source input/abc.rc; r {targFile}; r input/standard-cell/nangate_45nm_typ.lib; st; dch; amap; stime; w {blifName}; w {blifName1}\" | grep -e \"Area\"'
#     ret = exec(comm)
#     area = GetInfo(ret, 'Area')
#     delay = GetInfo(ret, 'Delay')
#     logFile = targFile.replace('final.blif', 'log.txt')
#     comm = f'cat {logFile} | tail -2'
#     ret = exec(comm)
#     _time = GetInfo(ret, 'runtime')
#     print(f'{maxseBound}\t{area}\t{delay}\t{_time}')
