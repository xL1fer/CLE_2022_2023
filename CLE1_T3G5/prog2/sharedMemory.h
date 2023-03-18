/**
 *  \file sortingSequence.h (interface file)
 *
 *  \brief Problem name: Problem name: Sorting Integer Sequence.
 *
 *  Synchronization based on monitors.
 *  Threads and the monitor are implemented using the pthread library which enables the creation of a
 *  monitor of the Lampson / Redell type.
 *
 *  Critical region implemented as a monitor.
 *
 *  Definition of the operations carried out by main and the workers:
 *     \li 
 *     \li 
 *     \li 
 *     \li .
 *
 *  \author Author Name - Month Year
 */

#ifndef SHAREDMEMORY_H
#define SHAREDMEMORY_H

extern void fillFileName(char* fileName);

extern void readIntegerSequence();

extern bool assignWork();

extern int* requestWork(int workerId, int integerSequence[], int* subSequenceLen, int* startOffset, int* endOffset, int* workLeft);

extern void informWork(int workerId);

extern void validateArray();

#endif /* SHAREDMEMORY_H */
