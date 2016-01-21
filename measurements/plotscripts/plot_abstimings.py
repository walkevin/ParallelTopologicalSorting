import numpy as np
import matplotlib.pyplot as plt
import glob
import re
import helper
import addline

def plotAbsTiming(allsize=1000000,allgraphtype='SOFTWARE',alladditionalwhere=' AND total_time>0 ',suffix='', title=''):
	fig = plt.figure()
	ax = fig.add_subplot(111)
	addline.addAbsTiming(axis=ax, algorithm='bitset_global', optimistic='0', size=allsize, graphtype=allgraphtype, hostnamelike='e%',colorindex=0,linelabel='Single insertion, Lock')
	addline.addAbsTiming(axis=ax, algorithm='bitset', optimistic='0', size=allsize, graphtype=allgraphtype, hostnamelike='e%',colorindex=1,linelabel='Batch insertion, Lock')
	addline.addAbsTiming(axis=ax, algorithm='bitset_global', optimistic='1', size=allsize, graphtype=allgraphtype, hostnamelike='e%',colorindex=2,linelabel='Single insertion, Atomic')
	addline.addAbsTiming(axis=ax, algorithm='bitset', optimistic='1', size=allsize, graphtype=allgraphtype, hostnamelike='e%',colorindex=3,linelabel='Batch insertion, Atomic')
	ax.legend(loc='upper left')
	ax.minorticks_on()

	filename = helper.plotdir + 'abstiming_gt' + allgraphtype + '_n' + str(allsize)
	
	plt.title('Time [sec] vs. Number of threads',fontsize=helper.fontsize_label)

	if(title!=''):
		plt.suptitle(title,fontsize=helper.fontsize_title)
	elif(suffix==''):
		plt.suptitle('Absolute Timing for ' + allgraphtype + ' Graph (' + str(allsize) + 'nodes)',fontsize=helper.fontsize_title)
	else:
		plt.suptitle('Absolute Timing for ' + allgraphtype + ' Graph (' + str(allsize) + 'nodes, ' + suffix + ')',fontsize=helper.fontsize_title)
	if(suffix!=''):
		filename = filename + '_' + suffix

	filename = filename + '.pdf'
	
	plt.savefig(filename, format='pdf',bbox_inches='tight',dpi=1000)
	print "File written to:\t", filename
	if(helper.show):
		plt.show()


############################################################
# Call Plotting functions
############################################################
plotAbsTiming(allsize=1000000,allgraphtype='RANDOMLIN',alladditionalwhere=' AND graph_num_edges=29999064',suffix='deg30', title='Absolute Timing for different optimizations')
