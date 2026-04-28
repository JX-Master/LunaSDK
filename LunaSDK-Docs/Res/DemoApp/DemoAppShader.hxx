#pragma once
#include <cppsl/core.hxx>
#include <cppsl/resource.hxx>
#include <cppsl/texture.hxx>

using namespace cppsl;

struct Camera
{
    float4x4 world_to_proj;
};

struct DemoAppDescSet0
{
    [[cppsl::cbuffer, cppsl::binding(0)]]
    Camera vertexBuffer;

    [[cppsl::binding(1)]]
    Texture2D<float4> tex;

    [[cppsl::binding(2)]]
    SamplerState tex_sampler;
};

[[cppsl::desc_set(0)]]
DemoAppDescSet0 g_set0;

struct VS_INPUT
{
    [[cppsl::location(0)]] float3 position;
    [[cppsl::location(1)]] float2 texcoord;
};

struct PS_INPUT
{
    [[cppsl::position]] float4 position;
    [[cppsl::location(1)]] float2 texcoord;
};
