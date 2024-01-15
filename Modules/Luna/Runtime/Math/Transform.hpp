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
	// 2D Affine Matrix (Float3x3):
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

	// 3D Affine Matrix (Float4x4):
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

	namespace AffineMatrix
	{
		//! Constructs one 2D affine matrix.
		Float3x3 make(const Float2& translation, f32 rotation, const Float2& scaling);
		//! Constructs one 3D affine matrix.
		Float4x4 make(const Float3& translation, const Quaternion& rotation, const Float3& scaling);
		//! Returns the up direction of one 2D affine matrix.
		//! 
		//! The returned vector is scaled by the Y factor of the scale component of the affine 
		//! matrix.
		Float2 up(const Float3x3& affine_matrix);
		//! Returns the up direction of one 3D affine matrix.
		//! 
		//! The returned vector is scaled by the Y factor of the scale component of the affine 
		//! matrix.
		Float3 up(const Float4x4& affine_matrix);
		//! Returns the down direction of one 2D affine matrix.
		//! 
		//! The returned vector is scaled by the Y factor of the scale component of the affine 
		//! matrix.
		Float2 down(const Float3x3& affine_matrix);
		//! Returns the down direction of one 3D affine matrix.
		//! 
		//! The returned vector is scaled by the Y factor of the scale component of the affine 
		//! matrix.
		Float3 down(const Float4x4& affine_matrix);
		//! Returns the left direction of one 2D affine matrix.
		//! 
		//! The returned vector is scaled by the X factor of the scale component of the affine 
		//! matrix.
		Float2 left(const Float3x3& affine_matrix);
		//! Returns the left direction of one 3D affine matrix.
		//! 
		//! The returned vector is scaled by the X factor of the scale component of the affine 
		//! matrix.
		Float3 left(const Float4x4& affine_matrix);
		//! Returns the right direction of one 2D affine matrix.
		//! 
		//! The returned vector is scaled by the X factor of the scale component of the affine 
		//! matrix.
		Float2 right(const Float3x3& affine_matrix);
		//! Returns the right direction.
		//! 
		//! The returned vector is scaled by the X factor of the scale component of the affine 
		//! matrix.
		Float3 right(const Float4x4& affine_matrix);
		//! Returns the forward direction of one 3D affine matrix.
		//! 
		//! The returned vector is scaled by the Z factor of the scale component of the affine 
		//! matrix.
		Float3 forward(const Float4x4& affine_matrix);
		//! Returns the backward direction of one 3D affine matrix.
		//! 
		//! The returned vector is scaled by the Z factor of the scale component of the affine 
		//! matrix.
		Float3 backward(const Float4x4& affine_matrix);
		//! Returns the translation component of one 2D affine matrix.
		Float2 translation(const Float3x3& affine_matrix);
		//! Returns the translation component of one 3D affine matrix.
		Float3 translation(const Float4x4& affine_matrix);
		//! Returns the rotation component of one 2D affine matrix.
		f32 rotation(const Float3x3& affine_matrix);
		//! Computes the quaternion from this rotation matrix. This method cannot be used for affine matrix directly,
		//! to use this method for affine matrix, call `rotation_matrix` to extract the rotation matrix from affine matrix
		//! first.
		Quaternion rotation(const Float4x4& affine_matrix);
		//! Computes the euler angles from this rotation matrix. 
		//! @details This method cannot be used for affine matrix directly,
		//! to use this method for affine matrix, call `rotation_matrix` to extract the rotation matrix from affine matrix
		//! first.
		//!
		//! The returned euler angles represents the radians of clockwise rotation along Z(roll), X(pitch), Y(yaw) axis in 
		//! that order.
		Float3 euler_angles(const Float4x4& affine_matrix);
		//! Returns the scaling component of one 2D affine matrix.
		Float2 scaling(const Float3x3& affine_matrix);
		//! Returns the scaling component of one 3D affine matrix.
		Float3 scaling(const Float4x4& affine_matrix);
		//! Extracts the translation matrix from one 2D affine matrix.
		Float3x3 translation_matrix(const Float3x3& affine_matrix);
		//! Extracts the translation matrix from one 3D affine matrix.
		Float4x4 translation_matrix(const Float4x4& affine_matrix);
		//! Extracts the rotation matrix from one 2D affine matrix.
		Float3x3 rotation_matrix(const Float3x3& affine_matrix);
		//! Extracts the rotation matrix from one 3D affine matrix.
		Float4x4 rotation_matrix(const Float4x4& affine_matrix);
		//! Extracts the scaling matrix from one 2D affine matrix.
		Float3x3 scaling_matrix(const Float3x3& affine_matrix);
		//! Extracts the scaling matrix from one 3D affine matrix.
		Float4x4 scaling_matrix(const Float4x4& affine_matrix);
		//! Constructs one 2D translation matrix from one translation vector.
		Float3x3 make_translation(const Float2& translation);
		//! Constructs one 2D translation matrix from x and y position.
		Float3x3 make_translation(f32 x, f32 y);
		//! Constructs one 3D translation matrix from one translation vector.
		Float4x4 make_translation(const Float3& translation);
		//! Constructs one 3D translation matrix from x, y and z position.
		Float4x4 make_translation(f32 x, f32 y, f32 z);
		//! Constructs one 2D rotation matrix from one rotation scalar.
		Float3x3 make_rotation(f32 rotation);
		//! Constructs one 3D rotation matrix from one rotation Quaternion.
		Float4x4 make_rotation(const Quaternion& rotation);
		//! Constructs one 3D rotation matrix that represents one rotation alone x axis.
		Float4x4 make_rotation_x(f32 angle);
		//! Constructs one 3D rotation matrix that represents one rotation alone y axis.
		Float4x4 make_rotation_y(f32 angle);
		//! Constructs one 3D rotation matrix that represents one rotation alone z axis.
		Float4x4 make_rotation_z(f32 angle);
		//! Constructs one 3D rotation matrix by specifying the rotation axis and rotation angle.
		Float4x4 make_rotation_axis_angle(const Float3& axis, f32 angle);
		//! Constructs one 3D rotation matrix from Euler angles (pitch, yaw, roll).
		Float4x4 make_rotation_euler_angles(const Float3& euler_angles);
		//! Constructs one 3D rotation matrix from Euler angles (pitch, yaw, roll).
		Float4x4 make_rotation_euler_angles(f32 pitch, f32 yaw, f32 roll);
		//! Constructs one 2D scaling matrix from one scaling vector.
		Float3x3 make_scaling(const Float2& scaling);
		//! Constructs one 2D scaling matrix from scaling factors in x and y directions.
		Float3x3 make_scaling(f32 scale_x, f32 scale_y);
		//! Constructs one 3D scaling matrix from one scaling vector.
		Float4x4 make_scaling(const Float3& scaling);
		//! Constructs one 3D scaling matrix from scaling factors in x, y and z directions.
		Float4x4 make_scaling(f32 scale_x, f32 scale_y, f32 scale_z);
		//! Constructs one view matrix that targets the specified position.
		Float4x4 make_look_at(const Float3& eye_pos, const Float3& target_pos, const Float3& up_dir);
		//! Constructs one view matrix that targets the specified direction.
		Float4x4 make_look_to(const Float3& eye_pos, const Float3& eye_dir, const Float3& up_dir);
	}

	namespace ProjectionMatrix
	{
		//! Constructs one perspective projection matrix using width, height, near clipping distance and far clipping distance.
		Float4x4 make_perspective(f32 near_width, f32 near_height, f32 near_z, f32 far_z);
		//! Constructs one perspective projection matrix using diagonal fov, aspect_ratio, near clipping distance and far clipping distance.
		Float4x4 make_perspective_fov(f32 fov, f32 aspect_ratio, f32 near_z, f32 far_z);
		//! Constructs one perspective projection matrix using width fov, aspect_ratio, near clipping distance and far clipping distance.
		Float4x4 make_perspective_fov_w(f32 fov_w, f32 aspect_ratio, f32 near_z, f32 far_z);
		//! Constructs one perspective projection matrix using height fov, aspect_ratio, near clipping distance and far clipping distance.
		Float4x4 make_perspective_fov_h(f32 fov_h, f32 aspect_ratio, f32 near_z, f32 far_z);
		//! Constructs one perspective projection matrix using four offset values from the camera center, near clipping distance and far clipping distance.
		Float4x4 make_perspective_off_center(f32 near_left, f32 near_right, f32 near_bottom, f32 near_top, f32 near_z, f32 far_z);
		//! Constructs one orthographic projection matrix using width, height near clipping distance, and far clipping distance.
		Float4x4 make_orthographic(f32 width, f32 height, f32 near_z, f32 far_z);
		//! Constructs one orthographic projection matrix using four offset values from the camera center, near clipping distance and far clipping distance.
		Float4x4 make_orthographic_off_center(f32 left, f32 right, f32 bottom, f32 top, f32 near_z, f32 far_z);
	}
}

#include "Impl/Transform.inl"