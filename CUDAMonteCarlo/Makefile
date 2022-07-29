#CUDA_PATH	=	/usr/local/apps/cuda/cuda-11.1
CUDA_PATH	=	/usr/local/apps/cuda/cuda-10.1
CUDA_BIN_PATH	=	$(CUDA_PATH)/bin
CUDA_NVCC	=	$(CUDA_BIN_PATH)/nvcc

montecarlo:	montecarlo.cu
		$(CUDA_NVCC) -o montecarlo  montecarlo.cu