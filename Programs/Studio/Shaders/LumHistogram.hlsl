
float color_to_lum(float3 p)
{
    return dot(p, float3(0.2125, 0.7154, 0.0721));
}

uint color_to_bin(float3 hdr_color, float min_brightness, float max_brightness)
{
    // Convert our RGB value to Luminance, see note for RGB_TO_LUM macro above
    float lum = color_to_lum(hdr_color);

    // Avoid taking the log of zero
    if (lum < min_brightness) 
    {
        return 0;
    }

    float min_log_lum = log2(min_brightness);
    float log_lum_range = log2(max_brightness) - min_log_lum;

    // Calculate the log_2 luminance and express it as a value in [0.0, 1.0]
    // where 0.0 represents the minimum luminance, and 1.0 represents the max.
    float log_lum = clamp((log2(lum) - min_log_lum) / log_lum_range, 0.0, 1.0);

    // Map [0, 1] to [1, 255]. The zeroth bin is handled by the epsilon check above.
    return uint(log_lum * 254.0 + 1.0);
}

cbuffer LumHistogramCB : register(b0)
{
	uint src_width;
    uint src_height;
    float min_brightness;
    float max_brightness;
}

Texture2D g_src_tex : register(t1);
RWStructuredBuffer<uint> g_dst_buffer : register(u2);

groupshared uint histogram_shared[256];

[numthreads(16, 16, 1)]
void main(int3 dispatch_thread_id : SV_DispatchThreadID, uint group_index : SV_GroupIndex)
{
    histogram_shared[group_index] = 0;
    GroupMemoryBarrierWithGroupSync();
    
    if((uint)dispatch_thread_id.x < src_width && (uint)dispatch_thread_id.y < src_height)
    {
        float3 hdr_color = g_src_tex[dispatch_thread_id.xy].xyz;
        uint bin_index = color_to_bin(hdr_color, min_brightness, max_brightness);
        InterlockedAdd(histogram_shared[bin_index], 1);
    }

    GroupMemoryBarrierWithGroupSync();
    InterlockedAdd(g_dst_buffer[group_index], histogram_shared[group_index]);
}