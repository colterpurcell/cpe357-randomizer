
typedef struct Barrier
{
    int n_processes;
    int count;
    int mutex;
    int turnstile;
} Barrier;

/* function prototypes */
int *generate(int);
int *readIn(int *);
int evenOddSort(int *, int, int *, int);
int compare(int *, int);
Barrier *barrierInit(int);
void barrierWait(Barrier *);
void customDelay(unsigned int);
