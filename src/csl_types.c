
#include "csl_types.h"
#include "state.h"
#include <soundio/soundio.h>

/* ready to use structs for input data types */

InputDtype CSL_U8_t = {
    .dtype = CSL_U8,
    .format = SoundIoFormatU8,
    .bit_depth = 8,
    .bytes_in_buffer = 1,
    .bytes_in_sample = 1,
    .is_signed = false,
    .max_size = CSL_U8_MAX,
    .min_size = 0
};

InputDtype CSL_S8_t = {
    .dtype = CSL_S8,
    .format = SoundIoFormatS8,
    .bit_depth = 8,
    .bytes_in_buffer = 1,
    .bytes_in_sample = 1,
    .is_signed = true,
    .max_size = CSL_S8_MAX,
    .min_size = CSL_S8_MIN
};

InputDtype CSL_U16_t = {
    .dtype = CSL_U16,
    .format = SoundIoFormatU16LE,
    .bit_depth = 16,
    .bytes_in_buffer = 2,
    .bytes_in_sample = 2,
    .is_signed = false,
    .max_size = CSL_U16_MAX,
    .min_size = 0
};

InputDtype CSL_S16_t = {
    .dtype = CSL_S16,
    .format = SoundIoFormatS16LE,
    .bit_depth = 8,
    .bytes_in_buffer = 2,
    .bytes_in_sample = 2,
    .is_signed = true,
    .max_size = CSL_S16_MAX,
    .min_size = CSL_S16_MIN
};

InputDtype CSL_U24_t = {
    .dtype = CSL_U24,
    .format = SoundIoFormatU24LE,
    .bit_depth = 24,
    .bytes_in_buffer = 4,
    .bytes_in_sample = 3,
    .is_signed = false,
    .max_size = CSL_U24_MAX,
    .min_size = 0
};

InputDtype CSL_S24_t = {
    .dtype = CSL_S24,
    .format = SoundIoFormatS24LE,
    .bit_depth = 24,
    .bytes_in_buffer = 4,
    .bytes_in_sample = 3,
    .is_signed = true,
    .max_size = CSL_S24_MAX,
    .min_size = CSL_S24_MIN
};

InputDtype CSL_U32_t = {
    .dtype = CSL_U32,
    .format = SoundIoFormatU32LE,
    .bit_depth = 32,
    .bytes_in_buffer = 4,
    .bytes_in_sample = 4,
    .is_signed = false,
    .max_size = CSL_U32_MAX,
    .min_size = 0
};

InputDtype CSL_S32_t = {
    .dtype = CSL_S32,
    .format = SoundIoFormatS32LE,
    .bit_depth = 32,
    .bytes_in_buffer = 4,
    .bytes_in_sample = 4,
    .is_signed = true,
    .max_size = CSL_S32_MAX,
    .min_size = CSL_S32_MIN
};

InputDtype CSL_FL32_t = {
    .dtype = CSL_FL32,
    .format = SoundIoFormatFloat32LE,
    .bit_depth = 32,
    .bytes_in_buffer = 4,
    .bytes_in_sample = 4,
    .is_signed = true,
    .max_size = 1.0,
    .min_size = 0.0
};

/* ********************************************* */
/* ********************************************* */

InputDtype get_dtype(CslDataType in) {
    switch(in) {
        case CSL_U8: return CSL_U8_t; break;
        case CSL_S8: return CSL_S8_t; break;
        case CSL_U16: return CSL_U16_t; break;
        case CSL_S16: return CSL_S16_t; break;
        case CSL_U24: return CSL_U24_t; break;
        case CSL_S24: return CSL_S24_t; break;
        case CSL_U32: return CSL_U32_t; break;
        case CSL_S32: return CSL_S32_t; break;
        default: return CSL_S32_t;
    }
}

size_t get_bytes_in_sample(CslDataType in) {
    switch(in) {
        case CSL_U8: return CSL_BYTES_IN_SAMPLE_8; break;
        case CSL_S8: return CSL_BYTES_IN_SAMPLE_8; break;
        case CSL_U16: return CSL_BYTES_IN_SAMPLE_16; break;
        case CSL_S16: return CSL_BYTES_IN_SAMPLE_16; break;
        case CSL_U24: return CSL_BYTES_IN_SAMPLE_24; break;
        case CSL_S24: return CSL_BYTES_IN_SAMPLE_24; break;
        case CSL_U32: return CSL_BYTES_IN_SAMPLE_32; break;
        case CSL_S32: return CSL_BYTES_IN_SAMPLE_32; break;
        default: return 0;
    }
}

// if reading a wav file buffer, 24 bit depth is stored in 3 bytes
// if reading an audio stream from a device, 24 bits are stored in 4 bytes
size_t get_bytes_in_buffer(CslDataType in, bool audio_file) {
    switch(in) {
        case CSL_U8: return CSL_BYTES_IN_BUFFER_8; break;
        case CSL_S8: return CSL_BYTES_IN_BUFFER_8; break;
        case CSL_U16: return CSL_BYTES_IN_BUFFER_16; break;
        case CSL_S16: return CSL_BYTES_IN_BUFFER_16; break;
        case CSL_U24: {
            if (audio_file) return 3;
            else return CSL_BYTES_IN_BUFFER_24; 
            break;
        }
        case CSL_S24: {
            if (audio_file) return 3;
            else return CSL_BYTES_IN_BUFFER_24; 
            break;
        }
        case CSL_U32: return CSL_BYTES_IN_BUFFER_32; break;
        case CSL_S32: return CSL_BYTES_IN_BUFFER_32; break;
        default: return 0;
    }
}

uint8_t get_bit_depth(CslDataType in) {
    switch(in) {
        case CSL_U8: return 8; break;
        case CSL_S8: return 8; break;
        case CSL_U16: return 16; break;
        case CSL_S16: return 16; break;
        case CSL_U24: return 24; break;
        case CSL_S24: return 24; break;
        case CSL_U32: return 32; break;
        case CSL_S32: return 32; break;
        default: return 0;
    }
}

int32_t get_max_value(CslDataType in) {
    switch(in) {
        case CSL_U8: return CSL_U8_MAX; break;
        case CSL_S8: return CSL_S8_MAX; break;
        case CSL_U16: return CSL_U16_MAX; break;
        case CSL_S16: return CSL_S16_MAX; break;
        case CSL_U24: return CSL_U24_MAX; break;
        case CSL_S24: return CSL_S24_MAX; break;
        case CSL_U32: return CSL_U32_MAX; break;
        case CSL_S32: return CSL_S32_MAX; break;
        default: return 0;
    }
}

int32_t get_min_value(CslDataType in) {
    switch(in) {
        case CSL_S8: return CSL_S8_MIN; break;
        case CSL_S16: return CSL_S16_MIN; break;
        case CSL_S24: return CSL_S24_MIN; break;
        case CSL_S32: return CSL_S32_MIN; break;
        default: return 0;
    }
}

bool is_signed_type(CslDataType in) {
    switch(in) {
        case CSL_U8: return false; break;
        case CSL_S8: return true; break;
        case CSL_U16: return false; break;
        case CSL_S16: return true; break;
        case CSL_U24: return false; break;
        case CSL_S24: return true; break;
        case CSL_U32: return false; break;
        case CSL_S32: return true; break;
        default: return 0;
    }
}

int get_sample_rate(CslSampleRate in) {
    switch(in) {
        case CSL_SR44100: return 44100; break;
        case CSL_SR48000: return 48000; break;
    }
}