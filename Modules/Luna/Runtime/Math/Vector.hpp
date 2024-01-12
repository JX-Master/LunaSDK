/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Vector.hpp
* @author JXMaster
* @date 2019/1/5
* @brief Luna Vector Math Library. SIMD Intrinsics are used when possible.
 */
#pragma once
#include "Math.hpp"
#include "../TypeInfo.hpp"

namespace Luna
{
	//! @addtogroup RuntimeMath
	//! @{
	//! @defgroup RuntimeMathVector Vectors
	//! @}

	//! @addtogroup RuntimeMathVector
	//! @{

	// Vectors with `U` suffixes like `Float2U`, `Float3U`, `Float4U`, `Int2U`, etc. are Unaligned vector versions  
	// used for storing vectors on files, or transfering into GPU or through network. They are not padded so that 
	// they get most reduced size. Unaligned vectors cannot be used for computation directly, they should be converted
	// to aligned versions firstly.
	//
	// Vectors without `U` suffixes like `Float2`, `Float3 and `Float4` are used for representing vectors during the 
	// runtime. They are aligned to 16 bytes with additional paddings (64 bits for `Float2`, 32 bits for `Float3`). 
	// Such vector types are used for calculations at run time, they use SIMD instructions (like SSE/AVX on x86, or Neon on ARM) 
	// whenever possible to accelerate computation.
	// 
	// Vectors in the `Simd` namespace like `Simd::float4`, `Simd::int4` are intended to map directly to CPU SIMD registers. 
	// When SIMD is enabled, data is loaded from vector types like `Float2`, `Float3 and `Float4` to SIMD registers, and stored
	// back to vector types from SIMD registers after computation. The number of SIMD registers are limited (usually 8, 16 or 32), 
	// so they are not used for storing data in memory and can only be used as function parameters, local variables and return types.

	struct Float2;
	struct Float3;
	struct Float4;

	//! 2D vector type with @ref f32 components.
	//! @details This vector type is 16-bytes aligned and will use SIMD to accelerate vector 
	//! calculations when possible.
	struct alignas(16) Float2
	{
		lustruct("Float2", "{69D3BC60-3EDA-49F5-B622-E832118FD3D2}");
		union
		{
			struct
			{
				//! The fist component of the vector.
				f32 x;
				//! The second component of the vector.
				f32 y;
			};
			//! The array of components.
			f32 m[2];
		};
		//! Constructs one vector with components uninitialized.
		Float2() = default;
		//! Constructs one vector by coping components from another vector.
		Float2(const Float2&) = default;
		//! Assigns one vector by coping components from another vector.
		Float2& operator=(const Float2&) = default;
		//! Constructs one vector by coping components from another vector.
		Float2(Float2&&) = default;
		//! Assigns one vector by coping components from another vector.
		Float2& operator=(Float2&&) = default;
		//! Constructs one vector from one scalar.
		//! @param[in] s The scalar to use. All components will be initialized to this value.
		constexpr explicit Float2(f32 s) :
			x(s), y(s) {}
		//! Constructs one vector from values.
		//! @param[in] x The value of the first component.
		//! @param[in] y The value of the second component.
		constexpr Float2(f32 x, f32 y) :
			x(x), y(y) {}
		//! Compares two vectors for equality.
		//! @param[in] v The vector to compare with.
		//! @return Returns `true` if two vectors are equal. Returns `false` otherwise.
		bool operator==(const Float2& v) const;
		//! Compares two vectors for non-equality.
		//! @param[in] v The vector to compare with.
		//! @return Returns `true` if two vectors are not equal. Returns `false` otherwise.
		bool operator!=(const Float2& v) const;
		//! Adds this vector with one vector, and stores the result to this vector.
		//! @details This function performs the following operations:
		//! ```
		//! this->x += v.x;
		//! this->y += v.y;
		//! ```
		//! @param[in] v The vector to add.
		//! @return Returns `*this`.
		Float2& operator+= (const Float2& v);
		//! Subtracts this vector with one vector, and stores the result to this vector.
		//! @details This function performs the following operations:
		//! ```
		//! this->x -= v.x;
		//! this->y -= v.y;
		//! ```
		//! @param[in] v The vector to subtract.
		//! @return Returns `*this`.
		Float2& operator-= (const Float2& v);
		//! Multiplies this vector with one vector, and stores the result to this vector.
		//! @details This function performs the following operations:
		//! ```
		//! this->x *= v.x;
		//! this->y *= v.y;
		//! ```
		//! @param[in] v The vector to multiply.
		//! @return Returns `*this`.
		Float2& operator*= (const Float2& v);
		//! Divides this vector with one vector, and stores the result to this vector.
		//! @details This function performs the following operations:
		//! ```
		//! this->x /= v.x;
		//! this->y /= v.y;
		//! ```
		//! @param[in] v The vector to divide.
		//! @return Returns `*this`.
		Float2& operator/= (const Float2& v);
		//! Adds this vector with one scalar, and stores the result to this vector.
		//! @details This function performs the following operations:
		//! ```
		//! this->x += s;
		//! this->y += s;
		//! ```
		//! @param[in] s The scalar to add.
		//! @return Returns `*this`.
		Float2& operator+= (f32 s);
		//! Subtracts this vector with one scalar, and stores the result to this vector.
		//! @details This function performs the following operations:
		//! ```
		//! this->x -= s;
		//! this->y -= s;
		//! ```
		//! @param[in] s The scalar to subtract.
		//! @return Returns `*this`.
		Float2& operator-= (f32 s);
		//! Multiplies this vector with one scalar, and stores the result to this vector.
		//! @details This function performs the following operations:
		//! ```
		//! this->x *= s;
		//! this->y *= s;
		//! ```
		//! @param[in] s The scalar to multiply.
		//! @return Returns `*this`.
		Float2& operator*= (f32 s);
		//! Divides this vector with one scalar, and stores the result to this vector.
		//! @details This function performs the following operations:
		//! ```
		//! this->x /= s;
		//! this->y /= s;
		//! ```
		//! @param[in] s The scalar to divide.
		//! @return Returns `*this`.
		Float2& operator/= (f32 s);
		//! Gets the vector as-is.
		//! @return Returns one copy of this vector.
		Float2 operator+ () const { return *this; }
		//! Gets a negation of this vector.
		//! @return Returns a negation of this vector.
		Float2 operator- () const { return Float2(-x, -y); }
		//! Creates one vector with value {0, 0}.
		//! @return Returns the created vector.
		static constexpr Float2 zero() { return Float2(0.0f, 0.0f); }
		//! Creates one vector with value {1, 1}.
		//! @return Returns the created vector.
		static constexpr Float2 one() { return Float2(1.0f, 1.0f); }
		//! Creates one vector with value {1, 0}.
		//! @return Returns the created vector.
		static constexpr Float2 unit_x() { return Float2(1.0f, 0.0f); }
		//! Creates one vector with value {0, 1}.
		//! @return Returns the created vector.
		static constexpr Float2 unit_y() { return Float2(0.0f, 1.0f); }
	};

	static_assert(sizeof(Float2) == sizeof(f32) * 4, "Incorrect Float2 size.");

	//! Adds two vectors.
	//! @details This function performs the following operations:
	//! ```
	//! Float2 r;
	//! r.x = v1.x + v2.x;
	//! r.y = v1.y + v2.y;
	//! return r;
	//! ```
	//! @param[in] v1 The first vector.
	//! @param[in] v2 The second vector.
	//! @return Returns the result vector.
	Float2 operator+ (const Float2& v1, const Float2& v2);
	//! Subtracts two vectors.
	//! @details This function performs the following operations:
	//! ```
	//! Float2 r;
	//! r.x = v1.x - v2.x;
	//! r.y = v1.y - v2.y;
	//! return r;
	//! ```
	//! @param[in] v1 The first vector.
	//! @param[in] v2 The second vector.
	//! @return Returns the result vector.
	Float2 operator- (const Float2& v1, const Float2& v2);
	//! Multiplies two vectors.
	//! @details This function performs the following operations:
	//! ```
	//! Float2 r;
	//! r.x = v1.x * v2.x;
	//! r.y = v1.y * v2.y;
	//! return r;
	//! ```
	//! @param[in] v1 The first vector.
	//! @param[in] v2 The second vector.
	//! @return Returns the result vector.
	Float2 operator* (const Float2& v1, const Float2& v2);
	//! Divides two vectors.
	//! @details This function performs the following operations:
	//! ```
	//! Float2 r;
	//! r.x = v1.x / v2.x;
	//! r.y = v1.y / v2.y;
	//! return r;
	//! ```
	//! @param[in] v1 The first vector.
	//! @param[in] v2 The second vector.
	//! @return Returns the result vector.
	Float2 operator/ (const Float2& v1, const Float2& v2);
	//! Adds one vector with one scalar.
	//! @details This function performs the following operations:
	//! ```
	//! Float2 r;
	//! r.x = v.x + s;
	//! r.y = v.y + s;
	//! return r;
	//! ```
	//! @param[in] v The vector.
	//! @param[in] s The scalar.
	//! @return Returns the result vector.
	Float2 operator+ (const Float2& v, f32 s);
	//! Adds one vector with one scalar.
	//! @details This function performs the following operations:
	//! ```
	//! Float2 r;
	//! r.x = v.x + s;
	//! r.y = v.y + s;
	//! return r;
	//! ```
	//! @param[in] s The scalar.
	//! @param[in] v The vector.
	//! @return Returns the result vector.
	Float2 operator+ (f32 s, const Float2& v);
	//! Subtracts one vector with one scalar.
	//! @details This function performs the following operations:
	//! ```
	//! Float2 r;
	//! r.x = v.x - s;
	//! r.y = v.y - s;
	//! return r;
	//! ```
	//! @param[in] v The vector.
	//! @param[in] s The scalar.
	//! @return Returns the result vector.
	Float2 operator- (const Float2& v, f32 s);
	//! Subtracts one scalar with one vector.
	//! @details This function performs the following operations:
	//! ```
	//! Float2 r;
	//! r.x = s - v.x;
	//! r.y = s - v.y;
	//! return r;
	//! ```
	//! @param[in] s The scalar.
	//! @param[in] v The vector.
	//! @return Returns the result vector.
	Float2 operator- (f32 s, const Float2& v1);
	//! Multiplies one vector with one scalar.
	//! @details This function performs the following operations:
	//! ```
	//! Float2 r;
	//! r.x = v.x * s;
	//! r.y = v.y * s;
	//! return r;
	//! ```
	//! @param[in] v The vector.
	//! @param[in] s The scalar.
	//! @return Returns the result vector.
	Float2 operator* (const Float2& v, f32 s);
	//! Multiplies one vector with one scalar.
	//! @details This function performs the following operations:
	//! ```
	//! Float2 r;
	//! r.x = v.x * s;
	//! r.y = v.y * s;
	//! return r;
	//! ```
	//! @param[in] s The scalar.
	//! @param[in] v The vector.
	//! @return Returns the result vector.
	Float2 operator* (f32 s, const Float2& v);
	//! Divides one vector with one scalar.
	//! @details This function performs the following operations:
	//! ```
	//! Float2 r;
	//! r.x = v.x / s;
	//! r.y = v.y / s;
	//! return r;
	//! ```
	//! @param[in] v The vector.
	//! @param[in] s The scalar.
	//! @return Returns the result vector.
	Float2 operator/ (const Float2& v, f32 s);
	//! Divides one scalar with one vector.
	//! @details This function performs the following operations:
	//! ```
	//! Float2 r;
	//! r.x = s / v.x;
	//! r.y = s / v.y;
	//! return r;
	//! ```
	//! @param[in] s The scalar.
	//! @param[in] v The vector.
	//! @return Returns the result vector.
	Float2 operator/ (f32 s, const Float2& v);

	//! Checks whether the point is in the specified boundary.
	//! @details This function performs the following operations:
	//! ```
	//! if (point.x >= min_bound.x && point.x <= max_bound.x 
	//! 	&& point.y >= min_bound.y && point.y <= max_bound.y)
	//! 	return true;
	//! else return false;
	//! ```
	//! @param[in] point The point to check.
	//! @param[in] min_bound The minimum boundary value.
	//! @param[in] max_bound The maximum boundary value.
	//! @return Returns `true` if the point is in the specified boundary.
	//! Returns `false` otherwise.
	bool in_bounds(const Float2& point, const Float2& min_bound, const Float2& max_bound);
	//! Computes the length of the vector.
	//! @details This function performs the following operations:
	//! ```
	//! return sqrt(vec.x * vec.x + vec.y * vec.y);
	//! ```
	//! @param[in] vec The vector.
	//! @return Returns the length of the vector.
	f32 length(const Float2& vec);
	//! Computes the squared length of the vector.
	//! @details This function performs the following operations:
	//! ```
	//! return vec.x * vec.x + vec.y * vec.y;
	//! ```
	//! @param[in] vec The vector.
	//! @return Returns the squared length of the vector.
	f32 length_squared(const Float2& vec);
	//! Computes the dot product of two vectors.
	//! @details This function performs the following operations:
	//! ```
	//! return v1.x * v2.x + v1.y * v2.y;
	//! ```
	//! @param[in] v1 The first vector.
	//! @param[in] v2 The second vector.
	//! @return Returns the dot product of two vectors.
	f32 dot(const Float2& v1, const Float2& v2);
	//! Computes the cross product of two vectors.
	//! @details This function performs the following operations:
	//! ```
	//! f32 cross = v1.x * v2.y - v1.y * v2.x;
	//! Float2 result;
	//! result.x = cross;
	//! result.y = cross;
	//! return result;
	//! ```
	//! @param[in] v1 The first vector.
	//! @param[in] v2 The second vector.
	//! @return Returns the cross product of two vectors.
	Float2 cross(const Float2& v1, const Float2& v2);
	//! Normalizes the vector so that the length of the vector is 1.
	//! @details This function performs the following operations:
	//! ```
	//! f32 len = length(vec);
    //! if (len > 0)
    //! {
    //!     len = 1.0f / len;
    //! }
	//! Float2 result;
	//! result.x = vec.x * len;
	//! result.y = vec.y * len;
	//! return result;
	//! ```
	//! @param[in] vec The vector to normalize.
	//! @return Returns the normalized vector.
	Float2 normalize(const Float2& vec);
	//! Clamps the vector to the specified range.
	//! @details This function performs the following operations:
	//! ```
	//! Float2 result;
	//! result.x = vec.x    > vec_min.x ? vec.x    : vec_min.x;
	//! result.x = result.x < vec_max.x ? result.x : vec_max.x;
	//! result.y = vec.y    > vec_min.y ? vec.y    : vec_min.y;
	//! result.y = result.y < vec_max.y ? result.y : vec_max.y;
	//! return result;
	//! ```
	//! @param[in] vec The vector to clamp.
	//! @param[in] vec_min The lower clamp value.
	//! @param[in] vec_max The upper clamp value.
	//! @return Returns the clampd vector.
	Float2 clamp(const Float2& vec, const Float2& vec_min, const Float2& vec_max);
	//! Computes the distance between two points.
	//! @details This function performs the following operations:
	//! ```
	//! f32 dx = v1.x - v2.x;
	//! f32 dy = v1.y - v2.y;
	//! return sqrt(dx * dx + dy * dy);
	//! ```
	//! @param[in] v1 The first point.
	//! @param[in] v2 The second point.
	//! @return Returns the distance between two points.
	f32 distance(const Float2& v1, const Float2& v2);
	//! Computes the squared distance between two points.
	//! @details This function performs the following operations:
	//! ```
	//! f32 dx = v1.x - v2.x;
	//! f32 dy = v1.y - v2.y;
	//! return dx * dx + dy * dy;
	//! ```
	//! @param[in] v1 The first point.
	//! @param[in] v2 The second point.
	//! @return Returns the squared distance between two points.
	f32 distance_squared(const Float2& v1, const Float2& v2);
	//! Computes the minimum value of two vectors.
	//! @details This function performs the following operations:
	//! ```
	//! Float2 result;
	//! result.x = v1.x < v2.x ? v1.x : v2.x;
	//! result.y = v1.y < v2.y ? v1.y : v2.y;
	//! return result;
	//! ```
	//! @param[in] v1 The first vector.
	//! @param[in] v2 The second vector.
	//! @return Returns the minimum value of two vectors.
	Float2 min(const Float2& v1, const Float2& v2);
	//! Computes the maximum value of two vectors.
	//! @details This function performs the following operations:
	//! ```
	//! Float2 result;
	//! result.x = v1.x > v2.x ? v1.x : v2.x;
	//! result.y = v1.y > v2.y ? v1.y : v2.y;
	//! return result;
	//! ```
	//! @param[in] v1 The first vector.
	//! @param[in] v2 The second vector.
	//! @return Returns the maximum value of two vectors.
	Float2 max(const Float2& v1, const Float2& v2);
	//! Performs linear interpolation between two vectors.
	//! @details This function performs the following operations:
	//! ```
	//! Float2 result;
	//! result.x = v1.x + t * (v2.x - v1.x);
	//! result.y = v1.y + t * (v2.y - v1.y);
	//! return result;
	//! ```
	//! @param[in] v1 The first vector.
	//! @param[in] v2 The second vector.
	//! @param[in] t The interpolation weight.
	//! @return Returns the interpolation result.
	Float2 lerp(const Float2& v1, const Float2& v2, f32 t);
	//! Performs smoothstep interpolation between two vectors.
	//! @details This function performs the following operations:
	//! ```
	//! Float2 result;
	//! t = clamp(t, 0, 1);
	//! t = t * t * (3 - 2 * t);
	//! result.x = v1.x + t * (v2.x - v1.x);
	//! result.y = v1.y + t * (v2.y - v1.y);
	//! return result;
	//! ```
	//! @param[in] v1 The first vector.
	//! @param[in] v2 The second vector.
	//! @param[in] t The interpolation weight.
	//! @return Returns the interpolation result.
	Float2 smoothstep(const Float2& v1, const Float2& v2, f32 t);
	//! Performs barycentric interpolation between three vectors.
	//! @details This function performs the following operations:
	//! ```
	//! Float2 result;
	//! result.x = v1.x + (v2.x - v1.x) * f + (v3.x - v1.x) * g;
	//! result.y = v1.y + (v2.y - v1.y) * f + (v3.y - v1.y) * g;
	//! return result;
	//! ```
	//! @param[in] v1 The first vector.
	//! @param[in] v2 The second vector.
	//! @param[in] v3 The third vector.
	//! @param[in] f The interpolation weight between v1 and v2.
	//! @param[in] g The interpolation weight between v1 and v3.
	//! @return Returns the interpolation result.
	Float2 barycentric(const Float2& v1, const Float2& v2, const Float2& v3, f32 f, f32 g);
	//! Performs centripetal Catmull–Rom spline interpolation.
	//! @details This function performs the following operations:
	//! ```
	//! f32 t2 = t * t;
	//! f32 t3 = t2 * t;
	//! return (v1 * (-t3 + 2 * t2 - t) +
	//!         v2 * (3 * t3 - 5 * t2 + 2) +
	//!         v3 * (-3 * t3 + 4 * t2 + t) +
	//!         v4 * (t3 - t2)) * 0.5;
	//! ```
	//! @param[in] v1 The first point of the curve.
	//! @param[in] v2 The second point of the curve.
	//! @param[in] v3 The third point of the curve.
	//! @param[in] v4 The fourth point of the curve.
	//! @param[in] t The interpolation weight.
	//! @return Returns the interpolation result.
	Float2 catmull_rom(const Float2& v1, const Float2& v2, const Float2& v3, const Float2& v4, f32 t);
	//! Performs Hermite spline interpolation.
	//! @details This function performs the following operations:
	//! ```
	//! Float2 result;
	//! f32 t2 = t * t;
	//! f32 t3 = t2 * t;
	//! return  v1 * (2 * t3 - 3 * t2 + 1) +
	//!         t1 * (t3 - 2 * t2 + t) +
	//!         v3 * (-2 * t3 + 3 * t2) +
	//!         t2 * (t3 - t2);
	//! ```
	//! @param[in] v1 The first point of the curve.
	//! @param[in] t1 The first tangent of the curve.
	//! @param[in] v2 The second point of the curve.
	//! @param[in] t2 The second tangent of the curve.
	//! @param[in] t The interpolation weight.
	//! @return Returns the interpolation result.
	Float2 hermite(const Float2& v1, const Float2& t1, const Float2& v2, const Float2& t2, f32 t);
	//! Computes the reflected vector of the input vector.
	//! @details This function performs the following operations:
	//! ```
	//! return ivec - (2 * dot(ivec, nvec) * nvec);
	//! ```
	//! @param[in] ivec The direction of the incident ray.
	//! @param[in] nvec The direction of the surface normal.
	//! @return Returns the direction of the reflected ray. The length of the reflected vector 
	//! is the same as the length of the `ivec`.
	//! @par Valid Usage
	//! * `nvec` must be a normalized vector.
	Float2 reflect(const Float2& ivec, const Float2& nvec);
	//! Computes the refracted vector of the input vector.
	//! @details This function performs the following operations:
	//! ```
	//! f32 proj = dot(ivec, nvec);
	//! f32 deter = 1.0f - refraction_index * refraction_index * (1.0f - proj * proj);
	//! if (deter >= 0.0f)
	//! {
	//! 	return ivec * refraction_index - nvec * (refraction_index * proj + sqrtf(deter));
	//! }
	//! return Float2(0.0f, 0.0f);
	//! ```
	//! @param[in] ivec The direction of the incident ray.
	//! @param[in] nvec The direction of the surface normal.
	//! @param[in] refraction_index The refraction index.
	//! @return Returns the direction of the refracted ray. The length of the refracted ray is normalized.
	//! Returns {0, 0} if the incident angle of `ivec` is too big that no refraction can occur 
	//! (full reflection).
	//! @par Valid Usage
	//! * Both `ivec` and `nvec` must be normalized vectors.
	//! * `refraction_index` must be a value greater than `0`.
	Float2 refract(const Float2& ivec, const Float2& nvec, f32 refraction_index);

	//! A generalized version of 2D vector. This vector type does not have specific alignment requirement.
	//! @tparam[in] _Ty The element type of the vector.
	template <typename _Ty>
	struct Vec2U
	{
		union
		{
			struct
			{
				//! The fist component of the vector.
				_Ty x;
				//! The second component of the vector.
				_Ty y;
			};
			//! The array of components.
			_Ty m[2];
		};
		//! Constructs one vector with components uninitialized.
		Vec2U() = default;
		//! Constructs one vector by coping components from another vector.
		Vec2U(const Vec2U&) = default;
		//! Assigns one vector by coping components from another vector.
		Vec2U& operator=(const Vec2U&) = default;
		//! Constructs one vector by coping components from another vector.
		Vec2U(Vec2U&&) = default;
		//! Assigns one vector by coping components from another vector.
		Vec2U& operator=(Vec2U&&) = default;
		//! Constructs one vector from one @ref Float2 vector.
		//! @param[in] rhs The vector to use.
		Vec2U(const Float2& rhs) :
			x((_Ty)rhs.x), y((_Ty)rhs.y) {}
		//! Assigns one vector from one @ref Float2 vector.
		//! @param[in] rhs The vector to use.
		//! @return Returns `*this`.
		Vec2U& operator=(const Float2& rhs)
		{
			x = (_Ty)rhs.x;
			y = (_Ty)rhs.y;
			return *this;
		}
		//! Converts this vector to @ref Float2.
		//! @return Returns the converted vector.
		operator Float2() const
		{
			return Float2((f32)x, (f32)y);
		}
		//! Constructs one vector from one scalar.
		//! @param[in] s The scalar to use. All components will be initialized to this value.
		constexpr explicit Vec2U(_Ty s) :
			x(s), y(s) {}
		//! Constructs one vector from values.
		//! @param[in] x The value of the first component.
		//! @param[in] y The value of the second component.
		constexpr Vec2U(_Ty x, _Ty y) :
			x(x), y(y) {}
		//! Compares two vectors for equality.
		//! @param[in] rhs The vector to compare with.
		//! @return Returns `true` if two vectors are equal. Returns `false` otherwise.
		bool operator==(const Vec2U& rhs) const
		{
			return (x == rhs.x) && (y == rhs.y);
		}
		//! Compares two vectors for non-equality.
		//! @param[in] rhs The vector to compare with.
		//! @return Returns `true` if two vectors are not equal. Returns `false` otherwise.
		bool operator!=(const Vec2U& rhs) const
		{
			return !(*this == rhs);
		}
	};

	//! Unaligned 2D floating-point vector type.
	using Float2U = Vec2U<f32>;
	//! Unaligned 2D signed integer vector type.
	using Int2U = Vec2U<i32>;
	//! Unaligned 2D unsigned integer vector type.
	using UInt2U = Vec2U<u32>;

	//! 3D vector type with @ref f32 components.
	//! @details This vector type is 16-bytes aligned and will use SIMD to accelerate vector 
	//! calculations when possible.
	struct alignas(16) Float3
	{
		lustruct("Float3", "{7727472C-AF79-40E8-8385-CD7677389E4F}");
		union
		{
			struct
			{
				//! The fist component of the vector.
				f32 x;
				//! The second component of the vector.
				f32 y;
				//! The third component of the vector.
				f32 z;
			};
			//! The array of components.
			f32 m[3];
		};
		//! Constructs one vector with components uninitialized.
		Float3() = default;
		//! Constructs one vector by coping components from another vector.
		Float3(const Float3&) = default;
		//! Assigns one vector by coping components from another vector.
		Float3& operator=(const Float3&) = default;
		//! Constructs one vector by coping components from another vector.
		Float3(Float3&&) = default;
		//! Assigns one vector by coping components from another vector.
		Float3& operator=(Float3&&) = default;
		//! Constructs one vector from one scalar.
		//! @param[in] s The scalar to use. All components will be initialized to this value.
		constexpr explicit Float3(f32 s) :
			x(s), y(s), z(s) {}
		//! Constructs one vector from values.
		//! @param[in] x The value of the first component.
		//! @param[in] y The value of the second component.
		//! @param[in] z The value of the third component.
		constexpr Float3(f32 x, f32 y, f32 z) :
			x(x), y(y), z(z) {}
		//! Compares two vectors for equality.
		//! @param[in] v The vector to compare with.
		//! @return Returns `true` if two vectors are equal. Returns `false` otherwise.
		bool operator==(const Float3& v) const;
		//! Compares two vectors for non-equality.
		//! @param[in] v The vector to compare with.
		//! @return Returns `true` if two vectors are not equal. Returns `false` otherwise.
		bool operator!=(const Float3& v) const;
		//! Adds this vector with one vector, and stores the result to this vector.
		//! @details This function performs the following operations:
		//! ```
		//! this->x += v.x;
		//! this->y += v.y;
		//! this->z += v.z;
		//! ```
		//! @param[in] v The vector to add.
		//! @return Returns `*this`.
		Float3& operator+= (const Float3& v);
		//! Subtracts this vector with one vector, and stores the result to this vector.
		//! @details This function performs the following operations:
		//! ```
		//! this->x -= v.x;
		//! this->y -= v.y;
		//! this->z -= v.z;
		//! ```
		//! @param[in] v The vector to subtract.
		//! @return Returns `*this`.
		Float3& operator-= (const Float3& v);
		//! Multiplies this vector with one vector, and stores the result to this vector.
		//! @details This function performs the following operations:
		//! ```
		//! this->x *= v.x;
		//! this->y *= v.y;
		//! this->z *= v.z;
		//! ```
		//! @param[in] v The vector to multiply.
		//! @return Returns `*this`.
		Float3& operator*= (const Float3& v);
		//! Divides this vector with one vector, and stores the result to this vector.
		//! @details This function performs the following operations:
		//! ```
		//! this->x /= v.x;
		//! this->y /= v.y;
		//! this->z /= v.z;
		//! ```
		//! @param[in] v The vector to divide.
		//! @return Returns `*this`.
		Float3& operator/= (const Float3& v);
		//! Adds this vector with one scalar, and stores the result to this vector.
		//! @details This function performs the following operations:
		//! ```
		//! this->x += s;
		//! this->y += s;
		//! this->z += s;
		//! ```
		//! @param[in] s The scalar to add.
		//! @return Returns `*this`.
		Float3& operator+= (f32 s);
		//! Subtracts this vector with one scalar, and stores the result to this vector.
		//! @details This function performs the following operations:
		//! ```
		//! this->x -= s;
		//! this->y -= s;
		//! this->z -= s;
		//! ```
		//! @param[in] s The scalar to subtract.
		//! @return Returns `*this`.
		Float3& operator-= (f32 s);
		//! Multiplies this vector with one scalar, and stores the result to this vector.
		//! @details This function performs the following operations:
		//! ```
		//! this->x *= s;
		//! this->y *= s;
		//! this->z *= s;
		//! ```
		//! @param[in] s The scalar to multiply.
		//! @return Returns `*this`.
		Float3& operator*= (f32 s);
		//! Divides this vector with one scalar, and stores the result to this vector.
		//! @details This function performs the following operations:
		//! ```
		//! this->x /= s;
		//! this->y /= s;
		//! this->z /= s;
		//! ```
		//! @param[in] s The scalar to divide.
		//! @return Returns `*this`.
		Float3& operator/= (f32 s);
		//! Gets the vector as-is.
		//! @return Returns one copy of this vector.
		Float3 operator+ () const { return *this; }
		//! Gets a negation of this vector.
		//! @return Returns a negation of this vector.
		Float3 operator- () const;
		//! Gets the first two components of this vector.
		//! @return Returns one 2D vector that contains the first two components of this vector.
		Float2 xy() const { return Float2(x, y); }

		//! Creates one vector with value {0, 0, 0}.
		//! @return Returns the created vector.
		static constexpr Float3 zero() { return Float3(0.0f, 0.0f, 0.0f); }
		//! Creates one vector with value {1, 1, 1}.
		//! @return Returns the created vector.
		static constexpr Float3 one() { return Float3(1.0f, 1.0f, 1.0f); }
		//! Creates one vector with value {1, 0, 0}.
		//! @return Returns the created vector.
		static constexpr Float3 unit_x() { return Float3(1.0f, 0.0f, 0.0f); }
		//! Creates one vector with value {0, 1, 0}.
		//! @return Returns the created vector.
		static constexpr Float3 unit_y() { return Float3(0.0f, 1.0f, 0.0f); }
		//! Creates one vector with value {0, 0, 1}.
		//! @return Returns the created vector.
		static constexpr Float3 unit_z() { return Float3(0.0f, 0.0f, 1.0f); }
		//! Creates one vector with value {0, 1, 0}.
		//! @return Returns the created vector.
		static constexpr Float3 up() { return Float3(0.0f, 1.0f, 0.0f); }
		//! Creates one vector with value {0, -1, 0}.
		//! @return Returns the created vector.
		static constexpr Float3 down() { return Float3(0.0f, -1.0f, 0.0f); }
		//! Creates one vector with value {1, 0, 0}.
		//! @return Returns the created vector.
		static constexpr Float3 right() { return Float3(1.0f, 0.0f, 0.0f); }
		//! Creates one vector with value {-1, 0, 0}.
		//! @return Returns the created vector.
		static constexpr Float3 left() { return Float3(-1.0f, 0.0f, 0.0f); }
		//! Creates one vector with value {0, 0, -1}.
		//! @return Returns the created vector.
		static constexpr Float3 forward() { return Float3(0.0f, 0.0f, -1.0f); }
		//! Creates one vector with value {0, 0, 1}.
		//! @return Returns the created vector.
		static constexpr Float3 backward() { return Float3(0.0f, 0.0f, 1.0f); }
	};

	static_assert(sizeof(Float3) == sizeof(f32) * 4, "Incorrect Float3 size.");

	//! Adds two vectors.
	//! @details This function performs the following operations:
	//! ```
	//! Float3 r;
	//! r.x = v1.x + v2.x;
	//! r.y = v1.y + v2.y;
	//! r.z = v1.z + v2.z;
	//! return r;
	//! ```
	//! @param[in] v1 The first vector.
	//! @param[in] v2 The second vector.
	//! @return Returns the result vector.
	Float3 operator+ (const Float3& v1, const Float3& v2);
	//! Subtracts two vectors.
	//! @details This function performs the following operations:
	//! ```
	//! Float3 r;
	//! r.x = v1.x - v2.x;
	//! r.y = v1.y - v2.y;
	//! r.z = v1.z - v2.z;
	//! return r;
	//! ```
	//! @param[in] v1 The first vector.
	//! @param[in] v2 The second vector.
	//! @return Returns the result vector.
	Float3 operator- (const Float3& v1, const Float3& v2);
	//! Multiplies two vectors.
	//! @details This function performs the following operations:
	//! ```
	//! Float3 r;
	//! r.x = v1.x * v2.x;
	//! r.y = v1.y * v2.y;
	//! r.z = v1.z * v2.z;
	//! return r;
	//! ```
	//! @param[in] v1 The first vector.
	//! @param[in] v2 The second vector.
	//! @return Returns the result vector.
	Float3 operator* (const Float3& v1, const Float3& v2);
	//! Divides two vectors.
	//! @details This function performs the following operations:
	//! ```
	//! Float3 r;
	//! r.x = v1.x / v2.x;
	//! r.y = v1.y / v2.y;
	//! r.z = v1.z / v2.z;
	//! return r;
	//! ```
	//! @param[in] v1 The first vector.
	//! @param[in] v2 The second vector.
	//! @return Returns the result vector.
	Float3 operator/ (const Float3& v1, const Float3& v2);
	//! Adds one vector with one scalar.
	//! @details This function performs the following operations:
	//! ```
	//! Float3 r;
	//! r.x = v.x + s;
	//! r.y = v.y + s;
	//! r.z = v.z + s;
	//! return r;
	//! ```
	//! @param[in] v The vector.
	//! @param[in] s The scalar.
	//! @return Returns the result vector.
	Float3 operator+ (const Float3& v, f32 s);
	//! Adds one vector with one scalar.
	//! @details This function performs the following operations:
	//! ```
	//! Float3 r;
	//! r.x = v.x + s;
	//! r.y = v.y + s;
	//! r.z = v.z + s;
	//! return r;
	//! ```
	//! @param[in] v The vector.
	//! @param[in] s The scalar.
	//! @return Returns the result vector.
	Float3 operator+ (f32 s, const Float3& v);
	//! Subtracts one vector with one scalar.
	//! @details This function performs the following operations:
	//! ```
	//! Float3 r;
	//! r.x = v.x - s;
	//! r.y = v.y - s;
	//! r.z = v.z - s;
	//! return r;
	//! ```
	//! @param[in] v The vector.
	//! @param[in] s The scalar.
	//! @return Returns the result vector.
	Float3 operator- (const Float3& v, f32 s);
	//! Subtracts one scalar with one vector.
	//! @details This function performs the following operations:
	//! ```
	//! Float3 r;
	//! r.x = s - v.x;
	//! r.y = s - v.y;
	//! r.z = s - v.z;
	//! return r;
	//! ```
	//! @param[in] s The scalar.
	//! @param[in] v The vector.
	//! @return Returns the result vector.
	Float3 operator- (f32 s, const Float3& v);
	//! Multiplies one vector with one scalar.
	//! @details This function performs the following operations:
	//! ```
	//! Float3 r;
	//! r.x = v.x * s;
	//! r.y = v.y * s;
	//! r.z = v.z * s;
	//! return r;
	//! ```
	//! @param[in] v The vector.
	//! @param[in] s The scalar.
	//! @return Returns the result vector.
	Float3 operator* (const Float3& v, f32 s);
	//! Multiplies one vector with one scalar.
	//! @details This function performs the following operations:
	//! ```
	//! Float3 r;
	//! r.x = v.x * s;
	//! r.y = v.y * s;
	//! r.z = v.z * s;
	//! return r;
	//! ```
	//! @param[in] s The scalar.
	//! @param[in] v The vector.
	//! @return Returns the result vector.
	Float3 operator* (f32 s, const Float3& v);
	//! Divides one vector with one scalar.
	//! @details This function performs the following operations:
	//! ```
	//! Float3 r;
	//! r.x = v.x / s;
	//! r.y = v.y / s;
	//! r.z = v.z / s;
	//! return r;
	//! ```
	//! @param[in] v The vector.
	//! @param[in] s The scalar.
	//! @return Returns the result vector.
	Float3 operator/ (const Float3& v, f32 s);
	//! Divides one scalar with one vector.
	//! @details This function performs the following operations:
	//! ```
	//! Float3 r;
	//! r.x = s / v.x;
	//! r.y = s / v.y;
	//! r.z = s / v.z;
	//! return r;
	//! ```
	//! @param[in] s The scalar.
	//! @param[in] v The vector.
	//! @return Returns the result vector.
	Float3 operator/ (f32 s, const Float3& v);

	//! Checks whether the point is in the specified boundary.
	//! @details This function performs the following operations:
	//! ```
	//! if (point.x >= min_bound.x && point.x <= max_bound.x 
	//! 	&& point.y >= min_bound.y && point.y <= max_bound.y
	//!		&& point.z >= min_bound.z && point.z <= max_bound.z)
	//! 	return true;
	//! else return false;
	//! ```
	//! @param[in] point The point to check.
	//! @param[in] min_bound The minimum boundary value.
	//! @param[in] max_bound The maximum boundary value.
	//! @return Returns `true` if the point is in the specified boundary.
	//! Returns `false` otherwise.
	bool in_bounds(const Float3& point, const Float3& min_bound, const Float3& max_bound);
	//! Computes the length of the vector.
	//! @details This function performs the following operations:
	//! ```
	//! return sqrt(vec.x * vec.x + vec.y * vec.y + vec.z * vec.z);
	//! ```
	//! @param[in] vec The vector.
	//! @return Returns the length of the vector.
	f32 length(const Float3& vec);
	//! Computes the squared length of the vector.
	//! @details This function performs the following operations:
	//! ```
	//! return vec.x * vec.x + vec.y * vec.y + vec.z * vec.z;
	//! ```
	//! @param[in] vec The vector.
	//! @return Returns the squared length of the vector.
	f32 length_squared(const Float3& vec);
	//! Computes the dot product of two vectors.
	//! @details This function performs the following operations:
	//! ```
	//! return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
	//! ```
	//! @param[in] v1 The first vector.
	//! @param[in] v2 The second vector.
	//! @return Returns the dot product of two vectors.
	f32 dot(const Float3& v1, const Float3& v2);
	//! Computes the cross product of two vectors.
	//! @details This function performs the following operations:
	//! ```
	//! Float3 result;
	//! result.x = v1.y * v2.z - v1.z * v2.y;
	//! result.y = v1.z * v2.x - v1.x * v2.z;
	//! result.z = v1.x * v2.y - v1.y * v2.x;
	//! return result;
	//! ```
	//! @param[in] v1 The first vector.
	//! @param[in] v2 The second vector.
	//! @return Returns the cross product of two vectors.
	Float3 cross(const Float3& v1, const Float3& v2);
	//! Normalizes the vector so that the length of the vector is 1.
	//! @details This function performs the following operations:
	//! ```
	//! f32 len = length(vec);
    //! if (len > 0)
    //! {
    //!     len = 1.0f / len;
    //! }
	//! Float3 result;
	//! result.x = vec.x * len;
	//! result.y = vec.y * len;
	//! result.z = vec.z * len;
	//! return result;
	//! ```
	//! @param[in] vec The vector to normalize.
	//! @return Returns the normalized vector.
	Float3 normalize(const Float3& vec);
	//! Clamps the vector to the specified range.
	//! @details This function performs the following operations:
	//! ```
	//! Float3 result;
	//! result.x = vec.x    > vec_min.x ? vec.x    : vec_min.x;
	//! result.x = result.x < vec_max.x ? result.x : vec_max.x;
	//! result.y = vec.y    > vec_min.y ? vec.y    : vec_min.y;
	//! result.y = result.y < vec_max.y ? result.y : vec_max.y;
	//! result.z = vec.z    > vec_min.z ? vec.z    : vec_min.z;
	//! result.z = result.z < vec_max.z ? result.z : vec_max.z;
	//! return result;
	//! ```
	//! @param[in] vec The vector to clamp.
	//! @param[in] vec_min The lower clamp value.
	//! @param[in] vec_max The upper clamp value.
	//! @return Returns the clampd vector.
	Float3 clamp(const Float3& vec, const Float3& vec_min, const Float3& vec_max);
	//! Computes the distance between two points.
	//! @details This function performs the following operations:
	//! ```
	//! f32 dx = v1.x - v2.x;
	//! f32 dy = v1.y - v2.y;
	//! f32 dz = v1.z - v2.z;
	//! return sqrt(dx * dx + dy * dy + dz * dz);
	//! ```
	//! @param[in] v1 The first point.
	//! @param[in] v2 The second point.
	//! @return Returns the distance between two points.
	f32 distance(const Float3& v1, const Float3& v2);
	//! Computes the squared distance between two points.
	//! @details This function performs the following operations:
	//! ```
	//! f32 dx = v1.x - v2.x;
	//! f32 dy = v1.y - v2.y;
	//! f32 dz = v1.z - v2.z;
	//! return dx * dx + dy * dy + dz * dz;
	//! ```
	//! @param[in] v1 The first point.
	//! @param[in] v2 The second point.
	//! @return Returns the squared distance between two points.
	f32 distance_squared(const Float3& v1, const Float3& v2);
	//! Computes the minimum value of two vectors.
	//! @details This function performs the following operations:
	//! ```
	//! Float3 result;
	//! result.x = v1.x < v2.x ? v1.x : v2.x;
	//! result.y = v1.y < v2.y ? v1.y : v2.y;
	//! result.z = v1.z < v2.z ? v1.z : v2.z;
	//! return result;
	//! ```
	//! @param[in] v1 The first vector.
	//! @param[in] v2 The second vector.
	//! @return Returns the minimum value of two vectors.
	Float3 min(const Float3& v1, const Float3& v2);
	//! Computes the maximum value of two vectors.
	//! @details This function performs the following operations:
	//! ```
	//! Float3 result;
	//! result.x = v1.x > v2.x ? v1.x : v2.x;
	//! result.y = v1.y > v2.y ? v1.y : v2.y;
	//! result.z = v1.z > v2.z ? v1.z : v2.z;
	//! return result;
	//! ```
	//! @param[in] v1 The first vector.
	//! @param[in] v2 The second vector.
	//! @return Returns the maximum value of two vectors.
	Float3 max(const Float3& v1, const Float3& v2);
	//! Performs linear interpolation between two vectors.
	//! @details This function performs the following operations:
	//! ```
	//! Float3 result;
	//! result.x = v1.x + t * (v2.x - v1.x);
	//! result.y = v1.y + t * (v2.y - v1.y);
	//! result.z = v1.z + t * (v2.z - v1.z);
	//! return result;
	//! ```
	//! @param[in] v1 The first vector.
	//! @param[in] v2 The second vector.
	//! @param[in] t The interpolation weight.
	//! @return Returns the interpolation result.
	Float3 lerp(const Float3& v1, const Float3& v2, f32 t);
	//! Performs smoothstep interpolation between two vectors.
	//! @details This function performs the following operations:
	//! ```
	//! Float3 result;
	//! t = clamp(t, 0, 1);
	//! t = t * t * (3 - 2 * t);
	//! result.x = v1.x + t * (v2.x - v1.x);
	//! result.y = v1.y + t * (v2.y - v1.y);
	//! result.z = v1.z + t * (v2.z - v1.z);
	//! return result;
	//! ```
	//! @param[in] v1 The first vector.
	//! @param[in] v2 The second vector.
	//! @param[in] t The interpolation weight.
	//! @return Returns the interpolation result.
	Float3 smoothstep(const Float3& v1, const Float3& v2, f32 t);
	//! Performs barycentric interpolation between three vectors.
	//! @details This function performs the following operations:
	//! ```
	//! Float3 result;
	//! result.x = v1.x + (v2.x - v1.x) * f + (v3.x - v1.x) * g;
	//! result.y = v1.y + (v2.y - v1.y) * f + (v3.y - v1.y) * g;
	//! result.z = v1.z + (v2.z - v1.z) * f + (v3.z - v1.z) * g;
	//! return result;
	//! ```
	//! @param[in] v1 The first vector.
	//! @param[in] v2 The second vector.
	//! @param[in] v3 The third vector.
	//! @param[in] f The interpolation weight between v1 and v2.
	//! @param[in] g The interpolation weight between v1 and v3.
	//! @return Returns the interpolation result.
	Float3 barycentric(const Float3& v1, const Float3& v2, const Float3& vec3, f32 f, f32 g);
	//! Performs centripetal Catmull–Rom spline interpolation.
	//! @details This function performs the following operations:
	//! ```
	//! f32 t2 = t * t;
	//! f32 t3 = t2 * t;
	//! return (v1 * (-t3 + 2 * t2 - t) +
	//!         v2 * (3 * t3 - 5 * t2 + 2) +
	//!         v3 * (-3 * t3 + 4 * t2 + t) +
	//!         v4 * (t3 - t2)) * 0.5;
	//! ```
	//! @param[in] v1 The first point of the curve.
	//! @param[in] v2 The second point of the curve.
	//! @param[in] v3 The third point of the curve.
	//! @param[in] v4 The fourth point of the curve.
	//! @param[in] t The interpolation weight.
	//! @return Returns the interpolation result.
	Float3 catmull_rom(const Float3& v1, const Float3& v2, const Float3& vec3, const Float3& vec4, f32 t);
	//! Performs Hermite spline interpolation.
	//! @details This function performs the following operations:
	//! ```
	//! Float3 result;
	//! f32 t2 = t * t;
	//! f32 t3 = t2 * t;
	//! return  v1 * (2 * t3 - 3 * t2 + 1) +
	//!         t1 * (t3 - 2 * t2 + t) +
	//!         v3 * (-2 * t3 + 3 * t2) +
	//!         t2 * (t3 - t2);
	//! ```
	//! @param[in] v1 The first point of the curve.
	//! @param[in] t1 The first tangent of the curve.
	//! @param[in] v2 The second point of the curve.
	//! @param[in] t2 The second tangent of the curve.
	//! @param[in] t The interpolation weight.
	//! @return Returns the interpolation result.
	Float3 hermite(const Float3& v1, const Float3& t1, const Float3& v2, const Float3& t2, f32 t);
	//! Computes the reflected vector of the input vector.
	//! @details This function performs the following operations:
	//! ```
	//! return ivec - (2 * dot(ivec, nvec) * nvec);
	//! ```
	//! @param[in] ivec The direction of the incident ray.
	//! @param[in] nvec The direction of the surface normal.
	//! @return Returns the direction of the reflected ray. The length of the reflected vector 
	//! is the same as the length of the `ivec`.
	//! @par Valid Usage
	//! * `nvec` must be a normalized vector.
	Float3 reflect(const Float3& ivec, const Float3& nvec);
	//! Computes the refracted vector of the input vector.
	//! @details This function performs the following operations:
	//! ```
	//! f32 proj = dot(ivec, nvec);
	//! f32 deter = 1.0f - refraction_index * refraction_index * (1.0f - proj * proj);
	//! if (deter >= 0.0f)
	//! {
	//! 	return ivec * refraction_index - nvec * (refraction_index * proj + sqrtf(deter));
	//! }
	//! return Float3(0.0f);
	//! ```
	//! @param[in] ivec The direction of the incident ray.
	//! @param[in] nvec The direction of the surface normal.
	//! @param[in] refraction_index The refraction index.
	//! @return Returns the direction of the refracted ray. The length of the refracted ray is normalized.
	//! Returns {0, 0} if the incident angle of `ivec` is too big that no refraction can occur 
	//! (full reflection).
	//! @par Valid Usage
	//! * Both `ivec` and `nvec` must be normalized vectors.
	//! * `refraction_index` must be a value greater than `0`.
	Float3 refract(const Float3& ivec, const Float3& nvec, f32 refraction_index);

	//! A generalized version of 3D vector. This vector type does not have specific alignment requirement.
	//! @tparam[in] _Ty The element type of the vector.
	template <typename _Ty>
	struct Vec3U
	{
		union
		{
			struct
			{
				//! The fist component of the vector.
				_Ty x;
				//! The second component of the vector.
				_Ty y;
				//! The third component of the vector.
				_Ty z;
			};
			//! The array of components.
			_Ty m[3];
		};
		//! Constructs one vector with components uninitialized.
		Vec3U() = default;
		//! Constructs one vector by coping components from another vector.
		Vec3U(const Vec3U&) = default;
		//! Assigns one vector by coping components from another vector.
		Vec3U& operator=(const Vec3U&) = default;
		//! Constructs one vector by coping components from another vector.
		Vec3U(Vec3U&&) = default;
		//! Assigns one vector by coping components from another vector.
		Vec3U& operator=(Vec3U&&) = default;
		//! Constructs one vector from one @ref Float3 vector.
		//! @param[in] rhs The vector to use.
		Vec3U(const Float3& rhs) :
			x((_Ty)rhs.x), y((_Ty)rhs.y), z((_Ty)rhs.z) {}
		//! Assigns one vector from one @ref Float3 vector.
		//! @param[in] rhs The vector to use.
		//! @return Returns `*this`.
		Vec3U& operator=(const Float3& rhs)
		{
			x = (_Ty)rhs.x;
			y = (_Ty)rhs.y;
			z = (_Ty)rhs.z;
			return *this;
		}
		//! Converts this vector to @ref Float3.
		//! @return Returns the converted vector.
		operator Float3() const
		{
			return Float3((f32)x, (f32)y, (f32)z);
		}
		//! Constructs one vector from one scalar.
		//! @param[in] s The scalar to use. All components will be initialized to this value.
		constexpr explicit Vec3U(_Ty s) :
			x(s), y(s), z(s) {}
		//! Constructs one vector from values.
		//! @param[in] x The value of the first component.
		//! @param[in] y The value of the second component.
		//! @param[in] y The value of the third component.
		constexpr Vec3U(_Ty x, _Ty y, _Ty z) :
			x(x), y(y), z(z) {}
		//! Compares two vectors for equality.
		//! @param[in] rhs The vector to compare with.
		//! @return Returns `true` if two vectors are equal. Returns `false` otherwise.
		bool operator==(const Vec3U& rhs) const
		{
			return (x == rhs.x) && (y == rhs.y) && (z == rhs.z);
		}
		//! Compares two vectors for non-equality.
		//! @param[in] rhs The vector to compare with.
		//! @return Returns `true` if two vectors are not equal. Returns `false` otherwise.
		bool operator!=(const Vec3U& rhs) const
		{
			return !(*this == rhs);
		}
	};

	//! Unaligned 3D floating-point vector type.
	using Float3U = Vec3U<f32>;
	//! Unaligned 3D signed integer vector type.
	using Int3U = Vec3U<i32>;
	//! Unaligned 3D unsigned integer vector type.
	using UInt3U = Vec3U<u32>;

	//! 4D vector type with @ref f32 components.
	//! @details This vector type is 16-bytes aligned and will use SIMD to accelerate vector 
	//! calculations when possible.
	struct alignas(16) Float4
	{
		lustruct("Float4", "{88547D46-4DF1-42ED-BB48-96571BBD651F}");
		union
		{
			struct
			{
				//! The fist component of the vector.
				f32 x;
				//! The second component of the vector.
				f32 y;
				//! The third component of the vector.
				f32 z;
				//! The fourth component of the vector.
				f32 w;
			};
			//! The array of components.
			f32 m[4];
		};

		//! Constructs one vector with components uninitialized.
		Float4() = default;
		//! Constructs one vector by coping components from another vector.
		Float4(const Float4&) = default;
		//! Assigns one vector by coping components from another vector.
		Float4& operator=(const Float4&) = default;
		//! Constructs one vector by coping components from another vector.
		Float4(Float4&&) = default;
		//! Assigns one vector by coping components from another vector.
		Float4& operator=(Float4&&) = default;
		//! Constructs one vector from one scalar.
		//! @param[in] s The scalar to use. All components will be initialized to this value.
		constexpr explicit Float4(f32 s) :
			x(s), y(s), z(s), w(s) {}
		//! Constructs one vector from values.
		//! @param[in] x The value of the first component.
		//! @param[in] y The value of the second component.
		//! @param[in] z The value of the third component.
		//! @param[in] w The value of the fourth component.
		constexpr Float4(f32 x, f32 y, f32 z, f32 w) :
			x(x), y(y), z(z), w(w) {}
		//! Compares two vectors for equality.
		//! @param[in] v The vector to compare with.
		//! @return Returns `true` if two vectors are equal. Returns `false` otherwise.
		bool operator==(const Float4& v) const;
		//! Compares two vectors for non-equality.
		//! @param[in] v The vector to compare with.
		//! @return Returns `true` if two vectors are not equal. Returns `false` otherwise.
		bool operator!=(const Float4& v) const;
		//! Adds this vector with one vector, and stores the result to this vector.
		//! @details This function performs the following operations:
		//! ```
		//! this->x += v.x;
		//! this->y += v.y;
		//! this->z += v.z;
		//! this->w += v.w;
		//! ```
		//! @param[in] v The vector to add.
		//! @return Returns `*this`.
		Float4& operator+= (const Float4& v);
		//! Subtracts this vector with one vector, and stores the result to this vector.
		//! @details This function performs the following operations:
		//! ```
		//! this->x -= v.x;
		//! this->y -= v.y;
		//! this->z -= v.z;
		//! this->w -= v.w;
		//! ```
		//! @param[in] v The vector to subtract.
		//! @return Returns `*this`.
		Float4& operator-= (const Float4& v);
		//! Multiplies this vector with one vector, and stores the result to this vector.
		//! @details This function performs the following operations:
		//! ```
		//! this->x *= v.x;
		//! this->y *= v.y;
		//! this->z *= v.z;
		//! this->w *= v.w;
		//! ```
		//! @param[in] v The vector to multiply.
		//! @return Returns `*this`.
		Float4& operator*= (const Float4& v);
		//! Divides this vector with one vector, and stores the result to this vector.
		//! @details This function performs the following operations:
		//! ```
		//! this->x /= v.x;
		//! this->y /= v.y;
		//! this->z /= v.z;
		//! this->w /= v.w;
		//! ```
		//! @param[in] v The vector to divide.
		//! @return Returns `*this`.
		Float4& operator/= (const Float4& v);
		//! Adds this vector with one scalar, and stores the result to this vector.
		//! @details This function performs the following operations:
		//! ```
		//! this->x += s;
		//! this->y += s;
		//! this->z += s;
		//! this->w += s;
		//! ```
		//! @param[in] s The scalar to add.
		//! @return Returns `*this`.
		Float4& operator+= (f32 s);
		//! Subtracts this vector with one scalar, and stores the result to this vector.
		//! @details This function performs the following operations:
		//! ```
		//! this->x -= s;
		//! this->y -= s;
		//! this->z -= s;
		//! this->w -= s;
		//! ```
		//! @param[in] s The scalar to subtract.
		//! @return Returns `*this`.
		Float4& operator-= (f32 s);
		//! Multiplies this vector with one scalar, and stores the result to this vector.
		//! @details This function performs the following operations:
		//! ```
		//! this->x *= s;
		//! this->y *= s;
		//! this->z *= s;
		//! this->w *= s;
		//! ```
		//! @param[in] s The scalar to multiply.
		//! @return Returns `*this`.
		Float4& operator*= (f32 s);
		//! Divides this vector with one scalar, and stores the result to this vector.
		//! @details This function performs the following operations:
		//! ```
		//! this->x /= s;
		//! this->y /= s;
		//! this->z /= s;
		//! this->w /= s;
		//! ```
		//! @param[in] s The scalar to divide.
		//! @return Returns `*this`.
		Float4& operator/= (f32 s);
		//! Gets the vector as-is.
		//! @return Returns one copy of this vector.
		Float4 operator+ () const { return *this; }
		//! Gets a negation of this vector.
		//! @return Returns a negation of this vector.
		Float4 operator- () const;
		//! Gets the first two components of this vector.
		//! @return Returns one 2D vector that contains the first two components of this vector.
		Float2 xy() const { return Float2(x, y); }
		//! Gets the first three components of this vector.
		//! @return Returns one 2D vector that contains the first three components of this vector.
		Float3 xyz() const { return Float3(x, y, z); }

		//! Creates one vector with value {0, 0, 0, 0}.
		//! @return Returns the created vector.
		static constexpr Float4 zero() { return Float4(0.0f, 0.0f, 0.0f, 0.0f); }
		//! Creates one vector with value {1, 1, 1, 1}.
		//! @return Returns the created vector.
		static constexpr Float4 one() { return Float4(1.0f, 1.0f, 1.0f, 1.0f); }
		//! Creates one vector with value {1, 0, 0, 0}.
		//! @return Returns the created vector.
		static constexpr Float4 unit_x() { return Float4(1.0f, 0.0f, 0.0f, 0.0f); }
		//! Creates one vector with value {0, 1, 0, 0}.
		//! @return Returns the created vector.
		static constexpr Float4 unit_y() { return Float4(0.0f, 1.0f, 0.0f, 0.0f); }
		//! Creates one vector with value {0, 0, 1, 0}.
		//! @return Returns the created vector.
		static constexpr Float4 unit_z() { return Float4(0.0f, 0.0f, 1.0f, 0.0f); }
		//! Creates one vector with value {0, 0, 0, 1}.
		//! @return Returns the created vector.
		static constexpr Float4 unit_w() { return Float4(0.0f, 0.0f, 0.0f, 1.0f); }
	};

	//! Adds two vectors.
	//! @details This function performs the following operations:
	//! ```
	//! Float4 r;
	//! r.x = v1.x + v2.x;
	//! r.y = v1.y + v2.y;
	//! r.z = v1.z + v2.z;
	//! r.w = v1.w + v2.w;
	//! return r;
	//! ```
	//! @param[in] v1 The first vector.
	//! @param[in] v2 The second vector.
	//! @return Returns the result vector.
	Float4 operator+ (const Float4& v1, const Float4& v2);
	//! Subtracts two vectors.
	//! @details This function performs the following operations:
	//! ```
	//! Float4 r;
	//! r.x = v1.x - v2.x;
	//! r.y = v1.y - v2.y;
	//! r.z = v1.z - v2.z;
	//! r.w = v1.w - v2.w;
	//! return r;
	//! ```
	//! @param[in] v1 The first vector.
	//! @param[in] v2 The second vector.
	//! @return Returns the result vector.
	Float4 operator- (const Float4& v1, const Float4& v2);
	//! Multiplies two vectors.
	//! @details This function performs the following operations:
	//! ```
	//! Float4 r;
	//! r.x = v1.x * v2.x;
	//! r.y = v1.y * v2.y;
	//! r.z = v1.z * v2.z;
	//! r.w = v1.w * v2.w;
	//! return r;
	//! ```
	//! @param[in] v1 The first vector.
	//! @param[in] v2 The second vector.
	//! @return Returns the result vector.
	Float4 operator* (const Float4& v1, const Float4& v2);
	//! Divides two vectors.
	//! @details This function performs the following operations:
	//! ```
	//! Float4 r;
	//! r.x = v1.x / v2.x;
	//! r.y = v1.y / v2.y;
	//! r.z = v1.z / v2.z;
	//! r.w = v1.w / v2.w;
	//! return r;
	//! ```
	//! @param[in] v1 The first vector.
	//! @param[in] v2 The second vector.
	//! @return Returns the result vector.
	Float4 operator/ (const Float4& v1, const Float4& v2);
	//! Adds one vector with one scalar.
	//! @details This function performs the following operations:
	//! ```
	//! Float4 r;
	//! r.x = v.x + s;
	//! r.y = v.y + s;
	//! r.z = v.z + s;
	//! r.w = v.w + s;
	//! return r;
	//! ```
	//! @param[in] v The vector.
	//! @param[in] s The scalar.
	//! @return Returns the result vector.
	Float4 operator+ (const Float4& v, f32 s);
	//! Adds one vector with one scalar.
	//! @details This function performs the following operations:
	//! ```
	//! Float4 r;
	//! r.x = v.x + s;
	//! r.y = v.y + s;
	//! r.z = v.z + s;
	//! r.w = v.w + s;
	//! return r;
	//! ```
	//! @param[in] v The vector.
	//! @param[in] s The scalar.
	//! @return Returns the result vector.
	Float4 operator+ (f32 s, const Float4& v);
	//! Subtracts one vector with one scalar.
	//! @details This function performs the following operations:
	//! ```
	//! Float4 r;
	//! r.x = v.x - s;
	//! r.y = v.y - s;
	//! r.z = v.z - s;
	//! r.w = v.w - s;
	//! return r;
	//! ```
	//! @param[in] v The vector.
	//! @param[in] s The scalar.
	//! @return Returns the result vector.
	Float4 operator- (const Float4& v, f32 s);
	//! Subtracts one scalar with one vector.
	//! @details This function performs the following operations:
	//! ```
	//! Float4 r;
	//! r.x = s - v.x;
	//! r.y = s - v.y;
	//! r.z = s - v.z;
	//! r.w = s - v.w;
	//! return r;
	//! ```
	//! @param[in] s The scalar.
	//! @param[in] v The vector.
	//! @return Returns the result vector.
	Float4 operator- (f32 s, const Float4& v);
	//! Multiplies one vector with one scalar.
	//! @details This function performs the following operations:
	//! ```
	//! Float4 r;
	//! r.x = v.x * s;
	//! r.y = v.y * s;
	//! r.z = v.z * s;
	//! r.w = v.w * s;
	//! return r;
	//! ```
	//! @param[in] v The vector.
	//! @param[in] s The scalar.
	//! @return Returns the result vector.
	Float4 operator* (const Float4& v, f32 s);
	//! Multiplies one vector with one scalar.
	//! @details This function performs the following operations:
	//! ```
	//! Float4 r;
	//! r.x = v.x * s;
	//! r.y = v.y * s;
	//! r.z = v.z * s;
	//! r.w = v.w * s;
	//! return r;
	//! ```
	//! @param[in] s The scalar.
	//! @param[in] v The vector.
	//! @return Returns the result vector.
	Float4 operator* (f32 s, const Float4& v);
	//! Divides one vector with one scalar.
	//! @details This function performs the following operations:
	//! ```
	//! Float4 r;
	//! r.x = v.x / s;
	//! r.y = v.y / s;
	//! r.z = v.z / s;
	//! r.w = v.w / s;
	//! return r;
	//! ```
	//! @param[in] v The vector.
	//! @param[in] s The scalar.
	//! @return Returns the result vector.
	Float4 operator/ (const Float4& v, f32 s);
	//! Divides one scalar with one vector.
	//! @details This function performs the following operations:
	//! ```
	//! Float4 r;
	//! r.x = s / v.x;
	//! r.y = s / v.y;
	//! r.z = s / v.z;
	//! r.w = s / v.w;
	//! return r;
	//! ```
	//! @param[in] s The scalar.
	//! @param[in] v The vector.
	//! @return Returns the result vector.
	Float4 operator/ (f32 s, const Float4& v);

	//! Checks whether the point is in the specified boundary.
	//! @details This function performs the following operations:
	//! ```
	//! if (point.x >= min_bound.x && point.x <= max_bound.x 
	//! 	&& point.y >= min_bound.y && point.y <= max_bound.y
	//!		&& point.z >= min_bound.z && point.z <= max_bound.z
	//!		&& point.w >= min_bound.w && point.w <= max_bound.w)
	//! 	return true;
	//! else return false;
	//! ```
	//! @param[in] point The point to check.
	//! @param[in] min_bound The minimum boundary value.
	//! @param[in] max_bound The maximum boundary value.
	//! @return Returns `true` if the point is in the specified boundary.
	//! Returns `false` otherwise.
	bool in_bounds(const Float4& point, const Float4& min_bound, const Float4& max_bound);
	//! Computes the length of the vector.
	//! @details This function performs the following operations:
	//! ```
	//! return sqrt(vec.x * vec.x + vec.y * vec.y + vec.z * vec.z + vec.w * vec.w);
	//! ```
	//! @param[in] vec The vector.
	//! @return Returns the length of the vector.
	f32 length(const Float4& vec);
	//! Computes the squared length of the vector.
	//! @details This function performs the following operations:
	//! ```
	//! return vec.x * vec.x + vec.y * vec.y + vec.z * vec.z + vec.w * vec.w;
	//! ```
	//! @param[in] vec The vector.
	//! @return Returns the squared length of the vector.
	f32 length_squared(const Float4& vec);
	//! Computes the dot product of two vectors.
	//! @details This function performs the following operations:
	//! ```
	//! return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z + v1.w * v2.w;
	//! ```
	//! @param[in] v1 The first vector.
	//! @param[in] v2 The second vector.
	//! @return Returns the dot product of two vectors.
	f32 dot(const Float4& v1, const Float4& v2);
	//! Computes the cross product of two vectors.
	//! @details This function performs the following operations:
	//! ```
	//! Float4 result;
	//! result.x = ((v2.z * v3.w - v2.w * v3.z) * v1.y) - ((v2.y * v3.w - v2.w * v3.y) * v1.z) + ((v2.y * v3.z - v2.z * v3.y) * v1.w);
	//! result.y = ((v2.w * v3.z - v2.z * v3.w) * v1.x) - ((v2.w * v3.x - v2.x * v3.w) * v1.z) + ((v2.z * v3.x - v2.x * v3.z) * v1.w);
	//! result.z = ((v2.y * v3.w - v2.w * v3.y) * v1.x) - ((v2.x * v3.w - v2.w * v3.x) * v1.y) + ((v2.x * v3.y - v2.y * v3.x) * v1.w);
	//! result.w = ((v2.z * v3.y - v2.y * v3.z) * v1.x) - ((v2.z * v3.x - v2.x * v3.z) * v1.y) + ((v2.y * v3.x - v2.x * v3.y) * v1.z);
	//! return result;
	//! ```
	//! @param[in] v1 The first vector.
	//! @param[in] v2 The second vector.
	//! @return Returns the cross product of two vectors.
	Float4 cross(const Float4& v1, const Float4& v2, const Float4& v3);
	//! Normalizes the vector so that the length of the vector is 1.
	//! @details This function performs the following operations:
	//! ```
	//! f32 len = length(vec);
    //! if (len > 0)
    //! {
    //!     len = 1.0f / len;
    //! }
	//! Float3 result;
	//! result.x = vec.x * len;
	//! result.y = vec.y * len;
	//! result.z = vec.z * len;
	//! result.w = vec.w * len;
	//! return result;
	//! ```
	//! @param[in] vec The vector to normalize.
	//! @return Returns the normalized vector.
	Float4 normalize(const Float4& vec);
	//! Clamps the vector to the specified range.
	//! @details This function performs the following operations:
	//! ```
	//! Float3 result;
	//! result.x = vec.x    > vec_min.x ? vec.x    : vec_min.x;
	//! result.x = result.x < vec_max.x ? result.x : vec_max.x;
	//! result.y = vec.y    > vec_min.y ? vec.y    : vec_min.y;
	//! result.y = result.y < vec_max.y ? result.y : vec_max.y;
	//! result.z = vec.z    > vec_min.z ? vec.z    : vec_min.z;
	//! result.z = result.z < vec_max.z ? result.z : vec_max.z;
	//! result.w = vec.w    > vec_min.w ? vec.w    : vec_min.w;
	//! result.w = result.w < vec_max.w ? result.w : vec_max.w;
	//! return result;
	//! ```
	//! @param[in] vec The vector to clamp.
	//! @param[in] vec_min The lower clamp value.
	//! @param[in] vec_max The upper clamp value.
	//! @return Returns the clampd vector.
	Float4 clamp(const Float4& vec, const Float4& vec_min, const Float4& vec_max);
	//! Computes the distance between two points.
	//! @details This function performs the following operations:
	//! ```
	//! f32 dx = v1.x - v2.x;
	//! f32 dy = v1.y - v2.y;
	//! f32 dz = v1.z - v2.z;
	//! f32 dw = v1.w - v2.w;
	//! return sqrt(dx * dx + dy * dy + dz * dz + dw * dw);
	//! ```
	//! @param[in] v1 The first point.
	//! @param[in] v2 The second point.
	//! @return Returns the distance between two points.
	f32 distance(const Float4& v1, const Float4& v2);
	//! Computes the squared distance between two points.
	//! @details This function performs the following operations:
	//! ```
	//! f32 dx = v1.x - v2.x;
	//! f32 dy = v1.y - v2.y;
	//! f32 dz = v1.z - v2.z;
	//! f32 dw = v1.w - v2.w;
	//! return dx * dx + dy * dy + dz * dz + dw * dw;
	//! ```
	//! @param[in] v1 The first point.
	//! @param[in] v2 The second point.
	//! @return Returns the squared distance between two points.
	f32 distance_squared(const Float4& v1, const Float4& v2);
	//! Computes the minimum value of two vectors.
	//! @details This function performs the following operations:
	//! ```
	//! Float4 result;
	//! result.x = v1.x < v2.x ? v1.x : v2.x;
	//! result.y = v1.y < v2.y ? v1.y : v2.y;
	//! result.z = v1.z < v2.z ? v1.z : v2.z;
	//! result.w = v1.w < v2.w ? v1.w : v2.w;
	//! return result;
	//! ```
	//! @param[in] v1 The first vector.
	//! @param[in] v2 The second vector.
	//! @return Returns the minimum value of two vectors.
	Float4 min(const Float4& v1, const Float4& v2);
	//! Computes the maximum value of two vectors.
	//! @details This function performs the following operations:
	//! ```
	//! Float4 result;
	//! result.x = v1.x > v2.x ? v1.x : v2.x;
	//! result.y = v1.y > v2.y ? v1.y : v2.y;
	//! result.z = v1.z > v2.z ? v1.z : v2.z;
	//! result.w = v1.w > v2.w ? v1.w : v2.w;
	//! return result;
	//! ```
	//! @param[in] v1 The first vector.
	//! @param[in] v2 The second vector.
	//! @return Returns the maximum value of two vectors.
	Float4 max(const Float4& v1, const Float4& v2);
	//! Performs linear interpolation between two vectors.
	//! @details This function performs the following operations:
	//! ```
	//! Float4 result;
	//! result.x = v1.x + t * (v2.x - v1.x);
	//! result.y = v1.y + t * (v2.y - v1.y);
	//! result.z = v1.z + t * (v2.z - v1.z);
	//! result.w = v1.w + t * (v2.w - v1.w);
	//! return result;
	//! ```
	//! @param[in] v1 The first vector.
	//! @param[in] v2 The second vector.
	//! @param[in] t The interpolation weight.
	//! @return Returns the interpolation result.
	Float4 lerp(const Float4& v1, const Float4& v2, f32 t);
	//! Performs smoothstep interpolation between two vectors.
	//! @details This function performs the following operations:
	//! ```
	//! Float4 result;
	//! t = clamp(t, 0, 1);
	//! t = t * t * (3 - 2 * t);
	//! result.x = v1.x + t * (v2.x - v1.x);
	//! result.y = v1.y + t * (v2.y - v1.y);
	//! result.z = v1.z + t * (v2.z - v1.z);
	//! result.w = v1.w + t * (v2.w - v1.w);
	//! return result;
	//! ```
	//! @param[in] v1 The first vector.
	//! @param[in] v2 The second vector.
	//! @param[in] t The interpolation weight.
	//! @return Returns the interpolation result.
	Float4 smoothstep(const Float4& v1, const Float4& v2, f32 t);
	//! Performs barycentric interpolation between three vectors.
	//! @details This function performs the following operations:
	//! ```
	//! Float4 result;
	//! result.x = v1.x + (v2.x - v1.x) * f + (v3.x - v1.x) * g;
	//! result.y = v1.y + (v2.y - v1.y) * f + (v3.y - v1.y) * g;
	//! result.z = v1.z + (v2.z - v1.z) * f + (v3.z - v1.z) * g;
	//! result.w = v1.w + (v2.w - v1.w) * f + (v3.w - v1.w) * g;
	//! return result;
	//! ```
	//! @param[in] v1 The first vector.
	//! @param[in] v2 The second vector.
	//! @param[in] v3 The third vector.
	//! @param[in] f The interpolation weight between v1 and v2.
	//! @param[in] g The interpolation weight between v1 and v3.
	//! @return Returns the interpolation result.
	Float4 barycentric(const Float4& v1, const Float4& v2, const Float4& vec3, f32 f, f32 g);
	//! Performs centripetal Catmull–Rom spline interpolation.
	//! @details This function performs the following operations:
	//! ```
	//! f32 t2 = t * t;
	//! f32 t3 = t2 * t;
	//! return (v1 * (-t3 + 2 * t2 - t) +
	//!         v2 * (3 * t3 - 5 * t2 + 2) +
	//!         v3 * (-3 * t3 + 4 * t2 + t) +
	//!         v4 * (t3 - t2)) * 0.5;
	//! ```
	//! @param[in] v1 The first point of the curve.
	//! @param[in] v2 The second point of the curve.
	//! @param[in] v3 The third point of the curve.
	//! @param[in] v4 The fourth point of the curve.
	//! @param[in] t The interpolation weight.
	//! @return Returns the interpolation result.
	Float4 catmull_rom(const Float4& v1, const Float4& v2, const Float4& vec3, const Float4& vec4, f32 t);
	//! Performs Hermite spline interpolation.
	//! @details This function performs the following operations:
	//! ```
	//! Float4 result;
	//! f32 t2 = t * t;
	//! f32 t3 = t2 * t;
	//! return  v1 * (2 * t3 - 3 * t2 + 1) +
	//!         t1 * (t3 - 2 * t2 + t) +
	//!         v3 * (-2 * t3 + 3 * t2) +
	//!         t2 * (t3 - t2);
	//! ```
	//! @param[in] v1 The first point of the curve.
	//! @param[in] t1 The first tangent of the curve.
	//! @param[in] v2 The second point of the curve.
	//! @param[in] t2 The second tangent of the curve.
	//! @param[in] t The interpolation weight.
	//! @return Returns the interpolation result.
	Float4 hermite(const Float4& v1, const Float4& t1, const Float4& v2, const Float4& t2, f32 t);
	//! Computes the reflected vector of the input vector.
	//! @details This function performs the following operations:
	//! ```
	//! return ivec - (2 * dot(ivec, nvec) * nvec);
	//! ```
	//! @param[in] ivec The direction of the incident ray.
	//! @param[in] nvec The direction of the surface normal.
	//! @return Returns the direction of the reflected ray. The length of the reflected vector 
	//! is the same as the length of the `ivec`.
	//! @par Valid Usage
	//! * `nvec` must be a normalized vector.
	Float4 reflect(const Float4& ivec, const Float4& nvec);
	//! Computes the refracted vector of the input vector.
	//! @details This function performs the following operations:
	//! ```
	//! f32 proj = dot(ivec, nvec);
	//! f32 deter = 1.0f - refraction_index * refraction_index * (1.0f - proj * proj);
	//! if (deter >= 0.0f)
	//! {
	//! 	return ivec * refraction_index - nvec * (refraction_index * proj + sqrtf(deter));
	//! }
	//! return Float4(0.0f);
	//! ```
	//! @param[in] ivec The direction of the incident ray.
	//! @param[in] nvec The direction of the surface normal.
	//! @param[in] refraction_index The refraction index.
	//! @return Returns the direction of the refracted ray. The length of the refracted ray is normalized.
	//! Returns {0, 0} if the incident angle of `ivec` is too big that no refraction can occur 
	//! (full reflection).
	//! @par Valid Usage
	//! * Both `ivec` and `nvec` must be normalized vectors.
	//! * `refraction_index` must be a value greater than `0`.
	Float4 refract(const Float4& ivec, const Float4& nvec, f32 refraction_index);

	//! A generalized version of 4D vector. This vector type does not have specific alignment requirement.
	//! @tparam[in] _Ty The element type of the vector.
	template <typename _Ty>
	struct Vec4U
	{
		union
		{
			struct
			{
				//! The fist component of the vector.
				_Ty x;
				//! The second component of the vector.
				_Ty y;
				//! The third component of the vector.
				_Ty z;
				//! The fourth component of the vector.
				_Ty w;
			};
			//! The array of components.
			_Ty m[4];
		};
		//! Constructs one vector with components uninitialized.
		Vec4U() = default;
		//! Constructs one vector by coping components from another vector.
		Vec4U(const Vec4U&) = default;
		//! Assigns one vector by coping components from another vector.
		Vec4U& operator=(const Vec4U&) = default;
		//! Constructs one vector by coping components from another vector.
		Vec4U(Vec4U&&) = default;
		//! Assigns one vector by coping components from another vector.
		Vec4U& operator=(Vec4U&&) = default;
		//! Constructs one vector from one @ref Float4 vector.
		//! @param[in] rhs The vector to use.
		Vec4U(const Float4& rhs) :
			x((_Ty)rhs.x), y((_Ty)rhs.y), z((_Ty)rhs.z), w((_Ty)rhs.w) {}
		//! Assigns one vector from one @ref Float4 vector.
		//! @param[in] rhs The vector to use.
		//! @return Returns `*this`.
		Vec4U& operator=(const Float4& rhs)
		{
			x = (_Ty)rhs.x;
			y = (_Ty)rhs.y;
			z = (_Ty)rhs.z;
			w = (_Ty)rhs.w;
			return *this;
		}
		//! Converts this vector to @ref Float4.
		//! @return Returns the converted vector.
		operator Float4() const
		{
			return Float4((f32)x, (f32)y, (f32)z, (f32)w);
		}
		//! Constructs one vector from one scalar.
		//! @param[in] s The scalar to use. All components will be initialized to this value.
		constexpr explicit Vec4U(_Ty _x) :
			x(_x), y(_x), z(_x), w(_x) {}
		//! Constructs one vector from values.
		//! @param[in] x The value of the first component.
		//! @param[in] y The value of the second component.
		//! @param[in] y The value of the third component.
		//! @param[in] w The value of the fourth component.
		constexpr Vec4U(_Ty _x, _Ty _y, _Ty _z, _Ty _w) :
			x(_x), y(_y), z(_z), w(_w) {}
		//! Compares two vectors for equality.
		//! @param[in] rhs The vector to compare with.
		//! @return Returns `true` if two vectors are equal. Returns `false` otherwise.
		bool operator==(const Vec4U& rhs) const
		{
			return (x == rhs.x) && (y == rhs.y) && (z == rhs.z) && (w == rhs.w);
		}
		//! Compares two vectors for non-equality.
		//! @param[in] rhs The vector to compare with.
		//! @return Returns `true` if two vectors are not equal. Returns `false` otherwise.
		bool operator!=(const Vec4U& rhs) const
		{
			return !(*this == rhs);
		}
	};
	//! Unaligned 4D floating-point vector type.
	using Float4U = Vec4U<f32>;
	//! Unaligned 4D signed integer vector type.
	using Int4U = Vec4U<i32>;
	//! Unaligned 4D unsigned integer vector type.
	using UInt4U = Vec4U<u32>;

	//! Gets the type object of @ref Float2.
	//! @return Returns the type object.
	LUNA_RUNTIME_API typeinfo_t float2_type();
	//! Gets the type object of @ref Float3.
	//! @return Returns the type object.
	LUNA_RUNTIME_API typeinfo_t float3_type();
	//! Gets the type object of @ref Float4.
	//! @return Returns the type object.
	LUNA_RUNTIME_API typeinfo_t float4_type();
	//! Gets the type object of @ref Vec2U. The type object is a generic structure type that should be instanced before use.
	//! @return Returns the type object.
	LUNA_RUNTIME_API typeinfo_t vec2u_type();
	//! Gets the type object of @ref Vec3U. The type object is a generic structure type that should be instanced before use.
	//! @return Returns the type object.
	LUNA_RUNTIME_API typeinfo_t vec3u_type();
	//! Gets the type object of @ref Vec4U. The type object is a generic structure type that should be instanced before use.
	//! @return Returns the type object.
	LUNA_RUNTIME_API typeinfo_t vec4u_type();

	template <> struct typeof_t<Float2> { typeinfo_t operator()() const { return float2_type(); } };
	template <> struct typeof_t<Float3> { typeinfo_t operator()() const { return float3_type(); } };
	template <> struct typeof_t<Float4> { typeinfo_t operator()() const { return float4_type(); } };

	template <typename _Ty> struct typeof_t<Vec2U<_Ty>> { typeinfo_t operator()() const { return get_generic_instanced_type(vec2u_type(), { typeof<_Ty>() }); } };
	template <typename _Ty> struct typeof_t<Vec3U<_Ty>> { typeinfo_t operator()() const { return get_generic_instanced_type(vec3u_type(), { typeof<_Ty>() }); } };
	template <typename _Ty> struct typeof_t<Vec4U<_Ty>> { typeinfo_t operator()() const { return get_generic_instanced_type(vec4u_type(), { typeof<_Ty>() }); } };

	//! @}
}

#include "Impl/Float2.inl"
#include "Impl/Float3.inl"
#include "Impl/Float4.inl"