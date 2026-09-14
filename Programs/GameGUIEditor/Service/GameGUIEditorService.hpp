/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file GameGUIEditorService.hpp
* @author JXMaster
* @date 2026/8/26
*/
#pragma once
#include "Authoring.hpp"
#include <Luna/GameGUI/Instance.hpp>
#include <Luna/Frontend/Frontend.hpp>
#include <Luna/Runtime/UniquePtr.hpp>

#ifndef LUNA_GAME_GUI_EDITOR_SERVICE_API
#define LUNA_GAME_GUI_EDITOR_SERVICE_API
#endif

namespace Luna
{
    namespace GameGUIEditor
    {
        //! Opens native_path and returns its working_directory_id and private vfs_path.
        inline constexpr const c8* OPEN_DIRECTORY_URL = "/GameGUIEditor/WorkingDirectories/Open";
        //! Lists opened directories and their cached folders and asset records (all asset types).
        inline constexpr const c8* LIST_DIRECTORIES_URL = "/GameGUIEditor/WorkingDirectories/List";
        //! Explicitly reloads metadata and saved dependencies; rejects unsaved owned documents.
        inline constexpr const c8* REFRESH_DIRECTORY_URL = "/GameGUIEditor/WorkingDirectories/Refresh";
        //! Resolves an absolute native .json path within an opened directory, without mounting or loading.
        inline constexpr const c8* RESOLVE_PATH_URL = "/GameGUIEditor/WorkingDirectories/ResolvePath";
        //! Freezes working_directory_id and returns close_token plus documents to confirm.
        inline constexpr const c8* PREPARE_CLOSE_DIRECTORY_URL = "/GameGUIEditor/WorkingDirectories/PrepareClose";
        //! Cancels close_token and permits normal editing again; does not discard documents.
        inline constexpr const c8* CANCEL_CLOSE_DIRECTORY_URL = "/GameGUIEditor/WorkingDirectories/CancelClose";
        //! Closes a prepared directory after checking close_token and a documents array of
        //! document_id, expected_revision and explicit discard decisions. Preview holders must
        //! be released before calling. Failure retains documents and retryable directory state.
        inline constexpr const c8* CLOSE_DIRECTORY_URL = "/GameGUIEditor/WorkingDirectories/Close";
        //! Creates an untitled document in working_directory_id, optionally under relative_directory.
        //! The ID may be omitted only when exactly one directory is open.
        inline constexpr const c8* CREATE_DOCUMENT_URL = "/GameGUIEditor/Documents/Create";
        //! Stable Frontend URL used to open one GameGUI asset.
        inline constexpr const c8* OPEN_DOCUMENT_URL = "/GameGUIEditor/Documents/Open";
        //! Stable Frontend URL used to list open documents.
        inline constexpr const c8* LIST_DOCUMENTS_URL = "/GameGUIEditor/Documents/List";
        //! Stable Frontend URL used to fetch an immutable document snapshot.
        inline constexpr const c8* GET_SNAPSHOT_URL = "/GameGUIEditor/Documents/GetSnapshot";
        //! Stable Frontend URL used to apply one atomic semantic command batch.
        inline constexpr const c8* APPLY_COMMANDS_URL = "/GameGUIEditor/Documents/ApplyCommands";
        //! Stable Frontend URL used to undo one document history state.
        inline constexpr const c8* UNDO_URL = "/GameGUIEditor/Documents/Undo";
        //! Stable Frontend URL used to redo one document history state.
        inline constexpr const c8* REDO_URL = "/GameGUIEditor/Documents/Redo";
        //! Stable Frontend URL used to save a bound document.
        inline constexpr const c8* SAVE_URL = "/GameGUIEditor/Documents/Save";
        //! Saves and binds a document to an asset-base VFS path in an opened directory.
        //! Replacing another existing asset requires overwrite=true; an asset open in another
        //! document cannot be overwritten. During prepared close, saves require close_token.
        inline constexpr const c8* SAVE_AS_URL = "/GameGUIEditor/Documents/SaveAs";
        //! Stable Frontend URL used to cook the current authoring snapshot into the main data unit.
        inline constexpr const c8* COOK_URL = "/GameGUIEditor/Documents/Cook";
        //! Stable Frontend URL used to close one document.
        inline constexpr const c8* CLOSE_DOCUMENT_URL = "/GameGUIEditor/Documents/Close";
        //! Stable Frontend URL used to enumerate registered GameGUI node schemas.
        inline constexpr const c8* GET_NODE_TYPES_URL = "/GameGUIEditor/NodeTypes/List";

        class ServiceImpl;

        //! Owns the headless GameGUI editor document service and its Frontend interface.
        //! @remark The service and its Frontend are not thread-safe. All calls must be serialized
        //! on the owning application thread.
        class LUNA_GAME_GUI_EDITOR_SERVICE_API Service
        {
        public:
            Service(const Service&) = delete;
            Service& operator=(const Service&) = delete;
            ~Service();

            //! Gets the protocol-independent Frontend exposed by this service.
            Frontend::IFrontend* frontend() const;

            //! Gets the revision of the editor-wide preview dependency graph. Changes on edits,
            //! saves, document close and directory changes, but not on read-only queries.
            u64 preview_revision() const;
            //! Prepares an immutable root and no-I/O resource snapshot for one document.
            //! May load dependencies and cook Authoring in memory; call before GUI generation.
            //! Does not save data or publish working copies into Asset's main data unit.
            R<GameGUI::InstanceDesc> prepare_preview(u64 document_id);

        private:
            friend LUNA_GAME_GUI_EDITOR_SERVICE_API R<UniquePtr<Service>> new_service();
            Service();
            RV init();
            ServiceImpl* m_impl;
        };

        //! Creates a headless GameGUI editor service and installs all Frontend resources.
        LUNA_GAME_GUI_EDITOR_SERVICE_API R<UniquePtr<Service>> new_service();
    }
}
