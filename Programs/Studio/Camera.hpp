/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Camera.hpp
* @author JXMaster
* @date 2020/5/14
*/
#pragma once
#include <Luna/Runtime/TypeInfo.hpp>
#include <Luna/Runtime/Math/Transform.hpp>
namespace Luna
{
    enum class CameraType : u32
    {
        perspective = 0,
        orthographic = 1,
    };
    struct Camera
    {
    public:
        lustruct("Camera", "{7c388740-d97a-4e6c-9b7f-dc04e704629f}");

        CameraType type = CameraType::perspective;
        f32 fov = PI / 3.0f;
        f32 size = 5.0f;
        f32 near_clipping_plane = 0.3f;
        f32 far_clipping_plane = 1000.0f;
        f32 aspect_ratio = 16.0f / 9.0f;

        Float4x4 get_projection_matrix() const
        {
            if (type == CameraType::perspective)
            {
                return ProjectionMatrix::make_perspective_fov(fov, aspect_ratio, near_clipping_plane, far_clipping_plane);
            }
            else
            {
                return ProjectionMatrix::make_orthographic(size, size / aspect_ratio, near_clipping_plane, far_clipping_plane);
            }
        }
    };
    luenum(CameraType, "CameraType", "{920C8F7F-7CEC-4776-BF01-1F63A4C51D9F}");
}