/**
 *  \file countWords.h (implementation file)
 *
 *  \brief Problem name: Problem name: Sorting Integer Sequence.
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

/** \brief return status on monitor initialization */
int statusInitMon;

/** \brief main thread return status */
int statusMain;

/** \brief distributor thread return status */
int statusDistributor;

/** \brief worker threads return status array */
int *statusWorkers;

/** \brief worker life cycle routine */
static void *worker(void *id);

/** \brief execution time measurement */
static double get_delta_time(void);

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
	int nWorkers = 4;
	
	if ((statusWorkers = malloc (nWorkers * sizeof (int))) == NULL)
	{
		fprintf(stderr, "error on allocating space to the return status arrays of producer / consumer threads\n");
		exit(EXIT_FAILURE);
	}
	
	pthread_t *tIdWorkers;			/* workers internal thread id array */
	pthread_t tIdDistributor;		/* distributor internal thread id thread */
	unsigned int *workers;			/* workers application defined thread id array */
	unsigned int distributor;		/* distributor application defined thread id */
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
	
	distributor = 0;
	
	(void) get_delta_time();
	
	/* fill shared memory with file name */
	fillSharedMem("test");

	/* generation of intervening entity threads */
	if (pthread_create(&tIdDistributor, NULL, worker, &distributor) != 0)		/* thread distributor */
	{
		perror ("error on creating thread distributor");
		exit (EXIT_FAILURE);
	}
	
	for (int i = 0; i < nWorkers; i++)
	{
		if (pthread_create(&tIdWorkers[i], NULL, worker, &workers[i]) != 0)		/* thread worker */
		{
			perror ("error on creating thread worker");
			exit (EXIT_FAILURE);
		}
	}

	/* waiting for the termination of the intervening entity threads */
	if (pthread_join(tIdDistributor, (void *) &pStatus) != 0)					/* thread distributor */
	{
		perror("error on waiting for thread distributor");
		exit (EXIT_FAILURE);
	}
	printf("thread distributor has terminated: ");
	printf("its status was %d\n", *pStatus);
	
	for (int i = 0; i < nWorkers; i++)
	{
		if (pthread_join(tIdWorkers[i], (void *) &pStatus) != 0)				/* thread worker */
		{
			perror("error on waiting for thread worker");
			exit (EXIT_FAILURE);
		}
		printf("thread workers, with id %u, has terminated: ", i);
		printf("its status was %d\n", *pStatus);
	}
	
	printf ("\nElapsed time = %.6f s\n", get_delta_time());

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
	unsigned int id = *((unsigned int *) par);									/* worker id */

	/*
	while (true)
	{

	}
	*/

	statusWorkers[id] = EXIT_SUCCESS;
	pthread_exit(&statusWorkers[id]);
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

	/*
	while (true)
	{

	}
	*/

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

