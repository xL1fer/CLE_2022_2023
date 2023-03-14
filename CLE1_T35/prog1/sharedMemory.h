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

extern void fillSharedMem(char** fileNames);

/**
 *  \brief Print file results.
 *
 *  Operation carried by main.
 */

extern void printResults(void);
 
/**
 *  \brief Request a chunk of text from the current file.
 *
 *  Operation carried out by the worker.
 *
 *  \param workerId woker id
 *  \param buffer buffer to store the text chunk
 *  \param fileId file identifier
 *  \param chunkSize size of the requested chunk
 *
 *	\return false if all files were parsed
 */

extern bool requestChunk(int workerId, unsigned char* buffer, int* fileId, int* chunkSize);

/**
 *  \brief Save processed results in shared memory.
 *
 *  Operation carried by the worker.
 *
 *  \param workerId worker id
 *  \param fileResult file result structure
 *  \param fileId file identifier
 */

extern void postResults(int workerId, struct FileResult fileResult, int* fileId);

#endif /* SHAREDMEMORY_H */
