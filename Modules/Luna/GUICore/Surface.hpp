/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file Surface.hpp
* @author JXMaster
* @date 2026/7/21
*/
#pragma once
#include <Luna/Runtime/Math/Matrix.hpp>
#include "Base.hpp"

namespace Luna
{
    namespace GUICore
    {
        //! Reports the intersection of a host pointer ray with the local Z=0 plane of a GUI surface.
        struct SurfaceRayHit
        {
            //! Hit position in GUI surface logical coordinates.
            Float2U position = Float2U(0.0f);
            //! Parametric distance along the original host ray.
            f32 ray_distance = 0.0f;
        };

        //! Intersects one world-space ray with a GUI surface plane.
        //! @param[in] ray_origin The ray origin in world coordinates.
        //! @param[in] ray_direction The ray direction in world coordinates. It does not need to be normalized.
        //! @param[in] world_to_surface Affine matrix that transforms world positions into GUI surface coordinates.
        //! @param[out] hit Receives the surface-local hit position and ray distance.
        //! @return Returns `true` if the forward ray intersects the surface plane. Returns `false` if the ray is
        //! parallel to the plane or the intersection is behind the ray origin.
        //! @remark The function intentionally does not reject positions outside the logical surface rectangle. A
        //! host should continue delivering such positions while GUICore owns pointer capture.
        LUNA_GUICORE_API bool ray_to_surface(const Float3U& ray_origin, const Float3U& ray_direction,
            const Float4x4U& world_to_surface, SurfaceRayHit& hit);
    }
}
