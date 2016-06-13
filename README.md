# Estimating Frequencies and finding Heavy Hitters
In this project...

# Main Papers
List of papers, authors etc.

# How to use
The

# Dependencies
All code in this library is dependent on some sort of linux distibution. It has been run and tested the
newest versions of *Arch Linux* and *Ubuntu*.

To be able to run the unit tests for all implementations the Criterion unit
testing library (https://github.com/Snaipe/Criterion) must be downloaded and installed.

Furthermore to run some of the bash scripts, the terminal calculator `bc` must
be installed. This is due to the fact that bash cannot handle floating values.

### Modules
To run the benchmark scripts the module `libmeasure` must be installed and
compiled. This can be done the following way:

```bash
git submodule init
git submodule update --remote
cd modules/libmeasure
make clean && make
cd ../..
```

Or simply by checking that the __modules/libmeasure__ folder contains the libmeasure source code and the shared object file `libmeasure.so`.

Note that `libmeasure` dependent on the PAPI library (http://icl.cs.utk.edu/papi/), which is used to measure low level stuff, such as L1/L2 cache misses/accesses, branch mispredictions, cycles, instructions, running times, TLB misses/accesses and plenty more.

### External Contribution
To generate data according to some Zipfian distribution fast, the Alias Method more or less as implemented in the [R library](https://github.com/wch/r-source/blob/e5b21d0397c607883ff25cca379687b86933d730/src/main/random.c#L340) is used. Moreover to generate uniform random numbers between [0;1) throughout the project, the [`unif_rand`](https://github.com/wch/r-source/blob/e5b21d0397c607883ff25cca379687b86933d730/src/main/RNG.c#L118) has likewise been used.

To be able to compare with [existing implementations](http://hadjieleftheriou.com/frequent-items/index.html), some of the implementations was made to fit our heavy hitter abstraction, which enabled testing of the implementation on the same terms as our own implementations. The expectations and results of the authors of those implementations can be found in the paper: [Finding Frequent Items in Data Streams](http://dl.acm.org/citation.cfm?id=1454225)

To compute the median in the Count-Median Sketch, the two [implementations](http://ndevilla.free.fr/median/median/index.html) that had been found the fastest among a lot is used. 

Finally to generate the absolute errors of the sketches when the &delta;-percent of the badest estimates are removed, a [quicksort](http://www.cs.princeton.edu/~rs/Algs3.c1-4/code.txt) is performed to determine the point to cut off.

# Unit Tests
To run the unit tests simply type:
```bash
make clean && make && make test
```
This will run all unit tests in this project. To run a specific set of unittests type:
```bash
make clean && make && make run_test_{TESTNAME}
```
where the {TESTNAME} is the name of a specific test set.
