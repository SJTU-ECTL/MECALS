# -*- coding: utf-8 -*-

import pandas as pd
import seaborn as sns
import matplotlib.pyplot as plt


# pd.options.display.notebook_repr_html=False  # 表格显示
# plt.rcParams['figure.dpi'] = 75  # 图形分辨率
# sns.set_theme(style='darkgrid')  # 图形主题
sns.set(font_scale=0.6)
plt.figure(figsize=(3, 3))
sns.set_style("whitegrid", {"axes.facecolor": "#ffffff"})
data = pd.read_csv('result/accuracy-efficiency/tradeoff.csv')
print(data)

ax = sns.lineplot(data=data, x='Percentage of nodes using exact PBDs $P$', y='Detection ratio', hue='circuit', style='circuit', markers=True)
# ax.set(xlabel='Percentage of nodes using exact PBDs $P', fontsize=ftSize)
# # plt.xlabel('Percentage of nodes using exact PBDs $P$', fontsize=ftSize)
# ax.set(ylabel='Detection ratio', fontsize=ftSize)
# # plt.ylabel('Detection ratio', fontsize=ftSize)
# ax.set_xticks(fontsize=ftSize)
# # plt.xticks(fontsize=ftSize)
# ax.set_yticks(fontsize=ftSize)
# # plt.yticks(fontsize=ftSize)
# ax.legend(labels=['add8u-detect', 'absdiff-detect', 'mac-detect'])
# plt.legend(prop=ftSize)

ax2 = plt.twinx()
ax2 = sns.lineplot(data=data, x='Percentage of nodes using exact PBDs $P$', y='Runtime ratio', hue='circuit', style='circuit', markers=True, ax=ax2)
# ax2.legend(labels=['add8u-runtime', 'absdiff-runtime', 'mac-runtime'])

# plt.show()
plt.savefig('./tradeoff.eps', dpi=1000, format='eps', bbox_inches='tight')
