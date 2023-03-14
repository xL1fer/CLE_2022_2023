/**
 *  \file sharedMemory.h (implementation file)
 *
 *  \brief Problem name: Count Portuguese Words.
 *
 *  Synchronization based on monitors.
 *  Threads and the monitor are implemented using the pthread library which enables the creation of a
 *  monitor of the Lampson / Redell type.
 *
 *  Critical region implemented as a monitor.
 *
 *  Definition of the operations carried out by main and the workers:
 *     \li fillSharedMem
 *     \li printResults
 *     \li requestChunk
 *     \li postResults.
 *
 *  \author Author Name - Month Year
 */
 
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <pthread.h>
#include <errno.h>

#include "consts.h"

/** \brief return status on monitor initialization */
extern int statusInitMon;

/** \brief worker threads return status array */
extern int *statusWorkers;

/** \brief main thread return status */
extern int statusMain;

/** \brief number of storage positions in the data transfer region */
extern int nStorePos;

/** \brief storage region */
static struct SharedMemory sharedMemory;

/** \brief locking flag which warrants mutual exclusion inside the monitor */
static pthread_mutex_t accessCR = PTHREAD_MUTEX_INITIALIZER;

/** \brief flag which warrants that the data transfer region is initialized exactly once */
static pthread_once_t init = PTHREAD_ONCE_INIT;

/**
 *  \brief Initialization of the shared region.
 *
 *  Internal monitor operation.
 */

static void initialization(void)
{
	sharedMemory.fileId = 0;
	sharedMemory.chunkSize = MAX_CHUNK_SIZE;
	sharedMemory.openFile = false;
	sharedMemory.totalFiles = 0;
	sharedMemory.currentFile = NULL;
}

/**
 *  \brief Fill the parameters of the shared region.
 *
 *  Internal monitor operation.
 */

void fillSharedMem(char** fileNames)
{	
	if ((statusMain = pthread_mutex_lock (&accessCR)) != 0)							/* enter monitor */
	{
		errno = statusMain;															/* save error in errno */
		perror("error on entering monitor(CF)");
		statusMain = EXIT_FAILURE;
		pthread_exit(&statusMain);
	}
	pthread_once(&init, initialization);                                       		/* internal data initialization */
	
	/* initialize files names */
	int filesNumber = -1;	// do not count the first argument ("./countWords")
	char** ptr = fileNames;
	while(*ptr != 0)
	{
		filesNumber++;
		++ptr;
	}
	sharedMemory.totalFiles = filesNumber;
	
	/* alocate file names memory */
	if (((sharedMemory.fileNames = malloc((filesNumber) * sizeof(char*))) == NULL))
	{
		fprintf(stderr, "error on allocating space to the file names\n");
		statusMain = EXIT_FAILURE;
		pthread_exit(&statusMain);
	}
	/* alocate file results memory */
	if (((sharedMemory.fileResults = malloc((filesNumber) * sizeof(*sharedMemory.fileResults))) == NULL))
	{
		fprintf(stderr, "error on allocating space to file results\n");
		statusMain = EXIT_FAILURE;
		pthread_exit(&statusMain);
	}
	
	ptr = fileNames;
	for (int i = 1; ptr[i] != 0; i++)
	{
		char* subptr = ptr[i];

		int nameSize;
		for (nameSize = 0; subptr[nameSize] != 0; nameSize++);
		
		/* alocate file name memory */
		if (((sharedMemory.fileNames[i - 1] = malloc((nameSize + 1) * sizeof(char*))) == NULL))
		{
			fprintf(stderr, "error on allocating space to file name\n");
			statusMain = EXIT_FAILURE;
			pthread_exit(&statusMain);
		}
		/* alocate result file name memory */
		if (((sharedMemory.fileResults[i - 1].fileName = malloc((nameSize + 1) * sizeof(char*))) == NULL))
		{
			fprintf(stderr, "error on allocating space to result file name\n");
			statusMain = EXIT_FAILURE;
			pthread_exit(&statusMain);
		}
		
		/* fill file names and file results */
		for (int j = 0; j < nameSize; j++)
		{
			sharedMemory.fileNames[i - 1][j] = subptr[j];
			sharedMemory.fileResults[i - 1].fileName[j] = subptr[j];
		}
		
		sharedMemory.fileResults[i - 1].nWords = 0;
		for (int j = 0; j < 6; j++)
			sharedMemory.fileResults[i - 1].vowels[j] = 0;
	}
	
	printf("Shared memory filled!\n");
	
	if ((statusMain = pthread_mutex_unlock (&accessCR)) != 0)						/* exit monitor */
	{
		errno = statusMain;															/* save error in errno */
		perror("error on exiting monitor(CF)");
		statusMain = EXIT_FAILURE;
		pthread_exit(&statusMain);
	}
}

void printResults(void)
{
	if ((statusMain = pthread_mutex_lock (&accessCR)) != 0)							/* enter monitor */
	{
		errno = statusMain;															/* save error in errno */
		perror("error on entering monitor(CF)");
		statusMain = EXIT_FAILURE;
		pthread_exit(&statusMain);
	}
	pthread_once(&init, initialization);                                       		/* internal data initialization */
	
	/* print results */
	
	// ...
	
	if ((statusMain = pthread_mutex_unlock (&accessCR)) != 0)						/* exit monitor */
	{
		errno = statusMain;															/* save error in errno */
		perror("error on exiting monitor(CF)");
		statusMain = EXIT_FAILURE;
		pthread_exit(&statusMain);
	}
}

bool requestChunk(int workerId, unsigned char* buffer, int* fileId, int* chunkSize)
{	
	if ((statusWorkers[workerId] = pthread_mutex_lock (&accessCR)) != 0)			/* enter monitor */
	{
		errno = statusWorkers[workerId];											/* save error in errno */
		perror("error on entering monitor(CF)");
		statusWorkers[workerId] = EXIT_FAILURE;
		pthread_exit(&statusWorkers[workerId]);
	}
	pthread_once(&init, initialization);											/* internal data initialization */
	
	/* all files were processed, return true to end worker threads */
	if (sharedMemory.fileId >= sharedMemory.totalFiles)
	{
		if ((statusWorkers[workerId] = pthread_mutex_unlock (&accessCR)) != 0)		/* exit monitor */
		{
			errno = statusWorkers[workerId];										/* save error in errno */
			perror("error on exiting monitor(CF)");
			statusWorkers[workerId] = EXIT_FAILURE;
			pthread_exit(&statusWorkers[workerId]);
		}
		
		return false;
	}
	
	/* request chunk of text */
	*fileId = sharedMemory.fileId;
	
	/* open file if not opened yet */
	if (sharedMemory.openFile == false)
	{
		if ((sharedMemory.currentFile = fopen(sharedMemory.fileNames[sharedMemory.fileId], "r")) == NULL)
		{
			fprintf(stderr, "error on opening text file \"%s\"\n", sharedMemory.fileNames[sharedMemory.fileId]);
			statusWorkers[workerId] = EXIT_FAILURE;
			pthread_exit(&statusWorkers[workerId]);
		}
		sharedMemory.openFile = true;
	}
	
	//char chunkBuffer[MAX_CHUNK_SIZE] = "";
    if ((*chunkSize = fread(buffer, 1, MAX_CHUNK_SIZE - 1, sharedMemory.currentFile)) == EOF)
	{
		fprintf(stderr, "error on getting file chunk\n");
		statusWorkers[workerId] = EXIT_FAILURE;
		pthread_exit(&statusWorkers[workerId]);
	}
	buffer[(*chunkSize - 1)] = '\0';
	
	/* the file was completely read, point to the next one */
	if (*chunkSize < MAX_CHUNK_SIZE - 1) {
		if (fclose(sharedMemory.currentFile) == EOF)
		{
			fprintf(stderr, "error on closing text file \"%s\"\n", sharedMemory.fileNames[sharedMemory.fileId]);
			statusWorkers[workerId] = EXIT_FAILURE;
			pthread_exit(&statusWorkers[workerId]);
		}
		sharedMemory.openFile = false;
		sharedMemory.fileId++;
	}
	/* the file still has content, check if we are in the middle of a word */
	else
	{
		int i, j = 0;
		unsigned char c;
		for (i = *chunkSize - 1; i >= 0; i--)
		{
			j++;
			c = buffer[i];
			if ((c == 0x20 || c == 0x9 || c == 0xA || c == 0xD) || (c == '-' || c == '[' || c == ']' || c == '(' || c == ')') || (c == '.' || c == ',' || c == ':' || c == ';' || c == '?' || c == '!'))
				break;
		}
		
		// TODO(L1fer): check this next step is being correctly done (i or j do not need an increment or decrement) and if the above solution is valid
		
		buffer[i] = '\0';
		*chunkSize = i;
		if (fseek(sharedMemory.currentFile, ftell(sharedMemory.currentFile) - j, SEEK_SET) != 0)
		{
			fprintf(stderr, "error on seeking text file \"%s\"\n", sharedMemory.fileNames[sharedMemory.fileId]);
			statusWorkers[workerId] = EXIT_FAILURE;
			pthread_exit(&statusWorkers[workerId]);
		}
	}
	
	printf("%s\n=========================================\n", buffer);
	
	if ((statusWorkers[workerId] = pthread_mutex_unlock (&accessCR)) != 0)			/* exit monitor */
	{
		errno = statusWorkers[workerId];											/* save error in errno */
		perror("error on exiting monitor(CF)");
		statusWorkers[workerId] = EXIT_FAILURE;
		pthread_exit(&statusWorkers[workerId]);
	}
	
	return true;
}

void postResults(int workerId, struct FileResult fileResult, int* fileId)
{
	if ((statusWorkers[workerId] = pthread_mutex_lock (&accessCR)) != 0)			/* enter monitor */
	{
		errno = statusWorkers[workerId];											/* save error in errno */
		perror("error on entering monitor(CF)");
		statusWorkers[workerId] = EXIT_FAILURE;
		pthread_exit(&statusWorkers[workerId]);
	}
	pthread_once(&init, initialization);                                           /* internal data initialization */
	
	/* post obtained results */
	
	// ...
	
	if ((statusWorkers[workerId] = pthread_mutex_unlock (&accessCR)) != 0)			/* exit monitor */
	{
		errno = statusWorkers[workerId];											/* save error in errno */
		perror("error on exiting monitor(CF)");
		statusWorkers[workerId] = EXIT_FAILURE;
		pthread_exit(&statusWorkers[workerId]);
	}
}