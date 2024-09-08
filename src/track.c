#include "track.h"
#include "state.h"
#include "errors.h"
#include "csl_util.h"

extern audio_state* csoundlib_state;

int lib_addTrack(int trackId) {
    trackObject* tp = malloc(sizeof(trackObject));
    EffectPointer* allocated_effects = (EffectPointer*)malloc(50 * sizeof(EffectPointer));
    trackObject track =
        {
            .track_id = trackId,
            .volume = 1.0,
            .solo_enabled = false,
            .mute_enabled = false,
            .input_device_index = lib_getDefaultInputDeviceIndex(),
            .input_channel_index = 0,
            .current_rms_levels = {0.0, 0.0},
            .input_buffer.buffer = {0},
            .input_buffer.write_bytes = 0,
            .effect_list = allocated_effects,
            .num_effects = 0
        };
    *tp = track;

    /* csoundlib_state->list_of_track_objects[csoundlib_state->num_tracks] = track;
    csoundlib_state->num_tracks += 1; */
    const char key[50];
    ht_getkey(trackId, key);
    ht_set(csoundlib_state->track_hash_table, key, (void*)(tp));
    return SoundIoErrorNone;
}

int lib_deleteTrack(int trackId) {
    const char key[50];
    ht_getkey(trackId, key);
    trackObject* track_p = (trackObject*)ht_get(csoundlib_state->track_hash_table, key);
    if (track_p == NULL) return SoundIoErrorTrackNotFound;

    track_p->num_effects = 0;
    free(track_p->effect_list);

    /* ht remove frees track_p */
    ht_remove(csoundlib_state->track_hash_table, key);
    csoundlib_state->num_tracks -= 1;
    return SoundIoErrorNone;
}

static int _deleteTrack(const char* key) {
    trackObject* track_p = (trackObject*)ht_get(csoundlib_state->track_hash_table, key);
    if (track_p == NULL) return SoundIoErrorTrackNotFound;

    track_p->num_effects = 0;
    free(track_p->effect_list);

    /* ht remove frees track_p */
    ht_remove(csoundlib_state->track_hash_table, key);
    csoundlib_state->num_tracks -= 1;
    return SoundIoErrorNone;
}

int lib_trackChooseInputDevice(int trackId, int device_index) {
    const char key[50];
    ht_getkey(trackId, key);
    trackObject* track_p = (trackObject*)ht_get(csoundlib_state->track_hash_table, key);
    if (track_p == NULL) return SoundIoErrorTrackNotFound;
    track_p->input_device_index = device_index;
    return SoundIoErrorNone;
}

int lib_trackChooseInputChannel(int trackId, int channel_index) {
    const char key[50];
    ht_getkey(trackId, key);
    trackObject* track_p = (trackObject*)ht_get(csoundlib_state->track_hash_table, key);
    if (track_p == NULL) return SoundIoErrorTrackNotFound;
    track_p->input_channel_index = channel_index;
    return SoundIoErrorNone;
}

float lib_getRmsVolumeTrackInput(int trackId) {
    const char key[50];
    ht_getkey(trackId, key);
    trackObject* track_p = (trackObject*)ht_get(csoundlib_state->track_hash_table, key);
    if (track_p == NULL) return 0.0;
    return track_p->current_rms_levels.input_rms_level;
}

float lib_getRmsVolumeTrackOutput(int trackId) {
    const char key[50];
    ht_getkey(trackId, key);
    trackObject* track_p = (trackObject*)ht_get(csoundlib_state->track_hash_table, key);
    if (track_p == NULL) return 0.0;
    return track_p->current_rms_levels.output_rms_level;
}

int lib_soloEnable(int trackId) {
    const char key[50];
    ht_getkey(trackId, key);
    trackObject* track_p = (trackObject*)ht_get(csoundlib_state->track_hash_table, key);
    if (track_p == NULL) return SoundIoErrorTrackNotFound;
    track_p->solo_enabled = true;
    csoundlib_state->tracks_solod += 1;
    csoundlib_state->solo_engaged = true;
    return SoundIoErrorNone;
}

int lib_soloDisable(int trackId) {
    const char key[50];
    ht_getkey(trackId, key);
    trackObject* track_p = (trackObject*)ht_get(csoundlib_state->track_hash_table, key);
    if (track_p == NULL) return SoundIoErrorTrackNotFound;
    track_p->solo_enabled = false;
    csoundlib_state->tracks_solod -= 1;
    if (csoundlib_state->tracks_solod > 0) csoundlib_state->solo_engaged = true;
    else csoundlib_state->solo_engaged = false;
    return SoundIoErrorNone;
}

int lib_muteEnable(int trackId) {
    const char key[50];
    ht_getkey(trackId, key);
    trackObject* track_p = (trackObject*)ht_get(csoundlib_state->track_hash_table, key);
    if (track_p == NULL) return SoundIoErrorTrackNotFound;
    track_p->mute_enabled = true;
    return SoundIoErrorNone;
}

int lib_muteDisable(int trackId) {
    const char key[50];
    ht_getkey(trackId, key);
    trackObject* track_p = (trackObject*)ht_get(csoundlib_state->track_hash_table, key);
    if (track_p == NULL) return SoundIoErrorTrackNotFound;
    track_p->mute_enabled = false;
    return SoundIoErrorNone;
}

int lib_setTrackVolume(int trackId, float logVolume) {
    const char key[50];
    ht_getkey(trackId, key);
    trackObject* track_p = (trackObject*)ht_get(csoundlib_state->track_hash_table, key);
    if (track_p == NULL) return SoundIoErrorTrackNotFound;
    /* turn db volume into magnitude volume */
    float mag = log_to_mag(logVolume);
    track_p->volume = mag;
    return SoundIoErrorNone;
}

void lib_setMasterVolume(float logVolume) {
    csoundlib_state->master_volume = log_to_mag(logVolume);
}

void lib_deleteAllTracks(void) {
    /* only frees the sound files from the track and frees memory hold track.
    *  does not call ht_destroy()
    */
    hti it = ht_iterator(csoundlib_state->track_hash_table);
    while( ht_next(&it) ) {
        _deleteTrack(it.key);
    }
}

int lib_registerEffect(int trackId, EffectPointer effect) {
    /* add effect to track */
    const char key[50];
    ht_getkey(trackId, key);
    trackObject* track_p = (trackObject*)ht_get(csoundlib_state->track_hash_table, key);
    if (track_p == NULL) return SoundIoErrorTrackNotFound;
    track_p->effect_list[track_p->num_effects] = effect;
    track_p->num_effects += 1;
    return SoundIoErrorNone;
}