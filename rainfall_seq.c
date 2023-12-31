/**
 * citations
 * OpenAI. (n.d.). ChatGPT. OpenAI.
*/

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

float ** allocate2DArray(int N) {
  float ** array = malloc(N * sizeof(float *));
  for (int i = 0; i < N; i++) {
    array[i] = (float *)calloc(N, sizeof(float));
  }
  return array;
}

void free2DArray(float ** array, int N) {
  for (int i = 0; i < N; i++) {
    free(array[i]);
  }
  free(array);
}

// Function to read the landscape elevations from a file
void readLandscape(FILE * file, float ** landscape, int N) {
  for (int i = 0; i < N; i++) {
    for (int j = 0; j < N; j++) {
      fscanf(file, "%f", &landscape[i][j]);
    }
  }
}

//Absorbing at the i, j step
void absorbRainDrops(int i, int j, float A, float ** absorbed, float ** water) {
  if (water[i][j] > 0) {
    float absorb = (water[i][j] < A) ? water[i][j] : A;
    absorbed[i][j] += absorb;
    water[i][j] -= absorb;
  }
}

//Trickling extra rainwater
void trickleRainDrops(int i,
                      int j,
                      int N,
                      float ** landscape,
                      float ** water,
                      float ** trickleWater) {
  int lowestElevation = landscape[i][j];
  int numLowest = 0;
  int dx[4] = {0, 0, 1, -1};
  int dy[4] = {1, -1, 0, 0};

  // Find the lowest neighboring elevation
  for (int k = 0; k < 4; k++) {
    int ni = i + dx[k];
    int nj = j + dy[k];
    if (ni >= 0 && ni < N && nj >= 0 && nj < N) {
      if (landscape[ni][nj] < lowestElevation) {
        lowestElevation = landscape[ni][nj];
        numLowest = 1;
      }
      else if (landscape[ni][nj] == lowestElevation) {
        numLowest++;
      }
    }
  }

  // Distribute water to lowest neighbors
  float amount = water[i][j] < 1.0 ? water[i][j] : 1.0;
  float amountEachPoint = amount / numLowest;

  if (numLowest > 0 && lowestElevation < landscape[i][j]) {
    for (int k = 0; k < 4; k++) {
      int ni = i + dx[k];
      int nj = j + dy[k];
      if (ni >= 0 && ni < N && nj >= 0 && nj < N &&
          landscape[ni][nj] == lowestElevation) {
        trickleWater[ni][nj] += amountEachPoint;
      }
    }
    water[i][j] -= amount;
  }
}

//Adding One Rain Drop
void addOneRainDrop(int i, int j, float ** water) {
  water[i][j] += 1.0;
}

// Function to simulate a single time step
void simulateStep(float ** landscape,
                  float ** absorbed,
                  float ** water,
                  int N,
                  float A,
                  int timestep,
                  int M) {
  float ** trickleWater = allocate2DArray(N);

  // Absorption and trickle
  for (int i = 0; i < N; i++) {
    for (int j = 0; j < N; j++) {
      // Distribute raindrops to the Landscape if the time step is less than M
      if (timestep < M) {
        addOneRainDrop(i, j, water);
      }

      absorbRainDrops(i, j, A, absorbed, water);

      // Trickle only if there is greater than 1 water droplet
      if (water[i][j] >= 0.0) {
        trickleRainDrops(i, j, N, landscape, water, trickleWater);
      }
    }
  }

  //Second pass Adding Trickle water
  for (int i = 0; i < N; i++) {
    for (int j = 0; j < N; j++) {
      water[i][j] += trickleWater[i][j];
    }
  }

  free2DArray(trickleWater, N);
}

// Function to check if all water is absorbed
int allWaterAbsorbed(float ** water, int N) {
  for (int i = 0; i < N; i++) {
    for (int j = 0; j < N; j++) {
      if (water[i][j] > 0) {
        return 0;  // Water is still present
      }
    }
  }
  return 1;  // All water is absorbed
}

int main(int argc, char * argv[]) {
  if (argc != 5) {
    printf("Usage: ./rainfall_seq <M> <A> <N> <elevation_file>\n");
    return 1;
  }

  int M = atoi(argv[1]);
  float A = atof(argv[2]);
  int N = atoi(argv[3]);
  char * elevationFile = argv[4];
  struct timespec start, end;

  FILE * file = fopen(elevationFile, "r");
  if (!file) {
    printf("Error: Unable to open elevation file\n");
    return 1;
  }

  float ** landscape = allocate2DArray(N);
  float ** absorbed = allocate2DArray(N);
  float ** water = allocate2DArray(N);

  readLandscape(file, landscape, N);
  fclose(file);

  int timestep = 0;
  clock_gettime(CLOCK_REALTIME, &start);
  do {
    simulateStep(landscape, absorbed, water, N, A, timestep, M);
    timestep++;
  } while (!allWaterAbsorbed(water, N));
  clock_gettime(CLOCK_REALTIME, &end);

  double time_taken = end.tv_sec - start.tv_sec;
  time_taken += (end.tv_nsec - start.tv_nsec) / 1e9;

  printf("Rainfall simulation completed in %d time steps\n", timestep);
  printf("Runtime = %.7f seconds\n\n", time_taken);
  printf("The following grid shows the number of raindrops absorbed at each point:\n");
  for (int i = 0; i < N; i++) {
    for (int j = 0; j < N; j++) {
      if (j != 0) {
        printf(" ");
      }
      printf("%8.6g ", absorbed[i][j]);
    }
    printf("\n");
  }

  free2DArray(landscape, N);
  free2DArray(absorbed, N);
  free2DArray(water, N);
  return 0;
}
