#include "state.h"
#include "csoundlib.h"

audio_state* csoundlib_state;

// keeps the state of the current audio session
float soundlib_get_current_output_rms() {
    return csoundlib_state->current_rms_ouput;
}