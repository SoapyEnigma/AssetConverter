#pragma once

#include <Base/Types.h>

namespace Audio
{
    class AudioConvert
    {
    public:
        void ConvertToWav(const void* data, size_t size, const std::string& outPath);

    private:
    };
}
