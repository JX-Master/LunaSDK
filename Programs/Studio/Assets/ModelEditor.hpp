/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*/
#pragma once
#include "Model.hpp"
#include "../StudioHeader.hpp"
#include "../Mesh.hpp"
#include "ModelEditor.generated.hpp"

namespace Luna
{
    class [[luna::struct("{46d8b09d-1d7d-4deb-95b1-ac008c7998d4}")]] ModelEditor : public IAssetEditor
    {
    public:
        luiimpl();

        Asset::asset_t m_model;

        String m_mesh_name;

        Vector<String> m_mat_names;

        bool m_open = true;

        ModelEditor() {}

        virtual void on_render(GUI::IContext* context) override;
        virtual bool closed() override
        {
            return !m_open;
        }
    };
}
