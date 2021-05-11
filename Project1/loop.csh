#!/bin/csh

# number of threads:
foreach t(1 2 4 8)
    echo NUMT = ""
    foreach s(1 10 100 1000 10000 100000 500000)
        echo NUMTRIALS =""
        g++ -DNUMTRIALS=$s -DNUMT=$t main.cpp -o main -lm -fopenmp
        ./main
    end
end