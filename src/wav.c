#include "wav.h"
#include "errors.h"
#include "state.h"
#include <string.h>
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include "callbacks.h"
#include "csl_util.h"
#include "streams.h"
#include <fcntl.h>

typedef struct _writeArgs {
    FILE* fp;
    const char* bytes;
    int num_bytes;
} writeArgs;

static wavHeader _createWavHeader(int numSamples, int sampleRate, int bitDepth, int numChannels) {
    wavHeader header;
    header.riff_header[0] = 'R'; header.riff_header[1] = 'I'; header.riff_header[2] = 'F'; header.riff_header[3] = 'F';
    header.wav_size = 36 + numSamples * numChannels * (bitDepth / 8);
    header.wave_header[0] = 'W'; header.wave_header[1] = 'A'; header.wave_header[2] = 'V'; header.wave_header[3] = 'E';
    header.fmt_header[0] = 'f'; header.fmt_header[1] = 'm'; header.fmt_header[2] = 't'; header.fmt_header[3] = ' ';
    header.fmt_chunk_size = 16; // PCM
    header.audio_format = 1; // PCM
    header.num_channels = numChannels;
    header.sample_rate = sampleRate;
    header.byte_rate = sampleRate * numChannels * (bitDepth / 8);
    header.sample_alignment = numChannels * (bitDepth / 8);
    header.bit_depth = bitDepth;
    header.data_header[0] = 'd'; header.data_header[1] = 'a'; header.data_header[2] = 't'; header.data_header[3] = 'a';
    header.data_bytes = numSamples * numChannels * (bitDepth / 8);
    return header;
}