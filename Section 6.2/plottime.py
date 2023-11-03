import matplotlib.pyplot as plt

# Initialize lists to store data
x = []
y1 = []  # First Fit
y2 = []  # Greedy Potential
y3 = []  # Greedy Uniform
y4 = []  # Swap and Move

# Read data from the 'time.plot' file
with open('time.data', 'r') as file:
    for line in file:
        parts = line.split()
        x.append(int(parts[0]))
        y1.append(float(parts[1]))
        y2.append(float(parts[2]))
        y3.append(float(parts[3]))
        y4.append(float(parts[4]))

# Create a figure and axis
fig, ax = plt.subplots()

# Plot the data
ax.plot(x, y1, label="First Fit", marker='o')
ax.plot(x, y2, label="Greedy Potential", marker='o')
ax.plot(x, y3, label="Greedy Uniform", marker='o')
ax.plot(x, y4, label="Swap and Move", marker='o')

# Customize the plot
ax.set_yscale('log')
ax.set_xlabel("Number of messages")
ax.set_ylabel("Average execution time (ms)")
ax.legend(loc="lower right")

# Save the plot as a PDF
plt.savefig('log.pdf', format='pdf')

# Display the plot (optional)
plt.show()
