#pragma once
#include <Base/Types.h>

#define MA_DATA_CONVERTER_STACK_BUFFER_SIZE
#include "miniaudio/miniaudio.h"

#include <string>

namespace AUDIO
{
    class AudioConvert
    {
    public:
        bool ConvertAudio(const void* data, size_t size, const std::string& outputPath);
    private:
        void Convert(ma_decoder* decoder, ma_encoder* encoder);
    };
}
