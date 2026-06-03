# CPPSL Shader Language

CPPSL is LunaSDK's C++ Shader Language. It uses a controlled subset of C++ syntax for shader code, but it is not host-side LunaSDK C++ code. CPPSL files are processed by the standalone CPPSL compiler, then lowered to HLSL, GLSL, Metal source, platform shader binaries, and reflection data.

CPPSL currently focuses on common graphics and compute shader workflows. Its goal is not to run full C++ on the GPU. Instead, it uses the C++ frontend for name lookup, overload resolution, template instantiation, and type checking, then translates a controlled shader semantic layer into each platform shader language.

## File Rules

CPPSL files must stay separate from regular C++ files:

- CPPSL header files use the `.hxx` extension.
- CPPSL source files use the `.cxx` extension.
- CPPSL files must not include regular LunaSDK C++ headers.
- Regular LunaSDK C++ files must not directly include CPPSL `.hxx` files.
- CPPSL code can only include headers from configured CPPSL include roots, such as `<cppsl/core.hxx>`.

Common headers:

```cpp
#include <cppsl/core.hxx>
#include <cppsl/math.hxx>
#include <cppsl/texture.hxx>
#include <cppsl/resource.hxx>
#include <cppsl/compute.hxx>

using namespace cppsl;
```

## Basic Types

CPPSL provides common shader scalar, vector, and matrix types:

```cpp
bool_t flag;
int_t i;
uint u;

float2 uv;
float3 normal;
float4 color;
float4x4 matrix;

int2 pixel;
uint3 dispatch_id;
bool4 mask;
```

Vectors support brace initialization, scalar splat construction, and vector-plus-scalar construction:

```cpp
float2 a = float2{1.0f, 2.0f};
float3 b = float3{a, 3.0f};
float4 c = float4{b, 1.0f};
float4 d = float4{1.0f};
```

Vectors support `xyzw` and `rgba` swizzles:

```cpp
float3 rgb = color.rgb;
float2 zw = color.zw;
float4 reversed = color.wzyx;
```

Do not mix swizzle naming sets. For example, prefer not to write `color.xgb`.

## Matrix ABI

CPPSL defines its cross-platform matrix ABI as shader-side column-major. Matrix fields written through `ConstantBuffer`, `StructuredBuffer`, and `RWStructuredBuffer` must be interpreted consistently by the HLSL, Vulkan GLSL, and Metal backends. The generated shader source must not depend on a platform compiler's implicit matrix packing default.

Backend rules:

- HLSL emits `#pragma pack_matrix(column_major)` and preserves CPPSL `mul(a, b)` calls.
- Vulkan GLSL emits `layout(std140, column_major)` for uniform buffers and `layout(std430, column_major)` for storage buffers.
- Metal uses MSL `float4x4` and follows the same CPPSL shader-side column-major contract.

Host code must follow LunaSDK's existing matrix upload convention. CPPSL does not implicitly transpose matrices in generated shader code, and it does not change the semantic direction of `mul(matrix, vector)` or `mul(vector, matrix)`.

## Stage Entry Points

Entry point functions must be marked with a stage attribute. Common stages:

```cpp
[[cppsl::vertex]]
VSOutput vs_main(VSInput v);

[[cppsl::fragment]]
PSOutput ps_main(PSInput p);

[[cppsl::pixel]]
PSOutput ps_main(PSInput p);

[[cppsl::compute(8, 8, 1)]]
void cs_main([[cppsl::builtin(dispatch_thread_id)]] uint3 dispatch_thread_id);
```

`fragment` and `pixel` are aliases for the pixel shader stage. The requested compile stage must match the stage declared on the entry point.

An entry point return type must be `void` or a CPPSL struct. Entry point parameters are usually CPPSL structs; compute shaders may also use builtin parameters marked with `cppsl::builtin`.

`input` and `output` are reserved parameter and local variable names in CPPSL.

## Stage IO

Vertex and pixel stage input/output values are represented by ordinary structs. Fields are bound to shader semantics using attributes:

```cpp
struct VSInput
{
    [[cppsl::location(0)]] float3 position;
    [[cppsl::location(1)]] float2 texcoord;
};

struct VSOutput
{
    [[cppsl::position]] float4 position;
    [[cppsl::location(1)]] float2 texcoord;
};

[[cppsl::vertex]]
VSOutput vs_main(VSInput v)
{
    VSOutput o;
    o.position = float4{v.position, 1.0f};
    o.texcoord = v.texcoord;
    return o;
}
```

One struct cannot declare duplicate `location` values, and it can only have one `cppsl::position` field.

## Descriptor Sets

CPPSL treats descriptor sets as first-class shader declarations. Resources must not be declared as scattered global resource variables. Instead, declare a descriptor set layout struct first, then declare one global variable marked with `cppsl::desc_set(n)`.

```cpp
struct FrameSet
{
    [[cppsl::cbuffer, cppsl::binding(0)]]
    Camera camera;

    [[cppsl::structured_buffer, cppsl::binding(1)]]
    const Vertex* vertices;

    [[cppsl::rwstructured_buffer, cppsl::binding(2)]]
    float4* output_values;

    [[cppsl::binding(3)]]
    Texture2D<float4> color_texture;

    [[cppsl::binding(4)]]
    SamplerState color_sampler;
};

[[cppsl::desc_set(0)]]
FrameSet frame;
```

Every field in a descriptor set layout must be a descriptor field and must declare `cppsl::binding(n)`. Field order does not need to be continuous or sorted by binding number; CPPSL backends handle platform-specific resource placement.

The global descriptor set variable should only declare `cppsl::desc_set(n)`. Do not add `cppsl::binding(n)` to the global descriptor set variable.

## Resource Types

Constant buffers are represented as ordinary struct fields in descriptor set layouts, marked with `cppsl::cbuffer`:

```cpp
struct Camera
{
    float4x4 world_to_proj;
};

struct FrameSet
{
    [[cppsl::cbuffer, cppsl::binding(0)]]
    Camera camera;
};

float4 clip = mul(frame.camera.world_to_proj, float4{position, 1.0f});
```

Structured buffers use `const T*`. RW structured buffers use `T*`:

```cpp
struct Light
{
    float3 position;
    float intensity;
};

struct FrameSet
{
    [[cppsl::structured_buffer, cppsl::binding(1)]]
    const Light* lights;

    [[cppsl::rwstructured_buffer, cppsl::binding(2)]]
    float4* output_values;
};

float3 p = frame.lights[index].position;
frame.output_values[index] = float4{p, 1.0f};
```

Non-RW structured buffers must be read-only. RW structured buffers must be writable.

Textures and samplers use CPPSL standard library types:

```cpp
Texture1D<float4> tex1d;
Texture2D<float4> tex2d;
Texture3D<float4> tex3d;
DepthTexture2D<float> depth;

RWTexture1D<float4> out1d;
RWTexture2D<float4> out2d;
RWTexture3D<float4> out3d;

SamplerState sampler0;
```

Common texture operations:

```cpp
float4 a = frame.tex2d.Sample(frame.sampler0, uv);
float4 b = frame.tex2d.SampleLevel(frame.sampler0, uv, 0.0f);
float4 c = frame.tex2d.Load(pixel);
float d = frame.depth.Load(pixel);

frame.out2d.Store(pixel, float4{1.0f});
```

## Compute Builtins

Compute shaders use `cppsl::compute(x, y, z)` to declare the thread group size and `cppsl::builtin(...)` to receive builtin inputs:

```cpp
[[cppsl::compute(16, 16, 1)]]
void cs_main(
    [[cppsl::builtin(dispatch_thread_id)]] uint3 dispatch_thread_id,
    [[cppsl::builtin(group_index)]] uint group_index)
{
    uint2 pixel = dispatch_thread_id.xy;
}
```

Currently supported compute builtins:

- `dispatch_thread_id`
- `group_index`

Group shared variables use `cppsl::group_shared`:

```cpp
[[cppsl::group_shared]]
uint bins[256];
```

Synchronization and atomic helpers are declared by `<cppsl/compute.hxx>`:

```cpp
GroupMemoryBarrierWithGroupSync();
InterlockedAdd(bins[index], 1u);
```

## Functions And Expressions

CPPSL supports ordinary free functions, local variables, control flow, function calls, member access, array/pointer indexing, and common arithmetic/comparison expressions.

```cpp
float luminance(float3 value)
{
    return dot(value, float3{0.2126f, 0.7152f, 0.0722f});
}

float3 normal_tangent_to_world(float3 normal_map, float3 normal_world, float3 tangent_world)
{
    float3 n = normal_world;
    float3 t = normalize(tangent_world - dot(tangent_world, n) * n);
    float3 b = cross(n, t);
    return normalize(t * normal_map.x + b * normal_map.y + n * normal_map.z);
}
```

Common math intrinsics from `<cppsl/math.hxx>` include:

```cpp
dot, cross, normalize, mul, sin, cos, tan, asin, atan2,
sqrt, pow, log2, exp2, abs, min, max, saturate, fwidth,
any, distance, reflect, lerp, clamp
```

CPPSL also supports common vector/scalar `+`, `-`, `*`, `/`, `+=`, `-=`, `*=`, and `/=` operators.

## Current Restrictions

CPPSL is a controlled C++ subset. Avoid the following:

- Including regular LunaSDK C++ headers.
- Declaring legacy global resources, such as a global `Texture2D<float4>` with `desc_set`/`binding`.
- Putting non-descriptor fields into a descriptor set layout.
- Using `input` or `output` as function parameter or local variable names.
- Exceptions, RTTI, virtual functions, lambdas, dynamic allocation, coroutines, or inline assembly.
- Arbitrary raw pointers and pointer arithmetic. Currently, raw pointer syntax is only allowed for structured buffer semantics through `const T*` / `T*`.
- Standard library containers or host-side runtime types.

If multiple shaders share the same resource layout, put the descriptor set layout into a `.hxx` header and include it from all related VS/PS/CS files. This avoids accidentally omitting resources that are not directly used by one shader but are still required by the pipeline layout.

## LunaBuild Integration

Add `.cxx` shaders in the target's `<Target>.Target.cs` rule through `Shader(sourceFile, stage, entryPoint)`:

```csharp
namespace LunaBuild.Core.Targets;

public sealed class MyProgramTargetRules : TargetRules
{
    public MyProgramTargetRules()
        : base("MyProgram", "Programs/MyProgram", "Programs/MyProgram/MyProgram.Target.cs")
    {
        Kind = BuildTargetKind.Executable;
        Sources("Source/**.cpp");
        Shader("Shaders/MyVS.cxx", "vertex", "vs_main");
        Shader("Shaders/MyPS.cxx", "pixel", "ps_main");
        DependsOn("Runtime", "RHI", "RHIUtility");
    }
}
```

`Shader(...)` selects the output format from the current RHI API:

- D3D12: DXIL
- Vulkan: SPIR-V
- Metal: metallib

Shader debug information is enabled automatically in Debug builds.

When debug is enabled, D3D12 keeps DXIL debug information, Vulkan emits SPIR-V nonsemantic debug information with embedded source for tools such as RenderDoc, and Metal emits source line information and records source text in the Metal compilation output.

The project build prepares the CPPSL compiler and native extractor before compiling shader files, so shader compilation does not fail because `cppslc` or `cppsl-native-extractor` is missing.
