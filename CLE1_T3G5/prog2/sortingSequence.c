/**
 *  \file countWords.h (implementation file)
 *
 *  \brief Problem name: Sorting Integer Sequence.
 *
 *  Synchronization based on monitors.
 *  Threads and the monitor are implemented using the pthread library which enables the creation of a
 *  monitor of the Lampson / Redell type.
 *
 *  Generator thread of the intervening entities.
 *
 *  \author Author Name - Month Year
 */
 
//	compile command
// 		gcc -Wall -O3 -o sortingSequence sortingSequence.c sharedMemory.c -lpthread -lm

//	run command
// 		./sortingSequence datSeq32.bin
 
#include <stdio.h>
#include <stdlib.h>
#include <libgen.h>
#include <unistd.h>
#include <stdbool.h>
#include <pthread.h>
#include <math.h>
#include <time.h>
#include <ctype.h>

#include "consts.h"
#include "sharedMemory.h"

#define nWorkers 4

/** \brief return status on monitor initialization */
int statusInitMon;

/** \brief main thread return status */
int statusMain;

/** \brief distributor thread return status */
int statusDistributor;

/** \brief worker threads return status array */
int *statusWorkers;

/** \brief distributor life cycle routine */
static void *distributor(void *id);

/** \brief worker life cycle routine */
static void *worker(void *id);

/** \brief execution time measurement */
static double get_delta_time(void);

/** \brief bitonic sort a sequence of integers */
static void sortSequence(int* integerSequence, int* subSequenceLen, int* startOffset, int* endOffset);

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
	// no file name was provided
	if (argc < 2)
	{
		fprintf(stderr, "no file name provided\n");
		exit(EXIT_FAILURE);
	}
	
	//int nWorkers = 4;
	
	if ((statusWorkers = malloc (nWorkers * sizeof (int))) == NULL)
	{
		fprintf(stderr, "error on allocating space to the return status arrays of producer / consumer threads\n");
		exit(EXIT_FAILURE);
	}
	
	pthread_t *tIdWorkers;			/* workers internal thread id array */
	pthread_t tIdDistributor;		/* distributor internal thread id thread */
	unsigned int *workers;			/* workers application defined thread id array */
	int *pStatus;					/* pointer to execution status */

	/* initializing the application defined thread id arrays for the workers */
	if (((tIdWorkers = malloc (nWorkers * sizeof (pthread_t))) == NULL) ||
		((workers = malloc (nWorkers * sizeof (unsigned int))) == NULL))
	{
		fprintf(stderr, "error on allocating space to both internal / external worker id arrays\n");
		exit(EXIT_FAILURE);
	}
	
	for (int i = 0; i < nWorkers; i++)
		workers[i] = i;
	
	(void) get_delta_time();
	
	/* fill shared memory with file name */
	fillFileName(argv[1]);

	/* generation of intervening entity threads */
	if (pthread_create(&tIdDistributor, NULL, distributor, 0) != 0)	/* thread distributor */
	{
		perror("error on creating thread distributor");
		exit(EXIT_FAILURE);
	}
	
	for (int i = 0; i < nWorkers; i++)
	{
		if (pthread_create(&tIdWorkers[i], NULL, worker, &workers[i]) != 0)		/* thread worker */
		{
			perror("error on creating thread worker");
			exit(EXIT_FAILURE);
		}
	}

	/* waiting for the termination of the intervening entity threads */
	if (pthread_join(tIdDistributor, (void *) &pStatus) != 0)					/* thread distributor */
	{
		perror("error on waiting for thread distributor");
		exit(EXIT_FAILURE);
	}
	printf("thread distributor has terminated: ");
	printf("its status was %d\n", *pStatus);
	
	for (int i = 0; i < nWorkers; i++)
	{
		if (pthread_join(tIdWorkers[i], (void *) &pStatus) != 0)				/* thread worker */
		{
			perror("error on waiting for thread worker");
			exit(EXIT_FAILURE);
		}
		printf("thread worker, with id %u, has terminated: ", i);
		printf("its status was %d\n", *pStatus);
	}
	
	/* validate sorted sequence */
	validateArray();
	
	printf("\nElapsed time = %.6f s\n", get_delta_time());

	exit(EXIT_SUCCESS);
}

/**
 *  \brief Distributor function.
 *
 *  Its role is to simulate the life cycle of a worker.
 *
 *  \param par pointer to application defined worker identification
 */

static void *distributor(void *par)
{
	//unsigned int id = *((unsigned int *) par);									/* distributor id */
	
	readIntegerSequence();
	while (assignWork());

	statusDistributor = EXIT_SUCCESS;
	pthread_exit(&statusDistributor);
}

/**
 *  \brief Woker function.
 *
 *  Its role is to simulate the life cycle of a worker.
 *
 *  \param par pointer to application defined worker identification
 */

static void *worker(void *par)
{
	unsigned int id = *((unsigned int *) par);									/* worker id */
	
	int* integerSequence = 0;
	int subSequenceLen = 0;
	int startOffset = 0;
	int endOffset = 0;
	int workLeft = 0;

	while ((integerSequence = requestWork(id, &subSequenceLen, &startOffset, &endOffset, &workLeft)))
	{
		if (workLeft == 0)
			break;
		//printf("thread %d got work! subSequenceLen = %d; startOffset = %d; endOffset = %d\n", id, subSequenceLen, startOffset, endOffset);
		
		//printf("[Worker %d] working!\n", id);
		sortSequence(integerSequence, &subSequenceLen, &startOffset, &endOffset);
		informWork(id);
	}

	statusWorkers[id] = EXIT_SUCCESS;
	pthread_exit(&statusWorkers[id]);
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
	if(clock_gettime(CLOCK_MONOTONIC, &t1) != 0)
	{
		perror("clock_gettime");
		exit(1);
	}
	return (double) (t1.tv_sec - t0.tv_sec) + 1.0e-9 * (double) (t1.tv_nsec - t0.tv_nsec);
}

/**
 *  \brief Bitonic sort a sequence of integers.
 *
 *
 *	Operation carried out by worker.
 *
 *	Addapted from https://en.wikipedia.org/wiki/Bitonic_sorter
 *
 *  \param integerSequence sequence to be sorted
 *  \param subSequenceLen length of the sequence
 *  \param startOffset sequence starting offset
 *  \param endOffset sequence ending offset
 */

static void sortSequence(int* integerSequence, int* subSequenceLen, int* startOffset, int* endOffset)
{
	if (*subSequenceLen == MIN_SUBLEN)
	{
		for (int k = 2; k <= *subSequenceLen; k *= 2) // k is doubled every iteration
		{
			for (int j = k / 2; j > 0; j /= 2) // j is halved at every iteration, with truncation of fractional parts
			{
				for (int i = *startOffset; i < *endOffset; i++)
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
	else
	{
		int k = *subSequenceLen;
		
		for (int j = k / 2; j > 0; j /= 2) // j is halved at every iteration, with truncation of fractional parts
		{
			for (int i = *startOffset; i < *endOffset; i++)
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