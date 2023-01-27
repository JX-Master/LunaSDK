/*
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file StudioHeader.hpp
* @author JXMaster
* @date 2020/4/20
*/
#pragma once
#include <HID/HID.hpp>
#include <RHI/RHI.hpp>
#include <ImGui/ImGui.hpp>
#include <Image/Image.hpp>
#include <Font/Font.hpp>
#include <Asset/Asset.hpp>
#include <ObjLoader/ObjLoader.hpp>
#include <VFS/VFS.hpp>
#include <Runtime/HashSet.hpp>

namespace Luna
{
	template <typename _Ty>
	inline RV load_object_from_json_file(_Ty& dest, const Path& path)
	{
		lutry
		{
			lulet(file, VFS::open_file(path, FileOpenFlag::read, FileCreationMode::open_existing));
			lulet(file_data, json_read(file));
			luexp(deserialize(dest, file_data));
		}
		lucatchret;
		return ok;
	}

	template <typename _Ty>
	inline R<ObjRef> load_json_asset(object_t userdata, const Path& path)
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
		Ref<RHI::ICommandQueue> graphics_queue;
		Ref<RHI::ICommandQueue> async_compute_queue;

		HashMap<Name, AssetImporterDesc> importer_types;

		HashMap<Name, AssetEditorDesc> editor_types;

		HashSet<typeinfo_t> component_types;
		HashSet<typeinfo_t> scene_component_types;

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