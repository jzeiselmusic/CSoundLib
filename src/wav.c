#include "wav.h"
#include <stdio.h>
#include "csoundlib.h"
#include "errors.h"
#include <string.h>

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

int open_wav_file(const char* path, CslFileInfo* info) {
    FILE* fp = fopen(path, "rb");
    if (fp == NULL) return CSLErrorFileNotFound;
    info->path = path;
    info->file_type = CSL_WAV;

    wavHeader header;
    fread(&header.riff_header, sizeof(header.riff_header), 1, fp);
    fread(&header.wav_size, sizeof(header.wav_size), 1, fp);
    fread(&header.wave_header, sizeof(header.wave_header), 1, fp);
    fread(&header.fmt_header, sizeof(header.fmt_header), 1, fp);
    fread(&header.fmt_chunk_size, sizeof(header.fmt_chunk_size), 1, fp);
    fread(&header.audio_format, sizeof(header.audio_format), 1, fp);
    fread(&header.num_channels, sizeof(header.num_channels), 1, fp);
    fread(&header.sample_rate, sizeof(header.sample_rate), 1, fp);
    fread(&header.byte_rate, sizeof(header.byte_rate), 1, fp);
    fread(&header.sample_alignment, sizeof(header.sample_alignment), 1, fp);
    fread(&header.bit_depth, sizeof(header.bit_depth), 1, fp);

    while (fread(&header.data_header, sizeof(header.data_header), 1, fp)) {
        fread(&header.data_bytes, sizeof(header.data_bytes), 1, fp);
        if (strncmp(header.data_header, "data", 4) == 0) {
            break;
        } else {
            // Skip unknown chunk by seeking forward
            fseek(fp, header.data_bytes, SEEK_CUR);
        }
    }
    // found data. now read into buffer
    fread(info->data, sizeof(unsigned char), header.data_bytes, fp);

    fclose(fp);

    if (header.sample_rate == 44100) {
        info->sample_rate = CSL_SR44100;
    }
    else if (header.sample_rate == 48000) {
        info->sample_rate = CSL_SR48000;
    }
    else {
        return CSLErrorSettingSampleRate;
    }

    /*

    8 bit (or lower) WAV files are always unsigned. 9 bit or higher are always signed
    
    */
    if (header.bit_depth == 8) {
        info->data_type = CSL_U8;
    }
    else if (header.bit_depth == 16) {
        info->data_type = CSL_S16;
    }
    else if (header.bit_depth == 24) {
        info->data_type = CSL_S24;
    }
    else if (header.bit_depth == 32) {
        info->data_type = CSL_S32;
    }
    else {
        return CSLErrorSettingBitDepth;
    }

    info->num_channels = header.num_channels;
    /* each frame has N samples where N is number of channels */
    /* this value divided by sample rate is the number of seconds in the track */
    info->num_frames = header.data_bytes / (header.num_channels * (header.bit_depth / 8));
}