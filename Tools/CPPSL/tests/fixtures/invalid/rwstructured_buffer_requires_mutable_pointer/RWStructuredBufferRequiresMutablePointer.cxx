#include <cppsl/core.hxx>

using namespace cppsl;

struct FrameSet
{
    [[cppsl::rwstructured_buffer, cppsl::binding(0)]]
    const float* values;
};

[[cppsl::desc_set(0)]]
FrameSet frame;

[[cppsl::vertex]]
void main_vs()
{
}
