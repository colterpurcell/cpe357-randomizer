#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sys/mman.h>
#include <wait.h>
#include <string.h>
#include "header.h"

/**
 * @brief a program that takes a piped list of numbers passed in from stdin and sorts them using a user specified
 * amount of threads. The program will then print the sorted list of numbers to the screen.
 * @param argv[1] the number of threads to use
 */

int main(int argc, char const *argv[])
{
    int i;
    int numThreads;
    int size = 0;
    int *initial = NULL;
    int *sorted = NULL;
    if (argc != 2)
    {
        printf("Usage: %s <number of threads>\n", argv[0]);
        exit(1);
    }

    numThreads = atoi(argv[1]);
    if (numThreads < 1)
    {
        printf("Error: number of threads must be greater than 0\n");
        exit(1);
    }

    initial = readIn(&size);
    sorted = sort(initial, numThreads, size);

    printf("Initial list: [");
    for (i = 0; i < size; i++)
    {
        if (i == size - 1)
        {
            printf("%d]\n", initial[i]);
            break;
        }
        printf("%d, ", initial[i]);
    }

    printf("Sorted list: [");
    for (i = 0; i < size; i++)
    {
        if (i == size - 1)
        {
            printf("%d]\n", sorted[i]);
            break;
        }
        printf("%d, ", sorted[i]);
    }

    free(initial);
    munmap(sorted, sizeof(int) * size);
    return 0;
}

/**
 * @brief reads in a list of numbers from stdin or a file
 * @return a pointer to the list of numbers
 */

int *readIn(int *size)
{
    int *nums = NULL;
    int i = 0;
    int num;

    nums = malloc(sizeof(int) * 16);

    while (scanf("%d", &num) == 1)
    {
        char c;
        nums[i] = num;
        i++;

        if (i % 16 == 0)
        {
            nums = realloc(nums, sizeof(int) * (i + 16));
        }

        c = getchar();
        if (c == '\n')
        {
            break;
        }
        else if (c == EOF)
        {
            i--;
            break;
        }
        else
        {
            ungetc(c, stdin);
        }
    }

    *size = i;
    return nums;
}

void synch(int *ready, int numThreads, int *phase, int *success, int *done, int id)
{
    int i;
    printf("Thread %d is ready\n", id);
    for (i = 0; i < numThreads; i++)
    {
        while (ready[i] == 0)
        {
        }
    }
    if (id == 0)
    {
        *success = 1;
        for (i = 0; i < numThreads; i++)
        {
            if (done[i] == 0)
            {
                *success = 0;
            }
            done[i] = 0;
        }
        (*phase)++;
    }
    else
    {
        for (i = 0; i < 100000; i++)
        {
        }
    }
}

/**
 * @brief sorts a list of numbers using a Odd-Even Transposition Sort,
 * forking off a user specified amount of threads. shared memory will be necessary.
 * @param nums the list of numbers to sort
 * @param numThreads the number of forks to use
 * @return a pointer to the sorted list of numbers
 */

int *sort(int *nums, int numThreads, int size)
{
    int i, index = 0, pid, id;
    int *sorted = NULL;
    int *success = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    int *swapped = mmap(NULL, sizeof(int) * numThreads, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    int *phase = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    int *ready = mmap(NULL, sizeof(int) * numThreads, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    sorted = mmap(NULL, sizeof(int) * size, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);

    *success = 0;
    *phase = 0;

    for (i = 0; i < numThreads; i++)
    {
        ready[i] = 0;
        swapped[i] = 0;
    }

    for (i = 0; i < size; i++)
    {
        sorted[i] = nums[i];
    }

    for (i = 0; i < numThreads; i++)
    {
        pid = fork();
        if (pid == 0)
        {
            id = i;
            /* int success = 0;
            while (1)
            {
                success = 1;
                for (i = 0; i < numThreads; i++)
                {
                    if (swapped[i] == 0)
                    {
                        success = 0;
                    }
                    swapped[i] = 0;
                }
                if (success)
                {
                    break;
                }

            } */
            while (!(*success))
            {
                ready[id] = 0;
                if (*phase % 2 == 0)
                {
                    for (i = 0; i < size / 2; i++)
                    {
                        index = 2 * i + id % 2;
                        swapped[id] = compare(sorted, index);
                    }
                }
                else
                {
                    for (i = 0; i < size / 2; i++)
                    {
                        index = 2 * i + 1 - id % 2;
                        swapped[id] = compare(sorted, index);
                    }
                }
                ready[id] = 1;
                synch(ready, numThreads, phase, success, swapped, id);
            }
        }
        else
        {
            wait(NULL);
        }
    }
    return sorted;
}

int compare(int *list, int index)
{
    int done = list[index] > list[index + 1] ? 1 : 0;
    if (list[index] > list[index + 1])
    {
        int temp = list[index];
        list[index] = list[index + 1];
        list[index + 1] = temp;
    }
    return done;
}