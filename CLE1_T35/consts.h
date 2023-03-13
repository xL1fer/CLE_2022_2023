/**
 *  \file consts.h (interface file)
 *
 *  \brief Problem name: Count Portuguese Words.
 *
 *  Problem simulation constants.
 *
 *  \author Author Name - Month Year
 */

#ifndef CONSTS_H_
#define CONSTS_H_

/* Generic parameters */

/** \brief chunck size to be read */
#define  MAX_CHUNK_SIZE           4096

/** \brief file results structure */
struct FileResults {
	int nWords;
	int vowels[6];
	char* fileName;
};

/** \brief shared region structure */
struct SharedMemory {
	struct FileResults* fileResults;
	char* fileNames[];
	int fileId;
	int chunkSize;
	FILE* currentFile;
};

#endif /* CONSTS_H_ */
