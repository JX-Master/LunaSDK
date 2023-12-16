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
	struct alignas(Float4) Quaternion
	{
		lustruct("Quaternion", "{213A7986-C939-4D2F-BD3B-39DAF5D25DF3}");
		union
		{
			struct
			{
				f32 x;
				f32 y;
				f32 z;
				f32 w;
			};
			f32 m[4];
		};

		Quaternion() = default;
		Quaternion(const Quaternion&) = default;
		Quaternion& operator=(const Quaternion&) = default;

		Quaternion(Quaternion&&) = default;
		Quaternion& operator=(Quaternion&&) = default;
		constexpr Quaternion(f32 _x, f32 _y, f32 _z, f32 _w) :
			x(_x), y(_y), z(_z), w(_w) {}
		Quaternion(const Float3& v, f32 scalar) :
			x(v.x), y(v.y), z(v.z), w(scalar) {}
		explicit Quaternion(const Float4& v) :
			x(v.x), y(v.y), z(v.z), w(v.w) {}

		// Comparison operators
		bool operator == (const Quaternion& q) const;
		bool operator != (const Quaternion& q) const;

		// Assignment operators
		Quaternion& operator= (const Float4& F) { x = F.x; y = F.y; z = F.z; w = F.w; return *this; }
		Quaternion& operator+= (const Quaternion& q);
		Quaternion& operator-= (const Quaternion& q);
		Quaternion& operator*= (const Quaternion& q);
		Quaternion& operator*= (f32 S);
		Quaternion& operator/= (const Quaternion& q);

		// Unary operators
		Quaternion operator+ () const { return *this; }
		Quaternion operator- () const;

		// Additional assignment.
		static Quaternion from_axis_angle(const Float3& axis, f32 angle);
		static Quaternion from_euler_angles(const Float3& euler_angles);
		static Quaternion from_euler_angles(f32 pitch, f32 yaw, f32 roll);

		// Constants
		static constexpr Quaternion identity() { return Quaternion(0.0f, 0.0f, 0.0f, 1.0f); }
	};

	// Binary operators
	Quaternion operator+ (const Quaternion& q1, const Quaternion& q2);
	Quaternion operator- (const Quaternion& q1, const Quaternion& q2);
	Quaternion operator* (const Quaternion& q1, const Quaternion& q2);
	Quaternion operator* (const Quaternion& q, f32 s);
	Quaternion operator/ (const Quaternion& q1, const Quaternion& q2);
	Quaternion operator* (f32 s, const Quaternion& q);

	f32 length(const Quaternion& q);
	f32 length_squared(const Quaternion& q);
	Quaternion normalize(const Quaternion& q);
	Quaternion conjugate(const Quaternion& q);
	Quaternion inverse(const Quaternion& q);
	f32 dot(const Quaternion& q1, const Quaternion& q2);
	Quaternion lerp(const Quaternion& q1, const Quaternion& q2, f32 t);
	//! Spherical interpolation.
	Quaternion slerp(const Quaternion& q1, const Quaternion& q2, f32 t);

	LUNA_RUNTIME_API typeinfo_t quaternion_type();
	template <> struct typeof_t<Quaternion> { typeinfo_t operator()() const { return quaternion_type(); } };
}

#include "Impl/Quaternion.inl"