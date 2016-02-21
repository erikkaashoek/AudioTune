#ifndef __GRAPHICS_H__
#define __GRAPHICS_H__

typedef struct graphics_handle
{
    float psmin; /* power spectrum */
    float psmax;
    float psavg;
} graphics_t;

graphics_t *graphics_init(int argc, char **argv, SAMPLE_TYPE *data);
graphics_close(graphics_t *gHdl);

#endif /* __GRAPHICS_H__ */
