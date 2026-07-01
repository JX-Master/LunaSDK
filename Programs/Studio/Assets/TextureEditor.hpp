/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*/
#pragma once
#include "Texture.hpp"
#include "TextureEditor.generated.hpp"

namespace Luna
{
    struct [[luna::struct("{E1F83CDB-D75C-4943-9428-AB1768C94677}")]] TextureEditor : public IAssetEditor
    {
        luiimpl();

        Asset::asset_t m_tex;

        bool m_open = true;

        TextureEditor() {}

        virtual void on_render(GUICore::IContext* context, const GUICore::LayoutConfig& layout) override;
        virtual bool closed() override
        {
            return !m_open;
        }
    };
}
