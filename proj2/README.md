# Programming Project 2: Ledyard Bridge

## Running the Program

```
make
```

This will create a single executable:`ledyard.out`. 

The program can take in an argument, namely an integer $x \geq 0$. It will run $x$ iterations of the simulation, each time selecting a random number of timesteps between $0$ and $9$, and at each timestep selecting a random number of cars to arrive from Hanover and from Norwhich (between $0$ and $9$). At each timestep, there is an extra $\frac{1}{4}$ probability that there are $0$ cars from either direction.

## Constants

```c
#define MAX_CARS 3

#define TIME_TO_CROSS 1

#define FORCE_REDIRECT 10 // The $p$ of note in the Graduate Extra credit component.
```

## System Design
```c
struct Car
{
    int id;
    enum Direction direction;
};
```

In addition to passing the `direction` into each thread, I decided to also pass in the `id` so that I could print nice clean descriptions for each cars position. This was not necessary and the `id` is not used in any logical way - strictly for debugging purposes. This obviosly removes some efficiency as I am allocating more memory on the stack with this struct, but it allows for further extensibility to this problem: say each car weighed $w$ and rather than restricting the number of cars we wanted to restrict the weight - having this struct would easily allow us to make this alteration to the system.

Otherwise the design choices I made are relatively simple and straightforward. 

## Graduate Extra Credit

I took the liberty of accomplishing the graduate extra credit by using randomness. 

GIVEN that there are cars waiting on the other side, WITH probability $\frac{1}{p}$, make flag `force_redirect` true.

IF `force_redirect` is true, do not allow anyone else on bridge from current direction.

Since we know there must be someone waiting (the precondition of the flag being set to true), and since we don't allow other cars to get on, we MUST reach 0 cars at some point, and then a car from the other side will get on - causing a lack of starvation (gauranteed by the randomness).

## Tests

When you run the program, you can see in `./logs` the output files, one for each of the simulations that you ran. The output shows when a car enters the bridge, the status when it is on the bridge, if it has to wait (possibly multiple times), and when it gets off the bridge. You can confirm that each car does each action sequentially, and observe that no car gets on the bridge when there are more than MAX_CARS (which is defined as $3$ in the program) on the bridge.