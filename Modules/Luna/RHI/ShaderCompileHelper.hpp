/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file ShaderCompileHelper.hpp
* @author JXMaster
* @date 2022/10/26
*/
#pragma once
#include "RHI.hpp"
#include <Luna/ShaderCompiler/ShaderCompiler.hpp>

namespace Luna
{
	namespace RHI
	{
		inline ShaderCompiler::TargetFormat get_current_platform_shader_target_format()
		{
			auto api_type = get_backend_type();
			switch (api_type)
			{
			case BackendType::d3d12:
				return ShaderCompiler::TargetFormat::dxil;
			case BackendType::vulkan:
				return ShaderCompiler::TargetFormat::spir_v;
            case BackendType::metal:
                return ShaderCompiler::TargetFormat::msl;
			}
			lupanic();
			return ShaderCompiler::TargetFormat::none;
		}
	}
}
