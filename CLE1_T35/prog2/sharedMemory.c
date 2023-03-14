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
 *  Definition of the operations carried out by main and the workers:
 *     \li fillSharedMem
 *     \li 
 *     \li 
 *     \li .
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

/** \brief distributor synchronization point when the data transfer region is full */
static pthread_cond_t fifoFull;

/** \brief consumers synchronization point when the data transfer region is empty */
static pthread_cond_t fifoEmpty;

/** \brief consumers synchronization point when the data transfer region is empty */
static pthread_cond_t fifoEmpty;

/**
 *  \brief Initialization of the shared region.
 *
 *  Internal monitor operation.
 */

static void initialization(void)
{
	
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
	
	
	printf("Shared memory filled!\n");
	
	if ((statusMain = pthread_mutex_unlock (&accessCR)) != 0)						/* exit monitor */
	{
		errno = statusMain;															/* save error in errno */
		perror("error on exiting monitor(CF)");
		statusMain = EXIT_FAILURE;
		pthread_exit(&statusMain);
	}
}
