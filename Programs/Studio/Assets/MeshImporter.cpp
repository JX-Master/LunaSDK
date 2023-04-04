/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file MeshImporter.cpp
* @author JXMaster
* @date 2022/12/17
*/
#include "Mesh.hpp"
#include <ObjLoader/ObjLoader.hpp>
#include <Window/FileDialog.hpp>
#include <Window/MessageBox.hpp>
#include <Runtime/File.hpp>
#include <VFS/VFS.hpp>
#include <Runtime/Serialization.hpp>
#include <Runtime/VariantJSON.hpp>
namespace Luna
{
	struct MeshImporter : public IAssetEditor
	{
		lustruct("MeshImporter", "{770ac671-c013-4b89-a0a2-ab222e919a35}");
		luiimpl();

		Path m_create_dir;

		Path m_source_file_path;

		ObjLoader::ObjMesh m_obj_file;

		Vector<String> m_import_names;

		MeshImporter() {}

		bool m_open;

		virtual void on_render() override;
		virtual bool closed() override
		{
			return !m_open;
		}
	};

	static RV create_mesh_asset_from_obj(MeshAsset& mesh, const ObjLoader::ObjMesh& obj_file, u32 shape_index)
	{
		auto& m = obj_file.shapes[shape_index].mesh;	// We only consider the mesh part of the specified shape.
		auto& faces = m.num_face_vertices;	// 
		auto& attrib = obj_file.attributes;

		// Convert indices to vertices.
		Vector<Vertex> vertices;
		for (auto& i : m.indices)
		{
			Vertex v;
			v.position = attrib.vertices[i.vertex_index];
			auto& color3 = attrib.colors[i.vertex_index];
			v.color = Float4U(color3.x, color3.y, color3.z, 1.0f);
			if (i.normal_index != -1 && i.normal_index < attrib.normals.size())
			{
				v.normal = attrib.normals[i.normal_index];
			}
			else
			{
				v.normal = Float3U(0.0f, 0.0f, 1.0f);
			}
			if (i.texcoord_index != -1 && i.texcoord_index < attrib.texcoords.size())
			{
				v.texcoord = attrib.texcoords[i.texcoord_index];
			}
			else
			{
				v.texcoord = Float2U(0.0f, 0.0f);
			}
			// Tangent is left to be computed later.
			vertices.push_back(v);
		}

		// Build index list.
		// Material ID -> Index list.
		HashMap<u32, Vector<u32>> mat_map;
		u32 vert_offset = 0;
		for (u32 i = 0; i < (u32)faces.size(); ++i)	// Once per face.
		{
			auto mat_id = m.material_ids[i];	// Finds the material ID for this face, this is used for pieces.
			auto iter = mat_map.find(mat_id);
			if (iter == mat_map.end())
			{
				iter = mat_map.insert(make_pair(mat_id, Vector<u32>())).first;
			}

			// If this is not a triangle face, convert this to triangles.
			for (u32 j = 0; j < (u32)(faces[i] - 2); ++j)
			{
				iter->second.push_back(vert_offset);
				iter->second.push_back(vert_offset + j + 1);
				iter->second.push_back(vert_offset + j + 2);
			}
			vert_offset += faces[i];
		}

		// Calculate tangents.
		Vector<Float3U> tangents;
		Vector<Float3U> binormals;
		tangents.resize(vertices.size(), Float3U(0.0f, 0.0f, 0.0f));
		binormals.resize(vertices.size(), Float3U(0.0f, 0.0f, 0.0f));

		u32 idx_offset = 0;
		for (auto& i : mat_map)
		{
			usize num_tris = i.second.size() / 3;
			for (usize j = 0; j < num_tris; ++j)
			{
				u32 i1 = i.second[j * 3];
				u32 i2 = i.second[j * 3 + 1];
				u32 i3 = i.second[j * 3 + 2];
				Vertex& v1 = vertices[i1];
				Vertex& v2 = vertices[i2];
				Vertex& v3 = vertices[i3];
				Float3 e1 = v3.position - v1.position;
				Float3 e2 = v2.position - v1.position;
				Float2 uv1 = v3.texcoord - v1.texcoord;
				Float2 uv2 = v2.texcoord - v1.texcoord;
				f32 r = 1.0f / (uv1.x * uv2.y - uv1.y * uv2.x);
				Float3U tangent(
					((e1.x * uv2.y) - (e2.x * uv1.y)) * r,
					((e1.y * uv2.y) - (e2.y * uv1.y)) * r,
					((e1.z * uv2.y) - (e2.z * uv1.y)) * r
				);
				Float3U binormal(
					((e1.x * uv2.x) - (e2.x * uv1.x)) * r,
					((e1.y * uv2.x) - (e2.y * uv1.x)) * r,
					((e1.z * uv2.x) - (e2.z * uv1.x)) * r
				);
				tangents[i1] = tangents[i1] + tangent;
				tangents[i2] = tangents[i2] + tangent;
				tangents[i3] = tangents[i3] + tangent;
				binormals[i1] = binormals[i1] + binormal;
				binormals[i2] = binormals[i2] + binormal;
				binormals[i3] = binormals[i3] + binormal;
			}

			idx_offset += (u32)i.second.size();
		}

		for (usize i = 0; i < vertices.size(); ++i)
		{
			Float3 n = vertices[i].normal;
			Float3 t = tangents[i];

			// Gram-Schmidt orthogonalize
			Float3 tang = normalize(t - n * dot(n, t));

			// Calculate handedness
			f32 w = dot(cross(n, t), binormals[i]);
			if (w < 0.0f)
			{
				tang = -tang;
			}
			vertices[i].tangent = tang;
		}

		// Fill vertex data.
		auto vb_blob = Blob(vertices.size() * sizeof(Vertex));
		memcpy(vb_blob.data(), vertices.data(), vertices.size() * sizeof(Vertex));

		// Fill indices data.
		usize idx_count = 0;
		for (auto& i : mat_map)
		{
			idx_count += i.second.size();
		}

		auto ib_blob = Blob(idx_count * sizeof(u32));

		idx_offset = 0;
		Vector<MeshPiece> pieces;
		for (auto& i : mat_map)
		{
			MeshPiece p;
			p.first_index_offset = idx_offset;
			p.num_indices = (u32)i.second.size();
			memcpy((u32*)ib_blob.data() + idx_offset, i.second.data(), sizeof(u32) * i.second.size());
			idx_offset += p.num_indices;
			pieces.push_back(p);
		}
		mesh.pieces = move(pieces);
		mesh.vertex_data = move(vb_blob);
		mesh.index_data = move(ib_blob);
		return ok;
	}

	static void import_static_mesh(const Path& path, const ObjLoader::ObjMesh& mesh, u32 shape_index)
	{
		lutry
		{
			auto file_path = path;
			lulet(asset, Asset::new_asset(file_path, get_static_mesh_asset_type()));
			file_path.append_extension("mesh");
			MeshAsset mesh_asset;
			luexp(create_mesh_asset_from_obj(mesh_asset, mesh, shape_index));
			lulet(f, VFS::open_file(file_path, FileOpenFlag::write | FileOpenFlag::user_buffering, FileCreationMode::create_always));
			lulet(data, serialize(mesh_asset));
			auto json_data = json_write(data);
			luexp(f->write({(byte_t*)json_data.data(), json_data.size()}));
			f.reset();
			Asset::load_asset(asset);
		}
		lucatch
		{
			auto _ = Window::message_box(explain(lures), "Failed to import obj mesh asset",
				Window::MessageBoxType::ok, Window::MessageBoxIcon::error);
		}
	}

	void MeshImporter::on_render()
	{
		char title[32];
		sprintf_s(title, "Obj Mesh Importer###%d", (u32)(usize)this);

		ImGui::Begin(title, &m_open, ImGuiWindowFlags_NoCollapse);

		if (ImGui::Button("Select Source File"))
		{
			lutry
			{
				lulet(file_path, Window::open_file_dialog("Obj File\0*.obj\0\0",
					"Select Source File"));
				// Open file.
				auto path = file_path[0];

				lulet(obj_file, open_file(path.encode(PathSeparator::system_preferred).c_str(),
					FileOpenFlag::read | FileOpenFlag::user_buffering, FileCreationMode::open_existing));
				lulet(obj_file_data, load_file_data(obj_file));

				path.replace_extension("mtl");
				auto f = open_file(path.encode(PathSeparator::system_preferred).c_str(),
					FileOpenFlag::read | FileOpenFlag::user_buffering, FileCreationMode::open_existing);

				Blob mtl_file_data;

				if (succeeded(f))
				{
					luset(mtl_file_data, load_file_data(f.get()));
				}

				luset(m_obj_file, ObjLoader::load(obj_file_data.cspan(), mtl_file_data.cspan()));

				m_source_file_path = file_path[0];

				m_import_names.clear();
				for (auto& i : m_obj_file.shapes)
				{
					m_import_names.push_back(String(i.name.c_str()));
				}
			}
			lucatch
			{
				if (lures != BasicError::interrupted())
				{
					auto _ = Window::message_box(explain(lures), "Failed to import obj file",
						Window::MessageBoxType::ok, Window::MessageBoxIcon::error);
				}
				m_source_file_path.clear();

			}
		}

		if (m_source_file_path.empty())
		{
			ImGui::Text("No obj file selected.");
		}
		else
		{
			ImGui::Text(m_source_file_path.encode().c_str());
			ImGui::Text("Object Information:");

			ImGui::Text("Vertex entries count: %u", (u32)m_obj_file.attributes.vertices.size());
			ImGui::Text("Normal entries count: %u", (u32)m_obj_file.attributes.normals.size());
			ImGui::Text("TexCoord entries count: %u", (u32)m_obj_file.attributes.texcoords.size());
			ImGui::Text("Color entries count: %u", (u32)m_obj_file.attributes.colors.size());

			if (m_obj_file.shapes.empty())
			{
				ImGui::Text("No Shape information detected, this model cannot be imported.");
			}
			else
			{
				ImGui::Text("%u meshes found", (u32)m_obj_file.shapes.size());
				if(ImGui::Button("Import All"))
				{
					for (u32 i = 0; i < (u32)m_obj_file.shapes.size(); ++i)
					{
						if(!m_import_names[i].empty())
						{
							Path file_path = m_create_dir;
							file_path.push_back(m_import_names[i]);
							import_static_mesh(file_path, m_obj_file, i);
						}
					}
				}
				if (ImGui::CollapsingHeader("Shapes"))
				{
					for (u32 i = 0; i < (u32)m_obj_file.shapes.size(); ++i)
					{
						ImGui::Text("Name: %s", m_obj_file.shapes[i].name.c_str());
						ImGui::Text("Faces: %u", (u32)m_obj_file.shapes[i].mesh.num_face_vertices.size());

						ImGui::PushID(i);
						ImGui::InputText("Asset Name", m_import_names[i]);
						if (!m_import_names[i].empty())
						{
							Path file_path = m_create_dir;
							file_path.push_back(m_import_names[i]);
							ImGui::Text("The mesh will be imported as: %s", file_path.encode().c_str());
							if (ImGui::Button("Import"))
							{
								import_static_mesh(file_path, m_obj_file, i);
							}
						}
						ImGui::PopID();
					}
				}
			}
		}
		ImGui::End();
	}

	static Ref<IAssetEditor> new_static_mesh_importer(const Path& create_dir)
	{
		auto importer = new_object<MeshImporter>();
		importer->m_create_dir = create_dir;
		return importer;
	}

	void register_static_mesh_importer()
	{
		register_boxed_type<MeshImporter>();
		impl_interface_for_type<MeshImporter, IAssetEditor>();
		AssetImporterDesc desc;
		desc.new_importer = new_static_mesh_importer;
		g_env->register_asset_importer_type(get_static_mesh_asset_type(), desc);
	}
}