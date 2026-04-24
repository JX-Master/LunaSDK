#pragma once
#include "StudioCommon.hxx"
#include <cppsl/texture.hxx>

struct GeometryCB
{
    float4x4 world_to_view;
    float4x4 view_to_proj;
    float4x4 world_to_proj;
    float4x4 proj_to_world;
    float4x4 view_to_world;
    uint screen_width;
    uint screen_height;
};

struct GeometryMeshBuffer
{
    float4x4 model_to_world;
    float4x4 world_to_model;
};

struct GeometryMaterialParameters
{
    float emissive_intensity;
};

struct GeometryDescSet0
{
    [[cppsl::cbuffer, cppsl::binding(0)]]
    GeometryCB cb_param;

    [[cppsl::structured_buffer, cppsl::binding(1)]]
    const GeometryMeshBuffer* g_MeshBuffer;

    [[cppsl::binding(2)]]
    Texture2D<float4> g_base_color;

    [[cppsl::binding(3)]]
    Texture2D<float4> g_roughness;

    [[cppsl::binding(4)]]
    Texture2D<float4> g_normal;

    [[cppsl::binding(5)]]
    Texture2D<float4> g_metallic;

    [[cppsl::binding(6)]]
    Texture2D<float4> g_emissive;

    [[cppsl::binding(7)]]
    SamplerState g_sampler;

    [[cppsl::structured_buffer, cppsl::binding(8)]]
    const GeometryMaterialParameters* g_material_params;
};

[[cppsl::desc_set(0)]]
GeometryDescSet0 g_set0;
