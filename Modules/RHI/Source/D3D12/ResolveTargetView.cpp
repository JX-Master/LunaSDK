/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file ResolveTargetView.cpp
* @author JXMaster
* @date 2023/5/16
*/
#include "ResolveTargetView.hpp"
#include "Resource.hpp"
namespace Luna
{
	namespace RHI
	{
		RV ResolveTargetView::init(ITexture* resource, const ResolveTargetViewDesc* desc)
		{
			lutry
			{
				ResolveTargetViewDesc d;
				if (desc)
				{
					d = *desc;
				}
				else
				{
					d.mip_slice = 0;
					d.array_slice = 0;
				}
				m_resource = resource;
				m_desc = d;
			}
			lucatchret;
			return ok;
		}
	}
}