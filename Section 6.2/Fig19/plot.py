import matplotlib.pyplot as plt 
import numpy as np
filenames = ['Greedy Uniform.data','FirstFit.data','Greedy Potential.data','Swap and Move.data']
labels = ['Greedy Uniform','FirstFit','Greedy Potential','Swap and Move']
fig, ax = plt.subplots()
for filename, label in zip(filenames, labels):
	data = np.loadtxt(filename, usecols=(0, 1), unpack=True)
	x, y = data[0], data[1]
	ax.plot(x, y, label=label)
ax.set_xlabel("Load")
ax.set_ylabel("Success rate (%)")
ax.legend(loc="lower left")
plt.xlim(0.5, 1)
plt.savefig('result.pdf', format='pdf')
