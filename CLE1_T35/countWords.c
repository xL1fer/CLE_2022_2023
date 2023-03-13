/**
 *  \file countWords.h (implementation file)
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
 *     \li 
 *     \li .
 *
 *  \author Author Name - Month Year
 */
 
#include <stdio.h>
#include <stdlib.h>
#include <libgen.h>
#include <unistd.h>
#include <stdbool.h>
#include <pthread.h>
#include <math.h>

#include "consts.h"
#include "sharedMemory.h"

/** \brief return status on monitor initialization */
int statusInitMon;

/** \brief worker life cycle routine */
static void *worker(void *id);

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

int main (int argc, char *argv[])
{
	int nThreads = 5;
	
	pthread_t *tIdWorkers;                                               /* workers internal thread id array */
	unsigned int *workers;                                               /* workers application defined thread id array */
	int *pStatus;                                                        /* pointer to execution status */

	/* initializing the application defined thread id arrays for the workers */

	if (((tIdWorkers = malloc (nThreads * sizeof (pthread_t))) == NULL) ||
		((workers = malloc (nThreads * sizeof (unsigned int))) == NULL))
	{
		fprintf (stderr, "error on allocating space to both internal / external worker id arrays\n");
		exit (EXIT_FAILURE);
	}
	
	for (int i = 0; i < nThreads; i++)
		workers[i] = i;

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
		if (pthread_join(tIdWorkers[i], (void *) &pStatus) != 0)			/* thread worker */
		{
			perror ("error on waiting for thread worker");
			exit (EXIT_FAILURE);
		}
		printf ("thread workers, with id %u, has terminated: ", i);
		printf ("its status was %d\n", *pStatus);
	}
	
	/* print obtained results */
	printResults();

  exit (EXIT_SUCCESS);
}

/**
 *  \brief Function worker.
 *
 *  Its role is to simulate the life cycle of a worker.
 *
 *  \param par pointer to application defined worker identification
 */

static void *worker (void *par)
{
	unsigned int id = *((unsigned int *) par),                                          /* worker id */

	while (requestChunk())
	{
		processChunk();
		postResults();
	}

	pthread_exit (&statusCons[id]);
}