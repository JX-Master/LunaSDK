cbuffer LumHistogramCollectCB : register(b0)
{
    float min_brightness;
    float max_brightness;
    float time_coeff;
    float num_pixels;
}
RWBuffer<uint> g_histogram : register(u1);
RWTexture2D<float> g_target : register(u2);

groupshared uint histogram_shared[256];

[numthreads(256, 1, 1)]
void main(int3 dispatch_thread_id : SV_DispatchThreadID, uint group_index : SV_GroupIndex)
{
    uint count_for_this_bin = g_histogram[group_index];
    histogram_shared[group_index] = count_for_this_bin * group_index;
    GroupMemoryBarrierWithGroupSync();
    g_histogram[group_index] = 0;

    for(uint cutoff = (256 >> 1); cutoff > 0; cutoff >>= 1)
    {
        if (uint(group_index) < cutoff) 
        {
            histogram_shared[group_index] += histogram_shared[group_index + cutoff];
        }
        GroupMemoryBarrierWithGroupSync();
    }
    
    // We only need to calculate this once, so only a single thread is needed.
    if (group_index == 0) 
    {
        // Here we take our weighted sum and divide it by the number of pixels
        // that had luminance greater than zero (since the index == 0, we can
        // use count_for_this_bin to find the number of black pixels)
        float weighted_log_average = (histogram_shared[0] / max(num_pixels - float(count_for_this_bin), 1.0)) - 1.0;

        // Map from our histogram space to actual luminance
        float min_log_lum = log2(min_brightness);
        float log_lum_range = log2(max_brightness) - min_log_lum;
        float weighted_avg_lum = exp2(((weighted_log_average / 254.0) * log_lum_range) + min_log_lum);

        // The new stored value will be interpolated using the last frames value
        // to prevent sudden shifts in the exposure.
        float lum_last_frame = g_target[uint2(0, 0)];
        float adapted_lum = lum_last_frame + (weighted_avg_lum - lum_last_frame) * time_coeff;
        g_target[uint2(0, 0)] = adapted_lum;
    }
}