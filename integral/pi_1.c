#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <math.h>

double func(double x)
{
  double res = sqrt(4 - x * x);
  return res;
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
  double res = 0.0, segm = 0.0, x = 0.0;

  if (argc != 2)
    perror("Missed arguments\n");

  int n = atoi(argv[1]);
  if (n <= 0)
    perror("Incorrect number of intervals of partition\n");

  gettimeofday(&start, NULL);

  segm = 2 / (double)n;
  x = segm;
  for (int i = 1; i < n; i++) {
    res += func(x);
    x += segm;
  }

  res += (func(0.0) + func(2.0)) / 2.0;
  res *= segm;
  printf("%lf\n", res);

  gettimeofday(&fin, NULL);
  calctime(&start, &fin);
}
