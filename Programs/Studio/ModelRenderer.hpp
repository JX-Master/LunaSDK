/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file ModelRenderer.hpp
* @author JXMaster
* @date 2022/12/17
*/
#pragma once
#include <Luna/Runtime/TypeInfo.hpp>
#include <Luna/Asset/Asset.hpp>

namespace Luna
{
	struct ModelRenderer
	{
		lustruct("ModelRenderer", "{27C69426-9BFB-4558-9904-9C5A05727E8C}");
		Asset::asset_t model;
	};
}