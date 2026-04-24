#include <cppsl/core.hxx>
#include <cppsl/texture.hxx>

using namespace cppsl;

[[cppsl::desc_set(0), cppsl::binding(0)]]
Texture2D<float4> color_tex;

[[cppsl::vertex]]
void main_vs()
{
}
