#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

#define MAX_CARS 3
#define TIME_TO_CROSS 1

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
    int cars;
    int waiting[2];
    enum Direction direction;
};

struct Bridge *ledyard;

// ArriveBridge must not return until it is safe for the car to get on the bridge.
void ArriveBridge(struct Car *car)
{
    int id = car->id;
    enum Direction dir = car->direction;
    while (1)
    {
        pthread_mutex_lock(&ledyard->lock);
        if (ledyard->cars < MAX_CARS && ledyard->direction == dir)
        {
            printf("[%d] Car Entering \n", id);
            ledyard->cars++;
            pthread_mutex_unlock(&ledyard->lock);
            break;
        }
        else if (ledyard->cars == 0 && ledyard->direction != dir)
        {
            printf("[%d] Car Entering, switching dir \n", id);
            ledyard->direction = dir;
            ledyard->cars++;
            pthread_mutex_unlock(&ledyard->lock);
            break;
        }
        pthread_mutex_unlock(&ledyard->lock);
        sleep(1);
    }
}

// OnBridge should, as a side-effect, print the state of the bridge and waiting cars, in some nice format, to make it easier to monitor the behavior of the system.
// (So.... watch out for race conditions here, too!)
void OnBridge(struct Car *car)
{

    int id = car->id;
    enum Direction dir = car->direction;
    int cars;

    pthread_mutex_lock(&ledyard->lock);

    char *direction = malloc(sizeof(char) * 10);
    if (dir == TO_NORWICH)
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
    | Waiting (Hanover): %d         |\n\
    | Waiting (Norwich): %d         |\n\
    |------------------------------|\n",
           id, direction, ledyard->cars, ledyard->waiting[0], ledyard->waiting[1]);

    pthread_mutex_unlock(&ledyard->lock);

    sleep(TIME_TO_CROSS);
}

void ExitBridge(struct Car *car)
{
    int id = car->id;
    enum Direction dir = car->direction;
    pthread_mutex_lock(&ledyard->lock);

    printf("[%d] Car Leaving\n", id);

    ledyard->cars--;

    pthread_mutex_unlock(&ledyard->lock);
}

void *OneVehicle(void *arg)
{

    struct Car *car = (struct Car *)arg;
    ArriveBridge(car);
    // now the car is on the bridge!

    OnBridge(car);

    ExitBridge(car);
    // now the car is off
}

int main(int argc, char **argv)
{

    if (argc < 3)
    {
        perror("Not enough arguments");
        exit(101);
    }

    int waiting[2] = {atoi(argv[1]), atoi(argv[2])};

    ledyard = malloc(sizeof(struct Bridge));

    int total_cars = waiting[0] + waiting[1];
    pthread_t *all_threads = malloc(total_cars * sizeof(pthread_t));

    // Ledyard Initialization
    if (pthread_mutex_init(&ledyard->lock, NULL) != 0)
    {
        printf("\n mutex init has failed\n");
        return 1;
    }
    ledyard->cars = 0;
    ledyard->direction = TO_HANOVER;

    // Initialization Complete

    int i;
    for (i = 0; i < waiting[0]; i++)
    {
        struct Car *car = malloc(sizeof(struct Car));
        car->id = i;
        car->direction = TO_HANOVER;
        pthread_create(&all_threads[i], NULL, OneVehicle, (void *)car);
    }
    for (i = i; i < total_cars; i++)
    {
        struct Car *car = malloc(sizeof(struct Car));
        car->id = i;
        car->direction = TO_NORWICH;
        pthread_create(&all_threads[i], NULL, OneVehicle, (void *)car);
    }

    for (int k = 0; k < total_cars; k++)
    {
        pthread_join(all_threads[k], NULL);
    }

    return 0;
}