#define _GNU_SOURCE 
#include <dirent.h>
#include <errno.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>   
#include <stdlib.h> 
#include <string.h>  
#include <time.h>
#include <unistd.h>
#include <limits.h>
#define HERE __LINE__, __FILE__
#define MAX_LINE_LENGTH 256

/** 
 * Data structure to store an instance of the TSP problem.
 */
typedef struct {
  char name[MAX_LINE_LENGTH];
  int dimension;
  int best_known;
  FILE *file;
} instance;

/** 
 * Data structure to store the x and y coordinates of a city.
 */
typedef struct {
  /**
   * x coordinate of the city.
   */
  double x;
  /**
   * y coordinate of the city.
   */
  double y;
} city;

/** 
 * Data structure to store all information regarding an ant.
 */
typedef struct {
  /**
   * The starting city of the ant.
  */
  int starting_city;
  /**
   * The current city of the ant.
  */
  int current_city;
  /**
   * The next city the ant is going to move.
  */
  int next_city;
  /**
   * The already visited cities in the ant's tour.
  */
  int *tour;
  /**
   * Not yet visited cities by the ant.
  */
  int *not_visited;
  /**
   * How many cities are left to visit.
  */
  int nv_len;
} ant;

/** 
 * Data structure to store the cost and pheromone of an edge.
 */
typedef struct {
  /**
   * The cost of the edge.
  */
  int cost;
  /**
   * The pheromone deposited in the edge.
  */
  double pheromone;
} edge;

/** 
 * Prints an error message and exits, used to debug.
 * 
 * @param message the message to be printed
 * @param line the line where the error occurred
 * @param file the file where the error occurred
 */
void stop(const char *message, int line, char *file) {
  if (errno == 0)
    fprintf(stderr, "== %d == %s\n", getpid(), message);
  else
    fprintf(stderr, "== %d == %s: %s\n", getpid(), message, strerror(errno));
  fprintf(stderr, "== %d == Line: %d, File: %s\n", getpid(), line, file);
  exit(1);
}

/** 
 * Computes the euclidean distance between two cities.
 * 
 * @param a the first city
 * @param b the second city
 * 
 * @return the distance between the two cities rounded to the closest integer.
 */
int distance(city a, city b) {
  double x = pow(a.x - b.x, 2);
  double y = pow(a.y - b.y, 2);
  return round(sqrt(x + y));
}

/** 
 * Prints the euclidean position of the cities.
 * 
 * @param a the array of cities
 * @param n the number of cities
 * @param f the file where to print the cities
 */
void print_cities(city *a, int n, FILE *f) {
  for (int i = 0; i < n; i++)
    fprintf(f, "x: %.10f, y: %.10f\n", a[i].x, a[i].y);
}

/**
 * Prints the tour.
 * 
 * @param a the array of cities
 * @param n the number of cities
 * @param f the file where to print the tour
*/
void print_tour(int *a, int n, FILE *f) {
  for (int i = 0; i < n - 1; i++)
    fprintf(f, "%d -> ", a[i]);
  printf("%d\n", a[n-1]);
}

/** 
 * Sets pheromone at position (row, column) and (column, row).
 * 
 * @param dim the dimension of the matrix
 * @param matrix the matrix to be modified
 * @param row the row of the matrix
 * @param column the column of the matrix
 * @param pheromone the pheromone to be set
 */
void set_pheromone(int dim, edge matrix[][dim], int row, int column, double pheromone) {
  matrix[row][column].pheromone = pheromone;
  matrix[column][row].pheromone = pheromone;
}

/**
 * Sets cost and pheromone at position (row, column) and (column, row).
 * 
 * @param dim the dimension of the matrix
 * @param matrix the matrix to be modified
 * @param row the row of the matrix
 * @param column the column of the matrix
 * @param data the data to be set
 */
void set_values(int dim, edge matrix[][dim], int row, int column, edge data) {
  matrix[row][column].pheromone = data.pheromone;
  matrix[column][row].pheromone = data.pheromone;
  matrix[row][column].cost = data.cost;
  matrix[column][row].cost = data.cost;
}

/** 
 * Tells if a file has .tsp extension.
 *  
 * @param name the name of the file
 * 
 * @return true if the file has .tsp extension, false otherwise
 */
bool has_tsp_extension(char const *name) {
  size_t len = strlen(name);
  return len > 4 && strcmp(name + len - 4, ".tsp") == 0;
}

/**
 * Nearest Neighbor algorithm.
 * 
 * @param cities the array of cities
 * @param N_CITIES the number of cities
 * 
 * @return the length of the tour
*/
int NN(city *cities, int N_CITIES) { 
  
  int random_city = rand() % N_CITIES;
  int length = 0;
  city visited[N_CITIES];
  int nv_len = N_CITIES - 1;
  city not_visited[nv_len];
  int k = 0;

  // initialize not visited cities as every city except the random extraction
  for (int i = 0; i < N_CITIES; i++) 
    if(i != random_city)
      not_visited[k++] = cities[i];
  
  // starting city is the random extraction
  visited[0] = cities[random_city];
  
  int best_city = -1;
  int d = -1;
  for (int i = 0; i < N_CITIES - 1; i++) {
    int min = INT_MAX;
    for (int j = 0; j < nv_len; j++) { // finds best city so far
      d = distance(visited[i], not_visited[j]);
      if (d <= min) {
        min = d;
        best_city = j;
      }
    }
    length = length + min;
    visited[i + 1] = not_visited[best_city];

    // remove best city from not visited cities list
    city tmp;
    not_visited[best_city] = not_visited[nv_len - 1];
    not_visited[nv_len - 1] = tmp;
    nv_len--;
    
  }
  // close the tour
  length = length + distance(visited[N_CITIES - 1], visited[0]);
  return length;
}

/** 
 * Gets useful information from TSP instances.
 * 
 * @param instance the instance to be read
 * 
 * @return a pointer to a city array containing the coordinates of the cities
 */
city *get_coordinates(instance *instance) {

  printf("Instance: %s\n", instance->name);
  
  char line[MAX_LINE_LENGTH];
  char best_known[MAX_LINE_LENGTH];
  char dimension[MAX_LINE_LENGTH];

  city *c = NULL;
  
  // Read lines until EOF
  while (fgets(line, MAX_LINE_LENGTH, instance->file) != NULL) {
  
    if (strncmp(line, "NAME", 4) == 0) {
      sscanf(line, "NAME : %s", instance->name);
    } else if (strncmp(line, "BEST_KNOWN", 10) == 0) {
      sscanf(line, "BEST_KNOWN : %s", best_known);
      instance->best_known = atoi(best_known);
    } else if (strncmp(line, "DIMENSION", 9) == 0) {
      sscanf(line, "DIMENSION : %s", dimension);
      instance->dimension = atoi(dimension);
      c = (city *) malloc(instance->dimension * sizeof(city));
      if (c == NULL)
        stop("Insufficient Memory", HERE);
    } else if (strncmp(line, "NODE_COORD_SECTION", 18) == 0)
      for (int i = 0; i < instance->dimension; i++)
        if(fgets(line, MAX_LINE_LENGTH, instance->file) != NULL)
          sscanf(line, "%*d %lf %lf", &c[i].x, &c[i].y);
  }

  return c;

}
