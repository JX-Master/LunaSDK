#pragma once

namespace cppsl
{
    using bool_t = bool;
    using int_t = int;
    using uint = unsigned int;

    struct float2 { float x; float y; };
    struct float3 { float x; float y; float z; };
    struct float4 { float x; float y; float z; float w; };

    struct bool2 { bool x; bool y; };
    struct bool3 { bool x; bool y; bool z; };
    struct bool4 { bool x; bool y; bool z; bool w; };

    struct int2 { int x; int y; };
    struct int3 { int x; int y; int z; };
    struct int4 { int x; int y; int z; int w; };

    struct uint2 { uint x; uint y; };
    struct uint3 { uint x; uint y; uint z; };
    struct uint4 { uint x; uint y; uint z; uint w; };

    struct float4x4
    {
        float4 rows[4];
    };
}
