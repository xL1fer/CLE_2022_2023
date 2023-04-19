/**
 *  \file countWords.c (implementation file)
 *
 *  \brief Problem name: Count Portuguese Words.
 *
 *  Synchronization based MPI (Message Passing Interface).
 *
 *  \author Author Name - Month Year
 */
 
//	compile command
// 		mpicc -Wall -O3 -o countWords countWords.c -lm

//	run command
// 		mpiexec -n 10 ./countWords text0.txt text1.txt text2.txt text3.txt text4.txt
 
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdbool.h>
#include <math.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

#include "consts.h"

// general definitions
#define WORKTODO       1
#define NOMOREWORK     0

#define FILECONTINUE	0
#define FILECOMPLETE	1
#define FILEERROR	    2

//#define DEBUG

// internal functions declaration
static double get_delta_time(void);
static int parseCommandLine(char** commandLineArgs, int* totalFiles, char*** fileNames, struct FileResult** fileResults);
static int getFileChunk(struct ChunkData** chunkData, FILE** currentFile, int* fileId);
static bool isSeparator(int c);
static void printResults(int totalFiles, struct FileResult* fileResults, char** fileNames);

static struct FileResult processChunk(unsigned char* buffer, int chunkSize);
static void setToZero(int array[6]);
static int extractAChar(unsigned char* buffer, int* curPos, int* chunkSize, unsigned char UTF8Char[5]);
static int isVowel(int c);
static void processAChar(int c, int* inWord, int* nWords, int nWordswVowel[6], int firstOccur[6]);
static int vowelOffset(int c);

/**
 *  \brief Main function.
 *
 *  Instantiation of the processing configuration.
 *
 *  \param argc number of words of the command line
 *  \param argv list of words of the command line
 *
 *  \return status of operation
 */
int main(int argc, char *argv[])
{
	// MPI initializations
	int rank, nProc;
	struct ChunkData *chunkData = NULL;
	struct FileResult *resultData = NULL;
	
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &nProc);
	int save = nProc;
	
	// not enough arguments provided
	if (argc < 2)
	{
		fprintf(stderr, "no file name provided\n");
		MPI_Finalize();
		return EXIT_FAILURE;
	}
	
	if (((chunkData = malloc(sizeof(struct ChunkData))) == NULL))
	{
		fprintf(stderr, "error on allocating space to the chunk data buffer\n");
		MPI_Finalize();
		return EXIT_FAILURE;
	}
	
	if (((resultData = malloc(sizeof(struct FileResult))) == NULL))
	{
		fprintf(stderr, "error on allocating space to the result data buffer\n");
		MPI_Finalize();
		return EXIT_FAILURE;
	}
	
	// dispatcher process
	if (rank == 0)
	{
		(void) get_delta_time();
		
		int totalFiles = 0;
		char** fileNames = NULL;
		struct FileResult* fileResults = NULL;
		int fileId = 0;
		FILE* currentFile = NULL;
		//int chunkSize = 0;	// NOT NEEDED, ALREADY IN THE "chunkData" STRUCTURE (DELETE THIS LATER)
		//bool openFile = true;
		bool* availWorkers;
		struct FileResult* resultsBuffer;
		MPI_Request reqSnd[nProc], reqRec[nProc];
		
		// parse command line arguments and initialize file names and file results array
		if (parseCommandLine(argv, &totalFiles, &fileNames, &fileResults) == 1)
		{
			MPI_Finalize();
			return EXIT_FAILURE;
		}
		
		// initialize available workers array
		if (((availWorkers = malloc((nProc - 1) * sizeof(bool))) == NULL))
		{
			fprintf(stderr, "error on allocating space to the file names\n");
			MPI_Finalize();
			return EXIT_FAILURE;
		}
		for (int i = 0; i < nProc - 1; i++) availWorkers[i] = true;
		
		// allocate memory for results buffer
		if (((resultsBuffer = malloc(nProc * sizeof(struct FileResult))) == NULL))
		{
			fprintf(stderr, "error on allocating space to the results buffer pointers\n");
			MPI_Finalize();
			return EXIT_FAILURE;
		}
		
		// open first text file
		if ((currentFile = fopen(fileNames[fileId], "r")) == NULL)
		{
			fprintf(stderr, "error on opening text file \"%s\"\n", fileNames[fileId]);
			MPI_Finalize();
			return EXIT_FAILURE;
		}
		
		// while not all files have been parsed
		while (fileId < totalFiles)
		{
			//printf("> %d < %d ?\n", fileId, totalFiles);
			
			// iterate workers
			for (int i = rank + 1; i < nProc && fileId < totalFiles; i++)
			{
				// workers available to receive work
				if (availWorkers[i])
				{
					// get chunk of text
					if (getFileChunk(&chunkData, &currentFile, &fileId) == FILECOMPLETE)
					{
						// check if all files have been parsed
						if (fileId < totalFiles)
						{
							// open next text file
							if ((currentFile = fopen(fileNames[fileId], "r")) == NULL)
							{
								fprintf(stderr, "error on opening text file \"%s\"\n", fileNames[fileId]);
								MPI_Finalize();
								return EXIT_FAILURE;
							}
						}
					}
					// send data chunk
					MPI_Isend(chunkData, sizeof(struct ChunkData), MPI_BYTE, i, 0, MPI_COMM_WORLD, &reqSnd[i]);
					
					// open receiving buffer worker
					MPI_Irecv(&resultsBuffer[i], sizeof(struct FileResult), MPI_BYTE, i, 0, MPI_COMM_WORLD, &reqRec[i]);
					availWorkers[i] = false;
				}
				// workers already with work assigned
				else
				{
					bool hasMessage;
					MPI_Test(&reqRec[i], (int *) &hasMessage, MPI_STATUS_IGNORE);
					// worker delivered results
					if (hasMessage)
					{
						struct FileResult* result = &resultsBuffer[i];
						printf("Receiving result of file %d in box %d\n", result->fileId, i);
						
						// save results
						fileResults[result->fileId].nWords += result->nWords;
						for (int j = 0; j < 6; j++)
							fileResults[result->fileId].vowels[j] += result->vowels[j];
						
						availWorkers[i] = true;
					}
					nProc = save;	// NOTE(L1fer): WHY THE FUCK IS "nProc" BEING RESETED ?????????????????????
				}
			}
			
			/*
			// open receiving buffer for each worker that does not have it opened yet
			for (int i = rank + 1; i < nProc; i++)
			{
				if (availWorkers[i])
				{
					MPI_Irecv(&resultsBuffer[i], sizeof(struct FileResult), MPI_BYTE, i, 0, MPI_COMM_WORLD, &reqRec[i]);
					availWorkers[i] = false;
				}
			}
			
			// test if there are any message from workers
			for (int i = rank + 1; i < nProc; i++)
			{
				if (!availWorkers[i])
				{
					bool hasMessage;
					MPI_Test(&reqRec[i], (int *) &hasMessage, MPI_STATUS_IGNORE);
					if (hasMessage)
					{
						//printf ("Received a result!");
						availWorkers[i] = true;
					}
				}
				nProc = save;	// NOTE(L1fer): WHY THE FUCK IS "nProc" BEING RESETED ?????????????????????
			}
			*/
		}
		
		// get remaining results and send message for workers to terminate
		for (int i = rank + 1; i < nProc; i++)
		{
			if (!availWorkers[i])
			{
				MPI_Wait(&reqRec[i], MPI_STATUS_IGNORE);
				
				struct FileResult* result = &resultsBuffer[i];
				
				// save results
				fileResults[result->fileId].nWords += result->nWords;
				for (int j = 0; j < 6; j++)
					fileResults[result->fileId].vowels[j] += result->vowels[j];
				
				nProc = save;
			}
			
			chunkData->hasWork = NOMOREWORK;
			MPI_Send(chunkData, sizeof(struct ChunkData), MPI_BYTE, i, 0, MPI_COMM_WORLD);
		}
		
		// print final results
		printResults(totalFiles, fileResults, fileNames);
		
		// print total time
		printf("\nElapsed time = %.6f s\n", get_delta_time());
	}
	// workers processes
	else
	{
		while (true)
		{
			// wait for work
			MPI_Recv(chunkData, sizeof(struct ChunkData), MPI_BYTE, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			
			if (chunkData->hasWork == NOMOREWORK)
				break;
			
			//printf("< %d\n", chunkData->fileId);
			//printf("%s\n========================================= (%d)\n", chunkData->buffer, rank);
			//printf("DEBUG: %d\n", chunkData->hasWork);
			
			*resultData = processChunk(chunkData->buffer, chunkData->chunkSize);
			resultData->fileId = chunkData->fileId;
			
			//printf("> %d\n", (*resultData).nWords);
			
			printf("%d Sending Results!\n", rank);
			
			// send results
			MPI_Send(resultData, sizeof(struct FileResult), MPI_BYTE, 0, 0, MPI_COMM_WORLD);
			
			printf("%d Results Delivered!\n", rank);
			
			//printf("%d SENT MESSAGE!\n", rank);
		}
	}
	
#ifdef DEBUG
	printf("Process %d finalized\n", rank);
#endif	// DEBUG
	
	MPI_Finalize();
	return EXIT_SUCCESS;
}

/**
 *  \brief Get the process time that has elapsed since last call of this time.
 *
 *  \return process elapsed time
 */
static double get_delta_time(void)
{
	static struct timespec t0, t1;

	t0 = t1;
	if (clock_gettime(CLOCK_MONOTONIC, &t1) != 0)
	{
		perror("clock_gettime");
		exit(1);
	}
	return (double)(t1.tv_sec - t0.tv_sec) + 1.0e-9 * (double)(t1.tv_nsec - t0.tv_nsec);
}

/**
 *  \brief Parse command line arguments.
 *
 *  Operation carried out by the dispatcher process.
 *
 *  \param commandLineArgs command line arguments string array
 *  \param totalFiles total number of files
 *  \param fileNames file names array
 *  \param fileResults file results array
 *
 *	\return function execution status (0 if nothing wrong)
 */
static int parseCommandLine(char** commandLineArgs, int* totalFiles, char*** fileNames, struct FileResult** fileResults)
{
	// initialize files names
	int filesNumber = -1;	// do not count the first argument ("./countWords") neither the number of threads
	char** ptr = commandLineArgs;
	while(*ptr != 0)
	{
		filesNumber++;
		++ptr;
	}
	*totalFiles = filesNumber;
	
	// allocate file names memory
	if (((*fileNames = malloc((filesNumber) * sizeof(char*))) == NULL))
	{
		fprintf(stderr, "error on allocating space to the file names\n");
		return 1;
	}
	// allocate file results memory
	if (((*fileResults = malloc((filesNumber) * sizeof(struct FileResult))) == NULL))
	{
		fprintf(stderr, "error on allocating space to file results\n");
		return 1;
	}
	
	ptr = commandLineArgs;
	int filesOffset = 1;
	for (int i = filesOffset; ptr[i] != 0; i++)
	{
		char* subptr = ptr[i];

		int nameLen;
		for (nameLen = 0; subptr[nameLen] != 0; nameLen++);
		
		// allocate file name memory
		if ((((*fileNames)[i - filesOffset] = malloc((nameLen + 1) * sizeof(char))) == NULL))
		{
			fprintf(stderr, "error on allocating space to file name\n");
			return 1;
		}
		
		// fill file names
		nameLen++;		// include '\0' termination
		for (int j = 0; j < nameLen; j++)
			(*fileNames)[i - filesOffset][j] = subptr[j];
		
		(*fileResults)[i - filesOffset].nWords = 0;
		for (int j = 0; j < 6; j++)
			(*fileResults)[i - filesOffset].vowels[j] = 0;
	}
	
	printf("Memory initialized!\n");
	
	return 0;
}

/**
 *  \brief Get a chunk of text from the current file.
 *
 *  Operation carried out by the dispatcher process.
 *
 *  \param chunkData chunk data structure
 *  \param currentFile current file being parsed
 *  \param fileId file identifier
 *
 *	\return function execution status
 */
static int getFileChunk(struct ChunkData** chunkData, FILE** currentFile, int* fileId)
{
	(*chunkData)->hasWork = WORKTODO;
	
	// save file id
	(*chunkData)->fileId = *fileId;
	
	// read chunk of text
    if (((*chunkData)->chunkSize = fread((*chunkData)->buffer, 1, MAX_CHUNK_SIZE - 1, *currentFile)) == EOF)
	{
		fprintf(stderr, "error on getting file chunk\n");
		return FILEERROR;
	}
	(*chunkData)->buffer[((*chunkData)->chunkSize - 1)] = '\0';
	
	// the file was completely read, point to the next one
	if ((*chunkData)->chunkSize < MAX_CHUNK_SIZE - 1)
	{
		if (fclose(*currentFile) == EOF)
		{
			fprintf(stderr, "error on closing text file\n");
			return FILEERROR;
		}
		(*fileId)++;
		return FILECOMPLETE;
	}
	// the file still has content, check if we are in the middle of a word
	else
	{
		long currentFilePos;
		int c = 0, j = 0, k = 0;
		char cbuffer[4] = "";
		
		// check for uncomplete utf8 character
		for (int i = (*chunkData)->chunkSize - 1; i >= 0; i--)
		{
			// if the character is ascii, we can continue
			if (j == 0 && ((*chunkData)->buffer[i] & 0x80) == 0)
			{
				//printf("ascii char, all good\n");
				break;
			}
			
			// get how many bytes the utf-8 char occupies
			int bytes;
			for (bytes = 1; (*chunkData)->buffer[i] & (0x80 >> bytes); bytes++);
			
			// first utf8 byte, check if the buffer has all its content
			if (bytes > 1)
			{
				// uncomplete utf-8 char, backtrack the buffer
				if (bytes > j)
				{
					//printf("uncomplete utf-8 char, backtracking buffer");
					if ((currentFilePos = ftell(*currentFile)) == -1L)
					{
						fprintf(stderr, "error on telling text file\n");
						return FILEERROR;
					}
					if (fseek(*currentFile, currentFilePos - (j + 1), SEEK_SET) != 0)
					{
						fprintf(stderr, "error on seeking text file\n");
						return FILEERROR;
					}
					(*chunkData)->chunkSize -= (j + 1);
				}
				break;
			}
			j++;
		}
	
		// check for separation character
		j = 0;
		for (int i = (*chunkData)->chunkSize - 1; i >= 0; i--)
		{
			cbuffer[j++] = (*chunkData)->buffer[i];
			
			// character is ascii, check if it is separator
			if (((*chunkData)->buffer[i] & 0x80) == 0)
			{
				j = 0;
				if (isSeparator((*chunkData)->buffer[i]))
				{
					//printf("ascci char is separator\n");
					(*chunkData)->buffer[i] = '\0';
					break;
				}
			}
			
			// get how many bytes the current utf-8 char occupies
			int bytes;
			for (bytes = 1; (*chunkData)->buffer[i] & (0x80 >> bytes); bytes++);
			
			// utf8 char first byte
			if (bytes > 1)
			{
				//printf("checking utf-8 character\n");
				for (int l = bytes - 1; l >= 0; l--)
					c = (c << 8) | cbuffer[l];
				
				if (isSeparator((*chunkData)->buffer[i]))
				{
					//printf("utf-8 char is separator\n");
					(*chunkData)->buffer[i] = '\0';
					break;
				}
				
				c = 0;
			}
			k++;
		}
	
		//printf("1 chunkSize = %d\n", *chunkSize);
		
		if ((currentFilePos = ftell(*currentFile)) == -1L)
		{
			fprintf(stderr, "error on telling text file\n");
			return FILEERROR;
		}
		if (fseek(*currentFile, currentFilePos - (k + 1), SEEK_SET) != 0)
		{
			fprintf(stderr, "error on seeking text file\n");
			return FILEERROR;
		}
		(*chunkData)->chunkSize -= (k + 1);
		
		//printf("2 chunkSize = %d\n", *chunkSize);
	}
	
	return FILECONTINUE;
}

/**
 *  \brief Check if character is seperator.
 *
 *  Auxilar function.
 *
 *  \param c character
 *
 *	\return true if character is separator
 */
static bool isSeparator(int c)
{
	if ((c == 0x20 || c == 0x9 || c == 0xA || c == 0xD) || (c == '-' || c == 0x22 || c == 0xE2809C || c == 0xE2809D || c == '[' || c == ']' || c == '(' || c == ')') || (c == '.' || c == ',' || c == ':' || c == ';' || c == '?' || c == '!' || c == 0xE28093 || c == 0xE280A6))
		return true;
	
	return false;
}

/**
 *  \brief Print final files results.
 *
 *  Operation carried by dispatcher process.
 */
static void printResults(int totalFiles, struct FileResult* fileResults, char** fileNames)
{
	for (int i = 0; i < totalFiles; i++)
	{
		int* vowels = fileResults[i].vowels;
		
		printf("File name: %s\n", fileNames[i]);
		printf("Total number of words = %d\n", fileResults[i].nWords);
		printf("Number of words with an\n");
		printf("\tA\tE\tI\tO\tU\tY\n");
		printf("\t%d\t%d\t%d\t%d\t%d\t%d\n\n", vowels[A], vowels[E], vowels[I], vowels[O], vowels[U], vowels[Y]);
	}
}

/**
 *  \brief Process a text chunk.
 *
 *  Operation carried by worker processes.
 *
 *	\param buffer buffer to be parsed
 *	\param chunkSize valid size of the buffer
 *
 *  \return file result
 */
static struct FileResult processChunk(unsigned char* buffer, int chunkSize)
{
	struct FileResult fileResult;
	fileResult.nWords = 0;
	setToZero(fileResult.vowels);
	
    int inWord = 0;
    int firstOccur[6];
    unsigned char UTF8Char[5];
	int cutf8;
	int curPos = 0;
	
	while ((cutf8 = extractAChar(buffer, &curPos, &chunkSize, UTF8Char)) != EOF)
	{
		processAChar(cutf8, &inWord, &fileResult.nWords, fileResult.vowels, firstOccur);
	}
	
	return fileResult;
}

/**
 *  \brief Set all elements of a given array to 0.
 *
 *	\param array buffer to be initialized
 */
static void setToZero(int array[6])
{
    for (int i = 0; i < 6; i++)
        array[i] = 0;
}

/**
 *  \brief Extract an utf-8 char from a given buffer.
 *
 *	\param buffer to get the char
 *	\param curPos current buffer position
 *	\param chunkSize maximum buffer size
 *	\param UTF8Char buffer to store the char
 *
 *	\return extracted char
 */
static int extractAChar(unsigned char* buffer, int* curPos, int* chunkSize, unsigned char UTF8Char[5])
{
	// buffer ended
	if (*curPos >= *chunkSize)
		return EOF;
	
    int cutf8;
	
	UTF8Char[0] = buffer[*curPos];
	(*curPos)++;

    // ASCII char
    if ((UTF8Char[0] & 0x80) == 0)
    {
        // 0x80 -> 128 decimal value, which is [ 1 0 0 0 0 0 0 0 ] binary value
        // by checking if the first bit is 0, we know that it only occupies 1 byte
        UTF8Char[0] = toupper(UTF8Char[0]);
        UTF8Char[1] = '\0';

        return UTF8Char[0];
    }
    // not the first byte of the UTF-8 char (the second most significant bit must be 1 as well)
    else if ((UTF8Char[0] & 0xC0) == 0x80)   // 0xC0 -> [ 1 1 0 0 0 0 0 0 ]
    {
        return -1;
    }
    // invalid UTF-8 stream (to be a valid UTF-8 char, it must have one zero in position 0, 2, 3 or 4 of the msb)
    else if ((UTF8Char[0] & 0xFE) == 0xFE)   // 0xFE -> [ 1 1 1 1 1 1 1 0 ]
    {
        return -1;
    }
    // UTF-8 char
    else
    {
        // get how many bytes the utf-8 char occupies
        int bytes;
        for (bytes = 1; UTF8Char[0] & (0x80 >> bytes); bytes++);

        // get remaining char bytes
        cutf8 = UTF8Char[0];
        for (int i = 1; i < bytes; i++)
        {
			UTF8Char[(i * sizeof(char))] = buffer[*curPos];
			(*curPos)++;
			
            cutf8 = (cutf8 << 8) | UTF8Char[i];
        }

        UTF8Char[bytes] = '\0';

        if (cutf8 == 0xC3A1 || cutf8 == 0xC381)
        {
            UTF8Char[0] = 'A';
            UTF8Char[1] = '\0';
            return 'A';
        }
        else if (cutf8 == 0xC3A0 || cutf8 == 0xC380)
        {
            UTF8Char[0] = 'A';
            UTF8Char[1] = '\0';
            return 'A';
        }
        else if (cutf8 == 0xC3A2 || cutf8 == 0xC382)
        {
            UTF8Char[0] = 'A';
            UTF8Char[1] = '\0';
            return 'A';
        }
        else if (cutf8 == 0xC3A3 || cutf8 == 0xC383)
        {
            UTF8Char[0] = 'A';
            UTF8Char[1] = '\0';
            return 'A';
        }
        else if (cutf8 == 0xC3A9 || cutf8 == 0xC389)
        {
            UTF8Char[0] = 'E';
            UTF8Char[1] = '\0';
            return 'E';
        }
        else if (cutf8 == 0xC3A8 || cutf8 == 0xC388)
        {
            UTF8Char[0] = 'E';
            UTF8Char[1] = '\0';
            return 'E';
        }
        else if (cutf8 == 0xC3AA || cutf8 == 0xC38A)
        {
            UTF8Char[0] = 'E';
            UTF8Char[1] = '\0';
            return 'E';
        }
        else if (cutf8 == 0xC3AD || cutf8 == 0xC38D)
        {
            UTF8Char[0] = 'I';
            UTF8Char[1] = '\0';
            return 'I';
        }
        else if (cutf8 == 0xC3AC || cutf8 == 0xC38C)
        {
            UTF8Char[0] = 'I';
            UTF8Char[1] = '\0';
            return 'I';
        }
        else if (cutf8 == 0xC3B3 || cutf8 == 0xC393)
        {
            UTF8Char[0] = 'O';
            UTF8Char[1] = '\0';
            return 'O';
        }
        else if (cutf8 == 0xC3B2 || cutf8 == 0xC392)
        {
            UTF8Char[0] = 'O';
            UTF8Char[1] = '\0';
            return 'O';
        }
        else if (cutf8 == 0xC3B4 || cutf8 == 0xC394)
        {
            UTF8Char[0] = 'O';
            UTF8Char[1] = '\0';
            return 'O';
        }
        else if (cutf8 == 0xC3B5 || cutf8 == 0xC395)
        {
            UTF8Char[0] = 'O';
            UTF8Char[1] = '\0';
            return 'O';
        }
        else if (cutf8 == 0xC3BA || cutf8 == 0xC39A)
        {
            UTF8Char[0] = 'U';
            UTF8Char[1] = '\0';
            return 'U';
        }
        else if (cutf8 == 0xC3B9 || cutf8 == 0xC399)
        {
            UTF8Char[0] = 'U';
            UTF8Char[1] = '\0';
            return 'U';
        }
        else if (cutf8 == 0xC3A7 || cutf8 == 0xC387)
        {
            UTF8Char[0] = 'C';
            UTF8Char[1] = '\0';
            return 'C';
        }
        else
            return cutf8;
    }
}

/**
 *  \brief Check if a given character is a vowel.
 *
 *	\param c character to be checked
 *
 *	\return 1 if is vowel
 */
static int isVowel(int c)
{
    if (c == 'A' || c == 'E' || c == 'I' || c == 'O' || c == 'U' || c == 'Y')
        return 1;

    return 0;
}

/**
 *  \brief Process a given character.
 *
 *	\param c character to be checked
 *	\param inWord inWord flag
 *	\param nWords total words
 *	\param nWordswVowel number of vowels per word
 *	\param firstOccur first vowel occurence flag
 */
static void processAChar(int c, int* inWord, int* nWords, int nWordswVowel[6], int firstOccur[6])
{
    // outside a word
    if (*inWord == 0)
    {
        // alpha numeric character or underscore
        if ((c > 64 && c < 91) || (c > 47 && c < 58) || c == '_')
        {
            *inWord = 1;
            (*nWords)++;
            setToZero(firstOccur);
            if (isVowel(c))
            {
                nWordswVowel[vowelOffset(c)] += 1;
                firstOccur[vowelOffset(c)] = 1;
            }
        }
    }
    // inside a word
    else 
    {
        // space or punctuation symbol or separation symbol
        if ((c == 0x20 || c == 0x9 || c == 0xA || c == 0xD) || (c == '-' || c == 0x22 || c == 0xE2809C || c == 0xE2809D || c == '[' || c == ']' || c == '(' || c == ')') || (c == '.' || c == ',' || c == ':' || c == ';' || c == '?' || c == '!' || c == 0xE28093 || c == 0xE280A6))
        {
            *inWord = 0;
        }
        // alpha numeric character or underscore or apostrophe
        if ((c > 64 && c < 91) || (c > 47 && c < 58) || c == '_' || c == 0x27)
        {
            if (isVowel(c) && firstOccur[vowelOffset(c)] == 0)
            {
                nWordswVowel[vowelOffset(c)] += 1;
                firstOccur[vowelOffset(c)] = 1;
            }
        }
    }
}

/**
 *  \brief Get the offset of a vowel.
 *
 *	\param c character to get the offset
 *
 *	\return offset
 */
static int vowelOffset(int c)
{
	int offset = -1;
	switch(c)
	{
		case 'A':
			offset = A;
			break;
		case 'E':
			offset = E;
			break;
		case 'I':
			offset = I;
			break;
		case 'O':
			offset = O;
			break;
		case 'U':
			offset = U;
			break;
		case 'Y':
			offset = Y;
			break;
	}
	
	return offset;
}
