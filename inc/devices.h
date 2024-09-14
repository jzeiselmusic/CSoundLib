#ifndef AUDIO_DEVICES_H
#define AUDIO_DEVICES_H

#include "csoundlib.h"

void cleanup_input_devices();
void cleanup_output_devices();
int soundlib_load_input_devices(); 
int soundlib_load_output_devices();

#endif
