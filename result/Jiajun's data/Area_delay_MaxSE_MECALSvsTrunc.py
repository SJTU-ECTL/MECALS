#!/usr/bin/env python
# -*- coding: utf-8 -*-


import csv
import numpy as np
from matplotlib import pyplot as plt
from matplotlib import ticker
from textwrap import fill


def drawSubPlot(_id, csvName, showLegend, showYLabel, errorTicks, truncBits, cktName):
    # general parameters
    tickSize = 13
    labelSize = 14.5
    markerSize = 6
    mk = ['o', '^', 'o', '^']
    clr = ['b', 'b', 'r', 'r']
    lines = ['-', ':', '-', ':']

    # parse csv and plot
    ax = plt.subplot(2, 2, _id)
    ax.grid(linewidth=0.6, linestyle='-')
    axtwin = ax.twiny()
    csvFile = open(csvName, 'r')
    reader = csv.reader(csvFile)
    cnt = 0
    cntMk = 0
    terms = ['Area', 'Delay', 'Area', 'Delay']
    for item in reader:
        circuit = item[0]
        if circuit == '' or circuit == 'circuit':
            continue
        erBound = float(item[4])
        oriArea = float(item[1])
        appArea = float(item[2])
        print(circuit, erBound, appArea, oriArea, appArea / oriArea)
        cnt += 1
        if cnt == 1:
            error, ar = [], []
        error.append(erBound)
        ar.append((1 - (appArea / oriArea)) * 100)
        if cnt == 5:
            if cntMk == 0 or cntMk == 1:
                ax.plot(error, ar, label=terms[cntMk], marker=mk[cntMk], color=clr[cntMk], linestyle=lines[cntMk], fillstyle='full', markersize=markerSize)
            else:
                axtwin.plot(error, ar, label=terms[cntMk], marker=mk[cntMk], color=clr[cntMk], linestyle=lines[cntMk], fillstyle='full', markersize=markerSize)
            cnt = 0
            cntMk += 1

    # adjust figure
    ax.set_xlabel(f'({chr(97+_id-1)}) {cktName}: normalized MaxSE', fontsize=labelSize)
    if showYLabel:
        ax.set_ylabel('Saving ratio (%)', fontsize=labelSize)
    ax.set_xscale('log')
    ax.tick_params(axis='x', labelsize=tickSize)
    ax.tick_params(axis='y', labelsize=tickSize)
    ax.set_xticks(error, errorTicks)
    if showLegend:
        ax.legend(fontsize=tickSize, bbox_to_anchor=(1.5, -0.1), loc='lower right', title='Truncation', title_fontsize=tickSize)
    axtwin.set_xscale('log')
    axtwin.set_xlabel('#truncation bits', fontsize=labelSize)
    axtwin.tick_params(axis='x', labelsize=tickSize)
    axtwin.set_xticks(error, truncBits)
    if showLegend:
        axtwin.legend(fontsize=tickSize, bbox_to_anchor=(1.5, 0.6), loc='lower right', title='MECALS', title_fontsize=tickSize)


plt.rcParams['figure.figsize'] = (8.0, 5.0)
drawSubPlot(1, './mecals_trunc_add8u.csv', False, True, ('4e-6', '3e-5', '2e-4', '9e-4', '4e-3'), (1, 2, 3, 4, 5), 'add8')
drawSubPlot(2, './mecals_trunc_rca32.csv', True, False, ('3e-18', '8e-16', '2e-13', '6e-11', '1e-8'), (4, 8, 12, 16, 20), 'add32')
drawSubPlot(3, './mecals_trunc_am8.csv', False, True, ('2e-9', '5e-8', '9e-7', '2e-5', '2e-4'), (2, 4, 6, 8, 10), 'mult8')
drawSubPlot(4, './mecals_trunc_mult16.csv', False, False, ('1e-17', '4e-15', '9e-13', '2e-10', '6e-8'), (4, 8, 12, 16, 20), 'mult16')
plt.tight_layout()
# plt.show()
plt.savefig('MECALS_trunc_MaxSE.eps', dpi=1000, bbox_inches='tight')
