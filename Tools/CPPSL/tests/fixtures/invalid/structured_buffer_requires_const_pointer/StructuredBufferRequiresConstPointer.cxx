#include <cppsl/core.hxx>

using namespace cppsl;

[[cppsl::structured_buffer, cppsl::desc_set(0), cppsl::binding(0)]]
float* values;

[[cppsl::vertex]]
void main_vs()
{
}
