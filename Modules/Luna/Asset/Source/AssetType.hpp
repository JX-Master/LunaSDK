/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file AssetType.hpp
* @author JXMaster
* @date 2022/7/25
*/
#pragma once
#include "../Asset.hpp"
#include <Luna/Runtime/Mutex.hpp>

namespace Luna
{
	namespace Asset
	{
		R<AssetTypeDesc> get_asset_type_desc(const Name& name);
		void init_asset_type();
		void close_asset_type();
		extern Ref<IMutex> g_asset_types_mutex;
	}
}