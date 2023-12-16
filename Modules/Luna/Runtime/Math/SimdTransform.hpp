/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file SimdTransform.hpp
* @author JXMaster
* @date 2022/3/27
* @brief The SIMD extension to support transform computations.
 */
#pragma once
#include "Simd.hpp"

#ifdef LUNA_SIMD
namespace Luna
{
	namespace Simd
	{
		//! Builds a 2D affine transformation matrix.
		//! ```
		//! ret[0].x := scaling.x * COS(rotation)
		//! ret[0].y := scaling.x * SIN(rotation)
		//! ret[0].z := 0
		//! ret[0].w := 0
		//! ret[1].x := scaling.y * -SIN(rotation)
		//! ret[1].y := scaling.y * COS(rotation)
		//! ret[1].z := 0
		//! ret[1].w := 0
		//! ret[2].x := translation.x
		//! ret[2].y := translation.y
		//! ret[2].z := 1
		//! ret[2].w := 0
		//! ```
		float3x4 LUNA_SIMD_CALL transform2d_f3x4(float4 translation, f32 rotation, float4 scaling);

		//! ```
		//! ret[0].x := 1
		//! ret[0].y := 0
		//! ret[0].z := 0
		//! ret[0].w := 0
		//! ret[1].x := 0
		//! ret[1].y := 1
		//! ret[1].z := 0
		//! ret[1].w := 0
		//! ret[2].x := translation.x
		//! ret[2].y := translation.y
		//! ret[2].z := 1
		//! ret[2].w := 0
		//! ```
		float3x4 LUNA_SIMD_CALL transform2d_translation_f3x4(float4 translation);

		//! ```
		//! ret[0].x := COS(rotation)
		//! ret[0].y := SIN(rotation)
		//! ret[0].z := 0
		//! ret[0].w := 0
		//! ret[1].x := -SIN(rotation)
		//! ret[1].y := COS(rotation)
		//! ret[1].z := 0
		//! ret[1].w := 0
		//! ret[2].x := 0
		//! ret[2].y := 0
		//! ret[2].z := 1
		//! ret[2].w := 0
		//! ```
		float3x4 LUNA_SIMD_CALL transform2d_rotation_f3x4(f32 rotation);

		//! ```
		//! ret[0].x := scaling.x
		//! ret[0].y := 0
		//! ret[0].z := 0
		//! ret[0].w := 0
		//! ret[1].x := 0
		//! ret[1].y := scaling.y
		//! ret[1].z := 0
		//! ret[1].w := 0
		//! ret[2].x := 0
		//! ret[2].y := 0
		//! ret[2].z := 1
		//! ret[2].w := 0
		//! ```
		float3x4 LUNA_SIMD_CALL transform2d_scaling_f3x4(float4 scaling);

		float4x4 LUNA_SIMD_CALL transform3d_f4x4(float4 translation, float4 rotation_quaternion, float4 scaling);
		
		//! ```
		//! ret[0].x := 1
		//! ret[0].y := 0
		//! ret[0].z := 0
		//! ret[0].w := 0
		//! ret[1].x := 0
		//! ret[1].y := 1
		//! ret[1].z := 0
		//! ret[1].w := 0
		//! ret[2].x := 0
		//! ret[2].y := 0
		//! ret[2].z := 1
		//! ret[2].w := 0
		//! ret[3].x := translation.x
		//! ret[3].y := translation.y
		//! ret[3].z := translation.z
		//! ret[3].w := 1
		//! ```
		float4x4 LUNA_SIMD_CALL transform3d_translation_f4x4(float4 translation);
		//! ```
		//! QX := quaternion.x
		//! QY := quaternion.y
		//! QZ := quaternion.z
		//! QW := quaternion.w
		//! ret[0].x := 1 - 2 * (QY * QY + QZ * QZ)
		//! ret[0].y :=		2 * (QX * QY + QZ * QW)
		//! ret[0].z :=		2 * (QX * QZ - QY * QW)
		//! ret[0].w := 0
		//! ret[1].x :=		2 * (QX * QY - QZ * QW)
		//! ret[1].y :=	1 - 2 * (QX * QX + QZ * QZ)
		//! ret[1].z :=		2 * (QY * QZ + QX * QW)
		//! ret[1].w := 0
		//! ret[2].x :=		2 * (QX * QZ + QY * QW)
		//! ret[2].y :=		2 * (QY * QZ - QX * QW)
		//! ret[2].z := 1 - 2 * (QX * QX + QY * QY)
		//! ret[2].w := 0
		//! ret[3].x := 0
		//! ret[3].y := 0
		//! ret[3].z := 0
		//! ret[3].w := 1
		//! ```
		float4x4 LUNA_SIMD_CALL transform3d_rotation_quaternion_f4x4(float4 quaternion);
		//! ```
		//! S := SIN(rotation)
		//! C := COS(rotation)
		//! ret[0].x := 1
		//! ret[0].y := 0
		//! ret[0].z := 0
		//! ret[0].w := 0
		//! ret[1].x := 0
		//! ret[1].y := C
		//! ret[1].z := S
		//! ret[1].w := 0
		//! ret[2].x := 0
		//! ret[2].y := -S
		//! ret[2].z := C
		//! ret[2].w := 0
		//! ret[3].x := 0
		//! ret[3].y := 0
		//! ret[3].z := 0
		//! ret[3].w := 1
		//! ```
		float4x4 LUNA_SIMD_CALL transform3d_rotation_x_f4x4(f32 rotation);
		//! ```
		//! S := SIN(rotation)
		//! C := COS(rotation)
		//! ret[0].x := C
		//! ret[0].y := 0
		//! ret[0].z := -S
		//! ret[0].w := 0
		//! ret[1].x := 0
		//! ret[1].y := 1
		//! ret[1].z := 0
		//! ret[1].w := 0
		//! ret[2].x := S
		//! ret[2].y := 0
		//! ret[2].z := C
		//! ret[2].w := 0
		//! ret[3].x := 0
		//! ret[3].y := 0
		//! ret[3].z := 0
		//! ret[3].w := 1
		//! ```
		float4x4 LUNA_SIMD_CALL transform3d_rotation_y_f4x4(f32 rotation);
		//! ```
		//! S := SIN(rotation)
		//! C := COS(rotation)
		//! ret[0].x := C
		//! ret[0].y := S
		//! ret[0].z := 0
		//! ret[0].w := 0
		//! ret[1].x := -S
		//! ret[1].y := C
		//! ret[1].z := 0
		//! ret[1].w := 0
		//! ret[2].x := 0
		//! ret[2].y := 0
		//! ret[2].z := 1
		//! ret[2].w := 0
		//! ret[3].x := 0
		//! ret[3].y := 0
		//! ret[3].z := 0
		//! ret[3].w := 1
		//! ```
		float4x4 LUNA_SIMD_CALL transform3d_rotation_z_f4x4(f32 rotation);
		float4x4 LUNA_SIMD_CALL transform3d_rotation_normal_angle_f4x4(float4 normal, f32 angle);
		float4x4 LUNA_SIMD_CALL transform3d_rotation_euler_angles_f4x4(float4 pitch_yaw_roll);
		//! ```
		//! ret[0].x := scaling.x
		//! ret[0].y := 0
		//! ret[0].z := 0
		//! ret[0].w := 0
		//! ret[1].x := 0
		//! ret[1].y := scaling.y
		//! ret[1].z := 0
		//! ret[1].w := 0
		//! ret[2].x := 0
		//! ret[2].y := 0
		//! ret[2].z := scaling.z
		//! ret[2].w := 0
		//! ret[3].x := 0
		//! ret[3].y := 0
		//! ret[3].z := 0
		//! ret[3].w := 1
		//! ```
		float4x4 LUNA_SIMD_CALL transform3d_scaling_f4x4(float4 scaling);
		//! Creates one affine matrix that trnasforms points and directions in world space to view space.
		//! `eyedir` and `updir` must be normalized.
		//! ```
		//! RZ := eyedir
		//! RX := CROSS3(updir, RZ)
		//! RY := CROSS3(RZ, RX)
		//! TX := DOT3(RX, -eye)
		//! TY := DOT3(RY, -eye)
		//! TZ := DOT3(RZ, -eye)
		//! dst[0] := VEC4(RX.x, RY.x, RZ.x, 0)
		//! dst[1] := VEC4(RX.y, RY.y, RZ.y, 0)
		//! dst[2] := VEC4(RX.z, RY.z, RZ.z, 0)
		//! dst[3] := VEC4(TX, TY, TZ, 1)
		//! ```
		float4x4 LUNA_SIMD_CALL transform3d_look_to_f4x4(float4 eye, float4 eyedir, float4 updir);
	}
}
#include "Impl/SimdTransform.inl"
#endif