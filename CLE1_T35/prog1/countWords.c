/**
 *  \file countWords.h (implementation file)
 *
 *  \brief Problem name: Count Portuguese Words.
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
// 		gcc -Wall -O3 -o countWords countWords.c sharedMemory.c -lpthread -lm

//	run command
// 		./countWords text0.txt text1.txt text2.txt text3.txt text4.txt
 
#include <stdio.h>
#include <stdlib.h>
#include <libgen.h>
#include <unistd.h>
#include <stdbool.h>
#include <pthread.h>
#include <math.h>
#include <time.h>

#include "consts.h"
#include "sharedMemory.h"

/** \brief return status on monitor initialization */
int statusInitMon;

/** \brief worker threads return status array */
int *statusWorkers;

/** \brief main thread return status */
int statusMain;

/** \brief worker life cycle routine */
static void *worker(void *id);

/** \brief execution time measurement */
static double get_delta_time(void);

/** \brief process chunk of text */
static struct FileResult processChunk();

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
	int nThreads = 5;
	
	if ((statusWorkers = malloc (nThreads * sizeof (int))) == NULL)
	{
		fprintf(stderr, "error on allocating space to the return status arrays of producer / consumer threads\n");
		exit(EXIT_FAILURE);
	}
	
	pthread_t *tIdWorkers;			/* workers internal thread id array */
	unsigned int *workers;			/* workers application defined thread id array */
	int *pStatus;					/* pointer to execution status */

	/* initializing the application defined thread id arrays for the workers */
	if (((tIdWorkers = malloc (nThreads * sizeof (pthread_t))) == NULL) ||
		((workers = malloc (nThreads * sizeof (unsigned int))) == NULL))
	{
		fprintf(stderr, "error on allocating space to both internal / external worker id arrays\n");
		exit(EXIT_FAILURE);
	}
	
	for (int i = 0; i < nThreads; i++)
		workers[i] = i;
	
	(void) get_delta_time();
	
	/* fill shared memory with files names */
	fillSharedMem(argv);

	/* generation of intervening entities threads */
	for (int i = 0; i < nThreads; i++)
	{
		if (pthread_create(&tIdWorkers[i], NULL, worker, &workers[i]) != 0)		/* thread worker */
		{
			perror ("error on creating thread worker");
			exit (EXIT_FAILURE);
		}
	}

	/* waiting for the termination of the intervening entity threads */
	for (int i = 0; i < nThreads; i++)
	{
		if (pthread_join(tIdWorkers[i], (void *) &pStatus) != 0)				/* thread worker */
		{
			perror("error on waiting for thread worker");
			exit (EXIT_FAILURE);
		}
		printf("thread workers, with id %u, has terminated: ", i);
		printf("its status was %d\n", *pStatus);
	}
	
	/* print obtained results */
	printResults();
	
	printf ("\nElapsed time = %.6f s\n", get_delta_time());

	exit(EXIT_SUCCESS);
}

/**
 *  \brief Function worker.
 *
 *  Its role is to simulate the life cycle of a worker.
 *
 *  \param par pointer to application defined worker identification
 */

static void *worker(void *par)
{
	unsigned int id = *((unsigned int *) par);                                          /* worker id */
	
	unsigned char buffer[MAX_CHUNK_SIZE] = "";
	int fileId = 0;
	int chunkSize = 0;
	struct FileResult fileResult;

	while (requestChunk(id, buffer, &fileId, &chunkSize))
	{
		fileResult = processChunk();
		postResults(id, fileResult, &fileId);
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
	if(clock_gettime (CLOCK_MONOTONIC, &t1) != 0)
	{
		perror ("clock_gettime");
		exit(1);
	}
	return (double) (t1.tv_sec - t0.tv_sec) + 1.0e-9 * (double) (t1.tv_nsec - t0.tv_nsec);
}

/**
 *  \brief xxxxxxxxxxxxxxxx.
 *
 *  \return xxxxxxxxxxxxxxx
 */

static struct FileResult processChunk()
{
	struct FileResult fileResult;
	fileResult.nWords = 0;
	
	return fileResult;
}