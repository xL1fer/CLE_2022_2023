/**
 *  \file sharedMemory.h (interface file)
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
 *     \li fillSharedMem
 *     \li printResults
 *     \li requestChunk
 *     \li postResults.
 *
 *  \author Author Name - Month Year
 */

#ifndef SHAREDMEMORY_H
#define SHAREDMEMORY_H

/**
 *  \brief Fill shared region.
 *
 *  Operation carried out by main.
 *
 *  \param fileNames array of file names to be proceced
 */

extern void fillSharedMem(char* fileNames[]);

/**
 *  \brief Print file results.
 *
 *  Operation carried by main.
 */

extern void printResults();

/**
 *  \brief Request a text chunk to process.
 *
 *  Operation carried out by the worker.
 *
 *  \param buffer text buffer
 *  \param fileId file identification
 *  \param chunkSize chunk length
 *
 *  \return true when got chunk
 */

extern bool requestChunk(char* buffer, int fileId, int* chunkSize);

/**
 *  \brief Save processed results in shared memory.
 *
 *  Operation carried by the worker.
 *
 *  \param fileResult file result structure
 *  \param fileId file identification
 */

extern void postResults(struct FileResults fileResult, int fileId);

#endif /* SHAREDMEMORY_H */
