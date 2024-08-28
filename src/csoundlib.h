#ifndef CSOUNDLIB_H
#define CSOUNDLIB_H

#include <stdint.h>
#include <stdlib.h>
#include <soundio/soundio.h>

/* errors.h */

#define SoundIoErrorDevicesNotInitialized         16
#define SoundIoErrorEnvironmentNotInitialized     17
#define SoundIoErrorIndexOutOfBounds              18
#define SoundIoErrorDevicesNotLoaded              19
#define SoundIoInputStreamError                   20
#define SoundIoInputMemoryNotAllocated            21
#define SoundIoOutputMemoryNotAllocated           22
#define SoundIoErrorTrackNotFound                 23
#define SoundIoErrorOpeningFile                   24
#define SoundIoErrorFileNotFound                  25
#define SoundIoErrorReadingWavForMetronome        26
#define SoundIoErrorInputStream                   27
#define SoundIoErrorOutputStream                  28
#define SoundIoErrorLoadingInputDevices           29
#define SoundIoErrorLoadingOutputDevices          30
#define SoundIoErrorSettingSampleRate             31

/* init.h */
int lib_startSession(int sample_rate, int bit_depth);
int lib_destroySession();
int lib_getCurrentBackend();

int lib_createAndStartInputStream(int deviceIndex, float microphone_latency);
int lib_stopInputStream();
int lib_createAndStartOutputStream(int deviceIndex, float microphone_latency);
int lib_stopOutputStream();
int lib_trackChooseInputDevice(int trackId, int device_index);
int lib_trackChooseInputChannel(int trackId, int channel_index);
float lib_getCurrentRmsOutput(void);

/* devices.h */

/* functions for input devices */
int lib_loadInputDevices();
int lib_getDefaultInputDeviceIndex();
int lib_getNumInputDevices();
char* lib_getDefaultInputDeviceName();
char* lib_getInputDeviceName(int index);
char* lib_getInputDeviceId(int index);
int lib_getNumChannelsOfInputDevice(int index);
char* lib_getNameOfChannelOfInputDevice(int deviceIndex, int channelIndex);

/* functions for output devices */
int lib_loadOutputDevices();
int lib_getDefaultOutputDeviceIndex();
int lib_getNumOutputDevices();
char* lib_getDefaultOutputDeviceName();
char* lib_getOutputDeviceName(int index);
char* lib_getOutputDeviceId(int index);
int lib_getNumChannelsOfOutputDevice(int index);
char* lib_getNameOfChannelOfOutputDevice(int deviceIndex, int channelIndex);

#endif
