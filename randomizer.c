#define _BSD_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/time.h>
#include "header.h"

/**
 * @brief a program that generates `n` random numbers and calls another program that sorts them using a user specified
 * amount of threads. The program will then print the sorted list of numbers to the screen.
 * @param argv[1] the number of random numbers to generate
 * @param argv[2] the number of threads to use
 */
int main(int argc, char const *argv[])
{
    int i, j;
    int randSize;
    int numThreads;
    int *randNums = NULL;
    char *args[3] = {NULL};
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

    if (numThreads > (randSize / 2))
    {
        numThreads = randSize / 2;
    }

    randNums = generate(randSize);
    args[0] = malloc(sizeof(char) * 10);
    strcpy(args[0], "./EvenOdd");
    args[1] = malloc(sizeof(char) * 10);
    sprintf(args[1], "%d", numThreads);

    if (pipe(fd) == -1)
    {
        perror("pipe");
        exit(1);
    }

    /* Switch the reading end of the pipe to stdin */
    dup2(fd[0], STDIN_FILENO);

    if ((pid = fork()) == -1)
    {
        perror("fork");
        exit(1);
    }
    else if (pid == 0)
    {
        execv("./EvenOdd", args);
        return 0;
    }
    else
    {
        for (i = 0; i < randSize; i++)
        {
            char strNum[100];

            if (i == randSize - 1)
            {
                sprintf(strNum, "%d\n", randNums[i]);
            }
            else
            {
                sprintf(strNum, "%d ", randNums[i]);
            }

            for (j = 0; j < strlen(strNum); j++)
            {
                write(fd[1], &strNum[j], sizeof(char));
            }
        }
        close(fd[1]);
        dup2(0, fd[0]);
        wait(NULL);
    }

    free(args[0]);
    free(args[1]);
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
