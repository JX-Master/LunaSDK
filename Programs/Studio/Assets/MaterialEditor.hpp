/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*/
#pragma once
#include "Material.hpp"
#include "../StudioHeader.hpp"
#include "../EditObject.hpp"
#include "MaterialEditor.generated.hpp"

namespace Luna
{
    struct [[luna::struct("{705b8d2f-75ef-4784-a72e-f99dcf3f67aa}")]] MaterialEditor : public IAssetEditor
    {
        luiimpl();

        Asset::asset_t m_material;

        String m_base_color_name;
        String m_roughness_name;
        String m_normal_name;
        String m_metallic_name;
        String m_emissive_name;

        bool m_open = true;

        MaterialEditor() {}

        virtual void on_render(GUI::IContext* context, const GUI::LayoutConfig& layout) override;
        virtual bool closed() override
        {
            return !m_open;
        }
    };
}
