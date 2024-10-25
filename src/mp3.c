#include "mp3.h"
#include "csoundlib.h"
#include <stdio.h>
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswresample/swresample.h>
#include <libavutil/opt.h>

void open_mp3_file(const char *path, CslFileInfo* info) {
    AVFormatContext *format_ctx = NULL;
    if (avformat_open_input(&format_ctx, path, NULL, NULL) < 0) {
        printf("Could not open input file\n");
        return;
    }
    if (avformat_find_stream_info(format_ctx, NULL) < 0) {
        printf("Could not find stream information\n");
        return;
    }

    // Find the first audio stream
    int audio_stream_index = -1;
    for (int i = 0; i < format_ctx->nb_streams; i++) {
        if (format_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            audio_stream_index = i;
            break;
        }
    }
    if (audio_stream_index == -1) {
        printf("No audio stream found\n");
        return;
    }

    // Get codec context
    AVCodecParameters *codecpar = format_ctx->streams[audio_stream_index]->codecpar;
    AVCodec *codec = avcodec_find_decoder(codecpar->codec_id);
    if (!codec) {
        printf("Unsupported codec\n");
        return;
    }
    AVCodecContext *codec_ctx = avcodec_alloc_context3(codec);
    avcodec_parameters_to_context(codec_ctx, codecpar);
    if (avcodec_open2(codec_ctx, codec, NULL) < 0) {
        printf("Could not open codec\n");
        return;
    }

    // Set up resampling context for conversion to PCM
    SwrContext *swr_ctx = swr_alloc();
    if (!swr_ctx) {
        printf("context was not allocated\n");
        return;
    }
    AVChannelLayout stereo_layout = (AVChannelLayout)AV_CHANNEL_LAYOUT_STEREO;
    AVChannelLayout in_layout = (codec_ctx->ch_layout);
    av_opt_set_chlayout(swr_ctx, "in_chlayout", &in_layout, 0);
    av_opt_set_chlayout(swr_ctx, "out_chlayout", &stereo_layout, 0);  
    av_opt_set_int(swr_ctx, "in_sample_rate", codec_ctx->sample_rate, 0);
    av_opt_set_int(swr_ctx, "out_sample_rate", 44100, 0);
    av_opt_set_sample_fmt(swr_ctx, "in_sample_fmt", codec_ctx->sample_fmt, 0);
    av_opt_set_sample_fmt(swr_ctx, "out_sample_fmt", AV_SAMPLE_FMT_S32, 0);
    int ret = swr_init(swr_ctx);
    if (ret < 0) {
        printf("context was not initialized\n");
        return;
    }
    // Read frames and decode
    AVPacket* packet = av_packet_alloc();
    AVFrame *frame = av_frame_alloc();
    uint8_t *input_buffer = NULL;
    int input_buffer_size = 0;
    int total_frames = 0;
    while (av_read_frame(format_ctx, packet) >= 0) {
        if (packet->stream_index == audio_stream_index) {
            if (avcodec_send_packet(codec_ctx, packet) == 0) {
                while (avcodec_receive_frame(codec_ctx, frame) == 0) {
                    // Allocate buffer for converted samples
                    int input_samples = swr_get_out_samples(swr_ctx, frame->nb_samples);
                    input_buffer_size = av_samples_alloc(&input_buffer, NULL, 2, input_samples, AV_SAMPLE_FMT_S32, 0);
                    int converted_samples = swr_convert(swr_ctx, &input_buffer, input_samples, 
                                                        (const uint8_t **)frame->extended_data, frame->nb_samples);
                    int converted_buffer_size = av_samples_get_buffer_size(NULL, 2, converted_samples, AV_SAMPLE_FMT_S32, 0);
                    total_frames += (converted_buffer_size / (2 * 4)); // 2 for channels and 4 for bytes in 32b sample
                    av_freep(&input_buffer);
                }
            }
        }
        av_packet_unref(packet);
    }
    // Clean up
    swr_free(&swr_ctx);
    av_frame_free(&frame);
    avcodec_free_context(&codec_ctx);
    avformat_close_input(&format_ctx);

    info->data_type = CSL_S32;
    info->sample_rate = CSL_SR44100;
    info->path = path;
    info->num_channels = 2;
    info->file_type = CSL_MP3;
    info->num_frames = total_frames;
}