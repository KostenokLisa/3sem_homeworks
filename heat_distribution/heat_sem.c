#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <pthread.h>
#include <time.h>
#include <semaphore.h>

#define MAX_STEP 1000
#define MAX_SEM 2000


pthread_mutex_t m;


struct params {
  int k;
  int max_k;
  int n_steps;
  int M;
  int N;
};
struct params thread_params = {};


double heat_array[MAX_STEP][MAX_STEP] = {};
sem_t sem[MAX_SEM] = {};


int params_init(int P, int n_steps, int M, int N)
{
  thread_params.k = 0;
  thread_params.max_k = P - 1;
  thread_params.n_steps = n_steps;
  thread_params.M = M;
  thread_params.N = N;
  return 0;
}


int arr_init(int N)
{
  for(int i = 0; i < MAX_STEP; i++)
    for(int j = 0; j < MAX_STEP; j++)
      if (i == N)
        heat_array[i][j] = 1.0;
      else
        heat_array[i][j] = 0.0;
  return 0;
}

double func(double u1, double u2, double u3)
{
  return u2 + 0.3 * (u3 - 2 * u2 + u1);
}

void* calc_heat(void* a)
{
  int num = 0, num_s = 0, P = 0, l_num = 0, N = 0, M = 0, start_idx = 0;

  pthread_mutex_lock (&m);
  num = thread_params.k;
  thread_params.k++;
  num_s = thread_params.n_steps;
  P = thread_params.max_k + 1;
  M = thread_params.M;
  N = thread_params.N;
  pthread_mutex_unlock (&m);

  for(int i = 1; i <= M; i++)
  {

    start_idx = num * num_s;
    if ((i >= 2) && (num > 0))
      sem_wait(&sem[2 * (num - 1)]);

    if ((i >= 2) && (num < P - 1))
      sem_wait(&sem[2 * num + 1]);

    if ((num == P - 1) && (P != 1))
    {
      for(int j = start_idx; j < N; j++)
        heat_array[j][i] = func(heat_array[j - 1][i - 1], heat_array[j][i - 1], heat_array[j + 1][i - 1]);
    }
    else
    {
      for(int j = start_idx; j < start_idx + num_s; j++)
        if (j > 0)
          heat_array[j][i] = func(heat_array[j - 1][i - 1], heat_array[j][i - 1], heat_array[j + 1][i - 1]);
    }

    if (num > 0)
      sem_post(&sem[2 * num - 1]);

    if (num < P - 1)
      sem_post(&sem[2 * num]);
    //pthread_barrier_wait (&b);
  }

  pthread_exit(NULL);
}

int print_arr(int N, double h, int M)
{
  double cur_l;
  FILE* dest = fopen("res1.txt", "w");

  for(int i = 0; i < N + 1; i++)
  {
    cur_l = i * h;
    fprintf(dest, "%lf %lf\n", cur_l, heat_array[i][M]);
  }

  /*for(int i = 0; i < N + 1; i++)
  {
    for (int j = 0; j < M + 1; j++)
      fprintf(dest, "%lf ", heat_array[i][j]);
    fprintf(dest, "\n");
  }*/
  fclose(dest);
  return 0;
}

int main(int argc, char* argv[])
{
  int i = 0, n_steps = 0, last_n = 0;


  if (argc != 4)
    perror("Missed arguments\n");

  int N = atoi(argv[1]);
  double T = atof(argv[2]);
  int P = atoi(argv[3]);

  double h = 1.0 / N;
  double t = 0.3 * h * h;
  int M = (int)(T / t);
  //printf("%d", M);

  n_steps = N / P;

  params_init(P, n_steps, M, N);
  arr_init(N);

  pthread_t thr[P];
  pthread_mutex_init (&m, NULL);

  for (i = 0; i < (2 * P - 2); i++)
  {
      sem_init(&sem[i], 0, 0);
  }

  for (i = 0; i < P; i++)
  {
      pthread_create(&thr[i], NULL, calc_heat, NULL);

  }
  for (i = 0; i < P; i++)
  {
      pthread_join(thr[i], NULL);
  }


  pthread_mutex_destroy (&m);

  print_arr(N, h, M);
}
