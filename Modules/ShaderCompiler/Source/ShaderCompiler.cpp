/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file ShaderCompiler.cpp
* @author JXMaster
* @date 2022/10/15
*/
#include <Runtime/PlatformDefines.hpp>
#define LUNA_SHADER_COMPILER_API LUNA_EXPORT

#if defined(LUNA_COMPILER_MSVC) && defined(LUNA_COMPILER_CPP17)
#define _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING
#endif

#include "ShaderCompiler.hpp"
#include "VFS/VFS.hpp"
#include <Runtime/VariantJSON.hpp>
#include <locale>
#include <codecvt>
#include <Runtime/HashSet.hpp>
#include <Runtime/Module.hpp>

#ifdef LUNA_PLATFORM_WINDOWS
#pragma comment(lib, "D3DCompiler.lib")
#endif

namespace Luna
{
	namespace ShaderCompiler
	{
#ifdef LUNA_PLATFORM_WINDOWS
		static const c8 NULL_STRING[] = "";
		class D3DInclude : public ID3DInclude
		{
		public:
			Vector<Path> m_local_search_list;
			HashSet<Path> m_included_files;
			Compiler* m_compiler;
			R<Blob> load_shader(const Path& path)
			{
				lutry
				{
					if (m_included_files.find(path) == m_included_files.end())
					{
						lulet(f, VFS::open_file(path, FileOpenFlag::read, FileCreationMode::open_existing));
						lulet(r, load_file_data(f));
						m_included_files.insert(path);
						return r;
					}
				}
				lucatchret;
				// One file can be included only once.
				return Blob((const byte_t*)NULL_STRING, 1);
			}
			HRESULT STDMETHODCALLTYPE Open(
				D3D_INCLUDE_TYPE IncludeType,
				LPCSTR           pFileName,
				LPCVOID          pParentData,
				LPCVOID* ppData,
				UINT* pBytes
			) override
			{
				Path file_path = pFileName;
				Path path;
				Blob ret;
				if (IncludeType == D3D_INCLUDE_LOCAL)
				{
					// Try to find the shader in local search list.
					for (auto iter = m_local_search_list.rbegin(); iter != m_local_search_list.rend(); ++iter)
					{
						path = *iter;
						path.append(file_path);
						auto r = load_shader(path);
						if (succeeded(r))
						{
							ret = move(r.get());
							break;
						}
					}
				}
				if (ret.empty())
				{
					// Find the shader in system paths.
					for (auto& p : m_compiler->m_include_paths)
					{
						path = p;
						path.append(file_path);
						auto r = load_shader(path);
						if (succeeded(r))
						{
							ret = move(r.get());
							break;
						}
					}
				}
				if (ret.empty())
				{
					set_error(BasicError::not_found(), "Failed to find shader header %s", file_path.encode().c_str());
					return E_FAIL;
				}
				path.pop_back();
				if (*(const c8*)ret.data() != '\0')
				{
					m_local_search_list.push_back(path);
				}
				*pBytes = (UINT)ret.size();
				*ppData = ret.detach();
				return S_OK;
			}

			HRESULT STDMETHODCALLTYPE Close(
				LPCVOID pData
			) override
			{
				if (*(const c8*)pData != '\0')
				{
					m_local_search_list.pop_back();
				}
				memfree((void*)pData);
				return S_OK;
			}
		};
#endif

		RV Compiler::compile_none()
		{
			RV r = dxc_compile(DxcTargetType::dxil);
			clear_output();
			return r;
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
			HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, _COM_Outptr_ void __RPC_FAR* __RPC_FAR* ppvObject) override
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
						lulet(f, VFS::open_file(path, FileOpenFlag::read, FileCreationMode::open_existing));
						lulet(data, load_file_data(f));
						m_compiler->m_dxc_utils->CreateBlob(data.data(), (UINT32)data.size(), CP_UTF8, pEncoding.GetAddressOf());
						*ppIncludeSource = pEncoding.Detach();
						m_included_files.insert(path);
					}
					else
					{
						// Return empty string blob if this file has been included before
						static const char nullStr[] = " ";
						m_compiler->m_dxc_utils->CreateBlob(nullStr, 2, CP_UTF8, pEncoding.GetAddressOf());
						*ppIncludeSource = pEncoding.Detach();
					}
				}
				lucatchret;
				// One file can be included only once.
				return ok;
			}
			HRESULT STDMETHODCALLTYPE LoadSource(_In_ LPCWSTR pFilename, _COM_Outptr_result_maybenull_ IDxcBlob** ppIncludeSource) override
			{
				ComPtr<IDxcBlobEncoding> pEncoding;
				Path path = wstring_to_utf8(pFilename);
				RV r = load_shader(path, ppIncludeSource);
				if (failed(r))
				{
					ppIncludeSource = nullptr;
					return E_FAIL;
				}
				return S_OK;
			}
		};
		RV Compiler::dxc_compile(DxcTargetType target_type)
		{
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
			// Build arguments.
			Vector<WString> arguments;
			Vector<LPCWSTR> argument_pointers;
			// entry point.
			argument_pointers.push_back(L"-E");
			arguments.push_back(utf8_to_wstring(m_entry_point.c_str(), m_entry_point.size()));
			argument_pointers.push_back(arguments.back().c_str());
			// shader model.
			argument_pointers.push_back(L"-T");
			c8 shader_type[16];
			const c8* sm = nullptr;
			switch (m_shader_type)
			{
			case ShaderType::vertex: sm = "vs"; break;
			case ShaderType::pixel: sm = "ps"; break;
			case ShaderType::compute: sm = "cs"; break;
			case ShaderType::geometry: sm = "gs"; break;
			case ShaderType::hull: sm = "hs"; break;
			case ShaderType::domain: sm = "ds"; break;
			}
			snprintf(shader_type, 16, "%s_%u_%u", sm, m_shader_model_major, m_shader_model_minor);
			arguments.push_back(utf8_to_wstring(shader_type));
			argument_pointers.push_back(arguments.back().c_str());
			// Optimization level.
			switch (m_optimization_level)
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
			if (m_debug)
			{
				argument_pointers.push_back(DXC_ARG_DEBUG);
			}
			// Skip validation.
			if (m_skip_validation)
			{
				argument_pointers.push_back(DXC_ARG_SKIP_VALIDATION);
			}
			// Matrix pack mode.
			if (m_matrix_pack_mode == MatrixPackMode::row_major)
			{
				argument_pointers.push_back(DXC_ARG_PACK_MATRIX_ROW_MAJOR);
			}
			else if (m_matrix_pack_mode == MatrixPackMode::column_major)
			{
				argument_pointers.push_back(DXC_ARG_PACK_MATRIX_COLUMN_MAJOR);
			}
			// defines.
			if (!m_definitions.empty())
			{
				argument_pointers.push_back(L"-D");
				for (auto& def : m_definitions)
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
			if (!m_include_paths.empty())
			{
				argument_pointers.push_back(L"-I");
				for (auto& inc : m_include_paths)
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
			source.Ptr = m_source.data();
			source.Size = m_source.size();
			source.Encoding = CP_UTF8;
			hr = m_dxc_compiler->Compile(&source, argument_pointers.data(), (UINT32)argument_pointers.size(), &include_handler, IID_PPV_ARGS(&m_dxc_result));
			if (FAILED(hr))
			{
				return BasicError::bad_platform_call();
			}
			// The shader compilation fail is not returned by error code for IDxcCompiler3::Compile, we need to handle it explicitly.
			ComPtr<IDxcBlobUtf8> err;
			hr = m_dxc_result->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&err), nullptr);
			if (FAILED(hr))
			{
				return BasicError::bad_platform_call();
			}
			hr = m_dxc_result->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&m_dxc_blob), nullptr);
			if (FAILED(hr))
			{
				return BasicError::bad_platform_call();
			}
			luassert(m_dxc_blob);
			m_out_data = (const byte_t*)m_dxc_blob->GetBufferPointer();
			m_out_size = m_dxc_blob->GetBufferSize();
			if (!(m_out_data && m_out_size))
			{
				if (err && err->GetStringLength() > 0)
				{
					return set_error(BasicError::bad_platform_call(), "Shader Compile Failed: %s - %s", m_source_name.c_str(), err->GetStringPointer());
				}
				else
				{
					return set_error(BasicError::bad_platform_call(), "Shader Compile Failed: %s", m_source_name.c_str());
				}
			}
			return ok;
		}
		RV Compiler::spirv_compile(SpirvOutputType output_type)
		{
			lutry
			{
				luexp(dxc_compile(DxcTargetType::spir_v));
				
			}
			lucatchret;
			return ok;
		}
		RV Compiler::compile()
		{
			lutsassert();
			clear_output();
			switch (m_target_format)
			{
			case TargetFormat::none:
				return compile_none();
			case TargetFormat::dxil:
				return dxc_compile(DxcTargetType::dxil);
			case TargetFormat::spir_v:
				return dxc_compile(DxcTargetType::spir_v);
			case TargetFormat::glsl:
				return spirv_compile(SpirvOutputType::glsl);
			case TargetFormat::essl:
				return spirv_compile(SpirvOutputType::essl);
			case TargetFormat::msl:
				return spirv_compile(SpirvOutputType::msl);
			default:
				lupanic_msg("Unsupportted output format.");
			}
			return ok;
		}
		Span<const byte_t> Compiler::get_output()
		{
			lutsassert();
			return Span<const byte_t>(m_out_data, m_out_size);
		}

		LUNA_SHADER_COMPILER_API Ref<ICompiler> new_compiler()
		{
			return new_object<Compiler>();
		}

		RV init()
		{
			register_boxed_type<Compiler>();
			impl_interface_for_type<Compiler, ICompiler>();
			return ok;
		}

		StaticRegisterModule mod("ShaderCompiler", "VFS", init, nullptr);
	}
}