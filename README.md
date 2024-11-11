DIMACS is a format describing a boolean formula in conjunctive normal form (CNF).
The aim of the fuzzer is to generate multiple random DIMACS to find bugs in SAT solvers.
The interface of the fuzzer is `fuzz-sat /path/to/SAT /path/to/inputs seed`

where:
- `/path/to/SAT` refers to the source directory of the SAT solver containing the built solver (with gcov coverage information, ASan, and UBSan enabled) and the `runsat.sh` script that needs to be used to run the solver. The `runsat.sh` script expects a single command line argument, the path to a file containing the input formula, and prints out whether the formula was satisfiable, optionally a model, and the error reports of ASan and UBSan if any issues were detected.
- `/path/to/inputs` refers to a directory containing a non-empty set of well-formed DIMACS files.
- `seed` is an integer that you should use to initialize a random number generator if you need one.
