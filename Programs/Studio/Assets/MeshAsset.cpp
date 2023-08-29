/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file MeshAsset.hpp
* @author JXMaster
* @date 2022/12/17
*/
#include "Mesh.hpp"
#include <Luna/VFS/VFS.hpp>
#include "../Mesh.hpp"
#include <Luna/VariantUtils/JSON.hpp>
#include <Luna/Runtime/Serialization.hpp>
#include "../StudioHeader.hpp"
#include <Luna/RHI/Utility.hpp>
namespace Luna
{
	Name get_static_mesh_asset_type()
	{
		return "Static Mesh";
	}

	static RV reset_mesh(Mesh& mesh, const MeshAsset& mesh_asset)
	{
		lutry
		{
			auto device = RHI::get_main_device();
			// Upload resource.
			lulet(vert_res, device->new_buffer(RHI::MemoryType::local, RHI::BufferDesc(
				RHI::BufferUsageFlag::vertex_buffer | RHI::BufferUsageFlag::copy_dest, mesh_asset.vertex_data.size())));
			lulet(index_res, device->new_buffer(RHI::MemoryType::local, RHI::BufferDesc(
				RHI::BufferUsageFlag::index_buffer | RHI::BufferUsageFlag::copy_dest, mesh_asset.index_data.size())));
			lulet(upload_cmdbuf, device->new_command_buffer(g_env->async_copy_queue));
			luexp(RHI::copy_resource_data(upload_cmdbuf, {
				RHI::CopyResourceData::write_buffer(vert_res, 0, mesh_asset.vertex_data.data(), mesh_asset.vertex_data.size()),
				RHI::CopyResourceData::write_buffer(index_res, 0, mesh_asset.index_data.data(), mesh_asset.index_data.size())}));
			mesh.pieces = mesh_asset.pieces;
			mesh.vb = vert_res;
			mesh.ib = index_res;
			mesh.vb_count = (u32)mesh_asset.vertex_data.size() / (u32)sizeof(Vertex);
			mesh.ib_count = (u32)mesh_asset.index_data.size() / (u32)sizeof(u32);
		}
		lucatchret;
		return ok;
	}

	static R<ObjRef> load_static_mesh_asset(object_t userdata, Asset::asset_t asset, const Path& path)
	{
		ObjRef ret;
		lutry
		{
			Path file_path = path;
			file_path.append_extension("mesh");
			lulet(file, VFS::open_file(file_path, FileOpenFlag::read | FileOpenFlag::user_buffering, FileCreationMode::open_existing));
			lulet(data, load_file_data(file));
			lulet(file_data, VariantUtils::read_json((const c8*)data.data(), data.size()));
			MeshAsset mesh_asset;
			luexp(deserialize(mesh_asset, file_data));
			Ref<Mesh> mesh = new_object<Mesh>();
			luexp(reset_mesh(*mesh.get(), mesh_asset));
			ret = mesh;
		}
		lucatchret;
		return ret;
	}
	void register_static_mesh_asset_type()
	{
		register_struct_type<Vertex>({
				luproperty(Vertex, Float3U, position),
				luproperty(Vertex, Float3U, normal),
				luproperty(Vertex, Float3U, tangent),
				luproperty(Vertex, Float2U, texcoord),
				luproperty(Vertex, Float4U, color),
			});
		set_serializable<Vertex>();
		register_struct_type<MeshPiece>({
				   luproperty(MeshPiece, u32, first_index_offset),
				   luproperty(MeshPiece, u32, num_indices)
			});
		set_serializable<MeshPiece>();
		register_boxed_type<Mesh>();
		register_struct_type<MeshAsset>({
				luproperty(MeshAsset, Vector<MeshPiece>, pieces),
				luproperty(MeshAsset, Blob, vertex_data),
				luproperty(MeshAsset, Blob, index_data) });
		set_serializable<MeshAsset>();
		Asset::AssetTypeDesc desc;
		desc.name = get_static_mesh_asset_type();
		desc.on_load_asset = load_static_mesh_asset;
		desc.on_save_asset = nullptr;
		desc.on_set_asset_data = nullptr;
		desc.userdata = nullptr;
		Asset::register_asset_type(desc);
	}
}