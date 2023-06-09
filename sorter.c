#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/wait.h>
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
    int *ready = NULL;
    pid_t *processes = NULL;
    int *phase = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);

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

    *phase = 0;
    ready = mmap(NULL, sizeof(int) * numThreads, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    processes = (pid_t *)malloc(numThreads * sizeof(pid_t));

    for (i = 0; i < numThreads; i++)
    {
        ready[i] = 0;
    }

    /* Create a new process for each portion of the array */
    for (i = 0; i < numThreads; i++)
    {
        /* Pass the start and end indices of the current portion of the array to the new process */
        int j;
        int id = i;
        pid_t pid = fork();
        if (pid == 0)
        {
            for (i = 0; i < size * 2; i++)
            {
                fprintf(stderr, "Phase: %d\n", *phase);
                for (j = id * 2; j < size - 1; j += 2 * numThreads)
                {
                    if (evenOddSort(initial, j, phase, size))
                        ;
                }
                ready[id] = 1;
                synch(phase, ready, id, numThreads);
            }
        }
    }

    /* Wait for all of the processes to finish. */
    for (i = 0; i < numThreads; i++)
    {
        waitpid(processes[i], NULL, 0);
    }

    printf("Final list: [");
    for (i = 0; i < size; i++)
    {
        if (i == size - 1)
        {
            printf("%d]\n", initial[i]);
            break;
        }
        printf("%d, ", initial[i]);
    }

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
    int currSize = 16;
    int num;

    nums = mmap(NULL, sizeof(int) * 16, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);

    while (scanf("%d", &num) == 1)
    {
        char c;
        nums[i] = num;
        i++;

        if (i % 16 == 0)
        {

            nums = mremap(nums, sizeof(int) * currSize, sizeof(int) * (currSize + 16), MREMAP_MAYMOVE | MREMAP_FIXED, nums);
            currSize += 16;
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

void synch(int *phase, int *ready, int id, int numThreads)
{
    int i;
    for (i = 0; i < numThreads; i++)
    {
        while (ready[i] == 0)
            ;
    }

    if (id == 0)
    {
        *phase = *phase == 0 ? 1 : 0;
    }
    else
    {
        for (i = 0; i < 10000; i++)
            ;
    }
    ready[id] = 0;
}

/**
 * @brief takes a list of numbers and sorts a given pair of indices
 * @param nums the list of numbers
 * @param id the id of the thread, also acts as first index and multiple of
 * all other indices
 */
int evenOddSort(int *nums, int index, int *phase, int size)
{
    int i, swap = 0;
    if (*phase == 0)
    {
        swap = compare(nums, index);
    }
    else
    {
        if (index + 1 < size)
        {
            swap = compare(nums, index + 1);
        }
    }
    /*     printf("Sorted list: [");
        for (i = 0; i < size; i++)
        {
            if (i == size - 1)
            {
                printf("%d]\n", nums[i]);
                break;
            }
            printf("%d, ", nums[i]);
        } */
    return swap;
}

int compare(int *nums, int index)
{
    int swap = nums[index] > nums[index + 1] ? 1 : 0;
    if (nums[index] > nums[index + 1])
    {
        int temp = nums[index];
        fprintf(stderr, "swapping %d and %d\n", nums[index], nums[index + 1]);

        nums[index] = nums[index + 1];
        nums[index + 1] = temp;
    }
    return swap;
}
