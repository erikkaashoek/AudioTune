#include <math.h>
#include <float.h>
#include "common.h"
#include "sigproc.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#define BUF_SIZE MAX_FRAMES_PER_BUFFER

void fft(float *p, struct FFT_OUT *f)
{
    struct FFT_IN omega[BUF_SIZE];
    unsigned int t,j,m,l,nv,s,i,n,k;
    float v,podd,qocc;

    n = BUF_SIZE;
    k = FFT_K;
    for(t=0; t<n; t++) {
        omega[t].x=cos(2.0*M_PI*t/(float)n);
        omega[t].y=sin(2.0*M_PI*t/(float)n);
    }
    for(t=0; t<n; t++) {
        m=t; s=0;
        for(i=0; i<k; i++) {
            j=(unsigned int)(m/2); s=2*s+(m-2*j);
            m=j;
        }
        f[t].a=p[s];
        f[t].b=0;
    }
    for(t=0; t<n-1; t+=2) {
        v=f[t].a; f[t].a=v+f[t+1].a;
        f[t+1].a=v-f[t+1].a;
    }
    m=n/2; nv=2;
    for(l=k-1; l>0; l--) {
        m=m/2; nv=2*nv;
        for(t=0; t<m*nv; t+=nv)
            for(j=0; j<nv/2; j++) {
                podd=omega[m*j].x*f[t+j+nv/2].a-omega[m*j].y*f[t+j+nv/2].b;
                qocc=omega[m*j].x*f[t+j+nv/2].b+omega[m*j].y*f[t+j+nv/2].a;
                f[t+j+nv/2].a=f[t+j].a-podd;
                f[t+j+nv/2].b=f[t+j].b-qocc;
                f[t+j].a=f[t+j].a+podd;
                f[t+j].b=f[t+j].b+qocc;
            }
    }
}

void fft2ps(struct FFT_OUT *f, float *ps_out, float *psmin, float *psmax, float *psavg)
{
    int i;
    float fftavg = 0.0;
    float fftmin = FLT_MAX;
    float fftmax = -FLT_MAX;
    
    for(i=0; i<BUF_SIZE; i++) {
        ps_out[i] = f[i].a * f[i].a + f[i].b * f[i].b;
        fftavg += ps_out[i];
        fftmax  = ps_out[i] > fftmax ? ps_out[i] : fftmax;
        fftmin  = ps_out[i] < fftmin ? ps_out[i] : fftmin;
    }
    fftavg /= (float)BUF_SIZE;
//    printf("fftavg = %f fft_max = %f fft_min = %f\n");
    *psmin = fftmin;
    *psmax = fftmax;
    *psavg = fftavg;
}

void rawstat(SAMPLE_TYPE *data, float *rawmin, float *rawmax, float *rawavg)
{
    int i;
    float avg = 0.0;
    float min = FLT_MAX;
    float max = -FLT_MAX;
    for(i=0; i<BUF_SIZE; i++) {
        avg += data[i];
        min = data[i] < min ? data[i] : min;
        max = data[i] > max ? data[i] : max;
    }
    avg /= (float)BUF_SIZE;
    for(i=0; i<BUF_SIZE; i++) {
        data[i] -= avg;
    }
    *rawmin = min - avg;
    *rawmax = max - avg;
    *rawavg = 0.0;
}

float psmainfreq(double rate, float *ps, int *index, float *max)
{
    int i;
    
    *index = 0;
    *max = -FLT_MAX;
    for(i=0; i<BUF_SIZE/2; i++) {
        if(ps[i] > *max) {
            *max = ps[i];
            *index = i;
        }
    }
    return rate/(double)BUF_SIZE * (*index);
}
