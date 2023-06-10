#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <semaphore.h>
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
    int memSize = 0;
    struct timeval start, end, result;
    int *initial = NULL;
    pid_t *processes = NULL;
    int *ready = NULL;
    int *waiting = NULL;
    int phase = 0;

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
    if (numThreads > size / 2)
    {
        numThreads = size / 2;
    }
    printf("Initial list: [");
    for (i = 0; i < size; i++)
    {
        if (i == size - 1)
        {
            printf("%d]\n\n", initial[i]);
            break;
        }
        printf("%d, ", initial[i]);
    }

    waiting = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    ready = mmap(NULL, numThreads * sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);

    processes = (pid_t *)malloc(numThreads * sizeof(pid_t));

    /* Create a new process for each portion of the array */
    gettimeofday(&start, NULL);
    for (i = 0; i < numThreads; i++)
    {
        /* Pass the start and end indices of the current portion of the array to the new process */
        int j, iter;
        int id = i;
        pid_t pid = fork();
        if (pid == 0)
        {
            for (iter = 0; iter < size * 2; iter++)
            {
                ready[id] = 0;
                for (j = id * 2; j < size - 1; j += 2 * numThreads)
                {
                    evenOddSort(initial, j, &phase, size);
                }
                ready[id] = 1;
                synch(&phase, ready, id, numThreads, waiting);
            }
            return 0;
        }
    }

    if (size % 1024 == 0)
    {
        memSize = size;
    }
    else
    {
        memSize = size + (1024 - (size % 1024));
    }

    /* Wait for all of the processes to finish. */
    for (i = 0; i < numThreads; i++)
    {
        waitpid(processes[i], NULL, 0);
    }
    gettimeofday(&end, NULL);
    timersub(&end, &start, &result);

    printf("Final list:   [");
    for (i = 0; i < size; i++)
    {
        if (i == size - 1)
        {
            printf("%d]\n", initial[i]);
            break;
        }
        printf("%d, ", initial[i]);
    }

    printf("\nProcesses: %d\n", numThreads);
    printf("Time: %ld.%06ld seconds\n", result.tv_sec, result.tv_usec);
    munmap(ready, sizeof(int) * numThreads);
    munmap(initial, sizeof(int) * memSize);
    free(processes);
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
    int *temp;

    nums = mmap(NULL, sizeof(int) * 1024, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);

    while (scanf("%d", &num) == 1)
    {
        int c;
        nums[i] = num;
        i++;

        if (i % 1024 == 0)
        {
            temp = mmap(NULL, sizeof(int) * (i + 1024), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
            memcpy(temp, nums, sizeof(int) * i);
            munmap(nums, sizeof(int) * i);
            nums = temp;
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

void synch(int *phase, int *ready, int id, int numThreads, int *waiting)
{
    int i;
    (*waiting)++;
    if (*waiting >= numThreads)
    {
        for (i = 0; i < numThreads; i++)
        {
            while (ready[i] != 1)
            {
            }
        }
        *waiting = 0;
    }
    else
    {
        while (ready[id] != 1)
        {
        }
    }

    *phase = *phase + 1;
    /* if last process to enter, wake all other processes */

    for (i = 0; i < 100000; i++)
        ;
}

/**
 * @brief takes a list of numbers and sorts a given pair of indices
 * @param nums the list of numbers
 * @param id the id of the thread, also acts as first index and multiple of
 * all other indices
 */
int evenOddSort(int *nums, int index, int *phase, int size)
{
    int swap = 0;
    if (*phase % 2 == 0)
    {
        swap = compare(nums, index);
    }
    else
    {
        if (index + 1 < size - 1)
        {
            swap = compare(nums, index + 1);
        }
    }
    return swap;
}

int compare(int *nums, int index)
{
    int swap = nums[index] > nums[index + 1] ? 1 : 0;
    if (nums[index] > nums[index + 1])
    {
        int temp = nums[index];
        nums[index] = nums[index + 1];
        nums[index + 1] = temp;
    }
    return swap;
}
