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

struct ListItem
{
    int id;
    struct ListItem *next;
};

struct List
{
    struct ListItem *head;
    struct ListItem *tail;
};

struct Bridge
{
    pthread_mutex_t lock;
    int cars;
    struct List *waiting[2];
    enum Direction direction;
};

struct Bridge *ledyard;

// ArriveBridge must not return until it is safe for the car to get on the bridge.
void ArriveBridge(struct Car *car)
{
    int id = car->id;
    enum Direction dir = car->direction;
    int am_waiting = 0;

    while (1)
    {
        pthread_mutex_lock(&ledyard->lock);
        /*
        Reqs:
        - cars going opposite directions crash on the bridge -- ensured by checking direction here, and switching direction below
        - the bridge collapses, because too many cars were on it -- ensured by making sure less than MAX_CARS are on bridge
        */

        if ((am_waiting && ledyard->waiting[dir]->head->id == id) || !am_waiting)
        {
            if (ledyard->cars < MAX_CARS && ledyard->direction == dir)
            {
                if (am_waiting)
                {
                    ledyard->waiting[dir]->head = ledyard->waiting[dir]->head->next;
                    if (ledyard->waiting[dir]->head == NULL)
                    {
                        ledyard->waiting[dir]->tail = NULL;
                    }
                }
                // printf("[%d] Car Entering \n", id);
                ledyard->cars++;
                pthread_mutex_unlock(&ledyard->lock);
                break;
            }
            else if (ledyard->cars == 0 && ledyard->direction != dir)
            // Cars don't crash here because we have 0 cars going the other direction, so switching direction is OK.
            {
                if (am_waiting)
                {
                    ledyard->waiting[dir]->head = ledyard->waiting[dir]->head->next;
                    if (ledyard->waiting[dir]->head == NULL)
                    {
                        ledyard->waiting[dir]->tail = NULL;
                    }
                }
                // printf("[%d] Car Entering, switching dir \n", id);
                ledyard->direction = dir;
                ledyard->cars++;
                pthread_mutex_unlock(&ledyard->lock);
                break;
            }
        }

        // Car cannot get on now... so we have to add it to the queue.
        // if (!am_waiting)
        //     printf("[%d] Arrived and is waiting\n", id);
        am_waiting = 1;
        struct ListItem *car_item = malloc(sizeof(struct ListItem));
        car_item->id = id;
        if (ledyard->waiting[dir]->tail == NULL)
        {
            ledyard->waiting[dir]->tail = car_item;
            ledyard->waiting[dir]->head = car_item;
        }
        else
        {

            ledyard->waiting[dir]->tail->next = car_item;
            ledyard->waiting[dir]->tail = car_item;
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

    int waits[2];
    for (int i = 0; i < 2; i++)
    {
        if (ledyard->waiting[i]->head != NULL)
        {
            waits[i] = ledyard->waiting[i]->head->id;
        }
        else
        {
            waits[i] = NULL;
        }
    }

    printf("\n\
    |------- [%d] Ledyard Bridge -------|\n\
    |------------------------------|\n\
    | Bridge Direction: %s |\n\
    | Cars on Bridge: %d            |\n\
    | Waiting (Hanover): %d         |\n\
    | Waiting (Norwich): %d         |\n\
    |------------------------------|\n",
           id, direction, ledyard->cars, waits[0], waits[1]);

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
    pthread_t *all_threads = malloc(total_cars * sizeof(pthread_t));

    // Ledyard Initialization
    if (pthread_mutex_init(&ledyard->lock, NULL) != 0)
    {
        printf("\n mutex init has failed\n");
        return 1;
    }
    ledyard->cars = 0;
    ledyard->waiting[0] = malloc(sizeof(struct List));
    ledyard->waiting[1] = malloc(sizeof(struct List));
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
            pthread_t *new = malloc(total_cars * sizeof(pthread_t));
            memcpy(new, all_threads, total_cars / 2);
            all_threads = new;
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

    for (int k = 0; k < total_cars; k++)
    {
        pthread_join(all_threads[k], NULL);
    }

    return 0;
}