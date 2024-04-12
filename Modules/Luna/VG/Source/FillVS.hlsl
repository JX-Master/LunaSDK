struct TransformParams
{
    float4x4 transform;
};
TransformParams g_cbuffer : register(b0);
StructuredBuffer<float> g_commands : register(t1);
Texture2D g_tex : register(t2);
SamplerState g_sampler : register(s3);

struct VSIn
{
    [[vk::location(0)]]
    float2 position      : POSITION;
    [[vk::location(1)]]
    float2 shapecoord : SHAPECOORD;
    [[vk::location(2)]]
    float2 texcoord   : TEXCOORD;
    [[vk::location(3)]]
    float4 color      : COLOR;
    [[vk::location(4)]]
    uint begin_command_offset : COMMAND_OFFSET;
    [[vk::location(5)]]
    uint num_commands : NUM_COMMANDS;
};

struct VSOut
{
    [[vk::location(0)]]
    float4 position        : SV_POSITION;
    [[vk::location(1)]]
    float2 shapecoord    : SHAPECOORD;
    [[vk::location(2)]]
    float2 texcoord        : TEXCOORD;
    [[vk::location(3)]]
    float4 color        : COLOR;
    [[vk::location(4)]]
    uint begin_command_offset : COMMAND_OFFSET;
    [[vk::location(5)]]
    uint num_commands    : NUM_COMMANDS;
};

VSOut main(VSIn v)
{
    // Map position using position in instancedata.
    float4 pos = float4(v.position, 0.0f, 1.0f);
    pos = mul(g_cbuffer.transform, pos);
    VSOut o;
    o.position = pos;
    o.shapecoord = v.shapecoord;
    o.texcoord = v.texcoord;
    o.color = v.color;
    o.begin_command_offset = v.begin_command_offset;
    o.num_commands = v.num_commands;
    return o;
}