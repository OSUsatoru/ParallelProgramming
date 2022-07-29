#!/bin/csh

# size
foreach s(1000 2500 5000 7500 10000 25000 50000 75000 100000 250000 500000 750000 1000000 2500000 5000000) #1k to 5M
    #echo NUMNODES =""
    g++ -DSIZE=$s main.cpp -o main -lm -fopenmp
    ./main
end

echo ""