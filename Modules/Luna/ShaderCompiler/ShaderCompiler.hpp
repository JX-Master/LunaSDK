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
#include <Luna/Runtime/Interface.hpp>
#include <Luna/Runtime/Span.hpp>
#include <Luna/Runtime/Path.hpp>
#include <Luna/Runtime/Result.hpp>
#include <Luna/Runtime/Ref.hpp>

#ifndef LUNA_SHADER_COMPILER_API
#define LUNA_SHADER_COMPILER_API
#endif

namespace Luna
{
	namespace ShaderCompiler
	{
		enum class TargetFormat : u8
		{
			//! Outputs nothing. This can be used if you only want to validate the input source code.
			none = 0,
			//! [Windows Only] Outputs DirectX Intermediate Language for shader model 6.0 and newer.
			dxil,
			//! Outputs SPIR-V for Vulkan and OpenGL.
			spir_v,
			//! Outputs Metal Shading Lauguage.
			msl
		};

		enum class MatrixPackMode : u8
		{
			dont_care = 0,
			row_major = 1,
			column_major = 2,
		};

		enum class ShaderType : u8
		{
			vertex = 1,
			pixel = 2,
			compute = 3
		};

		enum class OptimizationLevel : u8
		{
			none,
			speed,
			full
		};

		enum class MSLPlatform : u8
		{
			macos = 0,
			ios = 1
		};

		//! The compiler that compiles one shader source code into one target form.
		struct ICompiler : virtual Interface
		{
			luiid("{C2D6A83B-0B01-49AC-BFE4-94FAABBB5ACC}");

			//! Resets the compiler settings.
			//! The default settings are:
			//! * source : <Null>
			//! * source name : "unnamed"
			//! * source file path : <Null>
			//! * entry point : "main"
			//! * output format : none
			//! * shader type : vertex
			//! * shader model : 5.1
			//! * optimization level : none
			//! * debug : false
			//! * skip validation : false
			//! * matrix pack mode : dont_care
			//! * include paths: <Null>
			//! * definitions : <Null>
			//! * additional arguments : <Null>
			//! * MSL platform: macos
			virtual void reset() = 0;

			//! Sets the source code of the compiler to be compiled.
			//! For performance considerations, the compiler will not copy source code into one internal buffer, 
			//! it only keeps one reference to the source code, so make sure the source code string is available until
			//! the compiliation is finished.
			virtual void set_source(Span<const c8> data) = 0;

			//! Sets the source name. The source name will be used by the compiler and the debug tools to identify the shader.
			virtual void set_source_name(const Name& name) = 0;

			//! Sets the source file path. 
			//! The source file path is optional and will be used by the compiler or debugger to resolve local include file and PDB file.
			//! The user should provide this path whenever possible.
			virtual void set_source_file_path(const Path& path) = 0;

			//! Sets the name of the entry point function.
			//! The default entry point function name will be "main".
			virtual void set_entry_point(const Name& entry_point) = 0;

			//! Sets the target format of the compiler.
			virtual void set_target_format(TargetFormat format) = 0;

			//! Sets the shader type to be compiled.
			virtual void set_shader_type(ShaderType shader_type) = 0;

			//! Sets the shader model to compile. The default shader model is 5.1.
			virtual void set_shader_model(u32 major, u32 minor) = 0;

			//! Sets the optimization level.
			virtual void set_optimization_level(OptimizationLevel optimization_level) = 0;

			//! Whether to generate debug info for the shader.
			virtual void set_debug(bool debug) = 0;

			//! Whether to skip validating the shader.
			virtual void set_skip_validation(bool skip_validation) = 0;

			//! Sets the matrix pack mode for the shader.
			virtual void set_matrix_pack_mode(MatrixPackMode matrix_pack_mode) = 0;

			//! Gets the system include paths. The include path is one VFS directory that the system will use to search for the include file.
			//! The include file is searched in same order the include path was declared in the include path list.
			virtual Vector<Path>& get_include_paths() = 0;

			//! Gets a list of definitions for the shader.
			virtual HashMap<Name, Name>& get_definitions() = 0;

			//! Gets additional arguments for the compilation.
			virtual Variant& get_additional_arguments() = 0;

			// MSL target specifc settings.

			//! Sets the target platform for generated MSL output.
			virtual void set_msl_platform(MSLPlatform platform) = 0;

			//! Triggers compile for the source code.
			virtual RV compile() = 0;

			//! Gets the compiliation result if the compile was succeeded.
			//! The compiliation result is stored as a BLOB object, described by data pointer and data size.
			//! The blob object is valid until `reset` is called, or until another `compile` is called.
			//! The returned span is empty if `compile` throws errors, or if no valid compiled data exists in the compiler.
			virtual Span<const byte_t> get_output() = 0;


		};

		LUNA_SHADER_COMPILER_API Ref<ICompiler> new_compiler();
	}

	struct Module;
	LUNA_SHADER_COMPILER_API Module* module_shader_compiler();
}