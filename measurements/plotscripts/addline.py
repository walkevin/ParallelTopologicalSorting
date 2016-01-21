import numpy as np
import matplotlib.pyplot as plt
import glob
import re
import helper


plt.style.use('ggplot')

###############################################################################
# Absolute Timing
###############################################################################
def addAbsTiming(axis, algorithm, optimistic, size, graphtype='SOFTWARE', hostnamelike='e%',colorindex=0,markertype='D-',linelabel='nolabel', additionalwhere=''):

	fixedwhere = "enable_analysis=0 AND debug=0 AND verbose=0 AND processors>=number_of_threads AND algorithm='{0}' AND optimistic={1} AND graph_type='{2}' AND hostname LIKE '{3}' AND graph_num_nodes={4} {5}".format(algorithm,optimistic,graphtype,hostnamelike,size,additionalwhere)

	numthreads = helper.getData('number_of_threads', fixedwhere + ' GROUP BY number_of_threads')

	avgtimings = []

	if (np.size(numthreads)==0):
		return

	for nt in numthreads.flat:
		# print "NUMTHREADS = ", nt
		where = fixedwhere + ' AND number_of_threads={0}'.format(nt)
		timings = helper.getData('total_time',where)
		
		# Compute mean and stddev of first timing
		if(len(avgtimings)==0):
			mean, q25, q75 = helper.median_and_quantiles(data=timings)
			print algorithm, " :   ", mean, " , [", q25 , ",", q75, "]"
	
		violin_parts = axis.violinplot(timings,[nt],widths=0.8)

		for pc in violin_parts['bodies']:
			pc.set_color(helper.getFGcolor(colorindex))

		avgtimings.append(np.mean(timings))

	axis.plot(numthreads,avgtimings[0]/numthreads,'--',color=helper.getBGcolor(colorindex)) # ideal scaling
	axis.plot(numthreads,avgtimings,markertype,markersize=6,linewidth=1,color=helper.getFGcolor(colorindex),label=linelabel) # connecting dots




###############################################################################
# Strong Scaling
###############################################################################
def addStrongScaling(axis, algorithm, optimistic, size, graphtype='SOFTWARE', hostnamelike='e%',colorindex=0,markertype='D-',linelabel='nolabel',additionalwhere=''):

	fixedwhere = "enable_analysis=0 AND debug=0 AND verbose=0 AND processors>=number_of_threads AND algorithm='{0}' AND optimistic={1} AND graph_type='{2}' AND hostname LIKE '{3}' AND graph_num_nodes={4} {5}".format(algorithm,optimistic,graphtype,hostnamelike,size,additionalwhere)

	numthreads = helper.getData('number_of_threads', fixedwhere + ' GROUP BY number_of_threads')

	avgtimings = []

	if (np.size(numthreads)==0):
		return

	for nt in numthreads.flat:
		# print "NUMTHREADS = ", nt
		where = fixedwhere + ' AND number_of_threads={0}'.format(nt)
		timings = helper.getData('total_time',where)

		# Compute mean and stddev of first timing
		if(len(avgtimings)==0):
			mean, q25, q75 = helper.median_and_quantiles(data=timings)
			print algorithm, " :   ", mean, " , [", q25 , ",", q75, "]"

		avgtimings.append(np.mean(timings))
		speedups = avgtimings[0]/timings

		violin_parts = axis.violinplot(speedups,[nt],widths=0.8)

		for pc in violin_parts['bodies']:
			pc.set_color(helper.getFGcolor(colorindex))

	speedup = avgtimings[0]/avgtimings

	axis.plot(numthreads,speedup,markertype,markersize=6,linewidth=1,color=helper.getFGcolor(colorindex),label=linelabel) # connecting dots




def addWeakScaling(axis, algorithm, optimistic, size, graphtype='SOFTWARE', hostnamelike='e%',colorindex=0,markertype='D-',linelabel='nolabel'):

	fixedwhere = "enable_analysis=0 AND debug=0 AND verbose=0 AND processors>=number_of_threads AND algorithm='{0}' AND optimistic={1} AND graph_type='{2}' AND hostname LIKE '{3}' AND graph_num_nodes={4}*number_of_threads".format(algorithm,optimistic,graphtype,hostnamelike,size)

	numthreads = helper.getData('number_of_threads', fixedwhere + ' GROUP BY number_of_threads')

	# print(numthreads)

	avgtimings = []

	if (np.size(numthreads)==0):
		return

	for nt in numthreads.flat:
		# print "NUMTHREADS = ", nt
		where = fixedwhere + ' AND number_of_threads={0}'.format(nt)
		timings = helper.getData('total_time',where)

		# Compute mean and stddev of first timing
		if(len(avgtimings)==0):
			mean, q25, q75 = helper.median_and_quantiles(data=timings)
			print algorithm, " :   ", mean, " , [", q25 , ",", q75, "]"

		avgtimings.append(np.mean(timings))
		speedups = avgtimings[0]/timings

		violin_parts = axis.violinplot(speedups,[nt],widths=0.8)

		for pc in violin_parts['bodies']:
			pc.set_color(helper.getFGcolor(colorindex))

	speedup = avgtimings[0]/avgtimings

	axis.plot(numthreads,speedup,markertype,markersize=6,linewidth=1,color=helper.getFGcolor(colorindex),label=linelabel) # connecting dots
