#include <cppsl/core.hxx>
#include <cppsl/math.hxx>
#include <cppsl/texture.hxx>

using namespace cppsl;

struct MipmapCB
{
    float texel_size;
};

[[cppsl::cbuffer, cppsl::desc_set(0), cppsl::binding(0)]]
MipmapCB g_cb;

[[cppsl::desc_set(0), cppsl::binding(1)]]
Texture1D<float4> g_src_tex;

[[cppsl::desc_set(0), cppsl::binding(2)]]
RWTexture1D<float4> g_dst_tex;

[[cppsl::desc_set(0), cppsl::binding(3)]]
SamplerState g_sampler;

[[cppsl::compute(8, 1, 1)]]
void cs_main([[cppsl::builtin(dispatch_thread_id)]] uint3 dispatch_thread_id)
{
    float texcoords;
    texcoords = g_cb.texel_size * (dispatch_thread_id.x + 0.5f);

    float4 color;
    color = g_src_tex.SampleLevel(g_sampler, texcoords, 0.0f);

    g_dst_tex.Store(dispatch_thread_id.x, color);
}
