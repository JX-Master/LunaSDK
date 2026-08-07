/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file Surface.cpp
* @author JXMaster
* @date 2026/7/21
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_GUI_API LUNA_EXPORT
#include "../Surface.hpp"

namespace Luna
{
    namespace GUI
    {
        bool ray_to_surface(const Float3U& ray_origin, const Float3U& ray_direction,
            const Float4x4U& world_to_surface, SurfaceRayHit& hit)
        {
            Float4x4 transform = world_to_surface.to_float4x4();
            Float4 local_origin = mul(Float4(ray_origin.x, ray_origin.y, ray_origin.z, 1.0f),
                transform);
            Float4 local_direction = mul(Float4(ray_direction.x, ray_direction.y, ray_direction.z, 0.0f),
                transform);
            if(abs(local_direction.z) <= 0.000001f) return false;
            f32 distance = -local_origin.z / local_direction.z;
            if(distance < 0.0f) return false;
            hit.position = Float2U(local_origin.x + local_direction.x * distance,
                local_origin.y + local_direction.y * distance);
            hit.ray_distance = distance;
            return true;
        }
    }
}
