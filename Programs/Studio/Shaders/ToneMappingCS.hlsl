
cbuffer g_cb : register(b0)
{
    float g_exposure;
    uint g_auto_exposure;
    uint g_dst_width;
    uint g_dst_height;
    float g_bloom_intensity;
}
Texture2D<float4> g_scene_tex : register(t1);
Texture2D<float> g_lum_tex : register(t2);
Texture2D<float> g_bloom_tex : register(t3);
RWTexture2D<float4> g_dst_tex : register(u4);
SamplerState g_sampler : register(s5);

// @see: https://knarkowicz.wordpress.com/2016/01/06/aces-filmic-tone-mapping-curve/
float3 aces_film(float3 x)
{
    float a = 2.51;
    float b = 0.03;
    float c = 2.43;
    float d = 0.59;
    float e = 0.14;
    return clamp((x * (a * x + b)) / (x * (c * x + d) + e), 0.0, 1.0);
}

float3 tonemap(float3 color, float exposure)
{
    color *= exposure;
    return aces_film(color);
}

float3 gamma_correction(float3 color, float gamma)
{
    return pow(abs(color), 1.0 / gamma);  
}

[numthreads(8, 8, 1)]
void main(int3 dispatch_thread_id : SV_DispatchThreadID)
{
    float3 hdr_color = g_scene_tex[dispatch_thread_id.xy].xyz;
    float2 texel_size = float2(1 / (float)g_dst_width, 1 / (float)g_dst_height);
    float2 uv = texel_size * ((float2)dispatch_thread_id.xy + 0.5f);
    float3 bloom_color = g_bloom_tex.SampleLevel(g_sampler, uv, 0.0f) * g_bloom_intensity;
    hdr_color += bloom_color;
    float exposure;
    if(g_auto_exposure > 0)
    {
        float average_luminance = g_lum_tex[int2(0, 0)];
        exposure = g_exposure / max(0.0001, average_luminance);
    }
    else
    {
        exposure = g_exposure;
    }
    float3 ldr_color = tonemap(hdr_color, exposure);
    float3 final = gamma_correction(ldr_color, 2.2);

    g_dst_tex[dispatch_thread_id.xy] = float4(saturate(final.xyz), 1.0);
}