# Programming Project 2: Ledyard Bridge

## Running the Program

```
make
```

This will create two executables: `ledyard_cv.out` and `ledyard.out`. The first uses conditional variables, but does not necessarily ensure that the first car that arrives in any direction is the first to cross the bridge. The second on the other hand uses a FIFO queue and locks to ensure this. Both satisfy the requirements of the assignment.

You should pipe in a string of `1`s and `0`s that indicate the destination of the cars as they arrive at the bridge. I.E. 0101 indicates that a car destined for Hanover arrives, then a car for Norwich, then Hanover, then Norwich.

You can pipe this in, or type it into `stdin`. With no arguments the program will read from stdin a finite number or `1`s and `0`s. _Optional addition_ would be to allow for infinite input.

## Tests

I have tested with a series of binary strings, I have never seen more than MAX_CARS on the bridge, nor have I every seen the program not finish.

My confidence is compounded by the design of the system, that every touching of `ledyard` is surrounded by a lock, and in the case of `_cv` we are waiting using our conditional variable to re-acquire the lock after we get signaled.
