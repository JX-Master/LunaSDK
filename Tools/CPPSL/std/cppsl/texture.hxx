#pragma once
#include <cppsl/core.hxx>

namespace cppsl
{
    struct SamplerState {};

    template <typename T>
    struct Texture1D
    {
        T Sample(SamplerState sampler, float uv) const;
        T SampleLevel(SamplerState sampler, float uv, float lod) const;
        T Load(uint pixel) const;
    };

    template <typename T>
    struct Texture2D
    {
        T Sample(SamplerState sampler, float2 uv) const;
        T SampleLevel(SamplerState sampler, float2 uv, float lod) const;
        T Load(uint2 pixel) const;
    };

    template <typename T>
    struct DepthTexture2D
    {
        T Load(uint2 pixel) const;
    };

    template <typename T>
    struct Texture3D
    {
        T Sample(SamplerState sampler, float3 uv) const;
        T SampleLevel(SamplerState sampler, float3 uv, float lod) const;
        T Load(uint3 pixel) const;
    };

    template <typename T>
    struct RWTexture1D
    {
        T Load(uint pixel) const;
        void Store(uint pixel, T value);
    };

    template <typename T>
    struct RWTexture2D
    {
        T Load(uint2 pixel) const;
        void Store(uint2 pixel, T value);
    };

    template <typename T>
    struct RWTexture3D
    {
        T Load(uint3 pixel) const;
        void Store(uint3 pixel, T value);
    };
}
