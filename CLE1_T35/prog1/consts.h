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
struct FileResult {
	int nWords;
	int vowels[6];
	char* fileName;
};

/** \brief shared region structure */
struct SharedMemory {
	struct FileResult* fileResults;
	char** fileNames;
	int fileId;
	int totalFiles;
	int chunkSize;
	bool openFile;
	FILE* currentFile;
};

#endif /* CONSTS_H_ */
