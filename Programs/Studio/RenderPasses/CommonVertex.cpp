/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file CommonVertex.cpp
* @author JXMaster
* @date 2023/3/7
*/
#include "CommonVertex.hpp"
#include <ShaderCompiler/ShaderCompiler.hpp>
#include <RHI/ShaderCompileHelper.hpp>
namespace Luna
{
    RV CommonVertex::init()
    {
        using namespace RHI;
        lutry
        {
            static const char* vertexShaderCommon =
				"cbuffer vertexBuffer : register(b0) \
						{\
							float4x4 world_to_view; \
							float4x4 view_to_proj; \
							float4x4 world_to_proj; \
							float4x4 view_to_world; \
							float4 env_light_color; \
						};\
						struct MeshBuffer	\
						{\
							float4x4 model_to_world;	\
							float4x4 world_to_model;	\
						};\
						StructuredBuffer<MeshBuffer> g_MeshBuffer : register(t1);\
						struct VS_INPUT\
						{\
							float3 position : POSITION;	\
							float3 normal : NORMAL;	\
							float3 tangent : TANGENT;	\
							float2 texcoord : TEXCOORD;	\
							float4 color : COLOR;	\
						};\
						\
						struct PS_INPUT\
						{\
							float4 position : SV_POSITION;	\
							float3 normal : NORMAL;	\
							float3 tangent : TANGENT;	\
							float2 texcoord : TEXCOORD;	\
							float4 color : COLOR;	\
							float3 world_position : POSITION;	\
						};\
						\
						PS_INPUT main(VS_INPUT input)\
						{\
							PS_INPUT output;\
							output.world_position = mul(g_MeshBuffer[0].model_to_world, float4(input.position, 1.0f)).xyz;\
							output.position = mul(world_to_proj, float4(output.world_position, 1.0f));\
							output.normal = mul(float4(input.normal, 0.0f), g_MeshBuffer[0].world_to_model).xyz;\
							output.tangent = mul(float4(input.tangent, 0.0f), g_MeshBuffer[0].world_to_model).xyz;\
							output.texcoord = input.texcoord;	\
							output.color = input.color;	\
							return output;\
						}";

			auto compiler = ShaderCompiler::new_compiler();
			compiler->set_source({ vertexShaderCommon, strlen(vertexShaderCommon) });
			compiler->set_source_name("MeshDebugVS");
			compiler->set_entry_point("main");
			compiler->set_target_format(get_current_platform_shader_target_format());
			compiler->set_shader_type(ShaderCompiler::ShaderType::vertex);
			compiler->set_shader_model(5, 0);
			compiler->set_optimization_level(ShaderCompiler::OptimizationLevel::full);
			luexp(compiler->compile());

			vs_blob = compiler->get_output();

			input_layout_common = RHI::InputLayoutDesc({
					InputElementDesc("POSITION", 0, Format::rgb32_float),
					InputElementDesc("NORMAL", 0, Format::rgb32_float),
					InputElementDesc("TANGENT", 0, Format::rgb32_float),
					InputElementDesc("TEXCOORD", 0, Format::rg32_float),
					InputElementDesc("COLOR", 0, Format::rgba32_float),
				});
        }
        lucatchret;
        return ok;
    }
}