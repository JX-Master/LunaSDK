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

    //! @addtogroup RuntimeMath
    //! @{
    //! @defgroup RuntimeMathTransform Transform operations
    //! @}

    namespace AffineMatrix
    {
        //! @addtogroup RuntimeMathTransform
        //! @{
        
        //! Constructs one 2D affine matrix.
        //! @details The transform matrix is composed in scaling-rotation-translation order.
        //! @param[in] translation The translation vector.
        //! @param[in] rotation The rotation represented in radians.
        //! @param[in] scaling The scaling vector.
        //! @return Returns the result matrix. The matrix can be applied to one vector using `mul(vec, mat)`.
        Float3x3 make(const Float2& translation, f32 rotation, const Float2& scaling);
        //! Constructs one 3D affine matrix.
        //! @details The transform matrix is composed in scaling-rotation-translation order.
        //! @param[in] translation The translation vector.
        //! @param[in] rotation The rotation quaternion.
        //! @param[in] scaling The scaling vector.
        //! @return Returns the result matrix. The matrix can be applied to one vector using `mul(vec, mat)`.
        Float4x4 make(const Float3& translation, const Float4& rotation, const Float3& scaling);
        //! Extracts the up direction from one 2D affine matrix.
        //! @details
        //! The returned vector is scaled by the Y factor of the scale component of the affine 
        //! matrix.
        Float2 up(const Float3x3& affine_matrix);
        //! Extracts the up direction from one 3D affine matrix.
        //! @details
        //! The returned vector is scaled by the Y factor of the scale component of the affine 
        //! matrix.
        //! @param[in] affine_matrix The affine matrix to extract the vector.
        //! @return Returns the extracted vector.
        Float3 up(const Float4x4& affine_matrix);
        //! Extracts the down direction from one 2D affine matrix.
        //! @details
        //! The returned vector is scaled by the Y factor of the scale component of the affine 
        //! matrix.
        //! @param[in] affine_matrix The affine matrix to extract the vector.
        //! @return Returns the extracted vector.
        Float2 down(const Float3x3& affine_matrix);
        //! Extracts the down direction from one 3D affine matrix.
        //! @details
        //! The returned vector is scaled by the Y factor of the scale component of the affine 
        //! matrix.
        //! @param[in] affine_matrix The affine matrix to extract the vector.
        //! @return Returns the extracted vector.
        Float3 down(const Float4x4& affine_matrix);
        //! Extracts the left direction from one 2D affine matrix.
        //! @details
        //! The returned vector is scaled by the X factor of the scale component of the affine 
        //! matrix.
        //! @param[in] affine_matrix The affine matrix to extract the vector.
        //! @return Returns the extracted vector.
        Float2 left(const Float3x3& affine_matrix);
        //! Extracts the left direction from one 3D affine matrix.
        //! @details
        //! The returned vector is scaled by the X factor of the scale component of the affine 
        //! matrix.
        //! @param[in] affine_matrix The affine matrix to extract the vector.
        //! @return Returns the extracted vector.
        Float3 left(const Float4x4& affine_matrix);
        //! Extracts the right direction from one 2D affine matrix.
        //! @details
        //! The returned vector is scaled by the X factor of the scale component of the affine 
        //! matrix.
        //! @param[in] affine_matrix The affine matrix to extract the vector.
        //! @return Returns the extracted vector.
        Float2 right(const Float3x3& affine_matrix);
        //! Extracts the right direction.
        //! @details
        //! The returned vector is scaled by the X factor of the scale component of the affine 
        //! matrix.
        //! @param[in] affine_matrix The affine matrix to extract the vector.
        //! @return Returns the extracted vector.
        Float3 right(const Float4x4& affine_matrix);
        //! Extracts the forward direction from one 3D affine matrix.
        //! @details
        //! The returned vector is scaled by the Z factor of the scale component of the affine 
        //! matrix.
        //! @param[in] affine_matrix The affine matrix to extract the vector.
        //! @return Returns the extracted vector.
        Float3 forward(const Float4x4& affine_matrix);
        //! Extracts the backward direction from one 3D affine matrix.
        //! @details
        //! The returned vector is scaled by the Z factor of the scale component of the affine 
        //! matrix.
        //! @param[in] affine_matrix The affine matrix to extract the vector.
        //! @return Returns the extracted vector.
        Float3 backward(const Float4x4& affine_matrix);
        //! Extracts the translation component from one 2D affine matrix.
        //! @param[in] affine_matrix The affine matrix to extract the vector.
        //! @return Returns the extracted vector.
        Float2 translation(const Float3x3& affine_matrix);
        //! Extracts the translation component from one 3D affine matrix.
        //! @param[in] affine_matrix The affine matrix to extract the vector.
        //! @return Returns the extracted vector.
        Float3 translation(const Float4x4& affine_matrix);
        //! Extracts the rotation component from one 2D affine matrix.
        //! @param[in] affine_matrix The affine matrix to extract the vector.
        //! @return Returns the extracted rotation value represented in radians.
        f32 rotation(const Float3x3& affine_matrix);
        //! Computes the quaternion from one rotation matrix.
        //! @details This method cannot be used for affine matrix directly, to use this method for 
        //! affine matrix, call @ref rotation_matrix to extract the rotation matrix from affine matrix
        //! first.
        //! @param[in] rotation_matrix The rotation matrix.
        //! @return Returns the extracted quaternion.
        Float4 rotation(const Float4x4& rotation_matrix);
        //! Computes the euler angles from one rotation matrix. 
        //! @details This method cannot be used for affine matrix directly, to use this method for 
        //! affine matrix, call @ref rotation_matrix to extract the rotation matrix from affine matrix
        //! first.
        //!
        //! The returned euler angles represents the radians of clockwise rotation along Z(roll), X(pitch), Y(yaw) axis in 
        //! that order.
        //! @param[in] rotation_matrix The rotation matrix.
        //! @return Returns the rotation represented by euler angles (X = pitch, Y = yaw, Z = roll).
        Float3 euler_angles(const Float4x4& affine_matrix);
        //! Extracts the scaling component from one 2D affine matrix.
        //! @param[in] affine_matrix The affine matrix to extract the component.
        //! @return Returns the extracted component.
        Float2 scaling(const Float3x3& affine_matrix);
        //! Extracts the scaling component from one 3D affine matrix.
        //! @param[in] affine_matrix The affine matrix to extract the component.
        //! @return Returns the extracted component.
        Float3 scaling(const Float4x4& affine_matrix);
        //! Extracts the translation matrix from one 2D affine matrix.
        //! @param[in] affine_matrix The affine matrix to extract the matrix.
        //! @return Returns the extracted matrix.
        Float3x3 translation_matrix(const Float3x3& affine_matrix);
        //! Extracts the translation matrix from one 3D affine matrix.
        //! @param[in] affine_matrix The affine matrix to extract the matrix.
        //! @return Returns the extracted matrix.
        Float4x4 translation_matrix(const Float4x4& affine_matrix);
        //! Extracts the rotation matrix from one 2D affine matrix.
        //! @param[in] affine_matrix The affine matrix to extract the matrix.
        //! @return Returns the extracted matrix.
        Float3x3 rotation_matrix(const Float3x3& affine_matrix);
        //! Extracts the rotation matrix from one 3D affine matrix.
        //! @param[in] affine_matrix The affine matrix to extract the matrix.
        //! @return Returns the extracted matrix.
        Float4x4 rotation_matrix(const Float4x4& affine_matrix);
        //! Extracts the scaling matrix from one 2D affine matrix.
        //! @param[in] affine_matrix The affine matrix to extract the matrix.
        //! @return Returns the extracted matrix.
        Float3x3 scaling_matrix(const Float3x3& affine_matrix);
        //! Extracts the scaling matrix from one 3D affine matrix.
        //! @param[in] affine_matrix The affine matrix to extract the matrix.
        //! @return Returns the extracted matrix.
        Float4x4 scaling_matrix(const Float4x4& affine_matrix);
        //! Constructs one 2D translation matrix from one translation vector.
        //! @param[in] translation The translation vector.
        //! @return Returns the constructed matrix.
        Float3x3 make_translation(const Float2& translation);
        //! Constructs one 2D translation matrix from x and y position.
        //! @param[in] x The translation in X axis.
        //! @param[in] y The translation in Y axis.
        //! @return Returns the constructed matrix.
        Float3x3 make_translation(f32 x, f32 y);
        //! Constructs one 3D translation matrix from one translation vector.
        //! @param[in] translation The translation vector.
        //! @return Returns the constructed matrix.
        Float4x4 make_translation(const Float3& translation);
        //! Constructs one 3D translation matrix from x, y and z position.
        //! @param[in] x The translation in X axis.
        //! @param[in] y The translation in Y axis.
        //! @param[in] z The translation in Z axis.
        //! @return Returns the constructed matrix.
        Float4x4 make_translation(f32 x, f32 y, f32 z);
        //! Constructs one 2D rotation matrix from one rotation scalar.
        //! @param[in] rotation The counter-clockwise rotation represected in radians.
        //! @return Returns the constructed matrix.
        Float3x3 make_rotation(f32 rotation);
        //! Constructs one 3D rotation matrix from one rotation Quaternion.
        //! @param[in] rotation The rotation quaternion.
        //! @return Returns the constructed matrix.
        Float4x4 make_rotation(const Float4& rotation);
        //! Constructs one 3D rotation matrix that represents one rotation alone x axis.
        //! @param[in] rotation The rotation represected in radians.
        //! @return Returns the constructed matrix.
        Float4x4 make_rotation_x(f32 angle);
        //! Constructs one 3D rotation matrix that represents one rotation alone y axis.
        //! @param[in] rotation The rotation represected in radians.
        //! @return Returns the constructed matrix.
        Float4x4 make_rotation_y(f32 angle);
        //! Constructs one 3D rotation matrix that represents one rotation alone z axis.
        //! @param[in] rotation The rotation represected in radians.
        //! @return Returns the constructed matrix.
        Float4x4 make_rotation_z(f32 angle);
        //! Constructs one 3D rotation matrix by specifying the rotation axis and rotation angle.
        //! @param[in] axis The axis to rotate around.
        //! @param[in] rotation The rotation represected in radians.
        //! @return Returns the constructed matrix.
        Float4x4 make_rotation_axis_angle(const Float3& axis, f32 angle);
        //! Constructs one 3D rotation matrix from Euler angles (pitch, yaw, roll).
        //! @param[in] euler_angles The euler angles represented in radians.
        //! @return Returns the constructed matrix.
        Float4x4 make_rotation_euler_angles(const Float3& euler_angles);
        //! Constructs one 3D rotation matrix from Euler angles (pitch, yaw, roll).
        //! @param[in] pitch The pitch value (clockwise rotation around X axis) in radians.
        //! @param[in] yaw The yaw value (clockwise rotation around Y axis) in radians.
        //! @param[in] roll The roll value (clockwise rotation around Z axis) in radians.
        //! @return Returns the constructed matrix.
        Float4x4 make_rotation_euler_angles(f32 pitch, f32 yaw, f32 roll);
        //! Constructs one 2D scaling matrix from one scaling vector.
        //! @param[in] scaling The scaling vector.
        //! @return Returns the constructed matrix.
        Float3x3 make_scaling(const Float2& scaling);
        //! Constructs one 2D scaling matrix from scaling factors in x and y directions.
        //! @param[in] scale_x The scaling along X axis.
        //! @param[in] scale_y The scaling along Y axis.
        //! @return Returns the constructed matrix.
        Float3x3 make_scaling(f32 scale_x, f32 scale_y);
        //! Constructs one 3D scaling matrix from one scaling vector.
        //! @return Returns the constructed matrix.
        Float4x4 make_scaling(const Float3& scaling);
        //! Constructs one 3D scaling matrix from scaling factors in x, y and z directions.
        //! @param[in] scale_x The scaling along X axis.
        //! @param[in] scale_y The scaling along Y axis.
        //! @param[in] scale_z The scaling along Z axis.
        //! @return Returns the constructed matrix.
        Float4x4 make_scaling(f32 scale_x, f32 scale_y, f32 scale_z);
        //! Constructs one view matrix that targets the specified position.
        //! @param[in] eye_pos The position of the view point (camera position).
        //! @param[in] target_pos The position of the point to look at.
        //! @param[in] up_dir The up direction.
        //! @return Returns the view matrix.
        Float4x4 make_look_at(const Float3& eye_pos, const Float3& target_pos, const Float3& up_dir);
        //! Constructs one view matrix that targets the specified direction.
        //! @param[in] eye_pos The position of the view point (camera position).
        //! @param[in] eye_dir The direction to look to.
        //! @param[in] up_dir The up direction.
        //! @return Returns the view matrix.
        Float4x4 make_look_to(const Float3& eye_pos, const Float3& eye_dir, const Float3& up_dir);
    }
    namespace ProjectionMatrix
    {
        //! Constructs one perspective projection matrix using width, height, near clipping distance and far clipping distance.
        //! @param[in] near_width The width of the near plane.
        //! @param[in] near_height The height of the near plane.
        //! @param[in] near_z The Z position of the near plane.
        //! @param[in] far_z The Z position of the far plane.
        //! @return Returns the projection matrix.
        Float4x4 make_perspective(f32 near_width, f32 near_height, f32 near_z, f32 far_z);
        //! Constructs one perspective projection matrix using diagonal fov, aspect_ratio, near clipping distance and far clipping distance.
        //! @param[in] fov The diagonal field of view of the camera in radians.
        //! @param[in] aspect_ratio The aspect ratio (width / height) of the viewport.
        //! @param[in] near_z The Z position of the near plane.
        //! @param[in] far_z The Z position of the far plane.
        //! @return Returns the projection matrix.
        Float4x4 make_perspective_fov(f32 fov, f32 aspect_ratio, f32 near_z, f32 far_z);
        //! Constructs one perspective projection matrix using width fov, aspect_ratio, near clipping distance and far clipping distance.
        //! @param[in] fov The width field of view of the camera in radians.
        //! @param[in] aspect_ratio The aspect ratio (width / height) of the viewport.
        //! @param[in] near_z The Z position of the near plane.
        //! @param[in] far_z The Z position of the far plane.
        //! @return Returns the projection matrix.
        Float4x4 make_perspective_fov_w(f32 fov_w, f32 aspect_ratio, f32 near_z, f32 far_z);
        //! Constructs one perspective projection matrix using height fov, aspect_ratio, near clipping distance and far clipping distance.
        //! @param[in] fov The height field of view of the camera in radians.
        //! @param[in] aspect_ratio The aspect ratio (width / height) of the viewport.
        //! @param[in] near_z The Z position of the near plane.
        //! @param[in] far_z The Z position of the far plane.
        //! @return Returns the projection matrix.
        Float4x4 make_perspective_fov_h(f32 fov_h, f32 aspect_ratio, f32 near_z, f32 far_z);
        //! Constructs one perspective projection matrix using four offset values from the camera center, near clipping distance and far clipping distance.
        //! @param[in] near_left The offset of the left border of the near plane from camera center.
        //! @param[in] near_right The offset of the right border of the near plane from camera center.
        //! @param[in] near_bottom The offset of the bottom border of the near plane from camera center.
        //! @param[in] near_top The offset of the top border of the near plane from camera center.
        //! @param[in] near_z The Z position of the near plane.
        //! @param[in] far_z The Z position of the far plane.
        //! @return Returns the projection matrix.
        Float4x4 make_perspective_off_center(f32 near_left, f32 near_right, f32 near_bottom, f32 near_top, f32 near_z, f32 far_z);
        //! Constructs one orthographic projection matrix using width, height near clipping distance, and far clipping distance.
        //! @param[in] near_width The width of the viewport.
        //! @param[in] near_height The height of the viewport.
        //! @param[in] near_z The Z position of the near plane.
        //! @param[in] far_z The Z position of the far plane.
        //! @return Returns the projection matrix.
        Float4x4 make_orthographic(f32 width, f32 height, f32 near_z, f32 far_z);
        //! Constructs one orthographic projection matrix using four offset values from the camera center, near clipping distance and far clipping distance.
        //! @param[in] left The offset of the left border of the viewport from camera center.
        //! @param[in] right The offset of the right border of the viewport from camera center.
        //! @param[in] bottom The offset of the bottom border of the viewport from camera center.
        //! @param[in] top The offset of the top border of the viewport from camera center.
        //! @param[in] near_z The Z position of the near plane.
        //! @param[in] far_z The Z position of the far plane.
        //! @return Returns the projection matrix.
        Float4x4 make_orthographic_off_center(f32 left, f32 right, f32 bottom, f32 top, f32 near_z, f32 far_z);
        
        //! @}
    }
}

#include "Impl/Transform.inl"