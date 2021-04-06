#!/bin/csh

# number of threads:
foreach t(1 4)
    echo ""
    g++ -DNUMT=$t main.cpp -o main -lm -fopenmp
    ./main
end