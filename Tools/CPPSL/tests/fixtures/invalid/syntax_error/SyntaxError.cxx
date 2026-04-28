#include <cppsl/core.hxx>

using namespace cppsl;

struct BrokenInput
{
    float3 position
};

[[cppsl::vertex]]
void main_vs(BrokenInput v)
{
}
