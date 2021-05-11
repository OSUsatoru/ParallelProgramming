#!/bin/csh

# number of threads:
foreach t(1 2 4 8)
    #echo NUMT = ""
    echo ""
    foreach s(4 8 16 32 64 128 256 512 1024)
        #echo NUMNODES =""
        g++ -DNUMNODES=$s -DNUMT=$t main.cpp -o main -lm -fopenmp
        ./main
    end
end

echo ""