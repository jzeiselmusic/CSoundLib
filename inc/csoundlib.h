/**
 * @file soundlib.h
 * @brief A lightweight real-time sound processing library.
 *
 * This file contains the core API for a simple, lightweight sound library
 * that supports real-time audio processing. The library allows for session
 * management, track control, and audio I/O device management.
 */
#ifndef CSOUNDLIB_H
#define CSOUNDLIB_H

#ifdef __cplusplus
extern "C" {
#endif 

#include <stdint.h>
#include <stdlib.h>
#include <soundio/soundio.h>

#define MAX_NUM_EFFECTS                           50

/* errors.h */
/* 
#define SoundIoErrorNone 	                      0
#define SoundIoErrorNoMem 	                      1
#define SoundIoErrorInitAudioBackend 	          2
#define SoundIoErrorSystemResources 	          3
#define SoundIoErrorOpeningDevice 	              4
#define SoundIoErrorNoSuchDevice 	              5
#define SoundIoErrorInvalid 	                  6
#define SoundIoErrorBackendUnavailable 	          7
#define SoundIoErrorStreaming 	                  8
#define SoundIoErrorIncompatibleDevice 	          9
#define SoundIoErrorNoSuchClient 	              10
#define SoundIoErrorIncompatibleBackend 	      11
#define SoundIoErrorBackendDisconnected 	      12
#define SoundIoErrorInterrupted 	              13
#define SoundIoErrorUnderflow 	                  14
#define SoundIoErrorEncodingString 	              15
*/
#define CSLErrorDevicesNotInitialized             16
#define CSLErrorEnvironmentNotInitialized         17
#define CSLErrorIndexOutOfBounds                  18
#define CSLErrorDevicesNotLoaded                  19
#define CSLErrorInputMemoryNotAllocated           20
#define CSLErrorOutputMemoryNotAllocated          21
#define CSLErrorTrackNotFound                     22
#define CSLErrorOpeningFile                       23
#define CSLErrorFileNotFound                      24
#define CSLErrorReadingWavForMetronome            25
#define CSLErrorInputStream                       26
#define CSLErrorOutputStream                      27
#define CSLErrorLoadingInputDevices               28
#define CSLErrorLoadingOutputDevices              29
#define CSLErrorSettingSampleRate                 30

/**
 * @enum CSL_DTYPE
 * @brief Data types supported by the library.
 *
 * CSL types represent unsigned and signed integers of different sizes, as well as
 * floating point formats.
 */
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

/**
 * @enum CSL_SR
 * @brief Sample rates supported by the library.
 *
 * defines the sample rates for audio processing. Currently, it 
 * supports only 44.1 kHz and 48 kHz.
 */
typedef enum {
    SR44100,
    SR48000
} CSL_SR;

/**
 * @typedef TrackAudioAvailableCallback
 * @brief Callback for track-level audio data availability.
 *
 * This function pointer type defines the signature for a callback function 
 * that is triggered when audio data is available for a specific track.
 * 
 * This can be used as an effect function on a track, or as a callback function
 * when input or output data is available. 
 * 
 * Should be implemented by user and registered with one of the callback register functions.
 * 
 * @param trackId The ID of the track.
 * @param buffer The buffer containing the audio data.
 * @param length The length of the buffer.
 * @param data_type The type of audio data (from CSL_DTYPE).
 * @param sample_rate The sample rate of the audio (from CSL_SR).
 * @param num_channels The number of audio channels.
 */
typedef void (*TrackAudioAvailableCallback) (
    int trackId,
    unsigned char *buffer, 
    size_t length, 
    CSL_DTYPE data_type, 
    CSL_SR sample_rate, 
    size_t num_channels
);

/**
 * @typedef TrackAudioAvailableCallback
 * @brief Callback for track-level audio data availability.
 *
 * Same as above except not specific to an individual track.
 * When registered, will send audio data from master track buffer.
 * 
 * @param buffer The buffer containing the audio data.
 * @param length The length of the buffer.
 * @param data_type The type of audio data (from CSL_DTYPE).
 * @param sample_rate The sample rate of the audio (from CSL_SR).
 * @param num_channels The number of audio channels.
 */
typedef void (*MasterAudioAvailableCallback) (
    unsigned char *buffer,
    size_t length,
    CSL_DTYPE data_type,
    CSL_SR sample_rate,
    size_t num_channels
);

/**
 * @struct DeviceInfo
 * @brief Represents an audio input or output device.
 *
 * Basic information about an audio device.
 * 
 */
typedef struct {
    char* name;
    int index;
} DeviceInfo;


/**
 * @brief Starts a new real time audio session.
 *
 * This function initializes the sound library with the given sample rate 
 * and data type. It must be called before any other sound processing 
 * functions.
 *
 * @param sample_rate The desired sample rate (from CSL_SR).
 * @param data_type The data type for audio processing (from CSL_DTYPE).
 * @return SoundIoErrorNone (0) on success, non-zero on failure.
 */
int soundlib_start_session(CSL_SR sample_rate, CSL_DTYPE data_type);

/**
 * @brief Stops and cleans up the current sound session.
 *
 * This function destroys the current session, releasing
 * any resources that were allocated.
 *
 * @return SoundIoErrorNone (0) on success, non-zero on failure.
 */
int soundlib_destroy_session();

/**
 * @brief Gets the current audio backend in use.
 *
 * Returns the identifier for the current audio backend that
 * is being used by the library.
 * 
 * One of:
 *   (0) SoundIoBackendNone 
 *   (1) SoundIoBackendJack 
 *   (2) SoundIoBackendPulseAudio 
 *   (3) SoundIoBackendAlsa
 *   (4) SoundIoBackendCoreAudio 
 *   (5) SoundIoBackendWasapi
 *   (6) SoundIoBackendDummy 
 *
 * @return The backend identifier.
 */
int soundlib_get_current_backend();

/**
 * @brief Adds a new track to the session.
 *
 * Add new audio track to the current sound session using
 * the specified track ID.
 * 
 * Must have previously called soundlib_start_session.
 * 
 * Audio will not play until called soundlib_start_input_stream and 
 * soundlib_start_output_stream.
 *
 * @param trackId The ID of the new track.
 * @return SoundIoErrorNone (0) on success, non-zero on failure.
 */
int soundlib_add_track(int trackId);

/**
 * @brief Deletes an existing track from the session.
 *
 * Remove a track from the current session based on the 
 * specified track ID.
 *
 * @param trackId The ID of the track to be removed.
 * @return SoundIoErrorNone (0) on success, non-zero on failure if track doesn't exist.
 */
int soundlib_delete_track(int trackId);

/**
 * @brief Delete all existing tracks from session
 *
 * Call delete track on all current existing tracks.
 *
 */
void soundlib_delete_all_tracks(void);

/**
 * @brief Selects the input device for a specific track.
 *
 * Select an input device for a specified track by its device index.
 * (( does this need to restart stream ? ))
 * 
 * Get a list of available devices with soundlib_get_available_input_devices.
 *
 * @param trackId The ID of the track.
 * @param device_index The index of the input device.
 * @return SoundIoErrorNone (0) on success, non-zero on failure.
 */
int soundlib_choose_input_device(int trackId, int device_index);

/**
 * @brief Selects the input channel for a specific track.
 *
 * (( understand further how channels work ))
 * 
 * Get channel list with soundlib_get_num_channels_of_input_device.
 *
 * @param trackId The ID of the track.
 * @param channel_index The index of the channel for this device.
 * @return SoundIoErrorNone (0) on success, non-zero on failure.
 */
int soundlib_choose_input_channel(int trackId, int channel_index);

/**
 * @brief Return current track input stage RMS 
 *
 * @param trackId The ID of the track.
 * @return track RMS value between 0 and 1
 */
float soundlib_get_track_input_rms(int trackId);

/**
 * @brief Return current track output stage RMS 
 *
 * @param trackId The ID of the track.
 * @return track RMS value between 0 and 1
 */
float soundlib_get_track_output_rms(int trackId);

/**
 * @brief Solo this track
 *
 * @param trackId The ID of the track.
 * @return SoundIoErrorNone (0) on success, non-zero on failure.
 */
int soundlib_solo_enable(int trackId);

/**
 * @brief Unsolo this track
 *
 * @param trackId The ID of the track.
 * @return SoundIoErrorNone (0) on success, non-zero on failure.
 */
int soundlib_solo_disable(int trackId);

/**
 * @brief Mute this track
 *
 * @param trackId The ID of the track.
 * @return SoundIoErrorNone (0) on success, non-zero on failure.
 */
int soundlib_mute_enable(int trackId);

/**
 * @brief Unmute this track
 *
 * @param trackId The ID of the track.
 * @return SoundIoErrorNone (0) on success, non-zero on failure.
 */
int soundlib_mute_disable(int trackId);

/**
 * @brief Set volume for this track
 *
 * @param trackId The ID of the track.
 * @param logVolume Log value of the desired volume
 * @return SoundIoErrorNone (0) on success, non-zero on failure.
 */
int soundlib_set_track_volume(int trackId, float logVolume);


/* audio streams */

/**
 * @brief Start capturing input stream with a specific audio device
 *
 * @param deviceIndex The index of the desired input device.
 * @param microphone_latency Added latency to the system, set very low for real time applications
 * @return SoundIoErrorNone (0) on success, non-zero on failure.
 */
int soundlib_start_input_stream(int deviceIndex, float microphone_latency);

/**
 * @brief Stop whatever input stream has been started
 *
 * @return SoundIoErrorNone (0) on success, non-zero on failure.
 */
int soundlib_stop_input_stream();

/**
 * @brief Start sending output stream with a specific audio device. 
 * 
 * This will start sending audio to an output device (system speakers)
 *
 * @param deviceIndex The index of the desired output device.
 * @param microphone_latency Added latency to the system, set very low for real time applications
 * @return SoundIoErrorNone (0) on success, non-zero on failure.
 */
int soundlib_start_output_stream(int deviceIndex, float microphone_latency);

/**
 * @brief Stop whatever output stream has been started
 *
 * @return SoundIoErrorNone (0) on success, non-zero on failure.
 */
int soundlib_stop_output_stream();

/**
 * @brief Get RMS value of mixed output buffer (master)
 *
 * @return RMS value between 0 and 1
 */
float soundlib_get_current_output_rms();

/* functions for input devices */

/**
 * @brief Get index of defaut input device
 *
 * @return SoundIoErrorNone (0) on success, non-zero on failure.
 */
int soundlib_get_default_input_device_index();

/**
 * @brief Get number of input devices
 *
 * @return SoundIoErrorNone (0) on success, non-zero on failure.
 */
int soundlib_get_num_input_devices();

/**
 * @brief Get input device name by index
 *
 * @param deviceIndex The index of the desired input device.
 * @return Null terminated string
 */
char* soundlib_get_input_device_name(int index);

/**
 * @brief Get number of channels for input device by index
 *
 * @param deviceIndex The index of the desired input device.
 * @return Num channels
 */
int soundlib_get_num_channels_of_input_device(int index);

/**
 * @brief Receive information about available input devices
 *
 * @param in_buffer A pointer to allocated memory of DeviceInfo structs
 * @return Populate a list of DeviceInfo structs. User must preallocate space 
 * for total number of input devices to be returned, and pass pointer to this function.
 */
void soundlib_get_available_input_devices(DeviceInfo* in_buffer);

/* functions for output devices */

/**
 * @brief Get index of defaut output device
 *
 * @return SoundIoErrorNone (0) on success, non-zero on failure.
 */
int soundlib_get_default_output_device_index();

/**
 * @brief Get number of output devices
 *
 * @return SoundIoErrorNone (0) on success, non-zero on failure.
 */
int soundlib_get_num_output_devices();

/**
 * @brief Get output device name by index
 *
 * @param deviceIndex The index of the desired output device.
 * @return Null terminated string
 */
char* soundlib_get_output_device_name(int index);

/**
 * @brief Get number of channels for output device by index
 *
 * @param deviceIndex The index of the desired output device.
 * @return Num channels
 */
int soundlib_get_num_channels_of_output_device(int index);

/**
 * @brief Receive information about available input devices
 *
 * @param in_buffer A pointer to allocated memory of DeviceInfo structs
 * @return Populate a list of DeviceInfo structs. User must preallocate space 
 * for total number of input devices to be returned, and pass pointer to this function.
 */
void soundlib_get_available_output_devices(DeviceInfo* in_buffer);

/* master control */

/**
 * @brief Set volume for master output
 *
 * @param logVolume Log value of the desired volume
 */
void soundlib_set_master_volume(float logVolume);

/* callbacks */

/**
 * @brief Register a callback function that serves as an audio effect for a specific track
 *
 * @param trackId put the effect on track with this id
 * @param effect function pointer described by TrackAudioAvailableCallback
 * @return SoundIoErrorNone (0) on success, non-zero on failure.
 */
int soundlib_register_effect(int trackId, TrackAudioAvailableCallback effect);

/**
 * @brief Register a callback function that provides user with audio input to a specific track
 *
 * @param trackId audio to track with this id
 * @param callback function pointer described by TrackAudioAvailableCallback
 * @return SoundIoErrorNone (0) on success, non-zero on failure.
 */
int soundlib_register_input_ready_callback(int trackId, TrackAudioAvailableCallback callback);

/**
 * @brief Register a callback function that provides user with audio output from a specific track.
 * This audio buffer will be post any effects and volume applied.
 *
 * @param trackId audio from track with this id
 * @param callback function pointer described by TrackAudioAvailableCallback
 * @return SoundIoErrorNone (0) on success, non-zero on failure.
 */
int soundlib_register_output_ready_callback(int trackId, TrackAudioAvailableCallback callback);

/**
 * @brief Register a callback function that provides user with audio output from the master mix.
 * This audio buffer will be post any effects and volume applied.
 *
 * @param callback function pointer described by MasterAudioAvailableCallback
 * @return SoundIoErrorNone (0) on success, non-zero on failure.
 */
int soundlib_register_master_output_ready_callback(MasterAudioAvailableCallback callback);

/**
 * @brief Register a callback function that serves as an audio effect for a the master buffer
 *
 * @param effect function pointer described by MasterAudioAvailableCallback
 * @return SoundIoErrorNone (0) on success, non-zero on failure.
 */
int soundlib_register_master_effect(MasterAudioAvailableCallback effect);

#ifdef __cplusplus
}
#endif

#endif
