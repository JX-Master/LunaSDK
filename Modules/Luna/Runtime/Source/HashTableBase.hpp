/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file HashTableBase.hpp
* @author JXMaster
* @date 2022/5/1
* @brief Defines common types used by both hash set and hash map.
*/
#pragma once

namespace Luna
{
	namespace Impl
	{
		// Extract key reference from value reference.

		template <typename _Kty, typename _Vty>
		struct MapExtractKey
		{
			const _Kty& operator()(const _Vty& p) const
			{
				return p.first;
			}
		};

		template <typename _Kty, typename _Vty>
		struct SetExtractKey
		{
			const _Kty& operator()(const _Vty& p) const
			{
				return p;
			}
		};
	}
}