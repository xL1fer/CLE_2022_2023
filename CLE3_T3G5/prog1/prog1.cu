/**
 *   Tomás Oliveira e Silva, November 2017
 */

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include <cuda_runtime.h>

/**
 *   program configuration
 */

#ifndef SECTOR_SIZE
	#define SECTOR_SIZE 512
#endif
#ifndef N_SECTORS
	#define N_SECTORS (1 << 21) // it can go as high as (1 << 21)
#endif

#ifndef N 1024
	#define N 1024
#endif

// allusion to internal functions

static bool readIntegerSequence(int** integerSequence, int* sequenceLen, char* fileName);

static void validateArray(int** integerSequence, int* sequenceLen);

__global__ static void sort_sequence_cuda_kernel(int * __restrict__ integerSequence, int subSequenceLen, int startOffset, int endOffset);

static double get_delta_time(void);

/**
 *   main program
 */
int main(int argc, char **argv)
{
    printf("%s Starting...\n", argv[0]);
    if (sizeof(unsigned int) != (size_t)4)
        return 1; // it fails with prejudice if an integer does not have 4 bytes
	
	// verify arguments number
    if (argc != 2)
    {
        printf("Wrong number of arguments!\n");
        return 1;
    }

    // set up the device

    int dev = 0;

    cudaDeviceProp deviceProp;
    CHECK(cudaGetDeviceProperties(&deviceProp, dev));
    printf("Using Device %d: %s\n", dev, deviceProp.name);
    CHECK(cudaSetDevice(dev));
	
	// parse integer sequence in host memory
	
	int *integerSequence = NULL;
	int sequenceLen = 0;
	
	if (!readIntegerSequence(&integerSequence, &sequenceLen, argv[1]))
	{
		return 1;
	}
	
	// create memory areas in device memory
	
	int *deviceIntegerSequence;

	if (sequenceLen > (size_t)5e9)
	{
		fprintf(stderr, "The GeForce GTX 1660 Ti cannot handle more than 5GB of memory!\n");
		exit(1);
	}

	CHECK(cudaMalloc((void **)&deviceIntegerSequence, sequenceLen));

    // copy the host data to the device memory

    CHECK(cudaMemcpy(deviceIntegerSequence, integerSequence, sequenceLen, cudaMemcpyHostToDevice));

    // run the computational kernel

	(void)get_delta_time();

	for (int iter = 0; iter < 9; iter++)
	{
		unsigned int gridDimX, gridDimY, gridDimZ, blockDimX, blockDimY, blockDimZ;
		
		blockDimX = 1 << 0; // optimize!
		blockDimY = 1 << 0; // optimize!
		blockDimZ = 1 << 0; // do not change!
		gridDimX = 1 << 21; // optimize!
		gridDimY = 1 << 0;  // optimize!
		gridDimZ = 1 << 0;  // do not change!
		
		switch (iter)
		{
			case 0:
			
				break;
			case 1:
				
				break;
			case 2:
				
				break;
			case 3:
				
				break;
			case 4:
				
				break;
			case 5:
				
				break;
			case 6:
				
				break;
			case 7:
				
				break;
			case 8:
				
				break;
			case 9:
				
				break;
			default:
				break;
		}
		
		dim3 grid(gridDimX, gridDimY, gridDimZ);
		dim3 block(blockDimX, blockDimY, blockDimZ);
		
		sort_sequence_cuda_kernel<<<grid, block>>>();	// sort sequence
		CHECK(cudaDeviceSynchronize()); 				// wait for kernel to finish
		CHECK(cudaGetLastError());      				// check for kernel errors
	}
	
	printf("The CUDA kernel <<<(%d,%d,%d), (%d,%d,%d)>>> took %.3e seconds to run\n",
							   gridDimX, gridDimY, gridDimZ, blockDimX, blockDimY, blockDimZ, get_delta_time());

    // copy kernel result back to host side

    CHECK(cudaMemcpy(integerSequence, deviceIntegerSequence, sequenceLen, cudaMemcpyDeviceToHost));

    // free device global memory

    CHECK(cudaFree(deviceIntegerSequence));

    // reset the device

    CHECK(cudaDeviceReset());

	// validate results
	
	validateArray(&integerSequence, &sequenceLen);

    // free host memory

    free(integerSequence);

    return 0;
}

static bool readIntegerSequence(int** integerSequence, int* sequenceLen, char* fileName)
{
	FILE* filePointer = NULL;
	// open binary file
	if ((filePointer = fopen(fileName, "rb")) == NULL)
	{
		fprintf(stderr, "error on opening file \"%s\"\n", fileName);
		return false;
	}
	
	// get number of sequence elements
    if (fread(sequenceLen, sizeof(int), 1, filePointer) == EOF)
	{
		fprintf(stderr, "error on reading integer sequence length\n");
		return false;
	}
	
	// alocate integer sequence memory
	if ((*integerSequence = malloc((*sequenceLen) * sizeof(int))) == NULL)
	{
		fprintf(stderr, "error on allocating space to file name\n");
		return false;
	}

    // get the sequence of integers
    for (int i = 0; i < (*sequenceLen); i++)
    {	
		if (fread((*integerSequence) + i, sizeof(int), 1, filePointer) == EOF)
		{
			fprintf(stderr, "error on reading integer sequence length\n");
			return false;
		}
    }
	
	printf("Integer sequence parsed\n");
	
	// close binary file
	if (fclose(filePointer) == EOF)
	{
		fprintf(stderr, "error on closing text file \"%s\"\n", fileName);
		return false;
	}
	
	return true;
}

static void validateArray(int** integerSequence, int* sequenceLen)
{
    for (int i = 0; i < *sequenceLen - 1; i++)
    {
        if ((*integerSequence)[i] > (*integerSequence)[i + 1])
        {
            printf("Error in position %d between element %d and %d\n", i, (*integerSequence)[i], (*integerSequence)[i + 1]);
            return;
        }
    }
    printf("Everything is OK!\n");
}

__global__ static void sort_sequence_cuda_kernel(int * __restrict__ integerSequence, int subSequenceLen, int startOffset, int endOffset)
{
    unsigned int x, y, idx;

    // compute the thread number

    x = (unsigned int)threadIdx.x + (unsigned int)blockDim.x * (unsigned int)blockIdx.x;
    y = (unsigned int)threadIdx.y + (unsigned int)blockDim.y * (unsigned int)blockIdx.y;
    idx = (unsigned int)blockDim.x * (unsigned int)gridDim.x * y + x;

    // sort sequence
	
	for (int k = 2; k <= *subSequenceLen; k *= 2) // k is doubled every iteration
	{
		for (int j = k / 2; j > 0; j /= 2) // j is halved at every iteration, with truncation of fractional parts
		{
			for (int i = startOffset; i < endOffset; i++)
			{
				int l = i ^ j;
				if (l > i)
				{
					if ((((i & k) == 0) && (integerSequence[i] > integerSequence[l])) || (((i & k) != 0) && (integerSequence[i] < integerSequence[l])))
					{
						int temp = integerSequence[i];
						integerSequence[i] = integerSequence[l];
						integerSequence[l] = temp;
					}
				}
			}
		}
	}
	
	
}

static double get_delta_time(void)
{
    static struct timespec t0, t1;

    t0 = t1;
    if (clock_gettime(CLOCK_MONOTONIC, &t1) != 0)
    {
        perror("clock_gettime");
        exit(1);
    }
    return (double)(t1.tv_sec - t0.tv_sec) + 1.0e-9 * (double)(t1.tv_nsec - t0.tv_nsec);
}
