/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file FillShader.cpp
* @author JXMaster
* @date 2022/4/25
*/
#include "ShapeRenderer.hpp"

namespace Luna
{
	namespace VG
	{
		const c8 FILL_SHADER_SOURCE_VS[] = R"(
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
	float2 position	  : POSITION;
	[[vk::location(1)]]
	float2 shapecoord : SHAPECOORD;
	[[vk::location(2)]]
	float2 texcoord   : TEXCOORD;
	[[vk::location(3)]]
	float4 color	  : COLOR;
	[[vk::location(4)]]
	uint begin_command_offset : COMMAND_OFFSET;
	[[vk::location(5)]]
	uint num_commands : NUM_COMMANDS;
};

struct VSOut
{
	[[vk::location(0)]]
	float4 position		: SV_POSITION;
	[[vk::location(1)]]
	float2 shapecoord	: SHAPECOORD;
	[[vk::location(2)]]
	float2 texcoord		: TEXCOORD;
	[[vk::location(3)]]
	float4 color		: COLOR;
	[[vk::location(4)]]
	uint begin_command_offset : COMMAND_OFFSET;
	[[vk::location(5)]]
	uint num_commands	: NUM_COMMANDS;
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
})";
		usize FILL_SHADER_SOURCE_VS_SIZE = sizeof(FILL_SHADER_SOURCE_VS);

		const c8 FILL_SHADER_SOURCE_PS[] = R"(
struct TransformParams
{
    float4x4 transform;
};
TransformParams g_cbuffer : register(b0);
StructuredBuffer<float> g_commands : register(t1);
Texture2D g_tex : register(t2);
SamplerState g_sampler : register(s3);

struct PSIn
{
	[[vk::location(0)]]
	float4 position		: SV_POSITION;
	[[vk::location(1)]]
	float2 shapecoord	: SHAPECOORD;
	[[vk::location(2)]]
	float2 texcoord		: TEXCOORD;
	[[vk::location(3)]]
	float4 color		: COLOR;
	[[vk::location(4)]]
	uint begin_command_offset : COMMAND_OFFSET;
	[[vk::location(5)]]
	uint num_commands	: NUM_COMMANDS;
};

static float DENOMINATOR_EPSILON = 0.0001220703125f;

float line_test_x_axis(float2 v0, float2 v1, float2 pixels_per_unit)
{
	// Early out if this line does not touch this pixel.
	if (max(v0.x, v1.x) * pixels_per_unit.x < -0.5f) return 0.0f;
	//  0 if (y1 >  0 && y2 >  0) or (y1 <= 0 && y2 <= 0)
	//  1 if  y1 >  0 && y2 <= 0
	// -1 if  y1 <= 0 && y2 >  0
	int sign = ((v0.y > 0) ? 1 : 0) - ((v1.y > 0) ? 1 : 0);
	// Early out if not intersect with x.
	if (sign == 0) return 0.0f;
	// Compute the x coord of the intersection point.
	float xt = (v1.y * v0.x - v0.y * v1.x) / (v1.y - v0.y);
	return sign * saturate(xt * pixels_per_unit.x + 0.5f);
}

float line_test_y_axis(float2 v0, float2 v1, float2 pixels_per_unit)
{
	if (max(v0.y, v1.y) * pixels_per_unit.y < -0.5f) return 0.0f;
	int sign = ((v1.x > 0) ? 1 : 0) - ((v0.x > 0) ? 1 : 0);
	if (sign == 0) return 0.0f;
	float yt = (v1.x * v0.y - v0.x * v1.y) / (v1.x - v0.x);
	return sign * saturate(yt * pixels_per_unit.y + 0.5f);
}

float2 quad_curve_solve_x_axis(float2 v0, float2 v1, float2 v2)
{
	// Compute two points.
	float2 a = v0 - 2.0f * v1 + v2;
	float2 b = v0 - v1;
	float c = v0.y;
	float ra = 1.0f / a.y;
	float rb = 0.5f / b.y;
	// Clamp b^2 - ac > 0 to handle non-real root uniformly.
	float delta = sqrt(max(b.y * b.y - a.y * c, 0.0f));
	// t1 and t2 may be `Inf` if a -> 0 and b == 0 (the curve is parallel to x axis), but 
	// such case will be clipped because flags are 0.
	float2 t = float2((b.y - delta) * ra, (b.y + delta) * ra);
	if (abs(a.y) < 0.0001220703125) t = float2(c * rb, c * rb);
	// return x1, x2.
	return (a.x * t - b.x * 2.0) * t + v0.x;
}

float2 quad_curve_solve_y_axis(float2 v0, float2 v1, float2 v2)
{
	float2 a = v0 - 2.0f * v1 + v2;
	float2 b = v0 - v1;
	float c = v0.x;
	float ra = 1.0f / a.x;
	float rb = 0.5f / b.x;
	float delta = sqrt(max(b.x * b.x - a.x * c, 0.0f));
	float2 t = float2((b.x - delta) * ra, (b.x + delta) * ra);
	if (abs(a.x) < 0.0001220703125) t = float2(c * rb, c * rb);
	return (a.y * t - b.y * 2.0) * t + v0.y;
}

int get_curve_root_flags(float v0, float v1, float v2)
{
	int shift = ((v0 > 0) ? 2 : 0) + ((v1 > 0) ? 4 : 0) + ((v2 > 0) ? 8 : 0);
	return (0x2E74 >> shift) & 0x03;
}

float curve_test_x_axis(float2 v0, float2 v1, float2 v2, float2 pixels_per_unit)
{
	// Early out if this curve is before the pixel.
	if (max(max(v0.x, v1.x), v2.x) * pixels_per_unit.x < -0.5f) return 0.0f;
	int flags = get_curve_root_flags(v0.y, v1.y, v2.y);
	// Early out if this curve does not touch the pixel.
	if (flags == 0) return 0.0f;
	float2 x1x2 = quad_curve_solve_x_axis(v0, v1, v2) * pixels_per_unit.x;
	float ret = 0.0f;
	if ((flags & 0x01) != 0)
	{
		// use x1.
		ret += saturate(x1x2.x + 0.5f);
	}
	if ((flags & 0x02) != 0)
	{
		// use x2.
		ret -= saturate(x1x2.y + 0.5f);
	}
	return ret;
}

float curve_test_y_axis(float2 v0, float2 v1, float2 v2, float2 pixels_per_unit)
{
	if (max(max(v0.y, v1.y), v2.y) * pixels_per_unit.y < -0.5f) return 0.0f;
	int flags = get_curve_root_flags(v0.x, v1.x, v2.x);
	if (flags == 0) return 0.0f;
	float2 y1y2 = quad_curve_solve_y_axis(v0, v1, v2) * pixels_per_unit.y;
	float ret = 0.0f;
	if ((flags & 0x01) != 0)
	{
		// use x1.
		ret -= saturate(y1y2.x + 0.5f);
	}
	if ((flags & 0x02) != 0)
	{
		// use x2.
		ret += saturate(y1y2.y + 0.5f);
	}
	return ret;
}

static const float PI = 3.141592654f;
static const float PI_DIV_180 = PI / 180.0f;
static const float INV_PI_DIV_180 = 180.0f / PI;

float2 circle_get_point(float2 center, float radius, float angle)
{
	angle = angle * PI_DIV_180;
	float2 sc;
	sincos(angle, sc.y, sc.x);
	return center + radius * sc;
}

float circle_test_x_axis(float2 v0, float2 v1, float2 center, float radius, bool choose_first, float2 pixels_per_unit)
{
	// Early out if this curve is before the pixel.
	if (max(v0.x, v1.x) * pixels_per_unit.x < -0.5f) return 0.0f;
	//  0 if (y1 >  0 && y2 >  0) or (y1 <= 0 && y2 <= 0)
	//  1 if  y1 >  0 && y2 <= 0
	// -1 if  y1 <= 0 && y2 >  0
	int sign = ((v0.y > 0) ? 1 : 0) - ((v1.y > 0) ? 1 : 0);
	// Early out if not intersect with x.
	if (sign == 0) return 0.0f;
	// Compute the x coord of the intersection point.
	float delta = sqrt(radius * radius - center.y * center.y);
	float xt = choose_first ? center.x - delta : center.x + delta;
	return sign * saturate(xt * pixels_per_unit.x + 0.5f);
}

float circle_test_y_axis(float2 v0, float2 v1, float2 center, float radius, bool choose_first, float2 pixels_per_unit)
{
	if (max(v0.y, v1.y) * pixels_per_unit.y < -0.5f) return 0.0f;
	int sign = ((v1.x > 0) ? 1 : 0) - ((v0.x > 0) ? 1 : 0);
	if (sign == 0) return 0.0f;
	float delta = sqrt(radius * radius - center.x * center.x);
	float yt = choose_first ? center.y - delta : center.y + delta;
	return sign * saturate(yt * pixels_per_unit.y + 0.5f);
}


static const float COMMAND_MOVE_TO = 1.0f;
static const float COMMAND_LINE_TO = 2.0f;
static const float COMMAND_CURVE_TO = 3.0f;
static const float COMMAND_CIRCLE_Q1 = 4.0f;
static const float COMMAND_CIRCLE_Q2 = 5.0f;
static const float COMMAND_CIRCLE_Q3 = 6.0f;
static const float COMMAND_CIRCLE_Q4 = 7.0f;

[[vk::location(0)]]
float4 main(PSIn v) : SV_Target
{
	float2 units_per_pixel = fwidth(v.shapecoord);
	float2 pixels_per_unit = 1.0f / units_per_pixel;
	float coverage_x = 0.0f;
	float coverage_y = 0.0f;
	uint i = v.begin_command_offset;
	uint end = i + v.num_commands;
	float2 last_point = 0.0f;
	bool hit_test = false;
	while (i < end)
	{
		// Read next command.
		float command = g_commands[i];
		if (command == COMMAND_MOVE_TO)
		{
			last_point = float2(g_commands[i + 1], g_commands[i + 2]);
			i += 3;
		}
		else if (command == COMMAND_LINE_TO)
		{
			float2 v0 = last_point - v.shapecoord;
			last_point = float2(g_commands[i + 1], g_commands[i + 2]);
			float2 v1 = last_point - v.shapecoord;
			coverage_x += line_test_x_axis(v0, v1, pixels_per_unit);
			coverage_y += line_test_y_axis(v0, v1, pixels_per_unit);
			i += 3;
		}
		else if (command == COMMAND_CURVE_TO)
		{
			float2 v0 = last_point - v.shapecoord;
			float2 v1 = float2(g_commands[i + 1], g_commands[i + 2]) - v.shapecoord;
			last_point = float2(g_commands[i + 3], g_commands[i + 4]);
			float2 v2 = last_point - v.shapecoord;
			coverage_x += curve_test_x_axis(v0, v1, v2, pixels_per_unit);
			coverage_y += curve_test_y_axis(v0, v1, v2, pixels_per_unit);
			i += 5;
		}
		else if(command >= COMMAND_CIRCLE_Q1 && command <= COMMAND_CIRCLE_Q4)
		{
			float2 v0 = last_point - v.shapecoord;
			float radius = g_commands[i + 1];
			float begin = g_commands[i + 2];
			float end = g_commands[i + 3];
			float2 center = circle_get_point(last_point, radius, 180.0f + begin);
			last_point = circle_get_point(center, radius, end);
			float2 v1 = last_point - v.shapecoord;
			center = center - v.shapecoord;
			coverage_x += circle_test_x_axis(v0, v1, center, radius, (command == COMMAND_CIRCLE_Q2) || (command == COMMAND_CIRCLE_Q3), pixels_per_unit);
			coverage_y += circle_test_y_axis(v0, v1, center, radius, (command == COMMAND_CIRCLE_Q3) || (command == COMMAND_CIRCLE_Q4), pixels_per_unit);
			i += 4;
		}
	}
	float weight_x = 1.0f - abs(coverage_x * 2.0f - 1.0f);
	float weight_y = 1.0f - abs(coverage_y * 2.0f - 1.0f);
	float coverage = max(abs(coverage_x * weight_x + coverage_y * weight_y) / max(weight_x + weight_y, 0.0001220703125f), min(abs(coverage_x), abs(coverage_y)));
	float4 col = g_tex.Sample(g_sampler, v.texcoord);
	col *= v.color;
	col.w *= coverage;
	return col;
}
)";
		usize FILL_SHADER_SOURCE_PS_SIZE = sizeof(FILL_SHADER_SOURCE_PS);
	}
}
