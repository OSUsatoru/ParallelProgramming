#!/bin/bash

# size
for cpus in 1 2 4 8 16 32
do
    mpic++ -o autocorrelation autocorrelation.cpp
    mpiexec -mca btl self,tcp -np $cpus autocorrelation
done

echo ""
