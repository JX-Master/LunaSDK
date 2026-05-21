/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*/
#pragma once
#include "MeshAsset.hpp"
#include <Luna/ObjLoader/ObjLoader.hpp>
#include "MeshImporter.generated.hpp"

namespace Luna
{
    struct [[luna::struct("{770ac671-c013-4b89-a0a2-ab222e919a35}")]] MeshImporter : public IAssetEditor
    {
        luiimpl();

        Path m_create_dir;

        Path m_source_file_path;

        ObjLoader::ObjMesh m_obj_file;

        Vector<String> m_import_names;

        MeshImporter() {}

        bool m_open = true;

        virtual void on_render() override;
        virtual bool closed() override
        {
            return !m_open;
        }
    };
}
