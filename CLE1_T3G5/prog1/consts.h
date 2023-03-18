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

/** \brief "a" character offset */
#define A 0

/** \brief "e" character offset */
#define E 1

/** \brief "i" character offset */
#define I 2

/** \brief "o" character offset */
#define O 3

/** \brief "u" character offset */
#define U 4

/** \brief "y" character offset */
#define Y 5

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
