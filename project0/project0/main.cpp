#include <omp.h>
#include <stdio.h>
#include <math.h>

#ifndef NUMT
#define NUMT             4
#endif

#ifndef SIZE
#define SIZE             10000
#endif

#ifndef NUMTRIES
#define NUMTRIES         10
#endif


float A[SIZE];
float B[SIZE];
float C[SIZE];

int main()
{
     #ifndef _OPENMP
          fprintf(stderr, "OpenMP is not supported here -- sorry.\n");
          return 1;
     #endif

     // inialize the arrays:
     for (int i = 0; i < SIZE; i++)
     {
          A[i] = 1.;
          B[i] = 2.;
     }

     omp_set_num_threads(NUMT);
     fprintf(stderr, "Using %d threads\n", NUMT);

     double maxMegaMults = 0.;
     double summflops = 0.;

     for (int t = 0; t < NUMTRIES; t++)
     {
          // start time
          double time0 = omp_get_wtime();

          #pragma omp parallel for
          for (int i = 0; i < SIZE; i++)
          {
               C[i] = A[i] * B[i];
          }
          // end time
          double time1 = omp_get_wtime();
          // time1 - time0: time for processing time
          // megaMults: number of processes in 1 sec / 10^6
          double megaMults = (double)SIZE / (time1 - time0) / 1000000.;

          summflops += megaMults;
          if (megaMults > maxMegaMults)
               maxMegaMults = megaMults;
     }
     // more efficient, more MegaMults
     printf("Peak Performance = %8.2lf MegaMults/Sec\n", maxMegaMults);
     printf("Average Performance = %8.2lf MegaMults/Sec\n", summflops/(double)NUMTRIES );
     // note: %lf stands for "long float", which is how printf prints a "double"
     //        %d stands for "decimal integer", not "double"

     return 0;
}