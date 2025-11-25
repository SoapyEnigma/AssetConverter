#include "AudioConvert.h"

#include <Base/Util/DebugHandler.h>

#include <miniaudio/miniaudio.h>

namespace Audio
{
    void AudioConvert::ConvertToWav(const void* data, size_t size, const std::string& outPath)
    {
        ma_result result;
        ma_decoder decoder;
        ma_encoder encoder;

        result = ma_decoder_init_memory(data, size, nullptr, &decoder);
        if (result != MA_SUCCESS)
        {
            NC_LOG_WARNING("Decoder init failed for '%s' (miniaudio error: %d)\n",
                outPath.c_str(), (u32)result, size);

            return;
        }

        ma_encoder_config encoderConfig = ma_encoder_config_init(ma_encoding_format_wav, decoder.outputFormat, decoder.outputChannels, decoder.outputSampleRate);

        result = ma_encoder_init_file(outPath.c_str(), &encoderConfig, &encoder);
        if (result != MA_SUCCESS)
        {
            NC_LOG_WARNING("Encoder init failed for '%s' (miniaudio error: %d)\n",
                outPath.c_str(), (u32)result);

            ma_decoder_uninit(&decoder);

            return;
        }

        ma_uint32 framesPerChunk = 4096;
        ma_uint32 frameSizeInBytes = ma_get_bytes_per_frame(decoder.outputFormat, decoder.outputChannels);

        void* frameBuffer = malloc((size_t)framesPerChunk * frameSizeInBytes);
        if (frameBuffer == nullptr)
        {
            NC_LOG_WARNING("Out of memory in ConvertToWav for '%s'\n", outPath.c_str());

            ma_encoder_uninit(&encoder);
            ma_decoder_uninit(&decoder);

            return;
        }

        for (;;)
        {
            ma_uint64 framesRead = 0;
            result = ma_decoder_read_pcm_frames(&decoder, frameBuffer, framesPerChunk, &framesRead);
            if (result != MA_SUCCESS && result != MA_AT_END)
            {
                NC_LOG_WARNING("Decode error for '%s' (miniaudio error: %d)\n",
                outPath.c_str(), (u32)result);

                break;
            }

            if (framesRead == 0)
                break;

            result = ma_encoder_write_pcm_frames(&encoder, frameBuffer, framesRead, NULL);
            if (result != MA_SUCCESS)
            {
                NC_LOG_WARNING("Encode error for '%s' (miniaudio error: %d)\n",
                outPath.c_str(), (u32)result);

                break;
            }
        }

        free(frameBuffer);
        ma_encoder_uninit(&encoder);
        ma_decoder_uninit(&decoder);
    }
}