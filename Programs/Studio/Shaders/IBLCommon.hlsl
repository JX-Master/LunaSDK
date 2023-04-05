#include "Common.hlsl"
/* for image-based lighting pre-computing */
float radical_inverse_VdC(uint bits) {
	bits = (bits << 16u) | (bits >> 16u);
	bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
	bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
	bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
	bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
	return float(bits) * 2.3283064365386963e-10; // / 0x100000000
}

float2 hammersley2d(uint i, uint n) {
	return float2(float(i) / float(n), radical_inverse_VdC(i));
}

float3 importance_sample_ggx(float2 Xi, float3 N, float roughness)
{
	float a = roughness * roughness;

	float phi = 2.0 * PI * Xi.x;
	float cosTheta = sqrt((1.0 - Xi.y) / (1.0 + (a*a - 1.0) * Xi.y));
	float sinTheta = sqrt(1.0 - cosTheta * cosTheta);

	// from spherical coordinates to cartesian coordinates
	float3 H;
	H[0] = cos(phi) * sinTheta;
	H[1] = sin(phi) * sinTheta;
	H[2] = cosTheta;

	// from tangent-space vector to world-space sample vector
	float3 up = abs(N.z) < 0.999 ? float3(0.0, 0.0, 1.0) : float3(1.0, 0.0, 0.0);
	float3 tangent = normalize(cross(up, N));
	float3 bitangent = cross(N, tangent);

	float3 sampleVec = tangent * H.x + bitangent * H.y + N * H.z;
	return normalize(sampleVec);
}

float schlick_ggx_geometry(float n_dot_v, float roughness)
{
	float r = (1 + roughness);
	float k = r * r / 8.0;
	k = roughness * roughness / 2.0f;
	return n_dot_v / (n_dot_v*(1 - k) + k);
}

float geometry_smith(float n_dot_v, float n_dot_l, float roughness)
{
	float g1 = schlick_ggx_geometry(n_dot_v, roughness);
	float g2 = schlick_ggx_geometry(n_dot_l, roughness);

	return g1 * g2;
}

float3 get_integrate_brdf(float n_dot_v, float roughness)
{
    float3 V = float3(0, sqrt(1.0 - n_dot_v * n_dot_v), n_dot_v);

	float A = 0.0;
	float B = 0.0;
	float C = 0.0;

	float3 N = float3(0.0, 0.0, 1.0);

	const int SAMPLE_COUNT = 1024;
	for (int i = 0; i < SAMPLE_COUNT; ++i)
	{
		// generates a sample vector that's biased towards the
		// preferred alignment direction (importance sampling).
		float2 Xi = hammersley2d(i, SAMPLE_COUNT);

		{ // A and B
			float3 H = importance_sample_ggx(Xi, N, roughness);
			float3 L = normalize(2.0 * dot(V, H) * H - V);

			float NdotL = max(L.z, 0.0);
			float NdotV = max(V.z, 0.0);
			float NdotH = max(H.z, 0.0);
			float VdotH = max(dot(V, H), 0.0);

			if (NdotL > 0.0)
			{
				float G = geometry_smith(NdotV, NdotL, roughness);
				float G_Vis = (G * VdotH) / (NdotH * NdotV);
				float Fc = pow(1.0 - VdotH, 5.0);

				A += (1.0 - Fc) * G_Vis;
				B += Fc * G_Vis;
			}
		}

	}

	return float3(A, B, C) / float(SAMPLE_COUNT);
}