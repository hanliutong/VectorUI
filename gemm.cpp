#include "intrin/intrin_rvv_fixed.hpp"
#include <stdio.h>
#include <algorithm>
#define M 3
#define N 3
#define K 16

float matA[] = {1,2,3,4,5,6,7,8,9};
float matB[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10,11,12,13,14,15,16,
                17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,
                33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48};
float matC[M*K] = {0};
float matC_golden[M*K] = {0};

void goldenGEMM( const float* aptr, size_t astep, const float* bptr,
               size_t bstep, float* cptr, size_t cstep,
               int ma, int na, int nb )
{
    int n = 0;
    for( ; n < nb; n++ )
    {
        for( int m = 0; m < ma; m++ )
        {
            const float* aptr0 = aptr + astep*m;
            float* cptr0 = cptr + cstep*m;
            float d0 = 0.f;

            for( int k = 0; k < na; k++ )
                d0 += aptr0[k]*bptr[k*bstep + n];

            cptr0[n] = d0;
        }
    }
}

void GEMM( const float* aptr, size_t astep, const float* bptr,
               size_t bstep, float* cptr, size_t cstep,
               int ma, int na, int nb ) {
    int n = 0;
    int vl = v_float32::nlanes;
    int mvl0 = vl;

    int loopCnt = 0;
    for( ; n < nb; n += vl )
    {
        loopCnt++;
        if ( n + vl > nb) {
            mvl0 = nb - n;
        }

        for( int m = 0; m < ma; m++ )
        {
            const float* aptr0 = aptr + astep*m;
            float* cptr0 = cptr + cstep*m;
            v_float32 d00 = v_setzero_f32();
            for( int k = 0; k < na; k++ )
            {
                float a0 = aptr0[k];
                v_float32 b0 = v_load(bptr + k*bstep + n, mvl0);
                d00 = v_fma(a0, b0, d00, mvl0);
            }
            v_store(cptr0 + n, d00, mvl0);
        }
    }
    printf("f32:nlanes=%d;\tloop %d time(s)\n", v_float32::nlanes, loopCnt);
}

int main() {
    int astep = N, bstep = K, cstep = K;
    goldenGEMM(matA, astep, matB, bstep, matC_golden, cstep, M, N, K);
    GEMM(matA, astep, matB, bstep, matC, cstep, M, N, K);
    bool pass = true;
    for(int i = 0; i < M * K; ++i) {
        pass &= matC[i] == matC_golden[i];
    }
    printf("%s\n", pass ? "PASS" : "FAIL");
    if (!pass) {
        printf("output:\n");
        for(int i = 0; i < M * K; ++i) {
            printf("%.1f%s", matC[i], i%K < K-1 ? ", " : "\n");
        }
        printf("\ndiff:\n");
        for(int i = 0; i < M * K; ++i) {
            printf("%.1f%s", matC_golden[i]-matC[i], i%K < K-1 ? ", " : "\n");
        }
        printf("\n");
    }

    return 0;
}
