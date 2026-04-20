#include "ShaderCompiler.h"

#include <Luna/Runtime/Blob.hpp>
#include <Luna/Runtime/Memory.hpp>
#include <Luna/Runtime/Module.hpp>
#include <Luna/Runtime/Ref.hpp>
#include <Luna/ShaderCompiler/ShaderCompiler.hpp>

#include <cstring>

namespace
{
luna_errcode_t from_errcode(Luna::ErrCode code)
{
    return static_cast<luna_errcode_t>(code.code);
}

luna_errcode_t from_result(const Luna::RV& result)
{
    return from_errcode(result.errcode());
}

char* duplicate_string(const char* value)
{
    if (!value)
    {
        value = "";
    }
    Luna::usize size = strlen(value);
    char* result = static_cast<char*>(Luna::memalloc(size + 1));
    if (!result)
    {
        return nullptr;
    }
    memcpy(result, value, size);
    result[size] = 0;
    return result;
}
}

extern "C"
{
LUNA_SHADER_COMPILER_C_API luna_errcode_t luna_shader_compiler_init_module(void)
{
    Luna::Module* module = Luna::module_shader_compiler();
    Luna::RV result = Luna::add_module(module);
    if (!result.valid())
    {
        return from_result(result);
    }
    result = Luna::init_module(module);
    return from_result(result);
}

LUNA_SHADER_COMPILER_C_API luna_errcode_t luna_shader_compiler_compile(
    const char* source,
    uint64_t source_size,
    const char* source_name,
    const char* source_file_path,
    const char* entry_point,
    uint32_t target_format,
    uint32_t shader_type,
    uint32_t shader_model_major,
    uint32_t shader_model_minor,
    uint32_t optimization_level,
    int32_t debug,
    int32_t skip_validation,
    uint32_t matrix_pack_mode,
    uint32_t metal_platform,
    LunaShaderCompileResult* out_result)
{
    if (!source || !entry_point || !out_result)
    {
        return from_errcode(Luna::BasicError::bad_arguments());
    }

    out_result->data = nullptr;
    out_result->data_size = 0;
    out_result->format = 0;
    out_result->entry_point = nullptr;
    out_result->metal_numthreads_x = 0;
    out_result->metal_numthreads_y = 0;
    out_result->metal_numthreads_z = 0;

    Luna::ShaderCompiler::ShaderCompileParameters params;
    params.source = Luna::Span<const Luna::c8>(source, static_cast<Luna::usize>(source_size));
    params.source_name = source_name ? source_name : "";
    if (source_file_path && source_file_path[0])
    {
        params.source_file_path = source_file_path;
    }
    params.entry_point = entry_point;
    params.target_format = static_cast<Luna::ShaderCompiler::TargetFormat>(target_format);
    params.shader_type = static_cast<Luna::ShaderCompiler::ShaderType>(shader_type);
    params.shader_model = {shader_model_major, shader_model_minor};
    params.optimization_level = static_cast<Luna::ShaderCompiler::OptimizationLevel>(optimization_level);
    params.debug = debug != 0;
    params.skip_validation = skip_validation != 0;
    params.matrix_pack_mode = static_cast<Luna::ShaderCompiler::MatrixPackMode>(matrix_pack_mode);
    params.metal_platform = static_cast<Luna::ShaderCompiler::MetalPlatform>(metal_platform);

    Luna::Ref<Luna::ShaderCompiler::ICompiler> compiler = Luna::ShaderCompiler::new_compiler();
    auto result = compiler->compile(params);
    if (!result.valid())
    {
        return from_errcode(result.errcode());
    }

    Luna::ShaderCompiler::ShaderCompileResult compile_result = Luna::move(result.get());
    out_result->data_size = static_cast<uint64_t>(compile_result.data.size());
    if (out_result->data_size)
    {
        void* data = Luna::memalloc(static_cast<Luna::usize>(out_result->data_size));
        if (!data)
        {
            return from_errcode(Luna::BasicError::out_of_memory());
        }
        memcpy(data, compile_result.data.data(), static_cast<Luna::usize>(out_result->data_size));
        out_result->data = data;
    }
    out_result->format = static_cast<uint32_t>(compile_result.format);
    out_result->entry_point = duplicate_string(compile_result.entry_point.c_str());
    if (!out_result->entry_point)
    {
        if (out_result->data)
        {
            Luna::memfree(out_result->data);
            out_result->data = nullptr;
            out_result->data_size = 0;
        }
        return from_errcode(Luna::BasicError::out_of_memory());
    }
    out_result->metal_numthreads_x = compile_result.metal_numthreads_x;
    out_result->metal_numthreads_y = compile_result.metal_numthreads_y;
    out_result->metal_numthreads_z = compile_result.metal_numthreads_z;
    return 0;
}

LUNA_SHADER_COMPILER_C_API void luna_shader_compiler_free_compile_result(LunaShaderCompileResult* result)
{
    if (!result)
    {
        return;
    }
    if (result->data)
    {
        Luna::memfree(result->data);
    }
    if (result->entry_point)
    {
        Luna::memfree(const_cast<char*>(result->entry_point));
    }
    result->data = nullptr;
    result->data_size = 0;
    result->format = 0;
    result->entry_point = nullptr;
    result->metal_numthreads_x = 0;
    result->metal_numthreads_y = 0;
    result->metal_numthreads_z = 0;
}
}
