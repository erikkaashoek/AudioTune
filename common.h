#ifndef __COMMON_H__
#define __COMMON_H__

#define PA_SAMPLE_TYPE paFloat32
#define SAMPLE_TYPE float
#define MAX_FRAMES_PER_BUFFER 8192
#define FFT_K 13 /* 4096 == 2^12 */
#ifndef M_PI
#define M_PI 3.14159265358979323846264338327
#endif

/* utilities */
#define bitsof(x) (8*sizeof(x))

#ifdef DEBUG
  #define debug_printf(fmt, ...) do { fprintf(stderr, fmt , ##__VA_ARGS__); fflush(stderr); \
                                    } while (0)
#else
  #define debug_printf(...) ((void)0)
#endif
#define error_printf(fmt, ...) do { fprintf(stderr, fmt , ##__VA_ARGS__); fflush(stderr); \
                                  } while(0)

#define MIN(x,y) (((x)>(y))?(y):(x))
#define MAX(x,y) (((x)<(y))?(y):(x))

#endif /* __COMMON_H__ */
