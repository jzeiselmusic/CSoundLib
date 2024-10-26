#ifndef CSL_TYPES_H
#define CSL_TYPES_H

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include "csoundlib.h"
#include <soundio/soundio.h>


typedef struct _inputDtype {
    CslDataType dtype;
    enum SoundIoFormat format;
    uint8_t bit_depth;
    uint8_t bytes_in_buffer;
    uint8_t bytes_in_sample;
    int32_t max_size;
    int32_t min_size;
    bool is_signed;
} InputDtype;

#define emptyString                   ""
#define ATTACK                        0.001
#define RELEASE                       0.15

#define CSL_U8_MAX                    UINT8_MAX
#define CSL_S8_MAX                    INT8_MAX
#define CSL_S8_MIN                    INT8_MIN
#define CSL_U16_MAX                   UINT16_MAX
#define CSL_S16_MAX                   INT16_MAX
#define CSL_S16_MIN                   INT16_MIN
#define CSL_U24_MAX                   16777215
#define CSL_S24_MAX                   8388607
#define CSL_S24_MIN                   -8388608
#define CSL_U32_MAX                   UINT32_MAX
#define CSL_S32_MAX                   INT32_MAX
#define CSL_S32_MIN                   INT32_MIN

#define CSL_BYTES_IN_SAMPLE_8         1
#define CSL_BYTES_IN_SAMPLE_16        2
#define CSL_BYTES_IN_SAMPLE_24        3
#define CSL_BYTES_IN_SAMPLE_32        4

#define CSL_BYTES_IN_BUFFER_8         1
#define CSL_BYTES_IN_BUFFER_16        2
#define CSL_BYTES_IN_BUFFER_24        4
#define CSL_BYTES_IN_BUFFER_32        4

InputDtype get_dtype(CslDataType in);

InputDtype CSL_U8_t;
InputDtype CSL_S8_t;
InputDtype CSL_U16_t;
InputDtype CSL_S16_t;
InputDtype CSL_U24_t;
InputDtype CSL_S24_t;
InputDtype CSL_U32_t;
InputDtype CSL_S32_t;
InputDtype CSL_FL32_t;

#endif