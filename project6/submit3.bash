#!/bin/bash
#SBATCH -J Montecarlo
#SBATCH -A cs475-575
#SBATCH -p class
#SBATCH --gres=gpu:1
#SBATCH -o third.out
#SBATCH -e third.err
#SBATCH --mail-type=BEGIN,END,FAIL
#SBATCH --mail-user=yamamsat@oregonstate.edu
for s in 1024 2048 4096 8192 16384 32768 65536 131072 262144 524288 1048576 2097152 4194304 8388608
    do
    for t in 8 16 32 64 128 256 512
        do
        g++ -o third -DNUM_ELEMENTS=$s -DLOCAL_SIZE=$t third.cpp /usr/local/apps/cuda/cuda-10.1/lib64/libOpenCL.so.1.1 -lm -fopenmp
        ./third
    done

done