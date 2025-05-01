To compile and run:

1. Run make first to compile - run make clean first if necessary

2. Change any inputs in Makefile before running (it is where you set thread count for omp)

3. make chosen mode to run simulation

4. python3 plot.py output.tsv output.pdf to plot data

5. Can view timing.log for data collection

BENCHMARK TIMES:
SOLAR AT dt = 200 and 5000000 steps with 4 threads: Simulation time: 32.7608 seconds, with 8 threads: 43.0366 seconds -- This is weird?
100 PARTICLES AT dt = 1 and 10000 steps with 4 threads: Simulation time: 0.417912 seconds, with 8 threads: 0.333876 seconds
1000 PARTICLES AT dt =1 and 10000 steps with 4 threads: Simulation time: 24.0388 seconds, with 8 threads: 11.0282 seconds

Analysis Comparison:
SOLAR for some reason took longer?
Parallel performed better for both of the other cases, by .1 to .2 for 100 particles, and over half for 1000 particles. So it seems like the larger
the computation the more performance benefit you see. 
