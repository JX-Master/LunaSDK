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
	
	//! Used to represent one rotation operaiton using four @ref f32 values.
	struct alignas(Float4) Quaternion
	{
		lustruct("Quaternion", "{213A7986-C939-4D2F-BD3B-39DAF5D25DF3}");
		union
		{
			struct
			{
				//! The fist component of the quaternion.
				f32 x;
				//! The second component of the quaternion.
				f32 y;
				//! The third component of the quaternion.
				f32 z;
				//! The fourth component of the quaternion.
				f32 w;
			};
			//! The array of components.
			f32 m[4];
		};

		//! Constructs one quaternion with components uninitialized.
		Quaternion() = default;
		//! Constructs one quaternion by coping components from another quaternion.
		Quaternion(const Quaternion&) = default;
		//! Assigns one quaternion by coping components from another quaternion.
		Quaternion& operator=(const Quaternion&) = default;
		//! Constructs one quaternion by coping components from another quaternion.
		Quaternion(Quaternion&&) = default;
		//! Assigns one quaternion by coping components from another quaternion.
		Quaternion& operator=(Quaternion&&) = default;
		//! Constructs one quaternion from values.
		//! @param[in] x The value of the first component.
		//! @param[in] y The value of the second component.
		//! @param[in] z The value of the third component.
		//! @param[in] w The value of the fourth component.
		constexpr Quaternion(f32 x, f32 y, f32 z, f32 w) :
			x(x), y(y), z(z), w(w) {}
		//! Constructs one quaternion from one vector.
		//! @param[in] v The vector to copy components from.
		explicit Quaternion(const Float4& v) :
			x(v.x), y(v.y), z(v.z), w(v.w) {}
		//! Compares two quaternions for equality.
		//! @param[in] q The quaternion to compare with.
		//! @return Returns `true` if two quaternions are equal. Returns `false` otherwise.
		bool operator == (const Quaternion& q) const;
		//! Compares two quaternions for non-equality.
		//! @param[in] q The quaternion to compare with.
		//! @return Returns `true` if two quaternions are not equal. Returns `false` otherwise.
		bool operator != (const Quaternion& q) const;
		//! Assigns one quaternion by coping components from one vector.
		//! @param[in] v The vector to copy components from.
		//! @return Returns `*this`.
		Quaternion& operator= (const Float4& v) { x = v.x; y = v.y; z = v.z; w = v.w; return *this; }
		//! Adds this quaternion with one quaternion, and stores the result to this quaternion.
		//! @details This function performs the following operations:
		//! ```
		//! this->x += q.x;
		//! this->y += q.y;
		//! this->z += q.z;
		//! this->w += q.w;
		//! ```
		//! @param[in] q The quaternion to add.
		//! @return Returns `*this`.
		Quaternion& operator+= (const Quaternion& q);
		//! Subtracts this quaternion with one quaternion, and stores the result to this quaternion.
		//! @details This function performs the following operations:
		//! ```
		//! this->x -= q.x;
		//! this->y -= q.y;
		//! this->z -= q.z;
		//! this->w -= q.w;
		//! ```
		//! @param[in] q The quaternion to subtract.
		//! @return Returns `*this`.
		Quaternion& operator-= (const Quaternion& q);
		//! Concatenates two quaternions, and stores the result to this quaternion.
		//! @details This function performs the following operations:
		//! ```
		//! this->x = q.w * x + q.x * w + q.y * z - q.z * y;
		//! this->y = q.w * y - q.x * z + q.y * w + q.z * x;
		//! this->z = q.w * z + q.x * y - q.y * x + q.z * w;
		//! this->w = q.w * w - q.x * x - q.y * y - q.z * z;
		//! ```
		//! @param[in] q The quaternion to multiply.
		//! @return Returns `*this`.
		Quaternion& operator*= (const Quaternion& q);
		//! Multiplies this quaternion with one scalar, and stores the result to this quaternion.
		//! @details This function performs the following operations:
		//! ```
		//! this->x *= s;
		//! this->y *= s;
		//! this->z *= s;
		//! this->w *= s;
		//! ```
		//! @param[in] s The scalar to multiply.
		//! @return Returns `*this`.
		Quaternion& operator*= (f32 s);
		//! Concatenates one inversed quaternion of `q` to this quaternions, and stores the result to this quaternion.
		//! @details This function performs the following operations:
		//! ```
		//! Quaternion inv = inverse(q);
		//! *this = *this * inv;
		//! ```
		//! @param[in] q The quaternion to divide.
		//! @return Returns `*this`.
		Quaternion& operator/= (const Quaternion& q);
		//! Gets the quaternion as-is.
		//! @return Returns one copy of this quaternion.
		Quaternion operator+ () const { return *this; }
		//! Gets a negation of this quaternion.
		//! @return Returns a negation of this vector.
		Quaternion operator- () const;

		//! Creates one quaternion from rotation axis and rotation angle.
		//! @param[in] axis The rotation axis.
		//! @param[in] angle The rotation angle represented in radians.
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

	//! @}
}

#include "Impl/Quaternion.inl"