#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "header.h"

/**
 * @brief a program that generates `n` random numbers and calls another program that sorts them using a user specified
 * amount of threads. The program will then print the sorted list of numbers to the screen.
 * @param argv[1] the number of random numbers to generate
 * @param argv[2] the number of threads to use
 */
int main(int argc, char const *argv[])
{
    int randSize;
    int numThreads;
    int *randNums = NULL;

    if (argc != 3)
    {
        printf("Usage: %s <number of random numbers> <number of threads>\n", argv[0]);
        exit(1);
    }

    randSize = atoi(argv[1]);
    if (randSize < 1)
    {
        printf("Error: number of random numbers must be greater than 0\n");
        exit(1);
    }
    else
    {
        printf("Generating %d random numbers\n", randSize);
    }

    numThreads = atoi(argv[2]);
    if (numThreads < 1)
    {
        printf("Error: number of threads must be greater than 0\n");
        exit(1);
    }
    else
    {
        printf("Using %d threads\n", numThreads);
    }

    randNums = generate(randSize);

    return 0;
}

int *generate(int size)
{
    int *randNums = malloc(sizeof(int) * size);
    srand(time(NULL));
    for (int i = 0; i < size; i++)
    {
        randNums[i] = rand();
    }
    return randNums;
}