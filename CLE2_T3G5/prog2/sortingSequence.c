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
	int rank, nProc;
	
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &nProc);
	int* integerSequence = NULL;
	int sequenceLen = 0;
	
	if (nProc < 1)
	{
		fprintf(stderr, "there must be at least 1 processes\n");
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
		
		/*for (int i = 0; i < 32; i++)
			printf("%d ", integerSequence[i]);
		printf("\n");*/
	}
	
	// send sequence length to all workers
	MPI_Bcast(&sequenceLen, 1, MPI_INT, 0, MPI_COMM_WORLD);
	//printf("sequenceLen = %d\n", sequenceLen);
	
	int subSequenceLen = 2;
	while (subSequenceLen <= sequenceLen)
	{
		//MPI_Scatter(sequence, SUBSEQUENCE_LEN, MPI_INT, subSequence, SUBSEQUENCE_LEN, MPI_INT, 0, MPI_COMM_WORLD);
		
		//sortSequence();
		
		//MPI_Gather (sendResults, 2, MPI_INT, recvResults, 2, MPI_INT, 0, MPI_COMM_WORLD);
		
		subSequenceLen *= 2;
	}
	
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