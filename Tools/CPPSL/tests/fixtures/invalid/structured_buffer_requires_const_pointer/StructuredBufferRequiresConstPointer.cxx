#include <cppsl/core.hxx>

using namespace cppsl;

struct FrameSet
{
    [[cppsl::structured_buffer, cppsl::binding(0)]]
    float* values;
};

[[cppsl::desc_set(0)]]
FrameSet frame;

[[cppsl::vertex]]
void main_vs()
{
}
