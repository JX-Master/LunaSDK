#include <cppsl/core.hxx>
#include <cppsl/math.hxx>

using namespace cppsl;

struct VSInput
{
    [[cppsl::location(0)]] float3 position;
    [[cppsl::location(0)]] float2 texcoord;
};

[[cppsl::vertex]]
void main_vs(VSInput input)
{
}
