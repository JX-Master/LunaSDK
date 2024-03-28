#include "Common.hlsl"

cbuffer SkyboxParams : register(b0)
{
    float4x4 g_view_to_world;
    float g_fov;
    uint g_width;
    uint g_height;
}
Texture2D<float4> g_skybox : register(t1);
Texture2D<float> g_depth : register(t2);
RWTexture2D<float4> g_lighting_tex : register(u3);
SamplerState g_sampler : register(s4);


// asin [-1,1] -> [-PI/2, PI/2]
// acos [-1,1] -> [0, PI]
// atan -> [-PI/2, PI/2]
[numthreads(8, 8, 1)]
void main(int3 dispatch_thread_id : SV_DispatchThreadID)
{
    if(g_depth[dispatch_thread_id.xy] != 1.0) return;
    
    float focus_len = g_width / (2.0f * tan(g_fov / 2.0f));

    float3 world_dir = normalize(float3((float)dispatch_thread_id.x - (float)(g_width / 2), -((float)dispatch_thread_id.y - (float)(g_height / 2)), focus_len));
    world_dir = mul(g_view_to_world, float4(world_dir, 0.0f)).xyz;

    // Sample the texture.
    float2 env_uv = get_latlong_from_dir(world_dir);
    float4 src_color = g_skybox.SampleLevel(g_sampler, env_uv, 0.0f);
    g_lighting_tex[dispatch_thread_id.xy] = float4(src_color.rgb, 1.0f);
}