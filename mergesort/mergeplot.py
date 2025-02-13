import matplotlib.pyplot as plt
import numpy as np

# Load results from file
data = np.loadtxt('benchmark_results.txt')
sizes, times = data[:, 0], data[:, 1]

# Plot the results
plt.figure(figsize=(10, 5))
plt.loglog(sizes, times, marker='o', linestyle='-', label="Merge Sort Performance")
plt.xlabel('Array Size')
plt.ylabel('Time (seconds)')
plt.title('Merge Sort Benchmark on Centaurus')
plt.grid(True)

# Save the plot
plt.savefig('benchmark_plot.png')
plt.show()
