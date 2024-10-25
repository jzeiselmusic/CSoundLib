#ifndef SOUNDLIB_UTIL_H 
#define SOUNDLIB_UTIL_H

#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "csl_types.h"

int min_int(int a, int b);

float envelope_follower(float input, float attack, float release, float prev_envelope);

void add_and_scale_audio(const uint8_t *source, uint8_t *destination, float volume, int num_samples);

void scale_audio(uint8_t *source, float volume, int num_samples);

float calculate_rms_level(const unsigned char* source, int num_bytes);

float log_to_mag(float log);

float mag_to_log(float mag);

#endif