#!/bin/csh

# size
foreach s (1024 2048 4096 8192 16384 32768 65536 131072 262144 524288 1048576 2097152 4194304 8388608)
    foreach t (8 16 32 64 128 256 512)
        g++ -o second -DNUM_ELEMENTS=$s -DLOCAL_SIZE=$t second.cpp /usr/local/apps/cuda/cuda-10.1/lib64/libOpenCL.so.1.1 -lm -fopenmp
        ./second
    end
end

echo ""
