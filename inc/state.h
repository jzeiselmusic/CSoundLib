#ifndef AUDIO_STATE_H
#define AUDIO_STATE_H

#include <stdbool.h>
#include "csl_types.h"
#include "track.h"
#include "hash.h"
#include "effects.h"
#include <soundio/soundio.h>

typedef struct _audioState {
    struct SoundIo* soundio;
    CslSampleRate sample_rate; 
    InputDtype input_dtype;
    float master_volume; // 0.0 -> 1.0 (parity)

    /* initialization */
    bool input_memory_allocated;
    bool output_memory_allocated;
    bool environment_initialized;
    bool backend_connected;

    /* input */
    struct SoundIoDevice** input_devices;
    struct SoundIoRingBuffer** input_channel_buffers; // keep a ring buffer for every channel audio
    uint8_t num_input_channels;
    struct SoundIoInStream* input_stream;
    bool input_stream_started;
    bool input_stream_written;

    /* output */
    struct SoundIoDevice** output_devices;
    struct SoundIoOutStream* output_stream;
    uint8_t num_output_channels;
    bool output_stream_started; // should intialize to -1
    bool output_stream_initialized;
    MasterEffectList master_effects;
    MasterAudioAvailableCallback output_callback;

    /* mixed inputs */
    unsigned char* mixed_output_buffer; // every channel of data that is enabled gets mixed into output buffer
    size_t mixed_output_buffer_len;
    float current_rms_ouput;

    /* tracks */
    ht* track_hash_table;
    uint16_t num_tracks;

    /* solo and mute */
    uint16_t tracks_solod;
    bool solo_engaged;

} audio_state;

extern audio_state* csoundlib_state;

#endif