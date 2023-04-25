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
		
		//int test[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16,
		//			17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32 };
		//int test[] = { 32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17,
		//			16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1 };		
		//int test[] = { 321, 211, 323, 3214, 5313123, 623, 127, 32138, 339, 210, 11, 12312, 131233, 134, 1566, 1612,
		//			173113212, 18321, 1921, 23210, 233411, 214122, 2123, 24314, 2521, 261, 2317, 238, 293, 30, 31321, 3 };	
		//integerSequence = test;
		
		/*for (int i = 0; i < sequenceLen; i++)
			printf("%d ", integerSequence[i]);
		printf("\n\n");*/
	}
	
	// send sequence length to all workers
	MPI_Bcast(&sequenceLen, 1, MPI_INT, 0, MPI_COMM_WORLD);
	//printf("sequenceLen = %d\n", sequenceLen);
	
	recData = malloc(sequenceLen * sizeof(int));
	
	//nProcNow = nProc;
	presentComm = MPI_COMM_WORLD;
	MPI_Comm_group(presentComm, &presentGroup);
	for (int i = 0; i < 8; i++)
		gMemb[i] = i;
	
	int subSequenceLen = 2;
	while (subSequenceLen <= sequenceLen)
	{	
		// terminate unecessary processes
		if (subSequenceLen != 2)
		{
			//if (rank == 0) printf("> %d (nProcNow = %d)\n", sequenceLen / subSequenceLen, nProcNow);
			nProcNow = sequenceLen / subSequenceLen;
			if (nProcNow > nProc)
				nProcNow = nProc;
			//printf("> %d \n", nProcNow);
			MPI_Group_incl(presentGroup, nProcNow, gMemb, &nextGroup);
			MPI_Comm_create(presentComm, nextGroup, &nextComm);
			presentGroup = nextGroup;
			presentComm = nextComm;
			if (rank >= sequenceLen / subSequenceLen)
			{
				//printf("Process %d unecessary\n", rank);
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
			
			/*if (rank == 0)
			{
				for (int j = offset; j < offset + subSequenceLen; j++)
					printf("%d ", recData[j]);
				printf("\n");
			}*/
			//printf("%d ____DEBUG: %d ; %d\n", rank, offset, offset + subSequenceLen);
			
			sortSequence(&recData, &subSequenceLen, offset, offset + subSequenceLen);
			
			MPI_Gather(recData + offset, subSequenceLen, MPI_INT, integerSequence + offset, subSequenceLen, MPI_INT, 0, presentComm);
			
			//printf("> %d HERE (subSequenceLen = %d)\n", rank, subSequenceLen);
		}
		
		//if (rank == 0) printf("> %d\n", subSequenceLen);
		subSequenceLen *= 2;
	}
	
	validateArray(&integerSequence, &sequenceLen);
	/*for (int i = 0; i < sequenceLen; i++)
		printf("%d ", integerSequence[i]);
	printf("\n\n");*/
	
#ifdef DEBUG
	printf("Process %d finalized\n", rank);
#endif	// DEBUG
	
	MPI_Finalize();
	return EXIT_SUCCESS;
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
    //printf("> Sequence length: %d\n", sequenceLen);
	
	//maxRequests = sequenceLen / MIN_SUBLEN;
	/*if (sequenceLen < MIN_SUBLEN)
		maxRequests = sequenceLen / 32;
	else
		maxRequests = sequenceLen / MIN_SUBLEN;*/
	
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