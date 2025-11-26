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
            NC_LOG_WARNING("Decoder init failed with error '{}' for '{}'",
                (i16)result, outPath);

            return;
        }

        if (decoder.outputFormat == ma_encoding_format_wav)
            return;

        ma_encoder_config encoderConfig = ma_encoder_config_init(ma_encoding_format_wav, decoder.outputFormat, decoder.outputChannels, decoder.outputSampleRate);

        result = ma_encoder_init_file(outPath.c_str(), &encoderConfig, &encoder);
        if (result != MA_SUCCESS)
        {
            NC_LOG_WARNING("Encoder init failed with erro '{}' for '{}'",
                (i16)result, outPath);

            ma_decoder_uninit(&decoder);

            return;
        }

        ma_uint64 totalFrames;
        ma_decoder_get_length_in_pcm_frames(&decoder, &totalFrames);

        ma_uint32 frameSizeInBytes = ma_get_bytes_per_frame(decoder.outputFormat, decoder.outputChannels);

        void* frameBuffer = malloc(totalFrames * frameSizeInBytes);
        if (frameBuffer == nullptr)
        {
            NC_LOG_WARNING("frameBuffer is null for '{}'", outPath);

            ma_encoder_uninit(&encoder);
            ma_decoder_uninit(&decoder);

            return;
        }

        ma_uint64 framesRead;
        result = ma_decoder_read_pcm_frames(&decoder, frameBuffer, totalFrames, &framesRead);
        if (result != MA_SUCCESS && result != MA_AT_END)
        {
            NC_LOG_WARNING("Decode error '{}' for '{}'",
                (i16)result, outPath);
        }

        result = ma_encoder_write_pcm_frames(&encoder, frameBuffer, framesRead, nullptr);
        if (result != MA_SUCCESS)
        {
            NC_LOG_WARNING("Encode error '{}' for '{}'",
                (i16)result, outPath);
        }

        free(frameBuffer);
        ma_encoder_uninit(&encoder);
        ma_decoder_uninit(&decoder);
    }
}