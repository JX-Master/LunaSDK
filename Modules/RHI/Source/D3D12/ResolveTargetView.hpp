/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file ResolveTargetView.hpp
* @author JXMaster
* @date 2023/5/16
*/
#include "Device.hpp"

namespace Luna
{
	namespace RHI
	{
		struct ResolveTargetView : IResolveTargetView
		{
			lustruct("RHI::ResolveTargetView", "{8E373CFD-A971-4F97-9C8D-78B33EF17A37}");
			luiimpl();

			Ref<Device> m_device;
			Name m_name;
			Ref<ITexture> m_resource;
			ResolveTargetViewDesc m_desc;

			RV init(ITexture* resource, const ResolveTargetViewDesc* desc);

			virtual IDevice* get_device() override { return m_device.get(); }
			virtual void set_name(const Name& name) override { m_name = name; }
			virtual ITexture* get_texture() override { return m_resource; }
			virtual ResolveTargetViewDesc get_desc() override { return m_desc; }
		};
	}
}