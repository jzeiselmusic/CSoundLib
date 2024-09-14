#include "csoundlib.h"
#include <stdint.h>
#include "track.h"
#include "state.h"

int soundlib_register_effect(int trackId, TrackAudioAvailableCallback effect) {
    /* add effect to track */
    const char key[50];
    ht_getkey(trackId, key);
    trackObject* track_p = (trackObject*)ht_get(csoundlib_state->track_hash_table, key);
    if (track_p == NULL) return SoundIoErrorTrackNotFound;
    track_p->track_effects.track_effect_list[track_p->track_effects.num_effects] = effect;
    track_p->track_effects.num_effects += 1;
    return SoundIoErrorNone;
}

int soundlib_register_input_ready_callback(int trackId, TrackAudioAvailableCallback callback) {
    const char key[50];
    ht_getkey(trackId, key);
    trackObject* track_p = (trackObject*)ht_get(csoundlib_state->track_hash_table, key);
    if (track_p == NULL) return SoundIoErrorTrackNotFound;
    track_p->input_ready_callback = callback;
    return SoundIoErrorNone;
}

int soundlib_register_output_ready_callback(int trackId, TrackAudioAvailableCallback callback) {
    const char key[50];
    ht_getkey(trackId, key);
    trackObject* track_p = (trackObject*)ht_get(csoundlib_state->track_hash_table, key);
    if (track_p == NULL) return SoundIoErrorTrackNotFound;
    track_p->output_ready_callback = callback;
    return SoundIoErrorNone;
}

int soundlib_register_master_output_ready_callback(MasterAudioAvailableCallback callback) {
    csoundlib_state->output_callback = callback;
    return SoundIoErrorNone;
}

int soundlib_register_master_effect(MasterAudioAvailableCallback effect) {
    csoundlib_state->master_effects.master_effect_list[csoundlib_state->master_effects.num_effects] = effect;
    csoundlib_state->master_effects.num_effects += 1;
    return SoundIoErrorNone;
}