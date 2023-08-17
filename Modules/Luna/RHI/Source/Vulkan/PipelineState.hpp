/*!
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
*
* @file PipelineState.hpp
* @author JXMaster
* @date 2023/4/23
*/
#pragma once
#include "Device.hpp"

namespace Luna
{
	namespace RHI
	{
		struct PipelineState : IPipelineState
		{
			lustruct("RHI::PipelineState", "{1033D006-D337-49FB-A307-1D22E143E825}");
			luiimpl();

			Ref<Device> m_device;
			Name m_name;
			VkPipeline m_pipeline = VK_NULL_HANDLE;

			RV init_as_graphics(const GraphicsPipelineStateDesc& desc);
			RV init_as_compute(const ComputePipelineStateDesc& desc);
			~PipelineState();

			virtual IDevice* get_device() override { return m_device.get(); }
			virtual void set_name(const c8* name) override { m_name = name; }
		};
	}
}