/**
 *  \file consts.h (interface file)
 *
 *  \brief Problem name: Problem name: Sorting Integer Sequence.
 *
 *  Problem simulation constants.
 *
 *  \author Author Name - Month Year
 */

#ifndef CONSTS_H_
#define CONSTS_H_

/* Generic parameters */

/** \brief subsequence length at which the initial work will start (must be power of 2) */
#define MIN_SUBLEN 32

/** \brief shared region structure */
struct SharedMemory {
	char* fileName;
	int* integerSequence;
	FILE* filePointer;
	int sequenceLen;

	int maxRequests;
	int curRequests;
	int totalRequests;
	int completeRequests;
	
	bool workAvailable;
	bool workNeeded;
};

#endif /* CONSTS_H_ */
