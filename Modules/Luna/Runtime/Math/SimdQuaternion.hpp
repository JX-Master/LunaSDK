/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file SimdQuaternion.hpp
* @author JXMaster
* @date 2022/3/25
* @brief The SIMD extension to support Quaternion computations.
 */
#pragma once
#include "Simd.hpp"

#ifdef LUNA_SIMD
namespace Luna
{
	namespace Simd
	{
		//! Multiplies two quaternion `a` and `b`, and stores the result in `dst`.
		//! ```
		//! dst.x := (a.x * b.w) + (a.w * b.x) + (a.z * b.y) - (a.y * b.z)
		//! dst.y := (a.y * b.w) - (a.z * b.x) + (a.w * b.y) + (a.x * b.z)
		//! dst.z := (a.z * b.w) + (a.y * b.x) - (a.x * b.y) + (a.w * b.z)
		//! dst.w := (a.w * b.w) - (a.x * b.x) - (a.y * b.y) - (a.z * b.z)
		//! ```
		float4 LUNA_SIMD_CALL mulquat_f4(float4 a, float4 b);

		//! Inverts the quaternion `a`, and stores the result in `dst`.
		//! ```
		//! DOT := a.x * a.x + a.y * a.y + a.z * a.z + a.w * a.w;
		//! dst.x := -a.x / DOT;
		//! dst.y := -a.y / DOT;
		//! dst.z := -a.z / DOT;
		//! dst.w :=  a.w / DOT;
		//! ```
		float4 LUNA_SIMD_CALL quatinverse_f4(float4 a);

		//! Computes one rotation quaternion based on the given normal `a` and one angle `a`, and stores 
		//! the result quaternion in `dst`.
		//! ```
		//! SINE := SIN(a * 0.5)
		//! COSINE := COS(a * 0.5)
		//! dst.x := n.x * SINE
		//! dst.y := n.y * SINE
		//! dst.z := n.z * SINE
		//! dst.w := COSINE
		//! ```
		float4 LUNA_SIMD_CALL quatnormalangle_f4(float4 n, f32 a);

		//! Computes a rotation quaternion based on a vector containing the Euler angles (pitch, yaw, and roll),
		//! and stores the result in `dst`.
		//! ```
		//! PITCH := a.x
		//! YAW   := a.y
		//! ROLL  := a.z
		//! SP  := SIN(PITCH * 0.5)
		//! CP  := COS(PITCH * 0.5)
		//! SY  := SIN(YAW * 0.5)
		//! CY  := COS(YAW * 0.5)
		//! SR  := SIN(ROLL * 0.5)
		//! CR  := COS(ROLL * 0.5)
		//! dst.x := CR * SP * CY + SR * CP * SY
		//! dst.y := CR * CP * SY - SR * SP * CY
		//! dst.z := SR * CP * CY - CR * SP * SY
		//! dst.w := CR * CP * CY + SR * SP * SY
		//! ```
		float4 LUNA_SIMD_CALL quateulerangles_f4(float4 a);

		//! Interpolates between two unit quaternions `a` and `b` using linear interpolation, 
		//! and stores the result in `dst`.
		//! ```
		//! OMEGA := ACOS(DOT4(a, b))
		//! WA := SIN((1.0 - t) * OMEGA) / SIN(OMEGA)
		//! WB := SIN(t * OMEGA) / SIN(OMEGA)
		//! dst.x = a.x * WA + b.x * WB
		//! dst.y = a.y * WA + b.y * WB
		//! dst.z = a.z * WA + b.z * WB
		//! dst.w = a.w * WA + b.w * WB
		//! ```
		float4 LUNA_SIMD_CALL quatlerp_f4(float4 a, float4 b, f32 t);

		//! Interpolates between two unit quaternions `a` and `b` using spherical linear interpolation, 
		//! and stores the result in `dst`.
		//! ```
		//! OMEGA := ACOS(DOT4(a, b))
		//! WA := SIN((1.0 - t) * OMEGA) / SIN(OMEGA)
		//! WB := SIN(t * OMEGA) / SIN(OMEGA)
		//! dst.x = a.x * WA + b.x * WB
		//! dst.y = a.y * WA + b.y * WB
		//! dst.z = a.z * WA + b.z * WB
		//! dst.w = a.w * WA + b.w * WB
		//! ```
		float4 LUNA_SIMD_CALL quatslerp_f4(float4 a, float4 b, f32 t);
	}
}
#include "Impl/SimdQuaternion.inl"
#endif