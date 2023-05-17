/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file ShaderCompiler.hpp
* @author JXMaster
* @date 2022/10/15
*/
#pragma once
#include <dxc/Support/WinAdapter.h>
#include <dxc/Support/WinIncludes.h>
#include <dxc/dxcapi.h>
#include <wrl/client.h>
#include <Runtime/TSAssert.hpp>
#ifdef LUNA_PLATFORM_WINDOWS
#include <d3dcompiler.h>
#endif
#include "../ShaderCompiler.hpp"

using Microsoft::WRL::ComPtr;

namespace Luna
{
	namespace ShaderCompiler
	{
		enum class DxcTargetType : u8
		{
			dxil = 0,
			spir_v = 1
		};
		enum class SpirvOutputType : u8
		{
			glsl = 0,
			essl = 1,
			msl = 2
		};
		struct Compiler : public ICompiler
		{
			lustruct("ShaderCompiler::Compiler", "{E89511FE-424E-4076-8478-6BE1254714E0}");
			luiimpl();
			lutsassert_lock();

			Span<const c8> m_source;
			Path m_source_file_path;
			Name m_source_name;
			Name m_entry_point;
			Vector<Path> m_include_paths;
			HashMap<Name, Name> m_definitions;
			Variant m_additional_arguments;
			u32 m_shader_model_major;
			u32 m_shader_model_minor;
			OptimizationLevel m_optimization_level;
			TargetFormat m_target_format;
			ShaderType m_shader_type;
			MatrixPackMode m_matrix_pack_mode;
			bool m_debug;
			bool m_skip_validation;

			// Context.
			ComPtr<IDxcCompiler3> m_dxc_compiler;
			ComPtr<IDxcUtils> m_dxc_utils;
			ComPtr<IDxcIncludeHandler> m_default_include_handler;

			// Compiled data.
#ifdef LUNA_PLATFORM_WINDOWS
			ComPtr<ID3DBlob> m_d3d_blob; // For old DXBC.
#endif
			ComPtr<IDxcResult> m_dxc_result;
			ComPtr<IDxcBlob> m_dxc_blob;
			// Pointer to the final output data.
			const byte_t* m_out_data;
			usize m_out_size;

			Compiler()
			{
				reset();
			}

			void clear_output()
			{
#ifdef LUNA_PLATFORM_WINDOWS
				m_d3d_blob.Reset();
#endif
				m_dxc_result.Reset();
				m_dxc_blob.Reset();
				m_out_data = nullptr;
				m_out_size = 0;
			}

			void reset()
			{
				lutsassert();
				m_source = {};
				m_source_file_path.clear();
				m_source_name = "unnamed";
				m_entry_point = "main";
				m_target_format = TargetFormat::none;
				m_shader_type = ShaderType::vertex;
				m_shader_model_major = 5;
				m_shader_model_minor = 1;
				m_optimization_level = OptimizationLevel::none;
				m_include_paths.clear();
				m_definitions.clear();
				m_additional_arguments = VariantType::object;
				m_matrix_pack_mode = MatrixPackMode::dont_care;
				m_debug = false;
				m_skip_validation = false;

				clear_output();
			}
			virtual void set_source(Span<const c8> data) override { m_source = data; }
			virtual void set_source_name(const Name& name) override { m_source_name = name; }
			virtual void set_source_file_path(const Path& path) override { m_source_file_path = path; }
			virtual void set_entry_point(const Name& entry_point) override { m_entry_point = entry_point; }
			virtual void set_target_format(TargetFormat format) override { m_target_format = format; }
			virtual void set_shader_type(ShaderType shader_type) override { m_shader_type = shader_type; }
			virtual void set_shader_model(u32 major, u32 minor) override { m_shader_model_major = major; m_shader_model_minor = minor; }
			virtual void set_optimization_level(OptimizationLevel optimization_level) override { m_optimization_level = optimization_level; }
			virtual void set_debug(bool debug) override { m_debug = debug; }
			virtual void set_skip_validation(bool skip_validation) override { m_skip_validation = skip_validation; }
			virtual void set_matrix_pack_mode(MatrixPackMode matrix_pack_mode) override { m_matrix_pack_mode = matrix_pack_mode; }
			virtual Vector<Path>& get_include_paths() override { return m_include_paths; }
			virtual HashMap<Name, Name>& get_definitions() override { return m_definitions; }
			virtual Variant& get_additional_arguments() override { return m_additional_arguments; }
			RV compile_none();
			RV dxc_compile(DxcTargetType output_type);
			RV spirv_compile(SpirvOutputType output_type);
			RV compile();
			Span<const byte_t> get_output();
		};
	}
}