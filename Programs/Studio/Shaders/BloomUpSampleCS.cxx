#include "StudioCommon.hxx"
#include <cppsl/texture.hxx>

struct CB
{
    uint src_tex_width;
    uint src_tex_height;
    uint dst_tex_width;
    uint dst_tex_height;
    float up_sample_radius;
};

struct DescSet0
{
    [[cppsl::cbuffer, cppsl::binding(0)]]
    CB cb;

    [[cppsl::binding(1)]]
    Texture2D<float4> g_upsample_tex;

    [[cppsl::binding(2)]]
    Texture2D<float4> g_bloom_tex;

    [[cppsl::binding(3)]]
    RWTexture2D<float4> g_dst_tex;

    [[cppsl::binding(4)]]
    SamplerState g_sampler;
};

[[cppsl::desc_set(0)]]
DescSet0 g_set0;

uint clamp_u(uint value, uint max_value)
{
    return value > max_value ? max_value : value;
}

float3 filter_upsample(uint2 pos, uint2 tex_size)
{
    uint2 max_pos = uint2{tex_size.x - 1u, tex_size.y - 1u};
    uint left = pos.x > 0u ? pos.x - 1u : 0u;
    uint right = clamp_u(pos.x + 1u, max_pos.x);
    uint up = pos.y > 0u ? pos.y - 1u : 0u;
    uint down = clamp_u(pos.y + 1u, max_pos.y);

    float3 color = float3{0.0f, 0.0f, 0.0f};
    color += xyz(g_set0.g_upsample_tex.Load(pos)) * 4.0f;
    color += xyz(g_set0.g_upsample_tex.Load(uint2{right, pos.y})) * 2.0f;
    color += xyz(g_set0.g_upsample_tex.Load(uint2{left, pos.y})) * 2.0f;
    color += xyz(g_set0.g_upsample_tex.Load(uint2{pos.x, down})) * 2.0f;
    color += xyz(g_set0.g_upsample_tex.Load(uint2{pos.x, up})) * 2.0f;
    color += xyz(g_set0.g_upsample_tex.Load(uint2{right, down}));
    color += xyz(g_set0.g_upsample_tex.Load(uint2{right, up}));
    color += xyz(g_set0.g_upsample_tex.Load(uint2{left, down}));
    color += xyz(g_set0.g_upsample_tex.Load(uint2{left, up}));
    return color / 16.0f;
}

float3 filter_bloom(uint2 pos, uint2 tex_size)
{
    uint2 max_pos = uint2{tex_size.x - 1u, tex_size.y - 1u};
    uint left = pos.x > 0u ? pos.x - 1u : 0u;
    uint right = clamp_u(pos.x + 1u, max_pos.x);
    uint up = pos.y > 0u ? pos.y - 1u : 0u;
    uint down = clamp_u(pos.y + 1u, max_pos.y);

    float3 color = float3{0.0f, 0.0f, 0.0f};
    color += xyz(g_set0.g_bloom_tex.Load(pos)) * 4.0f;
    color += xyz(g_set0.g_bloom_tex.Load(uint2{right, pos.y})) * 2.0f;
    color += xyz(g_set0.g_bloom_tex.Load(uint2{left, pos.y})) * 2.0f;
    color += xyz(g_set0.g_bloom_tex.Load(uint2{pos.x, down})) * 2.0f;
    color += xyz(g_set0.g_bloom_tex.Load(uint2{pos.x, up})) * 2.0f;
    color += xyz(g_set0.g_bloom_tex.Load(uint2{right, down}));
    color += xyz(g_set0.g_bloom_tex.Load(uint2{right, up}));
    color += xyz(g_set0.g_bloom_tex.Load(uint2{left, down}));
    color += xyz(g_set0.g_bloom_tex.Load(uint2{left, up}));
    return color / 16.0f;
}

[[cppsl::compute(8, 8, 1)]]
void cs_main([[cppsl::builtin(dispatch_thread_id)]] uint3 dispatch_thread_id)
{
    uint2 pixel = xy_u(dispatch_thread_id);
    float3 prev_mip = filter_upsample(uint2{dispatch_thread_id.x / 2u, dispatch_thread_id.y / 2u}, uint2{g_set0.cb.src_tex_width, g_set0.cb.src_tex_height});
    float3 current_mip = filter_bloom(pixel, uint2{g_set0.cb.dst_tex_width, g_set0.cb.dst_tex_height});
    g_set0.g_dst_tex.Store(pixel, make_float4(prev_mip + current_mip, 1.0f));
}
