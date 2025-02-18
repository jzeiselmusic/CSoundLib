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

static inline void master_dummy_callback(
    unsigned char *buffer, 
    size_t length, 
    CslDataType data_type, 
    CslSampleRate sample_rate, 
    size_t num_channels   
) {};

static int _connectToBackend();
static void _deallocateAllMemory();
static int _setGlobalInputSampleRate(CslSampleRate sample_rate);
static int _setGlobalOutputSampleRate(CslSampleRate sample_rate);

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

int soundlib_start_session(
    CslSampleRate sample_rate, 
    CslDataType data_type, 
    CslStreamType stream_type,
    float software_latency) {
    int err;
    csoundlib_state = malloc( sizeof(audio_state) );

    csoundlib_state->sample_rate = sample_rate;
    csoundlib_state->stream_type = stream_type;

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
        case CSL_U8: csoundlib_state->input_dtype = CSL_U8_t; break;
        case CSL_U16: csoundlib_state->input_dtype = CSL_U16_t; break;
        case CSL_U24: csoundlib_state->input_dtype = CSL_U24_t; break;
        case CSL_U32: csoundlib_state->input_dtype = CSL_U32_t; break;
        case CSL_FL32: csoundlib_state->input_dtype = CSL_FL32_t; break;
    } 

    struct SoundIo* soundio = soundio_create();
    
    unsigned char* mixed_output_buffer = (unsigned char*)calloc(MAX_BUFFER_SIZE_BYTES, sizeof(char));
    MasterAudioAvailableCallback* effects = (MasterAudioAvailableCallback*)malloc(MAX_NUM_EFFECTS * sizeof(MasterAudioAvailableCallback));
    ht* hash_table = ht_create();

    if (soundio && mixed_output_buffer && csoundlib_state && effects) {
        csoundlib_state->soundio = soundio;
        csoundlib_state->mixed_output_buffer = mixed_output_buffer;
        csoundlib_state->environment_initialized = true;
        csoundlib_state->track_hash_table = hash_table;
        csoundlib_state->num_tracks = 0;
        csoundlib_state->master_effects.master_effect_list = effects;
        csoundlib_state->master_effects.num_effects = 0;
        csoundlib_state->output_callback = &master_dummy_callback;
        csoundlib_state->num_channels_audio_file = 2;
        int backend_err = _connectToBackend();
        int input_dev_err = soundlib_load_input_devices();
        int output_dev_err = soundlib_load_output_devices();

        if (backend_err != SoundIoErrorNone) {
            return SoundIoErrorBackendUnavailable;
        }
        if (input_dev_err != SoundIoErrorNone) {
            return CSLErrorLoadingInputDevices;
        }
        if (output_dev_err != SoundIoErrorNone) {
            return CSLErrorLoadingOutputDevices;
        }

        int ind = soundlib_get_default_input_device_index();
        int oind = soundlib_get_default_output_device_index();

        err = soundlib_start_input_stream(ind, software_latency);
        if (err != SoundIoErrorNone) {
            return err;
        }
        err = soundlib_start_output_stream(oind, software_latency);
        if (err !=SoundIoErrorNone) {
            return err;
        }
        return SoundIoErrorNone;
    }
    else {
        return SoundIoErrorNoMem;
    } 
}

int soundlib_destroy_session() {
    soundio_flush_events(csoundlib_state->soundio);
    if (csoundlib_state->output_stream_started) {
        soundlib_stop_output_stream();
    } 
    if (csoundlib_state->input_stream_started) {
        soundlib_stop_input_stream();
    }
    cleanup_input_devices();
    cleanup_output_devices();
    soundio_destroy(csoundlib_state->soundio);

    free(csoundlib_state->mixed_output_buffer);
    csoundlib_state->master_effects.num_effects = 0;
    free(csoundlib_state->master_effects.master_effect_list);

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

int soundlib_get_current_backend() {
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
        return CSLErrorEnvironmentNotInitialized;
    }
    if (!csoundlib_state->backend_connected) {
        return SoundIoErrorBackendDisconnected;
    }
    return SoundIoErrorNone;
}

static int _setGlobalInputSampleRate(CslSampleRate sample_rate) {
    AudioObjectPropertyAddress property = {
        kAudioHardwarePropertyDefaultInputDevice,
        kAudioObjectPropertyScopeGlobal,
        kAudioObjectPropertyElementMain
    };
    
    AudioDeviceID deviceID;
    UInt32 size = sizeof(deviceID);
    Float64 desiredSampleRate = (Float64)get_sample_rate(sample_rate);
    OSStatus status = AudioObjectGetPropertyData(kAudioObjectSystemObject, &property, 0, NULL, &size, &deviceID);
    if (status != noErr) return CSLErrorSettingSampleRate;

    // Set the sample rate
    property.mSelector = kAudioDevicePropertyNominalSampleRate;
    property.mScope = kAudioObjectPropertyScopeInput;
    
    status = AudioObjectSetPropertyData(deviceID, &property, 0, NULL, sizeof(desiredSampleRate), &desiredSampleRate);
    if (status != noErr) return CSLErrorSettingSampleRate;

    // Verify the new sample rate
    Float64 actualSampleRate;
    size = sizeof(actualSampleRate);
    status = AudioObjectGetPropertyData(deviceID, &property, 0, NULL, &size, &actualSampleRate);
    if (status != noErr) return CSLErrorSettingSampleRate;

    return SoundIoErrorNone;
}

static int _setGlobalOutputSampleRate(CslSampleRate sample_rate) {
    AudioObjectPropertyAddress property = {
        kAudioHardwarePropertyDefaultOutputDevice,
        kAudioObjectPropertyScopeGlobal,
        kAudioObjectPropertyElementMain
    };
    
    AudioDeviceID deviceID;
    UInt32 size = sizeof(deviceID);
    Float64 desiredSampleRate = (Float64)get_sample_rate(sample_rate);
    OSStatus status = AudioObjectGetPropertyData(kAudioObjectSystemObject, &property, 0, NULL, &size, &deviceID);
    if (status != noErr) return CSLErrorSettingSampleRate;

    // Set the sample rate
    property.mSelector = kAudioDevicePropertyNominalSampleRate;
    property.mScope = kAudioObjectPropertyScopeInput;
    
    status = AudioObjectSetPropertyData(deviceID, &property, 0, NULL, sizeof(desiredSampleRate), &desiredSampleRate);
    if (status != noErr) return CSLErrorSettingSampleRate;

    // Verify the new sample rate
    Float64 actualSampleRate;
    size = sizeof(actualSampleRate);
    status = AudioObjectGetPropertyData(deviceID, &property, 0, NULL, &size, &actualSampleRate);
    if (status != noErr) return CSLErrorSettingSampleRate;

    return SoundIoErrorNone;
}

void soundlib_set_num_channels_audio_file(uint8_t channels) {
    csoundlib_state->num_channels_audio_file = channels;
}

