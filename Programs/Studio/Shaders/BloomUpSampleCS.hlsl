cbuffer CB : register(b0)
{
    uint src_tex_width;
    uint src_tex_height;
    uint dst_tex_width;
    uint dst_tex_height;
    float up_sample_radius;
}
Texture2D<float4> g_upsample_tex : register(t1);
Texture2D<float4> g_bloom_tex : register(t2);
RWTexture2D<float4> g_dst_tex : register(u3);
SamplerState g_sampler : register(s4);

float3 filter(Texture2D<float4> tex, int2 pos, int2 tex_size)
{
    float3 color = 0;
    tex_size -= int2(1, 1);
    color += tex[pos].rgb * 4;
    color += tex[clamp(pos + int2(1, 0), int2(0, 0), tex_size)].rgb * 2;
    color += tex[clamp(pos + int2(-1, 0), int2(0, 0), tex_size)].rgb * 2;
    color += tex[clamp(pos + int2(0, 1), int2(0, 0), tex_size)].rgb * 2;
    color += tex[clamp(pos + int2(0, -1), int2(0, 0), tex_size)].rgb * 2;
    color += tex[clamp(pos + int2(1, 1), int2(0, 0), tex_size)].rgb;
    color += tex[clamp(pos + int2(1, -1), int2(0, 0), tex_size)].rgb;
    color += tex[clamp(pos + int2(-1, 1), int2(0, 0), tex_size)].rgb;
    color += tex[clamp(pos + int2(-1, -1), int2(0, 0), tex_size)].rgb;
    return color / 16;
}

[numthreads(8, 8, 1)]
void main(uint3 dispatch_thread_id : SV_DispatchThreadID)
{
    float2 src_texel_size = float2(1 / (float)src_tex_width, 1 / (float)src_tex_height);
    float2 dst_texel_size = float2(1 / (float)dst_tex_width, 1 / (float)dst_tex_height);
    float2 uv = dst_texel_size * ((float2)dispatch_thread_id.xy + 0.5f);
    //float3 prev_mip = filter(g_upsample_tex, uv, src_texel_size * up_sample_radius);
    //float3 prev_mip = g_upsample_tex.SampleLevel(g_sampler, uv, 0);
    float3 prev_mip = filter(g_upsample_tex, dispatch_thread_id.xy / 2, int2(src_tex_width, src_tex_height));
    float3 current_mip = filter(g_bloom_tex, dispatch_thread_id.xy, int2(dst_tex_width, dst_tex_height));
    g_dst_tex[dispatch_thread_id.xy].rgb = prev_mip + current_mip;
}