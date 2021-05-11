#include <stdio.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include <stdlib.h>
#include <omp.h>

#ifndef NUMT
#define NUMT             4
#endif

void Deer();
void Grain();
void Watcher();
void MyAgent();

unsigned int seed = 0;

int	NowYear;		// 2021 - 2026
int	NowMonth;		// 0 - 11

float	NowPrecip;		// inches of rain per month
float	NowTemp;		// temperature this month
float	NowHeight;		// grain height in inches
int	NowNumDeer;		// number of deer in the current population

int NowNumPredator;

const float GRAIN_GROWS_PER_MONTH =		9.0;
const float ONE_DEER_EATS_PER_MONTH =		1.0;

const float AVG_PRECIP_PER_MONTH =		7.0;	// average
const float AMP_PRECIP_PER_MONTH =		6.0;	// plus or minus
const float RANDOM_PRECIP =			2.0;	// plus or minus noise

const float AVG_TEMP =				60.0;	// average
const float AMP_TEMP =				20.0;	// plus or minus
const float RANDOM_TEMP =			10.0;	// plus or minus noise

const float MIDTEMP =				40.0;
const float MIDPRECIP =				10.0;

/*
Units of grain growth are inches.
Units of temperature are degrees Fahrenheit (°F).
Units of precipitation are inches.
*/
float Ranf( unsigned int *seedp,  float low, float high );
//int Ranf( unsigned int *seedp, int ilow, int ihigh );


float SQR(float x);
void temp_month();
void Deer();
void Grain();
void Watcher();
void MyAgent();

// main program:
int
main( int argc, char *argv[ ] )
{
    #ifndef _OPENMP
        fprintf( stderr, "No OpenMP support!\n" );
        return 1;
    #endif

    temp_month();

    // starting date and time:
    NowMonth =    0;
    NowYear  = 2021;

    // starting state (feel free to change this if you want):
    NowNumDeer = 1;
    NowHeight =  1.;
    NowNumPredator = 1;
    /*year_month ,temp (c), precip (cm), height (cm), numDeer*/
    printf("Month,Temperature,Precip,Grain Height,Num Deer, Num Predator\n\n");

	omp_set_num_threads( 4 );	// set the number of threads to use in parallelizing the for-loop:`
    	// get ready to record the maximum performance and the probability:
	double maxPerformance = 0.;	// must be declared outside the NUMTRIES loop


    #pragma omp parallel sections
    {
            #pragma omp section
            {
                Deer( );
            }

            #pragma omp section
            {
                Grain( );
            }

            #pragma omp section
            {
                Watcher( );
            }

            #pragma omp section
            {
                MyAgent( );	// your own
            }
    }       // implied barrier -- all functions must return in order
        // to allow any of them to get past here

	return 0;
}

float Ranf( unsigned int *seedp,  float low, float high )
{
        float r = (float) rand_r( seedp );              // 0 - RAND_MAX

        return(   low  +  r * ( high - low ) / (float)RAND_MAX   );
}

/*
int Ranf( unsigned int *seedp, int ilow, int ihigh )
{
        float low = (float)ilow;
        float high = (float)ihigh + 0.9999f;

        return (int)(  Ranf(seedp, low,high) );
}
*/
float SQR(float x){
        return x*x;
}
void temp_month()
{
    float ang = (  30.*(float)NowMonth + 15.  ) * ( M_PI / 180. );

    float temp = AVG_TEMP - AMP_TEMP * cos( ang );
    NowTemp = temp + Ranf( &seed, -RANDOM_TEMP, RANDOM_TEMP );

    float precip = AVG_PRECIP_PER_MONTH + AMP_PRECIP_PER_MONTH * sin( ang );
    NowPrecip = precip + Ranf( &seed,  -RANDOM_PRECIP, RANDOM_PRECIP );
    if( NowPrecip < 0. )
        NowPrecip = 0.;
}

void Deer()
{
    while( NowYear < 2027 )
    {
        int addition_deer;
        // compute a temporary next-value for this quantity
        // based on the current state of the simulation:

        //If the number of deer exceeds the number of inches of height of the grain at the end of a month, decrease the number of deer by one.
        //If the number of deer is less than this value at the end of a month, increase the number of deer by one.
        // same value -> no change
        if(NowHeight < NowNumDeer)
			addition_deer = -1;
		else if(NowHeight > NowNumDeer)
			addition_deer = 1;
        else
            addition_deer = 0;


        // DoneComputing barrier:
        #pragma omp barrier
        NowNumDeer += addition_deer;

        // DoneAssigning barrier:
        #pragma omp barrier


        // DonePrinting barrier:
        #pragma omp barrier
    }
}
void Grain()
{
    while( NowYear < 2027 )
    {
        // compute a temporary next-value for this quantity
        // based on the current state of the simulation:

        float tempFactor = exp(-SQR(( NowTemp - MIDTEMP ) / 10.));
        float precipFactor = exp(-SQR(( NowPrecip - MIDPRECIP ) / 10.));

        float nextHeight = NowHeight;
        nextHeight += tempFactor * precipFactor * GRAIN_GROWS_PER_MONTH;
        nextHeight -= (float)NowNumDeer * ONE_DEER_EATS_PER_MONTH;
        if( nextHeight < 0. )
            nextHeight = 0.;

        // DoneComputing barrier:
        #pragma omp barrier
        NowHeight = nextHeight;

        // DoneAssigning barrier:
        #pragma omp barrier


        // DonePrinting barrier:
        #pragma omp barrier
    }
}
void Watcher()
{
    while( NowYear < 2027 )
    {
        // compute a temporary next-value for this quantity
        // based on the current state of the simulation:


        // DoneComputing barrier:
        #pragma omp barrier


        // DoneAssigning barrier:
        #pragma omp barrier
        int year_month = (NowYear-2021)*12 + NowMonth;
        // °C = (5./9.)*(°F-32) = (°F-32)/(9./5.)
        float grain_height_cm = NowHeight*2.54;
        float precip_cm = NowPrecip*2.54;

        float temp_c = (NowTemp-32.)/1.8;
        /*year_month ,temp (c), precip (cm), height (cm), numDeer, numPredator*/
        printf("%d,%lf,%lf,%lf,%d,%d\n",year_month, temp_c, precip_cm, grain_height_cm, NowNumDeer, NowNumPredator);

        // Update globals
        NowMonth++;
        if (NowMonth >= 12){
            ++NowYear;
            NowMonth = 0;
        }
        temp_month();
        // DonePrinting barrier:
        #pragma omp barrier

    }
}

void MyAgent()
{
    while( NowYear < 2027 )
    {
        // compute a temporary next-value for this quantity
        // based on the current state of the simulation:
        int addition_Predator;
        int decrease_deer=0;
        if(NowNumPredator!=0 and (NowNumDeer < NowNumPredator or NowPrecip*2.54 < 10.)){
            addition_Predator = -1;
        }/* well performance on sunny day*/
		else if(NowNumDeer>1 and (NowNumDeer > NowNumPredator and NowPrecip*2.54 > 10.)){
            decrease_deer = -1;
            addition_Predator = 1;
        }else{
            addition_Predator = 0;
        }


        // DoneComputing barrier:
        #pragma omp barrier
        NowNumPredator+= addition_Predator;
        NowNumDeer+=decrease_deer;

        // DoneAssigning barrier:
        #pragma omp barrier


        // DonePrinting barrier:
        #pragma omp barrier
    }
}