/*
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file Mesh.hpp
* @author JXMaster
* @date 2020/5/10
*/
#pragma once
#include <Runtime/Math/Vector.hpp>
#include <RHI/RHI.hpp>

namespace Luna
{
	//! The vertex format.
	struct Vertex
	{
		lustruct("Vertex", "{6E4BB6E5-8619-47B8-9BFD-394BC8FB739C}");

		Float3U position;
		Float3U normal;
		Float3U tangent;
		Float2U texcoord;
		Float4U color;
	};

	static_assert(sizeof(Vertex) == 60, "Wrong Vertex struct size");

	struct MeshPiece
	{
		lustruct("MeshPiece", "{C79DC405-C231-4140-8E55-7EE0A1E882B1}");

		u32 first_index_offset; // The index of first u32's in the buffer.
		u32 num_indices; // Number of u32's
	};

	struct Mesh
	{
		lustruct("Mesh", "{1552A9F4-6DDD-4CC5-919F-48E1DEFF5A5B}");

		Ref<RHI::IResource> vb;
		Ref<RHI::IResource> ib;
		//! The number of vertices in vertex buffer.
		u32 vb_count;
		//! The number of indices in index buffer.
		u32 ib_count;

		//! Every piece of the mesh can be assigned with a different material.
		Vector<MeshPiece> pieces;
	};
}