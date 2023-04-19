/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file Resource.hpp
* @author JXMaster
* @date 2023/4/19
*/
#include "Common.hpp"

namespace Luna
{
	namespace RHI
	{
		struct BufferResource : IResource
		{
			lustruct("RHI::BufferResource", "{2CE2F6F7-9CCB-4DD5-848A-DBE27F8A8B7A}");
		};

		struct TextureResource : IResource
		{
			lustruct("RHI::TextureResource", "{731F1D3C-2864-44A4-B380-CF03CBB7AFED}");
		};
	}
}