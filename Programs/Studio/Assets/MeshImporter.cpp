/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file MeshImporter.cpp
* @author JXMaster
* @date 2022/12/17
*/
#include "MeshAsset.hpp"
#include "MeshImporter.hpp"
#include <Luna/GUI/Legacy/Editor.hpp>
#include <Luna/ObjLoader/ObjLoader.hpp>
#include <Luna/Window/FileDialog.hpp>
#include <Luna/Window/MessageBox.hpp>
#include <Luna/Runtime/File.hpp>
#include <Luna/VFS/VFS.hpp>
#include <Luna/Runtime/Serialization.hpp>
#include <Luna/VariantUtils/JSON.hpp>
namespace Luna
{
    namespace
    {
        GUICore::LayoutConfig fixed_height(f32 height)
        {
            GUICore::LayoutConfig layout;
            layout.width.kind = GUICore::SizeKind::percent;
            layout.width.value = 1.0f;
            layout.height.kind = GUICore::SizeKind::fixed;
            layout.height.value = height;
            return layout;
        }

        GUICore::LayoutConfig fill_layout()
        {
            GUICore::LayoutConfig layout;
            layout.width.kind = GUICore::SizeKind::percent;
            layout.width.value = 1.0f;
            layout.height.kind = GUICore::SizeKind::percent;
            layout.height.value = 1.0f;
            layout.flex_grow = 1.0f;
            return layout;
        }

        GUICore::FlexLayoutDesc vertical_layout(f32 gap = 6.0f)
        {
            GUICore::FlexLayoutDesc desc;
            desc.axis = GUICore::LayoutAxis::y;
            desc.main_axis_gap = gap;
            return desc;
        }

        RV select_obj_mesh_file(Path& source_file_path, ObjLoader::ObjMesh& obj_file, Vector<String>& import_names)
        {
            lutry
            {
                Window::FileDialogFilter filter;
                filter.name = "Obj File";
                const c8* extension = "obj";
                filter.extensions = {&extension, 1};
                lulet(file_path, Window::open_file_dialog("Select Source File", {&filter, 1}));
                Path path = file_path[0];

                lulet(obj_file_handle, open_file(path.encode(PathSeparator::system_preferred).c_str(),
                    FileOpenFlag::read | FileOpenFlag::user_buffering, FileCreationMode::open_existing));
                lulet(obj_file_data, load_file_data(obj_file_handle));

                path.replace_extension("mtl");
                auto f = open_file(path.encode(PathSeparator::system_preferred).c_str(),
                    FileOpenFlag::read | FileOpenFlag::user_buffering, FileCreationMode::open_existing);

                Blob mtl_file_data;
                if(succeeded(f))
                {
                    luset(mtl_file_data, load_file_data(f.get()));
                }

                luset(obj_file, ObjLoader::load(obj_file_data.cspan(), mtl_file_data.cspan()));
                source_file_path = file_path[0];

                import_names.clear();
                for(auto& i : obj_file.shapes)
                {
                    import_names.push_back(String(i.name.c_str()));
                }
            }
            lucatchret;
            return ok;
        }
    }

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

    void MeshImporter::on_render(GUICore::IContext* context, const GUICore::LayoutConfig& layout)
    {
        if(!m_open)
        {
            return;
        }
        context->push_data_scope(context->make_id((GUICore::id_t)(usize)this));
        GUI::DockPanelStyle panel_style;
        panel_style.min_floating_size = Float2U(500.0f, 420.0f);
        GUICore::ElementHandle panel;
        if(!GUI::begin_dock_panel(context, context->make_id("obj_mesh_importer"), "Obj Mesh Importer", &m_open,
            panel_style, layout, &panel))
        {
            context->pop_data_scope();
            return;
        }

        GUICore::ElementHandle select_source = GUI::text_button(context, context->make_id("select_source"),
            "Select Source File", fixed_height(30.0f));
        if(GUI::is_item_clicked(context, select_source))
        {
            RV r = select_obj_mesh_file(m_source_file_path, m_obj_file, m_import_names);
            if(failed(r) && r.errcode() != BasicError::interrupted())
            {
                auto _ = Window::message_box(explain(r.errcode()), "Failed to import obj file",
                    Window::MessageBoxType::ok, Window::MessageBoxIcon::error);
                m_source_file_path.clear();
            }
        }

        if(m_source_file_path.empty())
        {
            GUI::text(context, context->make_id("empty"), "No obj file selected.", fixed_height(26.0f));
        }
        else
        {
            GUICore::ElementHandle scroll = GUI::begin_scroll_view(context, context->make_id("scroll"),
                "Obj Mesh Importer Content", fill_layout());
            GUICore::ElementHandle content = GUI::begin_v_layout(context, context->make_id("content"),
                "Obj Mesh Importer Content", fill_layout());
            GUI::text(context, context->make_id("path"), m_source_file_path.encode().c_str(), fixed_height(24.0f));
            GUI::text(context, context->make_id("object_info"), "Object Information:", fixed_height(24.0f));

            String vertex_count;
            String normal_count;
            String texcoord_count;
            String color_count;
            strprintf(vertex_count, "Vertex entries count: %u", (u32)m_obj_file.attributes.vertices.size());
            strprintf(normal_count, "Normal entries count: %u", (u32)m_obj_file.attributes.normals.size());
            strprintf(texcoord_count, "TexCoord entries count: %u", (u32)m_obj_file.attributes.texcoords.size());
            strprintf(color_count, "Color entries count: %u", (u32)m_obj_file.attributes.colors.size());
            GUI::text(context, context->make_id("vertex_count"), vertex_count.c_str(), fixed_height(22.0f));
            GUI::text(context, context->make_id("normal_count"), normal_count.c_str(), fixed_height(22.0f));
            GUI::text(context, context->make_id("texcoord_count"), texcoord_count.c_str(), fixed_height(22.0f));
            GUI::text(context, context->make_id("color_count"), color_count.c_str(), fixed_height(22.0f));

            if(m_obj_file.shapes.empty())
            {
                GUI::text(context, context->make_id("no_shapes"),
                    "No Shape information detected, this model cannot be imported.", fixed_height(26.0f));
            }
            else
            {
                String mesh_count;
                strprintf(mesh_count, "%u meshes found", (u32)m_obj_file.shapes.size());
                GUI::text(context, context->make_id("mesh_count"), mesh_count.c_str(), fixed_height(24.0f));
                GUICore::ElementHandle import_all = GUI::text_button(context, context->make_id("import_all"),
                    "Import All", fixed_height(30.0f));
                if(GUI::is_item_clicked(context, import_all))
                {
                    for(u32 i = 0; i < (u32)m_obj_file.shapes.size(); ++i)
                    {
                        if(!m_import_names[i].empty())
                        {
                            Path file_path = m_create_dir;
                            file_path.push_back(m_import_names[i]);
                            import_static_mesh(file_path, m_obj_file, i);
                        }
                    }
                }
                if(GUI::collapsing_header(context, context->make_id("shapes"), "Shapes"))
                {
                    for(u32 i = 0; i < (u32)m_obj_file.shapes.size(); ++i)
                    {
                        context->push_data_scope(context->make_id((GUICore::id_t)i));
                        String shape_name;
                        String face_count;
                        strprintf(shape_name, "Name: %s", m_obj_file.shapes[i].name.c_str());
                        strprintf(face_count, "Faces: %u", (u32)m_obj_file.shapes[i].mesh.num_face_vertices.size());
                        GUI::text(context, context->make_id("shape_name"), shape_name.c_str(), fixed_height(22.0f));
                        GUI::text(context, context->make_id("face_count"), face_count.c_str(), fixed_height(22.0f));
                        GUI::text(context, context->make_id("asset_name_label"), "Asset Name", fixed_height(20.0f));
                        GUI::input_text(context, context->make_id("asset_name"), m_import_names[i], fixed_height(28.0f));
                        if(!m_import_names[i].empty())
                        {
                            Path file_path = m_create_dir;
                            file_path.push_back(m_import_names[i]);
                            String import_path;
                            strprintf(import_path, "The mesh will be imported as: %s", file_path.encode().c_str());
                            GUI::text(context, context->make_id("import_path"), import_path.c_str(), fixed_height(24.0f));
                            GUICore::ElementHandle import_button = GUI::text_button(context, context->make_id("import"),
                                "Import", fixed_height(30.0f));
                            if(GUI::is_item_clicked(context, import_button))
                            {
                                import_static_mesh(file_path, m_obj_file, i);
                            }
                        }
                        context->pop_data_scope();
                    }
                }
            }
            lupanic_if_failed(GUI::end_v_layout(context, content, vertical_layout(6.0f)));
            lupanic_if_failed(GUI::end_scroll_view(context, scroll));
        }

        GUI::end_dock_panel(context);
        context->pop_data_scope();
    }

    static Ref<IAssetEditor> new_static_mesh_importer(const Path& create_dir)
    {
        auto importer = new_object<MeshImporter>();
        importer->m_create_dir = create_dir;
        return importer;
    }

    void register_static_mesh_importer()
    {
        AssetImporterDesc desc;
        desc.new_importer = new_static_mesh_importer;
        g_env->register_asset_importer_type(get_static_mesh_asset_type(), desc);
    }
}
