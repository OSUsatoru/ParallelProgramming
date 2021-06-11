#include <stdio.h>
#include <math.h>
#include <mpi.h>

// which node is in charge?

#define BOSS 0

// files to read and write:

#define BIGSIGNALFILEBIN	(char*)"bigsignal.bin"
#define BIGSIGNALFILEASCII	(char*)"bigsignal.txt"
#define CSVPLOTFILE		    (char*)"plot.csv"
#define CSVPERFORMANCE		(char*)"performance.csv"

// tag to "scatter":

#define TAG_SCATTER		'S'

// tag to "gather":

#define TAG_GATHER		'G'


// how many elements are in the big signal:

#define NUMELEMENTS	(8*1024*1024)

// only do this many shifts, not all NUMELEMENTS of them (this is enough to uncover the secret sine wave):

#define MAXSHIFTS	1024

// how many autocorrelation sums to plot:

#define MAXPLOT		 256		// an excel limit

// pick which file type to read, BINARY or ASCII (BINARY is much faster to read):
// (pick one, not both)

#define BINARY
//#define ASCII

// print debugging messages?

#define DEBUG		false

// globals:

float * BigSums;		// the overall MAXSHIFTS autocorrelation array
float *	BigSignal;		// the overall NUMELEMENTS-big signal data
int	NumCpus;		// total # of cpus involved
float * PPSums;			// per-processor autocorrelation sums
float *	PPSignal;		// per-processor local array to hold the sub-signal
int	PPSize;			// per-processor local array size

// function prototype:

void	DoOneLocalAutocorrelation( int );


int
main( int argc, char *argv[ ] )
{
	MPI_Status status;

	MPI_Init( &argc, &argv );

	MPI_Comm_size( MPI_COMM_WORLD, &NumCpus );
	int  me;		// which one I am
	MPI_Comm_rank( MPI_COMM_WORLD, &me );

	// decide how much data to send to each processor:

	PPSize    = NUMELEMENTS / NumCpus;		// assuming it comes out evenly

	// local arrays:

	PPSignal  = new float [PPSize+MAXSHIFTS];	// per-processor local signal
	PPSums    = new float [MAXSHIFTS];		// per-processor local sums of the products

	// read the BigSignal array:

	if( me == BOSS )	// this is the big-data-owner
	{
		BigSignal = new float [NUMELEMENTS+MAXSHIFTS];		// so we can duplicate part of the array

#ifdef ASCII
		FILE *fp = fopen( BIGSIGNALFILEASCII, "r" );
		if( fp == NULL )
		{
			fprintf( stderr, "Cannot open data file '%s'\n", BIGSIGNALFILEASCII );
			return -1;
		}

		for( int i = 0; i < NUMELEMENTS; i++ )
		{
			float f;
			fscanf( fp, "%f", &f );
			BigSignal[i] = f;
		}
#endif

#ifdef BINARY
		FILE *fp = fopen( BIGSIGNALFILEBIN, "rb" );
		if( fp == NULL )
		{
			fprintf( stderr, "Cannot open data file '%s'\n", BIGSIGNALFILEBIN );
			return -1;
		}

		fread( BigSignal, sizeof(float), NUMELEMENTS, fp );
#endif

		// duplicate part of the array:

		for( int i = 0; i < MAXSHIFTS; i++ )
		{
			BigSignal[NUMELEMENTS+i] = BigSignal[i];
		}
	}

    /* here noise*/

	// create the array to hold all the sums:

	if( me == BOSS )
	{
		BigSums = new float [MAXSHIFTS];	// where all the sums will go
	}

	// start the timer:

	double time0 = MPI_Wtime( );

	// have the BOSS send to itself (really not a "send", just a copy):

	if( me == BOSS )
	{
		for( int i = 0; i < PPSize+MAXSHIFTS; i++ )
		{
			PPSignal[i] = BigSignal[ BOSS*PPSize + i ];
		}
	}

	// have the BOSS send to everyone else:

	if( me == BOSS )
	{
		for( int dst = 0; dst < NumCpus; dst++ )
		{
			if( dst == BOSS )
				continue;
            /*The tag to send (GATHER)*/
			MPI_Send( &BigSignal[dst*PPSize], PPSize, MPI_FLOAT, dst, TAG_GATHER, MPI_COMM_WORLD );
		}
	}
	else
	{
		MPI_Recv( PPSignal, PPSize, MPI_FLOAT, BOSS, TAG_GATHER, MPI_COMM_WORLD, &status );
	}

	// each processor does its own autocorrelation:

	DoOneLocalAutocorrelation( me );

	// each processor sends its sums back to the BOSS:

	if( me == BOSS )
	{
		for( int s = 0; s < MAXSHIFTS; s++ )
		{
			BigSums[s] = PPSums[s];		// start the overall sums with the BOSS's sums
		}
	}
	else
	{
        /*The tag to send (BOSS)*/
		MPI_Send( PPSums, MAXSHIFTS, MPI_FLOAT, BOSS, TAG_SCATTER, MPI_COMM_WORLD );
	}

	// receive the sums and add them into the overall sums:

	if( me == BOSS )
	{
		float tmpSums[MAXSHIFTS];
		for( int src = 0; src < NumCpus; src++ )
		{
			if( src == BOSS )
				continue;

			MPI_Recv( tmpSums, MAXSHIFTS, MPI_FLOAT, src, TAG_SCATTER, MPI_COMM_WORLD, &status );
			for( int s = 0; s < MAXSHIFTS; s++ )
				BigSums[s] += tmpSums[s];
		}
	}

	// stop the timer:

	double time1 = MPI_Wtime( );

	// print the performance:

	if( me == BOSS )
	{
		double seconds = time1 - time0;
		double performance = (double)MAXSHIFTS*(double)NUMELEMENTS/seconds/1000000.;	// mega-elements computed per second

        //fprintf( stderr, "%3d processors, %10d elements, %9.2lf mega-autocorrelations computed per second\n", NumCpus, NUMELEMENTS, performance );
        fprintf( stderr, "%3d, %9.2lf\n",
			NumCpus, performance );
	}

	// write the file to be plotted to look for the secret sine wave:

	if( me == BOSS )
	{
		FILE *fp = fopen( CSVPLOTFILE, "w" );
		if( fp == NULL )
		{
			fprintf( stderr, "Cannot write to plot file '%s'\n", CSVPLOTFILE );
		}
		else
		{
			for( int s = 1; s < MAXPLOT; s++ )		// BigSums[0] is huge -- don't use it
			{
				fprintf( fp, "%6d , %10.2f\n", s, BigSums[s] );
			}
			fclose( fp );
		}
	}

	// all done:

	MPI_Finalize( );
	return 0;
}


// read from the per-processor signal array, write into the local sums array:

void
DoOneLocalAutocorrelation( int me )
{
	MPI_Status status;

	if( DEBUG )	fprintf( stderr, "Node %3d entered DoOneLocalAutocorrelation( )\n", me );

	for( int s = 0; s < MAXSHIFTS; s++ )
	{
		float sum = 0.;
		for( int i = 0; i < PPSize; i++ )
		{
			sum += PPSignal[i] * PPSignal[i+s];
		}
		PPSums[s] = sum;
	}
}