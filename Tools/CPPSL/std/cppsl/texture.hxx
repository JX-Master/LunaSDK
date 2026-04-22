#pragma once
#include <cppsl/core.hxx>

namespace cppsl
{
    struct SamplerState {};

    template <typename T>
    struct Texture2D
    {
        T Sample(SamplerState sampler, float2 uv) const;
        T Load(uint2 pixel) const;
    };

    template <typename T>
    struct RWTexture2D
    {
        void Store(uint2 pixel, T value);
    };
}
