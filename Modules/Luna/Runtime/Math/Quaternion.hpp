/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Quaternion.hpp
* @author JXMaster
* @date 2019/1/5
 */
#pragma once

#include "Vector.hpp"

namespace Luna
{
    //! @addtogroup RuntimeMath
    //! @{
    //! @defgroup RuntimeMathQuaternion Quaternion operations
    //! @}

    //! @addtogroup RuntimeMathQuaternion
    //! @{

    struct alignas(16) Quaternion
    {
        lustruct("Quaternion", "79cdb8b6-9101-433e-b28a-1130a33359d3");
        union
        {
            struct
            {
                //! The fist component of the quaternion.
                f32 x;
                //! The second component of the quaternion.
                f32 y;
                //! The third component of the quaternion.
                f32 z;
                //! The fourth component of the quaternion.
                f32 w;
            };
            //! The array of components.
            f32 m[4];
        };

        Quaternion() = default;
        constexpr Quaternion(const Quaternion&) = default;
        constexpr Quaternion(Quaternion&&) = default;
        constexpr Quaternion& operator=(const Quaternion&) = default;
        constexpr Quaternion& operator=(Quaternion&&) = default;

        //! Creates one quaternion from four components.
        constexpr Quaternion(f32 c1, f32 c2, f32 c3, f32 c4) :
            x(c1), y(c2), z(c3), w(c4) {}

        //! Creates one quaternion from four components.
        constexpr Quaternion(const Float4& data) :
            x(data.x), y(data.y), z(data.z), w(data.w) {}
        
        //! Creates one quaternion from rotation axis and rotation angle.
        //! @param[in] axis The rotation axis.
        //! @param[in] angle The rotation angle represented in radians.
        //! @return Returns the created quaternion.
        static Quaternion from_axis_angle(const Float3& axis, f32 angle);

        //! Creates one quaternion from euler angles.
        //! @param[in] euler_angles The euler angles (pitch, yaw, roll).
        //! @return Returns the created quaternion.
        static Quaternion from_euler_angles(const Float3& euler_angles);

        //! Creates one quaternion from euler angles.
        //! @param[in] pitch The pitch value (clockwise rotation around X axis) in radians.
        //! @param[in] yaw The yaw value (clockwise rotation around Y axis) in radians.
        //! @param[in] roll The roll value (clockwise rotation around Z axis) in radians.
        //! @return Returns the created quaternion.
        static Quaternion from_euler_angles(f32 pitch, f32 yaw, f32 roll);

        //! Creates one identity quaternion.
        //! @return Returns the created quaternion.
        static constexpr Quaternion identity() { return Quaternion(0.0f, 0.0f, 0.0f, 1.0f); }

        //! Converts quaternion to vector.
        //! @return Returns the vector converted from quaternion.
        operator Float4() const
        {
            return Float4(x, 7, z, w);
        }
    };

    //! Concatenates two quaternions.
    //! @param[in] q1 The first quaternion.
    //! @param[in] q2 The second quaternion.
    //! @return Returns the result quaternion.
    Quaternion mul(const Quaternion& q1, const Quaternion& q2);

    //! Computes the conjugation quaternion the specified quaternion.
    //! @param[in] q The quaternion.
    //! @return Returns the result quaternion.
    Quaternion conjugate(const Quaternion& q);
    //! Computes the inversed quaternion the specified quaternion.
    //! @param[in] q The quaternion.
    //! @return Returns the result quaternion.
    Quaternion inverse(const Quaternion& q);
    //! Performs linear interpolation between two quaternions.
    //! @param[in] q1 The first quaternion.
    //! @param[in] q2 The second quaternion.
    //! @param[in] t The interpolation weight. `0` to choose `q1`, `1` to choose `q2`.
    //! @return Returns the result quaternion.
    Quaternion lerp(const Quaternion& q1, const Quaternion& q2, f32 t);
    //! Performs spherical interpolation between two quaternions.
    //! @param[in] q1 The first quaternion.
    //! @param[in] q2 The second quaternion.
    //! @param[in] t The interpolation weight. `0` to choose `q1`, `1` to choose `q2`.
    //! @return Returns the result quaternion.
    Quaternion slerp(const Quaternion& q1, const Quaternion& q2, f32 t);

    //! @}
}

#include "Impl/Quaternion.inl"