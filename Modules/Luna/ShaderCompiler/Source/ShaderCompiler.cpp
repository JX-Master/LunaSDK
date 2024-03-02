/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file ShaderCompiler.cpp
* @author JXMaster
* @date 2022/10/15
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_SHADER_COMPILER_API LUNA_EXPORT

#if defined(LUNA_COMPILER_MSVC) && defined(LUNA_COMPILER_CPP17)
#define _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING
#endif

#include "ShaderCompiler.hpp"
#include <Luna/VariantUtils/JSON.hpp>
#include <locale>
#include <codecvt>
#include <Luna/Runtime/HashSet.hpp>
#include <Luna/Runtime/Module.hpp>
#include <Luna/Runtime/File.hpp>
#include <spirv_cross/spirv_msl.hpp>

namespace Luna
{
    namespace ShaderCompiler
    {
        R<ShaderCompileResult> Compiler::compile_none(const ShaderCompileParameters& params)
        {
            ShaderCompileResult ret;
            ret.format = TargetFormat::none;
            R<DxcCompileResult> r = dxc_compile(params, DxcTargetType::dxil);
            if(failed(r)) return r.errcode();
            return ret;
        }
        inline WString utf8_to_wstring(const c8* src, usize size = USIZE_MAX)
        {
            size = (size == USIZE_MAX) ? strlen(src) : size;
            std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
            std::wstring ws = converter.from_bytes(src, src + size);
            return WString(ws.begin(), ws.end());
        }
        inline String wstring_to_utf8(LPCWSTR src)
        {
            std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
            std::string s = converter.to_bytes(src);
            return String(s.begin(), s.end());
        }
        class DxcIncludeHandler : public IDxcIncludeHandler
        {
        public:
            HashSet<Path> m_included_files;
            Compiler* m_compiler;
            HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void **ppvObject) override
            {
                return m_compiler->m_default_include_handler->QueryInterface(riid, ppvObject);
            }
            ULONG STDMETHODCALLTYPE AddRef(void) override { return 0; }
            ULONG STDMETHODCALLTYPE Release(void) override { return 0; }
            RV load_shader(const Path& path, IDxcBlob** ppIncludeSource)
            {
                lutry
                {
                    ComPtr<IDxcBlobEncoding> pEncoding;
                    if (m_included_files.find(path) == m_included_files.end())
                    {
                        lulet(f, open_file(path.encode().c_str(), FileOpenFlag::read, FileCreationMode::open_existing));
                        lulet(data, load_file_data(f));
                        m_compiler->m_dxc_utils->CreateBlob(data.data(), (UINT32)data.size(), CP_UTF8, pEncoding.get_address_of());
                        *ppIncludeSource = pEncoding.detach();
                        m_included_files.insert(path);
                    }
                    else
                    {
                        // Return empty string blob if this file has been included before
                        static const char nullStr[] = " ";
                        m_compiler->m_dxc_utils->CreateBlob(nullStr, 2, CP_UTF8, pEncoding.get_address_of());
                        *ppIncludeSource = pEncoding.detach();
                    }
                }
                lucatchret;
                // One file can be included only once.
                return ok;
            }
            HRESULT STDMETHODCALLTYPE LoadSource(_In_ LPCWSTR pFilename, _COM_Outptr_result_maybenull_ IDxcBlob** ppIncludeSource) override
            {
                Path path = *(m_compiler->m_source_file_path);
                path.pop_back(); // remove source filename.
                path.append(wstring_to_utf8(pFilename));
                RV r = load_shader(path, ppIncludeSource);
                if (failed(r))
                {
                    ppIncludeSource = nullptr;
                    return E_FAIL;
                }
                return S_OK;
            }
        };
        R<DxcCompileResult> Compiler::dxc_compile(const ShaderCompileParameters& params, DxcTargetType target_type)
        {
            if (params.shader_model.major < 6) return set_error(BasicError::not_supported(), "Shader model 5.1 and olders are not supported.");
            HRESULT hr;
            if (!m_dxc_compiler)
            {
                hr = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&m_dxc_compiler));
                if (FAILED(hr)) return BasicError::bad_platform_call();
            }
            if (!m_dxc_utils)
            {
                hr = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&m_dxc_utils));
                if (FAILED(hr)) return BasicError::bad_platform_call();
            }
            if (!m_default_include_handler)
            {
                hr = m_dxc_utils->CreateDefaultIncludeHandler(&m_default_include_handler);
                if (FAILED(hr)) return BasicError::bad_platform_call();
            }
            m_source_file_path = &params.source_file_path;
            // Build arguments.
            Vector<WString> arguments;
            Vector<LPCWSTR> argument_pointers;
            // entry point.
            argument_pointers.push_back(L"-E");
            Name entry_point = params.entry_point;
            if(!entry_point) entry_point = "main";
            arguments.push_back(utf8_to_wstring(entry_point.c_str(), entry_point.size()));
            argument_pointers.push_back(arguments.back().c_str());
            // shader model.
            argument_pointers.push_back(L"-T");
            c8 shader_type[16];
            const c8* sm = nullptr;
            switch (params.shader_type)
            {
            case ShaderType::vertex: sm = "vs"; break;
            case ShaderType::pixel: sm = "ps"; break;
            case ShaderType::compute: sm = "cs"; break;
            }
            snprintf(shader_type, 16, "%s_%u_%u", sm, params.shader_model.major, params.shader_model.minor);
            arguments.push_back(utf8_to_wstring(shader_type));
            argument_pointers.push_back(arguments.back().c_str());
            // Optimization level.
            switch (params.optimization_level)
            {
            case OptimizationLevel::none:
                argument_pointers.push_back(DXC_ARG_SKIP_OPTIMIZATIONS); break;
            case OptimizationLevel::speed:
                argument_pointers.push_back(DXC_ARG_OPTIMIZATION_LEVEL1); break;
            case OptimizationLevel::full:
                argument_pointers.push_back(DXC_ARG_OPTIMIZATION_LEVEL3); break;
            default:
                lupanic();
                break;
            }
            // Debug.
            if (params.debug)
            {
                argument_pointers.push_back(DXC_ARG_DEBUG);
            }
            // Skip validation.
            if (params.skip_validation)
            {
                argument_pointers.push_back(DXC_ARG_SKIP_VALIDATION);
            }
            // Matrix pack mode.
            if (params.matrix_pack_mode == MatrixPackMode::row_major)
            {
                argument_pointers.push_back(DXC_ARG_PACK_MATRIX_ROW_MAJOR);
            }
            else if (params.matrix_pack_mode == MatrixPackMode::column_major)
            {
                argument_pointers.push_back(DXC_ARG_PACK_MATRIX_COLUMN_MAJOR);
            }
            // defines.
            if (!params.definitions.empty())
            {
                argument_pointers.push_back(L"-D");
                for (auto& def : params.definitions)
                {
                    String define_entry;
                    define_entry.append(def.first.c_str());
                    define_entry.push_back('=');
                    define_entry.append(def.second.c_str());
                    arguments.push_back(utf8_to_wstring(define_entry.data(), define_entry.size()));
                    argument_pointers.push_back(arguments.back().c_str());
                }
            }
            // Global include directories.
            if (!params.include_paths.empty())
            {
                argument_pointers.push_back(L"-I");
                for (auto& inc : params.include_paths)
                {
                    String inc_path = inc.encode();
                    arguments.push_back(utf8_to_wstring(inc_path.data(), inc_path.size()));
                    argument_pointers.push_back(arguments.back().c_str());
                }
            }
            // Target type.
            if (target_type == DxcTargetType::spir_v)
            {
                argument_pointers.push_back(L"-spirv");
            }
            // Include handler.
            DxcIncludeHandler include_handler;
            include_handler.m_compiler = this;
            // Source.
            DxcBuffer source;
            source.Ptr = params.source.data();
            source.Size = params.source.size();
            source.Encoding = CP_UTF8;
            DxcCompileResult result;
            hr = m_dxc_compiler->Compile(&source, argument_pointers.data(), (UINT32)argument_pointers.size(), &include_handler, IID_PPV_ARGS(&result.m_dxc_result));
            if (FAILED(hr))
            {
                return BasicError::bad_platform_call();
            }
            // The shader compilation fail is not returned by error code for IDxcCompiler3::Compile, we need to handle it explicitly.
            ComPtr<IDxcBlobUtf8> err;
            hr = result.m_dxc_result->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&err), nullptr);
            if (FAILED(hr))
            {
                return BasicError::bad_platform_call();
            }
            hr = result.m_dxc_result->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&result.m_dxc_blob), nullptr);
            if (FAILED(hr))
            {
                return BasicError::bad_platform_call();
            }
            luassert(result.m_dxc_blob);
            void* out_data = result.m_dxc_blob->GetBufferPointer();
            usize out_size = result.m_dxc_blob->GetBufferSize();
            if (!(out_data && out_size))
            {
                if (err && err->GetStringLength() > 0)
                {
                    return set_error(BasicError::bad_platform_call(), "Shader Compile Failed: %s - %s", params.source_name.c_str(), err->GetStringPointer());
                }
                else
                {
                    return set_error(BasicError::bad_platform_call(), "Shader Compile Failed: %s", params.source_name.c_str());
                }
            }
            return result;
        }
        R<ShaderCompileResult> Compiler::spirv_compile(const ShaderCompileParameters& params, SpirvOutputType output_type)
        {
            ShaderCompileResult r;
            lutry
            {
                // Skip optimization if we are generating msl from spir-v.
                ShaderCompileParameters dxc_params = params;
                dxc_params.debug = true;
                dxc_params.optimization_level = OptimizationLevel::none;
                lulet(dxc_result, dxc_compile(dxc_params, DxcTargetType::spir_v));
                void* dxc_out_data = dxc_result.m_dxc_blob->GetBufferPointer();
                usize dxc_out_size = dxc_result.m_dxc_blob->GetBufferSize();
                if(output_type == SpirvOutputType::msl)
                {
                    spirv_cross::CompilerMSL msl((const uint32_t*)dxc_out_data, dxc_out_size / 4);
                    auto options = msl.get_msl_options();
                    options.argument_buffers = true;
                    options.set_msl_version(3, 0, 0);
                    options.force_active_argument_buffer_resources = true;
                    switch (params.metal_platform)
                    {
                    case MetalPlatform::macos:
                        options.platform = spirv_cross::CompilerMSL::Options::Platform::macOS;
                        break;
                    case MetalPlatform::ios:
                        options.platform = spirv_cross::CompilerMSL::Options::Platform::iOS;
                        break;
                    }
                    msl.set_msl_options(options);
                    auto compiled_data = msl.compile();
                    r.data = Blob((const byte_t*)compiled_data.c_str(), compiled_data.size());
                    r.format = TargetFormat::msl;
                    auto entry_point_and_stage = msl.get_entry_points_and_stages()[0];
                    auto entry_point = msl.get_entry_point(entry_point_and_stage.name, entry_point_and_stage.execution_model);
                    if(params.shader_type == ShaderType::compute)
                    {
                        r.metal_numthreads_x = entry_point.workgroup_size.x;
                        r.metal_numthreads_y = entry_point.workgroup_size.y;
                        r.metal_numthreads_z = entry_point.workgroup_size.z;
                    }
                    r.entry_point = entry_point.name.c_str();
                }
            }
            lucatchret;
            return r;
        }
        R<ShaderCompileResult> Compiler::compile(const ShaderCompileParameters& params)
        {
            lutsassert();
            ShaderCompileResult r;
            lutry
            {
                switch (params.target_format)
                {
                case TargetFormat::none:
                    luset(r, compile_none(params));
                    break;
                case TargetFormat::dxil:
                {
                    lulet(dxc_result, dxc_compile(params, DxcTargetType::dxil));
                    r.data = Blob((const byte_t*)dxc_result.m_dxc_blob->GetBufferPointer(), dxc_result.m_dxc_blob->GetBufferSize());
                    r.format = TargetFormat::dxil;
                    r.entry_point = params.entry_point;
                    break;
                }
                case TargetFormat::spir_v:
                {
                    lulet(dxc_result, dxc_compile(params, DxcTargetType::spir_v));
                    r.data = Blob((const byte_t*)dxc_result.m_dxc_blob->GetBufferPointer(), dxc_result.m_dxc_blob->GetBufferSize());
                    r.format = TargetFormat::spir_v;
                    r.entry_point = params.entry_point;
                    break;
                }
                case TargetFormat::msl:
                    luset(r, spirv_compile(params, SpirvOutputType::msl));
                    break;
                default:
                    lupanic_msg("Unsupportted output format.");
                    break;
                }
            }
            lucatchret;
            return r;
        }

        LUNA_SHADER_COMPILER_API Ref<ICompiler> new_compiler()
        {
            return new_object<Compiler>();
        }

        struct ShaderCompilerModule : public Module
        {
            virtual const c8* get_name() override { return "ShaderCompiler"; }
            virtual RV on_init() override
            {
                register_boxed_type<Compiler>();
                impl_interface_for_type<Compiler, ICompiler>();
                return ok;
            }
        };
    }
    LUNA_SHADER_COMPILER_API Module* module_shader_compiler()
    {
        static ShaderCompiler::ShaderCompilerModule m;
        return &m;
    }
}
