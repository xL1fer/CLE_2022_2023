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
	int* sortedSequence;
	FILE* filePointer;
};

#endif /* CONSTS_H_ */
