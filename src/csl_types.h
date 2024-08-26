#ifndef CSL_TYPES_H
#define CSL_TYPES_H

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

typedef enum {
    CSL_U8,   
    CSL_S8,                     
    CSL_U16,
    CSL_U32,
    CSL_S16,
    CSL_S32,
    CSL_U24,
    CSL_S24,
    CSL_FL32,
    CSL_FL64,
} CSL_DTYPE;

typedef struct _inputDtype {
    CSL_DTYPE dtype;
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

#define CSL_MAX_METRONOME_BUF_SIZE    4 * 48000 * CSL_BYTES_IN_BUFFER_32

size_t get_bytes_in_sample(CSL_DTYPE in);
size_t get_bytes_in_buffer(CSL_DTYPE in);
uint8_t get_bit_depth(CSL_DTYPE in);
int32_t get_max_value(CSL_DTYPE in);
int32_t get_min_value(CSL_DTYPE in);
bool is_signed_type(CSL_DTYPE in);

InputDtype CSL_U8_t;
InputDtype CSL_S8_t;
InputDtype CSL_U16_t;
InputDtype CSL_S16_t;
InputDtype CSL_U24_t;
InputDtype CSL_S24_t;
InputDtype CSL_U32_t;
InputDtype CSL_S32_t;

#endif