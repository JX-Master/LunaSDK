/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Transform.hpp
* @author JXMaster
* @date 2019/1/5
* @brief Contains functions to create affine matrices and to fetch information about affine matrices.
 */
#pragma once
#include "Quaternion.hpp"
#include "Matrix.hpp"

namespace Luna
{
	// 2D Affine Matrix (3x3):
	/*

	   +y(up)
	   /|\
		|
		|
		|
		|
		|
		|
		---------------->+x (right)

	 */

	namespace AffineMatrix2D
	{
		Float3x3 make(const Float2& translation, f32 rotation, const Float2& scaling);

		//! Returns the up direction.
		//! 
		//! The returned vector is scaled by the Y factor of the scale component of the affine 
		//! matrix.
		Float2 up(const Float3x3& affine_matrix);
		
		//! Returns the down direction.
		//! 
		//! The returned vector is scaled by the Y factor of the scale component of the affine 
		//! matrix.
		Float2 down(const Float3x3& affine_matrix);

		//! Returns the left direction.
		//! 
		//! The returned vector is scaled by the X factor of the scale component of the affine 
		//! matrix.
		Float2 left(const Float3x3& affine_matrix);

		//! Returns the right direction.
		//! 
		//! The returned vector is scaled by the X factor of the scale component of the affine 
		//! matrix.
		Float2 right(const Float3x3& affine_matrix);

		//! Returns the translation component.
		Float2 translation(const Float3x3& affine_matrix);

		//! Returns the rotation component.
		f32 rotation(const Float3x3& affine_matrix);

		//! Returns the scaling component.
		Float2 scaling(const Float3x3& affine_matrix);

		//! Extracts the translation matrix from the specified affine matrix.
		Float3x3 translation_matrix(const Float3x3& affine_matrix);

		//! Extracts the rotation matrix from the specified affine matrix.
		Float3x3 rotation_matrix(const Float3x3& affine_matrix);

		//! Extracts the scaling matrix from the specified affine matrix.
		Float3x3 scaling_matrix(const Float3x3& affine_matrix);

		Float3x3 make_translation(const Float2& translation);
		Float3x3 make_translation(f32 x, f32 y);
		Float3x3 make_rotation(f32 rotation);
		Float3x3 make_scaling(const Float2& scaling);
		Float3x3 make_scaling(f32 scale_x, f32 scale_y);
	}

	 // 3D Affine Matrix (4x4):
	 /*

				  +y(up)
				  /|\
				   |
	(backward)+z   |
			   \   |
				\  |
				 \ |
				  \|
				   ---------------->+x (right)

	 */

	namespace AffineMatrix3D
	{
		Float4x4 make(const Float3& translation, const Quaternion& rotation, const Float3& scaling);

		//! Returns the up direction.
		//! 
		//! The returned vector is scaled by the Y factor of the scale component of the affine 
		//! matrix.
		Float3 up(const Float4x4& affine_matrix);

		//! Returns the down direction.
		//! 
		//! The returned vector is scaled by the Y factor of the scale component of the affine 
		//! matrix.
		Float3 down(const Float4x4& affine_matrix);

		//! Returns the left direction.
		//! 
		//! The returned vector is scaled by the X factor of the scale component of the affine 
		//! matrix.
		Float3 left(const Float4x4& affine_matrix);

		//! Returns the right direction.
		//! 
		//! The returned vector is scaled by the X factor of the scale component of the affine 
		//! matrix.
		Float3 right(const Float4x4& affine_matrix);

		//! Returns the forward direction.
		//! 
		//! The returned vector is scaled by the Z factor of the scale component of the affine 
		//! matrix.
		Float3 forward(const Float4x4& affine_matrix);

		//! Returns the backward direction.
		//! 
		//! The returned vector is scaled by the Z factor of the scale component of the affine 
		//! matrix.
		Float3 backward(const Float4x4& affine_matrix);

		//! Returns the translation component.
		Float3 translation(const Float4x4& affine_matrix);

		//! Returns the rotation component.
		Quaternion rotation(const Float4x4& affine_matrix);

		//! Returns the rotation component in euler angles form (pitch, yaw, roll).
		Float3 euler_angles(const Float4x4& affine_matrix);

		//! Returns the scaling component.
		Float3 scaling(const Float4x4& affine_matrix);

		//! Extracts the translation matrix from the specified affine matrix.
		Float4x4 translation_matrix(const Float4x4& affine_matrix);

		//! Extracts the rotation matrix from the specified affine matrix.
		Float4x4 rotation_matrix(const Float4x4& affine_matrix);

		//! Extracts the scaling matrix from the specified affine matrix.
		Float4x4 scaling_matrix(const Float4x4& affine_matrix);

		Float4x4 make_translation(const Float3& translation);
		Float4x4 make_translation(f32 x, f32 y, f32 z);
		Float4x4 make_rotation(const Quaternion& rotation);
		Float4x4 make_rotation_x(f32 angle);
		Float4x4 make_rotation_y(f32 angle);
		Float4x4 make_rotation_z(f32 angle);
		Float4x4 make_rotation_axis_angle(const Float3& axis, f32 angle);
		Float4x4 make_rotation_euler_angles(const Float3& euler_angles);
		Float4x4 make_rotation_euler_angles(f32 pitch, f32 yaw, f32 roll);
		Float4x4 make_scaling(const Float3& scaling);
		Float4x4 make_scaling(f32 scale_x, f32 scale_y, f32 scale_z);

		Float4x4 make_look_at(const Float3& eye_pos, const Float3& target_pos, const Float3& up_dir);
		Float4x4 make_look_to(const Float3& eye_pos, const Float3& eye_dir, const Float3& up_dir);
	}

	Float4x4 perspective_projection(f32 width, f32 height, f32 near_z, f32 far_z);
	Float4x4 perspective_projection_fov(f32 fov, f32 aspect_ratio, f32 near_z, f32 far_z);
	Float4x4 perspective_projection_off_center(f32 left, f32 right, f32 bottom, f32 top, f32 near_z, f32 far_z);
	Float4x4 orthographic_projection(f32 width, f32 height, f32 near_z, f32 far_z);
	Float4x4 orthographic_projection_off_center(f32 left, f32 right, f32 bottom, f32 top, f32 near_z, f32 far_z);
}

#include "../Source/Math/Transform.inl"