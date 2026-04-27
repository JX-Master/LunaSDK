#include <cppsl/core.hxx>

using namespace cppsl;

struct VSInput
{
    [[cppsl::location(0)]]
    float3 position;
};

[[cppsl::vertex]]
void main_vs(VSInput input)
{
}
