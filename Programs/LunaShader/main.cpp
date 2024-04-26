/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file main.cpp
* @author JXMaster
* @date 2024/4/8
*/
#include <Luna/Runtime/Runtime.hpp>
#include <Luna/Runtime/Module.hpp>
#include <Luna/Runtime/Log.hpp>
#include <Luna/Runtime/StdIO.hpp>
#include <Luna/Runtime/Path.hpp>
#include <Luna/Runtime/File.hpp>
#include <Luna/ShaderCompiler/ShaderCompiler.hpp>
using namespace Luna;
RV print_help()
{
    const c8 help_text[] = R"(LunaShader v0.0.1
Shader Compiler for LunaSDK
This program compiles HLSL shader source code to multiple shader binary forms 
required by LunaSDK.
Usage LunaShader <source> [options]
<source>: The path of the source file to compile.
Options:
    -o <./output> Sets the output file.
        default: Outputs to $source$.cso on the current working directory.
    -f <dxil|spir_v|msl> Sets the target format.
        dxil: [Windows only] Outputs DirectX Intermediate Language for shader model 6.0 and newer.
        spir_v: Outputs SPIR-V for Vulkan API.
        msl: Outputs Metal Shading Lauguage.
        default: `dxil` on Windows, `msl` on macOS.
    -t <vertex|pixel|compute> Sets the shader type.
        vertex: Compiles vertex shader.
        pixel: Compiles pixel/fragment shader.
        compute: Compiles compute shader.
        default: vertex
    -e <entry_point> Sets the entry point function name.
        default: main
    --optimize <none|1|2> Sets the shader optimization level.
        none: Do not perform any optimization. This can be used if 
            you want to debug shader code using shader debugging tools.
        1: Specifies shader optimization level 1.
        2: Specifies shader optimization level 2.
        default: 2
    -sm <X_X> Sets the HLSL shader model version used when compiling HLSL source code.
        default: 6_0
    -i <path> Adds include search paths. 
        This option can be specified multiple times to add multiple include paths.
    -d <DEF|DEF=VALUE> Adds definitions. 
        This option can be specified multiple times to add multiple definitions.
    --debug Keeps debug information in the compiled shader.
    --skip_validation Skips shader validation.
    --matrix_pack <column|row> Sets the matrix pack mode.
        column: Use column major pack mode.
        row: Use row major pack mode.
        default: column
    --metal_platform <macos|ios> Sets the intended running platform for one metal shader.
        macos: The shader is intended to be running on macOS.
        ios: The shader is intended to be running on iOS/iPadOS.
        default: macos
    --cpp_output Converts the shader output to C++ byte array, so that it can be embedded to the program directly.)";
    auto io = get_std_io_stream();
    return io->write(help_text, sizeof(help_text));
}

RV run(int argc, const char* argv[])
{
    using namespace ShaderCompiler;
    lutry
    {
        set_log_to_platform_enabled(true);
        set_log_to_platform_verbosity(LogVerbosity::info);
        auto io = get_std_io_stream();
        luexp(add_module(module_shader_compiler()));
        luexp(init_modules());
        if(argc < 2)
        {
            const c8 usage[] = "Usage: LunaShader <source> [options]\nType \"LunaShader --help\" for details.";
            luexp(io->write(usage, sizeof(usage)));
            return ok;
        }
        auto source = Name(argv[1]);
        if(source == "-h" || source == "--help")
        {
            luexp(print_help());
            return ok;
        }
        // load source shader data.
        lulet(f, open_file(source.c_str(), FileOpenFlag::read, FileCreationMode::open_existing));
        lulet(shader_data, load_file_data(f));
        ShaderCompileParameters params;
        params.source = {(const c8*)shader_data.data(), shader_data.size()};
        params.source_file_path = source;
        params.source_name = params.source_file_path.filename();
        params.entry_point = "main";
#if LUNA_PLATFORM_WINDOWS
        params.target_format = TargetFormat::dxil;
#elif LUNA_PLATFORM_MACOS
        params.target_format = TargetFormat::msl;
#else
        return set_error(BasicError::not_supported(), "LunaShader does not support the current platform.");
#endif
        params.shader_type = ShaderType::vertex;
        params.shader_model = {6, 0};
        params.optimization_level = OptimizationLevel::full;
        params.debug = false;
        params.skip_validation = false;
        params.matrix_pack_mode = MatrixPackMode::column_major;
        params.metal_platform = MetalPlatform::macos;
        Name output_filename = strprintf("%s.cso", params.source_name.c_str());
        Vector<Path> include_paths;
        Vector<Pair<Name, Name>> definitions;
        bool cpp_output = false;
        // load additional parameters.
        int argi = 2;
        while(argi < argc)
        {
            const c8* option = argv[argi];
            if(strcmp(option, "-o") == 0)
            {
                ++argi;
                if(argi == argc) return set_error(BasicError::bad_arguments(), "Missing arguments of -o");
                output_filename = argv[argi];
            }
            else if(strcmp(option, "-f") == 0)
            {
                ++argi;
                if(argi == argc) return set_error(BasicError::bad_arguments(), "Missing arguments of -f");
                const c8* format = argv[argi];
                if(strcmp(format, "dxil") == 0)
                {
                    params.target_format = TargetFormat::dxil;
                }
                else if(strcmp(format, "spir_v") == 0)
                {
                    params.target_format = TargetFormat::spir_v;
                }
                else if(strcmp(format, "msl") == 0)
                {
                    params.target_format = TargetFormat::msl;
                }
                else
                {
                    set_error(BasicError::bad_arguments(), "Unknown argument for -f: %s", format);
                }
            }
            else if(strcmp(option, "-t") == 0)
            {
                ++argi;
                if(argi == argc) return set_error(BasicError::bad_arguments(), "Missing arguments of -t");
                const c8* type = argv[argi];
                if(strcmp(type, "vertex") == 0)
                {
                    params.shader_type = ShaderType::vertex;
                }
                else if(strcmp(type, "pixel") == 0)
                {
                    params.shader_type = ShaderType::pixel;
                }
                else if(strcmp(type, "compute") == 0)
                {
                    params.shader_type = ShaderType::compute;
                }
                else
                {
                    set_error(BasicError::bad_arguments(), "Unknown argument for -t: %s", type);
                }
            }
            else if(strcmp(option, "-e") == 0)
            {
                ++argi;
                if(argi == argc) return set_error(BasicError::bad_arguments(), "Missing arguments of -e");
                const c8* entry = argv[argi];
                params.entry_point = entry;
            }
            else if(strcmp(option, "--optimize") == 0)
            {
                ++argi;
                if(argi == argc) return set_error(BasicError::bad_arguments(), "Missing arguments of --optimize");
                const c8* opt = argv[argi];
                if(strcmp(opt, "none") == 0)
                {
                    params.optimization_level = OptimizationLevel::none;
                }
                else if(strcmp(opt, "1") == 0)
                {
                    params.optimization_level = OptimizationLevel::speed;
                }
                else if(strcmp(opt, "2") == 0)
                {
                    params.optimization_level = OptimizationLevel::full;
                }
                else
                {
                    set_error(BasicError::bad_arguments(), "Unknown argument for --optimize: %s", opt);
                }
            }
            else if(strcmp(option, "-sm") == 0)
            {
                ++argi;
                if(argi == argc) return set_error(BasicError::bad_arguments(), "Missing arguments of -sm");
                const c8* sm = argv[argi];
                c8* end = nullptr;
                u32 major = (u32)strtoul(sm, &end, 10);
                sm = end + 1;
                u32 minor = (u32)strtoul(sm, nullptr, 10);
                params.shader_model = {major, minor};
            }
            else if(strcmp(option, "-i") == 0)
            {
                ++argi;
                if(argi == argc) return set_error(BasicError::bad_arguments(), "Missing arguments of -i");
                const c8* path = argv[argi];
                include_paths.push_back(path);
            }
            else if(strcmp(option, "-d") == 0)
            {
                ++argi;
                if(argi == argc) return set_error(BasicError::bad_arguments(), "Missing arguments of -d");
                const c8* def = argv[argi];
                auto equal = strchr(def, '=');
                if(equal)
                {
                    Name key(def, equal - def);
                    Name value(equal + 1);
                    definitions.push_back(make_pair(key, value));
                }
                else
                {
                    definitions.push_back(make_pair(Name(def), Name()));
                }
            }
            else if(strcmp(option, "--debug") == 0)
            {
                params.debug = true;
            }
            else if(strcmp(option, "--skip_validation") == 0)
            {
                params.skip_validation = true;
            }
            else if(strcmp(option, "--matrix_pack") == 0)
            {
                ++argi;
                if(argi == argc) return set_error(BasicError::bad_arguments(), "Missing arguments of --matrix_pack");
                const c8* mode = argv[argi];
                if(strcmp(mode, "column") == 0)
                {
                    params.matrix_pack_mode = MatrixPackMode::column_major;
                }
                else if(strcmp(mode, "row") == 0)
                {
                    params.matrix_pack_mode = MatrixPackMode::row_major;
                }
                else
                {
                    set_error(BasicError::bad_arguments(), "Unknown argument for --matrix_pack: %s", mode);
                }
            }
            else if(strcmp(option, "--metal_platform") == 0)
            {
                ++argi;
                if(argi == argc) return set_error(BasicError::bad_arguments(), "Missing arguments of --metal_platform");
                const c8* plat = argv[argi];
                if(strcmp(plat, "macos") == 0)
                {
                    params.metal_platform = MetalPlatform::macos;
                }
                else if(strcmp(plat, "ios") == 0)
                {
                    params.metal_platform = MetalPlatform::ios;
                }
                else
                {
                    set_error(BasicError::bad_arguments(), "Unknown argument for --metal_platform: %s", plat);
                }
            }
            else if(strcmp(option, "--cpp_output") == 0)
            {
                cpp_output = true;
            }
            ++argi;
        }
        if(!include_paths.empty())
        {
            params.include_paths = include_paths.cspan();
        }
        if(!definitions.empty())
        {
            params.definitions = definitions.cspan();
        }
        auto compiler = new_compiler();
        lulet(result, compiler->compile(params));
        if(cpp_output)
        {
            String source_name = params.source_name;
            for(c8& ch : source_name)
            {
                if(ch == ' ') ch = '_';
            }
            String header;
            header.append(
R"(// Autogenerated by LunaShader, do not modity.
#pragma once
#include <Luna/ShaderCompiler/ShaderCompiler.hpp>

namespace Luna
{
    constexpr u8 SHADER_DATA_)");
            header.append(source_name);
            header.append(R"([] = {)");
            for(usize i = 0; i < result.data.size(); ++i)
            {
                c8 buf[8];
                u8 data = ((u8*)result.data.data())[i];
                snprintf(buf, 8, "%u", (u32)data);
                header.append(buf);
                if(i != result.data.size() - 1)
                {
                    header.push_back(',');
                }
            }
            header.append(R"(};
    constexpr usize SHADER_DATA_SIZE_)");
            header.append(source_name);
            header.append(" = sizeof(SHADER_DATA_");
            header.append(source_name);
            header.append(R"();
    constexpr ShaderCompiler::TargetFormat SHADER_DATA_FORMAT_)");
            header.append(source_name);
            header.append(" = ShaderCompiler::TargetFormat::");
            switch(result.format)
            {
                case TargetFormat::dxil:
                    header.append("dxil");
                    break;
                case TargetFormat::msl:
                    header.append("msl");
                    break;
                case TargetFormat::spir_v:
                    header.append("spir_v");
                    break;
                default: lupanic();
            }
            header.append(R"(;
    constexpr c8 SHADER_ENTRY_POINT_)");
            header.append(source_name);
            header.append("[] = \"");
            header.append(result.entry_point.c_str());
            header.append(R"(";
    constexpr u32 SHADER_METAL_NUMTHREADS_X_)");
            header.append(source_name);
            header.append(" = ");
            header.append(strprintf("%u", result.metal_numthreads_x));
            header.append(R"(;
    constexpr u32 SHADER_METAL_NUMTHREADS_Y_)");
            header.append(source_name);
            header.append(" = ");
            header.append(strprintf("%u", result.metal_numthreads_y));
            header.append(R"(;
    constexpr u32 SHADER_METAL_NUMTHREADS_Z_)");
            header.append(source_name);
            header.append(" = ");
            header.append(strprintf("%u", result.metal_numthreads_z));
            header.append(R"(;
})");
            lulet(out_header, open_file(output_filename.c_str(), FileOpenFlag::write, FileCreationMode::create_always));
            luexp(out_header->write(header.data(), header.size()));
        }
        else
        {
            lulet(out_file, open_file(output_filename.c_str(), FileOpenFlag::write, FileCreationMode::create_always));
            luexp(out_file->write(result.data.data(), result.data.size()));
        }
    }
    lucatchret;
    return ok;
}

int main(int argc, const char* argv[])
{
    bool inited = Luna::init();
    if(!inited) return -1;
    auto r = run(argc, argv);
    if(failed(r))
    {
        String errmsg = explain(r.errcode());
        auto io = get_std_io_stream();
        io->write(errmsg.c_str(), errmsg.size());
        Luna::close();
        return -1;
    }
    Luna::close();
    return 0;
}
