#include "state.h"
#include "csoundlib.h"

// keeps the state of the current audio session
audio_state* csoundlib_state;

float soundlib_get_current_output_rms() {
    return csoundlib_state->current_rms_ouput;
}