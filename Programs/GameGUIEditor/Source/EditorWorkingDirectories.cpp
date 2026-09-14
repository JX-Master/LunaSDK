/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file EditorWorkingDirectories.cpp
* @author JXMaster
* @date 2026/9/11
*/
#include "EditorApp.hpp"
#include <Luna/Window/FileDialog.hpp>
#include <Luna/Window/MessageBox.hpp>

namespace Luna::GameGUIEditor::Internal
{
    Path EditorApp::directory_native_path(u64 directory_id) const
    {
        for(const Variant& directory : working_directories.values())
            if(directory["working_directory_id"].unum() == directory_id)
                return Path(directory["native_path"].c_str());
        return Path();
    }

    bool EditorApp::refresh_working_directories()
    {
        Variant result;
        if(!invoke(LIST_DIRECTORIES_URL, Variant(VariantType::object), result)) return false;
        working_directories = move(result);
        bool selected_exists = false;
        for(const Variant& directory : working_directories.values())
            if(directory["working_directory_id"].unum() == selected_directory) selected_exists = true;
        if(!selected_exists)
        {
            selected_directory = working_directories.empty() ? 0 : working_directories[0]["working_directory_id"].unum();
            selected_folder.clear();
        }
        return true;
    }

    bool EditorApp::open_working_directory(const Path& path)
    {
        Path native = path;
        if(native.empty())
        {
            auto selected = Window::open_dir_dialog("Open Working Directory", directory_native_path(selected_directory));
            if(!selected.valid())
            {
                if(selected.errcode() != E_INTERRUPTED)
                {
                    error_message = explain(selected.errcode());
                    show_file_error("Open Directory Failed");
                }
                return false;
            }
            native = selected.get();
        }
        Variant params(VariantType::object);
        params["native_path"] = native.encode().c_str();
        Variant result;
        if(!invoke(OPEN_DIRECTORY_URL, params, result))
        {
            String error = error_message;
            refresh_working_directories();
            error_message = move(error);
            show_file_error("Open Directory Failed");
            return false;
        }
        selected_directory = result["working_directory_id"].unum();
        selected_folder.clear();
        return refresh_working_directories();
    }

    void EditorApp::synchronize_previews()
    {
        if(observed_preview_revision == service->preview_revision()) return;
        // Called at the frame boundary, after the previous renderer has consumed all callbacks.
        for(DocumentView& document : documents)
        {
            bool closing = false;
            for(u64 id : deferred_document_removals) if(id == document.id) closing = true;
            if(!closing) refresh_snapshot(document);
        }
        observed_preview_revision = service->preview_revision();
    }

    bool EditorApp::confirm_directory_close(u64 directory_id, Variant& plan)
    {
        Variant params(VariantType::object);
        params["working_directory_id"] = directory_id;
        if(!invoke(PREPARE_CLOSE_DIRECTORY_URL, params, plan)) return false;
        auto cancel = [&]()
        {
            String error = error_message;
            Variant ignored;
            invoke(CANCEL_CLOSE_DIRECTORY_URL, plan, ignored);
            directory_dialog_active = false;
            error_message = move(error);
        };
        directory_dialog_active = true;
        Vector<u64> to_save;
        // Collect every choice first. A later Cancel must not discard an earlier document.
        for(const Variant& metadata : plan["documents"].values())
        {
            if(!metadata["dirty"].boolean()) continue;
            const c8* buttons[] = {"Save", "Discard", "Cancel"};
            String message;
            strprintf(message, "Save changes to \"%s\" before unloading its working directory?",
                metadata["title"].c_str());
            auto response = Window::message_box(message.c_str(), "Unsaved Changes",
                {buttons, 3}, Window::MessageBoxIcon::warning, 0, 2);
            if(!response.valid() || response.get() == 2)
            {
                if(!response.valid()) error_message = explain(response.errcode());
                cancel();
                return false;
            }
            if(response.get() == 0) to_save.push_back(metadata["document_id"].unum());
        }
        for(u64 id : to_save)
        {
            auto document = find_document(id);
            if(!document || !save(*document, document->asset_path.empty(), plan["close_token"].unum()))
            {
                cancel();
                return false;
            }
        }
        Variant decisions(VariantType::array);
        for(DocumentView& document : documents)
        {
            if(document.working_directory_id != directory_id) continue;
            Variant decision = editing_params(document);
            decision["discard"] = document.dirty;
            decisions.push_back(move(decision));
        }
        plan["documents"] = move(decisions);
        directory_dialog_active = false;
        return true;
    }

    bool EditorApp::commit_directory_close(const Variant& plan)
    {
        // The service owns source state; GUI-owned prepared instances must also be released.
        for(DocumentView& document : documents)
        {
            document.preview.instance.reset();
            document.preview.revision = 0;
        }
        Variant result;
        if(!invoke(CLOSE_DIRECTORY_URL, plan, result))
        {
            String error = error_message;
            Variant ignored;
            invoke(CANCEL_CLOSE_DIRECTORY_URL, plan, ignored);
            refresh_working_directories();
            error_message = move(error);
            show_file_error("Unload Directory Failed");
            return false;
        }
        Vector<u64> removed;
        for(const DocumentView& document : documents)
            if(document.working_directory_id == plan["working_directory_id"].unum()) removed.push_back(document.id);
        for(u64 id : removed) remove_document_view(id);
        refresh_working_directories();
        return true;
    }

    void EditorApp::process_deferred_directory_closes()
    {
        for(const Variant& plan : deferred_directory_closes) commit_directory_close(plan);
        deferred_directory_closes.clear();
    }

    bool EditorApp::confirm_exit()
    {
        if(directory_dialog_active || !deferred_directory_closes.empty()) return false;
        Vector<Variant> plans;
        Vector<u64> directory_ids;
        for(const Variant& directory : working_directories.values())
            directory_ids.push_back(directory["working_directory_id"].unum());
        for(u64 directory_id : directory_ids)
        {
            Variant plan;
            if(!confirm_directory_close(directory_id, plan))
            {
                for(const Variant& previous : plans)
                {
                    Variant ignored;
                    invoke(CANCEL_CLOSE_DIRECTORY_URL, previous, ignored);
                }
                return false;
            }
            plans.push_back(move(plan));
        }
        // Window close callbacks run during poll_events, outside the GUI frame.
        for(usize i = 0; i < plans.size(); ++i)
        {
            if(commit_directory_close(plans[i])) continue;
            for(usize j = i + 1; j < plans.size(); ++j)
            {
                Variant ignored;
                invoke(CANCEL_CLOSE_DIRECTORY_URL, plans[j], ignored);
            }
            return false;
        }
        return true;
    }
}
