#pragma once

namespace cppsl
{
    using bool_t = bool;
    using int_t = int;
    using uint = unsigned int;

    struct float2;
    struct float3;
    struct float4;
    struct bool2;
    struct bool3;
    struct bool4;
    struct int2;
    struct int3;
    struct int4;
    struct uint2;
    struct uint3;
    struct uint4;

    namespace detail
    {
        template <typename _Vector>
        struct swizzle
        {
            operator _Vector() const;
            swizzle& operator=(const _Vector& value);
        };
    }

#define CPPSL_SWIZZLE2(_vec2, _a, _b) detail::swizzle<_vec2> _a##_b;
#define CPPSL_SWIZZLE3(_vec3, _a, _b, _c) detail::swizzle<_vec3> _a##_b##_c;
#define CPPSL_SWIZZLE4(_vec4, _a, _b, _c, _d) detail::swizzle<_vec4> _a##_b##_c##_d;

#define CPPSL_SWIZZLE2_ROW2(_vec2, _a, _b, _r) \
        CPPSL_SWIZZLE2(_vec2, _r, _a) \
        CPPSL_SWIZZLE2(_vec2, _r, _b)
#define CPPSL_SWIZZLE2_SET2(_vec2, _a, _b) \
        CPPSL_SWIZZLE2_ROW2(_vec2, _a, _b, _a) \
        CPPSL_SWIZZLE2_ROW2(_vec2, _a, _b, _b)
#define CPPSL_SWIZZLE3_ROW2(_vec3, _a, _b, _r, _s) \
        CPPSL_SWIZZLE3(_vec3, _r, _s, _a) \
        CPPSL_SWIZZLE3(_vec3, _r, _s, _b)
#define CPPSL_SWIZZLE3_PLANE2(_vec3, _a, _b, _r) \
        CPPSL_SWIZZLE3_ROW2(_vec3, _a, _b, _r, _a) \
        CPPSL_SWIZZLE3_ROW2(_vec3, _a, _b, _r, _b)
#define CPPSL_SWIZZLE3_SET2(_vec3, _a, _b) \
        CPPSL_SWIZZLE3_PLANE2(_vec3, _a, _b, _a) \
        CPPSL_SWIZZLE3_PLANE2(_vec3, _a, _b, _b)
#define CPPSL_SWIZZLE4_ROW2(_vec4, _a, _b, _r, _s, _t) \
        CPPSL_SWIZZLE4(_vec4, _r, _s, _t, _a) \
        CPPSL_SWIZZLE4(_vec4, _r, _s, _t, _b)
#define CPPSL_SWIZZLE4_PLANE2(_vec4, _a, _b, _r, _s) \
        CPPSL_SWIZZLE4_ROW2(_vec4, _a, _b, _r, _s, _a) \
        CPPSL_SWIZZLE4_ROW2(_vec4, _a, _b, _r, _s, _b)
#define CPPSL_SWIZZLE4_CUBE2(_vec4, _a, _b, _r) \
        CPPSL_SWIZZLE4_PLANE2(_vec4, _a, _b, _r, _a) \
        CPPSL_SWIZZLE4_PLANE2(_vec4, _a, _b, _r, _b)
#define CPPSL_SWIZZLE4_SET2(_vec4, _a, _b) \
        CPPSL_SWIZZLE4_CUBE2(_vec4, _a, _b, _a) \
        CPPSL_SWIZZLE4_CUBE2(_vec4, _a, _b, _b)

#define CPPSL_SWIZZLE2_ROW3(_vec2, _a, _b, _c, _r) \
        CPPSL_SWIZZLE2(_vec2, _r, _a) \
        CPPSL_SWIZZLE2(_vec2, _r, _b) \
        CPPSL_SWIZZLE2(_vec2, _r, _c)
#define CPPSL_SWIZZLE2_SET3(_vec2, _a, _b, _c) \
        CPPSL_SWIZZLE2_ROW3(_vec2, _a, _b, _c, _a) \
        CPPSL_SWIZZLE2_ROW3(_vec2, _a, _b, _c, _b) \
        CPPSL_SWIZZLE2_ROW3(_vec2, _a, _b, _c, _c)
#define CPPSL_SWIZZLE3_ROW3(_vec3, _a, _b, _c, _r, _s) \
        CPPSL_SWIZZLE3(_vec3, _r, _s, _a) \
        CPPSL_SWIZZLE3(_vec3, _r, _s, _b) \
        CPPSL_SWIZZLE3(_vec3, _r, _s, _c)
#define CPPSL_SWIZZLE3_PLANE3(_vec3, _a, _b, _c, _r) \
        CPPSL_SWIZZLE3_ROW3(_vec3, _a, _b, _c, _r, _a) \
        CPPSL_SWIZZLE3_ROW3(_vec3, _a, _b, _c, _r, _b) \
        CPPSL_SWIZZLE3_ROW3(_vec3, _a, _b, _c, _r, _c)
#define CPPSL_SWIZZLE3_SET3(_vec3, _a, _b, _c) \
        CPPSL_SWIZZLE3_PLANE3(_vec3, _a, _b, _c, _a) \
        CPPSL_SWIZZLE3_PLANE3(_vec3, _a, _b, _c, _b) \
        CPPSL_SWIZZLE3_PLANE3(_vec3, _a, _b, _c, _c)
#define CPPSL_SWIZZLE4_ROW3(_vec4, _a, _b, _c, _r, _s, _t) \
        CPPSL_SWIZZLE4(_vec4, _r, _s, _t, _a) \
        CPPSL_SWIZZLE4(_vec4, _r, _s, _t, _b) \
        CPPSL_SWIZZLE4(_vec4, _r, _s, _t, _c)
#define CPPSL_SWIZZLE4_PLANE3(_vec4, _a, _b, _c, _r, _s) \
        CPPSL_SWIZZLE4_ROW3(_vec4, _a, _b, _c, _r, _s, _a) \
        CPPSL_SWIZZLE4_ROW3(_vec4, _a, _b, _c, _r, _s, _b) \
        CPPSL_SWIZZLE4_ROW3(_vec4, _a, _b, _c, _r, _s, _c)
#define CPPSL_SWIZZLE4_CUBE3(_vec4, _a, _b, _c, _r) \
        CPPSL_SWIZZLE4_PLANE3(_vec4, _a, _b, _c, _r, _a) \
        CPPSL_SWIZZLE4_PLANE3(_vec4, _a, _b, _c, _r, _b) \
        CPPSL_SWIZZLE4_PLANE3(_vec4, _a, _b, _c, _r, _c)
#define CPPSL_SWIZZLE4_SET3(_vec4, _a, _b, _c) \
        CPPSL_SWIZZLE4_CUBE3(_vec4, _a, _b, _c, _a) \
        CPPSL_SWIZZLE4_CUBE3(_vec4, _a, _b, _c, _b) \
        CPPSL_SWIZZLE4_CUBE3(_vec4, _a, _b, _c, _c)

#define CPPSL_SWIZZLE2_ROW4(_vec2, _a, _b, _c, _d, _r) \
        CPPSL_SWIZZLE2(_vec2, _r, _a) \
        CPPSL_SWIZZLE2(_vec2, _r, _b) \
        CPPSL_SWIZZLE2(_vec2, _r, _c) \
        CPPSL_SWIZZLE2(_vec2, _r, _d)
#define CPPSL_SWIZZLE2_SET4(_vec2, _a, _b, _c, _d) \
        CPPSL_SWIZZLE2_ROW4(_vec2, _a, _b, _c, _d, _a) \
        CPPSL_SWIZZLE2_ROW4(_vec2, _a, _b, _c, _d, _b) \
        CPPSL_SWIZZLE2_ROW4(_vec2, _a, _b, _c, _d, _c) \
        CPPSL_SWIZZLE2_ROW4(_vec2, _a, _b, _c, _d, _d)
#define CPPSL_SWIZZLE3_ROW4(_vec3, _a, _b, _c, _d, _r, _s) \
        CPPSL_SWIZZLE3(_vec3, _r, _s, _a) \
        CPPSL_SWIZZLE3(_vec3, _r, _s, _b) \
        CPPSL_SWIZZLE3(_vec3, _r, _s, _c) \
        CPPSL_SWIZZLE3(_vec3, _r, _s, _d)
#define CPPSL_SWIZZLE3_PLANE4(_vec3, _a, _b, _c, _d, _r) \
        CPPSL_SWIZZLE3_ROW4(_vec3, _a, _b, _c, _d, _r, _a) \
        CPPSL_SWIZZLE3_ROW4(_vec3, _a, _b, _c, _d, _r, _b) \
        CPPSL_SWIZZLE3_ROW4(_vec3, _a, _b, _c, _d, _r, _c) \
        CPPSL_SWIZZLE3_ROW4(_vec3, _a, _b, _c, _d, _r, _d)
#define CPPSL_SWIZZLE3_SET4(_vec3, _a, _b, _c, _d) \
        CPPSL_SWIZZLE3_PLANE4(_vec3, _a, _b, _c, _d, _a) \
        CPPSL_SWIZZLE3_PLANE4(_vec3, _a, _b, _c, _d, _b) \
        CPPSL_SWIZZLE3_PLANE4(_vec3, _a, _b, _c, _d, _c) \
        CPPSL_SWIZZLE3_PLANE4(_vec3, _a, _b, _c, _d, _d)
#define CPPSL_SWIZZLE4_ROW4(_vec4, _a, _b, _c, _d, _r, _s, _t) \
        CPPSL_SWIZZLE4(_vec4, _r, _s, _t, _a) \
        CPPSL_SWIZZLE4(_vec4, _r, _s, _t, _b) \
        CPPSL_SWIZZLE4(_vec4, _r, _s, _t, _c) \
        CPPSL_SWIZZLE4(_vec4, _r, _s, _t, _d)
#define CPPSL_SWIZZLE4_PLANE4(_vec4, _a, _b, _c, _d, _r, _s) \
        CPPSL_SWIZZLE4_ROW4(_vec4, _a, _b, _c, _d, _r, _s, _a) \
        CPPSL_SWIZZLE4_ROW4(_vec4, _a, _b, _c, _d, _r, _s, _b) \
        CPPSL_SWIZZLE4_ROW4(_vec4, _a, _b, _c, _d, _r, _s, _c) \
        CPPSL_SWIZZLE4_ROW4(_vec4, _a, _b, _c, _d, _r, _s, _d)
#define CPPSL_SWIZZLE4_CUBE4(_vec4, _a, _b, _c, _d, _r) \
        CPPSL_SWIZZLE4_PLANE4(_vec4, _a, _b, _c, _d, _r, _a) \
        CPPSL_SWIZZLE4_PLANE4(_vec4, _a, _b, _c, _d, _r, _b) \
        CPPSL_SWIZZLE4_PLANE4(_vec4, _a, _b, _c, _d, _r, _c) \
        CPPSL_SWIZZLE4_PLANE4(_vec4, _a, _b, _c, _d, _r, _d)
#define CPPSL_SWIZZLE4_SET4(_vec4, _a, _b, _c, _d) \
        CPPSL_SWIZZLE4_CUBE4(_vec4, _a, _b, _c, _d, _a) \
        CPPSL_SWIZZLE4_CUBE4(_vec4, _a, _b, _c, _d, _b) \
        CPPSL_SWIZZLE4_CUBE4(_vec4, _a, _b, _c, _d, _c) \
        CPPSL_SWIZZLE4_CUBE4(_vec4, _a, _b, _c, _d, _d)

#define CPPSL_SWIZZLES_SET2(_vec2, _vec3, _vec4, _a, _b) \
        CPPSL_SWIZZLE2_SET2(_vec2, _a, _b) \
        CPPSL_SWIZZLE3_SET2(_vec3, _a, _b) \
        CPPSL_SWIZZLE4_SET2(_vec4, _a, _b)
#define CPPSL_SWIZZLES_SET3(_vec2, _vec3, _vec4, _a, _b, _c) \
        CPPSL_SWIZZLE2_SET3(_vec2, _a, _b, _c) \
        CPPSL_SWIZZLE3_SET3(_vec3, _a, _b, _c) \
        CPPSL_SWIZZLE4_SET3(_vec4, _a, _b, _c)
#define CPPSL_SWIZZLES_SET4(_vec2, _vec3, _vec4, _a, _b, _c, _d) \
        CPPSL_SWIZZLE2_SET4(_vec2, _a, _b, _c, _d) \
        CPPSL_SWIZZLE3_SET4(_vec3, _a, _b, _c, _d) \
        CPPSL_SWIZZLE4_SET4(_vec4, _a, _b, _c, _d)

#define CPPSL_DEFINE_VECTOR2(_name, _scalar, _vec2, _vec3, _vec4) \
    struct _name \
    { \
        _scalar x; \
        _scalar y; \
        _scalar r; \
        _scalar g; \
        CPPSL_SWIZZLES_SET2(_vec2, _vec3, _vec4, x, y) \
        CPPSL_SWIZZLES_SET2(_vec2, _vec3, _vec4, r, g) \
    };

#define CPPSL_DEFINE_VECTOR3(_name, _scalar, _vec2, _vec3, _vec4) \
    struct _name \
    { \
        _scalar x; \
        _scalar y; \
        _scalar z; \
        _scalar r; \
        _scalar g; \
        _scalar b; \
        CPPSL_SWIZZLES_SET3(_vec2, _vec3, _vec4, x, y, z) \
        CPPSL_SWIZZLES_SET3(_vec2, _vec3, _vec4, r, g, b) \
    };

#define CPPSL_DEFINE_VECTOR4(_name, _scalar, _vec2, _vec3, _vec4) \
    struct _name \
    { \
        _scalar x; \
        _scalar y; \
        _scalar z; \
        _scalar w; \
        _scalar r; \
        _scalar g; \
        _scalar b; \
        _scalar a; \
        CPPSL_SWIZZLES_SET4(_vec2, _vec3, _vec4, x, y, z, w) \
        CPPSL_SWIZZLES_SET4(_vec2, _vec3, _vec4, r, g, b, a) \
    };

    CPPSL_DEFINE_VECTOR2(float2, float, float2, float3, float4)
    CPPSL_DEFINE_VECTOR3(float3, float, float2, float3, float4)
    CPPSL_DEFINE_VECTOR4(float4, float, float2, float3, float4)

    CPPSL_DEFINE_VECTOR2(bool2, bool, bool2, bool3, bool4)
    CPPSL_DEFINE_VECTOR3(bool3, bool, bool2, bool3, bool4)
    CPPSL_DEFINE_VECTOR4(bool4, bool, bool2, bool3, bool4)

    CPPSL_DEFINE_VECTOR2(int2, int, int2, int3, int4)
    CPPSL_DEFINE_VECTOR3(int3, int, int2, int3, int4)
    CPPSL_DEFINE_VECTOR4(int4, int, int2, int3, int4)

    CPPSL_DEFINE_VECTOR2(uint2, uint, uint2, uint3, uint4)
    CPPSL_DEFINE_VECTOR3(uint3, uint, uint2, uint3, uint4)
    CPPSL_DEFINE_VECTOR4(uint4, uint, uint2, uint3, uint4)

#undef CPPSL_DEFINE_VECTOR4
#undef CPPSL_DEFINE_VECTOR3
#undef CPPSL_DEFINE_VECTOR2
#undef CPPSL_SWIZZLES_SET4
#undef CPPSL_SWIZZLES_SET3
#undef CPPSL_SWIZZLES_SET2
#undef CPPSL_SWIZZLE4_SET4
#undef CPPSL_SWIZZLE4_CUBE4
#undef CPPSL_SWIZZLE4_PLANE4
#undef CPPSL_SWIZZLE4_ROW4
#undef CPPSL_SWIZZLE3_SET4
#undef CPPSL_SWIZZLE3_PLANE4
#undef CPPSL_SWIZZLE3_ROW4
#undef CPPSL_SWIZZLE2_SET4
#undef CPPSL_SWIZZLE2_ROW4
#undef CPPSL_SWIZZLE4_SET3
#undef CPPSL_SWIZZLE4_CUBE3
#undef CPPSL_SWIZZLE4_PLANE3
#undef CPPSL_SWIZZLE4_ROW3
#undef CPPSL_SWIZZLE3_SET3
#undef CPPSL_SWIZZLE3_PLANE3
#undef CPPSL_SWIZZLE3_ROW3
#undef CPPSL_SWIZZLE2_SET3
#undef CPPSL_SWIZZLE2_ROW3
#undef CPPSL_SWIZZLE4_SET2
#undef CPPSL_SWIZZLE4_CUBE2
#undef CPPSL_SWIZZLE4_PLANE2
#undef CPPSL_SWIZZLE4_ROW2
#undef CPPSL_SWIZZLE3_SET2
#undef CPPSL_SWIZZLE3_PLANE2
#undef CPPSL_SWIZZLE3_ROW2
#undef CPPSL_SWIZZLE2_SET2
#undef CPPSL_SWIZZLE2_ROW2
#undef CPPSL_SWIZZLE4
#undef CPPSL_SWIZZLE3
#undef CPPSL_SWIZZLE2

    struct float4x4
    {
        float4 rows[4];
    };
}
