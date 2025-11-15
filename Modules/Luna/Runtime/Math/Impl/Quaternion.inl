/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Quaternion.inl
* @author JXMaster
* @date 2022/3/17
 */
#pragma once
#include "../Quaternion.hpp"
#include "../SimdQuaternion.hpp"
namespace Luna
{
    inline Quaternion mul(const Quaternion& Q1, const Quaternion& Q2)
    {
#ifdef LUNA_SIMD
        using namespace Simd;
        float4 q1 = load_f4(Q1.m);
        float4 q2 = load_f4(Q2.m);
        Quaternion R;
        store_f4(R.m, mulquat_f4(q1, q2));
        return R;
#else
        return Quaternion(
            Q2.w * Q1.x + Q2.x * Q1.w + Q2.y * Q1.z - Q2.z * Q1.y,
            Q2.w * Q1.y - Q2.x * Q1.z + Q2.y * Q1.w + Q2.z * Q1.x,
            Q2.w * Q1.z + Q2.x * Q1.y - Q2.y * Q1.x + Q2.z * Q1.w,
            Q2.w * Q1.w - Q2.x * Q1.x - Q2.y * Q1.y - Q2.z * Q1.z);
#endif
    }
    inline Quaternion Quaternion::from_axis_angle(const Float3& axis, f32 angle)
    {
#ifdef LUNA_SIMD
        using namespace Simd;
        float4 n = load_f4(axis.m);
        n = normalize3_f4(n);
        n = quatnormalangle_f4(n, angle);
        Quaternion result;
        store_f4(result.m, n);
        return result;
#else
        Float3 normal = normalize(axis);
        f32 sinV = sinf(0.5f * angle);
        f32 cosV = cosf(0.5f * angle);
        Quaternion result;
        result.x = normal.x * sinV;
        result.y = normal.y * sinV;
        result.z = normal.z * sinV;
        result.w = cosV;
        return result;
#endif
    }
    inline Quaternion Quaternion::from_euler_angles(const Float3& euler_angles)
    {
#ifdef LUNA_SIMD
        using namespace Simd;
        float4 angles = load_f4(euler_angles.m);
        float4 Q = quateulerangles_f4(angles);
        Quaternion result;
        store_f4(result.m, Q);
        return result;
#else
        f32 pitch = euler_angles.x * 0.5f;
        f32 yaw = euler_angles.y * 0.5f;
        f32 roll = euler_angles.z * 0.5f;
        f32 SP = sinf(pitch);
        f32 CP = cosf(pitch);
        f32 SY = sinf(yaw);
        f32 CY = cosf(yaw);
        f32 SR = sinf(roll);
        f32 CR = cosf(roll);
        Quaternion result;
        result.x = CR * SP * CY + SR * CP * SY;
        result.y = CR * CP * SY - SR * SP * CY;
        result.z = SR * CP * CY - CR * SP * SY;
        result.w = CR * CP * CY + SR * SP * SY;
        return result;
#endif
    }
    inline Quaternion Quaternion::from_euler_angles(f32 pitch, f32 yaw, f32 roll)
    {
        return Quaternion::from_euler_angles(Float3(pitch, yaw, roll));
    }
    inline Quaternion conjugate(const Quaternion& q)
    {
#ifdef LUNA_SIMD
        using namespace Simd;
        float4 vecq = load_f4(q.m);
        Quaternion result;
        store_f4(result.m, mul_f4(set_f4(-1.0f, -1.0f, -1.0f, 1.0f), vecq));
        return result;
#else
        Quaternion result;
        result.x = -q.x;
        result.y = -q.y;
        result.z = -q.z;
        result.w = q.w;
        return result;
#endif
    }
    inline Quaternion inverse(const Quaternion& q)
    {
#ifdef LUNA_SIMD
        using namespace Simd;
        float4 vecq = load_f4(q.m);
        Quaternion result;
        store_f4(result.m, quatinverse_f4(vecq));
        return result;
#else
        f32 l = length_squared(q);
        Quaternion conjugated = conjugate(q);
        Quaternion result;
        if (l <= 1.192092896e-7f)
        {
            result.x = 0.0f;
            result.y = 0.0f;
            result.z = 0.0f;
            result.w = 0.0f;
        }
        else
        {
            result.x = conjugated.x / l;
            result.y = conjugated.y / l;
            result.z = conjugated.z / l;
            result.w = conjugated.w / l;
        }
        return result;
#endif
    }
    inline Quaternion lerp(const Quaternion& q1, const Quaternion& q2, f32 t)
    {
#ifdef LUNA_SIMD
        using namespace Simd;
        float4 a = load_f4(q1.m);
        float4 b = load_f4(q2.m);
        float4 res = quatlerp_f4(a, b, t);
        Quaternion result;
        store_f4(result.m, res);
        return result;
#else
        Quaternion result;
        f32 cos_omega = q1.x * q2.x + q1.y * q2.y + q1.z * q2.z + q1.w * q2.w;
        if (cos_omega >= 0)
        {
            result.x = q1.x + t * (q2.x - q1.x);
            result.y = q1.y + t * (q2.y - q1.y);
            result.z = q1.z + t * (q2.z - q1.z);
            result.w = q1.w + t * (q2.w - q1.w);
        }
        else
        {
            result.x = q1.x * (1.0f - t) - q2.x * t;
            result.y = q1.y * (1.0f - t) - q2.y * t;
            result.z = q1.z * (1.0f - t) - q2.z * t;
            result.w = q1.w * (1.0f - t) - q2.w * t;
        }
        return result;
#endif
    }
    inline Quaternion slerp(const Quaternion& q1, const Quaternion& q2, f32 t)
    {
#ifdef LUNA_SIMD
        using namespace Simd;
        float4 a = load_f4(q1.m);
        float4 b = load_f4(q2.m);
        float4 res = quatslerp_f4(a, b, t);
        Quaternion r;
        store_f4(r.m, res);
        return r;
#else
        f32 cos_omega = dot(q1, q2);
        cos_omega = cos_omega < 0 ? -cos_omega : cos_omega;
        f32 sin_omega = sqrtf(1.0f - cos_omega * cos_omega);
        f32 omega = atan2f(sin_omega, cos_omega);
        // Result = Q0 * sin((1.0 - t) * omega) / sin(omega) + Q1 * sin(t * omega) / sin(omega)
        f32 wa = sinf((1.0 - t) * omega) / sin_omega;
        f32 wb = sinf(t * omega) / sin_omega;
        Quaternion result = q1 * wa + q2 * wb;
        return result;
#endif
    }
}