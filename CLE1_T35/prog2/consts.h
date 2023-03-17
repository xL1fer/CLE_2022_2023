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
