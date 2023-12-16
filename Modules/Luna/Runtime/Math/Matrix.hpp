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
	struct alignas(16) Float3x3
	{
		lustruct("Float3x3", "{7DD15385-7C4E-4018-9E0A-92A76671CC0B}");

		Float3 r[3];

		Float3x3() = default;
		Float3x3(const Float3x3&) = default;
		Float3x3& operator=(const Float3x3& rhs) = default;

		Float3x3(Float3x3&& rhs) = default;
		Float3x3& operator=(Float3x3&& rhs) = default;

		constexpr Float3x3(f32 m00, f32 m01, f32 m02,
			f32 m10, f32 m11, f32 m12,
			f32 m20, f32 m21, f32 m22) :
			r  {{m00, m01, m02},
				{m10, m11, m12},
				{m20, m21, m22}} {}

		const f32* data() const { return r[0].m; }
		f32* data() { return r[0].m; }

		// Comparison operators
		bool operator == (const Float3x3& m) const;
		bool operator != (const Float3x3& m) const;

		// Assignment operators
		Float3x3(const Float3& row1, const Float3& row2, const Float3& row3);

		Float3 r1() const { return r[0]; }
		Float3 r2() const { return r[1]; }
		Float3 r3() const { return r[2]; }
		Float3 c1() const { return Float3(r[0].x, r[1].x, r[2].x); }
		Float3 c2() const { return Float3(r[0].y, r[1].y, r[2].y); }
		Float3 c3() const { return Float3(r[0].z, r[1].z, r[2].z); }

		// Unary operators.
		Float3x3 operator+() const { return *this; }
		Float3x3 operator-() const;

		// per-component operations.
		Float3x3& operator+=(const Float3x3&);
		Float3x3& operator-=(const Float3x3&);
		Float3x3& operator*=(const Float3x3&);
		Float3x3& operator/=(const Float3x3&);
		Float3x3& operator+=(f32 s);
		Float3x3& operator-=(f32 s);
		Float3x3& operator*=(f32 s);
		Float3x3& operator/=(f32 s);

		// Constants
		static constexpr Float3x3 identity()
		{
			return Float3x3(
				1.0f, 0.0f, 0.0f,
				0.0f, 1.0f, 0.0f,
				0.0f, 0.0f, 1.0f);
		}
	};

	// per-component operations.
	Float3x3 operator+(const Float3x3& m1, const Float3x3& m2);
	Float3x3 operator+(const Float3x3& m1, f32 s);
	Float3x3 operator+(f32 s, const Float3x3& m1);
	Float3x3 operator-(const Float3x3& m1, const Float3x3& m2);
	Float3x3 operator-(const Float3x3& m1, f32 s);
	Float3x3 operator-(f32 s, const Float3x3& m1);
	Float3x3 operator*(const Float3x3& m1, const Float3x3& m2);
	Float3x3 operator*(const Float3x3& m1, f32 s);
	Float3x3 operator*(f32 s, const Float3x3& m1);
	Float3x3 operator/(const Float3x3& m1, const Float3x3& m2);
	Float3x3 operator/(const Float3x3& m1, f32 s);
	Float3x3 operator/(f32 s, const Float3x3& m1);

	// matrix math.
	Float3 mul(const Float3& vec, const Float3x3& mat);
	Float3 mul(const Float3x3& mat, const Float3& vec);
	Float3x3 mul(const Float3x3& m1, const Float3x3& m2);

	static_assert(sizeof(Float3x3) == 12 * sizeof(f32), "Incorrect Float3x3 size.");

	struct Float3x2U
	{
		f32 m[3][2];
		Float3x2U() = default;
		Float3x2U(const Float3x2U&) = default;
		Float3x2U(Float3x2U&&) = default;
		Float3x2U& operator=(const Float3x2U&) = default;
		Float3x2U& operator=(Float3x2U&&) = default;
		Float3x2U(const Float3x3 & rhs)
		{
			memcpy(m[0], rhs.r[0].m, sizeof(f32) * 2);
			memcpy(m[1], rhs.r[1].m, sizeof(f32) * 2);
			memcpy(m[2], rhs.r[2].m, sizeof(f32) * 2);
		}
		Float3x2U& operator=(const Float3x3 & rhs)
		{
			memcpy(m[0], rhs.r[0].m, sizeof(f32) * 2);
			memcpy(m[1], rhs.r[1].m, sizeof(f32) * 2);
			memcpy(m[2], rhs.r[2].m, sizeof(f32) * 2);
			return *this;
		}
		Float3x3 to_float3x3(const Float3& column3 = Float3(0.0f, 0.0f, 1.0f)) const
		{
			Float3x3 ret;
			memcpy(ret.r[0].m, m[0], sizeof(f32) * 2);
			memcpy(ret.r[1].m, m[1], sizeof(f32) * 2);
			memcpy(ret.r[2].m, m[2], sizeof(f32) * 2);
			ret.r[0].z = column3.x;
			ret.r[1].z = column3.y;
			ret.r[2].z = column3.z;
			return ret;
		}
	};
	struct Float3x3U
	{
		f32 m[3][3];
		Float3x3U() = default;
		Float3x3U(const Float3x3U&) = default;
		Float3x3U(Float3x3U&&) = default;
		Float3x3U& operator=(const Float3x3U&) = default;
		Float3x3U& operator=(Float3x3U&&) = default;
		Float3x3U(const Float3x3& rhs)
		{
			memcpy(m[0], rhs.r[0].m, sizeof(f32) * 3);
			memcpy(m[1], rhs.r[1].m, sizeof(f32) * 3);
			memcpy(m[2], rhs.r[2].m, sizeof(f32) * 3);
		}
		Float3x3U& operator=(const Float3x3& rhs)
		{
			memcpy(m[0], rhs.r[0].m, sizeof(f32) * 3);
			memcpy(m[1], rhs.r[1].m, sizeof(f32) * 3);
			memcpy(m[2], rhs.r[2].m, sizeof(f32) * 3);
			return *this;
		}
		Float3x3 to_float3x3() const
		{
			Float3x3 ret;
			memcpy(ret.r[0].m, m[0], sizeof(f32) * 3);
			memcpy(ret.r[1].m, m[1], sizeof(f32) * 3);
			memcpy(ret.r[2].m, m[2], sizeof(f32) * 3);
			return ret;
		}
		operator Float3x3() const
		{
			return to_float3x3();
		}
	};

	struct alignas(16) Float4x4
	{
		lustruct("Float4x4", "{EE1F1000-29F9-4B91-953F-EE4D63BEDE9D}");

		Float4 r[4];

		Float4x4() = default;

		Float4x4(const Float4x4&) = default;
		Float4x4& operator=(const Float4x4&) = default;

		Float4x4(Float4x4&&) = default;
		Float4x4& operator=(Float4x4&&) = default;

		constexpr Float4x4(f32 m00, f32 m01, f32 m02, f32 m03,
			f32 m10, f32 m11, f32 m12, f32 m13,
			f32 m20, f32 m21, f32 m22, f32 m23,
			f32 m30, f32 m31, f32 m32, f32 m33)
			: r{{m00, m01, m02, m03},
				{m10, m11, m12, m13},
				{m20, m21, m22, m23},
				{m30, m31, m32, m33}} {}
		// Comparison operators
		bool operator == (const Float4x4& m) const;
		bool operator != (const Float4x4& m) const;

		// Assignment operators
		Float4x4(const Float4& row1, const Float4& row2, const Float4& row3, const Float4& row4);

		// Element Access.
		Float4 r1() const { return r[0]; }
		Float4 r2() const { return r[1]; }
		Float4 r3() const { return r[2]; }
		Float4 r4() const { return r[3]; }
		Float4 c1() const { return Float4(r[0].x, r[1].x, r[2].x, r[3].x); }
		Float4 c2() const { return Float4(r[0].y, r[1].y, r[2].y, r[3].y); }
		Float4 c3() const { return Float4(r[0].z, r[1].z, r[2].z, r[3].z); }
		Float4 c4() const { return Float4(r[0].w, r[1].w, r[2].w, r[3].w); }

		//! Extracts the unscaled rotation matrix from this affine matrix.
		Float4x4 rotation_matrix() const;

		//! Computes the euler angles from this rotation matrix. This method cannot be used for affine matrix directly,
		//! to use this method for affine matrix, call `rotation_matrix` to extract the rotation matrix from affine matrix
		//! first.
		//!
		//! The returned euler angles represents the radians of clockwise rotation along Z(roll), X(pitch), Y(yaw) axis in 
		//! that order.
		Float3 euler_angles() const;

		//! Computes the quaternion from this rotation matrix. This method cannot be used for affine matrix directly,
		//! to use this method for affine matrix, call `rotation_matrix` to extract the rotation matrix from affine matrix
		//! first.
		Quaternion quaternion() const;

		//! Returns the scale component if this matrix represents a rotation or an affine matrix.
		Float3 scale_factor() const;

		// Unary operators
		Float4x4 operator+() const { return *this; }
		Float4x4 operator- () const;

		// per-component operations.
		Float4x4& operator+=(const Float4x4&);
		Float4x4& operator-=(const Float4x4&);
		Float4x4& operator*=(const Float4x4&);
		Float4x4& operator/=(const Float4x4&);
		Float4x4& operator+=(f32 s);
		Float4x4& operator-=(f32 s);
		Float4x4& operator*=(f32 s);
		Float4x4& operator/=(f32 s);

		// Constants
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

	// Binary operators
	Float4x4 operator+(const Float4x4& m1, const Float4x4& m2);
	Float4x4 operator+(const Float4x4& m1, f32 s);
	Float4x4 operator+(f32 s, const Float4x4& m1);
	Float4x4 operator-(const Float4x4& m1, const Float4x4& m2);
	Float4x4 operator-(const Float4x4& m1, f32 s);
	Float4x4 operator-(f32 s, const Float4x4& m1);
	Float4x4 operator*(const Float4x4& m1, const Float4x4& m2);
	Float4x4 operator*(const Float4x4& m1, f32 s);
	Float4x4 operator*(f32 s, const Float4x4& m1);
	Float4x4 operator/(const Float4x4& m1, const Float4x4& m2);
	Float4x4 operator/(const Float4x4& m1, f32 s);
	Float4x4 operator/(f32 s, const Float4x4& m1);

	f32 determinant(const Float3x3& mat);
	f32 determinant(const Float4x4& mat);

	Float4 mul(const Float4& vec, const Float4x4& mat);
	Float4 mul(const Float4x4& mat, const Float4& vec);
	Float4x4 mul(const Float4x4& mat1, const Float4x4& mat2);

	Float3x3 transpose(const Float3x3& m);
	Float4x4 transpose(const Float4x4& m);

	Float3x3 inverse(const Float3x3& m, f32* out_determinant = nullptr);
	Float4x4 inverse(const Float4x4& m, f32* out_determinant = nullptr);

	struct Float4x3U
	{
		f32 m[4][3];
		Float4x3U() = default;
		Float4x3U(const Float4x3U&) = default;
		Float4x3U(Float4x3U&&) = default;
		Float4x3U& operator=(const Float4x3U&) = default;
		Float4x3U& operator=(Float4x3U&&) = default;
		Float4x3U(const Float4x4& rhs)
		{
			memcpy(m[0], rhs.r[0].m, sizeof(f32) * 3);
			memcpy(m[1], rhs.r[1].m, sizeof(f32) * 3);
			memcpy(m[2], rhs.r[2].m, sizeof(f32) * 3);
			memcpy(m[3], rhs.r[3].m, sizeof(f32) * 3);
		}
		Float4x3U& operator=(const Float4x4& rhs)
		{
			memcpy(m[0], rhs.r[0].m, sizeof(f32) * 3);
			memcpy(m[1], rhs.r[1].m, sizeof(f32) * 3);
			memcpy(m[2], rhs.r[2].m, sizeof(f32) * 3);
			memcpy(m[3], rhs.r[3].m, sizeof(f32) * 3);
			return *this;
		}
		Float4x4 to_float4x4(const Float4& column4 = Float4(0.0f, 0.0f, 0.0f, 1.0f)) const
		{
			Float4x4 ret;
			memcpy(ret.r[0].m, m[0], sizeof(f32) * 3);
			memcpy(ret.r[1].m, m[1], sizeof(f32) * 3);
			memcpy(ret.r[2].m, m[2], sizeof(f32) * 3);
			memcpy(ret.r[3].m, m[3], sizeof(f32) * 3);
			ret.r[0].w = column4.x;
			ret.r[1].w = column4.y;
			ret.r[2].w = column4.z;
			ret.r[3].w = column4.w;
			return ret;
		}
	};
	struct Float4x4U
	{
		f32 m[4][4];
		Float4x4U() = default;
		Float4x4U(const Float4x4U&) = default;
		Float4x4U(Float4x4U&&) = default;
		Float4x4U& operator=(const Float4x4U&) = default;
		Float4x4U& operator=(Float4x4U&&) = default;
		Float4x4U(const Float4x4& rhs)
		{
			memcpy(m[0], rhs.r[0].m, sizeof(f32) * 4);
			memcpy(m[1], rhs.r[1].m, sizeof(f32) * 4);
			memcpy(m[2], rhs.r[2].m, sizeof(f32) * 4);
			memcpy(m[3], rhs.r[3].m, sizeof(f32) * 4);
		}
		Float4x4U& operator=(const Float4x4& rhs)
		{
			memcpy(m[0], rhs.r[0].m, sizeof(f32) * 4);
			memcpy(m[1], rhs.r[1].m, sizeof(f32) * 4);
			memcpy(m[2], rhs.r[2].m, sizeof(f32) * 4);
			memcpy(m[3], rhs.r[3].m, sizeof(f32) * 4);
			return *this;
		}
		Float4x4 to_float4x4() const
		{
			Float4x4 ret;
			memcpy(ret.r[0].m, m[0], sizeof(f32) * 4);
			memcpy(ret.r[1].m, m[1], sizeof(f32) * 4);
			memcpy(ret.r[2].m, m[2], sizeof(f32) * 4);
			memcpy(ret.r[3].m, m[3], sizeof(f32) * 4);
			return ret;
		}
		operator Float4x4() const
		{
			return to_float4x4();
		}
	};

	LUNA_RUNTIME_API typeinfo_t float3x3_type();
	LUNA_RUNTIME_API typeinfo_t float4x4_type();
	template <> struct typeof_t<Float3x3> { typeinfo_t operator()() const { return float3x3_type(); } };
	template <> struct typeof_t<Float4x4> { typeinfo_t operator()() const { return float4x4_type(); } };
}

#include "Impl/Float3x3.inl"
#include "Impl/Float4x4.inl"
