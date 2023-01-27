/*
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

	struct alignas(16) Float2
	{
		lustruct("Float2", "{69D3BC60-3EDA-49F5-B622-E832118FD3D2}");
		union
		{
			struct
			{
				f32 x;
				f32 y;
			};
			f32 m[2];
		};
		Float2() = default;
		Float2(const Float2&) = default;
		Float2& operator=(const Float2&) = default;
		Float2(Float2&&) = default;
		Float2& operator=(Float2&&) = default;
		constexpr explicit Float2(f32 _x) :
			x(_x), y(_x) {}
		constexpr Float2(f32 _x, f32 _y) :
			x(_x), y(_y) {}
		bool operator==(const Float2& v) const;
		bool operator!=(const Float2& v) const;
		Float2& operator+= (const Float2& v);
		Float2& operator-= (const Float2& v);
		Float2& operator*= (const Float2& v);
		Float2& operator/= (const Float2& v);
		Float2& operator+= (f32 s);
		Float2& operator-= (f32 s);
		Float2& operator*= (f32 s);
		Float2& operator/= (f32 s);
		Float2 operator+ () const { return *this; }
		Float2 operator- () const { return Float2(-x, -y); }
		static constexpr Float2 zero() { return Float2(0.0f, 0.0f); }
		static constexpr Float2 one() { return Float2(1.0f, 1.0f); }
		static constexpr Float2 unit_x() { return Float2(1.0f, 0.0f); }
		static constexpr Float2 unit_y() { return Float2(0.0f, 1.0f); }
	};

	static_assert(sizeof(Float2) == sizeof(f32) * 4, "Incorrect Float2 size.");

	Float2 operator+ (const Float2& v1, const Float2& v2);
	Float2 operator- (const Float2& v1, const Float2& v2);
	Float2 operator* (const Float2& v1, const Float2& v2);
	Float2 operator/ (const Float2& v1, const Float2& v2);
	Float2 operator+ (const Float2& v, f32 s);
	Float2 operator+ (f32 s, const Float2& v);
	Float2 operator- (const Float2& v, f32 s);
	Float2 operator- (f32 s, const Float2& v1);
	Float2 operator* (const Float2& v, f32 s);
	Float2 operator* (f32 s, const Float2& v);
	Float2 operator/ (const Float2& v, f32 s);
	Float2 operator/ (f32 s, const Float2& v);

	bool in_bounds(const Float2& vec, const Float2& bounds);
	f32 length(const Float2& vec);
	f32 length_squared(const Float2& vec);
	f32 dot(const Float2& vec1, const Float2& vec2);
	Float2 cross(const Float2& vec1, const Float2& vec2);
	Float2 normalize(const Float2& vec);
	Float2 clamp(const Float2& vec, const Float2& vec_min, const Float2& vec_max);
	f32 distance(const Float2& vec1, const Float2& vec2);
	f32 distance_squared(const Float2& vec1, const Float2& vec2);
	Float2 min(const Float2& vec1, const Float2& vec2);
	Float2 max(const Float2& vec1, const Float2& vec2);
	Float2 lerp(const Float2& vec1, const Float2& vec2, f32 t);
	Float2 smoothstep(const Float2& vec1, const Float2& vec2, f32 t);
	Float2 barycentric(const Float2& vec1, const Float2& vec2, const Float2& vec3, f32 f, f32 g);
	Float2 catmull_rom(const Float2& vec1, const Float2& vec2, const Float2& vec3, const Float2& vec4, f32 t);
	Float2 hermite(const Float2& vec1, const Float2& t1, const Float2& vec2, const Float2& t2, f32 t);
	Float2 reflect(const Float2& ivec, const Float2& nvec);
	Float2 refract(const Float2& ivec, const Float2& nvec, f32 refractionIndex);
	bool in_rect(const Float2& point, const Float2& min_point, const Float2& max_point);

	template <typename _Ty>
	struct Vec2U
	{
		union
		{
			struct
			{
				_Ty x;
				_Ty y;
			};
			_Ty m[2];
		};

		Vec2U() = default;
		Vec2U(const Vec2U&) = default;
		Vec2U& operator=(const Vec2U&) = default;
		Vec2U(Vec2U&&) = default;
		Vec2U& operator=(Vec2U&&) = default;
		Vec2U(const Float2& rhs) :
			x((_Ty)rhs.x), y((_Ty)rhs.y) {}
		Vec2U& operator=(const Float2& rhs)
		{
			x = (_Ty)rhs.x;
			y = (_Ty)rhs.y;
			return *this;
		}
		operator Float2() const
		{
			return Float2((f32)x, (f32)y);
		}
		constexpr explicit Vec2U(_Ty _x) :
			x(_x), y(_x) {}
		constexpr Vec2U(_Ty _x, _Ty _y) :
			x(_x), y(_y) {}
		bool operator==(const Vec2U& rhs) const
		{
			return (x == rhs.x) && (y == rhs.y);
		}
		bool operator!=(const Vec2U& rhs) const
		{
			return !(*this == rhs);
		}
	};

	using Float2U = Vec2U<f32>;
	using Int2U = Vec2U<i32>;
	using UInt2U = Vec2U<u32>;

	struct alignas(16) Float3
	{
		lustruct("Float3", "{7727472C-AF79-40E8-8385-CD7677389E4F}");
		union
		{
			struct
			{
				f32 x;
				f32 y;
				f32 z;
			};
			f32 m[3];
		};
		Float3() = default;
		Float3(const Float3&) = default;
		Float3& operator=(const Float3&) = default;
		Float3(Float3&&) = default;
		Float3& operator=(Float3&&) = default;
		constexpr explicit Float3(f32 _x) :
			x(_x), y(_x), z(_x) {}
		constexpr Float3(f32 _x, f32 _y, f32 _z) :
			x(_x), y(_y), z(_z) {}
		bool operator==(const Float3& v) const;
		bool operator!=(const Float3& v) const;
		Float3& operator+= (const Float3& v);
		Float3& operator-= (const Float3& v);
		Float3& operator*= (const Float3& v);
		Float3& operator/= (const Float3& v);
		Float3& operator+= (f32 s);
		Float3& operator-= (f32 s);
		Float3& operator*= (f32 s);
		Float3& operator/= (f32 s);
		Float3 operator+ () const { return *this; }
		Float3 operator- () const;

		Float2 xy() const { return Float2(x, y); }

		// Constants
		static constexpr Float3 zero() { return Float3(0.0f, 0.0f, 0.0f); }
		static constexpr Float3 one() { return Float3(1.0f, 1.0f, 1.0f); }
		static constexpr Float3 unit_x() { return Float3(1.0f, 0.0f, 0.0f); }
		static constexpr Float3 unit_y() { return Float3(0.0f, 1.0f, 0.0f); }
		static constexpr Float3 unit_z() { return Float3(0.0f, 0.0f, 1.0f); }
		static constexpr Float3 up() { return Float3(0.0f, 1.0f, 0.0f); }
		static constexpr Float3 down() { return Float3(0.0f, -1.0f, 0.0f); }
		static constexpr Float3 right() { return Float3(1.0f, 0.0f, 0.0f); }
		static constexpr Float3 left() { return Float3(-1.0f, 0.0f, 0.0f); }
		static constexpr Float3 forward() { return Float3(0.0f, 0.0f, -1.0f); }
		static constexpr Float3 backward() { return Float3(0.0f, 0.0f, 1.0f); }
	};

	static_assert(sizeof(Float3) == sizeof(f32) * 4, "Incorrect Float2 size.");

	Float3 operator+ (const Float3& v1, const Float3& v2);
	Float3 operator- (const Float3& v1, const Float3& v2);
	Float3 operator* (const Float3& v1, const Float3& v2);
	Float3 operator/ (const Float3& v1, const Float3& v2);
	Float3 operator+ (const Float3& v, f32 s);
	Float3 operator+ (f32 s, const Float3& v);
	Float3 operator- (const Float3& v, f32 s);
	Float3 operator- (f32 s, const Float3& v);
	Float3 operator* (const Float3& v, f32 s);
	Float3 operator* (f32 s, const Float3& v);
	Float3 operator/ (const Float3& v, f32 s);
	Float3 operator/ (f32 s, const Float3& v);

	bool in_bounds(const Float3& vec, const Float3& bounds);
	f32 length(const Float3& vec);
	f32 length_squared(const Float3& vec);
	f32 dot(const Float3& vec1, const Float3& vec2);
	Float3 cross(const Float3& vec1, const Float3& vec2);
	Float3 normalize(const Float3& vec);
	Float3 clamp(const Float3& vec, const Float3& vec_min, const Float3& vec_max);
	f32 distance(const Float3& vec1, const Float3& vec2);
	f32 distance_squared(const Float3& vec1, const Float3& vec2);
	Float3 min(const Float3& vec1, const Float3& vec2);
	Float3 max(const Float3& vec1, const Float3& vec2);
	Float3 lerp(const Float3& vec1, const Float3& vec2, f32 t);
	Float3 smoothstep(const Float3& vec1, const Float3& vec2, f32 t);
	Float3 barycentric(const Float3& vec1, const Float3& vec2, const Float3& vec3, f32 f, f32 g);
	Float3 catmull_rom(const Float3& vec1, const Float3& vec2, const Float3& vec3, const Float3& vec4, f32 t);
	Float3 hermite(const Float3& vec1, const Float3& t1, const Float3& vec2, const Float3& t2, f32 t);
	Float3 reflect(const Float3& ivec, const Float3& nvec);
	Float3 refract(const Float3& ivec, const Float3& nvec, f32 refractionIndex);
	bool in_box(const Float3& point, const Float3& min_point, const Float3& max_point);

	//! Unaligned version of Float3. This is used for storing or transferring data between CPU/GPU and network only. 
	template <typename _Ty>
	struct Vec3U
	{
		union
		{
			struct
			{
				_Ty x;
				_Ty y;
				_Ty z;
			};
			_Ty m[3];
		};

		Vec3U() = default;
		Vec3U(const Vec3U&) = default;
		Vec3U& operator=(const Vec3U&) = default;
		Vec3U(Vec3U&&) = default;
		Vec3U& operator=(Vec3U&&) = default;

		Vec3U(const Float3& rhs) :
			x((_Ty)rhs.x), y((_Ty)rhs.y), z((_Ty)rhs.z) {}
		Vec3U& operator=(const Float3& rhs)
		{
			x = (_Ty)rhs.x;
			y = (_Ty)rhs.y;
			z = (_Ty)rhs.z;
			return *this;
		}
		operator Float3() const
		{
			return Float3((f32)x, (f32)y, (f32)z);
		}

		constexpr explicit Vec3U(_Ty _x) :
			x(_x), y(_x), z(_x) {}
		constexpr Vec3U(_Ty _x, _Ty _y, _Ty _z) :
			x(_x), y(_y), z(_z) {}

		bool operator==(const Vec3U& rhs) const
		{
			return (x == rhs.x) && (y == rhs.y) && (z == rhs.z);
		}
		bool operator!=(const Vec3U& rhs) const
		{
			return !(*this == rhs);
		}
	};

	using Float3U = Vec3U<f32>;
	using Int3U = Vec3U<i32>;
	using UInt3U = Vec3U<u32>;

	struct alignas(16) Float4
	{
		lustruct("Float4", "{88547D46-4DF1-42ED-BB48-96571BBD651F}");
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

		Float4() = default;
		Float4(const Float4&) = default;
		Float4& operator=(const Float4&) = default;
		Float4(Float4&&) = default;
		Float4& operator=(Float4&&) = default;

		constexpr explicit Float4(f32 _x) :
			x(_x), y(_x), z(_x), w(_x) {}
		constexpr Float4(f32 _x, f32 _y, f32 _z, f32 _w) :
			x(_x), y(_y), z(_z), w(_w) {}
		bool operator==(const Float4& v) const;
		bool operator!=(const Float4& v) const;
		Float4& operator+= (const Float4& v);
		Float4& operator-= (const Float4& v);
		Float4& operator*= (const Float4& v);
		Float4& operator/= (const Float4& v);
		Float4& operator+= (f32 s);
		Float4& operator-= (f32 s);
		Float4& operator*= (f32 s);
		Float4& operator/= (f32 s);

		Float4 operator+ () const { return *this; }
		Float4 operator- () const;

		Float2 xy() const { return Float2(x, y); }
		Float3 xyz() const { return Float3(x, y, z); }

		static constexpr Float4 zero() { return Float4(0.0f, 0.0f, 0.0f, 0.0f); }
		static constexpr Float4 one() { return Float4(1.0f, 1.0f, 1.0f, 1.0f); }
		static constexpr Float4 unit_x() { return Float4(1.0f, 0.0f, 0.0f, 0.0f); }
		static constexpr Float4 unit_y() { return Float4(0.0f, 1.0f, 0.0f, 0.0f); }
		static constexpr Float4 unit_z() { return Float4(0.0f, 0.0f, 1.0f, 0.0f); }
		static constexpr Float4 unit_w() { return Float4(0.0f, 0.0f, 0.0f, 1.0f); }
	};

	Float4 operator+ (const Float4& v1, const Float4& v2);
	Float4 operator- (const Float4& v1, const Float4& v2);
	Float4 operator* (const Float4& v1, const Float4& v2);
	Float4 operator/ (const Float4& v1, const Float4& v2);
	Float4 operator+ (const Float4& v, f32 s);
	Float4 operator+ (f32 s, const Float4& v);
	Float4 operator- (const Float4& v, f32 s);
	Float4 operator- (f32 s, const Float4& v);
	Float4 operator* (const Float4& v, f32 s);
	Float4 operator* (f32 s, const Float4& v);
	Float4 operator/ (const Float4& v, f32 s);
	Float4 operator/ (f32 s, const Float4& v);

	bool in_bounds(const Float4& vec, const Float4& bounds);
	f32 length(const Float4& vec);
	f32 length_squared(const Float4& vec);
	f32 dot(const Float4& vec1, const Float4& vec2);
	Float4 cross(const Float4& v1, const Float4& v2, const Float4& v3);
	Float4 normalize(const Float4& vec);
	Float4 clamp(const Float4& vec, const Float4& vec_min, const Float4& vec_max);
	f32 distance(const Float4& vec1, const Float4& vec2);
	f32 distance_squared(const Float4& vec1, const Float4& vec2);
	Float4 min(const Float4& vec1, const Float4& vec2);
	Float4 max(const Float4& vec1, const Float4& vec2);
	Float4 lerp(const Float4& vec1, const Float4& vec2, f32 t);
	Float4 smoothstep(const Float4& vec1, const Float4& vec2, f32 t);
	Float4 barycentric(const Float4& vec1, const Float4& vec2, const Float4& vec3, f32 f, f32 g);
	Float4 catmull_rom(const Float4& vec1, const Float4& vec2, const Float4& vec3, const Float4& vec4, f32 t);
	Float4 hermite(const Float4& vec1, const Float4& t1, const Float4& vec2, const Float4& t2, f32 t);
	Float4 reflect(const Float4& ivec, const Float4& nvec);
	Float4 refract(const Float4& ivec, const Float4& nvec, f32 refractionIndex);

	template <typename _Ty>
	struct Vec4U
	{
		union
		{
			struct
			{
				_Ty x;
				_Ty y;
				_Ty z;
				_Ty w;
			};
			_Ty m[4];
		};

		Vec4U() = default;
		Vec4U(const Vec4U&) = default;
		Vec4U& operator=(const Vec4U&) = default;
		Vec4U(Vec4U&&) = default;
		Vec4U& operator=(Vec4U&&) = default;

		Vec4U(const Float4& rhs) :
			x((_Ty)rhs.x), y((_Ty)rhs.y), z((_Ty)rhs.z), w((_Ty)rhs.w) {}
		Vec4U& operator=(const Float4& rhs)
		{
			x = (_Ty)rhs.x;
			y = (_Ty)rhs.y;
			z = (_Ty)rhs.z;
			w = (_Ty)rhs.w;
			return *this;
		}
		operator Float4() const
		{
			return Float4((f32)x, (f32)y, (f32)z, (f32)w);
		}

		constexpr explicit Vec4U(_Ty _x) :
			x(_x), y(_x), z(_x), w(_x) {}
		constexpr Vec4U(_Ty _x, _Ty _y, _Ty _z, _Ty _w) :
			x(_x), y(_y), z(_z), w(_w) {}

		bool operator==(const Vec4U& rhs) const
		{
			return (x == rhs.x) && (y == rhs.y) && (z == rhs.z) && (w == rhs.w);
		}
		bool operator!=(const Vec4U& rhs) const
		{
			return !(*this == rhs);
		}
	};

	using Float4U = Vec4U<f32>;
	using Int4U = Vec4U<i32>;
	using UInt4U = Vec4U<u32>;

	LUNA_RUNTIME_API typeinfo_t float2_type();
	LUNA_RUNTIME_API typeinfo_t float3_type();
	LUNA_RUNTIME_API typeinfo_t float4_type();

	LUNA_RUNTIME_API typeinfo_t vec2u_type();
	LUNA_RUNTIME_API typeinfo_t vec3u_type();
	LUNA_RUNTIME_API typeinfo_t vec4u_type();

	template <> struct typeof_t<Float2> { typeinfo_t operator()() const { return float2_type(); } };
	template <> struct typeof_t<Float3> { typeinfo_t operator()() const { return float3_type(); } };
	template <> struct typeof_t<Float4> { typeinfo_t operator()() const { return float4_type(); } };

	template <typename _Ty> struct typeof_t<Vec2U<_Ty>> { typeinfo_t operator()() const { return get_generic_instanced_type(vec2u_type(), { typeof<_Ty>() }); } };
	template <typename _Ty> struct typeof_t<Vec3U<_Ty>> { typeinfo_t operator()() const { return get_generic_instanced_type(vec3u_type(), { typeof<_Ty>() }); } };
	template <typename _Ty> struct typeof_t<Vec4U<_Ty>> { typeinfo_t operator()() const { return get_generic_instanced_type(vec4u_type(), { typeof<_Ty>() }); } };
}

#include "../Source/Math/Float2.inl"
#include "../Source/Math/Float3.inl"
#include "../Source/Math/Float4.inl"