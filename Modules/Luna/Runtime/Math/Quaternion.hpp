/*!
* This file is a portion of Luna SDK.
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

    namespace Quaternion
    {
        //! @addtogroup RuntimeMathQuaternion
        //! @{
        
        //! Concatenates two quaternions.
        //! @param[in] q1 The first quaternion.
        //! @param[in] q2 The second quaternion.
        //! @return Returns the result quaternion.
        Float4 mul(const Float4& q1, const Float4& q2);
        //! Creates one quaternion from rotation axis and rotation angle.
        //! @param[in] axis The rotation axis.
        //! @param[in] angle The rotation angle represented in radians.
        //! @return Returns the created quaternion.
        Float4 from_axis_angle(const Float3& axis, f32 angle);
        //! Creates one quaternion from euler angles.
        //! @param[in] euler_angles The euler angles (pitch, yaw, roll).
        //! @return Returns the created quaternion.
        Float4 from_euler_angles(const Float3& euler_angles);
        //! Creates one quaternion from euler angles.
        //! @param[in] pitch The pitch value (clockwise rotation around X axis) in radians.
        //! @param[in] yaw The yaw value (clockwise rotation around Y axis) in radians.
        //! @param[in] roll The roll value (clockwise rotation around Z axis) in radians.
        //! @return Returns the created quaternion.
        Float4 from_euler_angles(f32 pitch, f32 yaw, f32 roll);
        //! Creates one identity quaternion.
        //! @return Returns the created quaternion.
        constexpr Float4 identity() { return Float4(0.0f, 0.0f, 0.0f, 1.0f); }
        //! Computes the conjugation quaternion the specified quaternion.
        //! @param[in] q The quaternion.
        //! @return Returns the result quaternion.
        Float4 conjugate(const Float4& q);
        //! Computes the inversed quaternion the specified quaternion.
        //! @param[in] q The quaternion.
        //! @return Returns the result quaternion.
        Float4 inverse(const Float4& q);
        //! Performs linear interpolation between two quaternions.
        //! @param[in] q1 The first quaternion.
        //! @param[in] q2 The second quaternion.
        //! @param[in] t The interpolation weight. `0` to choose `q1`, `1` to choose `q2`.
        //! @return Returns the result quaternion.
        Float4 lerp(const Float4& q1, const Float4& q2, f32 t);
        //! Performs spherical interpolation between two quaternions.
        //! @param[in] q1 The first quaternion.
        //! @param[in] q2 The second quaternion.
        //! @param[in] t The interpolation weight. `0` to choose `q1`, `1` to choose `q2`.
        //! @return Returns the result quaternion.
        Float4 slerp(const Float4& q1, const Float4& q2, f32 t);
        
        //! @}
    }
}

#include "Impl/Quaternion.inl"