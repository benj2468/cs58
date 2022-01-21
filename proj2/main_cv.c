#include <string.h>
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
    pthread_cond_t cvar;
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
        /*
        Reqs:
        - cars going opposite directions crash on the bridge -- ensured by checking direction here, and switching direction below
        - the bridge collapses, because too many cars were on it -- ensured by making sure less than MAX_CARS are on bridge
        */
        if (ledyard->cars < MAX_CARS && ledyard->direction == dir)
        {

            printf("[%d] Car Entering \n", id);
            ledyard->cars++;
            pthread_mutex_unlock(&ledyard->lock);
            break;
        }
        else if (ledyard->cars == 0 && ledyard->direction != dir)
        // Cars don't crash here because we have 0 cars going the other direction, so switching direction is OK.
        {
            printf("[%d] Car Entering, switching dir \n", id);
            ledyard->direction = dir;
            ledyard->cars++;
            pthread_mutex_unlock(&ledyard->lock);
            break;
        }
        ledyard->waiting[dir]++;
        pthread_cond_wait(&ledyard->cvar, &ledyard->lock);
        ledyard->waiting[dir]--;
        pthread_mutex_unlock(&ledyard->lock);
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
    | Waiting (Norwich): %d         |\n\
    | Waiting (Hanover): %d         |\n\
    |------------------------------|\n",
           id, direction, ledyard->cars, ledyard->waiting[TO_NORWICH], ledyard->waiting[TO_HANOVER]);

    pthread_mutex_unlock(&ledyard->lock);

    sleep(TIME_TO_CROSS);
}

void ExitBridge(struct Car *car)
{
    int id = car->id;
    enum Direction dir = car->direction;
    pthread_mutex_lock(&ledyard->lock);

    // printf("[%d] Car Leaving\n", id);

    ledyard->cars--;

    pthread_cond_signal(&ledyard->cvar);

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

    return NULL;
}

int main(int argc, char **argv)
{

    ledyard = malloc(sizeof(struct Bridge));
    int total_cars = 1;

    pthread_t **all_threads = (pthread_t **)malloc(total_cars * sizeof(pthread_t *));

    // Ledyard Initialization
    if (pthread_mutex_init(&ledyard->lock, NULL) != 0)
    {
        printf("\n mutex init has failed\n");
        return 1;
    }
    if (pthread_cond_init(&ledyard->cvar, NULL) != 0)
    {
        printf("\n cvar init failed\n");
        return 1;
    }
    ledyard->cars = 0;
    ledyard->direction = TO_HANOVER;
    // Initialization Complete

    int c;
    int i = 0;
    char ch;

    while (1)
    {
        c = getchar();
        ch = (char)c;
        if (ch == '\n')
            break;

        if (i == total_cars)
        {
            total_cars = total_cars * 2;

            all_threads = (pthread_t **)realloc(all_threads, (total_cars) * sizeof(pthread_t *));
        }

        if (ch != '0' && ch != '1') // To Hanover
        {
            printf("%c\n", ch);
            perror("Invalid Character");
            exit(102);
        }

        struct Car *car = malloc(sizeof(struct Car));
        car->id = i;
        car->direction = (int)atoi(&ch);

        pthread_create(&all_threads[i], NULL, OneVehicle, (void *)car);
        i++;
    }

    int k = 0;
    while (all_threads[k] != NULL)
    {
        pthread_join(all_threads[k], NULL);
        k++;
    }

    return 0;
}