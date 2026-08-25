/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file Internal.hpp
* @author JXMaster
* @date 2026/8/25
*/
#pragma once
#include "../GameGUI.hpp"
#include <Luna/Runtime/Guid.hpp>

namespace Luna
{
    namespace GameGUI
    {
        R<Variant> encode_document(const Document& document);
        R<Ref<Document>> decode_document(const Variant& data,
            Vector<Diagnostic>* diagnostics = nullptr);
        void get_direct_referred_assets(const Document& document,
            Vector<Asset::asset_t>& assets);
        void close_node_registry();
    }
}
