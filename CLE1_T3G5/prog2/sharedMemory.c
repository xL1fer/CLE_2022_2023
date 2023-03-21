/**
 *  \file sharedMemory.h (implementation file)
 *
 *  \brief Problem name: Sorting Integer Sequence.
 *
 *  Synchronization based on monitors.
 *  Threads and the monitor are implemented using the pthread library which enables the creation of a
 *  monitor of the Lampson / Redell type.
 *
 *  Critical region implemented as a monitor.
 *
 *  Definition of the operations carried out by main, distributor and the workers:
 *     \li fillFileName
 *     \li readIntegerSequence
 *     \li assignWork
 *     \li requestWork
 *     \li informWork
 *     \li validateArray.
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

/** \brief main thread return status */
extern int statusMain;

/** \brief distributor thread return status */
extern int statusDistributor;

/** \brief worker threads return status array */
extern int *statusWorkers;

/** \brief storage region */
static struct SharedMemory sharedMemory;

/** \brief locking flag which warrants mutual exclusion inside the monitor */
static pthread_mutex_t accessCR = PTHREAD_MUTEX_INITIALIZER;

/** \brief flag which warrants that the data transfer region is initialized exactly once */
static pthread_once_t init = PTHREAD_ONCE_INIT;

/** \brief distributor synchronization point when waiting for work requests */
static pthread_cond_t waitRequest;

/** \brief distributor synchronization point when waiting for work done */
static pthread_cond_t waitCompletion;

/** \brief worker synchronization point when waiting for work */
static pthread_cond_t waitAssignment;

/**
 *  \brief Initialization of the shared region.
 *
 *  Internal monitor operation.
 */

static void initialization(void)
{
	sharedMemory.sequenceLen = 0;
	
	sharedMemory.maxRequests = 0;
	sharedMemory.curRequests = 0;
	sharedMemory.completeRequests = 0;
	sharedMemory.totalRequests = 0;
	
	sharedMemory.workAvailable = false;
	sharedMemory.workNeeded = true;
	
	pthread_cond_init (&waitRequest, NULL);				/* initialize distributor synchronization point */
	pthread_cond_init (&waitCompletion, NULL);			/* initialize distributor synchronization point */
	pthread_cond_init (&waitAssignment, NULL);			/* initialize workers synchronization point */
}

/**
 *  \brief Fill file name.
 *
 *  Operation carried out by main.
 *
 *  \param fileNames array of file names to be proceced
 */

void fillFileName(char* fileName)
{	
	if ((statusMain = pthread_mutex_lock(&accessCR)) != 0)							/* enter monitor */
	{
		errno = statusMain;															/* save error in errno */
		perror("error on entering monitor(CF)");
		statusMain = EXIT_FAILURE;
		pthread_exit(&statusMain);
	}
	pthread_once(&init, initialization);                                       		/* internal data initialization */
	
	/* initialize file name */
	int nameLen;
	for (nameLen = 0; fileName[nameLen] != 0; nameLen++);
	
	/* alocate file name memory */
	if (((sharedMemory.fileName = malloc((nameLen + 1) * sizeof(char))) == NULL))
	{
		fprintf(stderr, "error on allocating space to file name\n");
		statusMain = EXIT_FAILURE;
		pthread_exit(&statusMain);
	}
	
	/* fill file name */
	nameLen++;		// include '\0' termination
	for (int i = 0; i < nameLen; i++)
		sharedMemory.fileName[i] = fileName[i];
	
	printf("File name parsed (%s)\n", sharedMemory.fileName);
	
	if ((statusMain = pthread_mutex_unlock(&accessCR)) != 0)						/* exit monitor */
	{
		errno = statusMain;															/* save error in errno */
		perror("error on exiting monitor(CF)");
		statusMain = EXIT_FAILURE;
		pthread_exit(&statusMain);
	}
}

/**
 *  \brief Verify if the integer sequence is sorted.
 *
 *  Operation carried out by main
 */

void validateArray(void)
{
	if ((statusMain = pthread_mutex_lock(&accessCR)) != 0)							/* enter monitor */
	{
		errno = statusMain;															/* save error in errno */
		perror("error on entering monitor(CF)");
		statusMain = EXIT_FAILURE;
		pthread_exit(&statusMain);
	}
	pthread_once(&init, initialization);                                       		/* internal data initialization */
	
    for (int i = 0; i < sharedMemory.sequenceLen - 1; i++)
    {
        if (sharedMemory.integerSequence[i] > sharedMemory.integerSequence[i + 1])
        {
            printf("Error in position %d between element %d and %d\n", i, sharedMemory.integerSequence[i], sharedMemory.integerSequence[i + 1]);
			
			if ((statusMain = pthread_mutex_unlock(&accessCR)) != 0)						/* exit monitor */
			{
				errno = statusMain;															/* save error in errno */
				perror("error on exiting monitor(CF)");
				statusMain = EXIT_FAILURE;
				pthread_exit(&statusMain);
			}
			
            return;
        }
    }
    printf("Everything is OK!\n");
	
	if ((statusMain = pthread_mutex_unlock(&accessCR)) != 0)						/* exit monitor */
	{
		errno = statusMain;															/* save error in errno */
		perror("error on exiting monitor(CF)");
		statusMain = EXIT_FAILURE;
		pthread_exit(&statusMain);
	}
}

/**
 *  \brief Read binary file integer sequence.
 *
 *  Operation carried out by distributor
 *
 *  \param fileNames array of file names to be proceced
 */

void readIntegerSequence(void)
{
	if ((statusDistributor = pthread_mutex_lock(&accessCR)) != 0)							/* enter monitor */
	{
		errno = statusDistributor;															/* save error in errno */
		perror("error on entering monitor(CF)");
		statusDistributor = EXIT_FAILURE;
		pthread_exit(&statusDistributor);
	}
	pthread_once(&init, initialization);                                       				/* internal data initialization */
	
	/* open binary file */
	if ((sharedMemory.filePointer = fopen(sharedMemory.fileName, "rb")) == NULL)
	{
		fprintf(stderr, "error on opening file \"%s\"\n", sharedMemory.fileName);
		statusDistributor = EXIT_FAILURE;
		pthread_exit(&statusDistributor);
	}
	
	/* get number of sequence elements */
    if (fread(&sharedMemory.sequenceLen, sizeof(int), 1, sharedMemory.filePointer) == EOF)
	{
		fprintf(stderr, "error on reading integer sequence length\n");
		statusDistributor = EXIT_FAILURE;
		pthread_exit(&statusDistributor);
	}
    //printf("> Sequence length: %d\n", sharedMemory.sequenceLen);
	sharedMemory.maxRequests = sharedMemory.sequenceLen / MIN_SUBLEN;
	
	/* alocate integer sequence memory */
	if ((sharedMemory.integerSequence = malloc(sharedMemory.sequenceLen * sizeof(int))) == NULL)
	{
		fprintf(stderr, "error on allocating space to file name\n");
		statusDistributor = EXIT_FAILURE;
		pthread_exit(&statusDistributor);
	}

    /* get the sequence of integers */
    for (int i = 0; i < sharedMemory.sequenceLen; i++)
    {	
		if (fread(sharedMemory.integerSequence + i, sizeof(int), 1, sharedMemory.filePointer) == EOF)
		{
			fprintf(stderr, "error on reading integer sequence length\n");
			statusDistributor = EXIT_FAILURE;
			pthread_exit(&statusDistributor);
		}
    }
	
	printf("[Distributor] integer sequence parsed\n");
	
	/* close binary file */
	if (fclose(sharedMemory.filePointer) == EOF)
	{
		fprintf(stderr, "error on closing text file \"%s\"\n", sharedMemory.fileName);
		statusDistributor = EXIT_FAILURE;
		pthread_exit(&statusDistributor);
	}
	
	if ((statusDistributor = pthread_mutex_unlock(&accessCR)) != 0)						/* exit monitor */
	{
		errno = statusDistributor;															/* save error in errno */
		perror("error on exiting monitor(CF)");
		statusDistributor = EXIT_FAILURE;
		pthread_exit(&statusDistributor);
	}
}

/**
 *  \brief Read binary file integer sequence.
 *
 *  Operation carried out by distributor
 *
 *  \return true if there is more work to be done
 */

bool assignWork(void)
{
	if ((statusDistributor = pthread_mutex_lock(&accessCR)) != 0)							/* enter monitor */
	{
		errno = statusDistributor;															/* save error in errno */
		perror("error on entering monitor(CF)");
		statusDistributor = EXIT_FAILURE;
		pthread_exit(&statusDistributor);
	}
	pthread_once(&init, initialization);                                       				/* internal data initialization */
  
	while (sharedMemory.totalRequests == 0 || sharedMemory.workAvailable)					/* wait while there are no work requests or no thread picked the current work */
	{
		if ((statusDistributor = pthread_cond_wait(&waitRequest, &accessCR)) != 0)
		{
			errno = statusDistributor;                            							/* save error in errno */
			perror("error on waiting in waitRequest");
			statusDistributor = EXIT_FAILURE;
			pthread_exit(&statusDistributor);
		}
	}
	
	sharedMemory.totalRequests--;
	
	/* prepare work */
	//int subSequenceLen = sharedMemory.sequenceLen / sharedMemory.maxRequests;
	
	sharedMemory.workAvailable = true;
	sharedMemory.curRequests++;
	
	/* give work */
	if ((statusDistributor = pthread_cond_signal(&waitAssignment)) != 0)
	{
		errno = statusDistributor;                            									/* save error in errno */
		perror ("error on signaling waitAssignment");
		statusDistributor = EXIT_FAILURE;
		pthread_exit (&statusDistributor);
	}
	
	/* if all iteration requests were completed wait for all work to be done and proceed to next iteration */
	if (sharedMemory.curRequests == sharedMemory.maxRequests)
	{
		//sharedMemory.workNeeded = false;
		
		while (sharedMemory.completeRequests != sharedMemory.maxRequests)						/* wait while the current iteration work has not been completed */
		{
			if ((statusDistributor = pthread_cond_wait(&waitCompletion, &accessCR)) != 0)
			{
				errno = statusDistributor;                            							/* save error in errno */
				perror ("error on waiting in waitCompletion");
				statusDistributor = EXIT_FAILURE;
				pthread_exit (&statusDistributor);
			}
		}
		
		sharedMemory.maxRequests /= 2;
		sharedMemory.curRequests = 0;
		sharedMemory.completeRequests = 0;
		//sharedMemory.workNeeded = true;
		
		printf("[Distributor] waiting for work completion\n");
	}
	
	/* check if all work is done */
	if (sharedMemory.maxRequests < 1)
	{
		sharedMemory.workNeeded = false;
		
		/* inform there is no more work */
		if ((statusDistributor = pthread_cond_signal(&waitAssignment)) != 0)
		{
			errno = statusDistributor;                            									/* save error in errno */
			perror ("error on signaling waitAssignment");
			statusDistributor = EXIT_FAILURE;
			pthread_exit (&statusDistributor);
		}
		
		if ((statusDistributor = pthread_mutex_unlock(&accessCR)) != 0)								/* exit monitor */
		{
			errno = statusDistributor;																/* save error in errno */
			perror("error on exiting monitor(CF)");
			statusDistributor = EXIT_FAILURE;
			pthread_exit(&statusDistributor);
		}
		
		return false;
	}
	
	if ((statusDistributor = pthread_mutex_unlock(&accessCR)) != 0)								/* exit monitor */
	{
		errno = statusDistributor;																/* save error in errno */
		perror("error on exiting monitor(CF)");
		statusDistributor = EXIT_FAILURE;
		pthread_exit(&statusDistributor);
	}
	
	return true;
}

/**
 *  \brief Request integer sequence to sort.
 *
 *  Operation carried out by worker
 *
 *  \return reference to the integer sequence
 */

int* requestWork(int workerId, int* subSequenceLen, int* startOffset, int* endOffset, int* workLeft)
{
	if ((statusWorkers[workerId] = pthread_mutex_lock(&accessCR)) != 0)								/* enter monitor */
	{
		errno = statusWorkers[workerId];															/* save error in errno */
		perror("error on entering monitor(CF)");
		statusWorkers[workerId] = EXIT_FAILURE;
		pthread_exit(&statusWorkers[workerId]);
	}
	pthread_once(&init, initialization);                                       						/* internal data initialization */
	
	/* request work */
	sharedMemory.totalRequests++;
	if ((statusWorkers[workerId] = pthread_cond_signal(&waitRequest)) != 0)
	{
		errno = statusWorkers[workerId];                         									/* save error in errno */
		perror ("error on signaling waitRequest");
		statusWorkers[workerId] = EXIT_FAILURE;
		pthread_exit(&statusWorkers[workerId]);
	}
	
	while (!sharedMemory.workAvailable)																/* wait while there is no work available */
	{
		/* check if all work is done */
		if (!sharedMemory.workNeeded)
		{
			if ((statusDistributor = pthread_cond_signal(&waitAssignment)) != 0)
			{
				errno = statusDistributor;                            								/* save error in errno */
				perror ("error on signaling waitAssignment");
				statusDistributor = EXIT_FAILURE;
				pthread_exit (&statusDistributor);
			}
			
			if ((statusWorkers[workerId] = pthread_mutex_unlock(&accessCR)) != 0)					/* exit monitor */
			{
				errno = statusWorkers[workerId];													/* save error in errno */
				perror("error on exiting monitor(CF)");
				statusWorkers[workerId] = EXIT_FAILURE;
				pthread_exit(&statusWorkers[workerId]);
			}
			
			*workLeft = 0;
			return sharedMemory.integerSequence;
		}
		
		if ((statusWorkers[workerId] = pthread_cond_wait(&waitAssignment, &accessCR)) != 0)
		{
			errno = statusWorkers[workerId];                            							/* save error in errno */
			perror ("error on waiting in waitAssignment");
			statusWorkers[workerId] = EXIT_FAILURE;
			pthread_exit (&statusWorkers[workerId]);
		}
	}
	
	/* get work */
	*subSequenceLen = sharedMemory.sequenceLen / sharedMemory.maxRequests;
	*startOffset = (sharedMemory.curRequests - 1) * (*subSequenceLen);
	*endOffset = sharedMemory.curRequests * (*subSequenceLen) - 1;
	
	sharedMemory.workAvailable = false;
	
	if ((statusWorkers[workerId] = pthread_mutex_unlock(&accessCR)) != 0)							/* exit monitor */
	{
		errno = statusWorkers[workerId];															/* save error in errno */
		perror("error on exiting monitor(CF)");
		statusWorkers[workerId] = EXIT_FAILURE;
		pthread_exit(&statusWorkers[workerId]);
	}
	
	*workLeft = 1;
	return sharedMemory.integerSequence;
}

/**
 *  \brief Inform that the assigned sequence is sorted.
 *
 *  Operation carried out by worker
 *
 *  \param workerId id of the worker informing the work is done
 */

void informWork(int workerId)
{
	if ((statusWorkers[workerId] = pthread_mutex_lock(&accessCR)) != 0)								/* enter monitor */
	{
		errno = statusWorkers[workerId];															/* save error in errno */
		perror("error on entering monitor(CF)");
		statusWorkers[workerId] = EXIT_FAILURE;
		pthread_exit(&statusWorkers[workerId]);
	}
	pthread_once(&init, initialization);                                       						/* internal data initialization */
	
	/* inform work done */
	sharedMemory.completeRequests++;
	if ((statusWorkers[workerId] = pthread_cond_signal(&waitCompletion)) != 0)
	{
		errno = statusWorkers[workerId];                         									/* save error in errno */
		perror ("error on signaling waitCompletion");
		statusWorkers[workerId] = EXIT_FAILURE;
		pthread_exit(&statusWorkers[workerId]);
	}
	
	if ((statusWorkers[workerId] = pthread_mutex_unlock(&accessCR)) != 0)							/* exit monitor */
	{
		errno = statusWorkers[workerId];															/* save error in errno */
		perror("error on exiting monitor(CF)");
		statusWorkers[workerId] = EXIT_FAILURE;
		pthread_exit(&statusWorkers[workerId]);
	}
}