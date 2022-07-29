#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <pthread.h>

#define NUM_THREADS 8

#define NUM_TRIALS 100000
#define MAX_TRACK_NUM8 5

#define GRIDSIZE_X 30
#define GRIDSIZE_Y 16
#define NUM_MINES  99


void * simulate (void *args)
{
    int grid[GRIDSIZE_Y][GRIDSIZE_X];
    int offset;
    int num_8;
    int *num_each_num8 = ((int **)args)[1];
    const int thread_num_trials = *((int **)args)[0];

    srand(time(NULL) + pthread_self());

    for(int i = 0; i < thread_num_trials; i++) {
        /* populate the grid with mines */
        memset(grid, 0, sizeof(int) * GRIDSIZE_X * GRIDSIZE_Y);
        for(int j = 0; j < NUM_MINES; j++) {
            do {
                offset = rand() % (GRIDSIZE_X * GRIDSIZE_Y);
            } while(*((int *)grid + offset));
            *((int *)grid + offset) = 1;
        }

        /* check for numbers of 8 */
        num_8 = 0;
        for(int j = 1; j < GRIDSIZE_X - 1; j++) {
            for(int k = 1; k < GRIDSIZE_Y - 1; k++) {
                if(!grid[k][j] && grid[k - 1][j - 1] && grid[k - 1][j]
                   && grid[k - 1][j + 1] && grid[k][j - 1]
                   && grid[k][j + 1] && grid[k + 1][j - 1]
                   && grid[k + 1][j] && grid[k + 1][j + 1]
                ) {
                    num_8++;
                }
            }
        }

        /* save number of 8s generated */
        if(num_8) {
            if(num_8 <= MAX_TRACK_NUM8) {
                ((int *)num_each_num8)[num_8 - 1]++;
            } else {
                /* too many */
                ((int *)num_each_num8)[MAX_TRACK_NUM8 + 1]++;
            }
        }
    }

    return NULL;
}

int main(void) {
    pthread_t threads[NUM_THREADS];
    void *args[NUM_THREADS][2];

    int num_trials_each_thread[NUM_THREADS];
    int num_each_num8[MAX_TRACK_NUM8 + 1] = {};

    /* spread the work evenly among the threads, if number of trials is
     * not divisible by 8 then num_trials_each_thread[0] will have more than
     * the other ones
     */
    for(int i = 0; i < NUM_THREADS; i++) {
        num_trials_each_thread[i] = NUM_TRIALS / NUM_THREADS;
        args[i][0] = num_trials_each_thread + i;
        args[i][1] = num_each_num8;
    }
    num_trials_each_thread[0] += (
        NUM_TRIALS - NUM_TRIALS / NUM_THREADS * NUM_THREADS
    );


    for(int i = 0; i < NUM_THREADS; i++) {
        if(pthread_create(&threads[i], NULL, simulate, args[i])) {
            fputs("something went wrong :)\n", stderr);
            return 1;
        }
    }

    for(int i = 0; i < NUM_THREADS; i++) {
        if(pthread_join(threads[i], NULL)) {
            fputs("something went wrong :)\n", stderr);
            return 1;
        }
    }

    printf("Total trials: %d\n", NUM_TRIALS);
    for(int i = 0; i < MAX_TRACK_NUM8; i++) {
        printf("%5d: %d\n", i + 1, num_each_num8[i]);
    }
    printf(">= %2d: %d\n", MAX_TRACK_NUM8 + 1,
           num_each_num8[MAX_TRACK_NUM8]);

    return 0;
}
