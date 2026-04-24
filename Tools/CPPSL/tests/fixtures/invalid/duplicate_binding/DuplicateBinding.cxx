#include <cppsl/core.hxx>
#include <cppsl/resource.hxx>

using namespace cppsl;

struct Camera
{
    float4x4 world_to_proj;
};

struct FrameSet
{
    [[cppsl::cbuffer, cppsl::binding(0)]]
    Camera camera_a;

    [[cppsl::cbuffer, cppsl::binding(0)]]
    Camera camera_b;
};

[[cppsl::desc_set(0)]]
FrameSet frame;

[[cppsl::vertex]]
void main_vs()
{
}
