#!/usr/bin/env python
# -*- coding: utf-8 -*-


import csv
from matplotlib import pyplot as plt


# plt.rcParams['figure.figsize'] = (7.0, 3.0)
# csvFile = open('result/maxLevel-1/ER/table/ERFilter.csv', 'r')
# reader = csv.reader(csvFile)
# cnt = 0
# mk = ['o', 'x', '+', '*', 's', 'd', 'v', '^', '<', '>', 'h', 'p']
# cntMk = 0
# for item in reader:
#     circuit = item[0]
#     if circuit == '' or circuit == 'circuit':
#         continue
#     erBound = float(item[3])
#     oriArea = float(item[1])
#     appArea = float(item[4])
#     print(circuit, erBound, appArea / oriArea)
#     cnt += 1
#     if cnt == 1:
#         error, ar = [], []
#     error.append(erBound * 100)
#     ar.append(appArea / oriArea)
#     if cnt == 7:
#         # print(error, ar)
#         plt.plot(error, ar, label=circuit, marker=mk[cntMk], fillstyle='none', markersize=5)
#         cnt = 0
#         cntMk += 1
# ftSize = 13
# plt.xlabel('Error rate (%)', fontsize=ftSize)
# plt.ylabel('Area ratio', fontsize=ftSize)
# plt.xticks(fontsize=ftSize)
# plt.yticks(fontsize=ftSize)
# plt.legend(loc='upper right', bbox_to_anchor=(1.20, 1.10), fontsize=10)
# plt.tight_layout()
# # plt.show()
# plt.savefig('ErrorRateVSArea.eps', dpi=1000, bbox_inches='tight')


# plt.rcParams['figure.figsize'] = (7.0, 3.0)
# # csvFile = open('/Users/user/Desktop/2022SU/ECE4530/MP1/mecals/mecals_delay_mse.csv', 'r')
# csvFile = open('/Users/user/Desktop/2022SU/ECE4530/MP1/mecals/mecals_area_mse.csv', 'r')
# reader = csv.reader(csvFile)
# cnt = 0
# mk = ['o', 'x', '+', '*', 's', 'd', 'v', '^', '<', '>', 'h', 'p']
# cntMk = 0
# for item in reader:
#     circuit = item[0]
#     if circuit == '' or circuit == 'circuit':
#         continue
#     erBound = float(item[3])
#     oriArea = float(item[1])
#     appArea = float(item[2])
#     print(circuit, erBound, appArea, oriArea, appArea / oriArea)
#     cnt += 1
#     if cnt == 1:
#         error, ar = [], []
#     error.append(erBound * 100)
#     ar.append(appArea / oriArea)
#     if cnt == 5:
#         plt.plot(error, ar, label=circuit, marker=mk[cntMk], fillstyle='none', markersize=5)
#         cnt = 0
#         cntMk += 1
# ftSize = 13
# plt.xlabel('Normalized MaxSE (%)', fontsize=ftSize)
# # plt.ylabel('Delay ratio', fontsize=ftSize)
# plt.ylabel('Area ratio', fontsize=ftSize)
# plt.xticks(fontsize=ftSize)
# plt.yticks(fontsize=ftSize)
# plt.legend(loc='upper right', bbox_to_anchor=(1.25, 1), fontsize=9)
# plt.tight_layout()
# # plt.show()
# # plt.savefig('MSEVSDelay.eps', dpi=1000, bbox_inches='tight')
# plt.savefig('MSEVSArea.eps', dpi=1000, bbox_inches='tight')

plt.rcParams['figure.figsize'] = (8, 3.0)

ax1 = plt.subplot(121)
ax1.grid(linewidth=0.8, linestyle='dotted')

csvFile = open('./mecals_area_mse.csv', 'r')
reader = csv.reader(csvFile)
cnt = 0
mk = ['o', 'x', '+', '*', 's', 'd', 'v', '^', '<', '>', 'h', 'p']
cntMk = 0

print('area saving table')
for item in reader:
    circuit = item[0]
    if circuit == '' or circuit == 'circuit':
        continue
    erBound = float(item[3])
    oriArea = float(item[1])
    appArea = float(item[2])
    print(circuit, erBound, appArea, oriArea, 1 - appArea / oriArea)
    cnt += 1
    if cnt == 1:
        error, ar = [], []
    error.append(erBound * 100)
    ar.append((1.0 - appArea / oriArea) * 100)
    if cnt == 5:
        plt.plot(error, ar, label=circuit, marker=mk[cntMk], fillstyle='none', markersize=5)
        cnt = 0
        cntMk += 1
tickSize = 13
leg_size = 15
plt.xlabel('Normalized MaxSE bound (%)', fontsize=leg_size)
plt.ylabel('Area saving ratio (%)', fontsize=leg_size)
plt.ylim([0, 100])
plt.xticks(fontsize=tickSize)
plt.yticks(fontsize=tickSize)
plt.tight_layout()


print('delay saving table')
ax2 = plt.subplot(122)
ax2.grid(linewidth=0.8, linestyle='dotted')
csvFile = open('./mecals_delay_mse.csv', 'r')
reader = csv.reader(csvFile)
cnt = 0
mk = ['o', 'x', '+', '*', 's', 'd', 'v', '^', '<', '>', 'h', 'p']
cntMk = 0

for item in reader:
    circuit = item[0]
    if circuit == '' or circuit == 'circuit':
        continue
    erBound = float(item[3])
    oriDelay = float(item[1])
    appDelay = float(item[2])
    print(circuit, erBound, appDelay, oriDelay, 1 - appDelay / oriDelay)
    cnt += 1
    if cnt == 1:
        error, ar = [], []
    error.append(erBound * 100)
    ar.append((1.0 - appDelay / oriDelay) * 100)
    if cnt == 5:
        plt.plot(error, ar, label=circuit, marker=mk[cntMk], fillstyle='none', markersize=5)
        cnt = 0
        cntMk += 1

plt.xlabel('Normalized MaxSE bound (%)', fontsize=leg_size)
plt.ylabel('Delay saving ratio (%)', fontsize=leg_size)
plt.ylim([0, 100])
plt.xticks(fontsize=tickSize)
plt.yticks(fontsize=tickSize)

plt.tight_layout()
plt.legend(loc='upper right', bbox_to_anchor=(1.4, 1.0), fontsize=tickSize)
# plt.show()
plt.savefig('MSEVSAreaDelay.eps', dpi=1000, bbox_inches='tight')
