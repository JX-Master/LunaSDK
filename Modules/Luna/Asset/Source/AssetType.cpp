/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file AssetType.cpp
* @author JXMaster
* @date 2022/7/25
*/
#include <Luna/Runtime/PlatformDefines.hpp>
#define LUNA_ASSET_API LUNA_EXPORT
#include "AssetType.hpp"
#include <Luna/Runtime/Mutex.hpp>
#include <Luna/Runtime/SelfIndexedHashMap.hpp>

namespace Luna
{
	namespace Asset
	{
		struct AssetTypeDescExtractKey
		{
			Name operator()(const AssetTypeDesc& v)
			{
				return v.name;
			}
		};

		Ref<IMutex> g_asset_types_mutex;
		SelfIndexedHashMap<Name, AssetTypeDesc, AssetTypeDescExtractKey> g_asset_types;

		void init_asset_type()
		{
			g_asset_types_mutex = new_mutex();
		}
		void close_asset_type()
		{
			g_asset_types.clear();
			g_asset_types.shrink_to_fit();
		}
		LUNA_ASSET_API void register_asset_type(const AssetTypeDesc& desc)
		{
			MutexGuard g(g_asset_types_mutex);
			g_asset_types.insert_or_assign(desc);
		}
		R<AssetTypeDesc> get_asset_type_desc(const Name& name)
		{
			MutexGuard g(g_asset_types_mutex);
			auto iter = g_asset_types.find(name);
			if (iter == g_asset_types.end()) return AssetError::unknown_asset_type();
			return *iter;
		}
	}
}