#include <cppsl/core.hxx>

using namespace cppsl;

struct DescSet0
{
    [[cppsl::rwstructured_buffer, cppsl::binding(0)]]
    uint* g_output;
};

[[cppsl::desc_set(0)]]
DescSet0 g_set0;

[[cppsl::compute(4, 1, 1)]]
void cs_main([[cppsl::builtin(group_index)]] uint group_index)
{
    g_set0.g_output[group_index] = group_index * 17u + 5u;
}
