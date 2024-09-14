#include "track.h"
#include "state.h"
#include "errors.h"
#include "csl_util.h"

static inline void dummy_callback(
    int trackId,
    unsigned char *buffer, 
    size_t length, 
    CSL_DTYPE data_type, 
    CSL_SR sample_rate, 
    size_t num_channels
) {};

int soundlib_add_track(int trackId) {
    trackObject* tp = malloc(sizeof(trackObject));
    TrackAudioAvailableCallback* allocated_effects = (TrackAudioAvailableCallback*)malloc(MAX_NUM_EFFECTS * sizeof(TrackAudioAvailableCallback));
    trackObject track =
        {
            .track_id = trackId,
            .volume = 1.0,
            .solo_enabled = false,
            .mute_enabled = false,
            .input_device_index = soundlib_get_default_input_device_index(),
            .input_channel_index = 0,
            .current_rms_levels = {0.0, 0.0},
            .input_buffer.buffer = {0},
            .input_buffer.write_bytes = 0,
            .track_effects.track_effect_list = allocated_effects,
            .track_effects.num_effects = 0,
            .input_ready_callback = &dummy_callback,
            .output_ready_callback = &dummy_callback
        };
    *tp = track;

    /* csoundlib_state->list_of_track_objects[csoundlib_state->num_tracks] = track;
    csoundlib_state->num_tracks += 1; */
    const char key[50];
    ht_getkey(trackId, key);
    ht_set(csoundlib_state->track_hash_table, key, (void*)(tp));
    return SoundIoErrorNone;
}

int soundlib_delete_track(int trackId) {
    const char key[50];
    ht_getkey(trackId, key);
    trackObject* track_p = (trackObject*)ht_get(csoundlib_state->track_hash_table, key);
    if (track_p == NULL) return SoundIoErrorTrackNotFound;

    track_p->track_effects.num_effects = 0;
    free(track_p->track_effects.track_effect_list);

    /* ht remove frees track_p */
    ht_remove(csoundlib_state->track_hash_table, key);
    csoundlib_state->num_tracks -= 1;
    return SoundIoErrorNone;
}

static int _deleteTrack(const char* key) {
    trackObject* track_p = (trackObject*)ht_get(csoundlib_state->track_hash_table, key);
    if (track_p == NULL) return SoundIoErrorTrackNotFound;

    track_p->track_effects.num_effects = 0;
    free(track_p->track_effects.track_effect_list);

    /* ht remove frees track_p */
    ht_remove(csoundlib_state->track_hash_table, key);
    csoundlib_state->num_tracks -= 1;
    return SoundIoErrorNone;
}

int soundlib_choose_input_device(int trackId, int device_index) {
    const char key[50];
    ht_getkey(trackId, key);
    trackObject* track_p = (trackObject*)ht_get(csoundlib_state->track_hash_table, key);
    if (track_p == NULL) return SoundIoErrorTrackNotFound;
    track_p->input_device_index = device_index;
    return SoundIoErrorNone;
}

int soundlib_choose_input_channel(int trackId, int channel_index) {
    const char key[50];
    ht_getkey(trackId, key);
    trackObject* track_p = (trackObject*)ht_get(csoundlib_state->track_hash_table, key);
    if (track_p == NULL) return SoundIoErrorTrackNotFound;
    track_p->input_channel_index = channel_index;
    return SoundIoErrorNone;
}

float soundlib_get_track_input_rms(int trackId) {
    const char key[50];
    ht_getkey(trackId, key);
    trackObject* track_p = (trackObject*)ht_get(csoundlib_state->track_hash_table, key);
    if (track_p == NULL) return 0.0;
    return track_p->current_rms_levels.input_rms_level;
}

float soundlib_get_track_output_rms(int trackId) {
    const char key[50];
    ht_getkey(trackId, key);
    trackObject* track_p = (trackObject*)ht_get(csoundlib_state->track_hash_table, key);
    if (track_p == NULL) return 0.0;
    return track_p->current_rms_levels.output_rms_level;
}

int soundlib_solo_enable(int trackId) {
    const char key[50];
    ht_getkey(trackId, key);
    trackObject* track_p = (trackObject*)ht_get(csoundlib_state->track_hash_table, key);
    if (track_p == NULL) return SoundIoErrorTrackNotFound;
    track_p->solo_enabled = true;
    csoundlib_state->tracks_solod += 1;
    csoundlib_state->solo_engaged = true;
    return SoundIoErrorNone;
}

int soundlib_solo_disable(int trackId) {
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

int soundlib_mute_enable(int trackId) {
    const char key[50];
    ht_getkey(trackId, key);
    trackObject* track_p = (trackObject*)ht_get(csoundlib_state->track_hash_table, key);
    if (track_p == NULL) return SoundIoErrorTrackNotFound;
    track_p->mute_enabled = true;
    return SoundIoErrorNone;
}

int soundlib_mute_disable(int trackId) {
    const char key[50];
    ht_getkey(trackId, key);
    trackObject* track_p = (trackObject*)ht_get(csoundlib_state->track_hash_table, key);
    if (track_p == NULL) return SoundIoErrorTrackNotFound;
    track_p->mute_enabled = false;
    return SoundIoErrorNone;
}

int soundlib_set_track_volume(int trackId, float logVolume) {
    const char key[50];
    ht_getkey(trackId, key);
    trackObject* track_p = (trackObject*)ht_get(csoundlib_state->track_hash_table, key);
    if (track_p == NULL) return SoundIoErrorTrackNotFound;
    /* turn db volume into magnitude volume */
    float mag = log_to_mag(logVolume);
    track_p->volume = mag;
    return SoundIoErrorNone;
}

void soundlib_set_master_volume(float logVolume) {
    csoundlib_state->master_volume = log_to_mag(logVolume);
}

void soundlib_delete_all_tracks(void) {
    /* only frees the sound files from the track and frees memory hold track.
    *  does not call ht_destroy()
    */
    hti it = ht_iterator(csoundlib_state->track_hash_table);
    while( ht_next(&it) ) {
        _deleteTrack(it.key);
    }
}