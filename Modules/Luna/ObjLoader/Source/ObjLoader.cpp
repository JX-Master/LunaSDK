/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file ObjLoader.cpp
* @author JXMaster
* @date 2020/5/12
*/
#include <Luna/Runtime/PlatformDefines.hpp>

#define LUNA_OBJ_LOADER_API LUNA_EXPORT
#include "../ObjLoader.hpp"
#include "tiny_obj_loader.h"
#include <Luna/Runtime/Module.hpp>

#include <filesystem>
#include <iostream>

namespace Luna
{
	namespace ObjLoader
	{
		static_assert(sizeof(Index) == sizeof(tinyobj::index_t), "Index size does not match");

		struct ObjLoaderModule : public Module
		{
			virtual const c8* get_name() override { return "ObjLoader"; }
		};

		void copy_shape(Shape& dst, const tinyobj::shape_t& src)
		{
			dst.name = Name(src.name.c_str());
			
			// Copy mesh.
			dst.mesh.indices.resize(src.mesh.indices.size());
			if (dst.mesh.indices.size())
			{
				memcpy(&dst.mesh.indices[0], &src.mesh.indices[0], dst.mesh.indices.size() * sizeof(Index));
			}
			dst.mesh.num_face_vertices.resize(src.mesh.num_face_vertices.size());
			if (dst.mesh.num_face_vertices.size())
			{
				memcpy(&dst.mesh.num_face_vertices[0], &src.mesh.num_face_vertices[0], dst.mesh.num_face_vertices.size() * sizeof(u8));
			}
			dst.mesh.material_ids.resize(src.mesh.material_ids.size());
			if (dst.mesh.material_ids.size())
			{
				memcpy(&dst.mesh.material_ids[0], &src.mesh.material_ids[0], dst.mesh.material_ids.size() * sizeof(i32));
			}
			dst.mesh.smoothing_group_ids.resize(src.mesh.smoothing_group_ids.size());
			if (dst.mesh.smoothing_group_ids.size())
			{
				memcpy(&dst.mesh.smoothing_group_ids[0], &src.mesh.smoothing_group_ids[0], dst.mesh.smoothing_group_ids.size() * sizeof(i32));
			}

			// Copy lines.
			dst.lines.indices.resize(src.lines.indices.size());
			if (dst.lines.indices.size())
			{
				memcpy(&dst.lines.indices[0], &src.lines.indices[0], dst.lines.indices.size() * sizeof(Index));
			}
			dst.lines.num_line_vertices.resize(src.lines.num_line_vertices.size());
			if (dst.lines.num_line_vertices.size())
			{
				memcpy(&dst.lines.num_line_vertices[0], &src.lines.num_line_vertices[0], dst.lines.num_line_vertices.size() * sizeof(i32));
			}

			// Copy points.
			dst.points.indices.resize(src.points.indices.size());
			if (dst.points.indices.size())
			{
				memcpy(&dst.points.indices[0], &src.points.indices[0], dst.points.indices.size() * sizeof(Index));
			}
		}

		LUNA_OBJ_LOADER_API R<ObjMesh> load(Span<const byte_t> obj_file, Span<const byte_t> mtl_file)
		{

			std::string obj_source((const c8*)obj_file.data(), obj_file.size());
			std::string mtl_source;
			if (!mtl_file.empty())
			{
				mtl_source.assign((const c8*)mtl_file.data(), mtl_file.size());
			}

			tinyobj::ObjReader reader;

			bool ret = reader.ParseFromString(obj_source, mtl_source);

			if (!ret)
			{
				auto& warn = reader.Warning();
				auto& err = reader.Error();
				if (!err.empty())
				{
					return set_error(BasicError::format_error(), err.c_str());
				}
				return BasicError::format_error();
			}

			const tinyobj::attrib_t& attrib = reader.GetAttrib();
			const std::vector<tinyobj::shape_t>& shapes = reader.GetShapes();
			const std::vector<tinyobj::material_t>& meterials = reader.GetMaterials();

			ObjMesh obj;

			auto& attributes = obj.attributes;

			// Copy attributes.
			attributes.vertices.resize(attrib.vertices.size() / 3);
			attributes.normals.resize(attrib.normals.size() / 3);
			attributes.texcoords.resize(attrib.texcoords.size() / 2);
			attributes.colors.resize(attrib.colors.size() / 3);

			if (attributes.vertices.size())
			{
				memcpy(&attributes.vertices[0], &attrib.vertices[0], sizeof(Float3U) * attributes.vertices.size());
			}
			if (attributes.normals.size())
			{
				memcpy(&attributes.normals[0], &attrib.normals[0], sizeof(Float3U) * attributes.normals.size());
			}
			if (attributes.texcoords.size())
			{
				memcpy(&attributes.texcoords[0], &attrib.texcoords[0], sizeof(Float2U) * attributes.texcoords.size());
			}
			if (attributes.colors.size())
			{
				memcpy(&attributes.colors[0], &attrib.colors[0], sizeof(Float3U) * attributes.colors.size());
			}

			// Copy shape information.
			obj.shapes.resize(shapes.size());
			for (usize i = 0; i < shapes.size(); ++i)
			{
				copy_shape(obj.shapes[i], shapes[i]);
			}

			return obj;
		}
	}

	LUNA_OBJ_LOADER_API Module* module_obj_loader()
	{
		static ObjLoader::ObjLoaderModule m;
		return &m;
	}
}