/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file PreviewResources.hpp
* @author JXMaster
* @date 2026/9/11
*/
#pragma once
#include "WorkingDirectories.hpp"
#include "../Authoring.hpp"
#include <Luna/GameGUI/Instance.hpp>
#include <Luna/Runtime/Functional.hpp>
#include <Luna/Runtime/HashMap.hpp>
#include "PreviewResources.generated.hpp"

namespace Luna::GameGUIEditor
{
    struct [[Luna::struct("{88C2CDFC-213C-405D-A2EB-F76C2EB6790C}")]] PreviewResources
    {
        HashMap<Guid, ObjRef> objects;
        HashMap<Guid, String> errors;
    };

    R<GameGUI::InstanceDesc> prepare_editor_preview(const AuthoringDocument& source,
        Asset::asset_t asset, WorkingDirectories& directories,
        const Function<Ref<AuthoringDocument>(Asset::asset_t)>& working_copy);

    R<Ref<AuthoringDocument>> load_editor_authoring(Asset::asset_t asset);
}
