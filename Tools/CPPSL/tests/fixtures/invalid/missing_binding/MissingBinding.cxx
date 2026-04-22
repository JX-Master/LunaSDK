#include <cppsl/core.hxx>
#include <cppsl/resource.hxx>

using namespace cppsl;

struct Camera
{
    float4x4 world_to_proj;
};

[[cppsl::set(0)]]
ConstantBuffer<Camera> camera;

[[cppsl::vertex]]
void main_vs()
{
}
