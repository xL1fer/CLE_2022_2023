/**
 *  \file sortingNumbers.c (implementation file)
 *
 *  \brief Problem name: Sorting Integer Sequence.
 *
 *  Single threaded version (CPU).
 *	
 *  \author Author Name - Month Year
 */
 
//  gcc -Wall -O3 -o sortingNumbers sortingNumbers.c

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

// allusion to internal functions

static void validateArray(int a[], int size);

static double get_delta_time(void);

/**
 *  \brief Main program.
 *
 *  The main thread parses the integer sequence file and sorts it by using binary sort bottom-up approach.
 *
 *  \param argc number of words of the command line
 *  \param argv list of words of the command line
 *
 *  \return status of operation
 */
int main(int argc, char* argv[])
{
    // check if the file name argument was passed
    if (argc < 2)
    {
        printf("No file name provided.\n");
        return EXIT_FAILURE;
    }

    FILE* fp = fopen(argv[1], "r");

    if (fp == NULL)
    {
        printf("Error opening file \"%s\".\n", argv[1]);
        return EXIT_FAILURE;
    }

    int N; // number of integers in the sequence
    if (fread(&N, sizeof(int), 1, fp) == EOF)
    {
        printf("Error parsing file \"%s\".\n", argv[1]);
        fclose(fp);
        return EXIT_FAILURE;
    }
    printf("> N: %d\n", N);

    int* val = (int*) malloc(N * sizeof(int));

    // get the sequence of integers
    for (int i = 0; i < N; i++)
    {
        if (fread(val + i, sizeof(int), 1, fp) == EOF)
        {
            printf("Error parsing file \"%s\".\n", argv[1]);
            free(val);
            fclose(fp);
            return EXIT_FAILURE;
        }
    }
	
	(void)get_delta_time();

    // https://en.wikipedia.org/wiki/Bitonic_sorter

    for (int k = 2; k <= N; k *= 2) // k is doubled every iteration
    {
        //int iteration = 0;
        for (int j = k / 2; j > 0; j /= 2) // j is halved at every iteration, with truncation of fractional parts
        {
            //printf("ITERATION %d=================\n", iteration++);
            for (int i = 0; i < N; i++)
            {
                int l = i ^ j;
                //printf("> %d ^ %d = %d", i, j, l);
                if (l > i)
                {
                    //printf(" ; l is higher than i\n");
                    if ((((i & k) == 0) && (val[i] > val[l])) || (((i & k) != 0) && (val[i] < val[l])))
                    {
                        int temp = val[i];
                        val[i] = val[l];
                        val[l] = temp;
                    }
                }
                //else printf(" ; NOT HIGHER\n");
            }
        }
    }
	
	printf("\nElapsed time = %.6f s\n", get_delta_time());

    //printArray(val, N);
    validateArray(val, N);

    free(val);
    fclose(fp);

    return EXIT_SUCCESS;
}

/**
 *  \brief Verify if the integer sequence is sorted.
 *
 *  Operation carried out by the CPU
 */
static void validateArray(int a[], int N)
{
    for (int i = 0; i < N - 1; i++)
    {
        if (a[i] > a[i + 1])
        {
            printf("Error in position %d between element %d and %d\n", i, a[i], a[i + 1]);
            return;
        }
    }
    printf("Everything is OK!\n");
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
