/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file MeshImporter.cpp
* @author JXMaster
* @date 2022/12/17
*/
#include "Mesh.hpp"
#include <Luna/ObjLoader/ObjLoader.hpp>
#include <Luna/Window/FileDialog.hpp>
#include <Luna/Window/MessageBox.hpp>
#include <Luna/Runtime/File.hpp>
#include <Luna/VFS/VFS.hpp>
#include <Luna/Runtime/Serialization.hpp>
#include <Luna/VariantUtils/JSON.hpp>
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

        bool m_open = true;

        virtual void on_render(GUI::IContext* context) override;
        virtual bool closed() override
        {
            return !m_open;
        }
    };

    template <>
    struct hash<ObjLoader::Index>
    {
        usize operator()(const ObjLoader::Index& v) const
        {
            return hash<i32>()(v.vertex_index) ^ hash<i32>()(v.normal_index) ^ hash<i32>()(v.texcoord_index);
        }
    };

    static RV create_mesh_asset_from_obj(MeshAsset& mesh, const ObjLoader::ObjMesh& obj_file, u32 shape_index)
    {
        auto& m = obj_file.shapes[shape_index].mesh;    // We only consider the mesh part of the specified shape.
        auto& faces = m.num_face_vertices;    // 
        auto& attrib = obj_file.attributes;

        // Collect vertex used in this shape.
        Vector<Vertex> vertices;
        HashMap<ObjLoader::Index, usize> vertices_map;
        for(auto& i : m.indices)
        {
            if(!vertices_map.contains(i))
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
                vertices.push_back(v);
                vertices_map.insert(make_pair(i, vertices.size() - 1));
            }
        }

        // Build index list for every material.
        // Material ID -> Index list.
        HashMap<u32, Vector<u32>> mat_map;
        usize index_offset = 0;
        for(usize face_index = 0; face_index < faces.size(); ++face_index)
        {
            i32 mat_id = m.material_ids[face_index];
            auto iter = mat_map.find(mat_id);
            if (iter == mat_map.end())
            {
                iter = mat_map.insert(make_pair(mat_id, Vector<u32>())).first;
            }
            u8 num_face_vertices = faces[face_index];
            // If this is not a triangle face, convert this to triangle fans.
            for (i32 j = 0; j < ((i32)num_face_vertices - 2); ++j)
            {
                auto index1 = m.indices[index_offset];
                auto index2 = m.indices[index_offset + j + 1];
                auto index3 = m.indices[index_offset + j + 2];
                iter->second.push_back(vertices_map.find(index1)->second);
                iter->second.push_back(vertices_map.find(index2)->second);
                iter->second.push_back(vertices_map.find(index3)->second);
            }
            index_offset += num_face_vertices;
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
                Vertex& p1 = vertices[i1];
                Vertex& p2 = vertices[i2];
                Vertex& p3 = vertices[i3];
                Float3 e1 = p3.position - p1.position;
                Float3 e2 = p2.position - p1.position;
                f32 u1 = p3.texcoord.x - p1.texcoord.x;
                f32 v1 = p3.texcoord.y - p1.texcoord.y;
                f32 u2 = p2.texcoord.x - p1.texcoord.x;
                f32 v2 = p2.texcoord.y - p1.texcoord.y;

                f32 r = 1.0f / (v1 * u2 - v2 * u1);

                Float3 tangent = (e2 * v1 - e1 * v2) * r;
                Float3 binormal = (e1 * u2 - e2 * u1) * r;

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
            Float3 n = normalize(vertices[i].normal);
            Float3 t = normalize(tangents[i]);

            // Gram-Schmidt orthogonalize
            Float3 tang = normalize(t - dot(t, n) * n);

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
            auto json_data = VariantUtils::write_json(data);
            luexp(f->write(json_data.data(), json_data.size()));
            f.reset();
            luexp(Asset::load_asset(asset));
        }
        lucatch
        {
            auto _ = Window::message_box(explain(luerr), "Failed to import obj mesh asset",
                Window::MessageBoxType::ok, Window::MessageBoxIcon::error);
        }
    }

    void MeshImporter::on_render(GUI::IContext* context)
    {
        char title[32];
        snprintf(title, 32, "Obj Mesh Importer###%d", (u32)(usize)this);

        if(!m_open) return;

        GUI::begin_window(context, title, &m_open, GUI::Size::fixed(620.0f, 720.0f));
        GUI::ItemHandle select_source = GUI::button(context, "Select Source File");

        if (GUI::is_item_clicked(select_source))
        {
            lutry
            {
                Window::FileDialogFilter filter;
                filter.name = "Obj File";
                const c8* extension = "obj";
                filter.extensions = {&extension, 1};
                lulet(file_path, Window::open_file_dialog("Select Source File", {&filter, 1}));
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
                if (luerr != BasicError::interrupted())
                {
                    auto _ = Window::message_box(explain(luerr), "Failed to import obj file",
                        Window::MessageBoxType::ok, Window::MessageBoxIcon::error);
                }
                m_source_file_path.clear();

            }
        }

        if (m_source_file_path.empty())
        {
            GUI::text(context, "No obj file selected.");
        }
        else
        {
            GUI::begin_scroll_view(context, "Obj Mesh Importer Content", GUI::Size::fixed(604.0f, 650.0f));
            GUI::text(context, m_source_file_path.encode().c_str());
            GUI::text(context, "Object Information:");

            String vertex_count;
            String normal_count;
            String texcoord_count;
            String color_count;
            strprintf(vertex_count, "Vertex entries count: %u", (u32)m_obj_file.attributes.vertices.size());
            strprintf(normal_count, "Normal entries count: %u", (u32)m_obj_file.attributes.normals.size());
            strprintf(texcoord_count, "TexCoord entries count: %u", (u32)m_obj_file.attributes.texcoords.size());
            strprintf(color_count, "Color entries count: %u", (u32)m_obj_file.attributes.colors.size());
            GUI::text(context, vertex_count.c_str());
            GUI::text(context, normal_count.c_str());
            GUI::text(context, texcoord_count.c_str());
            GUI::text(context, color_count.c_str());

            if (m_obj_file.shapes.empty())
            {
                GUI::text(context, "No Shape information detected, this model cannot be imported.");
            }
            else
            {
                String mesh_count;
                strprintf(mesh_count, "%u meshes found", (u32)m_obj_file.shapes.size());
                GUI::text(context, mesh_count.c_str());
                if(GUI::is_item_clicked(GUI::button(context, "Import All")))
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
                GUI::ItemHandle shapes_header = GUI::collapsing_header(context, "Shapes");
                if (GUI::get_item_state(shapes_header, GUI::State::open()))
                {
                    for (u32 i = 0; i < (u32)m_obj_file.shapes.size(); ++i)
                    {
                        String shape_name;
                        String face_count;
                        strprintf(shape_name, "Name: %s", m_obj_file.shapes[i].name.c_str());
                        strprintf(face_count, "Faces: %u", (u32)m_obj_file.shapes[i].mesh.num_face_vertices.size());
                        GUI::text(context, shape_name.c_str());
                        GUI::text(context, face_count.c_str());

                        GUI::push_id(context, i);
                        GUI::input_text(context, "Asset Name", m_import_names[i]);
                        if (!m_import_names[i].empty())
                        {
                            Path file_path = m_create_dir;
                            file_path.push_back(m_import_names[i]);
                            String import_path;
                            strprintf(import_path, "The mesh will be imported as: %s", file_path.encode().c_str());
                            GUI::text(context, import_path.c_str());
                            if (GUI::is_item_clicked(GUI::button(context, "Import")))
                            {
                                import_static_mesh(file_path, m_obj_file, i);
                            }
                        }
                        GUI::pop_id(context);
                    }
                }
            }
            GUI::end_scroll_view(context);
        }
        GUI::end_window(context);
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
