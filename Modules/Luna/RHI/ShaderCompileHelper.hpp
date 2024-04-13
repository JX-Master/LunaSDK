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
        //! @addtogroup RHI
        //! @{
        
        //! Gets the desired shader compile target format that can be used for the current graphics API.
        //! @return Returns the desired shader compile target format.
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

        //! Converts @ref ShaderCompiler::TargetFormat to @ref ShaderDataFormat.
        //! @param[in] format The shader compile target format to convert.
        //! @return Returns the converted shader data format.
        inline ShaderDataFormat get_shader_data_format_from_compile_target_format(ShaderCompiler::TargetFormat format)
        {
            switch(format)
            {
                case ShaderCompiler::TargetFormat::dxil: return ShaderDataFormat::dxil; break;
                case ShaderCompiler::TargetFormat::spir_v: return ShaderDataFormat::spirv; break;
                case ShaderCompiler::TargetFormat::msl: return ShaderDataFormat::msl; break;
            }
        }

        //! Gets one @ref ShaderData structure that referrs the specified compile result.
        //! @param[in] compile_result The shader compile result.
        //! @return Returns one @ref ShaderData structure that referrs the specified compile result.
        //! @par Valid Usage
        //! * `compile_result` must be valid so long as the returned shader data structure is valid, since the 
        //! shader data structure does not contain the compiled shader data, one refers to it.
        inline ShaderData get_shader_data_from_compile_result(const ShaderCompiler::ShaderCompileResult& compile_result)
        {
            ShaderData r;
            r.data = compile_result.data.cspan();
            r.entry_point = compile_result.entry_point;
            r.format = get_shader_data_format_from_compile_target_format(compile_result.format);
            return r;
        }
        //! Fills `cs`, `metal_numthreads_x`, `metal_numthreads_y` and `metal_numthreads_z` properties of 
        //! @ref ComputePipelineStateDesc using shader compile results.
        //! @param[out] desc The compute pipeline state descriptor to fill.
        //! @param[in] compile_result The compile result used for filling the descriptor.
        inline void fill_compute_pipeline_state_desc_from_compile_result(ComputePipelineStateDesc& desc, const ShaderCompiler::ShaderCompileResult& compile_result)
        {
            desc.cs = get_shader_data_from_compile_result(compile_result);
            desc.metal_numthreads_x = compile_result.metal_numthreads_x;
            desc.metal_numthreads_y = compile_result.metal_numthreads_y;
            desc.metal_numthreads_z = compile_result.metal_numthreads_z;
        }

        //! @}
    }
}

#define LUNA_GET_SHADER_DATA(_shader) Luna::RHI::ShaderData(Luna::Span<const Luna::byte_t>((const Luna::byte_t*)SHADER_DATA_##_shader, SHADER_DATA_SIZE_##_shader), Luna::Name(SHADER_ENTRY_POINT_##_shader), Luna::RHI::get_shader_data_format_from_compile_target_format(SHADER_DATA_FORMAT_##_shader))