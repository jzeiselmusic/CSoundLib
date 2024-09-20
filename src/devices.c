
#include "devices.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include "csl_types.h"
#include "csl_util.h"
#include "streams.h"
#include "init.h"
#include "state.h"
#include "errors.h"
#include <soundio/soundio.h>

int soundlib_load_input_devices() {
    soundio_flush_events(csoundlib_state->soundio);
    int num_input_devices = soundlib_get_num_input_devices();
    int default_input_device_index = soundlib_get_default_input_device_index();
    if (num_input_devices > 0) {
        struct SoundIoDevice** input_devices = malloc(num_input_devices * sizeof( struct SoundIoDevice*) );
        if (!input_devices) {
            csoundlib_state->input_memory_allocated = false;
            return SoundIoErrorNoMem;
        }
        csoundlib_state->input_devices = input_devices;
        csoundlib_state->input_stream_started = false;
        csoundlib_state->input_stream_written = false;
        csoundlib_state->input_memory_allocated = true;
        for (int i = 0; i < num_input_devices; i++) {
            struct SoundIoDevice* device = soundio_get_input_device(csoundlib_state->soundio, i);
            if (!device) {
                return SoundIoErrorInvalid;
            }
            else {
                csoundlib_state->input_devices[i] = soundio_get_input_device(csoundlib_state->soundio, i);
            }
        }
        int num_channels_of_default_input = soundlib_get_num_channels_of_input_device(default_input_device_index);
        struct SoundIoRingBuffer** channel_buffers = malloc(num_channels_of_default_input * sizeof( struct SoundIoRingBuffer*) );
        if (!channel_buffers) {
            csoundlib_state->input_memory_allocated = false;
            return SoundIoErrorNoMem;
        }
        csoundlib_state->input_channel_buffers = channel_buffers;
        csoundlib_state->num_input_channels = num_channels_of_default_input;
    }
    return SoundIoErrorNone;
}

void cleanup_input_devices() {
    int num_input_devices = soundlib_get_num_input_devices();
    for (int i = 0; i < num_input_devices; i++) {
        soundio_device_unref(csoundlib_state->output_devices[i]);
    }
}


int soundlib_get_default_input_device_index() {
    /* returns -1 on error */
    soundio_flush_events(csoundlib_state->soundio);
    return soundio_default_input_device_index(csoundlib_state->soundio);
}

int soundlib_get_num_input_devices() {
    /* returns -1 on error */
    soundio_flush_events(csoundlib_state->soundio);
    return soundio_input_device_count(csoundlib_state->soundio);
}

char* soundlib_get_input_device_name(int index) {
    /* returns "" on error */
    if (_checkEnvironmentAndBackendConnected() != SoundIoErrorNone) return emptyString;
    return (csoundlib_state->input_devices)[index]->name;
}

int soundlib_get_num_channels_of_input_device(int index) {
    /* returns -1 on error */
    if (_checkEnvironmentAndBackendConnected() != SoundIoErrorNone) return -1;
    return (csoundlib_state->input_devices)[index]->current_layout.channel_count;
}

int soundlib_get_num_formats_of_input_device(int deviceIndex) {
    struct SoundIoDevice* device = soundio_get_input_device(csoundlib_state->soundio, deviceIndex);
    return device->format_count;
}

enum SoundIoFormat* soundlib_get_formats_of_input_device(int deviceIndex) {
    struct SoundIoDevice* device = soundio_get_input_device(csoundlib_state->soundio, deviceIndex);
    return device->formats;
}

/********************/

/* functions for output devices */

/********************/

int soundlib_load_output_devices() {
    soundio_flush_events(csoundlib_state->soundio);
    int num_output_devices = soundlib_get_num_output_devices();
    int default_output_device_index = soundlib_get_default_output_device_index();
    if (num_output_devices > 0) {
        struct SoundIoDevice** output_devices = malloc(num_output_devices * sizeof( struct SoundIoDevice*) );
        if (!output_devices) {
            csoundlib_state->output_memory_allocated = false;
            return SoundIoErrorNoMem;
        }
        csoundlib_state->output_memory_allocated = true;
        csoundlib_state->output_devices = output_devices;
        csoundlib_state->output_stream_started = false;
        for (int i = 0; i < num_output_devices; i++) {
            struct SoundIoDevice* device = soundio_get_output_device(csoundlib_state->soundio, i);
            if (!device) {
                return SoundIoErrorInvalid;
            }
            else {
                csoundlib_state->output_devices[i] = soundio_get_output_device(csoundlib_state->soundio, i);
            }
        }
        int num_channels_of_default_output = soundlib_get_num_channels_of_input_device(default_output_device_index);
        csoundlib_state->num_output_channels = num_channels_of_default_output;
    }
    return SoundIoErrorNone;
}

void cleanup_output_devices() {
    int num_output_devices = soundlib_get_num_output_devices();
    for (int i = 0; i < num_output_devices; i++) {
        soundio_device_unref(csoundlib_state->output_devices[i]);
    }
}

int soundlib_get_default_output_device_index() {
    /* returns -1 on error */
    soundio_flush_events(csoundlib_state->soundio);
    return soundio_default_output_device_index(csoundlib_state->soundio);
}

int soundlib_get_num_output_devices() {
    /* returns -1 on error */
    soundio_flush_events(csoundlib_state->soundio);
    return soundio_output_device_count(csoundlib_state->soundio);
}

char* soundlib_get_output_device_name(int index) {
    /* returns "" on error */
    if (_checkEnvironmentAndBackendConnected() != SoundIoErrorNone) return emptyString;
    return (csoundlib_state->output_devices)[index]->name;
}

int soundlib_get_num_channels_of_output_device(int index) {
    /* returns -1 on error */
    if (_checkEnvironmentAndBackendConnected() != SoundIoErrorNone) return -1;
    return (csoundlib_state->output_devices)[index]->current_layout.channel_count;
}

void soundlib_get_available_input_devices(DeviceInfo* input_buffer) {
    int num_input_devices = soundio_input_device_count(csoundlib_state->soundio);
    for (int i = 0; i < num_input_devices; i++) {
        input_buffer[i].name = (csoundlib_state->input_devices)[i]->name;
        input_buffer[i].index = i;
    }
}

void soundlib_get_available_output_devices(DeviceInfo* input_buffer) {
    int num_output_devices = soundio_output_device_count(csoundlib_state->soundio);
    for (int i = 0; i < num_output_devices; i++) {
        input_buffer[i].name = (csoundlib_state->output_devices)[i]->name;
        input_buffer[i].index = i;
    }
}

int soundlib_get_num_formats_of_output_device(int deviceIndex) {
    struct SoundIoDevice* device = soundio_get_output_device(csoundlib_state->soundio, deviceIndex);
    return device->format_count;
}

enum SoundIoFormat* soundlib_get_formats_of_output_device(int deviceIndex) {
    struct SoundIoDevice* device = soundio_get_output_device(csoundlib_state->soundio, deviceIndex);
    return device->formats;
}