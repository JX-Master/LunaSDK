/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file ShaderCompiler.hpp
* @author JXMaster
* @date 2022/10/13
* @brief The cross-platform shader compiler interface.
*/
#pragma once
#include <Luna/Runtime/Span.hpp>
#include <Luna/Runtime/Path.hpp>
#include <Luna/Runtime/Result.hpp>

#ifndef LUNA_SHADER_COMPILER_API
#define LUNA_SHADER_COMPILER_API
#endif

namespace Luna
{
    namespace ShaderCompiler
    {
        //! @addtogroup ShaderCompiler Shader Compiler
        //! The ShaderCompiler module provides API to compile shaders.
        //! ShaderCompiler is designed to work with RHI, but it can also be used independently to implement shader compile command-line 
        //! tools without importing RHI module.
        //! @{
        
        //! The compile target to output.
        enum class TargetFormat : u8
        {
            //! Outputs nothing. This can be used if you only want to validate the input source code.
            none = 0,
            //! [Windows only] Outputs DirectX Intermediate Language for shader model 6.0 and newer.
            dxil,
            //! Outputs SPIR-V for Vulkan API.
            spir_v,
            //! Outputs Metal Shading Lauguage.
            msl,
        };

        //! The matrix pack mode.
        enum class MatrixPackMode : u8
        {
            //! Use column major pack mode.
            //! @details In column major pack mode, the matrix data is read as four columns, each column
            //! has four values arranged in memory continuously. This pack mode has slightly better performance
            //! than row major pack mode in certain hardware.
            column_major = 0,
            //! Use row major pack mode.
            //! @details In column major pack mode, the matrix data is read as four rows, each row
            //! has four values arranged in memory continuously.
            row_major = 1,
        };

        //! Specifies the shader compile type.
        enum class ShaderType : u8
        {
            //! Compiles the shader as vertex shader.
            vertex = 1,
            //! Compiles the shader as pixel (fragment) shader.
            pixel = 2,
            //! Compiles the shader as compute shader.
            compute = 3
        };
        
        //! Specifies the shader optimization level.
        enum class OptimizationLevel : u8
        {
            //! Do not perform any optimization. This can be used if 
            //! you want to debug shader code using shader debugging tools.
            none,
            //! Specifies shader optimization level 1.
            speed,
            //! Specifies shader optimization level 2.
            full
        };

        //! Specifies the HLSL shader model version used when compiling HLSL source code.
        struct ShaderModel
        {
            //! The shader model version major number.
            u32 major;
            //! The shader model version minor number.
            u32 minor;
        };

        //! Specifies the intended running platform for one metal shader.
        enum class MetalPlatform : u8
        {
            //! The shader is intended to be running on macOS.
            macos = 0,
            //! The shader is intended to be running on iOS.
            ios = 1
        };

        //! Describes one shader compile action.
        struct ShaderCompileParameters
        {
            //! The shader source data in HLSL or GLSL(SPIR-V) format.
            //! This is required for one shader compile action.
            Span<const c8> source;
            //! The source shader name.
            //! This will be used by the compiler and the debug tools to identify the shader if not empty.
            Name source_name;
            //! The platform-native shader source file path.
            //! This will be used by the compiler or debugger to resolve local include file and PDB file if not empty.
            Path source_file_path;
            //! The entry point function name of the shader. This must not be empty.
            Name entry_point = "main";
            //! The shader compile target format.
            //! If the target is @ref TargetFormat::none, no shader compilation is performed.
            TargetFormat target_format = TargetFormat::none;
            //! The type of the shader to compile.
            ShaderType shader_type = ShaderType::vertex;
            //! The shader model used for compiling shaders.
            ShaderModel shader_model = {6, 0};
            //! The optimization level used for compiling shaders.
            OptimizationLevel optimization_level = OptimizationLevel::full;
            //! Whether to add debug informations to the shader binary.
            bool debug = false;
            //! Whether to skip shader validation.
            bool skip_validation = false;
            //! The matrix pack mode when interpreting matrix data.
            MatrixPackMode matrix_pack_mode = MatrixPackMode::column_major;
            //! One array of paths that the compiler will use to find include files.
            Span<const Path> include_paths;
            //! One set of definitions the compiler will use when preprocessing shader files.
            Span<const Pair<Name, Name>> definitions;
            //! The target platform for one metal shader.
            //! This is used only if `target_format` is @ref TargetFormat::msl.
            MetalPlatform metal_platform = MetalPlatform::macos;
        };

        //! Describes shader compile result.
        struct ShaderCompileResult
        {
            //! The compiled shader data.
            Blob data;
            //! The format of the compiled data.
            TargetFormat format = TargetFormat::none;
            //! The shader entry point function name.
            //! @details This should be used instead of @ref ShaderCompileParameters::entry_point when specifying entry point
            //! in RHI APIs, since the compiler may marshall function names in source files, the entry point name may change 
            //! before and after compilation.
            Name entry_point;
            //! The number of threads for one thread group in X dimension.
            //! @details This is used only when the compile target is @ref TargetFormat::msl, since MSL does not record this 
            //! in shader code.
            u32 metal_numthreads_x = 0;
            //! The number of threads for one thread group in Y dimension.
            //! @details This is used only when the compile target is @ref TargetFormat::msl, since MSL does not record this 
            //! in shader code.
            u32 metal_numthreads_y = 0;
            //! The number of threads for one thread group in Z dimension.
            //! @details This is used only when the compile target is @ref TargetFormat::msl, since MSL does not record this 
            //! in shader code.
            u32 metal_numthreads_z = 0;
        };

        //! @interface ICompiler
        //! The compiler that compiles one shader source code into one target form.
        struct ICompiler : virtual Interface
        {
            luiid("{C2D6A83B-0B01-49AC-BFE4-94FAABBB5ACC}");

            //! Triggers compile for the source code.
            //! @param[in] params The parameters passed to the compiler.
            //! @return Returns the compile result.
            virtual R<ShaderCompileResult> compile(const ShaderCompileParameters& params) = 0;
        };

        //! Creates one new compiler.
        //! @return Returns the created compiler.
        LUNA_SHADER_COMPILER_API Ref<ICompiler> new_compiler();

        //! @}
    }

    struct Module;
    LUNA_SHADER_COMPILER_API Module* module_shader_compiler();
}