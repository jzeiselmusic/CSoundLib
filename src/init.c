#include "init.h"
#include "errors.h"
#include "csl_types.h"
#include "streams.h"
#include "track.h"
#include "devices.h"
#include "callbacks.h"
#include <stdlib.h>
#include "state.h"
#include <string.h>
#include "wav.h"

static int _connectToBackend();
static void _deallocateAllMemory();

static int _connectToBackend() {
    int ret = soundio_connect(csoundlib_state->soundio);
    if (ret == 0) {
        csoundlib_state->backend_connected = true;
        soundio_flush_events(csoundlib_state->soundio);
    }
    return ret;
}

static void _deallocateAllMemory() {
    logCallback("deallocating memory");
    if (csoundlib_state->input_memory_allocated) {
        free(csoundlib_state->input_channel_buffers);
        free(csoundlib_state->input_devices);
    }
    if (csoundlib_state->output_memory_allocated) {
        free(csoundlib_state->output_devices);
    }
    free(csoundlib_state);
}

int lib_startSession(int sample_rate, int bit_depth) {
    csoundlib_state = malloc( sizeof(audio_state) );
    csoundlib_state->sample_rate = sample_rate;
    switch(bit_depth) {
        case 8: csoundlib_state->input_dtype = CSL_S8_t; break;
        case 16: csoundlib_state->input_dtype = CSL_S16_t; break;
        case 24: csoundlib_state->input_dtype = CSL_S24_t; break;
        case 32: csoundlib_state->input_dtype = CSL_S32_t; break;
    } 
    struct SoundIo* soundio = soundio_create();
    char* mixed_output_buffer = calloc(MAX_BUFFER_SIZE_BYTES, sizeof(char));
    trackObject* track = malloc(1 * sizeof(trackObject));

    *track = (trackObject)
        {
            .volume = 1.0,
            .record_enabled = false,
            .is_recording = false,
            .input_device_index = lib_getDefaultInputDeviceIndex(),
            .input_channel_index = 0,
            .current_rms_levels = {0.0, 0.0},
            .input_buffer.buffer = {0},
            .input_buffer.write_bytes = 0
        };

    if (soundio && mixed_output_buffer && track && csoundlib_state) {
        csoundlib_state->soundio = soundio;
        csoundlib_state->mixed_output_buffer = mixed_output_buffer;
        csoundlib_state->environment_initialized = true;
        csoundlib_state->track = track;
        int backend_err = _connectToBackend();
        int input_dev_err = lib_loadInputDevices();
        int output_dev_err = lib_loadOutputDevices();

        if (backend_err != SoundIoErrorNone) {
            return SoundIoErrorBackendUnavailable;
        }
        if (input_dev_err != SoundIoErrorNone) {
            return SoundIoErrorLoadingInputDevices;
        }
        if (output_dev_err != SoundIoErrorNone) {
            return SoundIoErrorLoadingOutputDevices;
        }
        return SoundIoErrorNone;
    }
    else {
        return SoundIoErrorNoMem;
    }
}

int lib_destroySession() {
    int ret = lib_stopOutputStream();
    ret = lib_stopInputStream();
    cleanup_input_devices();
    cleanup_output_devices();
    soundio_flush_events(csoundlib_state->soundio);
    soundio_destroy(csoundlib_state->soundio);

    free(csoundlib_state->track);
    free(csoundlib_state->mixed_output_buffer);

    if (csoundlib_state->input_memory_allocated) {
        free(csoundlib_state->input_channel_buffers);
        free(csoundlib_state->input_devices);
    }
    if (csoundlib_state->output_memory_allocated) {
        free(csoundlib_state->output_devices);
    }

    free(csoundlib_state);
    return SoundIoErrorNone;
}

int lib_getCurrentBackend() {
    if (csoundlib_state->backend_connected) {
        soundio_flush_events(csoundlib_state->soundio);
        return csoundlib_state->soundio->current_backend;
    }
    else {
        return -1;
    }
}

int _checkEnvironmentAndBackendConnected() {
    if (!csoundlib_state->environment_initialized) {
        logCallback("uh oh. environment not initialized");
        return SoundIoErrorEnvironmentNotInitialized;
    }
    if (!csoundlib_state->backend_connected) {
        logCallback("uh oh. backend not connected");
        return SoundIoErrorBackendDisconnected;
    }
    return SoundIoErrorNone;
}