/*
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Mesh.hpp
* @author JXMaster
* @date 2022/12/17
*/
#pragma once
#include "../StudioHeader.hpp"
#include "../Mesh.hpp"

namespace Luna
{
	Name get_static_mesh_asset_type();
	void register_static_mesh_asset_type();
	void register_static_mesh_importer();

	//! The format used to save mesh data.
	struct MeshAsset
	{
		lustruct("MeshAsset", "{8953365D-B966-48DC-8B15-3A156DA5ED04}");
		Vector<MeshPiece> pieces;
		Blob vertex_data;
		Blob index_data;
	};
}