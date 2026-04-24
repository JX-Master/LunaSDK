#pragma once
#include <cppsl/core.hxx>
#include <cppsl/math.hxx>

using namespace cppsl;

struct WireframeCameraCB
{
    float4x4 world_to_view;
    float4x4 view_to_proj;
    float4x4 world_to_proj;
    float4x4 proj_to_world;
    float4x4 view_to_world;
    float4 env_light_color;
    uint screen_width;
    uint screen_height;
};

struct WireframeMeshBuffer
{
    float4x4 model_to_world;
    float4x4 world_to_model;
};

struct WireframeDescSet0
{
    [[cppsl::cbuffer, cppsl::binding(0)]]
    WireframeCameraCB g_cb;

    [[cppsl::structured_buffer, cppsl::binding(1)]]
    const WireframeMeshBuffer* g_MeshBuffer;
};

[[cppsl::desc_set(0)]]
WireframeDescSet0 g_set0;
