#include "support.h"
#define NUM_FILES 10
#define TIME_LIMIT 180
#define ANTS_NUMBER 10
#define alpha 0.1
#define rho 0.1
#define beta 2.0
#define q0 0.98

int *shift(int *array, int position, int *length) {

  array[position] = array[*length - 1];
  array[*length - 1] = -1;
  *length = *length - 1;
  
  return array;
}

ant *position_ants(int ants_number, int N_CITIES) {
  // printf("city number:%d\n", N_CITIES);
  ant *ants = malloc(ANTS_NUMBER * sizeof(ant));
  if (ants == NULL)
    stop("Position_ants malloc 1 failed", HERE);

  for (int i = 0; i < ants_number; i++) {
    int r = rand() % N_CITIES;
    ants[i].starting_city = r;
    ants[i].current_city = r;
    ants[i].nv_len = N_CITIES - 1;
    ants[i].not_visited = malloc(ants[i].nv_len * sizeof(int));
    if (ants[i].not_visited == NULL)
      stop("Position_ants not_visited malloc 2 failed", HERE);
    int k = 0;
    for (int j = 0; j < N_CITIES; j++) {
      if (j != r) {
        ants[i].not_visited[k] = j;
        k++;
      }
    }
    ants[i].tour = malloc((N_CITIES + 1) * sizeof(int));
    if (ants[i].tour == NULL)
      stop("Position_ants tour malloc 2 failed", HERE);
    ants[i].tour[0] = ants[i].starting_city;
  }

  return ants;
}

void kill_ants(ant *ants) {
  for(int i = 0; i < ANTS_NUMBER; i++) {
    free(ants[i].tour);
    free(ants[i].not_visited);
  }
  free(ants);
}

int exploit(int N_CITIES, edge adjacence[][N_CITIES], ant ant) {
  
  int next_city = -1;
  double max = -1;
  for (int i = 0; i < ant.nv_len; i++) {
    double pheromone = adjacence[ant.current_city][ant.not_visited[i]].pheromone;
    double distance = (double) adjacence[ant.current_city][ant.not_visited[i]].cost;
    pheromone = pheromone / pow(distance, beta);
    if (max <= pheromone) {
      max = pheromone;
      next_city = i; // index of next city in not visited list
    }
  }
  
  return next_city;
}

int explore(int N_CITIES, edge adjacence[][N_CITIES], ant ant) {
  
  int next_city = -1;
  double denominator = 0;
    for (int i = 0; i < ant.nv_len; i++) {
      double pheromone = adjacence[ant.current_city][ant.not_visited[i]].pheromone;
      double distance = (double) adjacence[ant.current_city][ant.not_visited[i]].cost;
      denominator += pheromone / pow(distance, beta);
    }
    
    double r = (double) rand() / (double) RAND_MAX;
    double total = 0;
    
    for (int i = 0; i < ant.nv_len; i++) {
      double pheromone = adjacence[ant.current_city][ant.not_visited[i]].pheromone;
      double distance = (double) adjacence[ant.current_city][ant.not_visited[i]].cost;
      total += pheromone * pow(1.0 / distance, beta) / denominator;
      if (r <= total) {
        next_city = i; // index of next city in not visited list
        break;
      }
    }
  
  return next_city;
}

int state_transition_rule(int N_CITIES, edge adjacence[][N_CITIES], ant ant) {
  
  double q = (double) rand() / (double) RAND_MAX;
  if (q <= q0) 
    return exploit(N_CITIES, adjacence, ant);
  return explore(N_CITIES, adjacence, ant);
  
}

int tour_cost(ant ant, int N_CITIES, edge adjacence[][N_CITIES]) {
  int length = 0;
  for(int i = 0; i < N_CITIES; i++) 
    length = length + adjacence[ant.tour[i]][ant.tour[i+1]].cost;
  return length;
} 

void swap(int *tour, int i, int j) {
  while(i < j) {
    int temp = tour[i];
    tour[i] = tour[j];
    tour[j] = temp;
    i++;
    j--;
  }
}

void two_opt(int *tour, int N_CITIES, edge adj[][N_CITIES]) {
  bool improvement = true;
  while(improvement) {
    improvement = false;
    for(int i = 0; i < N_CITIES-1; i++) {
      for(int j = i+1; j < N_CITIES; j++) {
        int a = tour[i];
        int b = tour[j];
        int c = tour[i+1];
        int d = tour[j+1];
        int gain = adj[a][b].cost + adj[c][d].cost - adj[a][c].cost - adj[b][d].cost;
        if(gain < 0) {
          swap(tour, i+1, j);
          improvement = true;
        }
      }
    }
  }
}

int solve(city *cities, int N_CITIES) {

  int NN_sol = NN(cities, N_CITIES);
  double tau0 = 1.0 / (NN_sol * N_CITIES);

  // initialization of pheromone and distance matrix
  edge adjacence[N_CITIES][N_CITIES];
  edge data;
  data.pheromone = tau0;
  for(int i = 0; i < N_CITIES; i++) {
    for(int j = i+1; j < N_CITIES; j++) {
      data.cost = distance(cities[i], cities[j]);
      set_values(N_CITIES, adjacence, i, j, data);
    }
  }

  // variable to save best tour cost found so far
  int best_found = INT_MAX;

  time_t start = time(NULL);
  while(time(NULL) - start < TIME_LIMIT) {
  
    ant *ants = position_ants(ANTS_NUMBER, N_CITIES);
    for (int i = 0; i < N_CITIES; i++) {
      
      if (i < N_CITIES - 1) { // ants choose next city
        for (int j = 0; j < ANTS_NUMBER; j++) {
          int position = state_transition_rule(N_CITIES, adjacence, ants[j]);
          ants[j].next_city = ants[j].not_visited[position];
          ants[j].tour[i+1] = ants[j].next_city;
          ants[j].not_visited = shift(ants[j].not_visited, position, &ants[j].nv_len);
        }    
      } else { // ants go back to their initial city      
        for (int j = 0; j < ANTS_NUMBER; j++) {
          ants[j].tour[i+1] = ants[j].starting_city;
          ants[j].next_city = ants[j].starting_city;
        }       
      }

      //local update
      for (int j = 0; j < ANTS_NUMBER; j++) {
        double pheromone = adjacence[ants[j].current_city][ants[j].next_city].pheromone;
        pheromone = (1 - rho) * pheromone + rho * tau0;
        set_pheromone(N_CITIES, adjacence, ants[j].current_city, ants[j].next_city, pheromone);
        ants[j].current_city = ants[j].next_city;
      }

    }

    //  compute best tour for best ant 
    int tours[ANTS_NUMBER];
    int best_tour = INT_MAX;
    int best_ant = -1;
    for (int j = 0; j < ANTS_NUMBER; j++) {
      two_opt(ants[j].tour, N_CITIES, adjacence);
      tours[j] = tour_cost(ants[j], N_CITIES, adjacence);
      if(tours[j] <= best_tour) {
        best_ant = j;
        best_tour = tours[j];
      }
    }

    // global update
    for(int i = 0; i < N_CITIES; i++) {
      int from = ants[best_ant].tour[i];
      int to = ants[best_ant].tour[i+1];
      double pheromone = adjacence[from][to].pheromone;
      pheromone =  (1 - alpha) * pheromone + alpha / best_tour;
      set_pheromone(N_CITIES, adjacence, from, to, pheromone);
    }

    if(best_tour <= best_found)
      best_found = best_tour;

    kill_ants(ants);

  }
  
  return best_found;
  
}

int main(int argc, char *argv[]) {

  srand(time(NULL));

  struct dirent *pDirent;
  DIR *pDir;
  char *directory = "AI_cup_2022_problems/";

  pDir = opendir(directory);
  if (pDir == NULL)
    stop("Cannot open directory...", HERE);

  instance instances[10];
  int i = 0;
  while ((pDirent = readdir(pDir)) != NULL) {
    if (has_tsp_extension(pDirent->d_name)) {
      strcpy(instances[i].name, directory);
      strcat(instances[i].name, pDirent->d_name);
      if ((instances[i].file = fopen(instances[i].name, "r")) == NULL)
        stop("Couldn't open a tsp instance", HERE);
      i++;     
    }
  }

  double err = 0;
  double avg = 0;
  int solutions[NUM_FILES];
  i = 0;
  while (i < NUM_FILES) {
    city *tsp = get_coordinates(&instances[i]);    
    solutions[i] = solve(tsp, instances[i].dimension);
    err = ((double) solutions[i] - (double) instances[i].best_known) / (double) instances[i].best_known;
    avg = avg + err;
    printf("Best found: %d\n", solutions[i]);
    printf("Error: %.10f\n", err);
    free(tsp);
    i++;
  }

  avg = avg / NUM_FILES;
  printf("Average error: %f\n", avg);

  // resources deallocation
  for (int i = 0; i < 10; i++)
    if (fclose(instances[i].file) == EOF)
      stop("Cannot close a file", HERE);
  closedir(pDir);

  return 0;
}