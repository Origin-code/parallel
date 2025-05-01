CXX = g++
CXXFLAGS = -O3 -fopenmp
THREADS ?= 8

nbody: nbody.cpp
	$(CXX) $(CXXFLAGS) nbody.cpp -o nbody

solar.out: nbody
	date
	OMP_NUM_THREADS=$(THREADS) ./nbody planet 200 5000000 10000 > solar.out
	date

solar.pdf: solar.out
	python3 plot.py solar.out solar.pdf 1000

random.out: nbody
	date
	OMP_NUM_THREADS=$(THREADS) ./nbody 1000 1 10000 100 > random.out
	date

small.out: nbody
	date
	OMP_NUM_THREADS=$(THREADS) ./nbody 100 1 10000 100 > small.out
	date

clean:
	rm -f nbody *.out *.pdf *.tsv timing.log
