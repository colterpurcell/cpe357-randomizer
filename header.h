typedef struct
{
    atomic_int count;
    int total_threads;
    atomic_int barrier_flag;
} Barrier;

/* function prototypes */
int *generate(int);
int *readIn(int *);
int evenOddSort(int *, int, int *, int);
int compare(int *, int);
void barrier_init(Barrier *, int);
void barrier_wait(Barrier *);
void customDelay(unsigned int);
