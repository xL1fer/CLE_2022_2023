/**
 *  \file countWords.c (implementation file)
 *
 *  \brief Problem name: Count Portuguese Words.
 *
 *  Synchronization based on monitors.
 *  Threads and the monitor are implemented using the pthread library which enables the creation of a
 *  monitor of the Lampson / Redell type.
 *
 *  Generator thread of the intervening entities.
 *
 *  \author Author Name - Month Year
 */
 
//	compile command
// 		gcc -Wall -O3 -o countWords countWords.c sharedMemory.c -lpthread -lm

//	run command
// 		./countWords 4 text0.txt text1.txt text2.txt text3.txt text4.txt
 
#include <stdio.h>
#include <stdlib.h>
#include <libgen.h>
#include <unistd.h>
#include <stdbool.h>
#include <pthread.h>
#include <math.h>
#include <time.h>
#include <ctype.h>

#include "consts.h"
#include "sharedMemory.h"

//#define nThreads 4

/** \brief worker threads return status array */
int *statusWorkers;

/** \brief main thread return status */
int statusMain;

/** \brief worker life cycle routine */
static void *worker(void *id);

/** \brief execution time measurement */
static double get_delta_time(void);

/** \brief process chunk of text */
static struct FileResult processChunk(unsigned char* buffer, int chunkSize);

/** \brief set all elements of a given array to 0 */
static void setToZero(int array[6]);

/** \brief extract a char from a given buffer */
static int extractAChar(unsigned char* buffer, int* curPos, int* chunkSize, unsigned char UTF8Char[5]);

/** \brief verify if a given char is a vowel */
static int isVowel(int c);

/** \brief process a character */
static void processAChar(int c, int* inWord, int* nWords, int nWordswVowel[6], int firstOccur[6]);

/** \brief offset of a vowel */
static int vowelOffset(int c);

/**
 *  \brief Main thread.
 *
 *  Its role is starting the simulation by generating the intervening entities threads (workers) and
 *  waiting for their termination.
 *
 *  \param argc number of words of the command line
 *  \param argv list of words of the command line
 *
 *  \return status of operation
 */

int main(int argc, char *argv[])
{	
	// not enough arguments provided
	if (argc < 3)
	{
		fprintf(stderr, "no thread number or file name provided\n");
		exit(EXIT_FAILURE);
	}
	
	// get number of threads
	int nThreads = argv[1][0] - '0';
	
	if ((statusWorkers = malloc (nThreads * sizeof (int))) == NULL)
	{
		fprintf(stderr, "error on allocating space to the return status arrays of producer / consumer threads\n");
		exit(EXIT_FAILURE);
	}
	
	pthread_t *tIdWorkers;			/* workers internal thread id array */
	unsigned int *workers;			/* workers application defined thread id array */
	int *pStatus;					/* pointer to execution status */

	/* initializing the application defined thread id arrays for the workers */
	if (((tIdWorkers = malloc (nThreads * sizeof (pthread_t))) == NULL) ||
		((workers = malloc (nThreads * sizeof (unsigned int))) == NULL))
	{
		fprintf(stderr, "error on allocating space to both internal / external worker id arrays\n");
		exit(EXIT_FAILURE);
	}
	
	for (int i = 0; i < nThreads; i++)
		workers[i] = i;
	
	(void) get_delta_time();
	
	/* fill shared memory with files names */
	fillSharedMem(argv);

	/* generation of intervening entity threads */
	for (int i = 0; i < nThreads; i++)
	{
		if (pthread_create(&tIdWorkers[i], NULL, worker, &workers[i]) != 0)		/* thread worker */
		{
			perror ("error on creating thread worker");
			exit (EXIT_FAILURE);
		}
	}

	/* waiting for the termination of the intervening entity threads */
	for (int i = 0; i < nThreads; i++)
	{
		if (pthread_join(tIdWorkers[i], (void *) &pStatus) != 0)				/* thread worker */
		{
			perror("error on waiting for thread worker");
			exit (EXIT_FAILURE);
		}
		printf("thread workers, with id %u, has terminated: ", i);
		printf("its status was %d\n", *pStatus);
	}
	
	/* print obtained results */
	printResults();
	
	printf ("\nElapsed time = %.6f s\n", get_delta_time());

	exit(EXIT_SUCCESS);
}

/**
 *  \brief Function worker.
 *
 *  Its role is to simulate the life cycle of a worker.
 *
 *  \param par pointer to application defined worker identification
 */

static void *worker(void *par)
{
	unsigned int id = *((unsigned int *) par);									/* worker id */
	
	unsigned char buffer[MAX_CHUNK_SIZE] = "";
	int fileId = 0;
	int chunkSize = 0;
	struct FileResult fileResult;

	while (requestChunk(id, buffer, &fileId, &chunkSize))
	{
		fileResult = processChunk(buffer, chunkSize);
		postResults(id, fileResult, &fileId);
	}

	statusWorkers[id] = EXIT_SUCCESS;
	pthread_exit(&statusWorkers[id]);
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
	if(clock_gettime (CLOCK_MONOTONIC, &t1) != 0)
	{
		perror ("clock_gettime");
		exit(1);
	}
	return (double) (t1.tv_sec - t0.tv_sec) + 1.0e-9 * (double) (t1.tv_nsec - t0.tv_nsec);
}

/**
 *  \brief Process a text chunk.
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
