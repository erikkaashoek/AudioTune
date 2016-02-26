#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>
#include <math.h>
#include <unistd.h>
#include <portaudio.h>
#ifdef WIN32
  #include <windows.h>
  #if PA_USE_ASIO
    #include <pa_asio.h>
  #endif
#endif
#include "common.h"
#include "aio.h"

static int callback(const void *input, void *output, unsigned long frameCount,
                    const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags,
                    void *userData)
{
    aio_t *aHdl = (aio_t*)userData;
    SAMPLE_TYPE *od;
    long i, j;
    double t;
    static double phaset = 0;
    
    if(aHdl->io) { /* input */
        const SAMPLE_TYPE *indata = (const SAMPLE_TYPE*)input;
        memcpy(aHdl->data, indata, sizeof(SAMPLE_TYPE) * MIN(frameCount, MAX_FRAMES_PER_BUFFER));
    } else { /* output */
        od = (SAMPLE_TYPE*)output;
        for(i=0; i<frameCount; i++) {
            t = (phaset + aHdl->freq/aHdl->rate * i);
            od[i] = 1.0*sin(2.0*M_PI * t);
        }
        t = (phaset + aHdl->freq/aHdl->rate * i); /* i passes maximum */
        j = (long)t;
        phaset = t - (double)j;
        memcpy(aHdl->data, output, sizeof(SAMPLE_TYPE) * MIN(frameCount, MAX_FRAMES_PER_BUFFER));
    }

    return paContinue;
}

aio_t *aio_init(int id, double rate, int io)
{
    aio_t *aHdl;
    PaError err;
    PaDeviceIndex nd;
    const PaDeviceInfo *devInfo;
    PaStreamParameters inputParam, *iP=NULL, outputParam, *oP=NULL;

    err = Pa_Initialize();
    if( err != paNoError ) {
        error_printf("Pa_Initialize returned 0x%x\n", err);
        return NULL;
    }

    printf("PortAudio version number = %d\nPortAudio version text = '%s'\n",
           Pa_GetVersion(), Pa_GetVersionText());

    nd = Pa_GetDeviceCount();
    if(nd < 0) {
        error_printf("Pa_GetDeviceCount() returned 0x%x\n", nd);
        err = nd;
        return NULL;
    }
    if(id < 0) id = Pa_GetDefaultInputDevice();
    devInfo = Pa_GetDeviceInfo(id);
    if(rate < 0.0) rate = devInfo->defaultSampleRate;

    aHdl = malloc(sizeof(aio_t));
    aHdl->io = io;
    aHdl->id = id;
    aHdl->rate = rate;
    aHdl->data = (SAMPLE_TYPE*)malloc(sizeof(SAMPLE_TYPE) * MAX_FRAMES_PER_BUFFER);

    inputParam.device                    = aHdl->id;
    inputParam.channelCount              = 1;
    inputParam.sampleFormat              = PA_SAMPLE_TYPE;
    inputParam.suggestedLatency          = devInfo->defaultLowInputLatency;
    inputParam.hostApiSpecificStreamInfo = NULL;
    if(io) /* input */ iP = &inputParam;
    
    outputParam.device                    = aHdl->id;
    outputParam.channelCount              = 1;
    outputParam.sampleFormat              = PA_SAMPLE_TYPE;
    outputParam.suggestedLatency          = devInfo->defaultLowOutputLatency;
    outputParam.hostApiSpecificStreamInfo = NULL;
    if(io == 0) /* output */ oP = &outputParam;
    
    err = Pa_OpenStream(&(aHdl->stream),
                        iP,
                        oP,
                        aHdl->rate,
                        MAX_FRAMES_PER_BUFFER, //paFramesPerBufferUnspecified,
                        paClipOff,
                        callback,
                        aHdl);
    if(err != paNoError)
        error_printf("%s\n", Pa_GetErrorText(err));
    printf("Stream opened on device %d, sample rate %g\n", aHdl->id, aHdl->rate);
    return aHdl;
}

int aio_startstream(aio_t *aHdl)
{
    PaError err;
    
    err = Pa_StartStream(aHdl->stream);
    if(err != paNoError)
        error_printf("%s\n", Pa_GetErrorText(err));
    return err;
}

int aio_close(aio_t *aHdl)
{
    PaError err;

    err = Pa_CloseStream(aHdl->stream);
    if(err != paNoError)
        error_printf("%s\n", Pa_GetErrorText(err));
    err = Pa_Terminate();
    if(err != paNoError)
        error_printf("%s\n", Pa_GetErrorText(err));
    if(aHdl) {
        free(aHdl->data);
        free(aHdl);
    }
    return err;
}

int aio_acquire(aio_t *aHdl)
{
    PaError err;

    err = Pa_ReadStream(aHdl->stream, aHdl->data, MAX_FRAMES_PER_BUFFER);
    if(err != paNoError)
        error_printf("%s\n", Pa_GetErrorText(err));
    return err;
}

static void PrintSupportedStandardSampleRates(
    const PaStreamParameters *inputParam,
    const PaStreamParameters *outputParam)
{
    static double standardSampleRates[] = {
        8000.0, 9600.0, 11025.0, 12000.0, 16000.0, 22050.0, 24000.0, 32000.0,
        44100.0, 48000.0, 88200.0, 96000.0, 176400.0, 192000.0, 352800.0, 384000.0,
        2822400.0, 5644800.0, -1 /* negative terminated list */
    };
    int i;
    PaError err;

    for(i=0; standardSampleRates[i] > 0; i++) {
        err = Pa_IsFormatSupported(inputParam, outputParam, standardSampleRates[i]);
        if(err == paFormatIsSupported) {
            printf( " %.1f", standardSampleRates[i] );
        }
    }
    printf("\n");
}

int aio_list_devices(void)
{
    PaError err;
    PaDeviceIndex id, nd;
    const PaDeviceInfo *devInfo;
    PaStreamParameters inputParam, outputParam;
    int defaultDisplayed;

    err = Pa_Initialize();
    if(err != paNoError) {
        error_printf("Pa_Initialize returned 0x%x\n", err);
        return err;
    }

    printf("PortAudio version number = %d\nPortAudio version text = '%s'\n",
           Pa_GetVersion(), Pa_GetVersionText());

    nd = Pa_GetDeviceCount();
    if(nd < 0) {
        error_printf("Pa_GetDeviceCount() returned 0x%x\n", nd);
        err = nd;
        goto error;
    }
    printf("Number of devices = %d\n", nd);
    printf("Devices with inputs:\n");
    for(id=0; id<nd; id++) {
        devInfo = Pa_GetDeviceInfo(id);
#ifndef AIO_DEBUG_ENABLEMAIN
        if(devInfo->maxInputChannels <= 0) continue;
#endif
#ifdef WIN32
        {   /* Use wide char on windows, so we can show UTF-8 encoded device names */
            wchar_t wideName[MAX_PATH];
            MultiByteToWideChar(CP_UTF8, 0, devInfo->name, -1, wideName, MAX_PATH-1);
            wprintf(L"\nDev %-23d : %s\n", id, wideName);
        }
#else
        printf("\nDev %-23d : %s\n", id, devInfo->name);
#endif
        /* Mark global and API specific default devices */
        defaultDisplayed = 0;
        if(id == Pa_GetDefaultInputDevice()) {
            printf( "[ Default Input" );
            defaultDisplayed = 1;
        } else if(id == Pa_GetHostApiInfo(devInfo->hostApi)->defaultInputDevice) {
            const PaHostApiInfo *hostInfo = Pa_GetHostApiInfo(devInfo->hostApi);
            printf( "[ Default %s Input", hostInfo->name);
            defaultDisplayed = 1;
        }
        if(id == Pa_GetDefaultOutputDevice()) {
            printf((defaultDisplayed ? "," : "["));
            printf(" Default Output");
            defaultDisplayed = 1;
        } else if(id == Pa_GetHostApiInfo(devInfo->hostApi)->defaultOutputDevice) {
            const PaHostApiInfo *hostInfo = Pa_GetHostApiInfo(devInfo->hostApi);
            printf((defaultDisplayed ? "," : "[") );
            printf(" Default %s Output", hostInfo->name);
            defaultDisplayed = 1;
        }
        if(defaultDisplayed) printf( " ]\n" );

        printf("Host API                    = %s\n", Pa_GetHostApiInfo(devInfo->hostApi)->name);
        printf("Max inputs = %d", devInfo->maxInputChannels);
        printf(", Max outputs = %d\n", devInfo->maxOutputChannels);

        printf("Default low  input  latency = %8.5f\n", devInfo->defaultLowInputLatency);
        printf("Default low  output latency = %8.5f\n", devInfo->defaultLowOutputLatency);
        printf("Default high input  latency = %8.5f\n", devInfo->defaultHighInputLatency);
        printf("Default high output latency = %8.5f\n", devInfo->defaultHighOutputLatency);

#ifdef WIN32
  #if PA_USE_ASIO
  /* ASIO specific latency information */
        if(Pa_GetHostApiInfo(devInfo->hostApi)->type == paASIO) {
            long minLatency, maxLatency, preferredLatency, granularity;

            err = PaAsio_GetAvailableLatencyValues(i, &minLatency, &maxLatency, 
                                                   &preferredLatency, &granularity);

            printf("ASIO minimum buffer size    = %ld\n", minLatency);
            printf("ASIO maximum buffer size    = %ld\n", maxLatency);
            printf("ASIO preferred buffer size  = %ld\n", preferredLatency);

            if(granularity == -1)
                printf( "ASIO buffer granularity     = power of 2\n" );
            else
                printf( "ASIO buffer granularity     = %ld\n", granularity);
        }
  #endif /* PA_USE_ASIO */
#endif /* WIN32 */

        printf("Default sample rate         = %8.2f\n", devInfo->defaultSampleRate);

        /* poll for standard sample rates */
        inputParam.device = id;
        inputParam.channelCount = devInfo->maxInputChannels;
        inputParam.sampleFormat = PA_SAMPLE_TYPE;
        inputParam.suggestedLatency = 0; /* ignored by Pa_IsFormatSupported() */
        inputParam.hostApiSpecificStreamInfo = NULL;
        
        outputParam.device = id;
        outputParam.channelCount = devInfo->maxOutputChannels;
        outputParam.sampleFormat = PA_SAMPLE_TYPE;
        outputParam.suggestedLatency = 0; /* ignored by Pa_IsFormatSupported() */
        outputParam.hostApiSpecificStreamInfo = NULL;

        if(inputParam.channelCount > 0) {
            printf("Supported standard sample rates for half-duplex %d channel input = \n",
                   inputParam.channelCount);
            PrintSupportedStandardSampleRates(&inputParam, NULL);
        }
        if(outputParam.channelCount > 0) {
            printf("Supported standard sample rates for half-duplex %d channel output = \n",
                    outputParam.channelCount);
            PrintSupportedStandardSampleRates(NULL, &outputParam);
        }
        if(inputParam.channelCount > 0 && outputParam.channelCount > 0) {
            printf("Supported standard sample rates for full-duplex input/output = \n");
            PrintSupportedStandardSampleRates( &inputParam, &outputParam );
        }
    }
error:
    Pa_Terminate();
    return nd;
}

#ifdef AIO_DEBUG_ENABLEMAIN
int main(int argc, char **argv)
{
    aio_t *aHdl;
    aio_list_devices();
    // error_printf(stderr, "Error message: %s\n", Pa_GetErrorText(err));
    aHdl = aio_init(1, -1, 0);
    printf("%d %g\n", aHdl->id, aHdl->rate);
    aHdl->freq = 440.0;
    aio_startstream(aHdl);

    sleep(3);

    aio_close(aHdl);
    return EXIT_SUCCESS;
}
#endif
