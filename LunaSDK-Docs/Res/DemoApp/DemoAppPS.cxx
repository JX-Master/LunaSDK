#include "DemoAppShader.hxx"

struct PS_OUTPUT
{
    [[cppsl::location(0)]] float4 color;
};

[[cppsl::fragment]]
PS_OUTPUT ps_main(PS_INPUT v)
{
    PS_OUTPUT o;
    o.color = g_set0.tex.Sample(g_set0.tex_sampler, v.texcoord);
    return o;
}
