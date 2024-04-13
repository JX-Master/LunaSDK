/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file ProjectSelector.cpp
* @author JXMaster
* @date 2020/4/20
*/
#include "ProjectSelector.hpp"
#include "StudioHeader.hpp"
#include <Luna/Runtime/Time.hpp>
#include <Luna/Runtime/File.hpp>
#include <Luna/VariantUtils/JSON.hpp>
#include <Luna/Window/FileDialog.hpp>
#include <Luna/Window/MessageBox.hpp>
#include <Luna/Runtime/Thread.hpp>

namespace Luna
{
    //! Creates project file at the specified directory.
    static R<Path> create_project_dir(const Path& dir_path, const String& project_name, bool should_create_dir)
    {
        if (project_name.size() == 0)
        {
            return set_error(BasicError::bad_arguments(), u8"Project name is empty.");
        }
        Path ret_path;
        lutry
        {
            auto project_path = Path();
            project_path.assign(dir_path);
            if (should_create_dir)
            {
                project_path.push_back(Name(project_name.c_str()));
                luexp(create_dir(project_path.encode().c_str()));
            }

            // Create Data folder.
            project_path.push_back("Data");
            luexp(create_dir(project_path.encode().c_str()));
            project_path.pop_back();
            project_path.push_back(project_name);
            project_path.append_extension("lunaproj");
            lulet(f, open_file(project_path.encode().c_str(), FileOpenFlag::write, FileCreationMode::create_always));
            f.reset();
            project_path.pop_back();
            ret_path = project_path;
        }
        lucatchret;
        return ret_path;
    }

    struct RecentFileRecord
    {
        u64 m_last_use_time;
        Path m_path;
    };

    void read_recents(Vector<RecentFileRecord>& recents)
    {
        lutry
        {
            lulet(f, open_file("RecentProjects.json", FileOpenFlag::read | FileOpenFlag::user_buffering, FileCreationMode::open_existing));
            lulet(blob, load_file_data(f));
            lulet(data, VariantUtils::read_json((c8*)blob.data(), blob.size()));
            for (auto& item : data.values())
            {
                RecentFileRecord rec;
                rec.m_path = item["path"].c_str();
                rec.m_last_use_time = item["last_use_time"].unum();
                auto attr = get_file_attribute(item["path"].c_str());
                if (succeeded(attr))
                {
                    recents.push_back(rec);
                }
            }
        }
        lucatch
        {
            return;
        }
    }

    void write_recents(Vector<RecentFileRecord>& recents, const Path& opened)
    {
        auto iter = recents.begin();
        if (!opened.empty())
        {
            bool insert = true;
            while (iter != recents.end())
            {
                if (iter->m_path.equal_to(opened))
                {
                    RecentFileRecord rec = *iter;
                    rec.m_last_use_time = get_utc_timestamp();
                    recents.erase(iter);
                    iter = recents.begin();
                    recents.insert(iter, rec);
                    insert = false;
                    break;
                }
                ++iter;
            }
            if (insert)
            {
                RecentFileRecord rec;
                rec.m_last_use_time = get_utc_timestamp();
                rec.m_path = opened;
                recents.insert(recents.begin(), rec);
            }
        }
        lutry
        {
            Variant var(VariantType::array);
            for (auto& i : recents)
            {
                Variant item(VariantType::object);
                item["path"] = i.m_path.encode();
                item["last_use_time"] = i.m_last_use_time;
                var.push_back(move(item));
            }
            String data = VariantUtils::write_json(var);
            lulet(f, open_file("RecentProjects.json", FileOpenFlag::write | FileOpenFlag::user_buffering, FileCreationMode::create_always));
            luexp(f->write(data.data(), data.size()));
        }
        lucatch
        {
            return;
        }
    }

    R<Path> select_project()
    {
        Path path;
        lutry
        {
            lulet(window, Window::new_window("Luna Studio - Open Project", Window::WindowDisplaySettings::as_windowed(Window::DEFAULT_POS, Window::DEFAULT_POS, 1000, 500)));
            lulet(swap_chain, g_env->device->new_swap_chain(g_env->graphics_queue, window, RHI::SwapChainDesc({0, 0, 2, RHI::Format::bgra8_unorm, true})));
            lulet(cmdbuf, g_env->device->new_command_buffer(g_env->graphics_queue));

            window->get_close_event().add_handler([](Window::IWindow* window) { window->close(); });

            // Create back buffer.
            u32 w = 0, h = 0;

            // Create ImGui context.
            ImGuiUtils::set_active_window(window);

            auto new_solution_name = String();

            Vector<RecentFileRecord> recents;
            read_recents(recents);

            bool create_dir = true;

            while (path.empty())
            {
                Window::poll_events();

                if (window->is_closed())
                {
                    break;
                }
                if (window->is_minimized())
                {
                    sleep(100);
                    continue;
                }

                // Recreate the back buffer if needed.
                auto fb_sz = window->get_framebuffer_size();
                if (fb_sz.x && fb_sz.y && (fb_sz.x != w || fb_sz.y != h))
                {
                    luexp(swap_chain->reset({fb_sz.x, fb_sz.y, 2, RHI::Format::unknown, true}));
                    f32 clear_color[] = { 0.0f, 0.0f, 0.0f, 1.0f };
                    w = fb_sz.x;
                    h = fb_sz.y;
                }
                auto sz = window->get_size();

                ImGuiUtils::update_io();
                ImGui::NewFrame();
                {
                    using namespace ImGui;
                    SetNextWindowPos({ 0.0f, 0.0f });
                    SetNextWindowSize({ (f32)sz.x, (f32)sz.y });
                    Begin("Luna Studio Project Selector", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);

                    if (CollapsingHeader("New Project", ImGuiTreeNodeFlags_DefaultOpen))
                    {
                        InputText("Project Name", new_solution_name);
                        Checkbox("Create Project Folder", &create_dir);
                        if (Button("Create New Project"))
                        {
                            auto rpath = Window::open_dir_dialog("Select Project Folder");
                            if (succeeded(rpath))
                            {
                                auto res2 = create_project_dir(rpath.get(), new_solution_name, create_dir);
                                if (succeeded(res2))
                                {
                                    path = res2.get();
                                }
                                else
                                {
                                    auto _ = Window::message_box(explain(res2.errcode()), "Project Creation Failed", Window::MessageBoxType::ok, Window::MessageBoxIcon::error);
                                }
                            }
                        }
                    }

                    if (CollapsingHeader("Open Existing Project", ImGuiTreeNodeFlags_DefaultOpen))
                    {
                        if (Button("Browse Project File"))
                        {
                            Window::FileDialogFilter filter;
                            filter.name = "Luna Project File";
                            const c8* extension = "lunaproj";
                            filter.extensions = {&extension, 1};
                            auto rpath = Window::open_file_dialog("Select Project File", {&filter, 1});
                            if (succeeded(rpath) && !rpath.get().empty())
                            {
                                path = rpath.get()[0];
                                path.pop_back();
                            }
                        }


                        if (!recents.empty())
                        {
                            // Show recent files.
                            PushStyleVar(ImGuiStyleVar_ChildRounding, 5.0f);
                            BeginChild("Recent Projects", { 0.0f, 0.0f }, true);

                            Text("Recent Projects");

                            Columns(4);

                            Text("Path");
                            NextColumn();
                            Text("Last Assess Date");
                            NextColumn();
                            NextColumn();
                            NextColumn();

                            SetColumnWidth(0, GetWindowContentRegionMax().x - GetWindowContentRegionMin().x - 430);
                            SetColumnWidth(1, 250);
                            SetColumnWidth(2, 80);
                            SetColumnWidth(3, 100);

                            auto iter = recents.begin();
                            while (iter != recents.end())
                            {
                                DateTime dt = timestamp_to_datetime(utc_timestamp_to_local_timestamp(iter->m_last_use_time));
                                Text("%s", iter->m_path.encode().c_str());
                                NextColumn();
                                Text("%hu/%hu/%hu %02hu:%02hu", dt.year, dt.month, dt.day, dt.hour, dt.minute);
                                NextColumn();
                                PushID(&(iter->m_path));
                                if (Button("Open"))
                                {
                                    path = iter->m_path;
                                }
                                NextColumn();
                                if (Button("Remove"))
                                {
                                    iter = recents.erase(iter);
                                    write_recents(recents, Path());
                                }
                                else
                                {
                                    ++iter;
                                }
                                PopID();
                                NextColumn();
                            }

                            EndChild();
                            PopStyleVar();
                        }
                    }

                    End();
                }
                ImGui::Render();
                Float4U clear_color = { 0.0f, 0.0f, 0.0f, 1.0f };

                RHI::RenderPassDesc render_pass;
                lulet(back_buffer, swap_chain->get_current_back_buffer());
                render_pass.color_attachments[0] = RHI::ColorAttachment(back_buffer, RHI::LoadOp::clear, RHI::StoreOp::store, clear_color);
                cmdbuf->begin_render_pass(render_pass);
                cmdbuf->end_render_pass();
                luexp(ImGuiUtils::render_draw_data(ImGui::GetDrawData(), cmdbuf, back_buffer));
                cmdbuf->resource_barrier({}, {
                    {back_buffer, RHI::TEXTURE_BARRIER_ALL_SUBRESOURCES, RHI::TextureStateFlag::automatic, RHI::TextureStateFlag::present, RHI::ResourceBarrierFlag::none}
                    });
                luexp(cmdbuf->submit({}, {}, true));
                cmdbuf->wait();
                luexp(cmdbuf->reset());
                luexp(swap_chain->present());
            }
            if (path.empty())
            {
                return BasicError::failure();
            }

            // Write to the recents.
            write_recents(recents, path);
        }
        lucatchret;
        return path;
    }
}
