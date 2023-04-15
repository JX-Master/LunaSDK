cbuffer CameraParams : register(b0)
{
    float4x4 world_to_view;
    float4x4 view_to_proj;
    float4x4 world_to_proj;
    float4x4 proj_to_world;
    float4x4 view_to_world;
    uint screen_width;
    uint screen_height;
};