#ifndef CSOUNDLIB_H
#define CSOUNDLIB_H

#ifdef __cplusplus
extern "C" {
#endif 

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

/* data types */

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

typedef enum {
    SR44100,
    SR48000
} CSL_SR;

/* function pointers */
typedef void (*EffectPointer) (unsigned char *buffer, size_t length, CSL_DTYPE data_type, CSL_SR sample_rate, size_t num_channels);

/* describes input or output devices to the user */
typedef struct {
    char* name;
    int index;
} DeviceInfo;

/* init.h */
int lib_startSession(CSL_SR sample_rate, CSL_DTYPE data_type);
int lib_destroySession();
int lib_getCurrentBackend();

/* tracks.h */
int lib_addTrack(int trackId);
int lib_deleteTrack(int trackId);
void lib_deleteAllTracks(void);
int lib_trackChooseInputDevice(int trackId, int device_index);
int lib_trackChooseInputChannel(int trackId, int channel_index);
float lib_getRmsVolumeTrackInput(int trackId);
float lib_getRmsVolumeTrackOutput(int trackId);
int lib_soloEnable(int trackId);
int lib_soloDisable(int trackId);
int lib_muteEnable(int trackId);
int lib_muteDisable(int trackId);
int lib_setTrackVolume(int trackId, float logVolume);

/* audio streams */
int lib_createAndStartInputStream(int deviceIndex, float microphone_latency);
int lib_stopInputStream();
int lib_createAndStartOutputStream(int deviceIndex, float microphone_latency);
int lib_stopOutputStream();
float lib_getCurrentRmsOutput();

/* functions for input devices */
int lib_loadInputDevices();
int lib_getDefaultInputDeviceIndex();
int lib_getNumInputDevices();
char* lib_getInputDeviceName(int index);
int lib_getNumChannelsOfInputDevice(int index);
void lib_getAvailableInputDevices(DeviceInfo* in_buffer);

/* functions for output devices */
int lib_loadOutputDevices();
int lib_getDefaultOutputDeviceIndex();
int lib_getNumOutputDevices();
char* lib_getOutputDeviceName(int index);
int lib_getNumChannelsOfOutputDevice(int index);
void lib_getAvailableOutputDevices(DeviceInfo* in_buffer);

/* master control */
void lib_setMasterVolume(float logVolume);

/* effects */
int lib_registerEffect(int trackId, EffectPointer effect);

#ifdef __cplusplus
}
#endif

#endif
