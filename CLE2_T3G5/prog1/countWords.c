/**
 *  \file countWords.c (implementation file)
 *
 *  \brief Problem name: Count Portuguese Words.
 *
 *  Synchronization based MPI (Message Passing Interface).
 *
 *  \author Author Name - Month Year
 */
 
//	compile command
// 		mpicc -Wall -O3 -o countWords countWords.c -lm

//	run command
// 		mpiexec -n 10 ./countWords text0.txt text1.txt text2.txt text3.txt text4.txt
 
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdbool.h>
#include <math.h>
#include <string.h>

#include "consts.h"

// general definitions
#define WORKTODO       1
#define NOMOREWORK     0

#define DEBUG

// internal functions declaration
static int parseCommandLine(char** commandLineArgs, int* totalFiles, char*** fileNames, struct FileResult** fileResults);

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
	// MPI initializations
	int rank, nProc;
	struct ChunkData *chunkData = NULL;
	struct FileResult *resultData = NULL;
	
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &nProc);
	
	// not enough arguments provided
	if (argc < 2)
	{
		fprintf(stderr, "no file name provided\n");
		MPI_Finalize();
		return EXIT_FAILURE;
	}
	
	if (((chunkData = malloc(sizeof(struct ChunkData))) == NULL))
	{
		fprintf(stderr, "error on allocating space to the chunk data buffer\n");
		MPI_Finalize();
		return EXIT_FAILURE;
	}
	
	if (((resultData = malloc(sizeof(struct FileResult))) == NULL))
	{
		fprintf(stderr, "error on allocating space to the result data buffer\n");
		MPI_Finalize();
		return EXIT_FAILURE;
	}
	
	// dispatcher process
	if (rank == 0)
	{
		int totalFiles = 0;
		char** fileNames = NULL;
		struct FileResult* fileResults = NULL;
		int fileId = 0;
		FILE* currentFile = NULL;
		//int chunkSize = 0;	// NOT NEEDED, ALREADY IN THE "chunkData" STRUCTURE (DELETE THIS LATER)
		bool openFile;
		bool* availWorkers;
		
		// parse command line arguments and initialize array variables
		if (parseCommandLine(argv, &totalFiles, &fileNames, &fileResults) == 1)
		{
			MPI_Finalize();
			return EXIT_FAILURE;
		}
		
		// initialize available workers array
		if (((availWorkers = malloc((nProc - 1) * sizeof(bool))) == NULL))
		{
			fprintf(stderr, "error on allocating space to the file names\n");
			MPI_Finalize();
			return EXIT_FAILURE;
		}
		for (int i = 0; i < nProc - 1; i++)
			availWorkers[i] = true;
		
#ifdef DEBUG
		for (int i = 0; i < totalFiles; i++)
			printf("> %s\n", fileNames[i]);
#endif // DEBUG

		// NOTE (L1fer): MOVE THIS TO THE FUNCTION THAT GETS CHUNKS OF DATA
		// open next file
		if ((currentFile = fopen(fileNames[fileId++], "r")) == NULL)
		{
			fprintf(stderr, "error on opening text file \"%s\"\n", fileNames[fileId - 1]);
			MPI_Finalize();
			return EXIT_FAILURE;
		}
		
		MPI_Request reqs[nProc];
		
		// while not all files have been parsed
		//while (true)
		{
			for (int i = (rank + 1) % nProc; i < nProc; i++)
			{
				if (availWorkers[i])
				{
					//getFileChunk(chunkData, currentFile);
					MPI_Isend(chunkData, sizeof(struct ChunkData), MPI_BYTE, i, 0, MPI_COMM_WORLD, &reqs[i]);
				}
			}
			/*
			for (int i = (rank + 1) % nProc; i < nProc; i++)
				MPI_Isend (sndData, strlen (sndData) + 1, MPI_CHAR, i, 0, MPI_COMM_WORLD, &reqSnd[i]);
			printf ("I, %d, am going to receive a message from all other processes in the group\n", rank);
			for (i = (rank + 1) % nProc; i < nProc; i++)
			{
				MPI_Irecv (recData[i], 100, MPI_CHAR, i, 0, MPI_COMM_WORLD, &reqRec[i]);
				msgRec[i] = false;
			}
			do
			{
				allMsgRec = true;
				for (i = (rank + 1) % nProc; i < nProc; i++)
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
			*/
		}
	}
	// workers processes
	else
	{
		do
		{
			MPI_Recv(chunkData, sizeof(struct ChunkData), MPI_BYTE, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			printf(">>>>>> %d\n", chunkData->hasWork);
		} while(chunkData->hasWork == 1);
	}
	
#ifdef DEBUG
	printf("Process %d finalized\n", rank);
#endif	// DEBUG
	
	MPI_Finalize();
	return EXIT_SUCCESS;
}

/**
 *  \brief Parse command line arguments.
 *
 *  Operation carried out by dispatcher process.
 *
 *  \param commandLineArgs command line arguments string array
 *  \param totalFiles total number of files
 *  \param fileNames file names array
 *  \param fileResults file results array
 *
 *	\return 0 if no error occured
 */
static int parseCommandLine(char** commandLineArgs, int* totalFiles, char*** fileNames, struct FileResult** fileResults)
{
	// initialize files names
	int filesNumber = -1;	// do not count the first argument ("./countWords") neither the number of threads
	char** ptr = commandLineArgs;
	while(*ptr != 0)
	{
		filesNumber++;
		++ptr;
	}
	*totalFiles = filesNumber;
	
	// allocate file names memory
	if (((*fileNames = malloc((filesNumber) * sizeof(char*))) == NULL))
	{
		fprintf(stderr, "error on allocating space to the file names\n");
		return 1;
	}
	// allocate file results memory
	if (((*fileResults = malloc((filesNumber) * sizeof(struct FileResult))) == NULL))
	{
		fprintf(stderr, "error on allocating space to file results\n");
		return 1;
	}
	
	ptr = commandLineArgs;
	int filesOffset = 1;
	for (int i = filesOffset; ptr[i] != 0; i++)
	{
		char* subptr = ptr[i];

		int nameLen;
		for (nameLen = 0; subptr[nameLen] != 0; nameLen++);
		
		// allocate file name memory
		if ((((*fileNames)[i - filesOffset] = malloc((nameLen + 1) * sizeof(char))) == NULL))
		{
			fprintf(stderr, "error on allocating space to file name\n");
			return 1;
		}
		
		// fill file names
		nameLen++;		// include '\0' termination
		for (int j = 0; j < nameLen; j++)
			(*fileNames)[i - filesOffset][j] = subptr[j];
		
		(*fileResults)[i - filesOffset].nWords = 0;
		for (int j = 0; j < 6; j++)
			(*fileResults)[i - filesOffset].vowels[j] = 0;
	}
	
	printf("Memory initialized!\n");
	
	return 0;
}