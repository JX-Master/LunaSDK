cbuffer CB : register(b0)
{
    uint dst_tex_width;
    uint dst_tex_height;
    float lum_threshold;
}
Texture2D<float4> g_src_tex : register(t1);
RWTexture2D<float4> g_dst_tex : register(u2);
SamplerState g_sampler : register(s3);

// float3 gauss_karis_n(float2 uv, int n, float2 stride, float sigma)
// {
//     float3 color = 0;
//     int r = n / 2;
//     float weight = 0;
//     for(int i = -r; i <= r; ++i)
//     {
//         for(int j = -r; j <= r; ++j)
//         {
//             float2 coord = uv + float2(i, j) * stride;
//             float3 c = g_src_tex.SampleLevel(g_sampler, coord, 0.0f).rgb;
//             float lum = dot(float3(0.2126, 0.7152, 0.0722), c);
//             c = lum > lum_threshold ? c : float3(0, 0, 0);
//             float w1 = gauss_weight_2d(i, j, sigma);
//             float w2 = 1.0 / (1.0 + lum);
//             float w = w1 * w2;
//             color += c * w;
//             weight += w;
//         }
//     }
//     color /= weight;
//     return color;
// }

float4 setup_sample(float2 uv)
{
    float3 color = g_src_tex.SampleLevel(g_sampler, uv, 0.0f).rgb;
    float lum = dot(float3(0.2126, 0.7152, 0.0722), color);
    if(lum < lum_threshold)
    {
        return float4(0, 0, 0, 1);
    }
    color *= (lum - lum_threshold) / lum;
    lum = dot(float3(0.2126, 0.7152, 0.0722), color);
    float w = 1.0 / (1.0 + lum);
    return float4(color, 1) * w;
}

[numthreads(8, 8, 1)]
void main(uint3 dispatch_thread_id : SV_DispatchThreadID)
{
    float4 color = float4(0, 0, 0, 0);
    float2 texel_size = float2(1 / (float)dst_tex_width, 1 / (float)dst_tex_height);
    float2 uv = texel_size * ((float2)dispatch_thread_id.xy + 0.5f);
    float2 stride = texel_size / 2;
    color += setup_sample(uv + stride) * 0.125f;
    color += setup_sample(uv - stride) * 0.125f;
    color += setup_sample(uv + float2(stride.x, -stride.y)) * 0.125f;
    color += setup_sample(uv + float2(-stride.x, stride.y)) * 0.125f;
    color += setup_sample(uv) * 0.5f;
    color /= color.a;
    g_dst_tex[dispatch_thread_id.xy] = color;
}