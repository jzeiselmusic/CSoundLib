
#include "csl_types.h"
#include "streams.h"
#include <stdbool.h>
#include "devices.h"
#include "stdint.h"
#include "csl_util.h"
#include "callbacks.h"
#include "init.h"
#include "state.h"
#include "wav.h"
#include "errors.h"
#include "track.h"

#include <fcntl.h>

static int _createInputStream(int device_index, float microphone_latency);
static int _createOutputStream(int device_index, float microphone_latency);
static void _processInputStreams(int* max_fill_samples, struct SoundIoRingBuffer* ring_buffer);
static void _copyInputBuffersToOutputBuffers();

__attribute__ ((cold))
__attribute__ ((noreturn))
__attribute__ ((format (printf, 1, 2)))
static void _panic(const char *format, ...) {
    panicCallback(format);
    abort();
}

__attribute__ ((cold))
__attribute__ ((format (printf, 1, 2)))
static void _formatPanic(const char *format, ...) {
    panicCallback(format);
}

static void _underflowCallback(struct SoundIoOutStream *outstream) {
    static int count = 0;
    logCallback("underflow reported");
}

static void _inputStreamReadCallback(struct SoundIoInStream *instream, int frame_count_min, int frame_count_max) {
    /* gets called repeatedly every time audio data is available to be read on this particular input device */

    /* every input stream started gets a read callback associated with it that gets repeatedly called */

    /* when a device sends data, it interleaves the data based on how many channels there are */

    int device_index = -1;
    while(csoundlib_state->input_stream_written == true) {
        /* wait until output is done to start reading again */
    }

    for (int i = 0; i < lib_getNumInputDevices(); i++) {
        if ((csoundlib_state->input_devices)[i]->id == instream->device->id) {
            device_index = i;
            break;
        }
    }
    if (device_index == -1) {
        _panic("error finding input device");
    }

    if (csoundlib_state->output_stream_initialized == false) {
        _panic("output device not initialized");
        return;
    }

    /* all should be the same */
    struct SoundIoRingBuffer* ring_buffer = csoundlib_state->input_channel_buffers[0];
    /* get the write ptr for this inputs ring buffer */
    int bytes_count = soundio_ring_buffer_free_count(ring_buffer);
    int frame_count = bytes_count / BYTES_PER_FRAME_MONO; 

    if (frame_count_min > frame_count) {
        _panic("ring buffer writer overtook reader");
    }

    int write_frames = min_int(frame_count, frame_count_max);

    int frames_left = write_frames;

    struct SoundIoChannelArea *areas;
    int err;

    for (;;) {
        int frame_count = frames_left;
        if ((err = soundio_instream_begin_read(instream, &areas, &frame_count))) {
            _panic("begin read error: %s", soundio_strerror(err));
        }
        if (!frame_count) {
            break;
        }
        if (!areas) {
            /* Due to an overflow there is a hole. Fill the ring buffer with
               silence for the size of the hole.  */
            _formatPanic("Dropped %d frames due to internal overflow\n", frame_count);
        } 
        else {
            for (int frame = 0; frame < frame_count; frame ++) {
                for (int ch = 0; ch < instream->layout.channel_count; ch ++) {
                    struct SoundIoRingBuffer* ring_buffer = csoundlib_state->input_channel_buffers[ch];
                    char* write_ptr = soundio_ring_buffer_write_ptr(ring_buffer);
                    char* bytes = areas[ch].ptr;
                    memcpy(write_ptr, bytes, instream->bytes_per_sample);
                    areas[ch].ptr += areas[ch].step;
                    soundio_ring_buffer_advance_write_ptr(ring_buffer, instream->bytes_per_sample);
                }
            }
        }
        if ((err = soundio_instream_end_read(instream))) {
            _panic("end read error: %s\n", soundio_strerror(err));
        }

        frames_left -= frame_count;
        if (frames_left <= 0) {
            break;
        }
    }
    csoundlib_state->input_stream_written = true;
}

static void _outputStreamWriteCallback(struct SoundIoOutStream *outstream, int frame_count_min, int frame_count_max) {
    /* gets called repeatedly every time audio data is ready to be posted to this particular output stream */

    /* this function takes the data in the mix buffer and places it into the stream associated with the output device */
    /* we then will increment the read ptr because we read the data placed in by the input streams */
    int frames_left;
    int frame_count;
    int err;
    struct SoundIoChannelArea *areas;
    struct SoundIoRingBuffer* ring_buffer;
    int max_fill_samples = 0;

    /* search for device index of this output stream */
    int device_index = -1;
    for (int i = 0; i < lib_getNumOutputDevices(); i++) {
        if (csoundlib_state->output_devices[i]->id == outstream->device->id) {
            device_index = i;
            break;
        }
    }
    if (device_index == -1) {
        _panic("error finding output device");
        return;
    }
    if (csoundlib_state->output_stream_initialized == false) {
        _panic("output device not initialized");
        return;
    }

    while (csoundlib_state->input_stream_written == false) {}

    /* clear mix buffer */
    memset(csoundlib_state->mixed_output_buffer, 0, MAX_BUFFER_SIZE_BYTES);

    /* clear track input buffers*/

    /* put input streams into track input buffers */
    _processInputStreams(&max_fill_samples, ring_buffer);

    /* now copy input buffer to output scaled by volume */
    /* note: THIS IS WHERE VOLUME SCALING HAPPENS */
    _copyInputBuffersToOutputBuffers();

    /* set master output rms level */
    csoundlib_state->current_rms_ouput = calculate_rms_level(csoundlib_state->mixed_output_buffer, frame_count_max * outstream->bytes_per_frame);

    /* now place data from mixed output buffer into output stream */
    int read_count_samples = min_int(frame_count_max, max_fill_samples);
    /* handle case of no input streams */
    if (read_count_samples == 0) read_count_samples = frame_count_min;
    /* there is data to be read to output */
    frames_left = read_count_samples;
    char* mixed_read_ptr = csoundlib_state->mixed_output_buffer;
    while (frames_left > 0) {
        int frame_count = frames_left;
        if ((err = soundio_outstream_begin_write(outstream, &areas, &frame_count)))
            _panic("begin write error: %s", soundio_strerror(err));
        if (frame_count <= 0)
            break;

        for (int frame = 0; frame < frame_count; frame += 1) {
            for (int ch = 0; ch < outstream->layout.channel_count; ch += 1) {
                memcpy(areas[ch].ptr, mixed_read_ptr, outstream->bytes_per_sample);
                areas[ch].ptr += areas[ch].step;
            }
            mixed_read_ptr += outstream->bytes_per_sample;
        }
        if ((err = soundio_outstream_end_write(outstream)))
            _panic("end write error: %s", soundio_strerror(err));

        frames_left -= frame_count;
    }
    csoundlib_state->input_stream_written = false;
}

static int _createInputStream(int device_index, float microphone_latency) {
    int err;
    err = _checkEnvironmentAndBackendConnected();
    if (err != SoundIoErrorNone) return err;

    struct SoundIoDevice* input_device = csoundlib_state->input_devices[device_index];
    struct SoundIoInStream* instream = soundio_instream_create(input_device);
    if (!instream) return SoundIoErrorNoMem;
    switch(csoundlib_state->input_dtype.dtype) {
        case CSL_S24: instream->format = SoundIoFormatS24LE; break;
        case CSL_S16: instream->format = SoundIoFormatS16LE; break;
        case CSL_S32: instream->format = SoundIoFormatS32LE; break;
        case CSL_S8: instream->format = SoundIoFormatS8; break;
    }
    instream->sample_rate = csoundlib_state->sample_rate;

    /* use whatever the default channel layout is (take all the channels available) */
    /* data should come in interleaved based on how many channels are sending data */
    instream->layout = input_device->current_layout;
    instream->software_latency = microphone_latency;
    instream->read_callback = _inputStreamReadCallback;
    csoundlib_state->input_stream = instream;

    err = soundio_instream_open(instream);
    if (err != SoundIoErrorNone) return SoundIoErrorInputStream;

    int num_channels = lib_getNumChannelsOfInputDevice(device_index);
    csoundlib_state->num_channels_available = num_channels;
    /* reset channel buffers */
    free(csoundlib_state->input_channel_buffers);
    csoundlib_state->input_channel_buffers = malloc(num_channels * sizeof(struct SoundIoRingBuffer*));

    /* create a ring buffer for each input channel available */
    for (int idx = 0; idx < num_channels; idx++) {
        int capacity = DEFAULT_BUFFER_SIZE * instream->bytes_per_sample;
        struct SoundIoRingBuffer* ring_buffer = soundio_ring_buffer_create(csoundlib_state->soundio, capacity);
        if (!ring_buffer) return SoundIoErrorNoMem;

        char *buf = soundio_ring_buffer_write_ptr(ring_buffer);
        int fill_count = soundio_ring_buffer_capacity(ring_buffer);
        memset(buf, 0, fill_count);
        csoundlib_state->input_channel_buffers[idx] = ring_buffer;
    }
    return SoundIoErrorNone;
}

int lib_createAndStartInputStream(int device_index, float microphone_latency) {
    int err;
    err = _createInputStream(device_index, microphone_latency);
    if (err != SoundIoErrorNone) return err;
    if ((err = soundio_instream_start(csoundlib_state->input_stream)) != SoundIoErrorNone) {
        return err;
    }
    csoundlib_state->input_stream_started = true;
    csoundlib_state->input_stream_written = false;
    return SoundIoErrorNone;
}

int lib_stopInputStream() {
    if (csoundlib_state->input_stream_started) {
        csoundlib_state->input_stream_started = false;
        csoundlib_state->input_stream_written = false;
        soundio_instream_destroy(csoundlib_state->input_stream);
    }
    return SoundIoErrorNone;
}

static int _createOutputStream(int device_index, float microphone_latency) {
    int err;
    err = _checkEnvironmentAndBackendConnected();
    if (err != SoundIoErrorNone) return err;

    if (csoundlib_state->output_stream_started) {
        soundio_outstream_destroy(csoundlib_state->output_stream);
        csoundlib_state->output_stream_started = false;
    }

    struct SoundIoDevice* output_device = csoundlib_state->output_devices[device_index];
    struct SoundIoOutStream* outstream = soundio_outstream_create(output_device);
    if (!outstream) return SoundIoErrorNoMem;
    switch(csoundlib_state->input_dtype.dtype) {
        case CSL_S24: outstream->format = SoundIoFormatS24LE; break;
        case CSL_S16: outstream->format = SoundIoFormatS16LE; break;
        case CSL_S32: outstream->format = SoundIoFormatS32LE; break;
        case CSL_S8: outstream->format = SoundIoFormatS8; break;
    }
    outstream->sample_rate = csoundlib_state->sample_rate;
    outstream->layout = output_device->current_layout;
    outstream->software_latency = microphone_latency;
    outstream->write_callback = _outputStreamWriteCallback;
    outstream->underflow_callback = _underflowCallback;

    csoundlib_state->output_stream = outstream;

    err = soundio_outstream_open(outstream);
    if (err != SoundIoErrorNone) return SoundIoErrorOutputStream;
    csoundlib_state->output_stream_initialized = true;

    return SoundIoErrorNone;
}

int lib_createAndStartOutputStream(int deviceIndex, float microphone_latency) {
    int err;
    err = _createOutputStream(deviceIndex, microphone_latency);
    if (err != SoundIoErrorNone) return err;

    if ((err = soundio_outstream_start(csoundlib_state->output_stream)) != SoundIoErrorNone) {
        return err;
    }
    csoundlib_state->output_stream_started = true;
    return SoundIoErrorNone;
}

int lib_stopOutputStream() {
    if (csoundlib_state->output_stream_started) {
        csoundlib_state->output_stream_started = false;
        csoundlib_state->output_stream_initialized = false;
        soundio_outstream_pause(csoundlib_state->output_stream, true);
        soundio_outstream_destroy(csoundlib_state->output_stream);
    }
    logCallback("returning true");
    return SoundIoErrorNone;
}

static void _processInputStreams(int* max_fill_samples, struct SoundIoRingBuffer* ring_buffer) {
    /* copy each input stream into each track input buffer */
    for (int channel = 0; channel < csoundlib_state->num_channels_available; channel++) {
        if (csoundlib_state->input_stream_started == true) {
            ring_buffer = csoundlib_state->input_channel_buffers[channel];
            char *read_ptr = soundio_ring_buffer_read_ptr(ring_buffer);
            /* number of bytes available for reading */
            int fill_bytes = soundio_ring_buffer_fill_count(ring_buffer);
            int fill_samples = fill_bytes / BYTES_PER_FRAME_MONO;
            if (fill_samples > *max_fill_samples) *max_fill_samples = fill_samples;

            /* calculate rms value for this particular input channel */
            float input_rms_val = calculate_rms_level(read_ptr, fill_bytes);

            trackObject* track_p = csoundlib_state->track;
            if (track_p->input_channel_index == channel) {
                /* this track has chosen this channel for input */

                /* set rms value based on input RMS of this channel */
                track_p->current_rms_levels.input_rms_level = input_rms_val;

                /* write the input stream to the track's input buffer */
                if (track_p->input_enabled) {
                    add_and_scale_audio(
                        (uint8_t*)read_ptr, 
                        (uint8_t*)(track_p->input_buffer.buffer),
                        1.0,
                        fill_bytes / csoundlib_state->input_dtype.bytes_in_buffer
                    );
                    track_p->input_buffer.write_bytes = fill_bytes;
                }
            } 
            soundio_ring_buffer_advance_read_ptr(ring_buffer, fill_bytes);
        }
    }
}

static void _copyInputBuffersToOutputBuffers() {
    trackObject* track_p = csoundlib_state->track;
    add_and_scale_audio(
        (uint8_t*)(track_p->input_buffer.buffer),
        (uint8_t*)(csoundlib_state->mixed_output_buffer),
        track_p->volume,
        track_p->input_buffer.write_bytes / csoundlib_state->input_dtype.bytes_in_buffer
    );

    track_p->current_rms_levels.output_rms_level = 
            calculate_rms_level(
                track_p->input_buffer.buffer,
                track_p->input_buffer.write_bytes) * track_p->volume;
}