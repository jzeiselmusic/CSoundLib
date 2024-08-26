#ifndef AUDIO_STATE_H
#define AUDIO_STATE_H

#include <stdbool.h>
#include "csl_types.h"
#include "track.h"
#include "csoundlib.h"

typedef struct _audioState {
    struct SoundIo* soundio;
    int sample_rate; 
    InputDtype input_dtype;
    trackObject* track;

    /* initialization */
    bool input_memory_allocated;
    bool output_memory_allocated;
    bool environment_initialized;
    bool backend_connected;

    /* input */
    struct SoundIoDevice** input_devices;
    struct SoundIoRingBuffer** input_channel_buffers; // keep a ring buffer for every channel audio
    int num_channels_available;
    struct SoundIoInStream* input_stream;
    bool input_stream_started;
    bool input_stream_written;

    /* output */
    struct SoundIoDevice** output_devices;
    struct SoundIoOutStream* output_stream;
    bool output_stream_started; // should intialize to -1
    bool output_stream_initialized;

    /* mixed inputs */
    char* mixed_output_buffer; // every channel of data that is enabled gets mixed into output buffer
    float current_rms_ouput;

} audio_state;

extern audio_state* csoundlib_state;

#endif