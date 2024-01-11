/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file ObjLoader.hpp
* @author JXMaster
* @date 2020/5/12
*/
#pragma once
#include <Luna/Runtime/Vector.hpp>
#include <Luna/Runtime/Math/Vector.hpp>
#include <Luna/Runtime/Result.hpp>

#ifndef LUNA_OBJ_LOADER_API
#define LUNA_OBJ_LOADER_API 
#endif

namespace Luna
{
	namespace ObjLoader
	{
		// Index struct to support different indices for vtx/normal/texcoord.
		// -1 means not used.
		struct Index
		{
			i32 vertex_index;
			i32 normal_index;
			i32 texcoord_index;

			bool operator==(const Index& rhs) const
			{
				return 
					vertex_index == rhs.vertex_index && 
					normal_index == rhs.normal_index && 
					texcoord_index == rhs.texcoord_index;
			}
			bool operator!=(const Index& rhs) const
			{
				return !(*this == rhs);
			}
		};

		struct Mesh
		{
			Vector<Index> indices;
			//! The number of vertices per
			//! face. 3 = triangle, 4 = quad,
			//! ... Up to 255 vertices per face.
			Vector<u8> num_face_vertices;
			//! per-face material ID
			Vector<i32> material_ids;
			// per-face smoothing group
			// ID(0 = off. positive value
			// = group id)
			Vector<u32> smoothing_group_ids;
		};

		//! Linear flattened indices.
		struct Lines
		{
			//! indices for vertices(polygon lines)
			Vector<Index> indices;
			//! The number of vertices per line.
			Vector<i32> num_line_vertices;
		};

		struct Points
		{
			//! indices for points
			Vector<Index> indices;
		};

		struct Shape
		{
			Name name;
			Mesh mesh;
			Lines lines;
			Points points;
		};

		struct Attributes
		{
			Vector<Float3U> vertices;
			Vector<Float3U> normals;
			Vector<Float2U> texcoords;
			Vector<Float3U> colors;
		};

		struct ObjMesh
		{
			Attributes attributes;
			Vector<Shape> shapes;
		};

		//! Loads object file from file.
		//! @param[in] file_name The platform path of the file.
		LUNA_OBJ_LOADER_API R<ObjMesh> load(Span<const byte_t> obj_file, Span<const byte_t> mtl_file);
	}

	struct Module;
	LUNA_OBJ_LOADER_API Module* module_obj_loader();
}