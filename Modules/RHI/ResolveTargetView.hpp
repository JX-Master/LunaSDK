/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file ResolveTargetView.hpp
* @author JXMaster
* @date 2023/4/21
*/
#pragma once
#include "DeviceChild.hpp"
#include "Resource.hpp"

namespace Luna
{
	namespace RHI
	{
		struct ResolveTargetViewDesc
		{
			u32 mip_slice;
			u32 array_slice;
		};

		struct IResolveTargetView : virtual IDeviceChild
		{
			luiid("{17AAA0B8-2CAB-40B0-A60E-7D38F28EC7EA}");

			virtual ITexture* get_texture() = 0;
			virtual ResolveTargetViewDesc get_desc() = 0;
		};
	}
}