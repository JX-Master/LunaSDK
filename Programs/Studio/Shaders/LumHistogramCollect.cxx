#include <cppsl/core.hxx>
#include <cppsl/math.hxx>
#include <cppsl/compute.hxx>
#include <cppsl/texture.hxx>

using namespace cppsl;

struct LumHistogramCollectCB
{
    float min_brightness;
    float max_brightness;
    float time_coeff;
    float num_pixels;
};

struct DescSet0
{
    [[cppsl::cbuffer, cppsl::binding(0)]]
    LumHistogramCollectCB cb;

    [[cppsl::rwstructured_buffer, cppsl::binding(1)]]
    uint* g_histogram;

    [[cppsl::binding(2)]]
    RWTexture2D<float> g_target;
};

[[cppsl::desc_set(0)]]
DescSet0 g_set0;

[[cppsl::group_shared]]
uint histogram_shared[256];

[[cppsl::compute(256, 1, 1)]]
void cs_main([[cppsl::builtin(group_index)]] uint group_index)
{
    uint count_for_this_bin = group_index > 0u ? g_set0.g_histogram[group_index] : 0u;
    histogram_shared[group_index] = count_for_this_bin * group_index;
    GroupMemoryBarrierWithGroupSync();

    for (uint cutoff = 128u; cutoff > 0u; cutoff >>= 1u)
    {
        if (group_index < cutoff)
        {
            histogram_shared[group_index] += histogram_shared[group_index + cutoff];
        }
        GroupMemoryBarrierWithGroupSync();
    }

    if (group_index == 0u)
    {
        float weighted_log_average = (float(histogram_shared[0]) / max(g_set0.cb.num_pixels - float(count_for_this_bin), 1.0f)) - 1.0f;
        float min_log_lum = log2(g_set0.cb.min_brightness);
        float log_lum_range = log2(g_set0.cb.max_brightness) - min_log_lum;
        float weighted_avg_lum = exp2(((weighted_log_average / 254.0f) * log_lum_range) + min_log_lum);

        float lum_last_frame = g_set0.g_target.Load(uint2{0u, 0u});
        float adapted_lum = lum_last_frame == 0.0f ? weighted_avg_lum : (lum_last_frame + (weighted_avg_lum - lum_last_frame) * g_set0.cb.time_coeff);
        g_set0.g_target.Store(uint2{0u, 0u}, adapted_lum);
    }
}
