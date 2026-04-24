#include <cppsl/core.hxx>

using namespace cppsl;

struct Camera
{
    float4x4 world_to_proj;
};

struct ResourceSet
{
    [[cppsl::rwstructured_buffer, cppsl::binding(15)]]
    float* output_values;

    [[cppsl::cbuffer, cppsl::binding(0)]]
    Camera camera;

    [[cppsl::structured_buffer, cppsl::binding(8)]]
    const float* values;
};

[[cppsl::desc_set(0)]]
ResourceSet resources;

[[cppsl::vertex]]
void main_vs()
{
    resources.output_values[0] = resources.values[0];
}
