/**
 *  \file sortingSequence.h (interface file)
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

#ifndef SHAREDMEMORY_H
#define SHAREDMEMORY_H

/**
 *  \brief Fill file name.
 *
 *  Operation carried out by main
 *
 *  \param fileNames array of file names to be proceced
 */

extern void fillFileName(char* fileName);

/**
 *  \brief Verify if the integer sequence is sorted.
 *
 *  Operation carried out by main
 */

extern void validateArray(void);

/**
 *  \brief Read binary file integer sequence.
 *
 *  Operation carried out by distributor
 *
 *  \param fileNames array of file names to be proceced
 */

extern void readIntegerSequence(void);

/**
 *  \brief Read binary file integer sequence.
 *
 *  Operation carried out by distributor
 *
 *  \return true if there is more work to be done
 */

extern bool assignWork(void);

/**
 *  \brief Request integer sequence to sort.
 *
 *  Operation carried out by worker
 *
 *  \return reference to the integer sequence
 */

extern int* requestWork(int workerId, int* subSequenceLen, int* startOffset, int* endOffset, int* workLeft);

/**
 *  \brief Inform that the assigned sequence is sorted.
 *
 *  Operation carried out by worker
 *
 *  \param workerId id of the worker informing the work is done
 */

extern void informWork(int workerId);

#endif /* SHAREDMEMORY_H */
