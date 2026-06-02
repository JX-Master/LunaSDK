/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*/
#include "../../Nodes/Common.hpp"

namespace Luna
{
    namespace GUI
    {
        Float4U smooth_color(const Float4U& a, const Float4U& b, f32 t)
{
            t = clamp(t, 0.0f, 1.0f);
            t = t * t * (3.0f - 2.0f * t);
            return Float4U(
                a.x + (b.x - a.x) * t,
                a.y + (b.y - a.y) * t,
                a.z + (b.z - a.z) * t,
                a.w + (b.w - a.w) * t);
        }

        LayoutMetrics fixed_height_metrics(f32 min_width, f32 preferred_width, f32 height)
{
            LayoutMetrics metrics;
            metrics.min_size = Float2U(min_width, height);
            metrics.preferred_size = Float2U(preferred_width, height);
            metrics.max_size = Float2U(F32_MAX, height);
            return metrics;
        }

        LayoutMetrics numeric_edit_metrics(const Node& node, const NumericBinding& binding)
	{
            f32 text_width = (f32)node.text.size() * 16.0f * 0.52f;
            if(binding.color_owner_id)
            {
                return fixed_height_metrics(88.0f, max(text_width + 92.0f, 116.0f), 30.0f);
            }
            return fixed_height_metrics(180.0f, max(text_width + 220.0f, 280.0f), 30.0f);
        }

    }
}
