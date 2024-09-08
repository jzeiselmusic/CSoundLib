#include "state.h"
#include "csoundlib.h"

// keeps the state of the current audio session
audio_state* csoundlib_state;

float lib_getCurrentRmsOutput() {
    return csoundlib_state->current_rms_ouput;
}