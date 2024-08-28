#include "track.h"
#include "state.h"
#include "errors.h"

extern audio_state* csoundlib_state;

int lib_trackChooseInputDevice(int trackId, int device_index) {
    csoundlib_state->track->input_device_index = device_index;
    return SoundIoErrorNone;
}

int lib_trackChooseInputChannel(int trackId, int channel_index) {
    csoundlib_state->track->input_channel_index = channel_index;
    return SoundIoErrorNone;
}