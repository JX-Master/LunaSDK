#pragma once

#include "../Runtime/Runtime.h"

#include <stdint.h>

#if defined(_WIN32)
#define LUNA_SHADER_COMPILER_C_API __declspec(dllexport)
#else
#define LUNA_SHADER_COMPILER_C_API __attribute__((visibility("default")))
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct LunaShaderCompileResult
{
    void* data;
    uint64_t data_size;
    uint32_t format;
    const char* entry_point;
    uint32_t metal_numthreads_x;
    uint32_t metal_numthreads_y;
    uint32_t metal_numthreads_z;
} LunaShaderCompileResult;

LUNA_SHADER_COMPILER_C_API luna_errcode_t luna_shader_compiler_init_module(void);
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
    LunaShaderCompileResult* out_result);
LUNA_SHADER_COMPILER_C_API void luna_shader_compiler_free_compile_result(LunaShaderCompileResult* result);

#ifdef __cplusplus
}
#endif
