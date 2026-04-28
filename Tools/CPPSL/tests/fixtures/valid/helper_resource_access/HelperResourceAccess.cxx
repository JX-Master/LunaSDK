#include <cppsl/core.hxx>
#include <cppsl/math.hxx>
#include <cppsl/texture.hxx>

using namespace cppsl;

struct Params
{
    float threshold;
};

struct MainSet
{
    [[cppsl::cbuffer, cppsl::binding(0)]]
    Params params;

    [[cppsl::binding(1)]]
    Texture2D<float4> src_tex;

    [[cppsl::binding(2)]]
    SamplerState linear_sampler;
};

[[cppsl::desc_set(0)]]
MainSet main_set;

float4 helper_sample(float2 uv)
{
    float4 color = main_set.src_tex.SampleLevel(main_set.linear_sampler, uv, 0.0f);
    if (color.x < main_set.params.threshold)
    {
        return float4{0.0f, 0.0f, 0.0f, 1.0f};
    }
    return color;
}

[[cppsl::compute(1, 1, 1)]]
void main_cs([[cppsl::builtin(dispatch_thread_id)]] uint3 dispatch_thread_id)
{
    float2 uv = float2{0.5f, 0.5f};
    float4 color = helper_sample(uv);
    (void)dispatch_thread_id;
    (void)color;
}
