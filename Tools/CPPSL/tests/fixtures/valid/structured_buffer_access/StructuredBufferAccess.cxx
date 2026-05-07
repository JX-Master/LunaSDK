#include <cppsl/core.hxx>
#include <cppsl/math.hxx>

using namespace cppsl;

struct LightParams
{
    float3 direction;
    float3 position;
};

struct LightSet
{
    [[cppsl::structured_buffer, cppsl::binding(0)]]
    const LightParams* lights;
};

[[cppsl::desc_set(0)]]
LightSet light_set;

[[cppsl::compute(1, 1, 1)]]
void main_cs([[cppsl::builtin(dispatch_thread_id)]] uint3 dispatch_thread_id)
{
    uint index = dispatch_thread_id.x;
    float3 world_position = float3{1.0f, 2.0f, 3.0f};
    float3 direction = -normalize(light_set.lights[index].direction);
    float3 to_light = -normalize(world_position - light_set.lights[index].position);
    float3 blend = lerp(direction, to_light, 0.5f);
    (void)blend;
}
