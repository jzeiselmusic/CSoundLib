#include "init.h"
#include "errors.h"
#include "csl_types.h"
#include "streams.h"
#include "track.h"
#include "devices.h"
#include <stdlib.h>
#include "state.h"
#include <string.h>
#include "wav.h"
#include "csoundlib.h"
#include <CoreAudio/CoreAudio.h>

extern audio_state* csoundlib_state;

static int _connectToBackend();
static void _deallocateAllMemory();
static int _setGlobalInputSampleRate(CSL_SR sample_rate);
static int _setGlobalOutputSampleRate(CSL_SR sample_rate);

static int _connectToBackend() {
    int ret = soundio_connect(csoundlib_state->soundio);
    if (ret == 0) {
        csoundlib_state->backend_connected = true;
        soundio_flush_events(csoundlib_state->soundio);
    }
    return ret;
}

static void _deallocateAllMemory() {
    if (csoundlib_state->input_memory_allocated) {
        free(csoundlib_state->input_channel_buffers);
        free(csoundlib_state->input_devices);
    }
    if (csoundlib_state->output_memory_allocated) {
        free(csoundlib_state->output_devices);
    }
    free(csoundlib_state);
}

int lib_startSession(CSL_SR sample_rate, CSL_DTYPE data_type) {
    int err;
    csoundlib_state = malloc( sizeof(audio_state) );

    csoundlib_state->sample_rate = sample_rate;

    if ((err = _setGlobalInputSampleRate(sample_rate)) != SoundIoErrorNone) {
        return err;
    }
    if ((err = _setGlobalOutputSampleRate(sample_rate)) != SoundIoErrorNone) {
        return err;
    }

    switch(data_type) {
        case CSL_S8: csoundlib_state->input_dtype = CSL_S8_t; break;
        case CSL_S16: csoundlib_state->input_dtype = CSL_S16_t; break;
        case CSL_S24: csoundlib_state->input_dtype = CSL_S24_t; break;
        case CSL_S32: csoundlib_state->input_dtype = CSL_S32_t; break;
    } 

    struct SoundIo* soundio = soundio_create();
    
    unsigned char* mixed_output_buffer = (unsigned char*)calloc(MAX_BUFFER_SIZE_BYTES, sizeof(char));
    ht* hash_table = ht_create();

    /* *track = (trackObject)
        {
            .volume = 1.0,
            .record_enabled = false,
            .is_recording = false,
            .input_device_index = 0,
            .input_channel_index = 0,
            .current_rms_levels = {0.0, 0.0},
            .input_buffer.buffer = {0},
            .input_buffer.write_bytes = 0
        };
    */

    if (soundio && mixed_output_buffer && csoundlib_state) {
        csoundlib_state->soundio = soundio;
        csoundlib_state->mixed_output_buffer = mixed_output_buffer;
        csoundlib_state->environment_initialized = true;
        csoundlib_state->track_hash_table = hash_table;
        csoundlib_state->num_tracks = 0;
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
        return SoundIoErrorEnvironmentNotInitialized;
    }
    if (!csoundlib_state->backend_connected) {
        return SoundIoErrorBackendDisconnected;
    }
    return SoundIoErrorNone;
}

static int _setGlobalInputSampleRate(CSL_SR sample_rate) {
    AudioObjectPropertyAddress property = {
        kAudioHardwarePropertyDefaultInputDevice,
        kAudioObjectPropertyScopeGlobal,
        kAudioObjectPropertyElementMain
    };
    
    AudioDeviceID deviceID;
    UInt32 size = sizeof(deviceID);
    Float64 desiredSampleRate = (Float64)get_sample_rate(sample_rate);
    OSStatus status = AudioObjectGetPropertyData(kAudioObjectSystemObject, &property, 0, NULL, &size, &deviceID);
    if (status != noErr) return SoundIoErrorSettingSampleRate;

    // Set the sample rate
    property.mSelector = kAudioDevicePropertyNominalSampleRate;
    property.mScope = kAudioObjectPropertyScopeInput;
    
    status = AudioObjectSetPropertyData(deviceID, &property, 0, NULL, sizeof(desiredSampleRate), &desiredSampleRate);
    if (status != noErr) return SoundIoErrorSettingSampleRate;

    // Verify the new sample rate
    Float64 actualSampleRate;
    size = sizeof(actualSampleRate);
    status = AudioObjectGetPropertyData(deviceID, &property, 0, NULL, &size, &actualSampleRate);
    if (status != noErr) return SoundIoErrorSettingSampleRate;

    return SoundIoErrorNone;
}

static int _setGlobalOutputSampleRate(CSL_SR sample_rate) {
    AudioObjectPropertyAddress property = {
        kAudioHardwarePropertyDefaultOutputDevice,
        kAudioObjectPropertyScopeGlobal,
        kAudioObjectPropertyElementMain
    };
    
    AudioDeviceID deviceID;
    UInt32 size = sizeof(deviceID);
    Float64 desiredSampleRate = (Float64)get_sample_rate(sample_rate);
    OSStatus status = AudioObjectGetPropertyData(kAudioObjectSystemObject, &property, 0, NULL, &size, &deviceID);
    if (status != noErr) return SoundIoErrorSettingSampleRate;

    // Set the sample rate
    property.mSelector = kAudioDevicePropertyNominalSampleRate;
    property.mScope = kAudioObjectPropertyScopeInput;
    
    status = AudioObjectSetPropertyData(deviceID, &property, 0, NULL, sizeof(desiredSampleRate), &desiredSampleRate);
    if (status != noErr) return SoundIoErrorSettingSampleRate;

    // Verify the new sample rate
    Float64 actualSampleRate;
    size = sizeof(actualSampleRate);
    status = AudioObjectGetPropertyData(deviceID, &property, 0, NULL, &size, &actualSampleRate);
    if (status != noErr) return SoundIoErrorSettingSampleRate;

    return SoundIoErrorNone;
}