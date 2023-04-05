cbuffer VisualizationParams : register(b0)
{
    uint vis_type;
};

static const uint VIS_BASE_COLOR = 0;
static const uint VIS_NORMAL = 1;
static const uint VIS_ROUGHNESS = 2;
static const uint VIS_METALLIC = 3;
static const uint VIS_DEPTH = 4;

Texture2D<float4> g_base_color_roughness : register(t1);
Texture2D<float4> g_normal_metallic : register(t2);
Texture2D<float> g_depth : register(t3);

RWTexture2D<float4> g_dest : register(u4);

[numthreads(8, 8, 1)]
void main(int3 dispatch_thread_id: SV_DispatchThreadID)
{
    float4 base_color_roughness = g_base_color_roughness[dispatch_thread_id.xy];
    float3 base_color = base_color_roughness.xyz;
    float roughness = base_color_roughness.w;
    float4 normal_metallic = g_normal_metallic[dispatch_thread_id.xy];
    float3 normal = normalize(normal_metallic.xyz * 2.0f - 1.0f);
    float metallic = normal_metallic.w;
    float depth = g_depth[dispatch_thread_id.xy];

    if (vis_type == VIS_BASE_COLOR)
    {
        g_dest[dispatch_thread_id.xy] = float4(base_color, 1.0f);
    }
    else if (vis_type == VIS_NORMAL)
    {
        g_dest[dispatch_thread_id.xy] = float4(normal_metallic.xyz, 1.0f);
    }
    else if (vis_type == VIS_ROUGHNESS)
    {
        g_dest[dispatch_thread_id.xy] = float4(roughness, roughness, roughness, 1.0f);
    }
    else if (vis_type == VIS_METALLIC)
    {
        g_dest[dispatch_thread_id.xy] = float4(metallic, metallic, metallic, 1.0f);
    }
    else if (vis_type == VIS_DEPTH)
    {
        g_dest[dispatch_thread_id.xy] = float4(depth, depth, depth, 1.0f);
    }
}