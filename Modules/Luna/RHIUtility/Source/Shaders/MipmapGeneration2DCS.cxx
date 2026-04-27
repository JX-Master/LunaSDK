#include <cppsl/core.hxx>
#include <cppsl/math.hxx>
#include <cppsl/texture.hxx>

using namespace cppsl;

struct MipmapCB
{
    float2 texel_size;
};

struct DescSet0
{
    [[cppsl::cbuffer, cppsl::binding(0)]]
    MipmapCB g_cb;

    [[cppsl::binding(1)]]
    Texture2D<float4> g_src_tex;

    [[cppsl::binding(2)]]
    RWTexture2D<float4> g_dst_tex;

    [[cppsl::binding(3)]]
    SamplerState g_sampler;
};

[[cppsl::desc_set(0)]]
DescSet0 g_set0;

[[cppsl::compute(8, 8, 1)]]
void cs_main([[cppsl::builtin(dispatch_thread_id)]] uint3 dispatch_thread_id)
{
    float2 texcoords;
    texcoords = g_set0.g_cb.texel_size * float2{dispatch_thread_id.x + 0.5f, dispatch_thread_id.y + 0.5f};

    float4 color;
    color = g_set0.g_src_tex.SampleLevel(g_set0.g_sampler, texcoords, 0.0f);

    g_set0.g_dst_tex.Store(dispatch_thread_id.xy, color);
}
