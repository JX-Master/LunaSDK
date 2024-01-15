/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Matrix.hpp
* @author JXMaster
* @date 2019/1/5
 */
#pragma once

#include "Quaternion.hpp"

namespace Luna
{
	//! @addtogroup RuntimeMath
	//! @{

	//! 3x3 matrix type with @ref f32 components.
	//! @details This matrix type is 16-bytes aligned and will use SIMD to accelerate matrix 
	//! calculations when possible.
	struct alignas(16) Float3x3
	{
		lustruct("Float3x3", "{7DD15385-7C4E-4018-9E0A-92A76671CC0B}");

		//! The array of rows of the matrix.
		Float3 r[3];

		//! Constructs one matrix with components uninitialized.
		Float3x3() = default;
		//! Constructs one matrix by coping components from another matrix.
		Float3x3(const Float3x3&) = default;
		//! Assigns one matrix by coping components from another matrix.
		Float3x3& operator=(const Float3x3& rhs) = default;
		//! Constructs one matrix by coping components from another matrix.
		Float3x3(Float3x3&& rhs) = default;
		//! Assigns one matrix by coping components from another matrix.
		Float3x3& operator=(Float3x3&& rhs) = default;
		//! Constructs one matrix from component values.
		//! @param[in] m00 The component at row 0 colume 0.
		//! @param[in] m01 The component at row 0 colume 1.
		//! @param[in] m02 The component at row 0 colume 2.
		//! @param[in] m10 The component at row 1 colume 0.
		//! @param[in] m11 The component at row 1 colume 1.
		//! @param[in] m12 The component at row 1 colume 2.
		//! @param[in] m20 The component at row 2 colume 0.
		//! @param[in] m21 The component at row 2 colume 1.
		//! @param[in] m22 The component at row 2 colume 2.
		constexpr Float3x3(f32 m00, f32 m01, f32 m02,
			f32 m10, f32 m11, f32 m12,
			f32 m20, f32 m21, f32 m22) :
			r  {{m00, m01, m02},
				{m10, m11, m12},
				{m20, m21, m22}} {}
		//! Gets the data of the matrix as one array of @ref f32 elements.
		//! @details Note that the matrix is 16 bytes aligned, so the row pitch in array data is 16 bytes, or 4 elements.
		//! For example, to fetch the element at row 1 column 2, use `data()[4 * 1 + 2]`.
		//! @return Returns one pointer to the data array.
		const f32* data() const { return r[0].m; }
		//! Gets the data of the matrix as one array of @ref f32 elements.
		//! @details Note that the matrix is 16 bytes aligned, so the row pitch in array data is 16 bytes, or 4 elements.
		//! For example, to fetch the element at row 1 column 2, use `data()[4 * 1 + 2]`.
		//! @return Returns one pointer to the data array.
		f32* data() { return r[0].m; }
		//! Compares two matrices for equality.
		//! @param[in] m The matrix to compare with.
		//! @return Returns `true` if two matrices are equal. Returns `false` otherwise.
		bool operator == (const Float3x3& m) const;
		//! Compares two matrices for non-equality.
		//! @param[in] m The matrix to compare with.
		//! @return Returns `true` if two matrices are not equal. Returns `false` otherwise.
		bool operator != (const Float3x3& m) const;
		//! Constructs one matrix from three @ref Float3 vectors.
		//! @param[in] row1 The vector for the first row of the matrix.
		//! @param[in] row2 The vector for the second row of the matrix.
		//! @param[in] row3 The vector for the third row of the matrix.
		Float3x3(const Float3& row1, const Float3& row2, const Float3& row3);
		//! Extracts the first row of the matrix.
		//! @return Returns the first row of the matrix.
		Float3 r1() const { return r[0]; }
		//! Extracts the second row of the matrix.
		//! @return Returns the second row of the matrix.
		Float3 r2() const { return r[1]; }
		//! Extracts the third row of the matrix.
		//! @return Returns the third row of the matrix.
		Float3 r3() const { return r[2]; }
		//! Extracts the first column of the matrix.
		//! @return Returns the first column of the matrix.
		Float3 c1() const { return Float3(r[0].x, r[1].x, r[2].x); }
		//! Extracts the second column of the matrix.
		//! @return Returns the second column of the matrix.
		Float3 c2() const { return Float3(r[0].y, r[1].y, r[2].y); }
		//! Extracts the third column of the matrix.
		//! @return Returns the third column of the matrix.
		Float3 c3() const { return Float3(r[0].z, r[1].z, r[2].z); }
		//! Gets the matrix as is.
		//! @return Returns one copy of the matrix.
		Float3x3 operator+() const { return *this; }
		//! Gets a negative version of the matrix.
		//! @return Returns a negative version of the matrix.
		Float3x3 operator-() const;
		//! Adds one matrix to this matrix. The addition is performed on every component of two matrices.
		//! @param[in] mat The matrix to add to this matrix.
		//! @return Returns `*this`.
		Float3x3& operator+=(const Float3x3& mat);
		//! Subtracts one matrix from this matrix. The subtraction is performed on every component of two matrices.
		//! @param[in] mat The matrix to subtract from this matrix.
		//! @return Returns `*this`.
		Float3x3& operator-=(const Float3x3& mat);
		//! Multiplies one matrix to this matrix. The multiplication is performed on every component of two matrices.
		//! @param[in] mat The matrix to multiply to this matrix.
		//! @return Returns `*this`.
		Float3x3& operator*=(const Float3x3& mat);
		//! Divides one matrix from this matrix. The division is performed on every component of two matrices.
		//! @param[in] mat The matrix to divide from this matrix.
		//! @return Returns `*this`.
		Float3x3& operator/=(const Float3x3& mat);
		//! Adds one scalar to this matrix. The scalar will be added to every component of the matrix.
		//! @param[in] s The scalar to add to this matrix.
		//! @return Returns `*this`.
		Float3x3& operator+=(f32 s);
		//! Subtracts one scalar from this matrix. The scalar will be subtracted from every component of the matrix.
		//! @param[in] s The scalar to subtract from this matrix.
		//! @return Returns `*this`.
		Float3x3& operator-=(f32 s);
		//! Multiplies one scalar to this matrix. The scalar will be multiplied to every component of the matrix.
		//! @param[in] s The scalar to multiply to this matrix.
		//! @return Returns `*this`.
		Float3x3& operator*=(f32 s);
		//! Divides one scalar from this matrix. The scalar will be divided from every component of the matrix.
		//! @param[in] s The scalar to divide from this matrix.
		//! @return Returns `*this`.
		Float3x3& operator/=(f32 s);

		//! Gets one identity matrix.
		//! @return Returns the constructed identity matrix.
		static constexpr Float3x3 identity()
		{
			return Float3x3(
				1.0f, 0.0f, 0.0f,
				0.0f, 1.0f, 0.0f,
				0.0f, 0.0f, 1.0f);
		}
	};

	static_assert(sizeof(Float3x3) == 12 * sizeof(f32), "Incorrect Float3x3 size.");

	//! Adds two matrices. The addition is performed on every component of two matrices.
	//! @param[in] m1 The first matrix.
	//! @param[in] m2 The second matrix.
	//! @return Returns the result matrix.
	Float3x3 operator+(const Float3x3& m1, const Float3x3& m2);
	//! Adds one matrix with one scalar. The scalar will be added to every component of the matrix.
	//! @param[in] m1 The matrix.
	//! @param[in] s The scalar.
	//! @return Reutrns the result matrix.
	Float3x3 operator+(const Float3x3& m1, f32 s);
	//! Adds one matrix with one scalar. The scalar will be added to every component of the matrix.
	//! @param[in] s The scalar.
	//! @param[in] m1 The matrix.
	//! @return Reutrns the result matrix.
	Float3x3 operator+(f32 s, const Float3x3& m1);
	//! Subtracts one matrix from another matrix (m1 - m2). The subtraction is performed on every component of two matrices.
	//! @param[in] m1 The first matrix.
	//! @param[in] m2 The second matrix.
	//! @return Returns the result matrix.
	Float3x3 operator-(const Float3x3& m1, const Float3x3& m2);
	//! Subtracts one scalar from one matrix (m1 - s). The scalar will be subtracted from every component of the matrix.
	//! @param[in] m1 The matrix.
	//! @param[in] s The scalar.
	//! @return Returns the result matrix.
	Float3x3 operator-(const Float3x3& m1, f32 s);
	//! Subtracts one matrix from one scalar (s - m1). This behaves like creating one matrix filled with scalar `s`, then
	//! subtracting matrix `m1` from the created matrix.
	//! @param[in] s The scalar.
	//! @param[in] m1 The matrix.
	//! @return Returns the result matrix.
	Float3x3 operator-(f32 s, const Float3x3& m1);
	//! Multiplies two matrices. The multiplication is performed on every component of two matrices.
	//! @param[in] m1 The first matrix.
	//! @param[in] m2 The second matrix.
	//! @return Returns the result matrix.
	Float3x3 operator*(const Float3x3& m1, const Float3x3& m2);
	//! Multiplies one matrix with one scalar. The scalar will be multiplied to every component of the matrix.
	//! @param[in] m1 The matrix.
	//! @param[in] s The scalar.
	//! @return Reutrns the result matrix.
	Float3x3 operator*(const Float3x3& m1, f32 s);
	//! Multiplies one matrix with one scalar. The scalar will be multiplied to every component of the matrix.
	//! @param[in] s The scalar.
	//! @param[in] m1 The matrix.
	//! @return Reutrns the result matrix.
	Float3x3 operator*(f32 s, const Float3x3& m1);
	//! Divides one matrix with another matrix (m1 / m2). The division is performed on every component of two matrices.
	//! @param[in] m1 The first matrix.
	//! @param[in] m2 The second matrix.
	//! @return Returns the result matrix.
	Float3x3 operator/(const Float3x3& m1, const Float3x3& m2);
	//! Divides one scalar with one matrix (m1 / s). The scalar will be divided from every component of the matrix.
	//! @param[in] m1 The matrix.
	//! @param[in] s The scalar.
	//! @return Returns the result matrix.
	Float3x3 operator/(const Float3x3& m1, f32 s);
	//! Divides one matrix with one scalar (s / m1). This behaves like creating one matrix filled with scalar `s`, then
	//! dividing the created matrix with matrix `m1`.
	//! @param[in] s The scalar.
	//! @param[in] m1 The matrix.
	//! @return Returns the result matrix.
	Float3x3 operator/(f32 s, const Float3x3& m1);
	//! Performs matrix multiplication between one vector and one matrix.
	//! @details This function performs the following operations:
	//! ```
	//! return Float3(
	//! 	vec.x * mat.r[0].x + vec.y * mat.r[1].x + vec.z * mat.r[2].x,
	//! 	vec.x * mat.r[0].y + vec.y * mat.r[1].y + vec.z * mat.r[2].y,
	//! 	vec.x * mat.r[0].z + vec.y * mat.r[1].z + vec.z * mat.r[2].z);
	//! ```
	//! @param[in] vec The vector.
	//! @param[in] mat The matrix.
	//! @return Returns the result vector.
	Float3 mul(const Float3& vec, const Float3x3& mat);
	//! Performs matrix multiplication between one matrix and one vector.
	//! @details This function performs the following operations:
	//! ```
	//! return Float3(
	//! 	vec.x * mat.r[0].x + vec.y * mat.r[0].y + vec.z * mat.r[0].z,
	//! 	vec.x * mat.r[1].x + vec.y * mat.r[1].y + vec.z * mat.r[1].z,
	//! 	vec.x * mat.r[2].x + vec.y * mat.r[2].y + vec.z * mat.r[2].z);
	//! ```
	//! @param[in] mat The matrix.
	//! @param[in] vec The vector.
	//! @return Returns the result vector.
	Float3 mul(const Float3x3& mat, const Float3& vec);
	//! Performs matrix multiplication between two matrices.
	//! @details This function performs the following operations:
	//! ```
	//! return Float3x3(
	//! 	m1.r[0].x * m2.r[0].x + m1.r[0].y * m2.r[1].x + m1.r[0].z * m2.r[2].x,
	//! 	m1.r[0].x * m2.r[0].y + m1.r[0].y * m2.r[1].y + m1.r[0].z * m2.r[2].y,
	//! 	m1.r[0].x * m2.r[0].z + m1.r[0].y * m2.r[1].z + m1.r[0].z * m2.r[2].z,
	//! 
	//! 	m1.r[1].x * m2.r[0].x + m1.r[1].y * m2.r[1].x + m1.r[1].z * m2.r[2].x,
	//! 	m1.r[1].x * m2.r[0].y + m1.r[1].y * m2.r[1].y + m1.r[1].z * m2.r[2].y,
	//! 	m1.r[1].x * m2.r[0].z + m1.r[1].y * m2.r[1].z + m1.r[1].z * m2.r[2].z,
	//! 
	//! 	m1.r[2].x * m2.r[0].x + m1.r[2].y * m2.r[1].x + m1.r[2].z * m2.r[2].x,
	//! 	m1.r[2].x * m2.r[0].y + m1.r[2].y * m2.r[1].y + m1.r[2].z * m2.r[2].y,
	//! 	m1.r[2].x * m2.r[0].z + m1.r[2].y * m2.r[1].z + m1.r[2].z * m2.r[2].z);
	//! ```
	//! @param[in] m1 The first matrix.
	//! @param[in] m2 The second matrix.
	//! @return Returns the result matrix.
	Float3x3 mul(const Float3x3& m1, const Float3x3& m2);
	//! Computes the determinant of the specified matrix.
	//! @details This function performs the following operations:
	//! ```
	//! return
	//! 	mat.r[0].x * (mat.r[1].y * mat.r[2].z - mat.r[1].z * mat.r[2].y) +
	//! 	mat.r[0].y * (mat.r[1].z * mat.r[2].x - mat.r[1].x * mat.r[2].z) +
	//! 	mat.r[0].z * (mat.r[1].x * mat.r[2].y - mat.r[1].y * mat.r[2].x);
	//! ```
	//! @param[in] mat The matrix to compute.
	//! @return Returns the computed determinant.
	f32 determinant(const Float3x3& mat);
	//! Computes the transpose matrix of the specified matrix.
	//! @details This function performs the following operations:
	//! ```
	//! return Float3x3(
	//! 	mat.r[0].x, mat.r[1].x, mat.r[2].x,
	//! 	mat.r[0].y, mat.r[1].y, mat.r[2].y,
	//! 	mat.r[0].z, mat.r[1].z, mat.r[2].z
	//! );
	//! ```
	//! @param[in] mat The matrix to compute.
	//! @return Returns the transpose matrix of the specified matrix.
	Float3x3 transpose(const Float3x3& mat);
	//! Computes the inversed matrix of the specified matrix.
	//! @details This function performs the following operations:
	//! ```
	//! f32 det = determinant(m);
	//! if (out_determinant)
	//! {
	//! 	*out_determinant = det;
	//! }
	//! if (det > -F32_EPSILON && det < F32_EPSILON)
	//! {
	//! 	det = F32_EPSILON;
	//! }
	//! f32 det_inv = 1.0f / det;
	//! Float3x3 r;
	//! r.r[0].x = det_inv * (mat.r[1].y * mat.r[2].z - mat.r[1].z * mat.r[2].y);
	//! r.r[1].x = det_inv * (mat.r[1].z * mat.r[2].x - mat.r[1].x * mat.r[2].z);
	//! r.r[2].x = det_inv * (mat.r[1].x * mat.r[2].y - mat.r[1].y * mat.r[2].x);
	//! r.r[0].y = det_inv * (mat.r[0].z * mat.r[2].y - mat.r[0].y * mat.r[2].z);
	//! r.r[1].y = det_inv * (mat.r[0].x * mat.r[2].z - mat.r[0].z * mat.r[2].x);
	//! r.r[2].y = det_inv * (mat.r[0].y * mat.r[2].x - mat.r[0].x * mat.r[2].y);
	//! r.r[0].z = det_inv * (mat.r[0].y * mat.r[1].z - mat.r[0].z * mat.r[1].y);
	//! r.r[1].z = det_inv * (mat.r[0].z * mat.r[1].x - mat.r[0].x * mat.r[1].z);
	//! r.r[2].z = det_inv * (mat.r[0].x * mat.r[1].y - mat.r[0].y * mat.r[1].x);
	//! return r;
	//! ```
	//! @param[in] mat The matrix to compute.
	//! @param[out] out_determinant If not `nullptr`, returns the determinant of `mat`.
	//! @return Returns the inversed matrix of the specified matrix.
	Float3x3 inverse(const Float3x3& mat, f32* out_determinant = nullptr);
	//! Unaligned 3x2 matrix with @ref f32 elements.
	//! @details This can be used to store 3x3 affine matrices since the third column is always (0, 0, 1), which
	//! can be removed to reduce memory size.
	struct Float3x2U
	{
		//! The array of rows of the matrix.
		Float2U r[3];
		//! Constructs one matrix with components uninitialized.
		Float3x2U() = default;
		//! Constructs one matrix by coping components from another matrix.
		Float3x2U(const Float3x2U&) = default;
		//! Assigns one matrix by coping components from another matrix.
		Float3x2U(Float3x2U&&) = default;
		//! Constructs one matrix by coping components from another matrix.
		Float3x2U& operator=(const Float3x2U&) = default;
		//! Assigns one matrix by coping components from another matrix.
		Float3x2U& operator=(Float3x2U&&) = default;
		//! Constructs one matrix by coping components from a @ref Float3x3 matrix.
		//! @details The third column of the source matrix will be discarded in the constructed matrix.
		//! @param[in] rhs The matrix to copy components from.
		Float3x2U(const Float3x3& rhs)
		{
			memcpy(r[0].m, rhs.r[0].m, sizeof(f32) * 2);
			memcpy(r[1].m, rhs.r[1].m, sizeof(f32) * 2);
			memcpy(r[2].m, rhs.r[2].m, sizeof(f32) * 2);
		}
		//! Assigns one matrix by coping components from a @ref Float3x3 matrix.
		//! @details The third column of the source matrix will be discarded in the constructed matrix.
		//! @param[in] rhs The matrix to copy components from.
		//! @return Returns `*this`.
		Float3x2U& operator=(const Float3x3& rhs)
		{
			memcpy(r[0].m, rhs.r[0].m, sizeof(f32) * 2);
			memcpy(r[1].m, rhs.r[1].m, sizeof(f32) * 2);
			memcpy(r[2].m, rhs.r[2].m, sizeof(f32) * 2);
			return *this;
		}
		//! Constructs one @ref Float3x3 matrix from this matrix.
		//! @param[in] column3 The component values for the third column of the destination matrix. The default 
		//! value is (0, 0, 1).
		//! @return Returns the constructed matrix.
		Float3x3 to_float3x3(const Float3& column3 = Float3(0.0f, 0.0f, 1.0f)) const
		{
			Float3x3 ret;
			memcpy(ret.r[0].m, r[0].m, sizeof(f32) * 2);
			memcpy(ret.r[1].m, r[1].m, sizeof(f32) * 2);
			memcpy(ret.r[2].m, r[2].m, sizeof(f32) * 2);
			ret.r[0].z = column3.x;
			ret.r[1].z = column3.y;
			ret.r[2].z = column3.z;
			return ret;
		}
	};
	//! Unaligned 3x3 matrix with @ref f32 elements.
	struct Float3x3U
	{
		//! The array of rows of the matrix.
		Float3U r[3];
		//! Constructs one matrix with components uninitialized.
		Float3x3U() = default;
		//! Constructs one matrix by coping components from another matrix.
		Float3x3U(const Float3x3U&) = default;
		//! Assigns one matrix by coping components from another matrix.
		Float3x3U& operator=(const Float3x3U&) = default;
		//! Constructs one matrix by coping components from another matrix.
		Float3x3U(Float3x3U&&) = default;
		//! Assigns one matrix by coping components from another matrix.
		Float3x3U& operator=(Float3x3U&&) = default;
		//! Constructs one matrix by coping components from a @ref Float3x3 matrix.
		//! @param[in] rhs The matrix to copy components from.
		Float3x3U(const Float3x3& rhs)
		{
			memcpy(r[0].m, rhs.r[0].m, sizeof(f32) * 3);
			memcpy(r[1].m, rhs.r[1].m, sizeof(f32) * 3);
			memcpy(r[2].m, rhs.r[2].m, sizeof(f32) * 3);
		}
		//! Assigns one matrix by coping components from a @ref Float3x3 matrix.
		//! @param[in] rhs The matrix to copy components from.
		//! @return Returns `*this`.
		Float3x3U& operator=(const Float3x3& rhs)
		{
			memcpy(r[0].m, rhs.r[0].m, sizeof(f32) * 3);
			memcpy(r[1].m, rhs.r[1].m, sizeof(f32) * 3);
			memcpy(r[2].m, rhs.r[2].m, sizeof(f32) * 3);
			return *this;
		}
		//! Constructs one @ref Float3x3 matrix from this matrix.
		//! @return Returns the constructed matrix.
		Float3x3 to_float3x3() const
		{
			Float3x3 ret;
			memcpy(ret.r[0].m, r[0].m, sizeof(f32) * 3);
			memcpy(ret.r[1].m, r[1].m, sizeof(f32) * 3);
			memcpy(ret.r[2].m, r[2].m, sizeof(f32) * 3);
			return ret;
		}
		//! Constructs one @ref Float3x3 matrix from this matrix.
		//! @return Returns the constructed matrix.
		operator Float3x3() const
		{
			return to_float3x3();
		}
	};
	//! 4x4 matrix type with @ref f32 components.
	//! @details This matrix type is 16-bytes aligned and will use SIMD to accelerate matrix 
	//! calculations when possible.
	struct alignas(16) Float4x4
	{
		lustruct("Float4x4", "{EE1F1000-29F9-4B91-953F-EE4D63BEDE9D}");

		//! The array of rows of the matrix.
		Float4 r[4];

		//! Constructs one matrix with components uninitialized.
		Float4x4() = default;
		//! Constructs one matrix by coping components from another matrix.
		Float4x4(const Float4x4&) = default;
		//! Assigns one matrix by coping components from another matrix.
		Float4x4& operator=(const Float4x4&) = default;
		//! Constructs one matrix by coping components from another matrix.
		Float4x4(Float4x4&&) = default;
		//! Assigns one matrix by coping components from another matrix.
		Float4x4& operator=(Float4x4&&) = default;
		//! Constructs one matrix from component values.
		//! @param[in] m00 The component at row 0 colume 0.
		//! @param[in] m01 The component at row 0 colume 1.
		//! @param[in] m02 The component at row 0 colume 2.
		//! @param[in] m03 The component at row 0 colume 3.
		//! @param[in] m10 The component at row 1 colume 0.
		//! @param[in] m11 The component at row 1 colume 1.
		//! @param[in] m12 The component at row 1 colume 2.
		//! @param[in] m13 The component at row 1 colume 3.
		//! @param[in] m20 The component at row 2 colume 0.
		//! @param[in] m21 The component at row 2 colume 1.
		//! @param[in] m22 The component at row 2 colume 2.
		//! @param[in] m23 The component at row 2 colume 3.
		//! @param[in] m30 The component at row 3 colume 0.
		//! @param[in] m31 The component at row 3 colume 1.
		//! @param[in] m32 The component at row 3 colume 2.
		//! @param[in] m33 The component at row 3 colume 3.
		constexpr Float4x4(f32 m00, f32 m01, f32 m02, f32 m03,
			f32 m10, f32 m11, f32 m12, f32 m13,
			f32 m20, f32 m21, f32 m22, f32 m23,
			f32 m30, f32 m31, f32 m32, f32 m33)
			: r{{m00, m01, m02, m03},
				{m10, m11, m12, m13},
				{m20, m21, m22, m23},
				{m30, m31, m32, m33}} {}
		//! Gets the data of the matrix as one array of @ref f32 elements.
		//! @details Note that the matrix is 16 bytes aligned, so the row pitch in array data is 16 bytes, or 4 elements.
		//! For example, to fetch the element at row 1 column 2, use `data()[4 * 1 + 2]`.
		//! @return Returns one pointer to the data array.
		const f32* data() const { return r[0].m; }
		//! Gets the data of the matrix as one array of @ref f32 elements.
		//! @details Note that the matrix is 16 bytes aligned, so the row pitch in array data is 16 bytes, or 4 elements.
		//! For example, to fetch the element at row 1 column 2, use `data()[4 * 1 + 2]`.
		//! @return Returns one pointer to the data array.
		f32* data() { return r[0].m; }
		//! Compares two matrices for equality.
		//! @param[in] m The matrix to compare with.
		//! @return Returns `true` if two matrices are equal. Returns `false` otherwise.
		bool operator == (const Float4x4& m) const;
		//! Compares two matrices for non-equality.
		//! @param[in] m The matrix to compare with.
		//! @return Returns `true` if two matrices are not equal. Returns `false` otherwise.
		bool operator != (const Float4x4& m) const;
		//! Constructs one matrix from four @ref Float4 vectors.
		//! @param[in] row1 The vector for the first row of the matrix.
		//! @param[in] row2 The vector for the second row of the matrix.
		//! @param[in] row3 The vector for the third row of the matrix.
		//! @param[in] row3 The vector for the fourth row of the matrix.
		Float4x4(const Float4& row1, const Float4& row2, const Float4& row3, const Float4& row4);
		//! Extracts the first row of the matrix.
		//! @return Returns the first row of the matrix.
		Float4 r1() const { return r[0]; }
		//! Extracts the second row of the matrix.
		//! @return Returns the second row of the matrix.
		Float4 r2() const { return r[1]; }
		//! Extracts the third row of the matrix.
		//! @return Returns the third row of the matrix.
		Float4 r3() const { return r[2]; }
		//! Extracts the fourth row of the matrix.
		//! @return Returns the fourth row of the matrix.
		Float4 r4() const { return r[3]; }
		//! Extracts the first column of the matrix.
		//! @return Returns the first column of the matrix.
		Float4 c1() const { return Float4(r[0].x, r[1].x, r[2].x, r[3].x); }
		//! Extracts the second column of the matrix.
		//! @return Returns the second column of the matrix.
		Float4 c2() const { return Float4(r[0].y, r[1].y, r[2].y, r[3].y); }
		//! Extracts the third column of the matrix.
		//! @return Returns the third column of the matrix.
		Float4 c3() const { return Float4(r[0].z, r[1].z, r[2].z, r[3].z); }
		//! Extracts the fourth column of the matrix.
		//! @return Returns the fourth column of the matrix.
		Float4 c4() const { return Float4(r[0].w, r[1].w, r[2].w, r[3].w); }
		//! Gets the matrix as is.
		//! @return Returns one copy of the matrix.
		Float4x4 operator+() const { return *this; }
		//! Gets a negative version of the matrix.
		//! @return Returns a negative version of the matrix.
		Float4x4 operator- () const;
		//! Adds one matrix to this matrix. The addition is performed on every component of two matrices.
		//! @param[in] mat The matrix to add to this matrix.
		//! @return Returns `*this`.
		Float4x4& operator+=(const Float4x4& mat);
		//! Subtracts one matrix from this matrix. The subtraction is performed on every component of two matrices.
		//! @param[in] mat The matrix to subtract from this matrix.
		//! @return Returns `*this`.
		Float4x4& operator-=(const Float4x4& mat);
		//! Multiplies one matrix to this matrix. The multiplication is performed on every component of two matrices.
		//! @param[in] mat The matrix to multiply to this matrix.
		//! @return Returns `*this`.
		Float4x4& operator*=(const Float4x4& mat);
		//! Divides one matrix from this matrix. The division is performed on every component of two matrices.
		//! @param[in] mat The matrix to divide from this matrix.
		//! @return Returns `*this`.
		Float4x4& operator/=(const Float4x4& mat);
		//! Adds one scalar to this matrix. The scalar will be added to every component of the matrix.
		//! @param[in] s The scalar to add to this matrix.
		//! @return Returns `*this`.
		Float4x4& operator+=(f32 s);
		//! Subtracts one scalar from this matrix. The scalar will be subtracted from every component of the matrix.
		//! @param[in] s The scalar to subtract from this matrix.
		//! @return Returns `*this`.
		Float4x4& operator-=(f32 s);
		//! Multiplies one scalar to this matrix. The scalar will be multiplied to every component of the matrix.
		//! @param[in] s The scalar to multiply to this matrix.
		//! @return Returns `*this`.
		Float4x4& operator*=(f32 s);
		//! Divides one scalar from this matrix. The scalar will be divided from every component of the matrix.
		//! @param[in] s The scalar to divide from this matrix.
		//! @return Returns `*this`.
		Float4x4& operator/=(f32 s);

		//! Gets one identity matrix.
		//! @return Returns the constructed identity matrix.
		static constexpr Float4x4 identity()
		{
			return Float4x4(
				1.0f, 0.0f, 0.0f, 0.0f,
				0.0f, 1.0f, 0.0f, 0.0f,
				0.0f, 0.0f, 1.0f, 0.0f,
				0.0f, 0.0f, 0.0f, 1.0f);
		}
	};

	static_assert(sizeof(Float4x4) == 16 * sizeof(f32), "Incorrect Float4x4 size.");

	//! Adds two matrices. The addition is performed on every component of two matrices.
	//! @param[in] m1 The first matrix.
	//! @param[in] m2 The second matrix.
	//! @return Returns the result matrix.
	Float4x4 operator+(const Float4x4& m1, const Float4x4& m2);
	//! Adds one matrix with one scalar. The scalar will be added to every component of the matrix.
	//! @param[in] m1 The matrix.
	//! @param[in] s The scalar.
	//! @return Reutrns the result matrix.
	Float4x4 operator+(const Float4x4& m1, f32 s);
	//! Adds one matrix with one scalar. The scalar will be added to every component of the matrix.
	//! @param[in] s The scalar.
	//! @param[in] m1 The matrix.
	//! @return Reutrns the result matrix.
	Float4x4 operator+(f32 s, const Float4x4& m1);
	//! Subtracts one matrix from another matrix (m1 - m2). The subtraction is performed on every component of two matrices.
	//! @param[in] m1 The first matrix.
	//! @param[in] m2 The second matrix.
	//! @return Returns the result matrix.
	Float4x4 operator-(const Float4x4& m1, const Float4x4& m2);
	//! Subtracts one scalar from one matrix (m1 - s). The scalar will be subtracted from every component of the matrix.
	//! @param[in] m1 The matrix.
	//! @param[in] s The scalar.
	//! @return Returns the result matrix.
	Float4x4 operator-(const Float4x4& m1, f32 s);
	//! Subtracts one matrix from one scalar (s - m1). This behaves like creating one matrix filled with scalar `s`, then
	//! subtracting matrix `m1` from the created matrix.
	//! @param[in] s The scalar.
	//! @param[in] m1 The matrix.
	//! @return Returns the result matrix.
	Float4x4 operator-(f32 s, const Float4x4& m1);
	//! Multiplies two matrices. The multiplication is performed on every component of two matrices.
	//! @param[in] m1 The first matrix.
	//! @param[in] m2 The second matrix.
	//! @return Returns the result matrix.
	Float4x4 operator*(const Float4x4& m1, const Float4x4& m2);
	//! Multiplies one matrix with one scalar. The scalar will be multiplied to every component of the matrix.
	//! @param[in] m1 The matrix.
	//! @param[in] s The scalar.
	//! @return Reutrns the result matrix.
	Float4x4 operator*(const Float4x4& m1, f32 s);
	//! Multiplies one matrix with one scalar. The scalar will be multiplied to every component of the matrix.
	//! @param[in] s The scalar.
	//! @param[in] m1 The matrix.
	//! @return Reutrns the result matrix.
	Float4x4 operator*(f32 s, const Float4x4& m1);
	//! Divides one matrix with another matrix (m1 / m2). The division is performed on every component of two matrices.
	//! @param[in] m1 The first matrix.
	//! @param[in] m2 The second matrix.
	//! @return Returns the result matrix.
	Float4x4 operator/(const Float4x4& m1, const Float4x4& m2);
	//! Divides one scalar with one matrix (m1 / s). The scalar will be divided from every component of the matrix.
	//! @param[in] m1 The matrix.
	//! @param[in] s The scalar.
	//! @return Returns the result matrix.
	Float4x4 operator/(const Float4x4& m1, f32 s);
	//! Divides one matrix with one scalar (s / m1). This behaves like creating one matrix filled with scalar `s`, then
	//! dividing the created matrix with matrix `m1`.
	//! @param[in] s The scalar.
	//! @param[in] m1 The matrix.
	//! @return Returns the result matrix.
	Float4x4 operator/(f32 s, const Float4x4& m1);
	//! Performs matrix multiplication between one vector and one matrix.
	//! @details This function performs the following operations:
	//! ```
	//! return Float4(
	//! 	vec.x * mat.r[0].x + vec.y * mat.r[1].x + vec.z * mat.r[2].x + vec.w * mat.r[3].x,
	//! 	vec.x * mat.r[0].y + vec.y * mat.r[1].y + vec.z * mat.r[2].y + vec.w * mat.r[3].y,
	//! 	vec.x * mat.r[0].z + vec.y * mat.r[1].z + vec.z * mat.r[2].z + vec.w * mat.r[3].z,
	//! 	vec.x * mat.r[0].w + vec.y * mat.r[1].w + vec.z * mat.r[2].w + vec.w * mat.r[3].w);
	//! ```
	//! @param[in] vec The vector.
	//! @param[in] mat The matrix.
	//! @return Returns the result vector.
	Float4 mul(const Float4& vec, const Float4x4& mat);
	//! Performs matrix multiplication between one matrix and one vector.
	//! @details This function performs the following operations:
	//! ```
	//! return Float4(
	//! 	vec.x * mat.r[0].x + vec.y * mat.r[0].y + vec.z * mat.r[0].z + vec.w * mat.r[0].w,
	//! 	vec.x * mat.r[1].x + vec.y * mat.r[1].y + vec.z * mat.r[1].z + vec.w * mat.r[1].w,
	//! 	vec.x * mat.r[2].x + vec.y * mat.r[2].y + vec.z * mat.r[2].z + vec.w * mat.r[2].w,
	//! 	vec.x * mat.r[3].x + vec.y * mat.r[3].y + vec.z * mat.r[3].z + vec.w * mat.r[3].w);
	//! ```
	//! @param[in] mat The matrix.
	//! @param[in] vec The vector.
	//! @return Returns the result vector.
	Float4 mul(const Float4x4& mat, const Float4& vec);
	//! Performs matrix multiplication between two matrices.
	//! @details This function performs the following operations:
	//! ```
	//! return Float4x4(
	//! 	m1.r[0].x * m2.r[0].x + m1.r[0].y * m2.r[1].x + m1.r[0].z * m2.r[2].x + m1.r[0].w * m2.r[3].x,
	//! 	m1.r[0].x * m2.r[0].y + m1.r[0].y * m2.r[1].y + m1.r[0].z * m2.r[2].y + m1.r[0].w * m2.r[3].y,
	//! 	m1.r[0].x * m2.r[0].z + m1.r[0].y * m2.r[1].z + m1.r[0].z * m2.r[2].z + m1.r[0].w * m2.r[3].z,
	//! 	m1.r[0].x * m2.r[0].w + m1.r[0].y * m2.r[1].w + m1.r[0].z * m2.r[2].w + m1.r[0].w * m2.r[3].w,
	//! 	m1.r[1].x * m2.r[0].x + m1.r[1].y * m2.r[1].x + m1.r[1].z * m2.r[2].x + m1.r[1].w * m2.r[3].x,
	//! 	m1.r[1].x * m2.r[0].y + m1.r[1].y * m2.r[1].y + m1.r[1].z * m2.r[2].y + m1.r[1].w * m2.r[3].y,
	//! 	m1.r[1].x * m2.r[0].z + m1.r[1].y * m2.r[1].z + m1.r[1].z * m2.r[2].z + m1.r[1].w * m2.r[3].z,
	//! 	m1.r[1].x * m2.r[0].w + m1.r[1].y * m2.r[1].w + m1.r[1].z * m2.r[2].w + m1.r[1].w * m2.r[3].w,
	//! 	m1.r[2].x * m2.r[0].x + m1.r[2].y * m2.r[1].x + m1.r[2].z * m2.r[2].x + m1.r[2].w * m2.r[3].x,
	//! 	m1.r[2].x * m2.r[0].y + m1.r[2].y * m2.r[1].y + m1.r[2].z * m2.r[2].y + m1.r[2].w * m2.r[3].y,
	//! 	m1.r[2].x * m2.r[0].z + m1.r[2].y * m2.r[1].z + m1.r[2].z * m2.r[2].z + m1.r[2].w * m2.r[3].z,
	//! 	m1.r[2].x * m2.r[0].w + m1.r[2].y * m2.r[1].w + m1.r[2].z * m2.r[2].w + m1.r[2].w * m2.r[3].w,
	//! 	m1.r[3].x * m2.r[0].x + m1.r[3].y * m2.r[1].x + m1.r[3].z * m2.r[2].x + m1.r[3].w * m2.r[3].x,
	//! 	m1.r[3].x * m2.r[0].y + m1.r[3].y * m2.r[1].y + m1.r[3].z * m2.r[2].y + m1.r[3].w * m2.r[3].y,
	//! 	m1.r[3].x * m2.r[0].z + m1.r[3].y * m2.r[1].z + m1.r[3].z * m2.r[2].z + m1.r[3].w * m2.r[3].z,
	//! 	m1.r[3].x * m2.r[0].w + m1.r[3].y * m2.r[1].w + m1.r[3].z * m2.r[2].w + m1.r[3].w * m2.r[3].w);
	//! ```
	//! @param[in] m1 The first matrix.
	//! @param[in] m2 The second matrix.
	//! @return Returns the result matrix.
	Float4x4 mul(const Float4x4& mat1, const Float4x4& mat2);
	//! Computes the determinant of the specified matrix.
	//! @details This function performs the following operations:
	//! ```
	//! return
	//! 	 m.r[0].x * (m.r[1].y * (m.r[2].z * m.r[3].w - m.r[2].w * m.r[3].z) + m.r[1].z * (m.r[2].w * m.r[3].y - m.r[2].y * m.r[3].w) + m.r[1].w * (m.r[2].y * m.r[3].z - m.r[2].z * m.r[3].y))
	//! 	-m.r[0].y * (m.r[1].x * (m.r[2].z * m.r[3].w - m.r[2].w * m.r[3].z) + m.r[1].z * (m.r[2].w * m.r[3].x - m.r[2].x * m.r[3].w) + m.r[1].w * (m.r[2].x * m.r[3].z - m.r[2].z * m.r[3].x))
	//! 	+m.r[0].z * (m.r[1].x * (m.r[2].y * m.r[3].w - m.r[2].w * m.r[3].y) + m.r[1].y * (m.r[2].w * m.r[3].x - m.r[2].x * m.r[3].w) + m.r[1].w * (m.r[2].x * m.r[3].y - m.r[2].y * m.r[3].x))
	//! 	-m.r[0].w * (m.r[1].x * (m.r[2].y * m.r[3].z - m.r[2].z * m.r[3].y) + m.r[1].y * (m.r[2].z * m.r[3].x - m.r[2].x * m.r[3].z) + m.r[1].z * (m.r[2].x * m.r[3].y - m.r[2].y * m.r[3].x));
	//! ```
	//! @param[in] mat The matrix to compute.
	//! @return Returns the computed determinant.
	f32 determinant(const Float4x4& mat);
	//! Computes the transpose matrix of the specified matrix.
	//! @details This function performs the following operations:
	//! ```
	//! return Float4x4(
	//! 	mat.r[0].x, mat.r[1].x, mat.r[2].x, mat.r[3].x,
	//! 	mat.r[0].y, mat.r[1].y, mat.r[2].y, mat.r[3].y,
	//! 	mat.r[0].z, mat.r[1].z, mat.r[2].z, mat.r[3].z,
	//! 	mat.r[0].w, mat.r[1].w, mat.r[2].w, mat.r[3].w
	//! );
	//! ```
	//! @param[in] mat The matrix to compute.
	//! @return Returns the transpose matrix of the specified matrix.
	Float4x4 transpose(const Float4x4& mat);
	//! Computes the inversed matrix of the specified matrix.
	//! @details This function performs the following operations:
	//! ```
	//! f32 det = determinant(m);
	//! if (out_determinant)
	//! {
	//! 	*out_determinant = det;
	//! }
	//! if (det > -F32_EPSILON && det < F32_EPSILON)
	//! {
	//! 	det = F32_EPSILON;
	//! }
	//! f32 det_inv = 1.0f / det;
	//! Float4x4 r;
	//! r.r[0].x =  det_inv * (mat.r[1].y * (mat.r[2].z * mat.r[3].w - mat.r[2].w * mat.r[3].z) + mat.r[1].z * (mat.r[2].w * mat.r[3].y - mat.r[2].y * mat.r[3].w) + mat.r[1].w * (mat.r[2].y * mat.r[3].z - mat.r[2].z * mat.r[3].y));
	//! r.r[1].x = -det_inv * (mat.r[1].x * (mat.r[2].z * mat.r[3].w - mat.r[2].w * mat.r[3].z) + mat.r[1].z * (mat.r[2].w * mat.r[3].x - mat.r[2].x * mat.r[3].w) + mat.r[1].w * (mat.r[2].x * mat.r[3].z - mat.r[2].z * mat.r[3].x));
	//! r.r[2].x =  det_inv * (mat.r[1].x * (mat.r[2].y * mat.r[3].w - mat.r[2].w * mat.r[3].y) + mat.r[1].y * (mat.r[2].w * mat.r[3].x - mat.r[2].x * mat.r[3].w) + mat.r[1].w * (mat.r[2].x * mat.r[3].y - mat.r[2].y * mat.r[3].x));
	//! r.r[3].x = -det_inv * (mat.r[1].x * (mat.r[2].y * mat.r[3].z - mat.r[2].z * mat.r[3].y) + mat.r[1].y * (mat.r[2].z * mat.r[3].x - mat.r[2].x * mat.r[3].z) + mat.r[1].z * (mat.r[2].x * mat.r[3].y - mat.r[2].y * mat.r[3].x));
	//! r.r[0].y = -det_inv * (mat.r[0].y * (mat.r[2].z * mat.r[3].w - mat.r[2].w * mat.r[3].z) + mat.r[0].z * (mat.r[2].w * mat.r[3].y - mat.r[2].y * mat.r[3].w) + mat.r[0].w * (mat.r[2].y * mat.r[3].z - mat.r[2].z * mat.r[3].y));
	//! r.r[1].y =  det_inv * (mat.r[0].x * (mat.r[2].z * mat.r[3].w - mat.r[2].w * mat.r[3].z) + mat.r[0].z * (mat.r[2].w * mat.r[3].x - mat.r[2].x * mat.r[3].w) + mat.r[0].w * (mat.r[2].x * mat.r[3].z - mat.r[2].z * mat.r[3].x));
	//! r.r[2].y = -det_inv * (mat.r[0].x * (mat.r[2].y * mat.r[3].w - mat.r[2].w * mat.r[3].y) + mat.r[0].y * (mat.r[2].w * mat.r[3].x - mat.r[2].x * mat.r[3].w) + mat.r[0].w * (mat.r[2].x * mat.r[3].y - mat.r[2].y * mat.r[3].x));
	//! r.r[3].y =  det_inv * (mat.r[0].x * (mat.r[2].y * mat.r[3].z - mat.r[2].z * mat.r[3].y) + mat.r[0].y * (mat.r[2].z * mat.r[3].x - mat.r[2].x * mat.r[3].z) + mat.r[0].z * (mat.r[2].x * mat.r[3].y - mat.r[2].y * mat.r[3].x));
	//! r.r[0].z =  det_inv * (mat.r[3].w * (mat.r[0].y * mat.r[1].z - mat.r[0].z * mat.r[1].y) + mat.r[3].z * (mat.r[0].w * mat.r[1].y - mat.r[0].y * mat.r[1].w) + mat.r[3].y * (mat.r[0].z * mat.r[1].w - mat.r[0].w * mat.r[1].z));
	//! r.r[1].z = -det_inv * (mat.r[3].w * (mat.r[0].x * mat.r[1].z - mat.r[0].z * mat.r[1].x) + mat.r[3].z * (mat.r[0].w * mat.r[1].x - mat.r[0].x * mat.r[1].w) + mat.r[3].x * (mat.r[0].z * mat.r[1].w - mat.r[0].w * mat.r[1].z));
	//! r.r[2].z =  det_inv * (mat.r[3].w * (mat.r[0].x * mat.r[1].y - mat.r[0].y * mat.r[1].x) + mat.r[3].y * (mat.r[0].w * mat.r[1].x - mat.r[0].x * mat.r[1].w) + mat.r[3].x * (mat.r[0].y * mat.r[1].w - mat.r[0].w * mat.r[1].y));
	//! r.r[3].z = -det_inv * (mat.r[3].z * (mat.r[0].x * mat.r[1].y - mat.r[0].y * mat.r[1].x) + mat.r[3].y * (mat.r[0].z * mat.r[1].x - mat.r[0].x * mat.r[1].z) + mat.r[3].x * (mat.r[0].y * mat.r[1].z - mat.r[0].z * mat.r[1].y));
	//! r.r[0].w = -det_inv * (mat.r[2].w * (mat.r[0].y * mat.r[1].z - mat.r[0].z * mat.r[1].y) + mat.r[2].z * (mat.r[0].w * mat.r[1].y - mat.r[0].y * mat.r[1].w) + mat.r[2].y * (mat.r[0].z * mat.r[1].w - mat.r[0].w * mat.r[1].z));
	//! r.r[1].w =  det_inv * (mat.r[2].w * (mat.r[0].x * mat.r[1].z - mat.r[0].z * mat.r[1].x) + mat.r[2].z * (mat.r[0].w * mat.r[1].x - mat.r[0].x * mat.r[1].w) + mat.r[2].x * (mat.r[0].z * mat.r[1].w - mat.r[0].w * mat.r[1].z));
	//! r.r[2].w = -det_inv * (mat.r[2].w * (mat.r[0].x * mat.r[1].y - mat.r[0].y * mat.r[1].x) + mat.r[2].y * (mat.r[0].w * mat.r[1].x - mat.r[0].x * mat.r[1].w) + mat.r[2].x * (mat.r[0].y * mat.r[1].w - mat.r[0].w * mat.r[1].y));
	//! r.r[3].w =  det_inv * (mat.r[2].z * (mat.r[0].x * mat.r[1].y - mat.r[0].y * mat.r[1].x) + mat.r[2].y * (mat.r[0].z * mat.r[1].x - mat.r[0].x * mat.r[1].z) + mat.r[2].x * (mat.r[0].y * mat.r[1].z - mat.r[0].z * mat.r[1].y));
	//! return r;
	//! ```
	//! @param[in] mat The matrix to compute.
	//! @param[out] out_determinant If not `nullptr`, returns the determinant of `mat`.
	//! @return Returns the inversed matrix of the specified matrix.
	Float4x4 inverse(const Float4x4& mat, f32* out_determinant = nullptr);
	//! Unaligned 4x3 matrix with @ref f32 elements.
	//! @details This can be used to store 4x4 affine matrices since the fourth column is always (0, 0, 0, 1), which
	//! can be removed to reduce memory size.
	struct Float4x3U
	{
		//! The array of rows of the matrix.
		Float3U r[4];
		//! Constructs one matrix with components uninitialized.
		Float4x3U() = default;
		//! Constructs one matrix by coping components from another matrix.
		Float4x3U(const Float4x3U&) = default;
		//! Assigns one matrix by coping components from another matrix.
		Float4x3U(Float4x3U&&) = default;
		//! Constructs one matrix by coping components from another matrix.
		Float4x3U& operator=(const Float4x3U&) = default;
		//! Assigns one matrix by coping components from another matrix.
		Float4x3U& operator=(Float4x3U&&) = default;
		//! Constructs one matrix by coping components from a @ref Float4x4 matrix.
		//! @details The fourth column of the source matrix will be discarded in the constructed matrix.
		//! @param[in] rhs The matrix to copy components from.
		Float4x3U(const Float4x4& rhs)
		{
			memcpy(r[0].m, rhs.r[0].m, sizeof(f32) * 3);
			memcpy(r[1].m, rhs.r[1].m, sizeof(f32) * 3);
			memcpy(r[2].m, rhs.r[2].m, sizeof(f32) * 3);
			memcpy(r[3].m, rhs.r[3].m, sizeof(f32) * 3);
		}
		//! Assigns one matrix by coping components from a @ref Float4x4 matrix.
		//! @details The fourth column of the source matrix will be discarded in the constructed matrix.
		//! @param[in] rhs The matrix to copy components from.
		//! @return Returns `*this`.
		Float4x3U& operator=(const Float4x4& rhs)
		{
			memcpy(r[0].m, rhs.r[0].m, sizeof(f32) * 3);
			memcpy(r[1].m, rhs.r[1].m, sizeof(f32) * 3);
			memcpy(r[2].m, rhs.r[2].m, sizeof(f32) * 3);
			memcpy(r[3].m, rhs.r[3].m, sizeof(f32) * 3);
			return *this;
		}
		//! Constructs one @ref Float4x4 matrix from this matrix.
		//! @param[in] column4 The component values for the fourth column of the destination matrix. The default 
		//! value is (0, 0, 0, 1).
		//! @return Returns the constructed matrix.
		Float4x4 to_float4x4(const Float4& column4 = Float4(0.0f, 0.0f, 0.0f, 1.0f)) const
		{
			Float4x4 ret;
			memcpy(ret.r[0].m, r[0].m, sizeof(f32) * 3);
			memcpy(ret.r[1].m, r[1].m, sizeof(f32) * 3);
			memcpy(ret.r[2].m, r[2].m, sizeof(f32) * 3);
			memcpy(ret.r[3].m, r[3].m, sizeof(f32) * 3);
			ret.r[0].w = column4.x;
			ret.r[1].w = column4.y;
			ret.r[2].w = column4.z;
			ret.r[3].w = column4.w;
			return ret;
		}
	};
	//! Unaligned 4x4 matrix with @ref f32 elements.
	struct Float4x4U
	{
		//! The array of rows of the matrix.
		Float4U r[4];
		//! Constructs one matrix with components uninitialized.
		Float4x4U() = default;
		//! Constructs one matrix by coping components from another matrix.
		Float4x4U(const Float4x4U&) = default;
		//! Assigns one matrix by coping components from another matrix.
		Float4x4U& operator=(const Float4x4U&) = default;
		//! Constructs one matrix by coping components from another matrix.
		Float4x4U(Float4x4U&&) = default;
		//! Assigns one matrix by coping components from another matrix.
		Float4x4U& operator=(Float4x4U&&) = default;
		//! Constructs one matrix by coping components from a @ref Float4x4 matrix.
		//! @param[in] rhs The matrix to copy components from.
		Float4x4U(const Float4x4& rhs)
		{
			memcpy(r[0].m, rhs.r[0].m, sizeof(f32) * 4);
			memcpy(r[1].m, rhs.r[1].m, sizeof(f32) * 4);
			memcpy(r[2].m, rhs.r[2].m, sizeof(f32) * 4);
			memcpy(r[3].m, rhs.r[3].m, sizeof(f32) * 4);
		}
		//! Assigns one matrix by coping components from a @ref Float4x4 matrix.
		//! @param[in] rhs The matrix to copy components from.
		//! @return Returns `*this`.
		Float4x4U& operator=(const Float4x4& rhs)
		{
			memcpy(r[0].m, rhs.r[0].m, sizeof(f32) * 4);
			memcpy(r[1].m, rhs.r[1].m, sizeof(f32) * 4);
			memcpy(r[2].m, rhs.r[2].m, sizeof(f32) * 4);
			memcpy(r[3].m, rhs.r[3].m, sizeof(f32) * 4);
			return *this;
		}
		//! Constructs one @ref Float4x4 matrix from this matrix.
		//! @return Returns the constructed matrix.
		Float4x4 to_float4x4() const
		{
			Float4x4 ret;
			memcpy(ret.r[0].m, r[0].m, sizeof(f32) * 4);
			memcpy(ret.r[1].m, r[1].m, sizeof(f32) * 4);
			memcpy(ret.r[2].m, r[2].m, sizeof(f32) * 4);
			memcpy(ret.r[3].m, r[3].m, sizeof(f32) * 4);
			return ret;
		}
		//! Constructs one @ref Float4x4 matrix from this matrix.
		//! @return Returns the constructed matrix.
		operator Float4x4() const
		{
			return to_float4x4();
		}
	};
	//! Gets the type object of @ref Float3x3.
	//! @return Returns the type object.
	LUNA_RUNTIME_API typeinfo_t float3x3_type();
	//! Gets the type object of @ref Float4x4.
	//! @return Returns the type object.
	LUNA_RUNTIME_API typeinfo_t float4x4_type();
	template <> struct typeof_t<Float3x3> { typeinfo_t operator()() const { return float3x3_type(); } };
	template <> struct typeof_t<Float4x4> { typeinfo_t operator()() const { return float4x4_type(); } };

	//! @}
}

#include "Impl/Float3x3.inl"
#include "Impl/Float4x4.inl"
