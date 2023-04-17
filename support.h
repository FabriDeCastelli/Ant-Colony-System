#define _GNU_SOURCE 
#include <dirent.h>
#include <errno.h> // error handling
#include <math.h>
#include <stdbool.h> // booleans
#include <stdio.h>   
#include <stdlib.h> 
#include <string.h>  
#include <time.h>
#include <unistd.h>
#include <limits.h>
#define HERE __LINE__, __FILE__
#define MAX_LINE_LENGTH 256

typedef struct {
  int city;
  int cost;
} data;

typedef struct {
  char name[MAX_LINE_LENGTH];
  int dimension;
  int best_known;
  FILE *file;
} instance;

typedef struct {
  double x;
  double y;
} city;

typedef struct {
  int starting_city;
  int current_city;
  int next_city;
  int *tour;
  int *not_visited;
  int nv_len;
} ant;

typedef struct {
  int cost;
  double pheromone;
} edge;

/** Stops the program in case of error and prints where the error occured */
void stop(const char *message, int line, char *file) {
  if (errno == 0)
    fprintf(stderr, "== %d == %s\n", getpid(), message);
  else
    fprintf(stderr, "== %d == %s: %s\n", getpid(), message, strerror(errno));
  fprintf(stderr, "== %d == Line: %d, File: %s\n", getpid(), line, file);
  exit(1);
}

/** Computes the Euclidian distance of two cities, return value is rounded */
int distance(city a, city b) {
  double x = pow(a.x - b.x, 2);
  double y = pow(a.y - b.y, 2);
  return round(sqrt(x + y));
}

/** Print cities position */
void print_cities(city *a, int n, FILE *f) {
  for (int i = 0; i < n; i++)
    fprintf(f, "x: %.10f, y: %.10f\n", a[i].x, a[i].y);
    //fprintf(f, "array[%d]: %8d\n", i, a[i]);
}

/** Print a tour in file f */
void print_tour(int *a, int n, FILE *f) {
  for (int i = 0; i < n - 1; i++)
    fprintf(f, "%d -> ", a[i]);
    //fprintf(f, "array[%d]: %8d\n", i, a[i]);
  printf("%d\n", a[n-1]);
}

/** set pheromone at position (row, column) */
void set_pheromone(int dim, edge matrix[][dim], int row, int column, double pheromone) {
  matrix[row][column].pheromone = pheromone;
  matrix[column][row].pheromone = pheromone;
}

/** set pheromone and distance at position (row, column) */
void set_values(int dim, edge matrix[][dim], int row, int column, edge data) {
  matrix[row][column].pheromone = data.pheromone;
  matrix[column][row].pheromone = data.pheromone;
  matrix[row][column].cost = data.cost;
  matrix[column][row].cost = data.cost;
}

/** states if a file ends with .tsp extension  */
bool has_tsp_extension(char const *name) {
  size_t len = strlen(name);
  return len > 4 && strcmp(name + len - 4, ".tsp") == 0;
}


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

/** Get all useful data from a TSP instance */
city *get_coordinates(instance *instance) {

  printf("Instance: %s\n", instance->name);
  
  char line[MAX_LINE_LENGTH];
  char best_known[MAX_LINE_LENGTH];
  char dimension[MAX_LINE_LENGTH];

  city *c = NULL;
  
  // read lines until EOF
  while (fgets(line, MAX_LINE_LENGTH, instance->file) != NULL) {
    
    //printf("Reading %s\n", line);
    if (strncmp(line, "NAME", 4) == 0)
      sscanf(line, "NAME : %s", instance->name);

    else if (strncmp(line, "BEST_KNOWN", 10) == 0) {
      sscanf(line, "BEST_KNOWN : %s", best_known);
      instance->best_known = atoi(best_known);
    }

    else if (strncmp(line, "DIMENSION", 9) == 0) {
      sscanf(line, "DIMENSION : %s", dimension);
      instance->dimension = atoi(dimension);
      c = (city *) malloc(instance->dimension * sizeof(city));
      if (c == NULL)
        stop("Insufficient Memory", HERE);
    } 
    
    else if (strncmp(line, "NODE_COORD_SECTION", 18) == 0)
      for (int i = 0; i < instance->dimension; i++)
        if(fgets(line, MAX_LINE_LENGTH, instance->file) != NULL)
          sscanf(line, "%*d %lf %lf", &c[i].x, &c[i].y);
  }

  return c;

}
