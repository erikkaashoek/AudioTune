#ifndef __SIGPROC_H__
#define __SIGPROC_H__

struct FFT_IN {
    float x;
    float y;
};
  
struct FFT_OUT {
    float a;
    float b;
};

void fft(float *p, struct FFT_OUT *f);
void fft2ps(struct FFT_OUT *f, float *ps_out, float *psmin, float *psmax, float *psavg);
void rawstat(SAMPLE_TYPE *data, float *rawmin, float *rawmax, float *rawavg);
float psmainfreq(double rate, float *ps, int *index, float *max);

#endif /* __SIGPROC_H__ */
