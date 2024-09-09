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
typedef void (*TrackAudioAvailableCallback) (
    int trackId,
    unsigned char *buffer, 
    size_t length, 
    CSL_DTYPE data_type, 
    CSL_SR sample_rate, 
    size_t num_channels
);

typedef void (*MasterAudioAvailableCallback) (
    unsigned char *buffer,
    size_t length,
    CSL_DTYPE data_type,
    CSL_SR sample_rate,
    size_t num_channels
);

/* describes input or output devices to the user */
typedef struct {
    char* name;
    int index;
} DeviceInfo;

/* init.h */
int soundlib_start_session(CSL_SR sample_rate, CSL_DTYPE data_type);
int soundlib_destroy_session();
int soundlib_get_current_backend();

/* tracks.h */
int soundlib_add_track(int trackId);
int soundlib_delete_track(int trackId);
void soundlib_delete_all_tracks(void);
int soundlib_choose_input_device(int trackId, int device_index);
int soundlib_choose_input_channel(int trackId, int channel_index);
float soundlib_get_track_input_rms(int trackId);
float soundlib_get_track_output_rms(int trackId);
int soundlib_solo_enable(int trackId);
int soundlib_solo_disable(int trackId);
int soundlib_mute_enable(int trackId);
int soundlib_mute_disable(int trackId);
int soundlib_set_track_volume(int trackId, float logVolume);

/* audio streams */
int soundlib_start_input_stream(int deviceIndex, float microphone_latency);
int soundlib_stop_input_stream();
int soundlib_start_output_stream(int deviceIndex, float microphone_latency);
int soundlib_stop_output_stream();
float soundlib_get_current_output_rms();

/* functions for input devices */
int soundlib_load_input_devices();
int soundlib_get_default_input_device_index();
int soundlib_get_num_input_devices();
char* soundlib_get_input_device_name(int index);
int soundlib_get_num_channels_of_input_device(int index);
void soundlib_get_available_input_devices(DeviceInfo* in_buffer);

/* functions for output devices */
int soundlib_load_output_devices();
int soundlib_get_default_output_device_index();
int soundlib_get_num_output_devices();
char* soundlib_get_output_device_name(int index);
int soundlib_get_num_channels_of_output_device(int index);
void soundlib_get_available_output_devices(DeviceInfo* in_buffer);

/* master control */
void soundlib_set_master_volume(float logVolume);

/* callbacks */
int soundlib_register_effect(int trackId, TrackAudioAvailableCallback effect);
int soundlib_register_input_ready_callback(int trackId, TrackAudioAvailableCallback callback);
int soundlib_register_output_ready_callback(int trackId, TrackAudioAvailableCallback callback);
int soundlib_register_master_output_ready_callback(MasterAudioAvailableCallback callback);

#ifdef __cplusplus
}
#endif

#endif
