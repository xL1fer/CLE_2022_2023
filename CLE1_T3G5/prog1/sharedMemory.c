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

/** \brief worker threads return status array */
extern int *statusWorkers;

/** \brief main thread return status */
extern int statusMain;

/** \brief storage region */
static struct SharedMemory sharedMemory;

/** \brief locking flag which warrants mutual exclusion inside the monitor */
static pthread_mutex_t accessCR = PTHREAD_MUTEX_INITIALIZER;

/** \brief flag which warrants that the data transfer region is initialized exactly once */
static pthread_once_t init = PTHREAD_ONCE_INIT;

/** \brief check if character is separator */
static bool isSeparator(int c);

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
 *  \brief Fill shared region.
 *
 *  Operation carried out by main.
 *
 *  \param fileNames array of file names to be proceced
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

		int nameLen;
		for (nameLen = 0; subptr[nameLen] != 0; nameLen++);
		
		/* alocate file name memory */
		if (((sharedMemory.fileNames[i - 1] = malloc((nameLen + 1) * sizeof(char))) == NULL))
		{
			fprintf(stderr, "error on allocating space to file name\n");
			statusMain = EXIT_FAILURE;
			pthread_exit(&statusMain);
		}
		/* alocate result file name memory */
		if (((sharedMemory.fileResults[i - 1].fileName = malloc((nameLen + 1) * sizeof(char))) == NULL))
		{
			fprintf(stderr, "error on allocating space to result file name\n");
			statusMain = EXIT_FAILURE;
			pthread_exit(&statusMain);
		}
		
		/* fill file names and file results */
		nameLen++;		// include '\0' termination
		for (int j = 0; j < nameLen; j++)
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

/**
 *  \brief Print file results.
 *
 *  Operation carried by main.
 */

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
	for (int i = 0; i < sharedMemory.totalFiles; i++)
	{
		int* vowels = sharedMemory.fileResults[i].vowels;
		
		printf("File name: %s\n", sharedMemory.fileResults[i].fileName);
		printf("Total number of words = %d\n", sharedMemory.fileResults[i].nWords);
		printf("Number of words with an\n");
		printf("\tA\tE\tI\tO\tU\tY\n");
		printf("\t%d\t%d\t%d\t%d\t%d\t%d\n\n", vowels[A], vowels[E], vowels[I], vowels[O], vowels[U], vowels[Y]);
	}
	
	if ((statusMain = pthread_mutex_unlock (&accessCR)) != 0)						/* exit monitor */
	{
		errno = statusMain;															/* save error in errno */
		perror("error on exiting monitor(CF)");
		statusMain = EXIT_FAILURE;
		pthread_exit(&statusMain);
	}
}

/**
 *  \brief Request a chunk of text from the current file.
 *
 *  Operation carried out by the worker.
 *
 *  \param workerId woker id
 *  \param buffer buffer to store the text chunk
 *  \param fileId file identifier
 *  \param chunkSize size of the requested chunk
 *
 *	\return false if all files were parsed
 */

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
	
	/* all files were processed, return false to end worker threads */
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
		long currentFilePos;
		int c = 0, j = 0, k = 0;
		char cbuffer[4] = "";
		
		// check for uncomplete utf8 character
		for (int i = *chunkSize - 1; i >= 0; i--)
		{
			// if the character is ascii, we can continue
			if (j == 0 && (buffer[i] & 0x80) == 0)
			{
				//printf("ascii char, all good\n");
				break;
			}
			
			// get how many bytes the utf-8 char occupies
			int bytes;
			for (bytes = 1; buffer[i] & (0x80 >> bytes); bytes++);
			
			// first utf8 byte, check if the buffer has all its content
			if (bytes > 1)
			{
				//printf("utf-8 char, checking!\n");
				
				// uncomplete utf-8 char, backtrack the buffer
				if (bytes > j)
				{
					printf("uncomplete utf-8 char, backtracking buffer");
					if ((currentFilePos = ftell(sharedMemory.currentFile)) == -1L)
					{
						fprintf(stderr, "error on telling text file \"%s\"\n", sharedMemory.fileNames[sharedMemory.fileId]);
						statusWorkers[workerId] = EXIT_FAILURE;
						pthread_exit(&statusWorkers[workerId]);
					}
					if (fseek(sharedMemory.currentFile, currentFilePos - (j + 1), SEEK_SET) != 0)
					{
						fprintf(stderr, "error on seeking text file \"%s\"\n", sharedMemory.fileNames[sharedMemory.fileId]);
						statusWorkers[workerId] = EXIT_FAILURE;
						pthread_exit(&statusWorkers[workerId]);
					}
					*chunkSize -= (j + 1);
				}
				break;
			}
			j++;
		}
		
		// check for separation character
		j = 0;
		for (int i = *chunkSize - 1; i >= 0; i--)
		{
			cbuffer[j++] = buffer[i];
			
			// character is ascii, check if it is separator
			if ((buffer[i] & 0x80) == 0)
			{
				j = 0;
				if (isSeparator(buffer[i]))
				{
					//printf("ascci char is separator\n");
					buffer[i] = '\0';
					break;
				}
			}
			
			// get how many bytes the current utf-8 char occupies
			int bytes;
			for (bytes = 1; buffer[i] & (0x80 >> bytes); bytes++);
			
			// utf8 char first byte
			if (bytes > 1)
			{
				//printf("checking utf-8 character\n");
				for (int l = bytes - 1; l >= 0; l--)
					c = (c << 8) | cbuffer[l];
				
				if (isSeparator(buffer[i]))
				{
					//printf("utf-8 char is separator\n");
					buffer[i] = '\0';
					break;
				}
				
				c = 0;
			}
			k++;
		}
		
		//printf("1 chunkSize = %d\n", *chunkSize);
		
		if ((currentFilePos = ftell(sharedMemory.currentFile)) == -1L)
		{
			fprintf(stderr, "error on telling text file \"%s\"\n", sharedMemory.fileNames[sharedMemory.fileId]);
			statusWorkers[workerId] = EXIT_FAILURE;
			pthread_exit(&statusWorkers[workerId]);
		}
		if (fseek(sharedMemory.currentFile, currentFilePos - (k + 1), SEEK_SET) != 0)
		{
			fprintf(stderr, "error on seeking text file \"%s\"\n", sharedMemory.fileNames[sharedMemory.fileId]);
			statusWorkers[workerId] = EXIT_FAILURE;
			pthread_exit(&statusWorkers[workerId]);
		}
		*chunkSize -= (k + 1);
		
		//printf("2 chunkSize = %d\n", *chunkSize);
	}
	
	//printf("%s\n=========================================\n", buffer);
	
	if ((statusWorkers[workerId] = pthread_mutex_unlock (&accessCR)) != 0)			/* exit monitor */
	{
		errno = statusWorkers[workerId];											/* save error in errno */
		perror("error on exiting monitor(CF)");
		statusWorkers[workerId] = EXIT_FAILURE;
		pthread_exit(&statusWorkers[workerId]);
	}
	
	return true;
}

/**
 *  \brief Check if character is seperator.
 *
 *  Auxilar function.
 *
 *  \param c character
 *
 *	\return true if character is separator
 */

static bool isSeparator(int c)
{
	if ((c == 0x20 || c == 0x9 || c == 0xA || c == 0xD) || (c == '-' || c == 0x22 || c == 0xE2809C || c == 0xE2809D || c == '[' || c == ']' || c == '(' || c == ')') || (c == '.' || c == ',' || c == ':' || c == ';' || c == '?' || c == '!' || c == 0xE28093 || c == 0xE280A6))
		return true;
	
	return false;
}

/**
 *  \brief Save processed results in shared memory.
 *
 *  Operation carried by the worker.
 *
 *  \param workerId worker id
 *  \param fileResult file result structure
 *  \param fileId file identifier
 */

void postResults(int workerId, struct FileResult fileResult, int* fileId)
{
	if ((statusWorkers[workerId] = pthread_mutex_lock (&accessCR)) != 0)			/* enter monitor */
	{
		errno = statusWorkers[workerId];											/* save error in errno */
		perror("error on entering monitor(CF)");
		statusWorkers[workerId] = EXIT_FAILURE;
		pthread_exit(&statusWorkers[workerId]);
	}
	pthread_once(&init, initialization);                                           	/* internal data initialization */
	
	/* post obtained results */
	sharedMemory.fileResults[*fileId].nWords += fileResult.nWords;
	for (int i = 0; i < 6; i++)
		sharedMemory.fileResults[*fileId].vowels[i] += fileResult.vowels[i];
	
	if ((statusWorkers[workerId] = pthread_mutex_unlock (&accessCR)) != 0)			/* exit monitor */
	{
		errno = statusWorkers[workerId];											/* save error in errno */
		perror("error on exiting monitor(CF)");
		statusWorkers[workerId] = EXIT_FAILURE;
		pthread_exit(&statusWorkers[workerId]);
	}
}
