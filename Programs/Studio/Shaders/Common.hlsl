static const float PI = 3.1415926f;

float2 get_latlong_from_dir(float3 dir)
{
    dir = normalize(dir);
    float H = dir.y;
    float texcoordV = 0.5 - asin(H) / PI; //(PI / 2.0 - asin(H)) / PI.
    float texcoordH = 0.5 - atan2(dir.z, dir.x) / (PI * 2);
    return float2(texcoordH, texcoordV);
}

float3 get_dir_from_latlong(float2 uv)
{
	float3 ret = 0;
	float theta = PI * (0.5 - uv.y); // [-PI/2, PI/2]
	ret.y = sin(theta);
	float cos_theta = cos(theta);
	ret.x = -cos(uv.x) * cos_theta;
	ret.z = sin(uv.x) * cos_theta;
	return normalize(ret);
}