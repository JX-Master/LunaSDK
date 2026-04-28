#include "DemoAppShader.hxx"
#include <cppsl/math.hxx>

[[cppsl::vertex]]
PS_INPUT vs_main(VS_INPUT v)
{
    PS_INPUT o;
    o.position = mul(g_set0.vertexBuffer.world_to_proj, float4{v.position, 1.0f});
    o.texcoord = v.texcoord;
    return o;
}
