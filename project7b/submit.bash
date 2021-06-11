#!/bin/bash
#SBATCH -J autocorrelation
#SBATCH -A cs475-575
#SBATCH -p class
#SBATCH -N 8 # number of nodes
#SBATCH -n 8 # number of tasks
#SBATCH -o autocorrelation.out
#SBATCH -e autocorrelation.csv
#SBATCH --mail-type=BEGIN,END,FAIL
#SBATCH --mail-user=yamamsat@oregonstate.edu
for cpus in 1 2 4 8 16 32
do
    mpic++ -o autocorrelation autocorrelation.cpp
    mpiexec -mca btl self,tcp -np $cpus autocorrelation
done