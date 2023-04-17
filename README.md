# TSP-Ant-Colony-System
Solves known instances of the Traveling Salesman Problem with Ant Colony System.

Optimal parameters of the algorithm were chosen with trial and error.

Estimated average error performance of 1% over all TSP instances.

## Instructions to run the program:
* ulimit -s 45000 (or more, to increase stack size);
* make (if it doesn’t work, type gcc -g -Wall -O3 -std=c99 -lm main.c manually); 
* ./main (.\a.exe).
