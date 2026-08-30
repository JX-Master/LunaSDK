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
#include <Luna/Frontend/Frontend.hpp>
#include <Luna/Runtime/UniquePtr.hpp>

#ifndef LUNA_GAME_GUI_EDITOR_SERVICE_API
#define LUNA_GAME_GUI_EDITOR_SERVICE_API
#endif

namespace Luna
{
    namespace GameGUIEditor
    {
        //! Stable Frontend URL used to create an untitled document.
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
        //! Stable Frontend URL used to save and bind a document to a path.
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
