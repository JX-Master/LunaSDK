/*!
* This file is a portion of LunaSDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file MeshAsset.hpp
* @author JXMaster
* @date 2022/12/17
*/
#pragma once
#include "../StudioHeader.hpp"
#include "../Mesh.hpp"
#include "MeshAsset.generated.hpp"
namespace Luna
{
    Name get_static_mesh_asset_type();
    void register_static_mesh_asset_type();
    void register_static_mesh_importer();
    //! The format used to save mesh data.
    struct [[luna::struct("{8953365D-B966-48DC-8B15-3A156DA5ED04}")]] MeshAsset
    {
        [[Luna::property]] Vector<MeshPiece> pieces;
        [[Luna::property]] Blob vertex_data;
        [[Luna::property]] Blob index_data;
    };
}
