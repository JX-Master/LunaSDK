/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Transform.hpp
* @author JXMaster
* @date 2025/11/9
*/
#pragma once
#include <Luna/Runtime/TypeInfo.hpp>
#include <Luna/Runtime/Math/Vector.hpp>
#include <Luna/Runtime/Math/Quaternion.hpp>
#include <Luna/Runtime/Math/Transform.hpp>

namespace Luna
{
    struct Transform
    {
        lustruct("Transform", "aff5aa04-bfb0-48a0-8ae9-f9f37d3052b8");

        Float3 position = Float3::zero();
        Quaternion rotation = Quaternion::identity();
        Float3 scale = Float3::one();

        Float4x4 get_this_to_parent_matrix() const
        {
            return AffineMatrix::make(position, rotation, scale);
        }
        Float4x4 get_parent_to_this_matrix() const
        {
            return inverse(get_this_to_parent_matrix());
        }
        void set_this_to_parent_matrix(const Float4x4& mat)
        {
            position = AffineMatrix::translation(mat);
            scale = AffineMatrix::scaling(mat);
            Float3 eular_angles = AffineMatrix::euler_angles(AffineMatrix::rotation_matrix(mat));
            rotation = Quaternion::from_euler_angles(Float3(eular_angles.x, eular_angles.y, eular_angles.z));
        }
        void set_parent_to_this_matrix(const Float4x4& mat)
        {
            set_this_to_parent_matrix(inverse(mat));
        }
    };
}