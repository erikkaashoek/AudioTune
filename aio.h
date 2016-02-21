#ifndef __AIO_H__
#define __AIO_H__

typedef struct aio_handle
{
    PaDeviceIndex id;
    double rate;
    PaStream *stream;
    SAMPLE_TYPE *data;
} aio_t;

int aio_list_devices();
aio_t *aio_init(int id, double rate);
int aio_close(aio_t *aHdl);
int aio_acquire(aio_t *aHdl);

#endif /* __AIO_H__ */
