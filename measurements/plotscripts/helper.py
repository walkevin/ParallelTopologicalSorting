############### Color#############################################################
import numpy as np
import scipy as sp
import scipy.stats
import sqlite3

myFGcolors = [	(0,0,200), (0,200,0), (200,0,0), \
				(200,0,200), (0,200,200), (200,200,0), \
				(100,100,200), (100,200,100), (200,100,100), \
				(200,200,100), (100,200,200), (200,100,200), \
				(50,50,50), (100,100,100), (200,200,200) ]
myFGcolors = list([tuple([x/255. for x in tup]) for tup in myFGcolors])
myBGcolors = list([tuple([np.min([x+0.4,1.0]) for x in tup]) for tup in myFGcolors])
 
def getFGcolor(i):
	return (myFGcolors[i%len(myFGcolors)])

def getBGcolor(i):
	return (myBGcolors[i%len(myBGcolors)])



fontsize_title=14
fontsize_label=12


def median_and_quantiles(data):
	a = np.array(data)
	median = np.median(a)
	q25 = np.percentile(a,25)
	q75 = np.percentile(a,75)
	return median, q25, q75

plotdir = "plots/";
show = False


def getData(field, wherestring):

	with sqlite3.connect('measurements.db') as db:
		query = db.cursor()
		querystring = "SELECT {0} FROM measurements WHERE {1}".format(field,wherestring)
		query.execute(querystring)
		data = query.fetchall() 
		
	return np.array(data)





##################################################################################
