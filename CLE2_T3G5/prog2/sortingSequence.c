/**
 *  \file sortingSequence.c (implementation file)
 *
 *  \brief Problem name: Sorting Integer Sequence.
 *
 *  Synchronization based MPI (Message Passing Interface).
 *
 *  \author Author Name - Month Year
 */

//	compile command
// 		mpicc -Wall -O3 -o sortingSequence sortingSequence.c -lm

//	run command
// 		mpiexec -n 4 ./sortingSequence datSeq32.bin

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdbool.h>
#include <math.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

#include "consts.h"

// general definitions
#define DEBUG

// internal functions declaration
static double get_delta_time(void);
static bool readIntegerSequence(int** integerSequence, int* sequenceLen, char* fileName);
static void sortSequence(int** integerSequence, int* subSequenceLen, int startOffset, int endOffset);
static void validateArray(int** integerSequence, int* sequenceLen);

/**
 *  \brief Main thread.
 *
 *  Its role is starting the simulation by generating the intervening entities threads (workers) and
 *  waiting for their termination.
 *
 *  \param argc number of words of the command line
 *  \param argv list of words of the command line
 *
 *  \return status of operation
 */
int main(int argc, char *argv[])
{
	// MPI initializations
	MPI_Comm presentComm, nextComm;
	MPI_Group presentGroup, nextGroup;
	int gMemb[8];
	int rank, nProc, nProcNow;//, length, nIter;
	int *integerSequence = NULL, *recData = NULL;
	int sequenceLen = 0;
	
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &nProc);
	
	// verify number of processes
	if (nProc < 1)
	{
		fprintf(stderr, "there must be at least 1 processes\n");
		MPI_Finalize();
		return EXIT_FAILURE;
	}
	else if (nProc > 8)
	{
		if (rank == 0) fprintf(stderr, "too many processes, it should be a power of 2, less or equal to 8\n");
		MPI_Finalize();
		return EXIT_FAILURE;
	}
	
	// not enough arguments provided
	if (argc < 2)
	{
		fprintf(stderr, "no file name provided\n");
		MPI_Finalize();
		return EXIT_FAILURE;
	}
	
	// distributor/worker process
	if (rank == 0)
	{	
		if (readIntegerSequence(&integerSequence, &sequenceLen, argv[1]) == false)
		{
			MPI_Finalize();
			return EXIT_FAILURE;
		}
	}
	
	// send sequence length to all workers
	MPI_Bcast(&sequenceLen, 1, MPI_INT, 0, MPI_COMM_WORLD);
	
	recData = malloc(sequenceLen * sizeof(int));
	
	presentComm = MPI_COMM_WORLD;
	MPI_Comm_group(presentComm, &presentGroup);
	for (int i = 0; i < 8; i++)
		gMemb[i] = i;
	
	(void) get_delta_time();
	
	int subSequenceLen = MIN_SUBLEN;
	if (subSequenceLen > sequenceLen) subSequenceLen = sequenceLen;
	while (subSequenceLen <= sequenceLen)
	{
		// terminate unecessary processes
		if (subSequenceLen != 2)
		{
			nProcNow = sequenceLen / subSequenceLen;
			if (nProcNow > nProc)
				nProcNow = nProc;
			
			MPI_Group_incl(presentGroup, nProcNow, gMemb, &nextGroup);
			MPI_Comm_create(presentComm, nextGroup, &nextComm);
			presentGroup = nextGroup;
			presentComm = nextComm;
			if (rank >= sequenceLen / subSequenceLen)
			{
				free(recData);
				MPI_Finalize();
				return EXIT_SUCCESS;
			}
		}
		MPI_Comm_size(presentComm, &nProc);
		
		for (int i = 0; i < sequenceLen; i += nProc * subSequenceLen)
		{
			int offset = i + (rank * subSequenceLen);
			
			MPI_Scatter(integerSequence + offset, subSequenceLen, MPI_INT, recData + offset, subSequenceLen, MPI_INT, 0, presentComm);
			
			sortSequence(&recData, &subSequenceLen, offset, offset + subSequenceLen);
			
			MPI_Gather(recData + offset, subSequenceLen, MPI_INT, integerSequence + offset, subSequenceLen, MPI_INT, 0, presentComm);
		}
		
		subSequenceLen *= 2;
		
		//if (rank == 0) printf("> %d ; %d\n", subSequenceLen, sequenceLen);
	}
	
	validateArray(&integerSequence, &sequenceLen);
	
#ifdef DEBUG
	printf("Process %d finalized\n", rank);
#endif	// DEBUG

	// print total time
	printf("\nElapsed time = %.6f s\n", get_delta_time());
	
	MPI_Finalize();
	return EXIT_SUCCESS;
}

/**
 *  \brief Get the process time that has elapsed since last call of this time.
 *
 *  \return process elapsed time
 */
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

/**
 *  \brief Read binary file integer sequence.
 *
 *  Operation carried out by distributor
 *
 *  \param integerSequence integer sequence array
 *  \param sequenceLen integer sequence length
 *  \param fileName integer sequence file name
 *
 *	\return true everything went well
 */
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
	
	printf("[Distributor] integer sequence parsed\n");
	
	// close binary file
	if (fclose(filePointer) == EOF)
	{
		fprintf(stderr, "error on closing text file \"%s\"\n", fileName);
		return false;
	}
	
	return true;
}

/**
 *  \brief Verify if the integer sequence is sorted.
 *
 *  Operation carried out by distributor
 */
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

/**
 *  \brief Bitonic sort a sequence of integers.
 *
 *	Operation carried out by worker.
 *
 *	Addapted from https://en.wikipedia.org/wiki/Bitonic_sorter
 *
 *  \param integerSequence sequence to be sorted
 *  \param subSequenceLen length of the sequence
 */
static void sortSequence(int** integerSequence, int* subSequenceLen, int startOffset, int endOffset)
{
	if (*subSequenceLen <= MIN_SUBLEN)
	{
		for (int k = 2; k <= *subSequenceLen; k *= 2) // k is doubled every iteration
		{
			for (int j = k / 2; j > 0; j /= 2) // j is halved at every iteration, with truncation of fractional parts
			{
				for (int i = startOffset; i < endOffset; i++)
				{
					int l = i ^ j;
					if (l > i)
					{
						if ((((i & k) == 0) && ((*integerSequence)[i] > (*integerSequence)[l])) || (((i & k) != 0) && ((*integerSequence)[i] < (*integerSequence)[l])))
						{
							int temp = (*integerSequence)[i];
							(*integerSequence)[i] = (*integerSequence)[l];
							(*integerSequence)[l] = temp;
						}
					}
				}
			}
		}
	}
	else
	{
		int k = *subSequenceLen;
		
		for (int j = k / 2; j > 0; j /= 2) // j is halved at every iteration, with truncation of fractional parts
		{
			for (int i = startOffset; i < endOffset; i++)
			{
				int l = i ^ j;
				if (l > i)
				{
					if ((((i & k) == 0) && ((*integerSequence)[i] > (*integerSequence)[l])) || (((i & k) != 0) && ((*integerSequence)[i] < (*integerSequence)[l])))
					{
						int temp = (*integerSequence)[i];
						(*integerSequence)[i] = (*integerSequence)[l];
						(*integerSequence)[l] = temp;
					}
				}
			}
		}
	}
}