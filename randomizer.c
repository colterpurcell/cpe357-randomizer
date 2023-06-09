#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include "header.h"

/**
 * @brief a program that generates `n` random numbers and calls another program that sorts them using a user specified
 * amount of threads. The program will then print the sorted list of numbers to the screen.
 * @param argv[1] the number of random numbers to generate
 * @param argv[2] the number of threads to use
 */
int main(int argc, char const *argv[])
{
    int i;
    int randSize;
    int numThreads;
    int *randNums = NULL;
    int fd[2];
    int pid;

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

    if (numThreads > (randSize / 2))
    {
        numThreads = randSize / 2;
    }

    randNums = generate(randSize);

    if (pipe(fd) == -1)
    {
        perror("pipe");
        exit(1);
    }

    dup2(fd[1], STDIN_FILENO);

    for (i = 0; i < randSize; i++)
    {
        write(fd[1], &randNums[i], sizeof(int));
    }

    /* Reset standard in to be orignial fileno */
    dup2(0, fd[1]);

    if ((pid = fork()) == -1)
    {
        perror("fork");
        exit(1);
    }
    else if (pid == 0)
    {
        execl("./EvenOdd", "./EvenOdd", argv[2], NULL);
        return 0;
    }
    else
    {
        wait(NULL);
    }

    free(randNums);
    return 0;
}

int *generate(int size)
{
    int i, *randNums = malloc(sizeof(int) * size);
    srand(time(NULL));
    for (i = 0; i < size; i++)
    {
        randNums[i] = rand() % 201 + (-100);
    }
    return randNums;
}
