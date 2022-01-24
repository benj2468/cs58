#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>

#define MAX_CARS 3
#define TIME_TO_CROSS 1
#define FORCE_REDICECT 10

#define MAX_TIME_STEPS 5
#define MAX_CARS_PER_TIMESTEP 10
#define PROB_ZERO 4

enum Direction
{
    TO_NORWICH,
    TO_HANOVER
};

struct Car
{
    int id;
    enum Direction direction;
};

struct Bridge
{
    pthread_mutex_t lock;
    pthread_cond_t cvar;
    int cars;
    int waiting[2];
    int force_redirect;
    enum Direction direction;
};

struct Bridge *ledyard;

// ArriveBridge must not return until it is safe for the car to get on the bridge.
void ArriveBridge(struct Car *car)
{
    int id = car->id;
    enum Direction dir = car->direction;

    // Take the lock
    pthread_mutex_lock(&ledyard->lock);
    while (1)
    {
        // GRAD_CREDIT: Check for random redirection
        // We could have put this anywhere i.e. at any stange in the car's journey.
        if (!ledyard->force_redirect && ledyard->waiting[!dir] > 0 && (rand() % FORCE_REDICECT) == 0)
        {
            printf("FORCING A REDIRECTION\n");
            ledyard->force_redirect = 1;
        }
        /*
        Reqs:
        - cars going opposite directions crash on the bridge -- ensured by checking direction here, and switching direction below
        - the bridge collapses, because too many cars were on it -- ensured by making sure less than MAX_CARS are on bridge
        */
        if (ledyard->cars < MAX_CARS && ledyard->direction == dir && !ledyard->force_redirect)
        {
            printf("[%d] Car Entering \n", id);
            ledyard->cars++;
            break;
        }
        else if (ledyard->cars == 0 && ledyard->direction != dir)
        // Cars don't crash here because we have 0 cars going the other direction, so switching direction is OK. (since we have the lock)
        {
            ledyard->force_redirect = 0;
            printf("[%d] Car Entering, switching dir \n", id);
            ledyard->direction = dir;
            ledyard->cars++;
            break;
        }
        printf("[%d] Car Waiting \n", id);
        ledyard->waiting[dir]++;
        pthread_cond_wait(&ledyard->cvar, &ledyard->lock);
        ledyard->waiting[dir]--;
    }
    pthread_mutex_unlock(&ledyard->lock);
}

// OnBridge should, as a side-effect, print the state of the bridge and waiting cars, in some nice format, to make it easier to monitor the behavior of the system.
// (So.... watch out for race conditions here, too!)
void OnBridge(struct Car *car)
{
    // Take the lock
    pthread_mutex_lock(&ledyard->lock);

    char *direction = malloc(sizeof(char) * 10);
    if (car->direction == TO_NORWICH)
    {
        direction = "To Norwich";
    }
    else
    {
        direction = "To Hanover";
    }

    printf("\n\
    |------- [%d] Ledyard Bridge -------|\n\
    |------------------------------|\n\
    | Bridge Direction: %s |\n\
    | Cars on Bridge: %d            |\n\
    | Waiting (Norwich): %d         |\n\
    | Waiting (Hanover): %d         |\n\
    |------------------------------|\n",
           car->id, direction, ledyard->cars, ledyard->waiting[TO_NORWICH], ledyard->waiting[TO_HANOVER]);

    // Release the lock
    pthread_mutex_unlock(&ledyard->lock);

    // Sleep for the time it takes us to cross the bridge - without the lock
    sleep(TIME_TO_CROSS);
}

void ExitBridge(struct Car *car)
{
    int id = car->id;
    enum Direction dir = car->direction;

    // Take the lock
    pthread_mutex_lock(&ledyard->lock);

    printf("[%d] Car Leaving\n", id);

    // Remove ourselves from the bridge
    ledyard->cars--;

    // Release the lock
    pthread_mutex_unlock(&ledyard->lock);

    // Broadcast on the cvar that we have left the bridge, and that their might be space!
    pthread_cond_broadcast(&ledyard->cvar);
}

// Thread Main
// The argument is going to be a pointer to a struct Car
void *OneVehicle(void *arg)
{
    // Parse the argument
    struct Car *car = (struct Car *)arg;
    ArriveBridge(car);
    // On the Bridge
    // |
    // |
    // |
    OnBridge(car);
    // |
    // |
    // |
    ExitBridge(car);
    // Off the Bridge

    return 0;
}

// Actual Simulation
// Structure of schedule: rows indicate time steps - columns indiate how many cars arrive at that time step destinated for Hanover and for Norwich.
// arg: `times` is the length of schedule
int simulate(int schedule[][2], int times)
{

    ledyard = malloc(sizeof(struct Bridge));

    // Ledyard Initialization
    // Initialize lock
    if (pthread_mutex_init(&ledyard->lock, NULL) != 0)
    {
        printf("\n mutex init has failed\n");
        return 1;
    }
    // Initialize cvar
    if (pthread_cond_init(&ledyard->cvar, NULL) != 0)
    {
        printf("\n cvar init failed\n");
        return 1;
    }
    // Initialize bridge
    ledyard->cars = 0;
    ledyard->direction = TO_HANOVER;
    // Initialization Complete

    // Calculation for total number of cars
    int total_cars = 0;

    for (int i = 0; i < times; i++)
    {
        total_cars += schedule[i][0] + schedule[i][1];
    }

    // Store our threads so we can join them later
    pthread_t all_threads[total_cars];
    // Our list of cars - so we can free them up later
    struct Car cars[total_cars];

    // Will keep track of which car we are on 0..total_cars
    int id = 0;
    // For each time-step
    for (int t = 0; t < times; t++)
    {
        // For each car destined for Norwich
        for (int j = 0; j < schedule[t][0]; j++)
        {
            struct Car *car = &cars[id];
            car->id = id;
            car->direction = 0;
            pthread_create(&all_threads[id], NULL, OneVehicle, (void *)car);
            id++;
        }
        // For each car destined for Hanover
        for (int k = 0; k < schedule[t][1]; k++)
        {
            struct Car *car = &cars[id];
            car->id = id;
            car->direction = 1;
            pthread_create(&all_threads[id], NULL, OneVehicle, (void *)car);
            id++;
        }

        // Sleep for a second to illustrate each timestep
        sleep(1);
    }
    for (int r = 0; r < total_cars; r++)
    {
        // Join all the threads - indicating that all the cars will have finished their path across the bridge
        pthread_join(all_threads[r], NULL);
        // I'm not sure why this doesn't work here - but I want to free up the cars so that we aren't keeping all these excess cars lying around.
        // free(all_threads[r]);
        // free(cars[id]);
    }

    // Cleanup by destroying our locks
    pthread_mutex_destroy(&ledyard->lock);
    pthread_cond_destroy(&ledyard->cvar);

    // Free our ledyard structure.
    free(ledyard);

    return 0;
}

// Helper to generate a schedule
void make_schedule(int times, int schedule[times][2])
{
    for (int i = 0; i < times; i++)
    {
        int op = rand() % PROB_ZERO;
        int t[2] = {rand() % MAX_CARS_PER_TIMESTEP, rand() % MAX_CARS_PER_TIMESTEP};

        if (op == 0)
        {
            t[0] = 0;
        }
        else if (op == 1)
        {
            t[1] = 0;
        }
        schedule[i][0] = t[0];
        schedule[i][1] = t[1];
    }
}

// Helper to print the schedule
void print_schedule(int times, int schedule[times][2])
{
    printf("Schedule:\n---------------\n");
    for (int i = 0; i < times; i++)
    {
        printf("---- [t = %d] Norwich: %d; Hanover: %d\n", i, schedule[i][0], schedule[i][1]);
    }
    printf("---------------\n");
}

// Driver for a random test & Simulation
void driver(char *out_file)
{
    int save_out = dup(fileno(stdout));

    int times = rand() % MAX_TIME_STEPS;
    int schedule[times][2];
    make_schedule(times, schedule);

    freopen(out_file, "w", stdout);

    printf("```\n");

    print_schedule(times, schedule);

    simulate(schedule, times);

    printf("```\n");

    fclose(stdout);
}

int main(int argc, char **argv)
{
    srand(time(NULL));

    int rc = fork();
    if (rc == 0)
    {
        // Make the logs dir if it doesn't already exist.
        execlp("mkdir", "mkdir", "-p", "./logs", NULL);
        exit(-1);
    }

    int tests = 1;
    if (argc > 1)
    {
        tests = atoi(argv[1]);
    }

    for (int i = 1; i <= tests; i++)
    {
        int digits = (int)ceil(log10(i));
        char output_file[11 + digits];
        sprintf(output_file, "./logs/test%d", i);
        driver(output_file);
    }
}
