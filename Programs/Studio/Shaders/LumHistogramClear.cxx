#include <cppsl/core.hxx>

using namespace cppsl;

[[cppsl::rwstructured_buffer, cppsl::desc_set(0), cppsl::binding(0)]]
uint* g_dst_buffer;

[[cppsl::compute(256, 1, 1)]]
void cs_main([[cppsl::builtin(group_index)]] uint group_index)
{
    g_dst_buffer[group_index] = 0u;
}
