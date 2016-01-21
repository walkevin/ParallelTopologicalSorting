import matplotlib.pyplot as plt
import matplotlib.patches as patches
import numpy as np

import helper

ncolors = np.shape(helper.myFGcolors)[0]
s = 0.8

print "Number of Colors defined: ", ncolors

fig = plt.figure()
ax = fig.add_subplot(111,aspect=1)
ax.set_xlim([0,0.1+ncolors*(s+0.1)])
ax.set_ylim([0,2*s+0.3])

ax.get_xaxis().set_visible(False)
ax.get_yaxis().set_visible(False)

for i in range(0,ncolors):

# Foreground Colors
	ax.add_patch(
			patches.Rectangle(
				(0.1+i*(0.1+s),0.1),	# (x,y)
				s,			# width
				s,			# height
				facecolor=helper.getFGcolor(i)
				)
			)

# Background Colors
	ax.add_patch(
			patches.Rectangle(
				(0.1+i*(0.1+s),s+0.2),	# (x,y)
				s,			# width
				s,			# height
				facecolor=helper.getBGcolor(i)
				)
			)


plt.show()
