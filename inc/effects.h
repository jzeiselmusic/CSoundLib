#ifndef EFFECTS_H
#define EFFECTS_H

#include "csoundlib.h"

typedef struct {
    TrackAudioAvailableCallback* track_effect_list;
    uint16_t num_effects;
} TrackEffectList;

typedef struct {
    MasterAudioAvailableCallback* master_effect_list;
    uint16_t num_effects;
} MasterEffectList;

#endif