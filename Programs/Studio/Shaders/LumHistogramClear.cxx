#include <cppsl/core.hxx>

using namespace cppsl;

struct DescSet0
{
    [[cppsl::rwstructured_buffer, cppsl::binding(0)]]
    uint* g_dst_buffer;
};

[[cppsl::desc_set(0)]]
DescSet0 g_set0;

[[cppsl::compute(256, 1, 1)]]
void cs_main([[cppsl::builtin(group_index)]] uint group_index)
{
    g_set0.g_dst_buffer[group_index] = 0u;
}
