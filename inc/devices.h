#ifndef AUDIO_DEVICES_H
#define AUDIO_DEVICES_H

#include "csoundlib.h"

void cleanup_input_devices();
void cleanup_output_devices();
int soundlib_load_input_devices(); 
int soundlib_load_output_devices();
enum SoundIoFormat* soundlib_get_formats_of_input_device(int deviceIndex);
enum SoundIoFormat* soundlib_get_formats_of_output_device(int deviceIndex);

#endif
