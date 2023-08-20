/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file StudioHeader.hpp
* @author JXMaster
* @date 2020/4/20
*/
#pragma once
#include <Luna/HID/HID.hpp>
#include <Luna/RHI/RHI.hpp>
#include <Luna/ImGui/ImGui.hpp>
#include <Luna/Image/Image.hpp>
#include <Luna/Font/Font.hpp>
#include <Luna/Asset/Asset.hpp>
#include <Luna/ObjLoader/ObjLoader.hpp>
#include <Luna/VFS/VFS.hpp>
#include <Luna/Runtime/HashSet.hpp>
#include <Luna/ShaderCompiler/ShaderCompiler.hpp>
#include <Luna/RHI/ShaderCompileHelper.hpp>
#include <Luna/Runtime/VariantJSON.hpp>

namespace Luna
{
	template <typename _Ty>
	inline RV load_object_from_json_file(_Ty& dst, const Path& path)
	{
		lutry
		{
			lulet(file, VFS::open_file(path, FileOpenFlag::read, FileCreationMode::open_existing));
			lulet(file_data, json_read(file));
			luexp(deserialize(dst, file_data));
		}
		lucatchret;
		return ok;
	}

	template <typename _Ty>
	inline R<ObjRef> load_json_asset(object_t userdata, Asset::asset_t asset, const Path& path)
	{
		ObjRef ret;
		lutry
		{
			Path file_path = path;
			file_path.append_extension("json");
			Ref<_Ty> obj = new_object<_Ty>();
			luexp(load_object_from_json_file(*obj.get(), file_path));
			ret = obj;
		}
		lucatchret;
		return ret;
	}

	template <typename _Ty>
	inline R<ObjRef> create_default_object(object_t userdata, Asset::asset_t asset)
	{
		return ObjRef(new_object<_Ty>().object());
	}

	template <typename _Ty>
	inline RV save_object_to_json_file(const _Ty& src, const Path& path)
	{
		lutry
		{
			lulet(file, VFS::open_file(path, FileOpenFlag::write, FileCreationMode::create_always));
			lulet(file_data, serialize(src));
			auto file_data_json = json_write(file_data);
			luexp(file->write(file_data_json.data(), file_data_json.size()));
		}
		lucatchret;
		return ok;
	}

	template <typename _Ty>
	inline RV save_json_asset(object_t userdata, Asset::asset_t asset, const Path& path, object_t data)
	{
		lutry
		{
			Path file_path = path;
			file_path.append_extension("json");
			Ref<_Ty> obj = ObjRef(data);
			luexp(save_object_to_json_file(*obj.get(), file_path));
		}
		lucatchret;
		return ok;
	}

	inline Image::ImagePixelFormat get_desired_format(Image::ImagePixelFormat format)
	{
		using namespace Image;
		switch (format)
		{
		case ImagePixelFormat::r8_unorm: return ImagePixelFormat::r8_unorm;
		case ImagePixelFormat::rg8_unorm: return ImagePixelFormat::rg8_unorm;
		case ImagePixelFormat::rgb8_unorm: return ImagePixelFormat::rgba8_unorm;
		case ImagePixelFormat::rgba8_unorm: return ImagePixelFormat::rgba8_unorm;
		case ImagePixelFormat::r16_unorm: return ImagePixelFormat::r16_unorm;
		case ImagePixelFormat::rg16_unorm: return ImagePixelFormat::rg16_unorm;
		case ImagePixelFormat::rgb16_unorm: return ImagePixelFormat::rgba16_unorm;
		case ImagePixelFormat::rgba16_unorm: return ImagePixelFormat::rgba16_unorm;
		case ImagePixelFormat::r32_float: return ImagePixelFormat::r32_float;
		case ImagePixelFormat::rg32_float: return ImagePixelFormat::rg32_float;
		case ImagePixelFormat::rgb32_float: return ImagePixelFormat::rgba32_float;
		case ImagePixelFormat::rgba32_float: return ImagePixelFormat::rgba32_float;
		default: lupanic(); return format;
		}
	}
	inline RHI::Format get_format_from_image_format(Image::ImagePixelFormat format)
	{
		using namespace Image;
		switch (format)
		{
		case ImagePixelFormat::r8_unorm: return RHI::Format::r8_unorm;
		case ImagePixelFormat::rg8_unorm: return RHI::Format::rg8_unorm;
		case ImagePixelFormat::rgb8_unorm: return RHI::Format::rgba8_unorm;
		case ImagePixelFormat::rgba8_unorm: return RHI::Format::rgba8_unorm;
		case ImagePixelFormat::r16_unorm: return RHI::Format::r16_unorm;
		case ImagePixelFormat::rg16_unorm: return RHI::Format::rg16_unorm;
		case ImagePixelFormat::rgb16_unorm: return RHI::Format::rgba16_unorm;
		case ImagePixelFormat::rgba16_unorm: return RHI::Format::rgba16_unorm;
		case ImagePixelFormat::r32_float: return RHI::Format::r32_float;
		case ImagePixelFormat::rg32_float: return RHI::Format::rg32_float;
		case ImagePixelFormat::rgb32_float: return RHI::Format::rgba32_float;
		case ImagePixelFormat::rgba32_float: return RHI::Format::rgba32_float;
		default: lupanic(); return RHI::Format::unknown;
		}
	}
	inline R<Image::ImagePixelFormat> get_image_format_from_format(RHI::Format format)
	{
		using namespace Image;
		switch (format)
		{
		case RHI::Format::r8_unorm: return ImagePixelFormat::r8_unorm;
		case RHI::Format::rg8_unorm: return ImagePixelFormat::rg8_unorm;
		case RHI::Format::rgba8_unorm: return ImagePixelFormat::rgba8_unorm;
		case RHI::Format::r16_unorm: return ImagePixelFormat::r16_unorm;
		case RHI::Format::rg16_unorm: return ImagePixelFormat::rg16_unorm;
		case RHI::Format::rgba16_unorm: return ImagePixelFormat::rgba16_unorm;
		case RHI::Format::r32_float: return ImagePixelFormat::r32_float;
		case RHI::Format::rg32_float: return ImagePixelFormat::rg32_float;
		case RHI::Format::rgba32_float: return ImagePixelFormat::rgba32_float;
		default: return BasicError::not_supported();
		}
	}
	inline R<Blob> compile_shader(const Path& shader_file, ShaderCompiler::ShaderType shader_type)
	{
		Blob ret;
		lutry
		{
			lulet(f, open_file(shader_file.encode().c_str(), FileOpenFlag::read, FileCreationMode::open_existing));
			auto file_size = f->get_size();
			auto file_blob = Blob((usize)file_size);
			luexp(f->read(file_blob.data(), file_blob.size()));
			f.reset();
			auto compiler = ShaderCompiler::new_compiler();
			compiler->set_source({ (const c8*)file_blob.data(), file_blob.size()});
			compiler->set_source_name(shader_file.filename());
			compiler->set_source_file_path(shader_file);
			compiler->set_entry_point("main");
			compiler->set_target_format(RHI::get_current_platform_shader_target_format());
			compiler->set_shader_type(shader_type);
			compiler->set_shader_model(6, 0);
#ifdef LUNA_DEBUG
			compiler->set_optimization_level(ShaderCompiler::OptimizationLevel::none);
			compiler->set_debug(true);
#else
			compiler->set_optimization_level(ShaderCompiler::OptimizationLevel::full);
			compiler->set_debug(false);
#endif
			luexp(compiler->compile());
			auto shader_data = compiler->get_output();
			ret = Blob(shader_data.data(), shader_data.size());
		}
		lucatchret;
		return ret;
	}

	//! @interface IAssetEditor
	//! Represents a window of the editor.
	struct IAssetEditor : virtual Interface
	{
		luiid("{410f7868-38b5-4e3f-b291-8e58d2cb7372}");

		virtual void on_render() = 0;
		virtual bool closed() = 0;
	};

	struct AssetEditorDesc
	{
		ObjRef userdata;
		//! Called when the tile is going to be drawn in asset browser.
		void (*on_draw_tile)(object_t userdata, Asset::asset_t asset, const RectF& draw_rect);
		//! Called when a new editor is requested to be open for the specified asset.
		Ref<IAssetEditor>(*new_editor)(object_t userdata, Asset::asset_t editing_asset);
	};

	struct AssetImporterDesc
	{
		//! Called when a new importer is requested to be open for the specified asset.
		Ref<IAssetEditor>(*new_importer)(const Path& create_dir);
	};

	struct AppEnv
	{
		HashSet<Name> new_asset_types; // Displayed on the "New" tab of asset browser.
		HashMap<Name, AssetImporterDesc> importer_types;

		HashMap<Name, AssetEditorDesc> editor_types;

		HashSet<typeinfo_t> component_types;
		HashSet<typeinfo_t> scene_component_types;

		Ref<RHI::IDevice> device;

		u32 graphics_queue;
		u32 async_compute_queue;
		u32 async_copy_queue;

		void register_asset_importer_type(const Name& name, const AssetImporterDesc& desc)
		{
			importer_types.insert(Pair<Name, AssetImporterDesc>(name, desc));
		}

		void register_asset_editor_type(const Name& name, const AssetEditorDesc& desc)
		{
			editor_types.insert(Pair<Name, AssetEditorDesc>(name, desc));
		}
	};

	extern AppEnv* g_env;
}
