#include <cppsl/core.hxx>

using namespace cppsl;

struct Camera
{
    float4x4 world_to_proj;
};

[[cppsl::cbuffer, cppsl::set(0), cppsl::binding(0)]]
Camera camera;

[[cppsl::structured_buffer, cppsl::set(0), cppsl::binding(1)]]
const float* values;

[[cppsl::rwstructured_buffer, cppsl::set(0), cppsl::binding(2)]]
float* output_values;

[[cppsl::vertex]]
void main_vs()
{
    output_values[0] = values[0];
}
