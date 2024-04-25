import matplotlib.pyplot as plt 
import numpy as np
filenames = ['FirstFit.data','MetaOffset.data','GreedyUniform.data','CompactPairs.data','CompactFit.data']
labels = ['FirstFit','MetaOffset','GreedyUniform','CompactPairs','CompactFit']
fig, ax = plt.subplots()
for filename, label in zip(filenames, labels):
	data = np.loadtxt(filename, usecols=(0, 1), unpack=True)
	x, y = data[0], data[1]
	ax.plot(x, y, label=label)
ax.set_xlabel("Load")
ax.set_ylabel("Success rate (%)")
ax.legend(loc="lower left")
plt.savefig('result.pdf', format='pdf')
