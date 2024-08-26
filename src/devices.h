#ifndef AUDIO_DEVICES_H
#define AUDIO_DEVICES_H

/* functions for input devices */
int lib_loadInputDevices();
void cleanup_input_devices();
int lib_getDefaultInputDeviceIndex();
int lib_getNumInputDevices();
char* lib_getDefaultInputDeviceName();
char* lib_getInputDeviceName(int index);
char* lib_getInputDeviceId(int index);
int lib_getNumChannelsOfInputDevice(int index);
char* lib_getNameOfChannelOfInputDevice(int deviceIndex, int channelIndex);

/* functions for output devices */
int lib_loadOutputDevices();
void cleanup_output_devices();
int lib_getDefaultOutputDeviceIndex();
int lib_getNumOutputDevices();
char* lib_getDefaultOutputDeviceName();
char* lib_getOutputDeviceName(int index);
char* lib_getOutputDeviceId(int index);
int lib_getNumChannelsOfOutputDevice(int index);
char* lib_getNameOfChannelOfOutputDevice(int deviceIndex, int channelIndex);

#endif
