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

float3 filter(Texture2D<float4> tex, float2 uv, float2 stride)
{
    float3 color = 0;
    color += tex.SampleLevel(g_sampler, uv, 0).rgb * 4;
    color += tex.SampleLevel(g_sampler, uv + float2(stride.x, 0), 0).rgb * 2;
    color += tex.SampleLevel(g_sampler, uv - float2(stride.x, 0), 0).rgb * 2;
    color += tex.SampleLevel(g_sampler, uv + float2(0, stride.y), 0).rgb * 2;
    color += tex.SampleLevel(g_sampler, uv - float2(0, stride.y), 0).rgb * 2;
    color += tex.SampleLevel(g_sampler, uv + stride, 0).rgb;
    color += tex.SampleLevel(g_sampler, uv - stride, 0).rgb;
    color += tex.SampleLevel(g_sampler, uv + float2(stride.x, -stride.y), 0).rgb;
    color += tex.SampleLevel(g_sampler, uv + float2(-stride.x, stride.y), 0).rgb;
    return color / 16;
}

[numthreads(8, 8, 1)]
void main(uint3 dispatch_thread_id : SV_DispatchThreadID)
{
    float2 src_texel_size = float2(1 / (float)src_tex_width, 1 / (float)src_tex_height);
    float2 dst_texel_size = float2(1 / (float)dst_tex_width, 1 / (float)dst_tex_height);
    float2 uv = dst_texel_size * ((float2)dispatch_thread_id.xy + 0.5f);
    //float3 prev_mip = filter(g_upsample_tex, uv, src_texel_size * up_sample_radius);
    float3 prev_mip = g_upsample_tex.SampleLevel(g_sampler, uv, 0);
    float3 current_mip = filter(g_bloom_tex, uv, dst_texel_size * up_sample_radius);
    g_dst_tex[dispatch_thread_id.xy].rgb = prev_mip + current_mip;
}