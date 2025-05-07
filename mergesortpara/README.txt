How to run and compile:

1. Run make to compile - or make clean if needed

2. Run ./merge_sort 10000 or any array size that then gets randomly populated

Output:
Format: Mode ArrSize TimeElapsed

Sequential 5000 0.000514808
Parallel   5000 0.000508102
Speedup: 1.0132x

Sequential 10000 0.00105677
Parallel   10000 0.00104492
Speedup: 1.01133x

After threshold of 10000:

Sequential 50000 0.00573908
Parallel   50000 0.00305793
Speedup: 1.87679x

Sequential 100000 0.0118652
Parallel   100000 0.00465319
Speedup: 2.54991x

Sequential 150000 0.0180946
Parallel   150000 0.004917
Speedup: 3.68001x


