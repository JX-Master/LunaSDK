#include "IBLCommon.hlsl"
#include "BRDF.hlsl"
cbuffer CB : register(b0)
{
	uint tex_width;
	uint tex_height;
	uint mip_0_width;
	uint mip_0_height;
	float roughness;
}
Texture2D<float4> g_src_mip : register(t1);
RWTexture2D<float4> g_dst_mip : register(u2);
SamplerState g_sampler : register(s3);

[numthreads(8, 8, 1)]
void main(uint3 dispatch_thread_id : SV_DispatchThreadID)
{
	//DTid is the thread ID * the values from numthreads above and in this case correspond to the pixels location in number of pixels.
	//As a result texcoords (in 0-1 range) will point at the center between the 4 pixels used for the mipmap.
	float2 texcoords = float2(1.0 / (float)tex_width, 1.0 / (float)tex_height) * (dispatch_thread_id.xy + 0.5f);

	float3 normal = get_dir_from_latlong(texcoords);

	// Keep a constant sampling rate for all mips.
	uint num_samples = (uint)(8192.0 * roughness * roughness);
	//uint num_samples = 1024;
	float pixels_per_omega = ((float)mip_0_width * (float)mip_0_height) / (4.0 * PI);

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
			float pdf = ggx_normal_distrb(n_dot_h, roughness);

			float sample_omega = 2.0 * PI / (pdf * num_samples);

			float sample_pixels = pixels_per_omega * sample_omega;

			float mip_level = 0.5 * log2(sample_pixels);

			float3 radiance = g_src_mip.SampleLevel(g_sampler, get_latlong_from_dir(l), mip_level).xyz;
			prefilter_color += radiance * n_dot_l;
			total_weight += n_dot_l;
		}
	}
	prefilter_color /= total_weight;
	//Write the final color into the destination texture.
	g_dst_mip[dispatch_thread_id.xy] = float4(prefilter_color, 1.0f);
}