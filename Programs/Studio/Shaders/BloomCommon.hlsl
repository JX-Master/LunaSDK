float gauss_weight_2d(float x, float y, float sigma)
{
    float PI = 3.14159265358;
    float E = 2.71828182846;
    float sigma_2 = pow(sigma, 2);
    float a = -(x * x + y * y) / (2.0 * sigma_2);
    return pow(E, a) / (2.0 * PI * sigma_2);
}

float3 gauss_n(Texture2D<float4> tex, SamplerState sampler, float2 uv, int n, float2 stride, float sigma)
{
    float3 color = 0;
    int r = n / 2;
    float weight = 0;
    for(int i = -r; i <= r; ++i)
    {
        for(int j = -r; j <= r; ++j)
        {
            float2 coord = uv + float2(i, j) * stride;
            float w = gauss_weight_2d(i, j, sigma);
            color += tex.SampleLevel(sampler, coord, 0.0f).rgb * w;
            weight += w;
        }
    }
    color /= weight;
    return color;
}