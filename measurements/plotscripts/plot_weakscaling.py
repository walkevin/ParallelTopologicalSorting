import numpy as np
import matplotlib.pyplot as plt
import glob
import re
import sqlite3
import helper
import addline



def plotWeakScaling(allsize=1000000,allgraphtype='SOFTWARE',alladditionalwhere=' AND total_time>0 ',suffix='',basesize=100000, title=''):
	fig = plt.figure()
	ax = fig.add_subplot(111)
	#addline.addWeakScaling(axis=ax, algorithm='dynamic_nobarrier', optimistic='1', size=basesize, graphtype=allgraphtype, hostnamelike='e%',colorindex=0,linelabel='DynNoBarrier')
	addline.addWeakScaling(axis=ax, algorithm='bitset', optimistic='1', size=basesize, graphtype=allgraphtype, hostnamelike='e%',colorindex=0,linelabel='Node-Lookup')
	addline.addWeakScaling(axis=ax, algorithm='worksteal', optimistic='1', size=basesize, graphtype=allgraphtype, hostnamelike='e%',colorindex=1,linelabel='Worksteal')
	addline.addWeakScaling(axis=ax, algorithm='locallist', optimistic='1', size=basesize, graphtype=allgraphtype, hostnamelike='e%',colorindex=2,linelabel='Scatter-Gather')
	ax.plot([1,24],[1,1],'r--')
	ax.legend(loc='upper right')
	ax.minorticks_on()
	
	filename = helper.plotdir + 'weakscaling_gt' + allgraphtype + '_n' + str(allsize)

	plt.title('Speedup vs. Number of Threads',fontsize=helper.fontsize_label)
	if(title!=''):
		plt.suptitle(title,fontsize=helper.fontsize_title)
	elif(suffix==''):
		plt.suptitle('Weak Scaling for ' + allgraphtype + ' Graph (' + str(allsize) + 'nodes)',fontsize=helper.fontsize_title)
	else:
		plt.suptitle('Weak Scaling for ' + allgraphtype + ' Graph (' + str(allsize) + 'nodes, ' + suffix + ')',fontsize=helper.fontsize_title)
	
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
plotWeakScaling(allsize=1000000,allgraphtype='SOFTWARE',title='Weak scaling for Software graph (basesize 100k nodes)') # software graph
plotWeakScaling(allsize=1000000,allgraphtype='RANDOMLIN8',suffix='deg8',title='Weak scaling for Random graph (basesize 100k nodes, degree 8)') # degree 8
plotWeakScaling(allsize=1000000,allgraphtype='RANDOMLIN16',suffix='deg16',title='Weak scaling for Random graph (basesize 100k nodes, degree 16)') # degree 16
plotWeakScaling(allsize=1000000,allgraphtype='RANDOMLIN32',suffix='deg32',title='Weak scaling for Random graph (basesize 100k nodes, degree 32)') # degree 32
# degree 64 has incomplete data
