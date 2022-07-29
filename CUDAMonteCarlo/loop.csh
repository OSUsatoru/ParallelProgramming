#!/bin/csh

# size
foreach t (1024 2048 4096 8192 16384 32768 65536 131072 262144 524288 1048576 2097152 4194304 8388608 16777216)
    foreach b (8 16 32 64 128 256)
        /usr/local/apps/cuda/cuda-10.1/bin/nvcc -DNUMTRIALS=$t -DBLOCKSIZE=$b -o montecarlo  montecarlo.cu
        ./montecarlo
    end
end

echo ""
