#include "AudioConvert.h"

#include <Base/Util/DebugHandler.h>

namespace AUDIO
{
    bool AudioConvert::ConvertAudio(const void* data, size_t size, const std::string& outputPath)
    {
        ma_decoder_config decoderConfig;
        decoderConfig = ma_decoder_config_init(ma_format_unknown, 0, 0);
        decoderConfig.resampling.algorithm = ma_resample_algorithm_linear;
        decoderConfig.resampling.linear.lpfOrder = 8;

        ma_decoder decoder;
        ma_result result = ma_decoder_init_memory(data, size, &decoderConfig, &decoder);
        if (result != MA_SUCCESS)
        {
            ma_decoder_uninit(&decoder);
            NC_LOG_WARNING("ma_decoder_init_memory failed ({0}) for {1}", static_cast<i16>(result), outputPath);
            return false;
        }

        ma_encoder_config encoderConfig;
        encoderConfig = ma_encoder_config_init(ma_encoding_format_wav, decoder.outputFormat, decoder.outputChannels, decoder.outputSampleRate);

        ma_encoder encoder;
        result = ma_encoder_init_file(outputPath.c_str(), &encoderConfig, &encoder);
        if (result != MA_SUCCESS)
        {
            ma_encoder_uninit(&encoder);
            ma_decoder_uninit(&decoder);
            NC_LOG_WARNING("ma_encoder_init_file failed ({0}) for {1}", static_cast<i16>(result), outputPath);
            return false;
        }

        Convert(&decoder, &encoder);

        ma_encoder_uninit(&encoder);
        ma_decoder_uninit(&decoder);

        return true;
    }

    void AudioConvert::Convert(ma_decoder* decoder, ma_encoder* encoder)
    {
        for (;;)
        {
            u8 buffer[4096];
            u64 framesRead;
            u64 framesToRead = sizeof(buffer) / ma_get_bytes_per_frame(decoder->outputFormat, decoder->outputChannels);

            ma_result readResult = ma_decoder_read_pcm_frames(decoder, buffer, framesToRead, &framesRead);
            if (readResult == MA_AT_END)
            {
                if (framesRead > 0)
                {
                    ma_result writeResult = ma_encoder_write_pcm_frames(encoder, buffer, framesRead, nullptr);
                    if (writeResult != MA_SUCCESS)
                        NC_LOG_WARNING("ma_decoder_write_pcm_frames tail failed ({0})", static_cast<i16>(writeResult));
                }
                break;
            }

            if (readResult != MA_SUCCESS)
            {
                NC_LOG_WARNING("ma_decoder_read_pcm_frames failed ({0})", static_cast<i16>(readResult));
                break;
            }

            if (framesRead > 0)
            {
                ma_result writeResult = ma_encoder_write_pcm_frames(encoder, buffer, framesRead, nullptr);
                if (writeResult != MA_SUCCESS)
                {
                    NC_LOG_WARNING("ma_encoder_write_pcm_frames failed ({0})", static_cast<i16>(writeResult));
                    break;
                }
            }
        }
    }
}
