#!/bin/bash


#SBATCH --job-name=merge_sort_benchmark
#SBATCH --output=benchmark_results.txt
#SBATCH --nodes=1
#SBATCH --tasks-per-node=1
#SBATCH --time=00:10:00
#SBATCH --partition=Centaurus

if [ ! -f Makefile ]; then
    echo "Error: Makefile not found!" >&2
    exit 1
fi

module load gcc
make

echo "" > benchmark_results.txt

for size in 10 100 1000 10000 100000 1000000 10000000 100000000; do
    ## runtime=$(./merge_sort $size | awk '{print $3}')
    ## echo "$size $runtime" >> benchmark_results.txt
    ./merge_sort $size >> benchmark_results.txt
done

