#include <cppsl/core.hxx>
#include <cppsl/math.hxx>

using namespace cppsl;

template <typename T>
struct Pair
{
    T left;
    T right;

    T Sum() const
    {
        return left + right;
    }
};

template <typename T>
class Box
{
public:
    T value;

    T Twice() const
    {
        return value + value;
    }
};

struct VSInput
{
    [[cppsl::location(0)]]
    float3 position;

    [[cppsl::location(1)]]
    float weight;
};

struct VSOutput
{
    [[cppsl::position]]
    float4 position;

    [[cppsl::location(0)]]
    float4 color;
};

[[cppsl::vertex]]
VSOutput main_vs(VSInput v)
{
    Pair<float> scalar;
    scalar.left = v.weight;
    scalar.right = 0.25f;

    Box<float> boxed;
    boxed.value = v.weight;

    Pair<float3> vector;
    vector.left = v.position;
    vector.right = float3(0.5f, 0.5f, 0.5f);

    VSOutput o;
    float combined = scalar.Sum() + boxed.Twice();
    float3 shifted = vector.Sum();
    o.position = float4(shifted, 1.0f);
    o.color = float4(shifted.x, shifted.y, shifted.z, combined);
    return o;
}
