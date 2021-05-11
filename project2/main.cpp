#include <omp.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>


// setting the number of threads:
#ifndef NUMT
#define NUMT		    2
#endif

// setting the number of subdivisions in the monte carlo simulation:
#ifndef NUMNODES
#define NUMNODES	16
#endif


#define XMIN     -1.
#define XMAX      1.
#define YMIN     -1.
#define YMAX      1.

#define N	0.70

float Height( int, int );	// function prototype

int main( int argc, char *argv[ ] )
{
    #ifndef _OPENMP
        fprintf(stderr, "OpenMP is not supported here -- sorry.\n");
        return 1;
    #endif
    omp_set_num_threads( NUMT );	// set the number of threads to use in parallelizing the for-loop:`

	// the area of a single full-sized tile
	// (not all tiles are full-sized, though)

	float fullTileArea = (  ( ( XMAX - XMIN )/(float)(NUMNODES-1) )  *
				( ( YMAX - YMIN )/(float)(NUMNODES-1) )  );

	// sum up the weighted heights into the variable "volume"
    float Volume = 0;
	// using an OpenMP for loop and a reduction:

    double time0 = omp_get_wtime( );

    #pragma omp parallel for collapse(2),default(none),shared(fullTileArea),reduction(+:Volume)
    for( int iv = 0; iv < NUMNODES; iv++ )
    {
        for( int iu = 0; iu < NUMNODES; iu++ )
        {
            float tmp = fullTileArea * Height(iu, iv);
            if(iv == 0 or iv == NUMNODES-1){
                tmp*=0.5;
            }
            if(iu==0 or iu==NUMNODES-1){
                tmp*=0.5;
            }
            Volume+=tmp;
        }
    }

    double time1 = omp_get_wtime( );
	double megaNUMNODESPerSecond = ((double)NUMNODES*(double)NUMNODES) / ( time1 - time0 ) / 1000000.;
    fprintf(stderr, "%2d threads : %8d NUMNODES ; Volume = %6.2f ; megaNumnodes/sec = %6.2lf\n",
    NUMT, NUMNODES, Volume*2., megaNUMNODESPerSecond);
    return 0;
}

float Height( int iu, int iv )	// iu,iv = 0 .. NUMNODES-1
{
	float x = -1.  +  2.*(float)iu /(float)(NUMNODES-1);	// -1. to +1.
	float y = -1.  +  2.*(float)iv /(float)(NUMNODES-1);	// -1. to +1.

	float xn = pow( fabs(x), (double)N );
	float yn = pow( fabs(y), (double)N );
	float r = 1. - xn - yn;
	if( r <= 0. )
	        return 0.;
	float height = pow( r, 1./(float)N );
	return height;
}