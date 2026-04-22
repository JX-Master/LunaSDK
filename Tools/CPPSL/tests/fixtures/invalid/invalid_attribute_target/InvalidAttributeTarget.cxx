#include <cppsl/core.hxx>
#include <cppsl/resource.hxx>

using namespace cppsl;

struct Camera
{
    [[cppsl::set(0)]] float4x4 world_to_proj;
};

[[cppsl::vertex]]
void main_vs()
{
}
