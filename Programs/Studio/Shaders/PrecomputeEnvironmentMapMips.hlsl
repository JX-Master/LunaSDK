#include "IBLCommon.hlsl"
#include "BRDF.hlsl"
cbuffer CB : register(b0)
{
	float texel_size_x;	// 1.0 / destination dimension
	float texel_size_y;
	float roughness;
}
Texture2D<float4> g_src_mip : register(t1);
RWTexture2D<float4> g_dest_mip : register(u2);
SamplerState g_sampler : register(s3);

[numthreads(8, 8, 1)]
void main(uint3 dispatch_thread_id : SV_DispatchThreadID)
{
	//DTid is the thread ID * the values from numthreads above and in this case correspond to the pixels location in number of pixels.
	//As a result texcoords (in 0-1 range) will point at the center between the 4 pixels used for the mipmap.
	float2 texcoords = float2(texel_size_x, texel_size_y) * (dispatch_thread_id.xy + 0.5f);

	float3 normal = get_dir_from_latlong(texcoords);

	const uint num_samples = 1024;

	float3 prefilter_color = 0;
	float total_weight = 0;

	for(uint i = 0; i < num_samples; ++i)
	{
		float2 Xi = hammersley2d(i, num_samples);
		float3 h = importance_sample_ggx(Xi, normal, roughness);
		float3 l = normalize(2 * dot(normal, h) * h - normal);
		float n_dot_l = dot(normal, l);
		if (n_dot_l > 0)
		{
			float n_dot_h = max(1e-8, dot(normal, h));
			// Probability Distribution Function
			float pdf = ggx_normal_distrb(n_dot_h, roughness) / 4.0;

			float omegaS = 1.0 / ((float)num_samples * pdf);
			float omegaP = 4.0 * PI * texel_size_x * texel_size_y;

			float mip_level = 0.5 * log2(omegaS / omegaP) + 1.0;

			float3 radiance = g_src_mip.SampleLevel(g_sampler, get_latlong_from_dir(l), mip_level).xyz;
			prefilter_color += radiance * n_dot_l;
			total_weight += n_dot_l;
		}
	}
	prefilter_color /= total_weight;
	//Write the final color into the destination texture.
	g_dest_mip[dispatch_thread_id.xy] = float4(prefilter_color, 1.0f);
}