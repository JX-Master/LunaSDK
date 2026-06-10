/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file EditorService.hpp
* @author JXMaster
* @date 2026/6/10
*/
#pragma once
#include <Luna/Frontend/Frontend.hpp>
#include <Luna/GUIAsset/GUIAsset.hpp>
#include <Luna/Runtime/Path.hpp>
#include <Luna/Runtime/UniquePtr.hpp>
#include <Luna/VariantUtils/VariantUtils.hpp>

namespace Luna
{
    namespace GUIEditor
    {
        namespace GA = GUIAsset;

        struct EditorDocument;

        struct EditOp
        {
            String label;
            virtual ~EditOp() {}
            virtual RV undo(EditorDocument& document) = 0;
            virtual RV redo(EditorDocument& document) = 0;
        };

        struct EditorDocument
        {
            u64 id = 0;
            Ref<GA::Asset> asset;
            Path path;
            bool has_path = false;
            bool dirty = false;
            Guid selected_node = Guid(0, 0);
            Vector<UniquePtr<EditOp>> undo_stack;
            Vector<UniquePtr<EditOp>> redo_stack;
        };

        struct EditorService
        {
            Ref<Frontend::IFrontend> frontend;
            Vector<UniquePtr<EditorDocument>> documents;
            u64 next_document_id = 1;
            u64 active_document_id = 0;
            String last_status;

            RV init();
            EditorDocument* active_document();
            EditorDocument* find_document(u64 id);
            R<EditorDocument*> new_document();
            R<EditorDocument*> open_document(const Path& path);
            RV save_document(u64 document_id, const Path* path = nullptr);

            RV create_node(u64 document_id, const Guid& parent, const Name& type, const c8* label, usize index = USIZE_MAX);
            RV remove_node(u64 document_id, const Guid& node);
            RV move_node(u64 document_id, const Guid& node, const Guid& parent, usize index = USIZE_MAX);
            RV reorder_node(u64 document_id, const Guid& node, usize index);
            RV set_node_common(u64 document_id, const Guid& node, const c8* label, bool enabled, const Name& style);
            RV set_node_property(u64 document_id, const Guid& node, const Name& key, Variant&& value);
            RV erase_node_property(u64 document_id, const Guid& node, const Name& key);
            RV set_selection(u64 document_id, const Guid& node);
            RV undo(u64 document_id);
            RV redo(u64 document_id);

            Variant invoke(const Name& method, const Variant& params);
            void register_frontend_functions();
        };
    }
}
