/**
 * citations
 * OpenAI. (n.d.). ChatGPT. OpenAI.
*/

#include <math.h>
#include <pthread.h>
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

pthread_mutex_t * initializeRowMutexes(int N) {
  pthread_mutex_t * rowMutexes = malloc(N * sizeof(pthread_mutex_t));
  for (int i = 0; i < N; i++) {
    pthread_mutex_init(&rowMutexes[i], NULL);
  }
  return rowMutexes;
}

void destroyRowMutexes(pthread_mutex_t * rowMutexes, int N) {
  for (int i = 0; i < N; i++) {
    pthread_mutex_destroy(&rowMutexes[i]);
  }
  free(rowMutexes);
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

typedef struct {
  int startRow;
  pthread_mutex_t startRowMutex;
  int endRow;
  pthread_mutex_t endRowMutex;
  int M;
  int N;
  int timestep;
  float A;
  float ** landscape;
  float ** absorbed;
  float ** water;
  float ** trickleWater;
} ThreadData;

void * simulatePortion(void * arg) {
  ThreadData * data = (ThreadData *)arg;
  float ** landscape = data->landscape;
  float ** absorbed = data->absorbed;
  float ** water = data->water;
  float ** trickleWater = data->trickleWater;
  int N = data->N;
  float A = data->A;
  int timestep = data->timestep;
  int M = data->M;

  // Absorption and trickle
  for (int i = data->startRow; i <= data->endRow; i++) {
    for (int j = 0; j < N; j++) {
      // Distribute raindrops to the Landscape if the time step is less than M
      if (timestep < M) {
        addOneRainDrop(i, j, water);
      }

      absorbRainDrops(i, j, A, absorbed, water);

      // Trickle only if there is greater than 1 water droplet
      if (water[i][j] >= 0.0) {
        //Locking if it is the first/last row of the section since trickle data is shared between different section
        if (i == data->startRow) {
          pthread_mutex_lock(&data->startRowMutex);
        }
        else if (i == data->endRow) {
          pthread_mutex_lock(&data->endRowMutex);
        }

        trickleRainDrops(i, j, N, landscape, water, trickleWater);

        //Locking if it is the first/last row of the section since trickle data is shared between different section
        if (i == data->startRow) {
          pthread_mutex_unlock(&data->startRowMutex);
        }
        else if (i == data->endRow) {
          pthread_mutex_unlock(&data->endRowMutex);
        }
      }
    }
  }
}

// Function to simulate a single time step
void simulateStep(float ** landscape,
                  float ** absorbed,
                  float ** water,
                  int N,
                  float A,
                  int timestep,
                  int M,
                  pthread_mutex_t * rowMutexes,
                  int P) {
  float ** trickleWater = allocate2DArray(N);

  int no_thread = P;
  if (N < P) {
    no_thread =
        N;  // if no of rows is less than no of threads then we are only using min no of threads
  }

  pthread_t threads[no_thread];
  ThreadData threadData[no_thread];

  int rowsPerThread = N / no_thread;

  // Create threads
  for (int i = 0; i < no_thread; i++) {
    threadData[i].startRow = i * rowsPerThread;
    threadData[i].startRowMutex = rowMutexes[i];
    threadData[i].endRow = (i + 1) * rowsPerThread - 1;
    if (i == no_thread - 1) {
      threadData[i].endRow = N - 1;  // Handle last thread boundary
    }
    threadData[i].endRowMutex = rowMutexes[i + 1];
    threadData[i].N = N;
    threadData[i].M = M;
    threadData[i].timestep = timestep;
    threadData[i].A = A;
    threadData[i].landscape = landscape;
    threadData[i].absorbed = absorbed;
    threadData[i].water = water;
    threadData[i].trickleWater = trickleWater;

    pthread_create(&threads[i], NULL, simulatePortion, &threadData[i]);
  }

  // Join threads
  for (int i = 0; i < no_thread; i++) {
    pthread_join(threads[i], NULL);
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
  if (argc != 6) {
    printf("Usage: ./rainfall_pt <P> <M> <A> <N> <elevation_file>\n");
    return 1;
  }

  int P = atoi(argv[1]);
  int M = atoi(argv[2]);
  float A = atof(argv[3]);
  int N = atoi(argv[4]);
  char * elevationFile = argv[5];
  struct timespec start, end;

  FILE * file = fopen(elevationFile, "r");
  if (!file) {
    printf("Error: Unable to open elevation file\n");
    return 1;
  }

  float ** landscape = allocate2DArray(N);
  float ** absorbed = allocate2DArray(N);
  float ** water = allocate2DArray(N);
  pthread_mutex_t * rowMutexes = initializeRowMutexes(P + 1);

  readLandscape(file, landscape, N);
  fclose(file);

  int timestep = 0;
  clock_gettime(CLOCK_REALTIME, &start);
  do {
    simulateStep(landscape, absorbed, water, N, A, timestep, M, rowMutexes, P);
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
  destroyRowMutexes(rowMutexes, P + 1);
  return 0;
}
