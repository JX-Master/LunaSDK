#include <cppsl/core.hxx>
#include <cppsl/math.hxx>
#include <cppsl/texture.hxx>

using namespace cppsl;

[[cppsl::desc_set(0), cppsl::binding(0)]]
Texture2D<float> src_tex;

[[cppsl::desc_set(0), cppsl::binding(1)]]
Texture2D<float> depth_tex;

[[cppsl::desc_set(0), cppsl::binding(2)]]
RWTexture2D<float> dst_tex;

[[cppsl::desc_set(0), cppsl::binding(3)]]
SamplerState linear_sampler;

[[cppsl::compute(8, 8, 1)]]
void main_cs([[cppsl::builtin(dispatch_thread_id)]] uint3 dispatch_thread_id)
{
    uint2 pixel = uint2{dispatch_thread_id.x, dispatch_thread_id.y};
    float2 uv = float2{0.5f, 0.5f};
    float sampled = src_tex.SampleLevel(linear_sampler, uv, 0.0f);
    float depth = depth_tex.Load(pixel);
    if (depth != 1.0f)
    {
        return;
    }
    dst_tex.Store(pixel, sampled);
}
