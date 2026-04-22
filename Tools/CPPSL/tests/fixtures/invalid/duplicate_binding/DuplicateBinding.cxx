#include <cppsl/core.hxx>
#include <cppsl/resource.hxx>

using namespace cppsl;

struct Camera
{
    float4x4 world_to_proj;
};

[[cppsl::cbuffer, cppsl::set(0), cppsl::binding(0)]]
Camera camera_a;

[[cppsl::cbuffer, cppsl::set(0), cppsl::binding(0)]]
Camera camera_b;

[[cppsl::vertex]]
void main_vs()
{
}
