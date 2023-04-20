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
	
	
}