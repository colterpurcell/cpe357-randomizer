#include <semaphore.h>

/* functuion prototypes */
int *generate(int);
int *readIn(int *);
int evenOddSort(int *, int, int *, int);
int compare(int *, int);
void synch(int *, sem_t *, int, int, int *);
