#ifndef TRACK_H
#define TRACK_H

#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include "streams.h"

typedef struct _inputBuffer {
    unsigned char buffer[MAX_BUFFER_SIZE_BYTES];
    size_t write_bytes;
} inputBuffer;

typedef struct _rmsVals {
    /* 
    input level is the volume level of the incoming audio stream.
    it should be grabbed by VU meter level NOT during playback (if input enabled). 
    it should also be grabbed to tell DAW level of waveform. 
     */
    float input_rms_level;
    /* 
    output level is volume level of the stream being sent to output mix.
    it should be grabbed by VU meter level only during playback.
    */
    float output_rms_level;
} rmsVals;

typedef struct _trackObj {
    float volume; // value greater than 0.0
    bool input_enabled;
    bool record_enabled;
    bool is_recording;
    int input_device_index; // input device currently attached to this track
    int input_channel_index;
    rmsVals current_rms_levels;
    inputBuffer input_buffer;
} trackObject;

#include "csoundlib.h"

#endif