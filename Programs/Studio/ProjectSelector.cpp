/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file ProjectSelector.cpp
* @author JXMaster
* @date 2020/4/20
*/
#include "ProjectSelector.hpp"
#include "StudioHeader.hpp"
#include <Luna/GUIWindow/GUIWindow.hpp>
#include <Luna/Runtime/Time.hpp>
#include <Luna/Runtime/File.hpp>
#include <Luna/VariantUtils/JSON.hpp>
#include <Luna/Window/FileDialog.hpp>
#include <Luna/Window/MessageBox.hpp>
#include <Luna/Runtime/Thread.hpp>
#include <Luna/Window/Event.hpp>

namespace Luna
{
    //! Creates project file at the specified directory.
    static R<Path> create_project_dir(const Path& dir_path, const String& project_name, bool should_create_dir)
    {
        if (project_name.size() == 0)
        {
            return set_error(BasicError::bad_arguments(), "Project name is empty.");
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
            lulet(window, Window::new_window("Luna Studio - Open Project", Window::DEFAULT_POS, Window::DEFAULT_POS, 1000, 500));
            lulet(swap_chain, g_env->device->new_swap_chain(g_env->graphics_queue, window, RHI::SwapChainDesc({0, 0, 2, RHI::Format::bgra8_unorm, true})));
            lulet(cmdbuf, g_env->device->new_command_buffer(g_env->graphics_queue));
            Ref<GUI::IGUIContext> gui = GUI::new_context(g_env->device);

            // Create back buffer.
            u32 w = 0, h = 0;

            GUIWindow::GUIWindowInputAdapter input_adapter;
            input_adapter.window = window;
            input_adapter.gui = gui;
            GUIWindow::install_window_event_handler(&input_adapter);

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

                GUI::GUIFrameDesc frame;
                frame.surface_size = Float2U((f32)sz.x, (f32)sz.y);
                frame.framebuffer_size = fb_sz;
                frame.dpi_scale = window->get_dpi_scale_factor();
                frame.delta_time = 1.0f / 60.0f;
                gui->begin_frame(frame);

                GUI::GUIItemHandle create_project_button;
                GUI::GUIItemHandle browse_project_button;
                Vector<GUI::GUIItemHandle> recent_open_buttons;
                Vector<GUI::GUIItemHandle> recent_remove_buttons;

                GUI::BeginWindow("Luna Studio Project Selector", GUI::GUISize::fixed((f32)sz.x, (f32)sz.y));
                {
                    GUI::GUIItemHandle new_project = GUI::CollapsingHeader("New Project");
                    if (GUI::GetItemState(new_project, GUI::GUIState::open()))
                    {
                        GUI::Text("Project Name");
                        GUI::InputText("Project Name", new_solution_name);
                        GUI::Checkbox("Create Project Folder", &create_dir);
                        create_project_button = GUI::Button("Create New Project");
                    }

                    GUI::GUIItemHandle open_project = GUI::CollapsingHeader("Open Existing Project");
                    if (GUI::GetItemState(open_project, GUI::GUIState::open()))
                    {
                        browse_project_button = GUI::Button("Browse Project File");

                        if (!recents.empty())
                        {
                            GUI::Text("Recent Projects");
                            f32 recent_h = max((f32)sz.y - 260.0f, 120.0f);
                            f32 recent_w = max((f32)sz.x - 32.0f, 120.0f);
                            GUI::BeginScrollView("Recent Projects", GUI::GUISize::fixed(recent_w, recent_h));
                            GUI::GUITableDesc recent_table;
                            recent_table.columns = 4;
                            recent_table.style.padding = GUI::GUIEdgeInsets::xy(8.0f, 4.0f);
                            recent_table.style.border_size = 1.0f;
                            recent_table.style.background_mode = GUI::GUITableBackgroundMode::alternate_rows;
                            recent_table.style.background_color = Float4U(0.08f, 0.10f, 0.12f, 0.72f);
                            recent_table.style.alternate_background_color = Float4U(0.12f, 0.14f, 0.17f, 0.72f);
                            recent_table.style.row_separators = true;
                            recent_table.style.column_separators = true;
                            recent_table.style.resize_fixed_columns = true;
                            f32 recent_table_w = max(recent_w - 16.0f, 120.0f);
                            recent_table.column_sizes.push_back(GUI::GUITableTrackSize::fixed(max(recent_table_w - 286.0f, 160.0f)));
                            recent_table.column_sizes.push_back(GUI::GUITableTrackSize::fixed(120.0f));
                            recent_table.column_sizes.push_back(GUI::GUITableTrackSize::fixed(72.0f));
                            recent_table.column_sizes.push_back(GUI::GUITableTrackSize::fixed(88.0f));
                            GUI::BeginTableLayout("Recent Project Table", recent_table);
                            for(usize i = 0; i < recents.size(); ++i)
                            {
                                DateTime dt = timestamp_to_datetime(utc_timestamp_to_local_timestamp(recents[i].m_last_use_time));
                                String time_text;
                                strprintf(time_text, "%hu/%hu/%hu %02hu:%02hu", dt.year, dt.month, dt.day, dt.hour, dt.minute);
                                GUI::PushID((u64)i);
                                GUI::Text(recents[i].m_path.encode().c_str());
                                GUI::Text(time_text.c_str());
                                recent_open_buttons.push_back(GUI::Button("Open"));
                                recent_remove_buttons.push_back(GUI::Button("Remove"));
                                GUI::PopID();
                            }
                            GUI::EndTableLayout();
                            GUI::EndScrollView();
                        }
                        else
                        {
                            GUI::Text("No recent projects.");
                        }
                    }
                }
                GUI::EndWindow();

                lulet(gui_desc, gui->end_build());
                luexp(gui->submit(gui_desc));
                luexp(GUIWindow::update_text_input(&input_adapter));

                if (GUI::IsItemClicked(create_project_button))
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

                if (GUI::IsItemClicked(browse_project_button))
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

                usize remove_recent_index = USIZE_MAX;
                for(usize i = 0; i < recent_open_buttons.size(); ++i)
                {
                    if (i < recents.size() && GUI::IsItemClicked(recent_open_buttons[i]))
                    {
                        path = recents[i].m_path;
                    }
                    if (i < recents.size() && GUI::IsItemClicked(recent_remove_buttons[i]))
                    {
                        remove_recent_index = i;
                    }
                }
                if(remove_recent_index != USIZE_MAX)
                {
                    recents.erase(recents.begin() + remove_recent_index);
                    write_recents(recents, Path());
                }

                Float4U clear_color = { 0.0f, 0.0f, 0.0f, 1.0f };

                RHI::RenderPassDesc render_pass;
                lulet(back_buffer, swap_chain->get_current_back_buffer());
                render_pass.color_attachments[0] = RHI::ColorAttachment(back_buffer, RHI::LoadOp::clear, RHI::StoreOp::store, clear_color);
                cmdbuf->begin_render_pass(render_pass);
                cmdbuf->end_render_pass();
                luexp(gui->render(cmdbuf, back_buffer));
                cmdbuf->resource_barrier({}, {
                    {back_buffer, RHI::TEXTURE_BARRIER_ALL_SUBRESOURCES, RHI::TextureStateFlag::automatic, RHI::TextureStateFlag::present, RHI::ResourceBarrierFlag::none}
                    });
                luexp(cmdbuf->submit({}, {}, true));
                cmdbuf->wait();
                luexp(cmdbuf->reset());
                luexp(swap_chain->present());
            }
            GUIWindow::uninstall_window_event_handler(&input_adapter);
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
