#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <pthread.h>
#include <time.h>
#include <math.h>


pthread_mutex_t m;

struct params {
  int k;
  int max_k;
  int n_segm;
  int last_num;
  double len_segm;
  double res;
};
struct params p = {};



double func(double x)
{
  double f = sqrt(4 - x * x);
  return f;
}


int params_init(int num_segm, double segm, int n_threads, int last_num)
{
  p.k = 0;
  p.res = 0.0;
  p.n_segm = num_segm;
  p.len_segm = segm;
  p.max_k = n_threads - 1;
  p.last_num = last_num;
  return 0;
}


void* calc_part(void* a)
{
  int num = 0, num_s = 0, l_num = 0;
  double x = 0.0, res = 0.0, len_s = 0.0;

  pthread_mutex_lock (&m);
  num = p.k;
  //printf("%d\n", num);
  p.k++;
  num_s = p.n_segm;
  if (num == p.max_k)
    l_num = p.last_num;
  len_s = p.len_segm;
  pthread_mutex_unlock (&m);



  x = len_s * num * num_s;

  if (l_num > 0)
    for(int i = 0; i < l_num; i++)
    {
      if (x < 2)
        res += func(x);
      x += len_s;
    }
  else
    for(int i = 0; i < num_s; i++)
    {
      //if (x < 2)
      res += func(x);
      x += len_s;
    }


  pthread_mutex_lock (&m);
  p.res += res;
  pthread_mutex_unlock (&m);


  pthread_exit(NULL);
}

int calctime(struct timeval* start, struct timeval* fin)
{
  time_t sec1 = start->tv_sec;
  suseconds_t mcs1 = start->tv_usec;
  time_t sec2 = fin->tv_sec;
  suseconds_t mcs2 = fin->tv_usec;

  if(mcs2 < mcs1)
  {
    mcs2 += 1000000;
    sec2 -= 1;
  }

  printf("time of calculating %ld s %ld mcs\n", sec2 - sec1, mcs2 - mcs1);
  return 0;
}


int main(int argc, char* argv[])
{
  struct timeval start, fin;
  int i = 0, num_segm = 0, last_num = 0, s = 0;
  double res = 0.0, segm = 0.0, x = 0.0;

  if (argc != 3)
    perror("Missed arguments\n");

  int n = atoi(argv[1]);
  int n_threads = atoi(argv[2]);

  if (n <= 0)
    perror("Incorrect number of intervals of partition\n");
  if (n_threads <= 0)
    perror("Incorrect number of threads\n");
  if (n_threads > n)
    perror("Warning: there should be more threads\n");

  gettimeofday(&start, NULL);


  segm = 2 / (double)n;
  if ((n % n_threads) == 0)
  {
    num_segm = n / n_threads;
    last_num = num_segm;
  }
  else
  {
    num_segm = n / n_threads;
    last_num = num_segm + (n % n_threads);
  }

  params_init(num_segm, segm, n_threads, last_num);


  pthread_t t[n_threads];
  pthread_mutex_init (&m , NULL);
    for (i = 0; i < n_threads; i++)
    {
        pthread_create(&t[i], NULL, calc_part, NULL);

    }
    for (i = 0; i < n_threads; i++)
    {
        pthread_join(t[i], NULL);
    }


  pthread_mutex_destroy (&m);

  res = p.res;
  res -= func(0.0) / 2.0;
  res *= segm;
  printf("%lf\n", res);


  gettimeofday(&fin, NULL);
  calctime(&start, &fin);
  return 0;
}
