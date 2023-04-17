/**
 *  \file countWords.c (implementation file)
 *
 *  \brief Problem name: Count Portuguese Words.
 *
 *  Synchronization based MPI (Message Passing Interface).
 *
 *  Generator thread of the intervening entities.
 *
 *  \author Author Name - Month Year
 */
 
//	compile command
// 		mpicc -Wall -O3 -o countWords countWords.c -lm

//	run command
// 		mpiexec -n 10 ./countWords
 
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdbool.h>
#include <math.h>

#include "consts.h"

// general definitions
# define  WORKTODO       1
# define  NOMOREWORK     0

// internal functions declaration
static void parseCommandLine(int totalFiles, char** fileNames, FileResult* fileResults);

/**
 *  \brief Main function.
 *
 *  Instantiation of the processing configuration.
 *
 *  \param argc number of words of the command line
 *  \param argv list of words of the command line
 *
 *  \return status of operation
 */
int main(int argc, char *argv[])
{
	// not enough arguments provided
	if (argc < 3)
	{
		fprintf(stderr, "no thread number or file name provided\n");
		MPI_Finalize();
		return EXIT_SUCCESS;
	}
	
	// MPI initializations
	int rank, nProc;
	ChunkData *chunkData = NULL;
	FileResult *resultData = NULL;
	
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &nProc);
	
	if (((chunkData = malloc(sizeof(ChunkData))) == NULL))
	{
		fprintf(stderr, "error on allocating space to the chunk data\n");
		statusMain = EXIT_FAILURE;
		pthread_exit(&statusMain);
	}
	
	if (((resultData = malloc(sizeof(FileResult))) == NULL))
	{
		fprintf(stderr, "error on allocating space to the result data\n");
		statusMain = EXIT_FAILURE;
		pthread_exit(&statusMain);
	}
	
	// root process
	if (rank == 0)
	{
		struct FileResult* fileResults;
		char** fileNames;
		int fileId;
		int totalFiles;
		int chunkSize;
		bool openFile;
		FILE* currentFile;
		
		parseCommandLine(&totalFiles, char** fileNames, FileResult* fileResults);
		
		bool* workersAvailable;
		if (((workersAvailable = malloc(nProc * sizeof(bool))) == NULL))
		{
			fprintf(stderr, "error on allocating space to the file names\n");
			statusMain = EXIT_FAILURE;
			pthread_exit(&statusMain);
		}
		for (int i = 0; i < nProc - 1; i++)
			workersAvailable[i] = true;
		
		// while not all files have been parsed
		while (true)
		{

			for (i = (rank + 1) % size; i < size; i++)
				MPI_Isend (sndData, strlen (sndData) + 1, MPI_CHAR, i, 0, MPI_COMM_WORLD, &reqSnd[i]);
			printf ("I, %d, am going to receive a message from all other processes in the group\n", rank);
			for (i = (rank + 1) % size; i < size; i++)
			{
				MPI_Irecv (recData[i], 100, MPI_CHAR, i, 0, MPI_COMM_WORLD, &reqRec[i]);
				msgRec[i] = false;
			}
			do
			{
				allMsgRec = true;
				for (i = (rank + 1) % size; i < size; i++)
					if (!msgRec[i])
					{
						recVal = false;
						MPI_Test(&reqRec[i], (int *) &recVal, MPI_STATUS_IGNORE);
						if (recVal)
						{
							printf ("I, %d, received the message: %s\n", rank, recData[i]);
							msgRec[i] = true;
						}
						else allMsgRec = false;
					}
			} while (!allMsgRec);

		}
	}
	// remaining processes
	else
	{
		
	}
	
	
	MPI_Finalize();
	return EXIT_SUCCESS;
}

static void parseCommandLine(int* totalFiles, char** fileNames, FileResult* fileResults)
{
	// initialize files names
	int filesNumber = -2;	// do not count the first argument ("./countWords") neither the number of threads
	char** ptr = fileNames;
	while(*ptr != 0)
	{
		filesNumber++;
		++ptr;
	}
	*totalFiles = filesNumber;
	
	// alocate file names memory
	if (((fileNames = malloc((filesNumber) * sizeof(char*))) == NULL))
	{
		fprintf(stderr, "error on allocating space to the file names\n");
		statusMain = EXIT_FAILURE;
		pthread_exit(&statusMain);
	}
	// alocate file results memory
	if (((fileResults = malloc((filesNumber) * sizeof(*fileResults))) == NULL))
	{
		fprintf(stderr, "error on allocating space to file results\n");
		statusMain = EXIT_FAILURE;
		pthread_exit(&statusMain);
	}
	
	ptr = fileNames;
	int filesOffset = 2;
	for (int i = filesOffset; ptr[i] != 0; i++)
	{
		char* subptr = ptr[i];

		int nameLen;
		for (nameLen = 0; subptr[nameLen] != 0; nameLen++);
		
		// alocate file name memory
		if (((fileNames[i - filesOffset] = malloc((nameLen + 1) * sizeof(char))) == NULL))
		{
			fprintf(stderr, "error on allocating space to file name\n");
			statusMain = EXIT_FAILURE;
			pthread_exit(&statusMain);
		}
		// alocate result file name memory
		if (((fileResults[i - filesOffset].fileName = malloc((nameLen + 1) * sizeof(char))) == NULL))
		{
			fprintf(stderr, "error on allocating space to result file name\n");
			statusMain = EXIT_FAILURE;
			pthread_exit(&statusMain);
		}
		
		// fill file names and file results
		nameLen++;		// include '\0' termination
		for (int j = 0; j < nameLen; j++)
		{
			fileNames[i - filesOffset][j] = subptr[j];
			fileResults[i - filesOffset].fileName[j] = subptr[j];
		}
		
		fileResults[i - filesOffset].nWords = 0;
		for (int j = 0; j < 6; j++)
			fileResults[i - filesOffset].vowels[j] = 0;
	}
	
	printf("Program initialized!\n");
}