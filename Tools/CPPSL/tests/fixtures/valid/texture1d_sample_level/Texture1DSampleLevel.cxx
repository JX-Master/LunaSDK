#include <cppsl/core.hxx>
#include <cppsl/math.hxx>
#include <cppsl/texture.hxx>

using namespace cppsl;

[[cppsl::desc_set(0), cppsl::binding(0)]]
Texture1D<float4> src_texture;

[[cppsl::desc_set(0), cppsl::binding(1)]]
RWTexture1D<float4> dst_texture;

[[cppsl::desc_set(0), cppsl::binding(2)]]
SamplerState linear_sampler;

[[cppsl::compute(8, 1, 1)]]
void main_cs([[cppsl::builtin(dispatch_thread_id)]] uint3 dispatch_thread_id)
{
    float uv = (dispatch_thread_id.x + 0.5f) * 0.25f;
    float4 color = src_texture.SampleLevel(linear_sampler, uv, 0.0f);
    dst_texture.Store(dispatch_thread_id.x, color);
}
