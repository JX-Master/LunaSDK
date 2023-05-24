/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file Fence.cpp
* @author JXMaster
* @date 2023/5/16
*/
#include "Fence.hpp"
namespace Luna
{
	namespace RHI
	{
		RV Fence::init()
		{
			lutry
			{
				luexp(encode_hresult(m_device->m_device->CreateFence(m_wait_value, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence))));
			}
			lucatchret;
			return ok;
		}
	}
}