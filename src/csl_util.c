#include "csl_util.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "csl_types.h"
#include "state.h"

/*

24 bit signed int - little endian format - stored in 32 bit buffer
< byte 1  -    byte 2     -  byte 3 -   byte 4>
<smallest - next smallest - largest -     0   >

*/

int min_int(int a, int b) {
    return (a < b) ? a : b;
}

float log_to_mag(float log) {
    return pow(10, (log / 20.0));
}

float mag_to_log(float mag) {
    return 20.0 * log10(mag);
}

float envelope_follower(float input, float attack, float release, float prev_envelope) {
    float attackAlpha = 1 - expf(-1.0 / (attack * 100.0));
    float releaseAlpha = 1 - expf(-1.0 / (release * 100.0));

    float envelope = 0.0;
    if (input > prev_envelope)
        envelope = attackAlpha * input + (1.0 - attackAlpha) * prev_envelope;
    else
        envelope = releaseAlpha * input + (1.0 - releaseAlpha) * prev_envelope;

    return envelope;
}

void add_and_scale_audio(const uint8_t *source, uint8_t *destination, float volume, int num_samples) {
    CSL_DTYPE dtype = csoundlib_state->input_dtype.dtype;
    uint8_t bytes_in_buffer = get_bytes_in_buffer(dtype);
    uint8_t bytes_in_sample = get_bytes_in_sample(dtype);
    for (int i = 0; i < num_samples; i++) {
        int32_t src_sample = 0;
        int32_t dst_sample = 0;

        // Read little endian samples
        for (int j = 0; j < bytes_in_sample; j++) {
            src_sample |= ((int32_t)source[i * bytes_in_buffer + j]) << (j * 8);
            dst_sample |= ((int32_t)destination[i * bytes_in_buffer + j]) << (j * 8);
        }

        // Sign-extend to 32 bits
        if (dtype == CSL_U24 || dtype == CSL_S24) {
            src_sample = (src_sample << 8) >> 8;
            dst_sample = (dst_sample << 8) >> 8;
        }
        if (dtype == CSL_U16 || dtype == CSL_S16) {
            src_sample = (src_sample << 16) >> 16;
            dst_sample = (dst_sample << 16) >> 16;
        }
        if (dtype == CSL_U8 || dtype == CSL_S8) {
            src_sample = (src_sample << 24) >> 24;
            dst_sample = (dst_sample << 24) >> 24;
        }

        // Add samples and apply volume scaling
        int32_t result = (int32_t)((src_sample + dst_sample) * volume);
        
        // Clip the result to max/min bit range
        if (csoundlib_state->input_dtype.is_signed) {
            int max_val = get_max_value(dtype);
            int min_val = get_min_value(dtype);
            result = (result > max_val) ? max_val : (result < min_val) ? min_val : result;
        }
        else {
            int max_val = get_max_value(dtype);
            result = (result > max_val) ? max_val : result;
        }

        // Write the result back to the destination buffer
        for (int j = 0; j < bytes_in_sample; j++) {
            destination[i * bytes_in_buffer + j] = (uint8_t)(result >> (j * 8));
        }
        // The fourth byte remains unused (zero)
        if (dtype == CSL_U24 || dtype == CSL_S24) {
            destination[i * bytes_in_buffer + 3] = 0;
        }
    }
}

void scale_audio(uint8_t *source, float volume, int num_samples) {
    CSL_DTYPE dtype = csoundlib_state->input_dtype.dtype;
    uint8_t bytes_in_buffer = get_bytes_in_buffer(dtype);
    uint8_t bytes_in_sample = get_bytes_in_sample(dtype);
    for (int i = 0; i < num_samples; i++) {
        int32_t src_sample = 0;

        // Read little endian samples
        for (int j = 0; j < bytes_in_sample; j++) {
            src_sample |= ((int32_t)source[i * bytes_in_buffer + j]) << (j * 8);
        }

        // Sign-extend to 32 bits
        if (dtype == CSL_U24 || dtype == CSL_S24) {
            src_sample = (src_sample << 8) >> 8;
        }
        if (dtype == CSL_U16 || dtype == CSL_S16) {
            src_sample = (src_sample << 16) >> 16;
        }
        if (dtype == CSL_U8 || dtype == CSL_S8) {
            src_sample = (src_sample << 24) >> 24;
        }

        // Add samples and apply volume scaling
        int32_t result = (int32_t)(src_sample * volume);
        
        // Clip the result to max/min bit range
        if (csoundlib_state->input_dtype.is_signed) {
            int max_val = get_max_value(dtype);
            int min_val = get_min_value(dtype);
            result = (result > max_val) ? max_val : (result < min_val) ? min_val : result;
        }
        else {
            int max_val = get_max_value(dtype);
            result = (result > max_val) ? max_val : result;
        }

        // Write the result back to the destination buffer
        for (int j = 0; j < bytes_in_sample; j++) {
            source[i * bytes_in_buffer + j] = (uint8_t)(result >> (j * 8));
        }
        // The fourth byte remains unused (zero)
        if (dtype == CSL_U24 || dtype == CSL_S24) {
            source[i * bytes_in_buffer + 3] = 0;
        }
    }
}

float calculate_rms_level(const unsigned char* source, int num_bytes) {
    float rms = 0.0;
    uint32_t num_samples = 0;
    /* 4 if 24 bit or 32 bit. 2 if 16 bit. 1 if 8 bit */
    int bytes_in_buffer = csoundlib_state->input_dtype.bytes_in_buffer;
    for (int idx = 0; idx < num_bytes; idx += bytes_in_buffer) {
        float sample = bytes_to_sample(source + idx, csoundlib_state->input_dtype);
        rms += sample * sample;
        num_samples += 1;
    }
    return sqrt(rms / (float)num_samples);
}

float bytes_to_sample(const unsigned char* bytes, InputDtype data_type) {
    if (data_type.dtype == CSL_S24 || data_type.dtype == CSL_S32 ||
        data_type.dtype == CSL_U24 || data_type.dtype == CSL_U32) {
        int sample_value = (int32_t)
            (
                (bytes[3] << 24) | 
                (bytes[2] << 16) |
                (bytes[1] << 8)  |
                (bytes[0])
            );
        float sample_val_float = (float)sample_value;
        if (sample_val_float > 0 || !data_type.is_signed) {
            return sample_val_float / data_type.max_size;
        }
        else {
            return sample_val_float / data_type.min_size;
        }
    }
    else if (data_type.dtype == CSL_S16 || data_type.dtype == CSL_U16) {
        int sample_value = (int16_t)
            (
                (bytes[1] << 8) | (bytes[0])
            );
        float sample_val_float = (float)sample_value;
        if (sample_val_float > 0 || !data_type.is_signed) {
            return sample_val_float / data_type.max_size;
        }
        else {
            return sample_val_float / data_type.min_size;
        }
    }
    else if (data_type.dtype == CSL_S8 || data_type.dtype == CSL_U8) {
        int sample_value = (int8_t)bytes[0];
        float sample_val_float = (float)sample_value;
        if (sample_val_float > 0 || !data_type.is_signed) {
            return sample_val_float / data_type.max_size;
        }
        else {
            return sample_val_float / data_type.min_size;
        }
    }
}

int byte_buffer_to_float_buffer(const unsigned char* byte_buffer, float* float_buffer, size_t len, CSL_DTYPE data_type) {
    int num_samples = len / get_bytes_in_buffer(data_type);
    int i;
    for (i = 0; i < num_samples; i++) {
        float sample = bytes_to_sample(byte_buffer + (i * get_bytes_in_buffer(data_type)), get_dtype(data_type));
        float_buffer[i] = sample;
    }
    return i;
}
