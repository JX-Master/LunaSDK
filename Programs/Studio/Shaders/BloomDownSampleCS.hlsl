cbuffer CB : register(b0)
{
    uint dst_tex_width;
    uint dst_tex_height;
}
Texture2D<float4> g_src_tex : register(t1);
RWTexture2D<float4> g_dst_tex : register(u2);
SamplerState g_sampler : register(s3);

[numthreads(8, 8, 1)]
void main(uint3 dispatch_thread_id : SV_DispatchThreadID)
{
    float4 color = float4(0, 0, 0, 1);
    float2 texel_size = float2(1 / (float)dst_tex_width, 1 / (float)dst_tex_height);
    float2 uv = texel_size * ((float2)dispatch_thread_id.xy + 0.5f);
    float2 stride = texel_size / 2;
    color.rgb += g_src_tex.SampleLevel(g_sampler, uv + stride, 0.0f).rgb * 0.125f;
    color.rgb += g_src_tex.SampleLevel(g_sampler, uv - stride, 0.0f).rgb * 0.125f;
    color.rgb += g_src_tex.SampleLevel(g_sampler, uv + float2(stride.x, -stride.y), 0.0f).rgb * 0.125f;
    color.rgb += g_src_tex.SampleLevel(g_sampler, uv + float2(-stride.x, stride.y), 0.0f).rgb * 0.125f;
    color.rgb += g_src_tex.SampleLevel(g_sampler, uv, 0.0f).rgb * 0.5;
    g_dst_tex[dispatch_thread_id.xy] = color;
}