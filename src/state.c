#include "state.h"
#include "csoundlib.h"

// keeps the state of the current audio session
audio_state* csoundlib_state;

float lib_getCurrentRmsOutput() {
    return csoundlib_state->current_rms_ouput;
}

void lib_setVolume(float volume) {
    csoundlib_state->track->volume = volume;
}

void lib_registerEffect(EffectPointer effect) {
    csoundlib_state->effect_list[csoundlib_state->num_effects] = effect;
    csoundlib_state->num_effects += 1;
}