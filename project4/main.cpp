#include <xmmintrin.h>
#include <omp.h>        // for getting the timing
#include <stdio.h>

#define SSE_WIDTH		4

#ifndef NUMT
#define NUMT             4
#endif

#ifndef SIZE
#define SIZE             5000000        // 5M
#endif

#ifndef NUMTRIES
#define NUMTRIES         10
#endif

float A[SIZE];
float B[SIZE];
float C[SIZE];

void fill_array(float *a, int len)
{
    for(int i = 0; i < len; ++i){
        a[i]=i;
    }
}

void
SimdMul( float *a, float *b, float *c, int len )
{
    int limit = ( len/SSE_WIDTH ) * SSE_WIDTH;
    __asm
    (
        ".att_syntax\n\t"
        "movq -24(%rbp), %r8\n\t" // a
        "movq -32(%rbp), %rcx\n\t" // b
        "movq -40(%rbp), %rdx\n\t" // c
    );
    for( int i = 0; i < limit; i += SSE_WIDTH )
    {
        __asm
        (
            ".att_syntax\n\t"
            "movups (%r8), %xmm0\n\t" // load the first sse register
            "movups (%rcx), %xmm1\n\t" // load the second sse register
            "mulps %xmm1, %xmm0\n\t" // do the multiply
            "movups %xmm0, (%rdx)\n\t" // store the result
            "addq $16, %r8\n\t"
            "addq $16, %rcx\n\t"
            "addq $16, %rdx\n\t"
        );
    }
    for( int i = limit; i < len; i++ )
    {
        c[ i ] = a[ i ] * b[ i ];
    }
}


float
SimdMulSum( float *a, float *b, int len )
{
    float sum[4] = { 0., 0., 0., 0. };
    int limit = ( len/SSE_WIDTH ) * SSE_WIDTH;
    __asm
    (
        ".att_syntax\n\t"
        "movq -40(%rbp), %r8\n\t" // a
        "movq -48(%rbp), %rcx\n\t" // b
        "leaq -32(%rbp), %rdx\n\t" // &sum[0]
        "movups (%rdx), %xmm2\n\t" // 4 copies of 0. in xmm2
    );
    for( int i = 0; i < limit; i += SSE_WIDTH )
    {
        __asm
        (
            ".att_syntax\n\t"
            "movups (%r8), %xmm0\n\t" // load the first sse register
            "movups (%rcx), %xmm1\n\t" // load the second sse register
            "mulps %xmm1, %xmm0\n\t" // do the multiply
            "addps %xmm0, %xmm2\n\t" // do the add
            "addq $16, %r8\n\t"
            "addq $16, %rcx\n\t"
        );
    }
    __asm
    (
        ".att_syntax\n\t"
        "movups %xmm2, (%rdx)\n\t" // copy the sums back to sum[ ]
    );
    for( int i = limit; i < len; i++ )
    {
        sum[0] += a[ i ] * b[ i ];
    }
    return sum[0] + sum[1] + sum[2] + sum[3];
}

void CppMul( float *a, float *b,   float *c,   int len )
{
    for(int i = 0; i < len; i++){
        c[i] = a[i] * b[i];
    }
}
float CppMulSum( float *a, float *b, int len )
{
    float sum = 0;
    for(int i = 0; i < len; ++i){
        sum += a[i]*b[i];
    }

}

int main()
{
    // for getting the timing
    #ifndef _OPENMP
          fprintf(stderr, "OpenMP is not supported here -- sorry.\n");
          return 1;
    #endif

    fill_array(A,SIZE);
    fill_array(B,SIZE);
    fill_array(C,SIZE);

    double SimdMul_performance = 0., SimdMulSum_performance = 0., CppMul_performance = 0., CppMulSum_performance = 0.;
    double time0,time1;
    for(int t = 0; t < NUMTRIES; t++){

        time0 = omp_get_wtime();
        CppMulSum(A,B,SIZE);
        time1 = omp_get_wtime();
        if(CppMulSum_performance < (double)SIZE / (time1 - time0) / 1000000.){
            CppMulSum_performance = (double)SIZE / (time1 - time0) / 1000000.;
        }

        time0 = omp_get_wtime();
        SimdMulSum(A,B,SIZE);
        time1 = omp_get_wtime();

        if(SimdMulSum_performance < (double)SIZE / (time1 - time0) / 1000000.){
            SimdMulSum_performance = (double)SIZE / (time1 - time0) / 1000000.;
        }

        time0 = omp_get_wtime();
        CppMul(C,A,B,SIZE);
        time1 = omp_get_wtime();

        if(CppMul_performance < (double)SIZE / (time1 - time0) / 1000000.){
            CppMul_performance = (double)SIZE / (time1 - time0) / 1000000.;
        }

        fill_array(C,SIZE);

        time0 = omp_get_wtime();
        SimdMul(C,A,B,SIZE);
        time1 = omp_get_wtime();

        if(SimdMul_performance < (double)SIZE / (time1 - time0) / 1000000.){
            SimdMul_performance = (double)SIZE / (time1 - time0) / 1000000.;
        }
    }
    double Speedup_NonReduction = SimdMul_performance/CppMul_performance;
    double Speedup_Reduction = SimdMulSum_performance/CppMulSum_performance;

    printf("%d,%8.2lf,%8.2lf\n",SIZE, Speedup_Reduction, Speedup_NonReduction);

}